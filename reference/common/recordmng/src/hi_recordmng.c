/**
* @file    hi_recordmng.c
* @brief   implementation of record manager interface
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
* @version   1.0

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "hi_appcomm.h"
#include "hi_recordmng.h"

#include "hi_recorder_pro.h"
#include "hi_eventhub.h"
#if defined(CONFIG_RECORDMNG_MP4)
#include "hi_mp4_format.h"
#endif

#if defined(CONFIG_RECORDMNG_TS)
#include "hi_ts_format.h"
#endif

#include "hi_mapi_venc.h"
#include "hi_mapi_aenc.h"
#include "mpi_sys.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "RecMng"


/** maximum stream count in specified file */
#define RECMNG_FILE_MAX_STREAM_CNT (2)

/** maximum track count in specified stream */
#define RECMNG_STREAM_MAX_TRACK_CNT (3)

#define RECMNG_MP4_VIDEO_TIME_SCALE (120000)

#define RECMNG_AUDIO_SLICE_COUNT (1)
#define RECMNG_PRIV_DATA_SLICE_COUNT (1)

#define RECMNG_FWRITE_RETRY_TIMES (200)

/** check module init state */
#define RECMNG_CHECK_INIT() \
    do{ \
        if(!s_bRECMNGInitFlg) \
        { \
            MLOGE("Module not init yet\n"); \
            return HI_RECMNG_ENOINIT; \
        } \
    }while(0)

/** check task index(handle) created or not */
#define RECMNG_CHECK_USED(idx) \
    do{ \
        if(!s_astRECMNGTaskCtx[idx].bUsed) \
        { \
            MLOGE("Task handle[%d] has already destroyed or not created yet\n", idx); \
            return HI_RECMNG_EINVAL; \
        } \
    }while(0)

/** check task index(handle) invalid or not */
#define RECMNG_CHECK_TSKINDEX(idx) \
    do{ \
        if(idx < 0 || idx >= HI_RECMNG_MAX_TASK_CNT) \
        { \
            MLOGE("Invalid Task handle[%d]\n", idx); \
            return HI_RECMNG_EINVAL; \
        } \
    }while(0)


/** track source information */
typedef struct tagRECMNG_TRACK_SOURCE_S
{
    HI_MW_PTR* ppRecObj; /**<record object */
    HI_BOOL bStart;
    HI_BOOL bInsertFrame;
    HI_U32 u32FrameCount;
    HI_U32 u32FrameInterval; /**<us */
    HI_U64 u64ContrastTimeStamp;
    HI_U64 u64RemainingTime; /**<us */
    HI_Track_Source_S stTrackSource;
} RECMNG_TRACK_SOURCE_S;

/** mmz buffer type information */
typedef struct hiRECMNG_MMZ_BUF_INFO_S
{
    HI_CHAR szBufName[HI_APPCOMM_COMM_STR_LEN];
    HI_U64 u64PhyAddr;
} HI_RECMNG_MMZ_BUF_INFO_S;

/** os buffer type information */
typedef struct hiRECMNG_OS_BUF_INFO_S
{
    /**<reserved */
} HI_RECMNG_OS_BUF_INFO_S;

typedef struct tagRECMNG_BUF_INFO_S
{
    HI_RECMNG_BUF_TYPE_E enBufType;
    union tagBUF_INFO_U
    {
        HI_RECMNG_MMZ_BUF_INFO_S stMmzBufInfo; /**<mmz buffer information */
        HI_RECMNG_OS_BUF_INFO_S stOsBufInfo; /**<os buffer information */
    } unBufInfo;
} RECMNG_BUF_INFO_S;

typedef struct tagRECMNG_STREAM_INFO_S
{
    HI_U32 u32TrackCnt; /**<track count in specified stream, should not beyond RECMNG_STREAM_MAX_TRACK_CNT */
    RECMNG_TRACK_SOURCE_S astTrackSource[RECMNG_STREAM_MAX_TRACK_CNT]; /**<track source information */
    RECMNG_BUF_INFO_S stBufInfo;
} RECMNG_STREAM_INFO_S;

typedef struct tagRECMNG_MUXER_INFO_S
{
    HI_MW_PTR pMuxerObj; /**<muxer object, valid between file begin and file end */
    HI_BOOL bAsyncDestroy;
    HI_RECMNG_MUXER_TYPE_E enMuxerType; /**<muxer type, eg. mp4/mov/ts */
    union tagMUXER_CFG
    {
#if defined(CONFIG_RECORDMNG_MP4)
        HI_MP4_CONFIG_S stMp4Cfg; /**<mp4/mov configure */
#endif
#if defined(CONFIG_RECORDMNG_TS)
        HI_TS_CONFIG_S stTsCfg; /**<ts muxer configre */
#endif
    } unMuxerCfg;
    pthread_mutex_t muxerMutex;
} RECMNG_MUXER_INFO_S;

/** record task stream information */
typedef struct tagRECMNG_FILE_INFO_S
{
    HI_U32 u32StreamCnt; /**<stream count in specified file, should not beyond RECMNG_FILE_MAX_STREAM_CNT */
    RECMNG_STREAM_INFO_S astStreamAttr[RECMNG_FILE_MAX_STREAM_CNT];
    HI_HANDLE ThmHdl; /**<thumbnail data source, venc handle, -1: not support */
    HI_MW_PTR pThmTrackObj; /**<thumbnail stream object : available only in ts muxer type */
    RECMNG_MUXER_INFO_S stMuxerCfg;
} RECMNG_FILE_INFO_S;

/** loop strategy manage information */
typedef struct tagRECMNG_LOOP_MNG_INFO_S
{
    HI_BOOL bLoop; /**<loop strategy enable or not */
    HI_U32  u32LoopNum; /**<loop file count */
    HI_U32  u32FileIdx; /**<the index of file used to count */
    HI_RECMNG_DEL_FILE_CALLBACK_FN_PTR pfnDelFile; /**<delete file callback */
    HI_U32  u32Idx; /**<index in loop queue */
    HI_CHAR aszFileName[HI_RECMNG_MAX_LOOP_NUM][HI_APPCOMM_MAX_FILENAME_LEN]; /**<loop record file name */
} RECMNG_LOOP_MNG_INFO_S;

/** record task status */
typedef enum tagRECMNG_TASK_STATE_E
{
    RECMNG_TASK_STATE_READY = 0, /**<created or receive stop event */
    RECMNG_TASK_STATE_STARTING,
    RECMNG_TASK_STATE_STARTED, /**<receive start event or manual split file end event */
    RECMNG_TASK_STATE_SPLITTING, /**<manual split */
    RECMNG_TASK_STATE_STOPPING,
    RECMNG_TASK_STATE_BUTT
} RECMNG_TASK_STATE_E;

/** record task context */
typedef struct tagRECMNG_TASK_CONTEXT_S
{
    HI_BOOL bUsed;
    HI_MW_PTR pRecObj; /**<record object, created by rec module, valid between create and destroy */

    HI_U32 u32FileCnt; /**<record file count, should not beyond HI_RECMNG_TASK_MAX_FILE_CNT */
    HI_U32 u32StreamCnt;
    RECMNG_FILE_INFO_S astFileInfo[HI_RECMNG_TASK_MAX_FILE_CNT]; /**<record file information */
    HI_RECMNG_GET_FILENAME_S stGetFilenameCb; /**<record  get filenames callbacks */

    HI_BOOL bStopAfterManualSplitEnd;

    HI_RECMNG_THM_TYPE_E enThmType; /**<thumbnail type, solo file or imbedded in record file */
    HI_RECMNG_THM_ATTR_S stThmAttr; /**<thumbnail attribute, include thumbnail data source */

    HI_U32 u32RecFileCnt; /**<record file count, increase when file begin and decrease when file end */
    /**<record filename and thumbnail filename, update when getfilename called */
    HI_RECMNG_FILENAME_S stFilename;

    RECMNG_LOOP_MNG_INFO_S stLoopMngInfo; /**<loop strategy manage information */

    RECMNG_TASK_STATE_E enTaskState; /**<task state */
    pthread_mutex_t stateMutex; /**<task state mutex */
    pthread_cond_t recEventCond; /**<recorder_pro event cond */
} RECMNG_TASK_CONTEXT_S;

static RECMNG_TASK_CONTEXT_S s_astRECMNGTaskCtx[HI_RECMNG_MAX_TASK_CNT];

 /** media operate */
static HI_RECMNG_MEDIA_OPERATE_S stRECMNGMediaOps;

/** recordmng init flag */
static HI_BOOL s_bRECMNGInitFlg = HI_FALSE;


static inline HI_VOID RECMNG_SetTaskState(HI_HANDLE Hdl, RECMNG_TASK_STATE_E enTaskState)
{
    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    s_astRECMNGTaskCtx[Hdl].enTaskState = enTaskState;
    HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
}

static HI_S32 RECMNG_GetTrackSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj, RECMNG_TRACK_SOURCE_S** ppstTrackSource)
{
    HI_APPCOMM_CHECK_POINTER(ppstTrackSource, HI_FAILURE);

    HI_U32 u32Idx;
    RECMNG_TASK_CONTEXT_S *pstTskCtx = NULL;
    for (u32Idx = 0; u32Idx < HI_RECMNG_MAX_TASK_CNT; ++u32Idx)
    {
        if (pRecObj == s_astRECMNGTaskCtx[u32Idx].pRecObj)
        {
            pstTskCtx = &s_astRECMNGTaskCtx[u32Idx];
            break;
        }
    }
    HI_APPCOMM_CHECK_POINTER(pstTskCtx, HI_FAILURE);

    HI_S32 s32I, s32J, s32K;
    for (s32I = 0; s32I < pstTskCtx->u32FileCnt; ++s32I)
    {
        for (s32J = 0; s32J < pstTskCtx->astFileInfo[s32I].u32StreamCnt; ++s32J)
        {
            for(s32K = 0; s32K < pstTskCtx->astFileInfo[s32I].astStreamAttr[s32J].u32TrackCnt; ++s32K)
            {
                if(pTrackSource == &pstTskCtx->astFileInfo[s32I].astStreamAttr[s32J].astTrackSource[s32K].stTrackSource)
                {
                    *ppstTrackSource = &pstTskCtx->astFileInfo[s32I].astStreamAttr[s32J].astTrackSource[s32K];
                    MLOGD("Task Idx[%d] File[%d] Stream[%d] TrackSource[%d] TrackSourceHdl[%p]\n",
                        u32Idx, s32I, s32J, s32K, pTrackSource);
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_FAILURE;
}

static HI_S32 RECMNG_GetFileInfoByMuxerObj(HI_MW_PTR pMuxerObj, HI_U32 *pu32TaskIdx, HI_U32 *pu32FileIdx)
{
    HI_U32 u32TaskIdx, u32FileIdx;

    for (u32TaskIdx = 0; u32TaskIdx < HI_RECMNG_MAX_TASK_CNT; ++u32TaskIdx)
    {
        if(HI_FALSE == s_astRECMNGTaskCtx[u32TaskIdx].bUsed)
        {
            continue;
        }

        for(u32FileIdx = 0; u32FileIdx < s_astRECMNGTaskCtx[u32TaskIdx].u32FileCnt; ++u32FileIdx)
        {
            if(pMuxerObj == s_astRECMNGTaskCtx[u32TaskIdx].astFileInfo[u32FileIdx].stMuxerCfg.pMuxerObj)
            {
                *pu32TaskIdx = u32TaskIdx;
                *pu32FileIdx = u32FileIdx;
                return HI_SUCCESS;
            }
        }
    }
    return HI_FAILURE;
}

static HI_S32 RECMNG_VencDataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S* pVStreamData, HI_VOID* pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pVStreamData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);

    HI_U32 u32Idx;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_REC_FRAME_DATA_S stFrameData;
    stFrameData.u32SliceCnt = 0;

    for(u32Idx = 0; u32Idx < pVStreamData->u32PackCount; ++u32Idx)
    {
        HI_U8 *pu8PackVirtAddr = pVStreamData->astPack[u32Idx].apu8Addr[0];
        HI_U64 u64PackPhyAddr = pVStreamData->astPack[u32Idx].au64PhyAddr[0];
        HI_U32 u32PackLen = pVStreamData->astPack[u32Idx].au32Len[0];
        HI_U32 u32PackOffset = pVStreamData->astPack[u32Idx].u32Offset;

        HI_U8 *pu8DataVirtAddr = pVStreamData->astPack[u32Idx].apu8Addr[1];
        HI_U64 u64DataPhyAddr = pVStreamData->astPack[u32Idx].au64PhyAddr[1];
        HI_U32 u32DataLen = pVStreamData->astPack[u32Idx].au32Len[1];

        if (u64PackPhyAddr + u32PackLen >= u64DataPhyAddr + u32DataLen)
        {
            /** physical address retrace in offset segment */
            if (u64PackPhyAddr + u32PackOffset >= u64DataPhyAddr + u32DataLen)
            {
                HI_U8 *pu8SrcVirtAddr = pu8DataVirtAddr
                    + ((u64PackPhyAddr + u32PackOffset) - (u64DataPhyAddr + u32DataLen));

                if(u32PackLen>u32PackOffset)
                {
                    stFrameData.apu8SliceAddr[stFrameData.u32SliceCnt] = pu8SrcVirtAddr;
                    stFrameData.au32SliceLen[stFrameData.u32SliceCnt] = u32PackLen - u32PackOffset;
                    stFrameData.u32SliceCnt++;
                }
            }
            else
            {
                /** physical address retrace in data segment */
                HI_U32 u32Left = (u64DataPhyAddr + u32DataLen) - u64PackPhyAddr;
                if(u32Left>u32PackOffset)
                {
                    stFrameData.apu8SliceAddr[stFrameData.u32SliceCnt] = pu8PackVirtAddr + u32PackOffset;
                    stFrameData.au32SliceLen[stFrameData.u32SliceCnt] = u32Left - u32PackOffset;
                    stFrameData.u32SliceCnt++;
                }
                if(u32PackLen>u32Left)
                {
                    stFrameData.apu8SliceAddr[stFrameData.u32SliceCnt] = pu8DataVirtAddr;
                    stFrameData.au32SliceLen[stFrameData.u32SliceCnt] = u32PackLen - u32Left;
                    stFrameData.u32SliceCnt++;
                }
            }
        }
        else
        {
            /** physical address retrace does not happen */
            if(u32PackLen>u32PackOffset)
            {
                stFrameData.apu8SliceAddr[stFrameData.u32SliceCnt] = pu8PackVirtAddr + u32PackOffset;
                stFrameData.au32SliceLen[stFrameData.u32SliceCnt] = u32PackLen - u32PackOffset;
                stFrameData.u32SliceCnt++;
            }
        }
    }

    u32Idx = pVStreamData->u32PackCount - 1;
    stFrameData.u64TimeStamp = pVStreamData->astPack[u32Idx].u64PTS;
    switch (pVStreamData->astPack[u32Idx].stDataType.enPayloadType)
    {
        case HI_MAPI_PAYLOAD_TYPE_H264:
            stFrameData.bKeyFrameFlag = (H264E_NALU_IDRSLICE == pVStreamData->astPack[u32Idx].stDataType.enH264EType
                || H264E_NALU_ISLICE == pVStreamData->astPack[u32Idx].stDataType.enH264EType
                || H264E_NALU_SPS == pVStreamData->astPack[u32Idx].stDataType.enH264EType) ? HI_TRUE : HI_FALSE;
            break;

        case HI_MAPI_PAYLOAD_TYPE_H265:
            stFrameData.bKeyFrameFlag = (H265E_NALU_IDRSLICE == pVStreamData->astPack[u32Idx].stDataType.enH265EType
                || H265E_NALU_ISLICE == pVStreamData->astPack[u32Idx].stDataType.enH265EType
                || H265E_NALU_SPS == pVStreamData->astPack[u32Idx].stDataType.enH265EType) ? HI_TRUE : HI_FALSE;
            break;

        default:
            stFrameData.bKeyFrameFlag = HI_FALSE;
            break;
    }

    RECMNG_TRACK_SOURCE_S *pstTrackSource = (RECMNG_TRACK_SOURCE_S*)pvPrivData;
    HI_APPCOMM_CHECK_POINTER(pstTrackSource->ppRecObj, HI_FAILURE);

    s32Ret = HI_REC_WriteData(*pstTrackSource->ppRecObj, &pstTrackSource->stTrackSource, &stFrameData);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(pstTrackSource->bInsertFrame)
    {
        if(pstTrackSource->u64ContrastTimeStamp != 0)
        {
            if((stFrameData.u64TimeStamp - pstTrackSource->u64ContrastTimeStamp + pstTrackSource->u64RemainingTime)
                >= (pstTrackSource->u32FrameCount + 1) * pstTrackSource->u32FrameInterval)
            {
                stFrameData.apu8SliceAddr[0] = HI_NULL;
                stFrameData.au32SliceLen[0] = 0;
                stFrameData.bKeyFrameFlag = HI_FALSE;
                stFrameData.u32SliceCnt = 1;

                s32Ret = HI_REC_WriteData(*pstTrackSource->ppRecObj, &pstTrackSource->stTrackSource, &stFrameData);

                HI_U64 u64WriteFrameCnt = (stFrameData.u64TimeStamp - pstTrackSource->u64ContrastTimeStamp + pstTrackSource->u64RemainingTime)
                    / pstTrackSource->u32FrameInterval - pstTrackSource->u32FrameCount;
                pstTrackSource->u64RemainingTime = (stFrameData.u64TimeStamp - pstTrackSource->u64ContrastTimeStamp + pstTrackSource->u64RemainingTime)
                    - (pstTrackSource->u32FrameCount + u64WriteFrameCnt) * pstTrackSource->u32FrameInterval;
                pstTrackSource->u32FrameCount = 0;
                pstTrackSource->u64ContrastTimeStamp = stFrameData.u64TimeStamp;

                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            ++(pstTrackSource->u32FrameCount);
        }
        else
        {
            pstTrackSource->u32FrameCount = 1;
            pstTrackSource->u64ContrastTimeStamp = stFrameData.u64TimeStamp;
            pstTrackSource->u64RemainingTime = 0;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 RECMNG_AencDataProc(HI_HANDLE AencHdl, const AUDIO_STREAM_S *pAStreamData, HI_VOID *pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pAStreamData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pAStreamData->pStream, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);

    HI_REC_FRAME_DATA_S stFrameData;
    stFrameData.u32SliceCnt = RECMNG_AUDIO_SLICE_COUNT;
    stFrameData.u64TimeStamp = pAStreamData->u64TimeStamp;
    stFrameData.apu8SliceAddr[0] = pAStreamData->pStream;
    stFrameData.au32SliceLen[0] = pAStreamData->u32Len;
    stFrameData.bKeyFrameFlag = HI_FALSE;

    RECMNG_TRACK_SOURCE_S *pstTrackSource = (RECMNG_TRACK_SOURCE_S*)pvPrivData;
    HI_APPCOMM_CHECK_POINTER(pstTrackSource->ppRecObj, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(*pstTrackSource->ppRecObj, HI_FAILURE);

    return HI_REC_WriteData(*pstTrackSource->ppRecObj, &pstTrackSource->stTrackSource, &stFrameData);
}

static HI_S32 RECMNG_StartVideoSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32VencFrameCnt;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = HI_NULL;
   MLOGI("Lorin Add  ->>   Enter RECMNG_StartVideoSource\n");
    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /** LapseRecord && VencFrameRate >= 1/64 */
    if((pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed > 1)
        && (pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed / pTrackSource->unTrackSourceAttr.stVideoInfo.u32FrameRate <= 64))
    {
        if(HI_FALSE == pstTrackSource->bStart)
        {
            s32VencFrameCnt = HI_MAPI_VENC_LIMITLESS_FRAME_COUNT;
        }
        else
        {
            return HI_SUCCESS;
        }
    }
    /** LapseRecord && VencFrameRate < 1/64 */
    else if((pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed > 1)
        && (pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed / pTrackSource->unTrackSourceAttr.stVideoInfo.u32FrameRate > 64))
    {
        s32VencFrameCnt = 1;
    }
    /** NormalRecord */
    else
    {
        s32VencFrameCnt = HI_MAPI_VENC_LIMITLESS_FRAME_COUNT;
    }

    HI_MAPI_VENC_CALLBACK_S stVencCB;
    stVencCB.pfnDataCB = RECMNG_VencDataProc;
    stVencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_VENC_RegisterCallback(pTrackSource->s32PrivateHandle, &stVencCB);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MAPI_VEnc_RegisterCallback");

    if(HI_TRUE == pstTrackSource->bInsertFrame)
    {
        pstTrackSource->u32FrameCount = 0;
        pstTrackSource->u64ContrastTimeStamp = 0;
        pstTrackSource->u64RemainingTime = 0;
    }

    MLOGD("Venchdl[%d] TrackSourcehdl[%p][%p]\n", pTrackSource->s32PrivateHandle, pTrackSource, &pstTrackSource->stTrackSource);
    s32Ret = stRECMNGMediaOps.pfnVencStart(pTrackSource->s32PrivateHandle, s32VencFrameCnt);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_VencStart");

    /** LapseRecord && VencFrameRate >= 1/64 */
    if((pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed > 1)
        && (pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed / pTrackSource->unTrackSourceAttr.stVideoInfo.u32FrameRate <= 64))
    {
        pstTrackSource->bStart = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_S32 RECMNG_StopVideoSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);
   MLOGI("Lorin Add2  ->>   Enter RECMNG_StopVideoSource\n");
    HI_S32 s32Ret = HI_SUCCESS;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = NULL;

    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = stRECMNGMediaOps.pfnVencStop(pTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_VencStop");

    /** LapseRecord && VencFrameRate >= 1/64 */
    if((pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed > 1)
        && (pTrackSource->unTrackSourceAttr.stVideoInfo.fSpeed / pTrackSource->unTrackSourceAttr.stVideoInfo.u32FrameRate <= 64))
    {
        pstTrackSource->bStart = HI_FALSE;
    }

    HI_MAPI_VENC_CALLBACK_S stVencCB;
    stVencCB.pfnDataCB = RECMNG_VencDataProc;
    stVencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    return HI_MAPI_VENC_UnRegisterCallback(pTrackSource->s32PrivateHandle, &stVencCB);
}

static HI_S32 RECMNG_StartAudioSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = NULL;

    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_MAPI_AENC_CALLBACK_S stAencCB;
    stAencCB.pfnDataCB = RECMNG_AencDataProc;
    stAencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_AENC_RegisterCallback(pTrackSource->s32PrivateHandle, &stAencCB);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MAPI_AEnc_RegisterCallback");

    MLOGD("Aenchdl[%d]\n", pTrackSource->s32PrivateHandle);
    s32Ret = stRECMNGMediaOps.pfnAencStart(pTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_AencStart");
    return HI_SUCCESS;
}

static HI_S32 RECMNG_StopAudioSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = NULL;

    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = stRECMNGMediaOps.pfnAencStop(pTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_AencStop");

    HI_MAPI_AENC_CALLBACK_S stAencCB;
    stAencCB.pfnDataCB = RECMNG_AencDataProc;
    stAencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    return HI_MAPI_AENC_UnregisterCallback(pTrackSource->s32PrivateHandle, &stAencCB);
}

static HI_S32 RECMNG_StartPrivDataSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);

    HI_S32 s32Ret;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = NULL;

    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = stRECMNGMediaOps.pfnPrivDataStart(pTrackSource->s32PrivateHandle, (HI_VOID *)pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_StopPrivDataSource(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pRecObj)
{
    HI_APPCOMM_CHECK_POINTER(pTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRecObj, HI_FAILURE);

    HI_S32 s32Ret;
    RECMNG_TRACK_SOURCE_S *pstTrackSource = NULL;

    s32Ret = RECMNG_GetTrackSource(pTrackSource, pRecObj, &pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = stRECMNGMediaOps.pfnPrivDataStop(pTrackSource->s32PrivateHandle, (HI_VOID *)pstTrackSource);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_GetVideoInfo(HI_HANDLE VencHdl, HI_Track_VideoSourceInfo_S* pstVideoInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoInfo, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MEDIA_VIDEOINFO_S stVideoInfo;
    s32Ret = stRECMNGMediaOps.pfnGetVideoInfo(VencHdl, &stVideoInfo);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);

    /** CodecType and RcParam */
    switch (stVideoInfo.stVencAttr.stTypeAttr.enType)
    {
        case HI_MAPI_PAYLOAD_TYPE_H264:
            pstVideoInfo->enCodecType = HI_TRACK_VIDEO_CODEC_H264;
            if (HI_MAPI_VENC_RC_MODE_CBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H264_CBR_S *pstCbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH264Cbr;
                pstVideoInfo->u32Gop = pstCbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstCbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstCbr->stAttr.u32BitRate << 10;
            }
            else if (HI_MAPI_VENC_RC_MODE_VBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H264_VBR_S *pstVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH264Vbr;
                pstVideoInfo->u32Gop = pstVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstVbr->stAttr.u32MaxBitRate << 10;
            }
            else if (HI_MAPI_VENC_RC_MODE_QVBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H264_QVBR_S *pstQVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH264QVbr;
                pstVideoInfo->u32Gop = pstQVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstQVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstQVbr->stAttr.u32TargetBitRate << 10;
            }
            else
            {
                MLOGE("Invalid Venc RC mode[%d]\n", stVideoInfo.stVencAttr.stRcAttr.enRcMode);
            }
            break;

        case HI_MAPI_PAYLOAD_TYPE_H265:
            pstVideoInfo->enCodecType = HI_TRACK_VIDEO_CODEC_H265;
            if (HI_MAPI_VENC_RC_MODE_CBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H265_CBR_S *pstCbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH265Cbr;
                pstVideoInfo->u32Gop = pstCbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstCbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstCbr->stAttr.u32BitRate << 10;
            }
            else if (HI_MAPI_VENC_RC_MODE_VBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H265_VBR_S *pstVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH265Vbr;
                pstVideoInfo->u32Gop = pstVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstVbr->stAttr.u32MaxBitRate << 10;
            }
            else if (HI_MAPI_VENC_RC_MODE_QVBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H265_QVBR_S *pstQVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH265QVbr;
                pstVideoInfo->u32Gop = pstQVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstQVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstQVbr->stAttr.u32TargetBitRate << 10;
            }
            else
            {
                MLOGE("Invalid Venc RC mode[%d]\n", stVideoInfo.stVencAttr.stRcAttr.enRcMode);
            }
            break;

        case HI_MAPI_PAYLOAD_TYPE_MJPEG:
            pstVideoInfo->enCodecType = HI_TRACK_VIDEO_CODEC_MJPEG;
            if (HI_MAPI_VENC_RC_MODE_CBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_MJPEG_CBR_S *pstCbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stMjpegCbr;
                pstVideoInfo->u32Gop = pstCbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstCbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstCbr->stAttr.u32BitRate << 10;
            }
            else if (HI_MAPI_VENC_RC_MODE_VBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_MJPEG_VBR_S *pstVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stMjpegVbr;
                pstVideoInfo->u32Gop = pstVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstVbr->stAttr.u32MaxBitRate << 10;
            }
            else
            {
                MLOGE("Invalid Venc RC mode[%d]\n", stVideoInfo.stVencAttr.stRcAttr.enRcMode);
            }
            break;

        default:
            MLOGE("Invalid Venc Payload Type[%d]\n", stVideoInfo.stVencAttr.stTypeAttr.enType);
            return HI_FAILURE;
    }
    MLOGI("CodecType[%d] RcMode[%d] FrameRate[%ufps] Gop[%u] BitRate[%ukbps]\n", pstVideoInfo->enCodecType,
        stVideoInfo.stVencAttr.stRcAttr.enRcMode, pstVideoInfo->u32FrameRate, pstVideoInfo->u32Gop, pstVideoInfo->u32BitRate >> 10);

    /** Resolution */
    pstVideoInfo->u32Width = stVideoInfo.stVencAttr.stTypeAttr.u32Width;
    pstVideoInfo->u32Height = stVideoInfo.stVencAttr.stTypeAttr.u32Height;
    MLOGI("Video Resolution[%ux%u]\n", pstVideoInfo->u32Width, pstVideoInfo->u32Height);

    /** PlayRate: default 1, canbe modified later */
    pstVideoInfo->fSpeed = 1;

    return HI_SUCCESS;
}

static HI_S32 RECMNG_GetAudioInfo(HI_HANDLE AencHdl, HI_Track_AudioSourceInfo_S* pstAudioInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstAudioInfo, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MEDIA_AUDIOINFO_S stAudioInfo;
    s32Ret = stRECMNGMediaOps.pfnGetAudioInfo(AencHdl, &stAudioInfo);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);

    /** CodecType */
    if (HI_MAPI_AUDIO_FORMAT_AACLC == stAudioInfo.enAencFormat)
    {
        pstAudioInfo->enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    }
    else
    {
        MLOGE("Invalid Aenc Format[%d]\n", stAudioInfo.enAencFormat);
        return HI_FAILURE;
    }
    MLOGD("CodecType[%d]\n", pstAudioInfo->enCodecType);

    if (HI_MAPI_AUDIO_SOUND_MODE_LEFT == stAudioInfo.enSoundMode
        || HI_MAPI_AUDIO_SOUND_MODE_RIGHT == stAudioInfo.enSoundMode)
    {
        pstAudioInfo->u32ChnCnt = 1;
    }
    else
    {
        pstAudioInfo->u32ChnCnt = 2;
    }
    pstAudioInfo->u32SampleRate = stAudioInfo.u32SampleRate;
    pstAudioInfo->u32AvgBytesPerSec = stAudioInfo.u32AvgBytesPerSec;
    pstAudioInfo->u32SamplesPerFrame = stAudioInfo.u32PtNumPerFrm;
    pstAudioInfo->u16SampleBitWidth = stAudioInfo.u16SampleBitWidth;

    MLOGD("ChnCnt[%u] SampleRate[%u] AvgBytesPerSec[%u] SamplesPerFrame[%u] SampleBitWidth[%u]\n",
        pstAudioInfo->u32ChnCnt, pstAudioInfo->u32SampleRate, pstAudioInfo->u32AvgBytesPerSec,
        pstAudioInfo->u32SamplesPerFrame, pstAudioInfo->u16SampleBitWidth);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_GetPrivDataInfo(HI_HANDLE PrivDataHdl, HI_Track_PrivateSourceInfo_S* pstPrivateInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstPrivateInfo, HI_FAILURE);

    HI_S32 s32Ret;
    HI_RECMNG_PRIV_DATA_INFO_S stPrivateInfo;
    s32Ret = stRECMNGMediaOps.pfnGetPrivDataCfg(PrivDataHdl, &stPrivateInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    pstPrivateInfo->u32PrivateData = 0;
    pstPrivateInfo->u32FrameRate = stPrivateInfo.u32FrameRate;
    pstPrivateInfo->u32BytesPerSec = stPrivateInfo.u32BytesPerSec;
    pstPrivateInfo->bStrictSync = stPrivateInfo.bStrictSync;

    return HI_SUCCESS;
}

HI_S32 RECMNG_AllocBuf(HI_VOID *pvPrivData, HI_U32 u32BufSize, HI_VOID** ppvVitAddr)
{
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppvVitAddr, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR((0 != u32BufSize), HI_FAILURE);

    HI_S32 s32Ret;
    RECMNG_BUF_INFO_S *pstBufInfo = (RECMNG_BUF_INFO_S*)pvPrivData;

    switch(pstBufInfo->enBufType)
    {
        case HI_RECMNG_BUF_TYPE_MMZ:
            s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&(pstBufInfo->unBufInfo.stMmzBufInfo.u64PhyAddr), ppvVitAddr,
                (const HI_CHAR*)pstBufInfo->unBufInfo.stMmzBufInfo.szBufName, (const HI_CHAR*)"anonymous", u32BufSize);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case HI_RECMNG_BUF_TYPE_OS:
            *ppvVitAddr = malloc(u32BufSize);
            if (HI_NULL == *ppvVitAddr)
            {
                MLOGE("malloc buf failed\n");
                return HI_FAILURE;
            }
            break;

        default:
            break;
    }

    memset(*ppvVitAddr, 0x0, u32BufSize);

    return HI_SUCCESS;

}

HI_S32 RECMNG_FreeBuf(HI_VOID *pvPrivData, HI_U32 u32BufSize, HI_VOID* pvVitAddr)
{
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvVitAddr, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR((0 != u32BufSize), HI_FAILURE);

    HI_S32 s32Ret;
    RECMNG_BUF_INFO_S *pstBufInfo = (RECMNG_BUF_INFO_S*)pvPrivData;

    switch(pstBufInfo->enBufType)
    {
        case HI_RECMNG_BUF_TYPE_MMZ:
            s32Ret = HI_MPI_SYS_MmzFree(pstBufInfo->unBufInfo.stMmzBufInfo.u64PhyAddr, pvVitAddr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case HI_RECMNG_BUF_TYPE_OS:
            free(pvVitAddr);
            break;

        default:
            break;
    }

    pvVitAddr = HI_NULL;

    return HI_SUCCESS;
}


static HI_S32 RECMNG_GetBufOps(const HI_RECMNG_BUF_CFG_S *pstBufCfg,
    RECMNG_BUF_INFO_S *pstBufInfo, HI_REC_BUF_ABSTRACTFUNC_S *pstBufOps)
{
    HI_APPCOMM_CHECK_POINTER(pstBufCfg, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pstBufOps, HI_FAILURE);

    memset(pstBufInfo, 0x0, sizeof(RECMNG_BUF_INFO_S));

    pstBufInfo->enBufType = pstBufCfg->enBufType;
    if(HI_RECMNG_BUF_TYPE_MMZ == pstBufCfg->enBufType)
    {
        memcpy(pstBufInfo->unBufInfo.stMmzBufInfo.szBufName, pstBufCfg->unBufCfg.stMmzBufCfg.szBufName, HI_APPCOMM_COMM_STR_LEN);
    }

    pstBufOps->pfnBufAlloc = RECMNG_AllocBuf;
    pstBufOps->pfnBufFree = RECMNG_FreeBuf;
    pstBufOps->pBufArg = (HI_VOID*)pstBufInfo;

    return HI_SUCCESS;
}

#if defined(CONFIG_RECORDMNG_MP4)
static HI_VOID RECMNG_DestroyMp4TrackAndMuxer(HI_MW_PTR pMuxerObj)
{
    HI_S32 s32Ret;

    s32Ret = HI_MP4_DestroyAllTracks(pMuxerObj, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MP4_DestroyAllTracks ret 0x%x\n", s32Ret);
    }

    s32Ret = HI_MP4_Destroy(pMuxerObj, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MP4_Destroy ret 0x%x\n", s32Ret);
    }

    return;
}
#endif

#if defined(CONFIG_RECORDMNG_TS)
static HI_VOID RECMNG_DestroyTsStreamAndMuxer(HI_MW_PTR pMuxerObj)
{
    HI_S32 s32Ret;

    s32Ret = HI_TS_DestroyAllStreams(pMuxerObj);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_TS_DestroyAllStreams ret 0x%x\n", s32Ret);
    }

    s32Ret = HI_TS_Destroy(pMuxerObj);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_TS_Destroy ret 0x%x\n", s32Ret);
    }

    return;
}
#endif

#ifdef RECMNG_SUPPORT_ASYNC_STOP
typedef struct tagRECMNG_Muxer_ASYNC_STOP_INFO_S
{
    HI_MW_PTR pMuxerObj;
    HI_RECMNG_MUXER_TYPE_E enMuxerType;
} RECMNG_Muxer_ASYNC_STOP_INFO_S;

typedef struct tagRECMNG_TASK_ASYNC_STOP_INFO_S
{
    HI_S32 s32MuxerCount;
    HI_S32 s32LastPublishMuxerCount;
    RECMNG_Muxer_ASYNC_STOP_INFO_S astMuxerInfo[HI_RECMNG_TASK_MAX_FILE_CNT];
} RECMNG_TASK_ASYNC_STOP_INFO_S;

typedef struct tagRECMNG_ASYNC_STOP_CONTEXT_S
{
    RECMNG_TASK_ASYNC_STOP_INFO_S astTaskInfo[HI_RECMNG_MAX_TASK_CNT];
    pthread_mutex_t muxerInfoMutex;
    pthread_t destroyMuxerThread;
    HI_BOOL bRunDestroyMuxerThread;
} RECMNG_ASYNC_STOP_CONTEXT_S;

static RECMNG_ASYNC_STOP_CONTEXT_S s_stAsyncStopContext;


static HI_VOID RECMNG_AddDestroyMuxerInfo(HI_HANDLE Hdl, RECMNG_Muxer_ASYNC_STOP_INFO_S *pstMuxerInfo)
{
    HI_S32 s32Ret;
    HI_MUTEX_LOCK(s_stAsyncStopContext.muxerInfoMutex);
    RECMNG_TASK_ASYNC_STOP_INFO_S *pstTaskInfo = &s_stAsyncStopContext.astTaskInfo[Hdl];
    if(pstTaskInfo->s32MuxerCount  < HI_RECMNG_TASK_MAX_FILE_CNT)
    {
        pstTaskInfo->astMuxerInfo[pstTaskInfo->s32MuxerCount].pMuxerObj = pstMuxerInfo->pMuxerObj;
        pstTaskInfo->astMuxerInfo[pstTaskInfo->s32MuxerCount].enMuxerType = pstMuxerInfo->enMuxerType;
        ++(pstTaskInfo->s32MuxerCount);

        if(pstTaskInfo->s32MuxerCount != pstTaskInfo->s32LastPublishMuxerCount)
        {
            HI_EVENT_S stEvent;
            stEvent.EventID = HI_EVENT_RECMNG_MUXER_CNT_CHANGE;
            stEvent.arg1 = Hdl;
            stEvent.arg2 = pstTaskInfo->s32MuxerCount;
            s32Ret = HI_EVTHUB_Publish(&stEvent);
            MLOGI("Add Muxer[%p], Publish MUXER_CNT_CHANGE %s, arg1[%d], arg2[%d]\n",
                pstMuxerInfo->pMuxerObj, ((HI_SUCCESS == s32Ret) ? "Success" : "Failed"), stEvent.arg1, stEvent.arg2);

            pstTaskInfo->s32LastPublishMuxerCount = pstTaskInfo->s32MuxerCount;
        }
    }
    HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);
}

static HI_VOID RECMNG_RemoveDestroyMuxerInfo(HI_HANDLE Hdl, HI_U32 u32MuxerInfoIndex)
{
    HI_U32 u32Index = 0;
    RECMNG_TASK_ASYNC_STOP_INFO_S *pstTaskInfo;

    if(u32MuxerInfoIndex >= 0 && u32MuxerInfoIndex < s_stAsyncStopContext.astTaskInfo[Hdl].s32MuxerCount)
    {
        pstTaskInfo = &s_stAsyncStopContext.astTaskInfo[Hdl];
        for(u32Index = u32MuxerInfoIndex; u32Index < (pstTaskInfo->s32MuxerCount - 1); ++u32Index)
        {
            pstTaskInfo->astMuxerInfo[u32Index].pMuxerObj = pstTaskInfo->astMuxerInfo[u32Index + 1].pMuxerObj;
            pstTaskInfo->astMuxerInfo[u32Index].enMuxerType = pstTaskInfo->astMuxerInfo[u32Index + 1].enMuxerType;
        }
        --(pstTaskInfo->s32MuxerCount);
    }
}

static HI_U32 RECMNG_GetMuxerCount(HI_VOID)
{
    HI_U32 u32TaskIndex = 0;
    HI_U32 u32MuxerCount = 0;

    for(u32TaskIndex = 0; u32TaskIndex < HI_RECMNG_MAX_TASK_CNT; ++u32TaskIndex)
    {
        u32MuxerCount += s_stAsyncStopContext.astTaskInfo[u32TaskIndex].s32MuxerCount;
    }
    return u32MuxerCount;
}

static HI_VOID* RECMNG_DestroyMuxerThread(HI_VOID* pData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Value = -1;
    HI_U32 u32TaskIndex = 0;
    RECMNG_Muxer_ASYNC_STOP_INFO_S stMuxerInfo;

    while(s_stAsyncStopContext.bRunDestroyMuxerThread)
    {
        HI_MUTEX_LOCK(s_stAsyncStopContext.muxerInfoMutex);
        if(0 == RECMNG_GetMuxerCount())
        {
            HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);
            HI_usleep(50000);
            continue;
        }
        HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);

        for(u32TaskIndex = 0; u32TaskIndex < HI_RECMNG_MAX_TASK_CNT; ++u32TaskIndex)
        {
            HI_MUTEX_LOCK(s_stAsyncStopContext.muxerInfoMutex);
            RECMNG_TASK_ASYNC_STOP_INFO_S *pstTaskInfo = &s_stAsyncStopContext.astTaskInfo[u32TaskIndex];
            if (pstTaskInfo->s32MuxerCount <= 0)
            {
                HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);
            }
            else
            {
                memcpy(&stMuxerInfo, &pstTaskInfo->astMuxerInfo[0], sizeof(RECMNG_Muxer_ASYNC_STOP_INFO_S));
                RECMNG_RemoveDestroyMuxerInfo(u32TaskIndex, 0);
                HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);

                if (HI_NULL_PTR != stMuxerInfo.pMuxerObj && HI_RECMNG_MUXER_BUTT != stMuxerInfo.enMuxerType)
                {
                    HI_U32 u32TaskIdx;
                    HI_U32 u32FileIdx;
                    RECMNG_FILE_INFO_S *pstFileInfo = HI_NULL;
                    s32Ret = RECMNG_GetFileInfoByMuxerObj(stMuxerInfo.pMuxerObj, &u32TaskIdx, &u32FileIdx);

                    switch(stMuxerInfo.enMuxerType)
                    {
#if defined(CONFIG_RECORDMNG_MP4)
                        case HI_RECMNG_MUXER_MP4:
                        case HI_RECMNG_MUXER_MOV:
                            if(s32Ret != HI_SUCCESS)
                            {   /* pMuxerObj may has been memseted when destroyTask after asyncStopTask */
                                RECMNG_DestroyMp4TrackAndMuxer(stMuxerInfo.pMuxerObj);
                            }
                            else
                            {
                                pstFileInfo = &s_astRECMNGTaskCtx[u32TaskIdx].astFileInfo[u32FileIdx];
                                HI_MUTEX_LOCK(pstFileInfo->stMuxerCfg.muxerMutex);
                                RECMNG_DestroyMp4TrackAndMuxer(stMuxerInfo.pMuxerObj);
                                pstFileInfo->stMuxerCfg.pMuxerObj = HI_NULL_PTR;
                                HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);
                            }
                            break;
#endif

#if defined(CONFIG_RECORDMNG_TS)
                        case HI_RECMNG_MUXER_TS:
                            if(s32Ret != HI_SUCCESS)
                            {
                                /* pMuxerObj may has been memseted when destroyTask after asyncStopTask */
                                RECMNG_DestroyTsStreamAndMuxer(stMuxerInfo.pMuxerObj);
                            }
                            else
                            {
                                pstFileInfo = &s_astRECMNGTaskCtx[u32TaskIdx].astFileInfo[u32FileIdx];
                                HI_MUTEX_LOCK(pstFileInfo->stMuxerCfg.muxerMutex);
                                RECMNG_DestroyTsStreamAndMuxer(stMuxerInfo.pMuxerObj);
                                pstFileInfo->stMuxerCfg.pMuxerObj = HI_NULL_PTR;
                                HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);
                            }
                            break;
#endif

                        default:
                            break;
                    }
                }

                HI_MUTEX_LOCK(s_stAsyncStopContext.muxerInfoMutex);
                if(pstTaskInfo->s32MuxerCount != pstTaskInfo->s32LastPublishMuxerCount)
                {
                    HI_EVENT_S stEvent;
                    stEvent.EventID = HI_EVENT_RECMNG_MUXER_CNT_CHANGE;
                    s32Value = RECMNG_GetMuxerCount();
                    stEvent.arg1 = (s32Value == 0 ? -1 : u32TaskIndex);
                    stEvent.arg2 = (s32Value == 0 ?  0 : pstTaskInfo->s32MuxerCount);
                    s32Ret = HI_EVTHUB_Publish(&stEvent);
                    MLOGI("Destroy Muxer[%p], Publish MUXER_CNT_CHANGE %s, arg1[%d], arg2[%d]\n",
                        stMuxerInfo.pMuxerObj, ((HI_SUCCESS == s32Ret) ? "Success" : "Failed"), stEvent.arg1, stEvent.arg2);

                    pstTaskInfo->s32LastPublishMuxerCount = pstTaskInfo->s32MuxerCount;
                }
                HI_MUTEX_UNLOCK(s_stAsyncStopContext.muxerInfoMutex);
                if(0 == s32Value)
                {
                    sync();
                }
            }
        }
    }
    return NULL;
}
#endif

#if defined(CONFIG_RECORDMNG_MP4)
static HI_S32 RECMNG_CreateMp4Muxer(HI_CHAR* pszFileName, HI_VOID* pMuxerInfo, HI_MW_PTR* ppMuxerObj)
{
    HI_APPCOMM_CHECK_POINTER(pszFileName, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pMuxerInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppMuxerObj, HI_FAILURE);

    HI_S32 s32Ret;

    RECMNG_MUXER_INFO_S *pstMuxerInfo = (RECMNG_MUXER_INFO_S*)pMuxerInfo;
    snprintf(pstMuxerInfo->unMuxerCfg.stMp4Cfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s", pszFileName);

    s32Ret =  HI_MP4_Create(ppMuxerObj, &pstMuxerInfo->unMuxerCfg.stMp4Cfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    pstMuxerInfo->pMuxerObj = *ppMuxerObj;

    return HI_SUCCESS;
}

static HI_S32 RECMNG_DestroyMp4Muxer(HI_MW_PTR pMuxerObj)
{
    HI_S32 s32Ret;
    HI_U32 u32TaskIdx;
    HI_U32 u32FileIdx;
    RECMNG_FILE_INFO_S *pstFileInfo = HI_NULL;

    s32Ret = RECMNG_GetFileInfoByMuxerObj(pMuxerObj, &u32TaskIdx, &u32FileIdx);
    if(s32Ret != HI_SUCCESS)
    {
        RECMNG_DestroyMp4TrackAndMuxer(pMuxerObj);
        return HI_SUCCESS;
    }

    pstFileInfo = &s_astRECMNGTaskCtx[u32TaskIdx].astFileInfo[u32FileIdx];
    HI_MUTEX_LOCK(pstFileInfo->stMuxerCfg.muxerMutex);

#ifdef RECMNG_SUPPORT_ASYNC_STOP
    if (HI_TRUE == pstFileInfo->stMuxerCfg.bAsyncDestroy)
    {
        pstFileInfo->stMuxerCfg.bAsyncDestroy = HI_FALSE;
        RECMNG_Muxer_ASYNC_STOP_INFO_S stMuxerInfo;
        stMuxerInfo.pMuxerObj = pMuxerObj;
        stMuxerInfo.enMuxerType = pstFileInfo->stMuxerCfg.enMuxerType;
        RECMNG_AddDestroyMuxerInfo(u32TaskIdx, &stMuxerInfo);
        MLOGI("Async destroy pMuxer[%p] in Task[%d] File[%d] later\n", pMuxerObj, u32TaskIdx, u32FileIdx);
        HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);
        return HI_SUCCESS;
    }
#endif

    RECMNG_DestroyMp4TrackAndMuxer(pMuxerObj);
    pstFileInfo->stMuxerCfg.pMuxerObj = HI_NULL_PTR;
    HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_CreateMp4MuxerTrack(HI_MW_PTR pMuxerObj, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* ppTrackObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSrcHandle, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppTrackObj, HI_FAILURE);

    HI_MP4_TRACK_INFO_S stTrackInfo = {};
    switch (pstTrackSrcHandle->enTrackType)
    {
        case HI_TRACK_SOURCE_TYPE_VIDEO:
        {
            switch (pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType)
            {
                case HI_TRACK_VIDEO_CODEC_H264:
                    stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
                    break;

                case HI_TRACK_VIDEO_CODEC_H265:
                    stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
                    break;

                case HI_TRACK_VIDEO_CODEC_MJPEG:
                    stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_MJPEG;
                    break;

                default:
                    MLOGD("Invalid track type[%d]\n", pstTrackSrcHandle->enTrackType);
                    break;
            }

            stTrackInfo.enTrackType = HI_MP4_STREAM_VIDEO;
            stTrackInfo.u32TimeScale = RECMNG_MP4_VIDEO_TIME_SCALE;
            stTrackInfo.fSpeed = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.fSpeed;
            stTrackInfo.stVideoInfo.u32Width = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Width;
            stTrackInfo.stVideoInfo.u32Height = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Height;
            stTrackInfo.stVideoInfo.u32BitRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32BitRate;
            if(stTrackInfo.fSpeed < 1) /** PlayFps < FrameRate, slow record */
            {
                stTrackInfo.stVideoInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate
                    * stTrackInfo.fSpeed;
            }
            else
            {
                stTrackInfo.stVideoInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;
            }
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");
            MLOGD("Create Video Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_AUDIO:
        {
            stTrackInfo.enTrackType = HI_MP4_STREAM_AUDIO;
            stTrackInfo.u32TimeScale = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
            stTrackInfo.fSpeed  = 1;
            stTrackInfo.stAudioInfo.u32Channels = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32ChnCnt;
            stTrackInfo.stAudioInfo.u32SampleRate = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
            stTrackInfo.stAudioInfo.u32SamplePerFrame = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame;
            stTrackInfo.stAudioInfo.u16SampleSize = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u16SampleBitWidth;
            stTrackInfo.stAudioInfo.enCodecID =
                (pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.enCodecType == HI_TRACK_AUDIO_CODEC_AAC)
                ? HI_MP4_CODEC_ID_AACLC : HI_MP4_CODEC_ID_BUTT;
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");
            MLOGD("Create Audio Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_PRIV:
        {
            stTrackInfo.enTrackType = HI_MP4_STREAM_DATA;
            stTrackInfo.u32TimeScale = RECMNG_MP4_VIDEO_TIME_SCALE;
            stTrackInfo.fSpeed = 1;
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon MetaData");
            stTrackInfo.stDataInfo.u32Width = 0;
            stTrackInfo.stDataInfo.u32Height = 0;
            stTrackInfo.stDataInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stPrivInfo.u32FrameRate;
            stTrackInfo.stDataInfo.enCodecID = HI_MP4_CODEC_ID_BUTT;
            MLOGD("Create private data track\n");
            break;
        }

        default:
            MLOGD("Invalid Track Type[%d]\n", pstTrackSrcHandle->enTrackType);
            return HI_FAILURE;
    }

    return HI_MP4_CreateTrack(pMuxerObj, ppTrackObj, &stTrackInfo);
}


static HI_S32 RECMNG_CreateMp4SlaveMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerInfo, HI_MW_PTR* ppMuxerObj)
{
    HI_APPCOMM_CHECK_POINTER(pszFileName, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pMuxerInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppMuxerObj, HI_FAILURE);

    *ppMuxerObj = ((RECMNG_MUXER_INFO_S*)pMuxerInfo)->pMuxerObj;

    return HI_SUCCESS;
}


static HI_S32 RECMNG_DestroyMp4SlaveMuxer(HI_MW_PTR pMuxerObj)
{
    return HI_SUCCESS;
}


static HI_S32 RECMNG_CreateMp4SlaveMuxerTrack(HI_MW_PTR pMuxerObj, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* ppTrackObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSrcHandle, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppTrackObj, HI_FAILURE);

    HI_MP4_TRACK_INFO_S stTrackInfo = {};
    switch (pstTrackSrcHandle->enTrackType)
    {
        case HI_TRACK_SOURCE_TYPE_VIDEO:
        {
            stTrackInfo.enTrackType = HI_MP4_STREAM_VIDEO;
            stTrackInfo.u32TimeScale = RECMNG_MP4_VIDEO_TIME_SCALE;
            stTrackInfo.fSpeed = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.fSpeed;
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

            stTrackInfo.stVideoInfo.u32Width = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Width;
            stTrackInfo.stVideoInfo.u32Height = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Height;
            stTrackInfo.stVideoInfo.u32BitRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32BitRate;
            stTrackInfo.stVideoInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;
            switch (pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType)
            {
                case HI_TRACK_VIDEO_CODEC_H264:
                    stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
                    break;

                case HI_TRACK_VIDEO_CODEC_H265:
                    stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
                    break;

                default:
                    MLOGD("Invalid track type[%d]\n", pstTrackSrcHandle->enTrackType);
                    break;
            }
            MLOGD("Create Video Slave Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_AUDIO:
        {
            stTrackInfo.enTrackType = HI_MP4_STREAM_AUDIO;
            stTrackInfo.u32TimeScale = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
            stTrackInfo.fSpeed  = 1;
            stTrackInfo.stAudioInfo.u32Channels = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32ChnCnt;
            stTrackInfo.stAudioInfo.u32SampleRate = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
            stTrackInfo.stAudioInfo.u32SamplePerFrame = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame;
            stTrackInfo.stAudioInfo.u16SampleSize = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u16SampleBitWidth;
            stTrackInfo.stAudioInfo.enCodecID =
                (pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.enCodecType == HI_TRACK_AUDIO_CODEC_AAC)
                ? HI_MP4_CODEC_ID_AACLC : HI_MP4_CODEC_ID_BUTT;
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");
            MLOGD("Create Audio Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_PRIV:
        {
            stTrackInfo.enTrackType = HI_MP4_STREAM_DATA;
            stTrackInfo.u32TimeScale = RECMNG_MP4_VIDEO_TIME_SCALE;
            stTrackInfo.fSpeed = 1;
            snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon MetaData");
            stTrackInfo.stDataInfo.u32Width = 0;
            stTrackInfo.stDataInfo.u32Height = 0;
            stTrackInfo.stDataInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stPrivInfo.u32FrameRate;
            stTrackInfo.stDataInfo.enCodecID = HI_MP4_CODEC_ID_BUTT;
            MLOGD("Create private data track\n");
            break;
        }

        default:
            MLOGD("Slave track type[%d] is not video\n", pstTrackSrcHandle->enTrackType);
            return HI_FAILURE;
    }

    return HI_MP4_CreateTrack(pMuxerObj, ppTrackObj, &stTrackInfo);
}


static HI_S32 RECMNG_WriteMp4MuxerFrame(HI_MW_PTR pMuxerObj, HI_MW_PTR pTrackObj, HI_REC_FRAME_DATA_S* pstFrameData)
{
    HI_APPCOMM_CHECK_POINTER(pstFrameData, HI_FAILURE);

    HI_MP4_FRAME_DATA_S stFrameData = {};
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;

    return HI_MP4_WriteFrame(pMuxerObj, pTrackObj, &stFrameData);
}
#endif

#if defined(CONFIG_RECORDMNG_TS)
static HI_S32 RECMNG_CreateTsMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerInfo, HI_MW_PTR* ppMuxerObj)
{
    HI_APPCOMM_CHECK_POINTER(pszFileName, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pMuxerInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppMuxerObj, HI_FAILURE);

    HI_S32 s32Ret;
    RECMNG_MUXER_INFO_S *pstMuxerInfo = (RECMNG_MUXER_INFO_S*)pMuxerInfo;
    snprintf(pstMuxerInfo->unMuxerCfg.stTsCfg.aszFileName, HI_TS_MAX_FILE_NAME, "%s", pszFileName);

    s32Ret =  HI_TS_Create(&pstMuxerInfo->unMuxerCfg.stTsCfg, ppMuxerObj);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    pstMuxerInfo->pMuxerObj = *ppMuxerObj;

    return HI_SUCCESS;
}

static HI_S32 RECMNG_DestroyTsMuxer(HI_MW_PTR pMuxerObj)
{
    HI_S32 s32Ret;
    HI_U32 u32TaskIdx;
    HI_U32 u32FileIdx;
    RECMNG_FILE_INFO_S *pstFileInfo = HI_NULL;

    s32Ret = RECMNG_GetFileInfoByMuxerObj(pMuxerObj, &u32TaskIdx, &u32FileIdx);
    if(s32Ret != HI_SUCCESS)
    {
        RECMNG_DestroyTsStreamAndMuxer(pMuxerObj);
        return HI_SUCCESS;
    }

    pstFileInfo = &s_astRECMNGTaskCtx[u32TaskIdx].astFileInfo[u32FileIdx];
    HI_MUTEX_LOCK(pstFileInfo->stMuxerCfg.muxerMutex);

#ifdef RECMNG_SUPPORT_ASYNC_STOP
    if (HI_TRUE == pstFileInfo->stMuxerCfg.bAsyncDestroy)
    {
        pstFileInfo->stMuxerCfg.bAsyncDestroy = HI_FALSE;
        RECMNG_Muxer_ASYNC_STOP_INFO_S stMuxerInfo;
        stMuxerInfo.pMuxerObj = pMuxerObj;
        stMuxerInfo.enMuxerType = pstFileInfo->stMuxerCfg.enMuxerType;
        RECMNG_AddDestroyMuxerInfo(u32TaskIdx, &stMuxerInfo);
        MLOGI("Async destroy pMuxer[%p] in Task[%d] File[%d] later\n", pMuxerObj, u32TaskIdx, u32FileIdx);
        HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);
        return HI_SUCCESS;
    }
#endif

    RECMNG_DestroyTsStreamAndMuxer(pMuxerObj);
    pstFileInfo->stMuxerCfg.pMuxerObj = HI_NULL_PTR;
    HI_MUTEX_UNLOCK(pstFileInfo->stMuxerCfg.muxerMutex);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_CreateTsMuxerTrack(HI_MW_PTR pMuxerObj, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* ppTrackObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSrcHandle, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppTrackObj, HI_FAILURE);

    HI_TS_STREAM_INFO_S stTrackInfo = {};
    switch (pstTrackSrcHandle->enTrackType)
    {
        case HI_TRACK_SOURCE_TYPE_VIDEO:
        {
            switch (pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType)
            {
                case HI_TRACK_VIDEO_CODEC_H264:
                    stTrackInfo.enCodecID = HI_TS_CODEC_ID_H264;
                    break;

                case HI_TRACK_VIDEO_CODEC_H265:
                    stTrackInfo.enCodecID = HI_TS_CODEC_ID_H265;
                    break;

                case HI_TRACK_VIDEO_CODEC_MJPEG:
                    stTrackInfo.enCodecID = HI_TS_CODEC_ID_JPEG;
                    break;

                default:
                    MLOGD("Invalid track type[%d]\n", pstTrackSrcHandle->enTrackType);
                    break;
            }
            MLOGD("Create Video Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_AUDIO:
        {
            stTrackInfo.enCodecID =
                (pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.enCodecType == HI_TRACK_AUDIO_CODEC_AAC)
                ? HI_TS_CODEC_ID_AAC : HI_TS_CODEC_ID_BUTT;
            MLOGD("Create Audio Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_PRIV:
        {
            stTrackInfo.enCodecID = HI_TS_CODEC_ID_BUTT;
            MLOGD("Create private data track\n");
            break;
        }

        default:
            MLOGD("Invalid Track Type[%d]\n", pstTrackSrcHandle->enTrackType);
            break;
    }

    return HI_TS_CreateStream(pMuxerObj, ppTrackObj, &stTrackInfo);
}

static HI_S32 RECMNG_CreateTsSlaveMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerInfo, HI_MW_PTR* ppMuxerObj)
{
    HI_APPCOMM_CHECK_POINTER(pszFileName, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pMuxerInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppMuxerObj, HI_FAILURE);

    *ppMuxerObj = ((RECMNG_MUXER_INFO_S*)pMuxerInfo)->pMuxerObj;

    return HI_SUCCESS;
}

static HI_S32 RECMNG_DestroyTsSlaveMuxer(HI_MW_PTR pTsMuxerObj)
{
    return HI_SUCCESS;
}

static HI_S32 RECMNG_CreateTsSlaveMuxerTrack(HI_MW_PTR pMuxerObj, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* ppTrackObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSrcHandle, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ppTrackObj, HI_FAILURE);

    HI_TS_STREAM_INFO_S stTrackInfo = {};
    switch (pstTrackSrcHandle->enTrackType)
    {
        case HI_TRACK_SOURCE_TYPE_VIDEO:
        {
            switch (pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType)
            {
                case HI_TRACK_VIDEO_CODEC_H264:
                    stTrackInfo.enCodecID = HI_TS_CODEC_ID_H264;
                    break;

                case HI_TRACK_VIDEO_CODEC_H265:
                    stTrackInfo.enCodecID = HI_TS_CODEC_ID_H265;
                    break;

                default:
                    MLOGD("Invalid codec type[%d]\n", pstTrackSrcHandle->enTrackType);
                    break;
            }
            MLOGD("Create Video Slave Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_AUDIO:
        {
            stTrackInfo.enCodecID =
                (pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.enCodecType == HI_TRACK_AUDIO_CODEC_AAC)
                ? HI_TS_CODEC_ID_AAC : HI_TS_CODEC_ID_BUTT;
            MLOGD("Create Audio Track\n");
            break;
        }

        case HI_TRACK_SOURCE_TYPE_PRIV:
        {
            stTrackInfo.enCodecID = HI_TS_CODEC_ID_BUTT;
            MLOGD("Create private data track\n");
            break;
        }

        default:
            MLOGD("Track type[%d] is not video\n", pstTrackSrcHandle->enTrackType);
            break;
    }

    return HI_TS_CreateStream(pMuxerObj, ppTrackObj, &stTrackInfo);
}

static HI_S32 RECMNG_WriteTsMuxerFrame(HI_MW_PTR pMuxerObj, HI_MW_PTR pTrackObj, HI_REC_FRAME_DATA_S* pstFrameData)
{
    HI_APPCOMM_CHECK_POINTER(pstFrameData, HI_FAILURE);

    HI_TS_FRAME_DATA_S stFrameData = {};
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;

    return HI_TS_WriteFrame(pMuxerObj, pTrackObj, &stFrameData);
}
#endif

static HI_S32 RECMNG_GetMuxerConfig(const HI_RECMNG_MUXER_CFG_S *pstMuxerCfg, RECMNG_MUXER_INFO_S *pstMuxerInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstMuxerCfg, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pstMuxerInfo, HI_FAILURE);

    MLOGD("muxer type[%d][0:MP4,1:MOV,2:TS]\n", pstMuxerCfg->enMuxerType);
    switch (pstMuxerCfg->enMuxerType)
    {
#if defined(CONFIG_RECORDMNG_MP4)
        case HI_RECMNG_MUXER_MP4:
        case HI_RECMNG_MUXER_MOV:
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.enConfigType = HI_MP4_CONFIG_MUXER;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32VBufSize = pstMuxerCfg->u32VBufSize;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32PreAllocUnit = pstMuxerCfg->u32PreAllocUnit;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.bCo64Flag = HI_FALSE;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.bConstantFps = pstMuxerCfg->bConstantFps;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32BackupUnit = pstMuxerCfg->unMuxerCfg.stMp4Cfg.u32RepairUnit;
            pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.enFormatProfile = HI_MP4_FORMAT_MP42;

            MLOGD("VBufsize[%uBytes], PreAllocUnit[%uBytes], BackupUnit[%uBytes]\n",
                pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32VBufSize,
                pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32PreAllocUnit,
                pstMuxerInfo->unMuxerCfg.stMp4Cfg.stMuxerConfig.u32BackupUnit);
            break;
#endif

#if defined(CONFIG_RECORDMNG_TS)
        case HI_RECMNG_MUXER_TS:
            pstMuxerInfo->unMuxerCfg.stTsCfg.enConfigType = HI_TS_CONFIG_MUXER;
            pstMuxerInfo->unMuxerCfg.stTsCfg.stMuxerConfig.u32VBufSize = pstMuxerCfg->u32VBufSize;
            pstMuxerInfo->unMuxerCfg.stTsCfg.stMuxerConfig.u32PreAllocUnit = pstMuxerCfg->u32PreAllocUnit;

            MLOGD("VBufsize[%uBytes], PreAllocUnit[%uBytes]\n",
                pstMuxerInfo->unMuxerCfg.stTsCfg.stMuxerConfig.u32VBufSize,
                pstMuxerInfo->unMuxerCfg.stTsCfg.stMuxerConfig.u32PreAllocUnit);
            break;
#endif
        default:
            MLOGE("Invalid muxer type[%d]\n", pstMuxerCfg->enMuxerType);
            return HI_FAILURE;
    }

    pstMuxerInfo->enMuxerType = pstMuxerCfg->enMuxerType;
    return HI_SUCCESS;
}

static HI_S32 RECMNG_GetMuxerOps(HI_RECMNG_MUXER_TYPE_E enMuxerType, HI_REC_MUXER_OPERATE_FN_S *pstMuxerOps)
{
    HI_APPCOMM_CHECK_POINTER(pstMuxerOps, HI_FAILURE);

    switch (enMuxerType)
    {
#if defined(CONFIG_RECORDMNG_MP4)
        case HI_RECMNG_MUXER_MP4:
        case HI_RECMNG_MUXER_MOV:
            pstMuxerOps->pfnCreateMuxer = RECMNG_CreateMp4Muxer;
            pstMuxerOps->pfnDestroyMuxer = RECMNG_DestroyMp4Muxer;
            pstMuxerOps->pfnCreateTrack = RECMNG_CreateMp4MuxerTrack;
            pstMuxerOps->pfnWriteFrame = RECMNG_WriteMp4MuxerFrame;
            break;
#endif

#if defined(CONFIG_RECORDMNG_TS)
        case HI_RECMNG_MUXER_TS:
            pstMuxerOps->pfnCreateMuxer = RECMNG_CreateTsMuxer;
            pstMuxerOps->pfnDestroyMuxer = RECMNG_DestroyTsMuxer;
            pstMuxerOps->pfnCreateTrack = RECMNG_CreateTsMuxerTrack;
            pstMuxerOps->pfnWriteFrame = RECMNG_WriteTsMuxerFrame;
            break;
#endif

        default:
            MLOGE("Invalid muxer type[%d]\n", enMuxerType);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}


static HI_S32 RECMNG_GetSlaveMuxerOps(HI_RECMNG_MUXER_TYPE_E enMuxerType, HI_REC_MUXER_OPERATE_FN_S *pstMuxerOps)
{
    HI_APPCOMM_CHECK_POINTER(pstMuxerOps, HI_FAILURE);

    switch (enMuxerType)
    {
#if defined(CONFIG_RECORDMNG_MP4)
        case HI_RECMNG_MUXER_MP4:
        case HI_RECMNG_MUXER_MOV:
            pstMuxerOps->pfnCreateMuxer = RECMNG_CreateMp4SlaveMuxer;
            pstMuxerOps->pfnDestroyMuxer = RECMNG_DestroyMp4SlaveMuxer;
            pstMuxerOps->pfnCreateTrack = RECMNG_CreateMp4SlaveMuxerTrack;
            pstMuxerOps->pfnWriteFrame = RECMNG_WriteMp4MuxerFrame;
            break;
#endif

#if defined(CONFIG_RECORDMNG_TS)
        case HI_RECMNG_MUXER_TS:
            pstMuxerOps->pfnCreateMuxer = RECMNG_CreateTsSlaveMuxer;
            pstMuxerOps->pfnDestroyMuxer = RECMNG_DestroyTsSlaveMuxer;
            pstMuxerOps->pfnCreateTrack = RECMNG_CreateTsSlaveMuxerTrack;
            pstMuxerOps->pfnWriteFrame = RECMNG_WriteTsMuxerFrame;
            break;
#endif

        default:
            MLOGE("Invalid muxer type[%d]\n", enMuxerType);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}


static HI_S32 RECMNG_GetFileNames(HI_MW_PTR pRecObj, HI_U32 u32FileCnt, HI_CHAR (*paszFilename)[HI_REC_FILE_NAME_LEN])
{
    HI_APPCOMM_CHECK_POINTER(paszFilename, HI_FAILURE);

    /* find record task */
    RECMNG_TASK_CONTEXT_S *pstTskCtx = NULL;
    HI_S32 s32Idx = 0;
    for (s32Idx = 0; s32Idx < HI_RECMNG_MAX_TASK_CNT; ++s32Idx)
    {
        if (pRecObj == s_astRECMNGTaskCtx[s32Idx].pRecObj)
        {
            pstTskCtx = &s_astRECMNGTaskCtx[s32Idx];
            break;
        }
    }
    HI_APPCOMM_CHECK_POINTER(pstTskCtx, HI_FAILURE);

    if (pstTskCtx->stGetFilenameCb.pfnGetFilenames)
    {
        memset(&pstTskCtx->stFilename, 0x0, sizeof(HI_RECMNG_FILENAME_S));
        pstTskCtx->stFilename.u32RecFileCnt = pstTskCtx->u32FileCnt;
        if(HI_RECMNG_THM_TYPE_INDEPENDENCE == pstTskCtx->enThmType)
        {
            pstTskCtx->stFilename.u32ThmFileCnt = pstTskCtx->stThmAttr.u32ThmCnt;
        }

        HI_S32 s32Ret;
        s32Ret = pstTskCtx->stGetFilenameCb.pfnGetFilenames(&pstTskCtx->stFilename, pstTskCtx->stGetFilenameCb.pvPrivData);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "GetFileName");

        // todo
        HI_S32 s32StreamIdx;
        HI_S32 s32FilenameIdx = 0;
        for (s32Idx = 0; s32Idx < pstTskCtx->u32FileCnt; ++s32Idx)
        {
            for(s32StreamIdx = 0; s32StreamIdx < pstTskCtx->astFileInfo[s32Idx].u32StreamCnt; ++s32StreamIdx)
            {
                snprintf(paszFilename[s32FilenameIdx], HI_APPCOMM_MAX_FILENAME_LEN, "%s",
                    pstTskCtx->stFilename.aszRecFilename[s32Idx]);
                ++s32FilenameIdx;
            }
            MLOGD("Filename[%u]: %s\n", s32Idx, pstTskCtx->stFilename.aszRecFilename[s32Idx]);
        }

        for(s32Idx = 0; s32Idx < pstTskCtx->stFilename.u32ThmFileCnt; ++s32Idx)
        {
            MLOGD("thumbnail filename: %s\n", pstTskCtx->stFilename.aszThmFilename[s32Idx]);
        }
        return HI_SUCCESS;
    }
    else
    {
        MLOGE("Invalid Get filename callback!\n");
        return HI_FAILURE;
    }
}

HI_S32 RECMNG_StopTask(HI_HANDLE Hdl)
{
    MLOGI(GREEN"StopTask[%d]\n"NONE, Hdl);
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_READY:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGD("task[%d] has already stoped\n", Hdl);
            return HI_SUCCESS;

        case RECMNG_TASK_STATE_STARTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is starting\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_STOPPING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is stopping\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_STARTED:
        case RECMNG_TASK_STATE_SPLITTING:

        default:
            break;
    }

    RECMNG_TASK_STATE_E enTempTaskState = s_astRECMNGTaskCtx[Hdl].enTaskState;
    s_astRECMNGTaskCtx[Hdl].enTaskState = RECMNG_TASK_STATE_STOPPING;
    HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);

    HI_S32 s32Ret = HI_REC_Stop(s_astRECMNGTaskCtx[Hdl].pRecObj, HI_FALSE);
    if(HI_SUCCESS != s32Ret)
    {
        RECMNG_SetTaskState(Hdl, enTempTaskState);
        MLOGE(RED"stop rec failure, ret[%08x]\n"NONE, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 RECMNG_ThmDataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S* pVStreamData, HI_VOID *pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pVStreamData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);
    if(HI_MAPI_PAYLOAD_TYPE_JPEG != pVStreamData->astPack[0].stDataType.enPayloadType)
    {
        MLOGE("VencHdl[%d] PayloadType[%d] not JPEG\n",
            VencHdl, pVStreamData->astPack[0].stDataType.enPayloadType);
        return HI_FAILURE;
    }

    HI_U32 u32Idx = 0;
    HI_U8 *pu8TmpBuf = NULL;
    MLOGI("RECMNG_ThmDataProc, VencHdl[%d]\n", VencHdl);
    /* Calculate THM Size */
    HI_U32 u32PicSize = 0;
    for (u32Idx = 0; u32Idx < pVStreamData->u32PackCount; ++u32Idx)
    {
        u32PicSize += pVStreamData->astPack[u32Idx].au32Len[0]- pVStreamData->astPack[u32Idx].u32Offset;
    }
    MLOGD("ThmSize[%uBytes] in [%u] packet\n", u32PicSize, pVStreamData->u32PackCount);

    /* Prepare Thm Data Buffer */
    HI_U8 *pu8DataBuf = (HI_U8*)malloc(u32PicSize);
    HI_APPCOMM_CHECK_POINTER(pu8DataBuf, HI_FAILURE);
    pu8TmpBuf = pu8DataBuf;
    for (u32Idx = 0; u32Idx < pVStreamData->u32PackCount; ++u32Idx)
    {
        memcpy(pu8TmpBuf, pVStreamData->astPack[u32Idx].apu8Addr[0] + pVStreamData->astPack[u32Idx].u32Offset,
            pVStreamData->astPack[u32Idx].au32Len[0] - pVStreamData->astPack[u32Idx].u32Offset);
        pu8TmpBuf += pVStreamData->astPack[u32Idx].au32Len[0]- pVStreamData->astPack[u32Idx].u32Offset;
    }

    /* Write Thm Data */
    RECMNG_TASK_CONTEXT_S *pstTskCtx = (RECMNG_TASK_CONTEXT_S*)pvPrivData;

    if(HI_RECMNG_THM_TYPE_EMBEDDED == pstTskCtx->enThmType)
    {
        for(u32Idx = 0; u32Idx < pstTskCtx->u32FileCnt; ++u32Idx)
        {
            if(pstTskCtx->astFileInfo[u32Idx].ThmHdl == VencHdl)
            {
#if defined(CONFIG_RECORDMNG_MP4)
                if ((HI_RECMNG_MUXER_MP4 == pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.enMuxerType)
                    || (HI_RECMNG_MUXER_MOV == pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.enMuxerType))
                {
                    HI_MP4_ATOM_INFO_S stAtomInfo;
                    stAtomInfo.u32DataLen = u32PicSize;
                    stAtomInfo.pu8DataBuf = pu8DataBuf;
                    stAtomInfo.aszType[0] = 't';
                    stAtomInfo.aszType[1] = 'h';
                    stAtomInfo.aszType[2] = 'm';
                    stAtomInfo.aszType[3] = ' ';
                    stAtomInfo.aszType[4] = '\0';

                    MLOGD("HI_MP4_AddAtom[%u, %p] HTM ...\n", u32Idx, pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.pMuxerObj);

                    HI_MUTEX_LOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                    if(HI_NULL_PTR == pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.pMuxerObj)
                    {
                        HI_MUTEX_UNLOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                        MLOGD("Mp4 muxer has been destroyed\n");
                        HI_APPCOMM_SAFE_FREE(pu8DataBuf);
                        return HI_SUCCESS;
                    }
                    HI_S32 s32Ret = HI_SUCCESS;
                    s32Ret = HI_MP4_AddAtom(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.pMuxerObj, "/", &stAtomInfo);
                    HI_MUTEX_UNLOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE("HI_MP4_AddAtom error[%x]\n", s32Ret);
                        HI_APPCOMM_SAFE_FREE(pu8DataBuf);
                        return HI_FAILURE;
                    }
                    MLOGI(GREEN"Mp4 add thm atom success, thmSize[%uBytes], VencHdl[%d]\n"NONE, u32PicSize, VencHdl);
                }
#endif

#if defined(CONFIG_RECORDMNG_TS)
                if (HI_RECMNG_MUXER_TS == pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.enMuxerType)
                {
                    HI_TS_FRAME_DATA_S stHtmTsFrmData;
                    stHtmTsFrmData.u64TimeStamp = pVStreamData->astPack[0].u64PTS;
                    stHtmTsFrmData.pu8DataBuffer = pu8DataBuf;
                    stHtmTsFrmData.u32DataLength = u32PicSize;
                    stHtmTsFrmData.bKeyFrameFlag = HI_FALSE;

                    HI_MUTEX_LOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                    if(HI_NULL_PTR == pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.pMuxerObj)
                    {
                        HI_MUTEX_UNLOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                        MLOGD("Ts muxer has been destroyed\n");
                        HI_APPCOMM_SAFE_FREE(pu8DataBuf);
                        return HI_SUCCESS;
                    }
                    HI_S32 s32Ret = HI_SUCCESS;
                    s32Ret = HI_TS_WriteFrame(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.pMuxerObj,
                        pstTskCtx->astFileInfo[u32Idx].pThmTrackObj, &stHtmTsFrmData);
                    HI_MUTEX_UNLOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE("HI_TS_WriteFrame error[%x]\n", s32Ret);
                        HI_APPCOMM_SAFE_FREE(pu8DataBuf);
                        return HI_FAILURE;
                    }
                    MLOGI(GREEN"Ts write thm frame success, thmSize[%uBytes], VencHdl[%d]\n"NONE, u32PicSize, VencHdl);
                }
#endif
            }
        }
    }
    else if(HI_RECMNG_THM_TYPE_INDEPENDENCE == pstTskCtx->enThmType)
    {
        for(u32Idx = 0; u32Idx < pstTskCtx->stThmAttr.u32ThmCnt; ++u32Idx)
        {
            if((pstTskCtx->stThmAttr.aThmHdl[u32Idx] == VencHdl)
                && (HI_NULL != pstTskCtx->stFilename.aszThmFilename[u32Idx]))
            {
                break;
            }
        }
        if(u32Idx >= pstTskCtx->stThmAttr.u32ThmCnt)
        {
            MLOGE("Invalid thmHdl[%u]\n", VencHdl);
            HI_APPCOMM_SAFE_FREE(pu8DataBuf);
            return HI_FAILURE;
        }

        MLOGD("THM: %s\n", pstTskCtx->stFilename.aszThmFilename[u32Idx]);
        FILE* pFile = fopen(pstTskCtx->stFilename.aszThmFilename[u32Idx], "w");
        if(pFile == NULL)
        {
            MLOGE("fopen file[%s] fail\n", pstTskCtx->stFilename.aszThmFilename[u32Idx]);
            HI_APPCOMM_SAFE_FREE(pu8DataBuf);
            return HI_FAILURE;
        }
        else
        {
            HI_U32 u32Offset = 0;
            HI_U32 u32Retry = 0;
            while(u32Offset < u32PicSize)
            {
                u32Offset += (HI_U32)fwrite(pu8DataBuf + u32Offset, 1, u32PicSize - u32Offset, pFile);
                ++u32Retry;
                if((u32Offset < u32PicSize) && ((errno != EINTR) || (u32Retry > RECMNG_FWRITE_RETRY_TIMES)))
                {
                    fclose(pFile);
                    HI_APPCOMM_SAFE_FREE(pu8DataBuf);
                    return HI_FAILURE;
                }
            }

            fclose(pFile);
            MLOGI(GREEN"Save picture %s success, pictureSize[%uBytes] VencHdl[%d]\n"NONE, pstTskCtx->stFilename.aszThmFilename[u32Idx], u32PicSize, VencHdl);
        }
    }

    /* Release Thm Data Buffer */
    HI_APPCOMM_SAFE_FREE(pu8DataBuf);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_FileEventProc(RECMNG_TASK_CONTEXT_S *pstTskCtx, const HI_REC_EVENT_INFO_S *pstEventInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstTskCtx, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pstEventInfo, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Idx = 0;

    /* find corresponding file index */
    for (u32Idx = 0; u32Idx < pstTskCtx->u32FileCnt; ++u32Idx)
    {
        if(0 == strncmp(pstTskCtx->stFilename.aszRecFilename[u32Idx],
                    pstEventInfo->unEventInfo.stFileInfo.aszFileName, HI_APPCOMM_MAX_FILENAME_LEN))
        {
            MLOGD("file idx = %u, filename %s\n", u32Idx, pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;
        }

    }
    if (u32Idx >= pstTskCtx->u32FileCnt)
    {
        MLOGD("not file name[%s], ignore it\n", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
        return HI_SUCCESS;
    }

    /* process loop strategy if loop strategy open and u32Idx is the index used to count */
    if (pstTskCtx->stLoopMngInfo.bLoop && (pstTskCtx->stLoopMngInfo.u32FileIdx == u32Idx))
    {
        if (HI_REC_EVENT_NEW_FILE_BEGIN == pstEventInfo->enEventCode)
        {
            MLOGD("loop index %u\n", pstTskCtx->stLoopMngInfo.u32Idx);
            if (strnlen(pstTskCtx->stLoopMngInfo.aszFileName[pstTskCtx->stLoopMngInfo.u32Idx], HI_APPCOMM_MAX_FILENAME_LEN - 1)
                && pstTskCtx->stLoopMngInfo.pfnDelFile)
            {
                s32Ret = pstTskCtx->stLoopMngInfo.pfnDelFile(pstTskCtx->stLoopMngInfo.aszFileName[pstTskCtx->stLoopMngInfo.u32Idx]);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("delete file[%s] failure\n", pstTskCtx->stLoopMngInfo.aszFileName[pstTskCtx->stLoopMngInfo.u32Idx]);
                }
            }
        }
        else
        {
            MLOGD("loop index %u, filename %s\n", pstTskCtx->stLoopMngInfo.u32Idx,
                pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            memcpy(pstTskCtx->stLoopMngInfo.aszFileName[pstTskCtx->stLoopMngInfo.u32Idx++],
                pstEventInfo->unEventInfo.stFileInfo.aszFileName, HI_APPCOMM_MAX_FILENAME_LEN);
            pstTskCtx->stLoopMngInfo.u32Idx %= pstTskCtx->stLoopMngInfo.u32LoopNum;
        }
    }

    if(HI_REC_EVENT_NEW_FILE_BEGIN == pstEventInfo->enEventCode)
    {
        pstTskCtx->u32RecFileCnt++;
        if(pstTskCtx->u32FileCnt == pstTskCtx->u32RecFileCnt)
        {
            if(HI_RECMNG_THM_TYPE_INDEPENDENCE == pstTskCtx->enThmType)
            {
                HI_U32 u32ThmIdx;
                for(u32ThmIdx = 0; u32ThmIdx < pstTskCtx->stThmAttr.u32ThmCnt; ++u32ThmIdx)
                {
                    /* thumbnail data process callback */
                    HI_MAPI_VENC_CALLBACK_S stVencCB;
                    stVencCB.pfnDataCB = RECMNG_ThmDataProc;
                    stVencCB.pPrivateData = (HI_VOID*)pstTskCtx;

                    s32Ret = HI_MAPI_VENC_RegisterCallback(pstTskCtx->stThmAttr.aThmHdl[u32ThmIdx], &stVencCB);
                    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
                    s32Ret = stRECMNGMediaOps.pfnVencStart(pstTskCtx->stThmAttr.aThmHdl[u32ThmIdx], 1);
                    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
                }
            }
            else if(HI_RECMNG_THM_TYPE_EMBEDDED == pstTskCtx->enThmType)
            {
                HI_U32 u32FileIdx;
                HI_U32 u32CmpIdx;
                HI_BOOL bSameThmHdlFlg;

                for(u32FileIdx = 0; u32FileIdx < pstTskCtx->u32FileCnt; ++u32FileIdx)
                {
                    if(HI_INVALID_HANDLE == pstTskCtx->astFileInfo[u32FileIdx].ThmHdl)
                    {
                        continue;
                    }
#if defined(CONFIG_RECORDMNG_TS)
                    /* create ts thm stream */
                    if(HI_RECMNG_MUXER_TS == pstTskCtx->astFileInfo[u32FileIdx].stMuxerCfg.enMuxerType)
                    {
                        HI_TS_STREAM_INFO_S stThmStreamInfo;
                        stThmStreamInfo.enCodecID = HI_TS_CODEC_ID_JPEG;
                        s32Ret = HI_TS_CreateStream(pstTskCtx->astFileInfo[u32FileIdx].stMuxerCfg.pMuxerObj,
                            &pstTskCtx->astFileInfo[u32FileIdx].pThmTrackObj, &stThmStreamInfo);
                        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
                        MLOGD("Thm stream create successful, ThmTrackObj[%p]\n", pstTskCtx->astFileInfo[u32FileIdx].pThmTrackObj);
                    }
#endif
                    bSameThmHdlFlg = HI_FALSE;
                    for(u32CmpIdx = u32FileIdx + 1; u32CmpIdx < pstTskCtx->u32FileCnt; ++u32CmpIdx)
                    {
                        if(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl == pstTskCtx->astFileInfo[u32CmpIdx].ThmHdl)
                        {
                            bSameThmHdlFlg = HI_TRUE;
                            break;
                        }
                    }
                    /** start venc when all files that have same ThmHdl begin */
                    if(HI_FALSE == bSameThmHdlFlg)
                    {
                        /* thumbnail data process callback */
                        HI_MAPI_VENC_CALLBACK_S stVencCB;
                        stVencCB.pfnDataCB = RECMNG_ThmDataProc;
                        stVencCB.pPrivateData = (HI_VOID*)pstTskCtx;

                        s32Ret = HI_MAPI_VENC_RegisterCallback(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl, &stVencCB);
                        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
                        s32Ret = stRECMNGMediaOps.pfnVencStart(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl, 1);
                        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
                    }
                }
            }
        }
    }
    else /** new file end */
    {
        pstTskCtx->u32RecFileCnt--;
        if(0 == pstTskCtx->u32RecFileCnt)
        {
            if(HI_RECMNG_THM_TYPE_INDEPENDENCE == pstTskCtx->enThmType)
            {
                HI_U32 u32ThmIdx;
                for(u32ThmIdx = 0; u32ThmIdx < pstTskCtx->stThmAttr.u32ThmCnt; ++u32ThmIdx)
                {
                    /* thumbnail data process callback */
                    HI_MAPI_VENC_CALLBACK_S stVencCB;
                    stVencCB.pfnDataCB = RECMNG_ThmDataProc;
                    stVencCB.pPrivateData = (HI_VOID*)pstTskCtx;

                    HI_MAPI_VENC_UnRegisterCallback(pstTskCtx->stThmAttr.aThmHdl[u32ThmIdx], &stVencCB);
                    stRECMNGMediaOps.pfnVencStop(pstTskCtx->stThmAttr.aThmHdl[u32ThmIdx]);
                }
            }
            else if(HI_RECMNG_THM_TYPE_EMBEDDED == pstTskCtx->enThmType)
            {
                HI_U32 u32FileIdx;
                HI_U32 u32CmpIdx;
                HI_BOOL bSameThmHdlFlg;
                for(u32FileIdx = 0; u32FileIdx < pstTskCtx->u32FileCnt; ++u32FileIdx)
                {
                    if(HI_INVALID_HANDLE == pstTskCtx->astFileInfo[u32FileIdx].ThmHdl)
                    {
                        continue;
                    }

                    bSameThmHdlFlg = HI_FALSE;
                    for(u32CmpIdx = u32FileIdx + 1; u32CmpIdx < pstTskCtx->u32FileCnt; ++u32CmpIdx)
                    {
                        if(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl == pstTskCtx->astFileInfo[u32CmpIdx].ThmHdl)
                        {
                            bSameThmHdlFlg = HI_TRUE;
                            break;
                        }
                    }

                    /** stop venc when all files that have same ThmHdl end */
                    if(HI_FALSE == bSameThmHdlFlg)
                    {
                        /* thumbnail data process callback */
                        HI_MAPI_VENC_CALLBACK_S stVencCB;
                        stVencCB.pfnDataCB = RECMNG_ThmDataProc;
                        stVencCB.pPrivateData = (HI_VOID*)pstTskCtx;

                        HI_MAPI_VENC_UnRegisterCallback(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl, &stVencCB);
                        stRECMNGMediaOps.pfnVencStop(pstTskCtx->astFileInfo[u32FileIdx].ThmHdl);
                    }
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 RECMNG_EventPreProc(HI_MW_PTR pRecObj, const HI_REC_EVENT_INFO_S* pstEventInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstEventInfo, HI_FAILURE);

    /* find record task */
    RECMNG_TASK_CONTEXT_S *pstTskCtx = NULL;
    HI_S32 s32Idx = 0;
    for (s32Idx = 0; s32Idx < HI_RECMNG_MAX_TASK_CNT; ++s32Idx)
    {
        if (pRecObj == s_astRECMNGTaskCtx[s32Idx].pRecObj)
        {
            pstTskCtx = &s_astRECMNGTaskCtx[s32Idx];
            break;
        }
    }
    HI_APPCOMM_CHECK_POINTER(pstTskCtx, HI_FAILURE);

    HI_EVENT_S stEvent;

    MLOGI("TaskHdl = %d, EventCode = %d\n", s32Idx, pstEventInfo->enEventCode);
    switch (pstEventInfo->enEventCode)
    {
        case HI_REC_EVENT_START:
            if (pstTskCtx->stLoopMngInfo.bLoop)
            {
                pstTskCtx->stLoopMngInfo.u32Idx = 0;
                memset(pstTskCtx->stLoopMngInfo.aszFileName, 0x0,
                    HI_RECMNG_MAX_LOOP_NUM * HI_APPCOMM_MAX_FILENAME_LEN);
            }

            HI_MUTEX_LOCK(s_astRECMNGTaskCtx[s32Idx].stateMutex);
            s_astRECMNGTaskCtx[s32Idx].enTaskState = RECMNG_TASK_STATE_STARTED;
            HI_COND_SIGNAL(pstTskCtx->recEventCond);
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[s32Idx].stateMutex);

            stEvent.EventID = HI_EVENT_RECMNG_TASK_START;
            break;

        case HI_REC_EVENT_STOP:
            HI_MUTEX_LOCK(s_astRECMNGTaskCtx[s32Idx].stateMutex);
            s_astRECMNGTaskCtx[s32Idx].enTaskState = RECMNG_TASK_STATE_READY;
            HI_COND_SIGNAL(pstTskCtx->recEventCond);
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[s32Idx].stateMutex);

            if(HI_TRUE == pstTskCtx->bStopAfterManualSplitEnd)
            {
                pstTskCtx->bStopAfterManualSplitEnd = HI_FALSE;
                stEvent.EventID = HI_EVENT_RECMNG_TASK_AUTO_STOP;
            }
            else
            {
                 stEvent.EventID = HI_EVENT_RECMNG_TASK_STOP;
            }
            break;

        case HI_REC_EVENT_NEW_FILE_BEGIN:
            RECMNG_FileEventProc(pstTskCtx, pstEventInfo);
            stEvent.EventID = HI_EVENT_RECMNG_FILE_BEGIN;
            snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;

        case HI_REC_EVENT_NEW_FILE_END:
            RECMNG_FileEventProc(pstTskCtx, pstEventInfo);
            stEvent.EventID = HI_EVENT_RECMNG_FILE_END;
            snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;

        case HI_REC_EVENT_NEW_MANUAL_SPLIT_FILE_END:
            RECMNG_FileEventProc(pstTskCtx, pstEventInfo);
            RECMNG_SetTaskState(s32Idx, RECMNG_TASK_STATE_STARTED);
            stEvent.EventID = HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END;
            snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pstEventInfo->unEventInfo.stFileInfo.aszFileName);

            // call RECMNG_StopTask after HI_REC_EVENT_NEW_MANUAL_SPLIT_FILE_END
            if(HI_TRUE == pstTskCtx->bStopAfterManualSplitEnd)
            {
                RECMNG_StopTask(s32Idx);
            }
            break;

        case HI_REC_EVENT_ERR_CREATE_FILE_FAIL:
        case HI_REC_EVENT_ERR_WRITE_FILE_FAIL:
        case HI_REC_EVENT_ERR_CLOSE_FILE_FAIL:
        case HI_REC_EVENT_ERR_READ_DATA_ERROR:
        case HI_REC_EVENT_ERR_INTERNAL_OPERATION_FAIL:
            stEvent.EventID = HI_EVENT_RECMNG_ERROR;
            stEvent.s32Result = pstEventInfo->unEventInfo.stErrorInfo.s32ErrorCode;
            snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pstEventInfo->unEventInfo.stErrorInfo.aszFileName);
            break;

        default:
            return HI_SUCCESS;
    }
    stEvent.arg1 = s32Idx;
    HI_EVTHUB_Publish(&stEvent);

    return HI_SUCCESS;
}

static HI_S32 RECMNG_WaitState(HI_HANDLE Hdl, RECMNG_TASK_STATE_E enState, HI_S32 s32Timeout_ms)
{
    struct timespec stTimespecBegin, stTimespecEnd;
    RECMNG_TASK_CONTEXT_S *pstTskCtx = &s_astRECMNGTaskCtx[Hdl];

    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecBegin);

    if(s32Timeout_ms > 0)
    {
        HI_MUTEX_LOCK(pstTskCtx->stateMutex);
        if(pstTskCtx->enTaskState == enState)
        {
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            return HI_SUCCESS;
        }
        HI_COND_TIMEDWAIT(pstTskCtx->recEventCond, pstTskCtx->stateMutex, s32Timeout_ms*1000);

        if(pstTskCtx->enTaskState == enState)
        {
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecEnd);
            HI_U32 u32WaitTime_ms = ((stTimespecEnd.tv_sec - stTimespecBegin.tv_sec) * 1000 + stTimespecEnd.tv_nsec / 1000000
                -  stTimespecBegin.tv_nsec / 1000000);
            MLOGI("Wait state[%d] duration %dms\n", enState, u32WaitTime_ms);
            return HI_SUCCESS;
        }
        else
        {
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            MLOGE("Wait state[%d] timeout[%dms]\n", enState, s32Timeout_ms);
            return HI_RECMNG_ETIMEOUT;
        }
    }
    else
    {
        HI_MUTEX_LOCK(pstTskCtx->stateMutex);
        while(pstTskCtx->enTaskState != enState)
        {
            HI_COND_WAIT(pstTskCtx->recEventCond, pstTskCtx->stateMutex);
        }
        HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);

        (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecEnd);
        HI_U32 u32WaitTime_ms = ((stTimespecEnd.tv_sec - stTimespecBegin.tv_sec) * 1000 + stTimespecEnd.tv_nsec / 1000000
            - stTimespecBegin.tv_nsec / 1000000);
        MLOGI("Wait state[%d] duration %dms\n", enState, u32WaitTime_ms);
        return HI_SUCCESS;
    }
}

HI_S32 HI_RECMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_EVTHUB_Register(HI_EVENT_RECMNG_FILE_BEGIN);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_FILE_END);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_ERROR);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_MUXER_CNT_CHANGE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_TASK_START);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_TASK_STOP);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_RECMNG_TASK_AUTO_STOP);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_Init(const HI_RECMNG_MEDIA_OPERATE_S *pstMediaOps)
{
    MLOGI(GREEN"InitRecMng\n"NONE);
    if (s_bRECMNGInitFlg)
    {
        MLOGE("has already inited!\n");
        return HI_RECMNG_EINITIALIZED;
    }

    HI_APPCOMM_CHECK_POINTER(pstMediaOps, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnGetVideoInfo, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnGetAudioInfo, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnVencStart, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnVencStop, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnAencStart, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnAencStop, HI_RECMNG_EINVAL);

    stRECMNGMediaOps.pfnGetVideoInfo = pstMediaOps->pfnGetVideoInfo;
    stRECMNGMediaOps.pfnGetAudioInfo = pstMediaOps->pfnGetAudioInfo;
    stRECMNGMediaOps.pfnVencStart = pstMediaOps->pfnVencStart;
    stRECMNGMediaOps.pfnVencStop = pstMediaOps->pfnVencStop;
    stRECMNGMediaOps.pfnAencStart = pstMediaOps->pfnAencStart;
    stRECMNGMediaOps.pfnAencStop = pstMediaOps->pfnAencStop;
    if(HI_NULL != pstMediaOps->pfnPrivDataStart &&
        HI_NULL != pstMediaOps->pfnPrivDataStop &&
        HI_NULL != pstMediaOps->pfnGetPrivDataCfg)
    {
        stRECMNGMediaOps.pfnPrivDataStart = pstMediaOps->pfnPrivDataStart;
        stRECMNGMediaOps.pfnPrivDataStop = pstMediaOps->pfnPrivDataStop;
        stRECMNGMediaOps.pfnGetPrivDataCfg = pstMediaOps->pfnGetPrivDataCfg;
    }
    else
    {
        stRECMNGMediaOps.pfnPrivDataStart = HI_NULL;
        stRECMNGMediaOps.pfnPrivDataStop = HI_NULL;
        stRECMNGMediaOps.pfnGetPrivDataCfg = HI_NULL;
    }

    HI_S32 s32Idx = 0;
    memset(&s_astRECMNGTaskCtx[0], 0x0, sizeof(RECMNG_TASK_CONTEXT_S) * HI_RECMNG_MAX_TASK_CNT);
    for (s32Idx = 0; s32Idx < HI_RECMNG_MAX_TASK_CNT; ++s32Idx)
    {
        RECMNG_TASK_CONTEXT_S *pstTskCtx = &s_astRECMNGTaskCtx[s32Idx];
        HI_MUTEX_INIT_LOCK(pstTskCtx->stateMutex);
        HI_COND_INIT(pstTskCtx->recEventCond);
    }

#ifdef RECMNG_SUPPORT_ASYNC_STOP
    memset(&s_stAsyncStopContext, 0x0, sizeof(RECMNG_ASYNC_STOP_CONTEXT_S));
    HI_MUTEX_INIT_LOCK(s_stAsyncStopContext.muxerInfoMutex);
    s_stAsyncStopContext.destroyMuxerThread = -1;
    s_stAsyncStopContext.bRunDestroyMuxerThread = HI_TRUE;

    if (0 != pthread_create(&s_stAsyncStopContext.destroyMuxerThread, NULL, RECMNG_DestroyMuxerThread, NULL))
    {
        MLOGE("pthread_create error:%s \n", strerror(errno));
        return HI_RECMNG_EINTER;
    }
#endif

    s_bRECMNGInitFlg = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_Deinit()
{
    MLOGI(GREEN"DeinitRecMng\n"NONE);
    RECMNG_CHECK_INIT();

    memset(&stRECMNGMediaOps, 0x0, sizeof(HI_RECMNG_MEDIA_OPERATE_S));

    HI_S32 s32Idx = 0;
    HI_S32 s32Ret;
    for (s32Idx = 0; s32Idx < HI_RECMNG_MAX_TASK_CNT; ++s32Idx)
    {
        RECMNG_TASK_CONTEXT_S *pstTskCtx = &s_astRECMNGTaskCtx[s32Idx];
        if(pstTskCtx->bUsed)
        {
            s32Ret = HI_RECMNG_DestroyTask(s32Idx);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("HI_RECMNG_DestroyTask[%d] failed[0x%08X]\n", s32Idx, s32Ret);
            }
        }
        HI_MUTEX_DESTROY(pstTskCtx->stateMutex);
        HI_COND_DESTROY(pstTskCtx->recEventCond);
    }

#ifdef RECMNG_SUPPORT_ASYNC_STOP
    s_stAsyncStopContext.bRunDestroyMuxerThread = HI_FALSE;
    pthread_join(s_stAsyncStopContext.destroyMuxerThread, NULL);
#endif

    s_bRECMNGInitFlg = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_CreateTask(const HI_RECMNG_ATTR_S *pstRecAttr, HI_HANDLE *pHdl)
{
    MLOGI(GREEN"CreateTask\n"NONE);
    RECMNG_CHECK_INIT();

    /* check input param */
    HI_APPCOMM_CHECK_POINTER(pHdl, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstRecAttr, HI_RECMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstRecAttr->stGetFilename.pfnGetFilenames, HI_RECMNG_EINVAL);

    if (!HI_APPCOMM_CHECK_RANGE(pstRecAttr->u32FileCnt, HI_RECMNG_TASK_MIN_FILE_CNT, HI_RECMNG_TASK_MAX_FILE_CNT))
    {
        MLOGE("Invalid file count[%u]\n", pstRecAttr->u32FileCnt);
        return HI_RECMNG_EINVAL;
    }

    /* check task count */
    RECMNG_TASK_CONTEXT_S *pstTskCtx = HI_NULL;
    HI_U32 u32Idx = 0;
    for(u32Idx = 0; u32Idx < HI_RECMNG_MAX_TASK_CNT; ++u32Idx)
    {
        if(!s_astRECMNGTaskCtx[u32Idx].bUsed)
        {
            pstTskCtx = &s_astRECMNGTaskCtx[u32Idx];
            *pHdl = u32Idx;
            break;
        }
    }
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(pstTskCtx, HI_RECMNG_EMAXTASK, "beyond maximum task");

    HI_REC_ATTR_S stRecAttr;
    memset(&stRecAttr, 0x0, sizeof(stRecAttr));

    /* record type and corresponding attribute */
    stRecAttr.enRecType = pstRecAttr->enRecType;
    if (HI_REC_TYPE_LAPSE == pstRecAttr->enRecType)
    {
        stRecAttr.unRecAttr.stLapseRecAttr.u32IntervalMs = pstRecAttr->unRecordAttr.stLapseAttr.u32IntervalMs;
    }
    MLOGI("record type[%d]\n", pstRecAttr->enRecType);

    /* split attribute */
    stRecAttr.stSplitAttr.enSplitType = pstRecAttr->stSplitAttr.enSplitType;
    stRecAttr.stSplitAttr.u32SplitTimeLenSec = pstRecAttr->stSplitAttr.u32SplitTimeLenSec;
    MLOGI("split type[%d], timelen[%us]\n", pstRecAttr->stSplitAttr.enSplitType, pstRecAttr->stSplitAttr.u32SplitTimeLenSec);

    /* get filename callback configure */
    pstTskCtx->stGetFilenameCb.pfnGetFilenames = pstRecAttr->stGetFilename.pfnGetFilenames;
    pstTskCtx->stGetFilenameCb.pvPrivData = pstRecAttr->stGetFilename.pvPrivData;

    stRecAttr.stRecCallbacks.pfnRequestFileNames = RECMNG_GetFileNames;

    /* buffer configure */
    stRecAttr.u32BufferTimeMSec = pstRecAttr->u32BufferTimeMSec;
    stRecAttr.u32PreRecTimeSec = pstRecAttr->u32PreRecTimeSec;


    /* thm configure */
    pstTskCtx->enThmType = pstRecAttr->enThmType;
    if(HI_RECMNG_THM_TYPE_INDEPENDENCE == pstRecAttr->enThmType)
    {
        if(pstRecAttr->stThmAttr.u32ThmCnt > HI_RECMNG_TASK_MAX_FILE_CNT)
        {
            MLOGE("Thumbnail count[%u] out of rang[0, %u]", pstRecAttr->stThmAttr.u32ThmCnt, HI_RECMNG_TASK_MAX_FILE_CNT);
            return HI_RECMNG_EINVAL;
        }
        pstTskCtx->stThmAttr.u32ThmCnt = pstRecAttr->stThmAttr.u32ThmCnt;
        for(u32Idx = 0; u32Idx < pstRecAttr->stThmAttr.u32ThmCnt; ++u32Idx)
        {
            pstTskCtx->stThmAttr.aThmHdl[u32Idx] = pstRecAttr->stThmAttr.aThmHdl[u32Idx];
        }
    }

    /* loop strategy */
    pstTskCtx->stLoopMngInfo.bLoop = pstRecAttr->stLoopAttr.bLoop;
    if (pstRecAttr->stLoopAttr.bLoop)
    {
        if (pstRecAttr->stLoopAttr.u32LoopNum > HI_RECMNG_MAX_LOOP_NUM)
        {
            MLOGE("loop num beyond maximum[%d]\n", HI_RECMNG_MAX_LOOP_NUM);
            return HI_RECMNG_EINVAL;
        }
        if (pstRecAttr->stLoopAttr.u32FileIdx >= pstRecAttr->u32FileCnt)
        {
            MLOGE("Invalid file idx[%u in %u] for loop strategy\n",
                pstRecAttr->stLoopAttr.u32FileIdx, pstRecAttr->u32FileCnt);
            return HI_RECMNG_EINVAL;
        }
        pstTskCtx->stLoopMngInfo.u32LoopNum = pstRecAttr->stLoopAttr.u32LoopNum;
        pstTskCtx->stLoopMngInfo.u32FileIdx = pstRecAttr->stLoopAttr.u32FileIdx;
        pstTskCtx->stLoopMngInfo.pfnDelFile = pstRecAttr->stLoopAttr.pfnDelFile;
        MLOGD("loop strategy open: loop num[%u], file idx[%u]\n",
            pstTskCtx->stLoopMngInfo.u32LoopNum, pstTskCtx->stLoopMngInfo.u32FileIdx);
    }

    /* stream attribute */
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FileIdx;
    HI_U32 u32StreamCnt;
    HI_U32 u32StreamIdx;
    HI_U32 u32TrackCnt;
    RECMNG_FILE_INFO_S *pstFileInfo = HI_NULL;
    RECMNG_STREAM_INFO_S *pstStreamInfo = HI_NULL;
    pstTskCtx->u32StreamCnt = 0;

    for (u32FileIdx = 0; u32FileIdx < pstRecAttr->u32FileCnt; ++u32FileIdx)
    {
        pstFileInfo = &pstTskCtx->astFileInfo[u32FileIdx];

        /** check data source's correctness */
        HI_U32 u32VencIdx;
        HI_U32 u32VencCnt = 0;
        /** check venc */
        if(0 == pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt
            || pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt > HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT)
        {
            MLOGE("File[%u]'s VencCnt[%u] out of rang\n", u32FileIdx,
                pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt);
            return HI_RECMNG_EINVAL;
        }

        for(u32VencIdx = 0; u32VencIdx < pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt; ++u32VencIdx)
        {
            if(HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aVencHdl[u32VencIdx])
            {
                ++u32VencCnt;
            }
        }

        if(u32VencCnt != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt)
        {
            MLOGE("File[%u]'s VencCnt[%u] is not right\n", u32FileIdx,
                pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt);
            return HI_RECMNG_EINVAL;
        }
        /** check aenc */
        HI_U32 u32AencIdx;
        HI_U32 u32AencCnt = 0;
        if(pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt > HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT)
        {
            MLOGE("File[%u]'s AencCnt[%u] out of rang\n", u32FileIdx,
                pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt);
            return HI_RECMNG_EINVAL;
        }

        for(u32AencIdx = 0; u32AencIdx < pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt; ++u32AencIdx)
        {
            if(HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aAencHdl[u32AencIdx])
            {
                ++u32AencCnt;
            }
        }

        if(u32AencCnt != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt)
        {
            MLOGE("File[%u]'s AencCnt[%u] is not right\n",
                u32FileIdx, pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt);
            return HI_RECMNG_EINVAL;
        }
        /** check private data */
        HI_U32 u32PrivDataIdx;
        HI_U32 u32PrivDataCnt = 0;
        if(pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt > HI_RECMNG_FILE_MAX_PRIV_DATA_TRACK_CNT)
        {
            MLOGE("File[%u]'s PrivDataCnt[%u] out of rang\n", u32FileIdx,
                pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt);
            return HI_RECMNG_EINVAL;
        }

        for(u32PrivDataIdx = 0; u32PrivDataIdx < pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt; ++u32PrivDataIdx)
        {
            if(HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aPrivDataHdl[u32PrivDataIdx])
            {
                ++u32PrivDataCnt;
            }
        }

        if(u32PrivDataCnt != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt)
        {
            MLOGE("File[%u]'s PrivDataCnt[%u] is not right\n",
                u32FileIdx, pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt);
            return HI_RECMNG_EINVAL;
        }

        if((pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt > 0) &&
            (HI_NULL == stRECMNGMediaOps.pfnPrivDataStart ||
            HI_NULL == stRECMNGMediaOps.pfnPrivDataStop ||
            HI_NULL == stRECMNGMediaOps.pfnGetPrivDataCfg))
        {
            MLOGE("The callback function of private data is NULL\n");
            return HI_RECMNG_EINVAL;
        }
        /** end of check data source's correctness */

        u32StreamCnt = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32VencCnt;
        for(u32StreamIdx = 0; u32StreamIdx < u32StreamCnt; ++u32StreamIdx)
        {
            pstStreamInfo = &pstFileInfo->astStreamAttr[u32StreamIdx];
            u32TrackCnt = 0;

            /** Video Track */
            if(HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aVencHdl[u32StreamIdx])
            {
                pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.s32PrivateHandle
                    = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aVencHdl[u32StreamIdx];
                pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStart = RECMNG_StartVideoSource;
                pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStop = RECMNG_StopVideoSource;
                pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;

                /* Video Information */
                s32Ret = RECMNG_GetVideoInfo(pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aVencHdl[u32StreamIdx],
                    &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                pstStreamInfo->astTrackSource[u32TrackCnt].ppRecObj = &pstTskCtx->pRecObj;

                if (pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps > 0)
                {
                    if (HI_REC_TYPE_LAPSE == pstRecAttr->enRecType)
                    {
                        pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed
                            = (HI_FLOAT)pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps * pstRecAttr->unRecordAttr.stLapseAttr.u32IntervalMs / 1000;
		    MLOGI("VENC u32BateRate = %d \n", pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate);
                        if(pstRecAttr->unRecordAttr.stLapseAttr.u32IntervalMs <= 64000)
                        {
                            pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate
                                *= pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed;
				MLOGI("LAPSE Mode u32BateRate = %d \n", pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate);
                        }

                        pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate
                            = pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps;
		MLOGI("LAPSE Mode u32PlayFps = %d \n", pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate);
                    }
                    else /** normal record */
                    {
                        /** whether insert frame */
                        if(pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps
                            > pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate)
                        {
                            pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed = 1;
                            pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32Gop
                                = pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps;
                            pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate
                                = pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps;

                            pstStreamInfo->astTrackSource[u32TrackCnt].u32FrameInterval
                                = 1000000 / pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps;

                            pstStreamInfo->astTrackSource[u32TrackCnt].bInsertFrame = HI_TRUE;
                        }
                        else
                        {
                            pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed
                                = (HI_FLOAT)pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps
                                / pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate;

                            pstStreamInfo->astTrackSource[u32TrackCnt].bInsertFrame = HI_FALSE;
                        }
                    }
                    MLOGI("venc frame rate: %u, play frame rate: %u, fspeed: %f\n",
                        pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate,
                        pstRecAttr->astFileAttr[u32FileIdx].s32PlayFps,
                        pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed);
                }
                else if (HI_REC_TYPE_LAPSE == pstRecAttr->enRecType)
                {
                    MLOGE("lapse type: play fps must be assigned\n");
                    return HI_RECMNG_EINVAL;
                }

                stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]
                    = &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource;
                MLOGD("Stream[%d] VideoTrackSourceHdl[%p]\n", stRecAttr.u32StreamCnt,
                    stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]);

                u32TrackCnt++;
            }
            else
            {
                MLOGE("Video data source handle can not be -1\n");
                return HI_RECMNG_EINVAL;
            }

            HI_U32 u32AencIdx;
            HI_U32 u32AencCnt = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32AencCnt;
            for(u32AencIdx = 0; u32AencIdx < u32AencCnt; ++u32AencIdx)
            {
                /** add audio track in descending order.
                    if you want to add audio track in ascending order.
                    you should  change "(u32StreamCnt - 1 - u32StreamIdx == u32AencIdx)" to "(u32StreamIdx == u32AencIdx)" */
                if((u32StreamCnt - 1 - u32StreamIdx == u32AencIdx) &&
                    HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aAencHdl[u32AencIdx])
                {
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.s32PrivateHandle
                        = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aAencHdl[u32AencIdx];
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStart = RECMNG_StartAudioSource;
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStop = RECMNG_StopAudioSource;
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;

                    /* Audio Information */
                    s32Ret = RECMNG_GetAudioInfo(pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aAencHdl[u32AencIdx],
                        &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stAudioInfo);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    pstStreamInfo->astTrackSource[u32TrackCnt].ppRecObj = &pstTskCtx->pRecObj;

                    stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]
                        = &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource;
                    MLOGD("Stream[%d] AudioTrackSourceHdl[%p]\n", stRecAttr.u32StreamCnt,
                        stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]);

                    u32TrackCnt++;
                }
            }

            HI_U32 u32PrivDataIdx;
            HI_U32 u32PrivDataCnt = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.u32PrivDataCnt;
            for(u32PrivDataIdx = 0; u32PrivDataIdx < u32PrivDataCnt; ++u32PrivDataIdx)
            {
                /** add private data track in descending order.
                    if you want to add private data track in ascending order.
                    you should  change "(u32StreamCnt - 1 - u32StreamIdx == u32PrivDataIdx)" to "(u32StreamIdx == u32PrivDataIdx)" */
                if((u32StreamCnt - 1 - u32StreamIdx == u32PrivDataIdx) &&
                    HI_INVALID_HANDLE != pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aPrivDataHdl[u32PrivDataIdx])
                {
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.s32PrivateHandle =
                    pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aPrivDataHdl[u32PrivDataIdx];
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStart = RECMNG_StartPrivDataSource;
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.pfnSourceStop = RECMNG_StopPrivDataSource;
                    pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_PRIV;


                    s32Ret = RECMNG_GetPrivDataInfo(pstRecAttr->astFileAttr[u32FileIdx].stDataSource.aPrivDataHdl[u32PrivDataIdx],
                        &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource.unTrackSourceAttr.stPrivInfo);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    pstStreamInfo->astTrackSource[u32TrackCnt].ppRecObj = &pstTskCtx->pRecObj;

                    stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]
                        = &pstStreamInfo->astTrackSource[u32TrackCnt].stTrackSource;
                    MLOGD("Stream[%d] PrivDataTrackSourceHdl[%p]\n", stRecAttr.u32StreamCnt,
                        stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].aHTrackSrcHandle[u32TrackCnt]);

                    u32TrackCnt++;
                }
            }

            if(u32StreamIdx == 0) /** main stream */
            {
                /* Muxer Configure */
                s32Ret = RECMNG_GetMuxerConfig(&pstRecAttr->astFileAttr[u32FileIdx].stMuxerCfg, &pstFileInfo->stMuxerCfg);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].stMuxerOps.pMuxerCfg = (HI_VOID*)&pstFileInfo->stMuxerCfg;

                /* Muxer Ops */
                s32Ret = RECMNG_GetMuxerOps(pstRecAttr->astFileAttr[u32FileIdx].stMuxerCfg.enMuxerType,
                    &stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].stMuxerOps);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            else /** slave stream */
            {
                /** Muxer configure */
                stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].stMuxerOps.pMuxerCfg = (HI_VOID*)&pstFileInfo->stMuxerCfg;

                /* Muxer Ops */
                s32Ret = RECMNG_GetSlaveMuxerOps(pstRecAttr->astFileAttr[u32FileIdx].stMuxerCfg.enMuxerType,
                    &stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].stMuxerOps);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            s32Ret = RECMNG_GetBufOps(&pstRecAttr->astFileAttr[u32FileIdx].astBufCfg[u32StreamIdx],
                &pstStreamInfo->stBufInfo, &stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].stBufOps);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            ++(pstTskCtx->u32StreamCnt);
            pstStreamInfo->u32TrackCnt = u32TrackCnt;
            MLOGD("file[%u] stream[%u], track count[%u]\n", u32FileIdx, u32StreamIdx, u32TrackCnt);

            stRecAttr.astStreamAttr[stRecAttr.u32StreamCnt].u32TrackCnt = u32TrackCnt;
            stRecAttr.u32StreamCnt++;
        } /** end of stream loop */
        pstFileInfo->u32StreamCnt = u32StreamCnt;
        if(HI_RECMNG_THM_TYPE_EMBEDDED == pstRecAttr->enThmType)
        {
            pstFileInfo->ThmHdl = pstRecAttr->astFileAttr[u32FileIdx].stDataSource.ThmHdl;
            MLOGI(GREEN"FileIdx[%u] ThmHdl[%d]\n"NONE, u32FileIdx, pstRecAttr->astFileAttr[u32FileIdx].stDataSource.ThmHdl);
        }
    } /** end of file loop */

    pstTskCtx->u32FileCnt = pstRecAttr->u32FileCnt;
    MLOGD("file cnt[%u]\n", pstTskCtx->u32FileCnt);

    for(u32Idx = 0; u32Idx < pstTskCtx->u32FileCnt; ++u32Idx)
    {
        HI_MUTEX_INIT_LOCK(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
    }

    s32Ret = HI_REC_Create(&stRecAttr, &pstTskCtx->pRecObj);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    RECMNG_SetTaskState(*pHdl, RECMNG_TASK_STATE_READY);
    MLOGD("record handle = %d\n", *pHdl);

    s32Ret = HI_REC_RegisterEventCallback(pstTskCtx->pRecObj, RECMNG_EventPreProc);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    pstTskCtx->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_DestroyTask(HI_HANDLE Hdl)
{
    MLOGI(GREEN"DestroyTask[%d]\n"NONE, Hdl);
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_READY:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            break;

        default:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is busy\n", Hdl);
            return HI_RECMNG_EBUSY;
    }

    RECMNG_TASK_CONTEXT_S *pstTskCtx = &s_astRECMNGTaskCtx[Hdl];

    HI_S32 s32Ret = HI_FAILURE;
    s32Ret = HI_REC_Destroy(pstTskCtx->pRecObj);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_REC_Destroy(%p) failed[0x%08X]\n", pstTskCtx->pRecObj, s32Ret);
        return HI_FAILURE;
    }

    HI_U32 u32Idx;
    for(u32Idx = 0; u32Idx < pstTskCtx->u32FileCnt; ++u32Idx)
    {
        HI_MUTEX_DESTROY(pstTskCtx->astFileInfo[u32Idx].stMuxerCfg.muxerMutex);
    }

    pstTskCtx->bUsed = HI_FALSE;
    memset(&pstTskCtx->astFileInfo, 0x0, sizeof(RECMNG_FILE_INFO_S) * HI_RECMNG_TASK_MAX_FILE_CNT);
    pstTskCtx->u32RecFileCnt = 0;
    memset(&pstTskCtx->stFilename, 0x0, sizeof(HI_RECMNG_FILENAME_S));
    memset(&pstTskCtx->stLoopMngInfo, 0x0, sizeof(RECMNG_LOOP_MNG_INFO_S));
    RECMNG_SetTaskState(Hdl, RECMNG_TASK_STATE_BUTT);

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_StartTask(HI_HANDLE Hdl, HI_S32 s32Timeout_ms)
{
    MLOGI(GREEN"StartTask[%d]\n"NONE, Hdl);
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_STARTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is starting\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_STOPPING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is stopping\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_SPLITTING:
        case RECMNG_TASK_STATE_STARTED:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGD("task[%d] has already started\n", Hdl);
            return HI_SUCCESS;

        case RECMNG_TASK_STATE_READY:
        default:

            break;
    }
    s_astRECMNGTaskCtx[Hdl].enTaskState = RECMNG_TASK_STATE_STARTING;
    HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);

    HI_S32 s32Ret = HI_REC_Start(s_astRECMNGTaskCtx[Hdl].pRecObj);
    if(HI_SUCCESS != s32Ret)
    {
        RECMNG_SetTaskState(Hdl, RECMNG_TASK_STATE_READY);
        MLOGE(RED"start rec failure, ret[%08x]\n"NONE, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = RECMNG_WaitState(Hdl, RECMNG_TASK_STATE_STARTED, s32Timeout_ms);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_StopTask(HI_HANDLE Hdl, HI_BOOL bQuickMode, HI_S32 s32Timeout_ms)
{
    MLOGI(GREEN"StopTask[%d]\n"NONE, Hdl);
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_READY:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGD("task[%d] has already stoped\n", Hdl);
            return HI_SUCCESS;

        case RECMNG_TASK_STATE_STARTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is starting\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_STOPPING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is stopping\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_SPLITTING:
            s_astRECMNGTaskCtx[Hdl].bStopAfterManualSplitEnd = HI_FALSE;
            break;

        case RECMNG_TASK_STATE_STARTED:
        default:
            break;
    }

    RECMNG_TASK_STATE_E enTempTaskState = s_astRECMNGTaskCtx[Hdl].enTaskState;
    s_astRECMNGTaskCtx[Hdl].enTaskState = RECMNG_TASK_STATE_STOPPING;
    HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);

    HI_S32 s32Ret = HI_REC_Stop(s_astRECMNGTaskCtx[Hdl].pRecObj, bQuickMode);
    if(HI_SUCCESS != s32Ret)
    {
        RECMNG_SetTaskState(Hdl, enTempTaskState);
        MLOGE(RED"stop rec failure, ret[%08x]\n"NONE, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = RECMNG_WaitState(Hdl, RECMNG_TASK_STATE_READY, s32Timeout_ms);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_AsyncStopTask(HI_HANDLE Hdl, HI_BOOL bQuickMode)
{
    MLOGI(GREEN"AsyncStopTask[%d]\n"NONE, Hdl);

#ifdef RECMNG_SUPPORT_ASYNC_STOP
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_READY:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGD("task[%d] has already stoped\n", Hdl);
            return HI_SUCCESS;

        case RECMNG_TASK_STATE_STARTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is starting\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_STOPPING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is stopping\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_SPLITTING:
            s_astRECMNGTaskCtx[Hdl].bStopAfterManualSplitEnd = HI_FALSE;
            break;

        case RECMNG_TASK_STATE_STARTED:
        default:
            break;
    }

    RECMNG_TASK_STATE_E enTempTaskState = s_astRECMNGTaskCtx[Hdl].enTaskState;
    s_astRECMNGTaskCtx[Hdl].enTaskState = RECMNG_TASK_STATE_STOPPING;
    HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);

    HI_S32 s32FileIndex = 0;

    for(s32FileIndex = 0; s32FileIndex < s_astRECMNGTaskCtx[Hdl].u32FileCnt; ++s32FileIndex)
    {
        HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].astFileInfo[s32FileIndex].stMuxerCfg.muxerMutex);
        if(HI_NULL_PTR != s_astRECMNGTaskCtx[Hdl].astFileInfo[s32FileIndex].stMuxerCfg.pMuxerObj)
        {
            s_astRECMNGTaskCtx[Hdl].astFileInfo[s32FileIndex].stMuxerCfg.bAsyncDestroy = HI_TRUE;
        }
        else
        {
            MLOGI(GREEN"Task[%d] File[%d] may not create or has been destroyed\n"NONE, Hdl, s32FileIndex);
        }
        HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].astFileInfo[s32FileIndex].stMuxerCfg.muxerMutex);
    }

    HI_S32 s32Ret = HI_REC_Stop(s_astRECMNGTaskCtx[Hdl].pRecObj, bQuickMode);
    if(HI_SUCCESS != s32Ret)
    {
        RECMNG_SetTaskState(Hdl, enTempTaskState);
        MLOGE(RED"stop rec failure, ret[%08x]\n"NONE, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = RECMNG_WaitState(Hdl, RECMNG_TASK_STATE_READY, -1);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#else
    HI_RECMNG_StopTask(Hdl, bQuickMode, -1);
#endif
    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_ManualSplit(HI_HANDLE Hdl, HI_REC_MANUAL_SPLIT_ATTR_S *pstSplitAttr, HI_BOOL bStopAfterManualSplitEnd)
{
    MLOGI(GREEN"ManualSplit[%d]"NONE, Hdl);
    RECMNG_CHECK_INIT();
    RECMNG_CHECK_TSKINDEX(Hdl);
    RECMNG_CHECK_USED(Hdl);

    HI_MUTEX_LOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
    switch (s_astRECMNGTaskCtx[Hdl].enTaskState)
    {
        case RECMNG_TASK_STATE_READY:
        case RECMNG_TASK_STATE_STARTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] not started yet\n", Hdl);
            return HI_RECMNG_ETASK_NOT_STARTED;

        case RECMNG_TASK_STATE_STOPPING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGE("task[%d] is stopping\n", Hdl);
            return HI_RECMNG_EBUSY;

        case RECMNG_TASK_STATE_SPLITTING:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            MLOGD("task[%d] last manual split not finished\n", Hdl);
            return HI_RECMNG_ESPLIT_NOT_FINISHED;

        case RECMNG_TASK_STATE_STARTED:
        default:
            HI_MUTEX_UNLOCK(s_astRECMNGTaskCtx[Hdl].stateMutex);
            break;
    }
    HI_S32 s32Ret = HI_REC_ManualSplit(s_astRECMNGTaskCtx[Hdl].pRecObj, pstSplitAttr);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
    RECMNG_SetTaskState(Hdl, RECMNG_TASK_STATE_SPLITTING);
    s_astRECMNGTaskCtx[Hdl].bStopAfterManualSplitEnd = bStopAfterManualSplitEnd;
    return HI_SUCCESS;
}

HI_S32 HI_RECMNG_WritePrivateData(HI_HANDLE PrivDataHdl, HI_RECMNG_PRIV_DATA_S *pstPrivData, HI_VOID* pvPrivDataInfo)
{
    HI_S32 s32Ret;
    HI_REC_FRAME_DATA_S stFrameData;
    stFrameData.u64TimeStamp = pstPrivData->u64TimeStamp;
    stFrameData.apu8SliceAddr[0] = pstPrivData->pu8DataBuffer;
    stFrameData.au32SliceLen[0] = pstPrivData->u32DataLength;
    stFrameData.bKeyFrameFlag = HI_FALSE;
    stFrameData.u32SliceCnt = RECMNG_PRIV_DATA_SLICE_COUNT;

    RECMNG_TRACK_SOURCE_S *pstTrackSource = (RECMNG_TRACK_SOURCE_S*)pvPrivDataInfo;
    HI_APPCOMM_CHECK_POINTER(pstTrackSource->ppRecObj, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(*pstTrackSource->ppRecObj, HI_FAILURE);

    s32Ret = HI_REC_WriteData(*pstTrackSource->ppRecObj, &pstTrackSource->stTrackSource, &stFrameData);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

