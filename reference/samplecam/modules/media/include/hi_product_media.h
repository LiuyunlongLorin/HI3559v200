
/**
 * @file    hi_product_media.h
 * @brief   media module struct and interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co., Ltd.
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2019/4/1
 * @version   1.0

 */

#ifndef __HI_PRODUCT_MEDIA_H__
#define __HI_PRODUCT_MEDIA_H__

#include "hi_product_media_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     MEDIA */
/** @{ */  /** <!-- [MEDIA] */


#define HI_PDT_MEDIA_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_EINVAL)       /**<Invalid argument */
#define HI_PDT_MEDIA_ENOTINIT       HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_ENOINIT)      /**<Not inited */


HI_S32 HI_PDT_MEDIA_Init(const MEDIA_ViVpssMode *viVpssMode, const MEDIA_VbCfg *vbCfg);
HI_S32 HI_PDT_MEDIA_Deinit(HI_VOID);

HI_S32 HI_PDT_MEDIA_VideoInit(const MEDIA_VideoCfg *videoCfg);
HI_S32 HI_PDT_MEDIA_VideoDeinit(HI_VOID);

HI_S32 HI_PDT_MEDIA_VideoOutInit(const MEDIA_VideoOutCfg *videoOutCfg);
HI_S32 HI_PDT_MEDIA_VideoOutDeinit(HI_VOID);
HI_S32 HI_PDT_MEDIA_VideoOutStart(const MEDIA_VideoOutCfg *videoOutCfg);
HI_S32 HI_PDT_MEDIA_VideoOutStop(HI_VOID);



/** @}*/  /** <!-- ==== MEDIA End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /** End of __HI_PRODUCT_MEDIA_H__ */

