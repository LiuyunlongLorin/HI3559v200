/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_hdmi.h
 * @brief   hdmi module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_HDMI_H__
#define __HI_MAPI_HDMI_H__

#include "hi_mapi_hdmi_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     HDMI */
/** @{ */ /** <!-- [HDMI] */

/**
 * @brief  HDMI Init
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @param[in] pstHdmiCallBack  HI_MAPI_HDMI_CALLBACK_FUNC_S: HDMI event Callback function
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_Init(HI_HDMI_ID_E enHdmi, const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc);

/**
 * @brief  HDMI Deinit
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_Deinit(HI_HDMI_ID_E enHdmi);

/**
 * @brief  Get sink capability
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @param[out] pstsinkcapabiility HI_MAPI_HDMI_SINKCAPABILITY_S: the capability of  sinck
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_GetSinkCapability(HI_HDMI_ID_E enHdmi, HI_MAPI_HDMI_SINKCAPABILITY_S *pstsinkcapabiility);

/**
 * @brief  Get HDMI attribute
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @param[out] psthdmiattr HI_MAPI_HDMI_ATTR_S: the attribute of  HDMI
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_GetAttr(HI_HDMI_ID_E enHdmi, HI_MAPI_HDMI_ATTR_S *psthdmiattr);

/**
 * @brief  Set HDMI attribute
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @param[in] psthdmiattr HI_MAPI_HDMI_ATTR_S: the attribute of  HDMI
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_E enHdmi, const HI_MAPI_HDMI_ATTR_S *psthdmiattr);

/**
 * @brief  Start HDMI
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_Start(HI_HDMI_ID_E enHdmi);

/**
 * @brief  Stop HDMI
 * @param[in] enHdmi HI_HDMI_ID_E: handle of HDMI
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_HDMI_Stop(HI_HDMI_ID_E enHdmi);

/** @} */ /** <!-- ==== HDMI End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_HDMI_H__ */

