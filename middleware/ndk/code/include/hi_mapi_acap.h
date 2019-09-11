/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_acap.h
 * @brief   acap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __HI_MAPI_ACAP_H__
#define __HI_MAPI_ACAP_H__

#include "hi_mapi_acap_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ACAP */
/** @{ */ /** <!-- [ACAP] */

/* Interface */

/**
 * @brief    init acap with attr
 * @param[in] AcapHdl
 * @param[in] pstACapAttr
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Init(HI_HANDLE AcapHdl, const HI_MAPI_ACAP_ATTR_S *pstACapAttr);

/**
 * @brief    deinit acap
 * @param[in] AcapHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Deinit(HI_HANDLE AcapHdl);

/**
 * @brief    start acap
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Start(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl);

/**
 * @brief    stop acap
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Stop(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl);

/**
 * @brief    enable audio quality enhance
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_EnableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl);

/**
 * @brief    disableVqe audio quality enhance
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_DisableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl);

/**
 * @brief    set acap volume
 * @param[in] AcapHdl
 * @param[in] s32AudioGain
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_SetVolume(HI_HANDLE AcapHdl, HI_S32 s32AudioGain);

/**
 * @brief    get acap volume
 * @param[in] AcapHdl
 * @param[out] ps32AudioGain
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_GetVolume(HI_HANDLE AcapHdl, HI_S32 *ps32AudioGain);

/**
 * @brief    mute acap
 * @param[in] AcapHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Mute(HI_HANDLE AcapHdl);

/**
 * @brief    unmute acap
 * @param[in] AcapHdl
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_Unmute(HI_HANDLE AcapHdl);

/**
 * @brief    get pcm raw or resample audio frame
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @param[out] pstFrm
 * @param[out] pstAecFrm
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_GetFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, AUDIO_FRAME_S *pstFrm,
                             AEC_FRAME_S *pstAecFrm);

/**
 * @brief    release pcm raw audio frame
 * @param[in] AcapHdl
 * @param[in] AcapChnHdl
 * @param[in] pstFrm
 * @param[in] pstAecFrm
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam NDK Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_MAPI_ACAP_ReleaseFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, const AUDIO_FRAME_S *pstFrm,
                                 const AEC_FRAME_S *pstAecFrm);

/** @} */ /** <!-- ==== ACAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_ACAP_H_ */
