#include "shell.h"
#include "command.h"
#include "uart.h"
#include "cpio.h"
#include "string.h"

void shell_start()
{
    char input_char;
    char string_buffer[MAX_BUFFER_LEN];
    int buffer_counter = 0;
    int input_value;

    strset(string_buffer, 0, MAX_BUFFER_LEN);

    uart_puts("# ");

    while(1)
    {
        input_char = uart_getc();

        input_value = distinguish(input_char);

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
    else if( in_v == NEWLINE)
    {
        uart_send(in_c);

        string[*buffer_counter] = '\0';

        if( strequ( string, "help" ) )
            command_help();
        else if( strequ( string, "hello" ) )
            command_hello();
        else if( strequ( string, "reboot" ) )
            command_reboot();
        else if( strequ( string, "cat" ) )
            command_cat();
        else
            command_not_found(string);
        
        strset(string, 0, MAX_BUFFER_LEN);
        (*buffer_counter) = 0;

        uart_puts("# ");
    }
    else if( in_v == LEGAL_INPUT )
    {
        uart_send(in_c);

        string[*buffer_counter] = in_c;
        (*buffer_counter)++;
    }
}