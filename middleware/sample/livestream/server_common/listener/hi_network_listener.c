/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_listener.c
* @brief     livestream common network listener src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#ifndef __LITEOS__
#include <arpa/inet.h>
#endif
#include "securec.h"
#include "hi_server_log.h"
#include "hi_server_state_listener.h"
#include "hi_network_common.h"
#include "hi_network_listener.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#ifndef MW_VERSION
#define MW_VERSION "0.0.0"
#endif
static volatile const HI_CHAR SERVERCOMMON_VERSIONINFO[] = "HIBERRY SERVERCOMMON MW Version: "MW_VERSION;
static volatile const HI_CHAR SERVERCOMMON_BUILD_DATE[] = "HIBERRY SERVERCOMMON Build Date:"__DATE__" Time:"__TIME__;

#define KEEPALIVE_IDLE_SECONDS (3)

typedef struct _HiNetListenerCtx
{
    pthread_t ptListenThd;
    HI_S32 s32ListenSock;
    HI_U16 s32ListenPort;
    /*state of the listener */
    HI_BOOL bInListening;
    HI_CHAR  aszFirstMsgBuff[MAX_RECV_BUFFER];
    HI_S32 s32ClientSockList[DEFAULT_MAX_FDNUM];
    onClientConnection connectCallback;
    HI_MW_PTR callbackObj;
} HiNetListenerCtx;

static HI_S32 NETWORK_Listener_SetNonblock(HI_S32 sockfd)
{
    HI_S32 s32Ret = HI_SUCCESS;
    int flags = 0;
    flags = fcntl(sockfd, F_GETFL, 0); //��ȡ�ļ���flagsֵ��
    s32Ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);   //���óɷ�����ģʽ��
    if( s32Ret < 0 )
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}


/*
keepalive: 1:����keepalive����, 0:�ر�
keepidle: ���������keepidle����û���κ���������,�����̽��
keepinterval: ̽��ʱ������ʱ����Ϊkeepinterval ��
keepcount:  ̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.
*/
static HI_S32 NETWORK_Listener_SetKeepAlive(HI_S32 sockfd, int keepalive, int keepidle, int keepinterval, int keepcount)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
    if( s32Ret < 0 )
    {
        return s32Ret;
    }

    s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
    if( s32Ret < 0 )
    {
        return s32Ret;
    }

    s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
    if( s32Ret < 0 )
    {
        return s32Ret;
    }

    s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
    if( s32Ret < 0 )
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 NETWORK_Listener_AddFd(HiNetListenerCtx* pstListenerCtx, HI_S32 sockfd)
{
    HI_S32  i = 0;

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] == INVALID_SOCKETFD)
        {
            pstListenerCtx->s32ClientSockList[i] = sockfd;
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

static HI_S32 NETWORK_Listener_DelFd(HiNetListenerCtx* pstListenerCtx, HI_S32 sockfd)
{
    HI_S32  i = 0;

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] == sockfd)
        {
            pstListenerCtx->s32ClientSockList[i] = INVALID_SOCKETFD;
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

static HI_S32 NETWORK_Listener_CloseClientFd(HiNetListenerCtx* pstListenerCtx)
{
    HI_S32  i = 0;

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != INVALID_SOCKETFD)
        {
            close(pstListenerCtx->s32ClientSockList[i]) ;
            pstListenerCtx->s32ClientSockList[i] = INVALID_SOCKETFD;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 NETWORK_Listener_UpdateMaxFd(HiNetListenerCtx* pstListenerCtx, HI_S32* pMaxFdNum)
{
    HI_S32 maxFdNum = -1;
    HI_S32  i = 0;

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != INVALID_SOCKETFD && pstListenerCtx->s32ClientSockList[i] > maxFdNum)
        {
            maxFdNum = pstListenerCtx->s32ClientSockList[i];
        }
    }

    if (pstListenerCtx->s32ListenSock > maxFdNum)
    {maxFdNum = pstListenerCtx->s32ListenSock;}
    *pMaxFdNum = maxFdNum + 1;
    return HI_FAILURE;
}

static HI_BOOL NETWORK_Listener_CheckFd(HiNetListenerCtx* pstListenerCtx, fd_set* pFdSet, HI_S32* pSockFd)
{
    HI_S32  i = 0;

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != INVALID_SOCKETFD)
        {
            HI_S32 s32Sockfd = pstListenerCtx->s32ClientSockList[i];
            if (FD_ISSET(s32Sockfd, pFdSet))
            {
                *pSockFd = s32Sockfd;
                return HI_TRUE;
            }
        }
    }

    return HI_FALSE;
}

static HI_S32 NETWORK_Listener_CreateSocket(HI_S32 s32ListenPort, HI_S32* pListenSock)
{
    HI_S32 s32ListenSockfd = INVALID_SOCKETFD;      /*temp listen socket*/
    HI_S32 socketOptVal = 1;
    HI_S32 s32Ret = 0;
    struct sockaddr_in svr_addr = {0};
    struct timeval timeout = {KEEPALIVE_IDLE_SECONDS,0};

    /* 1. Create socket as block mode and listen for accept new connection.*/
    s32ListenSockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (s32ListenSockfd  < 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "Creat  lisn sock error.\r\n");
        return HI_FAILURE;
    }

    /*2. set sock option*/
    if (-1 == setsockopt(s32ListenSockfd, SOL_SOCKET, SO_REUSEADDR, &socketOptVal, sizeof(HI_S32)))
    {
        close(s32ListenSockfd);
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set socket option SO_REUSEADDR error.\n");
        return HI_FAILURE;
    }
    printf("NETWORK_Listener_CreateSocket set timeout \n");
    if (-1 == setsockopt(s32ListenSockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval)))
    {
        close(s32ListenSockfd);
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set socket option SO_SNDTIMEO error.\n");
        return HI_FAILURE;
    }
    if (-1 == setsockopt(s32ListenSockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval)))
    {
        close(s32ListenSockfd);
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set socket option SO_RCVTIMEO error.\n");
        return HI_FAILURE;
    }

    s32Ret = NETWORK_Listener_SetKeepAlive(s32ListenSockfd, 1, KEEPALIVE_IDLE_SECONDS, 1, 3);
    if( HI_SUCCESS!=s32Ret )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "SetKeepAlive error=%d\n", s32Ret);
    }

    /*3. bind socket */
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port  =  htons(s32ListenPort);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (HI_SUCCESS != memset_s(&(svr_addr.sin_zero), sizeof(svr_addr.sin_zero), '\0', sizeof(svr_addr.sin_zero)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set sin_zero fail \n");
        close(s32ListenSockfd);
        return HI_FAILURE;
    }

    if (bind(s32ListenSockfd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)) != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "bind fail  error:%s  \n", strerror(errno));
        close(s32ListenSockfd);
        return HI_FAILURE;
    }

    /*4. listen on port*/
    s32Ret = listen(s32ListenSockfd, DEFAULT_MAX_FDNUM);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "socket listen error.\n");
        close(s32ListenSockfd);
        return HI_FAILURE;
    }

    *pListenSock = s32ListenSockfd;
    return HI_SUCCESS;
}

/*main func of listen thread */
static HI_VOID* NETWORK_Listener_Process(void* args)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32RecvBytes = 0;
    HiNetListenerCtx* pListenerCtx = (HiNetListenerCtx*) args;
    struct timeval lisnTimeoutVal;             /* Timeout value */
    fd_set read_fds;
    fd_set read_active_fds;
    HI_S32 s32MaxFdNum = 0;                    /*max socket number of read set*/
    HI_S32 s32AcceptSock = INVALID_SOCKETFD;             /*accepted client connect socket */
    struct sockaddr_in acceptAddr;      /*socket addr of accepted client connect*/
    HI_S32    s32SockAddrLen  = 0;
    if (HI_SUCCESS != memset_s(&lisnTimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, " Lorin Add ->set lisnTimeoutVal fail \n");
        return HI_NULL;
    }

    if (HI_SUCCESS != memset_s(&acceptAddr, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set acceptAddr fail \n");
        return HI_NULL;
    }
    s32Ret = NETWORK_Listener_CreateSocket(pListenerCtx->s32ListenPort, &pListenerCtx->s32ListenSock);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "create HI_NETWORK_Listener_Start failed error:%d \n", s32Ret);
        return HI_NULL;
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_INFO, "start listener \n");
    /*init read fds and set http lisen sock into read set*/
    FD_ZERO(&read_fds);
    FD_ZERO(&read_active_fds);
    s32MaxFdNum = pListenerCtx->s32ListenSock + 1;
    FD_SET(pListenerCtx->s32ListenSock, &read_active_fds);
    prctl(PR_SET_NAME, (unsigned long)"NetlistenProc", 0, 0, 0);

    while (pListenerCtx->bInListening)
    {
        lisnTimeoutVal.tv_sec = NETLISTEN_TIMOUT_SEC;
        lisnTimeoutVal.tv_usec = NETLISTEN_TIMOUT_USEC;
        if (HI_SUCCESS != memcpy_s(&read_fds, sizeof(fd_set), &read_active_fds, sizeof(fd_set)))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "copy read_fds fail \n");
            break;
        }
        /*jude if there is new connect or first message come through already connected link */
        s32Ret = select(s32MaxFdNum, &read_fds, NULL, NULL, &lisnTimeoutVal);
        if ( s32Ret < 0)
        {
            if (EINTR == errno || EAGAIN == errno)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, " [select err: %s]\n",  strerror(errno));
                continue;
            }

            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "listen thread: select error=%s\n", strerror(errno));
            break;
        }
        /*to do :select timeout over proset*/
        else if ( 0 == s32Ret)
        {
            continue;
        }
        if (FD_ISSET(pListenerCtx->s32ListenSock, &read_fds))
        {
            /*accept new connect*/
            s32SockAddrLen = sizeof(acceptAddr);
            s32AcceptSock = accept(pListenerCtx->s32ListenSock, (struct sockaddr*)&acceptAddr, (socklen_t*)&s32SockAddrLen);
            if (s32AcceptSock < 0)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "accept conn error=%s \r\n", strerror(errno));
                continue;
            }

            // TCP socket may block in many situations, e.g. wifi disconnected
            // so we use nonblock socket and enable TCP keep alive.
            s32Ret = NETWORK_Listener_SetNonblock(s32AcceptSock);
            if( HI_SUCCESS!=s32Ret )
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "SetNonblock error=%d\n", s32Ret);
            }

            s32Ret = NETWORK_Listener_SetKeepAlive(s32AcceptSock, 1, KEEPALIVE_IDLE_SECONDS, 1, 3);
            if( HI_SUCCESS!=s32Ret )
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "SetKeepAlive error=%d\n", s32Ret);
            }

            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_INFO, " a new connect has occured fd\r\n");
            /*add new connection link into readfds*/
            if ( s32AcceptSock + 1 > s32MaxFdNum )
            {
                s32MaxFdNum = s32AcceptSock  + 1;
            }

            FD_SET(s32AcceptSock , &read_active_fds);
            NETWORK_Listener_AddFd(pListenerCtx, s32AcceptSock);
        }
        else
        {
            HI_S32 clientSockfd = INVALID_SOCKETFD;
            if (NETWORK_Listener_CheckFd(pListenerCtx, &read_fds, &clientSockfd))
            {
                memset_s(pListenerCtx->aszFirstMsgBuff, MAX_RECV_BUFFER, 0, MAX_RECV_BUFFER);
                s32RecvBytes = recv(clientSockfd, pListenerCtx->aszFirstMsgBuff, MAX_RECV_BUFFER - 1, 0);
                FD_CLR(clientSockfd, &read_active_fds);
                NETWORK_Listener_DelFd(pListenerCtx, clientSockfd);
                /*updat Max read Fds Num*/
                NETWORK_Listener_UpdateMaxFd(pListenerCtx, &s32MaxFdNum);
                if (s32RecvBytes <= 0)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "recv data from  error= %s\n",  strerror(errno));
                    HI_NETWORK_CloseSocket(&clientSockfd);
                    continue;
                }

                /*callback to user*/
                if (pListenerCtx->connectCallback)
                {
                    pListenerCtx->connectCallback(pListenerCtx->callbackObj, clientSockfd, pListenerCtx->aszFirstMsgBuff,  s32RecvBytes);
                }
            }
        }
    }   /*end of while*/
    /*close the client fd before connect*/
    NETWORK_Listener_CloseClientFd(pListenerCtx);
    /*close the listen socket*/
    HI_NETWORK_CloseSocket(&pListenerCtx->s32ListenSock);
    HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_INFO, "Listen thread  stoped\n");

    return HI_NULL;
}


HI_S32 HI_NETWORK_Listener_Create(HI_S32 s32ListenPort,  HI_MW_PTR* pHandle)
{
    /* malloc listen server manage struct*/
    HI_S32 i = 0;
    if (s32ListenPort <= 0 || s32ListenPort > 65535)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "invalid listen port  \n");
        return HI_FAILURE;
    }
    HiNetListenerCtx* pListenCtx = (HiNetListenerCtx*)malloc(sizeof(HiNetListenerCtx));
    if (!pListenCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "malloc for lisn server error.\r\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != memset_s(pListenCtx, sizeof(HiNetListenerCtx), 0x00, sizeof(HiNetListenerCtx)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set pListenCtx fail \n");
        free(pListenCtx);
        return HI_FAILURE;
    }

    for (; i < DEFAULT_MAX_FDNUM; i++)
    {
        pListenCtx->s32ClientSockList[i] = INVALID_SOCKETFD;
    }

    pListenCtx->s32ListenPort = s32ListenPort;
    *pHandle = (HI_MW_PTR)pListenCtx;

    return HI_SUCCESS;
}

/*stop the lisen svr*/
HI_S32 HI_NETWORK_Listener_Destroy(HI_MW_PTR handle)
{
    HiNetListenerCtx* pListenCtx = (HiNetListenerCtx*)handle;
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_NETWORK_Listener_Stop(handle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_NETWORK_Listener_Stop failed :%d \n", s32Ret);
        return s32Ret;
    }

    if (pListenCtx)
    {
        free(pListenCtx);
        pListenCtx = NULL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_NETWORK_Listener_Start(HI_MW_PTR handle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HiNetListenerCtx* pListenCtx = (HiNetListenerCtx*)handle;
    if ( HI_TRUE == pListenCtx->bInListening )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "network listener already started in listening\n");
        return HI_FAILURE;
    }
    pListenCtx->bInListening = HI_TRUE;
#ifdef __LITEOS__
    /* Values for inheritsched*/
#define PTHREAD_INHERIT_SCHED           1
#define PTHREAD_EXPLICIT_SCHED          2
    pthread_attr_t attrSvr;
    pthread_attr_init (&attrSvr);
    pthread_attr_setinheritsched (&attrSvr, PTHREAD_EXPLICIT_SCHED);
    attrSvr.inheritsched = PTHREAD_EXPLICIT_SCHED;
    attrSvr.schedparam.sched_priority = 6;
    pthread_attr_setstacksize (&attrSvr, 64 * 1024);
    s32Ret = pthread_create(&(pListenCtx->ptListenThd),
                            &attrSvr, NETWORK_Listener_Process, (HI_VOID*)pListenCtx);
#else
    s32Ret = pthread_create(&(pListenCtx->ptListenThd),
                            NULL, NETWORK_Listener_Process, (HI_VOID*)pListenCtx);
#endif

    return s32Ret;
}


HI_S32 HI_NETWORK_Listener_Stop(HI_MW_PTR handle)
{
    HiNetListenerCtx* pListenCtx = (HiNetListenerCtx*)handle;
    HI_S32  s32Ret = HI_SUCCESS;

    if (!pListenCtx)
    {
        return HI_FAILURE;
    }

    /*set the listen svr stop flag*/
    if (pListenCtx->bInListening)
    {
        pListenCtx->bInListening = HI_FALSE;
        s32Ret = pthread_join(pListenCtx->ptListenThd, NULL);
        if (s32Ret != 0)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "destroy network listener  thread error\n");
            return  s32Ret;
        }
    }

    return HI_SUCCESS;
}


HI_S32 HI_NETWORK_Listener_RegisterCallback(HI_MW_PTR handle, onClientConnection onConnection, HI_MW_PTR obj)
{
    HiNetListenerCtx* pListenCtx = (HiNetListenerCtx*)handle;
    pListenCtx->connectCallback = onConnection;
    pListenCtx->callbackObj = obj;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
