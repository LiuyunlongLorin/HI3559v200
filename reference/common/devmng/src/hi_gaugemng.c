/**
* @file    hi_gaugemng.c
* @brief   product gaugemng function
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/15
* @version

*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_gaugemng.h"
#include "hi_hal_gauge.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** macro define */
#define GAUGEMNG_CHECK_INTERVAL  (1) /**< gauge check interval, unit s */
#define GAUGEMNG_LOWLEVEL_RESET  (20) /**< gauge level interval, percent */


static HI_BOOL s_bGAUGEMNGInitState = HI_FALSE;
static pthread_mutex_t s_GAUGEMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_GAUGEMNGCheckId;
static HI_BOOL s_bGAUGEMNGCheckRun;

/** gaugemng information */
typedef struct tagGAUGEMNG_INFO_S
{
    HI_S32 s32Level;
    HI_S32 s32LastLevel;
    HI_BOOL bLowLevelState;
    HI_BOOL bUltraLowLevelState;
    HI_BOOL bCharge;
    HI_BOOL bLastCharge;
    HI_GAUGEMNG_CFG_S stCfg;
} GAUGEMNG_INFO_S;


static GAUGEMNG_INFO_S s_stGAUGEMNGInfo;


static HI_S32 GAUGEMNG_InParmValidChck(const HI_GAUGEMNG_CFG_S* stCfg)
{
    /** parm check */
    HI_APPCOMM_CHECK_POINTER(stCfg, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(stCfg->s32LowLevel < 100, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(stCfg->s32UltraLowLevel >= 0, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(stCfg->s32UltraLowLevel < stCfg->s32LowLevel, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 GAUGEMNG_InternalParmInit(const HI_GAUGEMNG_CFG_S* pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);
    s_stGAUGEMNGInfo.s32Level = 0;
    s_stGAUGEMNGInfo.s32LastLevel = 0;
    s_stGAUGEMNGInfo.bLowLevelState = HI_FALSE;
    s_stGAUGEMNGInfo.bUltraLowLevelState = HI_FALSE;

    s_stGAUGEMNGInfo.stCfg.s32LowLevel = pstCfg->s32LowLevel;
    s_stGAUGEMNGInfo.stCfg.s32UltraLowLevel = pstCfg->s32UltraLowLevel;
    return HI_SUCCESS;
}

static HI_VOID GAUGEMNG_UltraLowLevelCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_EVENT_S stEvent;
    if (HI_FALSE == pstGaugemngInfo->bUltraLowLevelState)
    {
        if (pstGaugemngInfo->s32Level <= pstGaugemngInfo->stCfg.s32UltraLowLevel)
        {
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW;
            stEvent.arg1 = pstGaugemngInfo->s32Level;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("battery level ultralow event\n");
            pstGaugemngInfo->bUltraLowLevelState = HI_TRUE;
        }
    }
    return;
}


static HI_VOID GAUGEMNG_LowLevelCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_EVENT_S stEvent;

    if (HI_FALSE == pstGaugemngInfo->bLowLevelState)
    {
        if (pstGaugemngInfo->s32Level <= pstGaugemngInfo->stCfg.s32LowLevel)
        {
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_GAUGEMNG_LEVEL_LOW;
            stEvent.arg1 = pstGaugemngInfo->s32Level;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("battery level low event\n");
            pstGaugemngInfo->bLowLevelState = HI_TRUE;
        }
    }
    else
    {
        if (pstGaugemngInfo->s32Level >= (pstGaugemngInfo->stCfg.s32LowLevel + GAUGEMNG_LOWLEVEL_RESET))
        {
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_GAUGEMNG_LEVEL_NORMAL;
            stEvent.arg1 = pstGaugemngInfo->s32Level;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("battery level restore normal\n");

            pstGaugemngInfo->bLowLevelState = HI_FALSE;
            pstGaugemngInfo->bUltraLowLevelState = HI_FALSE;
        }
    }
    return;
}

static HI_VOID GAUGEMNG_LevelChangeCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_EVENT_S stEvent;

    if (pstGaugemngInfo->s32Level != pstGaugemngInfo->s32LastLevel)
    {
        memset(&stEvent, '\0', sizeof(stEvent));
        stEvent.EventID = HI_EVENT_GAUGEMNG_LEVEL_CHANGE;
        stEvent.arg1 = pstGaugemngInfo->s32Level;
        HI_EVTHUB_Publish(&stEvent);
        MLOGD("battery level changge event\n");

    }
    return;
}

static HI_S32  GAUGEMNG_LevelCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_S32 s32Level = 0;
    HI_APPCOMM_CHECK_EXPR(HI_HAL_GAUGE_GetLevel(&s32Level) == HI_SUCCESS, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(s32Level <= 100 && s32Level >= 0, HI_FAILURE);

    if (pstGaugemngInfo->s32Level != s32Level)
    {
        pstGaugemngInfo->s32Level = s32Level;
        GAUGEMNG_LevelChangeCheck(pstGaugemngInfo);
        GAUGEMNG_LowLevelCheck(pstGaugemngInfo);
        GAUGEMNG_UltraLowLevelCheck(pstGaugemngInfo);
        pstGaugemngInfo->s32LastLevel = s32Level;
    }
    return HI_SUCCESS;
}

static HI_VOID GAUGEMNG_ChargeStateChangeCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_EVENT_S stEvent;

    if (pstGaugemngInfo->bCharge != pstGaugemngInfo->bLastCharge)
    {
        memset(&stEvent, '\0', sizeof(stEvent));
        stEvent.EventID = HI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE;
        stEvent.arg1 = pstGaugemngInfo->bCharge;
        HI_EVTHUB_Publish(&stEvent);
        MLOGD("battery chargestate changge event\n");
    }
    return;
}

static HI_S32  GAUGEMNG_ChargeStateCheck(GAUGEMNG_INFO_S* pstGaugemngInfo)
{
    HI_BOOL bCharge = HI_FALSE;
    HI_APPCOMM_CHECK_EXPR(HI_HAL_GAUGE_GetChargeState(&bCharge) == HI_SUCCESS, HI_FAILURE);

    if (pstGaugemngInfo->bCharge!= bCharge)
    {
        pstGaugemngInfo->bCharge = bCharge;
        GAUGEMNG_ChargeStateChangeCheck(pstGaugemngInfo);
        pstGaugemngInfo->bLastCharge = bCharge;
    }
    return HI_SUCCESS;
}

static HI_VOID*  GAUGEMNG_LevelCheckThread(HI_VOID* pData)
{
    /** Set Task Name */
    prctl(PR_SET_NAME, "GAUGE_LEVEL_CHECK", 0, 0, 0);

    while (s_bGAUGEMNGCheckRun)
    {
        GAUGEMNG_LevelCheck(&s_stGAUGEMNGInfo);
        GAUGEMNG_ChargeStateCheck(&s_stGAUGEMNGInfo);
        HI_usleep(GAUGEMNG_CHECK_INTERVAL * 1000 * 1000);
    }

    MLOGD("gauge check thread exit\n");
    return NULL;
}

HI_S32 HI_GAUGEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GAUGEMNG_LEVEL_CHANGE);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register battery level change event fail\n");
        return HI_GAUGEMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GAUGEMNG_LEVEL_LOW);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register battery level low event fail\n");
        return HI_GAUGEMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register battery level ultra low event fail\n");
        return HI_GAUGEMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GAUGEMNG_LEVEL_NORMAL);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register battery level recover normal event fail\n");
        return HI_GAUGEMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register battery level recover normal event fail\n");
        return HI_GAUGEMNG_EREGISTEREVENT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_GAUGEMNG_Init(const HI_GAUGEMNG_CFG_S* pstCfg)
{
    HI_S32 s32Ret;

    if (HI_FAILURE == GAUGEMNG_InParmValidChck(pstCfg))
    {
        MLOGE("parm check error\n");
        return HI_GAUGEMNG_EINVAL;
    }

    HI_MUTEX_LOCK(s_GAUGEMNGMutex);

    if (HI_TRUE  == s_bGAUGEMNGInitState)
    {
        MLOGE("keymng has already been started\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_EINITIALIZED;
    }

    s32Ret = HI_HAL_GAUGE_Init();

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_GAUGE_Init Failed\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_EINTER;
    }

    GAUGEMNG_InternalParmInit(pstCfg);

    /* Create gauge Check Task */
    s_bGAUGEMNGCheckRun = HI_TRUE;
    s32Ret = pthread_create(&s_GAUGEMNGCheckId, NULL, GAUGEMNG_LevelCheckThread, NULL);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Create GaugeCheck Thread Fail!\n");
        HI_HAL_GAUGE_Deinit();
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_ETHREAD;
    }

    s_bGAUGEMNGInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
    return HI_SUCCESS;

}

HI_S32 HI_GAUGEMNG_GetBatteryLevel(HI_S32* ps32Level)
{
    HI_S32 s32Ret =HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(ps32Level, HI_GAUGEMNG_EINVAL);
    HI_MUTEX_LOCK(s_GAUGEMNGMutex);
    if (HI_FALSE  == s_bGAUGEMNGInitState)
    {
        MLOGW("gaugemng no init\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_ENOINIT;
    }
    s32Ret = HI_HAL_GAUGE_GetLevel(ps32Level);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL GAUGE GetLevel fail\n");
        s32Ret = HI_GAUGEMNG_EINTER;
    }
    HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
    return s32Ret;
}

HI_S32 HI_GAUGEMNG_GetChargeState(HI_BOOL* pbCharge)
{
    HI_S32 s32Ret =HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pbCharge, HI_GAUGEMNG_EINVAL);
    HI_MUTEX_LOCK(s_GAUGEMNGMutex);
    if (HI_FALSE  == s_bGAUGEMNGInitState)
    {
        MLOGW("gaugemng no init\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_ENOINIT;
    }
    s32Ret = HI_HAL_GAUGE_GetChargeState(pbCharge);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_GAUGE_GetChargeState fail\n");
        s32Ret = HI_GAUGEMNG_EINTER;
    }
    HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
    return s32Ret;
}

HI_S32 HI_GAUGEMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MUTEX_LOCK(s_GAUGEMNGMutex);
    /** Destory gauge Check Task */
    if (HI_FALSE  == s_bGAUGEMNGInitState)
    {
        MLOGW("gaugemng no init\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_ENOINIT;
    }
    s_bGAUGEMNGCheckRun = HI_FALSE;
    s32Ret = pthread_join(s_GAUGEMNGCheckId, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Join GaugeCheck Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_ETHREAD;
    }

    /** Close hal gauge */
    s32Ret = HI_HAL_GAUGE_Deinit();

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_GAUGE_Deinit Fail!\n");
        HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
        return HI_GAUGEMNG_EINTER;
    }
    s_bGAUGEMNGInitState = HI_FALSE;

    HI_MUTEX_UNLOCK(s_GAUGEMNGMutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
