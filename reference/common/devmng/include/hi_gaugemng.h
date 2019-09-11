/**
* @file    hi_gaugemng.h
* @brief   product gaugemng struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#ifndef __HI_GAUGEMNG_H__
#define __HI_GAUGEMNG_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     GAUGEMNG */
/** @{ */  /** <!-- [GAUGEMNG] */

/** macro define */
#define HI_GAUGEMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_EINVAL)   /**<parm error*/
#define HI_GAUGEMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_EINTER)        /**<intern error*/
#define HI_GAUGEMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_ENOINIT)  /**< no initialize*/
#define HI_GAUGEMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_EINITIALIZED) /**< already initialized */
#define HI_GAUGEMNG_EREGISTEREVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define HI_GAUGEMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_GAUGEMNG,HI_ERRNO_CUSTOM_BOTTOM+1)            /**<thread creat or join error*/



typedef enum hiEVENT_GAUGEMNG_E
{
    HI_EVENT_GAUGEMNG_LEVEL_CHANGE = HI_APPCOMM_EVENT_ID(HI_APP_MOD_GAUGEMNG, 0), /**<refresh current count of electric quantity*/
    HI_EVENT_GAUGEMNG_LEVEL_LOW,        /**<low level , an alarm show*/
    HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW,   /**<ultra low level , power off*/
    HI_EVENT_GAUGEMNG_LEVEL_NORMAL,      /**<after charging,restore normal*/
    HI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE,      /**<after charging,restore normal*/
    HI_EVENT_GAUGEMNG_BUIT
} HI_EVENT_GAUGEMNG_E;


/** gauge mng configure */
typedef struct hiGAUGEMNG_CFG_S
{
    HI_S32 s32LowLevel; /**< in percent */
    HI_S32 s32UltraLowLevel; /**< in percent */
} HI_GAUGEMNG_CFG_S;

/**
* @brief    register gaugemng event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/29
*/
HI_S32 HI_GAUGEMNG_RegisterEvent(HI_VOID);

/**
* @brief    gauge manage initialization, , create gauge event check task
* @param[in] pstCfg: gaugemng configure, including low level and ultra low level
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_GAUGEMNG_Init(const HI_GAUGEMNG_CFG_S* pstCfg);


/**
* @brief   get current battery level
* @param[in] ps32Level: battery level
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_GAUGEMNG_GetBatteryLevel(HI_S32* ps32Level);

/**
* @brief   get current chargestate
* @param[out] pbCharge: charge state, true is charging now
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_GAUGEMNG_GetChargeState(HI_BOOL* pbCharge);

/**
* @brief   gauge manage deinitialization, destroy gauge battery level event check task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_GAUGEMNG_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== GAUGEMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef __HI_GAUGEMNG_H__ */

