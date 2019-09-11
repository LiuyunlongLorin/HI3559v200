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
#include "sys_config.h"
.equ CPSR_IRQ_DISABLE,  0x80    /* IRQ disabled when =1 */
.equ CPSR_FIQ_DISABLE, 0x40    /* FIQ disabled when =1 */
.equ CPSR_THUMB_ENABLE, 0x20    /* Thumb mode when   =1 */
.equ CPSR_USER_MODE, 0x10
.equ CPSR_FIQ_MODE, 0x11
.equ CPSR_IRQ_MODE, 0x12
.equ CPSR_SVC_MODE, 0x13
.equ CPSR_ABT_MODE,0x17
.equ CPSR_UNDEF_MODE, 0x1B

.global __exc_stack_top
.global __irq_stack_top
.global __fiq_stack_top
.global __svc_stack_top
.global __abt_stack_top
.global __undef_stack_top
.global __undef_stack
.extern uart_init
.global warm_reset
.extern __ram_data_start
.extern __ram_data_end
.extern __rom_data_start
.extern __sram_data_start
.extern __sram_data_end
.extern __srom_data_start
.extern __bss_start
.extern __bss_end
.extern board_config
.extern hal_clock_initialize_start
.extern los_bss_init
.extern _osExceptFiqHdl
.extern _osExceptAddrAbortHdl
.extern _osExceptDataAbortHdl
.extern _osExceptPrefetchAbortHdl
.extern _osExceptSwiHdl
.extern _osExceptUndefInstrHdl
.extern __stack_chk_guard_setup
.fpu vfpv4
@.fpu neon
.arch armv7a

    .code   32
    .section ".vectors","ax"

    .global __exception_handlers
__exception_handlers:
    /*
    *Assumption:  ROM code has these vectors at the hardware reset address.
    *A simple jump removes any address-space dependencies [i.e. safer]
    */
    b   reset_vector
    b   _osExceptUndefInstrHdl
    b   _osExceptSwiHdl
    b   _osExceptPrefetchAbortHdl
    b   _osExceptDataAbortHdl
    b   _osExceptAddrAbortHdl
    b   _osExceptIrqHdl
    b   _osExceptFiqHdl

    .text
    /* Startup code which will get the machine into supervisor mode */
    .global reset_vector
    .type   reset_vector,function
reset_vector:
warm_reset:
    /* initialize interrupt/exception environments */
    mov     r0,#(CPSR_IRQ_DISABLE |CPSR_FIQ_DISABLE|CPSR_IRQ_MODE)
    msr     cpsr,r0
    ldr     sp, =__irq_stack_top

    mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_UNDEF_MODE)
    msr     cpsr,r0
    ldr     sp, =__undef_stack_top

    mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_ABT_MODE)
    msr     cpsr,r0
    ldr     sp, =__abt_stack_top

    mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_FIQ_MODE)
    msr     cpsr,r0
    ldr     sp, =__fiq_stack_top

    /* initialize CPSR (machine state register) */
    mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SVC_MODE)
    msr     cpsr,r0

    /* Note: some functions in LIBGCC1 will cause a "restore from SPSR"!! */
    msr     spsr,r0

    /* initialize stack */
    ldr     sp, =__svc_stack_top

    bl board_config
    bl hal_clock_initialize_start
    bl _platform_setup1

    /* enable fpu+neon */
    MRC p15, 0, r0, c1, c1, 2
    ORR r0, r0, #0xC00
    BIC r0, r0, #0xC000
    MCR p15, 0, r0, c1, c1, 2

    LDR r0, =(0xF << 20)
    MCR p15, 0, r0, c1, c0, 2

    MOV r3, #0x40000000
    VMSR FPEXC, r3

    LDR     r0, =__exception_handlers
    MCR     p15, 0, r0, c12, c0, 0

#ifdef LOSCFG_KERNEL_RUNSTOP
    bl     osSystemWakeup
#endif

 clear_bss:
        ldr r1, =__bss_start
        ldr r2, =__bss_end
        mov r0,#0

bss_loop:
        cmp r1,r2
        strlo r0,[r1],#4
        blo bss_loop

#if defined(LOSCFG_CC_STACKPROTECTOR_ALL) || defined(LOSCFG_CC_STACKPROTECTOR)
    bl      __stack_chk_guard_setup
#endif

    bl      uart_init
#ifdef LOSCFG_GDB_DEBUG
    /* GDB_START - generate a compiled_breadk,This function will get GDB stubs started, with a proper environment */
    bl      GDB_START
    .word      0xe7ffdeff
#endif

    bl      main

_start_hang:
    b       _start_hang
    .code   32

    .global reset_platform
    .type   reset_platform,function
reset_platform:
#ifdef A7SEM_HAL_ROM_MONITOR
    @ initialize CPSR (machine state register)
    mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SVC_MODE)
    msr     cpsr,r0
    b       warm_reset
#else
    mov     r0,#0
    mov     pc,r0           @ Jump to reset vector
#endif

init_done:
    .long   0xDEADB00B

    .code 32
    .data

init_flag:
    .balign 4
    .long   0

    /*
    * Temporary interrupt stack
    */
    .section ".int_stack", "wa", %nobits
    .align 3

__undef_stack:
#ifdef LOSCFG_GDB
    .space 512
#else
    .space 32
#endif
__undef_stack_top:

__abt_stack:
#ifdef LOSCFG_GDB
    .space 512
#else
    .space 32
#endif
__abt_stack_top:

__irq_stack:
    .space 64
__irq_stack_top:

__fiq_stack:
    .space 64
__fiq_stack_top:

__svc_stack:
    .space 8192
__svc_stack_top:

__exc_stack:
    .space 512
__exc_stack_top:
