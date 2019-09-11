/**
* @file    hi_hal_common.h
* @brief   product extern hal common interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#ifndef __HI_HAL_COMMON_H__
#define __HI_HAL_COMMON_H__

#include "hi_appcomm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_COMMON */
/** @{ */  /** <!-- [HAL_COMMON] */

/** macro define */
#define HI_HAL_EUNKNOW           HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_EUNKNOWN) /* unknow error*/
#define HI_HAL_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_EINTER)   /* intern error*/
#define HI_HAL_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_EINVAL)   /* parm invalid*/
#define HI_HAL_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ENOINIT)  /* no initialize*/
#define HI_HAL_ENOREG            HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ELOST)    /* no registered*/

#define HI_HAL_EREGRED           HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_EEXIST)   /* already registered*/
#define HI_HAL_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_EINITIALIZED)         /* already initialized */
#define HI_HAL_ENOSTART          HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM)  /* no start*/
#define HI_HAL_ESTARTED          HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM+1)/* already started*/
#define HI_HAL_ENOSTOP           HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM+2)/* no stop*/

#define HI_HAL_EGPIO             HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM+3)/* gpio cfg or read error */
#define HI_HAL_EINVOKESYS        HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM+4)/* invoke system function error */
#define HI_HAL_EIPCMSG           HI_APPCOMM_ERR_ID(HI_APP_MOD_HAL,HI_ERRNO_CUSTOM_BOTTOM+5)/* ipcmsg error */

#define HAL_FD_INITIALIZATION_VAL (-1)
#define HAL_MMAP_LENGTH (0x500)

/** @}*/  /** <!-- ==== HAL_COMMON End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HI_HAL_COMMON_H__ */

