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
#if defined LOSCFG_PLATFORM_HI3518EV200 || defined LOSCFG_PLATFORM_HI3516CV300
#include "hisoc/random.h"
#endif
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

typedef VOID (*HWI_PROC_FUNC0)(void);
typedef VOID (*HWI_PROC_FUNC2)(int,void *);
void hal_interrupt_acknowledge(unsigned int vector);
size_t g_curirqnum = 0;

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
extern VOID osCpupIrqStart(VOID);
extern VOID osCpupIrqEnd(VOID);
extern UINT32 g_uwIntNo;
#endif

void hal_IRQ_handler(void)
{
    unsigned int i = 0;
    UINT32 irqstatus = 0x0;
    HWI_HANDLE_FORM_S *pstHwiForm = NULL;

    READ_UINT32(irqstatus, IRQ_REG_BASE + IRQ_INT_IRQSTATUS);
    if (!irqstatus)
        return;
    i = 31 - CLZ(irqstatus);
    g_curirqnum = i;
#ifdef LOSCFG_KERNEL_TICKLESS
    if(g_uwSleepTicks > 1)
    {
        tickless_update(i);
    }
#endif

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    osCpupIrqStart();
#endif

    pstHwiForm = &m_astHwiForm[i];
    while (NULL != pstHwiForm->pstNext) {
        pstHwiForm = pstHwiForm->pstNext;
        if (!pstHwiForm->uwParam) {
            HWI_PROC_FUNC0 func0 = (HWI_PROC_FUNC0)(pstHwiForm->pfnHook);
            func0();
        } else {
            HWI_PROC_FUNC2 func2 = (HWI_PROC_FUNC2)(pstHwiForm->pfnHook);
            unsigned int *p = (unsigned int *)(pstHwiForm->uwParam);
            func2((int)(*p), (void *)(*(p + 1)));
        }
        ++g_vuwHwiFormCnt[i];
    }
    hal_interrupt_acknowledge(i);

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    g_uwIntNo = i;
    osCpupIrqEnd();
#endif

}

//----------------------------------------------------------------------------
// Interrupt control
VOID hal_interrupt_mask(unsigned int vector)
{
    UINT32 imr = 1 << vector;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);
}

VOID hal_interrupt_unmask(unsigned int vector)
{
    UINT32 imr = 1 << vector;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    READ_UINT32(status, IRQ_REG_BASE + IRQ_INT_INTENABLE);
    WRITE_UINT32(imr | status, IRQ_REG_BASE + IRQ_INT_INTENABLE);
#ifdef LOSCFG_PLATFORM_HI3911
    status = 0;
    imr = ~(1 << vector);
    READ_UINT32(status, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);
    WRITE_UINT32(imr & status, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);
#endif
}

VOID hal_interrupt_acknowledge(unsigned int vector)
{
    UINT32 imr = 1 << vector;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

#ifndef LOSCFG_PLATFORM_HI3911
    WRITE_UINT32(imr, IRQ_REG_BASE + IRQ_INT_SOFTINTCLEAR);
#else
    imr = ~(1 << vector);
    WRITE_UINT32(imr, IRQ_REG_BASE + IRQ_INT_SOFTINT);
#endif
}

VOID hal_interrupt_pending(unsigned int vector)
{
    UINT32 imr = 1 << vector;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    READ_UINT32(status, IRQ_REG_BASE + IRQ_INT_SOFTINT);
    WRITE_UINT32(imr | status, IRQ_REG_BASE + IRQ_INT_SOFTINT);
}

void hal_interrupt_init(void)
{
    WRITE_UINT32(0xffffffff, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);    /*mask all irq*/
#ifndef LOSCFG_PLATFORM_HI3911
    WRITE_UINT32(0x00000000, IRQ_REG_BASE + IRQ_INT_INTSELECT);     /*types of all interrupt are IRQ*/
#endif
}

UINT32 random_stack_guard(VOID)
{
    UINT32 result;
#if defined LOSCFG_PLATFORM_HI3911 || defined LOSCFG_PLATFORM_HIM5V100
    result = SysTick->uwVALUE;
#else
    int ret;
    ret = hi_random_hw_init();
    if (ret < 0) PRINT_ERR("hi_random_hw_init Error!\n");

    ret = hi_random_hw_getinteger(&result);
    if (ret < 0)
    {
        PRINT_ERR("hi_random_hw_getinteger Error!\n");
        result = SysTick->uwVALUE;
    }

    ret = hi_random_hw_deinit();
    if (ret < 0) PRINT_ERR("hi_random_hw_deinit Error!\n");
#endif
    return result;

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
