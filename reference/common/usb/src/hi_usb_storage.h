/**
 * @file      hi_usb_storage.h
 * @brief     usb storage mode interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#ifndef __HI_USB_STORAGE_H__
#define __HI_USB_STORAGE_H__

#include <sys/types.h>
#include "hi_usb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     USB */
/** @{ */  /** <!-- [USB] */

/**
 * @brief     usb storage mode init, load module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 USB_STORAGE_Init(HI_VOID);

/**
 * @brief     usb storage mode deinit, unload module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 USB_STORAGE_Deinit(HI_VOID);

/**
 * @brief     prepare storage device
 * @param[in] pstCfg : storage configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 USB_STORAGE_PrepareDev(const HI_USB_STORAGE_CFG_S* pstCfg);

/** @}*/  /** <!-- ==== USB End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_USB_STORAGE_H__ */

