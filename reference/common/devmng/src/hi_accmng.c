/**
* @file    hi_accmng.c
* @brief   productaccmng function
*
* Copyright (c) 2019 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2019/04/10
* @version

*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_accmng.h"
#include "hi_hal_acc.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ACCMNG_CHECK_INTERVAL  (1000) /**< voltage supply check interval, unit ms */

static HI_BOOL g_accInitState = HI_FALSE;
static pthread_t g_accPthreadId;
static HI_BOOL g_accCheckRun;
static HI_HAL_ACC_STATE lastAccVolDet;
static HI_HAL_ACC_STATE lastUsbVolSupply;

static HI_S32 ACCMNG_Check()
{
    HI_S32 ret = HI_SUCCESS;
    HI_ACC_VOLTAGE_SUPPLY accCurState;
    HI_EVENT_S event;
    ret = HI_HAL_ACC_GetState(&accCurState);
    if (HI_SUCCESS != ret) {
        MLOGE("get acc state failed!!\n");
        return HI_FAILURE;
    }
    if ((lastAccVolDet == HI_HAL_ACC_STATE_IGNITION && accCurState.accDetect == HI_HAL_ACC_STATE_FLAMEOUT) ||
            (lastUsbVolSupply == HI_HAL_ACC_STATE_IGNITION && accCurState.usbVolSupply == HI_HAL_ACC_STATE_FLAMEOUT)) {
        memset(&event, '\0', sizeof(HI_EVENT_S));
        event.EventID = HI_EVENT_ACCMNG_POWEROFF;
        HI_EVTHUB_Publish(&event);
    }
    lastAccVolDet    = accCurState.accDetect;
    lastUsbVolSupply = accCurState.usbVolSupply;
    return HI_SUCCESS;
}
static HI_VOID  *ACCMNG_CheckThread(HI_VOID *pData)
{
    HI_S32 ret = HI_SUCCESS;
    HI_ACC_VOLTAGE_SUPPLY accCurState;

    MLOGD("thread ACCMNG_CHECK enter\n");
    prctl(PR_SET_NAME, "ACCMNG_CHECK", 0, 0, 0); /**< Set Task Name */
    ret = HI_HAL_ACC_GetState(&accCurState);
    if (HI_SUCCESS != ret) {
        MLOGE("get acc state failed!!\n");
        return NULL;
    }
    lastAccVolDet    = accCurState.accDetect;
    lastUsbVolSupply = accCurState.usbVolSupply;
    while (g_accCheckRun) {
        ret = ACCMNG_Check();
        if (HI_SUCCESS != ret) {
            MLOGE("ACCMNG_Check failed!!\n");
        }
        HI_usleep(ACCMNG_CHECK_INTERVAL * 1000);
    }
    MLOGD("thread ACCMNG_Check exit\n");
    return NULL;
}

HI_S32 HI_ACCMNG_RegisterEvent(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    ret = HI_EVTHUB_Register(HI_EVENT_ACCMNG_POWEROFF);
    if (HI_SUCCESS != ret) {
        MLOGE("Register acc poweroff event fail\n");
        return HI_ACCMNG_EREGISTEREVENT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_ACCMNG_Init(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    MLOGI("accmng init!\n");
    if (g_accInitState  == HI_TRUE) {
        MLOGE("accmng has already been started\n");
        return HI_ACCMNG_EINITIALIZED;
    }
    ret = HI_HAL_ACC_init();
    if (ret != HI_SUCCESS) {
        MLOGE("HAL_ACC_init Failed\n");
        return HI_ACCMNG_EINTER;
    }
    g_accCheckRun = HI_TRUE;
    ret = pthread_create(&g_accPthreadId, NULL, ACCMNG_CheckThread, NULL);
    if (ret != HI_SUCCESS) {
        MLOGE("Create AccCheck Thread Fail!\n");
        g_accCheckRun = HI_FALSE;
        HI_HAL_ACC_deinit();
        return HI_ACCMNG_EREGISTEREVENT;
    }
    g_accInitState = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_ACCMNG_Deinit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    if (g_accInitState  == HI_FALSE) {
        MLOGE("accmng no init\n");
        return HI_ACCMNG_ENOINIT;
    }
    g_accCheckRun = HI_FALSE;
    ret = pthread_join(g_accPthreadId, NULL);
    if (ret != HI_SUCCESS) {
        MLOGE("Join AccCheck Thread Fail!\n");
        return HI_ACCMNG_ETHREAD;
    }
    ret = HI_HAL_ACC_deinit();
    if (ret != HI_SUCCESS) {
        MLOGE("HAL_ACC_deinit Fail!\n");
        return HI_ACCMNG_EINTER;
    }
    g_accInitState = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */
