void exc_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far);
void L0_timer_handler(long cntpct_el0, long cntfrq_el0);
void IRQ_parser(long cntpct_el0, long cntfrq_el0, void* IRQ_source, void* IIR_ADDR);
void L1_timer_handler(long cntpct_el0, long cntfrq_el0);
void TX_handler(char* irq,char *iir);
void RX_handler(char* irq,char *iir);