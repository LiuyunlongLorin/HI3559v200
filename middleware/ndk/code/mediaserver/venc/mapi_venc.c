/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_venc.c
 * @brief   venc server function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#include "mpi_region.h"
#include "mpi_venc.h"
#include "mpi_vi.h"
#include "mpi_vb.h"
#include "mpi_sys.h"

#include "hi_mapi_comm_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_vproc_define.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_errno.h"
#include "mapi_venc_inner.h"
#include "mapi_comm_inner.h"
#include "mapi_vcap_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static MAPI_VENC_CONTEXT_S g_stVencContext;
HI_BOOL g_abLiteBootStarted[HI_MAPI_VENC_MAX_CHN_NUM] = { HI_FALSE };
static pthread_mutex_t g_Venc_FuncLock[HI_MAPI_VENC_MAX_CHN_NUM] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
};

HI_BOOL MAPI_VENC_GetLiteBootStatus(HI_HANDLE VencHdl)
{
    HI_BOOL bStatus;
    bStatus = g_abLiteBootStarted[VencHdl];
    return bStatus;
}

HI_VOID MAPI_VENC_GetStreamInfo(PAYLOAD_TYPE_E enType, const VENC_STREAM_S *pstVStream,
                                HI_MAPI_VENC_STREAM_INFO_S *pstStreamInfo)
{
    switch (enType) {
        case PT_H264: {
            pstStreamInfo->enRefType = pstVStream->stH264Info.enRefType;
            pstStreamInfo->u32StartQp = pstVStream->stH264Info.u32StartQp;
        }
        break;

        case PT_H265: {
            pstStreamInfo->enRefType = pstVStream->stH265Info.enRefType;
            pstStreamInfo->u32StartQp = pstVStream->stH265Info.u32StartQp;
        }
        break;

        case PT_JPEG: {
            /* TODO */
        }
        break;

        case PT_MJPEG: {
            /* TODO */
        }
        break;

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "not support encode format,the curr is:%d\n", enType);
        }
        break;
    }
}

HI_VOID MAPI_VENC_GetPacketType(PAYLOAD_TYPE_E enType, VENC_DATA_TYPE_U DataType,
                                HI_MAPI_VENC_DATA_TYPE_S *pstDataType)
{
    switch (enType) {
        case PT_H264: {
            pstDataType->enPayloadType = HI_MAPI_PAYLOAD_TYPE_H264;
            pstDataType->enH264EType = DataType.enH264EType;
        }
        break;

        case PT_H265: {
            pstDataType->enPayloadType = HI_MAPI_PAYLOAD_TYPE_H265;
            pstDataType->enH265EType = DataType.enH265EType;
        }
        break;

        case PT_JPEG: {
            pstDataType->enPayloadType = HI_MAPI_PAYLOAD_TYPE_JPEG;
            pstDataType->enJPEGEType = DataType.enJPEGEType;
        }
        break;

        case PT_MJPEG: {
            pstDataType->enPayloadType = HI_MAPI_PAYLOAD_TYPE_MJPEG;
        }
        break;

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "not support encode format,the curr is:%d\n", enType);
        }
        break;
    }
}

HI_S32 MAPI_VENC_GetStartFrameCnt(HI_HANDLE VencHdl)
{
    return g_stVencContext.astVencChn[VencHdl].s32StartFrameCount;
}

HI_BOOL MAPI_VENC_IsRegisterCallback(HI_HANDLE VencHdl)
{
    HI_U32 i;

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        if (g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB != HI_NULL) {
            return HI_TRUE;
        }
    }

    return HI_FALSE;
}

static HI_VOID MAPI_VENC_Cond_TimeWait(pthread_cond_t *pCond, pthread_mutex_t *pMutex, HI_U64 u64delayUs)
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

HI_VOID *ProcVDataThread(HI_VOID *arg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VencHdl = *(HI_HANDLE *)arg;
    HI_CHAR szThreadName[16] = {0};
    fd_set read_fds;
    struct timeval stTimeoutVal;
    stTimeoutVal.tv_sec = 0;
    stTimeoutVal.tv_usec = VENC_GET_FRAME_THREAD_SLEECT_TIMEOUT_USEC;

    snprintf(szThreadName, 16, "Hi_pTVData_Chn%u", VencHdl);
    prctl(PR_SET_NAME, (unsigned long)szThreadName, 0, 0, 0);
    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    while (g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg == HI_TRUE) {
        if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE) {
            MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            FD_ZERO(&read_fds);

            g_stVencContext.astVencChn[VencHdl].s32VencFd = HI_MPI_VENC_GetFd(
                g_stVencContext.astVencChn[VencHdl].VencHandle);
            FD_SET(g_stVencContext.astVencChn[VencHdl].s32VencFd, &read_fds);

            s32Ret = select(g_stVencContext.astVencChn[VencHdl].s32VencFd + 1, &read_fds, HI_NULL, HI_NULL,
                            &stTimeoutVal);

            if (s32Ret < 0) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "select fail,Ret:%x\n", s32Ret);
            } else if (s32Ret == 0) {
                /* if No new data ,Update the activated  channel and release memory */
                MAPI_VENC_ClearDatafifoData(VencHdl);
            } else {
                if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE
                    && FD_ISSET(g_stVencContext.astVencChn[VencHdl].s32VencFd, &read_fds)) {
                    s32Ret = MAPI_VENC_GetVStream(g_stVencContext.astVencChn[VencHdl].VencHandle,
                                                  &g_stVencContext.astVencChn[VencHdl]);

                    if (s32Ret != HI_SUCCESS) {
                        MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_GetLink_VStream vencHdl[%d] fail,s32Ret:%x\n",
                                        VencHdl, s32Ret);
                        MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                        /* Low power mode,Let out the cpu, scheduling the other threads. */
                        MAPI_VENC_Cond_TimeWait(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond,
                                                &g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock,
                                                VENC_GET_FRAME_THREAD_USLEEP_TIME);
                        continue;
                    }
                } else {
                    /* Low power mode,Let out the cpu, scheduling the other threads. */
                    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                    MAPI_VENC_Cond_TimeWait(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond,
                                            &g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock,
                                            VENC_GET_FRAME_THREAD_USLEEP_TIME);
                    continue;
                }
            }

            MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        } else {
            /* Low power mode,Let out the cpu, scheduling the other threads. */
            MAPI_VENC_Cond_TimeWait(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond,
                                    &g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock,
                                    VENC_GET_FRAME_THREAD_USLEEP_TIME);
        }
    }

    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    return HI_NULL;
}

static HI_S32 MAPI_VENC_SetGopAttr(const HI_MAPI_VENC_GOP_ATTR_S *pstVencGopAttr,
    VENC_GOP_ATTR_S *pstVencChnGopAttr)
{
    switch (pstVencGopAttr->enGopMode) {
        case HI_MAPI_VENC_GOPMODE_NORMALP: {
            pstVencChnGopAttr->enGopMode = VENC_GOPMODE_NORMALP;
            pstVencChnGopAttr->stNormalP.s32IPQpDelta = pstVencGopAttr->stNormalP.s32IPQpDelta;
            break;
        }

        case HI_MAPI_VENC_GOPMODE_DUALP: {
            pstVencChnGopAttr->enGopMode = VENC_GOPMODE_DUALP;
            pstVencChnGopAttr->stDualP.u32SPInterval = pstVencGopAttr->stDualP.u32SPInterval;
            pstVencChnGopAttr->stDualP.s32SPQpDelta = pstVencGopAttr->stDualP.s32SPQpDelta;
            pstVencChnGopAttr->stDualP.s32IPQpDelta = pstVencGopAttr->stDualP.s32IPQpDelta;
            break;
        }

        case HI_MAPI_VENC_GOPMODE_SMARTP: {
            pstVencChnGopAttr->enGopMode = VENC_GOPMODE_SMARTP;
            pstVencChnGopAttr->stSmartP.s32BgQpDelta = pstVencGopAttr->stSmartP.s32BgQpDelta;
            pstVencChnGopAttr->stSmartP.s32ViQpDelta = pstVencGopAttr->stSmartP.s32ViQpDelta;
            pstVencChnGopAttr->stSmartP.u32BgInterval = pstVencGopAttr->stSmartP.u32BgInterval;
            break;
        }

        case HI_MAPI_VENC_GOPMODE_BIPREDB: {
            pstVencChnGopAttr->enGopMode = VENC_GOPMODE_BIPREDB;
            pstVencChnGopAttr->stBipredB.u32BFrmNum = pstVencGopAttr->stBipredB.u32BFrmNum;
            pstVencChnGopAttr->stBipredB.s32BQpDelta = pstVencGopAttr->stBipredB.s32BQpDelta;
            pstVencChnGopAttr->stBipredB.s32IPQpDelta = pstVencGopAttr->stBipredB.s32IPQpDelta;
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "not support enGopMode:%d\n", pstVencChnGopAttr->enGopMode);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_GetGopAttr(const VENC_GOP_ATTR_S *pstGopAttr, HI_MAPI_VENC_GOP_ATTR_S *pstVencGopAttr)
{
    switch (pstGopAttr->enGopMode) {
        case VENC_GOPMODE_NORMALP: {
            pstVencGopAttr->enGopMode = HI_MAPI_VENC_GOPMODE_NORMALP;
            pstVencGopAttr->stNormalP.s32IPQpDelta = pstGopAttr->stNormalP.s32IPQpDelta;
            break;
        }

        case VENC_GOPMODE_DUALP: {
            pstVencGopAttr->enGopMode = HI_MAPI_VENC_GOPMODE_DUALP;
            pstVencGopAttr->stDualP.u32SPInterval = pstGopAttr->stDualP.u32SPInterval;
            pstVencGopAttr->stDualP.s32SPQpDelta = pstGopAttr->stDualP.s32SPQpDelta;
            pstVencGopAttr->stDualP.s32IPQpDelta = pstGopAttr->stDualP.s32IPQpDelta;
            break;
        }

        case VENC_GOPMODE_SMARTP: {
            pstVencGopAttr->enGopMode = HI_MAPI_VENC_GOPMODE_SMARTP;
            pstVencGopAttr->stSmartP.s32BgQpDelta = pstGopAttr->stSmartP.s32BgQpDelta;
            pstVencGopAttr->stSmartP.s32ViQpDelta = pstGopAttr->stSmartP.s32ViQpDelta;
            pstVencGopAttr->stSmartP.u32BgInterval = pstGopAttr->stSmartP.u32BgInterval;
            break;
        }

        case VENC_GOPMODE_BIPREDB: {
            pstVencGopAttr->enGopMode = HI_MAPI_VENC_GOPMODE_BIPREDB;
            pstVencGopAttr->stBipredB.u32BFrmNum = pstGopAttr->stBipredB.u32BFrmNum;
            pstVencGopAttr->stBipredB.s32BQpDelta = pstGopAttr->stBipredB.s32BQpDelta;
            pstVencGopAttr->stBipredB.s32IPQpDelta = pstGopAttr->stBipredB.s32IPQpDelta;
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "not support enGopMode:%d\n", pstGopAttr->enGopMode);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_UpdateRcSrcFrameRate(HI_HANDLE VencHdl, HI_S32 s32SrcFrameRate)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencAttr;
    memset(&stVencAttr, 0, sizeof(VENC_CHN_ATTR_S));

    s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stVencAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnAttr error,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    switch (stVencAttr.stVencAttr.enType) {
        case PT_H264: {
            switch (stVencAttr.stRcAttr.enRcMode) {
                case VENC_RC_MODE_H264CBR: {
                    stVencAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;
                }

                case VENC_RC_MODE_H264VBR: {
                    stVencAttr.stRcAttr.stH264Vbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;
                }

                default:
                {
                    stVencAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;
                }
            }
            break;
        }

        case PT_H265: {
            switch (stVencAttr.stRcAttr.enRcMode) {
                case VENC_RC_MODE_H265CBR:
                    stVencAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;

                case VENC_RC_MODE_H265VBR:
                    stVencAttr.stRcAttr.stH265Vbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;

                default:
                    stVencAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;
            }
            break;
        }

        case PT_MJPEG: {
            switch (stVencAttr.stRcAttr.enRcMode) {
                case VENC_RC_MODE_MJPEGCBR:
                    stVencAttr.stRcAttr.stMjpegCbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;

                case VENC_RC_MODE_MJPEGVBR:
                    stVencAttr.stRcAttr.stMjpegVbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;

                default:
                    stVencAttr.stRcAttr.stMjpegCbr.u32SrcFrameRate = s32SrcFrameRate;
                    break;
            }
            break;
        }

        default:
            break;
    }

    s32Ret = HI_MPI_VENC_SetChnAttr(VencHdl, &stVencAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetChnAttr error,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_InitVDataThread(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;

    /* create server stream processing thread */
    MUTEX_INIT_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
#ifndef __HuaweiLite__
    pthread_condattr_t condAttr;
    (HI_VOID)pthread_condattr_init(&condAttr);
    (HI_VOID)pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
    (HI_VOID)pthread_cond_init(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond, &condAttr);
    (HI_VOID)pthread_condattr_destroy(&condAttr);
#else
    (HI_VOID)pthread_cond_init(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond, HI_NULL);
#endif

    g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread = -1;
    g_stVencContext.astVencChn[VencHdl].VencHandle = VencHdl;

    /* start thread and set chn state */
    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    s32Ret = pthread_create(&g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread, HI_NULL, ProcVDataThread,
                            &g_stVencContext.astVencChn[VencHdl].VencHandle);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "create ProcVDataThread fail,s32Ret:%x\n", s32Ret);

        MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
        g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_FALSE;
        ;
        MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

        MUTEX_DESTROY(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
        pthread_cond_destroy(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_DeinitVDataThread(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_FALSE;
    pthread_cond_signal(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond);
    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    if (g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread != -1) {
        s32Ret = pthread_join(g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "pthread_join fail,s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    }

    g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread = -1;
    MUTEX_DESTROY(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    pthread_cond_destroy(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_CheckAttr(const HI_MAPI_VENC_ATTR_S *pstVencChnAttr)
{
    CHECK_MAPI_VENC_NULL_PTR(pstVencChnAttr);

    switch (pstVencChnAttr->stVencPloadTypeAttr.enType) {
        case HI_MAPI_PAYLOAD_TYPE_H264: {
            /* H264 profile range:[0,2] */
            if (pstVencChnAttr->stVencPloadTypeAttr.u32Profile > 2) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC attr u32Profile:%d error\n",
                               pstVencChnAttr->stVencPloadTypeAttr.u32Profile);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
        }
        break;

        case HI_MAPI_PAYLOAD_TYPE_H265:

            /* H265 profile range:[0,1] */
            if (pstVencChnAttr->stVencPloadTypeAttr.u32Profile > 1) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC attr u32Profile:%d error\n",
                               pstVencChnAttr->stVencPloadTypeAttr.u32Profile);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
            break;

        case HI_MAPI_PAYLOAD_TYPE_MJPEG:

            /* MJPEG profile range:0 */
            if (pstVencChnAttr->stVencPloadTypeAttr.u32Profile != 0) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC attr u32Profile:%d error\n",
                               pstVencChnAttr->stVencPloadTypeAttr.u32Profile);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
            break;

        case HI_MAPI_PAYLOAD_TYPE_JPEG:

            /* JPEG profile range:0 */
            if (pstVencChnAttr->stVencPloadTypeAttr.u32Profile != 0) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC attr u32Profile:%d error\n",
                               pstVencChnAttr->stVencPloadTypeAttr.u32Profile);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
            break;

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC attr enType:%d unsupport\n",
                           pstVencChnAttr->stVencPloadTypeAttr.enType);
            return HI_MAPI_VENC_EILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_ConfigMjpegRcAttr(const HI_MAPI_VENC_RC_ATTR_S *pstRcAttr,
                                          VENC_RC_ATTR_S *pstRcChnAttr)
{
    CHECK_MAPI_VENC_NULL_PTR(pstRcAttr);
    CHECK_MAPI_VENC_NULL_PTR(pstRcChnAttr);

    switch (pstRcAttr->enRcMode) {
        case HI_MAPI_VENC_RC_MODE_CBR: {
            pstRcChnAttr->enRcMode = VENC_RC_MODE_MJPEGCBR;
            pstRcChnAttr->stMjpegCbr.u32BitRate = pstRcAttr->stAttrCbr.u32BitRate;
            pstRcChnAttr->stMjpegCbr.u32SrcFrameRate = pstRcAttr->stAttrCbr.u32SrcFrameRate;
            pstRcChnAttr->stMjpegCbr.fr32DstFrameRate = pstRcAttr->stAttrCbr.fr32DstFrameRate;
            pstRcChnAttr->stMjpegCbr.u32StatTime = pstRcAttr->stAttrCbr.u32StatTime;
            break;
        }

        case HI_MAPI_VENC_RC_MODE_VBR: {
            pstRcChnAttr->enRcMode = VENC_RC_MODE_MJPEGVBR;
            pstRcChnAttr->stMjpegVbr.u32MaxBitRate = pstRcAttr->stAttrVbr.u32MaxBitRate;
            pstRcChnAttr->stMjpegVbr.u32SrcFrameRate = pstRcAttr->stAttrVbr.u32SrcFrameRate;
            pstRcChnAttr->stMjpegVbr.fr32DstFrameRate = pstRcAttr->stAttrVbr.fr32DstFrameRate;
            pstRcChnAttr->stMjpegVbr.u32StatTime = pstRcAttr->stAttrVbr.u32StatTime;
            break;
        }

        default:
        {
            pstRcChnAttr->enRcMode = VENC_RC_MODE_MJPEGCBR;
            pstRcChnAttr->stMjpegCbr.u32BitRate = pstRcAttr->stAttrCbr.u32BitRate;
            pstRcChnAttr->stMjpegCbr.u32SrcFrameRate = pstRcAttr->stAttrCbr.u32SrcFrameRate;
            pstRcChnAttr->stMjpegCbr.fr32DstFrameRate = pstRcAttr->stAttrCbr.fr32DstFrameRate;
            pstRcChnAttr->stMjpegCbr.u32StatTime = pstRcAttr->stAttrCbr.u32StatTime;
            break;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_ConfigAttr(const HI_MAPI_VENC_ATTR_S *pstVencAttr, VENC_CHN_ATTR_S *pstVencChnAttr)
{
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_MAPI_VENC_NULL_PTR(pstVencAttr);
    CHECK_MAPI_VENC_NULL_PTR(pstVencChnAttr);

    pstVencChnAttr->stVencAttr.bByFrame = HI_TRUE;
    pstVencChnAttr->stVencAttr.u32BufSize = ALIGN_LENGTH(pstVencAttr->stVencPloadTypeAttr.u32BufSize, 64);
    pstVencChnAttr->stVencAttr.u32MaxPicHeight = pstVencAttr->stVencPloadTypeAttr.u32Height;
    pstVencChnAttr->stVencAttr.u32MaxPicWidth = pstVencAttr->stVencPloadTypeAttr.u32Width;
    pstVencChnAttr->stVencAttr.u32PicHeight = pstVencAttr->stVencPloadTypeAttr.u32Height;
    pstVencChnAttr->stVencAttr.u32PicWidth = pstVencAttr->stVencPloadTypeAttr.u32Width;
    pstVencChnAttr->stVencAttr.u32Profile = pstVencAttr->stVencPloadTypeAttr.u32Profile;

    switch (pstVencAttr->stVencPloadTypeAttr.enType) {
        case HI_MAPI_PAYLOAD_TYPE_H264: {
            pstVencChnAttr->stVencAttr.enType = PT_H264;
            s32Ret = HAL_MAPI_VENC_ConfigH264Attr(&pstVencAttr->stRcAttr, pstVencChnAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_ConfigH264RcAttr error,s32Ret:%x\n", s32Ret);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_PAYLOAD_TYPE_H265: {
            pstVencChnAttr->stVencAttr.enType = PT_H265;
            s32Ret = HAL_MAPI_VENC_ConfigH265Attr(&pstVencAttr->stRcAttr, pstVencChnAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_ConfigH265RcAttr error,s32Ret:%x\n", s32Ret);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_PAYLOAD_TYPE_JPEG: {
            pstVencChnAttr->stVencAttr.enType = PT_JPEG;
            pstVencChnAttr->stVencAttr.stAttrJpege.bSupportDCF =
                pstVencAttr->stVencPloadTypeAttr.stAttrJpege.bEnableDCF;
            pstVencChnAttr->stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum =
                pstVencAttr->stVencPloadTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum;

            for (i = 0; i < pstVencAttr->stVencPloadTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum; i++) {
                pstVencChnAttr->stVencAttr.stAttrJpege.stMPFCfg.astLargeThumbNailSize[i].u32Height =
                    pstVencAttr->stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[i].u32Height;
                pstVencChnAttr->stVencAttr.stAttrJpege.stMPFCfg.astLargeThumbNailSize[i].u32Width =
                    pstVencAttr->stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[i].u32Width;
            }
            break;
        }

        case HI_MAPI_PAYLOAD_TYPE_MJPEG: {
            pstVencChnAttr->stVencAttr.enType = PT_MJPEG;
            s32Ret = MAPI_VENC_ConfigMjpegRcAttr(&pstVencAttr->stRcAttr, &pstVencChnAttr->stRcAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_ConfigMjpegRcAttr error,s32Ret:%x\n", s32Ret);
                return s32Ret;
            }
            break;
        }

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC not support payload type,enType:%d\n",
                           pstVencAttr->stVencPloadTypeAttr.enType);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VENC_Init(const HI_MAPI_VENC_PARAM_MOD_S *pstVencModPara)
{
    HI_U32 i;
    HI_S32 s32Ret;
    VENC_PARAM_MOD_S stVencModPara;

    if (g_stVencContext.bVencInited == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "VENC has been pre inited\n");
        return HI_SUCCESS;
    }

    memset(&stVencModPara, 0, sizeof(VENC_PARAM_MOD_S));
    memset(&g_stVencContext, 0, sizeof(MAPI_VENC_CONTEXT_S));

    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        /* Initialize the channel lock */
        MUTEX_INIT_LOCK(g_stVencContext.astVencChn[i].m_VencChn_lock);

        MUTEX_LOCK(g_stVencContext.astVencChn[i].m_VencChn_lock);
        g_stVencContext.astVencChn[i].s32VcapPipe = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32PipeChn = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VprocGrp = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VprocChn = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32StartFrameCount = HI_MAPI_VENC_LIMITLESS_FRAME_COUNT;
        g_stVencContext.astVencChn[i].VencHandle = i;
        MUTEX_UNLOCK(g_stVencContext.astVencChn[i].m_VencChn_lock);
    }

    MAPI_VENC_OSInit();

    stVencModPara.enVencModType = MODTYPE_H264E;
    s32Ret = HI_MPI_VENC_GetModParam(&stVencModPara);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC mod para init fail\n");
        return HI_MAPI_VENC_EILLEGAL_PARAM;
    }

    stVencModPara.stH264eModParam.u32OneStreamBuffer = 1;
    stVencModPara.stH264eModParam.u32H264ePowerSaveEn = pstVencModPara->u32H264eLowPowerMode;
    s32Ret = HI_MPI_VENC_SetModParam(&stVencModPara);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC mod para init fail\n");
        return HI_MAPI_VENC_EILLEGAL_PARAM;
    }

    memset(&stVencModPara, 0, sizeof(VENC_PARAM_MOD_S));
    stVencModPara.enVencModType = MODTYPE_H265E;
    s32Ret = HI_MPI_VENC_GetModParam(&stVencModPara);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC mod para init fail\n");
        return HI_MAPI_VENC_EILLEGAL_PARAM;
    }

    stVencModPara.stH265eModParam.u32OneStreamBuffer = 1;
    stVencModPara.stH265eModParam.u32H265ePowerSaveEn = pstVencModPara->u32H265eLowPowerMode;
    s32Ret = HI_MPI_VENC_SetModParam(&stVencModPara);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC mod para init fail\n");
        return HI_MAPI_VENC_EILLEGAL_PARAM;
    }

    g_stVencContext.bVencInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 MAPI_VENC_Deinit(HI_VOID)
{
    HI_U32 i;
    HI_S32 s32Ret;

    if (g_stVencContext.bVencInited != HI_TRUE) {
        return HI_SUCCESS;
    }

    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        if (g_stVencContext.astVencChn[i].bInited == HI_TRUE) {
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "venc is not deinit,channel:%d\n", i);
            s32Ret = HI_MAPI_VENC_Deinit(i);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                               "venc deinit handle:%d failed! s32Ret:%x\n", i, s32Ret);
            }
        }

    }

    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        MUTEX_LOCK(g_stVencContext.astVencChn[i].m_VencChn_lock);
        g_stVencContext.astVencChn[i].bStarted = HI_FALSE;
        g_stVencContext.astVencChn[i].bInited = HI_FALSE;
        g_stVencContext.astVencChn[i].bFirstFrame = HI_FALSE;
        g_stVencContext.astVencChn[i].VencHandle = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VencFd = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VcapPipe = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32PipeChn = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VprocGrp = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].s32VprocChn = HI_INVALID_HANDLE;
        g_stVencContext.astVencChn[i].bStitched = HI_FALSE;
        g_stVencContext.astVencChn[i].s32StartFrameCount = HI_MAPI_VENC_LIMITLESS_FRAME_COUNT;
        memset(g_stVencContext.astVencChn[i].astVencRegisterCallbacks, 0,
               sizeof(HI_MAPI_VENC_CALLBACK_S) * HI_MAPI_VENC_CALLBACK_MAX_NUM);
        memset(g_stVencContext.astVencChn[i].abOsdStatus, 0, sizeof(VENC_OSD_STATUS_S) * HI_MAPI_VENC_OSD_MAX_NUM);
        MUTEX_UNLOCK(g_stVencContext.astVencChn[i].m_VencChn_lock);

        /* destroy the channel lock */
        MUTEX_DESTROY(g_stVencContext.astVencChn[i].m_VencChn_lock);
    }

    g_stVencContext.u32DatafifeNodeNum = 0;
    memset(g_stVencContext.astProcVDataThread, 0, sizeof(MAPI_VENC_PROCVDATA_THREAD_S) * HI_MAPI_VENC_MAX_CHN_NUM);

    MAPI_VENC_OSDeInit();

    g_stVencContext.bVencInited = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Init(HI_HANDLE VencHdl, const HI_MAPI_VENC_ATTR_S *pstVencAttr)
{
    HI_S32 s32Ret;
    HI_S32 s32Ret2 = HI_SUCCESS;
    HI_S32 i;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_SCENE_MODE_E enSceneMode;
    VENC_CHN_PARAM_S stVencParamChn;

    memset(&stVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    memset(&stVencParamChn, 0, sizeof(VENC_CHN_PARAM_S));
    stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 2; /* Recommended value from PQ tool debugging */

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);

    if (g_stVencContext.astVencChn[VencHdl].bInited == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been inited\n", VencHdl);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }

    s32Ret = MAPI_VENC_CheckAttr(pstVencAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_CheckAttr error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_ConfigAttr(pstVencAttr, &stVencChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC Config attribute error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencHdl, &stVencChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_CreateChn error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    if (pstVencAttr->stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H264 ||
        pstVencAttr->stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H265) {
        enSceneMode = (VENC_SCENE_MODE_E)VENC_SCENEMODE_TO_MPI(pstVencAttr->stVencPloadTypeAttr.enSceneMode);
        s32Ret = HI_MPI_VENC_SetSceneMode(VencHdl, enSceneMode);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetSceneMode error s32Ret:%x\n", s32Ret);
            s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

            if (s32Ret2 != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
            }

            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

            return s32Ret2;
        }

        s32Ret = HAL_MAPI_VENC_ConfigH265CarTrans(VencHdl, pstVencAttr->stVencPloadTypeAttr.enType,
                                                  pstVencAttr->stVencPloadTypeAttr.enSceneMode);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetSceneMode error s32Ret:%x\n", s32Ret);
            s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

            if (s32Ret2 != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
            }

            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

            return s32Ret2;
        }
    } else if (pstVencAttr->stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_JPEG) {
        VENC_JPEG_PARAM_S stVencParamJpeg;
        VENC_JPEG_ENCODE_MODE_E enJpegSnapMode;
        s32Ret = HI_MPI_VENC_GetJpegParam(VencHdl, &stVencParamJpeg);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetJpegParam error,s32Ret:%x\n", s32Ret);
            s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

            if (s32Ret2 != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
            }

            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

            return s32Ret;
        }

        stVencParamJpeg.u32Qfactor = pstVencAttr->stVencPloadTypeAttr.stAttrJpege.u32Qfactor;

        s32Ret = HI_MPI_VENC_SetJpegParam(VencHdl, &stVencParamJpeg);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetJpegParam error,s32Ret:%x\n", s32Ret);
            s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

            if (s32Ret2 != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
            }

            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

            return s32Ret;
        }

        enJpegSnapMode = pstVencAttr->stVencPloadTypeAttr.stAttrJpege.enJpegEncodeMode;
        s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, enJpegSnapMode);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetJpegEncodeMode error,s32Ret:%x\n", s32Ret);
            s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

            if (s32Ret2 != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
            }

            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

            return s32Ret;
        }
    }

    s32Ret = HI_MPI_VENC_GetChnParam(VencHdl, &stVencParamChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnParam error,s32Ret:%x\n", s32Ret);
        s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
        }

        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

        return s32Ret;
    }

    stVencParamChn.u32MaxStrmCnt = MAX_STREAM_CNT;
    s32Ret = HI_MPI_VENC_SetChnParam(VencHdl, &stVencParamChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetChnParam error,s32Ret:%x\n", s32Ret);
        s32Ret2 = HI_MPI_VENC_DestroyChn(VencHdl);

        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret2:%x\n", s32Ret2);
        }

        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

        return s32Ret;
    }

    s32Ret = MAPI_VENC_InitVDataThread(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_InitVDataThread error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    memcpy(&g_stVencContext.astVencChn[VencHdl].stVencAttr, pstVencAttr, sizeof(HI_MAPI_VENC_ATTR_S));

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = HI_NULL;
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = HI_NULL;
    }

    g_stVencContext.astVencChn[VencHdl].bInited = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Deinit(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);

    if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE) {
        s32Ret = HI_MAPI_VENC_Stop(VencHdl);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "venc stop failed,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
            return s32Ret;
        }
    }

    if (g_stVencContext.astVencChn[VencHdl].bInited != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been deinited\n", VencHdl);
        return HI_SUCCESS;
    }

    if (g_stVencContext.astVencChn[VencHdl].s32VprocGrp != HI_INVALID_HANDLE
        && g_stVencContext.astVencChn[VencHdl].s32VprocChn != HI_INVALID_HANDLE) {
        s32Ret = HI_MAPI_VENC_UnBindVProc(g_stVencContext.astVencChn[VencHdl].s32VprocGrp,
                                          g_stVencContext.astVencChn[VencHdl].s32VprocChn,
                                          VencHdl, g_stVencContext.astVencChn[VencHdl].bStitched);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC UnBind VProc error,s32Ret:%x\n", s32Ret);
        }
    } else if (g_stVencContext.astVencChn[VencHdl].s32VcapPipe != HI_INVALID_HANDLE
               && g_stVencContext.astVencChn[VencHdl].s32PipeChn != HI_INVALID_HANDLE) {
        s32Ret = HI_MAPI_VENC_UnBindVCap(g_stVencContext.astVencChn[VencHdl].s32VcapPipe,
                                         g_stVencContext.astVencChn[VencHdl].s32PipeChn, VencHdl);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC UnBind VCap error,s32Ret:%x\n", s32Ret);
        }
    }

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    s32Ret = HI_MPI_VENC_DestroyChn(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_DestroyChn error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_DeinitVDataThread(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_DeinitVDataThread error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    memset(&g_stVencContext.astVencChn[VencHdl].stVencAttr, 0, sizeof(HI_MAPI_VENC_ATTR_S));

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = HI_NULL;
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = HI_NULL;
    }

    g_stVencContext.astVencChn[VencHdl].bInited = HI_FALSE;

    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_RegisterCallback(HI_HANDLE VencHdl, HI_MAPI_VENC_CALLBACK_S *pstVencCB)
{
    HI_U32 i;
    HI_S32 s32RecordIdx = -1;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencCB);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        if (g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB == HI_NULL) {
            s32RecordIdx = i;
        }

        if (pstVencCB->pfnDataCB == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB &&
            pstVencCB->pPrivateData == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData) {
            MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = pstVencCB->pfnDataCB;
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = pstVencCB->pPrivateData;
            MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_SUCCESS;
        }
    }

    if (s32RecordIdx != -1) {
        MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[s32RecordIdx].pfnDataCB = pstVencCB->pfnDataCB;
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[s32RecordIdx].pPrivateData =
            pstVencCB->pPrivateData;
        MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
    return HI_MAPI_VENC_ENOT_PERM;
}

HI_S32 HI_MAPI_VENC_UnRegisterCallback(HI_HANDLE VencHdl, HI_MAPI_VENC_CALLBACK_S *pstVencCB)
{
    HI_U32 i;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencCB);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        if (pstVencCB->pfnDataCB == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB &&
            pstVencCB->pPrivateData == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData) {
            MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = HI_NULL;
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = HI_NULL;
            MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_SUCCESS;
        }
    }

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_MAPI_VENC_ENOT_PERM;
}

HI_S32 HI_MAPI_VENC_Start(HI_HANDLE VencHdl, HI_S32 s32FrameCnt)
{
    HI_S32 s32Ret;
    VENC_RECV_PIC_PARAM_S stRecvParam;
    stRecvParam.s32RecvPicNum = 0;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_FRAMECNT_RANGE(s32FrameCnt);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE && s32FrameCnt == HI_MAPI_VENC_LIMITLESS_FRAME_COUNT) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been started\n", VencHdl);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }

    stRecvParam.s32RecvPicNum = s32FrameCnt;
    s32Ret = HI_MPI_VENC_StartRecvFrame(VencHdl, &stRecvParam);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_StartRecvFrame fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].s32StartFrameCount = s32FrameCnt;
    g_stVencContext.astVencChn[VencHdl].bStarted = HI_TRUE;
    g_abLiteBootStarted[VencHdl] = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Stop(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been stopped\n", VencHdl);
        MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }
    g_stVencContext.astVencChn[VencHdl].bStarted = HI_FALSE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    s32Ret = HI_MPI_VENC_StopRecvFrame(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_VENC_StopRecvFrame fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_ResetChn(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_VENC_ResetChn fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_BindVProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl, HI_BOOL bStitch)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Ret2 = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stSrcTempChn;
    MPP_CHN_S stDestChn;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    if (bStitch == HI_TRUE) {
        CHECK_MAPI_STITCH_HANDLE(VProcHdl);
        CHECK_MAPI_STITCHPORT_HANDLE(VPortHdl);
    } else {
        CHECK_MAPI_VPSS_HANDLE(VProcHdl);
        CHECK_MAPI_VPORT_HANDLE(VPortHdl);
    }

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    memset(&stSrcTempChn, HI_INVALID_HANDLE, sizeof(MPP_CHN_S));

    if (bStitch == HI_TRUE) {
        stSrcChn.enModId = HI_ID_AVS;
    } else {
        stSrcChn.enModId = HI_ID_VPSS;
    }

    stSrcChn.s32DevId = VProcHdl;
    stSrcChn.s32ChnId = VPortHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    if ((HI_HANDLE)g_stVencContext.astVencChn[VencHdl].s32VprocGrp == VProcHdl
        && (HI_HANDLE) g_stVencContext.astVencChn[VencHdl].s32VprocChn == VPortHdl
        && g_stVencContext.astVencChn[VencHdl].bStitched == bStitch) {
        s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcTempChn);

        if (s32Ret != HI_SUCCESS) {
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "HI_MPI_SYS_GetBindbyDest fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        }

        if ((HI_HANDLE)stSrcTempChn.s32DevId == VProcHdl && (HI_HANDLE)stSrcTempChn.s32ChnId == VPortHdl) {
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "VENC have binded return success\n");
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_SUCCESS;
        }
    }

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_SYS_Bind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].s32VprocGrp = VProcHdl;
    g_stVencContext.astVencChn[VencHdl].s32VprocChn = VPortHdl;
    g_stVencContext.astVencChn[VencHdl].bStitched = bStitch;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_UnBindVProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl,
    HI_BOOL bStitch)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    if (bStitch == HI_TRUE) {
        CHECK_MAPI_STITCH_HANDLE(VProcHdl);
        CHECK_MAPI_STITCHPORT_HANDLE(VPortHdl);
    } else {
        CHECK_MAPI_VPSS_HANDLE(VProcHdl);
        CHECK_MAPI_VPORT_HANDLE(VPortHdl);
    }

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    if (bStitch == HI_TRUE) {
        stSrcChn.enModId = HI_ID_AVS;
    } else {
        stSrcChn.enModId = HI_ID_VPSS;
    }

    stSrcChn.s32DevId = VProcHdl;
    stSrcChn.s32ChnId = VPortHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    /* Unbind input source */
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_SYS_UnBind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].s32VprocGrp = HI_INVALID_HANDLE;
    g_stVencContext.astVencChn[VencHdl].s32VprocChn = HI_INVALID_HANDLE;
    g_stVencContext.astVencChn[VencHdl].bStitched = HI_FALSE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_BindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Ret2 = HI_SUCCESS;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VCAPPIPE_HANDLE(VcapPipeHdl);
    CHECK_MAPI_PIPECHN_HANDLE(PipeChnHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);
    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stSrcTempChn;
    MPP_CHN_S stDestChn;

    memset(&stSrcTempChn, HI_INVALID_HANDLE, sizeof(MPP_CHN_S));

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = VcapPipeHdl;
    stSrcChn.s32ChnId = PipeChnHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    if ((HI_HANDLE)g_stVencContext.astVencChn[VencHdl].s32VcapPipe == VcapPipeHdl
        && (HI_HANDLE) g_stVencContext.astVencChn[VencHdl].s32PipeChn == PipeChnHdl) {
        s32Ret = HI_MPI_SYS_GetBindbyDest(&stDestChn, &stSrcTempChn);

        if (s32Ret != HI_SUCCESS) {
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "HI_MPI_SYS_GetBindbyDest fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        }

        if ((HI_HANDLE)stSrcTempChn.s32DevId == VcapPipeHdl
            && (HI_HANDLE) stSrcTempChn.s32ChnId == PipeChnHdl
            && stSrcTempChn.enModId == HI_ID_VI) {
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "VENC have binded Vcap return success\n");
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_SUCCESS;
        }
    }

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_SYS_Bind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].s32VcapPipe = VcapPipeHdl;
    g_stVencContext.astVencChn[VencHdl].s32PipeChn = PipeChnHdl;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_UnBindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VCAPPIPE_HANDLE(VcapPipeHdl);
    CHECK_MAPI_PIPECHN_HANDLE(PipeChnHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VI;
    stSrcChn.s32DevId = VcapPipeHdl;
    stSrcChn.s32ChnId = PipeChnHdl;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VencHdl;

    /* Unbind input source */
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_SYS_UnBind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].s32VcapPipe = HI_INVALID_HANDLE;
    g_stVencContext.astVencChn[VencHdl].s32PipeChn = HI_INVALID_HANDLE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetAttr(HI_HANDLE VencHdl, const HI_MAPI_VENC_ATTR_S *pstStreamAttr)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstStreamAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    VENC_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0x0, sizeof(VENC_CHN_ATTR_S));

    s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_VENC_GetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_ConfigAttr(pstStreamAttr, &stChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC Config attribute error,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_SetChnAttr(VencHdl, &stChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_VENC_SetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    if (pstStreamAttr->stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_JPEG) {
        VENC_JPEG_PARAM_S stVencParamJpeg;
        VENC_JPEG_ENCODE_MODE_E enJpegSnapMode;
        s32Ret = HI_MPI_VENC_GetJpegParam(VencHdl, &stVencParamJpeg);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetJpegParam error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }

        stVencParamJpeg.u32Qfactor = pstStreamAttr->stVencPloadTypeAttr.stAttrJpege.u32Qfactor;
        s32Ret = HI_MPI_VENC_SetJpegParam(VencHdl, &stVencParamJpeg);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetJpegParam error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }

        enJpegSnapMode = pstStreamAttr->stVencPloadTypeAttr.stAttrJpege.enJpegEncodeMode;
        s32Ret = HI_MPI_VENC_SetJpegEncodeMode(VencHdl, enJpegSnapMode);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetJpegEncodeMode error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    memcpy(&g_stVencContext.astVencChn[VencHdl].stVencAttr, pstStreamAttr, sizeof(HI_MAPI_VENC_ATTR_S));
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetAttr(HI_HANDLE VencHdl, HI_MAPI_VENC_ATTR_S *pstStreamAttr)
{
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstStreamAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    memcpy(pstStreamAttr, &g_stVencContext.astVencChn[VencHdl].stVencAttr, sizeof(HI_MAPI_VENC_ATTR_S));

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_RequestIDR(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].bStarted != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "VencHdl:%d is not started, request IDR is not permit\n", VencHdl);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_MAPI_VENC_ENOT_PERM;
    }

    s32Ret = HI_MPI_VENC_RequestIDR(VencHdl, HI_TRUE);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                       "HI_MPI_VENC_RequestIDR fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetAttrEx(HI_HANDLE VencHdl, HI_MAPI_VENC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    switch (enCMD) {
        case HI_MAPI_VENC_CMD_H264_RC_ATTR_EX:
        case HI_MAPI_VENC_CMD_H265_RC_ATTR_EX:
        case HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX: {
            if (u32Len < sizeof(VENC_RC_PARAM_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_RC_PARAM_S stRcParam;
            memset(&stRcParam, 0, sizeof(VENC_RC_PARAM_S));

            s32Ret = HI_MPI_VENC_GetRcParam(VencHdl, &stRcParam);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetRcParam fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            memcpy(pAttr, &stRcParam, sizeof(VENC_RC_PARAM_S));

            break;
        }

        case HI_MAPI_VENC_CMD_GOP_MODE: {
            if (u32Len < sizeof(HI_MAPI_VENC_GOP_ATTR_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_CHN_ATTR_S stChnAttr;
            HI_MAPI_VENC_GOP_ATTR_S stVencGopAttr;
            memset(&stChnAttr, 0x0, sizeof(VENC_CHN_ATTR_S));
            memset(&stVencGopAttr, 0x0, sizeof(HI_MAPI_VENC_GOP_ATTR_S));

            s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stChnAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            s32Ret = MAPI_VENC_GetGopAttr(&stChnAttr.stGopAttr, &stVencGopAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC gop get attribute error,s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            memcpy(pAttr, &stVencGopAttr, sizeof(HI_MAPI_VENC_GOP_ATTR_S));

            break;
        }

        case HI_MAPI_VENC_CMD_INTRA_REFRESH: {
            if (u32Len < sizeof(VENC_INTRA_REFRESH_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_INTRA_REFRESH_S stVencParamIntraRefresh;
            memset(&stVencParamIntraRefresh, 0x0, sizeof(VENC_INTRA_REFRESH_S));

            s32Ret = HI_MPI_VENC_GetIntraRefresh(VencHdl, &stVencParamIntraRefresh);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetIntraRefresh fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            memcpy(pAttr, &stVencParamIntraRefresh, sizeof(VENC_INTRA_REFRESH_S));

            break;
        }

        case HI_MAPI_VENC_CMD_FRAME_RATE: {
            if (u32Len < sizeof(VENC_FRAME_RATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_FRAME_RATE_S stVencFrameRate;
            VENC_CHN_PARAM_S stVencParamChn;
            memset(&stVencFrameRate, 0x0, sizeof(VENC_FRAME_RATE_S));
            memset(&stVencParamChn, 0x0, sizeof(VENC_CHN_PARAM_S));

            s32Ret = HI_MPI_VENC_GetChnParam(VencHdl, &stVencParamChn);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnParam fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            stVencFrameRate.s32DstFrmRate = stVencParamChn.stFrameRate.s32DstFrmRate;
            stVencFrameRate.s32SrcFrmRate = stVencParamChn.stFrameRate.s32SrcFrmRate;
            memcpy(pAttr, &stVencFrameRate, sizeof(VENC_FRAME_RATE_S));

            break;
        }

        case HI_MAPI_VENC_CMD_HIERARCHICAL_QP: {
            if (u32Len < sizeof(VENC_HIERARCHICAL_QP_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_HIERARCHICAL_QP_S stVencHierarchicalQp;

            s32Ret = HI_MPI_VENC_GetHierarchicalQp(VencHdl, &stVencHierarchicalQp);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetHierarchicalQp fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            memcpy(pAttr, &stVencHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));

            break;
        }

        case HI_MAPI_VENC_CMD_FRAMELOST_STRATEGY: {
            if (u32Len < sizeof(HI_BOOL)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            if (g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H264
                || g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType ==
                    HI_MAPI_PAYLOAD_TYPE_H265
                || g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType ==
                    HI_MAPI_PAYLOAD_TYPE_MJPEG) {
                // get Frame Lost Strategy
                VENC_FRAMELOST_S stFrmLostParam;
                memset(&stFrmLostParam, 0, sizeof(stFrmLostParam));
                s32Ret = HI_MPI_VENC_GetFrameLostStrategy(VencHdl, &stFrmLostParam);

                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "GetFrameLostStrategy failed:0x%x\n", s32Ret);
                    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                    return s32Ret;
                }

                MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                    "current FrameLostStrategy:\n bFrmLostOpen(%d), u32FrmLostBpsThr(%u),enFrmLostMode(%d), u32EncFrmGaps(%u)\n",
                    stFrmLostParam.bFrmLostOpen, stFrmLostParam.u32FrmLostBpsThr, stFrmLostParam.enFrmLostMode,
                    stFrmLostParam.u32EncFrmGaps);

                memcpy(pAttr, &(stFrmLostParam.bFrmLostOpen), sizeof(HI_BOOL));
            } else {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC payload type(%d) err\n",
                               g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            break;
    }

    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetAttrEx(HI_HANDLE VencHdl, HI_MAPI_VENC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);
    HI_S32 s32Ret = HI_SUCCESS;
    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    switch (enCMD) {
        case HI_MAPI_VENC_CMD_H264_RC_ATTR_EX:
        case HI_MAPI_VENC_CMD_H265_RC_ATTR_EX:
        case HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX: {
            if (u32Len < sizeof(VENC_RC_PARAM_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_RC_PARAM_S *pstVencRcParam = (VENC_RC_PARAM_S *)pAttr;

            s32Ret = HI_MPI_VENC_SetRcParam(VencHdl, pstVencRcParam);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetRcParam fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_VENC_CMD_GOP_MODE: {
            if (u32Len < sizeof(HI_MAPI_VENC_GOP_ATTR_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_CHN_ATTR_S stChnAttr;
            HI_MAPI_VENC_GOP_ATTR_S *pstVencGopAttr = (HI_MAPI_VENC_GOP_ATTR_S *)pAttr;
            memset(&stChnAttr, 0x0, sizeof(VENC_CHN_ATTR_S));

            s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stChnAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            s32Ret = MAPI_VENC_SetGopAttr(pstVencGopAttr, &stChnAttr.stGopAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC gop attribute config error,s32Ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            s32Ret = HI_MPI_VENC_SetChnAttr(VencHdl, &stChnAttr);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_VENC_CMD_INTRA_REFRESH: {
            if (u32Len < sizeof(VENC_INTRA_REFRESH_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_INTRA_REFRESH_S *pstVencParamIntraRefresh = (VENC_INTRA_REFRESH_S *)pAttr;
            s32Ret = HI_MPI_VENC_SetIntraRefresh(VencHdl, pstVencParamIntraRefresh);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetIntraRefresh fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_VENC_CMD_FRAME_RATE: {
            if (u32Len < sizeof(VENC_FRAME_RATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_CHN_PARAM_S stVencParamChn;
            VENC_FRAME_RATE_S *pstVencFrameRate = (VENC_FRAME_RATE_S *)pAttr;
            memset(&stVencParamChn, 0x0, sizeof(VENC_CHN_PARAM_S));

            s32Ret = HI_MPI_VENC_GetChnParam(VencHdl, &stVencParamChn);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnParam fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }

            stVencParamChn.stFrameRate.s32DstFrmRate = pstVencFrameRate->s32DstFrmRate;
            stVencParamChn.stFrameRate.s32SrcFrmRate = pstVencFrameRate->s32SrcFrmRate;
            s32Ret = HI_MPI_VENC_SetChnParam(VencHdl, &stVencParamChn);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetChnParam fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_VENC_CMD_HIERARCHICAL_QP: {
            if (u32Len < sizeof(VENC_HIERARCHICAL_QP_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            VENC_HIERARCHICAL_QP_S *pstVencHierarchicalQp = (VENC_HIERARCHICAL_QP_S *)pAttr;

            s32Ret = HI_MPI_VENC_SetHierarchicalQp(VencHdl, pstVencHierarchicalQp);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetHierarchicalQp fail,VencHdl:%d,s32Ret:%x\n", VencHdl,
                               s32Ret);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return s32Ret;
            }
            break;
        }

        case HI_MAPI_VENC_CMD_FRAMELOST_STRATEGY: {
            if (u32Len < sizeof(HI_BOOL)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC pAttr u32Len:%x is small\n", u32Len);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }

            HI_BOOL *pbFrmLostOpen = (HI_BOOL *)pAttr;

            if (g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H264
                || g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType ==
                    HI_MAPI_PAYLOAD_TYPE_H265
                || g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType ==
                    HI_MAPI_PAYLOAD_TYPE_MJPEG) {
                // get Frame Lost Strategy
                VENC_FRAMELOST_S stFrmLostParam;
                memset(&stFrmLostParam, 0, sizeof(VENC_FRAMELOST_S));
                s32Ret = HI_MPI_VENC_GetFrameLostStrategy(VencHdl, &stFrmLostParam);

                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetFrameLostStrategy failed:0x%x\n", s32Ret);
                    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                    return s32Ret;
                }

                MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                    "default FrameLostStrategy:\n bFrmLostOpen(%d), u32FrmLostBpsThr(%u),enFrmLostMode(%d), u32EncFrmGaps(%u)\n",
                    stFrmLostParam.bFrmLostOpen, stFrmLostParam.u32FrmLostBpsThr, stFrmLostParam.enFrmLostMode,
                    stFrmLostParam.u32EncFrmGaps);

                stFrmLostParam.bFrmLostOpen = *pbFrmLostOpen;
                stFrmLostParam.u32FrmLostBpsThr = VENC_FRAME_LOST_BPS_THRESHOLD;  // bits per second
                stFrmLostParam.enFrmLostMode = FRMLOST_NORMAL;
                stFrmLostParam.u32EncFrmGaps = 0;

                MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                    "update FrameLostStrategy:\n bFrmLostOpen(%d), u32FrmLostBpsThr(%u),enFrmLostMode(%d), u32EncFrmGaps(%u)\n",
                    stFrmLostParam.bFrmLostOpen, stFrmLostParam.u32FrmLostBpsThr, stFrmLostParam.enFrmLostMode,
                    stFrmLostParam.u32EncFrmGaps);

                s32Ret = HI_MPI_VENC_SetFrameLostStrategy(VencHdl, &stFrmLostParam);

                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetFrameLostStrategy failed:0x%x\n", s32Ret);
                    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                    return s32Ret;
                }

            } else {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC payload type(%d) err\n",
                               g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType);
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
                return HI_MAPI_VENC_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "not support yet\n");
            break;
    }

    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetOSDAttr(HI_HANDLE VencHdl, HI_HANDLE OSDHdl, HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stRgnChnAttr;
    BITMAP_S stBitMap;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VENC_MIN_VAL("stBitmapAttr.enPixelFormat", pstOSDAttr->stBitmapAttr.enPixelFormat,
                            PIXEL_FORMAT_ARGB_1555);
    CHECK_MAPI_VENC_MAX_VAL("stBitmapAttr.enPixelFormat", pstOSDAttr->stBitmapAttr.enPixelFormat,
                            PIXEL_FORMAT_ARGB_4444);
    CHECK_MAPI_VENC_NULL_PTR(pstOSDAttr->stBitmapAttr.pData);
    CHECK_MAPI_VENC_MIN_VAL("stBitmapAttr.u32Width", pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_MIN_OSD_BMP_W);
    CHECK_MAPI_VENC_MIN_VAL("stBitmapAttr.u32Height", pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_MIN_OSD_BMP_H);
    CHECK_MAPI_VENC_MAX_VAL("stBitmapAttr.u32Width", pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_MAX_OSD_BMP_W);
    CHECK_MAPI_VENC_MAX_VAL("stBitmapAttr.u32Height", pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_MAX_OSD_BMP_H);
    CHECK_MAPI_VENC_MAX_VAL("stOsdDisplayAttr.u32BgAlpha", pstOSDAttr->stOsdDisplayAttr.u32BgAlpha,
                            HI_MAPI_MAX_OSD_ALPHA);
    CHECK_MAPI_VENC_MAX_VAL("stOsdDisplayAttr.u32FgAlpha", pstOSDAttr->stOsdDisplayAttr.u32FgAlpha,
                            HI_MAPI_MAX_OSD_ALPHA);
    CHECK_MAPI_VENC_MIN_VAL("stOsdDisplayAttr.s32RegionX", pstOSDAttr->stOsdDisplayAttr.s32RegionX,
                            HI_MAPI_MIN_OSD_REGION_X);
    CHECK_MAPI_VENC_MIN_VAL("stOsdDisplayAttr.s32RegionY", pstOSDAttr->stOsdDisplayAttr.s32RegionY,
                            HI_MAPI_MIN_OSD_REGION_Y);
    CHECK_MAPI_VENC_MAX_VAL("stOsdDisplayAttr.s32RegionX", pstOSDAttr->stOsdDisplayAttr.s32RegionX,
                            HI_MAPI_MAX_OSD_REGION_X);
    CHECK_MAPI_VENC_MAX_VAL("stOsdDisplayAttr.s32RegionY", pstOSDAttr->stOsdDisplayAttr.s32RegionY,
                            HI_MAPI_MAX_OSD_REGION_Y);
    CHECK_MAPI_VENC_MIN_VAL("u32CanvasNum", pstOSDAttr->u32CanvasNum, HI_MAPI_RGN_BUF_MIN_NUM);
    CHECK_MAPI_VENC_MAX_VAL("u32CanvasNum", pstOSDAttr->u32CanvasNum, HI_MAPI_RGN_BUF_MAX_NUM);
    CHECK_MEDIA_VENC_ALIGN(pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_OSD_BMP_W_ALIGN);
    CHECK_MEDIA_VENC_ALIGN(pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_OSD_BMP_H_ALIGN);
    CHECK_MEDIA_VENC_ALIGN(pstOSDAttr->stOsdDisplayAttr.s32RegionX, HI_MAPI_OSD_REGION_X_ALIGN);
    CHECK_MEDIA_VENC_ALIGN(pstOSDAttr->stOsdDisplayAttr.s32RegionY, HI_MAPI_OSD_REGION_Y_ALIGN);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    /* If the area of type OVERLAY is used for H265 or H264 protocol, the attribute is configured as ATTACH_JPEG_MAIN */
    HI_MAPI_PAYLOAD_TYPE_E enType = g_stVencContext.astVencChn[VencHdl].stVencAttr.stVencPloadTypeAttr.enType;

    if (enType == HI_MAPI_PAYLOAD_TYPE_H264 || enType == HI_MAPI_PAYLOAD_TYPE_H265) {
        if (pstOSDAttr->stOsdDisplayAttr.enAttachDest != ATTACH_JPEG_MAIN) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "enAttachDest error! h.264 or h.265 must be ATTACH_JPEG_MAIN\n");
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_MAPI_VENC_EILLEGAL_PARAM;
        }
    }

    /* if OSD  has been started, then set the dynamic attributes of the OSD and updates the picture */
    if (g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdStart == HI_TRUE) {
        RgnHdl = MAPI_GetRgnHdl(0, VencHdl, OSDHdl, HI_ID_VENC);
        stChn.enModId = HI_ID_VENC;
        stChn.s32DevId = 0;
        stChn.s32ChnId = VencHdl;

        memset(&stRgnChnAttr, 0x0, sizeof(RGN_CHN_ATTR_S));

        s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_GetDisplayAttr fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }

        stRgnChnAttr.bShow = pstOSDAttr->stOsdDisplayAttr.bShow;

        if (stRgnChnAttr.enType == OVERLAY_RGN) {
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstOSDAttr->stOsdDisplayAttr.s32RegionX;
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstOSDAttr->stOsdDisplayAttr.s32RegionY;
            stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = pstOSDAttr->stOsdDisplayAttr.u32BgAlpha;
            stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = pstOSDAttr->stOsdDisplayAttr.u32FgAlpha;
            stRgnChnAttr.unChnAttr.stOverlayChn.enAttachDest = pstOSDAttr->stOsdDisplayAttr.enAttachDest;
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "wrong rgn type\n");
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return HI_MAPI_VENC_EILLEGAL_PARAM;
        }

        s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_SetDisplayAttr fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }

        stBitMap.enPixelFormat = pstOSDAttr->stBitmapAttr.enPixelFormat;
        stBitMap.u32Width = pstOSDAttr->stBitmapAttr.u32Width;
        stBitMap.u32Height = pstOSDAttr->stBitmapAttr.u32Height;
        stBitMap.pData = pstOSDAttr->stBitmapAttr.pData;

        s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &stBitMap);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_SetBitMap fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
            return s32Ret;
        }
    }

    /* save osd attr */
    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.enPixelFormat =
        pstOSDAttr->stBitmapAttr.enPixelFormat;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.pData = pstOSDAttr->stBitmapAttr.pData;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Width = pstOSDAttr->stBitmapAttr.u32Width;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Height = pstOSDAttr->stBitmapAttr.u32Height;

    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.bShow = pstOSDAttr->stOsdDisplayAttr.bShow;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionX =
        pstOSDAttr->stOsdDisplayAttr.s32RegionX;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionY =
        pstOSDAttr->stOsdDisplayAttr.s32RegionY;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32BgAlpha =
        pstOSDAttr->stOsdDisplayAttr.u32BgAlpha;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32FgAlpha =
        pstOSDAttr->stOsdDisplayAttr.u32FgAlpha;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32Color =
        pstOSDAttr->stOsdDisplayAttr.u32Color;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.enAttachDest =
        pstOSDAttr->stOsdDisplayAttr.enAttachDest;
    g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].u32CanvasNum = pstOSDAttr->u32CanvasNum;
    g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdAttrSetted = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetOSDAttr(HI_HANDLE VencHdl, HI_HANDLE OSDHdl, HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstOSDAttr);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdAttrSetted == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "osd not been set yet\n");
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_MAPI_VENC_ENOT_PERM;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    pstOSDAttr->stBitmapAttr.enPixelFormat =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.enPixelFormat;
    pstOSDAttr->stBitmapAttr.pData = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.pData;
    pstOSDAttr->stBitmapAttr.u32Width = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Width;
    pstOSDAttr->stBitmapAttr.u32Height = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Height;

    pstOSDAttr->stOsdDisplayAttr.bShow = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.bShow;
    pstOSDAttr->stOsdDisplayAttr.s32RegionX =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionX;
    pstOSDAttr->stOsdDisplayAttr.s32RegionY =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionY;
    pstOSDAttr->stOsdDisplayAttr.u32BgAlpha =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32BgAlpha;
    pstOSDAttr->stOsdDisplayAttr.u32FgAlpha =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32FgAlpha;
    pstOSDAttr->stOsdDisplayAttr.u32Color =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32Color;
    pstOSDAttr->stOsdDisplayAttr.enAttachDest =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.enAttachDest;
    pstOSDAttr->u32CanvasNum = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].u32CanvasNum;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_StartOSD(HI_HANDLE VencHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stRgnChnAttr;
    BITMAP_S stBitMap;
    RGN_ATTR_S stRgnAttr;
    VENC_CHN_ATTR_S stChnAttr;

    memset(&stRgnChnAttr, 0x0, sizeof(RGN_CHN_ATTR_S));
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdAttrSetted == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "osd not been set yet\n");
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_MAPI_VENC_ENOT_PERM;
    }

    if (g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdStart == HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "osd already been started\n");
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnAttr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = MAPI_AddRgnHdl(0, VencHdl, OSDHdl, HI_ID_VENC);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_AddRgnHdl fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    RgnHdl = MAPI_GetRgnHdl(0, VencHdl, OSDHdl, HI_ID_VENC);

    stRgnAttr.enType = OVERLAY_RGN;
    stRgnAttr.unAttr.stOverlay.enPixelFmt =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.enPixelFormat;
    stRgnAttr.unAttr.stOverlay.stSize.u32Width =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Width;
    stRgnAttr.unAttr.stOverlay.stSize.u32Height =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Height;
    /* written a default value,wait for determin processing method */
    stRgnAttr.unAttr.stOverlay.u32BgColor =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32Color;

    /* When encoding B-frames, the u32CanvasNum value is the number of B-frames plus 2 */
    stRgnAttr.unAttr.stOverlay.u32CanvasNum = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].u32CanvasNum;

    s32Ret = HI_MPI_RGN_Create(RgnHdl, &stRgnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_Create fail,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    stBitMap.enPixelFormat = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.enPixelFormat;
    stBitMap.u32Width = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Width;
    stBitMap.u32Height = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.u32Height;
    stBitMap.pData = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stBitmapAttr.pData;

    s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &stBitMap);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_SetBitMap fail,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    stChn.enModId = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VencHdl;
    stRgnChnAttr.bShow = g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.bShow;
    stRgnChnAttr.enType = OVERLAY_RGN;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionX;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.s32RegionY;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32BgAlpha =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32BgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32FgAlpha =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.u32FgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayChn.enAttachDest =
        g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl].stOsdDisplayAttr.enAttachDest;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = OSDHdl;
    s32Ret = HI_MPI_RGN_AttachToChn(RgnHdl, &stChn, &stRgnChnAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_AttachToChn fail,s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdStart = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_StopOSD(HI_HANDLE VencHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 RgnHdl;
    MPP_CHN_S stChn;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);

    MUTEX_LOCK(g_Venc_FuncLock[VencHdl]);

    CHECK_MAPI_VENC_CHECK_PREINIT_UNLOCK(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT_UNLOCK(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdStart == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "osd already been stoped\n");
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return HI_SUCCESS;
    }

    RgnHdl = MAPI_GetRgnHdl(0, VencHdl, OSDHdl, HI_ID_VENC);

    s32Ret = MAPI_DelRgnHdl(0, VencHdl, OSDHdl, HI_ID_VENC);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_DelRgnHdl fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    stChn.enModId = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VencHdl;

    s32Ret = HI_MPI_RGN_DetachFromChn(RgnHdl, &stChn);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_DetachFromChn fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_RGN_Destroy(RgnHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_RGN_Destroy fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdStart = HI_FALSE;
    g_stVencContext.astVencChn[VencHdl].abOsdStatus[OSDHdl].bOsdAttrSetted = HI_FALSE;
    memset(&g_stVencContext.astVencChn[VencHdl].astOsdAttr[OSDHdl], 0x0, sizeof(HI_MAPI_OSD_ATTR_S));
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    MUTEX_UNLOCK(g_Venc_FuncLock[VencHdl]);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
