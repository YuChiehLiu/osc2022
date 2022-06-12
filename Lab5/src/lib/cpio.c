#include "string.h"
#include "cpio.h"
#include "uart.h"


void print_content(char* target_name)
{
    CPIO_NEWC_HEADER *target_addr = get_target_addr( (CPIO_NEWC_HEADER*)CPIO_START_ADDR, target_name );
    if( target_addr == 0 )
    {
        uart_puts("\rcat : ");
        uart_puts(target_name);
        uart_puts(": No such file or directory");
    }
    else
    {
        int namesize = get_size(target_addr, "name") -1;
        int filesize = get_size(target_addr, "file");
        int NUL_nums = 1;

        char* print_addr = (char*) (target_addr+1);
               
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        print_addr += (namesize+NUL_nums);
        
        uart_puts("\r");
        uart_myputs(print_addr, filesize);
    }  
}

void load_usrpgm(char* usrpgm_name, unsigned long load_addr)
{
    CPIO_NEWC_HEADER *target_addr = get_target_addr( (CPIO_NEWC_HEADER*)CPIO_START_ADDR, usrpgm_name );

    int namesize = get_size(target_addr, "name") -1;
    int filesize = get_size(target_addr, "file");
    int NUL_nums = 1;

    char* load_start_addr = (char*) load_addr;
    char* load_data_addr = (char*) (target_addr+1); // skip the content before filename

    while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;
    load_data_addr += (namesize+NUL_nums);

    for(int i=0 ; i<filesize ; i++)
    {
        *(load_start_addr+i) = *(load_data_addr+i);
    }
}

CPIO_NEWC_HEADER* get_target_addr(CPIO_NEWC_HEADER *current_file, char* target_name)
{
    int namesize = get_size(current_file, "name") -1;
    int filesize = get_size(current_file, "file");

    char *temp_addr = (char*) (current_file+1);

    char temp_name[20];

    for( int i=0 ; i<namesize ; i++)
    {
        temp_name[i] = temp_addr[i];
    }

    temp_name[namesize] = '\0';

    // No this file or directory
    if( strequ(temp_name, "TRAILER!!!") )
        return 0;
    // find the target
    else if( strequ(temp_name, target_name) )
        return current_file;
    // forward to next file
    else
    {
        char *next_file = temp_addr;
        int NUL_nums=1;

        //calculate the number of '\0' needed because of padding
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        next_file += (namesize+NUL_nums);
        NUL_nums=0;
        //calculate the number of '\0' needed because of padding 
        while( (filesize+NUL_nums) % 4 != 0 )
            NUL_nums++;        

        next_file += (filesize+NUL_nums);

        return get_target_addr( (CPIO_NEWC_HEADER*)next_file, target_name ); 
    }
    
    return 0;
}

int get_size(CPIO_NEWC_HEADER *root_addr, char* attr)
{
    char *temp_addr = (char*)root_addr;

    if ( strequ(attr, "name") )
        temp_addr += 94;
    else if( strequ(attr, "file") )
        temp_addr += 54;


    char size_string[9];
    for( int i=0 ; i<8 ; i++)
    {
        size_string[i] = temp_addr[i]; 
    }

    size_string[8] = '\0';
     

    //hexadecimal to decimal
    return hex2int(size_string);
    
}