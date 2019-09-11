/**
* @file    hi_ledmng.h
* @brief   product ledmng struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#ifndef _HI_LEDMNG_H
#define _HI_LEDMNG_H

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup    LEDMNG */
/** @{ */  /** <!-- [LEDMNG] */

/** macro define */
#define HI_LEDMNG_LED_MAXNUM       (2)                    /**<max led numer in board*/

#define HI_LEDMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_LEDMNG,HI_EINVAL)               /**<parm invlid*/
#define HI_LEDMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_LEDMNG,HI_EINTER)               /**<intern error*/
#define HI_LEDMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_LEDMNG,HI_ENOINIT)              /**< no initialize*/
#define HI_LEDMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_LEDMNG,HI_EINITIALIZED)         /**< already initialized */
#define HI_LEDMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_LEDMNG,HI_ERRNO_CUSTOM_BOTTOM)  /**<thread creat or join error*/


/** led index enum */
typedef enum hiLEDMNG_LED_IDX_E
{
    HI_LEDMNG_LED_IDX_0 = 0,
    HI_LEDMNG_LED_IDX_1,
    HI_LEDMNG_LED_IDX_BUTT,
} HI_LEDMNG_LED_IDX_E;

/** led status enum */
typedef enum hiLEDMNG_LED_STATUS_E
{
    HI_LEDMNG_LED_STATUS_ON = 0,   /**<led light on  */
    HI_LEDMNG_LED_STATUS_OFF,      /**<led light off  */
    HI_LEDMNG_LED_STATUS_FLASH,    /**<led light flash in certain time */
    HI_LEDMNG_LED_STATUS_BUTT      /**<BUTT */
} HI_LEDMNG_LED_STATUS_E;

/** ledmng configure */
typedef struct hiLEDMNG_LED_STATUS_S
{
    HI_LEDMNG_LED_STATUS_E enLedStatus;
    HI_S32 s32LightOnTime_us;           /**< The flash interval time when in falsh, The waited time when in On or Off*/
    HI_S32 s32LightOffTime_us;           /**< The flash interval time when in falsh, The waited time when in On or Off*/
} HI_LEDMNG_LED_STATUS_S;


/** ledmng configure */
typedef struct hiLEDMNG_CFG_S
{
    HI_LEDMNG_LED_STATUS_S stLedmngInitialStatus;
} HI_LEDMNG_CFG_S;

/**
* @brief    ledmng initialization, create led light task
* @param[in] enLedIdx:ledIdx
* @param[in] pstCfg: ledmng configure, including intial led status configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_LEDMNG_Init(HI_LEDMNG_LED_IDX_E enLedIdx, const HI_LEDMNG_CFG_S* pstCfg);

/**
* @brief    ledmng setstatus, set off / on or flash in some time
* @param[in] enLedIdx:ledIdx
* @param[in] pstLedStatus:change led status
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_LEDMNG_SetStatus(HI_LEDMNG_LED_IDX_E enLedIdx, const HI_LEDMNG_LED_STATUS_S* pstLedStatus);

/**
* @brief    ledmng deinitialization, destroy led light task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_LEDMNG_Deinit(HI_LEDMNG_LED_IDX_E enLedIdx);

/** @}*/  /** <!-- ==== KEYMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _HI_KEYMNG_H */

