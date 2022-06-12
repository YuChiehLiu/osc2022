#define CORE0_TIMER_IRQ_CTRL 0x40000040
.global core_timer_enable
.global core_timer_disable
.global set_switch_timer

core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    mrs x2, cntkctl_el1 // following three instructions for Lab5 to access cpu timer register
    orr x2, x2, #0x1
    msr cntkctl_el1, x2
    ret

core_timer_disable:
    mov x0, 0
    msr cntp_ctl_el0, x0 // disable
    ret

set_switch_timer:
    mrs x0, cntfrq_el0
    mov x0, x0, lsr#5
    msr cntp_tval_el0, x0
    ret

/*
    core time interrupt triggered condition is only "cntpct_el0 >= cval"
    
    cval's value is,
        if set cval -> cval = the value you set
        if set tval -> cval = cntpct_el0 + tval

    cntfrq_el0 holds the frequency in Hz.
*/