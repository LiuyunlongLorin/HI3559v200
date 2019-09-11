/**
 * @file    hi_tempcycle_server.c
 * @brief   tempcycle module server interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */

#include "hi_tempcycle.h"

#include <stdio.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "mpi_pm.h"

#if defined(AMP_LINUX_HUAWEILITE)
#include "hi_appcomm_msg.h"
#else
#include "hi_eventhub.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define TEMPCYCLE_CHECK_INTERVAL_MS            (500) /**< temp monitor check interval, unit ms */

/** ahd context */
typedef struct tagTEMPCYCLE_MONITOR_CONTEXT_S
{
    HI_BOOL taskInitState;              /**the task Init state*/
    pthread_mutex_t threadMutex;
    pthread_t threadCheckId;
    HI_BOOL threadRunFlag;              /**the flag of thread run or pause*/
    HI_TEMPCYCLE_MONITORCFG_S tempMonitorCfg;
    HI_BOOL highTempState;              /**the high temperature state*/
    HI_U32 tempLastCnt;                 /**the temperature lasts count*/
    HI_U32 hightempAlarmCnt;            /**the high temperature lasts count*/
} TEMPCYCLE_MONITOR_CONTEXT_S;

static TEMPCYCLE_MONITOR_CONTEXT_S s_monitorContext ={.taskInitState = HI_FALSE,.highTempState = HI_FALSE,
    .tempLastCnt = 0, .hightempAlarmCnt = 0,.threadMutex = PTHREAD_MUTEX_INITIALIZER};

static HI_S32 TEMPCYCLE_NotifyTempChange(HI_BOOL highTempState)
{
    HI_S32 ret = HI_SUCCESS;
#if defined(AMP_LINUX_HUAWEILITE)
    HI_S32 msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_NOTIFY_TEMPERATURE_CHANGE);
    HI_MSG_PRIV_DATA_S privData = {0};

    privData.as32PrivData[0] = TEMPCYCLE_NOTIFY_TEMPERATURE_CHANGE;
    privData.as32PrivData[1] = highTempState;
    ret = HI_MSG_SendSync(msgID, NULL, 0, &privData, NULL, 0);
    if(HI_SUCCESS != ret)
    {
        MLOGE("HI_MSG_SendSync error.%d\n", ret);
    }
#else
    HI_EVENT_S event;
    memset(&event, '\0', sizeof(event));
    event.EventID = HI_EVENT_TEMPCYCLE_TEMP_CHANGE;
    event.arg1 = highTempState;
    ret = HI_EVTHUB_Publish(&event);
    if(HI_SUCCESS != ret)
    {
        MLOGE("HI_EVTHUB_Publish error.%d\n", ret);
    }
#endif
    return ret;
}

static HI_S32 TEMPCYCLE_NotifyHighTempAlarm(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
#if defined(AMP_LINUX_HUAWEILITE)
    HI_S32 msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_NOTIFY_HIGHTEMPERATURE_ALARM);
    HI_MSG_PRIV_DATA_S privData = {0};

    privData.as32PrivData[0] = TEMPCYCLE_NOTIFY_HIGHTEMPERATURE_ALARM;
    ret = HI_MSG_SendSync(msgID, NULL, 0, &privData, NULL, 0);
    if(HI_SUCCESS != ret)
    {
        MLOGE("HI_MSG_SendSync error.%d\n", ret);
    }
#else
    HI_EVENT_S event;
    memset(&event, '\0', sizeof(event));
    event.EventID = HI_EVENT_TEMPCYCLE_HIGHTEMP_ALARM;
    ret = HI_EVTHUB_Publish(&event);
    if(HI_SUCCESS != ret)
    {
        MLOGE("HI_EVTHUB_Publish error.%d\n", ret);
    }
#endif
    return ret;
}

static HI_VOID TEMPCYCLE_TemperatureChangeCheck(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 currentTemp = 0;
    HI_U32 tempLastCnt = (s_monitorContext.tempMonitorCfg.tempLastTime_s * 1000) / TEMPCYCLE_CHECK_INTERVAL_MS;
    HI_U32 tempHighAlarmLastCnt = (s_monitorContext.tempMonitorCfg.tempHighAlarmTime_s * 1000) / TEMPCYCLE_CHECK_INTERVAL_MS;
    if (tempLastCnt <= 0 || tempHighAlarmLastCnt <= 0)
    {
        MLOGE("tempLastCnt error. [%d %d]\n",tempLastCnt, tempHighAlarmLastCnt);
    }
    ret = HI_MPI_PM_GetTemperature(&currentTemp);
    if(HI_SUCCESS != ret)
    {
        MLOGE("HI_MPI_PM_GetTemp error.\n");
    }
    if (!s_monitorContext.highTempState)
    {
        s_monitorContext.hightempAlarmCnt = 0;
        if (currentTemp >= s_monitorContext.tempMonitorCfg.highTemperature)
        {
            s_monitorContext.tempLastCnt++;
        }
        else
        {
            s_monitorContext.tempLastCnt = 0;
        }

        if (s_monitorContext.tempLastCnt ==
            tempLastCnt)
        {
            MLOGD("High Temp Occur.\n");
            s_monitorContext.highTempState = HI_TRUE;
            s_monitorContext.tempLastCnt = 0;
            ret = TEMPCYCLE_NotifyTempChange(s_monitorContext.highTempState);
            if(HI_SUCCESS != ret)
            {
                MLOGE("TEMPCYCLE_NotifyTempChange error.\n");
            }
        }
    }
    else
    {
        if (currentTemp <= s_monitorContext.tempMonitorCfg.normalTemperature)
        {
            s_monitorContext.tempLastCnt++;
            s_monitorContext.hightempAlarmCnt = 0;
        }
        else
        {
            s_monitorContext.tempLastCnt = 0;
            s_monitorContext.hightempAlarmCnt++;
        }

        if (s_monitorContext.tempLastCnt ==
            tempLastCnt)
        {
            MLOGD("Normal Temp Occur.\n");
            s_monitorContext.highTempState = HI_FALSE;
            s_monitorContext.tempLastCnt = 0;
            ret = TEMPCYCLE_NotifyTempChange(s_monitorContext.highTempState);
            if(HI_SUCCESS != ret)
            {
                MLOGE("TEMPCYCLE_NotifyTempChange error.\n");
            }
        }

        if (s_monitorContext.hightempAlarmCnt ==
            tempHighAlarmLastCnt)
        {
            MLOGD("High Temp Alarm Occur.\n");
            s_monitorContext.hightempAlarmCnt = 0;
            ret = TEMPCYCLE_NotifyHighTempAlarm();
            if(HI_SUCCESS != ret)
            {
                MLOGE("TEMPCYCLE_NotifyHighTempAlarm error.\n");
            }
        }
    }
    return;
}
static HI_VOID*  TEMPCYCLE_TempMonitorThread(HI_VOID* pData)
{
    /** Set Task Name */
    prctl(PR_SET_NAME, "TEMPCYCLE_MONITOR", 0, 0, 0);
    while (s_monitorContext.threadRunFlag)
    {
        TEMPCYCLE_TemperatureChangeCheck();
        HI_usleep(TEMPCYCLE_CHECK_INTERVAL_MS * 1000);
    }
    return NULL;
}

HI_S32 HI_TEMPCYCLE_SetPMParam(const HI_MPI_PM_SIGLE_MEDIA_CFG_S* pmParam)
{
    HI_S32 ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pmParam,HI_PWRCTRL_EINVAL);
    HI_MPI_PM_SIGLE_MEDIA_CFG_S pmMediaCfg = {0};
    memcpy(&pmMediaCfg, pmParam, sizeof(HI_MPI_PM_SIGLE_MEDIA_CFG_S));
    ret = HI_MPI_PM_SetSingleMediaParam(&pmMediaCfg);
    if(ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_PM_SetMediaParam error.\n");
    }
    return ret;
}

#if defined(AMP_LINUX_HUAWEILITE)
static HI_S32 TEMPCYCLE_SetPMParamCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    HI_S32 ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstPrivData,HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pvRequest,HI_PWRCTRL_EINVAL);
    const HI_MPI_PM_SIGLE_MEDIA_CFG_S* pmParam = pvRequest;
    ret = HI_TEMPCYCLE_SetPMParam(pmParam);
    if(ret != HI_SUCCESS)
    {
        MLOGE("HI_TEMPCYCLE_SetPMParam error.\n");
    }
    return ret;
}
#endif

HI_S32 HI_TEMPCYCLE_Init(const HI_TEMPCYCLE_MONITORCFG_S* tempMonitorCfg)
{
    HI_S32 ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(tempMonitorCfg,HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR((tempMonitorCfg->highTemperature > tempMonitorCfg->normalTemperature),HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR((tempMonitorCfg->highTemperature > 0) && (tempMonitorCfg->normalTemperature > 0),HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR((tempMonitorCfg->tempLastTime_s > 0) && (tempMonitorCfg->tempHighAlarmTime_s > 0),HI_PWRCTRL_EINVAL);

    HI_MUTEX_LOCK(s_monitorContext.threadMutex);

    if (HI_TRUE  == s_monitorContext.taskInitState)
    {
        MLOGE("powerctrl temp monitor has created\n");
        HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
        return HI_PWRCTRL_EINITIALIZED;
    }

    memcpy(&(s_monitorContext.tempMonitorCfg), tempMonitorCfg, sizeof(HI_TEMPCYCLE_MONITORCFG_S));

    /* Create AHD Plug Check Task */
    s_monitorContext.threadRunFlag = HI_TRUE;
    ret = pthread_create(&s_monitorContext.threadCheckId, NULL, TEMPCYCLE_TempMonitorThread, NULL);
    if (HI_SUCCESS != ret)
    {
        MLOGE("Create temperature Monitor Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
        return HI_PWRCTRL_EINTER;
    }

#if defined(AMP_LINUX_HUAWEILITE)
    HI_S32 msgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_PM, TEMPCYCLE_SET_PMPARAM);
    ret |= HI_MSG_RegisterMsgProc(msgID, TEMPCYCLE_SetPMParamCallback, NULL);
#endif

    s_monitorContext.taskInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
    return ret;
}

HI_S32 HI_TEMPCYCLE_Deinit(HI_VOID)  /**DeInit*/
{
    HI_S32 ret = HI_SUCCESS;
    HI_MUTEX_LOCK(s_monitorContext.threadMutex);
    /** Destory ahd Check Task */
    if (HI_FALSE  == s_monitorContext.taskInitState)
    {
        MLOGW("temp cycle monitor not create\n");
        HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
        return HI_PWRCTRL_ENOINIT;
    }
    s_monitorContext.threadRunFlag = HI_FALSE;
    ret = pthread_join(s_monitorContext.threadCheckId, NULL);
    if (HI_SUCCESS != ret)
    {
        MLOGE("Join temp cycle monitor check Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
        return HI_PWRCTRL_EINTER;
    }

    s_monitorContext.taskInitState = HI_FALSE;

    HI_MUTEX_UNLOCK(s_monitorContext.threadMutex);
    return HI_SUCCESS;
}

#if (!defined(AMP_LINUX_HUAWEILITE))
HI_S32 HI_TEMPCYCLE_RegisterEvent(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    ret |= HI_EVTHUB_Register(HI_EVENT_TEMPCYCLE_TEMP_CHANGE);
    ret |= HI_EVTHUB_Register(HI_EVENT_TEMPCYCLE_HIGHTEMP_ALARM);
    return HI_SUCCESS;
}
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

