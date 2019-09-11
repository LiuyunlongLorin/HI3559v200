/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_venc_client.c
 * @brief   venc client function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>

#include "hi_math.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_sys.h"
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "msg_venc.h"
#include "msg_define.h"
#include "hi_datafifo.h"
#include "mapi_venc_inner.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#define VENC_MEM_DEV "/dev/mmz_userdev"

static MAPI_VENC_CONTEXT_S g_stVencContext;
static HI_S32 g_siMapFd = -1;

static HI_S32 MAPI_VENC_VStreamProcess(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pstVencData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    MAPI_VENC_CHN_S *pstVencChn = HI_NULL;

    pstVencChn = &g_stVencContext.astVencChn[VencHdl];

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        if (pstVencChn->astVencRegisterCallbacks[i].pfnDataCB != HI_NULL) {
            s32Ret = pstVencChn->astVencRegisterCallbacks[i].pfnDataCB(VencHdl, pstVencData,
                pstVencChn->astVencRegisterCallbacks[i].pPrivateData);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "venc[%d] astVencRegisterCallbacks[%d].pfnDataCB fail,s32Ret:%x\n",
                               VencHdl, i, s32Ret);
            }
        }
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_GetVStream(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    HI_U32 i;
    HI_MAPI_VENC_DATA_S stOutVStream;
    MAPI_VENC_HISILINK_STREAM_S *pstStream = HI_NULL;

    /*
        we add while here before,  it will break the loop until HI_DATAFIFO_Read function return HI_DATAFIFO_ERR_NO_DATA,
        but if user execute  pfnDataCB function cost time,  HI_DATAFIFO_Read will always get data  in the loop,  so  m_VencChn_lock
        will not unlock and the other interface such as HI_MAPI_VENC_UnRegisterCallback get stuck.

        we have test lock and unlock mutex function cost time less than 1ms, so remove the loop here
    */

    s32Ret = HI_DATAFIFO_Read(g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl, (HI_VOID **)&pstStream);

    if (s32Ret != HI_SUCCESS) {
        if (s32Ret != HI_DATAFIFO_ERR_NO_DATA) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "read video data fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        }

        return HI_SUCCESS;
    }

    memcpy(&stOutVStream, &pstStream->stVencData, sizeof(HI_MAPI_VENC_DATA_S));

    if (pstStream->stVencData.u32PackCount > 0) {
        for (i = 0; i < pstStream->stVencData.u32PackCount; i++) {
            stOutVStream.astPack[i].apu8Addr[1] = (HI_U8 *)g_stVencContext.astVDatafifo[VencHdl].pDataVirtAddr;
            stOutVStream.astPack[i].au64PhyAddr[1] = g_stVencContext.astVDatafifo[VencHdl].au64StreamBufPhyAddr[0];

            if (g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0] > VENC_MAX_U32_VALUE) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "%llu can not larger than max val %u, value overflow!\n",
                               g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0], VENC_MAX_U32_VALUE);

                s32Ret = HI_DATAFIFO_CMD(g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl, DATAFIFO_CMD_READ_DONE,
                                         pstStream);

                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "datafifo read down error,s32Ret:%x\n", s32Ret);
                }

                return HI_FAILURE;
            }

            stOutVStream.astPack[i].au32Len[1] = (HI_U32)g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0];
            stOutVStream.astPack[i].apu8Addr[0] = (HI_U8 *)g_stVencContext.astVDatafifo[VencHdl].pDataVirtAddr +
                                                  (stOutVStream.astPack[i].au64PhyAddr[0] -
                                                   g_stVencContext.astVDatafifo[VencHdl].au64StreamBufPhyAddr[0]);
        }

        s32Ret = MAPI_VENC_VStreamProcess(VencHdl, &stOutVStream);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "getting video stream error,s32Ret:%x\n", s32Ret);
        }

        if (!g_stVencContext.astVencChn[VencHdl].bFirstFrame) {
            g_stVencContext.astVencChn[VencHdl].bFirstFrame = HI_TRUE;
            struct timeval tv;
            gettimeofday(&tv, 0);
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "[### VencHdl:%d the first time of VideoFrame(MAPI_VENC_GetVStream): time: [%ld][%ld]\n", 
                            VencHdl, tv.tv_sec, tv.tv_usec);
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "[### VencHdl:%d the first time of VideoFrame(MAPI_VENC_GetVStream): u64PTS: [%llu]\n", 
                            VencHdl, pstStream->stVencData.astPack[0].u64PTS);
        }
    } else {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "u32PackCount is 0\n");
    }

    s32Ret = HI_DATAFIFO_CMD(g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl, DATAFIFO_CMD_READ_DONE, pstStream);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "datafifo read down error,s32Ret:%x\n", s32Ret);
    }

    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum++;
    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq = pstStream->stVencData.u32Seq;

    return HI_SUCCESS;
}

static HI_VOID MAPI_VENC_CondTimeWait(pthread_cond_t *pCond, pthread_mutex_t *pMutex, HI_U64 u64delayUs)
{
    struct timespec ts;
#ifndef __HuaweiLite__
    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
#endif

    /* us  transition ns */
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
    HI_U32 i = 0;
    HI_U32 readLen = 0;

    /* ThreadName size is 16  byte */
    HI_CHAR szThreadName[16] = {0};

    /* ThreadName size is 16  byte */
    snprintf(szThreadName, 16, "Hi_pTVData_Chn%u", VencHdl);
    prctl(PR_SET_NAME, (unsigned long)szThreadName, 0, 0, 0);
    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    while (g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg == HI_TRUE) {
        MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE) {
            readLen = 0;

            HI_DATAFIFO_CMD(g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl,
                            DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);

            if (readLen > 0) {
                s32Ret = MAPI_VENC_GetVStream(VencHdl);

                if (s32Ret != HI_SUCCESS) {
                    MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_GetVStream fail,s32Ret:%x\n", s32Ret);
                }

                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            } else {
                /* No video data read,Let out a cpu, scheduling the other threads */
                MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
                MAPI_VENC_CondTimeWait(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond,
                                       &g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock,
                                       VENC_GET_FRAME_THREAD_USLEEP_TIME);
            }
        } else {
            /* No channel is started,Let out a cpu, scheduling the other threads */
            MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            MAPI_VENC_CondTimeWait(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond,
                                   &g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock,
                                   VENC_GET_FRAME_THREAD_USLEEP_TIME);
        }
    }

    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    return HI_NULL;
}

static HI_S32 MAPI_VENC_InitVDataThread(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;

    /* create client stream processing thread */
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

    /* set thread status */
    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    s32Ret = pthread_create(&g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread, HI_NULL, ProcVDataThread,
                            &g_stVencContext.astVencChn[VencHdl].VencHandle);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "create ProcVDataThread pthread fail! s32Ret:%x\n", s32Ret);

        MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
        g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_FALSE;
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

    /* destroy client thread */
    MUTEX_LOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);
    g_stVencContext.astProcVDataThread[VencHdl].bProcVDataThdStartFlg = HI_FALSE;
    pthread_cond_signal(&g_stVencContext.astProcVDataThread[VencHdl].c_ProcVDataThd_cond);
    MUTEX_UNLOCK(g_stVencContext.astProcVDataThread[VencHdl].m_ProcVDataThd_lock);

    if (g_stVencContext.astProcVDataThread[VencHdl].tProcVDataThread != (pthread_t)-1) {
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

HI_VOID MAPI_VENC_Client_SetDatafifoNodeNum(HI_U32 u32DatafifoNodeNum)
{
    g_stVencContext.u32DatafifeNodeNum = u32DatafifoNodeNum;
}

static HI_S32 MAPI_Venc_Datafifo_Reader_Open(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    HI_U64 u64DataPhyAddr;
    HI_U64 u64DataLen;
    HI_VOID *pDataVirtAddr = HI_NULL;
    HI_U32 u32VencDatafifoNodeNum;

    if (g_stVencContext.astVDatafifo[VencHdl].bUsed == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "datafifo has been opened\n");
        return HI_SUCCESS;
    }

    u32VencDatafifoNodeNum = g_stVencContext.u32DatafifeNodeNum;
    if (u32VencDatafifoNodeNum == 0) {
        /* use 100 node from experience in small memory scenes */
        u32VencDatafifoNodeNum = MAPI_VENC_DATAFIFONODE_CNT;
    }

    HI_DATAFIFO_PARAMS_S params = { u32VencDatafifoNodeNum, sizeof(MAPI_VENC_HISILINK_STREAM_S),
                                    HI_TRUE, DATAFIFO_READER
                                    };
    s32Ret = HI_DATAFIFO_OpenByAddr(&g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl, &params,
                                    g_stVencContext.astVDatafifo[VencHdl].u64DataFifoSharePhyAddr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "datafifo reader open fail,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    u64DataPhyAddr = g_stVencContext.astVDatafifo[VencHdl].au64StreamBufPhyAddr[0];
    u64DataLen = g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0];

    pDataVirtAddr = mmap(HI_NULL, u64DataLen, PROT_READ | PROT_WRITE, MAP_SHARED, g_siMapFd, u64DataPhyAddr);

    if (pDataVirtAddr == (void *)-1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "mmap pDataVirtAddr error,u64DataPhyAddr:%llx\n", u64DataPhyAddr);
        return HI_FAILURE;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVDatafifo[VencHdl].pDataVirtAddr = pDataVirtAddr;
    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum = 0;
    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq = 0;
    g_stVencContext.astVDatafifo[VencHdl].bUsed = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

static HI_S32 MAPI_Venc_Datafifo_Reader_Close(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;

    if (g_stVencContext.astVDatafifo[VencHdl].bUsed != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "datafifo has been closed\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_DATAFIFO_Close(g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "datafifo reader close fail,DataFifoHdl:%llx,s32Ret:%x\n",
                       g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl, s32Ret);
        return s32Ret;
    }

    if (0 != munmap(g_stVencContext.astVDatafifo[VencHdl].pDataVirtAddr,
                    g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0])) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "datafifo addr munmap error\n");
        return HI_FAILURE;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum = 0;
    g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq = 0;
    g_stVencContext.astVDatafifo[VencHdl].bUsed = HI_FALSE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

HI_S32 MAPI_VENC_Client_Init(HI_VOID)
{
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    if (g_stVencContext.bVencInited == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "VENC has been pre inited\n");
        return HI_SUCCESS;
    }

    /* init venc context attribute */
    memset(&g_stVencContext, 0, sizeof(MAPI_VENC_CONTEXT_S));

    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        g_stVencContext.astVencChn[i].VencHandle = i;
        g_stVencContext.astVDatafifo[i].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
    }

    if (g_siMapFd < 0) {
        g_siMapFd = open(VENC_MEM_DEV, O_RDWR | O_SYNC);

        if (g_siMapFd < 0) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "open VENC_MEM_DEV error\n");
            return HI_FAILURE;
        }
    }

    g_stVencContext.bVencInited = HI_TRUE;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_Client_Init\n");
    return HI_SUCCESS;
}

HI_S32 MAPI_VENC_Client_Deinit(HI_VOID)
{
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;

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

    memset(&g_stVencContext, 0, sizeof(MAPI_VENC_CONTEXT_S));

    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        g_stVencContext.astVDatafifo[i].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
    }

    if (close(g_siMapFd)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Close VENC_MEM_DEV error\n");
    }

    g_siMapFd = -1;
    g_stVencContext.bVencInited = HI_FALSE;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_Client_Deinit\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Init(HI_HANDLE VencHdl, const HI_MAPI_VENC_ATTR_S *pstVencAttr)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    MAPI_PRIV_DATA_S stMsgPriData;

    /* Check the parameters */
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);

    if (g_stVencContext.astVencChn[VencHdl].bInited == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been inited\n", VencHdl);
        return HI_SUCCESS;
    }

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    MAPI_VENC_DATAFIFO_S stDataFifo;
    stMsgPriData.as32PrivData[0] = g_stVencContext.u32DatafifeNodeNum;

    s32Ret = MAPI_SendSyncWithResp(u32ModFd, MSG_CMD_VENC_INIT, (HI_VOID *)pstVencAttr, &stDataFifo,
                                   sizeof(HI_MAPI_VENC_ATTR_S), &stMsgPriData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, " venc init failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    /* open reader datafifo */
    if (g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl != stDataFifo.DataFifoHdl) {
        g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl = stDataFifo.DataFifoHdl;
        g_stVencContext.astVDatafifo[VencHdl].u64DataFifoSharePhyAddr = stDataFifo.u64DataFifoSharePhyAddr;
        g_stVencContext.astVDatafifo[VencHdl].au64StreamBufPhyAddr[0] = stDataFifo.au64StreamBufPhyAddr[0];
        g_stVencContext.astVDatafifo[VencHdl].au64StreamBufLen[0] = stDataFifo.au64StreamBufLen[0];
    }

    /* Initialize the channel lock */
    MUTEX_INIT_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    s32Ret = MAPI_Venc_Datafifo_Reader_Open(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_Venc_Datafifo_Reader_Open fail,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    /* After the reader datafifo is initialized, writer open datafifo status */
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_SET_DATAFIFO_STATUS, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "venc datafifo open status fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_InitVDataThread(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_InitVDataThread error,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = HI_NULL;
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = HI_NULL;
    }

    g_stVencContext.astVencChn[VencHdl].bInited = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Deinit(HI_HANDLE VencHdl)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);

    if (g_stVencContext.astVencChn[VencHdl].bInited != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been deinited\n", VencHdl);
        return HI_SUCCESS;
    }

    if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE) {
        s32Ret = HI_MAPI_VENC_Stop(VencHdl);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                           "venc stop handle:%d failed! s32Ret:%x\n", VencHdl, s32Ret);
            return s32Ret;
        }
    }

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_DEINIT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, " venc destroy failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_DeinitVDataThread(VencHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_DeinitVDataThread error,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    /* close reader datafifo */
    s32Ret = MAPI_Venc_Datafifo_Reader_Close(VencHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_Venc_Datafifo_Reader_Close error,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    memset(&g_stVencContext.astVDatafifo[VencHdl], 0, sizeof(MAPI_VENC_DATAFIFO_S));
    g_stVencContext.astVDatafifo[VencHdl].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
    memset(&g_stVencContext.astVencChn[VencHdl].stVencAttr, 0, sizeof(HI_MAPI_VENC_ATTR_S));
    g_stVencContext.astVencChn[VencHdl].bInited = HI_FALSE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);

    /* destroy the channel lock */
    MUTEX_DESTROY(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_RegisterCallback(HI_HANDLE VencHdl, HI_MAPI_VENC_CALLBACK_S *pstVencCB)
{
    HI_U32 i;
    HI_S32 s32RecordIdx = -1;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencCB);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

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
            return HI_SUCCESS;
        }
    }

    if (s32RecordIdx != -1) {
        MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[s32RecordIdx].pfnDataCB = pstVencCB->pfnDataCB;
        g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[s32RecordIdx].pPrivateData = 
            pstVencCB->pPrivateData;
        MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
        return HI_SUCCESS;
    }

    return HI_MAPI_VENC_ENOT_PERM;
}

HI_S32 HI_MAPI_VENC_UnRegisterCallback(HI_HANDLE VencHdl, HI_MAPI_VENC_CALLBACK_S *pstVencCB)
{
    HI_U32 i;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVencCB);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    for (i = 0; i < HI_MAPI_VENC_CALLBACK_MAX_NUM; i++) {
        if (pstVencCB->pfnDataCB == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB &&
            pstVencCB->pPrivateData == g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData) {
            MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pfnDataCB = HI_NULL;
            g_stVencContext.astVencChn[VencHdl].astVencRegisterCallbacks[i].pPrivateData = HI_NULL;
            MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
            return HI_SUCCESS;
        }
    }

    return HI_MAPI_VENC_ENOT_PERM;
}

HI_S32 HI_MAPI_VENC_Start(HI_HANDLE VencHdl, HI_S32 s32FrameCnt)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_FRAMECNT_RANGE(s32FrameCnt);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].bStarted == HI_TRUE && s32FrameCnt == HI_MAPI_VENC_LIMITLESS_FRAME_COUNT) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been started\n", VencHdl);
        return HI_SUCCESS;
    }

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_START, &s32FrameCnt, sizeof(HI_S32), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, " venc start failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].bStarted = HI_TRUE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_Stop(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    MAPI_VENC_LINKDATA_INFO_S stLinkDataInfo;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    if (g_stVencContext.astVencChn[VencHdl].bStarted != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "chn %d has been stopped\n", VencHdl);
        return HI_SUCCESS;
    }

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_STOP_READY, &stLinkDataInfo, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, " venc stop failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    while (g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq != stLinkDataInfo.u32LastFrmSeq ||
           g_stVencContext.astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum != stLinkDataInfo.u32LinkFrmNum) {
        /* sleep 1ms Wait for the last frame */
        usleep(1000);
    }

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_STOP, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, " venc stop failed! s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    MUTEX_LOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    g_stVencContext.astVencChn[VencHdl].bStarted = HI_FALSE;
    MUTEX_UNLOCK(g_stVencContext.astVencChn[VencHdl].m_VencChn_lock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_BindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VCAPPIPE_HANDLE(VcapPipeHdl);
    CHECK_MAPI_PIPECHN_HANDLE(PipeChnHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32VProcFd = MODFD(HI_MAPI_MOD_VENC, VcapPipeHdl, PipeChnHdl);

    s32Ret = MAPI_SendSync(u32VProcFd, MSG_CMD_VENC_BIND_VCAP, &VencHdl, sizeof(HI_HANDLE), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Bind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_UnBindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VCAPPIPE_HANDLE(VcapPipeHdl);
    CHECK_MAPI_PIPECHN_HANDLE(PipeChnHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32VProcFd = MODFD(HI_MAPI_MOD_VENC, VcapPipeHdl, PipeChnHdl);

    s32Ret = MAPI_SendSync(u32VProcFd, MSG_CMD_VENC_UNBIND_VCAP, &VencHdl, sizeof(HI_HANDLE), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "UnBind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_BindVProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl, HI_BOOL bStitch)
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

    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    MAPI_PRIV_DATA_S stMsgPrivData;
    HI_U32 u32VProcFd = MODFD(HI_MAPI_MOD_VENC, VProcHdl, VPortHdl);

    stMsgPrivData.as32PrivData[0] = bStitch;
    s32Ret = MAPI_SendSync(u32VProcFd, MSG_CMD_VENC_BIND_VPROC, &VencHdl, sizeof(HI_HANDLE), &stMsgPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Bind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_UnBindVProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_HANDLE VencHdl, HI_BOOL bStitch)
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

    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    MAPI_PRIV_DATA_S stMsgPrivData;
    HI_U32 u32VProcFd = MODFD(HI_MAPI_MOD_VENC, VProcHdl, VPortHdl);
    stMsgPrivData.as32PrivData[0] = bStitch;

    s32Ret = MAPI_SendSync(u32VProcFd, MSG_CMD_VENC_UNBIND_VPROC, &VencHdl, sizeof(HI_HANDLE), &stMsgPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "UnBind fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetAttr(HI_HANDLE VencHdl, const HI_MAPI_VENC_ATTR_S *pstStreamAttr)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstStreamAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_SET_ATTR, (HI_VOID *)pstStreamAttr, sizeof(HI_MAPI_VENC_ATTR_S),
                           HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "set attr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetAttr(HI_HANDLE VencHdl, HI_MAPI_VENC_ATTR_S *pstStreamAttr)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstStreamAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_GET_ATTR, (HI_VOID *)pstStreamAttr, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "get attr fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_RequestIDR(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_REQ_IDR, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "VENC RequestIDR fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetAttrEx(HI_HANDLE VencHdl, HI_MAPI_VENC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 U32Len)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    MAPI_PRIV_DATA_S stMsgPrivData;
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);

    memcpy(&stMsgPrivData, &enCMD, sizeof(HI_MAPI_VENC_CMD_E));
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_SET_EXTEND_ATTR, pAttr, U32Len, &stMsgPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_SetAttrEx fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetAttrEx(HI_HANDLE VencHdl, HI_MAPI_VENC_CMD_E enCMD, HI_VOID *pAttr, HI_U32 U32Len)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    MAPI_PRIV_DATA_S stMsgPrivData;
    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, 0, VencHdl);

    memcpy(&stMsgPrivData, &enCMD, sizeof(HI_MAPI_VENC_CMD_E));
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_GET_EXTEND_ATTR, pAttr, U32Len, &stMsgPrivData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_GetAttrEx fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_SetOSDAttr(HI_HANDLE VencHdl, HI_HANDLE OSDHdl, HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, VencHdl, OSDHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_SET_OSD_ATTR, pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_SetOSDAttr fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_GetOSDAttr(HI_HANDLE VencHdl, HI_HANDLE OSDHdl, HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, VencHdl, OSDHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_GET_OSD_ATTR, pstOSDAttr, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_GetOSDAttr fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_StartOSD(HI_HANDLE VencHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, VencHdl, OSDHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_START_OSD, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_StartOSD fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VENC_StopOSD(HI_HANDLE VencHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_OSD_HANDLE_RANGE(OSDHdl);
    CHECK_MAPI_VENC_CHECK_PREINIT(g_stVencContext.bVencInited);
    CHECK_MAPI_VENC_CHECK_INIT(g_stVencContext.astVencChn[VencHdl].bInited);

    HI_U32 u32ModFd = MODFD(HI_MAPI_MOD_VENC, VencHdl, OSDHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VENC_STOP_OSD, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_StopOSD fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
