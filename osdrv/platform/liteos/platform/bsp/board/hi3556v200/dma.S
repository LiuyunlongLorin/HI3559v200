/*----------------------------------------------------------------------------
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
.global v7_dma_inv_range
.global v7_dma_clean_range
.global setjmp
.global longjmp
.global _platform_setup1

.extern hal_mmu_init

.fpu vfpv4
.arch armv7a
      .macro  dcache_line_size, reg, tmp
    mrc     p15, 0, \tmp, c0, c0, 1
    lsr     \tmp, \tmp, #16
    and     \tmp, \tmp, #0xf
    mov     \reg, #4
    mov     \reg, \reg, lsl \tmp
    .endm

     .macro FLUSH_DCACHE_ALL
    mrc     p15, 0, r10, c0, c1, 5
    tst     r10, #0xf << 16
    mov     r10, #0
    beq     hierarchical
    mcr     p15, 0, r10, c7, c14, 0
    b       iflush
    hierarchical:
    mcr     p15, 0, r10, c7, c10, 5
    stmfd   sp!, {r0-r7, r9-r11}
    mrc     p15, 1, r0, c0, c0, 1
    ands    r3, r0, #0x7000000
    mov     r3, r3, lsr #23
    beq     finished
    mov     r10, #0
    loop1:
    add     r2, r10, r10, lsr #1
    mov     r1, r0, lsr r2
    and     r1, r1, #7
    cmp     r1, #2
    blt     skip
    mcr     p15, 2, r10, c0, c0, 0
    mcr     p15, 0, r10, c7, c5, 4
    mrc     p15, 1, r1, c0, c0, 0
    and     r2, r1, #7
    add     r2, r2, #4
    ldr     r4, =0x3ff
    ands    r4, r4, r1, lsr #3
    clz     r5, r4
    ldr     r7, =0x7fff
    ands    r7, r7, r1, lsr #13
    loop2:
    mov     r9, r4
    loop3:
    orr     r11, r10, r9, lsl r5
    orr     r11, r11, r7, lsl r2

    mcr     p15, 0, r11, c7, c14, 2
    subs    r9, r9, #1
    bge     loop3
    subs    r7, r7, #1
    bge     loop2
    skip:
    add     r10, r10, #2
    cmp     r3, r10
    bgt     loop1
    finished:
    ldmfd   sp!, {r0-r7, r9-r11}
    mov     r10, #0
    mcr     p15, 2, r10, c0, c0, 0
    iflush:
    mcr     p15, 0, r10, c7, c10, 4
    mcr     p15, 0, r10, c7, c5, 0
    mcr     p15, 0, r10, c7, c10, 4
    mcr     p15, 0, r10, c7, c5, 4
 .endm


_platform_setup1:
    push {lr}

    bl  board_config


    mrc  p15,0,r0,c1,c0,0
    bic  r0,r0,#0x1000 //disable ICache [SCTRL:bit 12 set as 0]
    bic  r0,r0,#0x000f //disable DCache, write buffer,

    mcr  p15,0,r0,c1,c0,0
    nop
    nop
    mcr  p15,0,r0,c1,c0,0
    nop
    nop
    mov  r0,#0

    FLUSH_DCACHE_ALL

    bl  hal_mmu_init

    mrc     p15, 0, r0, c1, c0, 1 //ACTLR, Auxlliary Control Register, IMPLEMENTATION DEFINED
    orr     r0, #(1 << 6)  //SMP, Signals if the Cortex-A9 processor is taking part in coherency or not.
    mcr     p15, 0, r0, c1, c0, 1 //ACTLR, Auxlliary Control Register, IMPLEMENTATION DEFINED


    ldr     r2,=10f

    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4 //CP15DSB, Data Synchronization Barrier operation
    tst     r11, #0xf
    mcrne   p15, 0, r0, c8, c7, 0 //TBLIALL, invalidate unified TLB

    mrc     p15, 0, r0, c1, c0, 0 //SCTLB, System Control Register
    orr     r0, r0, #1<<12 // enable ICache
    orr     r0, r0, #1<<2 //Dcache enable
    orr     r0, r0, #1<<8 // 'S' bit
    //orr     r0, r0, #1<<1 // not check align
    orr     r0, r0, #1

    mcr     p15, 0, r0, c7, c5, 4 //CP15ISB, Instruction Synchronization Barrier operation
    mcr     p15, 0, r0, c1, c0, 0 //SCTLB, System Control Register
    mrc     p15, 0, r0, c1, c0, 0 //SCTLB, System Control Register

    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4 //CP15ISB, Instruction Synchronization Barrier operation

    mov        pc,r2
    nop
    nop
    nop
10:
    @b   warm_reset
    pop  {pc}

v7_dma_inv_range:
    push    {r2, r3}
    dcache_line_size r2, r3
    sub    r3, r2, #1
    tst    r0, r3
    bic    r0, r0, r3

    mcrne    p15, 0, r0, c7, c14, 1

    tst    r1, r3
    bic    r1, r1, r3
    mcrne    p15, 0, r1, c7, c14, 1
1:
    mcr    p15, 0, r0, c7, c6, 1
    add    r0, r0, r2
    cmp    r0, r1
    blo    1b
    dsb
    pop    {r2, r3}
    mov    pc, lr
.type v7_dma_inv_range,%function;
.size v7_dma_inv_range, .-v7_dma_inv_range

v7_dma_clean_range:
    push    {r2, r3}
    dcache_line_size r2, r3
    sub    r3, r2, #1
    bic    r0, r0, r3

1:
    mcr    p15, 0, r0, c7, c10, 1
    add    r0, r0, r2
    cmp    r0, r1
    blo    1b
    dsb
    pop    {r2, r3}
    mov    pc, lr
.type v7_dma_clean_range,%function;
.size v7_dma_clean_range, .-v7_dma_clean_range


longjmp:

        ldmfd   r0,{r4-r14}
        cmp     r1,#0
        moveq   r1,#1
        mov     r0,r1
        mov     pc,lr

.type longjmp,%function;
.size longjmp, .-longjmp


setjmp:
        stmea   r0,{r4-r14}
        mov     r0,#0

        mov     pc,lr

.type setjmp,%function;
.size setjmp, .-setjmp

