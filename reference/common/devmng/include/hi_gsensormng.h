/**
* @file    hi_gsensormng.h
* @brief   product gsensormng struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/10
* @version   1.0

*/

#ifndef __HI_GSENSORMNG_H__
#define __HI_GSENSORMNG_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     GSENSORMNG */
/** @{ */  /** <!-- [GSENSORMNG] */

/** macro define */
#define HI_GSENSORMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_EINVAL)   /**<parm error*/
#define HI_GSENSORMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_EINTER)        /**<intern error*/
#define HI_GSENSORMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_ENOINIT)  /**< no initialize*/
#define HI_GSENSORMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_EINITIALIZED) /**< already initialized */
#define HI_GSENSORMNG_EREGISTEREVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define HI_GSENSORMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_GSENSORMNG,HI_ERRNO_CUSTOM_BOTTOM+1)            /**<thread creat or join error*/

typedef enum hiEVENT_GSENSORMNG_E
{
    HI_EVENT_GSENSORMNG_COLLISION = HI_APPCOMM_EVENT_ID(HI_APP_MOD_GSENSORMNG, 0), /**<collision occur event*/
    HI_EVENT_GSENSORMNG_BUIT
} HI_EVENT_GSENSORMNG_E;

/* @brief sensitity enum*/
typedef enum hiGSENSORMNG_SENSITITY_E
{
    HI_GSENSORMNG_SENSITITY_OFF = 0,/**<gsensor off*/
    HI_GSENSORMNG_SENSITITY_LOW, /**<low sensitity*/
    HI_GSENSORMNG_SENSITITY_MIDDLE, /**<middle sensitity*/
    HI_GSENSORMNG_SENSITITY_HIGH, /**<high sensitity*/
    HI_GSENSORMNG_SENSITITY_BUIT
} HI_GSENSORMNG_SENSITITY_E;

typedef struct hiGSENSORMNG_VALUE_S
{
    HI_S32 s32XDirValue; /**<x direction value,unit acceleration of gravity g*/
    HI_S32 s32YDirValue; /**<y direction value,unit acceleration of gravity g*/
    HI_S32 s32ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} HI_GSENSORMNG_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct hiGSENSORMNG_ATTR_S
{
    HI_U32 u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit kps*/
} HI_GSENSORMNG_ATTR_S;

typedef struct hiGSENSORMNG_CFG_S
{
    HI_GSENSORMNG_SENSITITY_E enSensitity;
    HI_GSENSORMNG_ATTR_S stAttr;
} HI_GSENSORMNG_CFG_S;



/**
* @brief    register gsensormng event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
*/
HI_S32 HI_GSENSORMNG_RegisterEvent(HI_VOID);

/**
* @brief    gsensor manage initialization, create gsensor event check task
* @param[in] pstCfg: gsensormng configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
*/
HI_S32 HI_GSENSORMNG_Init(const HI_GSENSORMNG_CFG_S* pstCfg);

/**
* @brief    dynamic set Collision threshold value
* @param[in] pstThreshold: Collision threshold value
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_GSENSORMNG_SetSensitity(HI_GSENSORMNG_SENSITITY_E enSensitity);

/**
* @brief    get Collision  work attribute
* @param[out] pstAttr: Collision work attribute
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_GSENSORMNG_GetAttr(HI_GSENSORMNG_ATTR_S* pstAttr);
/**
* @brief    dynamic set Collision  work attribute
* @param[in] pstAttr: Collision work attribute
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/03/05
*/
HI_S32 HI_GSENSORMNG_SetAttr(const HI_GSENSORMNG_ATTR_S* pstAttr);
/**
* @brief   gsensor manage deinitialization, include destroy check task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_GSENSORMNG_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== GSENSORMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef GSENSORMNG */

