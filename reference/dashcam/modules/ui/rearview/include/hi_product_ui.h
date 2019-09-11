/**
 * @file      hi_product_ui.h
 * @brief     ui public interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
 * @version   1.0

 */

#ifndef __HI_PRODUCT_UI_H__
#define __HI_PRODUCT_UI_H__


#include "hi_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/** \addtogroup     UI */
/** @{ */  /** <!-- [UI] */

typedef enum hiEVENT_UI_E
{
    HI_EVENT_UI_TOUCH = HI_APPCOMM_EVENT_ID(HI_APP_MOD_UI, 0),
    HI_EVENT_UI_BUTT
} HI_EVENT_UI_E;

/**
 * @brief      init and start UI
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
 */
HI_S32 HI_PDT_UI_Init(HI_VOID);

/**
 * @brief    stop and deinit UI
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
 */
HI_S32 HI_PDT_UI_Deinit(HI_VOID);


/** @}*/  /** <!-- ==== UI End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
