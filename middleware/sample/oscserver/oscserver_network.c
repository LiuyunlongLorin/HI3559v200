#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*osc stream*/
#include "sample_oscserver.h"
#include "hi_osc_server.h"
#include "oscserver_network.h"

#ifndef __HuaweiLite__
#include <signal.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern HI_OSCSAMPLE_FILE_S g_stListFiles[OSCSAMPLE_LISTFILES_NUMBER];
extern pthread_mutex_t g_OSCFILELock;          /*the lock of file*/

HI_S32 OSCSVR_Create_Listener(HI_S32 s32HttpPort,  HI_MW_PTR* pNetHandle)
{
    /* malloc listen server manage struct*/
    HI_S32 i = 0;

    if (s32HttpPort <= 0 || s32HttpPort > 65535)
    {
        printf("invalid listen port!!\n");
        return HI_FAILURE;
    }

    HiOSC_NETWORK_CTX* pListenCtx = (HiOSC_NETWORK_CTX*)malloc(sizeof(HiOSC_NETWORK_CTX));

    if (!pListenCtx)
    {
        printf("malloc for lisn server error.\r\n");
        return HI_FAILURE;
    }

    memset_s(pListenCtx, sizeof(HiOSC_NETWORK_CTX), 0x00, sizeof(HiOSC_NETWORK_CTX));

    for (i = 0; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        pListenCtx->s32ClientSockList[i] = OSC_INVALID_SOCKETFD;
    }

    for (i = 0; i < OSC_DEFAULT_MAX_THD_NUM; i++)
    {
        pListenCtx->lThdList[i] = OSC_INVALID_THID;
    }

    pListenCtx->s32HttpPort = s32HttpPort;

    *pNetHandle = (HI_MW_PTR)pListenCtx;

    return HI_SUCCESS;

}
static HI_S32 OSCSVR_Listener_AddFd(HiOSC_NETWORK_CTX* pstListenerCtx, HI_S32 sockfd)
{
    HI_S32  i = 0;

    for (; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] == OSC_INVALID_SOCKETFD)
        {
            pstListenerCtx->s32ClientSockList[i] = sockfd;
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

static HI_BOOL OSCSVR_Listener_CheckFd(HiOSC_NETWORK_CTX* pstListenerCtx, fd_set* pFdSet, HI_S32* pSockFd)
{
    HI_S32  i = 0;

    for (; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != OSC_INVALID_SOCKETFD)
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

static HI_S32 OSCSVR_Listener_DelFd(HiOSC_NETWORK_CTX* pstListenerCtx, HI_S32 sockfd)
{
    HI_S32  i = 0;

    for (; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] == sockfd)
        {
            pstListenerCtx->s32ClientSockList[i] = OSC_INVALID_SOCKETFD;
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

static HI_S32 OSCSVR_Listener_UpdateMaxFd(HiOSC_NETWORK_CTX* pstListenerCtx, HI_S32* pMaxFdNum)
{
    HI_S32 maxFdNum = -1;
    HI_S32  i = 0;

    for (; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != OSC_INVALID_SOCKETFD && pstListenerCtx->s32ClientSockList[i] > maxFdNum)
        {
            maxFdNum = pstListenerCtx->s32ClientSockList[i];
        }
    }

    if (pstListenerCtx->s32ListenSock > maxFdNum)
    {
        maxFdNum = pstListenerCtx->s32ListenSock;
    }

    *pMaxFdNum = maxFdNum + 1;
    return HI_FAILURE;
}

static HI_S32 OSCSVR_Listener_CloseClientFd(HiOSC_NETWORK_CTX* pstListenerCtx)
{
    HI_S32  i = 0;

    for (; i < OSC_DEFAULT_MAX_FDNUM; i++)
    {
        if (pstListenerCtx->s32ClientSockList[i] != OSC_INVALID_SOCKETFD)
        {
            close(pstListenerCtx->s32ClientSockList[i]) ;
            pstListenerCtx->s32ClientSockList[i] = OSC_INVALID_SOCKETFD;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_CreateSocket(HI_S32 s32ListenPort, HI_S32* pListenSock)
{
    HI_S32 s32ListenSockfd = OSC_INVALID_SOCKETFD;      /*temp listen socket*/
    HI_S32 socketOptVal = 1;
    HI_S32 s32Ret = 0;
    struct sockaddr_in svr_addr;
    /* 1. Create socket as block mode and listen for accept new connection.*/

    s32ListenSockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (s32ListenSockfd  < 0)
    {
        printf("OSCSVR Create listen sock error.\r\n");
        return HI_FAILURE;
    }

    /*2. set sock option*/
    if (-1 == setsockopt(s32ListenSockfd, SOL_SOCKET, SO_REUSEADDR, &socketOptVal, sizeof(HI_S32)))
    {
        close(s32ListenSockfd);
        printf("OSCSVR set socket option SO_REUSEADDR error.\n");
        return HI_FAILURE;
    }

    /*3. bind socket */
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port  =  htons(s32ListenPort);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset_s(&(svr_addr.sin_zero), sizeof(svr_addr.sin_zero), '\0', sizeof(svr_addr.sin_zero));

    if (bind(s32ListenSockfd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)) != HI_SUCCESS)
    {
        printf("OSCSVR bind fail!! error:%s  \n", strerror(errno));
        close(s32ListenSockfd);
        return HI_FAILURE;
    }

    /*4. listen on port*/
    s32Ret = listen(s32ListenSockfd, OSC_DEFAULT_MAX_FDNUM);

    if (s32Ret != HI_SUCCESS)
    {
        printf("OSCSVR socket listen error.\n");
        close(s32ListenSockfd);
        return HI_FAILURE;
    }

    *pListenSock = s32ListenSockfd;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_CloseSocket(HI_S32* ps32Socket)
{
    if (*ps32Socket >= 0)
    {
        close(*ps32Socket);
        *ps32Socket = OSC_INVALID_SOCKETFD;
        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

HI_S32 OSCSVR_Response_Send(HI_S32 s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32RemSize = 0;
    HI_S32 s32Size    = 0;
    HI_CHAR*  ps8BufferPos = NULL;
    HI_S32 s32Errno = 0;
    fd_set write_fds;
    struct timeval TimeoutVal;  /* Timeout value */

    memset_s(&TimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval));
    u32RemSize = u32DataLen;
    ps8BufferPos = pszBuff;

    while (u32RemSize > 0)
    {
        FD_ZERO(&write_fds);
        FD_SET(s32WritSock, &write_fds);
        TimeoutVal.tv_sec = 10;
        TimeoutVal.tv_usec = 0;
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
                        printf("OSCSVR response Send error:%s\n", strerror(s32Errno));
                        return HI_FAILURE;
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
                printf("OSCSVR response Send error:%s fd not in fd_set\n", strerror(s32Errno));
                return HI_FAILURE;;
            }
        }
        /*select found over time or error happend*/
        else if ( s32Ret == 0 )
        {
            s32Errno = errno;
            printf("OSCSVR response Send error:%s  select overtime %d.%ds\n", strerror(s32Errno), 10, 0);
            return HI_FAILURE;
        }
        else if ( s32Ret < 0 )
        {
            s32Errno = errno;
            printf("OSCSVR response Send error!\n");
            return HI_FAILURE;
        }

    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_Get_HTTPResponse(HI_S32 s32StateCode, HI_CHAR* pszReply, HI_S32* ps32ReplyLen, HI_S32 s32BinaryLen)
{
    HI_S32  s32TempLen = 0;
    HI_S32  s32ReplyLen = 512;

    if (NULL == pszReply)
    {
        printf("generate Get reply header fail!!\n");
        return HI_FAILURE;
    }

    s32TempLen += snprintf_s(pszReply, s32ReplyLen, s32ReplyLen, "HTTP/1.1 %d OK\r\n", s32StateCode);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Pragma: no-cache\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Expires: 0\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Max-Age: 0\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Content-Type: image/jpeg\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Content-Length: %d\r\n", s32BinaryLen);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "X-Content-Type-Options: nosniff\r\n");
    /*Connection: close*/
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Connection: close\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "\r\n");

    *ps32ReplyLen = s32TempLen;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_CheckIsDownload(HI_CHAR* pcUrl, HI_S32 s32UrlLen)
{
    HI_S32 s32Looper = 0;
    HI_BOOL bFlag = HI_FALSE;

    OSCSAM_LOCK(g_OSCFILELock);

    for (s32Looper = 0; s32Looper < OSCSAMPLE_LISTFILES_NUMBER; s32Looper ++)
    {
        if (0 !=  strlen(g_stListFiles[s32Looper].aszFileName))
        {
            if (0 == strncmp(pcUrl, g_stListFiles[s32Looper].aszDownURL, strlen(g_stListFiles[s32Looper].aszDownURL)))
            {
                g_stListFiles[s32Looper].bDownLoad = HI_TRUE;
                bFlag = HI_TRUE;
                break;
            }
        }
    }

    OSCSAM_UNLOCK(g_OSCFILELock);

    if (bFlag == HI_FALSE)
    {
        printf("the url need to download is in the filelist!\n");
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_ProcessDownload(HI_S32* p32SocketFd, HI_CHAR* pcUrl, HI_S32 s32UrlLen)
{
    FILE* ImageFile = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32FileSize = 0;
    HI_CHAR aszBinaryData[OSCSAM_IMAGES_BINARAY_DATA_LEN + 1] = {0};
    HI_S32  s32BinaryLen = 0;
    HI_S32  s32StatCode = 200;
    HI_CHAR aszHeader[OSCSAM_DOWNLOAD_IMAGES_HEADER] = {0};
    HI_CHAR aszRealPath[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_S32  s32RespLen = 0;

    if (NULL == pcUrl)
    {
        printf("OSCSVR_Http_Download input is error!!\n");
        return HI_FAILURE;
    }

    printf("#####################download######################\n");

    if (NULL == realpath(pcUrl, aszRealPath))
    {
        printf("the File url:%s is err!!\n", pcUrl);
        return HI_FAILURE;
    }
    /*open image file!!*/
    ImageFile = fopen(aszRealPath, "rb");

    if ( NULL == ImageFile)
    {
        printf("fopen file err!!\n");
        return HI_FAILURE;
    }

    fseek(ImageFile, 0, SEEK_END);
    s32FileSize = ftell(ImageFile);
    fseek(ImageFile, 0, SEEK_SET);

    s32StatCode = 200;
    (HI_VOID)OSCSVR_Generate_Get_HTTPResponse(s32StatCode, aszHeader, &s32RespLen, s32FileSize);
    printf("Http Get response header len is %d!!\n", s32RespLen);

    if (s32RespLen > 0)
    {
        s32Ret = OSCSVR_Response_Send(*p32SocketFd, aszHeader, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            printf("GetImage http header response send fail!!\n");
            fclose(ImageFile);
            ImageFile = NULL;
            OSCSVR_CloseSocket(p32SocketFd);
            return HI_FAILURE;
        }
    }

    while (!feof(ImageFile))
    {
        memset_s(aszBinaryData, OSCSAM_IMAGES_BINARAY_DATA_LEN + 1, 0x00, OSCSAM_IMAGES_BINARAY_DATA_LEN + 1);
        s32BinaryLen = fread(aszBinaryData, 1,  OSCSAM_IMAGES_BINARAY_DATA_LEN, ImageFile);
        s32Ret = OSCSVR_Response_Send(*p32SocketFd, aszBinaryData, s32BinaryLen);

        if (HI_SUCCESS != s32Ret)
        {
            printf("GetImage binary data send fail!!\n");
            fclose(ImageFile);
            ImageFile = NULL;
            OSCSVR_CloseSocket(p32SocketFd);
            return HI_FAILURE;
        }

        usleep(5000);
    }

    fclose(ImageFile);
    ImageFile = NULL;

    (HI_VOID)OSCSVR_CheckIsDownload(pcUrl, s32UrlLen);
    (HI_VOID)OSCSVR_CloseSocket(p32SocketFd);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_CheckIsGetRequest(HI_CHAR* pcClientReq, HI_CHAR* pcUrl, HI_S32 s32UrlLen, HI_BOOL* pbDownload)
{
    HI_S32 s32Cnt = 0;
    HI_CHAR aszVersion[32] = {0};
    HI_CHAR aszUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszMethod[16] = {0};

    if ((NULL == pcClientReq) || (NULL == pcUrl))
    {
        printf("OSCSVR_Http_Download input is error!!\n");
        return HI_FAILURE;
    }

    s32Cnt = sscanf(pcClientReq, "%15s %63s %s", aszMethod, aszUrl, aszVersion);

    if (3 != s32Cnt)
    {
        printf("OSC sscanf err!!\n");
        return HI_FAILURE;
    }

    aszVersion[31] = '\0';
    aszUrl[HI_OSC_FILEURL_MAX_LEN - 1] = '\0';
    aszMethod[15] = '\0';

    strncpy_s(pcUrl, s32UrlLen, aszUrl, strlen(aszUrl) + 1);

    if ((0 == strncmp(aszMethod, "GET", strlen("GET"))) && ( NULL != strchr(aszUrl, '.')))
    {
        printf("http GET request!!\n");
        *pbDownload = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_VOID* OSCSVR_Msg_Process(void* args)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_OSC_NET_S* pNetCtx = (HI_OSC_NET_S*)args;
    HiOSC_NETWORK_CTX* pListenCtx = (HiOSC_NETWORK_CTX*)pNetCtx->s32ServerHandle;
    HI_BOOL bDownload = HI_FALSE;
    HI_CHAR aszUrl[HI_OSC_FILEURL_MAX_LEN] = {0};

    OSCSVR_CheckIsGetRequest(pNetCtx->aszFirstMsgBuff, aszUrl, HI_OSC_FILEURL_MAX_LEN, &bDownload);

    if (bDownload)
    {
        if (HI_SUCCESS != OSCSVR_ProcessDownload(&pNetCtx->s32CurrentFd, aszUrl, HI_OSC_FILEURL_MAX_LEN))
        {
            printf("OSCSVR_ProcessDownload process fail!!!\r\n");
            *pNetCtx->plThreadId = OSC_INVALID_THID;
            OSCSVR_CloseSocket(&pNetCtx->s32CurrentFd);
            free(pNetCtx);
            pNetCtx = NULL;

            return HI_NULL_PTR;
        }

        OSCSVR_CloseSocket(&pNetCtx->s32CurrentFd);
    }
    else
    {
        if (pListenCtx->connectCallback != NULL)
        {
            /*callback to user*/
            s32Ret = pListenCtx->connectCallback(pListenCtx->s32ServerHandle, &pNetCtx->s32CurrentFd, pNetCtx->aszFirstMsgBuff, pNetCtx->s32MsgLen);

            if (HI_SUCCESS !=  s32Ret)
            {
                printf("OSCSVR HI_OSCSVR_HTTP_DistribLink fun fail!!!\r\n");
                *pNetCtx->plThreadId = OSC_INVALID_THID;
                free(pNetCtx);
                pNetCtx = NULL;
                return HI_NULL_PTR;
            }
        }
    }

    *pNetCtx->plThreadId = OSC_INVALID_THID;
    free(pNetCtx);
    pNetCtx = NULL;

    return HI_NULL_PTR;
}

HI_S32 OSCSVR_Create_Process_Thread(HI_MW_PTR pNetHandle, long* plThreadID, HI_S32 s32MsgLen, HI_S32 s32SockFd)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_OSC_NET_S* pstNetMsg = NULL;

    pstNetMsg = (HI_OSC_NET_S*)malloc(sizeof(HI_OSC_NET_S));

    if (NULL == pstNetMsg)
    {
        printf("OSCSVR_Create_Process_Thread malloc fail!!");
        return HI_FAILURE;
    }

    memset_s(pstNetMsg, sizeof(HI_OSC_NET_S), 0x00, sizeof(HI_OSC_NET_S));

    HiOSC_NETWORK_CTX* pListenCtx = (HiOSC_NETWORK_CTX*)pNetHandle;
    memcpy_s(pstNetMsg->aszFirstMsgBuff, OSC_MAX_RECV_BUFFER, pListenCtx->aszFirstMsgBuff, OSC_MAX_RECV_BUFFER);
    pstNetMsg->s32ServerHandle = pNetHandle;
    pstNetMsg->plThreadId = plThreadID;
    pstNetMsg->s32MsgLen = s32MsgLen;
    pstNetMsg->s32CurrentFd = s32SockFd;

    s32Ret = pthread_create((pthread_t*)(pstNetMsg->plThreadId), NULL, OSCSVR_Msg_Process, (HI_VOID*)pstNetMsg);

    if (HI_SUCCESS != s32Ret)
    {
        printf("OSCSVR_Create_Process_Thread create thread fail!!");
        free(pstNetMsg);
        pstNetMsg = NULL;
    }

    return s32Ret;
}

HI_VOID* OSCSVR_Network_Process(HI_MW_PTR pNetHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32RecvBytes = 0;
    HiOSC_NETWORK_CTX* pListenerCtx = (HiOSC_NETWORK_CTX*)pNetHandle;
    struct timeval lisnTimeoutVal;                   /* Timeout value */
    fd_set read_fds;
    fd_set read_active_fds;
    HI_S32 s32MaxFdNum = 0;                          /*max socket number of read set*/
    HI_S32 s32AcceptSock = OSC_INVALID_SOCKETFD;     /*accepted client connect socket */
    struct sockaddr_in acceptAddr;                   /*socket addr of accepted client connect*/
    HI_S32 s32SockAddrLen  = 0;
    HI_S32 s32Looper = 0;
    HI_S32 clientSockfd = OSC_INVALID_SOCKETFD;

    memset_s(&lisnTimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval));
    memset_s(&acceptAddr, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in));

    s32Ret = OSCSVR_CreateSocket(pListenerCtx->s32HttpPort, &pListenerCtx->s32ListenSock);

    if (s32Ret != HI_SUCCESS)
    {
        printf("create OSCSVR_CreateSocket failed error:%d \n", s32Ret);
        return HI_NULL_PTR;
    }

    printf("start listener: %d \n", pListenerCtx->bInListening);
    /*init read fds and set http lisen sock into read set*/
    FD_ZERO(&read_fds);
    FD_ZERO(&read_active_fds);
    s32MaxFdNum = pListenerCtx->s32ListenSock + 1;
    FD_SET(pListenerCtx->s32ListenSock, &read_active_fds);

    prctl(PR_SET_NAME, (unsigned long)"OSChttpProc", 0, 0, 0);

    while (pListenerCtx->bInListening)
    {
        lisnTimeoutVal.tv_sec = OSC_NET_TIMOUT_SEC;
        lisnTimeoutVal.tv_usec = OSC_NET_TIMOUT_USEC;
        memcpy_s(&read_fds, sizeof(fd_set), &read_active_fds, sizeof(fd_set));
        /*jude if there is new connect or first message come through already connected link */
        s32Ret = select(s32MaxFdNum, &read_fds, NULL, NULL, &lisnTimeoutVal);

        if ( -1 == s32Ret)
        {
            printf("OSCSVR listen thread: select error=%s\n", strerror(errno));
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
                printf("accept conn error=%s \r\n", strerror(errno));
                continue;
            }

            printf("*******OSCSVR SAMPLE a new connect has occured fd***********\r\n");

            /*add new connection link into readfds*/
            if ( s32AcceptSock + 1 > s32MaxFdNum )
            {
                s32MaxFdNum = s32AcceptSock  + 1;
            }

            FD_SET(s32AcceptSock , &read_active_fds);
            OSCSVR_Listener_AddFd(pListenerCtx, s32AcceptSock);
        }
        else
        {
            if (OSCSVR_Listener_CheckFd(pListenerCtx, &read_fds, &clientSockfd))
            {
                memset_s(pListenerCtx->aszFirstMsgBuff, OSC_MAX_RECV_BUFFER, 0x00, OSC_MAX_RECV_BUFFER);
                s32RecvBytes = recv(clientSockfd, pListenerCtx->aszFirstMsgBuff, OSC_MAX_RECV_BUFFER - 1, 0);

                if (s32RecvBytes <= 0)
                {
                    printf("OSCSVR recv data from  error= %s\n",  strerror(errno));
                    OSCSVR_CloseSocket(&clientSockfd);
                    continue;
                }

                FD_CLR(clientSockfd, &read_active_fds);
                OSCSVR_Listener_DelFd(pListenerCtx, clientSockfd);
                /*updat Max read Fds Num*/
                OSCSVR_Listener_UpdateMaxFd(pListenerCtx, &s32MaxFdNum);
                pListenerCtx->s32CurrentFd = clientSockfd;

                for (s32Looper = 0; s32Looper < OSC_DEFAULT_MAX_THD_NUM; s32Looper ++)
                {
                    if (pListenerCtx->lThdList[s32Looper] == OSC_INVALID_THID)
                    {
                        printf(">>>>>>>>>>%d, %ld, %ld\n%s\n", s32Looper, pListenerCtx->lThdList[s32Looper], pListenerCtx->lThdList[0], pListenerCtx->aszFirstMsgBuff);
                        s32Ret = OSCSVR_Create_Process_Thread(pNetHandle, &pListenerCtx->lThdList[s32Looper], s32RecvBytes, clientSockfd);

                        if ( HI_SUCCESS != s32Ret)
                        {
                            printf("OSCSVR create process thread fail!!\n");
                            OSCSVR_CloseSocket(&clientSockfd);
                            break;
                        }

                        break;
                    }
                }
            }
        }
    }/*end of while*/

    /*close the client fd before connect*/
    OSCSVR_Listener_CloseClientFd(pListenerCtx);

    if (pListenerCtx->s32ListenSock != OSC_INVALID_SOCKETFD)
    {
        close(pListenerCtx->s32ListenSock) ;
        pListenerCtx->s32ListenSock = OSC_INVALID_SOCKETFD;
    }

    printf("Listen thread  stoped\n");

    return HI_NULL_PTR;
}

HI_S32 OSCSVR_Listen_Start(HI_MW_PTR pNetHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HiOSC_NETWORK_CTX* pListenerCtx = (HiOSC_NETWORK_CTX*)pNetHandle;

#ifndef __HuaweiLite__
    /* block Signal Handler SIGPIPE */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);
#endif
    if ( HI_TRUE == pListenerCtx->bInListening )
    {
        printf("network listener already started in listening\n");
        return HI_FAILURE;
    }

    pListenerCtx->bInListening = HI_TRUE;

    s32Ret = pthread_create((pthread_t*) & (pListenerCtx->lListenThd),
                            NULL, (HI_VOID*)OSCSVR_Network_Process, (HI_VOID*)pListenerCtx);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
