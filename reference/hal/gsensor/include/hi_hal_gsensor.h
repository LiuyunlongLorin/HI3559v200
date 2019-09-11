/**
* @file    hi_hal_gsensor.h
* @brief   product hal gsensor interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
* @version   1.0

*/

#ifndef __HI_HAL_GSENSOR_H__
#define __HI_HAL_GSENSOR_H__
#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_GSENSOR */
/** @{ */  /** <!-- [HAL_GSENSOR] */

/* @brief sensitity enum*/
typedef enum hiHAL_GSENSOR_SENSITITY_E
{
    HI_HAL_GSENSOR_SENSITITY_OFF = 0,/**<gsensor off*/
    HI_HAL_GSENSOR_SENSITITY_LOW, /**<low sensitity*/
    HI_HAL_GSENSOR_SENSITITY_MIDDLE, /**<middle sensitity*/
    HI_HAL_GSENSOR_SENSITITY_HIGH, /**<high sensitity*/
    HI_HAL_GSENSOR_SENSITITY_BUIT
} HI_HAL_GSENSOR_SENSITITY_E;

/* @brief gesensor value*/
typedef struct hiHAL_GSENSOR_VALUE_S
{
    HI_S32 s32XDirValue; /**<x direction value,unit acceleration of gravity g*/
    HI_S32 s32YDirValue; /**<y direction value,unit acceleration of gravity g*/
    HI_S32 s32ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} HI_HAL_GSENSOR_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct hiHAL_GSENSOR_ATTR_S
{
    HI_U32 u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit kps*/
} HI_HAL_GSENSOR_ATTR_S;

typedef HI_S32 (*HI_HAL_GSENSOR_ON_COLLISION_PFN)(HI_VOID* pvPrivData);

/** hal gsensor Configure */
typedef struct hiHAL_GSENSOR_CFG_S
{
    HI_HAL_GSENSOR_SENSITITY_E enSensitity;
    HI_HAL_GSENSOR_ATTR_S stAttr;
} HI_HAL_GSENSOR_CFG_S;

/**
* @brief    hal gsensor initialization, insmod gesnor driver and init gsensor threshold
* @param[in] stCfg: include Collision  work attribute¡¢threshold value  and collision callback function
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_Init(const HI_HAL_GSENSOR_CFG_S* pstCfg);


/**
* @brief    dynamic set Collision threshold value
* @param[in] enSensitity: Collision enSensitity level
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_SetSensitity(HI_HAL_GSENSOR_SENSITITY_E enSensitity);

/**
* @brief    get Collision  work attribute
* @param[out] pstAttr: Collision work attribute
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_GetAttr(HI_HAL_GSENSOR_ATTR_S* pstAttr);
/**
* @brief    dynamic set Collision  work attribute
* @param[in] pstAttr: Collision work attribute
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_SetAttr(const HI_HAL_GSENSOR_ATTR_S* pstAttr);

/**
* @brief    dynamic get current sample value
* @param[out] pstCurValue: current sample value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_GetCurValue(HI_HAL_GSENSOR_VALUE_S* pstCurValue);

/**
* @brief    dynamic get collision occur or not
* @param[out] pbOnCollison: current sample value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/10
*/
HI_S32 HI_HAL_GSENSOR_GetCollisionStatus(HI_BOOL * pbOnCollison);

/**
* @brief    hal gsensor deinitialization, rmmod driver
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_HAL_GSENSOR_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== HAL_GSENSOR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GSENSOR_H__*/
