/**
 * @file      hi_product_init_peripheral.h
 * @brief     init interface in HuaweiLite
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#ifndef __HI_PRODUCT_INIT_PERIPHERAL_H__
#define __HI_PRODUCT_INIT_PERIPHERAL_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     INIT */
/** @{ */  /** <!-- [INIT] */

/**
 * @brief     peripheral preinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_PERIPHERAL_PreInit(HI_VOID);

/**
 * @brief     peripheral init
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_PERIPHERAL_Init(HI_VOID);

/**
 * @brief     peripheral postinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 HI_PDT_INIT_PERIPHERAL_PostInit(HI_VOID);


/** @}*/  /** <!-- ==== INIT End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_INIT_PERIPHERAL_H__ */

