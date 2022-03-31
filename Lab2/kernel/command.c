#include "uart.h"
#include "command.h"
#include "shell.h"
#include "cpio.h"
#include "string.h"

void set(long addr, unsigned int value)
{
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void command_help()
{
    uart_puts("\n");
    uart_puts("help\t : print this help menu\n");
    uart_puts("hello\t : print Hello World!\n");
    uart_puts("cat\t : print the file contents\n");
    uart_puts("reboot\t : reboot the device\n");
    uart_puts("\n");
}

void command_hello()
{
    uart_send('\r');
    uart_puts("Hello World!\n");
}

void command_not_found(char* string)
{
    uart_send('\r');
    uart_puts("Err: command ");
    uart_puts(string);
    uart_puts(" not found, try <help>\n");
}

void command_reboot() 
{   
    uart_puts("Start Rebooting ... \n");
    set(PM_RSTC, PM_PASSWORD | 0x20); 
    set(PM_WDOG, PM_PASSWORD | 100); 
}

void command_cat()
{
    uart_puts("\rFile Name : ");

    char in_c;
    char string[MAX_BUFFER_LEN];
    int buffer_counter = 0;
    int in_v;

    strset(string, 0, MAX_BUFFER_LEN);

    while(1)
    {
        in_c = uart_getc();

        in_v = distinguish(in_c);

        if( in_v == UNKNOWN )
            return;
        else if( in_v == BACKSPACE )
        {
            uart_send(in_c);
            uart_send(' ');
            uart_send(in_c);
            if(buffer_counter > 0)
                buffer_counter--;
        }
        else if( in_v == NEWLINE)
        {
            uart_send(in_c);
            string[buffer_counter] = '\0';
            print_content(string);     
            uart_puts("\n");
            return;
        }
        else if( in_v == LEGAL_INPUT )
        {
            uart_send(in_c);

            string[buffer_counter] = in_c;
            buffer_counter++;
        }
    }
    
}