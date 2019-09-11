/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_common.h
* @brief     livestream common utils head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_NETWORK_COMMON_H__
#define __HI_NETWORK_COMMON_H__
#ifndef __LITEOS__
#include <netinet/in.h>
#else
#include <lwip/sockets.h>
#endif
#include <stdio.h>
#include "hi_mw_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_DATE_LEN (200)
#define MIN_SEND_PORT (5000)
#define MAX_SEND_PORT (6000)
#define IP_MAX_LEN (16)
#define HI_NETWORK_INVALID_SOCKET (-1)

typedef HI_CHAR HI_IP_ADDR[64];
typedef HI_U16 HI_PORT;
typedef HI_S32  HI_SOCKET;

HI_CHAR const* HI_NETWORK_DateHeader(HI_VOID);

HI_VOID HI_NETWORK_RandomNum(HI_U32* pSsrc);

HI_VOID HI_NETWORK_RandomID(HI_CHAR* pSsrc, HI_S32 s32Len);

HI_S32 HI_NETWORK_Base64Encode(const HI_U8* puchInput, HI_S32 s32InputLen, HI_U8*  puchOutput, HI_S32 s32OutputLen);

HI_S32 HI_NETWORK_Base64Decode(const HI_U8* puchInput, HI_S32 s32InputLen, HI_U8* puchOutput, HI_S32 s32OutputLen);

HI_S32      HI_NETWORK_GetPeerIPPort(HI_S32 s, HI_CHAR* ip, HI_U16* port);

HI_S32      HI_NETWORK_GetPeerSockAddr(HI_CHAR* ip, HI_U16 port, struct sockaddr_in* pSockAddr);

HI_S32      HI_NETWORK_GetHostIP(HI_S32 s, HI_CHAR* ip);

HI_S32      HI_NETWORK_Udp_OpenSocket(HI_U16 port);

HI_S32       HI_NETWORK_CloseSocket(HI_S32* ps32Socket);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_NETWORK_COMMON_H__*/
