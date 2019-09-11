/**
* @file    hi_product_netctrl.h
* @brief   netctrl module struct definition and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/1/15
* @version   1.0

*/

#include "hi_appcomm.h"

#ifndef __HI_PRODUCT_NETCTRL_H__
#define __HI_PRODUCT_NETCTRL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     NETCTRL */
/** @{ */  /** <!-- [NETCTRL] */

#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "NetCtrl"

/** error code define */
#define HI_PDT_NETCTRL_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_NETCTRL, HI_EINVAL)       /**<Invalid argument */
#define HI_PDT_NETCTRL_EUNSUPPORT     HI_APPCOMM_ERR_ID(HI_APP_MOD_NETCTRL, HI_EPAERM)       /**<Unsupport */

/** Product NetCtrl message enum */
typedef enum hiEVENT_NETCTRL_E {
    HI_EVENT_NETCTRL_CLIENT_CONNECTED = HI_APPCOMM_EVENT_ID(HI_APP_MOD_NETCTRL, 0),
    HI_EVENT_NETCTRL_CLIENT_DISCONNECTED,
    HI_EVENT_NETCTRL_BUTT
} HI_EVENT_NETCTRL_E;


/**
* @brief     the netctrl init interface
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 HI_PDT_NETCTRL_Init(HI_VOID);

/**
* @brief     the netctrl register event
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 HI_PDT_NETCTRL_RegisterEvent(HI_VOID);

/**
* @brief     the netctrl Client Connection Status
* @param[in] Private data,Fill in NULL
* @return value  0: No client connection  1:client connected
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 HI_PDT_NETCTRL_CheckClientConnect(HI_VOID *pvPrivData);

/**
* @brief     the netctrl deinit interface
* @param[in] HI_VOID
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 HI_PDT_NETCTRL_Deinit(HI_VOID);



/** @}*/  /** <!-- ==== NETCTRL End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_PRODUCT_NETCTRL_H__*/

