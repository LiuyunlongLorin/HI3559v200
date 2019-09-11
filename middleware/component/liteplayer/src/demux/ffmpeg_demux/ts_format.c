/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ffmpeg_format.c
* @brief     hidemuxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.11.03  */

#include "securec.h"
#include <sys/time.h>
#include <pthread.h>
#ifndef __HuaweiLite__
#include <sys/syscall.h>
#endif
#include <string.h>
#include "hi_demuxer.h"
#include "hi_demuxer_err.h"
#include "ffdemux_log.h"
#include "ts_format.h"
#include "ffdemux_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define TS_DEMUX  "tsDemux"
#define AAC_ADTS_LEN  (7)
#define AAC_SAMPLE_RATE_NUM  (13)
#define TS_DEFAULT_FPS (30.0)
/*define NULL handle for codedex check*/
#define HI_NULL_HANDLE (0)
#define NAL_PREFIX_3BYTE (3)
#define NAL_PREFIX_4BYTE (4)

typedef enum hEVC_NAL_TYPE_E
{
    HEVC_PSLICE = 2,
    HEVC_IDR = 19,
    HEVC_VPS = 32,
    HEVC_SPS = 33,
    HEVC_PPS = 34,
    HEVC_SEI = 39
} HEVC_NAL_TYPE_E;

typedef enum aVC_NAL_TYPE_E
{
    AVC_SEI = 6,
    AVC_IDR = 5,
    AVC_SPS = 7,
    AVC_PPS = 8,
} AVC_NAL_TYPE_E;




static HI_U32 s_au32SampRateTab[AAC_SAMPLE_RATE_NUM] =
{
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};

static const HI_U8 s_au8AudioChannels[8] =
{
    0, 1, 2, 3, 4, 5, 6, 8
};


static const HI_U8 s_au8NalPrefix_4Byte[NAL_PREFIX_4BYTE] = {0x00, 0x00, 0x00, 0x01};
static const HI_U8 s_au8NalPrefix_3Byte[NAL_PREFIX_3BYTE] = {0x00, 0x00, 0x01};


static HI_U32 TS_GetOneNalUnit(HI_U8* pBuffer , HI_U32 size, HI_U8* pu8HeaderLen)
{
    HI_U32 pos = 0;

    if (size < NAL_PREFIX_4BYTE)
    {
        return 0;
    }

    if (!memcmp(&pBuffer[pos], s_au8NalPrefix_4Byte, NAL_PREFIX_4BYTE))
    {
        *pu8HeaderLen = NAL_PREFIX_4BYTE;
        pos += NAL_PREFIX_4BYTE;
    }
    else if (!memcmp(&pBuffer[pos], s_au8NalPrefix_3Byte, NAL_PREFIX_3BYTE))
    {
        *pu8HeaderLen = NAL_PREFIX_3BYTE;
        pos += NAL_PREFIX_3BYTE;
    }
    else
    {
        return 0;
    }

    //find second nal header
    for (; pos <= size - NAL_PREFIX_4BYTE; pos++)
    {
        if (!memcmp(&pBuffer[pos], s_au8NalPrefix_4Byte, NAL_PREFIX_4BYTE)
            || !memcmp(&pBuffer[pos], s_au8NalPrefix_3Byte, NAL_PREFIX_3BYTE))
        {
            break;
        }
    }

    if (pos == size )              // next start code is not found, this must be the last nalu
    {
        return size;
    }
    else
    {
        return pos;
    }
}


static HI_S32 TS_Parse_VideoSPS(enum AVCodecID enVidCodecId, HI_U8* pData, HI_U32 u32DataLen, HI_U32* pu32Width, HI_U32* pu32Height)
{
    HI_U8* pTmpPtr = pData;
    HI_U32 u32LeftLen = u32DataLen;
    HI_U8 u8HeadLen = 0;
    HI_BOOL bParsedSPS = HI_FALSE;

    if (AV_CODEC_ID_H264 != enVidCodecId
        && AV_CODEC_ID_HEVC != enVidCodecId)
    {
        return HI_FAILURE;
    }

    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = TS_GetOneNalUnit(pTmpPtr, u32LeftLen, &u8HeadLen);
        if (u32NalLen == 0 || (u32NalLen <= u8HeadLen))
        {
            break;
        }

        if (AV_CODEC_ID_H264 == enVidCodecId)
        {
            u8NalType = pTmpPtr[u8HeadLen] & 0x1F;
            if (AVC_SPS == u8NalType)
            {
                HI_U8* pu8SPS = (HI_U8*)malloc(u32NalLen - u8HeadLen);
                if (NULL == pu8SPS)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "malloc avc stream sps failed\n");
                    return HI_FAILURE;
                }
                genSODPByRBSP(&pTmpPtr[u8HeadLen], u32NalLen - u8HeadLen, pu8SPS);
                AVC_SPS_INFO stSpsInfo = {0};

                FFAVCParser_parseSps(&stSpsInfo, pu8SPS, u32NalLen - u8HeadLen);
                *pu32Width = ((stSpsInfo.u32PicWidthInMbsMinus1 + 1) * 16) - stSpsInfo.u32FrameCropLeftOffset * 2 - stSpsInfo.u32FrameCropRightOffset * 2;
                *pu32Height = ((2 - (stSpsInfo.bFrameMbsOnlyFlag ? 1 : 0)) * (stSpsInfo.u32PicHeightInMapUnitsMinus1 + 1) * 16) -
                              (stSpsInfo.u32FrameCropTopOffset * 2) - (stSpsInfo.u32FrameCropBottomOffset * 2);
                if (pu8SPS)
                {
                    free(pu8SPS);
                    pu8SPS = NULL;
                }
                bParsedSPS = HI_TRUE;
                break;
            }
        }
        else if (AV_CODEC_ID_HEVC == enVidCodecId)
        {
            u8NalType = (pTmpPtr[u8HeadLen] & 0x7E) >> 1;
            if (HEVC_SPS == u8NalType)
            {
                HI_U8* pu8SPS = (HI_U8*)malloc(u32NalLen - u8HeadLen);
                if (NULL == pu8SPS)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "malloc hevc stream sps failed\n");
                    return HI_FALSE;
                }

                HEVC_SPS_INFO stSpsInfo = {0};
                genSODPByRBSP(&pTmpPtr[u8HeadLen], u32NalLen - u8HeadLen, pu8SPS);
                FFHEVCParser_parseSps(&stSpsInfo, pu8SPS, u32NalLen - u8HeadLen);
                *pu32Width = stSpsInfo.u32PicWidthInLumaSamples;
                *pu32Height = stSpsInfo.u32PicHeightInLumaSamples;
                if (pu8SPS)
                {
                    free(pu8SPS);
                    pu8SPS = NULL;
                }
                bParsedSPS = HI_TRUE;
                break;
            }
        }

        pTmpPtr += u32NalLen;
        u32LeftLen -= u32NalLen;
    }
    while (pTmpPtr < (pData + u32DataLen));

    if (!bParsedSPS)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "there no sps info in frame error\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 TS_ParseAudioFrame( HI_U8* pData, HI_U32 u32DataLen, AAC_MEDIA_INFO_S* pstAACInfo)
{
    HI_U32 u32Len = 0;
    AAC_ADTS_HEADER_S adtsHeader = {0};

    if (FFAACParser_parseAdtsHeader(&adtsHeader, pData, u32DataLen, &u32Len) != HI_SUCCESS)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "AAC parse audio frame error\n");
        return HI_FAILURE;
    }

    if ( 0 == u32Len)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "AAC parse audio frame error\n");
        return HI_FAILURE;
    }

    pstAACInfo->u32Sync = adtsHeader.u32Sync;
    pstAACInfo->u32ChannelNum = s_au8AudioChannels[adtsHeader.u8ChannelConfig];
    pstAACInfo->u32SampleRate = s_au32SampRateTab[adtsHeader.u8SampRateIdx];
    pstAACInfo->u32FrameLength = adtsHeader.u32FrameLength;
    return HI_SUCCESS;
}

static HI_BOOL TS_IsMulti_ADTSAudioFrame( HI_U8* pData, HI_U32 u32DataLen)
{
    HI_U8* pTmpPtr = pData;
    AAC_MEDIA_INFO_S stAACInfo;
    HI_U32 u32LeftLen = u32DataLen;


    if (HI_SUCCESS !=  memset_s(&stAACInfo, sizeof(stAACInfo), 0x00, sizeof(stAACInfo)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set stAACInfo error\n");
    }

    (HI_VOID)TS_ParseAudioFrame( pTmpPtr, u32LeftLen, &stAACInfo);
    if (stAACInfo.u32Sync != (HI_U32)0xfff)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "audio frame have no sync header error\n");
        return HI_TRUE;
    }

    pTmpPtr += stAACInfo.u32FrameLength;
    u32LeftLen -= stAACInfo.u32FrameLength;

    do
    {
        if (!(pTmpPtr[0] == 0xFF && ((pTmpPtr[1] & 0xF0) >> 4 == 0x0F)))
        {
            pTmpPtr++;
            u32LeftLen--;
        }
        else
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "do not support another aac frame in one packet \n");
            return HI_TRUE;
        }
    }
    while (pTmpPtr < pData + u32DataLen);

    return HI_FALSE;
}


static HI_FORMAT_VIDEO_TYPE_E TS_GetVideoType(enum AVCodecID enVidCodecId)
{
    if (AV_CODEC_ID_H264 == enVidCodecId)
    {
        return HI_FORMAT_VIDEO_TYPE_H264;
    }
    else if (AV_CODEC_ID_HEVC == enVidCodecId)
    {
        return HI_FORMAT_VIDEO_TYPE_H265;
    }
    else
    {
        return HI_FORMAT_VIDEO_TYPE_BUTT;
    }
}

static HI_BOOL TS_IS_IDRFrame(enum AVCodecID enVidCodecId, HI_U8* pData, HI_U32 u32DataLen)
{
    HI_U8* pTmpPtr = pData;
    HI_U32 u32LeftLen = u32DataLen;
    HI_U8 u8HeadLen = 0;
    HI_BOOL bIsIDR = HI_FALSE;

    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = TS_GetOneNalUnit(pTmpPtr, u32LeftLen, &u8HeadLen);
        if (u32NalLen == 0)
        {
            break;
        }

        if (AV_CODEC_ID_H264 == enVidCodecId)
        {
            u8NalType = pTmpPtr[u8HeadLen] & 0x1F;
            if (AVC_SPS == u8NalType || AVC_IDR == u8NalType)
            {
                bIsIDR = HI_TRUE;
                break;
            }
        }
        else if (AV_CODEC_ID_HEVC == enVidCodecId)
        {
            u8NalType = (pTmpPtr[u8HeadLen] & 0x7E) >> 1;
            if (HEVC_SPS == u8NalType ||  HEVC_PPS == u8NalType
                ||  HEVC_IDR == u8NalType)
            {
                bIsIDR = HI_TRUE;
                break;
            }
        }

        pTmpPtr += u32NalLen;
        u32LeftLen -= u32NalLen;
    }
    while (pTmpPtr < (pData + u32DataLen));

    return bIsIDR;
}



HI_S32 TS_Seek_MS(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_S64 s64MSec, __attribute__((unused))HI_S32 s32Flag)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64TimeUs = ((HI_S64)s64MSec * 1000);

    /*s64TimeUs may oversize s64 to be negative value*/
    if ( s64MSec < 0 || s64TimeUs < 0 )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "seek input time is negative or oversize: %lld ms\n", s64MSec);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (s64TimeUs > pstTsDemuxer->pstFormatContext->duration)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "seek input time beyond total time seektime: %lld\n",
                       s64TimeUs);
        s64TimeUs = pstTsDemuxer->pstFormatContext->duration;
    }

    HI_S32 s32SeekTotalSec, s32SeekHour, s32SeekMin, s32SeekSec;
    HI_S32 s32FileTotalSec, s32FileHour, s32FileMin, s32FileSec;

    /* translate microsecond to hour:min:sec format */
    s32FileTotalSec  = pstTsDemuxer->pstFormatContext->duration / 1000000LL;
    s32FileHour  = s32FileTotalSec / 3600;
    s32FileMin  = (s32FileTotalSec % 3600) / 60;
    s32FileSec  = (s32FileTotalSec % 60);

    s32SeekTotalSec = s64TimeUs / 1000000LL;
    s32SeekHour = s32SeekTotalSec / 3600;
    s32SeekMin = (s32SeekTotalSec % 3600) / 60;
    s32SeekSec = (s32SeekTotalSec % 60);

    if (pstTsDemuxer->pstFormatContext->start_time != AV_NOPTS_VALUE)
    {
        s64TimeUs += pstTsDemuxer->pstFormatContext->start_time;
    }

    //we use one second to ensure thar the avformat_seek_file interface can find the key frame beyond s64SeekMin and s64SeekMax
    HI_S64 s64SeekTarget = s64TimeUs;
    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "Seek to %lld (%2d:%02d:%02d) of total  (%2d:%02d:%02d)   \n",
                   s64TimeUs, s32SeekHour, s32SeekMin, s32SeekSec, s32FileHour, s32FileMin, s32FileSec);

    s32Ret = avformat_seek_file(pstTsDemuxer->pstFormatContext, -1,  INT64_MIN, s64SeekTarget, s64SeekTarget, 0);
    if (s32Ret < 0)
    {
        s32Ret = avformat_seek_file(pstTsDemuxer->pstFormatContext, -1,  s64SeekTarget, s64SeekTarget, INT64_MAX, 0);
        if (s32Ret < 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "%s: error while seeking %lld ,streamIndex:%d, s32Ret:%d\n",
                           pstTsDemuxer->pstFormatContext->filename, s64MSec, 0, s32Ret);
            return HI_ERR_DEMUXER_SEEK;
        }
    }

    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstTsDemuxer->iLastReadPts = s64SeekTarget;
    return HI_SUCCESS;
}

HI_S32 TS_Read_UntilIframe(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AVPacket stPkt;
    HI_BOOL bGetIDR =  HI_FALSE;

    if (HI_SUCCESS != memset_s(&stPkt, sizeof(stPkt), 0x00, sizeof(stPkt)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set stPkt error\n");
    }
    av_init_packet(&stPkt);

    while (!bGetIDR)
    {
        s32Ret = av_read_frame(pstTsDemuxer->pstFormatContext, &stPkt);
        if (s32Ret == AVERROR_EOF)
        {
            s32Ret = HI_SUCCESS;
            break;
        }
        else if (s32Ret != HI_SUCCESS || stPkt.size <= 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "av_read_frame ret failed or ret size 0\n");
            s32Ret = HI_FAILURE;
            break;
        }

        if (!bGetIDR && (stPkt.stream_index == pstTsDemuxer->s32UsedVideoStreamIndex))
        {
            if (TS_IS_IDRFrame(pstTsDemuxer->enVidCodecId, stPkt.data, stPkt.size))
            {
                bGetIDR = HI_TRUE;
                HI_S64 s64pos = stPkt.pos;
                av_packet_unref(&stPkt);
                if (avio_seek(pstTsDemuxer->pstFormatContext->pb, s64pos, SEEK_SET) < 0)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "seek to last Idr frame error\n");
                }
                avformat_flush(pstTsDemuxer->pstFormatContext);
                break;
            }
        }
        av_packet_unref(&stPkt);
    }

    return s32Ret;
}

HI_S32 TS_Probe_ByParseFrame(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer)
{
    HI_BOOL bGetVideo = HI_FALSE;
    HI_BOOL bGetAudio = HI_FALSE;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    enum AVCodecID enVidCodecId = AV_CODEC_ID_NONE;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32ParsedVidNum = 0;
    AVPacket stPkt;

    if (HI_SUCCESS != memset_s(&stPkt, sizeof(stPkt), 0x00, sizeof(stPkt)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set  audio&video stPkt failed\n");
    }

    av_init_packet(&stPkt);

    while ((pstTsDemuxer->s32UsedVideoStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM && !bGetVideo)
           || (pstTsDemuxer->s32AudioStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM && !bGetAudio)
           || (s32ParsedVidNum < pstTsDemuxer->s32VideoStreamNum))
    {
        s32Ret = av_read_frame(pstTsDemuxer->pstFormatContext, &stPkt);
        if (s32Ret != HI_SUCCESS || stPkt.size <= 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "parse audio&video frame failed\n");
            break;
        }

        if (!bGetAudio && (stPkt.stream_index == pstTsDemuxer->s32AudioStreamIndex))
        {
            AAC_MEDIA_INFO_S stAACInfo;
            bGetAudio = HI_TRUE;
            if (HI_SUCCESS != memset_s(&stAACInfo, sizeof(AAC_MEDIA_INFO_S), 0x00, sizeof(AAC_MEDIA_INFO_S)))
            {
                FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set  audio&video stAACInfo failed\n");
            }
            (HI_VOID)TS_ParseAudioFrame( stPkt.data, stPkt.size, &stAACInfo);
            pstTsDemuxer->u32SampleRate = stAACInfo.u32SampleRate;
            pstTsDemuxer->u32Chnnum = stAACInfo.u32ChannelNum;
        }
        else if ((stPkt.stream_index ==  pstTsDemuxer->s32UsedVideoStreamIndex)
                 && HI_DEMUXER_NO_MEDIA_STREAM == pstTsDemuxer->stSteamResolution[stPkt.stream_index].s32VideoStreamIndex)
        {
            bGetVideo = HI_TRUE;
            s32Ret = TS_Parse_VideoSPS(pstTsDemuxer->enVidCodecId, stPkt.data, stPkt.size,
                                       &pstTsDemuxer->u32Width, &pstTsDemuxer->u32Height);
            if (s32Ret != HI_SUCCESS)
            {
                FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "TS_Parse_VideoSPS failed\n");
            }

            pstTsDemuxer->stSteamResolution[stPkt.stream_index].s32VideoStreamIndex = stPkt.stream_index;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].u32Height = pstTsDemuxer->u32Height;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].u32Width = pstTsDemuxer->u32Width;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].enVideoType = TS_GetVideoType(pstTsDemuxer->enVidCodecId);
            s32ParsedVidNum++;
        }
        else if ((stPkt.stream_index !=  pstTsDemuxer->s32UsedVideoStreamIndex)
                 && HI_DEMUXER_NO_MEDIA_STREAM == pstTsDemuxer->stSteamResolution[stPkt.stream_index].s32VideoStreamIndex
                 && (pstTsDemuxer->pstFormatContext->streams[stPkt.stream_index]->codecpar->codec_id == AV_CODEC_ID_H264
                     || pstTsDemuxer->pstFormatContext->streams[stPkt.stream_index]->codecpar->codec_id == AV_CODEC_ID_HEVC))
        {
            enVidCodecId = pstTsDemuxer->pstFormatContext->streams[stPkt.stream_index]->codecpar->codec_id;

            s32Ret = TS_Parse_VideoSPS(enVidCodecId, stPkt.data, stPkt.size, &u32Width, &u32Height);
            if (s32Ret != HI_SUCCESS)
            {
                FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "TS_Parse_VideoSPS failed\n");
            }
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].s32VideoStreamIndex = stPkt.stream_index;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].u32Height = u32Height;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].u32Width = u32Width;
            pstTsDemuxer->stSteamResolution[stPkt.stream_index].enVideoType = TS_GetVideoType(pstTsDemuxer->enVidCodecId);
            s32ParsedVidNum++;

        }

        av_packet_unref(&stPkt);
    }

    if (avio_seek(pstTsDemuxer->pstFormatContext->pb, 0, SEEK_SET) < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "ts seek to file begin error\n");
        s32Ret = HI_FAILURE;
    }
    avformat_flush(pstTsDemuxer->pstFormatContext);

    return s32Ret;
}


HI_S32 TS_SET_AudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    AAC_MEDIA_INFO_S stAACInfo = {0};
    HI_S64 s64ReadPts = 0;


    if (TS_IsMulti_ADTSAudioFrame( pstTsDemuxer->stPkt.data, pstTsDemuxer->stPkt.size))
    {
        av_packet_unref(&pstTsDemuxer->stPkt);
        return HI_ERR_DEMUXER_READ_PACKET;
    }
    pstFmtFrame->pu8Data = pstTsDemuxer->stPkt.data;
    pstFmtFrame->u32Size = pstTsDemuxer->stPkt.size;
    s64ReadPts = av_rescale_q(pstTsDemuxer->stPkt.pts, pstTsDemuxer->pstAudioStream->time_base, AV_TIME_BASE_Q);
    if (s64ReadPts < pstTsDemuxer->pstFormatContext->start_time)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "audio pts is small than start_time\n");
    }
    else
    {
        pstTsDemuxer->iLastReadPts = s64ReadPts - pstTsDemuxer->pstFormatContext->start_time;
    }

    (HI_VOID)TS_ParseAudioFrame( pstFmtFrame->pu8Data, pstFmtFrame->u32Size, &stAACInfo);
    if (stAACInfo.u32FrameLength != pstFmtFrame->u32Size)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "aac frame len %d not eq to pkt len:  %d\n", stAACInfo.u32FrameLength, pstFmtFrame->u32Size);
    }
    pstFmtFrame->u32Size = stAACInfo.u32FrameLength;
    return HI_SUCCESS;
}

HI_S32 TS_SET_VideoInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    HI_S64 s64ReadPts = 0;

    s64ReadPts = av_rescale_q(pstTsDemuxer->stPkt.pts, pstTsDemuxer->pstVideoStream->time_base, AV_TIME_BASE_Q);
    if (s64ReadPts < pstTsDemuxer->pstFormatContext->start_time)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "video pts is small than start_time s64ReadPts: %lld \n", s64ReadPts);
    }
    else
    {
        pstTsDemuxer->iLastReadPts = s64ReadPts - pstTsDemuxer->pstFormatContext->start_time;
    }
    pstFmtFrame->pu8Data = pstTsDemuxer->stPkt.data;
    pstFmtFrame->u32Size = pstTsDemuxer->stPkt.size;
    return HI_SUCCESS;
}

HI_S32 TS_ProbeAudio(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer)
{
    /*audio stream could not been parsed, so we just use it*/
    HI_U32 i = 0;

    for (i = 0; i < pstTsDemuxer->pstFormatContext->nb_streams; i++)
    {
        if (pstTsDemuxer->pstFormatContext->streams[i]->codecpar->codec_id == AV_CODEC_ID_AAC)
        {

            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,
                "audio codec type  %s\n",
                avcodec_get_name(pstTsDemuxer->pstFormatContext->streams[i]->codecpar->codec_id));
            break;
        }
    }

    if (i == pstTsDemuxer->pstFormatContext->nb_streams)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Could not find AAC audio stream in input file '%s'\n", pstTsDemuxer->aszFilePath);
        pstTsDemuxer->s32AudioStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstTsDemuxer->pstAudioStream = NULL;
    }
    else
    {
        pstTsDemuxer->s32AudioStreamIndex = i;
        pstTsDemuxer->pstAudioStream = pstTsDemuxer->pstFormatContext->streams[i];
    }



    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
