/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file         ts_format.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */
#ifndef __COMMON_FORMAT_H__
#define __COMMON_FORMAT_H__

#include "hi_type.h"
#include "hi_defs.h"
#include "ffdemux_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
HI_S32 COMMON_Read(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame);
HI_S32 COMMON_Free(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_PACKET_S* pstFmtFrame);
HI_S32 COMMON_Seek(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_S32 s32StreamIndex, HI_S64 s64MSec, HI_S32 s32Flag);
HI_S32 COMMON_Probe(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer);
HI_S32 COMMON_Getinfo(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_FILE_INFO_S* pstFmtInfo);
HI_S32 COMMON_Setattr(HI_FFDEMUXER_FORMAT_MEMBER_S* pstDemuxer, HI_FORMAT_MEDIA_ATTR_S* pstFmtInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __COMMON_FORMAT_H__ */
