/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap_arch.h
 * @brief   NDK acap arch head file.
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#ifndef __MAPI_ACAP_ARCH_H__
#define __MAPI_ACAP_ARCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_ACAP_RegisterVQEModule(HI_VOID);
HI_VOID *HAL_MAPI_Resampler_Create(HI_S32 s32Inrate, HI_S32 s32Outrate, HI_S32 s32Chans);

HI_VOID HAL_MAPI_Resampler_Destroy(HI_VOID *inst);

HI_S32 HAL_MAPI_Resampler_Process(HI_VOID *inst, HI_S16 *s16Inbuf, HI_S32 s32Insamps, HI_S16 *s16Outbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_ACAP_ARCH_H__ */
