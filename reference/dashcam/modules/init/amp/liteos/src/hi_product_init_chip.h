/**
 * @file      hi_product_init_chip.h
 * @brief     init interface in HuaweiLite
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#ifndef __HI_PRODUCT_INIT_CHIP_H__
#define __HI_PRODUCT_INIT_CHIP_H__

#include <stdio.h>
#include "osal_mmz.h"
#include "stdlib.h"
#include "fcntl.h"
#include "string.h"
#include "hi_type.h"
#include "hi_appcomm_util.h"
#include "hi_module_param.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     INIT */
/** @{ */  /** <!-- [INIT] */

/**
 * @brief     chip preinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_CHIP_PreInit(HI_VOID);

/**
 * @brief     chip init
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_CHIP_Init(HI_VOID);

/**
 * @brief     chip postinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_CHIP_PostInit(HI_VOID);


/** @}*/  /** <!-- ==== INIT End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_INIT_CHIP_H__ */

