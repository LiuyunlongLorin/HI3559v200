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
#ifdef HI3559AV100
#ifdef BOARD_PANOCAM_REFB
    s32Ret = himm(0x12147100,0x0);/**GPIO7_6 off*/
    sleep(0.1);
    s32Ret |= himm(0x12147100,0x40);/**GPIO7_6 on*/
#else
        MLOGE("please point board type,fail\n\n");
#endif
#else
        MLOGE("please point chip type,fail\n\n");
#endif
    return s32Ret;
}

HI_S32 HAL_WIFI_HAL_GetInterface(const HI_CHAR *pszDevFile)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_CHAR szBuff[1024];
    FILE *pDevicefd;
    HI_CHAR *pszBegin, *pszEnd;
    HI_CHAR szIfName[HAL_WIFI_IFNAMSIZ] = {0,};
    HI_S32 s32Len;

    if (pszDevFile == NULL)
    {
        MLOGE("inparm szDevFile is null,fail\n");
        return HI_FAILURE;
    }

    /** STA and AP mode - 'wlan0'*/
    snprintf(szIfName, sizeof(szIfName),"%s",HAL_WIFI_INTERFACE_NAME);
    s32Len = strlen(szIfName);

    pDevicefd = fopen(pszDevFile, "r");
    if(pDevicefd != NULL)
    {
        /** Eat 2 lines of header */
        fgets(szBuff, sizeof(szBuff), pDevicefd);
        fgets(szBuff, sizeof(szBuff), pDevicefd);

        /** Read each device line */
        while(fgets(szBuff, sizeof(szBuff), pDevicefd))
        {
            /** Skip empty or almost empty lines. It seems that in some
             * cases fgets return a line with only a newline. */
            if((szBuff[0] == '\0') || (szBuff[1] == '\0'))
            {
                continue;
            }
            pszBegin = szBuff;
            while(*pszBegin == ' ')
            {
                pszBegin++;
            }
            pszEnd = strstr(pszBegin, ": ");
            if((pszEnd == NULL) || (((pszEnd - pszBegin) + 1) > (HAL_WIFI_IFNAMSIZ + 1)))/**< not found or pattern not suitable */
            {
                continue;
            }
            if (strncmp(pszBegin, szIfName, s32Len) != 0)
            {
                continue;
            }
            s32Ret = HI_SUCCESS;
            break;
        }

    }
    if(NULL != pDevicefd)
    {
        fclose(pDevicefd);
    }
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
#ifdef HI3559AV100
    MLOGD("HI3559AV100\n");
#ifdef BOARD_PANOCAM_REFB
/**sdio2 */
    himm(0x01F0010B0,0x00001405);
    himm(0x01F0010B8,0x00001405);
    himm(0x01F0010BC,0x00001405);
    himm(0x01F0010A0,0x00001405);
    himm(0x01F001084,0x00001405);
    himm(0x01F0010AC,0x00001405);
/**power on*/
    himm(0x01F0010C0,0x00001500);/**GPIO7_6*/
    himm(0x12147400,0x40);/**GPIO7_6 out direction*/

    himm(0x12147100,0x40);/**GPIO7_6 on*/

#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");
#endif
    bPinOutInit = HI_TRUE;
}


HI_S32 HAL_WIFI_HAL_LoadAPDriver(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;
#ifdef HI3559AV100
        MLOGD("HI3559AV100\n");
#ifdef BOARD_PANOCAM_REFB
        MLOGD("BOARD_PANOCAM_REFB\n");
        s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/bcmdhd.ko","firmware_path=/app/firmware/fw_bcm4356a2_ag.bin  nvram_path=/app/firmware/nvram_ap6356s.txt dhd_oob_num=48 sdio_slot=1");
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
