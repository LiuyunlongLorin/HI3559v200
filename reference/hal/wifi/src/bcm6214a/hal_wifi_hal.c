/**
* @file    hal_wifi_hal.c
* @brief   hal wifi hal implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "hi_hal_wifi_inner.h"
#include "hi_math.h"

#include "hi_type.h"
#include "hi_appcomm_util.h"
#include "hi_appcomm_log.h"
#include "hi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

HI_S32 HAL_WIFI_HAL_PowerOnReset(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Vaule = 0;
#ifdef HI3556AV100
#ifdef BOARD_ACTIONCAM_REFB
    himd(0x04590034, &u32Vaule);
    u32Vaule &= ~(0x2);
    s32Ret |= himm(0x04590034,u32Vaule);/**PWR_EN1 off*/
    sleep(0.1);
    u32Vaule |= 0x2;
    s32Ret |= himm(0x04590034,u32Vaule);/**PWR_EN1 on*/
    sleep(0.1);
#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");

#endif
        return s32Ret;
}

HI_VOID HAL_WIFI_HAL_SysPreInit(HI_VOID)
{
    static HI_BOOL bPinOutInit = HI_FALSE;
    if(HI_TRUE == bPinOutInit)
    {
        MLOGD("hal_system_preinit already init\n");
        return;
    }
    MLOGD("hal_system_preinit init register\n");

#ifdef HI3556AV100
#ifdef BOARD_ACTIONCAM_REFB
    MLOGD("\n");
    /** PWR_EN1_IO_CTRL*/
    himm(0x04590058,0x5A5AABCD);
    sleep(0.1);
    himm(0x04590020,0x000000bf);
    sleep(0.1);
    HI_U32 u32Vaule = 0;
    himd(0x04590034, &u32Vaule);
    u32Vaule &= ~(0x2);
    himm(0x04590034,u32Vaule);/**PWR_EN1 off*/
    sleep(0.5);
    u32Vaule |= 0x2;
    himm(0x04590034,u32Vaule);/**PWR_EN1 on*/
    sleep(0.1);
    /** SDIO1-MUX*/
    himm(0x047E0084,0x00001491);
    sleep(0.1);
    himm(0x047E0088,0x00001591);
    sleep(0.1);
    himm(0x047E008C,0x00001591);
    sleep(0.1);
    himm(0x047E0090,0x00001591);
    sleep(0.1);
    himm(0x047E0094,0x00001591);
    sleep(0.1);
    himm(0x047E0098,0x00001591);
    sleep(0.1);
#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");
#endif
}


HI_S32 HAL_WIFI_HAL_LoadAPDriver(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef HI3556AV100

#ifdef BOARD_ACTIONCAM_REFB
    MLOGD("HI3556AV100 BOARD_ACTIONCAM_REFB\n");
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/bcmdhd.ko","firmware_path=/app/firmware/fw_bcm43438a1_apsta.bin nvram_path=/app/firmware/nvram_ap6214a.txt dhd_oob_num=23 sdio_slot=1");
#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");
#endif
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod ap: failed, errno(%d)\n", errno);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


HI_S32 HAL_WIFI_HAL_LoadSTADriver(HI_VOID)
{
    MLOGE("insmod sta mode driver not support,failed\n");
    return HI_FAILURE;
}

HI_S32 HAL_WIFI_HAL_RemoveDriver(HI_VOID)
{
    HI_S32 s32Ret = 0;

    s32Ret = HI_rmmod(HI_APPFS_KOMOD_PATH"/bcmdhd.ko");
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod ap: failed, errno(%d)\n", errno);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 HAL_WIFI_HAL_UpdateApConfig(HI_CHAR *ifname,  const HI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const HI_CHAR *pszConfigFile)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32fd;
    HI_CHAR* szWbuf = NULL;
    /** open configure file, if not exist, create it */
    s32fd = open(pszConfigFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if(s32fd < 0)
    {
        MLOGE("WiFi: Cann't open configure file '%s',errno(%d)", pszConfigFile,errno);
        return HI_FAILURE;
    }

    asprintf(&szWbuf, "interface=%s\n"
                    "driver=%s\n"
                    "ctrl_interface=/dev/wifi/hostapd\n"
                    "ssid=%s\n"
                    "channel=%d\n"
                    "ignore_broadcast_ssid=%d\n"
                    "hw_mode=g\n"
                    "ieee80211n=1\n"
                    "#ht_capab=[SHORT-GI-20][SHORT-GI-40][HT20]\n",
                    ifname, "nl80211", pstApCfg->stCfg.szWiFiSSID, pstApCfg->s32Channel,
                    pstApCfg->bHideSSID? 1 : 0);

    if (write(s32fd, szWbuf, strlen(szWbuf)) < 0) {
        MLOGE("WiFi: Cann't write configuration to '%s',errno(%d)\n", pszConfigFile,errno);
        s32Ret = HI_FAILURE;
    }
    close(s32fd);
    free(szWbuf);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
