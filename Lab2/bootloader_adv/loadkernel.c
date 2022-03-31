#include "uart.h"
#include "loadkernel.h"
#include "string.h"

void copy_transfer()
{
    uart_puts("Booting...\n");

    /*
     * step1 : copy self to 0x60000
     */

    // get size of bootloader    
    long long copy_size = (long long)&__self_size;

    //print the size for test
    /*char print_size[10];
    itoa(copy_size, print_size, 0);
    uart_puts("\r size is:");
    uart_puts(print_size);
    uart_puts("\n");*/

    char *copy_to_addr = (char*)0x60000;
    char *copy_from_addr = (char*)0x80000;

    for( int i=0 ; i<copy_size ; i++)
    {
        copy_to_addr[i] = copy_from_addr[i];
    }

    /*
     * step2 : pc to next step address (-0x20000)
     */

    //calculate relocate address
    int *ret_addr = __builtin_return_address(0) - 0x20000;

    uart_puts("Over...\n");
  
    goto *ret_addr;
}

void load_kernel()
{
    /*
     * step3 : load the kernel img to 0x80000
     */

    uart_puts("Wait for kernel img sent...\n");

    // build simple protocol
    char sptl[5] = "START";

    // wait until the sign shows
    for( int i=0 ; i<5 ; i++ )
    {
        if( uart_getc() == sptl[i] );
        else i=0;
    }

    uart_puts("Start to load the kernel...\n");

    int kernel_size = 0;

    // get the size of kernel img
    for( int i=0 ; i<4 ; i++)
    {
        char size_buffer = uart_mygetc();
        kernel_size |= (size_buffer << (i*8)) ;
    }

    uart_puts("KernelSize: ");
    
    char temp[10];
    itoa(kernel_size, temp, 0);
    uart_puts(temp);
    uart_puts("\n");

    // load to the right address
    char *kernel_code=(char*)0x80000;

    // start to send the kernel img
    for( int i=0 ; i<kernel_size ; i++)
    {
        kernel_code[i] = uart_mygetc();
    }  

    uart_puts("Kernel has been loaded entirely...\n");

    /*
     * step4 : pc to 0x80000
     */
  
    goto *kernel_code;

}
