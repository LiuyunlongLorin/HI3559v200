/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_sys_arch.h
 * @brief   NDK sys arch head file.
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#ifndef __MAPI_SYS_ADAPT_H__
#define __MAPI_SYS_ADAPT_H__

#include "hi_mapi_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_Sys_EnableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr);
HI_S32 HAL_MAPI_Sys_DisableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr);
HI_VOID HAL_MAPI_VB_SetSupplementConfig(VB_SUPPLEMENT_CONFIG_S *pstSupplementConf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_SYS_ADAPT_H__ */
