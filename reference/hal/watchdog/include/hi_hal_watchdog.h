/**
* @file    hi_hal_watchdog.h
* @brief   product hal gsensor interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
* @version   1.0

*/

#ifndef __HI_HAL_WATCHDOG_H__
#define __HI_HAL_WATCHDOG_H__
#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**
* @brief    create watchdog task
* @param[in] s32Time_s: feed dog time, [2, MAX]
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_WATCHDOG_Init(HI_S32 s32Time_s);

/**
* @brief    feed watchdog
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_WATCHDOG_Feed(HI_VOID);


/**
* @brief    destroy watchdog task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_WATCHDOG_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== HAL_WATCHDOG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_WATCHDOG_H__*/
