/**
* @file    hi_hal_led.h
* @brief   product hal key interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version

*/
#ifndef __HI_HAL_LED_H__
#define __HI_HAL_LED_H__
#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_LED */
/** @{ */  /** <!-- [HAL_LED] */

/* @brief led status enum*/
typedef enum hiHAL_LED_STATE_E
{
    HI_HAL_LED_STATE_OFF = 0,/**<led off state*/
    HI_HAL_LED_STATE_ON, /**<led on state*/
    HI_HAL_LED_STATE_BUIT
} HI_HAL_LED_STATE_E;


/* @brief led index enum*/
typedef enum hiHAL_LED_IDX_E
{
    HI_HAL_LED_IDX_0 = 0, /**<led index 0*/
    HI_HAL_LED_IDX_1,
    HI_HAL_LED_IDX_BUIT
} HI_HAL_LED_IDX_E;


/**
* @brief    hal led initialization
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_LED_Init(HI_HAL_LED_IDX_E enLedIdx);


/**
* @brief    set hal led state
* @param[in] enKeyIdx: key index
* @param[out] penKeyState:key state
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_LED_SetState(HI_HAL_LED_IDX_E enLedIdx, HI_HAL_LED_STATE_E enLedState);


/**
* @brief    hal led deinitialization
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_LED_Deinit(HI_HAL_LED_IDX_E enLedIdx);

/** @}*/  /** <!-- ==== HAL_LED End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_KEY_H__*/
