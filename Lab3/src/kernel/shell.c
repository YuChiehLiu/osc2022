#include "shell.h"
#include "uart.h"
#include "gpio.h"
#include "cpio.h"
#include "string.h"
#include "mbox.h"
#include "mboxf.h"
#include "command.h"

#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define IRQs_1          ((volatile unsigned int*)(MMIO_BASE+0x0000b210))



void main()
{
    // set up serial console
    uart_init();
    
    uart_puts("\n========================================\n\n");

    mbox_info();

    uart_puts("\n========================================\n\n");

    char input_char=0;
    char string_buffer[MAX_BUFFER_LEN];
    int buffer_counter = 0;
    int input_value;


    strset(string_buffer, 0, MAX_BUFFER_LEN);

    // shell start
    uart_puts("# ");

    while(1)
    {
        input_char = uart_getc();

        input_value = distinguish(input_char);

        if( input_value == NEWLINE)
            exe_command2(input_value, string_buffer, &buffer_counter);
        else
            exe_command(input_char, input_value, string_buffer, &buffer_counter);
    }    


}

int distinguish(char c)
{
    if( c<0 || c>=128 )
        return UNKNOWN;
    else if( c == BACKSPACE )
        return BACKSPACE;
    else if( c == NEWLINE )
        return NEWLINE;
    else
        return LEGAL_INPUT;
}

void exe_command(char in_c, int in_v, char *string, int *buffer_counter)
{
    if( in_v == UNKNOWN )
        return;
    else if( in_v == BACKSPACE )
    {
        uart_send(in_c);
        uart_send(' ');
        uart_send(in_c);
        if((*buffer_counter) > 0)
            (*buffer_counter)--;
    }
    else if( in_v == LEGAL_INPUT )
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
    if( strequ( string, "help" ) )
        command_help();
    else if( strequ( string, "hello" ) )
        command_hello();
    else if( strequ( string, "reboot" ) )
        command_reboot();
    else if( strequ( string, "ls" ) )
        command_ls();
    else if( strequ( string, "cat" ) )
        command_cat();
    else if( strequ( string, "svc" ) )
        command_svc();
    else if( strequ( string, "time" ) )
        command_time();
    else if( string[0]=='s' && string[1]=='t' && string[2]=='o')
    {
        if( check_format(string, "sto\0") ) 
            command_setTimeout(string);
    }
    else if( strequ( string, "asyr" ) )
        asyn_read();
    else if( strequ( string, "asyw" ) )
    {
        asyn_write(read_buffer);
        asyn_write("\n");
    }
    else
        command_not_found(string);
    
    strset(string, 0, MAX_BUFFER_LEN);
    (*buffer_counter) = 0;
    uart_puts("# ");
}

int check_format(char *string, char* commandname)
{
    int len = strlen(string);
    if(len==3)
    {
        uart_puts("too few arguments for command: sto\n");
        uart_puts("Try 'help' for more information.\n");
        return 0;
    }
    else if(len!=3 && (string[3] != ' '))
    {
        command_not_found(string);
        return 0;
    }
    return 1;  
}