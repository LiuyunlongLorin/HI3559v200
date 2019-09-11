/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_aenc.c
 * @brief   aenc server functions
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <pthread.h>
#include "mpi_sys.h"
#include "hi_comm_aio.h"
#include "hi_mapi_aenc.h"
#include "mpi_audio.h"
#include "hi_comm_aenc.h"
#include "mapi_acap_inner.h"
#include "mapi_aenc_inner.h"
#include "adpt_aenc_aac.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static MAPI_AENC_CHN_CONTEXT_S g_stAencContext;

static HI_S32 g_hEncoderAacHdl = HI_INVALID_HANDLE; /* aac encoder handle */
#define SEC2NSEC_SCALE (1 * 1000 * 1000 * 1000)
static pthread_mutex_t g_aencFuncLock[HI_MAPI_AENC_CHN_MAX_NUM] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
};

static HI_VOID MAPI_AENC_GetAData(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "data process\n");
    s32Ret = MAPI_AENC_GetAStream(AencHdl, g_stAencContext.astAencChn);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " mapi get data fail, s32Ret:%x\n", s32Ret);
    }
}

static HI_VOID MAPI_AENC_Cond_TimeWait(pthread_cond_t *pCond, pthread_mutex_t *pMutex, HI_U64 u64delayUs)
{
    struct timespec ts;
#ifndef __HuaweiLite__
    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
#endif

    HI_U64 u64DelayNs = u64delayUs * 1000;
    ts.tv_sec += (u64DelayNs / SEC2NSEC_SCALE);
    u64DelayNs = u64DelayNs % SEC2NSEC_SCALE;

    if (ts.tv_nsec + u64DelayNs > SEC2NSEC_SCALE) {
        ts.tv_sec++;
        ts.tv_nsec = (ts.tv_nsec + u64DelayNs) - SEC2NSEC_SCALE;
    } else {
        ts.tv_nsec += u64DelayNs;
    }

    pthread_cond_timedwait(pCond, pMutex, &ts);
    return;
}

HI_VOID *ProcAStreamThread(HI_VOID *arg)
{
    HI_S32 s32SelRet = 0;
    HI_HANDLE ChnHdl = *(HI_HANDLE *)arg;

    fd_set read_fds;
    struct timeval stTimeoutVal;
    stTimeoutVal.tv_sec = 0;
    stTimeoutVal.tv_usec = AENCODE_GET_FRAME_THREAD_SLEECT_TIMEOUT_USEC;

    prctl(PR_SET_NAME, (unsigned long)"ProcAStreamThread", 0, 0, 0);
    MUTEX_LOCK(g_stAencContext.astProcADataThread[ChnHdl].m_ProcADataThd_lock);

    while (g_stAencContext.astProcADataThread[ChnHdl].bProcADataThdStartFlg == HI_TRUE) {
        s32SelRet = 0;
        MUTEX_LOCK(g_stAencContext.astAencChn[ChnHdl].m_AencChn_lock);

        /* chn not started , time wait for low power consumption */
        if (g_stAencContext.astAencChn[ChnHdl].enAencChnState != MAPI_AENC_CHN_STARTED) {
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "chn [%d] no start\n", ChnHdl);
            MUTEX_UNLOCK(g_stAencContext.astAencChn[ChnHdl].m_AencChn_lock);
            MAPI_AENC_Cond_TimeWait(&g_stAencContext.astProcADataThread[ChnHdl].c_ProcADataThd_cond,
                                    &g_stAencContext.astProcADataThread[ChnHdl].m_ProcADataThd_lock,
                                    AENCODE_GET_FRAME_THREAD_USLEEP_TIME);
            continue;
        }

        /* chn started */
        FD_ZERO(&read_fds);
        g_stAencContext.astAencChn[ChnHdl].s32AencChnFd = HI_MPI_AENC_GetFd(
            g_stAencContext.astAencChn[ChnHdl].AencChnHdl);
        FD_SET(g_stAencContext.astAencChn[ChnHdl].s32AencChnFd, &read_fds);
        s32SelRet = select(g_stAencContext.astAencChn[ChnHdl].s32AencChnFd + 1, &read_fds, NULL, NULL, &stTimeoutVal);

        /* select error */
        if (s32SelRet < 0) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "select error,Ret:%x\n", s32SelRet);
        }
        /* no data get , flush datafifo */
        else if (s32SelRet == 0) {
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "chn [%d] no data get\n", ChnHdl);
            MAPI_AENC_ClearDatafifo(ChnHdl);
            MUTEX_UNLOCK(g_stAencContext.astAencChn[ChnHdl].m_AencChn_lock);
            MAPI_AENC_Cond_TimeWait(&g_stAencContext.astProcADataThread[ChnHdl].c_ProcADataThd_cond,
                                    &g_stAencContext.astProcADataThread[ChnHdl].m_ProcADataThd_lock,
                                    AENCODE_GET_FRAME_THREAD_USLEEP_TIME);
            continue;
        }
        /* have data to get */
        else {
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "chn [%d] have data to read\n", ChnHdl);
            /* NULL callback , time wait for low power consumption */
            if (MAPI_AENC_IsRegisterCallback(ChnHdl, g_stAencContext.astAencChn) == HI_FALSE) {
                MUTEX_UNLOCK(g_stAencContext.astAencChn[ChnHdl].m_AencChn_lock);
                MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "no cb\n");
                MAPI_AENC_Cond_TimeWait(&g_stAencContext.astProcADataThread[ChnHdl].c_ProcADataThd_cond,
                                        &g_stAencContext.astProcADataThread[ChnHdl].m_ProcADataThd_lock,
                                        AENCODE_GET_FRAME_THREAD_USLEEP_TIME);
                continue;
            }

            /* callback not NULL , get data to process */
            MAPI_AENC_GetAData(ChnHdl);
        }
        MUTEX_UNLOCK(g_stAencContext.astAencChn[ChnHdl].m_AencChn_lock);
    }
    MUTEX_UNLOCK(g_stAencContext.astProcADataThread[ChnHdl].m_ProcADataThd_lock);
    return HI_NULL;
}

HI_S32 MAPI_AENC_Init(HI_VOID)
{
    HI_U32 i;

    /* check state */
    if (g_stAencContext.bAencInited == HI_TRUE) {
        return HI_SUCCESS;
    }

    /* init chn and datafifo attr */
    memset(&g_stAencContext, 0, sizeof(MAPI_AENC_CHN_CONTEXT_S));

    MAPI_AENC_OSInit();

    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        g_stAencContext.astAencChn[i].enAencChnState = MAPI_AENC_CHN_UNINITED;
        g_stAencContext.astAencChn[i].s32AiChn = HI_INVALID_HANDLE;
        g_stAencContext.astAencChn[i].s32AiDev = HI_INVALID_HANDLE;
        g_stAencContext.astAencChn[i].AencChnHdl = i;
    }

    /* init module  state and encoder handle */
    g_hEncoderAacHdl = HI_INVALID_HANDLE;
    g_stAencContext.bAencInited = HI_TRUE;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_Init!\n");
    return HI_SUCCESS;
}

HI_S32 MAPI_AENC_Deinit(HI_VOID)
{
    HI_U32 i;

    /* check state */
    if (g_stAencContext.bAencInited != HI_TRUE) {
        return HI_SUCCESS;
    }
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        if (g_stAencContext.astAencChn[i].enAencChnState != MAPI_AENC_CHN_UNINITED) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC %d is not deinit ,try deinit it now\n", i);
            HI_S32 s32Ret = HI_MAPI_AENC_Deinit(i);

            if (s32Ret != HI_SUCCESS) {
                return s32Ret;
            }
        }
    }

    /* deinit chn and datafifo attr */
    MAPI_AENC_OSDeinit();
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        memset(&g_stAencContext.astAencChn[i], 0, sizeof(MAPI_AENC_CHN_S));
        g_stAencContext.astAencChn[i].s32AiChn = HI_INVALID_HANDLE;
        g_stAencContext.astAencChn[i].s32AiDev = HI_INVALID_HANDLE;
        g_stAencContext.astAencChn[i].enAencChnState = MAPI_AENC_CHN_BUTT;
    }

    /* deinit module state and encorder handle */
    g_hEncoderAacHdl = HI_INVALID_HANDLE;
    g_stAencContext.bAencInited = HI_FALSE;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_DeInit!\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Init(HI_HANDLE AencHdl, const HI_MAPI_AENC_ATTR_S *pstAencAttr)
{
    HI_S32 s32Ret;
    AENC_CHN_ATTR_S stAencChnAttr;
    HI_S32 i;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencAttr);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is started, no need init aenc repeatedly!\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STOPED
        && pstAencAttr->enAencFormat == g_stAencContext.astAencChn[AencHdl].stAencChnAttr.enAencFormat
        && pstAencAttr->u32PtNumPerFrm == g_stAencContext.astAencChn[AencHdl].stAencChnAttr.u32PtNumPerFrm) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc init attr is not change, return successful\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* create chn and set attr */
    memset(&stAencChnAttr, 0, sizeof(AENC_CHN_ATTR_S));
    switch (pstAencAttr->enAencFormat) {
        case HI_MAPI_AUDIO_FORMAT_AACLC:
            stAencChnAttr.enType = PT_AAC;
            break;

        case HI_MAPI_AUDIO_FORMAT_G711A:
            stAencChnAttr.enType = PT_G711A;
            break;

        case HI_MAPI_AUDIO_FORMAT_G711U:
            stAencChnAttr.enType = PT_G711U;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "not support Aenc format, the curr is:%d\n", pstAencAttr->enAencFormat);
            MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
            return HI_MAPI_AENC_EILLPARAM;
    }
    stAencChnAttr.u32BufSize = MAX_AUDIO_FRAME_NUM;
    stAencChnAttr.u32PtNumPerFrm = pstAencAttr->u32PtNumPerFrm;
    CHECK_MAPI_AENC_NULL_PTR(pstAencAttr->pValue);
    stAencChnAttr.pValue = pstAencAttr->pValue;

    s32Ret = HI_MPI_AENC_CreateChn(AencHdl, &stAencChnAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_CreateChn fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* create thread */
    MUTEX_INIT_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
#ifndef __HuaweiLite__
    pthread_condattr_t condAttr;
    (HI_VOID)pthread_condattr_init(&condAttr);
    (HI_VOID)pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
    (HI_VOID)pthread_cond_init(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond, &condAttr);
    (HI_VOID)pthread_condattr_destroy(&condAttr);
#else
    (HI_VOID)pthread_cond_init(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond, HI_NULL);
#endif

    g_stAencContext.astProcADataThread[AencHdl].tProcADataThread = -1;

    /* set thread state */
    MUTEX_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    g_stAencContext.astProcADataThread[AencHdl].bProcADataThdStartFlg = HI_TRUE;
    MUTEX_UNLOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);

    s32Ret = pthread_create(&g_stAencContext.astProcADataThread[AencHdl].tProcADataThread, HI_NULL, ProcAStreamThread,
                            &g_stAencContext.astAencChn[AencHdl].AencChnHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "pthread_create ProcAStreamThread s32Ret:%x\n", s32Ret);

        MUTEX_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
        g_stAencContext.astProcADataThread[AencHdl].bProcADataThdStartFlg = HI_FALSE;
        MUTEX_UNLOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);

        MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
        pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* save chn attr and init lock */
    MUTEX_INIT_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    memcpy(&g_stAencContext.astAencChn[AencHdl].stAencChnAttr, pstAencAttr, sizeof(HI_MAPI_AENC_ATTR_S));
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;
    }

    /*  set chn state  */
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STOPED;
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Init [%d]\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Deinit(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;
    HI_S32 i;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is started ,can not be deinit ! try stop now\n");
        s32Ret = HI_MAPI_AENC_Stop(AencHdl);

        if (s32Ret != HI_SUCCESS) {
            return s32Ret;
        }
    }

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc has already deinited ,return successfull !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /*  for exceptional case : deinit when chn is started , flush datafifo to destory chn successful */
    MAPI_AENC_ClearDatafifo(AencHdl);

    /* destory chn */
    s32Ret = HI_MPI_AENC_DestroyChn(AencHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_DestroyChn fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* stop thread */
    MUTEX_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    g_stAencContext.astProcADataThread[AencHdl].bProcADataThdStartFlg = HI_FALSE;
    pthread_cond_signal(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);
    MUTEX_UNLOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    if (g_stAencContext.astProcADataThread[AencHdl].tProcADataThread != -1) {
        s32Ret = pthread_join(g_stAencContext.astProcADataThread[AencHdl].tProcADataThread, HI_NULL);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "pthread_join fail,ret:%x\n", s32Ret);
            MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
            pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);
            MUTEX_DESTROY(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
            MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
            return s32Ret;
        }
    }

    MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);

    /* deinit chn attr and destory lock */
    MUTEX_DESTROY(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    memset(&g_stAencContext.astAencChn[AencHdl].stAencChnAttr, 0x0, sizeof(HI_MAPI_AENC_ATTR_S));
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;
    }

    /* set chn state */
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_UNINITED;
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_DeInit [%d]\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Start(HI_HANDLE AencHdl)
{
    /* check state and attr */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinited ,can not be start !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STARTED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc has already started ,return successfull !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* start chn */
    MUTEX_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STARTED;
    MUTEX_UNLOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Start [%d]\n", AencHdl);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Stop(HI_HANDLE AencHdl)
{
    /* check state and attr */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinited ,can not be stop !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STOPED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc has already inited with stoped ,return successfull !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* stop chn */
    MUTEX_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STOPED;
    MUTEX_UNLOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Stop [%d]\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_RegisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB)
{
    HI_U32 i;
    HI_S32 s32NullIdx = -1;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencCB);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not reg callback func !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* reg CB , search the same callback to update */
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        /* save null callback position */
        if (g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB == HI_NULL) {
            s32NullIdx = i;
        }

        if (pstAencCB->pfnDataCB == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB
            && pstAencCB->pPrivateData == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData) {
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = pstAencCB->pfnDataCB;
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = pstAencCB->pPrivateData;
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "update register callback success!\n");
            MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
            return HI_SUCCESS;
        }
    }

    /* if no find same callback , set new callback to null callback postion */
    if (s32NullIdx != -1) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[s32NullIdx].pfnDataCB = pstAencCB->pfnDataCB;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[s32NullIdx].pPrivateData = pstAencCB->pPrivateData;
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "register new callback success!\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);

    return HI_MAPI_AENC_ERESFULL;
}

HI_S32 HI_MAPI_AENC_UnregisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB)
{
    HI_U32 i;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencCB);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not unreg callback func !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* unreg CB */
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        if (pstAencCB->pfnDataCB == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB
            && pstAencCB->pPrivateData == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData) {
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnregisterCallback!\n");
            MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
            return HI_SUCCESS;
        }
    }

    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);

    return HI_MAPI_AENC_EUNEXIST;
}

HI_S32 HI_MAPI_AENC_BindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl)
{
    MPP_CHN_S stSrcChn, stDestChn;
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_ACAPCHNHANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_AENC_ACAPHANDLE_RANGE(AcapHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not bind acap !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* if bind already , return success */
    if ((HI_HANDLE)g_stAencContext.astAencChn[AencHdl].s32AiDev == AcapHdl
        && (HI_HANDLE) g_stAencContext.astAencChn[AencHdl].s32AiChn == AcapChnHdl) {
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = AcapHdl;
    stSrcChn.s32ChnId = AcapChnHdl;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AencHdl;
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_SYS_Bind err:0x%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    g_stAencContext.astAencChn[AencHdl].s32AiDev = AcapHdl;
    g_stAencContext.astAencChn[AencHdl].s32AiChn = AcapChnHdl;
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_BindACap[%d]: aidev-%d aichn-%d ! \n",
                     AencHdl, g_stAencContext.astAencChn[AencHdl].s32AiDev,
                     g_stAencContext.astAencChn[AencHdl].s32AiChn);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_UnbindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl)
{
    MPP_CHN_S stSrcChn, stDestChn;
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_ACAPCHNHANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_AENC_ACAPHANDLE_RANGE(AcapHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is uninited ,can not bind acap !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* if unbind already , return success */
    if ((HI_HANDLE)g_stAencContext.astAencChn[AencHdl].s32AiDev != AcapHdl
        || (HI_HANDLE) g_stAencContext.astAencChn[AencHdl].s32AiChn != AcapChnHdl) {
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32DevId = AcapHdl;
    stSrcChn.s32ChnId = AcapChnHdl;
    stDestChn.enModId = HI_ID_AENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = AencHdl;
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_SYS_Bind err:0x%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    g_stAencContext.astAencChn[AencHdl].s32AiDev = HI_INVALID_HANDLE;
    g_stAencContext.astAencChn[AencHdl].s32AiChn = HI_INVALID_HANDLE;
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnBindACap[%d] !\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat)
{
    HI_S32 s32Ret;
    AENC_ENCODER_S stAencEncoder;
    HI_S32 *phEncoderHdl = HI_NULL;

    switch (enAudioFormat) {
        case HI_MAPI_AUDIO_FORMAT_AACLC:
            ADPT_AENC_GetAacEncoder(&stAencEncoder);
            phEncoderHdl = &g_hEncoderAacHdl;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "not support format\n");
            return HI_MAPI_AENC_EILLPARAM;
    }

    if (*phEncoderHdl != HI_INVALID_HANDLE) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_AENC, "encoder has been register, return successful\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_AENC_RegisterEncoder(phEncoderHdl, &stAencEncoder);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_RegeisterEncoder  err, s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_UnregisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat)
{
    HI_S32 s32Ret;
    HI_S32 *phEncoderHdl = HI_NULL;

    switch (enAudioFormat) {
        case HI_MAPI_AUDIO_FORMAT_AACLC:
            phEncoderHdl = &g_hEncoderAacHdl;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "not support format\n");
            return HI_MAPI_AENC_EILLPARAM;
    }

    if (*phEncoderHdl == HI_INVALID_HANDLE) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_AENC, "encoder has been unregister, return successful\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_AENC_UnRegisterEncoder(*phEncoderHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_UnRegisterEncoder  err, s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    *phEncoderHdl = HI_INVALID_HANDLE;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_SendFrame(HI_HANDLE AencHdl, const AUDIO_FRAME_S *pstFrm)
{
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstFrm);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not send frame !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    if (g_stAencContext.astAencChn[AencHdl].s32AiDev != HI_INVALID_HANDLE
        || g_stAencContext.astAencChn[AencHdl].s32AiChn != HI_INVALID_HANDLE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc have already bind ,can not send frame !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }

    s32Ret = HI_MPI_AENC_SendFrame(AencHdl, pstFrm, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_SendFrame  err, s32Ret:%x\n", s32Ret);
    }

    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
