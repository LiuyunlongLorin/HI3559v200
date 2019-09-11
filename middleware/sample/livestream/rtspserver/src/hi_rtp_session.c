/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtp_session.c
* @brief     rtspserver rtp session src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include "securec.h"
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
#include "hi_server_log.h"
#include "hi_rtsp_server.h"
#include "hi_network_common.h"
#include "hi_network_mbuffer.h"
#include "hi_rtp_packetize.h"
#include "hi_rtp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_VOID RTP_RTSP_ITLV_Packet(HI_CHAR* pPackAddr, HI_S32 s32Channel, HI_U32 u32PackLen,
                                    HI_U32 u32TimeStamp, HI_U32 marker,
                                    RTP_PT_E enPayload, HI_U32 u32Ssrc, HI_U16 u16LastSn)
{
    RTSP_ITLEAVED_HDR_S* pRtspItlvHdr = NULL;
    pRtspItlvHdr = (RTSP_ITLEAVED_HDR_S*)pPackAddr;
    pRtspItlvHdr->daollar    = '$';
    pRtspItlvHdr->channelid  = s32Channel;
    /*rtsp payload len represent the later packet length£¬packet len = rtp head + data len*/
    pRtspItlvHdr->payloadLen = htons((HI_U16)(u32PackLen - sizeof(RTSP_ITLEAVED_HDR_S) + sizeof(RTP_HDR_S)));
    /*packet rtp packet*/
    HI_RTP_Packet((HI_CHAR*) (&(pRtspItlvHdr->rtpHead)), u32TimeStamp, marker, enPayload, u32Ssrc, u16LastSn);

    return;
}

/*
FU-A
FU indicator
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+


FU header
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+

      Type   Packet      Type name
      ---------------------------------------------------------
      0      undefined                                    -
      1-23   NAL unit    Single NAL unit packet per H.264
      24     STAP-A     Single-time aggregation packet
      25     STAP-B     Single-time aggregation packet
      26     MTAP16    Multi-time aggregation packet
      27     MTAP24    Multi-time aggregation packet
      28     FU-A      Fragmentation unit
      29     FU-B      Fragmentation unit
      30-31  undefined                                    -
*/

static HI_S32 RTP_FU_PackageHeader(HI_CHAR* pHeader, HI_U8* pMessage, HI_U8 start, HI_U8 end)
{

    HI_U8   u8NALType;
    HI_U8   u8NRI;
    HI_S32  s32HeadLen = 0;

    if (NULL == pHeader || NULL == pMessage)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP_FU_PackageHeader erro : pHeader=%p pMessage=%p\n", pHeader, pMessage);
        fflush(stdout);
        return HI_FAILURE;
    }

    if ((1 == start && 1 == end) || start > 1 || end > 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP_FU_PackageHeader erro : start=%d end=%d\n", start, end);
        fflush(stdout);
        return HI_FAILURE;
    }

    if ((*pMessage != 0 || *(pMessage + 1) != 0 || *(pMessage + 2) != 0 ||  *(pMessage + 3) != 1)
        && (*pMessage != 0 || *(pMessage + 1) != 0 || *(pMessage + 2) != 1))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "this package is not have NALU!\n");
        fflush(stdout);
        return HI_FAILURE;
    }

    if (*pMessage == 0 && *(pMessage + 1) == 0 && *(pMessage + 2) == 0 && *(pMessage + 3) == 1)
    {
        s32HeadLen = 4;
    }
    else if (*pMessage == 0 && *(pMessage + 1) == 0 && *(pMessage + 2) == 1)
    {
        s32HeadLen = 3;
    }
    else
    {
        fflush(stdout);
        return HI_FAILURE;
    }

    u8NRI = (pMessage[s32HeadLen] & 0x60) >> 5;
    u8NALType = pMessage[s32HeadLen] & 0x1F;
    *pHeader = 0;
    *pHeader = u8NRI << 5 | 0x1c;/*NAL   is 0x1c=28£¬means FUA*/
    *(pHeader + 1) = 0;
    *(pHeader + 1) = start << 7 | end << 6 | u8NALType;

    return HI_SUCCESS;
}


static HI_S32 RTP_Session_StartTcpItlv(HI_RTP_SESSION_S* pSess)
{
    if ( HIRTP_SESSION_STATE_READY == pSess->enSessState )
    {
        pSess->enSessState = HIRTP_SESSION_STATE_PLAY;
    }
    else if ( HIRTP_SESSION_STATE_PLAY == pSess->enSessState)
    {
        /*do nothing*/
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start video error: video is in not ready state.\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 RTP_Session_StartUdp(HI_RTP_SESSION_S* pSess)
{
    HI_S32 s32TempSock = 0;

    if ( HIRTP_SESSION_STATE_READY == pSess->enSessState )
    {
        /*send rtp socket*/
        s32TempSock = HI_NETWORK_Udp_OpenSocket(pSess->s32SvrRTPPort);
        if (-1 == s32TempSock )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "error:get socket from port%d.\n",
                                 pSess->s32SvrRTPPort);
            return HI_FAILURE;
        }
        pSess->s32RTPSendSock = s32TempSock;
        HI_NETWORK_GetPeerSockAddr(pSess->aszClientIP, pSess->s32CliRTPPort, &(pSess->stCliAddr));
        pSess->enSessState = HIRTP_SESSION_STATE_PLAY;
    }
    else if ( HIRTP_SESSION_STATE_PLAY == pSess->enSessState)
    {
        /*do nothing*/
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start video error: video is in not ready state.\n");
        return HI_FAILURE;
    }


    return HI_SUCCESS;
}

HI_S32 HI_RTP_Session_UpdateSeqNum(HI_RTP_SESSION_S* pSess, HI_U32 u32SeqNum)
{

    /*check the appointed task exit or not*/
    if (NULL == pSess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "get RTP pSess NULL .\n");
        return HI_FAILURE;
    }
    pSess->u32SeqNum = u32SeqNum;

    return HI_SUCCESS;
}


HI_S32 HI_RTP_Session_GetPacketAndSendParam(HI_RTP_SESSION_S* pSess, HI_S32* pWriteSock, struct sockaddr_in* pPeerSockAddr, HI_U32* pu32LastSn,
        RTP_PACK_TYPE_E* penPackType, HI_U32* pu32Ssrc)
{
    HI_S32 s32Ret = 0;
    RTP_TRANS_MODE_E      enTransMode = RTP_TRANS_BUTT;
    enTransMode = pSess->enMediaTransMode;

    /*1 get packet type*/
    /*if stream waiting packet is video and vidieo is in playing state,
      stream can be packeted*/

    /*packet the stream according to different packet type */
    *penPackType = pSess->enPackType;
    *pu32Ssrc = pSess->u32Ssrc;

    if (RTP_TRANS_TCP_ITLV == enTransMode)
    {
        *pWriteSock = pSess->s32RTPSendSock;
        *pu32LastSn = pSess->u32SeqNum;

    }
    else if (RTP_TRANS_UDP == enTransMode) /*the branch must be udp transport*/
    {
        *pWriteSock = pSess->s32RTPSendSock;
        if (HI_SUCCESS != memcpy_s(pPeerSockAddr, sizeof(struct sockaddr_in), &(pSess->stCliAddr), sizeof(struct sockaddr_in)))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "CPY pPeerSockAddr fail\n");
            return HI_FAILURE;
        }
        *pu32LastSn = pSess->u32SeqNum;

    }
    else if (RTP_TRANS_BROADCAST == enTransMode)
    {
        *pWriteSock = pSess->s32RTPSendSock;
        if (HI_SUCCESS != memcpy_s(pPeerSockAddr, sizeof(struct sockaddr_in), &(pSess->stCliAddr), sizeof(struct sockaddr_in)))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy pPeerSockAddr fail \n");
            return HI_FAILURE;
        }

        *pu32LastSn = pSess->u32SeqNum;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "unsupport trans type in send enTransMode:%d.\n", enTransMode);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_S32 HI_RTP_Session_SendDataInRtspItlv(HI_RTP_SESSION_S* pSess, HI_U8* pDataAddr,
        HI_U32 u32DataLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
        HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr , HI_S32 s32PacketLen )
{
    HI_S32 s32Ret = 0;
    HI_U32 u32PackageLen = 0;
    HI_U32 u32RTPSendLen = 0;
    HI_U8  u8Marker = 0;
    HI_U8* pPackDataAddr = NULL ;
    HI_U32 u32Sendlen = 0;
    HI_CHAR* pPackBuf = NULL;
    HI_U32 u32SeqNum = 0;
    HI_S32  Channel = 0;
    HI_U32 u32PackPayloadLen = s32PacketLen;
    pPackBuf = pSess->pu8PackBuff; /*packbuffer  address */
    u32SeqNum = *pu32SeqNum;
    pPackDataAddr = pDataAddr ;   /*slice data's start address */

    if (RTP_PT_H264 == PackageType)
    {
        /*juege the slice data  head length*/
        HI_U32 u32SliceHeadLen = 0;

        /*H264  slice data header length*/
        if ((0x00 == (HI_U8)pDataAddr[0]) && (0x00 == (HI_U8)pDataAddr[1]))
        {
            if ((0x00 == (HI_U8)pDataAddr[2]) && (0x01 == (HI_U8)pDataAddr[3]))
            {
                u32SliceHeadLen = 5;
            }

            if (0x01 == (HI_U8)pDataAddr[2])
            {
                u32SliceHeadLen = 4;
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO,  "Unknow H264 Nalu Head:%x %x %x %x %x \n",
                                 pDataAddr[0],
                                 pDataAddr[1],
                                 pDataAddr[2],
                                 pDataAddr[3],
                                 pDataAddr[4]);
            u32SliceHeadLen = 5;
        }

        Channel = pSess->u32ItlvCliMediaChnId;

        /*when the frame data length is less than the packetsize 1500*/
        if (u32DataLen - (u32SliceHeadLen - 1) + sizeof(RTSP_ITLEAVED_HDR_S) <= u32PackPayloadLen)
        {

            u32Sendlen = u32DataLen - (u32SliceHeadLen - 1);
            u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S );

            /*payload data should cut the 4 bytes head :00 00 00 01*/
            if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ), u32Sendlen, pPackDataAddr + u32SliceHeadLen - 1, u32Sendlen))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "CPY  pPackBuf error\n");
                return HI_FAILURE;
            }


            /*form the  rtp itlv  head */
            if (0x65 == (HI_U8)pDataAddr[u32SliceHeadLen - 1] || 0x61 == (HI_U8)pDataAddr[u32SliceHeadLen - 1])
            {
                RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
            }
            else
            {
                RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);
            }

            /*send the rtp packet */
            s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);


            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                return s32Ret;
            }
        }
        /*when the frame data length is larger  than the packetsize 1500 */
        else
        {
            /*need to cut the frame into several rtp packets*/
            while (u32PackageLen < u32DataLen)
            {
                if (u32PackPayloadLen >= sizeof(RTSP_ITLEAVED_HDR_S) + 2 + u32DataLen - u32PackageLen)
                {
                    u32Sendlen = u32DataLen - u32PackageLen;
                    u8Marker = 1;
                }
                else
                {
                    u32Sendlen = u32PackPayloadLen - sizeof(RTSP_ITLEAVED_HDR_S) - 2;
                    u8Marker = 0;
                }

                u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S ) + 2;

                if (u32PackageLen == 0) /*the first package*/
                {
                    u32PackageLen += u32SliceHeadLen;
                    pPackDataAddr += u32SliceHeadLen;
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) , (HI_U8*)pDataAddr, 1, 0);

                }
                else if ((u32PackageLen + u32Sendlen) < u32DataLen) /*in the middle packages*/
                {
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ), (HI_U8*)pDataAddr, 0, 0);
                    u8Marker = 0;
                }
                else/*the last package*/
                {
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ), (HI_U8*)pDataAddr, 0, 1);
                    u8Marker = 1;
                }

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP FU Head pack error %d\n", s32Ret);
                    return HI_FAILURE;
                }

                /*copy the data */
                if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 2, u32Sendlen, pPackDataAddr, u32Sendlen))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy  pPackBuf error\n");
                    return HI_FAILURE;
                }
                /*form rtp itlv head*/
                RTP_RTSP_ITLV_Packet( pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, u8Marker, PackageType, u32Ssrc, u32SeqNum++);
                s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                    return s32Ret;
                }

                pPackDataAddr += u32Sendlen; /*update the ptr to the frame data*/
                u32PackageLen += u32Sendlen;  /*update the send data length*/
            }
        }
    }
    else if (RTP_PT_H265 == PackageType)
    {
        /*juege the slice data  head length*/
        HI_U32 u32SliceHeadLen = 0;

        /*H265  slice data header length*/
        if ((0x00 == (HI_U8)pDataAddr[0]) && (0x00 == (HI_U8)pDataAddr[1]))
        {
            if ((0x00 == (HI_U8)pDataAddr[2]) && (0x01 == (HI_U8)pDataAddr[3]))
            {
                u32SliceHeadLen = 5;
            }

            if (0x01 == (HI_U8)pDataAddr[2])
            {
                u32SliceHeadLen = 4;
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "Unknow H264 Nalu Head:%x %x %x %x %x \n",
                                 pDataAddr[0],
                                 pDataAddr[1],
                                 pDataAddr[2],
                                 pDataAddr[3],
                                 pDataAddr[4]);
            u32SliceHeadLen = 5;
        }

        int nal_type = (pDataAddr[u32SliceHeadLen - 1] >> 1) & 0x3F;


        Channel = pSess->u32ItlvCliMediaChnId;

        /*when the frame data length is less  than the packetsize 1500 */
        if (u32DataLen - (u32SliceHeadLen - 1) + sizeof(RTSP_ITLEAVED_HDR_S) <= u32PackPayloadLen)
        {

            u32Sendlen = u32DataLen - (u32SliceHeadLen - 1);
            u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S );

            /*payload data should cut the 4 bytes head :00 00 00 01*/

            if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ), u32Sendlen, pPackDataAddr + u32SliceHeadLen - 1, u32Sendlen))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy pPackBuf error\n");
                return HI_FAILURE;
            }



            if (0x26 == (HI_U8)pDataAddr[u32SliceHeadLen - 1] || 0x02 == (HI_U8)pDataAddr[u32SliceHeadLen - 1])
            {
                RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
            }
            else
            {
                RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);
            }


            s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                return s32Ret;
            }
        }
        /*when the frame data length is larger  than the packetsize 1500 */
        else
        {
            pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S)] = 49 << 1;
            pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 1] = 1;
            pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 2] = nal_type;
            /*need to cut the frame into several rtp packets*/

            while (u32PackageLen < u32DataLen)
            {

                if (u32PackPayloadLen >= sizeof(RTSP_ITLEAVED_HDR_S) + 3 + u32DataLen - u32PackageLen)
                {
                    u32Sendlen = u32DataLen - u32PackageLen;
                    u8Marker = 1;
                }
                else
                {
                    u32Sendlen = u32PackPayloadLen - sizeof(RTSP_ITLEAVED_HDR_S) - 3;
                    u8Marker = 0;
                }

                u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S ) + 3;

                if (u32PackageLen == 0) /*the first package*/
                {
                    u32PackageLen += u32SliceHeadLen + 1;
                    pPackDataAddr += u32SliceHeadLen + 1;
                    pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 2] |= 1 << 7;
                }
                else if ((u32PackageLen + u32Sendlen) < u32DataLen) /* in the middle packages*/
                {
                    u8Marker = 0;
                    pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 2] &= ~(1 << 7);
                }
                else/*the last package*/
                {
                    u8Marker = 1;
                    pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 2] &= ~(1 << 7);
                    pPackBuf[sizeof(RTSP_ITLEAVED_HDR_S) + 2] |= 1 << 6;
                }

                if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 3, u32Sendlen, pPackDataAddr, u32Sendlen))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "CPY pPackBuf error\n");
                    return HI_FAILURE;
                }

                /*form rtp itlv head*/
                RTP_RTSP_ITLV_Packet( pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, u8Marker, PackageType, u32Ssrc, u32SeqNum++);
                s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                    return s32Ret;
                }

                pPackDataAddr += u32Sendlen; /*update the ptr to the frame data*/
                u32PackageLen += u32Sendlen;  /*update the send data length*/
            }
        }
    }
    /*audio do not cut into packets*/
    else if (RTP_PT_PCMA == PackageType || RTP_PT_PCMU == PackageType || RTP_PT_G726 == PackageType)
    {
        if (u32DataLen >= (u32PackPayloadLen - sizeof( RTSP_ITLEAVED_HDR_S )))
        {
            if (pSess->pu8PackBuff )
            {
                free(pSess->pu8PackBuff );
                pSess->pu8PackBuff = NULL;
            }

            pSess->pu8PackBuff = (HI_CHAR*)malloc((u32DataLen + 128));
            if (NULL == pSess->pu8PackBuff)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc() fail!\n");
                return HI_FAILURE;
            }

            pPackBuf = pSess->pu8PackBuff;
        }

        Channel = pSess->u32ItlvCliMediaChnId;
        u32Sendlen = u32DataLen;
        u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S );
        if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ), u32Sendlen, pPackDataAddr, u32Sendlen))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy pPackBuf error\n");
            return HI_FAILURE;
        }
        /*form the rtp head*/
        RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);
        /*send the audio data*/
        s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
            return s32Ret;
        }
    }
    else if (RTP_PT_AAC == PackageType)
    {
        if ( (u32DataLen - 7) >= (u32PackPayloadLen - sizeof( RTSP_ITLEAVED_HDR_S ) - 4))
        {
            if (pSess->pu8PackBuff )
            {
                free(pSess->pu8PackBuff );
                pSess->pu8PackBuff = NULL;
            }

            pSess->pu8PackBuff = (HI_CHAR*)malloc( (u32DataLen + 128));

            if (NULL == pSess->pu8PackBuff)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc() fail!\n");
                return HI_FAILURE;
            }

            pPackBuf = pSess->pu8PackBuff;
        }

        Channel = pSess->u32ItlvCliMediaChnId;
        /*payload data should cut the first 7 bytes ADTS header */
        u32Sendlen = u32DataLen - 7 + 4;
        u32RTPSendLen = u32Sendlen + sizeof( RTSP_ITLEAVED_HDR_S );

        /*add 4 bytes au header*/
        *(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ))   = 0x00;
        *(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 1) = 0x10;
        *(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 2) = ((u32DataLen - 7) & 0x1fe0) >> 5;
        *(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 3) = ((u32DataLen - 7) & 0x1f) << 3;

        if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTSP_ITLEAVED_HDR_S ) + 4, u32DataLen - 7, pPackDataAddr + 7, u32DataLen - 7))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "CPY pPackBuf error\n");
            return HI_FAILURE;
        }

        /*add rtp head */
        RTP_RTSP_ITLV_Packet(pPackBuf, Channel, u32RTPSendLen, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
        /*send the data */
        s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
            return s32Ret;
        }
    }
    else
    {
        /*media type do not support*/
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "unsupport stream type:%d\n", PackageType);
        return HI_FAILURE;
    }

    *pu32SeqNum = u32SeqNum; /*update the seq when send several packet inside*/
    return HI_SUCCESS;
}
HI_S32 HI_RTP_Session_SendDataInRtp(HI_RTP_SESSION_S* pSess, HI_U8* pDataAddr,
                                    HI_U32 u32DataLen, HI_U32 u32TimeStamp, RTP_PT_E PackageType, HI_U32* pu32SeqNum,
                                    HI_U32 u32Ssrc, HI_S32 WriteSock, struct sockaddr_in* pPeerSockAddr, HI_S32 s32PacketLen )

{
    HI_S32 s32Ret = 0;
    HI_U32 u32PackageLen = 0;
    HI_U32 u32RTPSendLen = 0;
    HI_U8  u8Marker = 0;
    HI_U8* pPackDataAddr = NULL ;
    HI_U32 u32Sendlen = 0;
    HI_CHAR* pPackBuf = NULL;
    HI_U32 u32SeqNum = 0;
    HI_U32 u32PackPayloadLen = s32PacketLen;
    pPackBuf = pSess->pu8PackBuff;
    u32SeqNum = *pu32SeqNum;
    pPackDataAddr = pDataAddr ;

    if (RTP_PT_H264 == PackageType)
    {
        HI_U32 u32SliceHeadLen = 0;

        /*H264  head length */
        if ((0x00 == (HI_U8)pDataAddr[0]) && (0x00 == (HI_U8)pDataAddr[1]))
        {
            if ((0x00 == (HI_U8)pDataAddr[2]) && (0x01 == (HI_U8)pDataAddr[3]))
            {
                u32SliceHeadLen = 5;
            }

            if (0x01 == (HI_U8)pDataAddr[2])
            {
                u32SliceHeadLen = 4;
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "Unknow H264 Nalu Head:%x %x %x %x %x \n",
                                 pDataAddr[0],
                                 pDataAddr[1],
                                 pDataAddr[2],
                                 pDataAddr[3],
                                 pDataAddr[4]);
            u32SliceHeadLen = 5;
        }

        /*datalen less than packet length*/
        if (u32DataLen - (u32SliceHeadLen - 1) + sizeof(RTP_HDR_S) <= u32PackPayloadLen)
        {
            u32Sendlen = u32DataLen - (u32SliceHeadLen - 1);
            u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S );

            /*payload  data cut the first 4 bytres head ::00 00 00 01*/
            if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ), u32Sendlen, pPackDataAddr + u32SliceHeadLen - 1, u32Sendlen))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy packbuffer error\n");
                return HI_FAILURE;
            }


            /*add rtp head*/
            if (0x65 == (HI_U8)pDataAddr[u32SliceHeadLen - 1] || 0x61 == (HI_U8)pDataAddr[u32SliceHeadLen - 1])
            {
                HI_RTP_Packet(pPackBuf, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
            }
            else
            {
                HI_RTP_Packet(pPackBuf, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);
            }

            /*send the data */
            s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                return s32Ret;
            }
        }
        /*when the frame data is larger than packet length*/
        else
        {

            while (u32PackageLen < u32DataLen)
            {
                if (u32PackPayloadLen >= sizeof(RTP_HDR_S) + 2 + u32DataLen - u32PackageLen)
                {
                    u32Sendlen = u32DataLen - u32PackageLen;
                    u8Marker = 1;
                }
                else
                {
                    u32Sendlen =  u32PackPayloadLen - sizeof(RTP_HDR_S) - 2;
                    u8Marker = 0;
                }

                u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S ) + 2;

                if (u32PackageLen == 0) /*the first package*/
                {
                    u32PackageLen += u32SliceHeadLen;
                    pPackDataAddr += u32SliceHeadLen;
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTP_HDR_S ) , (HI_U8*)pDataAddr, 1, 0);

                }
                else if ((u32PackPayloadLen == u32RTPSendLen) && ((u32PackageLen + u32Sendlen) < u32DataLen)) // in the middle packages
                {
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTP_HDR_S ), (HI_U8*)pDataAddr, 0, 0);
                    u8Marker = 0;
                }
                else/*the last package*/
                {
                    s32Ret = RTP_FU_PackageHeader(pPackBuf + sizeof( RTP_HDR_S ), (HI_U8*)pDataAddr, 0, 1);
                    u8Marker = 1;
                }

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP FU Head pack error %d\n", s32Ret);
                    return HI_FAILURE;
                }

                /*cp  the data*/
                if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ) + 2, u32Sendlen, pPackDataAddr, u32Sendlen))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "CPY packbuf error \n");
                    return HI_FAILURE;
                }
                /*add the rtp head*/
                HI_RTP_Packet( pPackBuf, u32TimeStamp, u8Marker, PackageType, u32Ssrc, u32SeqNum++);
                s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                    return s32Ret;
                }

                pPackDataAddr += u32Sendlen;
                u32PackageLen += u32Sendlen;
            }
        }
    }
    else if (RTP_PT_H265 == PackageType)
    {
        HI_U32 u32SliceHeadLen = 0;

        /*H265  slice head length*/
        if ((0x00 == (HI_U8)pDataAddr[0]) && (0x00 == (HI_U8)pDataAddr[1]))
        {
            if ((0x00 == (HI_U8)pDataAddr[2]) && (0x01 == (HI_U8)pDataAddr[3]))
            {
                u32SliceHeadLen = 5;
            }

            if (0x01 == (HI_U8)pDataAddr[2])
            {
                u32SliceHeadLen = 4;
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_INFO, "Unknow H265 Nalu Head:%x %x %x %x %x \n",
                                 pDataAddr[0],
                                 pDataAddr[1],
                                 pDataAddr[2],
                                 pDataAddr[3],
                                 pDataAddr[4]);
            u32SliceHeadLen = 5;
        }

        HI_S32 nal_type = (pDataAddr[u32SliceHeadLen - 1] >> 1) & 0x3F;

        /*frame date less than the packet length*/
        if (u32DataLen - (u32SliceHeadLen - 1) + sizeof(RTP_HDR_S) <= u32PackPayloadLen)
        {
            u32Sendlen = u32DataLen - (u32SliceHeadLen - 1);
            u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S );

            /*payload  should cut the first 4 bytes head :00 00 00 01*/
            if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ), u32Sendlen, pPackDataAddr + u32SliceHeadLen - 1, u32Sendlen))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy packbuffer after rtp hdr error\n");
                return HI_FAILURE;
            }


            /*´òrtpÍ·*/
            if (0x26 == (HI_U8)pDataAddr[u32SliceHeadLen - 1] || 0x02 == (HI_U8)pDataAddr[u32SliceHeadLen - 1])
            {
                HI_RTP_Packet(pPackBuf, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
            }
            else
            {
                HI_RTP_Packet(pPackBuf, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);
            }

            /*send the data */
            s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                return s32Ret;
            }
        }
        /*when the frame date is larger than packet length*/
        else
        {

            pPackBuf[sizeof(RTP_HDR_S)] = 49 << 1;
            pPackBuf[sizeof(RTP_HDR_S) + 1] = 1;
            pPackBuf[sizeof(RTP_HDR_S) + 2] = nal_type;

            while (u32PackageLen < u32DataLen)
            {
                if (u32PackPayloadLen >= sizeof(RTP_HDR_S) + 3 + u32DataLen - u32PackageLen)
                {
                    u32Sendlen = u32DataLen - u32PackageLen;
                    u8Marker = 1;
                }
                else
                {
                    u32Sendlen =  u32PackPayloadLen - sizeof(RTP_HDR_S) - 3;
                    u8Marker = 0;
                }

                u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S ) + 3;

                if (u32PackageLen == 0) /*the first package*/
                {
                    u32PackageLen += u32SliceHeadLen + 1;
                    pPackDataAddr += u32SliceHeadLen + 1;
                    pPackBuf[sizeof(RTP_HDR_S) + 2] |= 1 << 7;
                }
                else if ((u32PackPayloadLen == u32RTPSendLen) && ((u32PackageLen + u32Sendlen) < u32DataLen)) // in the middle packages
                {
                    u8Marker = 0;
                    pPackBuf[sizeof(RTP_HDR_S) + 2] &= ~(1 << 7);
                }
                else/*the last package*/
                {
                    u8Marker = 1;
                    pPackBuf[sizeof(RTP_HDR_S) + 2] &= ~(1 << 7);
                    pPackBuf[sizeof(RTP_HDR_S) + 2] |= 1 << 6;
                }

                /*copy the data */
                if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ) + 3, u32Sendlen, pPackDataAddr, u32Sendlen))
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy packbuffer after rtp hdr error\n");
                    return HI_FAILURE;
                }
                /*add the rtp head */
                HI_RTP_Packet( pPackBuf, u32TimeStamp, u8Marker, PackageType, u32Ssrc, u32SeqNum++);
                s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
                    return s32Ret;
                }

                pPackDataAddr += u32Sendlen;
                u32PackageLen += u32Sendlen;
            }
        }
    }
    /*audio not cut */
    else if (RTP_PT_PCMA == PackageType || RTP_PT_PCMU == PackageType || RTP_PT_G726 == PackageType)
    {
        if (u32DataLen >= (u32PackPayloadLen - sizeof( RTP_HDR_S )))
        {
            if (pSess->pu8PackBuff )
            {
                free(pSess->pu8PackBuff );
                pSess->pu8PackBuff = NULL;
            }

            pSess->pu8PackBuff = (HI_CHAR*)malloc((u32DataLen + 128));

            if (NULL == pSess->pu8PackBuff)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc() fail!\n");
                return HI_FAILURE;
            }
            pPackBuf = pSess->pu8PackBuff;
        }

        u32Sendlen = u32DataLen;
        u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S );

        if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ), u32Sendlen, pPackDataAddr, u32Sendlen))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "set packbuf error\n");
            return HI_FAILURE;
        }

        HI_RTP_Packet(pPackBuf, u32TimeStamp, 0, PackageType, u32Ssrc, u32SeqNum++);

        s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
            return s32Ret;
        }
    }
    else if (RTP_PT_AAC == PackageType)
    {
        if ((u32DataLen - 7) >= (u32PackPayloadLen - sizeof( RTP_HDR_S ) - 4))
        {
            if (pSess->pu8PackBuff )
            {
                free(pSess->pu8PackBuff );
                pSess->pu8PackBuff = NULL;
            }

            pSess->pu8PackBuff = (HI_CHAR*)malloc( (u32DataLen + 128));

            if (NULL == pSess->pu8PackBuff)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "malloc() fail!\n");
                return HI_FAILURE;
            }
            pPackBuf = pSess->pu8PackBuff;
        }

        /*payload cut the first 7 BYTES ADTS head*/
        u32Sendlen = u32DataLen - 7 + 4;
        u32RTPSendLen = u32Sendlen + sizeof( RTP_HDR_S );

        /*add 4 bytes  au header*/
        *(pPackBuf + sizeof( RTP_HDR_S ))   = 0x00;
        *(pPackBuf + sizeof( RTP_HDR_S ) + 1) = 0x10;
        *(pPackBuf + sizeof( RTP_HDR_S ) + 2) = ((u32DataLen - 7) & 0x1fe0) >> 5;
        *(pPackBuf + sizeof( RTP_HDR_S ) + 3) = ((u32DataLen - 7) & 0x1f) << 3;

        if (HI_SUCCESS != memcpy_s(pPackBuf + sizeof( RTP_HDR_S ) + 4, u32DataLen - 7, pPackDataAddr + 7, u32DataLen - 7))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "cpy packbuffer  error\n");
            return HI_FAILURE;
        }

        /*add  rtp head */
        HI_RTP_Packet(pPackBuf, u32TimeStamp, 1, PackageType, u32Ssrc, u32SeqNum++);
        /*send the data */
        s32Ret = HI_RTP_SendSock(WriteSock, pPackBuf, u32RTPSendLen, pPeerSockAddr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "RTP video send error\n");
            return s32Ret;
        }
    }
    else
    {
        /*not support type */
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "unsupport stream type:%d\n", PackageType);
        return HI_FAILURE;
    }

    *pu32SeqNum = u32SeqNum; /*update the seq num */
    return HI_SUCCESS;
}

HI_S32 HI_RTP_Session_Start(HI_RTP_SESSION_S* pSess)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == pSess)
    {
        return HI_FAILURE;
    }

    /*if it is tcp trans type */
    if (RTP_TRANS_TCP_ITLV == pSess->enMediaTransMode)
    {
        /*do not need create the socke while tcp_itlv cause the data trans sock is the same with session sock*/
        s32Ret = RTP_Session_StartTcpItlv(pSess);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start task RTP_Session_StartTcpItlv fail %d.\n", s32Ret);
            return s32Ret;
        }
    }
    /*if it is udp trans type, record corresponding info*/
    else if (RTP_TRANS_UDP == pSess->enMediaTransMode )
    {
        s32Ret = RTP_Session_StartUdp(pSess);
        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start task RTP_Session_StartUdp fail %d.\n", s32Ret);
            return s32Ret;
        }

    }
    /*if it is tcp trans type, record corresponding info*/
    else if (RTP_TRANS_TCP == pSess->enMediaTransMode)
    {
        /*to do :add the part for tcp trans*/
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start rtpSession error: not support tcp trans now.\n");
        return HI_FAILURE;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "start rtpSession error: unrecognized trans now.\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
HI_S32 HI_RTP_Session_Stop(HI_RTP_SESSION_S* pSess)
{

    if (NULL == pSess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "Stop rtpSession error.\n");
        return HI_FAILURE;
    }

    /*set all meida state as init*/
    pSess->enSessState = HIRTP_SESSION_STATE_STOP;

    /*if it is tcp trans type*/
    if (RTP_TRANS_TCP_ITLV == pSess->enMediaTransMode)
    {
        /*do nothing*/
    }
    /*if it is udp trans type, record corresponding info*/
    else if (RTP_TRANS_UDP == pSess->enMediaTransMode )
    {
        HI_NETWORK_CloseSocket(&pSess->s32RTPSendSock);
    }
    /*if it is broadcast trans type, record corresponding info*/
    else if (RTP_TRANS_BROADCAST == pSess->enMediaTransMode)
    {
        HI_NETWORK_CloseSocket(&pSess->s32RTPSendSock);
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "top trans task error: not unrecognized trans type %d.\n",
                             pSess->enMediaTransMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;


}

HI_S32 HI_RTP_Session_Create(HI_RTP_SESSION_S** pSess, HI_S32 s32PacketLen)
{
    HI_RTP_SESSION_S* pRTPSession = NULL;

    if (s32PacketLen <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRTPSession packlen less than 0\n");
        return HI_FAILURE;
    }

    pRTPSession = (HI_RTP_SESSION_S*)malloc(sizeof(HI_RTP_SESSION_S));

    if (!pRTPSession)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRtpSession malloc failed\n");
        return HI_FAILURE;
    }
    if (HI_SUCCESS != memset_s(pRTPSession, sizeof(HI_RTP_SESSION_S), 0x00, sizeof(HI_RTP_SESSION_S)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pRTPSession set failed\n");
        free(pRTPSession);
        pRTPSession = NULL;
        return HI_FAILURE;
    }

    pRTPSession->pu8PackBuff = (HI_CHAR*)malloc(s32PacketLen);

    if ( !pRTPSession->pu8PackBuff)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pu8PackBuff malloc failed\n");
        free(pRTPSession);
        pRTPSession = NULL;
        return HI_FAILURE;
    }
    if (HI_SUCCESS != memset_s(pRTPSession->pu8PackBuff, s32PacketLen, 0x00, s32PacketLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_RTSPSERVER, HI_SERVER_LOG_LEVEL_ERR, "pu8PackBuff set failed\n");
        free(pRTPSession->pu8PackBuff);
        pRTPSession->pu8PackBuff = NULL;
        free(pRTPSession);
        pRTPSession = NULL;
        return HI_FAILURE;
    }

    *pSess = pRTPSession;
    return HI_SUCCESS;
}

HI_S32 HI_RTP_Session_Destroy(HI_RTP_SESSION_S* pSess)
{
    if (NULL ==  pSess )
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
