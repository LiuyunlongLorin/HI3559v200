/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         mp4_format.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __MP4_FORMAT_H__
#define __MP4_FORMAT_H__

#include "hi_type.h"
#include "hi_defs.h"
#include "ffdemux_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 MP4_AVBSF_Init(AVBSFContext** ppstBsfCtx,
                             const AVStream* pstStream, const HI_CHAR* pBsfName);
HI_S32 MP4_SET_AudioInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame);

HI_S32 MP4_SET_VideoInfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember, HI_FORMAT_PACKET_S* pstFmtFrame);

HI_S32 MP4_ProbeMeta(HI_FFDEMUXER_FORMAT_MEMBER_S* pstFormatMember);

HI_VOID MP4_AVBSF_Deinit(AVBSFContext** ppstBsfCtx);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MP4_FORMAT_H__ */
