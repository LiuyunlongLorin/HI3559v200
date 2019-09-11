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
#if defined(HI3556AV100) || defined(HI3519AV100)
#ifdef BOARD_DEMB
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

#elif defined HI3559AV100
#ifdef BOARD_DEMB

    himd(0x180C0034, &u32Vaule);
    u32Vaule &= ~(0x2);
    s32Ret |= himm(0x180C0034,u32Vaule);/**PWR_EN1 off*/
    sleep(0.1);
    u32Vaule |= 0x2;
    s32Ret |= himm(0x180C0034,u32Vaule);/**PWR_EN1 on*/
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
    HI_U32 u32Vaule = 0;
    if(HI_TRUE == bPinOutInit)
    {
        MLOGD("hal_system_preinit already init\n");
        return;
    }
    MLOGD("hal_system_preinit init register\n");
#ifdef HI3559
    MLOGD("\n");
#ifdef BOARD_DEMB
    /** 3.3V==>1.8V*/
    himm(0x1203000c, 0xac);

    /** PWR_EN1_IO_CTRL*/
    himm(0x1209802c, 0x17);
    himm(0x12098034, 0x10101020);/**<BIT5:pwr_en1 connect wifi REG_ON*/

    /** SDIO1-MUX*/
    himm(0x120400d0, 0x1);
    himm(0x120400d4, 0x1);
    himm(0x120400d8, 0x1);
    himm(0x120400dc, 0x1);
    himm(0x120400e0, 0x1);
    himm(0x120400e4, 0x1);
    himm(0x120400e8, 0x1);
    himm(0x120400ec, 0x1);
    himm(0x120400f0, 0x1);

    /** OOB-MUX*/
    himm(0x12040038, 0x0);
#else
    MLOGE("please point board type,fail\n\n");
#endif

#elif defined HI3559AV100
    MLOGD("\n");
#ifdef BOARD_DEMB
    himm(0x1f001084,0x1505);
    himm(0x1f0010a0,0x1505);
    himm(0x1f0010ac,0x1505);
    himm(0x1f0010b0,0x1505);
    himm(0x1f0010b8,0x1505);
    himm(0x1f0010bc,0x1505);

    himm(0x180C0058,0x5a5aabcd);
    himm(0x180C0020,0x000001bf);

    himd(0x180C0034, &u32Vaule);
    u32Vaule &= ~(0x2);
    himm(0x180C0034,u32Vaule);/**PWR_EN1 off*/
    sleep(0.1);
    u32Vaule |= 0x2;
    himm(0x180C0034,u32Vaule);/**PWR_EN1 on*/
#else
    MLOGE("please point board type,fail\n\n");
#endif

#elif defined(HI3556AV100) || defined(HI3519AV100)
#ifdef BOARD_DEMB
    /** SDIO1-MUX*/
    himm(0x047E0084,0x00001491);
    himm(0x047E0088,0x00001591);
    himm(0x047E008C,0x00001591);
    himm(0x047E0090,0x00001591);
    himm(0x047E0094,0x00001591);
    himm(0x047E0098,0x00001591);

    /** PWR_EN1_IO_CTRL*/
    himm(0x04590058,0x5A5AABCD);
    himm(0x04590020,0x000000bf);

    himd(0x04590034, &u32Vaule);
    u32Vaule &= ~(0x2);
    himm(0x04590034,u32Vaule);/**PWR_EN1 off*/
    sleep(0.1);
    u32Vaule |= 0x2;
    himm(0x04590034,u32Vaule);/**PWR_EN1 on*/

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
#ifdef HI3559
    MLOGD("HI3559\n");
    s32Ret = HI_insmod("/komod/bcmdhd.ko","firmware_path=/lib/firmware/fw_bcm43455c0_ag_apsta.bin nvram_path=/lib/firmware/nvram_ap6255.txt  dhd_oob_gpio_base=0x12142000 dhd_oob_gpio_num=3 dhd_oob_irq_num=75  sdio_slot=1 poweren1=1");
#elif defined HI3559AV100
    MLOGD("HI3559AV100\n");
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/bcmdhd.ko","firmware_path=/app/firmware/fw_bcm43455c0_ag_apsta.bin  nvram_path=/app/firmware/nvram_ap6255.txt dhd_oob_num=59 sdio_slot=2");
#elif defined (HI3556AV100) || defined(HI3519AV100)
        MLOGD("HI3556AV100 || HI3519AV100\n");
        s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/bcmdhd.ko","firmware_path=/app/firmware/fw_bcm43455c0_ag_apsta.bin nvram_path=/app/firmware/nvram_ap6255.txt dhd_oob_num=11 sdio_slot=1");
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
    HI_CHAR *szWbuf = NULL;
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
