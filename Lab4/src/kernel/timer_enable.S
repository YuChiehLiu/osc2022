#define CORE0_TIMER_IRQ_CTRL 0x40000040
.global core_timer_enable

core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret

/*
    core time interrupt triggered condition is only "cntpct_el0 >= cval"
    
    cval's value is,
        if set cval -> cval = the value you set
        if set tval -> cval = cntpct_el0 + tval

    cntfrq_el0 holds the frequency in Hz.
*/