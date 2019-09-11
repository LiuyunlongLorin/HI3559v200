/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtspserver_session.h
* @brief     rtspserver session head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#ifndef __HI_RTSPSERVER_SESSION_H__
#define __HI_RTSPSERVER_SESSION_H__

#include <pthread.h>
#include "hi_mw_type.h"
#include "hi_list.h"
#include "hi_rtsp_server.h"
#include "hi_track_source.h"
#include "hi_rtspserver_msgparser.h"
#include "hi_server_state_listener.h"
#include "hi_rtp_packetize.h"
#include "hi_rtp_session.h"
#include "hi_rtcp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define RTSP_TRACKID_VIDEO (0)
#define RTSP_TRACKID_AUDIO (1)
#define RTSP_INVALID_TRACK_ID (-1)
#define RTSP_INVALID_THREAD_ID (-1)
#define RTSP_MAX_NALBASE_LEN (256)
#define CHECK_WAIT_MAX_NUM (600)
#define CHECK_WAIT_TIMEOUT (10000)
#define CHECK_WAIT_I_FRAME_COUNT (100)

#define RTSPSVR_CHECK_NULL_ERROR(condition) \
    do \
    { \
        if(condition == NULL) \
        { \
            return HI_ERR_RTSPSERVER_NULL_PTR; \
        }\
    }while(0)

#define RTSP_LOCK(mutex) \
    do \
    { \
        (HI_VOID)pthread_mutex_lock(&mutex);  \
    }while(0)

#define RTSP_UNLOCK(mutex)  \
    do \
    { \
        (HI_VOID)pthread_mutex_unlock(&mutex); \
    }while(0)

/** max len for nal param*/
#define MAX_NALPARA_LEN (128)

typedef enum hi_RTSP_SESSION_STATE_E
{
    HIRTSP_SESSION_STATE_INIT    = 0,
    HIRTSP_SESSION_STATE_READY   = 1,
    HIRTSP_SESSION_STATE_PLAY = 2,
    HIRTSP_SESSION_STATE_STOP = 3,
    HIRTSP_SESSION_STATE_BUTT
} HI_RTSP_SESSION_STATE_E;

typedef enum
{
    AVC_SEI = 0x06,
    AVC_IDR = 0x05,
    AVC_SPS = 0x07,
    AVC_PPS = 0x08,
} HI_RTSP_AVC_NALU_E;

typedef enum
{
    HEVC_PSLICE = 0x01,
    HEVC_IDR = 0x13,
    HEVC_SPS = 0x21,
    HEVC_PPS = 0x22,
    HEVC_VPS = 0x20,
    HEVC_SEI = 0x27
} HI_RTSP_HEVC_NALU_E;

typedef struct hi_RTSP_MEDIA_SESSION_S
{
    HI_RTP_SESSION_S* pRtpSession;
    HI_RTCP_SESSION_S* pRtcpSession;
    pthread_mutex_t  mMutexGetPort;
    HI_S32 s32TrackId;
    HI_CHAR aszUrl[RTSP_URL_MAX_LEN];
} HI_RTSP_MEDIA_SESSION_S;

typedef struct hiLIVE_MEDIA_STREAM_NODE_S
{
    /*used to maintain link list*/
    List_Head_S  listPtr;
    HI_CHAR name[RTSP_MAX_STREAMNAME_LEN];
    HI_Track_Source_Handle pstVidStream;
    HI_Track_Source_Handle pstAudStream;
    HI_U32 u32BufSize;
    HI_U32 u32VidStartCnt;
    HI_U32 u32AudStartCnt;

} HI_LIVE_MEDIA_STREAM_NODE_S;

typedef struct _Hi_RTSP_SERVER_CTX_S
{
    List_Head_S pStreamlist;
    pthread_mutex_t streamListLock;
    List_Head_S pSessionlist;
    pthread_mutex_t sessionListLock;
    HI_SERVER_STATE_LISTENER_S stListener;
    HI_S32 s32MaxConNum;
    HI_S32 s32UserNum;
    HI_S32 s32StreamNum;
    HI_S32 s32ListenPort;
    HI_S32  s32PacketLen;
    HI_U32  u32BufSize;
    HI_S32  s32MaxPayload;
    HI_S32  s32Timeout;
    HI_MW_PTR hNetListener;
    HI_BOOL bStarted;
} HI_RTSP_SERVER_CTX_S;

/** nal param buffer struct*/
typedef struct hiNAL_PARAM_BUFFER_S
{
    HI_U8 u8Buffer[MAX_NALPARA_LEN];/**< nal data buf  */
    HI_U32 u32DataLen;/**< nal data len  */
} HI_NAL_PARAM_BUFFER_S;

typedef struct hi_RTSP_MEDIA_INFO_S
{
    HI_BOOL bVideoEnable;
    HI_BOOL bAudioEnable;
    HI_MW_PTR hMbufHandle;
    HI_NETWORK_BUFFER_INFO_S stBufInfo;
    HI_Track_VideoSourceInfo_S stVideoInfo;
    HI_Track_AudioSourceInfo_S stAudioInfo;
    HI_NAL_PARAM_BUFFER_S stSPS;
    HI_NAL_PARAM_BUFFER_S stPPS;
    HI_NAL_PARAM_BUFFER_S stVPS;
    HI_U64 u64VidStartPts;/**< start timestamp of frame */
    HI_U64 u64AudStartPts;/**< start timestamp of frame */
} HI_RTSP_MEDIA_INFO_S;

struct hi_RTSP_STREAM_SESSION_S;

typedef struct hi_RTSP_SESSION_LISTENER_S
{
    HI_S32 (*onSessionDestroy)(HI_MW_PTR obj, struct hi_RTSP_STREAM_SESSION_S* pSession);
} HI_RTSP_SESSION_LISTENER_S;

typedef struct hi_RTSP_STREAM_SESSION_S
{
    /*for the management of the list */
    List_Head_S listPtr;
    HI_Track_Source_Handle pstVidStream;
    HI_Track_Source_Handle pstAudStream;
    HI_RTSP_SESSION_LISTENER_S stSessListener;
    HI_MW_PTR u32listenerObj;
    HI_MW_PTR pRtspSvrHandle;//rtspserver handle
    pthread_t ptSessThdId;
    HI_S32 s32SessSock;
    HI_RTSP_SESSION_STATE_E enSessState;
    pthread_mutex_t mStateLock;
    HI_S32 s32CurReqSeq;
    HI_CHAR aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER];
    HI_CHAR aszResponseBuff[RTSP_MAX_PROTOCOL_BUFFER];
    HI_CHAR aszStreamName[RTSP_MAX_STREAMNAME_LEN];
    HI_CHAR aszUrl[RTSP_URL_MAX_LEN];
    HI_CHAR aszClientIP[RTSP_IP_MAX_LEN];
    HI_CHAR aszHostIP[RTSP_IP_MAX_LEN];
    HI_U32 u32RecvLen;
    HI_CHAR aszSessID[RTSP_SESSID_MAX_LEN];
    HI_S32  s32PacketLen;
    HI_U32  u32BufSize;
    HI_S32  s32MaxPayload;
    HI_S32  s32Timeout;
    HI_U64 u64StartPts;
    HI_RTSP_MEDIA_INFO_S stMediaInfo;
    HI_RTSP_MEDIA_SESSION_S stVideoSession;
    HI_RTSP_MEDIA_SESSION_S stAudioSession;
    HI_BOOL bGetIFrame;
    HI_U16 u16CliRTSPPort;
    HI_U32 u32ChnId;
} HI_RTSP_STREAM_SESSION_S;

/*newly add for supporting teardown one track and multi packets*/
#define RTSP_RTCP_HEADER_LENGTH            (4)
#define RTSP_RTCP_BYTES_LENGTH             (4)

#define RTSP_RTCP_PACKRT_TYPE_SR           (200)
#define RTSP_RTCP_PACKRT_TYPE_RR           (201)
#define RTSP_RTCP_PACKRT_TYPE_SDES         (202)
#define RTSP_RTCP_PACKRT_TYPE_BYE          (203)
#define RTSP_RTCP_PACKRT_TYPE_APP          (204)

#define RTSP_RESPONSE_PACKET_LENGTH        (4)
#define RTSP_RESPONSE_PACKRT_FIRST_BYTE    (36)
#define RTSP_RESPONSE_PACKRT_THIRD_BYTE    (0)



HI_S32 HI_RTSPSVR_StreamSession_Create(HI_RTSP_STREAM_SESSION_S** ppSession, HI_S32 s32SocketFd);

HI_S32 HI_RTSPSVR_StreamSession_Destroy(HI_RTSP_STREAM_SESSION_S* pSession);

HI_VOID HI_RTSPSVR_StreamSession_SetMediaSource(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_Track_Source_Handle pstVidStream, HI_Track_Source_Handle pstAudStream, const HI_CHAR* pStreamName);

HI_S32 HI_RTSPSVR_StreamSession_SetListener(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_RTSP_SESSION_LISTENER_S* listener, HI_MW_PTR obj);

HI_VOID HI_RTSPSVR_StreamSession_GetClientIPAddress(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_CHAR* pBuffer, HI_S32 s32bufLen);

HI_S32 HI_RTSPSVR_StreamSession_OnClientConnect(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_CHAR* pClientRequest, HI_U32 u32ReqLen);

HI_S32 HI_RTSPSVR_StreamSession_Send(HI_S32 s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen);

HI_S32 HI_RTSPSVR_StreamSession_Stop(HI_RTSP_STREAM_SESSION_S* pSession);
HI_S32 HI_RTSPSVR_StreamSession_WriteFrame(HI_RTSP_STREAM_SESSION_S* pSession, HI_Track_Source_Handle pTrackSrc, HI_RTSP_DATA_S* pstStreamData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTSPSERVER_SESSION_H__*/
