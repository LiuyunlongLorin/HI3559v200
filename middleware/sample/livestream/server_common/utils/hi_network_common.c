/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_common.c
* @brief     livestream common utils src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifndef __LITEOS__
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <net/route.h>
#else
#include <lwip/inet.h>
#include <lwip/sockets.h>
#endif
#include "securec.h"
#include "hi_server_log.h"
#include "hi_server_state_listener.h"
#include "hi_network_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define UNUSED(x) (void)x
static HI_CHAR s_dateBuf[MAX_DATE_LEN] = {0};
static HI_U8  s_uchBasis64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static HI_U8 s_uchIndex64[128] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   62, 0xff, 0xff, 0xff,   63,
    52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff
};
#define char64(c)  ((c > 127) ? 0xff : s_uchIndex64[(c)])  /*define char64(c)*/

HI_CHAR const* HI_NETWORK_DateHeader(HI_VOID)
{
    time_t tt = time(NULL);
    struct tm* ptime = NULL;
    ptime = gmtime(&tt);
    if (NULL == ptime )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "gmtime return null!\n");
        return HI_NULL;
    }

    if (strftime(s_dateBuf, sizeof(s_dateBuf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", ptime))
    {
        return s_dateBuf;
    }

    return HI_NULL;
}

HI_VOID HI_NETWORK_RandomNum(HI_U32* pSsrc)
{
    FILE* pFileRand = fopen("/dev/urandom", "r");
    if (NULL == pFileRand)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "fread file error\n");
        return;
    }

    if (1 != fread(pSsrc, sizeof(HI_U32), 1, pFileRand))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "fread file error\n");
    }

    fclose(pFileRand);
    return;
}
HI_VOID HI_NETWORK_RandomID(HI_CHAR* pSsrc, HI_S32 s32Len)
{
    HI_S32 i;
    FILE* file = fopen("/dev/urandom", "r");

    if (NULL == file)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "fopen file error\n");
        return;
    }

    for ( i = 0; i < s32Len ; ++i )
    {
        if (fread(&pSsrc[i], sizeof(char), 1, file) != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "fread file error\n");
            fclose(file);
            return;
        }
        else
        {
            pSsrc[i] =  (pSsrc[i] % 10) + '0';
        }
    }

    pSsrc[s32Len] = 0;
    fclose(file);
}


HI_S32 HI_NETWORK_Base64Encode(const HI_U8* puchInput, HI_S32 s32InputLen, HI_U8* puchOutput, HI_S32 s32OutputLen)
{
    HI_S32 i = 0;
    HI_S32 j = 0;
    HI_S32 s32Pad;
    UNUSED(s32OutputLen);
    while (i < s32InputLen)
    {
        s32Pad = 3 - (s32InputLen - i);
        if (s32Pad == 2)
        {
            puchOutput[j  ] = s_uchBasis64[puchInput[i] >> 2];
            puchOutput[j + 1] = s_uchBasis64[(puchInput[i] & 0x03) << 4];
            puchOutput[j + 2] = '=';
            puchOutput[j + 3] = '=';
        }
        else if (s32Pad == 1)
        {
            puchOutput[j  ] = s_uchBasis64[puchInput[i] >> 2];
            puchOutput[j + 1] = s_uchBasis64[((puchInput[i] & 0x03) << 4) | ((puchInput[i + 1] & 0xf0) >> 4)];
            puchOutput[j + 2] = s_uchBasis64[(puchInput[i + 1] & 0x0f) << 2];
            puchOutput[j + 3] = '=';
        }
        else
        {
            puchOutput[j  ] = s_uchBasis64[puchInput[i] >> 2];
            puchOutput[j + 1] = s_uchBasis64[((puchInput[i] & 0x03) << 4) | ((puchInput[i + 1] & 0xf0) >> 4)];
            puchOutput[j + 2] = s_uchBasis64[((puchInput[i + 1] & 0x0f) << 2) | ((puchInput[i + 2] & 0xc0) >> 6)];
            puchOutput[j + 3] = s_uchBasis64[puchInput[i + 2] & 0x3f];
        }

        i += 3;
        j += 4;
    }

    return j;
}



HI_S32 HI_NETWORK_Base64Decode(const HI_U8* puchInput, HI_S32 s32InputLen, HI_U8* puchOutput, HI_S32 s32OutputLen)
{
    HI_S32     i = 0;
    HI_S32     j = 0;
    HI_S32 s32Pad;
    HI_U8   uchTemp[4];
    UNUSED(s32OutputLen);
    while ((i + 3) < s32InputLen)
    {
        s32Pad  = 0;
        uchTemp[0] = char64(puchInput[i  ]);
        if (uchTemp[0] == 0xff)
        {
            s32Pad ++;
        }

        uchTemp[1] = char64(puchInput[i + 1]);
        if (uchTemp[1] == 0xff)
        {
            s32Pad ++ ;
        }

        uchTemp[2] = char64(puchInput[i + 2]);
        if (uchTemp[2] == 0xff)
        {
            s32Pad ++ ;
        }

        uchTemp[3] = char64(puchInput[i + 3]);
        if (uchTemp[3] == 0xff)
        {
            s32Pad ++ ;
        }

        if (s32Pad == 2)
        {
            puchOutput[j++] = (uchTemp[0] << 2) | ((uchTemp[1] & 0x30) >> 4);
            puchOutput[j]   = (uchTemp[1] & 0x0f) << 4;
        }
        else if (s32Pad == 1)
        {
            puchOutput[j++] = (uchTemp[0] << 2) | ((uchTemp[1] & 0x30) >> 4);
            puchOutput[j++] = ((uchTemp[1] & 0x0f) << 4) | ((uchTemp[2] & 0x3c) >> 2);
            puchOutput[j]   = (uchTemp[2] & 0x03) << 6;
        }
        else
        {
            puchOutput[j++] = (uchTemp[0] << 2) | ((uchTemp[1] & 0x30) >> 4);
            puchOutput[j++] = ((uchTemp[1] & 0x0f) << 4) | ((uchTemp[2] & 0x3c) >> 2);
            puchOutput[j++] = ((uchTemp[2] & 0x03) << 6) | (uchTemp[3] & 0x3f);
        }

        i += 4;
    }

    return j;
}


HI_S32 HI_NETWORK_CloseSocket(HI_S32* ps32Socket)
{
    if (*ps32Socket >= 0)
    {
        close(*ps32Socket);
        *ps32Socket = HI_NETWORK_INVALID_SOCKET;
        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

HI_S32 HI_NETWORK_GetPeerIPPort(HI_S32 s, HI_CHAR* ip, HI_U16* port)
{
    HI_S32 namelen = 0;
    HI_CHAR* pTemp = NULL;
    struct sockaddr_in addr;

    namelen = sizeof(struct sockaddr_in);
    if (0 != getpeername(s, (struct sockaddr*)&addr, (socklen_t*)&namelen))
    {
        return HI_FAILURE;
    }

    *port = (HI_U16)ntohs(addr.sin_port);
    pTemp = inet_ntoa(addr.sin_addr );
    if (NULL == pTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "ip address  inet_ntoa error\n");
        return  HI_FAILURE;
    }

    if (HI_FAILURE == snprintf_s(ip, IP_MAX_LEN, IP_MAX_LEN - 1, "%s", pTemp))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "string print ip fail \n");
        return  HI_FAILURE;
    }


    return HI_SUCCESS;

}

HI_S32      HI_NETWORK_GetPeerSockAddr(HI_CHAR* ip, HI_U16 port, struct sockaddr_in* pSockAddr)
{
    pSockAddr->sin_family = AF_INET;
    pSockAddr->sin_port = htons(port);
    if (HI_SUCCESS != memset_s(&(pSockAddr->sin_zero), sizeof(pSockAddr->sin_zero), '\0', sizeof(pSockAddr->sin_zero)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set inet_aton error\n");
        return HI_FAILURE;
    }
    if (0 == inet_aton(ip, (struct in_addr*) & (pSockAddr->sin_addr.s_addr)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "ip address  inet_aton error\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32      HI_NETWORK_GetHostIP(HI_S32 s, HI_CHAR* ip)
{
    HI_S32 namelen = 0;
    HI_CHAR* pTemp = NULL;
    struct sockaddr_in addr;
    namelen = sizeof(struct sockaddr);
    if (0 != getsockname(s, (struct sockaddr*)&addr, (socklen_t*)&namelen))
    {
        return HI_FAILURE;
    }

    pTemp = inet_ntoa(addr.sin_addr );
    if (NULL == pTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "ip address  inet_ntoa error\n");
        return  HI_FAILURE;
    }

    if (HI_FAILURE == snprintf_s(ip, IP_MAX_LEN, IP_MAX_LEN - 1, "%s", pTemp))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "string print ip address  error\n");
        return  HI_FAILURE;
    }
    return HI_SUCCESS;

}

HI_S32   HI_NETWORK_Udp_OpenSocket(HI_U16 port)
{
    HI_SOCKET fd;
    struct sockaddr_in addr = {0};

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_NETWORK_Udp_OpenSocket error\n");
        return HI_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (HI_SUCCESS != memset_s(&(addr.sin_zero), sizeof(addr.sin_zero), '\0', sizeof(addr.sin_zero))) /* zero the rest of the struct*/
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set sin_zero erro\n");
        close(fd);
        return HI_FAILURE;
    }
    if (bind (fd, (struct sockaddr*)&addr, sizeof (addr)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_NETWORK_Udp_OpenSocket bind erro: %s\n", strerror(errno));
        close(fd);
        return HI_FAILURE;
    }

    return fd;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
