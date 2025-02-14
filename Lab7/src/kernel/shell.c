#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "cpio.h"
#include "string.h"
#include "mbox.h"
#include "mboxf.h"
#include "command.h"
#include "buddy_sys.h"
#include "slub_sys.h"
#include "thread.h"
#include "tmpfs.h"

void main()
{
    // set up serial console
    uart_init();
    memory_init();
    thread_init(&kernel_thread);
    vfs_mount("/", "tmpfs");
    vfs_mount("/initramfs", "initramfs");
    strcpy(rootfs->root->internal->name, cwdpath);

    uart_puts("\n========================================\n\n");

    mbox_info();

    uart_puts("\n========================================\n\n");

    char input_char = 0;
    char string_buffer[MAX_BUFFER_LEN];
    int buffer_counter = 0;
    int input_value;

    strset(string_buffer, 0, MAX_BUFFER_LEN);

    // shell start
    uart_puts("# ");

    while (1)
    {
        input_char = uart_getc();

        input_value = distinguish(input_char);

        if (input_value == NEWLINE)
            exe_command2(input_value, string_buffer, &buffer_counter);
        else
            exe_command(input_char, input_value, string_buffer, &buffer_counter);
    }
}

int distinguish(char c)
{
    if (c < 0 || c >= 128)
        return UNKNOWN;
    else if (c == BACKSPACE)
        return BACKSPACE;
    else if (c == NEWLINE)
        return NEWLINE;
    else
        return LEGAL_INPUT;
}

void exe_command(char in_c, int in_v, char *string, int *buffer_counter)
{
    if (in_v == UNKNOWN)
        return;
    else if (in_v == BACKSPACE)
    {
        uart_send(in_c);
        uart_send(' ');
        uart_send(in_c);
        if ((*buffer_counter) > 0)
            (*buffer_counter)--;
    }
    else if (in_v == LEGAL_INPUT)
    {
        uart_send(in_c);

        string[*buffer_counter] = in_c;
        (*buffer_counter)++;
    }
}

// just for gdb debug, actually can merge two exe_command func.(see Lab2)
void exe_command2(int in_v, char *string, int *buffer_counter)
{
    uart_puts("\n\r");
    string[*buffer_counter] = '\0';
    if (strequ(string, "help"))
        command_help();
    else if (strequ(string, "hello"))
        command_hello();
    else if (strequ(string, "reboot"))
        command_reboot();
    else if (strequ(string, "bls"))
        command_bls();
    else if (strequ(string, "cat"))
        command_cat();
    else if (strequ(string, "svc"))
        command_svc();
    else if (strequ(string, "time"))
        command_time();
    else if (string[0] == 's' && string[1] == 't' && string[2] == 'o')
    {
        if (check_format(string, "sto\0"))
            command_setTimeout(string);
    }
    else if (string[0] == 'c' && string[1] == 'm' && string[2] == 'a')
    {
        if (check_format(string, "cma\0"))
            command_cma(string);
    }
    else if (string[0] == 'f' && string[1] == 'r' && string[2] == 'e' && string[3] == 'e')
    {
        if (check_format(string, "free\0"))
        {
            if (string[5] == '0' && string[6] == 'x')
                command_smfree(string);
            else
                command_free(string);
        }
    }
    else if (strequ(string, "lsrm"))
        command_lsrm();
    else if (strequ(string, "asyr"))
        asyn_read();
    else if (strequ(string, "asyw"))
    {
        asyn_write(read_buffer);
        asyn_write("\n\r");
    }
    else if (strequ(string, "tt"))
        command_tt();
    else if (strequ(string, "mfs"))
        command_mfs();
    else if (string[0] == 'c' && string[1] == 'w' && string[2] == 'd')
        printf("%s\n", cwdpath);
    else if (string[0] == 'l' && string[1] == 's')
    {
        char pathname[256];
        if(strlen(string)==2 || strlen(string)==3)
            command_ls(pathname, 0);
        else
        {
            handle_path(string+3, pathname);
            command_ls(pathname, 1);
        }    
    }
    else if (string[0] == 'c' && string[1] == 'd')
    {
        char pathname[256];
        handle_path(string+3, pathname);
        command_cd(pathname);
    }
    else if (string[0] == 'm' && string[1] == 'k' && string[2] == 'd' && string[3] == 'i' && string[4] == 'r')
    {
        char pathname[256];
        handle_path(string+6, pathname);
        vfs_mkdir(pathname);
    }
    else if (string[0] == 't' && string[1] == 'o' && string[2] == 'u' && string[3] == 'c' && string[4] == 'h')
    {
        char pathname[256];
        handle_path(string+6, pathname);
        vfs_create(pathname);
    }
    else
        command_not_found(string);

    strset(string, 0, MAX_BUFFER_LEN);
    (*buffer_counter) = 0;
    uart_puts("# ");
}

int check_format(char *string, char *commandname)
{
    int len = strlen(string);
    int len_comm = strlen(commandname);
    if (len == len_comm)
    {
        uart_puts("too few arguments for command: ");
        uart_puts(commandname);
        uart_puts("\n\rTry 'help' for more information.\n");
        return 0;
    }
    else if (len != len_comm && (string[len_comm] != ' '))
    {
        command_not_found(string);
        return 0;
    }
    return 1;
}

