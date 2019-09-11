/**
* @file    hi_hal_gpio_inner.h
* @brief   product inner hal gpio interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#ifndef __HI_HAL_GPIO_INNER__H__
#define __HI_HAL_GPIO_INNER__H__
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_GPIO */
/** @{ */  /** <!-- [HAL_GPIO] */

/* @brief gpio dir enum*/
typedef enum tagHAL_GPIO_DIR_E
{
    HAL_GPIO_DIR_READ = 0,/**<gpio read*/
    HAL_GPIO_DIR_WRITE, /**<gpio write*/
    HAL_GPIO_DIR_BUIT
} HAL_GPIO_DIR_E;

/**
* @brief    hal gpio initialization, open gpio device and return fd;every layer user must call this interface first,then use gpio function
* @param[out] ps32fd
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_GPIO_Init(HI_S32* ps32fd);

/**
* @brief   set gpio read or write direction
* @param[in] s32fd
* @param[in] u32GrpNum
* @param[in] u32BitNum
* @param[in] u32DirVal
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_GPIO_SetDir(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HAL_GPIO_DIR_E enDirVal);
/**
* @brief    get bit value of gpio
* @param[in] s32fd
* @param[in] u32GrpNum
* @param[in] u32BitNum
* @param[out] pu32BitVal
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_GPIO_GetBitVal(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HI_U32* pu32BitVal);
/**
* @brief    set bit value of gpio
* @param[in] s32fd
* @param[in] u32GrpNum
* @param[in] u32BitNum
* @param[in] u32BitVal
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_GPIO_SetBitVal(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HI_U32 u32BitVal);

/**
* @brief    hal gpio deinitialization,close fd;any user who has called HAL_GPIO_Init interface must call this function when exit
* @param[in] s32fd
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/19
*/
HI_S32 HAL_GPIO_Deinit(HI_S32 s32fd);


/** @}*/  /** <!-- ==== HAL_GPIO End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPIO_INNER__H__*/

