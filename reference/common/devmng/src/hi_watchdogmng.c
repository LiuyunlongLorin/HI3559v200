/**
 * @file      hi_watchdogmng.c
 * @brief     product parameter module interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 * @version   1.0

 */
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_hal_watchdog.h"
#include "hi_watchdogmng.h"
#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct tagWATCHDOGMNG_CONTEXT_S
{
    HI_BOOL bWDTMNGInitState;
    HI_BOOL bWdtCheckRun;
    pthread_t WdtCheckId;
    HI_S32 s32Time;
} WATCHDOGMNG_CONTEXT_S;
static WATCHDOGMNG_CONTEXT_S s_stWdtCtx;


static HI_VOID*  WATCHDOG_CheckThread(HI_VOID* pData)
{
    MLOGD("thread WATCHDOG Check enter\n");
    HI_CHAR szThreadName[HI_APPCOMM_MAX_PATH_LEN];
    snprintf(szThreadName, HI_APPCOMM_MAX_PATH_LEN, "%s", "WDT_CHECK");
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0); /**< Set Task Name */

    while (s_stWdtCtx.bWdtCheckRun)
    {
        HI_HAL_WATCHDOG_Feed();
        HI_usleep((s_stWdtCtx.s32Time - 1) * 1000 * 1000);
    }

    MLOGD("thread WATCHDOG_CheckThread exit\n");
    return NULL;
}


HI_S32 HI_WATCHDOGMNG_Init(HI_S32 s32Time_s)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (s_stWdtCtx.bWDTMNGInitState == HI_TRUE)
    {
        MLOGE("wdtmng has already been started\n");
        return HI_FAILURE;
    }

    if (s32Time_s < 2 || s32Time_s > 1000)
    {
        MLOGE("Interval time should not be less then two and bigger then 100. %d\n", s32Time_s);
        return HI_FAILURE;
    }

    s32Ret = HI_HAL_WATCHDOG_Init(s32Time_s);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_WATCHDOG_Init: failed, errno(%d)\n", s32Ret);
        return HI_FAILURE;
    }

    s_stWdtCtx.bWdtCheckRun = HI_TRUE;
    s_stWdtCtx.s32Time = s32Time_s;
    s32Ret = pthread_create(&(s_stWdtCtx.WdtCheckId), NULL, WATCHDOG_CheckThread, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Create WATCHDOG_CheckThread Thread Fail!\n");
        return HI_FAILURE;
    }

    s_stWdtCtx.bWDTMNGInitState = HI_TRUE;
    return HI_SUCCESS;
}



HI_S32 HI_WATCHDOGMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_stWdtCtx.bWDTMNGInitState  == HI_FALSE)
    {
        MLOGE("watchdogmng no init\n");
        return HI_FAILURE;
    }

     /** Destory Key Check Task */
    s_stWdtCtx.bWdtCheckRun = HI_FALSE;
    s32Ret = pthread_join(s_stWdtCtx.WdtCheckId, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Join WATCHDOG_CheckThread  Fail!\n");
        return HI_FAILURE;
    }

    HI_HAL_WATCHDOG_Deinit();
    s_stWdtCtx.bWDTMNGInitState = HI_FALSE;
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

