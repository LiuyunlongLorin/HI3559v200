/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtp_session.h
* @brief     rtspserver rtp session head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTP_SESSION_H__
#define __HI_RTP_SESSION_H__

#ifndef __LITEOS__
#include <netinet/in.h>
#else
#include <lwip/sockets.h>
#endif
#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*rtsp interleaved packet*/
typedef struct hiRTSP_ITLEAVED_HDR_S
{
    HI_U8  daollar;      /*8, $:dollar sign(24 decimal)*/
    HI_U8  channelid;    /*8, channel id*/
    HI_U16 payloadLen;   /*16, payload length*/

    RTP_HDR_S rtpHead;   /*rtp head*/

} RTSP_ITLEAVED_HDR_S;
/*hisi interleaved packet*/

typedef enum _Hi_RTP_SESSION_STATE_E
{
    HIRTP_SESSION_STATE_INIT    = 0,
    HIRTP_SESSION_STATE_READY   = 1,
    HIRTP_SESSION_STATE_PLAY = 2,
    HIRTP_SESSION_STATE_STOP = 3,
    HIRTP_SESSION_STATE_BUTT
} HI_RTP_SESSION_STATE_E;



typedef struct _Hi_RTP_SESSION_S
{
    RTP_PACK_TYPE_E enPackType;
    RTP_TRANS_MODE_E enMediaTransMode;
    HI_S32   s32RTPSendSock;
    HI_S32    s32CliRTPPort;
    HI_S32    s32SvrRTPPort;
    HI_U32     u32SeqNum;
    HI_U32      u32Ssrc;
    HI_U32        u32DataLen;
    HI_CHAR*      pu8PackBuff;
    HI_U32 u32ItlvCliMediaChnId;
    HI_U32 u32ItlvCliAdaptChnId;
    HI_RTP_SESSION_STATE_E enSessState;
    HI_CHAR aszClientIP[RTP_TRANS_IP_MAX_LEN];
    struct sockaddr_in stCliAddr;

} HI_RTP_SESSION_S;


HI_S32 HI_RTP_Session_UpdateSeqNum(HI_RTP_SESSION_S* pSess, HI_U32 u32SeqNum);

HI_S32 HI_RTP_Session_GetPacketAndSendParam(HI_RTP_SESSION_S* pSess, HI_S32* pWriteSock, struct sockaddr_in* pPeerSockAddr, HI_U32* pu32LastSn,
        RTP_PACK_TYPE_E* penPackType, HI_U32* pu32Ssrc);


HI_S32 HI_RTP_Session_SendDataInRtspItlv(HI_RTP_SESSION_S* pSess, HI_U8* pDataAddr,
        HI_U32 u32DataLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
        HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr  , HI_S32 s32PacketLen);

HI_S32 HI_RTP_Session_SendDataInRtp(HI_RTP_SESSION_S* pSess, HI_U8* pDataAddr,
                                    HI_U32 u32DataLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
                                    HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr , HI_S32 s32PacketLen );

HI_S32 HI_RTP_Session_Start(HI_RTP_SESSION_S* pSess);

HI_S32 HI_RTP_Session_Stop(HI_RTP_SESSION_S* pSess);

HI_S32 HI_RTP_Session_Create(HI_RTP_SESSION_S** pSess, HI_S32 s32PacketLen);

HI_S32 HI_RTP_Session_Destroy(HI_RTP_SESSION_S* pSess);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTP_SESSION_H__*/
