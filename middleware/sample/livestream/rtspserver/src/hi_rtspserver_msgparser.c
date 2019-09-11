/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtspserver_msgparser.c
* @brief     rtspserver msgparser src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#define _GNU_SOURCE
#include <string.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_rtsp_server.h"
#include "hi_network_common.h"
#include "hi_rtspserver_msgparser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define RTSP_INVALID_STATUS "Invalid Status"
#define RTSP_INVALID_STATUS_CODE  (-1)

static HiRTSPStatus HiRTSP_Status[] =
{
    {"Continue", 100},
    {"OK", 200},
    {"Created", 201},
    {"Accepted", 202},
    {"Non-Authoritative Information", 203},
    {"No Content", 204},
    {"Reset Content", 205},
    {"Partial Content", 206},
    {"Multiple Choices", 300},
    {"Moved Permanently", 301},
    {"Moved Temporarily", 302},
    {"Bad Request", 400},
    {"Unauthorized", 401},
    {"Payment Required", 402},
    {"Forbidden", 403},
    {"Stream Not Found", 404},
    {"Method Not Allowed", 405},
    {"Not Acceptable", 406},
    {"Proxy Authentication Required", 407},
    {"Request Time-out", 408},
    {"Conflict", 409},
    {"Gone", 410},
    {"Length Required", 411},
    {"Precondition Failed", 412},
    {"Request Entity Too Large", 413},
    {"Request-URI Too Large", 414},
    {"Unsupported Media Type", 415},
    {"Over Supported connection ", 416},
    {"Bad Extension", 420},
    {"Invalid Parameter", 450},
    {"Parameter Not Understood", 451},
    {"Conference Not Found", 452},
    {"Not Enough Bandwidth", 453},
    {"Session Not Found", 454},
    {"Method Not Valid In This State", 455},
    {"Header Field Not Valid for Resource", 456},
    {"Invalid Range", 457},
    {"Parameter Is Read-Only", 458},
    {"Unsupported Transport", 461},
    {"Internal Server Error", 500},
    {"Not Implemented", 501},
    {"Bad Gateway", 502},
    {"Service Unavailable", 503},
    {"Gateway Time-out", 504},
    {"RTSP Version Not Supported", 505},
    {"Option not support", 551},
    {"Extended Error:", 911},
    {0, RTSP_INVALID_STATUS_CODE}
};


/*response header example
RTSP/1.0 200 OK
Server: Hisilicon Streaming Media Server/1.0.0(Jul 30 2015)
Cseq: 1
Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
*/
HI_BOOL HI_RTSPSVR_MSGParser_IsResponse(const HI_CHAR* pszReq)
{
    HI_CHAR aszVersion[RTSP_VER_MAX_LEN] = {0};
    HI_U32 u32Stat = 0;
    HI_S32 s32Cnt = 0;

    s32Cnt = sscanf_s(pszReq, " %31s %d", aszVersion, RTSP_VER_MAX_LEN, &u32Stat);
    aszVersion[RTSP_VER_MAX_LEN - 1] = '\0';
    if (strncasecmp(aszVersion, "RTSP/", strlen("RTSP/"))
        || s32Cnt < RTSP_SCANF_RET_TWO
        || 0 == u32Stat )
    {
        return HI_FALSE; /* not a response message */
    }

    return HI_TRUE;
}


HI_BOOL HI_RTSPSVR_MSGParser_CheckValidRequest(const HI_CHAR* pszReq)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* check for request message. */
    if (HI_RTSPSVR_MSGParser_IsResponse(pszReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "msg formate error, not request, just rtsp response\n");
        return HI_FALSE;
    }

    HI_RTSP_REQ_METHOD_E eMethod;
    /* not a response message, check for method request. */
    s32Ret = HI_RTSPSVR_MSGParser_ParseMethod(pszReq, &eMethod);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "Method requested was invalid.  Message discarded."
                             "Method = %s\n", pszReq);
        return HI_FALSE;
    }

    return HI_TRUE;
}

HI_VOID HI_RTSPSVR_MSGParser_GetResponse(HI_S32 StatCode, HI_S32 s32Cseq,
        HI_CHAR* pReply, HI_U32 bufLen)
{
    if (HI_FAILURE == snprintf_s(pReply, bufLen, bufLen - 1,
                                 "%s %d %s\r\n"
                                 "CSeq: %d\r\n"
                                 "Cache-Control: no-cache\r\n"
                                 "Server: %s\r\n"
                                 "%s\r\n",
                                 RTSP_VER_STR,
                                 StatCode,
                                 HI_RTSPSVR_MSGParser_StatusCode2Str(StatCode),
                                 s32Cseq,
                                 RTSP_SERVER_DESCRIPTION,
                                 HI_NETWORK_DateHeader()))
    {

        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_MSGParser_GetResponse string print pReply error\n");
        return;
    }
    return;
}

/*OPTIONS rtsp://ip:554/12 RTSP/1.0
CSeq: 1
Authorization: Basic Og==
User-Agent: Hisilicon Streaming Media Client/1.0.0(May 23 2016)*/
HI_S32 HI_RTSPSVR_MSGParser_GetStreamName(const HI_CHAR* pszReq, HI_CHAR* pStreamName, HI_U32 bufLen)
{
    HI_CHAR aszMethod[RTSP_METHOD_MAX_LEN] = {0};
    HI_CHAR aszUrl[RTSP_URL_MAX_LEN] = {0};
    HI_S32 s32Cnt = 0;
    HI_CHAR* pTmpPtr = NULL;

    s32Cnt = sscanf_s(pszReq, " %15s %255s", aszMethod, RTSP_METHOD_MAX_LEN, aszUrl, RTSP_URL_MAX_LEN);
    if (s32Cnt != RTSP_SCANF_RET_TWO)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req format error\n");
        return HI_FAILURE;
    }
    aszMethod[RTSP_METHOD_MAX_LEN - 1] = '\0';
    aszUrl[RTSP_URL_MAX_LEN - 1] = '\0';

    pTmpPtr = strcasestr(aszUrl, "rtsp://");
    if (NULL == pTmpPtr)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req format error, url do not have rtsp://\n");
        return HI_FAILURE;
    }

    pTmpPtr += strlen("rtsp://");
    pTmpPtr = strchr(pTmpPtr, '/');
    if (!pTmpPtr)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp req  url do not have streamName\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != strncpy_s(pStreamName, bufLen, pTmpPtr + 1, bufLen - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy pStreamName fail\n");
        return HI_FAILURE;
    }
    pStreamName[bufLen - 1] = '\0';
    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_MSGParser_GetCSeq( const HI_CHAR* pszReq, HI_S32* pCseq)
{
    HI_CHAR* pTemp = NULL;

    pTemp = strcasestr(pszReq, RTSP_HEADER_CSEQ);
    if ( NULL == pTemp )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "there no Cseq in req str\n");
        return HI_FAILURE;
    }
    else
    {
        if (sscanf_s(pTemp, "%*s %d", pCseq) != RTSP_SCANF_RET_ONE)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;

}
HI_S32 HI_RTSPSVR_MSGParser_GetSessId(const HI_CHAR* pMsgStr, HI_CHAR* pszSessId)
{
    HI_CHAR* pTemp = NULL;

    /*get sessionid*/
    pTemp = strcasestr(pMsgStr, RTSP_HEADER_SESSION);
    if ( NULL == pTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "there no sessid in req str\n");
        return HI_FAILURE;
    }
    else
    {
        if (sscanf_s(pTemp, "%*s %15s", pszSessId, RTSP_SESSID_MAX_LEN) != RTSP_SCANF_RET_ONE)
        {
            return HI_FAILURE;
        }
    }
    pszSessId[RTSP_SESSID_MAX_LEN - 1] = '\0';


    return HI_SUCCESS;
}


HI_S32 HI_RTSPSVR_MSGParser_ParseMethod(const HI_CHAR* pszReq, HI_RTSP_REQ_METHOD_E* pMethod)
{
    HI_CHAR aszMethod [RTSP_METHOD_MAX_LEN] = {0};
    if ( RTSP_SCANF_RET_ONE != sscanf_s(pszReq, "%15[^ ]", aszMethod, RTSP_METHOD_MAX_LEN))
    {
        return HI_FAILURE;
    }
    aszMethod[RTSP_METHOD_MAX_LEN - 1] = '\0';

    if ( 0 == strncmp(aszMethod, RTSP_METHOD_DESCRIBE, strlen(RTSP_METHOD_DESCRIBE)))
    {
        *pMethod = HIRTSP_DISCRIBLE_METHOD;
    }
    else if ( 0 == strncmp(aszMethod, RTSP_METHOD_SETUP, strlen(RTSP_METHOD_SETUP)))
    {
        *pMethod = HIRTSP_SETUP_METHOD;
    }
    else if ( 0 == strncmp(aszMethod, RTSP_METHOD_PLAY, strlen(RTSP_METHOD_PLAY) ))
    {
        *pMethod = HIRTSP_PLAY_METHOD;
    }
    else if ( 0 == strncmp(aszMethod, RTSP_METHOD_TEARDOWN, strlen(RTSP_METHOD_TEARDOWN) ))
    {
        *pMethod = HIRTSP_TEARDOWN_METHOD;
    }
    else if ( 0 == strncmp(aszMethod, RTSP_METHOD_OPTIONS, strlen(RTSP_METHOD_OPTIONS) ))
    {
        *pMethod = HIRTSP_OPTIONS_METHOD;
    }
    else if ( 0 == strncmp(aszMethod, RTSP_METHOD_PAUSE, strlen(RTSP_METHOD_PAUSE) ))
    {
        *pMethod = HIRTSP_PAUSE_METHOD;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "not supported rtsp request method maybe rtcp in tcp_itlv\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



HI_CHAR* HI_RTSPSVR_MSGParser_StatusCode2Str(HI_S32 code)
{
    HiRTSPStatus*  pStatus;

    for (pStatus = HiRTSP_Status; pStatus->s32StatCode != RTSP_INVALID_STATUS_CODE; pStatus++ )
    {
        if ( pStatus->s32StatCode == code )
        {
            return pStatus->pszStatStr;
        }
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invalid status code.%d .\n", code);
    return RTSP_INVALID_STATUS;
}

HI_U32 HI_RTSPSVR_MSGParser_GetAAConfigInfo(HI_U32 u32SampleRate, HI_U32 u32ChanNum)
{
    HI_U32 u32ConfigNum = HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_16;

    if (u32ChanNum == RTSP_AUDIO_SINGLE_CHN)
    {
        switch (u32SampleRate)
        {
            case HI_LIVE_AUDIO_SAMPLE_RATE_8:
                u32ConfigNum = HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_8;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_16:
                u32ConfigNum = HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_16;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_32:
                u32ConfigNum = HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_32;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_48:
                u32ConfigNum = HI_LIVE_AUDIO_SINGLECHN_CONFIGNUM_48;
                break;

            default:
                break;
        }
    }
    else if (u32ChanNum == RTSP_AUDIO_DOUBLE_CHN)
    {
        switch (u32SampleRate)
        {
            case HI_LIVE_AUDIO_SAMPLE_RATE_8:
                u32ConfigNum = HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_8;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_16:
                u32ConfigNum = HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_16;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_32:
                u32ConfigNum = HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_32;
                break;

            case HI_LIVE_AUDIO_SAMPLE_RATE_48:
                u32ConfigNum = HI_LIVE_AUDIO_DOUBLECHN_CONFIGNUM_48;
                break;

            default:
                break;
        }

    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invalid audio channel.%d .\n", u32ChanNum);
    }

    return u32ConfigNum;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
