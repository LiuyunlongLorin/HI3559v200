/**
* @file    hi_hal_common_inner.h
* @brief   product inner hal common interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#ifndef __HI_HAL_COMMON_INNER__H__
#define __HI_HAL_COMMON_INNER__H__

#include "hi_appcomm_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_COMMON */
/** @{ */  /** <!-- [HAL_COMMON] */

typedef enum tagMSG_HAL_E
{
    MSG_HAL_GAUGE_GETLEVEL = HI_APPCOMM_MSG_ID(HI_APP_MOD_HAL, 0),   /**<get gauge level msg*/
    MSG_HAL_SCREEN_INIT,
    MSG_HAL_SCREEN_GETATTR,
    MSG_HAL_SCREEN_GETDISPLAYSTATE,
    MSG_HAL_SCREEN_SETDISPLAYSTATE,
    MSG_HAL_SCREEN_GETBACKLIGHTSTATE,
    MSG_HAL_SCREEN_SETBACKLIGHTSTATE,
    MSG_HAL_SCREEN_GETLUMA,
    MSG_HAL_SCREEN_SETLUMA,
    MSG_HAL_SCREEN_GETSATURATURE,
    MSG_HAL_SCREEN_SETSATURATURE,
    MSG_HAL_SCREEN_GETCONTRAST,
    MSG_HAL_SCREEN_SETCONTRAST,
    MSG_HAL_SCREEN_DEINIT,
    MSG_HAL_SCREEN_BUTT
} MSG_HAL_E;

/** @}*/  /** <!-- ==== HAL_COMMON End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HI_HAL_COMMON_INNER__H__ */

