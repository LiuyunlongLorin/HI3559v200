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

#include "los_tick.ph"
#include "los_hwi.h"
#include "hisoc/timer.h"
#include "hisoc/sys_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

static UINT32 timestamp;
static UINT32 lastdec;

#define READ_TIMER    (*(volatile unsigned int *) \
                (TIMER_TICK_REG_BASE + TIMER_VALUE))

VOID reset_timer_masked(VOID)
{
    lastdec = READ_TIMER;
    timestamp = 0;
}

UINT32 get_timer_masked(VOID)
{
    UINT32 now = READ_TIMER;

    if (lastdec >= now) {
        /* not roll back */
        timestamp += lastdec - now;
    } else {
        /* rollback */
        timestamp += lastdec + (g_uwSysClock/LOSCFG_BASE_CORE_TICK_PER_SECOND) - now;
    }

    lastdec = now;
    return timestamp;
}

#if (!defined(LOSCFG_PLATFORM_HI3731) && !defined(LOSCFG_PLATFORM_HI3911) && !defined(LOSCFG_PLATFORM_HIM5V100))
VOID hrtimer_clock_irqclear(VOID)
{
    WRITE_UINT32(1, HRTIMER_TIMER_REG_BASE + TIMER_INT_CLR);
}

VOID hrtimer_clock_initialize(VOID)
{
    UINT32 temp;

    /*
    *enable timer here,
    */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= HRTIMER_TIMER_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /* disable timer */
    WRITE_UINT32(0x0, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);

    /*
     * Timing mode:oneshot [bit 0 set as 1]
     * timersize:32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * interrupt enabled [bit 5 set as 1]
     * timing circulary [bit 6 set as 1]
     */
     temp = (1 << 6) | (1 << 5) | (1 << 1) | (1 << 0);
    WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

VOID hrtimer_clock_start(UINT32 period)
{
    UINT32 temp;

    /* set init value as period */
    WRITE_UINT32(period, HRTIMER_TIMER_REG_BASE + TIMER_LOAD);

    /*timer enabled [bit 7 set as 1]*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
    temp |= 1 << 7;
    WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

VOID hrtimer_clock_stop(VOID)
{
    UINT32 temp;

     /*timer disabled [bit 7 set as 0]*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
    temp &= ~(1 << 7);
    WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

UINT32 get_hrtimer_clock_value(VOID)
{
    UINT32 temp;

    /*Read the current value of the timer3*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_VALUE);
    return temp;
}
#endif

UINT32 time_clk_read(VOID)
{
    UINT32 value;

    READ_UINT32(value, TIMER_TIME_REG_BASE + TIMER_VALUE);
    value = TIMER_MAXLOAD - value;

    return value;
}

unsigned int arch_timer_rollback(VOID)
{
    UINT32 flag;

    READ_UINT32(flag, TIMER_TICK_REG_BASE + TIMER_RIS);
    return flag;
}

//this func is start timer2 for start time
VOID hal_clock_initialize_start(VOID)
{
    UINT32 temp;
    /*
     * enable timer2 here,
     * but only time0 is used for system clock.
     */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= TIMER2_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/256 clock frequency [bit 3 set as 1, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */

    /* init the timestamp and lastdec value */
    reset_timer_masked();

    /* disable timer2 */
    WRITE_UINT32(0x0, TIMER2_REG_BASE + TIMER_CONTROL);
    /* set init value as period */
    WRITE_UINT32(0xffffffff, TIMER2_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/256 clock frequency [bit 3 set as 1, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */
    temp = (1 << 7) | (1 << 6) | (1 << 3) | (1 << 1);
    WRITE_UINT32(temp, TIMER2_REG_BASE + TIMER_CONTROL);
}

UINT32 GetTimer2Value(VOID)
{
    UINT32 temp;

    READ_UINT32(temp, TIMER2_REG_BASE + TIMER_VALUE);
    return temp;
}

UINT32 GetTickTimerValue(VOID)
{
    UINT32 temp;

    READ_UINT32(temp, TIMER_TICK_REG_BASE + TIMER_VALUE);
    return temp;
}

/*
 * get the system ms clock since the system start
 */
UINT32 hi_getmsclock(VOID)
{
    UINT32 t32 = 0xffffffff - GetTimer2Value();
    UINT64 t64 = (UINT64)t32 * 256;

    return (UINT32)(t64 / (OS_TIME_TIMER_CLOCK/1000));
}

extern VOID platform_hal_clock_initialize(UINT32 period);

VOID hal_clock_initialize(UINT32 period)
{
    platform_hal_clock_initialize(period);
}

VOID hal_clock_irqclear(VOID)
{
#ifndef HI3911
    WRITE_UINT32(1, TIMER_TICK_REG_BASE + TIMER_INT_CLR);
#else
    UINT32 eio = 0;
    READ_UINT32(eio,TIMER_TICK_REG_BASE + TIMER_INT_CLR);
#endif
}

VOID hal_clock_enable(VOID)
{
#ifndef HI3911
    GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) = GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) | (1 << 7);
#else
    /*open HI3911 TIMER0*/
    GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) = GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) | (1 << 0);
#endif
}


// Delay for some number of micro-seconds

UINT32 get_timer(UINT32 base)
{
    return get_timer_masked() - base;
}

VOID hal_delay_us(UINT32 usecs)
{
    UINT32 tmo = 0;
    UINT32 tmp = 0;
    UINT32 uwRet;

    uwRet = LOS_IntLock();

    if (usecs >= 1000) {
        /* start to normalize for usec to ticks per sec */
        tmo = usecs / 1000;

        /* tmo *= 50000000 / 1000; */
        tmo *= (g_uwSysClock / 1000);//largest msecond 1374389

        /* usecs < 1000 */
        usecs -= (usecs / 1000 * 1000);
    }

    /* usecs < 1000 */
    if(usecs) {
        /*
         * translate us into sys_clock
         * prevent u32 overflow
         * */
        tmo += (usecs * (g_uwSysClock / 1000)) / 1000;
    }

    /* reset "advancing" timestamp to 0, set lastdec value */
    reset_timer_masked();

    tmp = get_timer(0);     /* get current timestamp */

    /* set advancing stamp wake up time */
    tmo += tmp;

    while (get_timer_masked() < tmo) ;
    LOS_IntRestore(uwRet);
}

VOID LOS_Udelay(UINT32 usecs)
{
    hal_delay_us(usecs);
}

VOID LOS_Mdelay(UINT32 msecs)
{
    LOS_Udelay(msecs * 1000);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

