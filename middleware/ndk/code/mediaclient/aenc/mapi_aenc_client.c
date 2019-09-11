/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_aenc_client.c
 * @brief   NDK aenc client arch functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

#include "hi_math.h"
#include "hi_mapi_comm_define.h"
#include "hi_ipcmsg.h"
#include "msg_aenc.h"
#include "msg_define.h"
#include "hi_datafifo.h"
#include "mapi_aenc_inner.h"
#include "mapi_acap_inner.h"
#include "hi_mapi_aenc_define.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define AENC_MEM_DEV "/dev/mmz_userdev"

static MAPI_AENC_CONTEXT_S g_stAencContext;
static HI_S32 g_s32MapFd = -1;
#define SEC2NSEC_SCALE (1 * 1000 * 1000 * 1000)
static pthread_mutex_t g_aencFuncLock[HI_MAPI_AENC_CHN_MAX_NUM] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
};

static HI_S32 MAPI_AENC_ProcAStream(HI_HANDLE AencHdl, AUDIO_STREAM_S *pstAencData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ProcAStream start\n");

    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        if (g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB != HI_NULL) {
            s32Ret = g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB(AencHdl, pstAencData,
                g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "astAencRegisterCallBacks[%d].pfnDataCB  Fail, s32Ret:%x\n", i,
                               s32Ret);
            }
        }
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ProcAStream complete\n");
    return HI_SUCCESS;
}

static HI_S32 MAPI_AENC_GetAData(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AUDIO_STREAM_S stOutAStream;
    stOutAStream.u64PhyAddr = 0;
    MAPI_AENC_HISILINK_STREAM_S *pstStream;

    while (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STARTED &&
           g_stAencContext.astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_OPENED) {
        s32Ret = HI_DATAFIFO_Read(g_stAencContext.astADatafifo[AencHdl].DatafifoHdl, (HI_VOID **)&pstStream);
        if (s32Ret != HI_SUCCESS) {
            if (s32Ret != HI_DATAFIFO_ERR_NO_DATA) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "s32AudioChn :%d read audio data Failed\n", AencHdl);
            }
            break;
        }

        memcpy(&stOutAStream, &pstStream->stAencData, sizeof(AUDIO_STREAM_S));
        stOutAStream.pStream = (HI_U8 *)g_stAencContext.astADatafifo[AencHdl].pDataVirtAddr +
                               (stOutAStream.u64PhyAddr - g_stAencContext.astADatafifo[AencHdl].u64StreamBufPhyAddr);

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "+++++get s32AudioChn:%d addr:%llx, len:%d, seq:%d, pts:%llx\n", AencHdl,
                         stOutAStream.u64PhyAddr, stOutAStream.u32Len, stOutAStream.u32Seq, stOutAStream.u64TimeStamp);

        s32Ret = MAPI_AENC_ProcAStream(AencHdl, &stOutAStream);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "getting audio stream err,s32Ret:%x\n", s32Ret);
        }
        s32Ret = HI_DATAFIFO_CMD(g_stAencContext.astADatafifo[AencHdl].DatafifoHdl, DATAFIFO_CMD_READ_DONE, pstStream);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "datafifo read down error,s32Ret:%x\n", s32Ret);
        }

        /* save the last read frame */
        g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum++;
        g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = pstStream->stAencData.u32Seq;

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Read down\n");
    }
    return HI_SUCCESS;
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
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE hAencChn = *(HI_HANDLE *)arg;
    HI_U32 readLen = 0;

    prctl(PR_SET_NAME, (unsigned long)"ProcAStreamThread", 0, 0, 0);
    MUTEX_LOCK(g_stAencContext.astProcADataThread[hAencChn].m_ProcADataThd_lock);
    while (g_stAencContext.astProcADataThread[hAencChn].bProcADataThdStartFlg == HI_TRUE) {
        readLen = 0;
        MUTEX_LOCK(g_stAencContext.astAencChn[hAencChn].m_AencChn_lock);
        /* chn is not start or datafifo is not open */
        if (g_stAencContext.astAencChn[hAencChn].enAencChnState != MAPI_AENC_CHN_STARTED ||
            g_stAencContext.astADatafifo[hAencChn].enADatafifoState != MAPI_AENC_DATAFIFO_OPENED) {
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "chn or datafifo [%d] no start or open\n", hAencChn);
            MUTEX_UNLOCK(g_stAencContext.astAencChn[hAencChn].m_AencChn_lock);
            MAPI_AENC_Cond_TimeWait(&g_stAencContext.astProcADataThread[hAencChn].c_ProcADataThd_cond,
                                    &g_stAencContext.astProcADataThread[hAencChn].m_ProcADataThd_lock,
                                    AENCODE_GET_FRAME_THREAD_USLEEP_TIME);
            continue;
        }

        HI_DATAFIFO_CMD(g_stAencContext.astADatafifo[hAencChn].DatafifoHdl, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);

        /* no data to read */
        if (readLen == 0) {
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "ProcAStreamThread [%d] no data to read\n", hAencChn);
            MUTEX_UNLOCK(g_stAencContext.astAencChn[hAencChn].m_AencChn_lock);
            MAPI_AENC_Cond_TimeWait(&g_stAencContext.astProcADataThread[hAencChn].c_ProcADataThd_cond,
                                    &g_stAencContext.astProcADataThread[hAencChn].m_ProcADataThd_lock,
                                    AENCODE_GET_FRAME_THREAD_USLEEP_TIME);
            continue;
        }

        /* have data to read */
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "ProcAStreamThread [%d] have data to read\n", hAencChn);
        s32Ret = MAPI_AENC_GetAData(g_stAencContext.astAencChn[hAencChn].AencChnHdl);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_GetAData fail,s32Ret:%x\n", s32Ret);
        }

        MUTEX_UNLOCK(g_stAencContext.astAencChn[hAencChn].m_AencChn_lock);
    }
    MUTEX_UNLOCK(g_stAencContext.astProcADataThread[hAencChn].m_ProcADataThd_lock);
    return HI_NULL;
}

static HI_S32 MAPI_AENC_OpenDatafifoReader(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;
    HI_U64 u64StreamBufPhyAddr;
    HI_U32 u32StreamBufLen;
    HI_VOID *pDataVirtAddr;

    /* check datafifo handle and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    if (g_stAencContext.astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_OPENED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "datafifo is already opened , return success ! \n");
        return HI_SUCCESS;
    }

    /* open datafifo and set attr */
    HI_DATAFIFO_PARAMS_S params = { 200, sizeof(MAPI_AENC_HISILINK_STREAM_S), HI_TRUE, DATAFIFO_READER };
    s32Ret = HI_DATAFIFO_OpenByAddr(&g_stAencContext.astADatafifo[AencHdl].DatafifoHdl, &params,
                                    g_stAencContext.astADatafifo[AencHdl].u64DataFifoSharePhyAddr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_OpenDatafifoReader fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }
    u64StreamBufPhyAddr = g_stAencContext.astADatafifo[AencHdl].u64StreamBufPhyAddr;
    u32StreamBufLen = g_stAencContext.astADatafifo[AencHdl].u32StreamBufLen;
    pDataVirtAddr = mmap(HI_NULL, u32StreamBufLen, PROT_READ | PROT_WRITE, MAP_SHARED, g_s32MapFd,
                         u64StreamBufPhyAddr);
    if ((HI_VOID *)-1 == pDataVirtAddr) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC,
                       "MAPI_AENC_OpenDatafifoReader mmap pDataVirtAddr error, u64StreamBufPhyAddr:%llx\n",
                       u64StreamBufPhyAddr);
        return HI_MAPI_AENC_EMEMERR;
    }
    g_stAencContext.astADatafifo[AencHdl].pDataVirtAddr = pDataVirtAddr;
    g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = 0;
    g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum = 0;

    /* set datafifo state */
    g_stAencContext.astADatafifo[AencHdl].enADatafifoState = MAPI_AENC_DATAFIFO_OPENED;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_OpenDatafifoReader[%d]\n", AencHdl);
    return HI_SUCCESS;
}

static HI_S32 MAPI_AENC_CloseDatafifoReader(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;

    /* check datafifo handle and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    if (g_stAencContext.astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_CLOSED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "datafifo is already closed , return success ! \n");
        return HI_SUCCESS;
    }

    /* close datafifo and set attr */
    s32Ret = HI_DATAFIFO_Close(g_stAencContext.astADatafifo[AencHdl].DatafifoHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_CloseDatafifoReader fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }
    if (0 != munmap(g_stAencContext.astADatafifo[AencHdl].pDataVirtAddr,
                    g_stAencContext.astADatafifo[AencHdl].u32StreamBufLen)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_CloseDatafifoReader munmap error\n");
        return HI_DATAFIFO_ERR_EINVAL_PAEAMETER;
    }
    g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = 0;
    g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum = 0;

    /* set datafifo state */
    g_stAencContext.astADatafifo[AencHdl].enADatafifoState = MAPI_AENC_DATAFIFO_CLOSED;
    g_stAencContext.astADatafifo[AencHdl].DatafifoHdl = HI_DATAFIFO_INVALID_HANDLE;
    g_stAencContext.astADatafifo[AencHdl].u64DataFifoSharePhyAddr = 0;
    g_stAencContext.astADatafifo[AencHdl].u64StreamBufPhyAddr = 0;
    g_stAencContext.astADatafifo[AencHdl].u32StreamBufLen = 0;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_CloseDatafifoReader [%d]\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 MAPI_AENC_Client_Init()
{
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    /* check state */
    if (g_stAencContext.bAencInited == HI_TRUE) {
        return HI_SUCCESS;
    }

    /* init chn and datafifo attr */
    memset(&g_stAencContext, 0, sizeof(MAPI_AENC_CONTEXT_S));
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        g_stAencContext.astAencChn[i].enAencChnState = MAPI_AENC_CHN_UNINITED;
        g_stAencContext.astAencChn[i].AencChnHdl = i;

        g_stAencContext.astADatafifo[i].enADatafifoState = MAPI_AENC_DATAFIFO_CLOSED;
        g_stAencContext.astADatafifo[i].DatafifoHdl = HI_INVALID_HANDLE;
    }

    /* open mem map */
    if (g_s32MapFd < 0) {
        g_s32MapFd = open(AENC_MEM_DEV, O_RDWR | O_SYNC);
        if (g_s32MapFd < 0) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "open AENC_MEM_DEV error\n");
            return HI_FAILURE;
        }
    }

    /* init module state */
    g_stAencContext.bAencInited = HI_TRUE;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_Client_Init\n");
    return HI_SUCCESS;
}

HI_S32 MAPI_AENC_Client_Deinit()
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

    /* close mem map */
    if (g_s32MapFd >= 0) {
        if (close(g_s32MapFd)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "close AENC_MEM_DEV error\n");
        }
        g_s32MapFd = -1;
    }

    /* deinit chn and datafifo attr */
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        g_stAencContext.astADatafifo[i].DatafifoHdl = HI_INVALID_HANDLE;
    }
    memset(&g_stAencContext, 0, sizeof(MAPI_AENC_CONTEXT_S));

    /* deinit module  state */
    g_stAencContext.bAencInited = HI_FALSE;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_Client_Deinit\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Init(HI_HANDLE AencHdl, const HI_MAPI_AENC_ATTR_S *pstAencAttr)
{
    HI_S32 s32Ret;
    HI_U32 i = 0;
    HI_MAPI_AENC_ATTR_S stAencAttr;
    MAPI_PRIV_DATA_S stMsgPriData;
    MAPI_AENC_DATAFIFO_S stDataFifo;

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

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STOPED &&
        pstAencAttr->enAencFormat == g_stAencContext.astAencChn[AencHdl].stAencChnAttr.enAencFormat &&
        pstAencAttr->u32PtNumPerFrm == g_stAencContext.astAencChn[AencHdl].stAencChnAttr.u32PtNumPerFrm) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc init attr is not change, return successful\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* send ipcmsg to init aenc server */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    memcpy(&stAencAttr, pstAencAttr, sizeof(HI_MAPI_AENC_ATTR_S));
    memcpy(stMsgPriData.as32PrivData, pstAencAttr, sizeof(HI_MAPI_AENC_ATTR_S));
    s32Ret = MAPI_SendSyncWithResp(u32ModFd, MSG_CMD_AENC_INIT, (HI_VOID *)stAencAttr.pValue, &stDataFifo,
                                   stAencAttr.u32Len, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc init failed! s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* open client datafifo */
    g_stAencContext.astADatafifo[AencHdl].DatafifoHdl = stDataFifo.DatafifoHdl;
    g_stAencContext.astADatafifo[AencHdl].u64DataFifoSharePhyAddr = stDataFifo.u64DataFifoSharePhyAddr;
    g_stAencContext.astADatafifo[AencHdl].u64StreamBufPhyAddr = stDataFifo.u64StreamBufPhyAddr;
    g_stAencContext.astADatafifo[AencHdl].u32StreamBufLen = stDataFifo.u32StreamBufLen;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC,
                     "DataFifo Info:\n Handle=%llu\n SharePhyAddr=%llx\n StreamBufPhyAddr=%llx\n StreamBufLen=%u\n",
                     stDataFifo.DatafifoHdl, stDataFifo.u64DataFifoSharePhyAddr, stDataFifo.u64StreamBufPhyAddr,
                     stDataFifo.u32StreamBufLen);
    s32Ret = MAPI_AENC_OpenDatafifoReader(AencHdl);
    if (s32Ret != HI_SUCCESS) {
        return s32Ret;
    }

    /* create client thread */
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
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "pthread_create ProcAStreamThread s32Ret:%x\n", s32Ret);

        MUTEX_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
        g_stAencContext.astProcADataThread[AencHdl].bProcADataThdStartFlg = HI_FALSE;
        MUTEX_UNLOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);

        MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
        pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* save client chn attr and init lock */
    MUTEX_INIT_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    memcpy(&g_stAencContext.astAencChn[AencHdl].stAencChnAttr, pstAencAttr, sizeof(HI_MAPI_AENC_ATTR_S));
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;
    }

    /* set chn state */
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STOPED;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Init [%d]\n", AencHdl);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Deinit(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;

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

    /* send ipcmsg to deinit server aenc */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_DEINIT, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " aenc deinit failed! s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* stop thread */
    MUTEX_LOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    g_stAencContext.astProcADataThread[AencHdl].bProcADataThdStartFlg = HI_FALSE;
    pthread_cond_signal(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);
    MUTEX_UNLOCK(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);

    if ((pthread_t)-1 != g_stAencContext.astProcADataThread[AencHdl].tProcADataThread) {
        s32Ret = pthread_join(g_stAencContext.astProcADataThread[AencHdl].tProcADataThread, HI_NULL);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " pthread_join failed! s32Ret:%x\n", s32Ret);
            MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
            pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);

            MUTEX_DESTROY(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
            MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
            return s32Ret;
        }
    }

    MUTEX_DESTROY(g_stAencContext.astProcADataThread[AencHdl].m_ProcADataThd_lock);
    pthread_cond_destroy(&g_stAencContext.astProcADataThread[AencHdl].c_ProcADataThd_cond);

    /* close client datafifo */
    s32Ret = MAPI_AENC_CloseDatafifoReader(AencHdl);
    if (s32Ret != HI_SUCCESS) {
        return s32Ret;
    }

    /* deinit client chn attr and destory lock */
    MUTEX_DESTROY(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    memset(&g_stAencContext.astAencChn[AencHdl].stAencChnAttr, 0, sizeof(HI_MAPI_AENC_ATTR_S));
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;
    }

    /* set chn state */
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_UNINITED;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Deinit [%d]\n", AencHdl);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Start(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is uninited ,can not be start !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STARTED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc has already started ,return successfull !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* send ipcmsg to open server datafifo and get info */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_START, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " aenc start failed! s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* set chn state */
    MUTEX_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STARTED;
    MUTEX_UNLOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Start [%d]\n", AencHdl);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_Stop(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;
    MAPI_AENC_LINKDATA_INFO_S stLinkDataInfo;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);

    MUTEX_LOCK(g_aencFuncLock[AencHdl]);

    if (!g_stAencContext.bAencInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AENC has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ENOTINITED;
    }

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is uninited ,can not be stop !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_MAPI_AENC_ESTATEERR;
    }
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_STOPED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "aenc has already inited with stoped ,return successfull !\n");
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return HI_SUCCESS;
    }

    /* send ipcmsg to stop server aenc */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_STOP, &stLinkDataInfo, sizeof(MAPI_AENC_LINKDATA_INFO_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " aenc stop failed! s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* wait client datafifo read complete */
    while (g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum != stLinkDataInfo.u32LinkFrmNum ||
           g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq != stLinkDataInfo.u32LastFrmSeq) {
        usleep(1000);

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC,
            "==== wait for read complete ==== \n client frmNum = %d , seq = %d \n server frmNum = %d , seq = %d \n",
            g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum,
            g_stAencContext.astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq,
            stLinkDataInfo.u32LinkFrmNum, stLinkDataInfo.u32LastFrmSeq);
    }

    /* send ipcmsg to stop finish server aenc */
    u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_STOP_FINISH, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, " aenc stop finish failed! s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
        return s32Ret;
    }

    /* set chn state */
    MUTEX_LOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);
    g_stAencContext.astAencChn[AencHdl].enAencChnState = MAPI_AENC_CHN_STOPED;
    MUTEX_UNLOCK(g_stAencContext.astAencChn[AencHdl].m_AencChn_lock);

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Stop [%d]\n", AencHdl);
    MUTEX_UNLOCK(g_aencFuncLock[AencHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_BindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;
    MAPI_PRIV_DATA_S stMsgPriData;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_ACAPHANDLE_RANGE(AcapHdl);
    CHECK_MAPI_AENC_ACAPCHNHANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is uninited ,can not bind acap !\n");
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* send ipcmsg */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    stMsgPriData.as32PrivData[0] = AcapChnHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_BIND_ACAP, &AcapHdl, sizeof(HI_HANDLE), &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "Bind fail,AcapHdl:%d AencHdl:%d,s32Ret:%x\n", AcapHdl, AencHdl, s32Ret);
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_BindACap\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_UnbindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;
    MAPI_PRIV_DATA_S stMsgPriData;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_ACAPHANDLE_RANGE(AcapHdl);
    CHECK_MAPI_AENC_ACAPCHNHANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is uninited ,can not unbind acap !\n");
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* send ipcmsg */
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    stMsgPriData.as32PrivData[0] = AcapChnHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_UNBIND_ACAP, &AcapHdl, sizeof(HI_HANDLE), &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "UnBind fail,AencHdl:%d,s32Ret:%x\n", AencHdl, s32Ret);
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnbindACap\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_RegisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB)
{
    HI_U32 i;
    HI_S32 s32NullIdx = -1;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencCB);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not reg callback func !\n");
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* reg CB , search the same callback to update */
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        /* save null callback position */
        if (g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB == HI_NULL) {
            s32NullIdx = i;
        }

        if (pstAencCB->pfnDataCB == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB &&
            pstAencCB->pPrivateData == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData) {
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = pstAencCB->pfnDataCB;
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = pstAencCB->pPrivateData;
            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "update register callback success!\n");
            return HI_SUCCESS;
        }
    }

    /* if no find same callback , set new callback to null callback postion */
    if (s32NullIdx != -1) {
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[s32NullIdx].pfnDataCB = pstAencCB->pfnDataCB;
        g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[s32NullIdx].pPrivateData = pstAencCB->pPrivateData;
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "register new callback success!\n");
        return HI_SUCCESS;
    }

    return HI_MAPI_AENC_ERESFULL;
}

HI_S32 HI_MAPI_AENC_UnregisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB)
{
    HI_U32 i;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencCB);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);
    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not unreg callback func !\n");
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* unreg callback */
    for (i = 0; i < HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM; i++) {
        if (pstAencCB->pfnDataCB == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB &&
            pstAencCB->pPrivateData == g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData) {
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pfnDataCB = HI_NULL;
            g_stAencContext.astAencChn[AencHdl].astAencRegCallBacks[i].pPrivateData = HI_NULL;

            MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnregisterCallback\n");
            return HI_SUCCESS;
        }
    }

    return HI_MAPI_AENC_EUNEXIST;
}

HI_S32 HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_REGISTER_AENCODER, &enAudioFormat, sizeof(HI_MAPI_AUDIO_FORMAT_E),
                           HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MSG_CMD_AENC_REGISTER_AENCODER fail\n");
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_RegisterAudioEncoder\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_UnregisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_UNREGISTER_AENCODER, &enAudioFormat, sizeof(HI_MAPI_AUDIO_FORMAT_E),
                           HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MSG_CMD_AENC_UNREGISTER_AENCODER fail\n");
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnregisterAudioEncoder\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AENC_SendFrame(HI_HANDLE AencHdl, const AUDIO_FRAME_S *pstFrm)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_IPCMSG_MESSAGE_S *pReq = HI_NULL;
    HI_IPCMSG_MESSAGE_S *pResp = HI_NULL;
    AUDIO_FRAME_S stFrm;

    /* check attr and state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstFrm);
    CHECK_MAPI_AENC_CHECK_INIT(g_stAencContext.bAencInited);

    if (g_stAencContext.astAencChn[AencHdl].enAencChnState == MAPI_AENC_CHN_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aenc is deinit ,can not send frame !\n");
        return HI_MAPI_AENC_ESTATEERR;
    }

    /* send ipcmsg to send frame to server */
    memcpy(&stFrm, pstFrm, sizeof(AUDIO_FRAME_S));
    stFrm.u64VirAddr[0] = (HI_U8 *)(HI_UL)stFrm.u64PhyAddr[0];
    stFrm.u64VirAddr[1] = (HI_U8 *)(HI_UL)stFrm.u64PhyAddr[1];

    u32ModFd = MODFD(HI_MAPI_MOD_AENC, 0, AencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_AENC_SEND_FRAME, &stFrm, sizeof(AUDIO_FRAME_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MSG_CMD_AENC_SEND_FRAME fail\n");
        return s32Ret;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
