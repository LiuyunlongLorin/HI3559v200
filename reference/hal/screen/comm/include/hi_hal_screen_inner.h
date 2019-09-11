/**
* @file    hi_hal_screen_inner.h
* @brief   product hal screen inner struct
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/
#ifndef __HI_HAL_SCREEN_INNER_H__
#define __HI_HAL_SCREEN_INNER_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_SCREEN */
/** @{ */  /** <!-- [HAL_SCREEN] */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "HALSCREEN"

/* check screen index */
#define HAL_SCREEN_CHECK_IDX(enIdx) \
    do { \
        if (!HI_APPCOMM_CHECK_RANGE(enIdx, HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_IDX_BUTT - 1)){ \
            MLOGE("Invalid ScreenIdx[%d]\n", enIdx); \
            return HI_HAL_EINVAL;  \
        }   \
    } while(0)

/** screen state */
typedef struct tagHAL_SCREEN_STATE_S
{
    HI_HAL_SCREEN_IDX_E   enScreenIndex;
    HI_HAL_SCREEN_STATE_E enState;
} HAL_SCREEN_STATE_S;

typedef struct tagHAL_SCREEN_NUMBER_S
{
    HI_HAL_SCREEN_IDX_E enScreenIndex;
    HI_U32 u32Number;
} HAL_SCREEN_NUMBER_S;

/** screen context */
typedef struct tagHAL_SCREEN_CTX_S
{
    HI_HAL_SCREEN_OBJ_S stScreenObj;
    HI_BOOL bRegister;
    HI_BOOL bInit;
} HAL_SCREEN_CTX_S;

/** @}*/  /** <!-- ==== HAL_SCREEN End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_SCREEN_INNER_H__*/

