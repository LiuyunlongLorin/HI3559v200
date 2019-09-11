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

#include "los_hwi.h"
#include "hisoc/cpu.h"
#include "hisoc/sys_ctrl.h"
#include "los_hw_tick_minor.h"
#ifdef LOSCFG_KERNEL_TICKLESS
#include "los_tickless.ph"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#ifdef LOSCFG_KERNEL_TICKLESS
extern VOID hal_clock_irqclear(VOID);
extern VOID hal_interrupt_acknowledge(unsigned int vector);
VOID tick_timer_reload(UINT32 period)
{
    UINT32 cycles_pertick;
    cycles_pertick = g_uwSysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    WRITE_UINT32(period, TIMER_TICK_REG_BASE + TIMER_LOAD);
    WRITE_UINT32(cycles_pertick, TIMER_TICK_REG_BASE + TIMER_BGLOAD);

    hal_clock_irqclear();
    hal_interrupt_acknowledge(NUM_HAL_INTERRUPT_TIMER);
}

VOID tickless_update(UINT32 irqnum)
{
    UINT32 cycles,ticks;
    UINT32 cycles_pertick;

    cycles_pertick = g_uwSysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    if(irqnum == NUM_HAL_INTERRUPT_TIMER)
    {
        osSysTimeUpdate(g_uwSleepTicks);
    }
    else
    {
        READ_UINT32(cycles, TIMER_TICK_REG_BASE + TIMER_VALUE);
        cycles = ((g_uwSleepTicks * cycles_pertick) - cycles);
        ticks =cycles / cycles_pertick;
        cycles = cycles % cycles_pertick;

        osSysTimeUpdate(ticks + 1);
        tick_timer_reload(cycles_pertick - cycles);
    }
    g_uwSleepTicks = 0;
}
#endif

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

/* only one GIC */
extern UINT32 g_vuwHwiFormCnt[OS_HWI_MAX_NUM];
size_t g_curirqnum = 0;
size_t g_cmpirqnum = 0;

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
extern UINT32 g_uwIntNo;
#endif

VOID hal_IRQ_handler(VOID)
{
    g_cmpirqnum = 1;
    READ_UINT32(g_curirqnum, CFG_ARM_GIC_CPU_BASE + ARM_GIC_CPU_INTACK);
    /*The 0x3ff interrupt is a fake irq,just ignored.*/
    g_curirqnum &= 0x3ff;
    /*Just care about the g_curirqnum is less then OS_HWI_MAX_NUM.*/
    if (g_curirqnum >= OS_HWI_MAX_NUM)
    {
        g_cmpirqnum = 0;
        return;
    }

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    g_uwIntNo = g_curirqnum;
#endif

    g_vuwHwiFormCnt[g_curirqnum]++;
#ifdef LOSCFG_KERNEL_TICKLESS
    if(g_uwSleepTicks > 1)
    {
        tickless_update(g_curirqnum);
    }
#endif
}

//----------------------------------------------------------------------------
// Interrupt control

VOID hal_interrupt_mask(unsigned int vector)
{
    UINT32 imr = 1 << (vector % 32);

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, CFG_ARM_GIC_DIST_BASE + ARM_GIC_DIST_ENABLE_CLEAR + (vector / 32) * 4);
}

VOID hal_interrupt_unmask(unsigned int vector)
{
    UINT32 imr = 1 << (vector % 32);

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, CFG_ARM_GIC_DIST_BASE + ARM_GIC_DIST_ENABLE_SET + (vector / 32) * 4);
}

VOID hal_interrupt_pending(unsigned int vector)
{
    UINT32 imr = 1 << (vector % 32);

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, CFG_ARM_GIC_DIST_BASE + ARM_GIC_DIST_PENDING_SET + (vector / 32) * 4);
}

VOID hal_interrupt_acknowledge(unsigned int vector)
{
    WRITE_UINT32(vector, CFG_ARM_GIC_CPU_BASE + ARM_GIC_CPU_EOI);
}

extern VOID platform_gic_dist_init(unsigned int gic_nr, VOID *base, unsigned int irq_start);
VOID gic_dist_init(unsigned int gic_nr, VOID *base, unsigned int irq_start)
{
    platform_gic_dist_init(gic_nr, base, irq_start);

}

VOID gic_cpu_init(unsigned int gic_nr, VOID *base)
{
    AARCHPTR addr_base = (AARCHPTR)base;
    /*
     * deal with the banked PPI and SGI interrupts:
     * disable all PPI interrupts, ensure all SGI interrupts are enabled.
     */

    /*
     * set priority on PPI and SGI
     */

    WRITE_UINT32(0xf0, addr_base + ARM_GIC_CPU_PRIMASK);
    WRITE_UINT32(0x1, addr_base + ARM_GIC_CPU_CTRL);
}

VOID hal_interrupt_init(VOID)
{
    gic_dist_init(0, (VOID *)CFG_ARM_GIC_DIST_BASE, NUM_HAL_ISR_MIN);
    gic_cpu_init(0, (VOID *)CFG_ARM_GIC_CPU_BASE);
}

UINT32 random_stack_guard(VOID)
{
    UINT32 uwHwCycle = SysTick->uwVALUE;
    return uwHwCycle;
}

VOID tick_interrupt_mask(VOID)
{
    hal_interrupt_mask(NUM_HAL_INTERRUPT_TIMER);
}

VOID tick_interrupt_unmask(VOID)
{
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_TIMER);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
