#include "gpio.h"
#include "uart.h"
#include "string.h"
#include "shell.h"
#include "timer.h"
#include "exception.h"

tq node[TIMER_QUEUE_SIZE];
int timer_front = 0;
int timer_back = 0;

void get_nowtime()
{
    long cntpct_el0, cntfrq_el0;

    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        "mrs %1, cntfrq_el0\n\t"
        : "=r"(cntpct_el0), "=r"(cntfrq_el0)::);

    long nowtime = cntpct_el0 / cntfrq_el0;

    char NOWTIME[10];

    itoa(nowtime, NOWTIME, 1);

    uart_puts("Now time is ");
    uart_puts(NOWTIME);
    uart_puts(" s\n");
}

void add_timer(int sec, char *mes)
{
    get_nowtime();

    if (is_full())
        return;

    for (int i = 0; i < 21; i++)
        node[timer_back].message[i] = mes[i];

    // CNTFRQ_EL0 : 62500000

    // transfer sec to frq and store to node.second
    disable_interrupt();

    asm volatile(
        "mrs x3, cntfrq_el0\n\t"
        "mrs x4, cntpct_el0\n\t"
        "mov x2, %1\n\t" // after secs seconds later will interrupt
        "mul x2, x2, x3\n\t"
        "add x2, x2, x4\n\t"
        "mov %0, x2\n\t"
        : "=r"(node[timer_back].second)
        : "r"(sec)
        : "x0", "x1", "x2", "x3", "x4", "memory");

    timer_back = (timer_back + 1) % TIMER_QUEUE_SIZE;

    find_min();
    asm volatile(
        "msr cntp_cval_el0, %0\n\t"
        :
        : "r"(node[timer_front].second)
        : "x0", "x1");

    core_timer_enable();
    enable_interrupt();
}

int is_full()
{
    if ((timer_back + 1) % TIMER_QUEUE_SIZE == timer_front)
        return 1;
    return 0;
}

int is_empty()
{
    if (timer_front == timer_back)
        return 1;
    return 0;
}

void find_min()
{
    int tmp_back = timer_back;
    if (tmp_back < timer_front)
        tmp_back += TIMER_QUEUE_SIZE;
    for (int i = timer_front; i < tmp_back; i++)
    {
        int cmp = i % TIMER_QUEUE_SIZE;
        if (node[cmp].second < node[timer_front].second)
        {
            long long sec_tmp;
            char mes_tmp[21];

            sec_tmp = node[timer_front].second;
            node[timer_front].second = node[cmp].second;
            node[cmp].second = sec_tmp;

            for (int j = 0; j < 21; j++)
            {
                mes_tmp[j] = node[timer_front].message[j];
                node[timer_front].message[j] = node[cmp].message[j];
                node[cmp].message[j] = mes_tmp[j];
            }
        }
    }
}

void delay(long seconds)
{
    long cntpct_el0, cntfrq_el0, nowtime, due;

    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        "mrs %1, cntfrq_el0\n\t"
        : "=r"(cntpct_el0), "=r"(cntfrq_el0)::);

    due = cntpct_el0 / cntfrq_el0 + seconds;
    nowtime = cntpct_el0 / cntfrq_el0;

    while (nowtime <= due)
    {
        asm volatile(
            "mrs %0, cntpct_el0\n\t"
            "mrs %1, cntfrq_el0\n\t"
            : "=r"(cntpct_el0), "=r"(cntfrq_el0)::);
        nowtime = cntpct_el0 / cntfrq_el0;
    }
}