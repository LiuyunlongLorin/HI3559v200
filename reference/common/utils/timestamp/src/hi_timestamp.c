/**
 * @file    hi_timestamp.c
 * @brief   timestamp log functions.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 * @version   1.0

 */
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef __HuaweiLite__
#define printf dprintf

#define TIME_STAMP_CNT   (TIME_STAMP_BASE_ADDR+0x1000)
#define TIME_STAMP_TAG   (30000)
#else
#define TIME_STAMP_CNT   (TIME_STAMP_BASE_ADDR)
#define TIME_STAMP_TAG   (40000)
#endif
#define TIME_STAMP_VALUE    (TIME_STAMP_CNT+0x10)

#define TIME_STAMP_MAX_CNT  (200)
#define TIME_STAMP_LEN      (16)

#ifndef REG_TIMER_VALUE
#define REG_TIMER_VALUE (0x4)
#endif
#define DIVIDE (1)
#define CYCLE (3)

#define TIMER_REG_BASE TIMERX_REG_BASE

static pthread_mutex_t s_TimeStampMutex = PTHREAD_MUTEX_INITIALIZER;

static HI_VOID TimeStampClean(HI_VOID)
{
#if defined (TIMERX_REG_BASE) && defined (TIME_STAMP_BASE_ADDR)
    himm((HI_U32)TIME_STAMP_CNT, 0);
    #ifdef __HuaweiLite__
    memset((HI_VOID*)TIME_STAMP_VALUE, 0, TIME_STAMP_MAX_CNT * TIME_STAMP_LEN);
    #endif
#endif
}

HI_VOID HI_TimeStamp(const HI_CHAR* pszFunc, HI_U32 u32Line, HI_S32 s32Type)
{
#if defined (TIMERX_REG_BASE) && defined (TIME_STAMP_BASE_ADDR)
    static HI_BOOL bFirst = HI_TRUE;
    HI_MUTEX_LOCK(s_TimeStampMutex);
    if (bFirst)
    {
        TimeStampClean();
        bFirst = HI_FALSE;
    }

    HI_U32 u32Cnt = 0;
    HI_U32 stamp = 0;
    HI_U32* addr = NULL;
    himd(TIME_STAMP_CNT, &u32Cnt);

    if (u32Cnt > TIME_STAMP_MAX_CNT)
    {
        TimeStampClean();
        u32Cnt = 0;
        MLOGW("\n time_stamp is full!!! now clean the buffer to start from 0\n");
    }

    himd((TIMER_REG_BASE + REG_TIMER_VALUE), &stamp);
    addr = (HI_U32*)(HI_UL)(TIME_STAMP_VALUE + u32Cnt * TIME_STAMP_LEN);
    stamp = ~stamp;
    himm((HI_UL)addr, stamp);
    addr++;
    himm((HI_UL)addr, (HI_UL)pszFunc);
    addr++;
    himm((HI_UL)addr, u32Line);
    addr++;

    if (s32Type == 0)
    {
        himm((HI_UL)addr, (HI_U32)TIME_STAMP_TAG);
    }
    else
    {
        himm((HI_UL)addr, (HI_U32)s32Type);
    }

    u32Cnt++;
    himm(TIME_STAMP_CNT, u32Cnt);
    HI_MUTEX_UNLOCK(s_TimeStampMutex);
#endif
}

HI_VOID HI_TimeStampClean(HI_VOID)
{
    HI_MUTEX_LOCK(s_TimeStampMutex);
    TimeStampClean();
    HI_MUTEX_UNLOCK(s_TimeStampMutex);
}

HI_VOID HI_TimeStampPrint(HI_VOID)
{
#if defined (TIMERX_REG_BASE) && defined (TIME_STAMP_BASE_ADDR)
    HI_U32 u32Time = 0;
    HI_MUTEX_LOCK(s_TimeStampMutex);
    himd((TIMER_REG_BASE + REG_TIMER_VALUE), &u32Time);

    if (u32Time == 0xffffffff)
    {
        MLOGE("timer didn't init!");
        return;
    }

    HI_U32 u32Cnt = 0;
    himd(TIME_STAMP_CNT, &u32Cnt);
    HI_U32 Time_0 = 0;
    HI_U32* addr, i, stamp;
    stamp =  0;

    for (i = 0; i < u32Cnt; i++)
    {
        addr = (HI_U32*)(HI_UL)(TIME_STAMP_VALUE + i * TIME_STAMP_LEN);
        HI_U32 nTag = 0;
        HI_U32 nTime = 0;
#ifdef __HuaweiLite__
        HI_U32 pcFunc = 0;
#else
        HI_CHAR* pcFunc = 0;
#endif
        HI_U32 nLine = 0;
        himd((HI_UL)(addr + 3), &nTag);
        himd((HI_UL)(addr + 0), &nTime);
        nTime = nTime * DIVIDE / CYCLE;
        himd((HI_UL)(addr + 2), &nLine);
        himd((HI_UL)(addr + 1), (HI_U32*)&pcFunc);

        if (nTag == TIME_STAMP_TAG)
        {
            printf("time stamp[%-3d] = %-8dus gap = %-8dus type: %-5d  line: %-5d func: %s\n", i + 1, nTime, nTime - stamp, nTag, nLine, pcFunc);
        }
        else
        {
            printf("time stamp[%-3d] = %-8dus gap = %-8dus type: %-5d  line: %-5d\n", i + 1, nTime, nTime - stamp, nTag, nLine);
        }

        stamp = nTime;

        if (i == 0)
        {
            Time_0 = nTime;
        }

        if (i == (u32Cnt - 1))
        {
            printf("total time from stamp[%-3d] to [%-3d]  gap = %-8dus\n", 1, i + 1, nTime - Time_0);
        }
    }
    HI_MUTEX_UNLOCK(s_TimeStampMutex);
#else
    MLOGW("TIMERX_REG_BASE or TIME_STAMP_BASE_ADDR is undefined.\n");
#endif
}

HI_VOID HI_PrintBootTime(const HI_CHAR* pcKeyword)
{
#ifdef CFG_TIME_MEASURE_ON
#if defined (TIMERX_REG_BASE) && defined (TIME_STAMP_BASE_ADDR)
    HI_U32 u32Boottime = 0;
    himd((TIMER_REG_BASE + REG_TIMER_VALUE), &u32Boottime);
    u32Boottime = ~u32Boottime;
    u32Boottime = u32Boottime * DIVIDE / CYCLE;
    if (pcKeyword)
    {
        MLOGI(YELLOW"%s: %u ms"NONE"\n", pcKeyword, u32Boottime / 1000);
    }
    else
    {
        MLOGI(YELLOW"%u ms"NONE"\n", u32Boottime / 1000);
    }
#else
    MLOGW("TIMERX_REG_BASE or TIME_STAMP_BASE_ADDR is undefined.\n");
#endif
#endif
}

HI_S32 HI_usleep(HI_U32 usec)
{
    HI_S32 s32Ret;
    struct timespec requst;
    struct timespec remain;
    remain.tv_sec = usec / 1000000;
    remain.tv_nsec = (usec % 1000000) * 1000;
    do
    {
        requst = remain;
        s32Ret = nanosleep(&requst, &remain);
    }while(-1 == s32Ret && errno == EINTR);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
