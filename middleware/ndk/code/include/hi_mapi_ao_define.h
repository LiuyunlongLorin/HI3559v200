/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_ao_define.h
 * @brief   ao module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_AO_DEFINE_H__
#define __HI_MAPI_AO_DEFINE_H__

#include "hi_mapi_comm_define.h"
#include "hi_comm_aio.h"
#include "hi_mapi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AO */
/** @{ */ /** <!-- [AO] */

/** error code
      invlalid Handle error for AO */
#define HI_MAPI_AO_EINVALIDHDL HI_MAPI_DEF_ERR(HI_MAPI_MOD_AO, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_DEVID)
/* NULL point error for AO */
#define HI_MAPI_AO_ENULLPTR    HI_MAPI_DEF_ERR(HI_MAPI_MOD_AO, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
/* media not inited */
#define HI_MAPI_AO_ENOTINITED  HI_MAPI_DEF_ERR(HI_MAPI_MOD_AO, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
/* state error for AO */
#define HI_MAPI_AO_ESTATEERR   HI_MAPI_DEF_ERR(HI_MAPI_MOD_AO, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
/* at lease one parameter is illagal for AO */
#define HI_MAPI_AO_EILLPARAM   HI_MAPI_DEF_ERR(HI_MAPI_MOD_AO, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)

/* Structure */
/** attr for ao pub */
typedef struct hiMAPI_AO_ATTR_S {
    AUDIO_SAMPLE_RATE_E enSampleRate;       /**< Audio output samplerate, in Hz */
    AUDIO_BIT_WIDTH_E enBitwidth;           /**< Audio bit width */
    HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode; /**< Number of audio channels */
    AIO_MODE_E enWorkmode;                  /**< work mode for ao */
    HI_U32 u32PtNumPerFrm;                  /**< sampling points every frame */
    AUDIO_SAMPLE_RATE_E enInSampleRate;     /**< ao input resample for resample */
    AIO_I2STYPE_E enI2sType;                /* i2s type */
} HI_MAPI_AO_ATTR_S;

/** @} */ /** <!-- ==== AO ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AO_H__ */
