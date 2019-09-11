/**
* @file    hi_hal_wifi_inner.h
* @brief   product hal wifi inner struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version   1.0

*/
#ifndef __HI_HAL_WIFI_INNER_H__
#define __HI_HAL_WIFI_INNER_H__

#include "hi_type.h"
#include "hi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_WIFI_INNER */
/** @{ */  /** <!-- [HAL_WIFI_INNER] */

/** macro define */
#define HAL_WIFI_IFNAMSIZ (16)
#define HAL_WIFI_INTERFACE_NAME ("wlan0")
#define HAL_WIFI_DEV_DIR  ( "/dev/wifi")
#define HAL_WIFI_KILL_EXECUTE_FILE  ( "/usr/bin/killall")

/** @}*/  /** <!-- ==== HAL_WIFI_INNER End ====*/

/**
* @brief get netcard
* @param[in] pszIfname:netcard name
* @return 0 get netcard success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
*/
HI_S32 HAL_WIFI_HAL_GetInterface(const HI_CHAR* pszIfname);

/**
* @brief ifconfig netcard up or down
* @param[in] pszIfname:netcard name
* @param[in] s32Up:represent up or down
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
*/
HI_S32 HAL_WIFI_UTILS_Ifconfig (const HI_CHAR* pszIfname, HI_S32 s32Up);

/**
* @brief  set ip address to netcard
* @param[in] pszIfname:netcard name
* @param[in] pszIp: ipaddress
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
*/
HI_S32 HAL_WIFI_UTILS_SetIp(const HI_CHAR* pszIfname, const HI_CHAR* pszIp);

/**
* @brief    set fd with FD_CLOEXEC
* @param[in]  None
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_UTILS_AllFDClosexec(HI_VOID);

/**
* @brief    REG ON pinout reset,off->on
* @return   None
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_HAL_PowerOnReset(HI_VOID);
/**
* @brief    pre sysinit,include pinout config
* @return   None
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_VOID HAL_WIFI_HAL_SysPreInit(HI_VOID);

/**
* @brief    load ap driver
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_HAL_LoadAPDriver(HI_VOID);

/**
* @brief    load sta driver
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_HAL_LoadSTADriver(HI_VOID);

/**
* @brief    remove driver
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_HAL_RemoveDriver(HI_VOID);

/**
* @brief    update ap config
* @param[in] ifname: netcard name
* @param[in] stApCfg:configure
* @param[in] pszConfigFile:absolute config file path
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_HAL_UpdateApConfig(HI_CHAR *ifname,  const HI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const HI_CHAR *pszConfigFile);

/**
* @brief start ap mode wifi
* @param[in] ifname: netcard name
* @param[in] stApCfg:configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_AP_Start(HI_CHAR *ifname,const HI_HAL_WIFI_APMODE_CFG_S* pstApCfg);

/**
* @brief  stop ap mode wifi
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/26
*/
HI_S32 HAL_WIFI_AP_Stop(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_WIFI_INNER_H__*/
