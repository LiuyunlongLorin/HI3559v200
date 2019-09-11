/**
* @file    hi_accmng.h
* @brief   product accmng struct and interface
*
* Copyright (c) 2019 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2019/04/10
* @version   1.0

*/
#ifndef __HI_ACCMNG_H__
#define __HI_ACCMNG_H__

#include "hi_type.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     ACCMNG */
/** @{ */  /** <!-- [ACCMNG] */
/** macro define */
#define HI_ACCMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_EINVAL)    /**<parm invlid*/
#define HI_ACCMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_EINTER)          /**<intern error*/
#define HI_ACCMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_ENOINIT)  /**< no initialize*/
#define HI_ACCMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_EINITIALIZED) /**< already initialized */
#define HI_ACCMNG_EREGISTEREVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define HI_ACCMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_ACCMNG,HI_ERRNO_CUSTOM_BOTTOM+1)           /**<thread creat or join error*/


typedef enum hiEVENT_ACCMNG {
    HI_EVENT_ACCMNG_POWEROFF = HI_APPCOMM_EVENT_ID(HI_APP_MOD_ACCMNG, 0),   /**<acc poweroff event*/
    HI_EVENT_ACCMNG_BUIT
}
HI_EVENT_ACCMNG;

HI_S32 HI_ACCMNG_RegisterEvent(HI_VOID);

HI_S32 HI_ACCMNG_Init(HI_VOID);

HI_S32 HI_ACCMNG_Deinit(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HI_ACCMNG_H__*/
