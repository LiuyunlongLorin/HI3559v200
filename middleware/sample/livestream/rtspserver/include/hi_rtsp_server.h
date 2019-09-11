/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtsp_server.h
* @brief     rtsp module header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTSP_SERVER_H__
#define __HI_RTSP_SERVER_H__

#include "hi_server_state_listener.h"
#include "hi_track_source.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define HI_RTSPSVR_FRAME_MAX_BLOCK (8)


/** \addtogroup     RTSPSVR */
/** @{ */  /** <!-- [RTSPSVR] */
#define MODULE_NAME_RTSPSERVER           "RTSPSERVER"

/** RTSP config object type*/
typedef struct hiRTSP_CONFIG_S
{
    HI_S32  s32PacketLen;  /**< length of the send packet [500,5000]recommend 1500 unit:byte*/
    HI_S32  s32MaxConnNum; /**< max connect client num[1,32]*/
    HI_S32  s32ListenPort; /**< listen port of rtspserver[1,65535] normal 554*/
    HI_S32  s32MaxPayload; /**< max payloadtype num in mbuffer[1,254] normal 2*/
    HI_S32  s32Timeout; /**<set the timeout of connect   <0 for not use timeout unit: s*/
} HI_RTSP_CONFIG_S;

typedef struct hiRTSP_DATA_S
{
    HI_U8* apu8DataPtr[HI_RTSPSVR_FRAME_MAX_BLOCK];                   /**< data address */
    HI_U32 au32DataLen[HI_RTSPSVR_FRAME_MAX_BLOCK];             /**< length of frame data */
    HI_U64 u64Pts;            /**< timestamp of frame unit:us*/
    HI_U32 u32Seq;   /**< frame sequence number */
    HI_U32 u32BlockCnt;/**< block count for one frame*/
    HI_BOOL bIsKeyFrame;   /**< key frame flag */
}HI_RTSP_DATA_S;

typedef struct hiRTSP_SOURCE_S
{
    HI_Track_Source_Handle pstVideoSrc;
    HI_Track_Source_Handle pstAudioSrc;
}HI_RTSP_SOURCE_S;

/**
 * @brief create server instance.
 * @param[in,out] ppRtspSvr HI_MW_PTR : return rtspserver handle
 * @param[in] pstRTSPConfig HI_RTSP_CONFIG_S : rtsp config info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_RTSPSVR_Create(HI_MW_PTR* ppRtspSvr, HI_RTSP_CONFIG_S* pstRTSPConfig);

/**
 * @brief destroy server instance.
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_RTSPSVR_Destroy(HI_MW_PTR pRtspSvr);

/**
 * @brief make server running, could handle client connecting and request.
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @return   0 success
 * @return  err num failure
 */
HI_S32 HI_RTSPSVR_Start(HI_MW_PTR pRtspSvr);

/**
 * @brief make server stoped, deny client connecting and request.
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @return   0 success
 * @return  err num failure
 */
HI_S32 HI_RTSPSVR_Stop(HI_MW_PTR pRtspSvr);

/**
 * @brief add media source to server, client could use specified URL \n
 * related with stream connect to this source.
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @param[in] pstMediaStream HI_LIVE_MEDIA_STREAM_S :  Source of outputing MediaInfo and \n
 * VideoStream&AudioStream
 * @param[in] pchStreamName string : stream Name related with this mediaStream,max length 128
 * @param[in]   u32BufSize HI_U32: size of allocate mbuffer[216K,50M] suggest frame size*5 unit:byte
 * @return   0 success
 * @return  err num failure
 */
HI_S32 HI_RTSPSVR_AddMediaStream(HI_MW_PTR pRtspSvr, HI_RTSP_SOURCE_S* pStreamSrc, HI_CHAR* pchStreamName,HI_U32 u32BufSize);


/**
 * @brief remove media source from server, client connecting with \n
 * specified URL related with this stream will receive failing
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @param[in] hTrack HI_Track_Source_Handle :  Source of outputing MediaInfo and \n
  * @param[in] pstData HI_RTSP_DATA_S :  Media data \n
 * VideoStream&AudioStream
 * @return 0 success
 * @return err num failure
 */

HI_S32 HI_RTSPSVR_WriteFrame(HI_MW_PTR pRtspSvr, HI_Track_Source_Handle pTrackSrc, HI_RTSP_DATA_S* pstData);

/**
 * @brief remove media source from server, client connecting with \n
 * specified URL related with this stream will receive failing
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @param[in] pstMediaStream HI_LIVE_MEDIA_STREAM_S :  Source of outputing MediaInfo and \n
 * VideoStream&AudioStream
 * @return 0 success
 * @return err num failure
 */
HI_S32 HI_RTSPSVR_RemoveMediaStream(HI_MW_PTR pRtspSvr, HI_CHAR* pchStreamName);

/**
 * @brief setting listener for listenning client connect&disconnect, server internal error.
 * @param[in] pRtspSvr HI_MW_PTR : rtspserver handle
 * @param[in] pstListener HI_SERVER_STATE_LISTENER_S : input listener object
 * @return    0 success
 * @return    err num failure
 */
HI_S32 HI_RTSPSVR_SetListener(HI_MW_PTR pRtspSvr, HI_SERVER_STATE_LISTENER_S* pstListener);

/** @}*/  /** <!-- ==== RTSPSVR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTSP_SERVER_H__*/
