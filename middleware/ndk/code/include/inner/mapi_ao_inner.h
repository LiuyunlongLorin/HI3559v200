/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_ao_inner.h
 * @brief   ao module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_AO_INNER_H__
#define __MAPI_AO_INNER_H__

#include "hi_mapi_comm_define.h"
#include "hi_comm_aio.h"
#include "hi_mapi_log.h"
#include "mapi_ao_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AO */
/** @{ */ /** <!-- [AO] */

#define MAPI_AO_SYS_CHN (HI_MAPI_AO_CHN_MAX_NUM - 1) /** ao system chn id */

#define CHECK_MAPI_AO_NULL_PTR(pstPtr)                                                                     \
    do {                                                                                                   \
        if (NULL == (pstPtr)) {                                                                            \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Null pointer!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__); \
            return HI_MAPI_AO_ENULLPTR;                                                                    \
        }                                                                                                  \
    } while (0)

#define CHECK_MAPI_AO_HANDLE_RANGE(AoHdl)                                                                    \
    do {                                                                                                     \
        if (AoHdl >= HI_MAPI_AO_DEV_MAX_NUM) {                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Invalid Handle(%d)!FUNCTION %s,LINE %d!\n", AoHdl, __FUNCTION__, \
                           __LINE__);                                                                        \
            return HI_MAPI_AO_EINVALIDHDL;                                                                   \
        }                                                                                                    \
    } while (0)

#define CHECK_MAPI_AO_INIT(bInit)                                                                              \
    do {                                                                                                       \
        if (HI_FALSE == bInit) {                                                                               \
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__); \
            return HI_MAPI_AO_ENOTINITED;                                                                      \
        }                                                                                                      \
    } while (0)

/* Structure */
/** ao current state */
typedef enum tagMAPI_AO_STATE_E {
    MAPI_AO_UNINITED = 0,
    MAPI_AO_STOPED, /**< inited but not start */
    MAPI_AO_STARTED,
    MAPI_AO_BUTT
} MAPI_AO_STATE_E;

/** extend attr which not apply to ao pub */
typedef struct tagMAPI_AO_ATTR_EX_S {
    HI_MAPI_AUDIO_SOUND_MODE_E enSoundModeEx; /**< different to stereo and mono */
    AUDIO_SAMPLE_RATE_E enInSampleRate;       /**< ao input resample for resample */
} MAPI_AO_ATTR_EX_S;

/** contain all info about ao */
typedef struct tagMAPI_AO_CONTEXT_S {
    AIO_ATTR_S stAoAttr;          /**< attr for ao pub */
    MAPI_AO_ATTR_EX_S stAoAttrEx; /**< Extra Info compatible to upper module attr which is difference to MPP attr */
    MAPI_AO_STATE_E enAoState;
} MAPI_AO_CONTEXT_S;

HI_S32 MAPI_AO_Init(HI_VOID);
HI_S32 MAPI_AO_Deinit(HI_VOID);

/** @} */ /** <!-- ==== AO ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AO_INNER_H__ */
