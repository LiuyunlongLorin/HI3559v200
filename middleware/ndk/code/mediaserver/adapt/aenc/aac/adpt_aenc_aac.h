/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file      adpt_aenc_aac.h
* @brief     aenc aac encoder adapter head file.
* @author    HiMobileCam ndk develop team
* @date  2019-3-26
*/

#ifndef __ADPT_AENC_AAC_H__
#define __ADPT_AENC_AAC_H__

#include "hi_mapi_aenc_adpt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AENC ADPT */
/** @{ */ /** <!-- [AENC ADPT] */

typedef struct tagAENC_AAC_ENCODER_S {
    AAC_ENCODER_S *pstAACState;
    HI_MAPI_AENC_ATTR_AAC_S stAACAttr; /**< AAC Attr */
} AENC_AAC_ENCODER_S;

/**
 * @brief    get aenc aac encoder struct
 * @param[out] pstAacEncoder
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 ADPT_AENC_GetAacEncoder(AENC_ENCODER_S *pstAacEncoder);

/** @} */ /** <!-- ==== AENC ADPT End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
