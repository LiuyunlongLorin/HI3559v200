/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ffmpeg_format.c
* @brief     hidemuxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.11.03  */


#include <sys/time.h>
#include <pthread.h>
#ifndef __HuaweiLite__
#include <sys/syscall.h>
#endif
#include <string.h>
#include "securec.h"
#include "hi_demuxer.h"
#include "hi_demuxer_err.h"
#include "ffdemux_log.h"
#include "ts_format.h"
#include "mp4_format.h"
#include "common_format.h"
#include "ffdemux_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define TS_DEFAULT_FPS (30.0)
/*define NULL handle for codedex check*/
#define FFMPEG_TIME_UNIT      (1000)
#define FFMPEG_DEFAULT_FPS (30.0)

static HI_S32 COMMON_SET_AudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
    pstFmtFrame->u32Size = pstFormatMember->stPkt.size;
    pstFormatMember->bHasAdts = HI_FALSE;
    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts, pstFormatMember->pstAudioStream->time_base, AV_TIME_BASE_Q);
    return HI_SUCCESS;
}

static HI_S32 COMMON_GET_Frame(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* read frames from the file */
    s32Ret = av_read_frame(pstDemuxer->pstFormatContext, &pstDemuxer->stPkt);

    if (AVERROR_EOF == s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "read endof s32Ret: %d\n", s32Ret);
        return HI_RET_FILE_EOF;
    }
    else if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "av_read_frame failed, ret:%d\n", s32Ret);
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    if (pstDemuxer->stPkt.size < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Demux err: stream len:%d\n", pstDemuxer->stPkt.size);
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    return HI_SUCCESS;
}
static HI_S32 COMMON_GET_FileSize(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_S64*  ps64FileSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64FileSize = 0;
    FILE* pMediaFd = fopen(pstFormatMember->aszFilePath, "rb");

    if (NULL == pMediaFd)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, " pstFormatMember->aszFilePath:%s , err info:%s\n", pstFormatMember->aszFilePath, strerror(errno));
        return HI_FAILURE;
    }

    s32Ret = fseeko (pMediaFd, 0, SEEK_END);

    /*if error comes, fseek return  -1*/
    if (s32Ret == -1)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, " fseeko exec failed,  err info:%s\n", strerror(errno));
        fclose(pMediaFd);
        return HI_FAILURE;
    }

    s64FileSize = (HI_S64)ftello(pMediaFd);

    /*if error comes, ftell return  -1*/
    if (s64FileSize == -1)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, " ftello exec failed,  err info:%s\n", strerror(errno));
        fclose(pMediaFd);
        return HI_FAILURE;
    }

    *ps64FileSize = s64FileSize;
    fclose(pMediaFd);
    return HI_SUCCESS;

}
static HI_S32 COMMON_ProbeVideo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_BOOL* pbBsInited)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bBsfInited = HI_FALSE;
    if (HI_FFDEMUX_FORMAT_TS != pstDemuxer->enFormat && HI_FFDEMUX_FORMAT_MP4 != pstDemuxer->enFormat )
    {
        pstDemuxer->s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstDemuxer->pstVideoStream = NULL;
        return HI_SUCCESS;
    }
    HI_U32 u32StreamNum = pstDemuxer->pstFormatContext->nb_streams;
    HI_U32 i = 0;
    s32Ret = av_find_best_stream(pstDemuxer->pstFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "Could not find video stream in input file '%s', ret:%d \n", pstDemuxer->aszFilePath, s32Ret);
        pstDemuxer->s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstDemuxer->pstVideoStream = NULL;
    }
    else
    {
        pstDemuxer->pstVideoStream = pstDemuxer->pstFormatContext->streams[s32Ret];
        pstDemuxer->s32VideoStreamIndex = s32Ret;
        pstDemuxer->s32UsedVideoStreamIndex = pstDemuxer->s32VideoStreamIndex;
        pstDemuxer->pstUsedVideoStream = pstDemuxer->pstVideoStream;
        pstDemuxer->enVidCodecId = pstDemuxer->pstVideoStream->codecpar->codec_id;
        pstDemuxer->enUsedVidCodecId = pstDemuxer->enVidCodecId;
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "video codec type  %s\n", avcodec_get_name(pstDemuxer->enVidCodecId));
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "video timebase: %d %d \n", pstDemuxer->pstVideoStream->time_base.den,
                       pstDemuxer->pstVideoStream->time_base.num);

        if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
        {
            if (AV_CODEC_ID_H264 == pstDemuxer->enVidCodecId
                || AV_CODEC_ID_HEVC == pstDemuxer->enVidCodecId)
            {
                const HI_CHAR* pszFilterName = (AV_CODEC_ID_H264 == pstDemuxer->enVidCodecId) ?
                                               "h264_mp4toannexb" : "hevc_mp4toannexb";
                s32Ret = MP4_AVBSF_Init(&pstDemuxer->pstH26XBsfCtx, pstDemuxer->pstVideoStream, pszFilterName);
                if (HI_SUCCESS != s32Ret)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_AVBSF_Init h264_mp4toannexb failed\n");
                    return HI_ERR_DEMUXER_PROBE;
                }
                bBsfInited = HI_TRUE;
            }
            u32StreamNum = (u32StreamNum > HI_DEMUXER_RESOLUTION_CNT) ? (HI_DEMUXER_RESOLUTION_CNT) : (u32StreamNum);
            for (i = 0; i < u32StreamNum; i++)
            {
                if (AVMEDIA_TYPE_VIDEO == pstDemuxer->pstFormatContext->streams[i]->codecpar->codec_type)
                {
                    pstDemuxer->stSteamResolution[i].s32VideoStreamIndex = i;
                    pstDemuxer->stSteamResolution[i].u32Height = pstDemuxer->pstFormatContext->streams[i]->codecpar->height;
                    pstDemuxer->stSteamResolution[i].u32Width = pstDemuxer->pstFormatContext->streams[i]->codecpar->width;
                    /*this demux lib will only deal with AV_CODEC_ID_H264/HI_FFMPEG_VIDEO_TYPE_H265 */
                    if (AV_CODEC_ID_H264 == pstDemuxer->pstFormatContext->streams[i]->codecpar->codec_id)
                    {
                        pstDemuxer->stSteamResolution[i].enVideoType = HI_FORMAT_VIDEO_TYPE_H264;
                    }
                    else if (AV_CODEC_ID_HEVC == pstDemuxer->pstFormatContext->streams[i]->codecpar->codec_id)
                    {
                        pstDemuxer->stSteamResolution[i].enVideoType = HI_FORMAT_VIDEO_TYPE_H265;
                    }
                    else
                    {
                        pstDemuxer->stSteamResolution[i].enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
                    }


                }
            }
        }


        if (HI_FFDEMUX_FORMAT_TS == pstDemuxer->enFormat)
        {
            HI_U32 i = 0;
            //get video stream num
            for (i = 0; i < pstDemuxer->pstFormatContext->nb_streams; i++)
            {
                if ((pstDemuxer->pstFormatContext->streams[i]->codecpar->codec_id == AV_CODEC_ID_H264)
                    || (pstDemuxer->pstFormatContext->streams[i]->codecpar->codec_id == AV_CODEC_ID_HEVC))
                {
                    pstDemuxer->s32VideoStreamNum++;
                }
            }
        }
    }

    *pbBsInited = bBsfInited;
    return HI_SUCCESS;

}

static HI_S32 COMMON_ProbeAudio(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_FFDEMUX_FORMAT_TS == pstFormatMember->enFormat)
    {
        s32Ret = TS_ProbeAudio(pstFormatMember);
        if (HI_SUCCESS != s32Ret)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "TS_ProbeAudio failed\n");
            return HI_ERR_DEMUXER_PROBE;
        }
    }
    else
    {
        s32Ret = av_find_best_stream(pstFormatMember->pstFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (s32Ret < 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG,
                "Could not find audio stream in input file '%s', ret:%d\n",
                pstFormatMember->aszFilePath, s32Ret);
            pstFormatMember->s32AudioStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
            pstFormatMember->pstAudioStream = NULL;
        }
        else
        {
            pstFormatMember->pstAudioStream = pstFormatMember->pstFormatContext->streams[s32Ret];
            pstFormatMember->s32AudioStreamIndex = s32Ret;
            pstFormatMember->enAudCodecId = pstFormatMember->pstAudioStream->codecpar->codec_id;
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,
                "audio codec type  %s\n",
                avcodec_get_name(pstFormatMember->enAudCodecId));
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,
                "audio timebase: %d %d \n",
                pstFormatMember->pstAudioStream->time_base.den,
                pstFormatMember->pstAudioStream->time_base.num);
            if (HI_FFDEMUX_FORMAT_WAV == pstFormatMember->enFormat)
            {
                if (pstFormatMember->enAudCodecId != AV_CODEC_ID_PCM_S16LE)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                        "WAV_Probe failed, not support wav codec %s\n",
                        avcodec_get_name(pstFormatMember->enAudCodecId));
                    return  HI_ERR_DEMUXER_PROBE;
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 COMMON_ProbeMeta(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_FFDEMUX_FORMAT_TS == pstFormatMember->enFormat)
    {


        s32Ret = TS_Probe_ByParseFrame(pstFormatMember);
        if (s32Ret != HI_SUCCESS)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Could not probe video&audio media info\n");
            return HI_ERR_DEMUXER_PROBE;
        }
    }


    if (HI_FFDEMUX_FORMAT_MP4 == pstFormatMember->enFormat)
    {
        s32Ret = MP4_ProbeMeta(pstFormatMember);
        if (HI_SUCCESS != s32Ret)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_ProbeMeta failed\n");
            return HI_ERR_DEMUXER_PROBE;
        }
    }

    return HI_SUCCESS;
}


static HI_S32 COMMON_Seekfile(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_S32 s32StreamIndex, HI_S64 s64MSec, HI_S32 s32Flag)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64VideoDuration  = 0;

    HI_S64 s64TimeUs = ((HI_S64)s64MSec * 1000);
    /*s64TimeUs may oversize s64 to be negative value*/
    if ( s64MSec < 0 || s64TimeUs < 0 )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "seek input time is negative or oversize: %lld ms\n", s64MSec);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (s64TimeUs > pstFormatMember->pstFormatContext->duration)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "seek input time beyond total time seektime: %lld\n",
                       s64TimeUs);
        s64TimeUs = pstFormatMember->pstFormatContext->duration;
    }

    if (s32StreamIndex == pstFormatMember->s32UsedVideoStreamIndex && pstFormatMember->pstUsedVideoStream)
    {
        s64VideoDuration = pstFormatMember->pstUsedVideoStream->duration * FFMPEG_TIME_UNIT * pstFormatMember->pstUsedVideoStream->time_base.num / pstFormatMember->pstUsedVideoStream->time_base.den;
        if ( s64MSec > s64VideoDuration )
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "seek input time beyond total video time seektime: %lld s64VideoDuration %lld \n", s64MSec, s64VideoDuration);
            s64TimeUs = (HI_S64)s64VideoDuration * 1000;
        }
    }

    HI_S32 s32SeekTotalSec, s32SeekHour, s32SeekMin, s32SeekSec;
    HI_S32 s32FileTotalSec, s32FileHour, s32FileMin, s32FileSec;

    /* translate microsecond to hour:min:sec format */
    s32FileTotalSec  = pstFormatMember->pstFormatContext->duration / 1000000LL;
    s32FileHour  = s32FileTotalSec / 3600;
    s32FileMin  = (s32FileTotalSec % 3600) / 60;
    s32FileSec  = (s32FileTotalSec % 60);

    s32SeekTotalSec = s64TimeUs / 1000000LL;
    s32SeekHour = s32SeekTotalSec / 3600;
    s32SeekMin = (s32SeekTotalSec % 3600) / 60;
    s32SeekSec = (s32SeekTotalSec % 60);

    if (pstFormatMember->pstFormatContext->start_time != AV_NOPTS_VALUE)
    {
        s64TimeUs += pstFormatMember->pstFormatContext->start_time;
    }


    //we use one second to ensure thar the avformat_seek_file interface can find the key frame beyond s64SeekMin and s64SeekMax
    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "Seek to %lld (%2d:%02d:%02d) of total  (%2d:%02d:%02d) \n",
                   s64TimeUs, s32SeekHour, s32SeekMin, s32SeekSec, s32FileHour, s32FileMin, s32FileSec);


    HI_S64 s64SeekTarget = s64TimeUs;
    if (s32StreamIndex != -1 && s32StreamIndex < (HI_S32)pstFormatMember->pstFormatContext->nb_streams)
    {
        s64SeekTarget = av_rescale(s64SeekTarget, pstFormatMember->pstFormatContext->streams[s32StreamIndex]->time_base.den,
                                   AV_TIME_BASE * (HI_S64) pstFormatMember->pstFormatContext->streams[s32StreamIndex]->time_base.num);
    }

    HI_S64 s64SeekMin = 0;
    HI_S64 s64SeekMax = 0;
    HI_S32 s32SeekFlag = 0;

    if (s32Flag == HI_DEMUXER_SEEK_BACKWARD)
    {
        s64SeekMin = INT64_MIN;
        s64SeekMax = s64SeekTarget;
    }
    else
    {
        s64SeekMin = s64SeekTarget;
        s64SeekMax = INT64_MAX;
    }

    s32Ret = avformat_seek_file(pstFormatMember->pstFormatContext, s32StreamIndex,  s64SeekMin, s64SeekTarget, s64SeekMax, s32SeekFlag);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "%s: error while seeking %lld ,streamIndex:%d, s32Ret:%d\n",
                       pstFormatMember->pstFormatContext->filename, s64MSec,  s32StreamIndex, s32Ret);
        return HI_ERR_DEMUXER_SEEK;
    }

    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstFormatMember->iLastReadPts = s64SeekTarget;

    return HI_SUCCESS;
}

static HI_S32 COMMON_GetVideoInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    if (HI_FFDEMUX_FORMAT_WAV == pstFormatMember->enFormat)
    {
        return HI_SUCCESS;
    }
    HI_S32 s32Index = 0;
    if (!(pstFormatMember->s32VideoStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM
          && pstFormatMember->pstVideoStream))
    {
        return HI_SUCCESS;
    }

    if (HI_FFDEMUX_FORMAT_TS == pstFormatMember->enFormat)
    {
        pstFmtInfo->s32UsedVideoStreamIndex = pstFormatMember->s32UsedVideoStreamIndex;
        pstFmtInfo->fFrameRate = TS_DEFAULT_FPS;
        pstFmtInfo->enVideoType = pstFmtInfo->stSteamResolution[pstFormatMember->s32VideoStreamIndex].enVideoType;
        for (s32Index = 0; s32Index < HI_DEMUXER_RESOLUTION_CNT; s32Index++)
        {
            pstFmtInfo->stSteamResolution[s32Index].u32Width =  pstFormatMember->stSteamResolution[s32Index].u32Width;
            pstFmtInfo->stSteamResolution[s32Index].u32Height =  pstFormatMember->stSteamResolution[s32Index].u32Height;
            pstFmtInfo->stSteamResolution[s32Index].s32VideoStreamIndex = pstFormatMember->stSteamResolution[s32Index].s32VideoStreamIndex;
            pstFmtInfo->stSteamResolution[s32Index].enVideoType = pstFormatMember->stSteamResolution[s32Index].enVideoType;
        }
        pstFmtInfo->enVideoType = pstFmtInfo->stSteamResolution[pstFormatMember->s32VideoStreamIndex].enVideoType;
    }

    if (HI_FFDEMUX_FORMAT_MP4 == pstFormatMember->enFormat)
    {

        for (s32Index = 0; s32Index < HI_DEMUXER_RESOLUTION_CNT; s32Index++)
        {
            pstFmtInfo->stSteamResolution[s32Index].u32Width =  pstFormatMember->stSteamResolution[s32Index].u32Width;
            pstFmtInfo->stSteamResolution[s32Index].u32Height =  pstFormatMember->stSteamResolution[s32Index].u32Height;
            pstFmtInfo->stSteamResolution[s32Index].s32VideoStreamIndex = pstFormatMember->stSteamResolution[s32Index].s32VideoStreamIndex;

            pstFmtInfo->stSteamResolution[s32Index].enVideoType = pstFormatMember->stSteamResolution[s32Index].enVideoType;
        }
        pstFmtInfo->s32UsedVideoStreamIndex = pstFormatMember->s32VideoStreamIndex;

        /*if video file duration short than 1s, den will be 0, use default fps 30*/
        AVRational stFrameRate = pstFormatMember->pstVideoStream->avg_frame_rate;

        pstFmtInfo->fFrameRate = (stFrameRate.den && stFrameRate.num) ?
                                 (HI_FLOAT)stFrameRate.num / (HI_FLOAT)stFrameRate.den : FFMPEG_DEFAULT_FPS;

        pstFmtInfo->enVideoType = pstFmtInfo->stSteamResolution[pstFormatMember->s32VideoStreamIndex].enVideoType;

    }

    if (pstFmtInfo->s32UsedVideoStreamIndex >= HI_DEMUXER_RESOLUTION_CNT)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "select videoidx: %d exceed max support %d \n",
                       pstFmtInfo->s32UsedVideoStreamIndex, HI_DEMUXER_RESOLUTION_CNT - 1);
        return  HI_ERR_DEMUXER_ACTION;
    }

    return HI_SUCCESS;
}

static HI_S32 COMMON_GetAudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    if (!(pstFormatMember->s32AudioStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM
          && pstFormatMember->pstAudioStream))
    {

        return HI_SUCCESS;
    }

    if (HI_FFDEMUX_FORMAT_TS == pstFormatMember->enFormat)
    {
        pstFmtInfo->u32SampleRate = pstFormatMember->u32SampleRate;
        pstFmtInfo->u32AudioChannelCnt = pstFormatMember->u32Chnnum;
    }
    else
    {

        pstFmtInfo->u32SampleRate = pstFormatMember->pstAudioStream->codecpar->sample_rate;
        pstFmtInfo->u32AudioChannelCnt = pstFormatMember->pstAudioStream->codecpar->channels;
    }

    if (HI_FFDEMUX_FORMAT_WAV == pstFormatMember->enFormat)
    {

        pstFmtInfo->enAudioType = HI_FORMAT_AUDIO_TYPE_PCM;
    }
    else
    {
        pstFmtInfo->enAudioType = HI_FORMAT_AUDIO_TYPE_AAC;
    }


    return HI_SUCCESS;
}
static HI_S32 COMMON_GetMetaInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    if (HI_FFDEMUX_FORMAT_MP4 != pstFormatMember->enFormat)
    {
        return HI_SUCCESS;
    }

    if (!pstFormatMember->pstMetaStream && !pstFormatMember->pstVideoStream)
    {
        pstFmtInfo->stSteamResolution[pstFormatMember->s32VideoStreamIndex].s32VideoStreamIndex = pstFormatMember->s32VideoStreamIndex;
        pstFmtInfo->fFrameRate = 0;
        pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
        pstFmtInfo->s32UsedVideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    }

    if (!(pstFormatMember->s32MetaStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM
          && pstFormatMember->pstMetaStream))
    {

        return HI_SUCCESS;
    }

    pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].u32Width =  pstFormatMember->pstMetaStream->codecpar->width;
    pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].u32Height =  pstFormatMember->pstMetaStream->codecpar->height;
    pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].s32VideoStreamIndex = pstFormatMember->s32MetaStreamIndex;

    /*this demux lib will only deal with AV_CODEC_ID_H264/HI_FFMPEG_VIDEO_TYPE_H265 */
    if (AV_CODEC_ID_H264 == pstFormatMember->pstMetaStream->codecpar->codec_id)
    {
        pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].enVideoType = HI_FORMAT_VIDEO_TYPE_H264;
    }
    else if (AV_CODEC_ID_HEVC == pstFormatMember->pstMetaStream->codecpar->codec_id)
    {
        pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].enVideoType = HI_FORMAT_VIDEO_TYPE_H265;
    }
    else
    {
        pstFmtInfo->stSteamResolution[pstFormatMember->s32MetaStreamIndex].enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
    }


    return HI_SUCCESS;
}

static HI_VOID COMMON_CheckData(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    if (pstDemuxer->stPkt.data != pstFmtFrame->pu8Data )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,  "last read packet is not equal to the packet which is going to be freed \n");
    }

    return;
}
//============================================

HI_S32 COMMON_Read(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    HI_S64 s64ReadPts = 0;
    if (NULL == pstFmtFrame)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    HI_S32 s32Ret = HI_SUCCESS ;
    HI_BOOL bGetPacket = HI_FALSE;

    while (bGetPacket != HI_TRUE)
    {
        s32Ret = COMMON_GET_Frame(pstDemuxer);

        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }

        if (pstDemuxer->stPkt.stream_index == pstDemuxer->s32AudioStreamIndex)
        {
            if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
            {
                s32Ret = MP4_SET_AudioInfo(pstDemuxer, pstFmtFrame);
            }
            else if (HI_FFDEMUX_FORMAT_TS == pstDemuxer->enFormat)
            {

                s32Ret = TS_SET_AudioInfo(pstDemuxer, pstFmtFrame);
            }
            else if (HI_FFDEMUX_FORMAT_WAV == pstDemuxer->enFormat)
            {

                s32Ret = COMMON_SET_AudioInfo(pstDemuxer, pstFmtFrame);
            }
            else
            {
                av_packet_unref(&pstDemuxer->stPkt);
                FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "fileformat not support%d  \n", pstDemuxer->enFormat);
                FFDEMUX_UNLOCK(pstDemuxer->demuxMutex);
                return HI_FAILURE;
            }

        }
        else if (pstDemuxer->stPkt.stream_index ==  pstDemuxer->s32UsedVideoStreamIndex)
        {
            if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
            {
                s32Ret = MP4_SET_VideoInfo(pstDemuxer, pstFmtFrame);
            }
            else if (HI_FFDEMUX_FORMAT_TS == pstDemuxer->enFormat)
            {

                s32Ret = TS_SET_VideoInfo(pstDemuxer, pstFmtFrame);
            }
            else
            {
                av_packet_unref(&pstDemuxer->stPkt);
                FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "fileformat not support%d read video\n", pstDemuxer->enFormat);
                FFDEMUX_UNLOCK(pstDemuxer->demuxMutex);
                return HI_FAILURE;
            }
        }
        else
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "have other stream in input file, just ignore it\n");
            av_packet_unref(&pstDemuxer->stPkt);
            continue;
        }
        if (s32Ret == HI_RET_FILE_EOF)
        {
            av_packet_unref(&pstDemuxer->stPkt);
            return s32Ret;
        }
        else if (s32Ret == HI_ERR_DEMUXER_SKIP_FILE)
        {
            av_packet_unref(&pstDemuxer->stPkt);
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "skip error frame\n");
            continue;
        }
        else if (s32Ret != HI_SUCCESS)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_SET_Info(format = %d) exec failed \n", pstDemuxer->enFormat);
            av_packet_unref(&pstDemuxer->stPkt);
            return HI_ERR_DEMUXER_READ_PACKET;
        }

        bGetPacket = HI_TRUE;
    }

    pstFmtFrame->s64Pts = pstDemuxer->iLastReadPts / 1000;
    pstFmtFrame->s64Dts = pstFmtFrame->s64Pts;
    pstFmtFrame->bKeyFrame = (AV_PKT_FLAG_KEY == (AV_PKT_FLAG_KEY & pstDemuxer->stPkt.flags)) ? HI_TRUE : HI_FALSE;
    pstFmtFrame->u32StreamIndex = pstDemuxer->stPkt.stream_index;
    pstFmtFrame->s64Duration = pstDemuxer->stPkt.duration;
    pstFmtFrame->s64Pos = pstDemuxer->stPkt.pos;
    if (pstDemuxer->pstVideoStream)
    {
        pstFmtFrame->pu8Header = pstDemuxer->pstVideoStream->codecpar->extradata;
        pstFmtFrame->u32HeaderLen = pstDemuxer->pstVideoStream->codecpar->extradata_size;
    }

    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "read index: %d orgpts: %lld pts: %lld len: %d\n", pstFmtFrame->u32StreamIndex, s64ReadPts, pstFmtFrame->s64Pts, pstFmtFrame->u32Size);

    return HI_SUCCESS;
}

HI_S32 COMMON_Free(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    if (NULL == pstFmtFrame)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (NULL == pstDemuxer->stPkt.data )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "you should call fmt_read first\n");
        return HI_ERR_DEMUXER_FREE_PACKET;
    }

    if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
    {

        if (! (HI_FALSE == pstDemuxer->bHasAdts && pstDemuxer->stPkt.stream_index == pstDemuxer->s32AudioStreamIndex))
        {
            COMMON_CheckData(pstDemuxer, pstFmtFrame);
        }
    }
    else if (HI_FFDEMUX_FORMAT_WAV == pstDemuxer->enFormat)
    {
        if (! (pstDemuxer->stPkt.stream_index == pstDemuxer->s32AudioStreamIndex))
        {
            COMMON_CheckData(pstDemuxer, pstFmtFrame);
        }
    }
    else
    {
        COMMON_CheckData(pstDemuxer, pstFmtFrame);
    }

    av_packet_unref(&pstDemuxer->stPkt);
    return HI_SUCCESS;

}

HI_S32 COMMON_Seek(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_S32 s32StreamIndex, HI_S64 s64MSec, HI_S32 s32Flag)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_DEMUXER_NO_MEDIA_STREAM == s32StreamIndex)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "s32StreamIndex  is  %d \n", s32StreamIndex);
        return  HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }
    if (HI_FFDEMUX_FORMAT_TS == pstDemuxer->enFormat)
    {

        s32Ret = TS_Seek_MS(pstDemuxer, s64MSec, s32Flag);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }

        s32Ret = TS_Read_UntilIframe(pstDemuxer);
        if (s32Ret != HI_SUCCESS)
        {
            return HI_ERR_DEMUXER_SEEK;
        }
    }
    else
    {
        s32Ret = COMMON_Seekfile( pstDemuxer, s32StreamIndex, s64MSec, s32Flag);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 COMMON_Probe(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bBsfInited = HI_FALSE;
    HI_S64 s64FileSize = 0;

    /* retrieve stream information */
    if (avformat_find_stream_info(pstDemuxer->pstFormatContext, NULL) < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Could not find stream information \n");
        return HI_ERR_DEMUXER_PROBE;
    }
    //probe video when ts & mp4
    s32Ret = COMMON_ProbeVideo(pstDemuxer, &bBsfInited);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_ProbeVideo failed\n");
        return HI_ERR_DEMUXER_PROBE;
    }


    //probe audio
    s32Ret = COMMON_ProbeAudio(pstDemuxer);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_ProbeAudio failed\n");
        return HI_ERR_DEMUXER_PROBE;
    }

    //parse meta
    s32Ret = COMMON_ProbeMeta(pstDemuxer);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_ProbeMeta failed\n");
        return HI_ERR_DEMUXER_PROBE;
    }

    /* dump input information to stderr */
    //av_dump_format(pstDemuxer->pstFormatContext, 0, pstDemuxer->aszFilePath, 0);

    if (COMMON_GET_FileSize(pstDemuxer, &s64FileSize) != HI_SUCCESS)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_GET_FileSize exec failed \n");
        if (bBsfInited && HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
        {
            MP4_AVBSF_Deinit(&pstDemuxer->pstH26XBsfCtx);
            pstDemuxer->pstH26XBsfCtx = NULL;
        }
        return  HI_ERR_DEMUXER_PROBE;
    }

    pstDemuxer->s64FileSize = s64FileSize;

    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "get out fmt_probe.\n");
    return HI_SUCCESS;
}

HI_S32 COMMON_Getinfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    HI_S32 s32Index = 0;
    HI_S64 s64VideoDuration = 0;
    HI_S64 s64AudioDuration = 0;
    HI_S64 s64DudationMax = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    if (NULL == pstFmtInfo)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtInfo is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }


    if (HI_DEMUXER_NO_MEDIA_STREAM ==  pstDemuxer->s32AudioStreamIndex
        && HI_DEMUXER_NO_MEDIA_STREAM == pstDemuxer->s32VideoStreamIndex)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "should call fmt_probe first \n");
        return  HI_ERR_DEMUXER_ACTION;
    }

    if(HI_SUCCESS != memset_s(pstFmtInfo, sizeof(HI_FORMAT_FILE_INFO_S), 0x00, sizeof(HI_FORMAT_FILE_INFO_S)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set  pstFmtInfo fail \n");
        return  HI_FAILURE;
    }


    for (s32Index = 0; s32Index < HI_DEMUXER_RESOLUTION_CNT; s32Index++)
    {
        pstFmtInfo->stSteamResolution[s32Index].u32Width = 0;
        pstFmtInfo->stSteamResolution[s32Index].u32Height = 0;
        pstFmtInfo->stSteamResolution[s32Index].s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFmtInfo->stSteamResolution[s32Index].enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
    }
    pstFmtInfo->fFrameRate = 0;
    pstFmtInfo->enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
    pstFmtInfo->s32UsedVideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    pstFmtInfo->s32UsedAudioStreamIndex = pstDemuxer->s32AudioStreamIndex;
    pstFmtInfo->enAudioType = HI_FORMAT_AUDIO_TYPE_BUTT;
    pstFmtInfo->u32SampleRate = 0;
    pstFmtInfo->u32AudioChannelCnt = 0;

    //get video info
    s32Ret = COMMON_GetVideoInfo(pstDemuxer, pstFmtInfo);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_GetVideoInfo fail \n");
        return s32Ret;
    }


    // get audio info
    s32Ret = COMMON_GetAudioInfo(pstDemuxer, pstFmtInfo);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_GetAudioInfo fail \n");
        return s32Ret;
    }

    // get audio info
    s32Ret = COMMON_GetMetaInfo(pstDemuxer, pstFmtInfo);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_GetMetaInfo fail \n");
        return s32Ret;
    }

    if (pstDemuxer->pstUsedVideoStream && pstDemuxer->pstAudioStream)
    {
        if (pstDemuxer->pstUsedVideoStream->time_base.den)
        {
            s64VideoDuration = pstDemuxer->pstUsedVideoStream->duration * FFMPEG_TIME_UNIT * pstDemuxer->pstUsedVideoStream->time_base.num / pstDemuxer->pstUsedVideoStream->time_base.den;
        }
        if (pstDemuxer->pstAudioStream->time_base.den )
        {
            s64AudioDuration = pstDemuxer->pstAudioStream->duration * FFMPEG_TIME_UNIT * pstDemuxer->pstAudioStream->time_base.num / pstDemuxer->pstAudioStream->time_base.den ;
        }
        s64DudationMax = (s64VideoDuration >= s64AudioDuration) ? s64VideoDuration : s64AudioDuration;
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,
            "get s64DudationMax %lld video %lld audio %lld\n",
            s64DudationMax, s64VideoDuration, s64AudioDuration);
    }

    pstFmtInfo->s64Duration  = pstDemuxer->pstFormatContext->duration / FFMPEG_TIME_UNIT;
    pstFmtInfo->s64Duration = (pstFmtInfo->s64Duration >= s64DudationMax) ? pstFmtInfo->s64Duration : s64DudationMax;
    pstFmtInfo->s64StartTime = 0;
    if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
    {
        pstFmtInfo->s64StartTime =  pstDemuxer->pstFormatContext->start_time / FFMPEG_TIME_UNIT ;
    }
    pstFmtInfo->s64FileSize = pstDemuxer->s64FileSize;

    pstFmtInfo->u32Bitrate   = pstDemuxer->pstFormatContext->bit_rate;


    return HI_SUCCESS;
}

HI_S32 COMMON_Setattr(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_MEDIA_ATTR_S* pstFmtInfo)
{
    HI_S32 s32Ret = HI_FALSE;
    HI_S32 s32Index = 0;
    HI_BOOL bFound = HI_FALSE;
    if (pstFmtInfo->s32AudStreamId != pstDemuxer->s32AudioStreamIndex)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COMMON_Setattr audio index:%d not support error \n", pstFmtInfo->s32AudStreamId);
        return HI_ERR_DEMUXER_NOT_SUPPORT;
    }

    if (HI_FFDEMUX_FORMAT_WAV == pstDemuxer->enFormat) //when wav just set audio
    {
        pstDemuxer->s32AudioStreamIndex = pstFmtInfo->s32AudStreamId;
        return HI_SUCCESS;
    }

    if (HI_FFDEMUX_FORMAT_MP4 == pstDemuxer->enFormat)
    {
        if (pstFmtInfo->s32VidStreamId >= HI_DEMUXER_RESOLUTION_CNT )
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set MP4 video index:%d not support error \n", pstFmtInfo->s32VidStreamId);
            return HI_ERR_DEMUXER_NOT_SUPPORT;
        }

        for (s32Index = 0; s32Index < HI_DEMUXER_RESOLUTION_CNT; s32Index++)
        {
            if (pstFmtInfo->s32VidStreamId == pstDemuxer->stSteamResolution[s32Index].s32VideoStreamIndex)
            {
                bFound = HI_TRUE;
                break;
            }
        }

        if ((pstFmtInfo->s32VidStreamId != pstDemuxer->s32VideoStreamIndex) && (pstFmtInfo->s32VidStreamId != pstDemuxer->s32MetaStreamIndex) && (HI_FALSE == bFound))
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set MP4 video index:%d not support error \n", pstFmtInfo->s32VidStreamId);
            return HI_ERR_DEMUXER_NOT_SUPPORT;
        }

        if (pstFmtInfo->s32VidStreamId == pstDemuxer->s32VideoStreamIndex)
        {
            pstDemuxer->pstUsedVideoStream = pstDemuxer->pstVideoStream;
            pstDemuxer->enUsedVidCodecId =  pstDemuxer->enVidCodecId;
            pstDemuxer->bPlayMetaVid = HI_FALSE;
        }
        else if (pstFmtInfo->s32VidStreamId == pstDemuxer->s32MetaStreamIndex)
        {
            pstDemuxer->pstUsedVideoStream = pstDemuxer->pstMetaStream;
            pstDemuxer->enUsedVidCodecId =  pstDemuxer->enMetaCodecId;
            pstDemuxer->bPlayMetaVid = HI_TRUE;
        }
        else
        {
            pstDemuxer->pstUsedVideoStream = pstDemuxer->pstFormatContext->streams[s32Index];
            pstDemuxer->enUsedVidCodecId =  pstDemuxer->pstFormatContext->streams[s32Index]->codecpar->codec_id;
            pstDemuxer->bPlayMetaVid = HI_FALSE;

            const HI_CHAR* pszFilterName = (AV_CODEC_ID_H264 == pstDemuxer->enUsedVidCodecId) ?
                                               "h264_mp4toannexb" : "hevc_mp4toannexb";
            if(pstDemuxer->pstH26XBsfCtx)
            {
                MP4_AVBSF_Deinit(&pstDemuxer->pstH26XBsfCtx);
                s32Ret = MP4_AVBSF_Init(&pstDemuxer->pstH26XBsfCtx, pstDemuxer->pstUsedVideoStream, pszFilterName);
                if (HI_SUCCESS != s32Ret)
                {
                    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_AVBSF_Init h264_mp4toannexb failed\n");
                    return HI_ERR_DEMUXER_PROBE;
                }
            }
        }
    }

    if (HI_FFDEMUX_FORMAT_TS == pstDemuxer->enFormat)
    {
        if ((HI_U32)pstFmtInfo->s32VidStreamId >= pstDemuxer->pstFormatContext->nb_streams || pstFmtInfo->s32VidStreamId < 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set ts video index:%d not support error \n", pstFmtInfo->s32VidStreamId);
            return HI_ERR_DEMUXER_NOT_SUPPORT;
        }
        pstDemuxer->pstUsedVideoStream = pstDemuxer->pstFormatContext->streams[pstFmtInfo->s32VidStreamId];
        pstDemuxer->enUsedVidCodecId =  pstDemuxer->enVidCodecId;

    }

    pstDemuxer->s32UsedVideoStreamIndex = pstFmtInfo->s32VidStreamId;
    pstDemuxer->s32AudioStreamIndex = pstFmtInfo->s32AudStreamId;

    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, " stream index used now %d \n", pstDemuxer->s32UsedVideoStreamIndex);
    return HI_SUCCESS;

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
