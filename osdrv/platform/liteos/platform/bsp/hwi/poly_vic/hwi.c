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
#include "los_toolchain.h"
#ifdef LOSCFG_KERNEL_TICKLESS
#include "los_tickless.ph"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_TICKLESS
extern VOID hal_interrupt_acknowledge(unsigned int vector);
extern VOID hal_clock_irqclear(VOID);
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

extern UINT32 g_vuwHwiFormCnt[OS_HWI_MAX_NUM];
size_t g_curirqnum = 0;
typedef VOID (*HWI_PROC_FUNC0)(void);
typedef VOID (*HWI_PROC_FUNC2)(int,void *);
void hal_interrupt_acknowledge(unsigned int vector);

static inline void hal_IRQ_Done(UINT32 index)
{
    HWI_HANDLE_FORM_S *pstHwiForm = (HWI_HANDLE_FORM_S *)NULL;
    g_curirqnum = index;
#ifdef LOSCFG_KERNEL_TICKLESS
    if(g_uwSleepTicks > 1)
    {
        tickless_update(index);
    }
#endif
    pstHwiForm = &m_astHwiForm[index];
    while (NULL != pstHwiForm->pstNext)
    {
        pstHwiForm = pstHwiForm->pstNext;
        if (!pstHwiForm->uwParam)
        {
            HWI_PROC_FUNC0 func0 = (HWI_PROC_FUNC0)(pstHwiForm->pfnHook);
            func0();
        }
        else
        {
            HWI_PROC_FUNC2 func2 = (HWI_PROC_FUNC2)(pstHwiForm->pfnHook);
            unsigned int *p = (unsigned int *)(pstHwiForm->uwParam);
            func2((int)(*p), (void *)(*(p + 1)));
        }
        ++g_vuwHwiFormCnt[index];
    }
    hal_interrupt_acknowledge(index);

}

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
extern VOID osCpupIrqStart(VOID);
extern VOID osCpupIrqEnd(VOID);
extern UINT32 g_uwIntNo;
#endif

void hal_IRQ_handler(void)
{
    UINT32 i = 0;
    UINT32 irqstatus0 = 0x0;
    UINT32 irqstatus1 = 0x0;

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    osCpupIrqStart();
#endif
    READ_UINT32(irqstatus0, IRQ0_REG_BASE + IRQ_INT_IRQSTATUS);
    if(irqstatus0 & (1U << 31))
    {
        READ_UINT32(irqstatus1, IRQ1_REG_BASE + IRQ_INT_IRQSTATUS);
        i = 31 - CLZ(irqstatus1);
        hal_IRQ_Done(i + OS_VIC0_IRQMAX);
    }
    else
    {
        i = 31 - CLZ(irqstatus0);
        hal_IRQ_Done(i);
    }
#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    g_uwIntNo = i;
    osCpupIrqEnd();
#endif
}

//----------------------------------------------------------------------------
// Interrupt control
VOID hal_interrupt_mask(unsigned int vector)
{
    UINT32 imr  = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    if (vector < OS_VIC0_IRQMAX)
    {
        imr = 1U << vector;
         WRITE_UINT32(imr, IRQ0_REG_BASE + IRQ_INT_INTENCLEAR);
    }
    else
    {
        imr = 1U << (vector - OS_VIC0_IRQMAX);
         WRITE_UINT32(imr, IRQ1_REG_BASE + IRQ_INT_INTENCLEAR);
    }

}

VOID hal_interrupt_unmask(unsigned int vector)
{
    UINT32 imr = 0;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    if (vector < OS_VIC0_IRQMAX)
    {
        imr = 1U << vector;
        READ_UINT32(status, IRQ0_REG_BASE + IRQ_INT_INTENABLE);
        WRITE_UINT32(imr | status, IRQ0_REG_BASE + IRQ_INT_INTENABLE);
    }
    else
    {
        imr = 1U << (vector - OS_VIC0_IRQMAX);
        READ_UINT32(status, IRQ1_REG_BASE + IRQ_INT_INTENABLE);
        WRITE_UINT32(imr | status, IRQ1_REG_BASE + IRQ_INT_INTENABLE);
    }
}

VOID hal_interrupt_acknowledge(unsigned int vector)
{
    UINT32 imr = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    if (vector < OS_VIC0_IRQMAX)
    {
        imr = 1U << vector;
        WRITE_UINT32(imr, IRQ0_REG_BASE + IRQ_INT_SOFTINTCLEAR);
    }
    else
    {
        imr = 1U << (vector - OS_VIC0_IRQMAX);
        WRITE_UINT32(imr, IRQ1_REG_BASE + IRQ_INT_SOFTINTCLEAR);
    }
}

VOID hal_interrupt_pending(unsigned int vector)
{
    UINT32 imr = 0;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    if (vector < OS_VIC0_IRQMAX)
    {
        imr = 1U << vector;
        READ_UINT32(status, IRQ0_REG_BASE + IRQ_INT_SOFTINT);
        WRITE_UINT32(imr | status, IRQ0_REG_BASE + IRQ_INT_SOFTINT);
    }
    else
    {
        imr = 1U << (vector - OS_VIC0_IRQMAX);
        READ_UINT32(status, IRQ1_REG_BASE + IRQ_INT_SOFTINT);
        WRITE_UINT32(imr | status, IRQ1_REG_BASE + IRQ_INT_SOFTINT);
    }
}

void hal_interrupt_init(void)
{
    UINT32 imr = 0;
    UINT32 status = 0;
    /*VIC0*/
    WRITE_UINT32(0xffffffff, IRQ0_REG_BASE + IRQ_INT_INTENCLEAR);    /*mask all irq*/
    WRITE_UINT32(0x00000000, IRQ0_REG_BASE + IRQ_INT_INTSELECT);     /*types of all interrupt are IRQ*/

    /*Enable VIC1 IRQ*/
    imr = 1U << 31;
    READ_UINT32(status, IRQ0_REG_BASE + IRQ_INT_INTENABLE);
    WRITE_UINT32(imr | status, IRQ0_REG_BASE + IRQ_INT_INTENABLE);
    /*VIC1*/
    WRITE_UINT32(0xffffffff, IRQ1_REG_BASE + IRQ_INT_INTENCLEAR);    /*mask all irq*/
    WRITE_UINT32(0x00000000, IRQ1_REG_BASE + IRQ_INT_INTSELECT);     /*types of all interrupt are IRQ*/

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
