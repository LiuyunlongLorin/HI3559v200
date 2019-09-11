/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         ffdemux_common.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __FFDEMUX_COMMON_H__
#define __FFDEMUX_COMMON_H__

#include "hi_type.h"
#include "hi_defs.h"
#include "ffavcparser.h"
#include "ffhevcparser.h"
#include "ffrbspparser.h"
#include "ffaacparser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/buffer.h"
#include "libavutil/mem.h"


typedef enum hiFFDEMUX_FORMAT_E
{
    HI_FFDEMUX_FORMAT_MP4 = 0,
    HI_FFDEMUX_FORMAT_TS,
    HI_FFDEMUX_FORMAT_WAV,
    HI_FFDEMUX_FORMAT_M4A,
    HI_FFDEMUX_FORMAT_BUTT
} HI_FFDEMUX_FORMAT_E;

/*define NULL handle for codedex check*/
#define HI_NULL_HANDLE (0)
#define UNUSED_ATTR __attribute__((unused))
#define FFMPEG_FRAME_BUF_DATA_LEN    (30 * 1024)

typedef struct hiFORMAT_MEMBER_S
{
    pthread_mutex_t demuxMutex;
    AVFormatContext*  pstFormatContext;
    AVPacket stPkt;
    HI_CHAR aszFilePath[PATH_MAX + 1];
    HI_S32 s32VideoStreamNum;
    HI_S32 s32UsedVideoStreamIndex;
    HI_S32  s32VideoStreamIndex;
    HI_S32  s32AudioStreamIndex;
    HI_S32 s32MetaStreamIndex;

    AVStream* pstUsedVideoStream;
    AVStream* pstVideoStream;
    AVStream* pstAudioStream;
    AVStream* pstMetaStream;

    enum AVCodecID enUsedVidCodecId;
    enum AVCodecID enVidCodecId;
    enum AVCodecID enAudCodecId;
    enum AVCodecID enMetaCodecId;

    AVBSFContext* pstH26XBsfCtx;
    int64_t iLastReadPts;
    HI_U8 aszAudioData[FFMPEG_FRAME_BUF_DATA_LEN];
    HI_BOOL bHasAdts;
    HI_S64  s64FileSize;
    HI_BOOL bPlayMetaVid;//play meta video or not

    HI_U32  u32SampleRate;
    HI_U32  u32Chnnum;
    HI_U32 u32Width;
    HI_U32 u32Height;

    HI_FFDEMUX_FORMAT_E enFormat;
    HI_FORMAT_STREAM_RESOLUTION_S stSteamResolution[HI_DEMUXER_RESOLUTION_CNT];
} HI_FFDEMUXER_FORMAT_MEMBER_S;

#define CHECK_VALID_HANDLE(handle) \
    {\
        if(NULL == handle)\
        {\
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "demux_handle:%p  is not valid \n", handle); \
            return HI_ERR_DEMUXER_INVALID_HANDLE;\
        }\
    }

#define FFDEMUX_LOCK(mutex) \
    do \
    {\
        (void)pthread_mutex_lock(&(mutex)); \
    }while(0)

#define FFDEMUX_UNLOCK(mutex) \
    do \
    {\
        (void)pthread_mutex_unlock(&(mutex));\
    }while(0)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FFDEMUX_COMMON_H__ */
