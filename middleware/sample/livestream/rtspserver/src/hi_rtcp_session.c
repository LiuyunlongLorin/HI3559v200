/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtcp_session.c
* @brief     rtspserver rtcp session src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_rtsp_server.h"
#include "hi_network_common.h"
#include "hi_rtcp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HI_RTCP_Session_StartUdp(HI_RTCP_SESSION_S* pSess)
{
    HI_S32 s32TempSock = 0;

    /*send rtcp socket*/
    s32TempSock = HI_NETWORK_Udp_OpenSocket(pSess->s32SvrRTCPPort);

    if (HI_NETWORK_INVALID_SOCKET == s32TempSock )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "error:get socket from port%d.\n",
                      pSess->s32SvrRTCPPort);
        return HI_FAILURE;
    }

    pSess->s32RTCPSendSock = s32TempSock;

    return HI_SUCCESS;
}
HI_S32 HI_RTCP_Session_StopUdp(HI_RTCP_SESSION_S* pSess)
{
    if (NULL == pSess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Stop rtcpSession error null param.\n");
        return HI_FAILURE;
    }

    HI_NETWORK_CloseSocket(&pSess->s32RTCPSendSock);

    return HI_SUCCESS;
}

HI_S32 HI_RTCP_Session_Create(HI_RTCP_SESSION_S** pSess, HI_S32 s32PacketLen)
{
    HI_RTCP_SESSION_S* pRtcpSession = NULL;

    if (s32PacketLen <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRtcpSession packlen less than 0\n");
        return HI_FAILURE;
    }

    pRtcpSession = (HI_RTCP_SESSION_S*)malloc(sizeof(HI_RTCP_SESSION_S));
    if (!pRtcpSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRtcpSession malloc failed\n");
        return HI_FAILURE;
    }
    if(HI_SUCCESS != memset_s(pRtcpSession,sizeof(HI_RTCP_SESSION_S),0x00,sizeof(HI_RTCP_SESSION_S)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRtcpSession set failed\n");
        free(pRtcpSession);
        pRtcpSession = NULL;
        return HI_FAILURE;
    }
    pRtcpSession->pu8PackBuff = (HI_CHAR*)malloc(s32PacketLen);

    if ( !pRtcpSession->pu8PackBuff)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pu8PackBuff malloc failed\n");
        free(pRtcpSession);
        pRtcpSession = NULL;
        return HI_FAILURE;
    }
    if(HI_SUCCESS != memset_s(pRtcpSession->pu8PackBuff,s32PacketLen,0x00,s32PacketLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRtcpSession set failed\n");
        free(pRtcpSession->pu8PackBuff);
        pRtcpSession->pu8PackBuff = NULL;
        free(pRtcpSession);
        pRtcpSession = NULL;
        return HI_FAILURE;
    }
    *pSess = pRtcpSession;
    return HI_SUCCESS;
}
HI_S32 HI_RTCP_Session_Destroy(HI_RTCP_SESSION_S* pSess)
{
    if ( NULL ==  pSess )
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "param pSess  is null !\n");
        return HI_FAILURE;
    }

    if (pSess->pu8PackBuff)
    {
        free(pSess->pu8PackBuff);
        pSess->pu8PackBuff = NULL;
    }

    free(pSess);
    pSess = NULL;


    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
