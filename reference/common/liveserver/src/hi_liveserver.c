/**
 * @file    hi_liveserver.c
 * @brief   implementation of live server interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/14
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/** head file of ndk module */
#include "hi_mapi_venc.h"
#include "hi_mapi_aenc.h"

/** head file of middleware module */
#include "hi_rtsp_server.h"
#include "hi_eventhub.h"

/** head file of appcommom module */
#include "hi_liveserver.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "LiveSvr"



/** length of the send packet, value[500,5000], recommend 1500, unit:byte */
#define LIVESVR_RTSP_PACKET_LEN (1500)

/** listen port of rtspserver, value[1,65535], normal 554 */
#define LIVESVR_RTSP_LISTEN_PORT (554)

/** max payload type num in mbuffer, value[1,254], normal 2 */
#define LIVESVR_RTSP_MAX_PAYLOAD_TYPE_NUM (3)

/** timeout of connect, <0 for not use timeout, unit:s */
#define LIVESVR_RTSP_TIMEOUT_SEC (6)


/** max number of media source */
#define LIVESVR_RTSP_STREAM_MAX_CNT (4)

/** min mbuffer size, 216K */
#define RTSP_MIN_MBUFFER_SIZE (216 << 10)
/** max mbuffer size, 50M */
#define RTSP_MAX_MBUFFER_SIZE (50 << 20)

/** 16K for audio */
#define LIVESVR_AUDIO_MBUFFER_SIZE (16 << 10)
/** the number of video frame */
#define LIVESVR_VIDEO_MBUFFER_FRAME (5)

#define LIVESVR_CHECK_POINTER(p) \
    do{ \
        if(!(p)) \
        { \
            MLOGE("Null pointer exception.\n"); \
        } \
    }while(0)

/** check module init state */
#define LIVESVR_CHECK_INIT() \
    do{ \
        if(!s_bLIVESVRInitFlg) \
        { \
            MLOGE("LiveSvr not init yet.\n"); \
            return HI_LIVESVR_ENOINIT; \
        } \
    }while(0)

/** rtsp source context */
typedef struct tagLIVESVR_RTSP_STREAM_CONTEXT_S
{
    HI_BOOL bUsed;
    HI_CHAR szStreamName[HI_APPCOMM_MAX_PATH_LEN];
    HI_Track_Source_S stVideoTrackSource;
    HI_Track_Source_S stAudioTrackSource;
} LIVESVR_RTSP_STREAM_CONTEXT_S;

/** liveserver init flag */
static HI_BOOL s_bLIVESVRInitFlg = HI_FALSE;

 /** media operate */
static HI_LIVESVR_MEDIA_OPERATE_S s_stLIVESVRMediaOps;

/** rtsp server object, created by rtsp server module */
static HI_MW_PTR s_pLIVESVRRtspSvrObj;

/** array of rtsp source context */
static LIVESVR_RTSP_STREAM_CONTEXT_S s_astRtspStreamCtx[LIVESVR_RTSP_STREAM_MAX_CNT];



HI_S32 LIVESVR_VencDataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pstVideoData, HI_VOID *pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);

    HI_U32 u32Idx;
    HI_RTSP_DATA_S stFrameData;
    stFrameData.u32BlockCnt = 0;

    for(u32Idx = 0; u32Idx < pstVideoData->u32PackCount; ++u32Idx)
    {
        HI_U8 *pu8PackVirtAddr = pstVideoData->astPack[u32Idx].apu8Addr[0];
        HI_U64 u64PackPhyAddr = pstVideoData->astPack[u32Idx].au64PhyAddr[0];
        HI_U32 u32PackLen = pstVideoData->astPack[u32Idx].au32Len[0];
        HI_U32 u32PackOffset = pstVideoData->astPack[u32Idx].u32Offset;

        HI_U8 *pu8DataVirtAddr = pstVideoData->astPack[u32Idx].apu8Addr[1];
        HI_U64 u64DataPhyAddr = pstVideoData->astPack[u32Idx].au64PhyAddr[1];
        HI_U32 u32DataLen = pstVideoData->astPack[u32Idx].au32Len[1];


        if (u64PackPhyAddr + u32PackLen >= u64DataPhyAddr + u32DataLen)
        {
            /** physical address retrace in offset segment */
            if (u64PackPhyAddr + u32PackOffset >= u64DataPhyAddr + u32DataLen)
            {
                HI_U8 *pu8SrcVirtAddr = pu8DataVirtAddr
                    + ((u64PackPhyAddr + u32PackOffset) - (u64DataPhyAddr + u32DataLen));
                if(u32PackLen>u32PackOffset)
                {
                    stFrameData.apu8DataPtr[stFrameData.u32BlockCnt] = pu8SrcVirtAddr;
                    stFrameData.au32DataLen[stFrameData.u32BlockCnt] = u32PackLen - u32PackOffset;
                    stFrameData.u32BlockCnt++;
                }
            }
            else
            {
                /** physical address retrace in data segment */
                HI_U32 u32Left = (u64DataPhyAddr + u32DataLen) - u64PackPhyAddr;
                if(u32Left>u32PackOffset)
                {
                    stFrameData.apu8DataPtr[stFrameData.u32BlockCnt] = pu8PackVirtAddr + u32PackOffset;
                    stFrameData.au32DataLen[stFrameData.u32BlockCnt] = u32Left - u32PackOffset;
                    stFrameData.u32BlockCnt++;
                }
                if(u32PackLen>u32Left)
                {
                    stFrameData.apu8DataPtr[stFrameData.u32BlockCnt]= pu8DataVirtAddr;
                    stFrameData.au32DataLen[stFrameData.u32BlockCnt]= u32PackLen - u32Left;
                    stFrameData.u32BlockCnt++;
                }
            }
        }
        else
        {
            /** physical address retrace does not happen */
            if(u32PackLen>u32PackOffset)
            {
                stFrameData.apu8DataPtr[stFrameData.u32BlockCnt] = pu8PackVirtAddr + u32PackOffset;
                stFrameData.au32DataLen[stFrameData.u32BlockCnt] = u32PackLen - u32PackOffset;
                stFrameData.u32BlockCnt++;
            }
        }


    }

    stFrameData.u64Pts = pstVideoData->astPack[0].u64PTS;
    stFrameData.u32Seq = pstVideoData->u32Seq;

    u32Idx = pstVideoData->u32PackCount - 1;
    switch (pstVideoData->astPack[u32Idx].stDataType.enPayloadType)
    {
        case HI_MAPI_PAYLOAD_TYPE_H264:
            stFrameData.bIsKeyFrame = (H264E_NALU_IDRSLICE == pstVideoData->astPack[u32Idx].stDataType.enH264EType
                || H264E_NALU_ISLICE == pstVideoData->astPack[u32Idx].stDataType.enH264EType
                || H264E_NALU_SPS  == pstVideoData->astPack[u32Idx].stDataType.enH264EType) ? HI_TRUE : HI_FALSE;
            break;

        case HI_MAPI_PAYLOAD_TYPE_H265:
            stFrameData.bIsKeyFrame = (H265E_NALU_IDRSLICE == pstVideoData->astPack[u32Idx].stDataType.enH265EType
                || H265E_NALU_ISLICE == pstVideoData->astPack[u32Idx].stDataType.enH265EType
                || H265E_NALU_SPS == pstVideoData->astPack[u32Idx].stDataType.enH265EType) ? HI_TRUE : HI_FALSE;
            break;

        default:
            stFrameData.bIsKeyFrame = HI_FALSE;
            break;
    }

    HI_S32 s32Ret;
    s32Ret = HI_RTSPSVR_WriteFrame(s_pLIVESVRRtspSvrObj, (HI_Track_Source_Handle)pvPrivData, &stFrameData);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_RTSPSVR_WriteFrame");

    return HI_SUCCESS;
}

HI_S32 LIVESVR_AEncDataProc(HI_HANDLE AencHdl, const AUDIO_STREAM_S *pstAudioData, HI_VOID *pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pstAudioData, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pvPrivData, HI_FAILURE);

    HI_RTSP_DATA_S stFrameData;
    stFrameData.apu8DataPtr[0] = pstAudioData->pStream;
    stFrameData.au32DataLen[0] = pstAudioData->u32Len;
    stFrameData.u64Pts = pstAudioData->u64TimeStamp;
    stFrameData.u32Seq = pstAudioData->u32Seq;
    stFrameData.u32BlockCnt = 1;
    stFrameData.bIsKeyFrame = HI_FALSE;

    return HI_RTSPSVR_WriteFrame(s_pLIVESVRRtspSvrObj, (HI_Track_Source_Handle)pvPrivData, &stFrameData);
}

HI_S32 LIVESVR_StartVideoSource(HI_Track_Source_Handle pstTrackSource, HI_MW_PTR pRtspSvrObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRtspSvrObj, HI_FAILURE);

    HI_S32 s32Ret;

    HI_MAPI_VENC_CALLBACK_S stVencCB;
    stVencCB.pfnDataCB = LIVESVR_VencDataProc;
    stVencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_VENC_RegisterCallback(pstTrackSource->s32PrivateHandle, &stVencCB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VENC_RegisterCallback failed[0x%08x]\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = s_stLIVESVRMediaOps.pfnVencStart(pstTrackSource->s32PrivateHandle, HI_MAPI_VENC_LIMITLESS_FRAME_COUNT);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_VencStart");

    return HI_SUCCESS;
}

HI_S32 LIVESVR_StopVideoSource(HI_Track_Source_Handle pstTrackSource, HI_MW_PTR pRtspSvrObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRtspSvrObj, HI_FAILURE);

    HI_S32 s32Ret;

    s32Ret = s_stLIVESVRMediaOps.pfnVencStop(pstTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_VencStop");

    HI_MAPI_VENC_CALLBACK_S stVencCB;
    stVencCB.pfnDataCB = LIVESVR_VencDataProc;
    stVencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_VENC_UnRegisterCallback(pstTrackSource->s32PrivateHandle, &stVencCB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VENC_UnRegisterCallback failed[0x%08x]\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 LIVESVR_StartAudioSource(HI_Track_Source_Handle pstTrackSource, HI_MW_PTR pRtspSvrObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRtspSvrObj, HI_FAILURE);

    HI_S32 s32Ret;

    HI_MAPI_AENC_CALLBACK_S stAencCB;
    stAencCB.pfnDataCB = LIVESVR_AEncDataProc;
    stAencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_AENC_RegisterCallback(pstTrackSource->s32PrivateHandle, &stAencCB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_AENC_RegisterCallback failed[0x%08x]\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = s_stLIVESVRMediaOps.pfnAencStart(pstTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_AencStart");

    return HI_SUCCESS;
}

HI_S32 LIVESVR_StopAudioSource(HI_Track_Source_Handle pstTrackSource, HI_MW_PTR pRtspSvrObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRtspSvrObj, HI_FAILURE);

    HI_S32 s32Ret;

    s32Ret = s_stLIVESVRMediaOps.pfnAencStop(pstTrackSource->s32PrivateHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_MEDIA_AencStop");

    HI_MAPI_AENC_CALLBACK_S stAencCB;
    stAencCB.pfnDataCB = LIVESVR_AEncDataProc;
    stAencCB.pPrivateData = (HI_VOID *)pstTrackSource;

    s32Ret = HI_MAPI_AENC_UnregisterCallback(pstTrackSource->s32PrivateHandle, &stAencCB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_AENC_UnregisterCallback failed[0x%08x]\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 LIVESVR_RequestKeyFrame(HI_Track_Source_Handle pstTrackSource, HI_MW_PTR pRtspSvrObj)
{
    HI_APPCOMM_CHECK_POINTER(pstTrackSource, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pRtspSvrObj, HI_FAILURE);

    HI_S32 s32Ret;

    s32Ret = HI_MAPI_VENC_RequestIDR(pstTrackSource->s32PrivateHandle);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VEnc_RequestIDR failed s32Ret[0x%08X]\n", s32Ret);
        return HI_FAILURE;
    }
    MLOGD("HI_MAPI_VEnc_RequestIDR success, VencHdl[%d]\n", pstTrackSource->s32PrivateHandle);

    return HI_SUCCESS;
}

static HI_S32 LIVESVR_GetVideoInfo(HI_HANDLE VencHdl, HI_Track_VideoSourceInfo_S *pstVideoInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoInfo, HI_LIVESVR_EINVAL);

    HI_S32 s32Ret;
    HI_MEDIA_VIDEOINFO_S stVideoInfo;

    s32Ret = s_stLIVESVRMediaOps.pfnGetVideoInfo(VencHdl, &stVideoInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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
            else if(HI_MAPI_VENC_RC_MODE_QVBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H264_QVBR_S *pstVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH264QVbr;
                pstVideoInfo->u32Gop = pstVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstVbr->stAttr.u32TargetBitRate << 10;
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
            else if(HI_MAPI_VENC_RC_MODE_QVBR == stVideoInfo.stVencAttr.stRcAttr.enRcMode)
            {
                HI_MEDIA_VENC_ATTR_H265_QVBR_S *pstVbr = &stVideoInfo.stVencAttr.stRcAttr.unAttr.stH265QVbr;
                pstVideoInfo->u32Gop = pstVbr->stAttr.u32Gop;
                pstVideoInfo->u32FrameRate = pstVbr->stAttr.fr32DstFrameRate;
                pstVideoInfo->u32BitRate = pstVbr->stAttr.u32TargetBitRate << 10;
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
    MLOGD("CodecType[%d] RcMode[%d] FrameRate[%ufps] Gop[%u] BitRate[%ukbps]\n", pstVideoInfo->enCodecType,
        stVideoInfo.stVencAttr.stRcAttr.enRcMode, pstVideoInfo->u32FrameRate, pstVideoInfo->u32Gop, pstVideoInfo->u32BitRate >> 10);

    /** Resolution */
    pstVideoInfo->u32Width = stVideoInfo.stVencAttr.stTypeAttr.u32Width;
    pstVideoInfo->u32Height = stVideoInfo.stVencAttr.stTypeAttr.u32Height;
    MLOGD("Video Resolution[%ux%u]\n", pstVideoInfo->u32Width, pstVideoInfo->u32Height);

    /** PlayRate = FrameRate */
    pstVideoInfo->fSpeed = 1;

    return HI_SUCCESS;
}

static HI_S32 LIVESVR_GetAudioInfo(HI_HANDLE AencHdl, HI_Track_AudioSourceInfo_S *pstAudioInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstAudioInfo, HI_LIVESVR_EINVAL);

    HI_S32 s32Ret;
    HI_MEDIA_AUDIOINFO_S stAudioInfo;

    s32Ret = s_stLIVESVRMediaOps.pfnGetAudioInfo(AencHdl, &stAudioInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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

HI_VOID LIVESVR_OnClientConnect(HI_SERVER_STATE_LISTENER_S* pstListener, HI_CHAR* pszIP)
{
    LIVESVR_CHECK_POINTER(pstListener);
    LIVESVR_CHECK_POINTER(pszIP);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_LIVESVR_CLIENT_CONNECT;
    snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pszIP);
    HI_EVTHUB_Publish(&stEvent);

    MLOGD("Client connect, ip[%s]\n", pszIP);
    return;
}

HI_VOID LIVESVR_OnClientDisconnect(HI_SERVER_STATE_LISTENER_S* pstListener, HI_CHAR* pszIP)
{
    LIVESVR_CHECK_POINTER(pstListener);
    LIVESVR_CHECK_POINTER(pszIP);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_LIVESVR_CLIENT_DISCONNECT;
    snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pszIP);
    HI_EVTHUB_Publish(&stEvent);

    MLOGD("Client disconnect, ip[%s].\n", pszIP);
    return;
}

HI_VOID LIVESVR_OnServerError(HI_SERVER_STATE_LISTENER_S* pstListener, HI_S32 s32ErrId, HI_CHAR* pszMsg)
{
    LIVESVR_CHECK_POINTER(pstListener);
    LIVESVR_CHECK_POINTER(pszMsg);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_LIVESVR_SERVER_ERROR;
    stEvent.s32Result = s32ErrId;
    snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, "%s", pszMsg);
    HI_EVTHUB_Publish(&stEvent);

    MLOGD("Live server error, error id[%d] msd[%s].\n", s32ErrId, pszMsg);
    return;
}

static HI_S32 LIVESVR_RTSP_ServerInit(HI_S32 s32MaxConnNum)
{
    HI_S32 s32Ret;
    HI_RTSP_CONFIG_S stRTSPConf;

    stRTSPConf.s32ListenPort = LIVESVR_RTSP_LISTEN_PORT;
    stRTSPConf.s32MaxConnNum = s32MaxConnNum;
    stRTSPConf.s32MaxPayload = LIVESVR_RTSP_MAX_PAYLOAD_TYPE_NUM;
    stRTSPConf.s32PacketLen = LIVESVR_RTSP_PACKET_LEN;
    stRTSPConf.s32Timeout = LIVESVR_RTSP_TIMEOUT_SEC;
    //stRTSPConf.s32Timeout = 0;

    s32Ret = HI_RTSPSVR_Create(&s_pLIVESVRRtspSvrObj, &stRTSPConf);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_RTSPSVR_Create");

    /** add the state listener of rtsp server */
    HI_SERVER_STATE_LISTENER_S stStateListener;
    stStateListener.onClientConnect = LIVESVR_OnClientConnect;
    stStateListener.onClientDisconnect = LIVESVR_OnClientDisconnect;
    stStateListener.onServerError = LIVESVR_OnServerError;

    s32Ret = HI_RTSPSVR_SetListener(s_pLIVESVRRtspSvrObj, &stStateListener);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"Set rtsp server listener failed[%08x].\n"NONE, s32Ret);
        (HI_VOID)HI_RTSPSVR_Destroy(s_pLIVESVRRtspSvrObj);
        return HI_FAILURE;
    }

    /** make rtsp server running */
    s32Ret = HI_RTSPSVR_Start(s_pLIVESVRRtspSvrObj);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"start rtsp server failed[%08x].\n"NONE, s32Ret);
        (HI_VOID)HI_RTSPSVR_Destroy(s_pLIVESVRRtspSvrObj);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 LIVESVR_RTSP_ServerDeinit(HI_VOID)
{
    HI_S32 s32Ret;

    s32Ret = HI_RTSPSVR_Stop(s_pLIVESVRRtspSvrObj);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_RTSPSVR_Stop failed[0x%08X]\n", s32Ret);
    }

    s32Ret = HI_RTSPSVR_Destroy(s_pLIVESVRRtspSvrObj);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_RTSPSVR_Destroy failed[0x%08X]\n", s32Ret);
    }

    return HI_SUCCESS;
}



HI_S32 HI_LIVESVR_Init(HI_S32 s32MaxConnNum, const HI_LIVESVR_MEDIA_OPERATE_S *pstMediaOps)
{
    if(s_bLIVESVRInitFlg)
    {
        MLOGE("LiveSvr had been inited.\n");
        return HI_LIVESVR_EINITIALIZED;
    }

    if(0 >= s32MaxConnNum || 32 < s32MaxConnNum)
    {
        MLOGE("Invalid s32MaxConnNum[%d].\n", s32MaxConnNum);
        return HI_LIVESVR_EINVAL;
    }

    HI_APPCOMM_CHECK_POINTER(pstMediaOps, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnGetVideoInfo, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnGetAudioInfo, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnVencStart, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnVencStop, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnAencStart, HI_LIVESVR_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaOps->pfnAencStop, HI_LIVESVR_EINVAL);

    s_stLIVESVRMediaOps.pfnGetVideoInfo = pstMediaOps->pfnGetVideoInfo;
    s_stLIVESVRMediaOps.pfnGetAudioInfo = pstMediaOps->pfnGetAudioInfo;
    s_stLIVESVRMediaOps.pfnVencStart = pstMediaOps->pfnVencStart;
    s_stLIVESVRMediaOps.pfnVencStop = pstMediaOps->pfnVencStop;
    s_stLIVESVRMediaOps.pfnAencStart = pstMediaOps->pfnAencStart;
    s_stLIVESVRMediaOps.pfnAencStop = pstMediaOps->pfnAencStop;

    HI_S32 s32Ret;

    s32Ret = LIVESVR_RTSP_ServerInit(s32MaxConnNum);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    memset(&s_astRtspStreamCtx[0], 0x0, sizeof(LIVESVR_RTSP_STREAM_CONTEXT_S) * LIVESVR_RTSP_STREAM_MAX_CNT);

    s_bLIVESVRInitFlg = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_LIVESVR_Deinit(HI_VOID)
{
    LIVESVR_CHECK_INIT();

    (HI_VOID)HI_LIVESVR_RemoveAllStream();
    (HI_VOID)LIVESVR_RTSP_ServerDeinit();

    s_bLIVESVRInitFlg = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 HI_LIVESVR_AddStream(HI_HANDLE VencHdl, HI_HANDLE AencHdl, const HI_CHAR *pszStreamName)
{
    LIVESVR_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pszStreamName, HI_LIVESVR_EINVAL);

    if((HI_INVALID_HANDLE == VencHdl) && (HI_INVALID_HANDLE == AencHdl))
    {
        return HI_LIVESVR_EINVAL;
    }

    if(0 == strnlen(pszStreamName, HI_APPCOMM_MAX_PATH_LEN))
    {
        return HI_LIVESVR_EINVAL;
    }

    HI_S32 s32Ret;
    HI_U32 u32Idx;
    LIVESVR_RTSP_STREAM_CONTEXT_S *pstRtspStreamCtx = HI_NULL;

    for(u32Idx = 0; u32Idx < LIVESVR_RTSP_STREAM_MAX_CNT; ++u32Idx)
    {
        if(s_astRtspStreamCtx[u32Idx].bUsed)
        {
            if(0 == strncmp(s_astRtspStreamCtx[u32Idx].szStreamName, pszStreamName, HI_APPCOMM_MAX_PATH_LEN))
            {
                return HI_LIVESVR_EEXIST;
            }
        }
    }

    for(u32Idx = 0; u32Idx < LIVESVR_RTSP_STREAM_MAX_CNT; ++u32Idx)
    {
        if(!s_astRtspStreamCtx[u32Idx].bUsed)
        {
            pstRtspStreamCtx = &s_astRtspStreamCtx[u32Idx];
            memset(&pstRtspStreamCtx->stVideoTrackSource, 0x0, sizeof(HI_Track_Source_S));
            memset(&pstRtspStreamCtx->stAudioTrackSource, 0x0, sizeof(HI_Track_Source_S));
            break;
        }
    }
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO((HI_NULL != pstRtspStreamCtx), HI_LIVESVR_EMAXSOURCE, "beyond max source count");

    HI_U32 u32MbufferSize = 0;

    if(HI_INVALID_HANDLE != VencHdl)
    {
        s32Ret = LIVESVR_GetVideoInfo(VencHdl, &pstRtspStreamCtx->stVideoTrackSource.unTrackSourceAttr.stVideoInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        pstRtspStreamCtx->stVideoTrackSource.pfnSourceStart = LIVESVR_StartVideoSource;
        pstRtspStreamCtx->stVideoTrackSource.pfnSourceStop = LIVESVR_StopVideoSource;
        pstRtspStreamCtx->stVideoTrackSource.pfnRequestKeyFrame = LIVESVR_RequestKeyFrame;
        pstRtspStreamCtx->stVideoTrackSource.s32PrivateHandle = VencHdl;
        pstRtspStreamCtx->stVideoTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;

        u32MbufferSize += pstRtspStreamCtx->stVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate
            / pstRtspStreamCtx->stVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate * LIVESVR_VIDEO_MBUFFER_FRAME; /** frame size * 5 */
    }

    if(HI_INVALID_HANDLE != AencHdl)
    {
        s32Ret = LIVESVR_GetAudioInfo(AencHdl, &pstRtspStreamCtx->stAudioTrackSource.unTrackSourceAttr.stAudioInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        pstRtspStreamCtx->stAudioTrackSource.pfnSourceStart = LIVESVR_StartAudioSource;
        pstRtspStreamCtx->stAudioTrackSource.pfnSourceStop = LIVESVR_StopAudioSource;
        pstRtspStreamCtx->stAudioTrackSource.s32PrivateHandle = AencHdl;
        pstRtspStreamCtx->stAudioTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;

        u32MbufferSize += LIVESVR_AUDIO_MBUFFER_SIZE;
    }

    HI_RTSP_SOURCE_S stStreamSrc;
    if(HI_INVALID_HANDLE != VencHdl)
    {
        stStreamSrc.pstVideoSrc = &pstRtspStreamCtx->stVideoTrackSource;
    }
    else
    {
        stStreamSrc.pstVideoSrc = HI_NULL;
    }

    if(HI_INVALID_HANDLE != AencHdl)
    {
        stStreamSrc.pstAudioSrc = &pstRtspStreamCtx->stAudioTrackSource;
    }
    else
    {
        stStreamSrc.pstAudioSrc = HI_NULL;
    }

    snprintf(pstRtspStreamCtx->szStreamName, HI_APPCOMM_MAX_PATH_LEN, "%s", pszStreamName);

    HI_CHAR szStreamNameWithPattern[HI_APPCOMM_MAX_PATH_LEN];
    snprintf(szStreamNameWithPattern, HI_APPCOMM_MAX_PATH_LEN, "%s/%s", LIVESTREAM_NAME_PATTERN, pszStreamName);

    u32MbufferSize = MAX(u32MbufferSize, RTSP_MIN_MBUFFER_SIZE);
    u32MbufferSize = MIN(u32MbufferSize, RTSP_MAX_MBUFFER_SIZE);

    MLOGD(GREEN"Stream name[%s] Venc[%d] Aenc[%d] MbufferSize[%ubyte]\n"NONE, pstRtspStreamCtx->szStreamName,
        HI_INVALID_HANDLE != VencHdl ? pstRtspStreamCtx->stVideoTrackSource.s32PrivateHandle : VencHdl,
        HI_INVALID_HANDLE != AencHdl ? pstRtspStreamCtx->stAudioTrackSource.s32PrivateHandle : AencHdl, u32MbufferSize);

    s32Ret = HI_RTSPSVR_AddMediaStream(s_pLIVESVRRtspSvrObj, &stStreamSrc, szStreamNameWithPattern, u32MbufferSize);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_astRtspStreamCtx[u32Idx].bUsed = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_LIVESVR_RemoveStream(const HI_CHAR *pszStreamName)
{
    LIVESVR_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(pszStreamName, HI_LIVESVR_EINVAL);

    if(0 == strnlen(pszStreamName, HI_APPCOMM_MAX_PATH_LEN))
    {
        return HI_LIVESVR_EINVAL;
    }

    HI_S32 s32Ret;
    HI_U32 u32Idx;

    for(u32Idx = 0; u32Idx < LIVESVR_RTSP_STREAM_MAX_CNT; ++u32Idx)
    {
        if((HI_TRUE == s_astRtspStreamCtx[u32Idx].bUsed)
            && (0 == strncmp(s_astRtspStreamCtx[u32Idx].szStreamName, pszStreamName, HI_APPCOMM_MAX_PATH_LEN)))
        {
            HI_CHAR szStreamNameWithPattern[HI_APPCOMM_MAX_PATH_LEN];
            snprintf(szStreamNameWithPattern, HI_APPCOMM_MAX_PATH_LEN, "%s/%s", LIVESTREAM_NAME_PATTERN, pszStreamName);

            s32Ret = HI_RTSPSVR_RemoveMediaStream(s_pLIVESVRRtspSvrObj, szStreamNameWithPattern);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s_astRtspStreamCtx[u32Idx].bUsed = HI_FALSE;

            return HI_SUCCESS;
        }
    }

    return HI_LIVESVR_ELOST;
}

HI_S32 HI_LIVESVR_RemoveAllStream(HI_VOID)
{
    LIVESVR_CHECK_INIT();

    HI_S32 s32Ret;
    HI_U32 u32Idx;

    for(u32Idx = 0; u32Idx < LIVESVR_RTSP_STREAM_MAX_CNT; ++u32Idx)
    {
        if(HI_TRUE == s_astRtspStreamCtx[u32Idx].bUsed)
        {
            HI_CHAR szStreamNameWithPattern[HI_APPCOMM_MAX_PATH_LEN];
            snprintf(szStreamNameWithPattern, HI_APPCOMM_MAX_PATH_LEN, "%s/%s", LIVESTREAM_NAME_PATTERN, s_astRtspStreamCtx[u32Idx].szStreamName);

            s32Ret = HI_RTSPSVR_RemoveMediaStream(s_pLIVESVRRtspSvrObj, szStreamNameWithPattern);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("HI_RTSPSVR_RemoveMediaStream failed[0x%08x]\n", s32Ret);
            }
            s_astRtspStreamCtx[u32Idx].bUsed = HI_FALSE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_LIVESVR_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret;

    s32Ret  = HI_EVTHUB_Register(HI_EVENT_LIVESVR_CLIENT_CONNECT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_LIVESVR_CLIENT_DISCONNECT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_LIVESVR_SERVER_ERROR);

    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

