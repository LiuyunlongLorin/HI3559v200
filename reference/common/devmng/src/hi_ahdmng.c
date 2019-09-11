/**
* @file    hi_ahdmng.c
* @brief   product ahdmng function
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
* @version

*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_ahdmng.h"
#include "hi_mapi_hal_ahd.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#ifdef CONFIG_AHD_ON
/** macro define */
#define AHDMNG_CHECK_INTERVAL  (1000) /**< ahd plug check interval, unit ms */

/** ahd context */
typedef struct tagAHDMNG_CONTEXT_S
{
    HI_BOOL bInitState;
    pthread_mutex_t Mutex;
    pthread_t ThreadCheckId;
    HI_BOOL bThreadRun;
    HI_AHDMNG_CFG_S stCfg;
    HI_HAL_AHD_STATUS_E aenPlug[HI_AHDMNG_MAX_CAM_NUM][HI_HAL_AHD_CHN_MAX];
} AHDMNG_CONTEXT_S;

static AHDMNG_CONTEXT_S s_stAHDMNGContext ={.bInitState = HI_FALSE,.Mutex= PTHREAD_MUTEX_INITIALIZER};

typedef struct tagAHDMNG_MSG_S
{
    HI_HANDLE CamID;
    HI_U32 u32Chn;
    HI_AHDMNG_PLUG_E enStatus;
} AHDMNG_MSG_S;

static HI_S32 AHDMNG_GetAhdPlugStatus(HI_AHDMNG_CAM_CFG_S* pstAhdCamCfg,HI_HAL_AHD_STATUS_E* penStatus)
{
    HI_S32 s32Ret;
    s32Ret = HI_MAPI_VCAP_AhdGetStatus(pstAhdCamCfg->CamID,penStatus,pstAhdCamCfg->u32ChnNum);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FAILURE);
    return HI_SUCCESS;
}

static HI_VOID AHDMNG_PlugCheck(HI_U32 u32CamIndex)
{
    HI_HAL_AHD_STATUS_E aenCurPlug[HI_HAL_AHD_CHN_MAX];
    HI_S32 s32Ret;

    s32Ret = AHDMNG_GetAhdPlugStatus(&s_stAHDMNGContext.stCfg.astCamCfg[u32CamIndex],aenCurPlug);
    if(HI_SUCCESS == s32Ret)
    {
        for(HI_S32 i = 0; i < s_stAHDMNGContext.stCfg.astCamCfg[u32CamIndex].u32ChnNum;i++)
        {
            if(aenCurPlug[i] != s_stAHDMNGContext.aenPlug[u32CamIndex][i])
            {
                HI_EVENT_S stEvent;
                memset(&stEvent, '\0', sizeof(stEvent));
                stEvent.EventID = HI_EVENT_AHDMNG_PLUG_STATUS;
                stEvent.arg1 = aenCurPlug[i];
                stEvent.arg2 = s_stAHDMNGContext.stCfg.astCamCfg[u32CamIndex].CamID;
                stEvent.aszPayload[0] = i;
                s32Ret = HI_EVTHUB_Publish(&stEvent);
            }
        }
    }
    return;
}
static HI_VOID*  AHDMNG_PlugCheckThread(HI_VOID* pData)
{
    /** Set Task Name */
    prctl(PR_SET_NAME, "AHD_PLUG_CHECK", 0, 0, 0);
    while (s_stAHDMNGContext.bThreadRun)
    {
        for(HI_S32 i = 0; i < s_stAHDMNGContext.stCfg.u32Cnt; i++)
        {
            AHDMNG_PlugCheck(i);
        }
        HI_usleep(AHDMNG_CHECK_INTERVAL * 1000);
    }
    MLOGI("ahd check thread exit\n");
    return NULL;
}

HI_S32 HI_AHDMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret=HI_EVTHUB_Register(HI_EVENT_AHDMNG_PLUG_STATUS);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register AHDMNG event fail\n");
        return HI_AHDMNG_EREGISTEREVENT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_AHDMNG_Init(HI_AHDMNG_CFG_S* pstCfg)
{
    HI_S32 s32Ret;
    HI_APPCOMM_CHECK_POINTER(pstCfg,HI_AHDMNG_EINVAL);
    HI_MUTEX_LOCK(s_stAHDMNGContext.Mutex);

    if (HI_TRUE  == s_stAHDMNGContext.bInitState)
    {
        MLOGE("ahdmng has init\n");
        HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
        return HI_AHDMNG_EINITIALIZED;
    }
    memcpy(&s_stAHDMNGContext.stCfg,pstCfg,sizeof(HI_AHDMNG_CFG_S));

    for(HI_S32 i = 0; i < s_stAHDMNGContext.stCfg.u32Cnt; i++)
    {
        s32Ret = AHDMNG_GetAhdPlugStatus(&s_stAHDMNGContext.stCfg.astCamCfg[i],s_stAHDMNGContext.aenPlug[i]);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_AHDMNG_EINTER,"Get Ahd Plug Status");
    }
    /* Create AHD Plug Check Task */
    s_stAHDMNGContext.bThreadRun= HI_TRUE;
    s32Ret = pthread_create(&s_stAHDMNGContext.ThreadCheckId, NULL, AHDMNG_PlugCheckThread, NULL);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Create ahd plug Check Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
        return HI_AHDMNG_ETHREAD;
    }

    s_stAHDMNGContext.bInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
    return HI_SUCCESS;

}

HI_S32 HI_AHDMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MUTEX_LOCK(s_stAHDMNGContext.Mutex);
    /** Destory ahd Check Task */
    if (HI_FALSE  == s_stAHDMNGContext.bInitState)
    {
        MLOGW("ahd manage no init\n");
        HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
        return HI_AHDMNG_ENOINIT;
    }
    s_stAHDMNGContext.bThreadRun= HI_FALSE;
    s32Ret = pthread_join(s_stAHDMNGContext.ThreadCheckId, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Join AHD check Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
        return HI_AHDMNG_ETHREAD;
    }

    s_stAHDMNGContext.bInitState = HI_FALSE;

    HI_MUTEX_UNLOCK(s_stAHDMNGContext.Mutex);
    return HI_SUCCESS;
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
