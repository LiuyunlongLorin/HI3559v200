/**
 * @file      hi_product_init_os.h
 * @brief     init interface in HuaweiLite
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#ifndef __HI_PRODUCT_INIT_OS_H__
#define __HI_PRODUCT_INIT_OS_H__

#include "hi_appcomm.h"
#include "hi_timestamp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     INIT */
/** @{ */  /** <!-- [INIT] */

/** redefine product module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "Init"

/**
 * @brief     os preinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_OS_PreInit(HI_VOID);

/**
 * @brief     os init
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_OS_Init(HI_VOID);

/**
 * @brief     os post init
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_OS_PostInit(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_INIT_OS_H__ */

