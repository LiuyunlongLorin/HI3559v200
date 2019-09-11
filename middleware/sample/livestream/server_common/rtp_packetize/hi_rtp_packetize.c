/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtp_packetize.c
* @brief     livestream common rtp packetize src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __LITEOS__
#include <arpa/inet.h>
#else
#include <lwip/inet.h>
#endif
#include <errno.h>
#include <string.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_server_state_listener.h"
#include "hi_rtp_packetize.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define  RTP_TRANS_TIMEVAL_SEC    (5)
#define  RTP_TRANS_TIMEVAL_USEC  (0)

HI_VOID HI_RTP_Packet(HI_CHAR* pPackAddr, HI_U32 u32TimeStamp, HI_U32 marker,
                      RTP_PT_E enPayload, HI_U32 u32Ssrc, HI_U16 u16LastSn)
{
    RTP_HDR_S* pRtpHdr = NULL;
    pRtpHdr = (RTP_HDR_S*)pPackAddr;
    RTP_HDR_SET_VERSION(pRtpHdr, RTP_VERSION);
    RTP_HDR_SET_P(pRtpHdr, 0);                  /*no padding*/
    RTP_HDR_SET_X(pRtpHdr, 0);                  /*no extension*/
    RTP_HDR_SET_CC(pRtpHdr, 0);                 /*0 CSRC*/
    RTP_HDR_SET_M(pRtpHdr, marker);
    RTP_HDR_SET_PT(pRtpHdr, enPayload);
    RTP_HDR_SET_SEQNO(pRtpHdr, htons(u16LastSn));
    RTP_HDR_SET_TS(pRtpHdr, htonl(u32TimeStamp));
    RTP_HDR_SET_SSRC(pRtpHdr, htonl(u32Ssrc));
    return ;
}
HI_S32 HI_RTP_SendSock(HI_S32 s32WritSock, HI_CHAR* pcBuff,
                       HI_U32 u32DataLen, struct sockaddr_in* pPeerSockAddr)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32RemSize = 0;
    HI_S32 s32Size    = 0;
    HI_CHAR*  pBufferPos = NULL;
    fd_set write_fds;
    struct timeval TimeoutVal;                      /* Timeout value */
    if(HI_SUCCESS != memset_s(&TimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval)))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set TimeoutVal fail \n");
        return HI_FAILURE;
    }
    HI_S32    s32Errno = 0;
    u32RemSize = u32DataLen;
    pBufferPos = pcBuff;

    while (u32RemSize > 0)
    {
        FD_ZERO(&write_fds);
        FD_SET(s32WritSock, &write_fds);
        TimeoutVal.tv_sec = RTP_TRANS_TIMEVAL_SEC;
        TimeoutVal.tv_usec = RTP_TRANS_TIMEVAL_USEC;
        /*judge if it can send */
        s32Ret = select(s32WritSock + 1, NULL, &write_fds, NULL, &TimeoutVal);
        /*select found over time or error happend*/
        if ( 0 == s32Ret )
        {
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "send media rtp data error: timeout,errno=%d,%s\n", errno, strerror(s32Errno));
            return HI_FAILURE;
        }
        else if ( 0 > s32Ret  )
        {
            if (EINTR == errno || EAGAIN == errno)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, " [select err: %s]\n",  strerror(errno));
                continue;
            }
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "send media rtp data ,select error: %s", strerror(s32Errno));
            return HI_FAILURE;
        }

        if (!FD_ISSET(s32WritSock, &write_fds))
        {
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "send media rtp data error: write fd not in fd set error:%s\n", strerror(s32Errno));
            return HI_FAILURE;
        }

        /*select ok and fd isset ok*/
        if (pPeerSockAddr == NULL)
        {
            s32Size = send(s32WritSock, pBufferPos, u32RemSize, MSG_DONTWAIT);
        }
        else
        {
            s32Size = sendto(s32WritSock, pBufferPos, u32RemSize, 0,
                             (struct sockaddr*)pPeerSockAddr, sizeof(struct sockaddr));
        }

        if (s32Size < 0)
        {
            /*if it is not eagain error, means can not send*/
            if (errno != EINTR && errno != EAGAIN)
            {
                s32Errno = errno;
                HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "send media data(%u Byte) error: %s\n", u32DataLen, strerror(s32Errno));
                return HI_FAILURE;
            }

            /*it is eagain error, means can try again*/
            continue;
        }

        u32RemSize -= s32Size;
        pBufferPos += s32Size;

    }

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
