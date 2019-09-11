/**
* @file    hi_ahdmng.h
* @brief   product ahd struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/10
* @version   1.0

*/

#ifndef __HI_AHDMNG_H__
#define __HI_AHDMNG_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AHDMNG */
/** @{ */  /** <!-- [AHDMNG] */

/** macro define */
#define HI_AHDMNG_MAX_CAM_NUM 2
#define HI_AHDMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_EINVAL)   /**<parm error*/
#define HI_AHDMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_EINTER)        /**<intern error*/
#define HI_AHDMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_ENOINIT)  /**< no initialize*/
#define HI_AHDMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_EINITIALIZED) /**< already initialized */
#define HI_AHDMNG_EREGISTEREVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define HI_AHDMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_AHDMNG,HI_ERRNO_CUSTOM_BOTTOM+1)            /**<thread creat or join error*/

typedef enum hiEVENT_AHDMNG_E
{
    HI_EVENT_AHDMNG_PLUG_STATUS = HI_APPCOMM_EVENT_ID(HI_APP_MOD_AHDMNG, 0), /**<plug in or plug out event*/
    HI_EVENT_AHDMNG_BUIT
} HI_EVENT_AHDMNG_E;

typedef enum hiAHDMNG_PLUG_E
{
    HI_AHDMNG_PLUG_IN = 0,
    HI_AHDMNG_PLUG_OUT,
    HI_AHDMNG_PLUG_BUTT,
} HI_AHDMNG_PLUG_E;

typedef struct hiAHDMNG_CAM_CFG_S
{
    HI_HANDLE CamID;
    HI_U32 u32ChnNum;
} HI_AHDMNG_CAM_CFG_S;

typedef struct hiAHDMNG_CFG_S
{
    HI_U32 u32Cnt;
    HI_AHDMNG_CAM_CFG_S astCamCfg[HI_AHDMNG_MAX_CAM_NUM];
} HI_AHDMNG_CFG_S;


/**
* @brief    register ahdmng event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
*/
HI_S32 HI_AHDMNG_RegisterEvent(HI_VOID);

/**
* @brief    ahd manage initialization, create ahd plug in or plug out event check task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
*/
HI_S32 HI_AHDMNG_Init(HI_AHDMNG_CFG_S* pstCfg);

/**
* @brief    ahd manage deinitialization, destroy ahd plug in or plug out event check task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/04/11
*/
HI_S32 HI_AHDMNG_Deinit(HI_VOID);
/** @}*/  /** <!-- ==== AHDMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef AHDMNG */

