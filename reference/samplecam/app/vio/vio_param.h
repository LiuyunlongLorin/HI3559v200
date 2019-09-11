
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

#ifndef __HI_VIO_PARAM_H__
#define __HI_VIO_PARAM_H__

#include "hi_product_media_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

typedef enum tagPARAM_DisplayType {
    PARAM_DISPLAY_TYPE_HDMI,
    PARAM_DISPLAY_TYPE_LCD,
    PARAM_DISPLAY_TYPE_NONE,
    PARAM_DISPLAY_TYPE_BUTT
} PARAM_DisplayType;

HI_S32 HI_PARAM_GetSnsSeqCnt(HI_VOID);

HI_S32 HI_PARAM_GetSnsSeqDesc(HI_S32 index, HI_CHAR* buff, HI_U32 buffLen);

HI_S32 HI_PARAM_GetMediaCfg(HI_S32 snsIdx, PARAM_DisplayType dispType, MEDIA_Cfg* mediaCfg);


/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /** End of __HI_PRODUCT_MEDIA_H__ */

