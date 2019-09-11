/**
* @file    hi_product_netctrl.c
* @brief   product netctrl module interface implementation
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/15
* @version   1.0

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "product_netctrl_inner.h"
#include "hi_product_netctrl.h"
#include "hi_filetrans.h"
#include "hi_exif.h"
#include "hi_gsensormng.h"
#include "hi_product_param.h"
#include "hi_server_log.h"
#include "hi_upgrade.h"

#include <sys/select.h>


#if defined(CONFIG_WIFI_ON)
#include "hi_hal_wifi.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static PDT_NETCTRL_CLIENT_INFO_S s_stNETCTRLClient[NETCTRL_MAX_WEBSERVER_CLIENT_NUM] = {{HI_FALSE, {0}},};
static PDT_NETCTRL_MESSAGE_CONTEXT s_stNETCTRLMessageCtx = {.bMsgProcessed = HI_TRUE, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static HI_S32 s_s32NETCTRLMsgResult = HI_FAILURE;
static sem_t s_NETCTRLSem;
static HI_BOOL s_bNETCTRLSdIsReady = HI_FALSE;
static HI_HANDLE s_TimedTaskHdl;
static HI_PDT_UPGRADE_RECEIVE_INFO_S s_stReceiveInfo[NETCTRL_MAX_RECEIVE_COUNT] = {};

HI_S32 PDT_NETCTRL_isMdReady(HI_VOID)
{
    HI_BOOL bMdReady = HI_TRUE;

#ifdef CONFIG_MOTIONDETECT_ON
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32MdSensitivity = 0;
    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, Front_CAM_ID, HI_PDT_PARAM_MD_SENSITIVITY, &s32MdSensitivity);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

    if(0 != s32MdSensitivity)
    {
        //get event history
        HI_EVENT_S stEvent;
        stEvent.arg1 = 0;
        s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE, &stEvent);
        if(((HI_SUCCESS != s32Ret) ||(0 == stEvent.arg1)))
        {
            bMdReady = HI_FALSE;
        }
        else /**motion detect occur*/
        {
            bMdReady = HI_TRUE;
        }
    }
    else
    {
        bMdReady = HI_TRUE;
    }

#endif
    MLOGI("MD Status[%d]\n", bMdReady);
    return bMdReady;
}

HI_BOOL PDT_NETCTRL_IsClientConnecting(HI_VOID)
{
    HI_S32 i = 0;

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if ( s_stNETCTRLClient[i].bConnected && strnlen(s_stNETCTRLClient[i].szIPv4Addr, sizeof(s_stNETCTRLClient[i].szIPv4Addr)) > 0)
        {
            return HI_TRUE;
        }
    }

    return HI_FALSE;
}

static HI_S32 PDT_NETCTRL_EnableUsb(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_USB_MODE_E enUsbMode;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &enUsbMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    MLOGI("Enable UsbMode[%d]\n", enUsbMode);
    s32Ret = HI_USB_SetMode(enUsbMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_DisableUsb(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_USB_MODE_E enUsbMode;
    s32Ret = HI_USB_GetMode(&enUsbMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_USB_STATE_E enUsbState;
    s32Ret = HI_USB_GetState(&enUsbState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if((HI_USB_MODE_CHARGE != enUsbMode) && (HI_USB_STATE_OUT != enUsbState))
    {
        MLOGI("current usb_mode[%d] usb_state[%d]; can not connect apk!\n", enUsbMode, enUsbState);
        return HI_FAILURE;
    }
    MLOGI("Disable UsbMode[%d], set UsbMode[%d]\n", enUsbMode, HI_USB_MODE_CHARGE);
    s32Ret = HI_USB_SetMode(HI_USB_MODE_CHARGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;

}


/** Publish event to EventHub*/
static HI_VOID PDT_NETCTRL_PublishEvent(HI_EVENT_S*  pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_EVTHUB_Publish(pstEvent);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("publish event fail,eventid: %d \n", pstEvent->EventID);
    }

    return;
}


HI_S32 PDT_NETCTRL_SendSyncMsg(HI_MESSAGE_S* pstMsg, HI_S32* ps32Result)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MUTEX_LOCK(s_stNETCTRLMessageCtx.MsgMutex);

    if ( !s_stNETCTRLMessageCtx.bMsgProcessed)
    {
        MLOGE("Current Msg not finished\n");
        HI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return HI_FAILURE;
    }

    s_stNETCTRLMessageCtx.bMsgProcessed = HI_FALSE;
    s_stNETCTRLMessageCtx.stMsg.what = pstMsg->what;
    s_stNETCTRLMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stNETCTRLMessageCtx.stMsg.arg2 = pstMsg->arg2;
    s_stNETCTRLMessageCtx.stMsg.s32Result = HI_FAILURE;
    memcpy(s_stNETCTRLMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stNETCTRLMessageCtx.stMsg.aszPayload));

    s32Ret = HI_PDT_STATEMNG_SendMessage(pstMsg);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%#x\n", s32Ret);
        s_stNETCTRLMessageCtx.bMsgProcessed = HI_TRUE;
        *ps32Result = HI_FAILURE;
        HI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return HI_FAILURE;
    }

    while ((HI_SUCCESS != sem_wait(&s_NETCTRLSem)) && (errno == EINTR));

    *ps32Result = s_s32NETCTRLMsgResult;
    s_stNETCTRLMessageCtx.bMsgProcessed = HI_TRUE;
    HI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
    return HI_SUCCESS;
}


static HI_S32 PDT_NETCTRL_DistribLink(HI_S32 s32LinkFd, HI_CHAR* pMsgBuffAddr, HI_U32 u32MsgLen)
{
    HI_S32 s32AddrLen = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    struct sockaddr_in struAddr;

    if (0 > s32LinkFd || NULL == pMsgBuffAddr || 0 == u32MsgLen)
    {
        return WEBSERVER_NO_DISTRIBLINK;
    }

    s32AddrLen = sizeof(struct sockaddr);
    s32Ret = getpeername(s32LinkFd, (struct sockaddr*)&struAddr, (socklen_t*)&s32AddrLen);

    if (s32Ret != HI_SUCCESS)
    {
        return WEBSERVER_NO_DISTRIBLINK;
    }

    pMsgBuffAddr[u32MsgLen] = '\0';

    if ((strstr(pMsgBuffAddr, "Keep-Alive") != NULL ) || (strstr(pMsgBuffAddr, "keep-alive") != NULL ))
    {
        if ((NULL == strstr(pMsgBuffAddr, "cgi-bin")) &&
            (NULL == strstr(pMsgBuffAddr, "Range:")) &&
            (NULL != strstr(pMsgBuffAddr, ".THM")))
        {
            return HI_FILETRANS_DistribLink(s32LinkFd, pMsgBuffAddr, u32MsgLen);
        }
    }

    return WEBSERVER_NO_DISTRIBLINK;
}


HI_S32 PDT_NETCTRL_TCP_Connect(HI_CHAR* pzsIPv4Addr, HI_U32 u32Port)
{
    HI_S32 iResult = 0;
    HI_S32 ret = 0;
    HI_S32 ConnSock;
    struct sockaddr_in Service = {};
    struct timeval tm;
    fd_set set, rset;
    socklen_t len = sizeof(HI_S32);
    HI_S32 error = -1;
    HI_S32 flag = 0;

    /* Create a SOCKET for connecting to server */
    ConnSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (ConnSock < 0)
    {
        MLOGD("socket error:%d %s\n", errno, strerror(errno));
        return HI_FAILURE;
    }

    /*The sockaddr_in structure specifies the address family, */
    /* IP address, and port of the server to be connected to. */
    memset(&Service, 0, sizeof(struct sockaddr_in));
    Service.sin_family = AF_INET;
    Service.sin_port = htons(u32Port);

    if (inet_pton(AF_INET, pzsIPv4Addr, &Service.sin_addr) <= 0)
    {
        MLOGD("ip:%s inet_pton error:%d %s\n", pzsIPv4Addr, errno, strerror(errno));
        close(ConnSock);
        return HI_FAILURE;
    }

    /*Set connect use unblock type*/
    flag = fcntl(ConnSock, F_GETFL, 0);
    ret = fcntl(ConnSock, F_SETFL, (HI_U32)flag | O_NONBLOCK);

    if (ret == -1)
    {
        close(ConnSock);
        return HI_FAILURE;
    }

    /*Connect to server.*/
    if (connect(ConnSock, (struct sockaddr*)&Service, sizeof(Service)) == -1)
    {
        if (errno != EINPROGRESS)
        {
            MLOGD("connect error:%d %s\n", errno, strerror(errno));
            close(ConnSock);
            return HI_FAILURE;
        }

        tm.tv_sec = CLIENT_CONNECT_TIMEOUT;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_ZERO(&rset);
        FD_SET(ConnSock, &set);
        FD_SET(ConnSock, &rset);
        ret = select(ConnSock + 1, &rset, &set, NULL, &tm);

        if (0 > ret)
        {
            MLOGD("select error:%d %s,Sockfd=%d\n", errno, strerror(errno), ConnSock);
            iResult = HI_FAILURE;
        }
        else if (0 == ret)
        {
            MLOGD("connect %s timeout. error:%d %s\n", pzsIPv4Addr, errno, strerror(errno));
            iResult = HI_FAILURE;
        }
        else
        {
            ret = getsockopt(ConnSock, SOL_SOCKET, SO_ERROR, (HI_VOID*)&error, &len);

            if (ret == -1)
            {
                iResult = HI_FAILURE;
            }

            if (error == 0)
            {
                iResult = HI_SUCCESS;
            }
            else
            {
                MLOGD("getsockopt SO_ERROR=%d errno:%d %s\n", error, errno, strerror(errno));
                iResult = HI_FAILURE;
            }
        }
    }

    /*set to block*/
    flag &= ~O_NONBLOCK;
    ret = fcntl(ConnSock, F_SETFL, flag);

    if (ret == -1)
    {
        close(ConnSock);
        return HI_FAILURE;
    }

    if (iResult != HI_SUCCESS)
    {
        close(ConnSock);
        return HI_FAILURE;
    }

    return ConnSock;
}

HI_S32 PDT_NETCTRL_TCP_Send(HI_S32 s32SockFd, const HI_CHAR* SendBuff, HI_S32 s32Sendlen, HI_S32 s32Timeout)
{
    HI_S32   s32SendSize = 0;
    fd_set   struWriteFdSet;
    HI_S32   s32SendRtn, s32SelRtn;
    struct   timeval struTimeout;
    HI_S32   s32Retry = 0;

    FD_ZERO(&struWriteFdSet);
    FD_SET(s32SockFd, &struWriteFdSet);
    struTimeout.tv_sec = s32Timeout;
    struTimeout.tv_usec = 0;
    s32SelRtn = select(s32SockFd + 1, NULL, &struWriteFdSet, NULL, &struTimeout);

    if (s32SelRtn <= 0)
    {
        MLOGD("select error:%d %s", errno, strerror(errno));
        return HI_FAILURE;
    }

    while (s32SendSize < s32Sendlen)
    {
        s32SendRtn = send(s32SockFd, SendBuff + s32SendSize, s32Sendlen - s32SendSize, 0);

        if (s32SendRtn <= 0)
        {
            if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
            {
                HI_usleep(5000);
                s32Retry++;

                if (s32Retry < 10)
                {
                    continue;
                }
            }

            MLOGD("send error:%d %s", errno, strerror(errno));
            return HI_FAILURE;
        }

        s32SendSize += s32SendRtn;
    }

    return HI_SUCCESS;
}


HI_S32 PDT_NETCTRL_EventToString(HI_EVENT_S* pstEvent, HI_CHAR* pszBuffer, HI_U32 u32BufLen)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szEventID[32] = {};
    HI_CHAR szArg1[32] = {};
    HI_CHAR szArg2[32] = {};
    HI_CHAR szResult[32] = {};
    HI_CHAR payLoad[32] = {};
    HI_S32* ps32Value = NULL;

    s32Ret = PDT_NETCTRL_GetEventIDDescByValue(pstEvent->EventID, szEventID, sizeof(szEventID));
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    snprintf(szResult, sizeof(szResult), "%s",
             HI_SUCCESS == pstEvent->s32Result ? PDT_NETCTRL_SUCCESS : PDT_NETCTRL_FAILURE);

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SETTING:
            /*arg1: param type */
            s32Ret = PDT_NETCTRL_GetParamTypeDescByValue(szArg1, pstEvent->arg1, sizeof(szArg1));
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            /* arg2: WorkMode */
            s32Ret = PDT_NETCTRL_GetWorkModeDescByValue(szArg2, pstEvent->arg2, sizeof(szArg2));
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            /* payload:  setting value*/
            s32Ret = PDT_NETCTRL_GetParamValueDesc(pstEvent->arg2, pstEvent->arg1, payLoad, sizeof(payLoad));

            if (s32Ret)
            {
                MLOGI("PDT_NETCTRL_GetParamValueDesc(%d,%d) returned %#x\n", pstEvent->arg2, pstEvent->arg1, s32Ret);
            }

            break;

        case HI_EVENT_PDT_PARAM_CHANGE:
            /*arg1: param type */
            s32Ret = PDT_NETCTRL_GetParamTypeDescByValue(szArg1, pstEvent->arg1, sizeof(szArg1));
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_EVENT_STATEMNG_VO_SWITCH:
            /* payload: PreviewCam ID */
            ps32Value = (HI_S32*)(pstEvent->aszPayload);
            snprintf(payLoad, sizeof(payLoad), "%d", *ps32Value);
            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
        case HI_EVENT_STATEMNG_START:
        case HI_EVENT_STATEMNG_STOP:
        case HI_EVENT_STATEMNG_EMR_END:
        case HI_EVENT_STATEMNG_EMR_BEGIN:
        case HI_EVENT_STATEMNG_TRIGGER:
            /* arg2: WorkMode */
            s32Ret = PDT_NETCTRL_GetWorkModeDescByValue(szArg2, pstEvent->arg2, sizeof(szArg2));
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STATEMNG_SD_FORMAT:
        case HI_EVENT_GSENSORMNG_COLLISION:
            /*this events don't need  arg1 arg2 result payload,  just an eventID */
            break;
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
        case HI_EVENT_STORAGEMNG_FS_CHECKING:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNTED:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            s32Ret = PDT_NETCTRL_GetSdPromptInfo(payLoad, sizeof(payLoad));
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        {
            HI_EVENT_S stEvent = {};
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_DEV_ERROR, &stEvent);

            if (s32Ret != HI_SUCCESS)
            {
                memcpy(payLoad, "SDPROMPT_AVAILABLE", sizeof(payLoad));
                return HI_SUCCESS;
            }

            if (stEvent.arg1 > 3)
            {
                memcpy(payLoad, "SDPROMPT_DAMAGED_CHANGE", sizeof(payLoad));
                return HI_SUCCESS;
            }

            break;
        }
#ifdef CONFIG_MOTIONDETECT_ON
        case HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE:
            /* payload: PreviewCam ID */
            snprintf(szArg1, sizeof(szArg1), "%d", pstEvent->arg1);
            break;
#endif
        default:
            break;
    }


    snprintf(pszBuffer, u32BufLen, "{\"eventid\":\"%s\",\"arg1\":\"%s\",\"arg2\":\"%s\",\"result\":\"%s\",\"payload\":\"%s\"}",
             szEventID, szArg1, szArg2, szResult, payLoad);

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_NotifyToClient(const HI_CHAR* pszMsgContent)
{
    HI_S32 s32Retry = 0;
    HI_S32 i = 0;
    HI_S32 s32SockFd = -1;
    HI_S32 s32Ret = 0;
    HI_EVENT_S stEvent = {};

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if ( !s_stNETCTRLClient[i].bConnected)
        {
            continue;
        }

        s32Retry = 0;

        while ( s32Retry < 5 )
        {
            /*1.construct short link*/
            s32SockFd = PDT_NETCTRL_TCP_Connect(s_stNETCTRLClient[i].szIPv4Addr, CLIENT_MSG_RECV_PORT);

            if (s32SockFd >= 0)
            {
                s32Ret = PDT_NETCTRL_TCP_Send(s32SockFd, pszMsgContent, strnlen(pszMsgContent, EVENT_ID_MAX_LEN), CLIENT_CONNECT_TIMEOUT);

                if (HI_SUCCESS != s32Ret)
                {
                    /* client disconnectd*/
                    s_stNETCTRLClient[i].bConnected = HI_FALSE;

                    stEvent.EventID = HI_EVENT_NETCTRL_CLIENT_DISCONNECTED;
                    PDT_NETCTRL_PublishEvent(&stEvent);

                    MLOGI("PDT_NETCTRL_TCP_Send failed,ClientIP[%s] Disconnect!\n", s_stNETCTRLClient[i].szIPv4Addr);
                }

                MLOGD("send message %s to client [%s] return %#x\n", pszMsgContent, s_stNETCTRLClient[i].szIPv4Addr, s32Ret);

                close(s32SockFd);
                break;
            }

            s32Retry++;

        }

        if (s32Retry >= 5)
        {
            /* client disconnectd */
            s_stNETCTRLClient[i].bConnected = HI_FALSE;

            stEvent.EventID = HI_EVENT_NETCTRL_CLIENT_DISCONNECTED;
            PDT_NETCTRL_PublishEvent(&stEvent);
            MLOGI("PDT_NETCTRL_TCP_Connect failed,ClientIP[%s] Disconnect!\n", s_stNETCTRLClient[i].szIPv4Addr);
        }

    }

    return HI_SUCCESS;
}


HI_S32 PDT_NETCTRL_EnableCheckConnect(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_ATTR_S stTimeTskAttr;

    s32Ret = HI_TIMEDTASK_GetAttr(s_TimedTaskHdl, &stTimeTskAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stTimeTskAttr.bEnable = HI_TRUE;
    s32Ret = HI_TIMEDTASK_SetAttr(s_TimedTaskHdl, &stTimeTskAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_TIMEDTASK_ResetTime(s_TimedTaskHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_DIsableCheckConnect(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_ATTR_S stTimeTskAttr;

    s32Ret = HI_TIMEDTASK_GetAttr(s_TimedTaskHdl, &stTimeTskAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stTimeTskAttr.bEnable = HI_FALSE;
    s32Ret = HI_TIMEDTASK_SetAttr(s_TimedTaskHdl, &stTimeTskAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_CheckClientConnect(HI_VOID* pvPrivData)
{
    HI_S32 s32Retry = 0;
    HI_S32 i = 0;
    HI_S32 s32SockFd = -1;
    HI_S32 s32Ret = 0;
    HI_BOOL bHaveConnected = HI_FALSE;

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if ( !s_stNETCTRLClient[i].bConnected)
        {
            continue;
        }

        s32Retry = 0;

        while ( s32Retry < 5 )
        {
            /*1.construct short link*/
            s32SockFd = PDT_NETCTRL_TCP_Connect(s_stNETCTRLClient[i].szIPv4Addr, CLIENT_MSG_RECV_PORT);

            if (s32SockFd >= 0)
            {
                close(s32SockFd);
                break;
            }

            s32Retry++;

        }

        if (s32Retry >= 5)
        {
            /* client disconnectd*/
            s_stNETCTRLClient[i].bConnected = HI_FALSE;
            HI_EVENT_S stEvent = {};
            stEvent.EventID = HI_EVENT_NETCTRL_CLIENT_DISCONNECTED;
            PDT_NETCTRL_PublishEvent(&stEvent);
            MLOGI("Check ClientIP[%s] failed, Disconnect now!\n", s_stNETCTRLClient[i].szIPv4Addr);
        }

    }


    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if (s_stNETCTRLClient[i].bConnected)
        {
            bHaveConnected = HI_TRUE;
            break;
        }
    }

    if (!bHaveConnected)
    {

        s32Ret = PDT_NETCTRL_DIsableCheckConnect();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if ((!stWorkModeState.bRunning) && (HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
            && (PDT_NETCTRL_isMdReady()))
        {

            if (stWorkModeState.bStateMngInProgress)
            {
                MLOGE("StateMng in progress,and Start record failed!\n");
                return HI_FAILURE;
            }

            HI_MESSAGE_S stMessage;
            HI_S32  s32Result = HI_FAILURE;
            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_START;
            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

            if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
            {
                MLOGE("Disconnet client, and Start record failed!\n");
                return HI_FAILURE;
            }

        }

        if(!stWorkModeState.bEMRRecord)
        {
            s32Ret = PDT_NETCTRL_EnableUsb();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

    }
    else
    {
        PDT_NETCTRL_EnableCheckConnect();
    }

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if ((!s_stNETCTRLClient[i].bConnected) && (strnlen(s_stNETCTRLClient[i].szIPv4Addr, sizeof(s_stNETCTRLClient[i].szIPv4Addr)) > 0))
        {
            MLOGI("Clear client IP[%s]!\n", s_stNETCTRLClient[i].szIPv4Addr);
            memset(&s_stNETCTRLClient[i].szIPv4Addr, 0, CLIENT_IP_LEN);
            break;
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_AddClientIP(HI_CHAR* pszIpAddr)
{
    HI_S32 i = 0;

    if (NULL == pszIpAddr)
    {
        MLOGE("pszIpAddr is NULL\n");
        return HI_FAILURE;
    }

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if (s_stNETCTRLClient[i].bConnected
            && 0 == strncmp(s_stNETCTRLClient[i].szIPv4Addr, pszIpAddr, sizeof(s_stNETCTRLClient[i].szIPv4Addr)))
        {
            return HI_SUCCESS;  /*already added*/
        }
    }

    if (NETCTRL_MAX_WEBSERVER_CLIENT_NUM == i)
    {
        for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
        {
            if (!s_stNETCTRLClient[i].bConnected)
            {
                snprintf(s_stNETCTRLClient[i].szIPv4Addr,sizeof(s_stNETCTRLClient[i].szIPv4Addr),pszIpAddr);
                s_stNETCTRLClient[i].bConnected = HI_TRUE;
                MLOGI("Connected client IP[%s]!\n", s_stNETCTRLClient[i].szIPv4Addr);
                return HI_SUCCESS;
            }
        }

        if (NETCTRL_MAX_WEBSERVER_CLIENT_NUM == i)
        {
            MLOGE("Reach the maximum number(%d) of connections\n", NETCTRL_MAX_WEBSERVER_CLIENT_NUM);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


HI_S32 PDT_NETCTRL_EventProc(HI_EVENT_S* pstEvent, HI_VOID* pargv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szBuffer[EVENT_ID_MAX_LEN] = {};

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);

    if (!s_stNETCTRLMessageCtx.bMsgProcessed)
    {
        if ((s_stNETCTRLMessageCtx.stMsg.what == pstEvent->EventID)
            && (s_stNETCTRLMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stNETCTRLMessageCtx.stMsg.arg2 == pstEvent->arg2))
        {
            s_s32NETCTRLMsgResult = pstEvent->s32Result;
            s32Ret = sem_post(&s_NETCTRLSem);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            s_bNETCTRLSdIsReady = HI_FALSE;
            break;

        case HI_EVENT_STATEMNG_SD_AVAILABLE:
            s_bNETCTRLSdIsReady = HI_TRUE;
            break;

        default:
            break;
    }

    s32Ret = PDT_NETCTRL_EventToString(pstEvent, szBuffer, sizeof(szBuffer));
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_NETCTRL_NotifyToClient(szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_SubscribeEvents(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_SUBSCRIBER_S stSubscriber = {"NETCTRL", PDT_NETCTRL_EventProc, NULL, HI_FALSE};
    HI_MW_PTR SubscriberHdl = NULL;
    HI_EVENT_ID aEvents[] =
    {
        HI_EVENT_STATEMNG_SWITCH_WORKMODE,
        HI_EVENT_STATEMNG_LIVE_SWITCH,
        HI_EVENT_STATEMNG_VO_SWITCH,
        HI_EVENT_STATEMNG_START,
        HI_EVENT_STATEMNG_STOP,
        HI_EVENT_STATEMNG_TRIGGER,
        HI_EVENT_STATEMNG_SETTING,
        HI_EVENT_STATEMNG_POWEROFF,
        HI_EVENT_STATEMNG_SD_AVAILABLE,
        HI_EVENT_STATEMNG_SD_UNAVAILABLE,
        HI_EVENT_STATEMNG_SD_FORMAT,
        HI_EVENT_STATEMNG_EMR_END,
        HI_EVENT_STATEMNG_EMR_BEGIN,
        HI_EVENT_PDT_PARAM_CHANGE,
        HI_EVENT_STATEMNG_PREVIEW_PIP,

        HI_EVENT_GSENSORMNG_COLLISION,

        HI_EVENT_STORAGEMNG_DEV_UNPLUGED,
        HI_EVENT_STORAGEMNG_DEV_CONNECTING,
        HI_EVENT_STORAGEMNG_DEV_ERROR,
        HI_EVENT_STORAGEMNG_FS_CHECKING,
        HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
        HI_EVENT_STORAGEMNG_FS_EXCEPTION,
        HI_EVENT_STORAGEMNG_MOUNTED,
        HI_EVENT_STORAGEMNG_MOUNT_FAILED,

#ifdef CONFIG_MOTIONDETECT_ON
        HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE,
#endif
    };

    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("CreateSubscriber failed. %#x\n", s32Ret);
        return HI_FAILURE;
    }

    HI_S32 s32ArraySize;
    s32ArraySize = NETCTRL_ARRAY_SIZE(aEvents);

    for (i = 0; i < s32ArraySize; i++)
    {
        s32Ret = HI_EVTHUB_Subscribe(SubscriberHdl, aEvents[i]);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("Subscribe Event(%#x) failed. %#x\n", aEvents[i], s32Ret);
            continue;
        }

    }

    return HI_SUCCESS;
}


static HI_S32 PDT_NETCTRL_GetDevAttr(HISNET_DEVICE_ATTR_S* pstDevAttr)
{
    HI_PDT_DEV_INFO_S stDevInfo;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_GetCommParam");

    snprintf(pstDevAttr->aszDevModel, sizeof(pstDevAttr->aszDevModel), "%s", stDevInfo.szModel);
    snprintf(pstDevAttr->aszSoftwareVersion, sizeof(pstDevAttr->aszSoftwareVersion), "%s", stDevInfo.szSoftVersion);

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetSDInfo(HISNET_SDSTATUS_S* pstSDStatus)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    HI_STORAGE_FS_INFO_S stFSInfo = {0};
    HI_STORAGEMNG_CFG_S stStorageCfg = {};

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_GetStorageCfg");

    s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_STORAGEMNG_GetState");

    switch (enState)
    {
        case HI_STORAGE_STATE_MOUNTED:
            pstSDStatus->enSdStatus = HISNET_SDSTATUS_MOUNT;
            break;
        case HI_STORAGE_STATE_DEV_ERROR:
        case HI_STORAGE_STATE_FS_CHECK_FAILED:
        case HI_STORAGE_STATE_FS_EXCEPTION:
        case HI_STORAGE_STATE_MOUNT_FAILED:
            pstSDStatus->enSdStatus = HISNET_SDSTATUS_NOPARTITION;
            pstSDStatus->s32SDFreeSpace = 0;
            pstSDStatus->s32TotalSpace = 0;
            pstSDStatus->s32PartitionNum = 0;
            return HI_SUCCESS;
        case HI_STORAGE_STATE_DEV_UNPLUGGED:
        default:
            pstSDStatus->enSdStatus = HISNET_SDSTATUS_OUT;
            pstSDStatus->s32SDFreeSpace = 0;
            pstSDStatus->s32TotalSpace = 0;
            pstSDStatus->s32PartitionNum = 0;
            return HI_SUCCESS;
    }

    pstSDStatus->s32PartitionNum = 1;

    s32Ret = HI_STORAGEMNG_GetFSInfo(stStorageCfg.szMntPath, &stFSInfo);
    if (s32Ret==HI_SUCCESS)
    {
        pstSDStatus->s32SDFreeSpace = (stFSInfo.u64AvailableSize) >> 20; /** 1024 = 2^10,unit byte turn to MB*/
        pstSDStatus->s32TotalSpace = (stFSInfo.u64TotalSize) >> 20; /** 1024 = 2^10,unit byte turn to MB*/
    }
    else
    {
        pstSDStatus->s32SDFreeSpace = 0;
        pstSDStatus->s32TotalSpace = 0;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_SDCommand(HISNET_SYS_SDCOMMAND_S* pstSDCommand)
{
    HI_S32 s32Ret = HI_FAILURE;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

    if (stWorkModeState.bStateMngInProgress)
    {
        return HI_FAILURE;
    }

    if (HISNET_SDCOMMAND_FORMAT == pstSDCommand->enSDCommand)
    {
        HI_S32 ps32ProcResult = HI_FAILURE;
        HI_MESSAGE_S stMessage;
        memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
        stMessage.what = HI_EVENT_STATEMNG_SD_FORMAT;
        stMessage.arg1 =  HI_FSTOOL_FORMAT_MODE_ADVANCED;
        s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &ps32ProcResult);

        if (HI_SUCCESS != s32Ret || HI_SUCCESS != ps32ProcResult)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_NETCTRL_GetSysTime(HISNET_TIME_ATTR_S* pstSysTime)
{
    time_t NowTime = time(NULL);
    struct tm stTm;
    localtime_r(&NowTime, &stTm);
    snprintf(pstSysTime->aszTime, HISNET_MAX_TIMESIZE, "%04d%02d%02d%02d%02d%02d",
             stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
    MLOGD("CurTime[%s]\n", pstSysTime->aszTime);
    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_SetSysTime(const HISNET_TIME_ATTR_S* pstSysTime)
{
    return PDT_NETCTRL_SetTime(pstSysTime);
}

static HI_S32 PDT_NETCTRL_FactoryReset(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;
    memset(s_stNETCTRLClient, 0, sizeof(s_stNETCTRLClient));

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

    if (stWorkModeState.bStateMngInProgress)
    {
        return HI_FAILURE;
    }

    HI_MESSAGE_S stMessage;
    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
    stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_STATEMNG_SendMessage");

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetBatteryInfo(HISNET_BATTERY_STATUS_S* pstBatteryStatus)
{
    return HI_SUCCESS;
}

#if defined(CONFIG_WIFI_ON)
static HI_S32 PDT_NETCTRL_SetWifiAttr(const HISNET_WIFI_ATTR_S* pstWifiAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_WIFI_APMODE_CFG_S stWifiCfg = {};

    memset(&stWifiCfg, 0, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stWifiCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_GetCommParam");

    snprintf(stWifiCfg.stCfg.szWiFiSSID, sizeof(stWifiCfg.stCfg.szWiFiSSID), "%s", pstWifiAttr->aszSSID);
    stWifiCfg.s32Channel = pstWifiAttr->u32Channel;

    HI_BOOL bCfglid = HI_FALSE;
    HI_HAL_WIFI_CFG_S stCfg;
    stCfg.enMode = HI_HAL_WIFI_MODE_AP;
    memcpy(&stCfg.unCfg.stApCfg, &stWifiCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));

    s32Ret = HI_HAL_WIFI_CheckeCfgValid(&stCfg, &bCfglid);

    if (HI_SUCCESS != s32Ret || !bCfglid)
    {
        MLOGE("HI_HAL_WIFI_CheckeCfgValid failed!\n");
        return HI_FAILURE;
    }

    HI_BOOL bWiFiOn = HI_FALSE;
    s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWiFiOn);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_WIFI_GetStartedStatus");

    if (HI_TRUE == bWiFiOn)
    {
        s32Ret = HI_HAL_WIFI_Stop();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_WIFI_Stop");

        s32Ret = HI_HAL_WIFI_Start(&stCfg);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_WIFI_Start");
    }

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stWifiCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_SetCommParam");

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetWifiAttr(HISNET_WIFI_ATTR_S* pstWifiAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HAL_WIFI_APMODE_CFG_S stWifiCfg = {};
    HI_S32 i = 0;
    HI_U32 u32Links = 0;
    memset(&stWifiCfg, 0, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stWifiCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_PARAM_GetCommParam");

    snprintf(pstWifiAttr->aszSSID, sizeof(stWifiCfg.stCfg.szWiFiSSID), "%s", stWifiCfg.stCfg.szWiFiSSID);
    pstWifiAttr->u32Channel = stWifiCfg.s32Channel;
    HI_BOOL bWiFiOn = HI_FALSE;
    s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWiFiOn);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_WIFI_GetStartedStatus");

    if (bWiFiOn)
    {
        pstWifiAttr->bEnable = HI_TRUE;
    }
    else
    {
        pstWifiAttr->bEnable = HI_FALSE;
    }

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if (s_stNETCTRLClient[i].bConnected)
        {
            u32Links++;
        }
    }

    pstWifiAttr->u32Links = u32Links;

    return HI_SUCCESS;
}

#endif

static HI_S32 PDT_NETCTRL_GetFileInfo(HISNET_REMOTEFILENAME_S* pstFileName, FILEMNG_REMOTEFILEINFO_S* pstFileInfo)
{

    HI_S32 s32Ret = HI_SUCCESS;
    HI_FILEMNG_FILE_INFO_S stFileInfo = {};
    HI_CHAR szFileAbsPath[HI_APPCOMM_MAX_PATH_LEN] = {};

    snprintf(szFileAbsPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", NETCTRL_FILE_ROOT_PATH, pstFileName->aszName);
    memset(&stFileInfo, 0, sizeof(stFileInfo));
    s32Ret = HI_FILEMNG_GetFileInfoByName(szFileAbsPath, &stFileInfo);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_FILEMNG_GetFileInfoByName %s failed : 0x%x\n\n", pstFileName->aszName, s32Ret);
        return HI_FAILURE;
    }

    pstFileInfo->u32Duration = stFileInfo.u32Duration_sec;
    pstFileInfo->u32Size = stFileInfo.u64FileSize_byte;
    snprintf(pstFileInfo->aszPath, HI_APPCOMM_MAX_PATH_LEN, "%s", stFileInfo.szAbsPath);
    snprintf(pstFileInfo->aszCreate, HISNET_MAX_TIMESIZE, "%s", stFileInfo.szCreateTime);
    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_DeleteFile(HISNET_REMOTEFILENAME_S* pstFileName)
{
    HI_CHAR szFileAbsPath[HI_APPCOMM_MAX_PATH_LEN] = {};

    snprintf(szFileAbsPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", NETCTRL_FILE_ROOT_PATH, pstFileName->aszName);
    return HI_FILEMNG_RemoveFile(szFileAbsPath);
}

static HI_S32 PDT_NETCTRL_DeleteAllFiles(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FileNum = 0;

    s32Ret = HI_FILEMNG_GetFileObjCnt(HI_FILEMNG_FILE_TYPE_BUTT, &u32FileNum);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_FILEMNG_GetFileObjCnt");

    if (0 == u32FileNum)
    {
        return HI_SUCCESS;
    }

    HI_S32 u32Index;
    HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN];

    for (u32Index = u32FileNum - 1; u32Index >= 0; u32Index--)
    {
        s32Ret = HI_FILEMNG_GetFileByIndex(u32Index, szAbsPath, HI_APPCOMM_MAX_PATH_LEN);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_FILEMNG_GetFileByIndex");

        s32Ret = HI_FILEMNG_RemoveFile(szAbsPath);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_FILEMNG_RemoveFile");
    }

    return HI_SUCCESS;
}


static HI_VOID PDT_NETCTRL_RegCb(HI_VOID)
{
    HISNET_DEFAULT_SVR_CB_S stDefaultCmdProc;

    memset(&stDefaultCmdProc, 0, sizeof(HISNET_DEFAULT_SVR_CB_S));
    stDefaultCmdProc.stSysSvrCB.pfnGetDevAttr = PDT_NETCTRL_GetDevAttr;
    stDefaultCmdProc.stSysSvrCB.pfnGetSDStatus = PDT_NETCTRL_GetSDInfo;
    stDefaultCmdProc.stSysSvrCB.pfnSDCommand = PDT_NETCTRL_SDCommand;
    stDefaultCmdProc.stSysSvrCB.pfnSetSysTime = PDT_NETCTRL_SetSysTime;
    stDefaultCmdProc.stSysSvrCB.pfnGetSysTime = PDT_NETCTRL_GetSysTime;
    stDefaultCmdProc.stSysSvrCB.pfnReset = PDT_NETCTRL_FactoryReset;
    stDefaultCmdProc.stSysSvrCB.pfnGetBatteryStatus = PDT_NETCTRL_GetBatteryInfo;
#if defined(CONFIG_WIFI_ON)
    stDefaultCmdProc.stSysSvrCB.pfnSetWifiAttr = PDT_NETCTRL_SetWifiAttr;
    stDefaultCmdProc.stSysSvrCB.pfnGetWifiAttr = PDT_NETCTRL_GetWifiAttr;
#endif
    stDefaultCmdProc.stFileSvrCB.pfnGetFileInfo = PDT_NETCTRL_GetFileInfo;
    stDefaultCmdProc.stFileSvrCB.pfnDeleteFile = PDT_NETCTRL_DeleteFile;
    stDefaultCmdProc.stFileSvrCB.pfnDeleteAllFiles = PDT_NETCTRL_DeleteAllFiles;
    HISNET_RegisterDefaultCmd(&stDefaultCmdProc);
    return ;
}



static HI_S32 PDT_NETCTRL_RegisterClient(HI_CHAR* pszIpAddr)
{
    HI_S32 s32Ret = 0;
    HI_EVENT_S stEvent = {};

    if (HI_NULL == pszIpAddr)
    {
        MLOGE("pszIpAddr is invalid.\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_NETCTRL_DisableUsb();
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    s32Ret = PDT_NETCTRL_AddClientIP(pszIpAddr);

    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    stEvent.EventID = HI_EVENT_NETCTRL_CLIENT_CONNECTED;
    PDT_NETCTRL_PublishEvent(&stEvent);

    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_UnRegisterClient(HI_CHAR* pszIpAddr)
{
    HI_S32 s32Ret = 0;
    HI_EVENT_S stEvent = {};
    HI_S32 i = 0;

    if (HI_NULL == pszIpAddr)
    {
        MLOGE("pszIpAddr is invalid.\n");
        return HI_FAILURE;
    }

    for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
    {
        if (0 == strncmp(s_stNETCTRLClient[i].szIPv4Addr, pszIpAddr, sizeof(s_stNETCTRLClient[i].szIPv4Addr)))
        {
            s_stNETCTRLClient[i].bConnected = HI_FALSE;
            MLOGI("UnRegister ClientIP[%s], DisConnect it now!\n", s_stNETCTRLClient[i].szIPv4Addr);

            s32Ret = PDT_NETCTRL_CheckClientConnect(NULL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            stEvent.EventID = HI_EVENT_NETCTRL_CLIENT_DISCONNECTED;
            PDT_NETCTRL_PublishEvent(&stEvent);
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_NETCTRL_Client(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszOut[64] = {0};
    HI_CHAR aszOperation[32] = {0};
    HI_CHAR aszIpAddr[32] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, "get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"operation",     ARG_TYPE_MUST | ARG_TYPE_STRING, "register|unregister", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(aszOperation), sizeof(aszOperation)
        },

        {
            (HI_CHAR*)"ip",  ARG_TYPE_MUST | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(aszIpAddr), sizeof(aszIpAddr)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pvData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pvData, aszOut);
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("act", opts))
    {
        if (0 == strncmp(aszOperation, "register", sizeof("register")))
        {
            MLOGD("Register client %s\n", aszIpAddr);
            s32Ret = PDT_NETCTRL_RegisterClient(aszIpAddr);

            if (HI_SUCCESS != s32Ret)
            {
                pfnCGIStringProc(pvData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;

            }

            PDT_NETCTRL_EnableCheckConnect();

            pfnCGIStringProc(pvData, "Success\r\n");
        }
        else if (0 == strncmp(aszOperation, "unregister", sizeof("unregister")))
        {
            MLOGD("Unregister client %s\n", aszIpAddr);
            s32Ret = PDT_NETCTRL_UnRegisterClient(aszIpAddr);

            if (HI_SUCCESS != s32Ret)
            {
                pfnCGIStringProc(pvData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnCGIStringProc(pvData, "Success\r\n");
        }
        else
        {
            pfnCGIStringProc(pvData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pvData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_ClientReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi, MAX_CGI_INTER, "client.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER, "client&-act=set&");
    snprintf(stCgiCmd.method, METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_Client;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_CheckConnectState(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszIpAddr[32] = {0};
    HI_S32 i = 0;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"ip",  ARG_TYPE_MUST | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(aszIpAddr), sizeof(aszIpAddr)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            for (i = 0; i < NETCTRL_MAX_WEBSERVER_CLIENT_NUM; i++)
            {
                if (0 == strncmp(s_stNETCTRLClient[i].szIPv4Addr, aszIpAddr, sizeof(s_stNETCTRLClient[i].szIPv4Addr)))
                {
                    if(s_stNETCTRLClient[i].bConnected == HI_FALSE)
                    {
                       s_stNETCTRLClient[i].bConnected = HI_TRUE;
                    }
                    MLOGD("Apk check Connect success!\n");
                    pfnStringProc(pvData, "Success\r\n");
                    return HI_SUCCESS;
                }
            }
            MLOGI("Apk check Connect failed,IP has been cleared!\n");
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_CheckConnectStateReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER,"checkconnect.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER,"checkconnect&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN,CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_CheckConnectState;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_GetWorkState(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_CHAR szWorkMode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {};
            s32Ret = PDT_NETCTRL_GetWorkModeDescByValue(szWorkMode, stWorkModeState.enWorkMode, sizeof(szWorkMode));

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var workmode=\"%s\";\r\n", szWorkMode);
            pfnStringProc(pvData, (HI_CHAR*)"var running=\"%s\";\r\n", stWorkModeState.bRunning ? "true" : "false");
            pfnStringProc(pvData, (HI_CHAR*)"var emrrecord=\"%s\";\r\n", stWorkModeState.bEMRRecord ? "true" : "false");

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetWorkStateReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER,"getworkstate.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER,"getworkstate&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN,CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetWorkState;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_SetWorkmode(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32Result = HI_FAILURE;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szWorkMode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_PDT_WORKMODE_E enWorkMode;
    HI_MESSAGE_S stMessage;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"workmode",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szWorkMode), sizeof(szWorkMode)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (stWorkModeState.bStateMngInProgress)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_GetWorkModeValueByDesc(szWorkMode, (HI_S32*)&enWorkMode);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;

            switch (enWorkMode)
            {
                case HI_PDT_WORKMODE_NORM_REC:
                case HI_PDT_WORKMODE_PHOTO:
                    stMessage.arg2 = enWorkMode;
                    break;

                default:
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
                    return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

            if (HI_SUCCESS != s32Result || HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, "Success\r\n");

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_SetWorkmodeReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER,"setworkmode.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER,"setworkmode&-act=set&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_SetWorkmode;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_GetWorkmode(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_CHAR szWorkmode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {};
            s32Ret = PDT_NETCTRL_GetWorkModeDescByValue(szWorkmode, stWorkModeState.enWorkMode, sizeof(szWorkmode));

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var workmode=\"%s\";\r\n", szWorkmode);
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetWorkmodeReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi, MAX_CGI_INTER, "getworkmode.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getworkmode&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetWorkmode;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_WorkModeCMD(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_FAILURE;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szCmd[20] = {0};
    HI_MESSAGE_S stMessage;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"cmd",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szCmd), sizeof(szCmd)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (stWorkModeState.bStateMngInProgress)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_STORAGEMNG_CFG_S stStorageCfg = {};
            HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;

            s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
            s32Ret = HI_STORAGEMNG_GetState(stStorageCfg.szMntPath, &enStorageState);

            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));

            if (HI_NULL == szCmd)
            {
                MLOGE("Null args\n");
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            if (NULL != strstr(szCmd, "start"))
            {
                if (!s_bNETCTRLSdIsReady || HI_STORAGE_STATE_MOUNTED != enStorageState)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"sd is not ready\"\r\n");
                    return HI_FAILURE;
                }
                if(!PDT_NETCTRL_isMdReady())
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"md is not ready\"\r\n");
                    return HI_FAILURE;
                }

                stMessage.what = HI_EVENT_STATEMNG_START;
            }
            else if (NULL != strstr(szCmd, "stop"))
            {
                stMessage.what = HI_EVENT_STATEMNG_STOP;
                stMessage.arg1 = HI_TRUE;
            }
            else if (NULL != strstr(szCmd, "trigger"))
            {
                if (HI_PDT_WORKMODE_NORM_REC != stWorkModeState.enWorkMode
                    && HI_PDT_WORKMODE_PHOTO != stWorkModeState.enWorkMode)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"only norm_rec and photo support trigger\r\n");
                    return HI_FAILURE;
                }

                stMessage.what = HI_EVENT_STATEMNG_TRIGGER;
            }
            else if (NULL != strstr(szCmd, "emrbegin"))
            {
                stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
            }
            else
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            stMessage.arg2 = stWorkModeState.enWorkMode;
            s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

            if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, "Success\r\n");
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_WorkModeCMDReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "workmodecmd.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "workmodecmd&-act=set&");
    snprintf(stCgiCmd.method, METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_WorkModeCMD;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_GetCamNum(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {

#ifdef ONE_SENSOR_CONNECT
            pfnStringProc(pvData, (HI_CHAR*)"var camnum=\"1\";\r\n");
#else
            pfnStringProc(pvData, (HI_CHAR*)"var camnum=\"2\";\r\n");
#endif

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCamNumReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER,"getcamnum.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER,"getcamnum&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN,CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCamNum;
    HISNET_RegisterCmd(&stCgiCmd);
}




HI_S32 PDT_NETCTRL_GetPreViewCamID(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof ("get")))
        {


            HI_S32 s32PreViewCamID = -1;
            s32PreViewCamID = HI_PDT_STATEMNG_GetPreViewCamID();

            if (0 == s32PreViewCamID)
            {
                pfnStringProc(pvData, (HI_CHAR*)"var previewcamid=\"0\";\r\n");
            }
            else
            {
                pfnStringProc(pvData, (HI_CHAR*)"var previewcamid=\"1\";\r\n");
            }

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetPreViewCamIDReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER,"getpreviewcamid.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getpreviewcamid&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetPreViewCamID;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_GetCamChnl(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Result = HI_FAILURE;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_U32 camid = 0;
    HI_MESSAGE_S stMessage;
    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"camid",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(&camid), sizeof(HI_U32)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_S32 s32Value = 1;
            s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, camid, HI_PDT_PARAM_TYPE_APK_VENCID, &s32Value);
            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_LIVE_SWITCH;
            memcpy(stMessage.aszPayload, &camid, sizeof(HI_S32));
            s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

            HI_S32 s32Cnt = 0;
            while((HI_FAILURE == s32Ret) && (s32Cnt < 10))
            {
                s32Cnt++;
                hi_usleep(5*1000);
                s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);
            }

            if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var camchnl=\"%u\";\r\n", s32Value);

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCamChnlReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getcamchnl.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER,"getcamchnl&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCamChnl;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_VoSwitch(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_FAILURE;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_MESSAGE_S stMessage;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (stWorkModeState.bStateMngInProgress)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
            HI_S32 s32PreviewCamID = -1;

            s32PreviewCamID = HI_PDT_STATEMNG_GetPreViewCamID();
            s32PreviewCamID = (s32PreviewCamID + 1) % HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;

            stMessage.what = HI_EVENT_STATEMNG_VO_SWITCH;
            memcpy(stMessage.aszPayload, &s32PreviewCamID, sizeof(HI_S32));

            s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

            if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, "Success\r\n");
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_VoSwitchReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi, MAX_CGI_INTER,"voswitch.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "voswitch&-act=set&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_VoSwitch;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_SetCamParam(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_FAILURE;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_MESSAGE_S stMessage;
    HI_CHAR szWorkMode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szValue[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"workmode",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szWorkMode), sizeof(szWorkMode)
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"value",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szValue), sizeof(szValue)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (stWorkModeState.bStateMngInProgress)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_PDT_WORKMODE_E enWorkMode;
            HI_PDT_PARAM_TYPE_E enParamType;
            HI_S32 s32Value;
            s32Ret = PDT_NETCTRL_GetWorkModeValueByDesc(szWorkMode, (HI_S32*)&enWorkMode);
            s32Ret |= PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_GetParamValueByDesc(enWorkMode, enParamType, szValue, sizeof(szValue), &s32Value);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            if (enWorkMode == stWorkModeState.enWorkMode)
            {
                if (stWorkModeState.bRunning)
                {
                    stMessage.what = HI_EVENT_STATEMNG_STOP;
                    stMessage.arg1 = HI_TRUE;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                    if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                    {
                        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                        return HI_FAILURE;
                    }

                }

                memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SETTING;
                stMessage.arg1 = enParamType;
                stMessage.arg2 = enWorkMode;
                memcpy(stMessage.aszPayload, &s32Value, sizeof(HI_S32));
                s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }

                if (stWorkModeState.bRunning && PDT_NETCTRL_isMdReady())
                {
                    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                    stMessage.what = HI_EVENT_STATEMNG_START;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                    if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                    {
                        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                        return HI_FAILURE;
                    }

                }

            }
            else
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                MLOGE("current workmode is not norm_rec, can not set cam param!\n");
                return HI_FAILURE;

            }

            pfnStringProc(pvData, "Success\r\n");

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_SetCamParamReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "setcamparam.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER,"setcamparam&-act=set&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_SetCamParam;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_GetCamParam(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szWorkMode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"workmode",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szWorkMode), sizeof(szWorkMode)
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_PDT_WORKMODE_E enWorkMode;
            HI_PDT_PARAM_TYPE_E enParamType;
            HI_CHAR szValue[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

            s32Ret = PDT_NETCTRL_GetWorkModeValueByDesc(szWorkMode, (HI_S32*)&enWorkMode);
            s32Ret |= PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_GetParamValueDesc(enWorkMode, enParamType, szValue, sizeof(szValue));

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var value=\"%s\";\r\n", szValue);
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCamParamReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getcamparam.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getcamparam&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCamParam;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_SetCommonParam(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_FAILURE;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szValue[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_MESSAGE_S stMessage;
    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"value",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szValue), sizeof(szValue)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;
            HI_PDT_PARAM_TYPE_E enParamType;
            HI_S32 s32Value;
            s32Ret = PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_GetParamValueByDesc(enWorkMode, enParamType, szValue, sizeof(szValue), &s32Value);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (((HI_PDT_PARAM_TYPE_AUDIO == enParamType) ||(HI_PDT_PARAM_TYPE_PREVIEWPIP == enParamType)) && (HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode))
            {
                if (stWorkModeState.bStateMngInProgress)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }

                if (stWorkModeState.bRunning)
                {
                    stMessage.what = HI_EVENT_STATEMNG_STOP;
                    stMessage.arg1 = HI_TRUE;;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                    if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                    {
                        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                        return HI_FAILURE;
                    }
                }

                if(HI_PDT_PARAM_TYPE_PREVIEWPIP == enParamType)
                {
                    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                    stMessage.what = HI_EVENT_STATEMNG_PREVIEW_PIP;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    HI_CHAR* payload = (stMessage.aszPayload);
                    *(HI_BOOL*)payload = (HI_BOOL)s32Value;
                }
                else
                {
                    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                    stMessage.what = HI_EVENT_STATEMNG_SETTING;
                    stMessage.arg1 = enParamType;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    memcpy(stMessage.aszPayload, &s32Value, sizeof(HI_S32));
                }

                s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }

                if (stWorkModeState.bRunning && PDT_NETCTRL_isMdReady())
                {
                    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
                    stMessage.what = HI_EVENT_STATEMNG_START;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                    if (HI_SUCCESS != s32Ret || HI_SUCCESS != s32Result)
                    {
                        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                        return HI_FAILURE;
                    }
                }

            }
            else
            {
                s32Ret = PDT_NETCTRL_SetCommParam(enParamType, &s32Value);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }
            }

            pfnStringProc(pvData, "Success\r\n");

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_SetCommonParamReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "setcommparam.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER,"setcommparam&-act=set&");
    snprintf(stCgiCmd.method, METHOD_LEN,CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_SetCommonParam;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_GetCommonParam(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;
            HI_PDT_PARAM_TYPE_E enParamType;
            HI_CHAR szValue[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

            s32Ret = PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = PDT_NETCTRL_GetParamValueDesc(enWorkMode, enParamType, szValue, sizeof(szValue));

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var value=\"%s\";\r\n", szValue);

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCommonParamReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getcommparam.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER ,"getcommparam&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCommonParam;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_GetCamParamCapability(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_PDT_ITEM_VALUESET_S stCapabilityValue = {.s32Cnt = 0};
    HI_CHAR szWorkMode[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"workmode",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szWorkMode), sizeof(szWorkMode)
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    memset(&stCapabilityValue, 0, sizeof(stCapabilityValue));

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_CHAR szCapability[NETCTRL_MAX_CABABILITYE_STR_LEN] = {0};
            HI_PDT_WORKMODE_E enWorkMode;
            HI_PDT_PARAM_TYPE_E enParamType;
            HI_U32 i;

            s32Ret = PDT_NETCTRL_GetWorkModeValueByDesc(szWorkMode, (HI_S32*)&enWorkMode);
            s32Ret |= PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            HI_S32 s32PreViewCamID = -1;
            s32PreViewCamID = HI_PDT_STATEMNG_GetPreViewCamID();
            s32Ret |= HI_PDT_PARAM_GetCamItemValues(enWorkMode, s32PreViewCamID, enParamType, &stCapabilityValue);

            if (HI_SUCCESS != s32Ret || stCapabilityValue.s32Cnt <= 0)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            for (i = 0; i < stCapabilityValue.s32Cnt - 1; i++)
            {
                strncat(szCapability, stCapabilityValue.astValues[i].szDesc, 16);
                strncat(szCapability, ",", 2);
            }

            strncat(szCapability, stCapabilityValue.astValues[i].szDesc, 16);

            pfnStringProc(pvData, (HI_CHAR*)"var capability=\"%s\";\r\n", szCapability);
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCamParamCapabilityReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getcamparamcapability.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getcamparamcapability&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCamParamCapability;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_GetCommParamCapability(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_PDT_ITEM_VALUESET_S stCapabilityValue = {};
    HI_CHAR szParamType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szCapability[NETCTRL_MAX_CABABILITYE_STR_LEN] = {0};
    HI_U32 i;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szParamType), sizeof(szParamType)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    memset(&stCapabilityValue, 0, sizeof(stCapabilityValue));

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_PDT_PARAM_TYPE_E enParamType;
            s32Ret = PDT_NETCTRL_GetParamTypeValueByDesc(szParamType, (HI_S32*)&enParamType);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = HI_PDT_PARAM_GetCommItemValues(enParamType, &stCapabilityValue);

            if (HI_SUCCESS != s32Ret || stCapabilityValue.s32Cnt <= 0)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            for (i = 0; i < stCapabilityValue.s32Cnt - 1; i++)
            {
                strncat(szCapability, stCapabilityValue.astValues[i].szDesc, 16);
                strncat(szCapability, ",", 2);
            }

            strncat(szCapability, stCapabilityValue.astValues[i].szDesc, 16);
            pfnStringProc(pvData, (HI_CHAR*)"var capability=\"%s\";\r\n", szCapability);
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetCommParamCapabilityReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getcommparamcapability.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getcommparamcapability&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetCommParamCapability;
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_GetDirCapability(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szCapability[NETCTRL_MAX_CABABILITYE_STR_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_U32 u32Index = 0;
            HI_PDT_FILEMNG_CFG_S stCfg = {};

            s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            for (u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
            {
                if ( 0 < strnlen(stCfg.stDtcfCfg.aszDirNames[u32Index], HI_DIR_LEN_MAX))
                {
                    strncat(szCapability, stCfg.stDtcfCfg.aszDirNames[u32Index], 16);
                    strncat(szCapability, ",", 2);
                }
            }

            pfnStringProc(pvData, (HI_CHAR*)"var capability=\"%s\";\r\n", szCapability);

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetDirCapabilityReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getdircapability.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getdircapability&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetDirCapability;
    HISNET_RegisterCmd(&stCgiCmd);
}



HI_S32 PDT_NETCTRL_GetDirFilecount(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szDir[20] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"dir",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szDir), sizeof(szDir)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

            if (stWorkModeState.bStateMngInProgress)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            if ((HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode) && stWorkModeState.bRunning)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                MLOGE("Recording can not get dir file count!");
                return HI_FAILURE;
            }

            HI_U32 u32FileObjCnt = 0;
            HI_DTCF_DIR_E enDir;

            s32Ret = PDT_NETCTRL_GetFileDirValueByDesc(szDir, &enDir);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = HI_FILEMNG_SetSearchScope(&enDir, 1, &u32FileObjCnt);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var count=\"%u\";\r\n", u32FileObjCnt);


            HI_U32 u32Index = 0;
            HI_U32 u32DirCount = 0;
            HI_PDT_FILEMNG_CFG_S stCfg = {};
            HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

            s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            for (u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
            {
                if ( 0 < strnlen(stCfg.stDtcfCfg.aszDirNames[u32Index], HI_DIR_LEN_MAX))
                {
                    aenDirs[u32DirCount++] = u32Index;
                }
            }

            u32FileObjCnt = 0;
            s32Ret = HI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32FileObjCnt);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetDirFilecountReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getdirfilecount.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getdirfilecount&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetDirFilecount;
    HISNET_RegisterCmd(&stCgiCmd);
}



HI_S32 PDT_NETCTRL_GetDirFileList(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szDir[20] = {0};
    HI_U32 u32Start = 0;
    HI_U32 u32End = 0;
    HI_U32 i = 0;

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"dir",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szDir), sizeof(szDir)
        },

        {
            (HI_CHAR*)"start",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(&u32Start), sizeof(HI_U32)
        },

        {
            (HI_CHAR*)"end",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(&u32End), sizeof(HI_U32)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

        if (stWorkModeState.bStateMngInProgress)
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }

        if ((HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode) && stWorkModeState.bRunning)
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            MLOGE("Recording can not get dir file count!");
            return HI_FAILURE;
        }

        HI_U32 u32FileObjCnt = 0;
        HI_DTCF_DIR_E enDir;
        HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN];
        HI_CHAR* pszRelativePath = NULL;

        s32Ret = PDT_NETCTRL_GetFileDirValueByDesc(szDir, &enDir);

        if (HI_SUCCESS != s32Ret)
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }

        s32Ret = HI_FILEMNG_SetSearchScope(&enDir, 1, &u32FileObjCnt);

        if (HI_SUCCESS != s32Ret)
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }

        for (i = u32Start; i <= u32End && i <= u32FileObjCnt; i++)
        {

            s32Ret |= HI_FILEMNG_GetFileByIndex(i, szAbsPath, HI_APPCOMM_MAX_PATH_LEN);

            if (HI_SUCCESS != s32Ret)
            {
                continue;
            }
            pszRelativePath = &szAbsPath[strlen(NETCTRL_FILE_ROOT_PATH)];
            pfnStringProc(pvData, "%s;", pszRelativePath);
        }

        pfnStringProc(pvData, "\r\n");


        HI_U32 u32Index = 0;
        HI_U32 u32DirCount = 0;
        HI_PDT_FILEMNG_CFG_S stCfg = {};
        HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

        s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        for (u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
        {
            if ( 0 < strnlen(stCfg.stDtcfCfg.aszDirNames[u32Index], HI_DIR_LEN_MAX))
            {
                aenDirs[u32DirCount++] = u32Index;
            }
        }

        u32FileObjCnt = 0;
        s32Ret = HI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32FileObjCnt);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    }
    else
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetDirFileListReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getdirfilelist.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getdirfilelist&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetDirFileList;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_GetVideoVerboseFileInfo(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN] = {};
    HI_CHAR szFileAbsPath[HI_APPCOMM_MAX_PATH_LEN] = {};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"name",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szFileName), sizeof(szFileName)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {
            HI_FORMAT_FILE_INFO_S stFileInfo = {};
            snprintf(szFileAbsPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", NETCTRL_FILE_ROOT_PATH, szFileName);

            HI_CHAR* pSuffix = NULL;
            pSuffix = strrchr(szFileAbsPath, '.');

            if (!pSuffix)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            MLOGD("File:%s, Suffix:%s\n", szFileAbsPath, pSuffix);

            if (!strncmp(pSuffix, ".MP4", sizeof(".MP4")) || !strncmp(pSuffix, ".LRV", sizeof(".LRV")))
            {
                s32Ret = PDT_NETCTRL_GetVideoFileExhaustiveInfo(szFileAbsPath, &stFileInfo);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }

                pfnStringProc(pvData, (HI_CHAR*)"var filename=\"%s\";\r\n", szFileAbsPath);
                pfnStringProc(pvData, (HI_CHAR*)"var filesize=\"%d\";\r\n", stFileInfo.s64FileSize);
                pfnStringProc(pvData, (HI_CHAR*)"var videocodec=\"%d\";\r\n", stFileInfo.enVideoType);
                pfnStringProc(pvData, (HI_CHAR*)"var width=\"%d\";\r\n", stFileInfo.stSteamResolution[0].u32Width);
                pfnStringProc(pvData, (HI_CHAR*)"var height=\"%d\";\r\n", stFileInfo.stSteamResolution[0].u32Height);
                pfnStringProc(pvData, (HI_CHAR*)"var fileduration=\"%d\";\r\n", stFileInfo.s64Duration);
                pfnStringProc(pvData, (HI_CHAR*)"var fps=\"%f\";\r\n", stFileInfo.fFrameRate);
                pfnStringProc(pvData, (HI_CHAR*)"var videobitrate=\"%d\";\r\n", stFileInfo.u32Bitrate);
                pfnStringProc(pvData, (HI_CHAR*)"var audiosamplerate=\"%d\";\r\n", stFileInfo.u32SampleRate);
                pfnStringProc(pvData, (HI_CHAR*)"var audiochannels=\"%d\";\r\n", stFileInfo.u32AudioChannelCnt);
            }
            else
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_GetVideoVerboseFileInfoReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "getvideoverbosefileinfo.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "getvideoverbosefileinfo&-act=get&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetVideoVerboseFileInfo;
    PDT_NETCTRL_GetDlsym();
    HISNET_RegisterCmd(&stCgiCmd);
}

HI_S32 PDT_NETCTRL_SendClickKey(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32  s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szKeyType[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"type",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szKeyType), sizeof(szKeyType)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_EVENT_S stEvent = {.EventID = HI_EVENT_KEYMNG_SHORT_CLICK,};

            if (0 == strncasecmp(PDT_NETCTRL_KEY_OK, szKeyType, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
            {
                stEvent.arg1 = HI_KEYMNG_KEY_IDX_0;
            }
            else if (0 == strncasecmp(PDT_NETCTRL_KEY_MENU, szKeyType, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
            {
                stEvent.arg1 = HI_KEYMNG_KEY_IDX_1;
            }
            else if (0 == strncasecmp(PDT_NETCTRL_KEY_BACK, szKeyType, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN))
            {
                stEvent.arg1 = HI_KEYMNG_KEY_IDX_2;
            }

            PDT_NETCTRL_PublishEvent(&stEvent);
            pfnStringProc(pvData, "Success\r\n");
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_SendClickKeyReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "sendclickkey.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "sendclickkey&-act=set&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_SendClickKey;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_S32 PDT_NETCTRL_SetLogLevel(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szbLog[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szbLogTime[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};
    HI_CHAR szValue[PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN] = {0};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"blog",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szbLog), sizeof(szbLog)
        },

        {
            (HI_CHAR*)"blogtime",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szbLogTime), sizeof(szbLogTime)
        },

        {
            (HI_CHAR*)"value",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szValue), sizeof(szValue)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "set", sizeof("set")))
        {
            HI_BOOL bLogOn;
            HI_BOOL bLogTime;
            HI_LOG_LEVEL_E enLevel;

            bLogOn = strncmp(PDT_NETCTRL_ON, szbLog, strnlen(szbLog, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN)) == 0 ? HI_TRUE : HI_FALSE;
            bLogTime = strncmp(PDT_NETCTRL_ON, szbLogTime, strnlen(szbLogTime, PDT_NETCTRL_ITEM_VALUE_DESCRIPTION_LEN)) == 0 ? HI_TRUE : HI_FALSE;

            s32Ret = PDT_NETCTRL_GetLogLevelValueByDesc(szValue, (HI_S32*)&enLevel);

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            HI_LOG_Config(bLogOn, bLogTime, enLevel);

            pfnStringProc(pvData, "Success\r\n");

        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 HI_PDT_NETCTRL_UpgradeReceivePktInfo(const HI_CHAR* pszPath, HI_U32 u32Offset, HI_U32 u32UnitLen, FILE** pPktFP)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Count = 0;

    if (u32UnitLen > PDT_UPGRADE_UNITSIZE)
    {
        MLOGE("Unit Size Invalid\n");
        return HI_FAILURE;
    }

    for (s32Count = 0; s32Count < NETCTRL_MAX_RECEIVE_COUNT; s32Count++)
    {
        if (HI_TRUE == s_stReceiveInfo[s32Count].bEnable
            && 0 != s_stReceiveInfo[s32Count].u32OffSet
            && u32Offset <= s_stReceiveInfo[s32Count].u32OffSet
            && HI_NULL == s_stReceiveInfo[s32Count].pFileFP)
        {
            MLOGI("Save %s from breakpoint,offset[%d]\n", s_stReceiveInfo[s32Count].szPath, s_stReceiveInfo[s32Count].u32OffSet);
            //s_stReceiveInfo[s32Count].u32OffSet = u32Offset;

            *pPktFP = fopen(s_stReceiveInfo[s32Count].szPath, "r+");

            if (NULL == *pPktFP)
            {
                MLOGE("%s fopen fail.\n", pszPath);
                return HI_FAILURE;
            }

            break;
        }
    }

    if (NETCTRL_MAX_RECEIVE_COUNT == s32Count)
    {
        for (s32Count = 0; s32Count < NETCTRL_MAX_RECEIVE_COUNT; s32Count++)
        {
            if (HI_TRUE == s_stReceiveInfo[s32Count].bEnable
                && HI_NULL == s_stReceiveInfo[s32Count].pFileFP
                && u32Offset <= s_stReceiveInfo[s32Count].u32OffSet)
            {
                //s_stReceiveInfo[s32Count].u32OffSet = u32Offset;
                snprintf(s_stReceiveInfo[s32Count].szPath, PDT_UPGRADE_MAX_FILENAME, "%s", pszPath);

                *pPktFP = fopen(s_stReceiveInfo[s32Count].szPath, "w");

                if (NULL == *pPktFP)
                {
                    MLOGE("%s fopen fail.\n", pszPath);
                    return HI_FAILURE;
                }

                break;
            }
        }

        if (NETCTRL_MAX_RECEIVE_COUNT == s32Count)
        {
            MLOGE("Receive task excessive!\n");
            return HI_FAILURE;
        }
    }

    s32Ret = fseek(*pPktFP, s_stReceiveInfo[s32Count].u32OffSet, 0);
    s_stReceiveInfo[s32Count].pFileFP = *pPktFP;

    return s32Ret;
}


static HI_S32 HI_PDT_NETCTRL_UpgradeReceivePkt(const HI_U8* pszRecvBuf, HI_U32 u32BufLen, FILE* pPktFP)
{
    HI_S32 s32Ret;
    HI_MESSAGE_S stMessage = {};
    HI_S32 s32Count = 0;

    for (s32Count = 0; s32Count < NETCTRL_MAX_RECEIVE_COUNT; s32Count++)
    {
        if (pPktFP == s_stReceiveInfo[s32Count].pFileFP)
        {
            break;
        }
    }

    if (NETCTRL_MAX_RECEIVE_COUNT == s32Count)
    {
        MLOGE("Unknown file pointer!\n");
        return HI_FAILURE;
    }

    if (u32BufLen != fwrite(pszRecvBuf, 1, u32BufLen, pPktFP))
    {
        MLOGE("fwrite fail.\n");
        return HI_FAILURE;
    }

    s_stReceiveInfo[s32Count].pFileFP = pPktFP;
    s_stReceiveInfo[s32Count].u32OffSet += u32BufLen;
    s_stReceiveInfo[s32Count].u32RecvBufLen += u32BufLen;

    if (s_stReceiveInfo[s32Count].u32UnitSize == s_stReceiveInfo[s32Count].u32RecvBufLen)
    {
        s_stReceiveInfo[s32Count].pFileFP = HI_NULL;
        s_stReceiveInfo[s32Count].u32RecvBufLen = 0;
    }

    if (s_stReceiveInfo[s32Count].u32PktLen == s_stReceiveInfo[s32Count].u32OffSet)
    {

        HI_UPGRADE_DEV_INFO_S stUpgradeDevInfo = {};
        HI_PDT_DEV_INFO_S stDevInfo;
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        memcpy(stUpgradeDevInfo.szModel,stDevInfo.szModel,HI_APPCOMM_COMM_STR_LEN);
        memcpy(stUpgradeDevInfo.szSoftVersion,stDevInfo.szSoftVersion,HI_APPCOMM_COMM_STR_LEN);

        if(HI_UPGRADE_PKT_AVAILABLE==HI_UPGRADE_CheckPkt(s_stReceiveInfo[s32Count].szPath, &stUpgradeDevInfo))
        {
            fsync(fileno(s_stReceiveInfo[s32Count].pFileFP));
            stMessage.what = HI_EVENT_STATEMNG_UPGRADE_START;
            snprintf(stMessage.aszPayload,MESSAGE_PAYLOAD_LEN,"%s",s_stReceiveInfo[s32Count].szPath);
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            stMessage.what = HI_EVENT_STATEMNG_UPGRADE_FAILED;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s_stReceiveInfo[s32Count].pFileFP = HI_NULL;
        s_stReceiveInfo[s32Count].bEnable = HI_FALSE;
        s_stReceiveInfo[s32Count].u32RecvBufLen = 0;
    }
    else if (s_stReceiveInfo[s32Count].u32PktLen < s_stReceiveInfo[s32Count].u32OffSet)
    {
        s_stReceiveInfo[s32Count].u32OffSet = 0;
        MLOGE("Receive file size exceeds upgrade package size!\n");

        stMessage.what = HI_EVENT_STATEMNG_UPGRADE_FAILED;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s_stReceiveInfo[s32Count].pFileFP = HI_NULL;
        s_stReceiveInfo[s32Count].bEnable = HI_FALSE;
        s_stReceiveInfo[s32Count].u32RecvBufLen = 0;
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID* PDT_NETCTRL_TIMEOUT_CHECK(HI_VOID* pArg)
{
    HI_S32 s32Ret;
    HI_S32 s32TimeoutFlag = 0;
    HI_MESSAGE_S stMessage;
    HI_S32 s32Count = *(HI_S32*)pArg;

    HI_PDT_UPGRADE_RECEIVE_INFO_S stReceiveInfo = s_stReceiveInfo[s32Count];

    while (1)
    {
        if (s_stReceiveInfo[s32Count].u32PktLen <= s_stReceiveInfo[s32Count].u32OffSet)
        {
            s_stReceiveInfo[s32Count].u32OffSet = 0;
            s_stReceiveInfo[s32Count].bEnable = HI_FALSE;
            MLOGI(GREEN"Receive Upgrade pkt End!\n"NONE);
            break;
        }

        struct timeval t_timeval;

        t_timeval.tv_sec = PDT_UPGRADE_RECEIVE_SEC;

        select(0, NULL, NULL, NULL, &t_timeval);

        if (stReceiveInfo.u32OffSet == s_stReceiveInfo[s32Count].u32OffSet)
        {
            MLOGI(YELLOW"Receive time out!\n"NONE);
            s32TimeoutFlag++;
        }
        else
        {
            s32TimeoutFlag = 0;
            stReceiveInfo.u32OffSet = s_stReceiveInfo[s32Count].u32OffSet;
        }

        if (PDT_UPGRADE_MAX_WAITCOUNT == s32TimeoutFlag)
        {
            MLOGE(RED"Do Upgrade failed! Receive time out!\n"NONE);
            s_stReceiveInfo[s32Count].bEnable = HI_FALSE;
            s_stReceiveInfo[s32Count].pFileFP = NULL;
            s_stReceiveInfo[s32Count].u32RecvBufLen = 0;
            stMessage.what = HI_EVENT_STATEMNG_UPGRADE_FAILED;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "HI_PDT_STATEMNG_SendMessage");
            break;
        }
    }

    return HI_NULL;
}

HI_S32 PDT_NETCTRL_CheckUpgradePktInfo(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_FAILURE;
    HI_U32 u32PktLen = 0;
    HI_S32 s32Count = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN] = {0,};
    HI_CHAR szModel[HI_APPCOMM_COMM_STR_LEN] = {0,};
    HI_MESSAGE_S stMessage;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            (HI_CHAR*)"model",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szModel), HI_APPCOMM_COMM_STR_LEN
        },

        {
            (HI_CHAR*)"softversion",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(szSoftVersion), HI_APPCOMM_COMM_STR_LEN
        },

        {
            (HI_CHAR*)"pktlen",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(&u32PktLen), sizeof(u32PktLen)
        },

        {
            (HI_CHAR*)"END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {

            MLOGD("Pkt Model[%s] SoftVersion[%s]\n", szModel, szSoftVersion);
            HI_PDT_DEV_INFO_S stDevInfo;
            HI_UPGRADE_DEV_INFO_S stUpGradeDevInfo = {};
            HISNET_SDSTATUS_S stSDStatus;
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("Get DevInfo\n");
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            memcpy(stUpGradeDevInfo.szModel, stDevInfo.szModel, HI_APPCOMM_COMM_STR_LEN);
            memcpy(stUpGradeDevInfo.szSoftVersion, stDevInfo.szSoftVersion, HI_APPCOMM_COMM_STR_LEN);

            s32Ret = PDT_NETCTRL_GetSDInfo(&stSDStatus);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_NETCTRL_GetSDInfo");

            if (strncmp(szModel, stDevInfo.szModel, HI_APPCOMM_COMM_STR_LEN))
            {
                MLOGE("Invalid Model[%s]\n", szModel);
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            s32Ret = strncmp(szSoftVersion, stDevInfo.szSoftVersion, HI_APPCOMM_COMM_STR_LEN);

            if (0 >= s32Ret)
            {
                MLOGI("OldVersion[%s]\n", szSoftVersion);
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            //TODO storage detection(Battery Not yet implemented)
            if (HI_FAILURE == s32Ret || stSDStatus.s32SDFreeSpace * 1024 * 1024 <= u32PktLen)
            {
                MLOGI(RED"Upgrade Failed! Not enough storage space!"NONE"\n");
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%s\"\r\n", "not enough mem");
                return HI_FAILURE;
            }

            MLOGI(YELLOW"New Version[%s] PktLen[%d]"NONE"\n", szSoftVersion, u32PktLen);

            HI_THTTPD_RegisterUploadFileProc(HI_PDT_NETCTRL_UpgradeReceivePktInfo, HI_PDT_NETCTRL_UpgradeReceivePkt);

            for (s32Count = 0; s32Count < NETCTRL_MAX_RECEIVE_COUNT; s32Count++)
            {
                if (0 != s_stReceiveInfo[s32Count].u32OffSet
                    && u32PktLen == s_stReceiveInfo[s32Count].u32PktLen
                    && 0 == strncmp(szSoftVersion, s_stReceiveInfo[s32Count].stDevInfo.szSoftVersion, HI_APPCOMM_COMM_STR_LEN))
                {
                    s_stReceiveInfo[s32Count].bEnable = HI_TRUE;
                    s_stReceiveInfo[s32Count].u32UnitSize = PDT_UPGRADE_UNITSIZE;
                    pfnStringProc(pvData, (HI_CHAR*)"var offset=\"%d\";\r\n"
                                  "var unitsize=\"%d\";\r\n",
                                  s_stReceiveInfo[s32Count].u32OffSet,
                                  s_stReceiveInfo[s32Count].u32UnitSize);
                    break;
                }
            }

            if (NETCTRL_MAX_RECEIVE_COUNT == s32Count)
            {
                for (s32Count = 0; s32Count < NETCTRL_MAX_RECEIVE_COUNT; s32Count++)
                {
                    if (HI_FALSE == s_stReceiveInfo[s32Count].bEnable)
                    {
                        s_stReceiveInfo[s32Count].bEnable = HI_TRUE;
                        s_stReceiveInfo[s32Count].u32PktLen = u32PktLen;
                        s_stReceiveInfo[s32Count].u32OffSet = 0;
                        snprintf(s_stReceiveInfo[s32Count].stDevInfo.szSoftVersion,HI_APPCOMM_COMM_STR_LEN, szSoftVersion);
                        s_stReceiveInfo[s32Count].u32UnitSize = PDT_UPGRADE_UNITSIZE;
                        s_stReceiveInfo[s32Count].pFileFP = HI_NULL;
                        pfnStringProc(pvData, (HI_CHAR*)"var offset=\"%d\";\r\n"
                                      "var unitsize=\"%d\";\r\n",
                                      s_stReceiveInfo[s32Count].u32OffSet,
                                      s_stReceiveInfo[s32Count].u32UnitSize);
                        break;
                    }
                }

                if (NETCTRL_MAX_RECEIVE_COUNT == s32Count)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
                    return HI_FAILURE;
                }
            }

            HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            if (HI_PDT_WORKMODE_UPGRADE != stWorkModeState.enWorkMode)
            {
                MLOGD("switch to upgrade mode\n");
                /*when rev pkt we should stop rec, make sure sd space is enough*/
                stMessage.what = HI_EVENT_STATEMNG_STOP;
                stMessage.arg1 = HI_TRUE;;
                stMessage.arg2 = stWorkModeState.enWorkMode;
                s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                memset(&stMessage, 0 , sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
                stMessage.arg2 = HI_PDT_WORKMODE_UPGRADE;
                s32Ret = PDT_NETCTRL_SendSyncMsg(&stMessage, &s32Result);

                if (HI_SUCCESS != s32Result || HI_SUCCESS != s32Ret)
                {
                    pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }

                pthread_t PthreadTimeOut;
                pthread_attr_t User_attr;
                pthread_attr_init(&User_attr);
                pthread_attr_setdetachstate(&User_attr, PTHREAD_CREATE_DETACHED);
                pthread_create(&PthreadTimeOut, &User_attr, PDT_NETCTRL_TIMEOUT_CHECK, (HI_VOID *)&s32Count);
            }
        } else {
            pfnStringProc(pvData, (HI_CHAR *)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_NETCTRL_GetSDPrompt(HI_S32 argc, const HI_CHAR* pargv[], PfnCgiStringProc pfnStringProc,
                                      HI_VOID* pvData)
{
    HI_S32 s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR sdPromptDesc[PDT_NETCTRL_STR_LEN] = {};

    ARG_OPT_S opts[] =
    {
        {
            (HI_CHAR*)"?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            (HI_CHAR*)"act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, (HI_CHAR*)"get|set", HI_FALSE, {0},
            (HI_CHAR*)ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, pargv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnStringProc(pvData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet((HI_CHAR*)"act", opts))
    {
        if (0 == strncmp(_act, "get", sizeof("get")))
        {

            s32Ret = PDT_NETCTRL_GetSdPromptInfo(sdPromptDesc, sizeof(sdPromptDesc));

            if (HI_SUCCESS != s32Ret)
            {
                pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
                return HI_FAILURE;
            }

            pfnStringProc(pvData, (HI_CHAR*)"var sdPromptDesc=\"%s\";\r\n", sdPromptDesc);
        }
        else
        {
            pfnStringProc(pvData, (HI_CHAR*)"SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID PDT_NETCTRL_SetUpgradePktInfoReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi, MAX_CGI_INTER, "checkupgradepktinfo.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER, "checkupgradepktinfo&-act=get&");
    snprintf(stCgiCmd.method, METHOD_LEN, CGI_METHOD_GET);
    stCgiCmd.cmdProc = PDT_NETCTRL_CheckUpgradePktInfo;
    HISNET_RegisterCmd(&stCgiCmd);
}


HI_VOID PDT_NETCTRL_SetLogLevelReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi,MAX_CGI_INTER, "setloglevel.cgi");
    snprintf(stCgiCmd.cmd,MAX_CMD_INTER, "setloglevel&-act=set&");
    snprintf(stCgiCmd.method,METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_SetLogLevel;
    HISNET_RegisterCmd(&stCgiCmd);
}

static HI_VOID PDT_NETCTRL_GetSDPromptReg(HI_VOID)
{
    cgi_cmd_trans_s stCgiCmd;
    snprintf(stCgiCmd.cgi, MAX_CGI_INTER, "getsdpromptinfo.cgi");
    snprintf(stCgiCmd.cmd, MAX_CMD_INTER, "getsdpromptinfo&-act=get&");
    snprintf(stCgiCmd.method, METHOD_LEN, CGI_METHOD_SET);
    stCgiCmd.cmdProc = PDT_NETCTRL_GetSDPrompt;
    HISNET_RegisterCmd(&stCgiCmd);
}

static HI_VOID PDT_NETCTRL_CmdReg(HI_VOID)
{
    PDT_NETCTRL_ClientReg();
    PDT_NETCTRL_CheckConnectStateReg();
    PDT_NETCTRL_GetWorkStateReg();
    PDT_NETCTRL_SetWorkmodeReg();
    PDT_NETCTRL_GetWorkmodeReg();
    PDT_NETCTRL_WorkModeCMDReg();
    PDT_NETCTRL_GetCamNumReg();
    PDT_NETCTRL_GetPreViewCamIDReg();
    PDT_NETCTRL_GetCamChnlReg();
    PDT_NETCTRL_VoSwitchReg();
    PDT_NETCTRL_SetCamParamReg();
    PDT_NETCTRL_GetCamParamReg();
    PDT_NETCTRL_SetCommonParamReg();
    PDT_NETCTRL_GetCommonParamReg();
    PDT_NETCTRL_GetCamParamCapabilityReg();
    PDT_NETCTRL_GetCommParamCapabilityReg();
    PDT_NETCTRL_GetDirCapabilityReg();
    PDT_NETCTRL_GetDirFilecountReg();
    PDT_NETCTRL_GetDirFileListReg();

    PDT_NETCTRL_GetVideoVerboseFileInfoReg();
    PDT_NETCTRL_SendClickKeyReg();

    PDT_NETCTRL_SetLogLevelReg();
    PDT_NETCTRL_SetUpgradePktInfoReg();
    PDT_NETCTRL_GetSDPromptReg();
}

/** register all events published from this module */
HI_S32 HI_PDT_NETCTRL_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    HI_EVENT_NETCTRL_E enEvents[] =
    {
        HI_EVENT_NETCTRL_CLIENT_CONNECTED ,
        HI_EVENT_NETCTRL_CLIENT_DISCONNECTED
    };

    HI_S32 s32ArraySize;
    s32ArraySize = NETCTRL_ARRAY_SIZE(enEvents);

    for (i = 0; i < s32ArraySize; i++)
    {
        s32Ret = HI_EVTHUB_Register(enEvents[i]);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("Event:%d register failed\n", enEvents[i]);
            continue;
        }
    }

    return HI_SUCCESS;
}


HI_S32 HI_PDT_NETCTRL_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_CFG_S stTimedTaskCfg;

    s32Ret = HI_FILETRANS_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


    s32Ret = sem_init(&s_NETCTRLSem, 0, 0);
    s32Ret |= PDT_NETCTRL_SubscribeEvents();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    PDT_NETCTRL_RegCb();
    PDT_NETCTRL_CmdReg();

    s32Ret = HISNET_RegisterDistribLink(PDT_NETCTRL_DistribLink);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HISNET_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stTimedTaskCfg.pfnProc = PDT_NETCTRL_CheckClientConnect;
    stTimedTaskCfg.pvPrivData = HI_NULL;
    stTimedTaskCfg.stAttr.bEnable = HI_FALSE;
    stTimedTaskCfg.stAttr.u32Time_sec = 5;

    s32Ret = HI_TIMEDTASK_Create(&stTimedTaskCfg, &s_TimedTaskHdl);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Check Connect TimeTask Task(%d) creat error\n", s_TimedTaskHdl);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_NETCTRL_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_TIMEDTASK_Destroy(s_TimedTaskHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s32Ret = sem_destroy(&s_NETCTRLSem);
    s32Ret |= HISNET_Deinit();

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_FILETRANS_Deinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
