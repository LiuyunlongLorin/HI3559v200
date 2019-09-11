/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         ts_format.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __TS_FORMAT_H__
#define __TS_FORMAT_H__

#include "hi_type.h"
#include "hi_defs.h"
#include "ffdemux_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct hiAAC_MEDIA_INFO_S
{
    HI_U32   u32Sync;                           /* syncword */
    HI_U32   u32SampleRate;
    HI_U32   u32ChannelNum;
    HI_U32  u32FrameLength;
} AAC_MEDIA_INFO_S;


HI_S32 TS_SET_AudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame);
HI_S32 TS_SET_VideoInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame);
HI_S32 TS_ProbeAudio(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer);
HI_S32 TS_Seek_MS(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer, HI_S64 s64MSec, __attribute__((unused))HI_S32 s32Flag);

HI_S32 TS_Read_UntilIframe(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer);

HI_S32 TS_Probe_ByParseFrame(HI_FFDEMUXER_FORMAT_MEMBER_S* pstTsDemuxer);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TS_FORMAT_H__ */
