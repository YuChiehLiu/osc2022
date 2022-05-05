#include "gpio.h"
#include "uart.h"
#include "string.h"
#include "exception.h"
#include "timer_handler.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))
#define IRQs_1          ((volatile unsigned int*)(MMIO_BASE+0x0000b210))

int buffer_count=0;

void exc_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far)
{
    // print out interruption type
    switch(type) {
        case 0: uart_puts("Synchronous"); break;
        case 1: uart_puts("IRQ"); break;
        case 2: uart_puts("FIQ"); break;
        case 3: uart_puts("SError"); break;
    }
    uart_puts(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(esr>>26) {
        case 0b000000: uart_puts("Unknown"); break;
        case 0b000001: uart_puts("Trapped WFI/WFE"); break;
        case 0b001110: uart_puts("Illegal execution"); break;
        case 0b010101: uart_puts("System call"); break;
        case 0b100000: uart_puts("Instruction abort, lower EL"); break;
        case 0b100001: uart_puts("Instruction abort, same EL"); break;
        case 0b100010: uart_puts("Instruction alignment fault"); break;
        case 0b100100: uart_puts("Data abort, lower EL"); break;
        case 0b100101: uart_puts("Data abort, same EL"); break;
        case 0b100110: uart_puts("Stack alignment fault"); break;
        case 0b101100: uart_puts("Floating point"); break;
        default: uart_puts("Unknown"); break;
    }
    // decode data abort cause
    if(esr>>26==0b100100 || esr>>26==0b100101) {
        uart_puts(", ");
        switch((esr>>2)&0x3) {
            case 0: uart_puts("Address size fault"); break;
            case 1: uart_puts("Translation fault"); break;
            case 2: uart_puts("Access flag fault"); break;
            case 3: uart_puts("Permission fault"); break;
        }
        switch(esr&0x3) {
            case 0: uart_puts(" at level 0"); break;
            case 1: uart_puts(" at level 1"); break;
            case 2: uart_puts(" at level 2"); break;
            case 3: uart_puts(" at level 3"); break;
        }
    }
    // dump registers
    uart_puts("\nSPSR_EL1 ");
    uart_hex(spsr>>32);
    uart_hex(spsr);
    uart_puts(" ; ELR_EL1 ");
    uart_hex(elr>>32);
    uart_hex(elr);
    uart_puts(" ; ESR_EL1 ");
    uart_hex(esr>>32);
    uart_hex(esr);
    uart_puts("\n");
}

void L0_timer_handler(long cntpct_el0, long cntfrq_el0)
{
    // disable core timer interrupt  
    asm volatile
    (
        "mov x3, 0\n\t"
        "msr cntp_ctl_el0, x3\n\t"
    );
    
    long nowtime = cntpct_el0/cntfrq_el0;
    char NOWTIME[10];

    itoa(nowtime, NOWTIME, 1);

    uart_puts("\nTime's out!!! Now time is ");
    uart_puts(NOWTIME);
    uart_puts(" s\n");
}

void IRQ_parser(long cntpct_el0, long cntfrq_el0, void* IRQ_source, void* IIR_ADDR)
{
    int irq_source = *((int*)IRQ_source);
    int aux_mu_iir = *((int*)IIR_ADDR);
    char IRQS[5], IIR[10];
    itoa(irq_source, IRQS, 4);
    itohexa(aux_mu_iir, IIR, 8);

    if(irq_source==256)
    {
        if(aux_mu_iir&0x02)
            TX_handler(IRQS, IIR);
        else if(aux_mu_iir&0x04)
            RX_handler(IRQS, IIR);
    }
    else if(irq_source==2)
        L1_timer_handler(cntpct_el0, cntfrq_el0);

}

void L1_timer_handler(long cntpct_el0, long cntfrq_el0)
{
    // disable core timer interrupt  
    asm volatile
    (
        "mov x3, 0\n\t"
        "msr cntp_ctl_el0, x3\n\t"
    );
    
    long nowtime = cntpct_el0/cntfrq_el0;
    char NOWTIME[10];
    
    itoa(nowtime, NOWTIME, 1);
    uart_puts("\nTime's out!!! Now time is ");
    uart_puts(NOWTIME);
    uart_puts(" s\n");

    uart_puts("Message : ");
    uart_puts(node[front].message);
    uart_puts("\n");

    front++;
    if(!is_empty())
    {
        find_min();
        asm volatile
        (
            "msr cntp_cval_el0, %0\n\t" // set expired time
            "bl core_timer_enable\n\t"
            :
            :"r"(node[front].second)
            :
        );
    }
}

void TX_handler(char* irq,char *iir)
{
    if(buffer_count<len_WB)
        *AUX_MU_IO=write_buffer[buffer_count++];
    else
    {
        *AUX_MU_IER=0;
        buffer_count=0;
    }
}

void RX_handler(char* irq,char *iir)
{
    read_buffer[len_RB++]=*AUX_MU_IO;
    
    if(read_buffer[len_RB-1]=='\r')
    {
        uart_puts("IRQ Source : ");
        uart_puts(irq);
        uart_puts(" ; IIR : ");
        uart_puts(iir);
        uart_puts("\n");
        *AUX_MU_IER=0;
        read_buffer[len_RB]='\0';
        len_RB=0;
    }
    // else
    // {
    //     uart_puts(read_buffer);
    //     uart_puts("\n");
    // }   
}
