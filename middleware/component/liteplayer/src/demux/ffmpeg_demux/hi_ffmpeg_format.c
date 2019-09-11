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
#include "securec.h"
#include <string.h>
#include "hi_demuxer.h"
#include "hi_demuxer_err.h"
#include "ffdemux_log.h"
#include "ts_format.h"
#include "mp4_format.h"
#include "ffdemux_common.h"
#include "common_format.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


static HI_VOID FFMPEG_AVBSF_Deinit(AVBSFContext** ppstBsfCtx)
{
    av_bsf_free(ppstBsfCtx);
    return;
}

static HI_VOID FFMPEG_DICARD_Stream(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_S32 i = 0;
    HI_S32 s32StreamNum = pstFormatMember->pstFormatContext->nb_streams;
    for (i = 0; i < s32StreamNum; i++)
    {
        if (i != pstFormatMember->s32UsedVideoStreamIndex && i != pstFormatMember->s32AudioStreamIndex )
        {
            pstFormatMember->pstFormatContext->streams[i]->discard = AVDISCARD_ALL;

        }
        else
        {
            pstFormatMember->pstFormatContext->streams[i]->discard = AVDISCARD_NONE;
        }
    }
    return;
}

HI_S32 HI_FFMPEG_Open(HI_VOID** ppFormat, const HI_CHAR* pszFileName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AVFormatContext* pstFmtCtx = NULL;
    HI_S32 i = 0;
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)malloc(sizeof(HI_FFDEMUXER_FORMAT_MEMBER_S));

    if (NULL ==  pstFormatMember)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "malloc HI_FFDEMUXER_FORMAT_MEMBER_S failed \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }
    if (HI_SUCCESS != memset_s(pstFormatMember, sizeof(HI_FFDEMUXER_FORMAT_MEMBER_S), 0x00, sizeof(HI_FFDEMUXER_FORMAT_MEMBER_S)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "SET HI_FFDEMUXER_FORMAT_MEMBER_S failed \n");
        free(pstFormatMember);
        return HI_FAILURE;
    }

    pstFormatMember->s32AudioStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    pstFormatMember->s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    pstFormatMember->s32MetaStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
    for (i = 0; i < HI_DEMUXER_RESOLUTION_CNT; i++)
    {
        pstFormatMember->stSteamResolution[i].enVideoType = HI_FORMAT_VIDEO_TYPE_BUTT;
        pstFormatMember->stSteamResolution[i].s32VideoStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFormatMember->stSteamResolution[i].u32Height = 0;
        pstFormatMember->stSteamResolution[i].u32Width = 0;
    }

    av_log_set_flags(AV_LOG_SKIP_REPEATED);

    av_init_packet(&pstFormatMember->stPkt);

    /* register all formats and codecs */
    av_register_all();
    HI_S32 s32FileLen = strlen(pszFileName);

    if (s32FileLen < 1  || s32FileLen > PATH_MAX)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "ffmpeg demux  file len is %d beyond [1,%d] \n", s32FileLen, PATH_MAX);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (NULL == realpath(pszFileName, pstFormatMember->aszFilePath))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "realpath %s ret failed errno: %d!\n", pszFileName, errno);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    /* open input file, and allocate format context */
    s32Ret =  avformat_open_input(&pstFmtCtx, pstFormatMember->aszFilePath, NULL, NULL);

    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Could not open source file %s, s32Ret:%d \n",
                       pstFormatMember->aszFilePath, s32Ret);
        free(pstFormatMember);
        pstFormatMember = NULL;
        return HI_ERR_DEMUXER_OPEN_FILE;
    }
    pstFormatMember->pstFormatContext = pstFmtCtx;

    if (strstr(pstFmtCtx->iformat->name, "mpegts"))
    {
        HI_U32 u32Flag = (HI_U32)pstFmtCtx->flags;
        u32Flag = (u32Flag | AVFMT_FLAG_KEEP_SIDE_DATA);
        pstFmtCtx->flags = (HI_S32)u32Flag;
        pstFormatMember->enFormat = HI_FFDEMUX_FORMAT_TS;
    }
    else if (strstr(pstFmtCtx->iformat->name, "mov") || strstr(pstFmtCtx->iformat->name, "mp4") || strstr(pstFmtCtx->iformat->name, "m4a"))
    {
        pstFormatMember->enFormat = HI_FFDEMUX_FORMAT_MP4;
    }
    else if (strstr(pstFmtCtx->iformat->name, "wav"))
    {
        pstFormatMember->enFormat = HI_FFDEMUX_FORMAT_WAV;
    }
    else
    {
        avformat_close_input(&pstFormatMember->pstFormatContext);
        free(pstFormatMember);
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "invalid  file format %s\n", pstFmtCtx->iformat->name);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (pstFmtCtx->nb_streams > HI_DEMUXER_RESOLUTION_CNT)
    {
        avformat_close_input(&pstFormatMember->pstFormatContext);
        free(pstFormatMember);
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "stream num:%d reach max  %d\n", pstFmtCtx->nb_streams, HI_DEMUXER_RESOLUTION_CNT);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    pthread_mutex_init(&pstFormatMember->demuxMutex, NULL);
    *ppFormat = (HI_VOID*) pstFormatMember;
    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "input format: %s\n", pstFmtCtx->iformat->name);
    return HI_SUCCESS;
}




HI_S32 HI_FFMPEG_Close(HI_VOID* pFormat)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;

    FFDEMUX_LOCK(pstFormatMember->demuxMutex);
    avformat_close_input(&pstFormatMember->pstFormatContext);

    if (pstFormatMember->pstH26XBsfCtx)
    {
        FFMPEG_AVBSF_Deinit(&pstFormatMember->pstH26XBsfCtx);
        pstFormatMember->pstH26XBsfCtx = NULL;
    }


    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
    pthread_mutex_destroy(&(pstFormatMember->demuxMutex));

    free(pstFormatMember);
    pstFormatMember = NULL;
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Read(HI_VOID* pFormat, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;
    HI_FORMAT_PACKET_S stFrame = {0};

    if (NULL == pstFmtFrame)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    HI_S32 s32Ret = HI_SUCCESS ;
    FFDEMUX_LOCK(pstFormatMember->demuxMutex);

    s32Ret = COMMON_Read(pstFormatMember, &stFrame);
    if (HI_SUCCESS != s32Ret )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "read packet fail:%d  \n", s32Ret);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }

    if (HI_SUCCESS != memcpy_s(pstFmtFrame, sizeof(HI_FORMAT_PACKET_S), &stFrame, sizeof(HI_FORMAT_PACKET_S)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COPY pstFmtFrame FAIL  \n");

    }

    if (pstFmtFrame->s64Pts < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "demux out pts:%lld negative err \n", pstFmtFrame->s64Pts);
        pstFmtFrame->s64Pts = 0;
    }

    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);

    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Free(HI_VOID* pFormat, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;
    HI_S32 s32Ret = HI_SUCCESS ;

    if (NULL == pstFmtFrame)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtFrame is null \n");
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    FFDEMUX_LOCK(pstFormatMember->demuxMutex);

    s32Ret = COMMON_Free(pstFormatMember, pstFmtFrame);

    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "free packet fail:%d  \n", s32Ret);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }

    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
    return HI_SUCCESS;

}

HI_S32 HI_FFMPEG_Seek(HI_VOID* pFormat, HI_S32 s32StreamIndex, HI_S64 s64MSec, HI_S32 s32Flag)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;

    if (HI_DEMUXER_NO_MEDIA_STREAM == s32StreamIndex)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "s32StreamIndex  is  %d \n", s32StreamIndex);
        return  HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64TimeUs = ((HI_S64)s64MSec * 1000);
    /*s64TimeUs may oversize s64 to be negative value*/
    if ( s64MSec < 0 || s64TimeUs < 0 )
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "seek input time is negative or oversize: %lld ms\n", s64MSec);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    if (s64TimeUs > pstFormatMember->pstFormatContext->duration)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "seek input time is bigger total time seektime: %lld\n",
                       s64TimeUs);
        return HI_ERR_DEMUXER_ILLEGAL_PARAM;
    }

    FFDEMUX_LOCK(pstFormatMember->demuxMutex);
    s32Ret = COMMON_Seek(pstFormatMember, s32StreamIndex, s64MSec, s32Flag);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_WARN, "seek fail:%x,max len=%lld,seek to : %lld \n", s32Ret,pstFormatMember->pstFormatContext->duration / 1000000LL,s64MSec/ 1000LL);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }

    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Probe(HI_VOID* pFormat)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;
    FFDEMUX_LOCK(pstFormatMember->demuxMutex);

    s32Ret = COMMON_Probe(pstFormatMember);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "probe fail:%d  \n", s32Ret);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }
    FFMPEG_DICARD_Stream(pstFormatMember);

    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Getinfo(HI_VOID* pFormat, HI_FORMAT_FILE_INFO_S* pstFmtInfo)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_FORMAT_FILE_INFO_S stFileInfo = {0};

    if (NULL == pstFmtInfo)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "pstFmtInfo is null \n");
        return HI_ERR_DEMUXER_NULL_PTR;
    }

    FFDEMUX_LOCK(pstFormatMember->demuxMutex);

    s32Ret = COMMON_Getinfo(pstFormatMember, &stFileInfo);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "get fileinfo fail:%d  \n", s32Ret);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }

    if (HI_SUCCESS != memcpy_s(pstFmtInfo, sizeof(HI_FORMAT_FILE_INFO_S), &stFileInfo, sizeof(HI_FORMAT_FILE_INFO_S)))
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "COPY pstFmtInfo FAIL  \n");
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return HI_FAILURE;
    }


    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_FFMPEG_Setattr(UNUSED_ATTR HI_VOID* pFormat , HI_FORMAT_MEDIA_ATTR_S* pstFmtInfo)
{
    CHECK_VALID_HANDLE(pFormat);
    HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember = (HI_FFDEMUXER_FORMAT_MEMBER_S*)pFormat;
    HI_S32 s32Ret = HI_SUCCESS;
    FFDEMUX_LOCK(pstFormatMember->demuxMutex);

    s32Ret = COMMON_Setattr(pstFormatMember, pstFmtInfo);
    if (HI_SUCCESS != s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set attr fail:%d  \n", s32Ret);
        FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);
        return s32Ret;
    }
    FFMPEG_DICARD_Stream(pstFormatMember);

    FFDEMUX_UNLOCK(pstFormatMember->demuxMutex);

    return HI_SUCCESS;
}

#ifndef __HI3518EV300__
/** ffmpeg demuxer symbol */
HI_DEMUX_S   g_stFormat_entry = {"ffmpeg_demuxer", "mp4,lrv,mov,ts,m4a,wav",
    {HI_FFMPEG_Open, HI_FFMPEG_Close, HI_FFMPEG_Read, HI_FFMPEG_Free, HI_FFMPEG_Seek, HI_FFMPEG_Probe, HI_FFMPEG_Getinfo, HI_FFMPEG_Setattr}, 2
};
#else
HI_DEMUX_S   g_stFormat_entry = {"ffmpeg_demuxer", "mp4,m4a",
    {HI_FFMPEG_Open, HI_FFMPEG_Close, HI_FFMPEG_Read, HI_FFMPEG_Free, HI_FFMPEG_Seek, HI_FFMPEG_Probe, HI_FFMPEG_Getinfo, HI_FFMPEG_Setattr}, 2
};

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
