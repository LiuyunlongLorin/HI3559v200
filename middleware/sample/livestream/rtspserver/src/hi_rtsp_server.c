/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtsp_server.c
* @brief     rtspserver
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_network_common.h"
#include "hi_network_listener.h"
#include "hi_network_mbuffer.h"
#include "hi_rtspserver_session.h"
#include "hi_rtspserver_msgparser.h"
#include "hi_rtsp_server.h"
#include "hi_rtsp_server_err.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#ifndef MW_VERSION
#define MW_VERSION "0.0.0"
#endif
static volatile const HI_CHAR RTSPSERVER_VERSIONINFO[] = "HIBERRY RTSPSERVER MW Version: "MW_VERSION;
static volatile const HI_CHAR RTSPSERVER_BUILD_DATE[] = "HIBERRY RTSPSERVER Build Date:"__DATE__" Time:"__TIME__;

#define RTSP_MAX_CONN_NUM (32)
#define RTSP_MAX_LISTEN_PORT (65535)
#define RTSP_MIN_BUFFER_SIZE (216*1024)
#define RTSP_MAX_BUFFER_SIZE (50*1024*1024)
#define RTSP_MAX_PAYLOAD_TYPE (254)
#define RTSP_MAX_TIME_OUT (60)
#define RTSP_MAX_PACKET_LEN (5000)
#define RTSP_MIN_PACKET_LEN (500)
#define DEFAULT_LISTEN_PORT (554)
#define RTSP_MAX_STREAM_NUM (8)
#define RTSP_KEEPALIVE_SECONDS_MIN (6)

static HI_RTSP_SERVER_CTX_S* g_pstServerCtx = NULL;

static HI_S32 RTSPSVR_CheckFrameData(HI_RTSP_DATA_S* pstFrameData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32SliceIdx = 0;
    RTSPSVR_CHECK_NULL_ERROR(pstFrameData);
    if (0 == pstFrameData->u32BlockCnt || pstFrameData->u32BlockCnt > HI_RTSPSVR_FRAME_MAX_BLOCK)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_CheckFrameData illegal slice count\n");
        s32Ret = HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    for (u32SliceIdx = 0; u32SliceIdx < pstFrameData->u32BlockCnt; u32SliceIdx++)
    {
        RTSPSVR_CHECK_NULL_ERROR(pstFrameData->apu8DataPtr[u32SliceIdx]);
        if (0 == pstFrameData->au32DataLen[u32SliceIdx])
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_CheckFrameData illegal frame data len\n");
            s32Ret = HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
    }

    return s32Ret;
}

static HI_S32 RTSPSVR_Check_StreamData(HI_Track_Source_Handle pTrackSrc, HI_RTSP_DATA_S* pstData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_TRACK_SOURCE_TYPE_VIDEO == pTrackSrc->enTrackType)
    {

        if (HI_TRACK_VIDEO_CODEC_H264 != pTrackSrc->unTrackSourceAttr.stVideoInfo.enCodecType && HI_TRACK_VIDEO_CODEC_H265 != pTrackSrc->unTrackSourceAttr.stVideoInfo.enCodecType )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "not support video type :%d \n", pTrackSrc->unTrackSourceAttr.stVideoInfo.enCodecType);
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
        if (NULL == pTrackSrc->pfnSourceStart || NULL == pTrackSrc->pfnSourceStop)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "video source call back null error \n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }

    }
    else if (HI_TRACK_SOURCE_TYPE_AUDIO == pTrackSrc->enTrackType)
    {

        if ( HI_TRACK_AUDIO_CODEC_AAC != pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G711A != pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G711Mu != pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G726 != pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_ADPCM != pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType
           )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "not support audio type :%d\n", pTrackSrc->unTrackSourceAttr.stAudioInfo.enCodecType);
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }

        if (NULL == pTrackSrc->pfnSourceStart || NULL == pTrackSrc->pfnSourceStop)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "audio source call back null error \n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "source track type illegal \n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    s32Ret =  RTSPSVR_CheckFrameData(pstData);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_CheckFrameData error \n");
        return s32Ret;
    }

    return HI_SUCCESS;
}


static HI_S32 RTSPSVR_SetRTSPconfig(HI_RTSP_STREAM_SESSION_S* pstStreamSession, HI_RTSP_SERVER_CTX_S* pstServerCtx)
{
    RTSPSVR_CHECK_NULL_ERROR(pstStreamSession);
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    pstStreamSession->s32MaxPayload = pstServerCtx->s32MaxPayload ;
    pstStreamSession->s32Timeout = pstServerCtx->s32Timeout ;
    pstStreamSession->s32PacketLen = pstServerCtx->s32PacketLen ;
    pstStreamSession->pRtspSvrHandle = (HI_MW_PTR)pstServerCtx;
    return HI_SUCCESS;
}

static HI_S32 RTSPSVR_CheckRTSPconfig(HI_RTSP_CONFIG_S* pstRTSPConfig)
{

    if (pstRTSPConfig->s32PacketLen < RTSP_MIN_PACKET_LEN || pstRTSPConfig->s32PacketLen > RTSP_MAX_PACKET_LEN)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " param s32PacketLen not in the range  min:%d- max:%d!\n", RTSP_MIN_PACKET_LEN, RTSP_MAX_PACKET_LEN);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (pstRTSPConfig->s32MaxConnNum <= 0 || pstRTSPConfig->s32MaxConnNum > RTSP_MAX_CONN_NUM)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " param s32MaxConnNum not in the range  max:%d!\n", RTSP_MAX_CONN_NUM);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (pstRTSPConfig->s32ListenPort <= 0 || pstRTSPConfig->s32ListenPort > RTSP_MAX_LISTEN_PORT)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " param s32ListenPort not in the range  max:%d!\n", RTSP_MAX_LISTEN_PORT);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (pstRTSPConfig->s32MaxPayload <= 0 || pstRTSPConfig->s32MaxPayload > RTSP_MAX_PAYLOAD_TYPE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " param s32MaxPayload not in the range  max:%d!\n", RTSP_MAX_PAYLOAD_TYPE);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if ( pstRTSPConfig->s32Timeout >= 0 && pstRTSPConfig->s32Timeout < RTSP_KEEPALIVE_SECONDS_MIN)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "s32Timeout out of range, min:%d!\n", RTSP_KEEPALIVE_SECONDS_MIN);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

/*destroy a session when  connect  teardown */
static HI_S32 RTSPSVR_OnSessionDestroy(HI_MW_PTR obj, HI_RTSP_STREAM_SESSION_S* pstSession)
{
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_RTSP_STREAM_SESSION_S* pstSessionInList = NULL;
    HI_BOOL bFound = HI_FALSE;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszClientIP[RTSP_IP_MAX_LEN] = {0};
    HI_RTSP_SERVER_CTX_S* pstServerCtx = (HI_RTSP_SERVER_CTX_S*)obj;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    RTSPSVR_CHECK_NULL_ERROR(pstSession);



    RTSP_LOCK(pstServerCtx->sessionListLock);

    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pSessionlist)
    {
        pstSessionInList = HI_LIST_ENTRY(pstPosNode, HI_RTSP_STREAM_SESSION_S, listPtr);
        if (0 == strncmp(pstSession->aszSessID, pstSessionInList->aszSessID, RTSP_SESSID_MAX_LEN))
        {
            bFound = HI_TRUE;
            break;
        }
    }

    if (!bFound || NULL == pstSessionInList)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "could not find correspond media session\n");
        RTSP_UNLOCK(pstServerCtx->sessionListLock);
        return HI_ERR_RTSPSERVER_ERR_SESS_NOT_EXISTED;
    }

    /* when teardown destroy the session and remove it from the sessionlist*/
    HI_List_Del(&(pstSessionInList->listPtr));
    pstServerCtx->s32UserNum -= 1;


    HI_RTSPSVR_StreamSession_GetClientIPAddress(pstSession, aszClientIP, RTSP_IP_MAX_LEN);
    s32Ret = HI_RTSPSVR_StreamSession_Destroy(pstSession);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_StreamSession_Destroy failed %d\n", s32Ret);
        RTSP_UNLOCK(pstServerCtx->sessionListLock);
        return HI_ERR_RTSPSERVER_ERR_SESS_DESTROY_FAIL;
    }

    if (pstServerCtx->stListener.onClientDisconnect && strlen(aszClientIP) > 0)
    {
        pstServerCtx->stListener.onClientDisconnect(&pstServerCtx->stListener, aszClientIP);
    }


    RTSP_UNLOCK(pstServerCtx->sessionListLock);


    return HI_SUCCESS;
}
/*add a connect session for rtspserver related to a stream*/
static HI_S32 RTSPSVR_AddStreamSession(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pClientReq,
                                       HI_U32 u32ReqLen, HI_S32 s32SocketFd, HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode)
{
    HI_RTSP_SESSION_LISTENER_S stListener;
    HI_RTSP_STREAM_SESSION_S* pstStreamSession = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    List_Head_S* pstPosNode = NULL;
    HI_RTSP_STREAM_SESSION_S* pstSessionNode = NULL;
    HI_CHAR aszClientIP[RTSP_IP_MAX_LEN] = {0};

    if (NULL == pstServerCtx || NULL == pClientReq )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_AddStreamSession param null \n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (pstServerCtx->s32UserNum >= pstServerCtx->s32MaxConNum)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "reach the max connect num:%d \n", pstServerCtx->s32MaxConNum);
        return HI_ERR_RTSPSERVER_ERR_REACH_MAX_CONNECT;
    }

    s32Ret = HI_RTSPSVR_StreamSession_Create(&pstStreamSession, s32SocketFd);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_StreamSession_Create failed ret %d \n", s32Ret);
        return HI_ERR_RTSPSERVER_ERR_SESS_CREATE_FAIL;
    }

    s32Ret =  RTSPSVR_SetRTSPconfig(pstStreamSession, pstServerCtx);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set session rtsp config failed  \n");
        HI_RTSPSVR_StreamSession_Destroy(pstStreamSession);
        return HI_ERR_RTSPSERVER_ERR_SESS_SET_FAIL;
    }


    if (NULL != pstStreamNode)
    {
        pstStreamSession->u32BufSize = pstStreamNode->u32BufSize;
        HI_RTSPSVR_StreamSession_SetMediaSource(pstStreamSession, pstStreamNode->pstVidStream, pstStreamNode->pstAudStream, pstStreamNode->name);
    }

    stListener.onSessionDestroy = RTSPSVR_OnSessionDestroy;
    s32Ret = HI_RTSPSVR_StreamSession_SetListener(pstStreamSession, &stListener, (HI_MW_PTR)pstServerCtx);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set session listerner  failed  \n");
        HI_RTSPSVR_StreamSession_Destroy(pstStreamSession);
        return HI_ERR_RTSPSERVER_ERR_SESS_SET_FAIL;
    }

    s32Ret = HI_RTSPSVR_StreamSession_OnClientConnect(pstStreamSession, pClientReq, u32ReqLen);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "on client connect failed  ret :%d  \n", s32Ret);
        HI_RTSPSVR_StreamSession_Destroy(pstStreamSession);
        return HI_ERR_RTSPSERVER_ERR_SESS_CONNECT_FAIL;
    }

    RTSP_LOCK(pstServerCtx->sessionListLock);

    HI_List_For_Each(pstPosNode, &pstServerCtx->pSessionlist)
    {
        pstSessionNode = HI_LIST_ENTRY(pstPosNode, HI_RTSP_STREAM_SESSION_S, listPtr);

        if (pstSessionNode == pstStreamSession)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsp stream session already exist\n");
            RTSP_UNLOCK(pstServerCtx->sessionListLock);
            s32Ret = HI_ERR_RTSPSERVER_ERR_SESS_EXISTED;
            goto DES_SESSION;
        }
    }
    HI_List_Add(&(pstStreamSession->listPtr), &(pstServerCtx->pSessionlist));
    pstServerCtx->s32UserNum += 1;

    RTSP_UNLOCK(pstServerCtx->sessionListLock);

    HI_RTSPSVR_StreamSession_GetClientIPAddress(pstStreamSession, aszClientIP, RTSP_IP_MAX_LEN);
    if (pstServerCtx->stListener.onClientConnect && strlen(aszClientIP) > 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "rtsp client  connected !!!\n");
        pstServerCtx->stListener.onClientConnect(&pstServerCtx->stListener, aszClientIP);
    }

    return HI_SUCCESS;

DES_SESSION:

    HI_RTSPSVR_StreamSession_Destroy(pstStreamSession);
    return s32Ret;
}

static HI_S32 RTSPSVR_CheckListNULL(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pStreamName)
{
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_RTSP_STREAM_SESSION_S* pstSessionInList = NULL;
    HI_BOOL bNeedWait = HI_FALSE;
    HI_U32 u32WaitCnt = 0;

    while (u32WaitCnt <= CHECK_WAIT_MAX_NUM)
    {
        RTSP_LOCK(pstServerCtx->sessionListLock);

        HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pSessionlist)
        {
            pstSessionInList = HI_LIST_ENTRY(pstPosNode, HI_RTSP_STREAM_SESSION_S, listPtr);
            if ( 0 == strncmp(pStreamName, pstSessionInList->aszStreamName, RTSP_MAX_STREAMNAME_LEN) )
            {
                bNeedWait = HI_TRUE;
                break;
            }
        }

        RTSP_UNLOCK(pstServerCtx->sessionListLock);

        if (bNeedWait)
        {
            hi_usleep(CHECK_WAIT_TIMEOUT);
            u32WaitCnt++;
            bNeedWait = HI_FALSE;
            continue;
        }
        else
        {
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

static HI_BOOL RTSPSVR_FindSessionValid(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pStreamName, HI_RTSP_STREAM_SESSION_S** pstSession)
{
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_RTSP_STREAM_SESSION_S* pstSessionInList = NULL;
    HI_BOOL bFound = HI_FALSE;
    RTSP_LOCK(pstServerCtx->sessionListLock);

    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pSessionlist)
    {
        pstSessionInList = HI_LIST_ENTRY(pstPosNode, HI_RTSP_STREAM_SESSION_S, listPtr);

        if ( 0 == strncmp(pStreamName, pstSessionInList->aszStreamName, RTSP_MAX_STREAMNAME_LEN) )
        {
            if (pstSessionInList->enSessState != HIRTSP_SESSION_STATE_STOP)
            {
                bFound = HI_TRUE;
                *pstSession = pstSessionInList;
                break;
            }
        }
    }

    RTSP_UNLOCK(pstServerCtx->sessionListLock);

    return bFound;
}

/*remove all the session related to the media stream if a stream is removed*/
static HI_S32 RTSPSVR_RemoveStreamSession(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pStreamName)
{
    HI_BOOL bFound = HI_TRUE;
    HI_RTSP_STREAM_SESSION_S* pstSessionInList = NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == pstServerCtx  || NULL == pStreamName)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_RemoveStreamSession param null \n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    while (bFound)
    {
        bFound = RTSPSVR_FindSessionValid(pstServerCtx, pStreamName, &pstSessionInList);


        if (!bFound)
        {
            break;
        }

        /*when remove a streamsession set stop state and wait for the destroy*/
        s32Ret = HI_RTSPSVR_StreamSession_Stop(pstSessionInList);
        if ( HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_StreamSession_Destroy failed ret:%d\n", s32Ret);
            return HI_ERR_RTSPSERVER_ERR_SESS_DESTROY_FAIL;
        }

    }

    s32Ret = RTSPSVR_CheckListNULL(pstServerCtx, pStreamName);
    if ( HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "remove session timeout, and ret:%d\n", s32Ret);
        return HI_ERR_RTSPSERVER_ERR_SESS_DESTROY_FAIL;
    }

    return HI_SUCCESS;
}


static HI_S32 RTSPSVR_OnClientConnection(HI_MW_PTR obj, HI_S32 s32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_RTSP_SERVER_CTX_S* pstServerCtx = (HI_RTSP_SERVER_CTX_S*)obj;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszStream[RTSP_MAX_STREAMNAME_LEN] = {0};
    HI_S32 StatCode = HIRTSP_STATUS_OK;
    HI_CHAR aszReply[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32 s32Cseq = 0;
    HI_S32 s32RespLen = 0;
    List_Head_S* pstPosNode = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    HI_BOOL bFound = HI_FALSE;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    RTSPSVR_CHECK_NULL_ERROR(pClientReq);
    HI_BOOL bGetStreamName = HI_TRUE;
    if (!HI_RTSPSVR_MSGParser_CheckValidRequest(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp request, just ignore this connection\n");
        StatCode = HIRTSP_STATUS_BAD_REQUEST;
        s32Ret = HI_ERR_RTSPSERVER_ERR_SESS_BAD_REQUEST;
        goto Failed;
    }

    s32Ret = HI_RTSPSVR_MSGParser_GetStreamName(pClientReq, aszStream, RTSP_MAX_STREAMNAME_LEN);
    if (s32Ret  != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " could not find  streamName\n");
        bGetStreamName = HI_FALSE;
    }

    s32Ret = HI_RTSPSVR_MSGParser_GetCSeq(pClientReq, &s32Cseq);
    if (s32Ret  != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp url, could not find  CSeq\n");
        StatCode = HIRTSP_STATUS_BAD_REQUEST;
        s32Ret = HI_ERR_RTSPSERVER_ERR_SESS_BAD_REQUEST;
        goto Failed;
    }

    if (!bGetStreamName)
    {
        if (RTSPSVR_AddStreamSession(pstServerCtx, pClientReq, u32ReqLen, s32SocketFd, NULL) != HI_SUCCESS)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_AddStreamSession failed\n");
            StatCode = HIRTSP_STATUS_INTERNAL_SERVER_ERROR;
            s32Ret = HI_ERR_RTSPSERVER_ERR_STREAM_ADD_SESS_FAIL;
            goto Failed;
        }
        return HI_SUCCESS;
    }

    //if find stream name ,start session with streamnode
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

    if (!bFound || NULL == pstStreamNode )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "invalid rtsp request, could not find correspond stream\n");
        StatCode = HIRTSP_STATUS_BAD_REQUEST;
        s32Ret = HI_ERR_RTSPSERVER_ERR_SESS_STREAM_NOT_FOUND;
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        goto Failed;
    }


    if (RTSPSVR_AddStreamSession(pstServerCtx, pClientReq, u32ReqLen, s32SocketFd, pstStreamNode) != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_AddStreamSession failed\n");
        StatCode = HIRTSP_STATUS_INTERNAL_SERVER_ERROR;
        s32Ret = HI_ERR_RTSPSERVER_ERR_STREAM_ADD_SESS_FAIL;
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        goto Failed;
    }

    RTSP_UNLOCK(pstServerCtx->streamListLock);

    return HI_SUCCESS;

Failed:
    HI_RTSPSVR_MSGParser_GetResponse(StatCode, s32Cseq, aszReply, RTSP_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply) ;
    if (s32RespLen > 0)
    {
        HI_RTSPSVR_StreamSession_Send(s32SocketFd, aszReply, s32RespLen);
    }

    HI_NETWORK_CloseSocket(&s32SocketFd);
    return s32Ret;
}


static HI_S32 RTSPSVR_CheckStreamCfg(HI_RTSP_SOURCE_S* pStreamSrc,  HI_CHAR* pchStreamName, HI_U32 u32BufSize)
{
    if (RTSP_MAX_STREAMNAME_LEN < strlen(pchStreamName) || 0 == strlen(pchStreamName))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver  HI_RTSPSVR_AddMediaStream streamname too long  max len :%d \n", RTSP_MAX_STREAMNAME_LEN);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (u32BufSize < RTSP_MIN_BUFFER_SIZE || u32BufSize > RTSP_MAX_BUFFER_SIZE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " param u32BufSize not in the range min:%d  max:%d!\n", RTSP_MIN_BUFFER_SIZE, RTSP_MAX_BUFFER_SIZE);
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (NULL == pStreamSrc->pstVideoSrc && NULL == pStreamSrc->pstAudioSrc)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "can not add a stream with no audio and no video\n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    if (NULL != pStreamSrc->pstAudioSrc && HI_TRACK_SOURCE_TYPE_AUDIO != pStreamSrc->pstAudioSrc->enTrackType)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "can not add a audio src with video type\n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }


    if (NULL != pStreamSrc->pstVideoSrc)
    {
        if (HI_TRACK_SOURCE_TYPE_VIDEO != pStreamSrc->pstVideoSrc->enTrackType)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "can not add a video src with audio type\n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
        if (HI_TRACK_VIDEO_CODEC_H264 != pStreamSrc->pstVideoSrc->unTrackSourceAttr.stVideoInfo.enCodecType && HI_TRACK_VIDEO_CODEC_H265 != pStreamSrc->pstVideoSrc->unTrackSourceAttr.stVideoInfo.enCodecType )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "not support video type :%d \n", pStreamSrc->pstVideoSrc->unTrackSourceAttr.stVideoInfo.enCodecType);
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
        if (NULL == pStreamSrc->pstVideoSrc->pfnSourceStart || NULL == pStreamSrc->pstVideoSrc->pfnSourceStop || NULL == pStreamSrc->pstVideoSrc->pfnRequestKeyFrame)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "video source call back null error \n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }

    }

    if (NULL != pStreamSrc->pstAudioSrc )
    {
        if (HI_TRACK_SOURCE_TYPE_AUDIO != pStreamSrc->pstAudioSrc->enTrackType)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "can not add a audio src with video type\n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
        if ( HI_TRACK_AUDIO_CODEC_AAC != pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G711A != pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G711Mu != pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_G726 != pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType &&
             HI_TRACK_AUDIO_CODEC_ADPCM != pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType
           )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "not support audio type :%d\n", pStreamSrc->pstAudioSrc->unTrackSourceAttr.stAudioInfo.enCodecType);
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }

        if (NULL == pStreamSrc->pstAudioSrc->pfnSourceStart || NULL == pStreamSrc->pstAudioSrc->pfnSourceStop)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "audio source call back null error \n");
            return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}
static HI_VOID RTSPSVR_GetOneStream(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszName)
{
    List_Head_S* pstPosNode = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    RTSP_LOCK(pstServerCtx->streamListLock);
    /* delete all the  session related to the mediastream*/
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode  , &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if (HI_FAILURE == snprintf_s(pszName, RTSP_MAX_STREAMNAME_LEN, RTSP_MAX_STREAMNAME_LEN - 1, "%s", pstStreamNode->name))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTSPSVR_GetOneStream pszName error\n");
        }
        break;
    }
    RTSP_UNLOCK(pstServerCtx->streamListLock);

    return;
}

static HI_BOOL RTSPSVR_FindOneStreamAndDel(HI_RTSP_SERVER_CTX_S* pstServerCtx, HI_CHAR* pchStreamName)
{
    HI_BOOL bFound = HI_FALSE;
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;

    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if (!strcasecmp(pstStreamNode->name, pchStreamName))
        {
            bFound = HI_TRUE;
            HI_List_Del(&(pstStreamNode->listPtr));
            free(pstStreamNode);
            pstStreamNode = NULL;
            pstServerCtx->s32StreamNum--;
            break;
        }
    }

    RTSP_UNLOCK(pstServerCtx->streamListLock);

    return bFound;
}

HI_S32 HI_RTSPSVR_WriteFrame(HI_MW_PTR handle, HI_Track_Source_Handle pTrackSrc, HI_RTSP_DATA_S* pstData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bFoundStream = HI_FALSE;

    HI_BOOL bFoundSession = HI_FALSE;
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    List_Head_S* pstPosNode = NULL;
    HI_RTSP_STREAM_SESSION_S* pstSessionInList = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    List_Head_S* pstTmpNode = NULL;

    RTSPSVR_CHECK_NULL_ERROR(handle);
    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_WriteFrame error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }
    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    RTSPSVR_CHECK_NULL_ERROR(pTrackSrc);
    RTSPSVR_CHECK_NULL_ERROR(pstData);
    s32Ret = RTSPSVR_Check_StreamData(pTrackSrc, pstData);
    if ( HI_SUCCESS != s32Ret )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver write frame data illegal\n");
        return s32Ret;
    }

    //find the stream node in the list
    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if ( pstStreamNode->pstVidStream == pTrackSrc ||  pstStreamNode->pstAudStream == pTrackSrc)
        {
            bFoundStream = HI_TRUE;
            break;
        }
    }

    RTSP_UNLOCK(pstServerCtx->streamListLock);

    if ( HI_TRUE != bFoundStream )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "stream not find, write frame fail\n");
        return HI_ERR_RTSPSERVER_ERR_STREAM_NOT_EXIST;
    }

    RTSP_LOCK(pstServerCtx->sessionListLock);

    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pSessionlist)
    {
        pstSessionInList = HI_LIST_ENTRY(pstPosNode, HI_RTSP_STREAM_SESSION_S, listPtr);
        if (pstSessionInList->pstVidStream == pTrackSrc || pstSessionInList->pstAudStream == pTrackSrc)
        {
            bFoundSession = HI_TRUE;
            /*write frame for the session*/
            s32Ret = HI_RTSPSVR_StreamSession_WriteFrame(pstSessionInList, pTrackSrc, pstData);
            if ( HI_SUCCESS != s32Ret)
            {
                RTSP_UNLOCK(pstServerCtx->sessionListLock);
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_WriteFrame failed ret:%x mediatype %d\n", s32Ret, pTrackSrc->enTrackType);
                return s32Ret;
            }
        }
    }

    RTSP_UNLOCK(pstServerCtx->sessionListLock);

    if ( HI_TRUE != bFoundSession )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "HI_RTSPSVR_WriteFrame can't find streamsrc in connect session\n");
        return HI_ERR_RTSPSERVER_ERR_SESS_NOT_EXISTED;
    }

    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_AddMediaStream(HI_MW_PTR handle, HI_RTSP_SOURCE_S* pStreamSrc, HI_CHAR* pchStreamName, HI_U32 u32BufSize)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    List_Head_S* pstPosNode = NULL;
    HI_LIVE_MEDIA_STREAM_NODE_S* pstStreamNode = NULL;
    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_AddMediaStream errror\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }


    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    RTSPSVR_CHECK_NULL_ERROR(pchStreamName);
    RTSPSVR_CHECK_NULL_ERROR(pStreamSrc);


    if (pstServerCtx->s32StreamNum >= RTSP_MAX_STREAM_NUM)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver  HI_RTSPSVR_AddMediaStream stream num reach max :%d \n", RTSP_MAX_STREAM_NUM);
        return HI_ERR_RTSPSERVER_ERR_STREAM_ADD_FAIL;
    }


    s32Ret = RTSPSVR_CheckStreamCfg(pStreamSrc, pchStreamName, u32BufSize);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver  HI_RTSPSVR_AddMediaStream check param illegal \n");
        return s32Ret;
    }


    /*find the stream node in the list */
    RTSP_LOCK(pstServerCtx->streamListLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pStreamlist)
    {
        pstStreamNode = HI_LIST_ENTRY(pstPosNode, HI_LIVE_MEDIA_STREAM_NODE_S, listPtr);
        if ( !strcasecmp(pstStreamNode->name, pchStreamName))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "media source streamname alreay exist\n");
            RTSP_UNLOCK(pstServerCtx->streamListLock);
            return HI_ERR_RTSPSERVER_ERR_STREAM_EXISTED;
        }
    }

    /* stream node not exist then create and add to the list*/
    pstStreamNode = (HI_LIVE_MEDIA_STREAM_NODE_S*)malloc(sizeof(HI_LIVE_MEDIA_STREAM_NODE_S));
    if (!pstStreamNode)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc rtsp HiLiveMediaStreamNode failed\n");
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_ERR_RTSPSERVER_ERR_MALLOC_FAIL;
    }

    if (HI_SUCCESS != memset_s(pstStreamNode, sizeof(HI_LIVE_MEDIA_STREAM_NODE_S), 0x00, sizeof(HI_LIVE_MEDIA_STREAM_NODE_S)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set pstStreamNode fail\n");
        free(pstStreamNode);
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_FAILURE;
    }
    if (HI_SUCCESS != strncpy_s(pstStreamNode->name, RTSP_MAX_STREAMNAME_LEN , pchStreamName, RTSP_MAX_STREAMNAME_LEN - 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "string cpy  name fail\n");
        free(pstStreamNode);
        RTSP_UNLOCK(pstServerCtx->streamListLock);
        return HI_FAILURE;
    }
    pstStreamNode->name[RTSP_MAX_STREAMNAME_LEN - 1] = '\0';
    pstStreamNode->pstAudStream = pStreamSrc->pstAudioSrc;
    pstStreamNode->pstVidStream = pStreamSrc->pstVideoSrc;
    pstStreamNode->u32BufSize = u32BufSize;
    pstStreamNode->u32AudStartCnt = 0;
    pstStreamNode->u32VidStartCnt = 0;

    pstServerCtx->s32StreamNum++;

    HI_List_Add(&(pstStreamNode->listPtr), &(pstServerCtx->pStreamlist));
    RTSP_UNLOCK(pstServerCtx->streamListLock);

    return HI_SUCCESS;
}


HI_S32 HI_RTSPSVR_RemoveMediaStream(HI_MW_PTR handle, HI_CHAR* pchStreamName)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    HI_BOOL bFound = HI_FALSE;
    HI_S32 s32Ret = HI_SUCCESS;
    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_RemoveMediaStream error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }

    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);
    RTSPSVR_CHECK_NULL_ERROR(pchStreamName);


    if (RTSP_MAX_STREAMNAME_LEN < strlen(pchStreamName) || 0 == strlen(pchStreamName))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver  streamname error len \n");
        return HI_ERR_RTSPSERVER_ILLEGAL_PARAM;
    }

    /*when remove a media stream destroy the related connections if existed*/
    s32Ret = RTSPSVR_RemoveStreamSession(pstServerCtx, pchStreamName);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "could not find remove stream session for stream: %s ret:%d\n", pchStreamName, s32Ret);
        return HI_ERR_RTSPSERVER_ERR_STREAM_REMOVE_FAIL;
    }

    bFound = RTSPSVR_FindOneStreamAndDel(pstServerCtx, pchStreamName);
    if (!bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "could not find correspond media stream\n");
        return HI_ERR_RTSPSERVER_ERR_STREAM_NOT_EXIST;
    }

    return HI_SUCCESS;
}


HI_S32 HI_RTSPSVR_Start(HI_MW_PTR handle)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);
    HI_RTSP_SERVER_CTX_S* pstServerCtx = NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_Start error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }

    pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);

    if (pstServerCtx->bStarted)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtspserver already started  \n");
        return HI_ERR_RTSPSERVER_ERR_START_LISTENER_AGAIN;
    }

    s32Ret = HI_NETWORK_Listener_Start(pstServerCtx->hNetListener);//创建套接字，监听客户端的链接
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtsp start listener failed ret:%d \n", s32Ret);
        return HI_ERR_RTSPSERVER_ERR_START_LISTENER_FAIL;
    }
    //注册链接成功后要执行的函数
    HI_NETWORK_Listener_RegisterCallback(pstServerCtx->hNetListener, RTSPSVR_OnClientConnection, (HI_MW_PTR)pstServerCtx);

    pstServerCtx->bStarted = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_Stop(HI_MW_PTR handle)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszTmpName[RTSP_MAX_STREAMNAME_LEN];


    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_Stop error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }

    HI_RTSP_SERVER_CTX_S* pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);


    if (!pstServerCtx->bStarted)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtspserver have not been started\n");
        return HI_ERR_RTSPSERVER_ERR_STOP_LISTENER_NOT_STARTED;
    }

    s32Ret = HI_NETWORK_Listener_Stop(pstServerCtx->hNetListener);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtsp start listener failed ret:%d \n", s32Ret);
        return HI_ERR_RTSPSERVER_ERR_STOP_LISTENER_FAIL;
    }

    while (0 != pstServerCtx->s32StreamNum)
    {
        RTSPSVR_GetOneStream(pstServerCtx, aszTmpName);
        s32Ret = RTSPSVR_RemoveStreamSession(pstServerCtx, aszTmpName);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "could not find remove stream session for stream: %s\n", aszTmpName);
            return HI_ERR_RTSPSERVER_ERR_STREAM_REMOVE_SESS_FAIL;
        }

        RTSPSVR_FindOneStreamAndDel(pstServerCtx, aszTmpName);
    }

    pstServerCtx->bStarted = HI_FALSE;

    return HI_SUCCESS;
}


HI_S32 HI_RTSPSVR_SetListener(HI_MW_PTR handle, HI_SERVER_STATE_LISTENER_S* pstListener)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_SetListener error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }

    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }


    HI_RTSP_SERVER_CTX_S* pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;

    RTSPSVR_CHECK_NULL_ERROR(pstListener);
    RTSPSVR_CHECK_NULL_ERROR(pstListener->onClientConnect);
    RTSPSVR_CHECK_NULL_ERROR(pstListener->onClientDisconnect);
    RTSPSVR_CHECK_NULL_ERROR(pstListener->onServerError);
    pstServerCtx->stListener.onClientConnect = pstListener->onClientConnect;
    pstServerCtx->stListener.onClientDisconnect = pstListener->onClientDisconnect;
    pstServerCtx->stListener.onServerError = pstListener->onServerError;

    return HI_SUCCESS;
}

HI_S32 HI_RTSPSVR_Create(HI_MW_PTR* pHandle, HI_RTSP_CONFIG_S* pstRTSPConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RTSP_SERVER_CTX_S* pstServerCtx;
    RTSPSVR_CHECK_NULL_ERROR(pHandle);
    RTSPSVR_CHECK_NULL_ERROR(pstRTSPConfig);
    s32Ret = RTSPSVR_CheckRTSPconfig(pstRTSPConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " config param null !\n");
        return s32Ret;
    }

    if (NULL != g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, " RTSPSERVER  already created do not create again!\n");
        return HI_ERR_RTSPSERVER_ERR_CREATE_AGAIN;
    }

    pstServerCtx =  (HI_RTSP_SERVER_CTX_S*)malloc(sizeof(HI_RTSP_SERVER_CTX_S));
    if (!pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc HI_RTSP_SERVER_CTX_S failed\n");
        return HI_ERR_RTSPSERVER_ERR_MALLOC_FAIL;
    }

    if (HI_SUCCESS != memset_s(pstServerCtx, sizeof(HI_RTSP_SERVER_CTX_S), 0x00, sizeof(HI_RTSP_SERVER_CTX_S)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "SET HI_RTSP_SERVER_CTX_S failed\n");
        free(pstServerCtx);
        return HI_FAILURE;
    }

    pthread_mutex_init(&pstServerCtx->sessionListLock,  NULL);
    pthread_mutex_init(&pstServerCtx->streamListLock,  NULL);
    pstServerCtx->s32MaxConNum = pstRTSPConfig->s32MaxConnNum;
    pstServerCtx->s32ListenPort = pstRTSPConfig->s32ListenPort;
    pstServerCtx->s32MaxPayload = pstRTSPConfig->s32MaxPayload;
    pstServerCtx->s32PacketLen = pstRTSPConfig->s32PacketLen;
    pstServerCtx->s32Timeout =  pstRTSPConfig->s32Timeout;
    pstServerCtx->s32UserNum = 0;
    pstServerCtx->s32StreamNum = 0;

    HI_LIST_INIT_HEAD_PTR(&pstServerCtx->pStreamlist);
    HI_LIST_INIT_HEAD_PTR(&pstServerCtx->pSessionlist);
    s32Ret = HI_NETWORK_Listener_Create(pstServerCtx->s32ListenPort, &pstServerCtx->hNetListener);
    if ( HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtsp create listener failed ret:%d\n", s32Ret);
        pthread_mutex_destroy(&pstServerCtx->streamListLock);
        pthread_mutex_destroy(&pstServerCtx->sessionListLock);
        free(pstServerCtx);
        pstServerCtx = NULL;
        return HI_ERR_RTSPSERVER_ERR_CREATE_LISTENER_FAIL;
    }


    *pHandle = (HI_MW_PTR)pstServerCtx;
    g_pstServerCtx = pstServerCtx;
    return HI_SUCCESS;

}

HI_S32 HI_RTSPSVR_Destroy(HI_MW_PTR handle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszTmpName[RTSP_MAX_STREAMNAME_LEN];


    RTSPSVR_CHECK_NULL_ERROR(handle);
    if ( NULL == g_pstServerCtx )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtsperver not created HI_RTSPSVR_Destroy error\n");
        return HI_ERR_RTSPSERVER_ERR_NOT_CREATE;
    }


    if (handle != (HI_MW_PTR)g_pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "rtspserver handle invalid \n");
        return HI_ERR_RTSPSERVER_HANDLE_INVALID;
    }


    HI_RTSP_SERVER_CTX_S* pstServerCtx = (HI_RTSP_SERVER_CTX_S*)handle;
    RTSPSVR_CHECK_NULL_ERROR(pstServerCtx);

    while (0 != pstServerCtx->s32StreamNum)
    {
        RTSPSVR_GetOneStream(pstServerCtx, aszTmpName);
        s32Ret = RTSPSVR_RemoveStreamSession(pstServerCtx, aszTmpName);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "could not find remove stream session for stream: %s\n", aszTmpName);
            return HI_ERR_RTSPSERVER_ERR_STREAM_REMOVE_SESS_FAIL;
        }

        RTSPSVR_FindOneStreamAndDel(pstServerCtx, aszTmpName);
    }

    pthread_mutex_destroy(&pstServerCtx->streamListLock);
    pthread_mutex_destroy(&pstServerCtx->sessionListLock);

    s32Ret = HI_NETWORK_Listener_Destroy(pstServerCtx->hNetListener);
    if ( HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Rtsp destroy listener failed ret:%d \n", s32Ret);
        return HI_ERR_RTSPSERVER_ERR_DESTROY_LISTENER_FAIL;
    }

    free(pstServerCtx);
    pstServerCtx = NULL;
    g_pstServerCtx = NULL;

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
