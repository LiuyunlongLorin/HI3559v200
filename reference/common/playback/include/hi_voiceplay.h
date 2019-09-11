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
#ifndef __HI_VOICEPLAY_H__
#define __HI_VOICEPLAY_H__

#include "hi_liteplayer.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VOICEPLAY */
/** @{ */  /** <!-- [VOICEPLAY] */
#define HI_VOICE_MAX_SEGMENT_CNT (5)

typedef struct hiVOICEPLAY_VOICETABLE_S
{
    HI_U32 u32VoiceIdx;
    HI_CHAR aszFilePath[HI_APPCOMM_MAX_PATH_LEN];
} HI_VOICEPLAY_VOICETABLE_S;

/** voiceplay configuration */
typedef struct hiVOICEPLAY_CFG_S
{
    HI_U32 u32MaxVoiceCnt;
    HI_VOICEPLAY_VOICETABLE_S* pstVoiceTab;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
} HI_VOICEPLAY_CFG_S;

typedef struct hiVOICEPLAY_VOICE_S
{
    HI_U32 u32VoiceCnt;
    HI_U32 au32VoiceIdx[HI_VOICE_MAX_SEGMENT_CNT];
    HI_BOOL bDroppable;
} HI_VOICEPLAY_VOICE_S;

/** function interface */
/**
 * @brief    init voiceplay.
 * @param[in] pstCfg:playback configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_VOICEPLAY_Init(const HI_VOICEPLAY_CFG_S* pstCfg);

/**
 * @brief    deinit voiceplay.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_VOICEPLAY_Deinit(HI_VOID);

/**
 * @brief    push voice to play queue.
 * @param[in] pstVoice:voice attr.
 * @param[in] u32Timeout_ms:wait timeout if no droppable.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_VOICEPLAY_Push(const HI_VOICEPLAY_VOICE_S* pstVoice, HI_U32 u32Timeout_ms);

/** @}*/  /** <!-- ==== VOICEPLAY End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of __HI_VOICEPLAY_H__ */

