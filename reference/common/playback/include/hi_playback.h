/**
 * @file    hi_playback.h
 * @brief   playback struct and interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#ifndef __HI_PLAYBACK_H__
#define __HI_PLAYBACK_H__

#include "hi_comm_video.h"
#include "hi_liteplayer.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PLAYBACK */
/** @{ */  /** <!-- [PLAYBACK] */

/** callback function for event */
typedef HI_S32 (*HI_PLAYBACK_EVENT_CALLBACK_FN_PTR)(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pData, HI_VOID* pvUserData);

/** playback configuration */
typedef struct hiPLAYBACK_CFG_S
{
    HI_LITEPLAYER_PARAM_S stPlayerParam;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
    HI_PLAYBACK_EVENT_CALLBACK_FN_PTR pfnEventProc;
    HI_VOID* pvUserData;
} HI_PLAYBACK_CFG_S;

/** function interface */
/**
 * @brief    init playback.
 * @param[in] pstCfg:playback configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Init(const HI_PLAYBACK_CFG_S* pstCfg);

/**
 * @brief    deinit playback.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Deinit(HI_VOID);

/**
 * @brief    get media file information.
 * @param[in] pszFilePath:the file path.
 * @param[out] pstMediaInfo:media information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetMediaInfo(const HI_CHAR* pszFilePath, HI_FORMAT_FILE_INFO_S* pstMediaInfo);

/**
 * @brief    start play media file.
 * @param[in] s32VideoIdx:video stream index.
 * @param[in] s32AudioIdx:audio stream index.
 * @param[in] pstPlaySize:vpss port size in VoutTypeVpss, use default when NULL or w/h is 0.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Play(HI_S32 s32VideoIdx, HI_S32 s32AudioIdx, const SIZE_S* pstPlaySize);

/**
 * @brief    stop player.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Stop(HI_VOID);

/**
 * @brief    pause player.
 * @return 0 success,non-zero error code.
 * @exception    just support pause when the play speed is normal.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Pause(HI_VOID);

/**
 * @brief    resume player.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Resume(HI_VOID);

/**
 * @brief    set play speed.
 * @param[in] enSpeed:play speed
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_E enSpeed);

/**
 * @brief    get play speed.
 * @param[out] penSpeed:play speed
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetSpeed(HI_LITEPLAYER_PLAY_SPEED_E* penSpeed);

/**
 * @brief    seek.
 * @param[in] s64Time_ms:seek time in ms.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Seek(HI_S64 s64Time_ms);

/**
 * @brief    get play states.
 * @param[out] penState:play states
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetPlayState(HI_LITEPLAYER_STATE_E* penState);

/** @}*/  /** <!-- ==== PLAYBACK End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of __HI_PLAYBACK_H__ */

