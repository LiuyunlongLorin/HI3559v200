/**
* @file    hi_hal_key.h
* @brief   product hal key interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version

*/
#ifndef __HI_HAL_KEY_H__
#define __HI_HAL_KEY_H__
#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_KEY */
/** @{ */  /** <!-- [HAL_KEY] */

/* @brief key status enum*/
typedef enum hiHAL_KEY_STATE_E
{
    HI_HAL_KEY_STATE_DOWN = 0,/**<key down state*/
    HI_HAL_KEY_STATE_UP, /**<key up state*/
    HI_HAL_KEY_STATE_BUIT
} HI_HAL_KEY_STATE_E;


/* @brief key index enum*/
typedef enum hiHAL_KEY_IDX_E
{
    HI_HAL_KEY_IDX_0 = 0, /**<key index 0*/
    HI_HAL_KEY_IDX_1,
    HI_HAL_KEY_IDX_2,
    HI_HAL_KEY_IDX_3,
    HI_HAL_KEY_IDX_BUIT
} HI_HAL_KEY_IDX_E;


/**
* @brief    hal key initialization, open gpio device and mmap CRG regist address
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_KEY_Init(HI_VOID);


/**
* @brief    get hal key state
* @param[in] enKeyIdx: key index
* @param[out] penKeyState:key state
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_KEY_GetState(HI_HAL_KEY_IDX_E enKeyIdx, HI_HAL_KEY_STATE_E* penKeyState);


/**
* @brief    hal key deinitialization, close gpio device and ummap CRG regist address
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_KEY_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== HAL_KEY End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_KEY_H__*/
