/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_hal_ahd.h
 * @brief   ahd module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_HAL_AHD_H__
#define __HI_MAPI_HAL_AHD_H__

#include "hi_mapi_hal_ahd_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AHD */
/** @{ */ /** <!-- [AHD] */

/* Interface */

HI_S32 HI_MAPI_VCAP_RegAhdModule(HI_HANDLE vcapHandle, HI_HAL_AHD_DEV_S *pstHalDev);

HI_S32 HI_MAPI_VCAP_AhdChangeChn(HI_HANDLE vcapHandle, HI_BOOL u32Chn[], HI_U32 u32Cnt);

HI_S32 HI_MAPI_VCAP_AhdGetStatus(HI_HANDLE VcapDevHdl, HI_HAL_AHD_STATUS_E astAhdStatus[], HI_U32 u32Cnt);

HI_S32 HI_MAPI_VCAP_UnRegAhdModule(HI_HANDLE vcapHandle);

/** @} */ /** <!-- ==== AHD End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_HAL_AHD_H__ */
