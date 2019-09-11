/**
 * @file    player_adapt.h
 * @brief   player adapt interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#ifndef __PLAYER_ADAPT_H__
#define __PLAYER_ADAPT_H__

#include "hi_liteplayer.h"
#include "hi_appcomm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** player configuration */
typedef struct tagPLAYER_CFG_S
{
    HI_BOOL bEnableVideo;
    HI_BOOL bEnableAudio;
    HI_LITEPLAYER_PARAM_S stPlayerParam;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
    HI_LITEPLAYER_EVENT_FN pfnPlayerCallback;
} PLAYER_CFG_S;

typedef enum tagPLAYER_TYPE_E
{
    PLAYER_TYPE_PLAYBACK  = 0,
    PLAYER_TYPE_VOICEPLAY,
    PLAYER_TYPE_BUTT
} PLAYER_TYPE_E;

HI_S32 PLAYER_Create(PLAYER_TYPE_E enType, const PLAYER_CFG_S* pstPlayerCfg);

HI_S32 PLAYER_Destroy(PLAYER_TYPE_E enType);

HI_VOID* PLAYER_GetHdl(PLAYER_TYPE_E enType);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of __PLAYER_ADAPT_H__ */

