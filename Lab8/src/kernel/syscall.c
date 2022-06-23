#include "mbox.h"
#include "mboxf.h"
#include "cpio.h"
#include "uart.h"
#include "command.h"
#include "string.h"
#include "thread.h"
#include "syscall.h"
#include "buddy_sys.h"
#include "slub_sys.h"
#include "timer.h"
#include "exception.h"
#include "printf.h"
#include "tmpfs.h"

/* systemcall #0 */
int getpid()
{
    int ret = get_current()->id;
    return ret;
}

/* systemcall #1 */
size_t uart_read(char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        buf[i] = uart_getc();
        if (buf[i] == '\n' || buf[i] == '\r')
            return i;
    }

    return size;
}

/* systemcall #2 */
size_t uart_write(const char buf[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (buf[i] == '\n')
            uart_send('\r');
        else if (buf[i] == '\0')
            return i;
        uart_send(buf[i]);
    }

    return size;
}

/* systemcall #3 */
int exec(char *name, char *const argv[])
{
    thread_t *current = get_current();
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE;

    set_switch_timer();
    core_timer_enable();
    enable_interrupt();

    load_usrpgm(name, target_addr);

    asm volatile(
        "mov x0, 0x0\n\t" // EL0t
        "msr spsr_el1, x0\n\t"
        "mov x0, %0\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, %1\n\t"
        "msr sp_el0, x0\n\t"
        "eret"
        :
        : "r"(target_addr), "r"(target_sp)
        : "x0");

    return 0;
}

/* systemcall #4 */
int fork()
{
    schedule();
    return get_current()->child_id;
}

/* systemcall #5 */
void exit(int status)
{
    thread_t *current = (thread_t *)get_current();
    current->status = ZOMBIE;
    current->next = zombie_thread;
    zombie_thread = current;
    // schedule();
    switch_to(current, &kernel_thread);
}

/* systemcall #6 */
// refer to mbox.c : int mbox_call_u(unsigned char ch, unsigned int *mbox_user)

/* systemcall #7 */
void kill(int pid)
{
    thread_t *tmp = get_current();
    if (tmp->id == pid)
        exit(0);
    else
        tmp = rq_head;

    while (tmp != NULL)
    {
        if (tmp->id == pid)
        {
            tmp->status = ZOMBIE;
            return;
        }
        tmp = tmp->next;
    }
}

/* systemcall #11 */
int open(char *pathname, int flags)
{
    file_t* target;
    int ret = vfs_open(pathname, flags, &target);

    if(ret!=0)
        return ret;
    
    kfd[fd_count] = target;
    fd_count++;

    return fd_count-1;
}

/* systemcall #12 */
int close(int fd)
{
    int ret = vfs_close(kfd[fd]);
    return ret;
}

/* systemcall #13 */
// remember to return read size or error code
long write(int fd, void *buf, unsigned long count)
{
    int ret = vfs_write(kfd[fd], buf, count);
    return ret;
}

/* systemcall #14 */
// remember to return read size or error code
long read(int fd, void *buf, unsigned long count)
{
    int ret = vfs_read(kfd[fd], buf, count);
    return ret;
}

/* systemcall #15 */
// you can ignore mode, since there is no access control
int mkdir(char *pathname, unsigned mode)
{
    int ret = vfs_mkdir(pathname);
    return ret;
}

/* systemcall #16 */
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int mount(char *src, char *target, char *filesystem, unsigned long flags, void *data)
{
    int ret = vfs_mount(target, filesystem);
    return ret;
}

/* systemcall #17 */
int chdir(char *path)
{
    char pathname[256];
    handle_path(path, pathname);
    int ret = command_cd(pathname);

    return ret;
}