/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtcp_session.h
* @brief     rtspserver rtcp session head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTCP_SESSION_H__
#define __HI_RTCP_SESSION_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _Hi_RTCP_SESSION_S
{

    HI_S32 s32RTCPSendSock;

    HI_S32 s32CliRTCPPort;

    HI_S32 s32SvrRTCPPort;

    HI_U32    u32SeqNum;

    HI_U32  u32DataLen;

    HI_S32  s32PacketLen;

    HI_CHAR*      pu8PackBuff;

} HI_RTCP_SESSION_S;

HI_S32 HI_RTCP_Session_StartUdp(HI_RTCP_SESSION_S* pSess);

HI_S32 HI_RTCP_Session_StopUdp(HI_RTCP_SESSION_S* pSess);

HI_S32 HI_RTCP_Session_Create(HI_RTCP_SESSION_S** pSess, HI_S32 s32PacketLen);

HI_S32 HI_RTCP_Session_Destroy(HI_RTCP_SESSION_S* pSess);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTCP_SESSION_H__*/
