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

#ifdef CONFIG_MOTIONDETECT_ON
/**
 * @brief     get VideoDetect config
 * @param[out]pstVideoDetectCfg : VideoDetect configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/12/26
 */
HI_S32 HI_PDT_PARAM_GetVideoDetectCfg(HI_S32 s32CamID, HI_PDT_PARAM_VIDEODETECT_CFG_S* pstVideoDetectCfg);
#endif

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
* @brief get gsensor configure
* @param[in] pstCfg: gsensor configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/21
*/
HI_S32 HI_PDT_PARAM_GetGsensorCfg(HI_PDT_GSENSOR_CFG_S *pstCfg);


/**
 * @brief     get osd video attribute
 * @param[out]pstOsdCfg : configure for osd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/2
 */
HI_S32 HI_PDT_PARAM_GetOsdCfg(HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg);

/**
* @brief  get Cam media configure through pstMediaModeCfg
* @param[in] enWorkMode: work mode
* @param[in] pstMediaModeCfg: MediaMode configure
* @param[in] pstCamCfg: Cam Media configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/3
*/
HI_S32 HI_PDT_PARAM_GetCamMediaCfg(HI_PDT_WORKMODE_E enWorkMode,
    HI_PDT_PARAM_MEDIAMODE_CFG_S *pstMediaModeCfg,
    HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCamCfg,
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S *pstSceneAttr);


/**
* @brief   get media comm configure
* @param[out] pstMediaCommCfg: media comm configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/3
*/
HI_S32 HI_PDT_PARAM_GetMediaCommCfg(HI_PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaCommCfg);

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
* @brief    get specified mediamode configure's venc configure
* @param[in] enWorkMode : work mode enum
* @param[in] pstMediaModeCfg: mediamode configure
* @param[in] enPayload: payload type
* @param[out] pstVencCfg:venc configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/11
*/
HI_S32 HI_PDT_PARAM_GetMainVencCfgByPayloadType(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_MEDIAMODE_CFG_S *pstMediaModeCfg,
    HI_MAPI_PAYLOAD_TYPE_E enPayload, HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg);

/**
* @brief    get cam related Item Values.
* @param[in] enWorkMode:workmode
* @param[in] s32CamID: the specify cam id,valid value range: [0,HI_PDT_MEDIA_VCAP_DEV_MAX_CNT]
* @param[in] enType: param type
* @param[in] pvParam: param value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/22
*/
HI_S32 HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam);

/**
* @brief    set cam related Item Values.
* @param[in] enWorkMode: workmode
* @param[in] s32CamID: the specify cam id,valid value range: [0,HI_PDT_MEDIA_VCAP_DEV_MAX_CNT]
* @param[in] enType: param type
* @param[in] pvParam: param value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/22
*/
HI_S32 HI_PDT_PARAM_SetCamParam(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam);

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
* @brief get valueset that specify CamID by s32CamID and type bype enType
* @param[in] s32CamID :CamId
* @param[in] enType :param type
* @param[out] pstValueSet: valueset
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/4/4
*/
HI_S32 HI_PDT_PARAM_GetCamItemValues(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet);

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

/**
* @brief    get live source by cam ID
* @param[in] s32CamID: cam ID
* @param[out] pstDispSrc: live source pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_GetLiveSrcByCamID(HI_S32 s32CamID, HI_PDT_MEDIA_BIND_SRC_CFG_S * pstLiveSrc);

/**
* @brief    get disp source by cam ID
* @param[in] s32CamID: cam ID
* @param[out] pstDispSrc: disp source pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_GetDispSrcByCamID(HI_S32 s32CamID, HI_PDT_MEDIA_BIND_SRC_CFG_S *pstDispSrc);

/**
* @brief get media mode configure by specified workmode
* @param[in] enWorkMode: workmode
* @param[out] pstCamCfg: Cam mediamode configure structure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_GetMediaModeCfg(HI_PDT_WORKMODE_E enWorkMode,HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S *pstCamCfg);

/**
* @brief  get media configure by Cam context
* @param[in] enWorkMode: work mode
* @param[in] pstCamCtx: cam context array pointer, the array element count is HI_PDT_MEDIA_VCAP_DEV_MAX_CNT
* @param[out] pstMediaCfg: media out configure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_CAM_CONTEXT_S *pstCamCtx,HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
* @brief    get scene configure by Cam context
* @param[in] enWorkMode: work mode
* @param[in] pstCamCtx: cam context array pointer, the array element count is HI_PDT_MEDIA_VCAP_DEV_MAX_CNT
* @param[in] pstSceneMode: scene mode structure pointer
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_GetSceneCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_CAM_CONTEXT_S *pstCamCtx,HI_PDT_SCENE_MODE_S *pstSceneMode);


/**
* @brief    update media config by record type,but not saved by param module
* @param[in-out] pstMediaCfg: media configure
             after updated,used by upper app
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/5/23
*/
HI_S32 HI_PDT_PARAM_RecordUpdateMediaCfg(HI_PDT_MEDIA_CFG_S *pstMediaCfg);

/**
* @brief    get exif info
* @param[out] pstExifInfo: exif info struct
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/6/1
*/
HI_S32 HI_PDT_PARAM_GetExifInfo(HI_MAPI_SNAP_EXIF_INFO_S * pstExifInfo);

/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_H__ */

