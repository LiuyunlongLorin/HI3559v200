/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtspserver_msgparser.h
* @brief     rtspserver msgparser head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTSPSERVER_MSGPARSER_H__
#define __HI_RTSPSERVER_MSGPARSER_H__

#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define RTSP_VER_MAX_LEN  (32)
#define RTSP_METHOD_MAX_LEN   (16)
#define RTSP_URL_MAX_LEN   (256)

#define RTSP_MAX_ONLINE_USER     (16)
#define RTSP_IPADDE_LEN          (16)
#define RTSP_TIME_LEN            (32)
#define RTSP_SPS_MAX_LEN         (128)
#define RTSP_PPS_MAX_LEN         (128)
#define RTSP_VPS_MAX_LEN         (128)

#define RTSP_SESSID_MAX_LEN      (16)
#define RTSP_PASSWORD_MAX_LEN    (256)
#define RTSP_USERNAME_MAX_LEN    (256)
#define RTSP_AGENTBUF_MAX_LEN    (128)
#define RTSP_LINE_MAX_LEN        (256)
#define RTSP_TRASH_MAX_LEN       (256)
#define RTSP_RANGE_MAX_LEN       (64)
#define RTSP_OBJ_MAX_LEN         (256)
#define RTSP_UNCARE_MAX_LEN      (256)
#define RTSP_IP_MAX_LEN          (16)
#define RTSP_COOK_MAX_LEN        (64)
#define RTSP_CHAR_MAX_LEN        (32)
#define RTSP_MAX_PROTOCOL_BUFFER (1024)
#define RTSP_SCANF_RET_ONE (1)
#define RTSP_SCANF_RET_TWO (2)
#define RTSP_SCANF_RET_THREE (3)
#define RTSP_AUDIO_SINGLE_CHN (1)
#define RTSP_AUDIO_DOUBLE_CHN (2)
#define RTSP_MAX_STREAMNAME_LEN (128)

#define HIRTSP_VER_STR  "RTSP/1.0"
#define RTSP_LR   "\r"
#define RTSP_LF   "\n"
#define RTSP_LRLF "\r\n"
#define RTSP_SERVER_DESCRIPTION "Hisilicon RTSP Streaming Media Server/1.0.0"
#define RTSP_METHOD_OPTIONS "OPTIONS"
#define RTSP_METHOD_DESCRIBE "DESCRIBE"
#define RTSP_METHOD_SETUP "SETUP"
#define RTSP_METHOD_PLAY "PLAY"
#define RTSP_METHOD_PAUSE "PAUSE"
#define RTSP_METHOD_TEARDOWN "TEARDOWN"

/* message header keywords */
#define RTSP_HEADER_CONTENTLENGTH "Content-Length"
#define RTSP_HEADER_ACCEPT "Accept"
#define RTSP_HEADER_CONTENTTYPE "Content-Type"
#define RTSP_HEADER_DATE "Date"
#define RTSP_HEADER_CSEQ "CSeq"
#define RTSP_HEADER_SESSION "Session"
#define RTSP_HEADER_TRANSPORT "Transport"
#define RTSP_VER_STR "RTSP/1.0"
#define RTSP_TRACK_ID "trackID="
#define RTSP_SDP_CONTENT_TYPE "application/sdp"
#define RTSP_SUPPORTED_CMD_LIST   "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN"

#define HIRTSP_STATUS_CONTINUE             (100)
#define HIRTSP_STATUS_OK                   (200)
#define HIRTSP_STATUS_ACCEPTED             (202)
#define HIRTSP_STATUS_BAD_REQUEST          (400)
#define HIRTSP_STATUS_UNAUTHORIZED         (401)
#define HIRTSP_STATUS_STREAM_NOT_FOUND   (404)
#define HIRTSP_STATUS_METHOD_NOT_ALLOWED   (405)
#define HIRTSP_STATUS_OVER_SUPPORTED_CONNECTION   (416)
#define HIRTSP_STATUS_SESSION_NOT_FOUND   (454)
#define HIRTSP_STATUS_UNSUPPORT_TRANSPORT   (461)
#define HIRTSP_STATUS_INTERNAL_SERVER_ERROR (500)
#define HIRTSP_STATUS_SERVICE_UNAVAILIABLE (503)
#define HIRTSP_STATUS_OPTION_UNSUPPORT (551)

typedef struct _HiRTSPStatus
{
    char*  pszStatStr;
    HI_S32   s32StatCode;
} HiRTSPStatus;

typedef enum _Hi_RTSP_REQ_METHOD_E
{
    /* method codes */
    HIRTSP_OPTIONS_METHOD     = 0 ,
    HIRTSP_DISCRIBLE_METHOD     = 1 ,
    HIRTSP_SETUP_METHOD   = 2,
    HIRTSP_PLAY_METHOD   = 3 ,
    HIRTSP_PAUSE_METHOD  = 4 ,
    HIRTSP_TEARDOWN_METHOD  = 5 ,
    HIRTSP_GET_PARAM_METHOD = 6 ,
    HIRTSP_SET_PARAM_METHOD = 7,
    HIRTSP_REQ_METHOD_BUTT
} HI_RTSP_REQ_METHOD_E;

/** audio sample rate*/
typedef enum hiLIVE_AUDIO_SAMPLE_RATE_E
{
    HI_LIVE_AUDIO_SAMPLE_RATE_8     = 8000,   /* 8K Sample rate     */
    HI_LIVE_AUDIO_SAMPLE_RATE_11025 = 11025,   /* 11.025K Sample rate*/
    HI_LIVE_AUDIO_SAMPLE_RATE_16    = 16000,   /* 16K Sample rate    */
    HI_LIVE_AUDIO_SAMPLE_RATE_22050 = 22050,   /* 22.050K Sample rate*/
    HI_LIVE_AUDIO_SAMPLE_RATE_24    = 24000,   /* 24K Sample rate    */
    HI_LIVE_AUDIO_SAMPLE_RATE_32    = 32000,   /* 32K Sample rate    */
    HI_LIVE_AUDIO_SAMPLE_RATE_441   = 44100,   /* 44.1K Sample rate  */
    HI_LIVE_AUDIO_SAMPLE_RATE_48    = 48000,   /* 48K Sample rate    */
    HI_LIVE_AUDIO_SAMPLE_RATE_BUTT
} HI_LIVE_AUDIO_SAMPLE_RATE_E;
/**
aac low profile
8k     single chn 1588  double chn 1590
16k     single chn 1408  double chn  1410
22.05K  single chn 1388  double chn  1390
24K     single chn 1308  double chn  1310
32k     single chn 1288  double chn  1290
44.1k   single chn 1208  double chn  1210
48k   single chn 1188  double chn  1190
*/
typedef enum hiLIVE_AUDIO_SINGLECHN_CONFIGNUM_E
{
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_8     = 1588,   /**< 8K Sample rate config num    */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_16    = 1408,   /* *<16K Sample rate config num      */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_22050 = 1388,   /**<22.050K Sample rate config num  */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_24    = 1308,   /* *<24K Sample rate config num      */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_32    = 1288,   /**< 32K Sample rate config num      */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_441   = 1208,   /**< 44.1K Sample rate config num    */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_48    = 1188,   /* *<48K Sample rate config num      */
    HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_BUTT
} HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_E;

typedef enum hiRTSP_AUDIO_DOUBLECHN_CONFIGNUM_E
{
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_8     = 1590,   /**< 8K Sample rate  config num      */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_16    = 1410,   /**<16K Sample rate config num     */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_22050 = 1390,   /**< 22.050K Sample rate config num  */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_24    = 1310,   /**<24K Sample rate  config num     */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_32    = 1290,   /* *<32K Sample rate   config num    */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_441   = 1210,   /* *<44.1K Sample rate  config num   */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_48    = 1190,   /* *<48K Sample rate config num      */
    HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_BUTT
} HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_E;


HI_BOOL HI_RTSPSVR_MSGParser_CheckValidRequest(const HI_CHAR* pszReq);

HI_VOID HI_RTSPSVR_MSGParser_GetResponse(HI_S32 StatCode, HI_S32 s32Cseq,
        HI_CHAR* pReply, HI_U32 bufLen);

HI_S32 HI_RTSPSVR_MSGParser_GetStreamName(const HI_CHAR* pszReq,
        HI_CHAR* pStreamName, HI_U32 bufLen);

HI_S32 HI_RTSPSVR_MSGParser_GetCSeq( const HI_CHAR* pszReq, HI_S32* pCseq);

HI_S32 HI_RTSPSVR_MSGParser_GetSessId(const HI_CHAR* pMsgStr, HI_CHAR* pszSessId);

HI_S32 HI_RTSPSVR_MSGParser_ParseMethod(const HI_CHAR* pszReq, HI_RTSP_REQ_METHOD_E* pMethod);

HI_CHAR* HI_RTSPSVR_MSGParser_StatusCode2Str(HI_S32 code);

HI_U32 HI_RTSPSVR_MSGParser_GetAAConfigInfo(HI_U32 u32SampleRate, HI_U32 u32ChanNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTSPSERVER_MSGPARSER_H__*/
