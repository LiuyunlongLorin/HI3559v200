/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ffmpeg_format.c
* @brief     hidemuxer module header file
* @author    HiMobileCam middleware develop team
* @date      2016.11.03  */


#include <sys/time.h>
#include <pthread.h>
#ifndef __HuaweiLite__
#include <sys/syscall.h>
#endif
#include "securec.h"
#include <string.h>
#include "hi_demuxer.h"
#include "hi_demuxer_err.h"
#include "ffdemux_log.h"
#include "mp4_format.h"

#include "ffdemux_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define FFMPEG_AAC_ADTS_LEN  (7)
#define FFMPEG_SAMPLE_RATE_NUM  (13)
#define MAX_TRY_READ_TIMES (20)


typedef enum hiFFMPEG_AAC_PROFILE_E
{
    AAC_PROFILE_MAIN = 0,
    AAC_PROFILE_LC,
    AAC_PROFILE_SSR,
    AAC_PROFILE_RESERVED,
} HI_FFMPEG_AAC_PROFILE_E;

#define FFMPEG_H264_NAL_MASK (0x1f)
#define FFMPEG_H265_NAL_MASK (0x7f)


typedef enum hiFFMPEG_AVC_NAL
{
    FFMPEG_AVC_SEI = 0x06,
    FFMPEG_AVC_IDR = 0x05,
    FFMPEG_AVC_SPS = 0x07,
    FFMPEG_AVC_PPS = 0x08,
} HI_FFMPEG_AVC_NAL;


typedef enum hiFFMPEG_HEVC_NAL
{
    FFMPEG_HEVC_PSLICE = 0x01,
    FFMPEG_HEVC_IDR = 0x13,
    FFMPEG_HEVC_SPS = 0x21,
    FFMPEG_HEVC_PPS = 0x22,
    FFMPEG_HEVC_VPS = 0x20,
    FFMPEG_HEVC_SEI = 0x27
} HI_FFMPEG_HEVC_NAL;


typedef struct hiFFMPEG_ADTS_HEADER
{
    /* fixed */
    HI_U32  u32Sync;                           /* syncword */
    HI_U8   u8ID;                             /* MPEG bit - should be 1 */
    HI_U8   u8Layer;                          /* MPEG u8Layer - should be 0 */
    HI_U8   u8ProtectBit;                     /* 0 = CRC word follows, 1 = no CRC word */
    HI_U8   u8Profile;                        /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
    HI_U8   u8SampRateIdx;                    /* sample rate index range = [0, 11] */
    HI_U8   u8PrivateBit;                     /* ignore */
    HI_U8   u8ChannelConfig;                  /* 0 = implicit, >0 = use default table */
    HI_U8   u8OrigCopy;                       /* 0 = copy, 1 = original */
    HI_U8   u8Home;                           /* ignore */

    /* variable */
    HI_U8   u8CopyBit;                        /* 1 bit of the 72-bit copyright ID (transmitted as 1 bit per frame) */
    HI_U8   u8CopyStart;                      /* 1 = this bit starts the 72-bit ID, 0 = it does not */
    HI_U32 u32FrameLength;                    /* length of frame */
    HI_U32 u32BufferFull;                     /* number of 32-bit words left in enc buffer, 0x7FF = VBR */
    HI_U8   u8NumRawDataBlocks;               /* number of raw data blocks in frame */

    /* CRC */
    HI_S32   s32CrcCheckWord;                   /* 16-bit CRC check word (present if u8ProtectBit == 0) */
} HI_FFMPEG_ADTS_HEADER_S;

static HI_U32 s_au32SampRateTab[FFMPEG_SAMPLE_RATE_NUM] =
{
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};


static HI_VOID MP4_GET_ADTSHeader(const HI_S32 sampleRateidx,             /*!< aacPlus sampling frequency (incl. SBR) */
                                  HI_U32     u32FrameLength,            /*!< raw data length */
                                  const HI_S32     s32Profile,                /* 0 = main, 1 = LC, 2 = SSR, 3 = reserved */
                                  const HI_S32     s32Ch,
                                  HI_U8*      pADTSHeadBuf)
{
    HI_U32  u32Head0 = 0, u32Head1 = 0;
    HI_FFMPEG_ADTS_HEADER_S stADTSHeader = {0};

    u32FrameLength += FFMPEG_AAC_ADTS_LEN;

    /* fixed fields - should not change from frame to frame */

    stADTSHeader.u32Sync =             0x0fff;          /* 12bit: */
    stADTSHeader.u8ID =               0;               /* 1bit: MPEG bit - should be 1 */
    stADTSHeader.u8Layer =            0;               /* 2bit: MPEG u8Layer - should be 0 */
    stADTSHeader.u8ProtectBit =       1;               /* 1bit: */
    stADTSHeader.u8Profile =          s32Profile;         /* 2bit: */
    stADTSHeader.u8SampRateIdx =      sampleRateidx;     /* 4bit: */
    stADTSHeader.u8PrivateBit =       0;               /* 1bit: */
    stADTSHeader.u8ChannelConfig =    s32Ch;              /* 3bit: */
    stADTSHeader.u8OrigCopy =         0;               /* 1bit: */
    stADTSHeader.u8Home =             0;               /* 1bit: */

    /* variable fields - can change from frame to frame */

    stADTSHeader.u8CopyBit =          0;              /* 1bit: */
    stADTSHeader.u8CopyStart =        0;              /* 1bit: */
    stADTSHeader.u32FrameLength =      u32FrameLength;    /* 13bit: */
    stADTSHeader.u32BufferFull =       0x07ff;         /* 11bit: */
    stADTSHeader.u8NumRawDataBlocks = 0;              /* 2bit: */

    u32Head0  = stADTSHeader.u32Sync       << (32 - 12);
    u32Head0 |= stADTSHeader.u8ID         << (32 - 12 - 1);
    u32Head0 |= stADTSHeader.u8Layer      << (32 - 12 - 1 - 2);
    u32Head0 |= stADTSHeader.u8ProtectBit << (32 - 12 - 1 - 2 - 1);

    u32Head0 |= stADTSHeader.u8Profile       << (32 - 12 - 1 - 2 - 1 - 2);
    u32Head0 |= stADTSHeader.u8SampRateIdx   << (32 - 12 - 1 - 2 - 1 - 2 - 4);
    u32Head0 |= stADTSHeader.u8PrivateBit    << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1);
    u32Head0 |= stADTSHeader.u8ChannelConfig << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3);
    u32Head0 |= stADTSHeader.u8OrigCopy      << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1);
    u32Head0 |= stADTSHeader.u8Home          << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1);
    u32Head0 |= stADTSHeader.u8CopyBit       << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1 - 1);
    u32Head0 |= stADTSHeader.u8CopyStart     << (32 - 12 - 1 - 2 - 1 - 2 - 4 - 1 - 3 - 1 - 1 - 1 - 1);
    u32Head0 |= ((stADTSHeader.u32FrameLength >> (13 - 2)) & 0x3);

    u32Head1  = stADTSHeader.u32FrameLength      << (32 - 13 + 2);
    u32Head1 |= stADTSHeader.u32BufferFull       << (32 - 13 + 2 - 11);
    u32Head1 |= stADTSHeader.u8NumRawDataBlocks << (32 - 13 + 2 - 11 - 2);

    pADTSHeadBuf[0] = (unsigned char)(u32Head0 >> 24) & 0xff;
    pADTSHeadBuf[1] = (unsigned char)(u32Head0 >> 16) & 0xff;
    pADTSHeadBuf[2] = (unsigned char)(u32Head0 >> 8) & 0xff;
    pADTSHeadBuf[3] = (unsigned char)(u32Head0 >> 0) & 0xff;

    pADTSHeadBuf[4] = (unsigned char)(u32Head1 >> 24) & 0xff;
    pADTSHeadBuf[5] = (unsigned char)(u32Head1 >> 16) & 0xff;
    pADTSHeadBuf[6] = (unsigned char)(u32Head1 >> 8) & 0xff;

    return;
}

static HI_BOOL MP4_JUDGE_HasAdtsHeader(HI_U8* psrc, HI_U32 u32srcsize, HI_U8* pdst, HI_U32 u32dstsize)
{
    if (NULL == psrc
        || NULL == pdst
        || u32srcsize < FFMPEG_AAC_ADTS_LEN
        || u32dstsize < FFMPEG_AAC_ADTS_LEN)
    {
        return HI_FALSE;
    }

    /* adts fixed header
     * syncword: 12bits, fixed,      0xFFF
     * ID:       1bit,   non-fixed,  MPEG version, 0 for MPEG-4, 1 for MPEG-2.
     * ...
     * Compare 28bits is enough, and we must compare without 'ID', because it is non-fixed */

    if (psrc[0] != pdst[0]
        || (psrc[1] & 0xF7) != (pdst[1] & 0xF7)
        || psrc[2] != pdst[2]
        || (psrc[3] & 0xF0) != (pdst[3] & 0xF0))
    {
        return HI_FALSE;
    }

    return HI_TRUE;
}



static HI_S32 MP4_AVBSF_Filter(AVBSFContext* pstBsfCtx,
                               AVPacket* pSrcPkt, AVPacket* pDstPkt)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = av_bsf_send_packet(pstBsfCtx, pSrcPkt);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                       "av_bsf_send_packet failed ret %d\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = av_bsf_receive_packet(pstBsfCtx, pDstPkt);
    if (s32Ret == AVERROR(EAGAIN) || s32Ret == AVERROR_EOF)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                       "need more data input or no data output  ret: %d\n", s32Ret);
        return HI_RET_FILE_EOF;
    }
    else if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,
                       "av_bsf_receive_packet err  ret %x\n", s32Ret);
        return HI_FAILURE;
    }

    AVPacket stAvpkt = { 0 };
    av_init_packet(&stAvpkt);
    /* drain all the remaining packets we cannot return */
    while (s32Ret >= 0)
    {
        s32Ret = av_bsf_receive_packet(pstBsfCtx, &stAvpkt);
        if (s32Ret >= 0)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                           "av_bsf_receive_packet extra packet len: %d are not handled\n", stAvpkt.size);
        }
        av_packet_unref(&stAvpkt);
    }

    //todo: here we only consider about h264,h265, so extradata update do not
    //handle here
    return HI_SUCCESS;
}



static HI_S32  MP4_FILTER_Packet(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    AVPacket  stOutAVPacket = {0};
    HI_S32 s32Ret = 0;

    av_init_packet(&stOutAVPacket);

    s32Ret = MP4_AVBSF_Filter(pstFormatMember->pstH26XBsfCtx, &pstFormatMember->stPkt, &stOutAVPacket);
    if (s32Ret != HI_SUCCESS)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "MP4_AVBSF_Filter err s32Ret: %x\n", s32Ret);
        av_packet_unref(&pstFormatMember->stPkt);
        /*filter err may come from that repaired file stream-end have invalid frames,
        so here just return EOF, ignore all frames at stream-end*/
        return s32Ret;
    }

    if (NULL == stOutAVPacket.buf)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "av_bitstream_filter_filter, receive packet buffer is NULL \n");
        av_packet_unref(&pstFormatMember->stPkt);
        return HI_FAILURE;
    }
    av_packet_move_ref(&pstFormatMember->stPkt, &stOutAVPacket);

    return HI_SUCCESS;
}

static HI_U32 MP4_GetOneNalUnit(HI_U8* pNaluType, HI_U8* pBuffer , HI_U32 size)
{
    HI_U32 code = 0;
    HI_U32 tmp = 0;
    HI_U32 pos = 0;

    for (code = 0xffffffff, pos = 0; pos < 4; pos++)
    {
        tmp = pBuffer[pos];
        code = (code << 8) | tmp;
    }

    if (code != 0x00000001)
    {
        return 0;
    }

    *pNaluType = pBuffer[pos++] & 0xFF;

    for (code = 0xffffffff; pos < size; pos++)
    {
        tmp = pBuffer[pos];

        if ((code = (code << 8) | tmp) == 0x00000001)
        {
            break;                //next start code is found
        }
    }

    if (pos == size )              // next start code is not found, this must be the last nalu
    {
        return size;
    }
    else
    {
        return pos - 4 + 1;
    }
}
static HI_S32  MP4_GetMetaResolution(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_U8* pBuffer , HI_U32 size)
{
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;

    if (AV_CODEC_ID_H264 == pstFormatMember->pstMetaStream->codecpar->codec_id )
    {
        AVC_SPS_INFO stSpsInfo = {0};
        FFAVCParser_parseSps(&stSpsInfo, pBuffer, size);
        if (0 == stSpsInfo.u32PicWidthInMbsMinus1)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_GetMetaResolution spsInfo.u32PicWidthInMbsMinus1v:%d error \n", stSpsInfo.u32PicWidthInMbsMinus1);
            return HI_FAILURE;
        }

        u32Width = ((stSpsInfo.u32PicWidthInMbsMinus1 + 1) * 16) - stSpsInfo.u32FrameCropLeftOffset * 2 - stSpsInfo.u32FrameCropRightOffset * 2;
        u32Height = ((2 - (stSpsInfo.bFrameMbsOnlyFlag ? 1 : 0)) * (stSpsInfo.u32PicHeightInMapUnitsMinus1 + 1) * 16) - (stSpsInfo.u32FrameCropTopOffset * 2) - (stSpsInfo.u32FrameCropBottomOffset * 2);
    }
    else if (AV_CODEC_ID_HEVC == pstFormatMember->pstMetaStream->codecpar->codec_id )
    {
        HEVC_SPS_INFO stSpsInfo = {0};
        FFHEVCParser_parseSps(&stSpsInfo, pBuffer, size);
        if (0 == stSpsInfo.u32PicWidthInLumaSamples || 0 == stSpsInfo.u32PicHeightInLumaSamples)
        {
            return HI_FAILURE;
        }
        u32Width = stSpsInfo.u32PicWidthInLumaSamples;
        u32Height = stSpsInfo.u32PicHeightInLumaSamples;
    }
    FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "MP4_GetMetaResolution width %d height %d \n", u32Width, u32Height);

    pstFormatMember->pstMetaStream->codecpar->width = u32Width;
    pstFormatMember->pstMetaStream->codecpar->height = u32Height;
    return HI_SUCCESS;
}

static HI_S32 MP4_GET_Frame(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* read frames from the file */
    s32Ret = av_read_frame(pstFormatMember->pstFormatContext, &pstFormatMember->stPkt);

    if (AVERROR_EOF == s32Ret)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "read endof s32Ret: %d \n", s32Ret);
        return HI_RET_FILE_EOF;
    }
    else if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "av_read_frame failed, ret:%d\n", s32Ret);
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    if (pstFormatMember->stPkt.size <= 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "HiffmpegDemux err: read stream len 0\n");
        return HI_ERR_DEMUXER_READ_PACKET;
    }

    return HI_SUCCESS;
}

static HI_S32 MP4_PARSE_SeekStreamStart(AVFormatContext * context, HI_S32 streamIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S64 s64SeekMin = 0;
    HI_S64 s64SeekMax = 0;
    HI_S32 s32SeekFlag = 0;

    s64SeekMin = INT64_MIN;
    s64SeekMax = 0;

    s32Ret = avformat_seek_file(context, streamIndex, s64SeekMin, 0, s64SeekMax, s32SeekFlag);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_DEBUG, "%s: error while seeking 0  metastream  s32Ret:%d\n",
                       context->filename,  s32Ret);
        s32Ret =  HI_ERR_DEMUXER_SEEK;
    }
    return s32Ret;
}

static HI_S32  MP4_PARSE_MetaIFrame(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{
    HI_BOOL bGetPacket = HI_FALSE;
    HI_U32 u32NalLen = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8Buffer = NULL;
    HI_U32 u32DataLen = 0;
    HI_U8 u8NaluType = 0;
    HI_U32 u32ReadTimes = 0;
    HI_S32 s32FirstStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;

    while (bGetPacket != HI_TRUE)
    {
        s32Ret = MP4_GET_Frame(pstFormatMember);
        if (s32Ret != HI_SUCCESS)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_PARSE_MetaIFrame read frame failed s32Ret=0x%x\n",s32Ret);
            goto PARSE_MEATAI_2STREAMSTART_END;
        }
        if (s32FirstStreamIndex == HI_DEMUXER_NO_MEDIA_STREAM)
        {
            s32FirstStreamIndex = pstFormatMember->stPkt.stream_index;
        }
        u32ReadTimes++;

        if (u32ReadTimes > MAX_TRY_READ_TIMES)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "read frame for meta timeout \n");
            goto PARSE_MEATAI_UNREF_END;
        }

        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "stream_index=%d \n", pstFormatMember->stPkt.stream_index);

        if (pstFormatMember->stPkt.stream_index == pstFormatMember->s32MetaStreamIndex)
        {
            pu8Buffer = pstFormatMember->stPkt.data;
            u32DataLen = pstFormatMember->stPkt.size;
            break;
        }
        else
        {
            av_packet_unref(&pstFormatMember->stPkt);
            continue;
        }
    }

    u32NalLen = MP4_GetOneNalUnit(&u8NaluType, pu8Buffer, u32DataLen);
    if (u32NalLen <= 4)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_GetOneNalUnit nalu len error %d \n", u32NalLen);
        goto PARSE_MEATAI_UNREF_END;
    }

    if (FFMPEG_AVC_SPS == (u8NaluType & FFMPEG_H264_NAL_MASK))
    {
        pstFormatMember->pstMetaStream->codecpar->codec_id = AV_CODEC_ID_H264;
    }
    else if (FFMPEG_HEVC_SPS == ((u8NaluType >> 1) & FFMPEG_H265_NAL_MASK))
    {
        pstFormatMember->pstMetaStream->codecpar->codec_id = AV_CODEC_ID_HEVC;
    }
    else
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
            "I Frame codec type illegal:%d\n", u8NaluType);
        goto PARSE_MEATAI_UNREF_END;
    }

    s32Ret = MP4_GetMetaResolution(pstFormatMember, pu8Buffer + 4, u32NalLen - 4);
    if (s32Ret != HI_SUCCESS)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_GetMetaResolution  failed \n");
        goto PARSE_MEATAI_UNREF_END;
    }

PARSE_MEATAI_UNREF_END:
    av_packet_unref(&pstFormatMember->stPkt);
PARSE_MEATAI_2STREAMSTART_END:
    if (s32FirstStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM)
    {
        (HI_VOID)MP4_PARSE_SeekStreamStart(pstFormatMember->pstFormatContext, s32FirstStreamIndex);
    }
    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstFormatMember->iLastReadPts = 0;
    return s32Ret;
}

HI_S32 MP4_AVBSF_Init(AVBSFContext** ppstBsfCtx,
                      const AVStream* pstStream, const HI_CHAR* pBsfName)
{
    HI_S32 s32Ret = HI_FAILURE;
    AVBSFContext* pstBsfCtx = NULL;

    const AVBitStreamFilter* pstABSFilter = av_bsf_get_by_name(pBsfName);
    if (!pstABSFilter)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                       "Unknown bitstream filter %s\n", pBsfName);
        return HI_FAILURE;
    }

    s32Ret = av_bsf_alloc(pstABSFilter, &pstBsfCtx);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                       "av_bsf_alloc failed ret %d\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = avcodec_parameters_copy(pstBsfCtx->par_in, pstStream->codecpar);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR,
                       "avcodec_parameters_from_context failed ret %d\n", s32Ret);
        av_bsf_free(&pstBsfCtx);
        return HI_FAILURE;
    }

    pstBsfCtx->time_base_in = pstStream->time_base;

    s32Ret = av_bsf_init(pstBsfCtx);
    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "Error initializing bitstream filter: %s\n",
                       pstBsfCtx->filter->name);
        av_bsf_free(&pstBsfCtx);
        return HI_FAILURE;
    }

    *ppstBsfCtx = pstBsfCtx;
    return  HI_SUCCESS;
}

HI_VOID MP4_AVBSF_Deinit(AVBSFContext** ppstBsfCtx)
{
    av_bsf_free(ppstBsfCtx);
}

HI_S32 MP4_SET_AudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    //get  adts header for aac aduio
    HI_U8  u8ADTSHeader[FFMPEG_AAC_ADTS_LEN] = {0};
    HI_U32 u32SampleRateIdx = 3;/* 3 means 48000 sample rate, we use it as the default */
    HI_U32 u32Profile = AAC_PROFILE_LC;
    HI_U32 u32Index = 0;
    HI_U32 u32Channel =  pstFormatMember->pstAudioStream->codecpar->channels;
    HI_BOOL bHasADTS = HI_FALSE;

    for ( u32Index = 0 ; u32Index < FFMPEG_SAMPLE_RATE_NUM ; u32Index++ )
    {
        if (s_au32SampRateTab[u32Index] == (HI_U32)pstFormatMember->pstAudioStream->codecpar->sample_rate)
        {
            u32SampleRateIdx = u32Index;
            break;
        }
    }

    if (u32Index == FFMPEG_SAMPLE_RATE_NUM)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, " warning invalid s32SampleRate:%d,i guess as 48000HZ\n", pstFormatMember->pstAudioStream->codecpar->sample_rate);
    }

    /*
    0:Main profile
    1:Low Complexity profile(LC)
    2:Scalable Sampling Rate profile(SSR)
    3:(Reserved)
    */
    if (pstFormatMember->pstAudioStream->codecpar->profile < 0 || pstFormatMember->pstAudioStream->codecpar->profile >= 3)
    {
        u32Profile = AAC_PROFILE_LC;
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO,  " warning invalid profile:%d,i guess Low Complexity profile\n", pstFormatMember->pstAudioStream->codecpar->profile);
    }
    else
    {
        u32Profile = pstFormatMember->pstAudioStream->codecpar->profile;
    }

    MP4_GET_ADTSHeader(u32SampleRateIdx, (HI_U32)(pstFormatMember->stPkt.size), u32Profile, u32Channel, u8ADTSHeader);
    bHasADTS = MP4_JUDGE_HasAdtsHeader(u8ADTSHeader, FFMPEG_AAC_ADTS_LEN, pstFormatMember->stPkt.data, pstFormatMember->stPkt.size);

    if (!bHasADTS)
    {
        if (pstFormatMember->stPkt.size + FFMPEG_AAC_ADTS_LEN > FFMPEG_FRAME_BUF_DATA_LEN)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "audio data len beyond %d \n", FFMPEG_FRAME_BUF_DATA_LEN);
            return HI_FAILURE;
        }

        if (HI_SUCCESS !=  memset_s(pstFormatMember->aszAudioData, FFMPEG_FRAME_BUF_DATA_LEN, 0, FFMPEG_FRAME_BUF_DATA_LEN))
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set audio data fail\n");
            return HI_FAILURE;
        }
        if (HI_SUCCESS != memcpy_s(pstFormatMember->aszAudioData, FFMPEG_AAC_ADTS_LEN, u8ADTSHeader, FFMPEG_AAC_ADTS_LEN))
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "copy audio adts fail\n");
            return HI_FAILURE;
        }
        if (HI_SUCCESS != memcpy_s(pstFormatMember->aszAudioData + FFMPEG_AAC_ADTS_LEN, pstFormatMember->stPkt.size, pstFormatMember->stPkt.data,
                                   pstFormatMember->stPkt.size))
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "set audio data fail\n");
            return HI_FAILURE;
        }

        pstFmtFrame->pu8Data = pstFormatMember->aszAudioData;
        pstFmtFrame->u32Size = pstFormatMember->stPkt.size + FFMPEG_AAC_ADTS_LEN;
    }
    else
    {
        pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
        pstFmtFrame->u32Size = pstFormatMember->stPkt.size;
    }

    pstFormatMember->bHasAdts = bHasADTS;
    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts - pstFormatMember->pstAudioStream->start_time, pstFormatMember->pstAudioStream->time_base, AV_TIME_BASE_Q);
    return HI_SUCCESS;
}

HI_S32 MP4_SET_VideoInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ((AV_CODEC_ID_H264 == pstFormatMember->enVidCodecId
         || AV_CODEC_ID_HEVC == pstFormatMember->enVidCodecId) && !pstFormatMember->bPlayMetaVid )
    {
        s32Ret = MP4_FILTER_Packet(pstFormatMember);
        if (s32Ret != HI_SUCCESS && s32Ret != HI_RET_FILE_EOF)
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "exec MP4_FILTER_Packet failed\n");
            return  HI_ERR_DEMUXER_SKIP_FILE;
        }
        else if (s32Ret != HI_SUCCESS )
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "exec MP4_FILTER_Packet failed\n");
            return  s32Ret;
        }
    }

    //pkt.pts is time base on stream->timebase, need expressed in AV_TIME_BASE units
    pstFormatMember->iLastReadPts = av_rescale_q(pstFormatMember->stPkt.pts - pstFormatMember->pstUsedVideoStream->start_time, pstFormatMember->pstUsedVideoStream->time_base, AV_TIME_BASE_Q);
    pstFmtFrame->pu8Data = pstFormatMember->stPkt.data;
    pstFmtFrame->u32Size = pstFormatMember->stPkt.size;
    return HI_SUCCESS;
}



HI_S32 MP4_ProbeMeta(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember)
{

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = av_find_best_stream(pstFormatMember->pstFormatContext, AVMEDIA_TYPE_DATA, -1, -1, NULL, 0);

    if (s32Ret < 0)
    {
        FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "Could not find metadata stream in input file '%s', ret:%d\n", pstFormatMember->aszFilePath, s32Ret);
        pstFormatMember->s32MetaStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        pstFormatMember->pstMetaStream = NULL;
    }
    else
    {
        pstFormatMember->pstMetaStream = pstFormatMember->pstFormatContext->streams[s32Ret];
        pstFormatMember->s32MetaStreamIndex = s32Ret;

        //get metadata width height codectype by parse first i frame
        if (HI_SUCCESS != MP4_PARSE_MetaIFrame(pstFormatMember))
        {
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_ERR, "MP4_PARSE_MetaIFrame exec failed \n");
            pstFormatMember->pstMetaStream = NULL;
            pstFormatMember->s32MetaStreamIndex = HI_DEMUXER_NO_MEDIA_STREAM;
        }
        else
        {

            pstFormatMember->enMetaCodecId = pstFormatMember->pstMetaStream->codecpar->codec_id;
            if (NULL == pstFormatMember->pstUsedVideoStream)
            {
                pstFormatMember->s32UsedVideoStreamIndex = pstFormatMember->s32MetaStreamIndex ;
                pstFormatMember->pstUsedVideoStream = pstFormatMember->pstMetaStream;
                pstFormatMember->enUsedVidCodecId = pstFormatMember->enMetaCodecId;
                pstFormatMember->bPlayMetaVid = HI_TRUE;
            }

            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "meta codec type  %s\n", avcodec_get_name(pstFormatMember->enMetaCodecId));
            FFDEMUX_Printf(MODULE_NAME_DEMUX, HI_FFDEMUX_LOG_LEVEL_INFO, "meta timebase: %d %d \n", pstFormatMember->pstMetaStream->time_base.den, pstFormatMember->pstMetaStream->time_base.num);
        }
    }
    return HI_SUCCESS;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
