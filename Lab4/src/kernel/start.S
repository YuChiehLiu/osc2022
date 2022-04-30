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

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    bl      set_exception_vector_table
    bl      from_el2_to_el1
    # the next instruction runs in EL1

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      main
    // for failsafe, halt this core too
    b       1b

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

/*
hcr_el2 :   Hypervisor Configuration Register (64bit)
            bit[31] : 0b1 -> The Execution state for EL1 is AArch64. The Execution state for EL0 is determined by the current value of PSTATE.nRW when executing at EL0.

spsr_elx :  Saved Program Status Register (64bit)
            bit[11-10] : Reserved, RES0
            bit[9-6] : D,A,I,F mask
            bit[5] : Reserved, RES0
            bit[4] : 0 -> AArch64 ; 1 ->AArch32
            bit[3:0] : AArch64 Exception level and selected Stack Pointer
                        0b0000 -> EL0t
                        0b0100 -> EL1t
                        0b0101 -> EL1h
                        0b1000 -> EL2t
                        0b1001 -> EL2h

elr_elx :   Exception Link Register 64(bit)

lr :        link register
            hold the return address for a function call

spel :      t -> sp_el0 stack pointer is selected
            h -> sp_elx stack pointer is selected

esr_elx :   Exception Syndrome Register (64bit)
            Holds syndrome information for an exception taken to EL2  

reference : https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers?lang=en (AArch64 System Register)
*/