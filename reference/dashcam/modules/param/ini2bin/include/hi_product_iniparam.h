/**
 * @file      hi_product_iniparam.h
 * @brief     iniparam header
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */

#ifndef __HI_PRODUCT_INIPARAM_H__
#define __HI_PRODUCT_INIPARAM_H__

#include "hi_appcomm.h"
#include "hi_confaccess.h"
#include "hi_product_param_inner.h"

#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/** \addtogroup     INIPARAM */
/** @{ */  /** <!-- [INIPARAM] */

/** product ini identification */
#define PDT_INIPARAM  "productini"

/** ini configure access totality information file */
#define PDT_INIPARAM_PATH "./config_cfgaccess_entry.ini"

/** ini configure module: media_common */
#define PDT_INIPARAM_MEDIA_COMM "media_common"

/** ini configure module: workmode_common */
#define PDT_INIPARAM_WORKMODE_COMM "workmode_common"

/** ini configure module: workmode_record */
#define PDT_INIPARAM_WORKMODE_RECORD "workmode_record"

/** ini configure module: workmode_record */
#define PDT_INIPARAM_WORKMODE_PHOTO "workmode_photo"

/** ini configure module: workmode_playback */
#define PDT_INIPARAM_WORKMODE_PLAYBACK "workmode_playback"

/** ini configure module: workmode_usb */
#define PDT_INIPARAM_WORKMODE_USB "workmode_usb"

/** ini configure module: filemng */
#define PDT_INIPARAM_FILEMNG "filemng"

/** ini configure module: storagemng */
#define PDT_INIPARAM_DEVMNG "devmng"

/** ini configure module: valueset */
#define PDT_INIPARAM_VALUESET "valueset"

/** ini configure module: mediamode */
#define PDT_INIPARAM_MEDIAMODE "mediamode_"


/** ini module name maximum length */
#define PDT_INIPARAM_MODULE_NAME_LEN HI_APPCOMM_COMM_STR_LEN

/** ini node name maximum length */
#define PDT_INIPARAM_NODE_NAME_LEN  (128)


/** Load IniNode Result Check */
#define PDT_INIPARAM_CHECK_LOAD_RESULT(ret, name) \
    do{ \
        if (HI_SUCCESS != ret){   \
            MLOGE(" Load [%s] failed\n", name); \
            return HI_FAILURE;  \
        }   \
    }while(0)


HI_S32 HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(const HI_CHAR *pazEnumStr,
                HI_PDT_MEDIAMODE_E *penMediaMode);

/**
 * @brief     load filemanager configure
 * @param[out]pstParam : filemng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PDT_INIPARAM_LoadFileMngConf(HI_PDT_FILEMNG_CFG_S *pstParam);

/**
 * @brief     load devmng configure
 * @param[out]pstParam : devmng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PDT_INIPARAM_LoadDevMngConf(PDT_PARAM_DEVMNG_CFG_S *pstParam);

/**
 * @brief     load media common configure
 * @param[out]pstParam : media common configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PDT_INIPARAM_LoadMediaCommConf(PDT_PARAM_MEDIA_COMM_CFG_S *pstParam);

/**
 * @brief     load workmode configure
 * @param[out]pstParam : workmode configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PDT_INIPARAM_LoadWorkModeConf(PDT_PARAM_WORKMODE_CFG_S *pstParam);

/**
 * @brief     load valueset configure
 * @param[out]pstParam : configure item valueset
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PDT_INIPARAM_LoadValueSetConf(PDT_PARAM_CONFITEM_VALUESET_S *pstParam);


/**
* @brief load all cam Media configure
* @param[in] pstParam
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/30
*/
HI_S32 HI_PDT_INIPARAM_LoadCamMediaCfg(PDT_PARAM_CAM_MEDIA_CFG_S *pstParam);

/** @}*/  /** <!-- ==== INIPARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_PRODUCT_INIPARAM_H__ */

