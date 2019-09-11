/**
* @file    hi_hal_wifi.h
* @brief   product hal wifi struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/
#ifndef __HI_HAL_WIFI_H__
#define __HI_HAL_WIFI_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_WIFI */
/** @{ */  /** <!-- [HAL_WIFI] */

/** macro define */
#define  HI_HAL_WIFI_SSID_LEN (32)
#define  HI_HAL_WIFI_STR_LEN (64)
#define  HI_HAL_WIFI_RESULT_SIZE (64)

/* WiFi mode enum*/
typedef enum hiHAL_WIFI_MODE_E
{
    HI_HAL_WIFI_MODE_STA = 0,/**<STA mode*/
    HI_HAL_WIFI_MODE_AP,/**<AP mode*/
    HI_HAL_WIFI_MODE_BUIT
} HI_HAL_WIFI_MODE_E;

/* WiFi STA workmode*/
typedef enum hiHAL_STA_WIFI_MODE_E
{
    HI_HAL_WIFI_STA_MODE_COMMON = 0,/**<ap broadcast ssid*/
    HI_HAL_WIFI_STA_MODE_SENIOR,/**<ap hide ssid*/
    HI_HAL_WIFI_STA_MODE_BUIT
} HI_HAL_WIFI_STA_MODE_E;



/** wifi ap && sta common configure */
typedef struct hiHAL_WIFI_COMMON_CFG_S
{
    HI_CHAR szWiFiSSID[HI_HAL_WIFI_SSID_LEN]; /**<wifi ssid,aszWiFiSSID len decided by strlen(aszWiFiSSID)*/
} HI_HAL_WIFI_COMMON_CFG_S;


/** wifi ap configure */
typedef struct hiHAL_WIFI_APMODE_CFG_S
{
    HI_BOOL bHideSSID;/**<true:ssid broadcast,false:hide*/
    HI_S32 s32Channel; /**<wifi channel */
    HI_HAL_WIFI_COMMON_CFG_S stCfg;/**<wifi common cfg*/
} HI_HAL_WIFI_APMODE_CFG_S;


/** wifi sta common configure */
typedef struct hiHAL_WIFI_STAMODE_COMMON_CFG_S
{
    HI_HAL_WIFI_COMMON_CFG_S stCfg;
} HI_HAL_WIFI_STAMODE_COMMON_CFG_S;


/** wifi sta common configure */
typedef struct hiHAL_WIFI_STAMODE_SENIOR_CFG_S
{
    HI_HAL_WIFI_COMMON_CFG_S stCfg;
} HI_HAL_WIFI_STAMODE_SENIOR_CFG_S;

/** wifi sta configure */
typedef struct hiHAL_WIFI_STAMODE_CFG_S
{
    HI_HAL_WIFI_STA_MODE_E enStaMode;/**<STA mode*/
    union tagHAL_WIFI_STAMODE_CFG_U
    {
        HI_HAL_WIFI_STAMODE_COMMON_CFG_S stCommonCfg;/**<cfg for common connect ap mode*/
        HI_HAL_WIFI_STAMODE_SENIOR_CFG_S stSeniorCfg;/**<cfg for senior connect ap mode*/
    } unCfg;
} HI_HAL_WIFI_STAMODE_CFG_S;


/** wifi configure */
typedef struct hiHAL_WIFI_CFG_S
{
    HI_HAL_WIFI_MODE_E enMode;/**<wifi mode*/
    union tagHAL_WIFI_CFG_U
    {
        HI_HAL_WIFI_APMODE_CFG_S stApCfg;/**<wifi AP cfg*/
        HI_HAL_WIFI_STAMODE_CFG_S stStaCfg;/**<wifi STA cfg*/
    } unCfg;
} HI_HAL_WIFI_CFG_S;


/* WiFi scan result*/
typedef struct hiHAL_WIFI_SCAN_RESULT_S
{
    HI_CHAR szSSID[HI_HAL_WIFI_SSID_LEN];/**<Hotspot SSID */
    HI_CHAR szBssid[HI_HAL_WIFI_STR_LEN];/**<Hotspot BSSID */
    HI_U32 u32Channel;/**<Hotspot channel*/
    HI_U32 u32Signal;/**<Hotspot signal strength,value[0,100]*/
} HI_HAL_WIFI_SCAN_RESULT_S;

/* WiFi scan result*/
typedef struct hiHAL_WIFI_SCAN_RESULTSET_S
{
    HI_U32 u32Num;/**<Hotspot num*/
    HI_HAL_WIFI_SCAN_RESULT_S astResult[HI_HAL_WIFI_RESULT_SIZE];
} HI_HAL_WIFI_SCAN_RESULTSET_S;



/**
* @brief    hal wifihal initialization, insmod driver
* @param[in] enMode: wifi mode
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Init(HI_HAL_WIFI_MODE_E enMode);

/**
* @brief   wifi cfg valid check
* @param[in] pstCfg: wifi cfg
* @param[out] pCfgValid: wifi cfg valid check result
              HI_TRUE:cfg valid
              HI_FALSE:cfg invalid
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/08/01
*/
HI_S32 HI_HAL_WIFI_CheckeCfgValid(const HI_HAL_WIFI_CFG_S* pstCfg,HI_BOOL* pCfgValid);

/**
* @brief    wifi on
* @param[in] pstCfg: wifi start cfg
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Start(const HI_HAL_WIFI_CFG_S* pstCfg);

/**
* @brief    get wifi started status
* @param[out] pbEnable: HI_TRUE :STARTED, HI_FAILURE:NO STARTED
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_GetStartedStatus(HI_BOOL* pbEnable);

/**
* @brief    wifi scan hostap,only for STA mode
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Scan(HI_VOID);

/**
* @brief    get scan result,only for STA mode
* @param[out] pstResultSet: store scan result
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_GetScanResult(HI_HAL_WIFI_SCAN_RESULTSET_S* pstResultSet);

/**
* @brief    connect ap,only for STA mode
* @param[in] pstCfg: connect ap,cfg from scan
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Connect(const HI_HAL_WIFI_STAMODE_SENIOR_CFG_S* pstCfg);

/**
* @brief    get connect status,only for STA mode
* @param[out] bConnected: HI_TRUE :connected, HI_FAILURE:NO connected
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_GetConnectStatus(HI_BOOL* bConnected);

/**
* @brief    disconnect,only for STA mode
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Disconnect(HI_VOID);



/**
* @brief    wifi module standby
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Suspend(HI_VOID);

/**
* @brief    wifi module wakeup
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_WakeUp(HI_VOID);

/**
* @brief    wifi stop
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Stop(HI_VOID);

/**
* @brief    hal wifi deinitialization, rmmod driver
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_HAL_WIFI_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== HAL_WIFI End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_WIFI_H__*/
