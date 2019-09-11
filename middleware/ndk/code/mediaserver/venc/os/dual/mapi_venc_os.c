/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_venc_os.c
 * @brief   venc os module function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_datafifo.h"
#include "mapi_venc_inner.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static MAPI_VENC_DATAFIFO_S g_astVDatafifo[HI_MAPI_VENC_MAX_CHN_NUM] = {0};

HI_VOID MAPI_VENC_OSInit(HI_VOID)
{
    HI_U32 i;
    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        memset(&g_astVDatafifo[i], 0, sizeof(MAPI_VENC_DATAFIFO_S));
        g_astVDatafifo[i].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
        g_astVDatafifo[i].bFirstFrame = HI_FALSE;
        g_astVDatafifo[i].bStart = HI_FALSE;
        g_astVDatafifo[i].bUsed = HI_FALSE;
        MUTEX_INIT_LOCK(g_astVDatafifo[i].m_DatafifoChn_lock);
    }
}

HI_VOID MAPI_VENC_OSDeInit(HI_VOID)
{
    HI_U32 i;
    for (i = 0; i < HI_MAPI_VENC_MAX_CHN_NUM; i++) {
        memset(&g_astVDatafifo[i], 0, sizeof(MAPI_VENC_DATAFIFO_S));
        g_astVDatafifo[i].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
        g_astVDatafifo[i].bFirstFrame = HI_FALSE;
        g_astVDatafifo[i].bStart = HI_FALSE;
        g_astVDatafifo[i].bUsed = HI_FALSE;
        MUTEX_DESTROY(g_astVDatafifo[i].m_DatafifoChn_lock);
    }
}

HI_VOID MAPI_VENC_OpenDatafifoStatus(HI_HANDLE VencHdl)
{
    g_astVDatafifo[VencHdl].bUsed = HI_TRUE;
    return;
}

HI_VOID MAPI_VENC_CloseDatafifoStatus(HI_HANDLE VencHdl)
{
    g_astVDatafifo[VencHdl].bUsed = HI_FALSE;
    return;
}

HI_BOOL MAPI_VENC_GetDatafifoStatus(HI_HANDLE VencHdl)
{
    return g_astVDatafifo[VencHdl].bUsed;
}

static HI_S32 MAPI_VENC_Release_VStream(HI_HANDLE VencHdl,
                                        MAPI_VENC_HISILINK_STREAM_S *pstOutLINK_VStream)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_NULL_PTR(pstOutLINK_VStream);
    CHECK_MAPI_VENC_NULL_PTR(pstOutLINK_VStream->pPriv);
    VENC_STREAM_S *pstVencStream = (VENC_STREAM_S *)pstOutLINK_VStream->pPriv;

    s32Ret = HI_MPI_VENC_ReleaseStream(VencHdl, pstVencStream);

    if (pstVencStream->pstPack != HI_NULL) {
        free(pstVencStream->pstPack);
    }

    pstVencStream->pstPack = HI_NULL;
    free(pstVencStream);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_ReleaseStream error,ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_Datafifo_DataProc(HI_HANDLE VencHdl, MAPI_VENC_HISILINK_STREAM_S *pVDDCStreamData)
{
    HI_S32 s32Ret;

    s32Ret = HI_DATAFIFO_Write(g_astVDatafifo[VencHdl].DataFifoHdl, pVDDCStreamData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_Write failed,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_CMD(g_astVDatafifo[VencHdl].DataFifoHdl, DATAFIFO_CMD_WRITE_DONE, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_CMD failed,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_VOID MAPI_VENC_Datafifo_ReleaseStream(MAPI_VENC_HISILINK_STREAM_S *pstLinkStream)
{
    if (pstLinkStream != HI_NULL) {
        HI_S32 s32Ret = MAPI_VENC_Release_VStream(pstLinkStream->handle, pstLinkStream);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_Release_Link_VStream failed,handle:%d,s32Ret:%x\n",
                           pstLinkStream->handle, s32Ret);
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "pstLinkStream is HI_NULL \n");
    }
}

HI_S32 MAPI_VENC_OpenDatafifoWriter(HI_HANDLE VencHdl, HI_U32 u32DatafifoNodeNum,
                                    MAPI_VENC_DATAFIFO_S *pstVEncDatafifo)
{
    HI_S32 s32Ret;
    HI_S32 s32Ret2;
    HI_U32 u32VencDatafifoNodeNum;
    VENC_STREAM_BUF_INFO_S stStreamBufInfo;

    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);
    CHECK_MAPI_VENC_NULL_PTR(pstVEncDatafifo);

    if (MAPI_VENC_GetDatafifoStatus(VencHdl) == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "datafifo has been opened\n");
        return HI_SUCCESS;
    }

    memset(&stStreamBufInfo, 0, sizeof(VENC_STREAM_BUF_INFO_S));

    u32VencDatafifoNodeNum = u32DatafifoNodeNum;
    if (u32VencDatafifoNodeNum == 0) {
        /* use 100 node from experience in small memory scenes */
        u32VencDatafifoNodeNum = MAPI_VENC_DATAFIFONODE_CNT;
    }

    HI_DATAFIFO_PARAMS_S stParams = {
        u32VencDatafifoNodeNum, sizeof(MAPI_VENC_HISILINK_STREAM_S), HI_TRUE, DATAFIFO_WRITER
    };

    s32Ret = HI_DATAFIFO_Open(&g_astVDatafifo[VencHdl].DataFifoHdl, &stParams);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_Open fail,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_CMD(g_astVDatafifo[VencHdl].DataFifoHdl, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK,
                             MAPI_VENC_Datafifo_ReleaseStream);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_CMD set data release callback fail,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_CMD(g_astVDatafifo[VencHdl].DataFifoHdl, DATAFIFO_CMD_GET_PHY_ADDR,
                             &g_astVDatafifo[VencHdl].u64DataFifoSharePhyAddr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_CMD get physical address fail,s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_GetStreamBufInfo(VencHdl, &stStreamBufInfo);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetStreamBufInfo fail,s32Ret:%x\n", s32Ret);
        s32Ret2 = HI_DATAFIFO_Close(g_astVDatafifo[VencHdl].DataFifoHdl);

        if (s32Ret2 != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_Close fail,s32Ret:%x\n", s32Ret2);
        }

        return s32Ret;
    }

    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    g_astVDatafifo[VencHdl].au64StreamBufPhyAddr[0] = stStreamBufInfo.u64PhyAddr[0];
    g_astVDatafifo[VencHdl].au64StreamBufLen[0] = stStreamBufInfo.u64BufSize[0];

    memcpy(pstVEncDatafifo, &g_astVDatafifo[VencHdl], sizeof(MAPI_VENC_DATAFIFO_S));
    g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum = 0;
    g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq = 0;
    g_astVDatafifo[VencHdl].pFnVencHisiLinkDataProc = MAPI_VENC_Datafifo_DataProc;
    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    return HI_SUCCESS;
}

HI_S32 MAPI_VENC_CloseDatafifoWriter(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VENC_HANDLE_RANGE(VencHdl);

    if (MAPI_VENC_GetDatafifoStatus(VencHdl) != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "datafifo has been closed\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_DATAFIFO_Write(g_astVDatafifo[VencHdl].DataFifoHdl, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_Write HI_NULL fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_DATAFIFO_Close(g_astVDatafifo[VencHdl].DataFifoHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_DATAFIFO_Close fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        return s32Ret;
    }

    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    MAPI_VENC_CloseDatafifoStatus(VencHdl);

    g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum = 0;
    g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq = 0;
    g_astVDatafifo[VencHdl].pFnVencHisiLinkDataProc = HI_NULL;
    g_astVDatafifo[VencHdl].DataFifoHdl = HI_DATAFIFO_INVALID_HANDLE;
    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    return HI_SUCCESS;
}

HI_VOID MAPI_VENC_ClearDatafifoData(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;

    if (g_astVDatafifo[VencHdl].pFnVencHisiLinkDataProc != HI_NULL
        && MAPI_VENC_GetDatafifoStatus(VencHdl) == HI_TRUE) {
        s32Ret = g_astVDatafifo[VencHdl].pFnVencHisiLinkDataProc(VencHdl, HI_NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_ClearDatafifoData fail,s32Ret:%x\n", s32Ret);
        }
    }
}

HI_S32 MAPI_VENC_StartDataInfo(HI_HANDLE VencHdl)
{
    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    g_astVDatafifo[VencHdl].bStart = HI_TRUE;
    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    return HI_SUCCESS;
}

HI_BOOL MAPI_VENC_StopDataInfo(HI_HANDLE VencHdl)
{
    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    g_astVDatafifo[VencHdl].bStart = HI_FALSE;
    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    return HI_SUCCESS;
}

HI_VOID MAPI_VENC_GetDataInfo(HI_HANDLE VencHdl, MAPI_VENC_LINKDATA_INFO_S *pstLinkDataInfo)
{
    pstLinkDataInfo->u32LinkFrmNum = g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LinkFrmNum;
    pstLinkDataInfo->u32LastFrmSeq = g_astVDatafifo[VencHdl].stVLinkDataInfo.u32LastFrmSeq;
    return;
}

HI_S32 MAPI_VENC_ClearDatafifoFrame(HI_HANDLE VencHdl)
{
    HI_S32 s32Ret;
    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);

    if (g_astVDatafifo[VencHdl].stUnhandleStream.pPriv != NULL) {
        s32Ret = MAPI_VENC_Release_VStream(VencHdl, &g_astVDatafifo[VencHdl].stUnhandleStream);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC,
                           "MAPI_VENC_Release_VStream fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }
        memset(&g_astVDatafifo[VencHdl].stUnhandleStream, 0, sizeof(MAPI_VENC_HISILINK_STREAM_S));
    }

    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VENC_Link_VStreamProcess(MAPI_VENC_HISILINK_STREAM_S *pstVencLinkStream,
                                            const MAPI_VENC_CHN_S *pstVencChn)
{
    HI_S32 s32Ret;
    HI_U32 i = 0;
    /* if datafifo is full, we will try 10 times at most, then transfer mutex */
    HI_U32 u32MaxLoopCnt = 10;

    do {
        s32Ret = g_astVDatafifo[pstVencChn->VencHandle].pFnVencHisiLinkDataProc(pstVencChn->VencHandle,
            pstVencLinkStream);
        if (s32Ret == HI_SUCCESS) {
            break;
        }

        g_astVDatafifo[pstVencChn->VencHandle].pFnVencHisiLinkDataProc(pstVencChn->VencHandle, HI_NULL);
        usleep(VENC_PROCESS_STREAM_USLEEP_TIME);
        i++;
    } while (i < u32MaxLoopCnt);

    if (s32Ret == HI_SUCCESS) {
        g_astVDatafifo[pstVencChn->VencHandle].stVLinkDataInfo.u32LinkFrmNum++;
        g_astVDatafifo[pstVencChn->VencHandle].stVLinkDataInfo.u32LastFrmSeq = pstVencLinkStream->stVencData.u32Seq;
    }

    return s32Ret;
}

static HI_VOID MAPI_VENC_Link_ConfigVStreamInfo(HI_HANDLE VencHdl,
    MAPI_VENC_HISILINK_STREAM_S *pstOutLINK_VStream, PAYLOAD_TYPE_E enVencType, VENC_STREAM_S *pstVStream,
    const VENC_CHN_STATUS_S *pstVencChnStat)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    pstOutLINK_VStream->stVencData.u32PackCount = pstVStream->u32PackCount;
    pstOutLINK_VStream->stVencData.u32Seq = pstVStream->u32Seq;
    pstOutLINK_VStream->handle = VencHdl;
    pstOutLINK_VStream->pPriv = (HI_VOID *)pstVStream;

    MAPI_VENC_GetStreamInfo(enVencType, pstVStream, &pstOutLINK_VStream->stVencData.stStreamInfo);

    if (((pstVencChnStat->u32LeftPics == 0 && pstVencChnStat->u32LeftStreamFrames == 1 &&
            pstVencChnStat->u32LeftEncPics == 0 &&
            pstVencChnStat->bJpegSnapEnd == HI_TRUE) /* for snap, bJpegSnapEnd come from vi. */
          || (pstVencChnStat->u32LeftPics == 0 && pstVencChnStat->u32LeftStreamFrames == 1 &&
              pstVencChnStat->u32LeftEncPics == 0 && pstVencChnStat->u32LeftRecvPics == 0)) /* for video THM */
        && HI_MAPI_VENC_LIMITLESS_FRAME_COUNT != MAPI_VENC_GetStartFrameCnt(VencHdl)) {
        pstOutLINK_VStream->stVencData.bEndOfStream = HI_TRUE;
        s32Ret = HI_MPI_VENC_StopRecvFrame(VencHdl);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_StopRecvFrame fail,VencHdl:%d,s32Ret:%x\n", VencHdl, s32Ret);
        }
    } else {
        pstOutLINK_VStream->stVencData.bEndOfStream = HI_FALSE;
    }

    for (i = 0; i < pstVStream->u32PackCount; i++) {
        pstOutLINK_VStream->stVencData.astPack[i].apu8Addr[0] = pstVStream->pstPack[i].pu8Addr;
        pstOutLINK_VStream->stVencData.astPack[i].au32Len[0] = pstVStream->pstPack[i].u32Len;
        pstOutLINK_VStream->stVencData.astPack[i].au64PhyAddr[0] = pstVStream->pstPack[i].u64PhyAddr;
        pstOutLINK_VStream->stVencData.astPack[i].apu8Addr[1] = HI_NULL;
        pstOutLINK_VStream->stVencData.astPack[i].au64PhyAddr[1] = HI_NULL;
        pstOutLINK_VStream->stVencData.astPack[i].au32Len[1] = 0;
        pstOutLINK_VStream->stVencData.astPack[i].u32Offset = pstVStream->pstPack[i].u32Offset;
        pstOutLINK_VStream->stVencData.astPack[i].u64PTS = pstVStream->pstPack[i].u64PTS;
        MAPI_VENC_GetPacketType(enVencType, pstVStream->pstPack[i].DataType,
                                &pstOutLINK_VStream->stVencData.astPack[i].stDataType);
    }
}

HI_S32 MAPI_VENC_GetVStream(HI_HANDLE VencHdl, MAPI_VENC_CHN_S *pstVencChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_U32 u32FrameCount = 0;
    VENC_CHN_STATUS_S stVencChnStat; /* Encoding channel status */
    VENC_STREAM_S *pstVStream = HI_NULL;
    MAPI_VENC_HISILINK_STREAM_S stOutLINK_VStream;
    VENC_CHN_ATTR_S stVencChnAttr;
    MUTEX_LOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);

    if (g_astVDatafifo[VencHdl].bStart == HI_FALSE || g_astVDatafifo[VencHdl].pFnVencHisiLinkDataProc == NULL) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VENC, "datafifo[%d] is not start or pFnVencHisiLinkDataProc is null\n", VencHdl);
        MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
        return HI_FAILURE;
    }

    /* for the quick start process */
    if (MAPI_VENC_GetDatafifoStatus(VencHdl) != HI_TRUE) {
        MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
        return HI_SUCCESS;
    }

    if (g_astVDatafifo[VencHdl].stUnhandleStream.pPriv == NULL) {
        s32Ret = HI_MPI_VENC_GetChnAttr(VencHdl, &stVencChnAttr);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetChnAttr fail,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }

        s32Ret = HI_MPI_VENC_QueryStatus(VencHdl, &stVencChnStat);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "query channel status error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }

        if (stVencChnStat.u32CurPacks == 0 || stVencChnStat.u32LeftStreamFrames == 0) {
            MAPI_WARN_TRACE(HI_MAPI_MOD_VENC, "query channel status u32CurPacks is 0 or leftStreamFrames is 0\n");
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return HI_SUCCESS;
        }

        if (stVencChnStat.u32CurPacks > HI_MAPI_VENC_MAX_FRAME_PACKCOUNT) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "stVencChnStat.u32CurPacks:%u >  HI_MAPI_VENC_MAX_FRAME_PACKCOUNT\n",
                           stVencChnStat.u32CurPacks);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return HI_FAILURE;
        }

        pstVStream = (VENC_STREAM_S *)malloc(sizeof(VENC_STREAM_S));

        if (pstVStream == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "malloc fail, pstVStream is null\n");
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return HI_FAILURE;
        }

        pstVStream->u32PackCount = stVencChnStat.u32CurPacks;
        pstVStream->pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stVencChnStat.u32CurPacks);

        if (pstVStream->pstPack == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "malloc fail, pstVStream->pstPack is null\n");
            free(pstVStream);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VENC_GetStream(VencHdl, pstVStream, HI_FALSE);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "get Stream error,s32Ret:%x\n", s32Ret);
            free(pstVStream->pstPack);
            pstVStream->pstPack = HI_NULL;
            free(pstVStream);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }

        if (pstVStream->u32PackCount == 0) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "pstVStream->u32PackCount:%d\n", pstVStream->u32PackCount);
            free(pstVStream->pstPack);
            free(pstVStream);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return HI_SUCCESS;
        }

        if (!g_astVDatafifo[VencHdl].bFirstFrame) {
            g_astVDatafifo[VencHdl].bFirstFrame = HI_TRUE;
            struct timeval tv;
            gettimeofday(&tv, 0);
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "[#### VencHdl:%d the first time of VideoFrame(MAPI_VENC_GetLink_VStream): time: [%ld][%ld]\n",
                            VencHdl, tv.tv_sec, tv.tv_usec);
            MAPI_INFO_TRACE(HI_MAPI_MOD_VENC,
                            "[#### VencHdl:%d the first time of VideoFrame(MAPI_VENC_GetLink_VStream): u64PTS: [%llu]\n",
                            VencHdl, pstVStream->pstPack[0].u64PTS);
        }

        MAPI_VENC_Link_ConfigVStreamInfo(VencHdl, &stOutLINK_VStream, stVencChnAttr.stVencAttr.enType, pstVStream,
                                         &stVencChnStat);
        s32Ret = MAPI_VENC_Link_VStreamProcess(&stOutLINK_VStream, pstVencChn);

        if (s32Ret != HI_SUCCESS) {
            memcpy(&g_astVDatafifo[VencHdl].stUnhandleStream, &stOutLINK_VStream, sizeof(MAPI_VENC_HISILINK_STREAM_S));
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_Link_VStreamProcess error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }
    } else {
        /* there is unhandled stream last time */
        s32Ret = MAPI_VENC_Link_VStreamProcess(&g_astVDatafifo[VencHdl].stUnhandleStream, pstVencChn);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_Link_VStreamProcess error,s32Ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
            return s32Ret;
        }
    }

    if (g_astVDatafifo[VencHdl].stUnhandleStream.pPriv != NULL) {
        memset(&g_astVDatafifo[VencHdl].stUnhandleStream, 0, sizeof(MAPI_VENC_HISILINK_STREAM_S));
    }

    MUTEX_UNLOCK(g_astVDatafifo[VencHdl].m_DatafifoChn_lock);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
