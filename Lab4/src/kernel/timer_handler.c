#include "gpio.h"
#include "uart.h"
#include "string.h"
#include "shell.h"
#include "timer_handler.h"

tq node[10];
int front=0;
int back=0;

void get_nowtime()
{
    long cntpct_el0, cntfrq_el0;

    asm volatile
    (
        "mrs %0, cntpct_el0\n\t"
        "mrs %1, cntfrq_el0\n\t"
        :"=r"(cntpct_el0), "=r"(cntfrq_el0)
        ::
    );

    long nowtime = cntpct_el0/cntfrq_el0;

    char NOWTIME[10];

    itoa(nowtime, NOWTIME, 1);

    uart_puts("Now time is ");
    uart_puts(NOWTIME);
    uart_puts(" s\n");
}


void add_timer(int sec, char* mes)
{
    get_nowtime();

    for(int i= 0 ; i<21 ; i++)
        node[back].message[i] = mes[i];
    
    // CNTFRQ_EL0 : 62500000

    // transfer sec to frq and store to node.second
    asm volatile
    (
        "msr DAIFSet, 0xf\n\t"
        "mrs x3, cntfrq_el0\n\t"
        "mrs x4, cntpct_el0\n\t"
        "mov x2, %1\n\t" // after secs seconds later will interrupt
        "mul x2, x2, x3\n\t"
        "add x2, x2, x4\n\t"
        "mov %0, x2\n\t"
        :"=r"(node[back].second)
        :"r"(sec)
        :"x0", "x1", "x2", "x3", "x4", "memory"
    );

    back++;

    find_min(0);
    asm volatile
    (
        "msr cntp_cval_el0, %0\n\t"
        "bl core_timer_enable\n\t"
        "msr DAIFClr, 0xf\n\t"
        :
        :"r"(node[front].second)
        :"x0", "x1"
    );
}

int is_full()
{
    if(back==9) return 1;
    return 0;
}

int is_empty()
{
    if(front==back) return 1;
    return 0;
}

void find_min()
{  
    for(int i=front ; i<back ; i++)
    {
        if(node[i].second < node[front].second)
        {
            long long sec_tmp;
            char mes_tmp[21];
            
            sec_tmp = node[front].second;
            node[front].second = node[i].second;
            node[i].second = sec_tmp;
            
            for(int j=0 ; j<21 ; j++)
            {
                mes_tmp[j] = node[front].message[j];
                node[front].message[j] = node[i].message[j];
                node[i].message[j] = mes_tmp[j];
            }
        }
    }
}