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
* @brief get workmode string
* @param[in] enWorkMode:workmode enum
* @return workmode string
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
const HI_CHAR* PDT_PARAM_GetWorkModeStr(HI_PDT_WORKMODE_E enWorkMode);

/**
* @brief get Mediamode string
* @param[in] enMediaMode: mediamode enum
* @return mediamode string
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
const HI_CHAR* PDT_PARAM_GetMediaModeStr(HI_PDT_MEDIAMODE_E enMediaMode);

/**
* @brief get param type string
* @param[in] enType:param type enum
* @return param type string.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
const HI_CHAR* PDT_PARAM_GetParamTypeStr(HI_PDT_PARAM_TYPE_E enType);


#ifdef CFG_PARAM_DEBUG_LOG_ON

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
* @brief    debug gsensor configure
* @param[in] pstCfg: gsensor configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
HI_VOID PDT_PARAM_DebugGsensorCfg(const HI_PDT_GSENSOR_CFG_S *pstCfg);

/**
 * @brief     debug WiFi info
 * @param[in] pstCfg : WiFi enable and mode
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/8/1
 */
HI_VOID PDT_PARAM_DebugWiFiInfo(const HI_PDT_WIFI_INFO_S *pstCfg);

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
HI_VOID PDT_PARAM_DebugDevInfo(const HI_PDT_DEV_INFO_S *pstCfg);

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
* @brief    debug file attr
* @param[in] pstCfg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
HI_VOID PDT_PARAM_DebugFileAttr(const HI_RECMNG_FILE_ATTR_S *pstCfg);

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
* @brief    debug playback configure
* @param[in] pstCfg: playback configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/23
*/
HI_VOID PDT_PARAM_DebugPlayBackCfg(const HI_PDT_PARAM_PLAYBACK_CFG_S *pstCfg);

/**
 * @brief     debug uvc configure
 * @param[in] pstCfg : uvc configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugUVCCfg(const HI_PDT_PARAM_UVC_CFG_S *pstCfg);

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
* @brief    debug venc config
* @param[in] pstCfg:venc config pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/16
*/
HI_VOID PDT_PARAM_DebugVencCfg(const HI_PDT_MEDIA_VENC_CFG_S *pstCfg);

/**
* @brief debug cam media configure
* @param[in] pstCfg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
HI_VOID PDT_PARAM_DebugCamMediaCfg(const HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCfg);

/**
* @brief    debug scene vcap configure
* @param[in] pstCfg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/7
*/
HI_VOID PDT_PARAM_DebugSceneCamVcapCfg(const HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S *pstCfg);

/**
* @brief debug media comm configure
* @param[in] pstCfg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
HI_VOID PDT_PARAM_DEBUGMediaCommCfg(HI_PDT_PARAM_MEDIA_COMM_CFG_S *pstCfg);

/**
 * @brief     debug valueset configure
 * @param[in] pstCfg : valueset configure
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/16
 */
HI_VOID PDT_PARAM_DebugItemValues(const HI_PDT_ITEM_VALUESET_S* pstValues);

/**
 * @brief     debug osd video attr
 * @param[in] pstVideoCfg : osd video attr
 * @return    NA
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/15
 */
HI_VOID PDT_PARAM_DebugOSDVideoAttr(HI_PDT_MEDIA_OSD_VIDEO_CFG_S* pstVideoCfg);
#else

#define PDT_PARAM_DebugFileMngCfg(pstCfg)
#ifdef CONFIG_MOTIONDETECT_ON
#define PDT_PARAM_DebugVideoDetectCfg(pstVideoDetectCfg)
#endif
#define PDT_PARAM_DebugKeyMngCfg(pstCfg)
#define PDT_PARAM_DebugGsensorCfg(pstCfg)
#define PDT_PARAM_DebugWiFiAPCfg(pstCfg)
#define PDT_PARAM_DebugWiFiInfo(pstCfg)
#define PDT_PARAM_DebugDevInfo(pstCfg)
#define PDT_PARAM_DebugTimedTask(pszName,pstCfg)
#define PDT_PARAM_DebugFileAttr(pstCfg)
#define PDT_PARAM_DebugNormRecCfg(pstCfg)
#define PDT_PARAM_DebugPhotoCfg(pstCfg)
#define PDT_PARAM_DebugPlayBackCfg(pstCfg)
#define PDT_PARAM_DebugUVCCfg(pstCfg)
#define PDT_PARAM_DebugUSBStorageCfg(pstCfg)
#define PDT_PARAM_DebugVencCfg(pstCfg)
#define PDT_PARAM_DebugCamMediaCfg(pstCfg)
#define PDT_PARAM_DebugSceneCamVcapCfg(pstCfg)
#define PDT_PARAM_DEBUGMediaCommCfg(pstCfg)
#define PDT_PARAM_DebugItemValues(pstValues)
#define PDT_PARAM_DebugOSDVideoAttr(pstVideoCfg)

#endif

/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_DEBUG_H__ */

