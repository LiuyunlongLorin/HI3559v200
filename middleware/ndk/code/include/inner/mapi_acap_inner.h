/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap_inner.h
 * @brief   acap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_ACAP_INNER_H__
#define __MAPI_ACAP_INNER_H__

#include <pthread.h>
#include "hi_mapi_acap_define.h"
#include "hi_mapi_log.h"
#include "hi_resampler_api.h"
#include "mapi_acap_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ACAP */
/** @{ */ /** <!-- [ACAP] */

#define CHECK_MAPI_ACAP_NULL_PTR(ptr)                                \
    do {                                                             \
        if (NULL == ptr) {                                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acap NULL pointer\n"); \
            return HI_MAPI_ACAP_ENULLPTR;                            \
        }                                                            \
    } while (0)
#define CHECK_MAPI_ACAP_HANDLE_RANGE(handle)                                \
    do {                                                                    \
        if (handle >= HI_MAPI_ACAP_DEV_MAX_NUM) {                           \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP handle out of range\n"); \
            return HI_MAPI_ACAP_EINVALIDHDL;                                \
        }                                                                   \
    } while (0)
#define CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(handle)                                \
    do {                                                                        \
        if (handle >= HI_MAPI_ACAP_CHN_MAX_NUM) {                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP Chn handle out of range\n"); \
            return HI_MAPI_ACAP_EINVALIDHDL;                                    \
        }                                                                       \
    } while (0)
#define CHECK_MAPI_ACAP_CHECK_INIT(state)                                                                        \
    do {                                                                                                         \
        if (!state) {                                                                                            \
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n"); \
            return HI_MAPI_ACAP_ENOINITED;                                                                       \
        }                                                                                                        \
    } while (0)

/** Structure */
/** acap which handle current state */
typedef enum tagMAPI_ACAP_STATE_E {
    MAPI_ACAP_UNINITED = 0, /**< not inited */
    MAPI_ACAP_STOPED,       /**< inited but not start */
    MAPI_ACAP_STARTED,      /**< start */
    MAPI_ACAP_BUTT
} MAPI_ACAP_STATE_E;

/** extent acap attr which not apply to ai pub */
typedef struct tagMAPI_ACAP_ATTR_EX_S {
    HI_MAPI_AUDIO_SOUND_MODE_E enSoundModeEx; /**< different to stereo and mono */
    ACODEC_MIXER_E enMixerMicMode;            /**< mixer mic input mode for inner acodec */
    AUDIO_SAMPLE_RATE_E enResampleRate;       /**< for get resampler frame */
} MAPI_ACAP_ATTR_EX_S;

/** resample info */
typedef struct tagMAPI_ACAP_RESAMPLE_INFO_S {
    HI_VOID *pAcapResampler; /**< resampler instance point */
    HI_U32 u32SavedFrmLen;   /**< save origin frame len which get from mpi when resample process */
    HI_U64 u64ResMMZPhyAddr; /**< save allocate resample process mmz buffer phy address */
    HI_VOID *pResMMZVirAddr; /**< save allocate resample process mmz buffer vitual address */
} MAPI_ACAP_RESAMPLE_INFO_S;

/** acap all info context */
typedef struct tagMAPI_ACAP_CONTEXT_S {
    MAPI_ACAP_STATE_E enAcapState;                   /**< acap current state */
    HI_BOOL bAcapChnstart[HI_MAPI_ACAP_CHN_MAX_NUM]; /**< acap  chn current state */
    AIO_ATTR_S stAcapAttr;                           /**< Attr set to acap pub */
    MAPI_ACAP_ATTR_EX_S stAcapAttrEx;                /**< Extra attr which not set to acap pub */
    MAPI_ACAP_RESAMPLE_INFO_S stAcapResmpInfo;       /**< Resample info */
} MAPI_ACAP_CONTEXT_S;

HI_S32 MAPI_ACAP_GetChn(HI_HANDLE AcapHdl, HI_S32 *pAiChn);
HI_S32 MAPI_ACAP_Init(HI_VOID);
HI_S32 MAPI_ACAP_Deinit(HI_VOID);

/** @} */ /** <!-- ==== ACAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_ACAP_H_ */
