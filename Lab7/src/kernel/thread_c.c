#include "cpio.h"
#include "thread.h"
#include "syscall.h"
#include "buddy_sys.h"
#include "slub_sys.h"
#include "exception.h"
#include "timer.h"
#include "printf.h"

/* thread node */
thread_t *zombie_thread = NULL;
thread_t kernel_thread = {0}; // as idle_thread
long threadcnt = 0;

/* run queue */
thread_t *rq_head = NULL, *rq_tail = NULL;

void schedule()
{
    thread_t *current = get_current();
    thread_t *next = del_rq();

    if (next == NULL)
        next = &kernel_thread;

    if (current != &kernel_thread)
        add_rq(current);
    set_switch_timer();
    enable_interrupt();

    if (next->status == FORKING)
    {
        add_rq(next);
        switch_to(current, &kernel_thread);
    }
    else if (next->status == ZOMBIE)
    {
        next->next = zombie_thread;
        zombie_thread = next;
        switch_to(current, &kernel_thread);
    }
    else
    {
        switch_to(current, next);
    }
}

thread_t *thread_create(threadtask task)
{
    thread_t *newthread = (thread_t *)malloc(sizeof(thread_t));

    add_rq(newthread);

    newthread->kstack_start = (unsigned long)malloc(MIN_PAGE_SIZE);
    newthread->ustack_start = (unsigned long)malloc(MIN_PAGE_SIZE);
    newthread->usrpgm_load_addr = USRPGM_BASE + threadcnt * USRPGM_SIZE;

    newthread->context.fp = newthread->kstack_start + MIN_PAGE_SIZE;
    newthread->context.lr = (unsigned long)task_wrapper;
    newthread->context.sp = newthread->kstack_start + MIN_PAGE_SIZE; // stack will grow downward
    newthread->id = threadcnt++;
    newthread->status = READY;
    newthread->task = task;
    newthread->next = NULL;
    return newthread;
}

void idle()
{
    while (rq_head != NULL || zombie_thread != NULL)
    {
        disable_interrupt();
        kill_zombies();
        do_fork();
        enable_interrupt();
        schedule();
    }
}

/* threads' routine for any task */
void task_wrapper()
{
    thread_t *current = (thread_t *)get_current();
    (current->task)();
    exit(0);
}

/* kill the zombie threads and recycle their resources */
void kill_zombies()
{
    while (zombie_thread != NULL)
        recycle_thread(zombie_thread);
}

int add_rq(thread_t *thread)
{
    if (rq_head == NULL)
    {
        rq_head = thread;
        rq_tail = thread;
        thread->next = NULL;
    }
    else
    {
        rq_tail->next = thread;
        rq_tail = thread;
        thread->next = NULL;
    }
    return 1;
}

thread_t *del_rq()
{
    if (rq_head == NULL)
        return &kernel_thread;
    else
    {
        thread_t *ret = rq_head;
        rq_head = rq_head->next;
        return ret;
    }
}

/* clean thread data struct and make it available */
void recycle_thread(thread_t *current)
{
    zombie_thread = zombie_thread->next;
    free((void*)current->kstack_start);
    free((void*)current->ustack_start);
    free(current);
}

void do_fork()
{
    thread_t *parent = rq_head;
    while (parent != NULL)
    {
        if (parent->status == FORKING)
            create_child(parent);
        else
            parent = parent->next;
    }
}

/* copy all data including stack and program for child process and set the corresponding sp and lr for it*/
void create_child(thread_t *parent)
{
    thread_t *child = thread_create(0);

    char *parent_d, *child_d;

    parent->status = READY;

    parent->child_id = child->id;
    child->child_id = 0;

    // copy context
    parent_d = (char *)&(parent->context);
    child_d = (char *)&(child->context);
    for (int i = 0; i < sizeof(context_t); i++)
        child_d[i] = parent_d[i];

    // copy kernel stack
    parent_d = (char *)parent->kstack_start;
    child_d = (char *)child->kstack_start;
    for (int i = 0; i < MIN_PAGE_SIZE; i++)
        child_d[i] = parent_d[i];

    // copy user stack
    parent_d = (char *)parent->ustack_start;
    child_d = (char *)child->ustack_start;
    for (int i = 0; i < MIN_PAGE_SIZE; i++)
        child_d[i] = parent_d[i];

    // copy user program
    parent_d = (char *)parent->usrpgm_load_addr;
    child_d = (char *)child->usrpgm_load_addr;
    for (int i = 0; i < USRPGM_SIZE; i++)
        child_d[i] = parent_d[i];

    // set offset to child's stack
    unsigned long kstack_offset = child->kstack_start - parent->kstack_start;
    unsigned long ustack_offset = child->ustack_start - parent->ustack_start;
    unsigned long usrpgm_offset = child->usrpgm_load_addr - parent->usrpgm_load_addr;

    // set child kernel space offset
    child->context.fp += kstack_offset;
    child->context.sp += kstack_offset;
    child->trapframe = parent->trapframe + kstack_offset;

    // set child user space offset
    trapframe_t *ctrapframe = (trapframe_t *)child->trapframe; // because of data type problem
    ctrapframe->x[29] += ustack_offset;
    ctrapframe->sp_el0 += ustack_offset;
    ctrapframe->elr_el1 += usrpgm_offset;
}