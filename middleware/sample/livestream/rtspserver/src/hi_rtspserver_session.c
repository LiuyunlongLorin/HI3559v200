/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtspserver_session.c
* @brief     rtspserver session src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <pthread.h>
#ifndef __HuaweiLite__
#include <signal.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_network_common.h"
#include "hi_network_mbuffer.h"
#include "hi_rtsp_server.h"
#include "hi_rtsp_server_err.h"
#include "hi_rtspserver_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define  RTSP_MAX_SDP_LEN (1024)
#define  RTSP_TRANS_TIMEVAL_SEC    (3)
#define  RTSP_TRANS_TIMEVAL_USEC  (0)
#define RTSP_CLOCK_RATE (90000)
#define RTSP_TIME_SCALE (1000000)
#define H265_NAL_MASK (0x7f)
#define H264_NAL_MASK (0x1f)

static HI_VOID RTSPSVR_CutMsg(HI_CHAR* pMsgBuf, HI_U32* pu32Len)
{
    HI_CHAR aszTemp[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR* pszIndexTemp = NULL;
    HI_U32 u32MsgLen = 0;

    if (HI_SUCCESS != strncpy_s(aszTemp, RTSP_MAX_PROTOCOL_BUFFER, pMsgBuf, RTSP_MAX_PROTOCOL_BUFFER - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  pMsgBuf fail\n");
        return;
    }
    aszTemp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';


    pszIndexTemp = strstr(aszTemp, "\r\n\r\n");
    if (HI_NULL == pszIndexTemp)
    {
        return;
    }

    if (HI_SUCCESS !=  memset_s(pMsgBuf, *pu32Len, 0x00, *pu32Len))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  pMsgBuf fail\n");
        return;
    }

    u32MsgLen = (HI_U32)(pszIndexTemp - aszTemp) + 4;

    pszIndexTemp += 4;
    *pu32Len =  *pu32Len - u32MsgLen;
    if (*pu32Len > 0)
    {
        if (HI_SUCCESS != strncpy_s(pMsgBuf, RTSP_MAX_PROTOCOL_BUFFER, pszIndexTemp, RTSP_MAX_PROTOCOL_BUFFER - 1))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  pMsgBuf fail\n");
            return;
        }
        aszTemp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
    }

    return;
}

/**
 * @brief cut header response msg.
 * @param[in] pcRequest : msg from client, pu32Strlen : length of Msg
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_CutInvaildMsg(HI_CHAR* pszRequest, HI_U32* pu32Strlen)
{
    HI_U32 u32MsgLen = 0;
    HI_CHAR aszTemp[RTSP_MAX_PROTOCOL_BUFFER] = {0};

    if ((NULL == pszRequest) || (NULL == pu32Strlen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "The input msg is NULL\n");
        return HI_FAILURE;
    }

    if ((*(pszRequest + 0) == RTSP_RESPONSE_PACKRT_FIRST_BYTE))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "it is contains response packet\n");
        u32MsgLen = RTSP_RESPONSE_PACKET_LENGTH;
        if (HI_SUCCESS != memset_s(aszTemp, RTSP_MAX_PROTOCOL_BUFFER, 0, RTSP_MAX_PROTOCOL_BUFFER))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set aszTemp fail\n");
            return HI_FAILURE;
        }
        if (HI_SUCCESS != memcpy_s(aszTemp, RTSP_MAX_PROTOCOL_BUFFER, pszRequest, RTSP_MAX_PROTOCOL_BUFFER))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy request fail\n");
            return HI_FAILURE;
        }
        aszTemp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';

        if (HI_SUCCESS != memset_s(pszRequest, RTSP_MAX_PROTOCOL_BUFFER, 0x00, RTSP_MAX_PROTOCOL_BUFFER))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set protocal fail\n");
            return HI_FAILURE;
        }
        if (HI_SUCCESS != memcpy_s(pszRequest, RTSP_MAX_PROTOCOL_BUFFER - u32MsgLen, aszTemp + u32MsgLen, RTSP_MAX_PROTOCOL_BUFFER - u32MsgLen))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy protocal fail\n");
            return HI_FAILURE;
        }

        if (*pu32Strlen == u32MsgLen)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "it is just a single response packet\n");
            *pu32Strlen = 0;
            return HI_SUCCESS;
        }

        *pu32Strlen = *pu32Strlen - u32MsgLen;
    }

    return HI_SUCCESS;
}

/**
 * @brief cut header rtcp msg.
 * @param[in] pcRequest : msg from client, pu32Strlen : length of Msg, ps32HalfFlag: the flag of half packet
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_CutRtcpMsg(HI_CHAR* pszRequest, HI_U32* pu32Strlen, HI_BOOL* pb32HalfFlag)
{
    HI_U32 u32MsgLen = 0;
    HI_CHAR aszTemp[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    HI_U32 u32PT = 0;

    if ((NULL == pszRequest) || (NULL == pu32Strlen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "The input msg is NULL\n");
        return HI_FAILURE;
    }

    u32PT = *(pszRequest + 1) & 0xFF;

    while ((RTSP_RTCP_PACKRT_TYPE_SR == u32PT) || (RTSP_RTCP_PACKRT_TYPE_RR == u32PT) || (RTSP_RTCP_PACKRT_TYPE_SDES == u32PT) || (RTSP_RTCP_PACKRT_TYPE_BYE == u32PT) || (RTSP_RTCP_PACKRT_TYPE_APP == u32PT))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "it contains just rtcp packet\n");
        if (HI_SUCCESS != memset_s(aszTemp, RTSP_MAX_PROTOCOL_BUFFER, 0, RTSP_MAX_PROTOCOL_BUFFER))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  aszTemp fail\n");
            return HI_FAILURE;
        }

        if (HI_SUCCESS != memcpy_s(aszTemp, RTSP_MAX_PROTOCOL_BUFFER, pszRequest, RTSP_MAX_PROTOCOL_BUFFER))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy  aszTemp fail\n");
            return HI_FAILURE;
        }
        aszTemp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';

        u32MsgLen = (((aszTemp[2] & 0xFF) << 8) + aszTemp[3] ) * RTSP_RTCP_BYTES_LENGTH  + RTSP_RTCP_HEADER_LENGTH;

        if (*pu32Strlen >= u32MsgLen)
        {
            if (HI_SUCCESS != memset_s(pszRequest, RTSP_MAX_PROTOCOL_BUFFER, 0x00, RTSP_MAX_PROTOCOL_BUFFER))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  pszRequest fail\n");
                return HI_FAILURE;
            }
            if (HI_SUCCESS != memcpy_s(pszRequest, RTSP_MAX_PROTOCOL_BUFFER - u32MsgLen, aszTemp + u32MsgLen, RTSP_MAX_PROTOCOL_BUFFER - u32MsgLen))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy  pszRequest fail\n");
                return HI_FAILURE;
            }

            if (*pu32Strlen == u32MsgLen)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "it is just a single rtcp packet\n");
                *pu32Strlen = 0;
                *pb32HalfFlag = HI_FALSE;
                return HI_SUCCESS;
            }

            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "the packet contains a rtcp and other packet(s)\n");
            *pu32Strlen = *pu32Strlen - u32MsgLen;
            u32PT = *(pszRequest + 1) & 0xFF;

            if ((RTSP_RTCP_PACKRT_TYPE_SR == u32PT) || (RTSP_RTCP_PACKRT_TYPE_RR == u32PT) || (RTSP_RTCP_PACKRT_TYPE_SDES == u32PT) || (RTSP_RTCP_PACKRT_TYPE_BYE == u32PT) || (RTSP_RTCP_PACKRT_TYPE_APP == u32PT))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "the packet still contains a rtcp and other packet(s)\n");
                continue;
            }

            *pb32HalfFlag = HI_FALSE;
            return HI_SUCCESS;
        }

        *pb32HalfFlag = HI_TRUE;
        return HI_SUCCESS;
    }

    *pb32HalfFlag = HI_FALSE;
    return HI_SUCCESS;
}

/**
 * @brief refresh the select time out.
 * @param[in] pstSession : session info, pcRecvBuff : msg from client, pu32BuffLen: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static HI_VOID RTSPSVR_RecvMsg_RefreshSelectTime(HI_RTSP_STREAM_SESSION_S* pstSession, HI_CHAR* pszRecvBuff, HI_U32* pu32BuffLen, struct timeval* TimeoutVal )
{
    if ((NULL == pstSession) || (NULL == pszRecvBuff))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return;
    }

    /*media data transmiting period, do not block media sending*/
    if (HIRTSP_SESSION_STATE_PLAY == pstSession->enSessState)
    {
        TimeoutVal->tv_sec = 0;
        TimeoutVal->tv_usec = 0;
    }
    else
    {
        /*if request not deal completely, not block the thread*/
        if (0 != *pu32BuffLen)
        {
            /*if there is only a half,not need to change the waiting time*/
            if (NULL != strstr(pszRecvBuff, "\r\n\r\n"))
            {
                /*have request msg to be handled*/
                TimeoutVal->tv_sec = 0;
                TimeoutVal->tv_usec = 0;
            }
            else
            {
                TimeoutVal->tv_sec = RTSP_TRANS_TIMEVAL_SEC;
                TimeoutVal->tv_usec = 0;
            }
        }
        else
        {
            /*single packet condition*/
            TimeoutVal->tv_sec = RTSP_TRANS_TIMEVAL_SEC;
            TimeoutVal->tv_usec = 0;
        }

    }
}

/**
 * @brief handle request when select time out.
 * @param[in] pstSession : session info, pcRecvBuff : msg from client, pu32BuffLen: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_RecvMsg_Timeout(HI_RTSP_STREAM_SESSION_S* pstSession , HI_CHAR* pszRecvBuff, HI_U32* pu32BuffLen)
{
    if ((NULL == pstSession) || (NULL == pszRecvBuff))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return HI_FAILURE;
    }

    if (HIRTSP_SESSION_STATE_PLAY != pstSession->enSessState)
    {
        if (0 != *pu32BuffLen)
        {
            if (NULL != strstr(pszRecvBuff, "\r\n\r\n"))
            {
                /*have complete request packet to be handled*/
                return HI_SUCCESS;
            }
        }

        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/*
static HI_BOOL RTSPSVR_IsMutiMethod(HI_CHAR* pMsgBuf, HI_U32 u32Len)
{
    HI_CHAR* pszIndexTemp = NULL;
    HI_U32 u32MsgLen = 0;
    HI_BOOL bRet = HI_FALSE;

    pszIndexTemp = strstr(pMsgBuf, "\r\n\r\n");
    if (pszIndexTemp != HI_NULL)
    {
        u32MsgLen = (HI_U32)(pszIndexTemp - pMsgBuf) + 4;
    }
    else
    {
        u32MsgLen = 0;
    }

    if (u32Len == u32MsgLen || 0 == u32MsgLen)
    {
        bRet = HI_FALSE;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "multi method in one packet \n");
        bRet = HI_TRUE;
    }

    return bRet;
}
*/


/*
static HI_VOID RTSPSVR_Handle_Ok(HI_RTSP_STREAM_SESSION_S* pSess)
{
    snprintf_s(pSess->aszResponseBuff,sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff)-1,
             "%s %d %s\r\n"
             "%sAllow: %s\r\n\r\n",
             RTSP_VER_STR,
             HIRTSP_STATUS_OK,
             HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
             HI_NETWORK_DateHeader(),
             RTSP_SUPPORTED_CMD_LIST);
}
*/

static HI_VOID RTSPSVR_Handle_BadReq(HI_RTSP_STREAM_SESSION_S* pSess)
{
    if (HI_FAILURE == snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                                 "%s %d %s\r\n"
                                 "%sAllow: %s\r\n\r\n",
                                 RTSP_VER_STR,
                                 HIRTSP_STATUS_BAD_REQUEST,
                                 HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_BAD_REQUEST),
                                 HI_NETWORK_DateHeader(),
                                 RTSP_SUPPORTED_CMD_LIST))
    {

        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_Handle_BadReq string print aszResponseBuff error\n");
        return;
    }
    return;
}

static HI_VOID RTSPSVR_Handle_ServerError(HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_RTSPSVR_MSGParser_GetResponse(
        HIRTSP_STATUS_INTERNAL_SERVER_ERROR,
        pSess->s32CurReqSeq,
        pSess->aszResponseBuff,
        sizeof(pSess->aszResponseBuff));
}

static HI_VOID RTSPSVR_Handle_NotSupported(HI_RTSP_STREAM_SESSION_S* pSess)
{
    if (HI_FAILURE == snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                                 "%s %d %s\r\n"
                                 "CSeq: %d \r\n"
                                 "%sAllow: %s\r\n\r\n",
                                 RTSP_VER_STR,
                                 HIRTSP_STATUS_METHOD_NOT_ALLOWED,
                                 HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_METHOD_NOT_ALLOWED),
                                 pSess->s32CurReqSeq,
                                 HI_NETWORK_DateHeader(),
                                 RTSP_SUPPORTED_CMD_LIST))
    {

        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_Handle_NotSupported string print aszResponseBuff error\n");
        return;
    }
    return;
}

static HI_VOID RTSPSVR_Handle_StreamNotFound(HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_RTSPSVR_MSGParser_GetResponse(
        HIRTSP_STATUS_STREAM_NOT_FOUND,
        pSess->s32CurReqSeq,
        pSess->aszResponseBuff,
        sizeof(pSess->aszResponseBuff));
}

static HI_VOID RTSPSVR_Handle_OptionUnsupport(HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_RTSPSVR_MSGParser_GetResponse(
        HIRTSP_STATUS_OPTION_UNSUPPORT,
        pSess->s32CurReqSeq,
        pSess->aszResponseBuff,
        sizeof(pSess->aszResponseBuff));
}

HI_VOID RTSPSVR_Handle_UnsupportedTransport(
    HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_RTSPSVR_MSGParser_GetResponse(
        HIRTSP_STATUS_UNSUPPORT_TRANSPORT,
        pSess->s32CurReqSeq,
        pSess->aszResponseBuff,
        sizeof(pSess->aszResponseBuff));
}
static HI_VOID RTSPSVR_Update_State(HI_RTSP_STREAM_SESSION_S* pSess, HI_RTSP_REQ_METHOD_E eMethod)
{
    (HI_VOID)pthread_mutex_lock(&pSess->mStateLock);

    if (HIRTSP_SETUP_METHOD == eMethod && HIRTSP_SESSION_STATE_INIT == pSess->enSessState)
    {
        pSess->enSessState = HIRTSP_SESSION_STATE_READY;
    }
    if (HIRTSP_PLAY_METHOD == eMethod && HIRTSP_SESSION_STATE_READY == pSess->enSessState)
    {
        hi_usleep(10000);
        pSess->enSessState = HIRTSP_SESSION_STATE_PLAY;
    }
    (HI_VOID)pthread_mutex_unlock(&pSess->mStateLock);

    return;
}

static HI_S32 RTSPSVR_GetRtpPayloadType(HI_RTSP_STREAM_SESSION_S* pSess, BUF_DATATYPE_E enBuffPayloadType, RTP_PT_E* pPayloadType)
{
    HI_Track_AudioCodec_E enAudioFormat = HI_TRACK_AUDIO_CODEC_BUTT;   /*audio format*/
    HI_Track_VideoCodec_E enVideoFormat = HI_TRACK_VIDEO_CODEC_BUTT;   /*video format*/
    RTSPSVR_CHECK_NULL_ERROR(pSess);

    if (BUF_DATATYPE_VIDEO == enBuffPayloadType)
    {
        /* get currnet video format*/
        enVideoFormat = pSess->stMediaInfo.stVideoInfo.enCodecType;
        if (HI_TRACK_VIDEO_CODEC_H265 == enVideoFormat)
        {
            *pPayloadType = RTP_PT_H265;
        }
        else if (HI_TRACK_VIDEO_CODEC_H264 == enVideoFormat)
        {
            *pPayloadType = RTP_PT_H264;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "the video formate is :%d \n", enVideoFormat);
            return HI_ERR_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE;
        }
    }
    else if (BUF_DATATYPE_AUDIO == enBuffPayloadType)
    {
        /* get currnet audio format*/
        enAudioFormat = pSess->stMediaInfo.stAudioInfo.enCodecType;
        if (HI_TRACK_AUDIO_CODEC_G711A == enAudioFormat)
        {
            *pPayloadType = RTP_PT_PCMA;
        }
        else if (HI_TRACK_AUDIO_CODEC_G711Mu == enAudioFormat)
        {
            *pPayloadType = RTP_PT_PCMU;
        }
        else if (HI_TRACK_AUDIO_CODEC_G726 == enAudioFormat)
        {
            *pPayloadType = RTP_PT_G726;
        }
        else if (HI_TRACK_AUDIO_CODEC_AMR == enAudioFormat)
        {
            *pPayloadType = RTP_PT_AMR;
        }
        else if (HI_TRACK_AUDIO_CODEC_ADPCM == enAudioFormat)
        {
            *pPayloadType = RTP_PT_ADPCM;
        }
        else if (HI_TRACK_AUDIO_CODEC_AAC == enAudioFormat)
        {
            *pPayloadType = RTP_PT_AAC;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "the audio formate is :%d \n", enAudioFormat);
            return HI_ERR_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE;
        }

    }
    else
    {
        return HI_ERR_RTSPSERVER_ERR_SESS_UNSUPPORT_MEDIATYPE;
    }

    return HI_SUCCESS;
}

static HI_VOID RTSPSVR_GetMediaSendPort(HI_RTSP_MEDIA_SESSION_S* pMediaSess)
{
    HI_U32 u32MaxPort = 0;
    HI_U32 u32MinPort = 0;
    static HI_U32 u32CurrPort = 0;
    u32MinPort = MIN_SEND_PORT;
    u32MaxPort = MAX_SEND_PORT;
    (HI_VOID)pthread_mutex_lock(&(pMediaSess->mMutexGetPort));

    /*allocate the port num between the min and max user set*/
    if (u32CurrPort < u32MaxPort - 2 && u32CurrPort >= u32MinPort)
    {
        u32CurrPort += 2;
    }
    else
    {
        u32CurrPort = u32MinPort;
    }

    pMediaSess->pRtpSession->s32SvrRTPPort = u32CurrPort;
    pMediaSess->pRtcpSession->s32SvrRTCPPort = u32CurrPort + 1;
    (HI_VOID)pthread_mutex_unlock(&(pMediaSess->mMutexGetPort));

    return;
}



static HI_S32 RTSPSVR_ParseTransports(HI_RTSP_STREAM_SESSION_S* pSess,
                                      const HI_CHAR* pRequest, HI_RTSP_MEDIA_SESSION_S* pMediaSession)
{
    HI_BOOL bLastTransTypeFlag = HI_FALSE;
    HI_CHAR* pTemp = NULL;
    HI_CHAR trash[RTSP_TRASH_MAX_LEN] = {0};
    HI_CHAR line[RTSP_LINE_MAX_LEN] = {0};
    HI_CHAR line1[RTSP_LINE_MAX_LEN] = {0};
    HI_CHAR line2[RTSP_LINE_MAX_LEN] = {0};
    HI_CHAR* pLine1 = NULL;
    HI_CHAR* pLine2 = NULL;
    HI_CHAR aszChId1[RTSP_CHAR_MAX_LEN] = {0};
    HI_CHAR aszChId2[RTSP_CHAR_MAX_LEN] = {0};
    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pMediaSession);

    /* Get the URL */
    if (!sscanf_s(pRequest, " %*s %255s ", pMediaSession->aszUrl, RTSP_URL_MAX_LEN))
    {
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;

    }
    pMediaSession->aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';

    pTemp = strstr(pRequest, RTSP_HEADER_TRANSPORT);
    if ( NULL == pTemp )
    {
        /*must has "Transport:", otherwise not accetpable*/
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "there no Transport field in setup url\n");
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;
    }

    /*
    Transport: RTP/AVP/TCP;unicast;interleaved=0-1;source=ip;ssrc=00003654
    trash-- Transport:
    line: RTP/AVP/TCP;unicast;interleaved=0-1;source=ip;ssrc=00003654
    */
    if (sscanf_s(pTemp, "%10s %255s", trash, RTSP_TRASH_MAX_LEN, line, RTSP_LINE_MAX_LEN) != RTSP_SCANF_RET_TWO)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Transport formed unstandard\n");
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;
    }
    else
    {
        pLine1 = line;
    }
    trash[RTSP_TRASH_MAX_LEN - 1] = '\0';
    line[RTSP_LINE_MAX_LEN - 1] = '\0';

    /*assert how mang trans type contained by Transport*/
    do
    {

        if (HI_SUCCESS != memset_s(line1, RTSP_LINE_MAX_LEN, 0, RTSP_LINE_MAX_LEN))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  line1 fail \n");
        }
        if (HI_SUCCESS != memset_s(line2, RTSP_LINE_MAX_LEN, 0, RTSP_LINE_MAX_LEN))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  line2 fail \n");
        }
        if (HI_SUCCESS != memset_s(aszChId1, RTSP_CHAR_MAX_LEN, 0, RTSP_CHAR_MAX_LEN))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  aszChId1 fail \n");
        }
        if (HI_SUCCESS != memset_s(aszChId2, RTSP_CHAR_MAX_LEN, 0, RTSP_CHAR_MAX_LEN))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set  aszChId2 fail \n");
        }
        pTemp = NULL;

        /*if the string has no "," it means has reached the last trans type info */
        /*line: RTP/AVP/TCP;unicast;interleaved=0-1,RTP/AVP;unicast;clientport=a-b
          line1: RTP/AVP/TCP;unicast;interleaved=0-1
          line2: RTP/AVP;unicast;clientport=c-d*/
        if (sscanf_s(pLine1, "%255s,%255s", line1, RTSP_LINE_MAX_LEN, line2, RTSP_LINE_MAX_LEN) != RTSP_SCANF_RET_TWO)
        {
            bLastTransTypeFlag = HI_TRUE;
        }

        pLine2 = line2;

        if ( strstr(line1, "RTP/AVP/TCP") != NULL )
        {
            pMediaSession->pRtpSession->enMediaTransMode = RTP_TRANS_TCP_ITLV;
            pMediaSession->pRtpSession->enPackType = PACK_TYPE_RTSP_ITLV;
            pMediaSession->pRtpSession->s32RTPSendSock = pSess->s32SessSock ;
            /*get client's tcp receive channel: port1-- media data packet channel(rtp),port2-- net adapt packet channel(rtcp)*/
            pTemp = strstr(line1, "interleaved");

            if (NULL == pTemp )
            {
                pMediaSession->pRtpSession->u32ItlvCliMediaChnId = pSess->u32ChnId;
                pSess->u32ChnId++;
                pMediaSession->pRtpSession->u32ItlvCliAdaptChnId = pSess->u32ChnId;
                pSess->u32ChnId++;
            }
            else
            {
                if (RTSP_SCANF_RET_TWO != sscanf_s(pTemp, "%*[^=]=%31[^-]-%31[^,]", aszChId1, RTSP_CHAR_MAX_LEN, aszChId2, RTSP_CHAR_MAX_LEN))
                {
                    pMediaSession->pRtpSession->u32ItlvCliMediaChnId = pSess->u32ChnId;
                    pSess->u32ChnId++;
                    pMediaSession->pRtpSession->u32ItlvCliAdaptChnId = pSess->u32ChnId;
                    pSess->u32ChnId++;
                }
                else
                {
                    aszChId1[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    aszChId2[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    pMediaSession->pRtpSession->u32ItlvCliMediaChnId = (HI_U32)atoi(aszChId1);
                    pMediaSession->pRtpSession->u32ItlvCliAdaptChnId = (HI_U32)atoi(aszChId2);
                }
            }
        }
        else if ( (strstr(line1, "RTP/AVP") != NULL && strstr(line1, "interleaved") == NULL) || strstr(line1, "RTP/AVP/UDP") != NULL )
        {
            pMediaSession->pRtpSession->enMediaTransMode = RTP_TRANS_UDP;
            pMediaSession->pRtpSession->enPackType = PACK_TYPE_RTP_FUA;

            /*get client's udp receive port: port1--recv media data packet(rtp),port2--recv net adapt packet(rtcp) */
            pTemp = strstr(line1, "client_port");
            if (NULL == pTemp )
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "client_port not found in setup msg \n");
                RTSPSVR_Handle_BadReq(pSess);
                return HI_FAILURE;
            }
            else
            {
                if (RTSP_SCANF_RET_TWO != sscanf_s(pTemp, "%*[^=]=%31[^-]-%31[^,]", aszChId1, RTSP_CHAR_MAX_LEN, aszChId2, RTSP_CHAR_MAX_LEN))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "udp port not found in setup msg \n");
                    RTSPSVR_Handle_BadReq(pSess);
                    return HI_FAILURE;
                }
                else
                {
                    aszChId1[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    aszChId2[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    pMediaSession->pRtpSession->s32CliRTPPort = (HI_S32)atoi(aszChId1);
                    pMediaSession->pRtcpSession->s32CliRTCPPort = (HI_S32)atoi(aszChId2);
                }
            }
        }
        /*not support tans type or not understand characters*/
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "not support tans type in setup msg \n");
            RTSPSVR_Handle_BadReq(pSess);
            return HI_FAILURE;
        }

        if ( (RTP_TRANS_UDP == pMediaSession->pRtpSession->enMediaTransMode
              || RTP_TRANS_TCP_ITLV == pMediaSession->pRtpSession->enMediaTransMode
              || RTP_TRANS_BROADCAST == pMediaSession->pRtpSession->enMediaTransMode) )
        {
            /*if parse out one of the support transtype  then stop else go on */
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "parse out the support trans type\n");
            break;
        }
        else if ( HI_TRUE == bLastTransTypeFlag )
        {
            /*if cant parse out one of the support transtype at the last time return failure */
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "last time dont get the support transtype\n");
            RTSPSVR_Handle_BadReq(pSess);
            return HI_FAILURE;
        }
        else
        {
            pLine1 = pLine2; /*search trans info in line2  at next loop*/
        }

    }
    while ( bLastTransTypeFlag != HI_TRUE);

    return HI_SUCCESS;
}

HI_U32 RTSPSVR_GetHEVCNalUnit(HI_U8* pNaluType, HI_U8* pBuffer , HI_U32 size)
{
    HI_U32 code = 0;
    HI_U32 tmp = 0;
    HI_U32 pos = 0;
    HI_U8 u8NalHead = 0;
    for (code = 0xffffffff, pos = 0; pos < 4; pos++)
    {
        tmp = pBuffer[pos];
        code = (code << 8) | tmp;
    }

    if (code != 0x00000001)
    {
        return 0;
    }
    u8NalHead = pBuffer[pos++];
    u8NalHead = u8NalHead >> 1;
    *pNaluType = u8NalHead & H265_NAL_MASK;

    for (code = 0xffffffff; pos < size; pos++)
    {
        tmp = pBuffer[pos];

        if ((code = (code << 8) | tmp) == 0x00000001)
        {
            break;                //next start code is found
        }
    }

    if (pos == size )              // next start code is not found, this must be the last nalu
    {
        return size;
    }
    else
    {
        return pos - 4 + 1;
    }
}

HI_U32 RTSPSVR_GetAVCNalUnit(HI_U8* pNaluType, HI_U8* pBuffer , HI_U32 size)
{
    HI_U32 code = 0;
    HI_U32 tmp = 0;
    HI_U32 pos = 0;
    HI_U8 u8NalHead = 0;

    for (code = 0xffffffff, pos = 0; pos < 4; pos++)
    {
        tmp = pBuffer[pos];
        code = (code << 8) | tmp;
    }

    if (code != 0x00000001)
    {
        return 0;
    }
    u8NalHead = pBuffer[pos++];

    *pNaluType = u8NalHead & H264_NAL_MASK;

    for (code = 0xffffffff; pos < size; pos++)
    {
        tmp = pBuffer[pos];

        if ((code = (code << 8) | tmp) == 0x00000001)
        {
            break;                //next start code is found
        }
    }

    if (pos == size )              // next start code is not found, this must be the last nalu
    {
        return size;
    }
    else
    {
        return pos - 4 + 1;
    }
}


static HI_S32 RTSPSVR_ParseAvcKeyFrame(HI_RTSP_STREAM_SESSION_S* pSess,  HI_U8* pu8Frame, HI_U32 u32FrameLen, HI_BOOL* pbIDRFound)
{
    HI_U32 u32Cursor = 0;

    //get first I frame and parse sps pps
    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = RTSPSVR_GetAVCNalUnit(&u8NalType, pu8Frame + u32Cursor, u32FrameLen - u32Cursor);
        if (u32NalLen <= 4 || u32Cursor + u32NalLen  > u32FrameLen )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseAvcKeyFrame u32NalLen  illegal: %d \n", u32NalLen);
            return HI_FAILURE;
        }

        if (AVC_SPS == u8NalType )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_DEBUG, "RTSPSVR_ParseAvcKeyFrame sps \n");
            if (u32NalLen - 4 > MAX_NALPARA_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseAvcKeyFrame u32NalLen  reach max: %d \n", u32NalLen);
                return HI_FAILURE;
            }
            if (!pSess->bGetIFrame)
            {
                pSess->stMediaInfo.stSPS.u32DataLen =  u32NalLen - 4;
                if (HI_SUCCESS != memcpy_s(pSess->stMediaInfo.stSPS.u8Buffer, u32NalLen - 4, pu8Frame + u32Cursor + 4, u32NalLen - 4))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "copy  stSPS u8Buffer fail \n");
                    return HI_FAILURE;
                }
            }

        }
        else if (AVC_PPS == u8NalType )
        {
            if (u32NalLen - 4 > MAX_NALPARA_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseAvcKeyFrame u32NalLen  reach max: %d \n", u32NalLen);
                return HI_FAILURE;
            }
            if (!pSess->bGetIFrame)
            {
                pSess->stMediaInfo.stPPS.u32DataLen =  u32NalLen - 4;
                if (HI_SUCCESS != memcpy_s(pSess->stMediaInfo.stPPS.u8Buffer, u32NalLen - 4, pu8Frame + u32Cursor + 4, u32NalLen - 4))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "stPPS copy  u8Buffer fail \n");
                    return HI_FAILURE;
                }
            }
        }

        if ( AVC_IDR == u8NalType )
        {
            *pbIDRFound = HI_TRUE;
        }

        u32Cursor += u32NalLen;
    }
    while (u32Cursor + 4 < u32FrameLen);

    return HI_SUCCESS;
}


static HI_S32  RTSPSVR_ParseHevcKeyFrame(HI_RTSP_STREAM_SESSION_S* pSess, HI_U8* pu8Frame, HI_U32 u32FrameLen, HI_BOOL* pbIDRFound)
{
    HI_U32 u32Cursor = 0;

    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = RTSPSVR_GetHEVCNalUnit(&u8NalType, pu8Frame + u32Cursor, u32FrameLen - u32Cursor);
        if (u32NalLen <= 4 || u32Cursor + u32NalLen  > u32FrameLen)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseHevcKeyFrame u32NalLen  illegal: %d \n", u32NalLen);
            return HI_FAILURE;
        }


        if (HEVC_SPS == u8NalType)
        {
            if (u32NalLen - 4 > MAX_NALPARA_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseHevcKeyFrame u32NalLen  reach max: %d \n", u32NalLen);
                return HI_FAILURE;
            }
            if (!pSess->bGetIFrame)
            {
                pSess->stMediaInfo.stSPS.u32DataLen =  u32NalLen - 4;
                if (HI_SUCCESS != memcpy_s(pSess->stMediaInfo.stSPS.u8Buffer, u32NalLen - 4, pu8Frame + u32Cursor + 4, u32NalLen - 4))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "stSPS copy  u8Buffer fail \n");
                    return HI_FAILURE;
                }
            }
        }
        else if (HEVC_PPS == u8NalType)
        {
            if (u32NalLen - 4 > MAX_NALPARA_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseHevcKeyFrame u32NalLen  reach max: %d \n", u32NalLen);
                return HI_FAILURE;
            }
            if (!pSess->bGetIFrame)
            {
                pSess->stMediaInfo.stPPS.u32DataLen =  u32NalLen - 4;
                if (HI_SUCCESS != memcpy_s(pSess->stMediaInfo.stPPS.u8Buffer, u32NalLen - 4, pu8Frame + u32Cursor + 4, u32NalLen - 4))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "stPPS copy  u8Buffer fail \n");
                    return HI_FAILURE;
                }
            }
        }
        else if (HEVC_VPS == u8NalType)
        {
            if (u32NalLen - 4 > MAX_NALPARA_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseHevcKeyFrame u32NalLen  reach max: %d \n", u32NalLen);
                return HI_FAILURE;
            }
            if (!pSess->bGetIFrame)
            {
                pSess->stMediaInfo.stVPS.u32DataLen =  u32NalLen - 4;
                if (HI_SUCCESS != memcpy_s(pSess->stMediaInfo.stVPS.u8Buffer, u32NalLen - 4, pu8Frame + u32Cursor + 4, u32NalLen - 4))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "stVPS copy  u8Buffer fail \n");
                    return HI_FAILURE;
                }
            }
        }

        if (HEVC_IDR == u8NalType )
        {
            *pbIDRFound = HI_TRUE;
        }

        u32Cursor += u32NalLen;
    }
    while (u32Cursor + 4 < u32FrameLen);

    return HI_SUCCESS;
}

HI_S32 RTSPSVR_CheckKeyFrame(HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_U32 u32WaitCount = 0;

    while (!pSess->bGetIFrame)
    {
        //wait for the i frame come and get sps pps
        hi_usleep(CHECK_WAIT_TIMEOUT);
        u32WaitCount++;
        if (u32WaitCount > CHECK_WAIT_I_FRAME_COUNT)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_Generate_SDPLines(HI_RTSP_STREAM_SESSION_S* pSess,
                                        HI_CHAR* pSdpBuffer, HI_U32 u32BufLen)
{
    HI_U32 u32MsgLen = 0;
    HI_S32 s32Ret = 0;
    HI_S32 s32TemLen = 0;
    HI_CHAR chBaseSPSOutInfo[RTSP_MAX_NALBASE_LEN] = {0};
    HI_CHAR chBasePPSOutInfo[RTSP_MAX_NALBASE_LEN] = {0};
    HI_CHAR chBaseVPSOutInfo[RTSP_MAX_NALBASE_LEN] = {0};
    HI_CHAR chInteropCons[RTSP_MAX_NALBASE_LEN] = {0};
    HI_U32 u32LevelId = 0;
    HI_U32 u32ProfileSpace = 0;
    HI_U32 u32ProfileID = 0;
    HI_U32 u32TierFlag = 0;

    RTSPSVR_CHECK_NULL_ERROR(pSess);

    s32Ret = snprintf_s(pSdpBuffer, u32BufLen, u32BufLen - 1,
                        "v=0\r\n"
                        "o=StreamingServer 3331435948 1116907222000 IN IP4 %s\r\n"
                        "s=%s\r\n"
                        "c=IN IP4 0.0.0.0\r\n"
                        "t=0 0\r\n"
                        "a=control:*\r\n"
                        "a=range:npt=0-\r\n",
                        pSess->aszHostIP,
                        pSess->aszStreamName);
    if (HI_FAILURE == s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf sdp buffer error\n");
        return HI_FAILURE;
    }

    u32MsgLen += s32Ret;

    if ( u32MsgLen >= u32BufLen )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "sdp buffer len is too short\n");
        return HI_FAILURE;
    }

    if (pSess->stMediaInfo.bVideoEnable)
    {
        HI_Track_VideoCodec_E enVideoType = pSess->stMediaInfo.stVideoInfo.enCodecType;
        if (HI_TRACK_VIDEO_CODEC_H264 == enVideoType)
        {
            if (HI_SUCCESS != RTSPSVR_CheckKeyFrame(pSess))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "get sps pps failed, i frame not come yet \n");
                return HI_FAILURE;
            }
            HI_U32 u32TempId1 = pSess->stMediaInfo.stSPS.u8Buffer[1] << 16;
            HI_U32 u32TempId2 = pSess->stMediaInfo.stSPS.u8Buffer[2] << 8;
            HI_U32 u32TempId3 = pSess->stMediaInfo.stSPS.u8Buffer[3];
            u32LevelId = u32TempId1 | u32TempId2 | u32TempId3;
            HI_NETWORK_Base64Encode((const HI_U8*)pSess->stMediaInfo.stSPS.u8Buffer, pSess->stMediaInfo.stSPS.u32DataLen, ( HI_U8*)chBaseSPSOutInfo, s32TemLen);
            HI_NETWORK_Base64Encode((const HI_U8*)pSess->stMediaInfo.stPPS.u8Buffer, pSess->stMediaInfo.stPPS.u32DataLen, ( HI_U8*)chBasePPSOutInfo, s32TemLen);

            s32Ret =  snprintf_s(pSdpBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                 "m=video 0 RTP/AVP %d\r\n"
                                 "a=framerate:%d\r\n"
                                 "a=transform:1,0,0;0,1,0;0,0,1\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d H264/90000\r\n"
                                 "a=fmtp:%d packetization-mode=1;profile-level-id=%06X;sprop-parameter-sets=%s,%s\r\n"
                                 "a=x-dimensions:%d,%d\r\n",
                                 RTP_PT_H264,
                                 pSess->stMediaInfo.stVideoInfo.u32FrameRate,
                                 RTSP_TRACKID_VIDEO,
                                 RTP_PT_H264,
                                 RTP_PT_H264,
                                 u32LevelId,
                                 chBaseSPSOutInfo,
                                 chBasePPSOutInfo,
                                 pSess->stMediaInfo.stVideoInfo.u32Width,
                                 pSess->stMediaInfo.stVideoInfo.u32Height);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf sdp buffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else if (HI_TRACK_VIDEO_CODEC_H265 == enVideoType)
        {
            if (HI_SUCCESS != RTSPSVR_CheckKeyFrame(pSess))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "get sps pps failed, i frame not come yet \n");
                return HI_FAILURE;
            }

            u32ProfileSpace = pSess->stMediaInfo.stVPS.u8Buffer[6] >> 6;
            u32ProfileID = pSess->stMediaInfo.stVPS.u8Buffer[6] & 0x1F;
            u32TierFlag = (pSess->stMediaInfo.stVPS.u8Buffer[6] >> 5) & 0x1;
            u32LevelId = pSess->stMediaInfo.stVPS.u8Buffer[17];
            s32Ret =  snprintf_s(chInteropCons, RTSP_MAX_NALBASE_LEN, RTSP_MAX_NALBASE_LEN - 1, "%02X%02X%02X%02X%02X%02X",
                                 pSess->stMediaInfo.stVPS.u8Buffer[11], pSess->stMediaInfo.stVPS.u8Buffer[12],
                                 pSess->stMediaInfo.stVPS.u8Buffer[13], pSess->stMediaInfo.stVPS.u8Buffer[14],
                                 pSess->stMediaInfo.stVPS.u8Buffer[15], pSess->stMediaInfo.stVPS.u8Buffer[16]);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf chInteropCons error\n");
                return HI_FAILURE;
            }
            HI_NETWORK_Base64Encode((const HI_U8*)pSess->stMediaInfo.stSPS.u8Buffer, pSess->stMediaInfo.stSPS.u32DataLen, ( HI_U8*)chBaseSPSOutInfo, s32TemLen);
            HI_NETWORK_Base64Encode((const HI_U8*)pSess->stMediaInfo.stPPS.u8Buffer, pSess->stMediaInfo.stPPS.u32DataLen, ( HI_U8*)chBasePPSOutInfo, s32TemLen);
            HI_NETWORK_Base64Encode((const HI_U8*)pSess->stMediaInfo.stVPS.u8Buffer, pSess->stMediaInfo.stVPS.u32DataLen, ( HI_U8*)chBaseVPSOutInfo, s32TemLen);
            s32Ret =  snprintf_s(pSdpBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                 "m=video 0 RTP/AVP %d\r\n"
                                 "a=framerate:%d\r\n"
                                 "a=transform:1,0,0;0,1,0;0,0,1\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d H265/90000\r\n"
                                 "a=fmtp:%d profile-space=%u;profile-id=%u;tier-flag=%u;level-id=%u;interop-constraints=%s;"
                                 "sprop-vps=%s;sprop-sps=%s;sprop-pps=%s\r\n"
                                 "a=x-dimensions:%d,%d\r\n",
                                 RTP_PT_H265,
                                 pSess->stMediaInfo.stVideoInfo.u32FrameRate,
                                 RTSP_TRACKID_VIDEO,
                                 RTP_PT_H265,
                                 RTP_PT_H265,
                                 u32ProfileSpace,
                                 u32ProfileID,
                                 u32TierFlag,
                                 u32LevelId,
                                 chInteropCons,
                                 chBaseVPSOutInfo,
                                 chBaseSPSOutInfo,
                                 chBasePPSOutInfo,
                                 pSess->stMediaInfo.stVideoInfo.u32Width,
                                 pSess->stMediaInfo.stVideoInfo.u32Height);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pSdpBuffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "video stream format %d not supported\n", enVideoType);
            return HI_FAILURE;
        }

        if (u32MsgLen >= u32BufLen)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "sdp buffer len is too short\n");
            return HI_FAILURE;
        }
    }

    if (pSess->stMediaInfo.bAudioEnable)
    {
        HI_Track_AudioCodec_E enAudioType = pSess->stMediaInfo.stAudioInfo.enCodecType;

        if (HI_TRACK_AUDIO_CODEC_G726 == enAudioType)
        {
            s32Ret = snprintf_s(pSdpBuffer + u32MsgLen,  u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                "m=audio 0 RTP/AVP %d\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d G726-%d/8000/%d\r\n",
                                RTP_PT_G726, RTSP_TRACKID_AUDIO, RTP_PT_G726,
                                pSess->stMediaInfo.stAudioInfo.u32AvgBytesPerSec,
                                pSess->stMediaInfo.stAudioInfo.u32ChnCnt);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pSdpBuffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else if (HI_TRACK_AUDIO_CODEC_G711A == enAudioType)
        {
            s32Ret =  snprintf_s(pSdpBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                 "m=audio 0 RTP/AVP %d\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d PCMA/8000/%d\r\n",
                                 RTP_PT_PCMA, RTSP_TRACKID_AUDIO, RTP_PT_PCMA,
                                 pSess->stMediaInfo.stAudioInfo.u32ChnCnt);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pSdpBuffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else if (HI_TRACK_AUDIO_CODEC_G711Mu == enAudioType)
        {
            s32Ret =  snprintf_s(pSdpBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                 "m=audio 0 RTP/AVP %d\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d PCMU/8000/%d\r\n",
                                 RTP_PT_PCMU, RTSP_TRACKID_AUDIO, RTP_PT_PCMU,
                                 pSess->stMediaInfo.stAudioInfo.u32ChnCnt);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pSdpBuffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else if (HI_TRACK_AUDIO_CODEC_AAC == enAudioType)
        {
            HI_U32 u32configID = HI_RTSPSVR_MSGParser_GetAAConfigInfo(pSess->stMediaInfo.stAudioInfo.u32SampleRate,
                                 pSess->stMediaInfo.stAudioInfo.u32ChnCnt);
            s32Ret = snprintf_s(pSdpBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                                "m=audio 0 RTP/AVP %d\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d MPEG4-GENERIC/%d/%d\r\n"
                                "a=fmtp:%d streamtype=5;profile-level-id=1;mode=AAC-hbr;"
                                "sizelength=13;indexlength=3;indexdeltalength=3;config=%d\r\n",
                                RTP_PT_AAC, RTSP_TRACKID_AUDIO, RTP_PT_AAC,
                                RTSP_CLOCK_RATE,
                                pSess->stMediaInfo.stAudioInfo.u32ChnCnt,
                                RTP_PT_AAC,
                                u32configID);
            if (HI_FAILURE == s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pSdpBuffer error\n");
                return HI_FAILURE;
            }
            u32MsgLen += s32Ret;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "audio stream format %d not supported\n", enAudioType);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}
static HI_S32 RTSPSVR_Generate_SetupResponse(HI_RTSP_STREAM_SESSION_S* pSess, HI_RTSP_MEDIA_SESSION_S* pMediaSess,
        HI_CHAR* pRespBuffer, HI_U32 u32BufLen)
{
    HI_U32 u32MsgLen = 0;
    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pMediaSess);
    HI_S32 s32Ret = 0;


    if (RTP_TRANS_UDP == pMediaSess->pRtpSession->enMediaTransMode)
    {
        s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                            "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08x;mode=\"PLAY\"\r\n",
                            pMediaSess->pRtpSession->s32CliRTPPort, pMediaSess->pRtcpSession->s32CliRTCPPort,
                            pMediaSess->pRtpSession->s32SvrRTPPort,
                            pMediaSess->pRtcpSession->s32SvrRTCPPort,
                            pMediaSess->pRtpSession->u32Ssrc);
        if (HI_FAILURE == s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
            return HI_FAILURE;
        }
        u32MsgLen += s32Ret;
    }
    else if (RTP_TRANS_TCP_ITLV == pMediaSess->pRtpSession->enMediaTransMode)
    {
        s32Ret =  snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1,
                             "Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d;ssrc=%08x;mode=\"PLAY\"\r\n",
                             pMediaSess->pRtpSession->u32ItlvCliMediaChnId,
                             pMediaSess->pRtpSession->u32ItlvCliAdaptChnId,
                             pMediaSess->pRtpSession->u32Ssrc);
        if (HI_FAILURE == s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
            return HI_FAILURE;
        }
        u32MsgLen += s32Ret;
    }
    else
    {
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
static HI_S32 RTSPSVR_Generate_PlayResponse(HI_RTSP_STREAM_SESSION_S* pSess,
        HI_CHAR* pRespBuffer, HI_U32 u32BufLen)
{
    HI_U32 u32MsgLen = 0;
    RTSPSVR_CHECK_NULL_ERROR(pSess);
    HI_S32 s32Ret = 0;

    s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1, "Range: %s\r\n" , "npt=0.000-");
    if (HI_FAILURE == s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
        return HI_FAILURE;
    }

    u32MsgLen += s32Ret;

    s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen  - u32MsgLen, u32BufLen  - u32MsgLen - 1, "RTP-Info: ");
    if (HI_FAILURE == s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
        return HI_FAILURE;
    }

    u32MsgLen += s32Ret;

    if (pSess->stVideoSession.s32TrackId != RTSP_INVALID_TRACK_ID)
    {
        pSess->stVideoSession.pRtpSession->u32SeqNum = 0;
        s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1, "url=%s;seq=%u;rtptime=%u,",
                            pSess->stVideoSession.aszUrl,
                            pSess->stVideoSession.pRtpSession->u32SeqNum,
                            0);
        if (HI_FAILURE == s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
            return HI_FAILURE;
        }
        u32MsgLen += s32Ret;
    }

    if (pSess->stAudioSession.s32TrackId != RTSP_INVALID_TRACK_ID)
    {
        pSess->stAudioSession.pRtpSession->u32SeqNum = 0;
        s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1, "url=%s;seq=%u;rtptime=%u",
                            pSess->stAudioSession.aszUrl,
                            pSess->stAudioSession.pRtpSession->u32SeqNum,
                            0);
        if (HI_FAILURE == s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
            return HI_FAILURE;
        }

        u32MsgLen += s32Ret;
    }

    s32Ret = snprintf_s(pRespBuffer + u32MsgLen, u32BufLen - u32MsgLen, u32BufLen - u32MsgLen - 1, "\r\n");
    if (HI_FAILURE == s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
        return HI_FAILURE;
    }
    u32MsgLen += s32Ret;
    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_ReduceCount(HI_RTSP_STREAM_SESSION_S* pSess, HI_Track_Source_Handle pstStream, HI_U32* pu32Count)
{
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    List_Head_S* pstPosNode = NULL;
    HI_BOOL bFoundStream = HI_FALSE;

    //get stream node, and cal start num
    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)pSess->pRtspSvrHandle;

    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if (HI_TRACK_SOURCE_TYPE_VIDEO == pstStream->enTrackType )
        {
            if ( pstStreamNode->pstVidStream == pstStream && !strcasecmp(pstStreamNode->name, pSess->aszStreamName))
            {
                bFoundStream = HI_TRUE;
                break;
            }
        }
        else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstStream->enTrackType )
        {
            if ( pstStreamNode->pstAudStream == pstStream && !strcasecmp(pstStreamNode->name, pSess->aszStreamName))
            {
                bFoundStream = HI_TRUE;
                break;
            }
        }
    }

    if (bFoundStream)
    {
        if (HI_TRACK_SOURCE_TYPE_VIDEO == pstStream->enTrackType )
        {
            if (0 == pstStreamNode->u32VidStartCnt)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "video already stop, do not stop again\n");
                RTSP_UNLOCK(pstServerCtx->streamListLock);
                return HI_FAILURE;
            }
            pstStreamNode->u32VidStartCnt--;
            *pu32Count = pstStreamNode->u32VidStartCnt;
        }
        else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstStream->enTrackType )
        {
            if (0 == pstStreamNode->u32AudStartCnt)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "audio already stop, do not stop again\n");
                RTSP_UNLOCK(pstServerCtx->streamListLock);
                return HI_FAILURE;
            }
            pstStreamNode->u32AudStartCnt--;
            *pu32Count = pstStreamNode->u32AudStartCnt;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ReduceCount session videostream not found\n");
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_FAILURE;
    }

    RTSP_UNLOCK(pstServerCtx->streamListLock);

    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_AddCount(HI_RTSP_STREAM_SESSION_S* pSess, HI_Track_Source_Handle pstStream, HI_U32* pu32Count)
{
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    List_Head_S* pstPosNode = NULL;
    HI_BOOL bFoundStream = HI_FALSE;

    //get stream node, and cal start num
    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)pSess->pRtspSvrHandle;

    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if (HI_TRACK_SOURCE_TYPE_VIDEO == pstStream->enTrackType )
        {
            if ( pstStreamNode->pstVidStream == pstStream && !strcasecmp(pstStreamNode->name, pSess->aszStreamName))
            {
                bFoundStream = HI_TRUE;
                break;
            }
        }
        else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstStream->enTrackType )
        {
            if ( pstStreamNode->pstAudStream == pstStream && !strcasecmp(pstStreamNode->name, pSess->aszStreamName))
            {
                bFoundStream = HI_TRUE;
                break;
            }
        }

    }
    if (bFoundStream)
    {
        if (HI_TRACK_SOURCE_TYPE_VIDEO == pstStream->enTrackType )
        {
            pstStreamNode->u32VidStartCnt++;
            *pu32Count = pstStreamNode->u32VidStartCnt;
        }
        else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstStream->enTrackType )
        {
            pstStreamNode->u32AudStartCnt++;
            *pu32Count = pstStreamNode->u32AudStartCnt;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_AddCount session videostream not found\n");
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_FAILURE;
    }

    RTSP_UNLOCK(pstServerCtx->streamListLock);
    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_VideoSource_Remove(HI_RTSP_STREAM_SESSION_S* pSess,
        HI_Track_Source_Handle pVideoStream)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pVideoStream);

    //get stream node, and cal start num
    s32Ret = RTSPSVR_ReduceCount(pSess, pVideoStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session video RTSPSVR_ReduceCount failed Ret: %d\n", s32Ret);
        return s32Ret;
    }

    if (0 == u32Count)
    {
        s32Ret = pVideoStream->pfnSourceStop(pVideoStream, pSess->pRtspSvrHandle);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session videostream stop failed Ret: %d\n", s32Ret);
        }

    }
    return s32Ret;
}

static HI_S32 RTSPSVR_VideoSource_Add(HI_RTSP_STREAM_SESSION_S* pSess,
                                      HI_Track_Source_Handle pVideoStream)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pVideoStream);

    //get stream node, and  start num
    s32Ret = RTSPSVR_AddCount(pSess, pVideoStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session video RTSPSVR_AddCount failed Ret: %d\n", s32Ret);
        return s32Ret;
    }

    if (1 == u32Count) //need to start venc
    {
        s32Ret =  pVideoStream->pfnSourceStart(pVideoStream, pSess->pRtspSvrHandle);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session videostream start failed Ret: %d\n", s32Ret);
            goto ERROR;
        }
    }
    else if (u32Count > 1)
    {
        s32Ret =  pVideoStream->pfnRequestKeyFrame(pVideoStream, pSess->pRtspSvrHandle);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session RequestKeyFrame failed Ret: %d\n", s32Ret);
            goto ERROR;
        }
    }

    return HI_SUCCESS;
ERROR:
    //reduce count
    s32Ret = RTSPSVR_ReduceCount(pSess, pVideoStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session video RTSPSVR_ReduceCount failed Ret: %d\n", s32Ret);
    }

    return HI_FAILURE;
}

static HI_S32 RTSPSVR_AudioSource_Remove(HI_RTSP_STREAM_SESSION_S* pSess, HI_Track_Source_Handle pAudioStream)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pAudioStream);
    //get stream node, and cal start num
    s32Ret = RTSPSVR_ReduceCount(pSess, pAudioStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio RTSPSVR_ReduceCount failed Ret: %d\n", s32Ret);
        return s32Ret;
    }

    if (0 == u32Count)
    {
        s32Ret = pAudioStream->pfnSourceStop(pAudioStream, pSess->pRtspSvrHandle);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio stream stop failed Ret: %d\n", s32Ret);
        }
    }
    return s32Ret;
}

static HI_S32 RTSPSVR_AudioSource_Add(HI_RTSP_STREAM_SESSION_S* pSess, HI_Track_Source_Handle pAudioStream)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pAudioStream);
    //get stream node, and cal start num
    s32Ret = RTSPSVR_AddCount(pSess, pAudioStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio RTSPSVR_AddCount failed Ret: %d\n", s32Ret);
        return s32Ret;
    }

    if (1 == u32Count)
    {
        s32Ret = pAudioStream->pfnSourceStart(pAudioStream, pSess->pRtspSvrHandle);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audiostream start failed Ret: %d\n", s32Ret);
            goto ERROR;
        }
    }

    return HI_SUCCESS;
ERROR:
    //reduce count
    s32Ret = RTSPSVR_ReduceCount(pSess, pAudioStream, &u32Count);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio RTSPSVR_ReduceCount failed Ret: %d\n", s32Ret);
    }

    return HI_FAILURE;
}

static HI_VOID RTSPSVR_Handle_Options(HI_RTSP_STREAM_SESSION_S* pSess)
{
    if (HI_FAILURE ==  snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                                  "%s %d %s\r\n"
                                  "CSeq: %d\r\n"
                                  "Cache-Control: no-cache\r\n"
                                  "Server: %s\r\n"
                                  "%sPublic: %s\r\n\r\n",
                                  RTSP_VER_STR,
                                  HIRTSP_STATUS_OK,
                                  HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
                                  pSess->s32CurReqSeq,
                                  RTSP_SERVER_DESCRIPTION,
                                  HI_NETWORK_DateHeader(),
                                  RTSP_SUPPORTED_CMD_LIST))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf pRespBuffer error\n");
        return;
    }
    return;
}



static HI_S32 RTSPSVR_GetStream(HI_RTSP_STREAM_SESSION_S* pSess,
                                const HI_CHAR* pRequest)
{
    HI_S32 s32Ret = 0;
    List_Head_S* pstPosNode = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    HI_BOOL bFound = HI_FALSE;
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;

    //get stream name
    HI_CHAR aszStream[RTSP_MAX_STREAMNAME_LEN] = {0};

    s32Ret = HI_RTSPSVR_MSGParser_GetStreamName(pRequest, aszStream, RTSP_MAX_STREAMNAME_LEN);
    if (s32Ret  != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp url, could not find  streamName\n");
        return HI_FAILURE;
    }

    //get stream node
    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)pSess->pRtspSvrHandle;

    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if (!strcasecmp(pstStreamNode->name, aszStream))
        {
            bFound = HI_TRUE;
            break;
        }
    }

    if (!bFound || NULL == pstStreamNode)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp request, could not find correspond stream\n");
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_FAILURE;
    }

    //set stream
    pSess->u32BufSize = pstStreamNode->u32BufSize;
    HI_RTSPSVR_StreamSession_SetMediaSource(pSess, pstStreamNode->pstVidStream, pstStreamNode->pstAudStream, aszStream);

    RTSP_UNLOCK(pstServerCtx->streamListLock);
    return HI_SUCCESS;
}
/*

DESCRIBE rtsp://ip:554/12? RTSP/1.0

CSeq: 2

Accept: application/sdp

User-Agent: Hisilicon Streaming Media Client/1.0.0(May 23 2016)

///////////////////////////////////////////////////////////////////

RTSP/1.0 200 OK

Server: Hisilicon Streaming Media Server/1.0.0(Jul 30 2015)

Cseq: 2

Content-Type: application/sdp

Content-Length: 403

Date: Sat, 08 Apr 2000 22:04:47 STD

Content-Base: rtsp://ip:554/12?/



v=0

o=StreamingServer 3434249851 1110182534000 IN IP4 ip

c=IN IP4 ip

m=video 0 RTP/AVP 96

a=rtpmap:96 H264/90000

a=control:trackID=3

a=fmtp:96 packetization-mode=1;profile-level-id=4D400C;sprop-parameter-sets=J01ADKkYYELxCA==,KM4JiA==

m=audio 0 RTP/AVP 97

a=rtpmap:97 mpeg4-generic/8000/2

a=control:trackID=4

a=fmtp:97 profile-level-id=15;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=1590

*/

static HI_S32 RTSPSVR_Handle_Describe(HI_RTSP_STREAM_SESSION_S* pSess, const HI_CHAR* pRequest)
{
    HI_S32 s32Ret = 0;
    HI_CHAR aszSdpMsg[RTSP_MAX_SDP_LEN] = {0};
    HI_S32 s32RespLen = 0;
    HI_Track_Source_Handle pVideoStream = NULL;
    HI_Track_Source_Handle pAudioStream = NULL;

    //if not get streamname yet
    if (NULL == pSess->pstVidStream && NULL == pSess->pstAudStream)
    {
        s32Ret = RTSPSVR_GetStream(pSess, pRequest);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp Handle_Describe GetStream error\n");
            RTSPSVR_Handle_BadReq(pSess);
            return HI_FAILURE;
        }
    }

    pVideoStream = pSess->pstVidStream;
    pAudioStream = pSess->pstAudStream;

    if (strcasestr(pRequest, RTSP_HEADER_ACCEPT) != NULL
        && strcasestr(pRequest, RTSP_SDP_CONTENT_TYPE) == NULL)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "describe handle Only accept %s.\n", RTSP_SDP_CONTENT_TYPE);
        RTSPSVR_Handle_OptionUnsupport(pSess);
        return HI_FAILURE;
    }

    if (!pVideoStream && !pAudioStream)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session vid &aud  source null, not exist \n");
        RTSPSVR_Handle_StreamNotFound(pSess);
        return HI_FAILURE;
    }

    if (HI_NULL == pSess->stMediaInfo.hMbufHandle)
    {
        s32Ret = HI_NETWORK_MBufferCreate(&(pSess->stMediaInfo.stBufInfo), &pSess->stMediaInfo.hMbufHandle, pSess->u32BufSize, pSess->s32MaxPayload);
        if (s32Ret != HI_SUCCESS)
        {
            RTSPSVR_Handle_ServerError(pSess);
            return s32Ret;
        }
    }

    if (pVideoStream && !pSess->stMediaInfo.bVideoEnable)
    {
        if (HI_SUCCESS != memcpy_s(&pSess->stMediaInfo.stVideoInfo, sizeof(HI_Track_VideoSourceInfo_S), &pVideoStream->unTrackSourceAttr.stVideoInfo,
                                   sizeof(HI_Track_VideoSourceInfo_S)))
        {
            RTSPSVR_Handle_ServerError(pSess);
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy stVideoInfo error\n");
            s32Ret = HI_FAILURE;
            goto MBUF_DESTROY;
        }
        s32Ret = HI_NETWORK_MBufferRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stVideoInfo.enCodecType);
        if (s32Ret != HI_SUCCESS)
        {
            RTSPSVR_Handle_ServerError(pSess);
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_NETWORK_MBufferRegister video error\n");
            goto MBUF_DESTROY;
        }
        s32Ret = RTSPSVR_VideoSource_Add(pSess, pVideoStream);
        if (s32Ret != HI_SUCCESS)
        {
            RTSPSVR_Handle_ServerError(pSess);
            goto VIDEO_UNREG;
        }
        pSess->stMediaInfo.bVideoEnable = HI_TRUE;
    }

    if (pAudioStream && !pSess->stMediaInfo.bAudioEnable)
    {
        if (HI_SUCCESS != memcpy_s(&pSess->stMediaInfo.stAudioInfo, sizeof(HI_Track_AudioSourceInfo_S), &pAudioStream->unTrackSourceAttr.stAudioInfo,
                                   sizeof(HI_Track_AudioSourceInfo_S)))
        {
            RTSPSVR_Handle_ServerError(pSess);
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "COPY stAudioInfo error\n");
            s32Ret = HI_FAILURE;
            goto VIDEO_REMOVE;
        }
        s32Ret = HI_NETWORK_MBufferRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stAudioInfo.enCodecType);
        if (s32Ret != HI_SUCCESS)
        {
            RTSPSVR_Handle_ServerError(pSess);
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_NETWORK_MBufferRegister audio error\n");
            goto VIDEO_REMOVE;
        }
        s32Ret = RTSPSVR_AudioSource_Add(pSess, pAudioStream);
        if (s32Ret != HI_SUCCESS)
        {
            RTSPSVR_Handle_ServerError(pSess);
            goto AUDIO_UNREG;
        }
        pSess->stMediaInfo.bAudioEnable = HI_TRUE;
    }

    s32Ret = RTSPSVR_Generate_SDPLines(pSess, aszSdpMsg, RTSP_MAX_SDP_LEN);
    s32RespLen = strlen(aszSdpMsg);
    if (s32Ret != HI_SUCCESS || s32RespLen <= 0)
    {
        RTSPSVR_Handle_ServerError(pSess);
        goto AUDIO_REMOVE;
    }

    if (HI_FAILURE ==  snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                                  "%s %d %s\r\n"
                                  "CSeq: %d\r\n"
                                  "Content-Type: application/sdp\r\n"
                                  "Cache-Control: no-cache\r\n"
                                  "Server: %s\r\n"
                                  "Content-Length: %d\r\n"
                                  "%s\r\n"
                                  "%s",
                                  RTSP_VER_STR,
                                  HIRTSP_STATUS_OK,
                                  HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
                                  pSess->s32CurReqSeq,
                                  RTSP_SERVER_DESCRIPTION,
                                  s32RespLen,
                                  HI_NETWORK_DateHeader(),
                                  aszSdpMsg
                                 ))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf aszResponseBuff error\n");
        s32Ret = HI_FAILURE;
        goto AUDIO_REMOVE;
    }
    return HI_SUCCESS;

AUDIO_REMOVE:
    if (pSess->stMediaInfo.bAudioEnable && pAudioStream)
    {
        if (HI_SUCCESS != RTSPSVR_AudioSource_Remove(pSess, pAudioStream))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_AudioSource_Remove fail \n");
        }
    }
AUDIO_UNREG:
    if (pSess->stMediaInfo.bAudioEnable && pAudioStream)
    {
        HI_NETWORK_MBufferUnRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stAudioInfo.enCodecType);
        pSess->stMediaInfo.bAudioEnable = HI_FALSE;
    }
VIDEO_REMOVE:
    if (pSess->stMediaInfo.bVideoEnable && pVideoStream)
    {
        if (HI_SUCCESS != RTSPSVR_VideoSource_Remove(pSess, pVideoStream))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_VideoSource_Remove fail \n");
        }

    }
VIDEO_UNREG:
    if (pSess->stMediaInfo.bVideoEnable && pVideoStream)
    {
        HI_NETWORK_MBufferUnRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stVideoInfo.enCodecType);
        pSess->stMediaInfo.bVideoEnable = HI_FALSE;
    }
MBUF_DESTROY:
    if (HI_NULL != pSess->stMediaInfo.hMbufHandle)
    {
        if (HI_SUCCESS != HI_NETWORK_MBufferDestroy(&(pSess->stMediaInfo.stBufInfo), pSess->stMediaInfo.hMbufHandle))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_NETWORK_MBufferDestroy fail \n");
        }
        pSess->stMediaInfo.hMbufHandle = HI_NULL;
    }
    return s32Ret;
}

/*
SETUP rtsp://ip/sample_h264_100kbit.mp4/trackID=4 RTSP/1.0

CSeq: 3

Transport: RTP/AVP;unicast;client_port=a-b

==================================================

RTSP/1.0 200 OK

Server: DSS/5.5.5 (Build/489.16; Platform/Win32; Release/Darwin; state/beta; )

Cseq: 3

Session: x

Transport: RTP/AVP;unicast;source=ip;client_port=c-d;server_port=a-b;ssrc=0000711E

*/
static HI_S32 RTSPSVR_Handle_Setup(HI_RTSP_STREAM_SESSION_S* pSess, const HI_CHAR* pRequest)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszUrl[RTSP_URL_MAX_LEN] = {0};
    HI_S32 s32Cnt = 0;
    HI_CHAR* pTmpPtr = NULL;
    HI_S32 s32TrackId  = RTSP_INVALID_TRACK_ID;
    HI_CHAR aszRespMsg[RTSP_MAX_SDP_LEN] = {0};
    HI_RTP_SESSION_S* pRtpSess = NULL;
    HI_RTSP_MEDIA_SESSION_S* pRTSPMediaSess = NULL;
    HI_S32 s32UrlLen = 0;

    /*get the url*/
    s32Cnt = sscanf_s(pRequest, "SETUP %255s", aszUrl, RTSP_URL_MAX_LEN);
    if (s32Cnt != RTSP_SCANF_RET_ONE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req format error\n");
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;
    }
    aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';
    s32UrlLen = strlen(aszUrl) ;
    if (s32UrlLen > 0 && s32UrlLen < RTSP_URL_MAX_LEN)
    {
        if (HI_SUCCESS != strncpy_s(pSess->aszUrl, RTSP_MAX_STREAMNAME_LEN, aszUrl, RTSP_MAX_STREAMNAME_LEN - 1))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszUrl fail\n");
            RTSPSVR_Handle_BadReq(pSess);
            return HI_FAILURE;
        }
        pSess->aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';
    }

    pTmpPtr = strcasestr(aszUrl, RTSP_TRACK_ID);
    if (!pTmpPtr)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "there no trackID field in setup url\n");
        RTSPSVR_Handle_BadReq(pSess);
        return HI_FAILURE;
    }

    /*get the track id 0-video 1-audio*/
    pTmpPtr += strlen(RTSP_TRACK_ID);
    s32TrackId = atoi(pTmpPtr);
    /*malloc of rtpsession and rtcpsession when request video or audio */
    if (RTSP_TRACKID_VIDEO == s32TrackId)
    {
        /*do not setup again*/
        if (pSess->stVideoSession.s32TrackId != RTSP_INVALID_TRACK_ID)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "setup video request repeat \n");
            RTSPSVR_Handle_BadReq(pSess);
            return HI_FAILURE;
        }

        /*judge whether need to register mbuff and start video venc*/
        if (!pSess->stMediaInfo.bVideoEnable)
        {
            s32Ret = HI_NETWORK_MBufferRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stVideoInfo.enCodecType);

            if (s32Ret != HI_SUCCESS)
            {
                RTSPSVR_Handle_ServerError(pSess);
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_NETWORK_MBufferRegister video error\n");
                return HI_FAILURE;
            }

            s32Ret = RTSPSVR_VideoSource_Add(pSess, pSess->pstVidStream);

            if (s32Ret != HI_SUCCESS)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "add audio stream fail \n");
                RTSPSVR_Handle_BadReq(pSess);
                return HI_FAILURE;
            }

            pSess->stMediaInfo.bVideoEnable = HI_TRUE;
        }

        pSess->stVideoSession.s32TrackId = RTSP_TRACKID_VIDEO;
        /*init the video rtcp session and rtp session when connect */
        s32Ret = HI_RTP_Session_Create(&pSess->stVideoSession.pRtpSession, pSess->s32PacketLen);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session video create rtpsession error\n");
            RTSPSVR_Handle_ServerError(pSess);
            return HI_FAILURE;
        }
        s32Ret = HI_RTCP_Session_Create(&pSess->stVideoSession.pRtcpSession, pSess->s32PacketLen);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session video create rtcpsession error\n");
            RTSPSVR_Handle_ServerError(pSess);
            goto RELEASE_VID_RTP;
        }
        pRtpSess = pSess->stVideoSession.pRtpSession;
        pRTSPMediaSess = &pSess->stVideoSession;

    }
    else if (RTSP_TRACKID_AUDIO == s32TrackId)
    {
        /*do not setup again */
        if (pSess->stAudioSession.s32TrackId != RTSP_INVALID_TRACK_ID)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "setup audio request repeat \n");
            RTSPSVR_Handle_BadReq(pSess);
            goto RELEASE_VID_RTCP;
        }

        /*judge whether need to register mbuff and start audio venc*/
        if (!pSess->stMediaInfo.bAudioEnable)
        {
            s32Ret = HI_NETWORK_MBufferRegister(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stAudioInfo.enCodecType);

            if (s32Ret != HI_SUCCESS)
            {
                RTSPSVR_Handle_ServerError(pSess);
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_NETWORK_MBufferRegister audio error\n");
                goto RELEASE_VID_RTCP;
            }

            s32Ret = RTSPSVR_AudioSource_Add(pSess, pSess->pstAudStream);

            if (s32Ret != HI_SUCCESS)
            {
                RTSPSVR_Handle_ServerError(pSess);
                goto RELEASE_VID_RTCP;
            }

            pSess->stMediaInfo.bAudioEnable = HI_TRUE;
        }

        pSess->stAudioSession.s32TrackId = RTSP_TRACKID_AUDIO;
        /*init the audio rtcp session and rtp session when connect */
        s32Ret = HI_RTP_Session_Create(&pSess->stAudioSession.pRtpSession, pSess->s32PacketLen);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio create rtcpsession error\n");
            RTSPSVR_Handle_ServerError(pSess);
            goto RELEASE_VID_RTCP;
        }
        s32Ret = HI_RTCP_Session_Create(&pSess->stAudioSession.pRtcpSession, pSess->s32PacketLen);
        if (s32Ret != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "session audio create rtcpsession error\n");
            RTSPSVR_Handle_ServerError(pSess);
            goto RELEASE_AUD_RTP;
        }
        pRtpSess = pSess->stAudioSession.pRtpSession;
        pRTSPMediaSess = &pSess->stAudioSession;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "setup video request unsupport trackid: %d\n", s32TrackId);
        RTSPSVR_Handle_StreamNotFound(pSess);
        return HI_FAILURE;
    }

    /*parse the transformat of rtp packet */
    s32Ret = RTSPSVR_ParseTransports(pSess, pRequest, pRTSPMediaSess);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req format error\n");
        RTSPSVR_Handle_UnsupportedTransport(pSess);
        goto RELEASE_SOURCE;
    }

    /*get the send port of server*/
    RTSPSVR_GetMediaSendPort(pRTSPMediaSess);

    /*get the random ssrc*/
    HI_NETWORK_RandomNum(&pRtpSess->u32Ssrc);

    s32Ret = snprintf_s(pRtpSess->aszClientIP, RTP_TRANS_IP_MAX_LEN, RTP_TRANS_IP_MAX_LEN - 1, "%s", pSess->aszClientIP);
    if (s32Ret == HI_FAILURE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf aszClientIP error\n");
        RTSPSVR_Handle_BadReq(pSess);
        goto RELEASE_SOURCE;
    }

    s32Ret = RTSPSVR_Generate_SetupResponse(pSess, pRTSPMediaSess, aszRespMsg, RTSP_MAX_PROTOCOL_BUFFER);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp GenerateSetupResponse error\n");
        RTSPSVR_Handle_BadReq(pSess);
        goto RELEASE_SOURCE;
    }

    if (pSess->s32Timeout > 0)
    {
        s32Ret = snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                            "%s %d %s\r\n"
                            "CSeq: %d\r\n"
                            "Session: %s;timeout=%d\r\n"
                            "Server: %s\r\n"
                            "%s"
                            "%s\r\n",
                            RTSP_VER_STR,
                            HIRTSP_STATUS_OK,
                            HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
                            pSess->s32CurReqSeq,
                            pSess->aszSessID,
                            pSess->s32Timeout,
                            RTSP_SERVER_DESCRIPTION,
                            HI_NETWORK_DateHeader(),
                            aszRespMsg);
    }
    else
    {
        s32Ret = snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                            "%s %d %s\r\n"
                            "CSeq: %d\r\n"
                            "Session: %s\r\n"
                            "Server: %s\r\n"
                            "%s"
                            "%s\r\n",
                            RTSP_VER_STR,
                            HIRTSP_STATUS_OK,
                            HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
                            pSess->s32CurReqSeq,
                            pSess->aszSessID,
                            RTSP_SERVER_DESCRIPTION,
                            HI_NETWORK_DateHeader(),
                            aszRespMsg);
    }

    if (s32Ret == HI_FAILURE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string printf aszClientIP error\n");
        RTSPSVR_Handle_BadReq(pSess);
        goto RELEASE_SOURCE;
    }

    if ((HIRTSP_SESSION_STATE_INIT != pSess->enSessState) && (HIRTSP_SESSION_STATE_READY != pSess->enSessState) && (HIRTSP_SESSION_STATE_PLAY != pSess->enSessState))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp server session state error\n");
        RTSPSVR_Handle_BadReq(pSess);
        s32Ret = HI_FAILURE;
        goto RELEASE_SOURCE;
    }

    pRtpSess->enSessState = HIRTP_SESSION_STATE_READY;//rtp state

    return HI_SUCCESS;

RELEASE_SOURCE:
    if (RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId)
    {
        HI_RTCP_Session_Destroy(pSess->stAudioSession.pRtcpSession);
        pSess->stAudioSession.pRtcpSession = NULL;
    }
RELEASE_AUD_RTP:
    if (RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId)
    {
        pSess->stAudioSession.s32TrackId = RTSP_INVALID_TRACK_ID;
        HI_RTP_Session_Destroy(pSess->stAudioSession.pRtpSession);
        pSess->stAudioSession.pRtpSession = NULL;
    }
RELEASE_VID_RTCP:
    if (RTSP_INVALID_TRACK_ID !=  pSess->stVideoSession.s32TrackId)
    {
        HI_RTCP_Session_Destroy(pSess->stVideoSession.pRtcpSession);
        pSess->stVideoSession.pRtcpSession = NULL;
    }
RELEASE_VID_RTP:
    if (RTSP_INVALID_TRACK_ID !=  pSess->stVideoSession.s32TrackId)
    {
        pSess->stVideoSession.s32TrackId = RTSP_INVALID_TRACK_ID;
        HI_RTP_Session_Destroy(pSess->stVideoSession.pRtpSession);
        pSess->stVideoSession.pRtpSession = NULL;
    }

    return s32Ret;
}


/*
PLAY rtsp://ip/sample_h264_100kbit.mp4 RTSP/1.0

CSeq: 7

Range: npt=0.000000-70.000000

Session: x

======================================================

RTSP/1.0 200 OK

Server: Hisilicon Streaming Media Server/1.0.0(Jul 30 2015)

Cseq: 7

Session: x

Range: npt=0.00000-70.00000

RTP-Info: url=rtsp://ip:554/12?/trackID=0;seq=379;rtptime=2384477337,
*/
static HI_S32 RTSPSVR_Handle_Play(HI_RTSP_STREAM_SESSION_S* pSess, const  HI_CHAR* pRequest)
{

    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszRespMsg[RTSP_MAX_SDP_LEN] = {0};
    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pRequest);

    if ( (RTSP_INVALID_TRACK_ID == pSess->stVideoSession.s32TrackId) && (RTSP_INVALID_TRACK_ID == pSess->stAudioSession.s32TrackId))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "nothing to play \n");
        RTSPSVR_Handle_StreamNotFound(pSess);
        return HI_FAILURE;
    }

    if (((  RTSP_INVALID_TRACK_ID != pSess->stVideoSession.s32TrackId) &&
         (HIRTP_SESSION_STATE_READY != pSess->stVideoSession.pRtpSession->enSessState))
        || ((  RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId) &&
            (HIRTP_SESSION_STATE_READY != pSess->stAudioSession.pRtpSession->enSessState))
       )
    {
        if (((  RTSP_INVALID_TRACK_ID != pSess->stVideoSession.s32TrackId) &&
             (HIRTP_SESSION_STATE_PLAY == pSess->stVideoSession.pRtpSession->enSessState))
            || ((  RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId) &&
                (HIRTP_SESSION_STATE_PLAY == pSess->stAudioSession.pRtpSession->enSessState))
           )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "One track already is in playing state!!\r\n");
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "the media is not in ready state\n");
            return HI_FAILURE;
        }

    }

    /*start trans task for media*/
    if (RTSP_INVALID_TRACK_ID != pSess->stVideoSession.s32TrackId)
    {
        if (HIRTP_SESSION_STATE_PLAY != pSess->stVideoSession.pRtpSession->enSessState)
        {
            /* start video trans task*/
            s32Ret = HI_RTP_Session_Start(pSess->stVideoSession.pRtpSession);

            if (s32Ret != HI_SUCCESS)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_RTP_Session_Start video error\n");
                return HI_FAILURE;
            }

            if (RTP_TRANS_UDP == pSess->stVideoSession.pRtpSession->enMediaTransMode)
            {
                s32Ret = HI_RTCP_Session_StartUdp(pSess->stVideoSession.pRtcpSession);

                if (s32Ret != HI_SUCCESS)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_RTCP_Session_StartUdp video error\n");
                    goto VIDEO_RTP_STOP;
                }
            }

            HI_NETWORK_MBufferGetPts(pSess->stMediaInfo.hMbufHandle, pSess->stMediaInfo.stVideoInfo.enCodecType, &pSess->stMediaInfo.u64VidStartPts);
        }
    }

    if (RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId)
    {
        if (HIRTP_SESSION_STATE_PLAY != pSess->stAudioSession.pRtpSession->enSessState)
        {
            /* start audio trans task*/
            s32Ret = HI_RTP_Session_Start(pSess->stAudioSession.pRtpSession);

            if (s32Ret != HI_SUCCESS)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_RTP_Session_Start audio error\n");
                goto VIDEO_RTCP_STOP;
            }

            if (RTP_TRANS_UDP == pSess->stAudioSession.pRtpSession->enMediaTransMode)
            {
                s32Ret = HI_RTCP_Session_StartUdp(pSess->stAudioSession.pRtcpSession);

                if (s32Ret != HI_SUCCESS)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp HI_RTCP_Session_StartUdp audio error\n");
                    goto AUDIO_RTP_STOP;
                }
            }

            HI_NETWORK_MBufferGetPts(pSess->stMediaInfo.hMbufHandle , pSess->stMediaInfo.stAudioInfo.enCodecType, &pSess->stMediaInfo.u64AudStartPts);
        }
    }

    s32Ret = RTSPSVR_Generate_PlayResponse(pSess, aszRespMsg, RTSP_MAX_PROTOCOL_BUFFER);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp GeneratePlayResponse error\n");
        RTSPSVR_Handle_BadReq(pSess);
        goto AUDIO_RTCP_STOP;
    }

    s32Ret = snprintf_s(pSess->aszResponseBuff, sizeof(pSess->aszResponseBuff), sizeof(pSess->aszResponseBuff) - 1,
                        "%s %d %s\r\n"
                        "Server: %s\r\n"
                        "CSeq: %d\r\n"
                        "Session: %s\r\n"
                        "Date: %s"
                        "%s\r\n",
                        RTSP_VER_STR,
                        HIRTSP_STATUS_OK,
                        HI_RTSPSVR_MSGParser_StatusCode2Str(HIRTSP_STATUS_OK),
                        RTSP_SERVER_DESCRIPTION,
                        pSess->s32CurReqSeq,
                        pSess->aszSessID,
                        HI_NETWORK_DateHeader(),
                        aszRespMsg);
    if (s32Ret == HI_FAILURE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string print aszResponseBuff error\n");
        RTSPSVR_Handle_BadReq(pSess);
        goto AUDIO_RTCP_STOP;
    }

    if ((HIRTSP_SESSION_STATE_READY != pSess->enSessState) && (HIRTSP_SESSION_STATE_PLAY != pSess->enSessState))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp server state  error\n");
        RTSPSVR_Handle_ServerError(pSess);
        s32Ret = HI_FAILURE;
        goto AUDIO_RTCP_STOP;
    }

    return HI_SUCCESS;

AUDIO_RTCP_STOP:
    if ( RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId)
    {
        if (RTP_TRANS_UDP == pSess->stAudioSession.pRtpSession->enMediaTransMode)
        {
            HI_RTCP_Session_StopUdp(pSess->stAudioSession.pRtcpSession);
        }
    }
AUDIO_RTP_STOP:
    if ( RTSP_INVALID_TRACK_ID != pSess->stAudioSession.s32TrackId)
    {
        HI_RTP_Session_Stop(pSess->stAudioSession.pRtpSession);
    }

VIDEO_RTCP_STOP:
    if ( RTSP_INVALID_TRACK_ID != pSess->stVideoSession.s32TrackId)
    {
        if (RTP_TRANS_UDP == pSess->stVideoSession.pRtpSession->enMediaTransMode)
        {
            HI_RTCP_Session_StopUdp(pSess->stVideoSession.pRtcpSession);
        }
    }
VIDEO_RTP_STOP:
    if ( RTSP_INVALID_TRACK_ID != pSess->stVideoSession.s32TrackId)
    {
        HI_RTP_Session_Stop(pSess->stVideoSession.pRtpSession);
    }

    return s32Ret;
}

/*
TEARDOWN rtsp://ip/sample_100kbit.mp4/ RTSP/1.0

Session: x

User-Agent: RealMedia Player/mc.30.26.01 (s60; epoc_av30_armv5)

CSeq: 7

===============================================================

RTSP/1.0 200 OK

Server: DSS/5.5.5 (Build/489.16; Platform/Win32; Release/Darwin; state/beta; )

Cseq: 7

Session: x

Connection: Close
*/
/**
 * @brief teardown video track.
 * @param[in] pstSession : session info,
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_VideoTrack_Teardown(HI_RTSP_STREAM_SESSION_S* pstSession)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == pstSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return HI_FAILURE;
    }

    if (RTSP_INVALID_TRACK_ID == pstSession->stVideoSession.s32TrackId)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Video track is NULL, not need to teardown again\n");
        return HI_SUCCESS;
    }

    if (pstSession->stMediaInfo.bVideoEnable)
    {
        pstSession->stMediaInfo.bVideoEnable = HI_FALSE;
        s32Ret = RTSPSVR_VideoSource_Remove(pstSession, pstSession->pstVidStream);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " RTSPSVR_VideoSource_Remove  fail %X \n", s32Ret);
        }

        HI_NETWORK_MBufferUnRegister(pstSession->stMediaInfo.hMbufHandle, pstSession->stMediaInfo.stVideoInfo.enCodecType);

        if (pstSession->stVideoSession.s32TrackId != RTSP_INVALID_TRACK_ID )
        {
            HI_RTP_Session_Stop(pstSession->stVideoSession.pRtpSession);

            if (RTP_TRANS_UDP == pstSession->stVideoSession.pRtpSession->enMediaTransMode)
            {
                HI_RTCP_Session_StopUdp(pstSession->stVideoSession.pRtcpSession);
            }

            pstSession->stVideoSession.s32TrackId = RTSP_INVALID_TRACK_ID;
            HI_RTP_Session_Destroy(pstSession->stVideoSession.pRtpSession);
            pstSession->stVideoSession.pRtpSession = NULL;

            HI_RTCP_Session_Destroy(pstSession->stVideoSession.pRtcpSession);
            pstSession->stVideoSession.pRtcpSession = NULL;
        }

    }

    if (!pstSession->stMediaInfo.bAudioEnable)
    {
        (HI_VOID)pthread_mutex_lock(&pstSession->mStateLock);
        pstSession->enSessState = HIRTSP_SESSION_STATE_STOP;
        (HI_VOID)pthread_mutex_unlock(&pstSession->mStateLock);
    }

    return HI_SUCCESS;
}

/**
 * @brief teardown audio track.
 * @param[in] pstSession : session info,
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_AudioTrack_Teardown(HI_RTSP_STREAM_SESSION_S* pstSession)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == pstSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return HI_FAILURE;
    }

    if (RTSP_INVALID_TRACK_ID == pstSession->stAudioSession.s32TrackId)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Audio track is NULL, not need to teardown again\n");
        return HI_SUCCESS;
    }

    if (pstSession->stMediaInfo.bAudioEnable)
    {
        pstSession->stMediaInfo.bAudioEnable = HI_FALSE;
        s32Ret = RTSPSVR_AudioSource_Remove(pstSession, pstSession->pstAudStream);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " RTSPSVR_AudioSource_Remove  fail %X \n", s32Ret);
        }

        HI_NETWORK_MBufferUnRegister(pstSession->stMediaInfo.hMbufHandle, pstSession->stMediaInfo.stAudioInfo.enCodecType);

        if (pstSession->stAudioSession.s32TrackId != RTSP_INVALID_TRACK_ID )
        {
            HI_RTP_Session_Stop(pstSession->stAudioSession.pRtpSession);

            if (RTP_TRANS_UDP == pstSession->stAudioSession.pRtpSession->enMediaTransMode)
            {
                HI_RTCP_Session_StopUdp(pstSession->stAudioSession.pRtcpSession);
            }

            pstSession->stAudioSession.s32TrackId = RTSP_INVALID_TRACK_ID;
            HI_RTP_Session_Destroy(pstSession->stAudioSession.pRtpSession);
            pstSession->stAudioSession.pRtpSession = NULL;

            HI_RTCP_Session_Destroy(pstSession->stAudioSession.pRtcpSession);
            pstSession->stAudioSession.pRtcpSession = NULL;
        }
    }

    if (!pstSession->stMediaInfo.bVideoEnable)
    {
        (HI_VOID)pthread_mutex_lock(&pstSession->mStateLock);
        pstSession->enSessState = HIRTSP_SESSION_STATE_STOP;
        (HI_VOID)pthread_mutex_unlock(&pstSession->mStateLock);
    }

    return HI_SUCCESS;
}

/**
 * @brief handle teardown request.
 * @param[in] pstSession : session info, pcRequest : msg from client, u32ReqLen: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_Handle_Teardown(HI_RTSP_STREAM_SESSION_S* pstSession, const HI_CHAR* pszRequest)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszUrl[RTSP_URL_MAX_LEN] = {0};
    HI_S32 s32Cnt = 0;
    HI_CHAR aszSessID[RTSP_SESSID_MAX_LEN] = {0};
    HI_S32 s32UrlLen = 0;
    HI_CHAR* pTmpPtr = NULL;
    HI_S32 s32TrackId = RTSP_INVALID_TRACK_ID;

    RTSPSVR_CHECK_NULL_ERROR(pstSession);
    if (HI_SUCCESS != memset_s(pstSession->aszResponseBuff, RTSP_MAX_PROTOCOL_BUFFER, 0, RTSP_MAX_PROTOCOL_BUFFER))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set aszResponseBuff error\n");
    }

    s32Ret = HI_RTSPSVR_MSGParser_GetSessId(pszRequest, aszSessID);

    if (HI_SUCCESS != s32Ret)
    {
        /*reply the wrong message and close the socket*/
        if (HI_SUCCESS != strncpy_s(aszSessID, RTSP_SESSID_MAX_LEN, pstSession->aszSessID, RTSP_SESSID_MAX_LEN - 1))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszSessID fail\n");
            RTSPSVR_Handle_BadReq(pstSession);
            return HI_FAILURE;
        }
        aszSessID[RTSP_SESSID_MAX_LEN - 1] = '\0';
        RTSPSVR_Handle_BadReq(pstSession);
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "get teardown session id failed :%d \n", s32Ret);
        return s32Ret;
    }

    /*Get the trackid, judge which track will be tear down*/
    s32Cnt = sscanf_s(pszRequest, "TEARDOWN %255s", aszUrl, RTSP_URL_MAX_LEN);

    if (s32Cnt != RTSP_SCANF_RET_ONE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req format error\n");
        RTSPSVR_Handle_BadReq(pstSession);
        return HI_FAILURE;
    }

    aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';
    s32UrlLen = strlen(aszUrl);

    if ((s32UrlLen > 0) && (s32UrlLen < RTSP_URL_MAX_LEN))
    {
        if (HI_SUCCESS != strncpy_s(pstSession->aszUrl, RTSP_MAX_STREAMNAME_LEN, aszUrl, RTSP_MAX_STREAMNAME_LEN - 1))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszUrl fail\n");
            RTSPSVR_Handle_BadReq(pstSession);
            return HI_FAILURE;
        }
        pstSession->aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';
    }

    pTmpPtr = strcasestr(aszUrl, RTSP_TRACK_ID);

    if (!pTmpPtr)
    {
        s32Ret = HI_RTSPSVR_StreamSession_Stop(pstSession);

        if (HI_SUCCESS != s32Ret)
        {
            RTSPSVR_Handle_BadReq(pstSession);
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "teardown session stop failed :%d \n", s32Ret);
            return s32Ret;
        }

        return HI_SUCCESS;
    }
    else
    {
        /*get the track id 0-video 1-audio*/
        pTmpPtr += strlen(RTSP_TRACK_ID);
        s32TrackId = atoi(pTmpPtr);

        if (RTSP_TRACKID_VIDEO == s32TrackId)
        {
            s32Ret = RTSPSVR_VideoTrack_Teardown(pstSession);

            if (HI_SUCCESS != s32Ret)
            {
                RTSPSVR_Handle_BadReq(pstSession);
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "teardown video track failed :%d \n", s32Ret);
                return s32Ret;
            }
        }
        else if (RTSP_TRACKID_AUDIO == s32TrackId)
        {
            s32Ret = RTSPSVR_AudioTrack_Teardown(pstSession);

            if (HI_SUCCESS != s32Ret)
            {
                RTSPSVR_Handle_BadReq(pstSession);
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "teardown audio track failed :%d \n", s32Ret);
                return s32Ret;
            }
        }
        else
        {
            /*no need to return error!!*/
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "teardown request unsupport trackid: %d\n", s32TrackId);
            RTSPSVR_Handle_BadReq(pstSession);
            return HI_SUCCESS;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_SendFrame(HI_RTP_SESSION_S* pRtpSession, RTP_PACK_TYPE_E enPackType, HI_U8* pu8Frame,
                                HI_U32 u32FrameLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
                                HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr, HI_S32 s32PacketLen)
{
    HI_U32 u32SeqNum = *pu32SeqNum;
    HI_S32 s32Ret = HI_SUCCESS;

    /*rtp on udp  pack in fua send in udp */
    if (PACK_TYPE_RTP_FUA == enPackType) /*RTP/UDP*/
    {
        s32Ret = HI_RTP_Session_SendDataInRtp(pRtpSession, pu8Frame, u32FrameLen, u32TimeStamp,
                                              PackageType, &u32SeqNum, u32Ssrc, WriteSock, pPeerSockAddr, s32PacketLen);

    }
    /*rtp on tcp pack in PACK_TYPE_RTSP_ITLV send in tcp*/
    else if (PACK_TYPE_RTSP_ITLV == enPackType || PACK_TYPE_RTSP_O_HTTP == enPackType) /*RTP/TCP*/
    {

        s32Ret = HI_RTP_Session_SendDataInRtspItlv(pRtpSession, pu8Frame, u32FrameLen, u32TimeStamp,
                 PackageType, &u32SeqNum, u32Ssrc, WriteSock, NULL, s32PacketLen);
    }
    else
    {
        /*rtsp only support tcl_itlv and udp now ;broadcast is not supported*/
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "packtype not support %d.\n", enPackType);
        return HI_FAILURE;
    }

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_SendAvcKeyFrame fail\n");
        return s32Ret;
    }

    *pu32SeqNum  = u32SeqNum;

    return HI_SUCCESS;

}
static HI_S32 RTSPSVR_SendAvcKeyFrame(HI_RTP_SESSION_S* pRtpSession, RTP_PACK_TYPE_E enPackType, HI_U8* pu8Frame,
                                      HI_U32 u32FrameLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
                                      HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr, HI_S32 s32PacketLen)
{
    HI_U32 u32Cursor = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8TmpBuffer = NULL;
    HI_U32 u32BufLen = 0;
    HI_U32 u32SeqNum = *pu32SeqNum;
    //get first I frame and parse sps pps
    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = RTSPSVR_GetAVCNalUnit(&u8NalType, pu8Frame + u32Cursor, u32FrameLen - u32Cursor);
        if (u32NalLen <= 4 || u32Cursor + u32NalLen  > u32FrameLen )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseAvcKeyFrame u32NalLen  illegal: %d \n", u32NalLen);
            return HI_FAILURE;
        }

        pu8TmpBuffer = pu8Frame + u32Cursor; //  + 4,;
        u32BufLen = u32NalLen ;//- 4;


        s32Ret = RTSPSVR_SendFrame(pRtpSession, enPackType, pu8TmpBuffer, u32BufLen, u32TimeStamp, PackageType, &u32SeqNum, u32Ssrc, WriteSock, pPeerSockAddr, s32PacketLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_SendAvcKeyFrame fail\n");
            return s32Ret;
        }


        u32Cursor += u32NalLen;
    }
    while (u32Cursor + 4 < u32FrameLen);

    *pu32SeqNum  = u32SeqNum;

    return HI_SUCCESS;
}


static HI_S32  RTSPSVR_SendHevcKeyFrame(HI_RTP_SESSION_S* pRtpSession, RTP_PACK_TYPE_E enPackType, HI_U8* pu8Frame,
                                        HI_U32 u32FrameLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
                                        HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr, HI_S32 s32PacketLen)
{
    HI_U32 u32Cursor = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8TmpBuffer = NULL;
    HI_U32 u32BufLen = 0;
    HI_U32 u32SeqNum = *pu32SeqNum;

    do
    {
        HI_U8 u8NalType = 0;
        HI_U32 u32NalLen = RTSPSVR_GetHEVCNalUnit(&u8NalType, pu8Frame + u32Cursor, u32FrameLen - u32Cursor);
        if (u32NalLen <= 4 || u32Cursor + u32NalLen  > u32FrameLen)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_ParseHevcKeyFrame u32NalLen  illegal: %d \n", u32NalLen);
            return HI_FAILURE;
        }

        pu8TmpBuffer = pu8Frame + u32Cursor; //  + 4,;
        u32BufLen = u32NalLen ;//- 4;

        s32Ret = RTSPSVR_SendFrame(pRtpSession, enPackType, pu8TmpBuffer, u32BufLen, u32TimeStamp, PackageType, &u32SeqNum, u32Ssrc, WriteSock, pPeerSockAddr, s32PacketLen);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_SendAvcKeyFrame fail\n");
            return s32Ret;
        }

        u32Cursor += u32NalLen;
    }
    while (u32Cursor + 4 < u32FrameLen);

    *pu32SeqNum  = u32SeqNum;

    return HI_SUCCESS;
}


static HI_S32 RTSPSVR_MediaSendingProcess(HI_RTSP_STREAM_SESSION_S* pSess)
{
    HI_S32 s32Ret = 0;
    HI_MW_PTR hBuffHandle  = HI_NULL;
    HI_BOOL bKeyFlag = HI_FALSE;
    HI_U8*  pBuffAddr = NULL;/*pointer to addr of stream ready to send */
    HI_U32      u32DataLen  = 0;  /*len of stream ready to send*/
    HI_U64     u64Pts   = 0;  /*stream pts*/
    HI_U32    u32CalcedPts    = 0;  /*calculated pts,used in FU-A*/
    BUF_DATATYPE_E enBuffPayloadType = BUF_DATATYPE_BUTT;
    HI_S32 s32WriteSock = HI_NETWORK_INVALID_SOCKET;
    HI_BOOL   bDiscardFlag = HI_FALSE;
    HI_U32    u32LastSn = 0;
    RTP_TRANS_MODE_E    enTransMode = RTP_TRANS_BUTT;
    struct sockaddr_in stPeerSockAddr;
    RTP_PACK_TYPE_E enPackType = PACK_TYPE_BUTT;
    RTP_PT_E enRtpPayloadType = RTP_PT_INVALID;
    HI_U32 u32Ssrc = 0;
    HI_RTP_SESSION_STATE_E enVideoState = HIRTP_SESSION_STATE_BUTT;
    HI_RTP_SESSION_STATE_E enAudioState = HIRTP_SESSION_STATE_BUTT;
    HI_RTP_SESSION_S*  pRtpSession = NULL;
    HI_RTP_SESSION_S*  pVideoRtpSession = NULL;
    HI_RTP_SESSION_S*  pAudioRtpSession = NULL;

    if (NULL == pSess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_MediaSendingProcess null param ! \n");
        return HI_FAILURE;
    }

    if (pSess->stVideoSession.s32TrackId != RTSP_INVALID_TRACK_ID)
    {
        pVideoRtpSession = pSess->stVideoSession.pRtpSession;
        enTransMode = pVideoRtpSession->enMediaTransMode;
        enVideoState = pVideoRtpSession->enSessState;
    }

    if (pSess->stAudioSession.s32TrackId != RTSP_INVALID_TRACK_ID)
    {
        pAudioRtpSession = pSess->stAudioSession.pRtpSession;
        enTransMode = pAudioRtpSession->enMediaTransMode;
        enAudioState = pAudioRtpSession->enSessState;
    }

    /* trans mode must be tcp interleaved or udp otherwise, media data is not send by session thread*/
    if (RTP_TRANS_TCP_ITLV != enTransMode && RTP_TRANS_UDP != enTransMode && RTP_TRANS_BROADCAST != enTransMode)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "trans mode must be tcp or udp or broadcast.\n");
        return HI_SUCCESS;
    }

    hBuffHandle = pSess->stMediaInfo.hMbufHandle;

    if ( (HIRTP_SESSION_STATE_PLAY != enVideoState) && (HIRTP_SESSION_STATE_PLAY != enAudioState))
    {
        /*video,audio and data are all not playing,so sleep 1 s*/
        hi_usleep(1500000);
        return HI_SUCCESS;
    }
    else
    {

        /*request data from mediabuff */
        s32Ret = HI_NETWORK_MBufferRead(hBuffHandle, (HI_VOID**)(&pBuffAddr), &u32DataLen, &u64Pts, &enBuffPayloadType, &bKeyFlag);

        /*there is no data ready */
        if ( HI_SUCCESS != s32Ret || 0 == u32DataLen )
        {
            hi_usleep(10000);
            return HI_ERR_RTSPSERVER_ERR_SESS_NO_DATA;
        }
        /*judge the readed data is whether in playing state, if is not, just free this slice and not sending*/
        else
        {
            switch (enBuffPayloadType)
            {
                case BUF_DATATYPE_VIDEO:
                    (HIRTP_SESSION_STATE_PLAY != enVideoState) ?
                    (bDiscardFlag = HI_TRUE) : (bDiscardFlag = HI_FALSE);
                    break;

                case BUF_DATATYPE_AUDIO:
                    (HIRTP_SESSION_STATE_PLAY != enAudioState) ?
                    (bDiscardFlag = HI_TRUE) : (bDiscardFlag = HI_FALSE);
                    break;

                default:
                    bDiscardFlag = HI_TRUE;
                    break;
            }


            /*if this media type is not in playing state, discard the slice*/
            if (HI_TRUE == bDiscardFlag)
            {
                HI_NETWORK_MBufferSet(hBuffHandle);
                return HI_SUCCESS;
            }
        }
    }

    /*calculate the pts of the audio and video,64 bit in venc&aenc ,32 bit in rtp*/
    if (BUF_DATATYPE_VIDEO == enBuffPayloadType)
    {
        u64Pts = u64Pts - pSess->stMediaInfo.u64VidStartPts; /*setup agian after teardown may cause two kinds of time*/
        pRtpSession = pSess->stVideoSession.pRtpSession;
        u64Pts = (u64Pts * RTSP_CLOCK_RATE / RTSP_TIME_SCALE );
        u32CalcedPts = (HI_U32)u64Pts;
        pRtpSession->u32DataLen += u32DataLen;
    }
    else if (BUF_DATATYPE_AUDIO == enBuffPayloadType)
    {
        u64Pts = u64Pts - pSess->stMediaInfo.u64AudStartPts;  /*setup agian after teardown may cause two kinds of time*/
        pRtpSession = pSess->stAudioSession.pRtpSession;
        u64Pts = (u64Pts * RTSP_CLOCK_RATE / RTSP_TIME_SCALE );
        u32CalcedPts = (HI_U32)u64Pts;
        pRtpSession->u32DataLen += u32DataLen;

    }

    /*get the send data length*/
    HI_RTP_Session_GetPacketAndSendParam(pRtpSession, &s32WriteSock,
                                         &stPeerSockAddr, &u32LastSn, &enPackType,  &u32Ssrc);

    /*get rtp payloadtype according to stream's type (mbuff modle naming the stream type)*/
    s32Ret = RTSPSVR_GetRtpPayloadType(pSess, enBuffPayloadType, &enRtpPayloadType);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetRtpPayloadType fail %d.\n", s32Ret);
        return s32Ret;
    }

    //if video i frame,parse iframe into sps pps idr frame and send rtp packet
    if ( bKeyFlag && BUF_DATATYPE_VIDEO == enBuffPayloadType)
    {
        if (RTP_PT_H264 == enRtpPayloadType)
        {
            s32Ret = RTSPSVR_SendAvcKeyFrame(pRtpSession, enPackType, pBuffAddr, u32DataLen, u32CalcedPts,
                                             enRtpPayloadType, &u32LastSn, u32Ssrc, s32WriteSock, &stPeerSockAddr, pSess->s32PacketLen);
        }
        else if (RTP_PT_H265 == enRtpPayloadType)
        {
            s32Ret = RTSPSVR_SendHevcKeyFrame(pRtpSession, enPackType, pBuffAddr, u32DataLen, u32CalcedPts,
                                              enRtpPayloadType, &u32LastSn, u32Ssrc, s32WriteSock, &stPeerSockAddr, pSess->s32PacketLen);
        }
        else
        {
            //not support type
            s32Ret = HI_FAILURE;
        }
    }
    else
    {

        s32Ret = RTSPSVR_SendFrame(pRtpSession, enPackType, pBuffAddr, u32DataLen, u32CalcedPts,
                                   enRtpPayloadType, &u32LastSn, u32Ssrc, s32WriteSock, &stPeerSockAddr, pSess->s32PacketLen);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_SendAvcKeyFrame fail\n");
            return s32Ret;
        }

    }

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver Send Stream Data Fail.\n");
        HI_NETWORK_MBufferSet(hBuffHandle);
        return s32Ret;
    }
    else
    {
        HI_NETWORK_MBufferSet(hBuffHandle);
        HI_RTP_Session_UpdateSeqNum(pRtpSession, u32LastSn);
    }

    return HI_SUCCESS;

}

/**
 * @brief send the data.
 * @param[in] pstSession : session info
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_SendMedia(HI_RTSP_STREAM_SESSION_S* pstSession)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == pstSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return HI_FAILURE;
    }

    if (HIRTSP_SESSION_STATE_PLAY == pstSession->enSessState)
    {
        s32Ret = RTSPSVR_MediaSendingProcess(pstSession);

        if (s32Ret != HI_SUCCESS && s32Ret != HI_ERR_RTSPSERVER_ERR_SESS_NO_DATA)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

/*
rfc2326bis06-Page42 SETUP can be used in all three states; INIT,and READY
InitState:
    Describe: --> init_state
    Setup   : --> *ready_state
    Teardown: --> init_state
    Options : --> init_state
    Play    : --> init_state
    Pause   : --> init_state

ReadyState:
    Play    : --> *play_state
    Setup   : --> ready
    Teardown: --> ??
    Options : -->
    pause   : -->
    describe: -->

*/
static HI_S32 RTSPSVR_StreamSession_HandleRequest(HI_RTSP_STREAM_SESSION_S* pSess, const HI_CHAR* pRequest)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Cseq = 0;
    HI_S32 s32RespLen = 0;
    HI_RTSP_REQ_METHOD_E eMethod = HIRTSP_REQ_METHOD_BUTT;
    RTSPSVR_CHECK_NULL_ERROR(pSess);
    RTSPSVR_CHECK_NULL_ERROR(pRequest);

    /*response msg or rtcp msg do not handle and return success*/
    if (!HI_RTSPSVR_MSGParser_CheckValidRequest(pRequest))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "response msg or rtcp in tcp_itlv invalid rtsp \n");
        return HI_SUCCESS;
    }

    s32Ret = HI_RTSPSVR_MSGParser_ParseMethod(pRequest, &eMethod);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp request, invalid method \n");
        return s32Ret;
    }

    s32Ret = HI_RTSPSVR_MSGParser_GetCSeq(pRequest, &s32Cseq);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp request, have no cseq \n");
        return s32Ret;
    }

    pSess->s32CurReqSeq = s32Cseq;

    switch (eMethod)
    {
        case HIRTSP_OPTIONS_METHOD:
            RTSPSVR_Handle_Options(pSess);
            break;

        case HIRTSP_DISCRIBLE_METHOD:
            s32Ret = RTSPSVR_Handle_Describe(pSess, pRequest);
            break;

        case HIRTSP_SETUP_METHOD:
            s32Ret = RTSPSVR_Handle_Setup(pSess, pRequest);
            break;

        case HIRTSP_PLAY_METHOD:
            s32Ret = RTSPSVR_Handle_Play(pSess, pRequest);
            break;

        case HIRTSP_TEARDOWN_METHOD:
            s32Ret = RTSPSVR_Handle_Teardown(pSess, pRequest);
            break;

        default:
            RTSPSVR_Handle_NotSupported(pSess);
            break;
    }

    s32RespLen = strlen(pSess->aszResponseBuff);
    if ( s32RespLen > 0)
    {
        if (HI_SUCCESS != HI_RTSPSVR_StreamSession_Send(pSess->s32SessSock, pSess->aszResponseBuff, s32RespLen))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "send RTSP response to client fail\n");
            return HI_FAILURE;
        }
    }

    if (HI_SUCCESS != memset_s(pSess->aszResponseBuff, RTSP_MAX_PROTOCOL_BUFFER, 0x00, RTSP_MAX_PROTOCOL_BUFFER))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set aszResponseBuff error\n");
    }


    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle request failed\n");
        return s32Ret;
    }



    RTSPSVR_Update_State(pSess, eMethod);

    return HI_SUCCESS;
}



/**
 * @brief handle request.
 * @param[in] pstSession : session info, pcRecvBuff : msg from client, pu32BuffLen: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static HI_S32 RTSPSVR_HandleMsg(HI_RTSP_STREAM_SESSION_S* pstSession, HI_CHAR* pszRecvBuff, HI_U32* pu32BuffLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bHalfFlag = HI_FALSE;

    if ((NULL == pstSession) || (NULL == pszRecvBuff))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input argument is null!!!\n");
        return HI_FAILURE;
    }

    if (0 != *pu32BuffLen)
    {
        (HI_VOID)RTSPSVR_CutInvaildMsg(pszRecvBuff, pu32BuffLen);
        s32Ret = RTSPSVR_CutRtcpMsg(pszRecvBuff, pu32BuffLen , &bHalfFlag);
        if (HI_SUCCESS != s32Ret)
        {
            /*it is only a rtcp packet*/
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "it is only a rtcp packet!!!\n");
            return HI_SUCCESS;
        }

        if (bHalfFlag != HI_TRUE)
        {
            if (NULL != strstr(pszRecvBuff, "\r\n\r\n"))
            {
                /*normal rtsp packet*/
                s32Ret = RTSPSVR_StreamSession_HandleRequest(pstSession, pszRecvBuff);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Handle Request msg failed Ret: %d\n", s32Ret);
                    return HI_FAILURE;
                }

                RTSPSVR_CutMsg(pszRecvBuff, pu32BuffLen);
            }
            else
            {
                /*other invaild packets, not rtcp not response*/
                if (HI_SUCCESS != memset_s(pszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER, 0x00, RTSP_MAX_PROTOCOL_BUFFER))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set pszRecvBuff error\n");
                }
                *pu32BuffLen = 0;
            }
        }
        else
        {
            if (NULL == strstr(pszRecvBuff, "\r\n\r\n"))
            {
                /*half rtcp packet!*/
                if (HI_SUCCESS != memset_s(pszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER, 0x00, RTSP_MAX_PROTOCOL_BUFFER))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set pszRecvBuff error\n");
                }
                *pu32BuffLen = 0;
            }
        }
    }

    if (HIRTSP_SESSION_STATE_STOP == pstSession->enSessState)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSP Session state change into stop !!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_VOID* RTSPSVR_StreamSession_Process(HI_VOID* arg)
{
    HI_S32 s32Ret = 0;
    HI_RTSP_STREAM_SESSION_S* pSession = (HI_RTSP_STREAM_SESSION_S*)arg;
    fd_set read_fds;
    struct timeval TimeoutVal;
    HI_S32 s32Len = 0;
    HI_S32 s32RecvBytes = 0;
    HI_CHAR aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    HI_U32  u32BuffLen = 0;

    if (HI_SUCCESS != memset_s(&TimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set TimeoutVal fail\n");
        return HI_NULL;
    }
    TimeoutVal.tv_sec = RTSP_TRANS_TIMEVAL_SEC;
    TimeoutVal.tv_usec = 0;
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, (unsigned long)"RtspSessProc", 0, 0, 0);

#ifndef __HuaweiLite__

    /* block Signal Handler SIGPIPE */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);
#endif
    s32Len = strlen(pSession->aszRecvBuff);
    if (s32Len <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RECV buffer len not supported\n");
        return HI_NULL;
    }
    /*already have first request in buffer to be handle*/
    else
    {
        pSession->aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        if (HI_SUCCESS != strncpy_s(aszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER, pSession->aszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER - 1))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszRecvBuff fail\n");
            return HI_NULL;
        }
        aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        u32BuffLen = u32BuffLen + s32Len;
    }

    time_t LastTimeTick = time(NULL);
    time_t CurTimeTick = time(NULL);
    /*process RTSP live session*/
    while (HIRTSP_SESSION_STATE_STOP != pSession->enSessState)
    {
        s32Ret = RTSPSVR_HandleMsg(pSession, aszRecvBuff, &u32BuffLen);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Need to break the msg proc looper!!\n");
            break;
        }

        time(&CurTimeTick);
        /*if set the timeout value then check if timeout  and destroy  the session */
        if (pSession->s32Timeout > 0 && CurTimeTick - LastTimeTick > pSession->s32Timeout)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " lorin add -> client disconnected :[RTSP session timeout %llds]\n",
                                 (HI_S64)(CurTimeTick - LastTimeTick));
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(pSession->s32SessSock, &read_fds);

        RTSPSVR_RecvMsg_RefreshSelectTime(pSession, aszRecvBuff, &u32BuffLen, &TimeoutVal);

        s32Ret = select(pSession->s32SessSock + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if ( s32Ret < 0 )
        {
            if (EINTR == errno || EAGAIN == errno)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "  [select err: %s]\n",  strerror(errno));
                continue;
            }
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "client  disconnected :[select err: %s]\n", strerror(errno));
            break;
        }
        else if (0 == s32Ret)
        {
            /*connection establish period, timeout will close session*/
            s32Ret = RTSPSVR_RecvMsg_Timeout(pSession, aszRecvBuff, &u32BuffLen);
            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "client  disconnected :" \
                                     "[not complete RTSP session,select: overtime %llu s.%llu us)]\n",
                                     (HI_U64)TimeoutVal.tv_sec, (HI_U64)TimeoutVal.tv_usec);
                break;
            }
        }
        else if (s32Ret > 0)
        {
            if (FD_ISSET(pSession->s32SessSock, &read_fds))
            {
                if (HI_SUCCESS != memset_s(pSession->aszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER, 0, RTSP_MAX_PROTOCOL_BUFFER))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set TimeoutVal fail\n");
                }
                s32RecvBytes = recv(pSession->s32SessSock, pSession->aszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER - 1, 0);
                if (s32RecvBytes <= 0)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "client  disconnected :[RTSP msg read from socket: %s]\n",
                                         strerror(errno));
                    break;
                }
                else
                {
                    pSession->aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
                    pSession->u32RecvLen = s32RecvBytes;
                    if (RTSP_MAX_PROTOCOL_BUFFER - u32BuffLen > pSession->u32RecvLen)
                    {
                        if (HI_SUCCESS != memcpy_s(aszRecvBuff + u32BuffLen, pSession->u32RecvLen, pSession->aszRecvBuff, pSession->u32RecvLen))
                        {
                            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy recvbuffer fail \n");
                            break;
                        }
                        u32BuffLen = u32BuffLen + pSession->u32RecvLen;
                    }
                    else
                    {
                        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "the ache buff is over!\n");
                        break;
                    }
                    //HI_SERVER_LOG_Printf( MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Lorin Add -> Recv_Buf = %s\n", pSession->aszRecvBuff);
                    /*refresh session timeout*/
                    time(&LastTimeTick);
                }
            }
            else
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "select readset ok, but socket not in read set, error happen\n");
                break;
            }
        }

        if (HI_SUCCESS != RTSPSVR_SendMedia(pSession))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "something wrong in send data Ret: %d\n", s32Ret);
            break;
        }
        time(&LastTimeTick);  //Lorin Add 刷新一下时间戳，否则 7秒之后断流
    }   /*end of while()*/

    s32Ret = HI_RTSPSVR_StreamSession_Stop(pSession);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "process session stop  failed :%d \n", s32Ret);
    }

    if (pSession->stSessListener.onSessionDestroy)
    {
        pSession->stSessListener.onSessionDestroy(pSession->u32listenerObj, pSession);
    }

    return HI_NULL;
}

HI_S32 RTSPSVR_WriteVideoStream( HI_RTSP_STREAM_SESSION_S* pSession, HI_Track_Source_Handle pTrackSrc,  HI_RTSP_DATA_S* pstStreamData)
{
    HI_MBUF_SLICEINFO_S stSliceInfo = {0};
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrameLen = 0;
    HI_BOOL bIDRFound = HI_FALSE;
    HI_U32 i = 0;
    for (i = 0; i < pstStreamData->u32BlockCnt; i++)
    {
        u32FrameLen += pstStreamData->au32DataLen[i];
        stSliceInfo.pAddr[i] = pstStreamData->apu8DataPtr[i];
        stSliceInfo.au32BlockLen[i] = pstStreamData->au32DataLen[i];
    }
    stSliceInfo.u32BlockCount = pstStreamData->u32BlockCnt;
    stSliceInfo.u64Pts = pstStreamData->u64Pts;
    stSliceInfo.u32Seq = pstStreamData->u32Seq;
    stSliceInfo.u32FrameLen = u32FrameLen;
    stSliceInfo.u32SliceLen = u32FrameLen;
    stSliceInfo.u32HeadLen = 32;
    stSliceInfo.u32TailLen = 0;
    stSliceInfo.u32SliceNum = 1;
    stSliceInfo.u32SliceCnt = 1;
    stSliceInfo.bIsKeyFrame = pstStreamData->bIsKeyFrame;
    stSliceInfo.u8PayloadType = pTrackSrc->unTrackSourceAttr.stVideoInfo.enCodecType;
    stSliceInfo.u8SliceType = 0x05;


    s32Ret = HI_NETWORK_MBuf_OnCBFrmWrite(pSession->stMediaInfo.hMbufHandle, &stSliceInfo);
    if ( HI_SUCCESS == s32Ret)
    {
        if (!pSession->u64StartPts && !pSession->bGetIFrame)
        {
            pSession->u64StartPts = stSliceInfo.u64Pts;
        }
    }


    if (HI_SUCCESS == s32Ret && pstStreamData->bIsKeyFrame && !pSession->bGetIFrame)//parse sps pps for first I frame
    {
        //read first i frame
        HI_U8* pu8Buffer = NULL;
        HI_U32 u32IFrameLen = u32FrameLen;
        HI_U64 u64Pts = 0;
        BUF_DATATYPE_E enBuffPayloadType = BUF_DATATYPE_BUTT;
        HI_BOOL bKeyFlag = HI_FALSE;
        s32Ret = HI_NETWORK_MBufferRead(pSession->stMediaInfo.hMbufHandle, (HI_VOID**)(&pu8Buffer), &u32IFrameLen, &u64Pts, &enBuffPayloadType, &bKeyFlag);
        if (HI_SUCCESS != s32Ret || !bKeyFlag)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_WriteVideoStream get first iframe fail\n");
            return s32Ret;
        }

        //first I frame
        if (HI_TRACK_VIDEO_CODEC_H264 == stSliceInfo.u8PayloadType)
        {
            s32Ret = RTSPSVR_ParseAvcKeyFrame(pSession,  pu8Buffer, u32IFrameLen, &bIDRFound);
        }
        else if (HI_TRACK_VIDEO_CODEC_H265 == stSliceInfo.u8PayloadType)
        {
            s32Ret = RTSPSVR_ParseHevcKeyFrame(pSession,  pu8Buffer, u32IFrameLen, &bIDRFound);
        }


        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        if (bIDRFound)
        {
            pSession->bGetIFrame = HI_TRUE;
        }

    }

    return s32Ret;
}


HI_S32 RTSPSVR_WriteAudioStream(HI_RTSP_STREAM_SESSION_S* pSession, HI_Track_Source_Handle pTrackSrc, HI_RTSP_DATA_S* pstStreamData)
{
    HI_MBUF_SLICEINFO_S stSliceInfo = {0};
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrameLen = 0;
    HI_U32 i = 0;
    for (i = 0; i < pstStreamData->u32BlockCnt; i++)
    {
        u32FrameLen += pstStreamData->au32DataLen[i];
        stSliceInfo.pAddr[i] = pstStreamData->apu8DataPtr[i];
        stSliceInfo.au32BlockLen[i] = pstStreamData->au32DataLen[i];
    }
    stSliceInfo.u32BlockCount = pstStreamData->u32BlockCnt;

    stSliceInfo.u64Pts = pstStreamData->u64Pts;
    stSliceInfo.u32Seq = pstStreamData->u32Seq;
    stSliceInfo.u32FrameLen = u32FrameLen;
    stSliceInfo.u32SliceLen = u32FrameLen;
    stSliceInfo.u32HeadLen = 32;
    stSliceInfo.u32TailLen = 0;
    stSliceInfo.u32SliceNum = 1;
    stSliceInfo.u32SliceCnt = 1;
    stSliceInfo.bIsKeyFrame = HI_TRUE;
    stSliceInfo.u8PayloadType = pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType;
    stSliceInfo.u8SliceType = 0x05;

    //if i frame come and audio pts later than video,put in
    if (HI_TRUE == pSession->bGetIFrame && stSliceInfo.u64Pts >= pSession->u64StartPts)
    {
        s32Ret = HI_NETWORK_MBuf_OnCBFrmWrite(pSession->stMediaInfo.hMbufHandle, &stSliceInfo);
    }

    return s32Ret;
}

HI_S32 HI_RTSPSVR_StreamSession_WriteFrame(HI_RTSP_STREAM_SESSION_S* pSession, HI_Track_Source_Handle pTrackSrc,  HI_RTSP_DATA_S* pstStreamData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //if session stop ,do not need to write,just return
    if ( HIRTSP_SESSION_STATE_STOP ==  pSession->enSessState )
    {
        return HI_SUCCESS;
    }

    if (HI_TRACK_SOURCE_TYPE_VIDEO == pTrackSrc->enTrackType)
    {
        s32Ret = RTSPSVR_WriteVideoStream(pSession, pTrackSrc, pstStreamData);
    }
    else if (HI_TRACK_SOURCE_TYPE_AUDIO == pTrackSrc->enTrackType)
    {
        s32Ret = RTSPSVR_WriteAudioStream(pSession, pTrackSrc, pstStreamData);
    }

    if (HI_ERR_MBUF_FULL == s32Ret)
    {
        s32Ret = HI_ERR_RTSPSERVER_ERR_MBUF_FULL;
    }
    else if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_RTSPSERVER_ERR_WRITE_FRAME;
    }

    return s32Ret;
}


HI_S32 HI_RTSPSVR_StreamSession_Send(HI_SOCKET s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32RemSize = 0;
    HI_S32 s32Size    = 0;
    HI_CHAR*  ps8BufferPos = NULL;
    fd_set write_fds;
    struct timeval TimeoutVal;  /* Timeout value */
    HI_S32 s32Errno = 0;

    if (HI_SUCCESS != memset_s(&TimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval)) )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set TimeoutVal fail\n");
        return HI_FAILURE;
    }
    u32RemSize = u32DataLen;
    ps8BufferPos = pszBuff;

    while (u32RemSize > 0)
    {
        FD_ZERO(&write_fds);
        FD_SET(s32WritSock, &write_fds);
        TimeoutVal.tv_sec = RTSP_TRANS_TIMEVAL_SEC;
        TimeoutVal.tv_usec = RTSP_TRANS_TIMEVAL_USEC;
        /*judge if it can send */
        s32Ret = select(s32WritSock + 1, NULL, &write_fds, NULL, &TimeoutVal);

        if (s32Ret > 0)
        {
            if ( FD_ISSET(s32WritSock, &write_fds))
            {
                s32Size = send(s32WritSock, ps8BufferPos, u32RemSize, 0);

                if (s32Size < 0)
                {
                    /*if it is not eagain error, means can not send*/
                    if (errno != EINTR && errno != EAGAIN)
                    {
                        s32Errno = errno;
                        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSP response Send error:%s\n", strerror(s32Errno));
                        return HI_ERR_RTSPSERVER_ERR_SESS_SEND_FAIL;
                    }

                    /*it is eagain error, means can try again*/
                    continue;
                }

                u32RemSize -= s32Size;
                ps8BufferPos += s32Size;
            }
            else
            {
                s32Errno = errno;
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSP response Send error:fd not in fd_set error:%s\n", strerror(s32Errno));
                return HI_ERR_RTSPSERVER_ERR_SESS_SEND_FAIL;
            }
        }
        /*select found over time or error happend*/
        else if ( s32Ret == 0 )
        {
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSP response Send error:%s  select overtime %d.%ds\n", strerror(s32Errno),
                                 RTSP_TRANS_TIMEVAL_SEC, RTSP_TRANS_TIMEVAL_USEC);
            return HI_ERR_RTSPSERVER_ERR_SESS_SEND_FAIL;
        }
        else if ( s32Ret < 0 )
        {
            if (EINTR == errno || EAGAIN == errno)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "  [select err: %s]\n",  strerror(errno));
                continue;
            }
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSP response Send error:%s\n", strerror(s32Errno));
            return HI_ERR_RTSPSERVER_ERR_SESS_SEND_FAIL;
        }

    }

    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_StreamSession_OnClientConnect(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_CHAR* pClientRequest, HI_U32 u32ReqLen)
{
    HI_S32   s32Ret  = HI_SUCCESS;
    HI_CHAR* pTemp = NULL;
    HI_S32 s32AddrLen  = 0;
    struct sockaddr_in stSockAddr;
    if (HI_SUCCESS != memset_s(&stSockAddr, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set stSockAddr fail\n");
        return HI_FAILURE;
    }
    RTSPSVR_CHECK_NULL_ERROR(pClientRequest);
    RTSPSVR_CHECK_NULL_ERROR(pSession);

    /*cache first request into recvbuffer to be handled in threadprocess*/
    if (HI_SUCCESS != memset_s(pSession->aszRecvBuff, sizeof(pSession->aszRecvBuff), 0, sizeof(pSession->aszRecvBuff)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set aszRecvBuff fail\n");
        return HI_FAILURE;
    }
    if (HI_SUCCESS != strncpy_s(pSession->aszRecvBuff, RTSP_MAX_PROTOCOL_BUFFER, pClientRequest, RTSP_MAX_PROTOCOL_BUFFER - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszRecvBuff fail\n");
        return HI_FAILURE;
    }
    pSession->aszRecvBuff[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
    pSession->u32RecvLen = u32ReqLen;

    s32AddrLen = sizeof(stSockAddr);
    if (0 != getpeername(pSession->s32SessSock, (struct sockaddr*)(&stSockAddr), (socklen_t*)&s32AddrLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "getpeername  error.\n");
        return HI_FAILURE;
    }

    pTemp = inet_ntoa(stSockAddr.sin_addr);
    if (HI_SUCCESS != strncpy_s(pSession->aszClientIP, RTSP_IP_MAX_LEN, pTemp, RTSP_IP_MAX_LEN - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszClientIP fail\n");
        return HI_FAILURE;
    }
    pSession->aszClientIP[RTSP_IP_MAX_LEN - 1] = '\0';

    s32Ret = pthread_create((&(pSession->ptSessThdId)), NULL, RTSPSVR_StreamSession_Process, (HI_VOID*)pSession);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Start RTSP session process thread error.\n");
        pSession->ptSessThdId = RTSP_INVALID_THREAD_ID;
        return  HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID HI_RTSPSVR_StreamSession_GetClientIPAddress(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_CHAR* pBuffer, HI_S32 s32bufLen)
{
    if ( NULL == pSession )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "param pSession is null.\n");
        return ;
    }

    if (HI_SUCCESS != strncpy_s(pBuffer, s32bufLen, pSession->aszClientIP, s32bufLen - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  pBuffer fail\n");
        return ;
    }
    pBuffer[RTSP_IP_MAX_LEN - 1] = '\0';
}

HI_S32 HI_RTSPSVR_StreamSession_SetListener(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_RTSP_SESSION_LISTENER_S* listener, HI_MW_PTR obj)
{
    RTSPSVR_CHECK_NULL_ERROR(listener);
    RTSPSVR_CHECK_NULL_ERROR(listener->onSessionDestroy);
    pSession->stSessListener.onSessionDestroy = listener->onSessionDestroy;
    pSession->u32listenerObj = obj;
    return HI_SUCCESS;
}

HI_VOID HI_RTSPSVR_StreamSession_SetMediaSource(HI_RTSP_STREAM_SESSION_S* pSession,
        HI_Track_Source_Handle pstVidStream, HI_Track_Source_Handle pstAudStream, const HI_CHAR* pStreamName)
{
    pSession->pstAudStream = pstAudStream;
    pSession->pstVidStream = pstVidStream;
    if (HI_SUCCESS != strncpy_s(pSession->aszStreamName, RTSP_MAX_STREAMNAME_LEN, pStreamName, RTSP_MAX_STREAMNAME_LEN - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  aszStreamName fail\n");
        return ;
    }
    pSession->aszStreamName[RTSP_MAX_STREAMNAME_LEN - 1] = '\0';
    return;
}

HI_S32 HI_RTSPSVR_StreamSession_Stop(HI_RTSP_STREAM_SESSION_S* pSession)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if ( NULL == pSession )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "param pSession is null.\n");
        return HI_FAILURE;
    }

    if (HIRTSP_SESSION_STATE_STOP  == pSession->enSessState)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_DEBUG, "session already stop.\n");
        return HI_SUCCESS;
    }

    if (pSession->stMediaInfo.bVideoEnable)
    {
        pSession->stMediaInfo.bVideoEnable = HI_FALSE;
        s32Ret = RTSPSVR_VideoSource_Remove(pSession, pSession->pstVidStream);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR,
                                 " RTSPSVR_VideoSource_Remove  fail %X \n", s32Ret);
        }
        HI_NETWORK_MBufferUnRegister(pSession->stMediaInfo.hMbufHandle, pSession->stMediaInfo.stVideoInfo.enCodecType);
    }

    if (pSession->stMediaInfo.bAudioEnable)
    {
        pSession->stMediaInfo.bAudioEnable = HI_FALSE;
        s32Ret = RTSPSVR_AudioSource_Remove(pSession, pSession->pstAudStream);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR,
                                 " RTSPSVR_AudioSource_Remove  fail %X \n", s32Ret);
        }
        HI_NETWORK_MBufferUnRegister(pSession->stMediaInfo.hMbufHandle, pSession->stMediaInfo.stAudioInfo.enCodecType);

    }


    (HI_VOID)pthread_mutex_lock(&pSession->mStateLock);

    pSession->enSessState = HIRTSP_SESSION_STATE_STOP;

    (HI_VOID)pthread_mutex_unlock(&pSession->mStateLock);


    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_StreamSession_Destroy(HI_RTSP_STREAM_SESSION_S* pSession)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ( NULL == pSession )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "param pSession is null.\n");
        return HI_FAILURE;
    }

    if (pSession->stVideoSession.s32TrackId != RTSP_INVALID_TRACK_ID )
    {
        HI_RTP_Session_Stop(pSession->stVideoSession.pRtpSession);
        if (RTP_TRANS_UDP == pSession->stVideoSession.pRtpSession->enMediaTransMode)
        {
            HI_RTCP_Session_StopUdp(pSession->stVideoSession.pRtcpSession);
        }
        pSession->stVideoSession.s32TrackId = RTSP_INVALID_TRACK_ID;
        HI_RTP_Session_Destroy(pSession->stVideoSession.pRtpSession);
        pSession->stVideoSession.pRtpSession = NULL;
        HI_RTCP_Session_Destroy(pSession->stVideoSession.pRtcpSession);
        pSession->stVideoSession.pRtcpSession = NULL;
    }

    if (pSession->stAudioSession.s32TrackId != RTSP_INVALID_TRACK_ID )
    {
        HI_RTP_Session_Stop(pSession->stAudioSession.pRtpSession);

        if (RTP_TRANS_UDP == pSession->stAudioSession.pRtpSession->enMediaTransMode)
        {
            HI_RTCP_Session_StopUdp(pSession->stAudioSession.pRtcpSession);
        }
        pSession->stAudioSession.s32TrackId = RTSP_INVALID_TRACK_ID;
        HI_RTP_Session_Destroy(pSession->stAudioSession.pRtpSession);
        pSession->stAudioSession.pRtpSession = NULL;
        HI_RTCP_Session_Destroy(pSession->stAudioSession.pRtcpSession);
        pSession->stAudioSession.pRtcpSession = NULL;
    }

    if (HI_NULL != pSession->stMediaInfo.hMbufHandle)
    {
        s32Ret = HI_NETWORK_MBufferDestroy(&(pSession->stMediaInfo.stBufInfo), pSession->stMediaInfo.hMbufHandle);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR,
                                 " HI_NETWORK_MBufferDestroy fail %X \n", s32Ret);
        }
        pSession->stMediaInfo.hMbufHandle = HI_NULL;

    }

    pthread_mutex_destroy(&pSession->stVideoSession.mMutexGetPort);
    pthread_mutex_destroy(&pSession->stAudioSession.mMutexGetPort);
    pthread_mutex_destroy(&pSession->mStateLock);
    HI_NETWORK_CloseSocket(&pSession->s32SessSock);
    free(pSession);
    pSession = NULL;

    return HI_SUCCESS;
}


HI_S32 HI_RTSPSVR_StreamSession_Create(HI_RTSP_STREAM_SESSION_S** ppSession, HI_S32 s32SocketFd)
{
    HI_RTSP_STREAM_SESSION_S* pSession = NULL;
    pSession = (HI_RTSP_STREAM_SESSION_S*)malloc(sizeof(HI_RTSP_STREAM_SESSION_S));

    if (!pSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HiRTSPStreamSession malloc failed\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != memset_s(pSession, sizeof(HI_RTSP_STREAM_SESSION_S), 0x00, sizeof(HI_RTSP_STREAM_SESSION_S)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set pSession fail\n");
        free(pSession);
        return HI_FAILURE;
    }
    pSession->enSessState = HIRTSP_SESSION_STATE_INIT;
    pSession->ptSessThdId = RTSP_INVALID_THREAD_ID;
    pSession->stVideoSession.s32TrackId = RTSP_INVALID_TRACK_ID;
    pSession->stAudioSession.s32TrackId = RTSP_INVALID_TRACK_ID;
    pSession->bGetIFrame = HI_FALSE;
    pSession->stMediaInfo.stPPS.u32DataLen = MAX_NALPARA_LEN;
    pSession->stMediaInfo.stSPS.u32DataLen = MAX_NALPARA_LEN;
    pSession->stMediaInfo.stVPS.u32DataLen = MAX_NALPARA_LEN;
    pSession->stMediaInfo.hMbufHandle = HI_NULL;
    pSession->s32SessSock = s32SocketFd;
    /*get session id*/
    if (HI_SUCCESS != memset_s(pSession->aszSessID, sizeof(HI_CHAR)* RTSP_SESSID_MAX_LEN, 0, sizeof(HI_CHAR)* RTSP_SESSID_MAX_LEN))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set aszSessID fail\n");
        free(pSession);
        return HI_FAILURE;
    }
    HI_NETWORK_RandomID(pSession->aszSessID, RTSP_SESSID_MAX_LEN - 1);
    /*get peer host info */
    HI_NETWORK_GetPeerIPPort(s32SocketFd, pSession->aszClientIP, &(pSession->u16CliRTSPPort));
    /*get local host info*/
    HI_NETWORK_GetHostIP(s32SocketFd, pSession->aszHostIP);

    (HI_VOID)pthread_mutex_init(&pSession->mStateLock, NULL);
    (HI_VOID)pthread_mutex_init(&pSession->stVideoSession.mMutexGetPort,  NULL);
    (HI_VOID)pthread_mutex_init(&pSession->stAudioSession.mMutexGetPort,  NULL);
    *ppSession = pSession;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
