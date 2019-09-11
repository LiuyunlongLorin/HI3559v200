/**
 * @file      hi_product_param_debug.h
 * @brief     parameter module debug interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/15
 * @version   1.0

 */

#ifndef __HI_PRODUCT_PARAM_DEBUG_H__
#define __HI_PRODUCT_PARAM_DEBUG_H__

#include "hi_product_param_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

/**
 * @brief     get workmode string
 * @param[in] enWorkMode : workmode enum
 * @return    workmode string
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
const HI_CHAR* PDT_PARAM_GetWorkModeStr(HI_PDT_WORKMODE_E enWorkMode);

/**
 * @brief     get mediamode string
 * @param[in] enMediaMode : mediamode enum
 * @return    mediamode string
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
const HI_CHAR* PDT_PARAM_GetMediaModeStr(HI_PDT_MEDIAMODE_E enMediaMode);

/**
 * @brief     get poweron action string
 * @param[in] enPoweronAction : poweron action enum
 * @return    poweron action string
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
const HI_CHAR* PDT_PARAM_GetPoweronActionStr(HI_PDT_POWERON_ACTION_E enPoweronAction);

/**
 * @brief     get param type string
 * @param[in] enType : param type enum
 * @return    param type string
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
const HI_CHAR* PDT_PARAM_GetParamTypeStr(HI_PDT_PARAM_TYPE_E enType);

#ifdef CFG_DEBUG_LOG_ON

/**
 * @brief     debug filemng configure
 * @param[in] pstCfg : filemng configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/15
 */
HI_VOID PDT_PARAM_DebugFileMngCfg(const HI_PDT_FILEMNG_CFG_S *pstCfg);

/**
 * @brief     debug keymng configure
 * @param[in] pstCfg : keymng configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/15
 */
HI_VOID PDT_PARAM_DebugKeyMngCfg(const HI_KEYMNG_CFG_S *pstCfg);

/**
 * @brief     debug WiFi AP configure
 * @param[in] pstCfg : WiFi AP configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugWiFiAPCfg(const HI_HAL_WIFI_APMODE_CFG_S *pstCfg);

/**
 * @brief     debug device information configure
 * @param[in] pstCfg : device information configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugDevInfo(const HI_UPGRADE_DEV_INFO_S *pstCfg);

/**
 * @brief     debug timedtask configure
 * @param[in] pszName : timedtask name
 * @param[in] pstCfg : timedtask configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugTimedTask(const HI_CHAR* pszName, const HI_TIMEDTASK_ATTR_S *pstCfg);

/**
 * @brief     debug normal record configure
 * @param[in] pstCfg : normal record configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugNormRecCfg(const HI_PDT_NORM_REC_ATTR_S *pstCfg);

/**
 * @brief     debug loop record configure
 * @param[in] pstCfg : loop record configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugLoopRecCfg(const HI_PDT_LOOP_REC_ATTR_S *pstCfg);

/**
 * @brief     debug lapse record configure
 * @param[in] pstCfg : lapse record configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugLapseRecCfg(const HI_PDT_LAPSE_REC_ATTR_S *pstCfg);

/**
 * @brief     debug slow record configure
 * @param[in] pstCfg : slow record configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugSlowRecCfg(const HI_PDT_SLOW_REC_ATTR_S *pstCfg);

/**
 * @brief     debug single photo configure
 * @param[in] pstCfg : single photo configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugSinglePhotoCfg(const HI_PDT_SINGLE_PHOTO_ATTR_S *pstCfg);

/**
 * @brief     debug delay photo configure
 * @param[in] pstCfg : delay photo configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugDelayPhotoCfg(const HI_PDT_DELAY_PHOTO_ATTR_S *pstCfg);

/**
 * @brief     debug lapse photo configure
 * @param[in] pstCfg : lapse photo configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugLapsePhotoCfg(const HI_PDT_LAPSE_PHOTO_ATTR_S *pstCfg);

/**
 * @brief     debug burst configure
 * @param[in] pstCfg : burst configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugBurstCfg(const HI_PDT_BURST_ATTR_S *pstCfg);

/**
 * @brief     debug rec+snap configure
 * @param[in] pstCfg : rec+snap configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugRecSnapCfg(const HI_PDT_RECSNAP_ATTR_S *pstCfg);

/**
 * @brief     debug uvc configure
 * @param[in] pstCfg : uvc configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugUVCCfg(const HI_PDT_PARAM_UVC_ATTR_S *pstCfg);

/**
 * @brief     debug usb storage configure
 * @param[in] pstCfg : usb storage configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugUSBStorageCfg(const HI_USB_STORAGE_CFG_S *pstCfg);

/**
 * @brief     debug HDMI preview configure
 * @param[in] pstCfg : HDMI preview configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/27
 */
HI_VOID PDT_PARAM_DebugHDMIPreviewCfg(const HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S *pstCfg);

/**
 * @brief     debug HDMI playback configure
 * @param[in] pstCfg : HDMI playback configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/27
 */
HI_VOID PDT_PARAM_DebugHDMIPlaybackCfg(const HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S *pstCfg);

/**
 * @brief     debug playback configure
 * @param[in] pstCfg : playback configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/27
 */
HI_VOID PDT_PARAM_DebugPlaybackCfg(const HI_PDT_PARAM_PLAYBACK_ATTR_S *pstCfg);

/**
 * @brief     debug venc attribute
 * @param[in] pstAttr : venc attribute
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/9
 */
HI_VOID PDT_PARAM_DebugVencAttr(const HI_MEDIA_VENC_ATTR_S *pstAttr);

/**
 * @brief     debug media configure
 * @param[in] pstCfg : mediamode configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugMediaCfg(const HI_PDT_MEDIA_CFG_S *pstCfg);

/**
 * @brief     debug valueset configure
 * @param[in] pstCfg : valueset configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugItemValues(const HI_PDT_ITEM_VALUESET_S* pstValues);

#else

#define PDT_PARAM_DebugFileMngCfg(pstCfg)
#define PDT_PARAM_DebugKeyMngCfg(pstCfg)
#define PDT_PARAM_DebugWiFiAPCfg(pstCfg)
#define PDT_PARAM_DebugDevInfo(pstCfg)
#define PDT_PARAM_DebugTimedTask(pszName,pstCfg)
#define PDT_PARAM_DebugNormRecCfg(pstCfg)
#define PDT_PARAM_DebugLoopRecCfg(pstCfg)
#define PDT_PARAM_DebugLapseRecCfg(pstCfg)
#define PDT_PARAM_DebugSlowRecCfg(pstCfg)
#define PDT_PARAM_DebugSinglePhotoCfg(pstCfg)
#define PDT_PARAM_DebugDelayPhotoCfg(pstCfg)
#define PDT_PARAM_DebugLapsePhotoCfg(pstCfg)
#define PDT_PARAM_DebugBurstCfg(pstCfg)
#define PDT_PARAM_DebugRecSnapCfg(pstCfg)
#define PDT_PARAM_DebugUVCCfg(pstCfg)
#define PDT_PARAM_DebugUSBStorageCfg(pstCfg)
#define PDT_PARAM_DebugHDMIPreviewCfg(pstCfg)
#define PDT_PARAM_DebugHDMIPlaybackCfg(pstCfg)
#define PDT_PARAM_DebugPlaybackCfg(pstCfg)
#define PDT_PARAM_DebugVencAttr(pstAttr)
#define PDT_PARAM_DebugMediaCfg(pstCfg)
#define PDT_PARAM_DebugItemValues(pstValues)

#endif

/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_DEBUG_H__ */

