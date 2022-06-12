void default_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far);

/* Exception from the currentEL while using SP_ELx */
void IRQ_parser(long cntpct_el0, long cntfrq_el0, void *IRQ_source, void *IIR_ADDR);
void L1_timer_handler(long cntpct_el0, long cntfrq_el0);
void TX_handler(char *irq, char *iir);
void RX_handler(char *irq, char *iir);

/* Exception from a lower EL and at least one lower EL is AARCH64 */
void IRQ_parser_0(long cntpct_el0, long cntfrq_el0, void *IRQ_source, void *IIR_ADDR);
void L0_to_L1_handler(unsigned long trapframe_addr);
void L0_timer_handler(long cntpct_el0, long cntfrq_el0);

/* ASM code */
void enable_interrupt();
void disable_interrupt();