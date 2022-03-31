#include "uart.h"
#include "command.h"

void command_help()
{
    uart_puts("\n");
    uart_puts("help\t : print this help menu\n");
    uart_puts("hello\t : print Hello World!\n");
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

void set(long addr, unsigned int value)
{
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void command_reboot() 
{   
    uart_puts("Start Rebooting ... \n");
    set(PM_RSTC, PM_PASSWORD | 0x20); 
    set(PM_WDOG, PM_PASSWORD | 100); 
}