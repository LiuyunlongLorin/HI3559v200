/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_aenc_os.c
 * @brief   server aenc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "mapi_aenc_inner.h"
#include "mapi_comm_inner.h"
#include "hi_datafifo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static MAPI_AENC_DATAFIFO_S g_astADatafifo[HI_MAPI_AENC_CHN_MAX_NUM];

HI_VOID MAPI_AENC_OSInit(HI_VOID)
{
    HI_U32 i;
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        memset(&g_astADatafifo[i], 0, sizeof(MAPI_AENC_DATAFIFO_S));

        g_astADatafifo[i].enADatafifoState = MAPI_AENC_DATAFIFO_CLOSED;
        g_astADatafifo[i].DatafifoHdl = HI_INVALID_HANDLE;
        g_astADatafifo[i].pFnALinkDataProcCB = HI_NULL;
    }
}

HI_VOID MAPI_AENC_OSDeinit(HI_VOID)
{
    HI_U32 i;
    for (i = 0; i < HI_MAPI_AENC_CHN_MAX_NUM; i++) {
        memset(&g_astADatafifo[i], 0, sizeof(MAPI_AENC_DATAFIFO_S));

        g_astADatafifo[i].DatafifoHdl = HI_INVALID_HANDLE;
        g_astADatafifo[i].enADatafifoState = MAPI_AENC_DATAFIFO_BUTT;
        g_astADatafifo[i].pFnALinkDataProcCB = HI_NULL;
    }
}

HI_BOOL MAPI_AENC_IsRegisterCallback(HI_HANDLE AencHdl, const MAPI_AENC_CHN_S *pstAencChn)
{
    MAPI_UNUSED(pstAencChn);
    if (g_astADatafifo[AencHdl].pFnALinkDataProcCB != HI_NULL
        && g_astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_OPENED) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

HI_U32 MAPI_AENC_GetLinkDataInfo(HI_HANDLE AencHdl, MAPI_AENC_LINKDATA_INFO_S *pstLinkDataInfo)
{
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstLinkDataInfo);

    pstLinkDataInfo->u32LastFrmSeq = g_astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq;
    pstLinkDataInfo->u32LinkFrmNum = g_astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum;

    return HI_SUCCESS;
}

static HI_S32 MAPI_AENC_WriteLinkData(HI_HANDLE AencHdl, MAPI_AENC_HISILINK_STREAM_S *pstALinkStreamData)
{
    HI_S32 s32Ret;
    s32Ret = HI_DATAFIFO_Write(g_astADatafifo[AencHdl].DatafifoHdl, pstALinkStreamData);
    if (s32Ret != HI_SUCCESS) {
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_CMD(g_astADatafifo[AencHdl].DatafifoHdl, DATAFIFO_CMD_WRITE_DONE, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static HI_VOID MAPI_AENC_ReleaseLinkData(MAPI_AENC_HISILINK_STREAM_S *pstALinkStream)
{
    HI_S32 s32Ret;

    if (pstALinkStream->pPriv != HI_NULL) {
        AUDIO_STREAM_S *pstAencStream = (AUDIO_STREAM_S *)pstALinkStream->pPriv;
        HI_HANDLE AencHandle = pstALinkStream->handle;

        s32Ret = HI_MPI_AENC_ReleaseStream(AencHandle, pstAencStream);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_ReleaseStream  AEncHdl:%d err, s32Ret:%x\n", AencHandle,
                           s32Ret);
        }

        if (pstAencStream != HI_NULL) {
            free(pstAencStream);
        }

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ReleaseLinkData, handle = %d   Stream seq = %d\n",
                         AencHandle, pstALinkStream->stAencData.u32Seq);
    }
}

static HI_S32 MAPI_AENC_ProcLinkAStream(HI_S32 AencHdl, MAPI_AENC_HISILINK_STREAM_S *pstAencLinkStream)
{
    HI_S32 s32Ret = HI_FAILURE;
    /* if datafifo is full, we will try 10 times at most, then transfer mutex */
    HI_U32 u32MaxLoopCnt = 10;
    HI_U32 i = 0;

    do {
        s32Ret = g_astADatafifo[AencHdl].pFnALinkDataProcCB(AencHdl, pstAencLinkStream);
        if (s32Ret == HI_SUCCESS) {
            break;
        }

        g_astADatafifo[AencHdl].pFnALinkDataProcCB(AencHdl, HI_NULL);
        usleep(AENC_PROCESS_STREAM_USLEEP_TIME);
        i++;
    } while (i < u32MaxLoopCnt);

    if (s32Ret == HI_SUCCESS) {
        g_astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum++;
        g_astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = pstAencLinkStream->stAencData.u32Seq;
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ProcLinkAStream , write down seq = %d , addr = %llu \n",
                         pstAencLinkStream->stAencData.u32Seq, pstAencLinkStream->stAencData.u64PhyAddr);
    }

    return s32Ret;
}

HI_S32 MAPI_AENC_GetAStream(HI_HANDLE AencHdl, const MAPI_AENC_CHN_S *pstAencChn)
{
    MAPI_UNUSED(pstAencChn);
    HI_S32 s32Ret;
    MAPI_AENC_HISILINK_STREAM_S stAencOutLinkStream;
    AUDIO_STREAM_S *pstAencStream;

    /* get frame */
    if (g_astADatafifo[AencHdl].stUnhandleStream.pPriv == HI_NULL) {
        pstAencStream = (AUDIO_STREAM_S *)malloc(sizeof(AUDIO_STREAM_S));
        if (pstAencStream == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "malloc err, pstAencStream is null\n");
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_AENC_GetStream(AencHdl, pstAencStream, HI_FALSE);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_GetStream err,ret:%x\n", s32Ret);
            if (pstAencStream != HI_NULL) {
                free(pstAencStream);
            }

            return s32Ret;
        }

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "Server GetSteam Info: handle = %d , seq = %d , len = %d\n", AencHdl,
                         pstAencStream->u32Seq, pstAencStream->u32Len);

        /* process frame */
        memcpy(&stAencOutLinkStream.stAencData, pstAencStream, sizeof(AUDIO_STREAM_S));
        stAencOutLinkStream.handle = AencHdl;
        stAencOutLinkStream.pPriv = pstAencStream;
        s32Ret = MAPI_AENC_ProcLinkAStream(AencHdl, &stAencOutLinkStream);
        if (s32Ret != HI_SUCCESS) {
            memcpy(&g_astADatafifo[AencHdl].stUnhandleStream, &stAencOutLinkStream,
                   sizeof(MAPI_AENC_HISILINK_STREAM_S));
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AEnc_Link_AStreamProcess Audio stream err,s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    } else {
        /* there is unhandled stream last time */
        s32Ret = MAPI_AENC_ProcLinkAStream(AencHdl, &g_astADatafifo[AencHdl].stUnhandleStream);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AEnc_Link_AStreamProcess Audio stream err,s32Ret:%x\n", s32Ret);
            return s32Ret;
        }
    }

    if (g_astADatafifo[AencHdl].stUnhandleStream.pPriv != HI_NULL) {
        memset(&g_astADatafifo[AencHdl].stUnhandleStream, 0, sizeof(MAPI_AENC_HISILINK_STREAM_S));
    }

    return HI_SUCCESS;
}

HI_VOID MAPI_AENC_ClearDatafifo(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;

    if (g_astADatafifo[AencHdl].pFnALinkDataProcCB != HI_NULL
        && g_astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_OPENED) {
        s32Ret = g_astADatafifo[AencHdl].pFnALinkDataProcCB(AencHdl, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ClearDatafifo fail, s32Ret:%x\n", s32Ret);
        }
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_ClearDatafifo \n");
    }
}

HI_VOID MAPI_AENC_ReleaseUnhandleStream(HI_HANDLE AencHdl)
{
    MAPI_AENC_ClearDatafifo(AencHdl);

    if (g_astADatafifo[AencHdl].stUnhandleStream.pPriv != HI_NULL) {
        MAPI_AENC_ReleaseLinkData(&g_astADatafifo[AencHdl].stUnhandleStream);
        memset(&g_astADatafifo[AencHdl].stUnhandleStream, 0, sizeof(MAPI_AENC_HISILINK_STREAM_S));
    }
}

HI_S32 MAPI_AENC_OpenDatafifoWriter(HI_HANDLE AencHdl, MAPI_AENC_DATAFIFO_S *pstAencDatafifo)
{
    HI_S32 s32Ret;
    HI_S32 s32Ret2;
    HI_U64 u64PhysAddr = 0;
    HI_U32 u32Size = 0;

    /* check datafifo state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    CHECK_MAPI_AENC_NULL_PTR(pstAencDatafifo);
    if (g_astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_OPENED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "datafifo is already opened , return success ! \n");
        return HI_SUCCESS;
    }

    /* open datafifo and set info */
    HI_DATAFIFO_PARAMS_S stParams = { 200, sizeof(MAPI_AENC_HISILINK_STREAM_S), HI_TRUE, DATAFIFO_WRITER };
    s32Ret = HI_DATAFIFO_Open(&g_astADatafifo[AencHdl].DatafifoHdl, &stParams);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Open failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = HI_DATAFIFO_CMD(g_astADatafifo[AencHdl].DatafifoHdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK,
                             MAPI_AENC_ReleaseLinkData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_CMD failed with %#x!\n", s32Ret);
        s32Ret2 = HI_DATAFIFO_Close(g_astADatafifo[AencHdl].DatafifoHdl);
        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Close failed with %#x!\n", s32Ret2);
        }
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_CMD(g_astADatafifo[AencHdl].DatafifoHdl, DATAFIFO_CMD_GET_PHY_ADDR,
                             &g_astADatafifo[AencHdl].u64DataFifoSharePhyAddr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_CMD failed with %#x!\n", s32Ret);
        s32Ret2 = HI_DATAFIFO_Close(g_astADatafifo[AencHdl].DatafifoHdl);
        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Close failed with %#x!\n", s32Ret2);
        }
        return s32Ret;
    }

    s32Ret = HI_MPI_AENC_GetStreamBufInfo(AencHdl, &u64PhysAddr, &u32Size);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MPI_AENC_GetStreamBufInfo failed with %#x!\n", s32Ret);
        s32Ret2 = HI_DATAFIFO_Close(g_astADatafifo[AencHdl].DatafifoHdl);
        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Close failed with %#x!\n", s32Ret2);
        }
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC,
                     "Datafifo Info : DatafifoHdl=%llu ,SharePhyAddr=%llu, StreamBufPhyAddr=%llu, BufSize=%u\n",
                     g_astADatafifo[AencHdl].DatafifoHdl, g_astADatafifo[AencHdl].u64DataFifoSharePhyAddr, u64PhysAddr,
                     u32Size);

    g_astADatafifo[AencHdl].u64StreamBufPhyAddr = u64PhysAddr;
    g_astADatafifo[AencHdl].u32StreamBufLen = u32Size;
    memcpy(pstAencDatafifo, &g_astADatafifo[AencHdl], sizeof(MAPI_AENC_DATAFIFO_S));
    g_astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum = 0;
    g_astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = 0;
    g_astADatafifo[AencHdl].pFnALinkDataProcCB = MAPI_AENC_WriteLinkData;

    /* set datafifo state */
    g_astADatafifo[AencHdl].enADatafifoState = MAPI_AENC_DATAFIFO_OPENED;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_OpenDatafifoWriter[%d]\n", AencHdl);
    return HI_SUCCESS;
}

HI_S32 MAPI_AENC_CloseDatafifoWriter(HI_HANDLE AencHdl)
{
    HI_S32 s32Ret;

    /* check datafifo state */
    CHECK_MAPI_AENC_HANDLE_RANGE(AencHdl);
    if (g_astADatafifo[AencHdl].enADatafifoState == MAPI_AENC_DATAFIFO_CLOSED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AENC, "datafifo is already closed , return success ! \n");
        return HI_SUCCESS;
    }

    /* close datafifo and set info */
    s32Ret = HI_DATAFIFO_Close(g_astADatafifo[AencHdl].DatafifoHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "HI_DATAFIFO_Close Fail. VencHdl:%d\n", AencHdl);
        return HI_FAILURE;
    }
    g_astADatafifo[AencHdl].pFnALinkDataProcCB = HI_NULL;
    g_astADatafifo[AencHdl].stALinkDataInfo.u32LinkFrmNum = 0;
    g_astADatafifo[AencHdl].stALinkDataInfo.u32LastFrmSeq = 0;

    /* set datafifo state */
    g_astADatafifo[AencHdl].enADatafifoState = MAPI_AENC_DATAFIFO_CLOSED;
    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "MAPI_AENC_CloseDatafifoWriter[%d]\n", AencHdl);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
