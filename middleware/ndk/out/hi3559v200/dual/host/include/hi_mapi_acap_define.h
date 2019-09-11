/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_acap_define.h
 * @brief   acap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_ACAP_DEFINE_H__
#define __HI_MAPI_ACAP_DEFINE_H__

#include "hi_comm_aio.h"
#include "hi_mapi_comm_define.h"
#include "acodec.h"
#include "hi_mapi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ACAP */
/** @{ */ /** <!-- [ACAP] */

/* Error code
    Invalid handle error for acap */

#define HI_MAPI_ACAP_EINVALIDHDL HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_DEVID)
/** Null pointer error for acap */
#define HI_MAPI_ACAP_ENULLPTR    HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
/** media sys not inited error for acap */
#define HI_MAPI_ACAP_ENOINITED   HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
/** current state error for acap */
#define HI_MAPI_ACAP_ESTATEERR   HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
/** illegal parameter error for acap */
#define HI_MAPI_ACAP_EILLPARAM   HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
/** invalid fd error for acap */
#define HI_MAPI_ACAP_EINVALIDFD  HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)
/** operate failed  for acap */
#define HI_MAPI_ACAP_EOPERATE    HI_MAPI_DEF_ERR(HI_MAPI_MOD_ACAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_OPERATE_FAIL)

/* Structure */
/** Acap init attr */
typedef struct hiMAPI_ACAP_ATTR_S {
    AUDIO_SAMPLE_RATE_E enSampleRate;       /**< Audio sampling rate, in Hz */
    AUDIO_BIT_WIDTH_E enBitwidth;           /**< Audio bit width */
    HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode; /**< Number of audio channels */
    AIO_MODE_E enWorkmode;                  /**< Audio workmode , eg I2S master or slave */
    HI_U32 u32PtNumPerFrm;                  /**< sampling points every frame */
    ACODEC_MIXER_E enMixerMicMode;          /**< mixer mic input mode for acodec */
    AUDIO_SAMPLE_RATE_E enResampleRate;     /**< for resampler frame */
    AIO_I2STYPE_E enI2sType;                /* i2s type */
} HI_MAPI_ACAP_ATTR_S;

/** @} */ /** <!-- ==== ACAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_ACAP_H_ */
