#include "uart.h"
#include "command.h"
#include "shell.h"
#include "cpio.h"
#include "string.h"
#include "timer_handler.h"

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
    uart_puts("ls\t : list all files\n");
    uart_puts("cat\t : print the file contents\n");
    uart_puts("svc\t : test svc\n");
    uart_puts("time\t : display now time and trigger set a 2s timer interrupt \n");
    uart_puts("sto\t : [MESSAGE] [SECONDS]\n");
    uart_puts("asyr\t : for test asynchronous UART read\n");
    uart_puts("asyw\t : for test asynchronous UART write\n");
    uart_puts("reboot\t : reboot the device\n");
    uart_puts("\n");
}

void command_hello()
{
    uart_send('\r');
    uart_puts("Hello World!\n\r");
}

void command_ls()
{
    CPIO_NEWC_HEADER *current_file=(CPIO_NEWC_HEADER*)CPIO_START_ADDR;
    int namesize;
    int filesize;
    char *temp_addr;
    char temp_name[20];
    char *next_file;
    int NUL_nums;
    
    while(!strequ(temp_name, "TRAILER!!!"))
    {
        namesize = get_size(current_file, "name") -1;
        filesize = get_size(current_file, "file");

        temp_addr = (char*) (current_file+1);

        for( int i=0 ; i<namesize ; i++)
            temp_name[i] = temp_addr[i];

        temp_name[namesize] = '\0';

        // No this file or directory
        if(!strequ(temp_name, "TRAILER!!!"))
        {
            uart_puts(temp_name);
            uart_puts("\n\r");
        }

        // forward to next file
        next_file = temp_addr;
        NUL_nums=1;

        //calculate the number of '\0' needed because of padding
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        next_file += (namesize+NUL_nums);
        NUL_nums=0;
        //calculate the number of '\0' needed because of padding 
        while( (filesize+NUL_nums) % 4 != 0 )
            NUL_nums++;        

        next_file += (filesize+NUL_nums);

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

void command_svc() // transfer exception level
{
    load_usrpgm("usrpgm.img");
    asm volatile
    (
        "mov x0, 0x3c0\n\t" // EL0t
        "msr spsr_el1, x0\n\t"
        "mov x0, 0x14000000\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, 0x14000000\n\t"
        "msr sp_el0, x0\n\t"
        "eret"
    );
}

void command_time()
{
    get_nowtime();

    // asm volatile
    // (
    //     "msr DAIFSet, 0xf\n\t"
    //     "mrs x1, cntfrq_el0\n\t"
    //     "add x1, x1, x1\n\t" // after 2 seconds later will interrupt
    //     "msr cntp_tval_el0, x1\n\t" // set expired time
    //     "mov x10, 0\n\t" // clean x10 for no message
    //     "bl core_timer_enable\n\t"
    //     "msr DAIFClr, 0xf\n\t"
    // );

    // for EL0
    asm volatile
    (
        "mov x3, 0x0\n\t" // use x1 instead of x0 because x0 is reserved automatically to store the address of "handling_info" 
        "msr spsr_el1, x3\n\t"
        "mov x0, %0\n\t"
        "add x0, x0, 76\n\t"
        "msr elr_el1, x0\n\t"
        "mov x0, 0x1000000\n\t"
        "msr sp_el0, x0\n\t"
        "mrs x0, cntfrq_el0\n\t"
        "add x0, x0, x0\n\t" // after 2 seconds later will interrupt
        "msr cntp_tval_el0, x0\n\t" // set expired time
        "bl core_timer_enable\n\t"
        "eret"
        :
        :"r"(main)
        :
    );


}

void command_setTimeout(char* string)
{
    char message[21];
    char seconds[10];
    int count=0;
    int flag=1;
    int i=3;

    for(; i<30 ; i++)
    {
        if(string[i]!=' ' && flag)
        {
            message[count] = string[i];
            count++; 
        }
        else if(string[i]==' ' && count!=0)
        {
            message[count] = '\0';
            break;
        }
    }

    flag=1;
    count=0;

    for(; i<50 ; i++)
    {
        if(string[i]!=' ' && string[i]!='\0' && flag)
        {
            seconds[count] = string[i];
            count++; 
        }
        else if((string[i]=='\0' || string[i]==' ') && count!=0)
        {
            seconds[count] = '\0';
            break;
        }
    }
   
    int sec;
    sec=atoi(seconds);

    uart_puts("You have set a ");
    uart_puts(seconds);
    uart_puts("s timer\n");

    add_timer(sec, message);  
}

void command_reboot() 
{   
    uart_puts("Start Rebooting ... \n");
    set(PM_RSTC, PM_PASSWORD | 0x20); 
    set(PM_WDOG, PM_PASSWORD | 100); 
}

void command_not_found(char* string)
{
    uart_send('\r');
    uart_puts("Err: command ");
    uart_puts(string);
    uart_puts(" not found, try <help>\n");
}