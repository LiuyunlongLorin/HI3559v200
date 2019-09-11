/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_aenc.h
 * @brief   aenc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_AENC_H__
#define __HI_MAPI_AENC_H__

#include "hi_mapi_aenc_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AENC */
/** @{ */ /** <!-- [AENC] */

/* Interface */

/**
 * @brief    aenc init
 * @param[in] AencHdl
 * @param[in] pstAencAttr
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_Init(HI_HANDLE AencHdl, const HI_MAPI_AENC_ATTR_S *pstAencAttr);

/**
 * @brief    aenc deinit
 * @param[in] AencHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_Deinit(HI_HANDLE AencHdl);

/**
 * @brief    aenc start
 * @param[in] AencHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_Start(HI_HANDLE AencHdl);

/**
 * @brief    aenc stop
 * @param[in] AencHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_Stop(HI_HANDLE AencHdl);

/**
 * @brief    aenc register callback function
 * @param[in] AencHdl
 * @param[in] pstAencCB
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_RegisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB);

/**
 * @brief    aenc unregister callback function
 * @param[in] AencHdl
 * @param[in] pstAencCB
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_UnregisterCallback(HI_HANDLE AencHdl, const HI_MAPI_AENC_CALLBACK_S *pstAencCB);

/**
 * @brief    aenc bind acap
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @param[in] AencHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_BindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl);

/**
 * @brief    aenc unbind acap
 * @param[in]AcapHdl
 * @param[in] AcapChnHdl
 * @param[in] AencHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_UnbindACap(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, HI_HANDLE AencHdl);

/**
 * @brief    aenc register audio encoder
 * @param[in] enAudioFormat
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat);

/**
 * @brief    aenc unregister audio encoder
 * @param[in] enAudioFormat
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_UnregisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_E enAudioFormat);

/**
 * @brief    send raw audio frame to encode
 * @param[in] AencHdl
 * @param[in] pstFrm
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_AENC_SendFrame(HI_HANDLE AencHdl, const AUDIO_FRAME_S *pstFrm);

/** @} */ /** <!-- ==== AENC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AENC_H__ */
