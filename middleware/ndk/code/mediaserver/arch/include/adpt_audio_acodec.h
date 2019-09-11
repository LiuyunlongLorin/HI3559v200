/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    adapt_audio_acodec.h
 * @brief   adapt audio acodec header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __ADPT_ACAP_ACODEC_H__
#define __ADPT_ACAP_ACODEC_H__

#include "hi_type.h"
#include "hi_comm_aio.h"
#include "hi_mapi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* interface */

HI_S32 ADPT_ACODEC_Open(HI_VOID);

HI_S32 ADPT_ACODEC_Close(HI_VOID);

HI_S32 ADPT_ACODEC_ConfigAttr(AUDIO_SAMPLE_RATE_E enSampleRate, ACODEC_MIXER_E enMixerMicMode);

HI_S32 ADPT_ACODEC_SetAIVol(HI_S32 s32Vol);

HI_S32 ADPT_ACODEC_GetAIVol(HI_S32 *s32Vol);

HI_S32 ADPT_ACODEC_SetAOVol(HI_S32 s32Vol);

HI_S32 ADPT_ACODEC_GetAOVol(HI_S32 *s32Vol);

HI_S32 ADPT_ACODEC_MuteAI(HI_VOID);

HI_S32 ADPT_ACODEC_UnmuteAI(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __ADPT_ACAP_ACODEC_H__ */
