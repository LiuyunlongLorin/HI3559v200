/**
 * @file    hi_player.h
 * @brief   player interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#ifndef __HI_PLAYER_H__
#define __HI_PLAYER_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HI_PLAYER_Deinit(HI_VOID);

HI_S32 HI_PLAYER_Init(HI_VOID);

HI_S32 HI_PLAYER_RegAdec(HI_VOID);

HI_S32 HI_PLAYER_UnRegAdec(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of __HI_PLAYER_H__ */

