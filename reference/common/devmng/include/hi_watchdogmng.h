/**
* @file    hi_watchdogmng.h
* @brief   product watchdogmng struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#ifndef _HI_WATCHDOGMNG_H
#define _HI_WATCHDOGMNG_H

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup    WATCHDOGMNG */
/** @{ */  /** <!-- [WATCHDOGMNG] */

/**
* @brief    create watchdog task
* @param[in] s32Time_s:interval time
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_WATCHDOGMNG_Init(HI_S32 s32Time_s);

/**
* @brief    ledmng deinitialization, destroy led light task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_WATCHDOGMNG_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== KEYMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _HI_WATCHDOGMNG_H */

