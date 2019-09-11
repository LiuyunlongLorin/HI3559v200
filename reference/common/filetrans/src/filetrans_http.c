/**
 * @file    filetrans_http.c
 * @brief   filetrans http module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/26
 * @version   1.0

 */
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include "hi_appcomm.h"
#include "filetrans_http.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define FILETRANS_SVRVERSION HI_MODULE"("__DATE__")"
#define FILETRANS_TIMEOUT (1)
#define FILETRANS_DATETIME_LEN (32)
#define FILETRANS_RETRAY_TIMES (200)
#define FILETRANS_SEND_RETRAY_INTERVAL (5000)
#define FILETRANS_SEND_BUFF_LEN (10000)

typedef struct tagHTTP_STATUS_S
{
    const HI_CHAR* pszToken;
    HI_S32 s32Code;
} HTTP_STATUS_S;

static HTTP_STATUS_S s_HTTP_Status [] =
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
    {"Not Found", 404},
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
    {"Internal Server Error", 500},
    {"Not Implemented", 501},
    {"Bad Gateway", 502},
    {"Service Unavailable", 503},
    {"Gateway Time-out", 504},
    {"HTTP Version Not Supported", 505},
    {"Extended Error:", 911},
    {NULL, -1}
};

static const HI_CHAR* FILETRANS_GetHttpStatus(HI_S32 s32Code)
{
    HTTP_STATUS_S* pStatus;

    for ( pStatus = s_HTTP_Status; pStatus->s32Code != -1; pStatus++ )
    {
        if ( pStatus->s32Code == s32Code)
        {
            return ( pStatus->pszToken);
        }
    }

    return NULL;
}

static HI_S32 FILETRANS_ComposeHttpResp(const FILETRANS_RESP_INFO_S* stRespInfo, HI_CHAR* pszResp, HI_S32 s32RespLen)
{
    HI_S32 s32Len = 0;
    s32Len += snprintf(pszResp, s32RespLen, "HTTP/1.1 %d %s\r\n", stRespInfo->s32ReturnCode, FILETRANS_GetHttpStatus(stRespInfo->s32ReturnCode));
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    HI_CHAR Datetime[FILETRANS_DATETIME_LEN] = {0};
    strftime(Datetime, FILETRANS_DATETIME_LEN, "%a, %d %b %Y %H:%M:%S %Z", &tm_now);
    s32Len += snprintf(pszResp + s32Len, s32RespLen - s32Len, "Date: %s\r\n", Datetime);
    s32Len += snprintf(pszResp + s32Len, s32RespLen - s32Len, "Server: %s\r\n", FILETRANS_SVRVERSION);

    if (0 < strnlen(stRespInfo->szContentType, FILETRANS_CONTENT_TYPE_LEN))
    {
        s32Len += snprintf(pszResp + s32Len, s32RespLen - s32Len, "Content-Type: %s\r\n", stRespInfo->szContentType);
    }

    s32Len += snprintf(pszResp + s32Len, s32RespLen - s32Len, "Content-Length: %d\r\n", stRespInfo->s32ContentLen);
    s32Len += snprintf(pszResp + s32Len, s32RespLen - s32Len, "Connection: %s\r\n\r\n", stRespInfo->szConnection);
    return HI_SUCCESS;
}

/**
 * @brief    parse the data of http requset.
 * @param[in] pszReq:http requset packet.
 * @param[out] pstReqInfo:requset information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_ParseHttpReq(const HI_CHAR* pszReq, FILETRANS_REQ_INFO_S* pstReqInfo)
{
    /*get file path*/
    HI_CHAR*  pszTemp1 = (HI_CHAR*)strstr(pszReq, "/");

    if (NULL == pszTemp1)
    {
        MLOGE("invalid url:%s", pszReq);
        return  HI_EINVAL;
    }

    pszTemp1 = pszTemp1 + 1;
    HI_CHAR*  pszTemp2 = strstr(pszTemp1, " ");

    if (NULL == pszTemp2)
    {
        MLOGE("invalid url:%s", pszReq);
        return  HI_EINVAL;
    }

    snprintf(pstReqInfo->szFileName, MIN(HI_APPCOMM_MAX_PATH_LEN, pszTemp2 - pszTemp1 + 1), pszTemp1);
    pszTemp1 = strstr(pszReq, "Connection:");

    if (NULL == pszTemp1)
    {
        MLOGE("invalid url:%s", pszReq);
        return  HI_EINVAL;
    }

    if ((NULL == strstr(pszTemp1, FILETRANS_HTTP_KEEPALIVE)) && (NULL == strstr(pszTemp1, "keep-alive")))
    {
        pstReqInfo->bKeepAlive = HI_FALSE;
    }
    else
    {
        pstReqInfo->bKeepAlive = HI_TRUE;
    }

    return HI_SUCCESS;
}

/**
 * @brief    send http response packet.
 * @param[in] s32Socket:http connection socket.
 * @param[in] pstRespInfo:response information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendHttpResp(HI_S32 s32Socket, const FILETRANS_RESP_INFO_S* pstRespInfo)
{
    HI_CHAR szResp[FILETRANS_HTTP_BUFFLEN] = {0};
    memset(szResp, 0, sizeof(szResp));
    FILETRANS_ComposeHttpResp(pstRespInfo, szResp, sizeof(szResp));
    MLOGD(LIGHT_BLUE"\n%s\n"NONE, szResp);
    return FILETRANS_SendData(s32Socket, (HI_U8*)szResp, strnlen(szResp, FILETRANS_HTTP_BUFFLEN));
}

/**
 * @brief    recv http requset data.
 * @param[in] s32Socket:http connection socket.
 * @param[out] pszRecvBuf:recv buff.
 * @param[in] u32BufSize:recv buff size.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_RecvHttpReq(HI_S32 s32Socket, HI_CHAR* pszRecvBuf, HI_U32 u32BufSize)
{
    fd_set stFdSet;
    HI_S32 s32RtnCode = -1;
    HI_U32 u32RecvSize = 0;
    HI_U32 u32MaxRecvLen = u32BufSize - 1;
    HI_S32 s32Retry = 0;
    struct timeval struTimeout;
    FD_ZERO(&stFdSet);
    FD_SET(s32Socket, &stFdSet);
    struTimeout.tv_sec = FILETRANS_TIMEOUT;
    struTimeout.tv_usec = 0;

    while (u32RecvSize < u32MaxRecvLen)
    {
        s32RtnCode = select(s32Socket + 1, &stFdSet, NULL, NULL, &struTimeout);

        if (0 > s32RtnCode)
        {
            if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
            {
                s32Retry++;

                if (s32Retry < FILETRANS_RETRAY_TIMES)
                {
                    continue;
                }
            }

            MLOGE("select fail:%s\n", strerror(errno));
            return HI_EINTER;
        }
        else if (0 == s32RtnCode)
        {
            /**connection timeout*/
            return HI_EINTR;
        }

        s32RtnCode = recv(s32Socket, pszRecvBuf + u32RecvSize, u32MaxRecvLen - u32RecvSize, 0);

        if (0 > s32RtnCode)
        {
            if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
            {
                s32Retry++;

                if (s32Retry < FILETRANS_RETRAY_TIMES)
                {
                    continue;
                }
            }

            MLOGE("recv fail:%s\n", strerror(errno));
            break;
        }
        else if (0 == s32RtnCode)
        {
            /**connection shutdown*/
            return HI_EINTR;
        }

        u32RecvSize += s32RtnCode;

        if (u32RecvSize >= 4)
        {
            if (pszRecvBuf[u32RecvSize - 1] == '\n' && pszRecvBuf[u32RecvSize - 2] == '\r'
                && pszRecvBuf[u32RecvSize - 3] == '\n' && pszRecvBuf[u32RecvSize - 4] == '\r')
            {
                /**recv end*/
                break;
            }
        }
    }

    if (0 == u32RecvSize)
    {
        return HI_EINTER;
    }

    if (u32RecvSize >= u32MaxRecvLen)
    {
        MLOGE(" http response 's pack over %d bytes "\
              "and http procedure failed\n", (u32RecvSize - u32MaxRecvLen));
        return HI_FAILURE;
    }

    pszRecvBuf[u32RecvSize] = '\0';
    MLOGD(BROWN"\n%s\n"NONE, pszRecvBuf);
    return HI_SUCCESS;
}

/**
 * @brief    read file and send by s32Socket.
 * @param[in] s32Socket:http connection socket.
 * @param[in] fpFile:file ptr.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendFile(HI_S32 s32Socket, FILE* fpFile)
{
    HI_U8  u8SendBuf[FILETRANS_SEND_BUFF_LEN] = {0};
    HI_U32 u32ReadLen = FILETRANS_SEND_BUFF_LEN;
    HI_U32 u32SendLen = 0;
    HI_S32 s32RtnCode = 0;
    HI_S32 s32Retry = 0;

    while ((u32ReadLen = fread(u8SendBuf, 1, FILETRANS_SEND_BUFF_LEN , fpFile)) > 0)
    {
        u32SendLen = 0;

        while (u32SendLen < u32ReadLen)
        {
            s32RtnCode = send(s32Socket, u8SendBuf + u32SendLen, u32ReadLen - u32SendLen, 0);

            if (0 > s32RtnCode)
            {
                if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
                {
                    HI_usleep(FILETRANS_SEND_RETRAY_INTERVAL);
                    s32Retry++;

                    if (s32Retry < FILETRANS_RETRAY_TIMES)
                    {
                        continue;
                    }
                }

                MLOGE("send fail:%s\n", strerror(errno));
                return HI_EINTER;
            }

            s32Retry = 0;
            u32SendLen += s32RtnCode;
        }
    }

    return HI_SUCCESS;
}

/**
 * @brief    send data by s32Socket.
 * @param[in] s32Socket:http connection socket.
 * @param[in] pu8Data:data buff.
 * @param[in] u32DataLen:data length.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendData(HI_S32 s32Socket, HI_U8* pu8Data, HI_U32 u32DataLen)
{
    HI_S32 s32RtnCode = -1;
    HI_U32 u32SendLen = 0;
    HI_S32 s32Retry = 0;

    while (u32SendLen < u32DataLen)
    {
        s32RtnCode = send(s32Socket, pu8Data + u32SendLen, u32DataLen - u32SendLen, 0);

        if (0 > s32RtnCode)
        {
            if ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
            {
                HI_usleep(FILETRANS_SEND_RETRAY_INTERVAL);
                s32Retry++;

                if (s32Retry < FILETRANS_RETRAY_TIMES)
                {
                    continue;
                }
            }

            MLOGE("send fail:%s\n", strerror(errno));
            return HI_EINTER;
        }

        u32SendLen += s32RtnCode;
    }

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
