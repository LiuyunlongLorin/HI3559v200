/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_hdmi_os.h
 * @brief   hdmi os header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_DISP_OS_H__
#define __MAPI_DISP_OS_H__

#include "mapi_hdmi_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_HDMI_Config_CallbackFunc(const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc,
                                         HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackconfig);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_DISP_OS_H__ */

