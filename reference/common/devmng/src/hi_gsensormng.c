/**
* @file    hi_gsensormng.c
* @brief   product gsensormng function
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

#include "hi_gsensormng.h"
#include "hi_hal_gsensor.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#ifdef CONFIG_GSENSOR_ON
/** macro define */
#define GSENSORMNG_CHECK_INTERVAL  (50) /**< gsensor collision check interval, unit ms */

/** GSENSORMNG context */
typedef struct tagGSENSORMNG_CONTEXT_S
{
    HI_BOOL bInitState;
    pthread_mutex_t Mutex;
    pthread_t ThreadCheckId;
    HI_BOOL bThreadRun;
    HI_GSENSORMNG_CFG_S stCfg;
} GSENSORMNG_CONTEXT_S;

static GSENSORMNG_CONTEXT_S s_stGSENSORMNGContext ={.bInitState = HI_FALSE,.Mutex= PTHREAD_MUTEX_INITIALIZER};

static HI_S32 GSENSORMNG_InParmValidChck(const HI_GSENSORMNG_CFG_S* stCfg)
{
    /** parm check */
    HI_APPCOMM_CHECK_POINTER(stCfg, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER((stCfg->enSensitity >= HI_GSENSORMNG_SENSITITY_OFF &&
        stCfg->enSensitity < HI_GSENSORMNG_SENSITITY_BUIT),HI_GSENSORMNG_EINVAL);

    return HI_SUCCESS;
}

static HI_S32 GSENSORMNG_InternalParmInit(const HI_GSENSORMNG_CFG_S* pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);
    s_stGSENSORMNGContext.bInitState = HI_FALSE;
    s_stGSENSORMNGContext.bThreadRun = HI_FALSE;
    memcpy(&s_stGSENSORMNGContext.stCfg,pstCfg,sizeof(HI_GSENSORMNG_CFG_S));
    return HI_SUCCESS;
}

static HI_VOID GSENSORMNG_GsensorHalParmwitch(const HI_GSENSORMNG_CFG_S* pstMngCfg,HI_HAL_GSENSOR_CFG_S* psthalCfg)
{
    psthalCfg->enSensitity = pstMngCfg->enSensitity;
    psthalCfg->stAttr.u32SampleRate= pstMngCfg->stAttr.u32SampleRate;
    return;
}


static HI_VOID GSENSORMNG_CollisionEventPublish(HI_VOID)
{
    HI_EVENT_S stEvent;
    memset(&stEvent, '\0', sizeof(stEvent));
    stEvent.EventID = HI_EVENT_GSENSORMNG_COLLISION;
    HI_EVTHUB_Publish(&stEvent);
    MLOGD("collision event\n");
    return;
}

static HI_VOID GSENSORMNG_CollisionCheck(HI_BOOL* pbStatus)
{
    HI_BOOL bCollisionOccur = HI_FALSE;
    if(HI_SUCCESS == HI_HAL_GSENSOR_GetCollisionStatus(&bCollisionOccur))
    {
        if(bCollisionOccur != *pbStatus)
        {
            if(HI_TRUE == bCollisionOccur)
            {
                GSENSORMNG_CollisionEventPublish();
            }
            *pbStatus = bCollisionOccur;
        }
    }
    return;
}


static HI_VOID*  GSENSORMNG_CollisionCheckThread(HI_VOID* pData)
{
    /** Set Task Name */
    prctl(PR_SET_NAME, "GSENSOR_LEVEL_CHECK", 0, 0, 0);
    HI_BOOL bCurCollisionOccur = HI_FALSE;

    while (s_stGSENSORMNGContext.bThreadRun)
    {
        GSENSORMNG_CollisionCheck(&bCurCollisionOccur);
        HI_usleep(GSENSORMNG_CHECK_INTERVAL * 1000);
    }

    MLOGD("gsensor check thread exit\n");
    return NULL;
}

HI_S32 HI_GSENSORMNG_Init(const HI_GSENSORMNG_CFG_S* pstCfg)
{
    HI_S32 s32Ret;

    if (HI_FAILURE == GSENSORMNG_InParmValidChck(pstCfg))
    {
        MLOGE("parm check error\n");
        return HI_GSENSORMNG_EINVAL;
    }

    HI_MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);

    if (HI_TRUE  == s_stGSENSORMNGContext.bInitState)
    {
        MLOGE("keymng has already been started\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_EINITIALIZED;
    }
    HI_HAL_GSENSOR_CFG_S stGsensorCfg;
    GSENSORMNG_GsensorHalParmwitch(pstCfg,&stGsensorCfg);
    s32Ret = HI_HAL_GSENSOR_Init(&stGsensorCfg);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_GSENSOR_Init Failed\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_EINTER;
    }

    GSENSORMNG_InternalParmInit(pstCfg);

    /* Create Gsensor collision Check Task */
    s_stGSENSORMNGContext.bThreadRun= HI_TRUE;
    s32Ret = pthread_create(&s_stGSENSORMNGContext.ThreadCheckId, NULL, GSENSORMNG_CollisionCheckThread, NULL);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Create Collsion Check Thread Fail!\n");
        HI_HAL_GSENSOR_Deinit();
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ETHREAD;
    }

    s_stGSENSORMNGContext.bInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
    return HI_SUCCESS;

}

HI_S32 HI_GSENSORMNG_SetSensitity(HI_GSENSORMNG_SENSITITY_E enSensitity)
{
    HI_S32 s32Ret;
    HI_APPCOMM_CHECK_POINTER((enSensitity >= HI_GSENSORMNG_SENSITITY_OFF &&
        enSensitity < HI_GSENSORMNG_SENSITITY_BUIT),HI_GSENSORMNG_EINVAL);


    HI_MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
    if (HI_FALSE  == s_stGSENSORMNGContext.bInitState)
    {
        MLOGW("gsensor manage no init\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ENOINIT;
    }

    s32Ret = HI_HAL_GSENSOR_SetSensitity((HI_HAL_GSENSOR_SENSITITY_E)enSensitity);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("gsensor set Sensitity error\n");
        s32Ret = HI_GSENSORMNG_EINTER;
    }
    HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
    return s32Ret;
}

HI_S32 HI_GSENSORMNG_GetAttr(HI_GSENSORMNG_ATTR_S* pstAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_GSENSORMNG_EINVAL);

    HI_MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
    if (HI_FALSE  == s_stGSENSORMNGContext.bInitState)
    {
        MLOGW("gsensor manage no init\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ENOINIT;
    }
    pstAttr->u32SampleRate = s_stGSENSORMNGContext.stCfg.stAttr.u32SampleRate;
    HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
    return HI_SUCCESS;
}

HI_S32 HI_GSENSORMNG_SetAttr(const HI_GSENSORMNG_ATTR_S* pstAttr)
{
    HI_HAL_GSENSOR_ATTR_S stGsensorAttr;
    HI_S32 s32Ret;
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_GSENSORMNG_EINVAL);

    HI_MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
    if (HI_FALSE  == s_stGSENSORMNGContext.bInitState)
    {
        MLOGW("gsensor manage no init\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ENOINIT;
    }
    stGsensorAttr.u32SampleRate = pstAttr->u32SampleRate;

    s32Ret = HI_HAL_GSENSOR_SetAttr(&stGsensorAttr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("gsensor set gensor attr error\n");
        s32Ret = HI_GSENSORMNG_EINTER;
    }
    HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
    return s32Ret;
}


HI_S32 HI_GSENSORMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;
    HI_MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
    /** Destory Gsensor Check Task */
    if (HI_FALSE  == s_stGSENSORMNGContext.bInitState)
    {
        MLOGW("gsensor manage no init\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ENOINIT;
    }
    s_stGSENSORMNGContext.bThreadRun= HI_FALSE;
    s32Ret = pthread_join(s_stGSENSORMNGContext.ThreadCheckId, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Join collsion Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_ETHREAD;
    }

    /** Close hal gsensor */
    s32Ret = HI_HAL_GSENSOR_Deinit();

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_HAL_GSENSOR_Deinit Fail!\n");
        HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
        return HI_GSENSORMNG_EINTER;
    }
    s_stGSENSORMNGContext.bInitState = HI_FALSE;

    HI_MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
    return HI_SUCCESS;
}
#else
HI_S32 HI_GSENSORMNG_Init(const HI_GSENSORMNG_CFG_S* pstCfg)
{
    return HI_SUCCESS;
}

HI_S32 HI_GSENSORMNG_SetSensitity(HI_GSENSORMNG_SENSITITY_E enSensitity)
{
    return HI_SUCCESS;
}

HI_S32 HI_GSENSORMNG_GetAttr(HI_GSENSORMNG_ATTR_S* pstAttr)
{
    return HI_SUCCESS;
}

HI_S32 HI_GSENSORMNG_SetAttr(const HI_GSENSORMNG_ATTR_S* pstAttr)
{
    return HI_SUCCESS;
}

HI_S32 HI_GSENSORMNG_Deinit(HI_VOID)
{
    return HI_SUCCESS;
}

#endif
HI_S32 HI_GSENSORMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret=HI_EVTHUB_Register(HI_EVENT_GSENSORMNG_COLLISION);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register collision event fail\n");
        return HI_GSENSORMNG_EREGISTEREVENT;
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
