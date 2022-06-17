#include "uart.h"
#include "command.h"
#include "shell.h"
#include "cpio.h"
#include "string.h"
#include "math.h"
#include "timer.h"
#include "exception.h"
#include "buddy_sys.h"
#include "slub_sys.h"
#include "thread.h"
#include "printf.h"
#include "syscall.h"
#include "tmpfs.h"

void set(long addr, unsigned int value)
{
    volatile unsigned int *point = (unsigned int *)addr;
    *point = value;
}

void command_help()
{
    uart_puts("\n");
    uart_puts("help\t : print this help menu\n");
    uart_puts("hello\t : print Hello World!\n");
    uart_puts("bls\t : list all files\n");
    uart_puts("cat\t : print the file contents\n");
    uart_puts("svc\t : test svc\n");
    uart_puts("time\t : display now time and trigger set a 2s timer interrupt \n");
    uart_puts("sto\t : [MESSAGE] [SECONDS]\n");
    uart_puts("cma\t : [SIZE]\n");
    uart_puts("free\t : [address]\n");
    uart_puts("lsrm\t : list all reserved memory\n");
    uart_puts("asyr\t : for test asynchronous UART read\n");
    uart_puts("asyw\t : for test asynchronous UART write\n");
    uart_puts("tt\t : demo Lab5\n");
    uart_puts("cd\t : change cwd\n");
    uart_puts("ls\t : list directory\n");
    uart_puts("mkdir\t : create a directory\n");
    uart_puts("touch\t : create a directory\n");
    uart_puts("reboot\t : reboot the device\n");
    uart_puts("\n");
}

void command_hello()
{
    uart_send('\r');
    uart_puts("Hello World!\n\r");
}

void command_bls()
{
    CPIO_NEWC_HEADER *current_file = (CPIO_NEWC_HEADER *)CPIO_START_ADDR;
    int namesize;
    int filesize;
    char *temp_addr;
    char temp_name[20];
    char *next_file;
    int NUL_nums;

    while (!strequ(temp_name, "TRAILER!!!"))
    {
        namesize = get_size(current_file, "name") - 1;
        filesize = get_size(current_file, "file");

        temp_addr = (char *)(current_file + 1);

        for (int i = 0; i < namesize; i++)
            temp_name[i] = temp_addr[i];

        temp_name[namesize] = '\0';

        // No this file or directory
        if (!strequ(temp_name, "TRAILER!!!"))
        {
            uart_puts(temp_name);
            uart_puts("\n\r");
        }

        // forward to next file
        next_file = temp_addr;
        NUL_nums = 1;

        // calculate the number of '\0' needed because of padding
        while ((2 + namesize + NUL_nums) % 4 != 0)
            NUL_nums++;

        next_file += (namesize + NUL_nums);
        NUL_nums = 0;
        // calculate the number of '\0' needed because of padding
        while ((filesize + NUL_nums) % 4 != 0)
            NUL_nums++;

        next_file += (filesize + NUL_nums);

        current_file = (CPIO_NEWC_HEADER *)next_file;
    }
}

void command_cat()
{
    uart_puts("\rFile Name : ");

    char in_c;
    char string[MAX_BUFFER_LEN];
    int buffer_counter = 0;
    int in_v;

    strset(string, 0, MAX_BUFFER_LEN);

    while (1)
    {
        in_c = uart_getc();

        in_v = distinguish(in_c);

        if (in_v == UNKNOWN)
            return;
        else if (in_v == BACKSPACE)
        {
            uart_send(in_c);
            uart_send(' ');
            uart_send(in_c);
            if (buffer_counter > 0)
                buffer_counter--;
        }
        else if (in_v == NEWLINE)
        {
            uart_send(in_c);
            string[buffer_counter] = '\0';
            print_content(string);
            uart_puts("\n");
            return;
        }
        else if (in_v == LEGAL_INPUT)
        {
            uart_send(in_c);

            string[buffer_counter] = in_c;
            buffer_counter++;
        }
    }
}

void command_svc() // transfer exception level
{
    load_usrpgm("usrpgm.img", 0x14000000);
    asm volatile(
        "mov x0, 0x3c0\n\t" // EL0t
        "msr spsr_el1, x0\n\t"
        "mov x0, 0x14000000\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, 0x14000000\n\t"
        "msr sp_el0, x0\n\t"
        "eret");
}

void command_time()
{
    get_nowtime();
}

void command_setTimeout(char *string)
{
    char message[21];
    char seconds[10];
    int count = 0;
    int flag = 1;
    int i = 3;

    for (; i < 30; i++)
    {
        if (string[i] != ' ' && flag)
        {
            message[count] = string[i];
            count++;
        }
        else if (string[i] == ' ' && count != 0)
        {
            message[count] = '\0';
            break;
        }
    }

    flag = 1;
    count = 0;

    for (; i < 50; i++)
    {
        if (string[i] != ' ' && string[i] != '\0' && flag)
        {
            seconds[count] = string[i];
            count++;
        }
        else if ((string[i] == '\0' || string[i] == ' ') && count != 0)
        {
            seconds[count] = '\0';
            break;
        }
    }

    int sec;
    sec = atoi(seconds);

    uart_puts("You have set a ");
    uart_puts(seconds);
    uart_puts("s timer\n");

    add_timer(sec, message);
}

void command_cma(char *string)
{
    char size_c[20], index_c[10];
    int size = 0;
    int i = 4;

    while (string[i] != '\0')
    {
        size_c[i - 4] = string[i];
        i++;
    }

    size_c[i - 4] = '\0';
    size = atoi(size_c);

    if (size > pow(2, MAX_ORDER) * MIN_PAGE_SIZE)
    {
        uart_puts("Too large size, limit is 1048576.\n\r");
        return;
    }

    if (size == 0)
        demo();
    else if (size > MAX_POOL_SIZE)
    {
        int index = ((long)malloc(size) >> 12) % TOTAL_PAGE_FRAME;
        // index = (index>>12) % TOTAL_PAGE_FRAME;

        if (index == -1)
        {
            uart_puts("No free area can be allocated.\n\r");
            demo();
            return;
        }

        int count = 1;
        int tmp_index = index;

        while (tmp_index / 10 != 0)
        {
            tmp_index /= 10;
            count++;
        }

        itoa(index, index_c, count);

        asyn_write("-----------------------------------------------------");
        uart_puts("\nconfigure page frame index: ");
        uart_puts(index_c);
        uart_puts(" \n");

        int start = pf[index].addr;
        int end = pf[index].addr + MIN_PAGE_SIZE * pow(2, pf[index].allord) - 1;
        char start_c[20], end_c[20];

        strset(start_c, '\0', 20);
        strset(end_c, '\0', 20);

        itohexa(start, start_c, 8);
        itohexa(end, end_c, 8);

        uart_puts("configure address section: ");
        uart_puts(start_c);
        uart_puts(" - ");
        uart_puts(end_c);
        uart_puts(" \n");
        asyn_write("-----------------------------------------------------\n");

        demo();
        // uart_puts("---------------opposite---------------\n");
        // demo_opp();
    }
    else if (size <= MAX_POOL_SIZE)
    {
        long addr = (long)malloc(size);

        if (addr == -1)
        {
            uart_puts("No free area can be allocated.\n\r");
            demo();
            return;
        }

        int start = addr;
        int end = addr + roundup_size(size) * MIN_CHUNK_SIZE - 1;
        char start_c[20], end_c[20];

        strset(start_c, '\0', 20);
        strset(end_c, '\0', 20);

        itohexa(start, start_c, 8);
        itohexa(end, end_c, 8);

        asyn_write("-----------------------------------------------------");
        uart_puts("\nconfigure address section: ");
        uart_puts(start_c);
        uart_puts(" - ");
        uart_puts(end_c);
        uart_puts(" \n");
        asyn_write("-----------------------------------------------------\n");

        demo();
    }
}

void command_free(char *string)
{
    char index_c[10];
    int index = 0;
    int i = 5;

    while (string[i] != '\0')
    {
        index_c[i - 5] = string[i];
        i++;
    }

    index_c[i - 5] = '\0';
    index = atoi(index_c);

    if (!is_freeable(index) || find_pool(pf[index].addr) != NULL)
    {
        uart_puts("Page frame ");
        uart_puts(index_c);
        uart_puts(" cannot be freed\n\r");
        return;
    }

    free_pf(index);

    demo();
    // uart_puts("---------------opposite---------------\n");
    // demo_opp();
}

void command_smfree(char *string)
{
    char addr_c[20];
    unsigned long addr = 0;
    int i = 7;

    while (string[i] != '\0')
    {
        addr_c[i - 7] = string[i];
        i++;
    }

    addr_c[i - 7] = '\0';
    addr = hex2int(addr_c);

    int result = free((void*)addr);

    if (result)
        demo();
    else
    {
        uart_puts("0x");
        uart_puts(addr_c);
        uart_puts(" cannot be freed\n\r");
    }
}

void command_lsrm()
{
    char start_c[20], end_c[20];

    uart_puts("Reserved Memory : \n");

    for (int i = 1; i <= RMindex; i++)
    {
        strset(start_c, '\0', 20);
        strset(end_c, '\0', 20);
        itohexa(RMlist[i].start, start_c, 8);
        itohexa(RMlist[i].end, end_c, 8);

        uart_puts(RMlist[i].name);
        uart_puts(" \t( ");
        uart_puts(start_c);
        uart_puts(" - ");
        uart_puts(end_c);
        uart_puts(" )\n");
    }
}

void command_tt()
{
    thread_create(load_usrpgm_in_umode);
    idle();
}

void command_mfs()
{
    file_t* target=NULL;
    vfs_open("./test", O_CREAT, &target);

    char buf[20];
    if(vfs_read(target, buf, 20) == 0)
        printf("read 0 byte.\n");

    strcpy("abcde", buf);
    int write_len = vfs_write(target, buf, strlen(buf));
    printf("write %d bytes.\n", write_len);

    strcpy("12345", buf);
    printf("before reading, buf = %s\n", buf);

    target->f_pos = 0;
    int read_len = vfs_read(target, buf, 20);
    printf("read %d bytes.\n", read_len);

    printf("buf = %s\n", buf);
    printf("f_ops = %d\n", target->f_pos);

    strcpy("12345", buf);
    write_len = vfs_write(target, buf, strlen(buf));

    target->f_pos = 0;
    read_len = vfs_read(target, buf, 20);
    printf("read %d bytes.\n", read_len);

    buf[read_len] = '\0';
    printf("buf = %s\n", buf);
}

void command_ls(char* pathname, int flag)
{
    vnode_t* target_vnode;
    int ret;

    if(flag)
        ret = vfs_lookup(pathname, &target_vnode);
    else
        ret = vfs_lookup(cwdpath, &target_vnode);

    switch(ret)
    {
        case NOTFOUND:
            printf("%s is not existed.\n", pathname);
            return;
        case NOTDIR:
            return;
    }

    if(target_vnode->internal->type != DIR)
    {
        printf("%s is not a directory.\n", pathname);
        return;
    }

    for(int i=0 ; i<target_vnode->internal->size ; i++)
    {
        printf("%s", target_vnode->internal->entry[i]->internal->name);
        if(target_vnode->internal->entry[i]->internal->type == DIR)
            printf("/");
        printf("    ");
    }
    printf("\n");
}

int command_cd(char* target_dir)
{
    vnode_t* dir_node=NULL;  
    
    switch(vfs_lookup(target_dir, &dir_node))
    {
        case EXISTED:
            if(dir_node->internal->type!=DIR)
            {
                printf("%s is not a directory.\n", target_dir);
                return -1;
            }
            break;
        case NOTDIR:
            return -1;
        case NOTFOUND:
            printf("%s is not existed.\n", target_dir);
            return -1;
    }
        
    strcpy(target_dir, cwdpath);

    return 0;
}

void command_reboot()
{
    uart_puts("Start Rebooting ... \n");
    set(PM_RSTC, PM_PASSWORD | 0x20);
    set(PM_WDOG, PM_PASSWORD | 100);
}

void command_not_found(char *string)
{
    uart_send('\r');
    uart_puts("Err: command ");
    uart_puts(string);
    uart_puts(" not found, try <help>\n");
}

void load_usrpgm_in_umode()
{
    thread_t *current = get_current();
    unsigned long target_addr = current->usrpgm_load_addr;
    unsigned long target_sp = current->ustack_start + MIN_PAGE_SIZE;

    load_usrpgm("vfs1.img", target_addr);

    set_switch_timer();
    core_timer_enable();
    enable_interrupt();

    asm volatile(
        "mov x0, 0x0\n\t" // EL0t, and open diaf(interrupt)
        "msr spsr_el1, x0\n\t"
        "mov x0, %0\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, %1\n\t"
        "msr sp_el0, x0\n\t"
        "eret"
        :
        : "r"(target_addr), "r"(target_sp)
        : "x0");
}
