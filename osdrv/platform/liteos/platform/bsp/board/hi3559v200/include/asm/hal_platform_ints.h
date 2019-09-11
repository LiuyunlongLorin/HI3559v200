/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
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


#ifndef PLATFORM_HAL_PLATFORM_INTS_H
#define PLATFORM_HAL_PLATFORM_INTS_H

#include"los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts is 128.
 */
#define OS_HWI_MAX_NUM                  128

/**
 * Maximum interrupt number.
 */

#define OS_HWI_MAX                      ((OS_HWI_MAX_NUM) - 1)

/**
 * Minimum interrupt number.
 */

#define OS_HWI_MIN                      0

/**
 * Maximum usable interrupt number.
 */

#define OS_USER_HWI_MAX                 OS_HWI_MAX

/**
 * Minimum usable interrupt number.
 */

#define OS_USER_HWI_MIN                 OS_HWI_MIN

#define NUM_HAL_INTERRUPT_TIMER0        33
#define NUM_HAL_INTERRUPT_TIMER1        33
#define NUM_HAL_INTERRUPT_TIMER2        34
#define NUM_HAL_INTERRUPT_TIMER3        34
#define NUM_HAL_INTERRUPT_TIMER4        35
#define NUM_HAL_INTERRUPT_TIMER5        35
#define NUM_HAL_INTERRUPT_TIMER6        36
#define NUM_HAL_INTERRUPT_TIMER7        36

#define NUM_HAL_INTERRUPT_DMAC          60
#define NUM_HAL_INTERRUPT_UART0         38
#define NUM_HAL_INTERRUPT_UART1         39
#define NUM_HAL_INTERRUPT_UART2         40
#define NUM_HAL_INTERRUPT_UART3         41
#define NUM_HAL_INTERRUPT_UART4         42

#define NUM_HAL_INTERRUPT_SDIO          62
#define NUM_HAL_INTERRUPT_SDIO1         63

#define NUM_HAL_INTERRUPT_ETH           64

#define NUM_HAL_INTERRUPT_TIMER         NUM_HAL_INTERRUPT_TIMER4
#define NUM_HAL_INTERRUPT_HRTIMER       NUM_HAL_INTERRUPT_TIMER7

#define NUM_HAL_INTERRUPT_GPIO0         48 
#define NUM_HAL_INTERRUPT_GPIO1         49
#define NUM_HAL_INTERRUPT_GPIO2         50
#define NUM_HAL_INTERRUPT_GPIO3         51
#define NUM_HAL_INTERRUPT_GPIO4         52
#define NUM_HAL_INTERRUPT_GPIO5         53
#define NUM_HAL_INTERRUPT_GPIO6         54
#define NUM_HAL_INTERRUPT_GPIO7         55
#define NUM_HAL_INTERRUPT_GPIO8         56
#define NUM_HAL_INTERRUPT_GPIO9         57
#define NUM_HAL_INTERRUPT_GPIO10        58
#define NUM_HAL_INTERRUPT_GPIO11        112

#define NUM_HAL_INTERRUPT_NONE          -1

#define NUM_HAL_ISR_MIN                 OS_HWI_MIN 

#define IO_ADDRESS(x)       (x)

#define HAL_READ_UINT8(addr, data)  READ_UINT8(data, addr)

#define HAL_WRITE_UINT8(addr, data) WRITE_UINT8(data, addr)

#define HAL_READ_UINT32(addr, data) READ_UINT32(data, addr)

#define HAL_WRITE_UINT32(addr, data) WRITE_UINT32(data, addr)

VOID hal_interrupt_mask(unsigned int vector);
VOID hal_interrupt_unmask(unsigned int vector);
VOID hal_interrupt_init(VOID);

extern VOID hrtimer_clock_irqclear(VOID);
extern VOID hrtimer_clock_start(UINT32 period);
extern VOID hrtimer_clock_stop(VOID);
extern UINT32 get_hrtimer_clock_value(VOID);
extern VOID hrtimer_clock_initialize(VOID);


VOID tick_interrupt_mask(VOID);
VOID tick_interrupt_unmask(VOID);

#ifdef LOSCFG_KERNEL_TICKLESS
VOID tick_timer_reload(UINT32 period);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // PLATFORM_HAL_PLATFORM_INTS_H
