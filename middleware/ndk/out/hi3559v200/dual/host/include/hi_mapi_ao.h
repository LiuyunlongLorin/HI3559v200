/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_ao.h
 * @brief   ao module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_AO_H__
#define __HI_MAPI_AO_H__

#include "hi_mapi_ao_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AO */
/** @{ */ /** <!-- [AO] */

/* Interface */
/**
 * @brief    ao init
 * @param[in] AoHdl
 * @param[in] pstAoAttr
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Init(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S *pstAoAttr);

/**
 * @brief    ao deinit
 * @param[in] AoHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Deinit(HI_HANDLE AoHdl);

/**
 * @brief    ao start
 * @param[in] AoHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Start(HI_HANDLE AoHdl);

/**
 * @brief    ao stop
 * @param[in] AoHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Stop(HI_HANDLE AoHdl);

/**
 * @brief    ao set volume gain
 * @param[in] AoHdl
 * @param[in] s32AudioGain
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_SetVolume(HI_HANDLE AoHdl, HI_S32 s32AudioGain);

/**
 * @brief    ao get volume gain
 * @param[in] AoHdl
 * @param[in] s32AudioGain
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_GetVolume(HI_HANDLE AoHdl, HI_S32 *s32AudioGain);

/**
 * @brief    ao mute
 * @param[in] AoHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Mute(HI_HANDLE AoHdl);

/**
 * @brief    ao unmute
 * @param[in] AoHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_Unmute(HI_HANDLE AoHdl);

/**
 * @brief    send pcm raw audio frame to output
 * @param[in] AoHdl
 * @param[in] pstAudioFrame
 * @param[in] u32Timeout
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_SendFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout);

/**
 * @brief    send system pcm raw audio frame to output
 * @param[in] AoHdl
 * @param[in] pstAudioFrame
 * @param[in] u32Timeout
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AO_SendSysFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout);

/** @} */ /** <!-- ==== AO ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AO_H__ */
