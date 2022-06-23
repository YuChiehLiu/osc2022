/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
#include "string.h"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001C
#define PM_WDOG 0x3F100024

extern int __self_size;

void copy_transfer();
void load_kernel();

void bootloader()
{
    // set up serial console
    uart_init();

    // step1 : copy self to 0x60000
    // step2 : pc to next step address (-0x20000)
    copy_transfer();

    // step3 : load the kernel img to 0x80000
    // step4 : pc to 0x80000
    load_kernel();

}

void copy_transfer()
{
    uart_puts("Booting...\n");

    /*
     * step1 : copy self to 0x60000
     */

    // get size of bootloader    
    long long copy_size = (long long)&__self_size;

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