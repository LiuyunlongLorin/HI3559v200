/**
 * @file      hi_uvc_gadget.h
 * @brief     uvc gadget interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#ifndef __HI_UVC_GADGET_H__
#define __HI_UVC_GADGET_H__

#include "hi_uvc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     USB */
/** @{ */  /** <!-- [USB] */


/**
 * @brief     uvc gadget init, set device capabilities
 * @param[in] pstDevCaps : device capabilities
 * @param[in] u32MaxFrameSize : maximum frame size
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_GADGET_Init(const HI_UVC_DEVICE_CAP_S *pstDevCaps, HI_U32 u32MaxFrameSize);

/**
 * @brief     open uvc device
 * @param[in] pazDevPath : device path
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_GADGET_DeviceOpen(const HI_CHAR *pazDevPath);

/**
 * @brief     close uvc device
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_GADGET_DeviceClose(HI_VOID);

/**
 * @brief     check device events
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_GADGET_DeviceCheck(HI_VOID);


/** @}*/  /** <!-- ==== USB End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_UVC_GADGET_H__ */

