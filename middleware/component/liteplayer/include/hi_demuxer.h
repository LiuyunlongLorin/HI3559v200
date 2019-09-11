/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_demuxer.h
* @brief     hidemuxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.06
*/

#ifndef __HI_DEMUXER_H__
#define __HI_DEMUXER_H__

#include "hi_type.h"

/** \addtogroup     PLAYER */
/** @{ */  /** <!-- [PLAYER] */

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define HI_DEMUXER_NAME_LEN (20)
#define HI_DEMUXER_SUPPORT_NAMES_LEN (56)
#define HI_DEMUXER_RESOLUTION_CNT (5)
#define HI_DEMUXER_NO_MEDIA_STREAM  (-1)

#define HI_DEMUXER_SEEK_FORWARD  (1)
#define HI_DEMUXER_SEEK_BACKWARD  (2)

#define HI_RET_FILE_EOF  (2)

typedef struct hiFORMAT_PACKET_S {
    HI_S64 s64Pts;                             /**< the time at which the decompressed packet will be presented to the user. */
    HI_S64 s64Dts;                             /**< the time at which the packet is decompressed. Can be AV_NOPTS_VALUE if it is not stored in the file */
    HI_U8*  pu8Data;                          /**< the address of the packet. */
    HI_U32   u32Size;                          /**< the size of the packet. */
    HI_U8*   pu8Header;              /**< Frame data header address */
    HI_U32   u32HeaderLen;        /**< Frame data header length */
    HI_U32   u32StreamIndex;                  /**< the index of the stream. */
    HI_BOOL   bKeyFrame;                     /**< whether is the key frame. */
    HI_S64 s64Duration;                        /**< Duration of this packet in AVStream->time_base units, 0 if unknown. Equals next_pts - this_pts in presentation order. */
    HI_S64 s64Pos;                             /**< byte position in stream, -1 if unknown. */
} HI_FORMAT_PACKET_S;

typedef struct hiFORMAT_MEDIA_ATTR_S
{
    HI_S32 s32VidStreamId;   /**< Video stream ID.  */
    HI_S32 s32AudStreamId;   /**< Audio stream ID. */
} HI_FORMAT_MEDIA_ATTR_S;

typedef enum hiFORMAT_VIDEO_TYPE_E
{
    HI_FORMAT_VIDEO_TYPE_H264,
    HI_FORMAT_VIDEO_TYPE_H265,
    HI_FORMAT_VIDEO_TYPE_JPEG,
    HI_FORMAT_VIDEO_TYPE_BUTT
}HI_FORMAT_VIDEO_TYPE_E;

typedef enum hiFORMAT_AUDIO_TYPE_E
{
    HI_FORMAT_AUDIO_TYPE_AAC,
    HI_FORMAT_AUDIO_TYPE_PCM,
    HI_FORMAT_AUDIO_TYPE_BUTT
}HI_FORMAT_AUDIO_TYPE_E;

typedef struct hiFORMAT_STREAM_RESOLUTION_S
{
    HI_S32  s32VideoStreamIndex;             /**< the index of the video stream */
    HI_U32  u32Width;                       /**< The height of the media file's resolution */
    HI_U32  u32Height;                      /**< The width of the media file's resolution */
    HI_FORMAT_VIDEO_TYPE_E enVideoType;
}HI_FORMAT_STREAM_RESOLUTION_S;

typedef struct hiFORMAT_FILE_INFO_S
{
    HI_S64  s64FileSize;                    /**< File size, in the unit of byte. */
    HI_S64  s64StartTime;                 /*the media file begin time*/
    HI_S64  s64Duration;                    /**< Total duration of a file, in the unit of ms. */
    HI_FORMAT_STREAM_RESOLUTION_S stSteamResolution[HI_DEMUXER_RESOLUTION_CNT];
    HI_S32  s32UsedVideoStreamIndex;  /*<default used video index>*/
    HI_FLOAT  fFrameRate;                   /**< the frame rate of the stream*/
    HI_U32  u32Bitrate;                     /**< File bit rate, in the unit of bit/s. */
    HI_U32  u32AudioChannelCnt;
    HI_U32  u32SampleRate; /**< the sample rate of the audio stream */
    HI_S32  s32UsedAudioStreamIndex; /**< the index of the audio stream. one file may have many audio streams*/
    HI_FORMAT_VIDEO_TYPE_E enVideoType;
    HI_FORMAT_AUDIO_TYPE_E enAudioType;
} HI_FORMAT_FILE_INFO_S;

typedef struct hiFORMAT_FUN_S
{
    HI_S32 (*fmt_open)(HI_VOID** ppFmt, const HI_CHAR* pszFileName);
    HI_S32 (*fmt_close)(HI_VOID* pFmt);
    HI_S32 (*fmt_read)(HI_VOID* pFmt, HI_FORMAT_PACKET_S* pstFmtFrame);
    HI_S32 (*fmt_free)(HI_VOID* pFmt, HI_FORMAT_PACKET_S* pstFmtFrame);
    HI_S32 (*fmt_seek)(HI_VOID* pFmt, HI_S32 s32StreamIndex, HI_S64 s64MSec, HI_S32 s32Flag);
    HI_S32 (*fmt_probe)(HI_VOID* pFmt);
    HI_S32 (*fmt_getinfo)(HI_VOID* pFmt, HI_FORMAT_FILE_INFO_S* pstFmtInfo);
    HI_S32 (*fmt_setattr)(HI_VOID* pFmt, HI_FORMAT_MEDIA_ATTR_S* pstFmtInfo);
}HI_FORMAT_FUN_S;

typedef struct hiDEMUX_S
{
    HI_CHAR aszDemuxerName[HI_DEMUXER_NAME_LEN]; /**< the demuxer's name */
    HI_CHAR aszSupportFormat[HI_DEMUXER_SUPPORT_NAMES_LEN]; /**< File formats supported by the DEMUX. The file formats are separated by commas (,). */
    HI_FORMAT_FUN_S stFmtFun;
    HI_U32  u32Priority; /**< the priority of the demuxers. the lower value, the higher priority. range from 0-10*/
}HI_DEMUX_S;


/** @}*/  /** <!-- ==== PLAYER End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
