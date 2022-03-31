#include "shell.h"
#include "command.h"
#include "uart.h"

#define MAX_BUFFER_LEN 14
#define UNKNOWN -1
#define BACKSPACE 8
#define NEWLINE 10
#define LEGAL_INPUT 200

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

void strset(char *string, int value, int size)
{
    for(int i=0 ; i<size ; i++)
        string[i] = value;
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

int strlen(char *string)
{
    int count = 0;
    while(1)
    {
        if( *(string+count) == '\0')
            break;
        count++;
    }
    
    return count;
}

int strequ(char *str1, char *str2)
{
    if(strlen(str1) != strlen(str2))
        return 0;
    else
    {
        for( int i=0 ; i<strlen(str1) ; i++)
        {
            if( str1[i] != str2[i] )
                return 0;
        }
    }
    
    return 1;
}

