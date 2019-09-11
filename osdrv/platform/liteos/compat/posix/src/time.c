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


#include "asm/hal_platform_ints.h"
#include "hisoc/timer.h"
#include "errno.h"
#include "time.h"
#include "unistd.h"
#include "signal.h"
#include "fcntl.h"
#include "stdio.h"
#include "string.h"
#include "sys/times.h"
#include "time.h"
#include "stdint.h"
#include "los_swtmr.ph"
#include "los_sys.ph"
#include "los_tick.ph"

extern unsigned int arch_timer_rollback(void);
extern unsigned long long hi_sched_clock(void);
// Do a time package defined return. This requires the error code
// to be placed in errno, and if it is non-zero, -1 returned as the
// result of the function. This also gives us a place to put any
// generic tidyup handling needed for things like signal delivery and
// cancellation.
#define TIME_RETURN(err)    \
    do {    \
        int __retval = 0;   \
        if (err != 0) {    \
            __retval = -1; \
            errno = err;   \
        }                   \
        return __retval;    \
    } while (0) //lint -e506

//==========================================================================
#ifdef LOSCFG_AARCH64
#define timeval64 timeval
#define timespec64 timespec
#endif

// internal functions
static inline BOOL valid_timespec(const struct timespec *tp)
{
    /* Fail a NULL pointer */
    if (tp == NULL) {
        return FALSE;
    }

    /* Fail illegal nanosecond values */
    if (tp->tv_nsec < 0 || tp->tv_nsec >= 1000000000 || tp->tv_sec < 0) {
        return FALSE;
    }

    return TRUE;
}

static inline BOOL valid_timeval(const struct timeval *tv)
{
    /* Fail a NULL pointer */
    if (tv == NULL) {
        return FALSE;
    }

    /* Fail illegal microseconds values */
    if (tv->tv_usec < 0 || tv->tv_usec >= 1000000 || tv->tv_sec < 0) {
        return FALSE;
    }

    return TRUE;
}

static inline BOOL valid_timeval64(const struct timeval64 *tv)
{
    /* Fail a NULL pointer */
    if (tv == NULL) {
        return FALSE;
    }

    /* Fail illegal microseconds values */
    if (tv->tv_usec < 0 || tv->tv_usec >= 1000000 || tv->tv_sec < 0) {
        return FALSE;
    }

    return TRUE;
}

//==========================================================================
static long long adjTimeLeft; /* absolute value of adjtime */
static int adjDirection; /* 1, speed up; 0, slow down; */

/*Adjust pacement, nanoseconds per 100 ticks*/
static long long adjPacement = ((LOSCFG_BASE_CORE_ADJ_PER_SECOND *100 /LOSCFG_BASE_CORE_TICK_PER_SECOND) * 1000);

/* accumulative time delta from continuous modify, such as adjtime */
static struct timespec64 accDeltaFromAdj;
/* accumulative time delta from discontinuous modify, such as settimeofday */
static struct timespec64 accDeltaFromSet ;

/*****************************************************************************
Function   : os_adjtime
Description: deal with the adjtime
Input      : None
Output     : None
Return     : None
*****************************************************************************/
void os_adjtime(void)
{
    if(!adjTimeLeft)
    {
        return;
    }

    if (adjTimeLeft > adjPacement)
    {
        if (adjDirection) {
            if (accDeltaFromAdj.tv_nsec + adjPacement >= 1000000000L)
            {
                accDeltaFromAdj.tv_sec ++;
                accDeltaFromAdj.tv_nsec  = (accDeltaFromAdj.tv_nsec + adjPacement) % 1000000000L;
            }
            else
            {
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec + adjPacement;
            }
        }
        else {
            if (accDeltaFromAdj.tv_nsec - adjPacement < 0)
            {
                accDeltaFromAdj.tv_sec --;
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec - adjPacement + 1000000000L;
            }
            else
            {
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec - adjPacement;
            }
        }

        adjTimeLeft -= adjPacement;
    }
    else
    {
        if(adjDirection)
        {
            if(accDeltaFromAdj.tv_nsec + adjTimeLeft >= 1000000000L)
            {
                accDeltaFromAdj.tv_sec ++;
                accDeltaFromAdj.tv_nsec  = (accDeltaFromAdj.tv_nsec + adjTimeLeft) % 1000000000L;
            }
            else
            {
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec + adjTimeLeft;
            }
        }
        else
        {
            if(accDeltaFromAdj.tv_nsec - adjTimeLeft < 0)
            {
                accDeltaFromAdj.tv_sec --;
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec - adjTimeLeft + 1000000000L;
            }
            else
            {
                accDeltaFromAdj.tv_nsec  = accDeltaFromAdj.tv_nsec - adjTimeLeft;
            }
        }

        adjTimeLeft = 0;
    }
    return;
}

/*****************************************************************************
Function: adjtime
Description:  correct the time to synchronize the system clock
Input:   delta - The amount of time by which the clock is to be adjusted
Output: olddelta - the amount of time remaining from any previous
                         adjustment that  has  not  yet  been completed.
Return: On success, returns 0.  On failure, -1 is returned, and errno is set to indicate the error.
*****************************************************************************/
int adjtime(const struct timeval *delta, struct timeval *olddelta)
{
    unsigned int uwIntSave = LOS_IntLock();
    /* return the amount of time remaining from any previous adjustment that has not yet been completed. */
    if (olddelta != NULL)
    {
        if (adjDirection == 1)
        {
            olddelta->tv_sec = adjTimeLeft / 1000000000L;
            olddelta->tv_usec = (adjTimeLeft % 1000000000L) / 1000L;
        }
        else
        {
            olddelta->tv_sec =  - adjTimeLeft / 1000000000L;
            olddelta->tv_usec = - (adjTimeLeft % 1000000000L) / 1000L;
        }
    }

    if (delta == NULL || (delta->tv_sec == 0 && delta->tv_usec == 0)) {
        (void)LOS_IntRestore(uwIntSave);
        return 0;
    }

    if (delta->tv_usec > 1000000L || delta->tv_usec < -1000000L) {
        (void)LOS_IntRestore(uwIntSave);
        TIME_RETURN(EINVAL);
    }

    if (delta->tv_sec < (INT_MIN / 1000000L + 2) || delta->tv_sec > (INT_MAX / 1000000L + 2)) {
        (void)LOS_IntRestore(uwIntSave);
        TIME_RETURN(EINVAL);
    }

    adjTimeLeft = delta->tv_sec * 1000000000LL + delta->tv_usec * 1000LL;
    if (adjTimeLeft > 0)
    {
        adjDirection = 1;
    }
    else
    {
        adjDirection = 0;
        adjTimeLeft = -adjTimeLeft;
    }

    (void)LOS_IntRestore(uwIntSave);
    return 0;
}

static inline struct timespec64 osTimeSpecAdd(const struct timespec64 t1, const struct timespec64 t2)
{
    struct timespec64 ret = {0};

    ret.tv_sec = t1.tv_sec + t2.tv_sec;
    ret.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    if(ret.tv_nsec > 1000000000L)
    {
        ret.tv_sec += 1;
        ret.tv_nsec -= 1000000000L;
    }
    else if(ret.tv_nsec < 0L)
    {
        ret.tv_sec -= 1;
        ret.tv_nsec += 1000000000L;
    }

    return ret;
}

static inline struct timespec64 osTimeSpecSub(const struct timespec64 t1 , const struct timespec64 t2)
{
    struct timespec64 ret = {0};

    ret.tv_sec = t1.tv_sec - t2.tv_sec;
    ret.tv_nsec = t1.tv_nsec - t2.tv_nsec;
    if (ret.tv_nsec < 0)
    {
        ret.tv_sec -= 1;
        ret.tv_nsec += 1000000000L;
    }

    return ret;
}

static int os_settimeofday(const struct timeval64* tv, const struct timezone* tz)
{
    unsigned long long now_ns = 0;
    unsigned int uwIntSave;
    struct timespec64 stSetTime = {0};
    struct timespec64 stHwTime = {0};
    struct timespec64 stRealTime = {0};
    struct timespec64 stTemp = {0};

    now_ns = hi_sched_clock();
    stSetTime.tv_sec = tv->tv_sec;
    stSetTime.tv_nsec = tv->tv_usec * 1000L;
    stHwTime.tv_sec = now_ns / 1000000000L;
    stHwTime.tv_nsec = now_ns -  stHwTime.tv_sec * 1000000000LL;

    uwIntSave = LOS_IntLock();
    /* stop on-going continuous adjusement */
    if (adjTimeLeft) {
        adjTimeLeft = 0;
    }
    stRealTime = osTimeSpecAdd(stHwTime, accDeltaFromAdj);
    stRealTime = osTimeSpecAdd(stRealTime, accDeltaFromSet);

    stTemp = osTimeSpecSub(stSetTime, stRealTime);
    accDeltaFromSet = osTimeSpecAdd(accDeltaFromSet, stTemp);

    (void)LOS_IntRestore(uwIntSave);

    return 0;
}

int settimeofday(const struct timeval* tv, const struct timezone* tz)
{
    struct timeval64 stTimeVal64 = {0};

    if (!valid_timeval(tv)) {
        TIME_RETURN(EINVAL);
    }

    stTimeVal64.tv_sec = tv->tv_sec;
    stTimeVal64.tv_usec = tv->tv_usec;

    return os_settimeofday(&stTimeVal64, tz);
}

#ifndef LOSCFG_AARCH64
int settimeofday64(const struct timeval64* tv, const struct timezone* tz)
{
    if (!valid_timeval64(tv)) {
        TIME_RETURN(EINVAL);
    }

    return os_settimeofday(tv, tz);
}
#endif

int setlocalseconds(int seconds)
{
    struct timeval tv = {0};

    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    return settimeofday(&tv, NULL);
}

static int os_gettimeofday(struct timeval64* tv, struct timezone* tz)
{
    unsigned long long now_ns = 0;
    unsigned int uwIntSave;

    struct timespec64 stHwTime = {0};
    struct timespec64 stRealTime = {0};

    now_ns = hi_sched_clock();
    stHwTime.tv_sec = now_ns / 1000000000L;
    stHwTime.tv_nsec = now_ns - stHwTime.tv_sec * 1000000000LL;
    uwIntSave = LOS_IntLock();

    stRealTime = osTimeSpecAdd(stHwTime, accDeltaFromAdj);
    stRealTime = osTimeSpecAdd(stRealTime, accDeltaFromSet);

    tv->tv_sec = stRealTime.tv_sec;
    tv->tv_usec = stRealTime.tv_nsec /1000L;
    (void)LOS_IntRestore(uwIntSave);

    if (tv->tv_sec < 0)
        TIME_RETURN(EINVAL);

    return 0;
}

#ifndef LOSCFG_AARCH64
int gettimeofday64(struct timeval64* tv, struct timezone* tz)
{
    if (tv == NULL)
        TIME_RETURN(EINVAL);

    return os_gettimeofday(tv, tz);
}
#endif

int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    struct timeval64 stTimeVal64 = {0};

    if (tv == NULL)
        TIME_RETURN(EINVAL);

    if (-1 == os_gettimeofday(&stTimeVal64, tz))
    {
        return -1;
    }

#ifdef LOSCFG_AARCH64
    tv->tv_sec = stTimeVal64.tv_sec;
    tv->tv_usec = stTimeVal64.tv_usec;
#else
    if (stTimeVal64.tv_sec > (long long)LONG_MAX)
    {
        return -1;
    }
    tv->tv_sec = (__kernel_time_t)stTimeVal64.tv_sec;
    tv->tv_usec = (__kernel_suseconds_t)stTimeVal64.tv_usec;
#endif

    return 0;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    struct timeval tv = {0};

    if (clock_id != CLOCK_REALTIME) {
        TIME_RETURN(EINVAL);
    }

    if (!valid_timespec(tp)) {
        TIME_RETURN(EINVAL);
    }

    tv.tv_sec = tp->tv_sec;
    tv.tv_usec = tp->tv_nsec / 1000;
    return settimeofday(&tv, NULL);
}

//-----------------------------------------------------------------------------
// Get the clocks current time
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    unsigned int intr_Save;
    unsigned long long now_ns = 0;
    struct timespec64 stTemp = {0};
    struct timespec64 stHwTime = {0};

    if (clock_id > MAX_CLOCKS || clock_id < CLOCK_REALTIME){
        goto errout;
    }

    if (tp == NULL) {
        goto errout;
    }

    now_ns = hi_sched_clock();
    stHwTime.tv_sec = now_ns / 1000000000L;
    stHwTime.tv_nsec = now_ns - stHwTime.tv_sec * 1000000000LL;

    switch (clock_id) {
        case CLOCK_MONOTONIC_RAW:
            tp->tv_sec = stHwTime.tv_sec ;
            tp->tv_nsec = stHwTime.tv_nsec;
            break;
        case CLOCK_MONOTONIC:
            intr_Save = LOS_IntLock();
            stTemp = osTimeSpecAdd(stHwTime, accDeltaFromAdj);
            tp->tv_sec = stTemp.tv_sec;
            tp->tv_nsec = stTemp.tv_nsec;
            (void)LOS_IntRestore(intr_Save);
            break;
        case CLOCK_REALTIME:
            intr_Save = LOS_IntLock();
            stTemp = osTimeSpecAdd(stHwTime, accDeltaFromAdj);
            stTemp = osTimeSpecAdd(stTemp, accDeltaFromSet);
            tp->tv_sec = stTemp.tv_sec;
            tp->tv_nsec = stTemp.tv_nsec;
            (void)LOS_IntRestore(intr_Save);
            break;
        default:
           goto errout;
    }

    return 0;

errout:
    TIME_RETURN(EINVAL);
}

//-----------------------------------------------------------------------------
// Get the clocks resolution
int clock_getres(clockid_t clock_id, struct timespec *tp)
{
    if (clock_id != CLOCK_REALTIME) {
        TIME_RETURN(EINVAL);
    }

    if (tp == NULL) {
        TIME_RETURN(EINVAL);
    }

    /* the accessable rtc resolution */
    tp->tv_nsec = 1000; /* the precision of clock_gettime is 1us */
    tp->tv_sec = 0;

    TIME_RETURN(0);
}

//==========================================================================
// Timer functions

//-----------------------------------------------------------------------------
// Create a timer based on the given clock.
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timer_id)
{
    SWTMR_CTRL_S *pstSwtmr = (SWTMR_CTRL_S *)NULL;
    UINT32 uwRet;
    UINT16 usSwTmrID;

    if ((clock_id != CLOCK_REALTIME) || (timer_id == NULL || evp == NULL)) {
        errno = EINVAL;
        return -1;
    }

    if (evp->sigev_notify != SIGEV_THREAD) {
        errno = EAGAIN;
        return -1;
    }

    uwRet = LOS_SwtmrCreate(1, LOS_SWTMR_MODE_PERIOD,
            (SWTMR_PROC_FUNC)evp->sigev_notify_function,
            &usSwTmrID, (AARCHPTR)evp->sigev_value.sival_int);

    if (uwRet != LOS_OK) {
        errno = EAGAIN;
        return -1;
    }

    pstSwtmr = OS_SWT_FROM_SID(usSwTmrID);

    *timer_id = pstSwtmr;

    return 0;
}

//-----------------------------------------------------------------------------
// Delete the timer
int timer_delete(timer_t timerid)
{
    SWTMR_CTRL_S *pstSwtmr;

    if (OS_INT_ACTIVE || timerid == NULL) {
        goto errout;
    }

    pstSwtmr = (SWTMR_CTRL_S *)timerid;

    if (LOS_SwtmrDelete(pstSwtmr->usTimerID)) {
         goto errout;
    }

    return 0;

errout:
    errno = EINVAL;
    return -1;
}

//-----------------------------------------------------------------------------
// Set the expiration time of the timer.
int timer_settime(timer_t timerid, int flags,
        const struct itimerspec *value, /* new value */
        struct itimerspec *ovalue) /* old value to return, always 0 */
{
    SWTMR_CTRL_S *pstSwtmr;
    UINT32 uwInterval, uwExpiry, millisec, uwRet;
    UINT64 tmp;

    if (value == NULL || OS_INT_ACTIVE || timerid == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (value->it_value.tv_nsec >= 1000000000 ||
            value->it_interval.tv_nsec >= 1000000000 ||
            value->it_value.tv_nsec < 0 ||
            value->it_value.tv_sec < 0 ||
            value->it_interval.tv_nsec < 0 ||
            value->it_interval.tv_sec < 0) {
        errno = EINVAL;
        return -1;
    }

    pstSwtmr = (SWTMR_CTRL_S *)timerid;
    if (OS_SWTMR_STATUS_TICKING == pstSwtmr->ucState) {
        uwRet = LOS_SwtmrStop(pstSwtmr->usTimerID);
        if (LOS_OK != uwRet) {
            errno = EINVAL;
            return -1;
        }
    }

    tmp = value->it_value.tv_nsec / 1000000;
    if ((value->it_value.tv_nsec < (1000000 * (1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND)))
            && (value->it_value.tv_nsec) != 0) {
        tmp = 1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    }

    millisec = (UINT32)tmp;
    millisec += value->it_value.tv_sec * 1000;
    uwExpiry = LOS_MS2Tick(millisec);

    tmp = value->it_interval.tv_nsec / 1000000;
    if ((value->it_interval.tv_nsec < (1000000 * (1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND))) &&
            (value->it_interval.tv_nsec) != 0) {
        tmp = 1000 / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    }
    millisec = (UINT32)tmp;
    millisec += value->it_interval.tv_sec * 1000;
    uwInterval = LOS_MS2Tick(millisec);

    if (uwExpiry == 0) {
        /*
        * 1) when uwExpiry is 0, means timer should be stopped.
        * 2) If timer is ticking, stopping timer is already done before.
        * 2) If timer is created but not ticking, return 0 as well.
        */
        return 0;
    } else {
        if (uwInterval == 0) {
            pstSwtmr->ucMode = LOS_SWTMR_MODE_ONCE;
        } else {
            pstSwtmr->ucMode = LOS_SWTMR_MODE_OPP;
        }
    }

    pstSwtmr->uwExpiry = uwExpiry;
    pstSwtmr->uwInterval = uwInterval;
    pstSwtmr->ucOverrun = 0;
    if (LOS_SwtmrStart(pstSwtmr->usTimerID)) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Get current timer values
int timer_gettime(timer_t timerid, struct itimerspec *value)
{
    UINT32 uwTick = 0;
    UINT32 uwMillisec = 0;
    UINT32 temp = 0;
    SWTMR_CTRL_S *pstSwtmr = (SWTMR_CTRL_S *)NULL;

    pstSwtmr = (SWTMR_CTRL_S *)timerid;

    /* expire time */
    if (value == NULL || pstSwtmr == NULL ||
        (LOS_OK != LOS_SwtmrTimeGet(pstSwtmr->usTimerID, &uwTick))) {
        errno = EINVAL;
        return -1;
    }

    uwMillisec = LOS_Tick2MS(uwTick);
    temp = uwMillisec / 1000;
    value->it_value.tv_sec = temp;
    value->it_value.tv_nsec = (uwMillisec - temp * 1000) * 1000000;

    /* interval time */
    uwTick = pstSwtmr->uwInterval;
    uwMillisec = LOS_Tick2MS(uwTick);
    temp = uwMillisec / 1000;
    value->it_interval.tv_sec = temp;
    value->it_interval.tv_nsec = (uwMillisec - temp * 1000) * 1000000;

    return 0;
}

//-----------------------------------------------------------------------------
// Get number of missed triggers
int timer_getoverrun(timer_t timerid)
{
    SWTMR_CTRL_S *pstSwtmr = (SWTMR_CTRL_S *)NULL;

    pstSwtmr = (SWTMR_CTRL_S *)timerid;
    if (pstSwtmr == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (pstSwtmr->usTimerID >= OS_SWTMR_MAX_TIMERID ) {
        errno = EINVAL;
        return -1;
    }

    if ((pstSwtmr->ucOverrun) >= (unsigned char)DELAYTIMER_MAX) {
        return (int)DELAYTIMER_MAX;
    } else {
        return (int)(pstSwtmr->ucOverrun);
    }
}

static int __sleep(unsigned int useconds)
{
    UINT32 uwInterval = 0;
    UINT32 uwRet = 0;

    if (useconds == 0) {
        uwInterval = 0;
    } else {
        uwInterval = LOS_MS2Tick(useconds);
        if (uwInterval == 0) {
             uwInterval = 1;
        }
    }

    uwRet = LOS_TaskDelay(uwInterval);

    if (uwRet == LOS_OK) {
        return uwRet;
    } else {
        return -1;
    }
}

int usleep(unsigned useconds)
{
    unsigned int uwInterval;

    /* the values not less than per Millisecond */
    if (useconds < OS_SYS_MS_PER_SECOND) {
        if(useconds != 0) {
            uwInterval = OS_SYS_MS_PER_SECOND/LOSCFG_BASE_CORE_TICK_PER_SECOND;
        } else {
            uwInterval = 0;
        }
        return __sleep(uwInterval);
    }

    return __sleep(useconds / 1000);
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    unsigned int useconds;
    int uwRet = -1;
    int uwMax_tv_sec = 0;

    /* expire time */
    if (rqtp == NULL)
    {
        errno = EINVAL;
        return uwRet;
    }
    uwMax_tv_sec = (int) ((UINT32_MAX - INT32_MAX / 1000) /1000000);
    if (rqtp->tv_nsec >= 1000000000 || rqtp->tv_nsec < 0 || rqtp->tv_sec < 0 || rqtp->tv_sec > uwMax_tv_sec)
    {
        errno = EINVAL;
        return uwRet;
    }
    useconds = (rqtp->tv_sec * 1000000 + rqtp->tv_nsec / 1000);
    if (useconds == 0 && rqtp->tv_nsec != 0)
    {
        useconds = 1;
    }
    uwRet = usleep(useconds);

    return uwRet;
}

unsigned int sleep(unsigned int seconds)
{
    UINT32 uwInterval = 0;
    UINT32 uwRet = 0;

    uwInterval = LOS_MS2Tick(seconds * 1000);
    uwRet = LOS_TaskDelay(uwInterval);

    if (uwRet == LOS_OK) {
        return uwRet;
    } else {
        return seconds;
    }
}

double difftime(time_t time2, time_t time1)
{
    return (double)(time2 - time1);
}

clock_t clock(VOID)
{
    clock_t clock_ms = 0;
    unsigned long long now_ns = 0;

    now_ns = hi_sched_clock();
    clock_ms = (clock_t)(now_ns / 1000000L);

    return clock_ms;
}

clock_t times(struct tms *buf)
{
    clock_t clock_tick;

    clock_tick = LOS_TickCountGet();
    if (buf != NULL) {
        buf->tms_cstime = clock_tick;
        buf->tms_cutime = clock_tick;
        buf->tms_stime  = clock_tick;
        buf->tms_utime  = clock_tick;
    }

    return clock_tick;
}
