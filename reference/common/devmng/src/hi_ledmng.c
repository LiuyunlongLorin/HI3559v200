/**
* @file    hi_ledmng.c
* @brief   product ledmng function
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/14
* @version

*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_ledmng.h"
#include "hi_hal_led.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#define LEDMNG_INTERVALTIME_MIN_NUM       (100 * 1000)

typedef struct tagLEDMNG_CONTEXT_S
{
    HI_BOOL bLEDMNGInitState;
    pthread_mutex_t LEDMNGMutex;
    HI_BOOL bLEDMNGCheckRun;
    pthread_t LEDMNGCheckId;
    HI_LEDMNG_LED_STATUS_S stLedStatus;
    HI_LEDMNG_LED_IDX_E enLedIdx;
} LEDMNG_CONTEXT_S;
static LEDMNG_CONTEXT_S s_astLedmngCtx[HI_LEDMNG_LED_IDX_BUTT];

HI_S32 HI_LEDMNG_SetStatus(HI_LEDMNG_LED_IDX_E enLedIdx, const HI_LEDMNG_LED_STATUS_S* pstLedStatus)
{
    HI_APPCOMM_CHECK_POINTER(pstLedStatus, HI_FAILURE);

    HI_MUTEX_LOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);

    if (s_astLedmngCtx[enLedIdx].bLEDMNGInitState  == HI_FALSE)
    {
        MLOGE("ledmng no init\n");
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_ENOINIT;
    }

    if ((HI_LEDMNG_LED_STATUS_FLASH == pstLedStatus->enLedStatus) &&
        (pstLedStatus->s32LightOnTime_us < LEDMNG_INTERVALTIME_MIN_NUM ||
         pstLedStatus->s32LightOffTime_us < LEDMNG_INTERVALTIME_MIN_NUM))
    {
        MLOGE("Intervel time is to small, must be lager than %d.\n",
            LEDMNG_INTERVALTIME_MIN_NUM);
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_EINVAL;
    }

    memcpy(&(s_astLedmngCtx[enLedIdx].stLedStatus), pstLedStatus, sizeof(HI_LEDMNG_LED_STATUS_S));

    HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
    return HI_SUCCESS;
}


static HI_S32  LEDMNG_LedCheck(HI_LEDMNG_LED_IDX_E enLedIdx, HI_LEDMNG_LED_STATUS_S* pstLedStatus)
{
    HI_APPCOMM_CHECK_POINTER(pstLedStatus, HI_FAILURE);
    HI_HAL_LED_STATE_E enLedState = HI_HAL_LED_STATE_BUIT;
    switch(pstLedStatus->enLedStatus)
    {
        case HI_LEDMNG_LED_STATUS_ON:
            enLedState = HI_HAL_LED_STATE_ON;
            HI_HAL_LED_SetState(enLedIdx, enLedState);
            HI_usleep(500 * 1000);
            break;
        case HI_LEDMNG_LED_STATUS_OFF:
            enLedState = HI_HAL_LED_STATE_OFF;
            HI_HAL_LED_SetState(enLedIdx, enLedState);
            HI_usleep(500 * 1000);
            break;
        case HI_LEDMNG_LED_STATUS_FLASH:
            HI_HAL_LED_SetState(enLedIdx, HI_HAL_LED_STATE_ON);
            HI_usleep(pstLedStatus->s32LightOnTime_us);
            HI_HAL_LED_SetState(enLedIdx, HI_HAL_LED_STATE_OFF);
            HI_usleep(pstLedStatus->s32LightOffTime_us);
            break;
        default:
            MLOGE("unKnow LED status.\n");
            break;
    }
    return HI_SUCCESS;
}

static HI_VOID*  LEDMNG_CheckThread(HI_VOID* pData)
{

    MLOGD("thread LED Check enter\n");
    HI_LEDMNG_LED_IDX_E *penIndex = (HI_LEDMNG_LED_IDX_E*)pData;
    HI_CHAR szThreadName[64];
    snprintf(szThreadName, 64, "%s%d", "LED_CHECK_", *penIndex);
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0); /**< Set Task Name */

    while (s_astLedmngCtx[*penIndex].bLEDMNGCheckRun)
    {
        LEDMNG_LedCheck(*penIndex, &(s_astLedmngCtx[*penIndex].stLedStatus));
    }

    MLOGD("thread LED_CHECK exit\n");
    return NULL;
}

HI_S32 HI_LEDMNG_Init(HI_LEDMNG_LED_IDX_E enLedIdx, const HI_LEDMNG_CFG_S* pstCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);

    if (s_astLedmngCtx[enLedIdx].bLEDMNGInitState == HI_TRUE)
    {
        MLOGE("ledmng has already been started\n");
        return HI_LEDMNG_EINITIALIZED;
    }

    if ((HI_LEDMNG_LED_STATUS_FLASH == pstCfg->stLedmngInitialStatus.enLedStatus) &&
        (pstCfg->stLedmngInitialStatus.s32LightOnTime_us< LEDMNG_INTERVALTIME_MIN_NUM ||
         pstCfg->stLedmngInitialStatus.s32LightOffTime_us< LEDMNG_INTERVALTIME_MIN_NUM))
    {
        MLOGE("Intervel time is to small, must be lager than %d.\n", LEDMNG_INTERVALTIME_MIN_NUM);
        return HI_LEDMNG_EINVAL;
    }

    HI_MUTEX_INIT_LOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);

    HI_MUTEX_LOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
    s32Ret = HI_HAL_LED_Init(enLedIdx);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_HAL_LED_Init Failed\n");
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_EINTER;
    }

    memcpy(&(s_astLedmngCtx[enLedIdx].stLedStatus), &(pstCfg->stLedmngInitialStatus), sizeof(HI_LEDMNG_LED_STATUS_S));
    /** Create Led Check Task */
    s_astLedmngCtx[enLedIdx].bLEDMNGCheckRun = HI_TRUE;
    s_astLedmngCtx[enLedIdx].enLedIdx = enLedIdx;
    s32Ret = pthread_create(&(s_astLedmngCtx[enLedIdx].LEDMNGCheckId), NULL, LEDMNG_CheckThread, (HI_VOID*)&(s_astLedmngCtx[enLedIdx].enLedIdx));
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Create LEDCheck Thread Fail!\n");
        HI_HAL_LED_Deinit(enLedIdx);
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_ETHREAD;
    }

    s_astLedmngCtx[enLedIdx].bLEDMNGInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
    return HI_SUCCESS;

}

HI_S32 HI_LEDMNG_Deinit(HI_LEDMNG_LED_IDX_E enLedIdx)
{
    HI_S32 s32Ret;

    HI_MUTEX_LOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);

    if (s_astLedmngCtx[enLedIdx].bLEDMNGInitState  == HI_FALSE)
    {
        MLOGE("ledmng no init\n");
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_ENOINIT;
    }

    /** Destory Key Check Task */
    s_astLedmngCtx[enLedIdx].bLEDMNGCheckRun = HI_FALSE;

    s32Ret = pthread_join(s_astLedmngCtx[enLedIdx].LEDMNGCheckId, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Join LedCheck Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_ETHREAD;
    }

    /** Close Key */
    s32Ret = HI_HAL_LED_Deinit(enLedIdx);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_HAL_KEY_Deinit Fail!\n");
        HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
        return HI_LEDMNG_EINTER;
    }

    s_astLedmngCtx[enLedIdx].bLEDMNGInitState = HI_FALSE;
    HI_MUTEX_UNLOCK(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
    HI_MUTEX_DESTROY(s_astLedmngCtx[enLedIdx].LEDMNGMutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
