/**
* @file    hi_hal_wifi.c
* @brief   hal wifi implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "hi_hal_wifi_inner.h"

#include "hi_appcomm_log.h"
#include "hi_appcomm.h"
#include "hi_hal_wifi.h"
#include "hi_hal_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** macro define */
#define SSID_MAX_LEN (31)
#define SSID_MIN_LEN (1)
#define PWD_MAX_LEN (63)
#define PWD_MIN_LEN (8)

#define HAL_WIFI_HOSTAPD_CONFIG_FILE ("/dev/wifi/hostapd/hostapd.conf")

static HI_BOOL s_bHALWIFIInitState = HI_FALSE;
static HI_HAL_WIFI_MODE_E s_enHALWIFIMode = HI_HAL_WIFI_MODE_BUIT;
static HI_BOOL s_bHALWIFIStartState = HI_FALSE;
static pthread_mutex_t sWifiMutex = PTHREAD_MUTEX_INITIALIZER;

static HI_S32 HAL_WIFI_ApValidChck(const HI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    /** cahnnel check */
    if((pstApCfg->s32Channel < 1) ||(pstApCfg->s32Channel > 165))
    {
        MLOGE("[Errot]s32Channel(%d),invalid\n",pstApCfg->s32Channel);
        return HI_FAILURE;
    }
    if ((strlen(pstApCfg->stCfg.szWiFiSSID) < SSID_MIN_LEN) ||
            (strlen(pstApCfg->stCfg.szWiFiSSID) > SSID_MAX_LEN)) { /** < ssid check */
        MLOGE("[Errot]szWiFiSSID len (%lu),invalid\n",(HI_UL)strlen(pstApCfg->stCfg.szWiFiSSID));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}
static HI_S32 HAL_WIFI_StaValidChck(const HI_HAL_WIFI_STAMODE_CFG_S* pstStaCfg)
{
    MLOGW("[Errot]wifi STA mode parm validcheck not support");
    return HI_FAILURE;
}

static HI_S32 HAL_WIFI_InParmValidChck(const HI_HAL_WIFI_CFG_S* pstCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    switch(pstCfg->enMode)
    {
        case HI_HAL_WIFI_MODE_STA:
            s32Ret = HAL_WIFI_StaValidChck(&pstCfg->unCfg.stStaCfg);
            break;
        case HI_HAL_WIFI_MODE_AP:
            s32Ret = HAL_WIFI_ApValidChck(&pstCfg->unCfg.stApCfg);
            break;
        default:
            s32Ret = HI_FAILURE;
            MLOGE("[Errot]enMode(%d),invalid\n",pstCfg->enMode);
            break;

    }
    return s32Ret;
}

HI_S32 HI_HAL_WIFI_Init(HI_HAL_WIFI_MODE_E enMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MUTEX_LOCK(sWifiMutex);
    if (HI_TRUE == s_bHALWIFIInitState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi init already\n");
        return HI_HAL_EINITIALIZED;
    }
    HAL_WIFI_HAL_SysPreInit();

    switch (enMode)
    {
        case HI_HAL_WIFI_MODE_STA:
            MLOGE("wifi sta not support at present!\n");
            s32Ret = HI_FAILURE;
            break;
        case HI_HAL_WIFI_MODE_AP:
            s32Ret = HAL_WIFI_HAL_LoadAPDriver();
            break;
        default:
            MLOGE("enMode(%d) error!\n",enMode);
            s32Ret = HI_HAL_EINVAL;
    }
    if (HI_SUCCESS == s32Ret)
    {
        s_enHALWIFIMode = enMode;
        s_bHALWIFIInitState = HI_TRUE;
    }
    else
    {
        s32Ret = HI_HAL_EINTER;
    }
    HI_MUTEX_UNLOCK(sWifiMutex);
    return s32Ret;
}

HI_S32 HI_HAL_WIFI_CheckeCfgValid(const HI_HAL_WIFI_CFG_S* pstCfg,HI_BOOL* pCfgValid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if ((NULL == pstCfg) || (NULL == pCfgValid))
    {
        MLOGE("[Errot]pstCfg or pCfgValid is null,error\n");
        return HI_HAL_EINVAL;
    }

    *pCfgValid = HI_FALSE;

    s32Ret = HAL_WIFI_InParmValidChck(pstCfg);
    if (HI_SUCCESS == s32Ret)
    {
        *pCfgValid = HI_TRUE;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_WIFI_Start(const HI_HAL_WIFI_CFG_S* pstCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MUTEX_LOCK(sWifiMutex);
    if (HI_FALSE == s_bHALWIFIInitState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi not init\n");
        return HI_HAL_ENOINIT;
    }
    if (HI_TRUE == s_bHALWIFIStartState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi init already\n");
        return HI_HAL_ESTARTED;
    }
    if(pstCfg->enMode != s_enHALWIFIMode)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]start mode(%d) and init mode(%d) must be same,error\n",s_enHALWIFIMode,pstCfg->enMode);
        return HI_HAL_EINVAL;
    }
    s32Ret = HAL_WIFI_InParmValidChck(pstCfg);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("wifi inparm valid error!\n");
        return HI_HAL_EINVAL;
    }
    switch (s_enHALWIFIMode)
    {
        case HI_HAL_WIFI_MODE_STA:
            MLOGE("wifi sta not support at present!\n");
            s32Ret = HI_FAILURE;
            break;
        case HI_HAL_WIFI_MODE_AP:
            s32Ret = HAL_WIFI_AP_Start(HAL_WIFI_INTERFACE_NAME,&pstCfg->unCfg.stApCfg);
            if(HI_SUCCESS == s32Ret)
            {
                s_bHALWIFIStartState = HI_TRUE;
            }
            else
            {
                s32Ret = HI_HAL_EINTER;
            }
            break;
        default:
            MLOGE("enMode(%d) error!\n",s_enHALWIFIMode);
            s32Ret = HI_HAL_EINVAL;
    }
    HI_MUTEX_UNLOCK(sWifiMutex);
    return s32Ret;
}

HI_S32 HI_HAL_WIFI_GetStartedStatus(HI_BOOL* pbEnable)
{
    if (NULL == pbEnable)
    {
        MLOGE("[Errot]pbEnable is null,error\n");
        return HI_HAL_EINVAL;
    }
    HI_MUTEX_LOCK(sWifiMutex);
    *pbEnable = s_bHALWIFIStartState;
    HI_MUTEX_UNLOCK(sWifiMutex);
    return HI_SUCCESS;
}

HI_S32 HI_HAL_WIFI_Scan(HI_VOID)
{
    MLOGE("[Errot]wifi sta mode not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_GetScanResult(HI_HAL_WIFI_SCAN_RESULTSET_S* pstResultSet)
{
    MLOGE("[Errot]wifi sta mode not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_Connect(const HI_HAL_WIFI_STAMODE_SENIOR_CFG_S* pstCfg)
{
    MLOGE("[Errot]wifi sta mode not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_GetConnectStatus(HI_BOOL* bConnected)
{
    MLOGE("[Errot]wifi  get connect status not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_Disconnect(HI_VOID)
{
    MLOGE("[Errot]wifi  dis connect not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_Suspend(HI_VOID)
{
    MLOGE("[Errot]wifi  suspend not support at present\n");
    return HI_FAILURE;
}


HI_S32 HI_HAL_WIFI_WakeUp(HI_VOID)
{
    MLOGE("[Errot]wifi  WakeUp not support at present\n");
    return HI_FAILURE;
}



HI_S32 HI_HAL_WIFI_Stop(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MUTEX_LOCK(sWifiMutex);
    if (HI_FALSE == s_bHALWIFIInitState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi not init,can not stop\n");
        return HI_HAL_ENOINIT;
    }
    if (HI_FALSE == s_bHALWIFIStartState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi stop already\n");
        return HI_HAL_ENOSTART;
    }
    s32Ret = HAL_WIFI_AP_Stop();
    if(HI_SUCCESS == s32Ret)
    {
        s_bHALWIFIStartState = HI_FALSE;
    }
    else
    {
        s32Ret = HI_HAL_EINTER;
    }
    HI_MUTEX_UNLOCK(sWifiMutex);
    return s32Ret;
}

HI_S32 HI_HAL_WIFI_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MUTEX_LOCK(sWifiMutex);
    if (HI_FALSE == s_bHALWIFIInitState)
    {
        HI_MUTEX_UNLOCK(sWifiMutex);
        MLOGE("[Errot]wifi deinit already\n");
        return HI_HAL_ENOINIT;
    }
    s32Ret = HAL_WIFI_HAL_RemoveDriver();
    if(HI_SUCCESS == s32Ret)
    {
        s_enHALWIFIMode = HI_HAL_WIFI_MODE_BUIT;
        s_bHALWIFIInitState = HI_FALSE;
    }
    else
    {
        s32Ret = HI_HAL_EINTER;
    }
    HI_MUTEX_UNLOCK(sWifiMutex);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

