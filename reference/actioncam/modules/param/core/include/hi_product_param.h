/**
 * @file      hi_product_param.h
 * @brief     parameter module interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#ifndef __HI_PRODUCT_PARAM_H__
#define __HI_PRODUCT_PARAM_H__

#include "hi_product_param_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

/** error code define */
#define HI_PDT_PARAM_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_PARAM, HI_EINVAL)       /**<Invalid argument */
#define HI_PDT_PARAM_ENOTINIT       HI_APPCOMM_ERR_ID(HI_APP_MOD_PARAM, HI_ENOINIT)      /**<Not inited */
#define HI_PDT_PARAM_EUNSUPPORT     HI_APPCOMM_ERR_ID(HI_APP_MOD_PARAM, HI_EPAERM)       /**<Unsupport */
#define HI_PDT_PARAM_EINITIALIZED   HI_APPCOMM_ERR_ID(HI_APP_MOD_PARAM, HI_EINITIALIZED) /**<re-initialized */

/** event id define */
#define HI_EVENT_PDT_PARAM_CHANGE   HI_APPCOMM_EVENT_ID(HI_APP_MOD_PARAM, 1)


/**
 * @brief     register event
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/4
 */
HI_S32 HI_PDT_PARAM_RegisterEvent(HI_VOID);

/**
 * @brief     Parameter Module Init
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_Init(HI_VOID);

/**
 * @brief     Parameter Module Deinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_Deinit(HI_VOID);

/**
 * @brief     Recover Default Parameter
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_SetDefault(HI_VOID);

/**
 * @brief     get filemng configure
 * @param[out]pstCfg : filemng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetFileMngCfg(HI_PDT_FILEMNG_CFG_S *pstCfg);

/**
 * @brief     get storage configure
 * @param[out]pstCfg : storage configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetStorageCfg(HI_STORAGEMNG_CFG_S *pstCfg);

/**
 * @brief     get keymng configure
 * @param[out]pstCfg : keymng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetKeyMngCfg(HI_KEYMNG_CFG_S *pstCfg);

/**
 * @brief     get gaugemng configure
 * @param[out]pstCfg : gaugemng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetGaugeMngCfg(HI_GAUGEMNG_CFG_S *pstCfg);

/**
 * @brief     get media configure except vo(hal screen)
 * @param[in] enWorkMode  : workmode enum
 * @param[in] enMediaMode : media mode enum
 * @param[out]pstCfg : media configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_MEDIA_CFG_S *pstCfg, HI_PDT_SCENE_MODE_S* pstSceneMode);

/**
 * @brief     get scene param with specified media mode and scene type
 * @param[in] enMediaMode : media mode enum
 * @param[in] enSceneType : scene type, linear/hdr/mfnr
 * @param[in] pstSceneParam : scene param, eg. param index
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/29
 */
HI_S32 HI_PDT_PARAM_GetSceneCfg(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_PARAM_SCENE_TYPE_E enSceneType,
                HI_PDT_PARAM_SCENE_CFG_S* pstSceneCfg);

/**
 * @brief     get media capability with specified mediamode
 * @param[in] enMediaMode : mediamode
 * @param[in] pstCapability : media capability, eg. LDC/DIS
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/9
 */
HI_S32 HI_PDT_PARAM_GetMediaCapability(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_PARAM_MEDIA_CAPABILITY_S* pstCapability);

/**
 * @brief     get main record venc attribute by payload type
 * @param[in] enMediaMode : mediamode
 * @param[in] enPayload : encoder payload type, eg. h264/h265
 * @param[in] pstAttr : video encoder attribute
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/9
 */
HI_S32 HI_PDT_PARAM_GetVencAttrByPayloadType(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_MAPI_PAYLOAD_TYPE_E enPayload, HI_MEDIA_VENC_ATTR_S* pstAttr);

/**
 * @brief     get osd video attribute
 * @param[out]pstVideoAttr : video attribute for osd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/2
 */
HI_S32 HI_PDT_PARAM_GetOSDVideoAttr(HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr);

/**
 * @brief     get workmode configure
 * @param[in] enWorkMode : work mode enum
 * @param[out]pstCfg : workmode configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_WORKMODE_CFG_S *pstCfg);

/**
 * @brief     get burst type by mediamode, called in case switch resolution in mphoto workmode
 * @param[in] enMediaMode : mediamode
 * @param[in] penType : burst type
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/9/26
 */
HI_S32 HI_PDT_PARAM_GetBurstType(HI_PDT_MEDIAMODE_E enMediaMode, HI_PDT_JPG_BURST_TYPE_E* penType);

/**
 * @brief     get workmode dependent parameter configure
 * @param[in] enWorkMode : work mode enum
 * @param[in] enType : param type
 * @param[out]pvParam : param value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam);

/**
 * @brief     set workmode dependent parameter configure
 * @param[in] enWorkMode : work mode enum
 * @param[in] enType : param type
 * @param[in] pvParam : param value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_SetWorkModeParam(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam);

/**
 * @brief     get common parameter configure
 * @param[in] enType : param type
 * @param[out]pvParam : param value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam);

/**
 * @brief     set common parameter configure
 * @param[in] enType : param type
 * @param[in] pvParam : param value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam);

/**
 * @brief     get workmode dependent configure item valueset
 * @param[in] enWorkMode : work mode enum
 * @param[in] enType : param type
 * @param[out]pstValueSet : valueset
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetWorkModeItemValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet);

/**
 * @brief     get common configure item valueset
 * @param[in] enType : param type
 * @param[out]pstValueSet : valueset
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_PDT_PARAM_GetCommItemValues(HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet);


/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_H__ */

