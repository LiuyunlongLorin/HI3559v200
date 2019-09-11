/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_rtp_packetize.h
* @brief     livestream common rtp packetize head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_RTP_PACKTIZE_H__
#define __HI_RTP_PACKTIZE_H__
#ifndef __LITEOS__
#include <netinet/in.h>
#else
#include <lwip/sockets.h>
#endif
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define RTP_VERSION    2

#define RTP_TRANS_STREAM_VIDEO (0)
#define RTP_TRANS_STREAM_AUDIO (1)
#define RTP_TRANS_STREAM_DATA  (2)
#define RTP_TRANS_STREAM_MAX   (3)
#define RTP_TRANS_IP_MAX_LEN (32)

/*RTP Payload type define rfc3551*/
typedef enum hiRTP_PT_E
{
    RTP_PT_PCMU             = 0,        /* mu-law G.711Mu*/
    RTP_PT_GSM              = 3,        /* GSM */
    RTP_PT_G723             = 4,        /* G.723 */
    RTP_PT_PCMA             = 8,        /* a-law G.711A*/
    RTP_PT_G722             = 9,        /* G.722 */
    RTP_PT_L16_STEREO       = 10,       /* linear 16, 44.1khz, 2 channel */
    RTP_PT_L16_MONO         = 11,       /* linear 16, 44.1khz, 1 channel */
    RTP_PT_MPA              = 14,       /* mpeg audio */
    RTP_PT_JPEG             = 26,       /* jpeg */
    RTP_PT_H261             = 31,       /* h.261 */
    RTP_PT_MPV              = 32,       /* mpeg video */
    RTP_PT_MP2T             = 33,       /* mpeg2 TS stream */
    RTP_PT_H263             = 34,       /* old H263 encapsulation */

    /*96-127 dynamic*/
    RTP_PT_H264             = 96,       /* hisilicon define as h.264 */
    RTP_PT_G726             = 97,       /* hisilicon define as G.726 */
    RTP_PT_H265             = 98,       /* hisilicon define as h.265 */
    RTP_PT_DATA             = 100,      /* hisilicon define as md alarm data*/
    RTP_PT_AMR              = 101,      /* hisilicon define as AMR*/
    RTP_PT_MJPEG            = 102,      /* hisilicon define as MJPEG */
    RTP_PT_YUV              = 103,      /* hisilicon define as YUV*/
    RTP_PT_ADPCM            = 104,      /* hisilicon define as ADPCM */
    RTP_PT_AAC              = 105,      /* hisilicon define as AAC */
    RTP_PT_SEC               = 106,  /* hisilicon define as SEC*/
    RTP_PT_RED              = 107,  /* hisilicon define as RED */
    RTP_PT_INVALID          = 128
} RTP_PT_E;

typedef enum hiRTP_PACK_TYPE_E
{
    PACK_TYPE_RAW = 0, /*海思媒体包头类型MEDIA_HDR_S，头长8个字节，用于TCP传输*/
    PACK_TYPE_RTP,     /*普通RTP打包方式，头是12个字节*/
    PACK_TYPE_RTP_STAP,/* STAP-A打包方式，加了3个字节的净荷头，头是15个字节*/
    PACK_TYPE_RTP_FUA, /*FU-A打包方式,一个slice分为若干rtp包发送，数据前加了2type头*/
    PACK_TYPE_RTSP_ITLV, /*rtsp interleaved 打包方式,在rtp头上前加了4个净荷头*/
    PACK_TYPE_HISI_ITLV, /*hisi interleaved 打包方式,在rtp头上前加了8个净荷头*/
    PACK_TYPE_RTSP_O_HTTP,
    PACK_TYPE_BUTT
} RTP_PACK_TYPE_E;

typedef enum hiRTP_TRANS_MODE_E
{
    RTP_TRANS_UDP = 0,
    RTP_TRANS_UDP_ITLV,
    RTP_TRANS_TCP,
    RTP_TRANS_TCP_ITLV,
    RTP_TRANS_BROADCAST,
    RTP_TRANS_BUTT
} RTP_TRANS_MODE_E;

/* op-codes */
#define RTP_OP_PACKETFLAGS  (1)               /* opcode datalength = 1 */
#define RTP_OP_CODE_DATA_LENGTH     (1)

/* flags for opcode RTP_OP_PACKETFLAGS */
#define RTP_FLAG_LASTPACKET 0x00000001      /* last packet in stream */
#define RTP_FLAG_KEYFRAME   0x00000002      /* keyframe packet */

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* total 12Bytes */
typedef struct hiRTP_HDR_S
{

#if (BYTE_ORDER == LITTLE_ENDIAN)
    /* byte 0 */
    HI_U16 cc      : 4;  /* CSRC count */
    HI_U16 x       : 1;  /* header extension flag */
    HI_U16 p       : 1;  /* padding flag */
    HI_U16 version : 2;  /* protocol version */

    /* byte 1 */
    HI_U16 pt      : 7;  /* payload type */
    HI_U16 marker  : 1;  /* marker bit */
#elif (BYTE_ORDER == BIG_ENDIAN)
    /* byte 0 */
    HI_U16 version : 2;  /* protocol version */
    HI_U16 p       : 1;  /* padding flag */
    HI_U16 x       : 1;  /* header extension flag */
    HI_U16 cc      : 4;  /* CSRC count */
    /*byte 1*/
    HI_U16 marker  : 1;  /* marker bit */
    HI_U16 pt      : 7;  /* payload type */
#else
#error YOU MUST DEFINE BYTE_ORDER == LITTLE_ENDIAN OR BIG_ENDIAN !
#endif

    /* bytes 2, 3 */
    HI_U16 seqno  : 16;  /* sequence number */

    /* bytes 4-7 */
    HI_U32 ts;            /* timestamp in ms */

    /* bytes 8-11 */
    HI_U32 ssrc;          /* synchronization source */
} RTP_HDR_S;

/*rtp field packet*/
#define RTP_HDR_SET_VERSION(pHDR, val)  ((pHDR)->version = val)
#define RTP_HDR_SET_P(pHDR, val)        ((pHDR)->p       = val)
#define RTP_HDR_SET_X(pHDR, val)        ((pHDR)->x       = val)
#define RTP_HDR_SET_CC(pHDR, val)       ((pHDR)->cc      = val)

#define RTP_HDR_SET_M(pHDR, val)        ((pHDR)->marker  = val)
#define RTP_HDR_SET_PT(pHDR, val)       ((pHDR)->pt      = val)

#define RTP_HDR_SET_SEQNO(pHDR, _sn)    ((pHDR)->seqno  = (_sn))
#define RTP_HDR_SET_TS(pHDR, _ts)       ((pHDR)->ts     = (_ts))

#define RTP_HDR_SET_SSRC(pHDR, _ssrc)    ((pHDR)->ssrc  = _ssrc)

#define RTP_HDR_LEN  sizeof(RTP_HDR_S)

#define RTP_DEFAULT_SSRC 41030

#if (BYTE_ORDER == LITTLE_ENDIAN)
typedef struct Frag_unit_indicator
{
    HI_U8 Forbit    : 1;
    HI_U8 NRI       : 2;
    HI_U8 Type      : 5;
} HI_S_FU_INDICATOR;

typedef struct Farg_Unit_Header
{
    HI_U8 Start     : 1;
    HI_U8 End       : 1;
    HI_U8 Reserve   : 1;
    HI_U8 Type      : 5;
} HI_S_FU_HEADER;
#else
typedef struct Frag_unit_indicator
{
    HI_U8 Type          : 5;
    HI_U8   NRI         : 2;
    HI_U8   Forbit      : 1;
} HI_S_FU_INDICATOR;

typedef struct Farg_Unit_Header
{
    HI_U8   Type            : 5;
    HI_U8   Reserve      : 1;
    HI_U8   End              : 1;
    HI_U8   Start            : 1;
} HI_S_FU_HEADER;

#endif

typedef enum
{
    FU_A = 28,
    FU_B = 29,
} HI_FU_SORT;

HI_VOID HI_RTP_Packet(HI_CHAR* pPackAddr, HI_U32 u32TimeStamp, HI_U32 marker, RTP_PT_E enPayload, HI_U32 u32Ssrc, HI_U16 u16LastSn);

HI_S32 HI_RTP_SendSock(HI_S32 s32WritSock, HI_CHAR* pcBuff, HI_U32 u32DataLen, struct sockaddr_in* pPeerSockAddr);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_RTP_PACKTIZE_H__*/
