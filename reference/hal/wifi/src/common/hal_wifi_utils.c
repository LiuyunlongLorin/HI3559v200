/**
* @file    hal_wifi_utils.c
* @brief   hal wifi utils implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef __LITEOS__
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#else
#include <liteos/if.h>
#endif
#include <net/ethernet.h>
#include "hi_hal_wifi_inner.h"
#include "hi_type.h"
#include "hi_appcomm_log.h"
#include "hi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */
#define HAL_WIFI_HAL_PROC "/proc/net/dev"

HI_S32 HAL_WIFI_HAL_GetInterface(const HI_CHAR* pszIfname)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_CHAR szBuff[1024];
    FILE *pDevicefd;
    HI_CHAR *pszBegin, *pszEnd;
    HI_CHAR szIfName[HAL_WIFI_IFNAMSIZ] = {0,};
    HI_S32 s32Len;

    if (pszIfname == NULL)
    {
        MLOGE("inparm pszIfname is null,fail\n");
        return HI_FAILURE;
    }

    /** STA and AP mode - 'wlan0'*/
    snprintf(szIfName, sizeof(szIfName),"%s",pszIfname);
    s32Len = strlen(szIfName);

    pDevicefd = fopen(HAL_WIFI_HAL_PROC, "r");
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


HI_S32 HAL_WIFI_UTILS_Ifconfig(const HI_CHAR* pszIfname, HI_S32 s32Up)
{
    struct ifreq stIfr;
    HI_S32 s32fd;

    /* open socket to kernel */
    if ((s32fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        MLOGE("cannot open socket %s", strerror(errno));
        return HI_FAILURE;
    }

    memset(&stIfr, 0, sizeof(struct ifreq));
    strncpy(stIfr.ifr_name, pszIfname, HAL_WIFI_IFNAMSIZ-1);

    if (ioctl(s32fd, SIOCGIFFLAGS, &stIfr) < 0)
    {
        MLOGE(" %s failed!\n", __FUNCTION__);
        close(s32fd);
        return HI_FAILURE;
    }
    if (s32Up)
    {
        stIfr.ifr_flags = stIfr.ifr_flags | IFF_UP;
    }
    else
    {
        stIfr.ifr_flags = stIfr.ifr_flags & (~IFF_UP);
    }

    if (ioctl(s32fd, SIOCSIFFLAGS, &stIfr) < 0)
    {
        MLOGE("  ioctl failed!");
        close(s32fd);
        return HI_FAILURE;
    }
    close(s32fd);
    return HI_SUCCESS;
}


HI_S32 HAL_WIFI_UTILS_SetIp(const HI_CHAR* pszIfname, const HI_CHAR* pszIp)
{
    struct sockaddr_in *pstAddr;
    struct ifreq stIfr;
    HI_S32 s32fd;

    /* open socket to kernel */
    if ((s32fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        MLOGE("cannot open socket %s", strerror(errno));
        return HI_FAILURE;
    }

    memset(&stIfr, 0, sizeof(struct ifreq));
    strncpy(stIfr.ifr_name, pszIfname, HAL_WIFI_IFNAMSIZ-1);

    pstAddr = (struct sockaddr_in *)&stIfr.ifr_addr;
    pstAddr->sin_family = AF_INET;
    pstAddr->sin_addr.s_addr = inet_addr(pszIp);

    if (ioctl(s32fd, SIOCSIFADDR, &stIfr) < 0)
    {
        MLOGE(" ioctl failed!");
        close(s32fd);
        return HI_FAILURE;
    }
    close(s32fd);
    return HI_SUCCESS;
}

/**set fd with FD_CLOEXEC,in exec process,fd cannot been used */
HI_S32 HAL_WIFI_UTILS_AllFDClosexec()
{
    DIR *pDir = NULL;
    HI_S32 s32fd,s32Value;
    if(!(pDir = opendir("/proc/self/fd/")))
    {
        MLOGE("[%s]: open fd dir faild !", __FUNCTION__);
        return HI_FAILURE;
    }
    struct dirent *pDirent = NULL;
    while((pDirent = readdir(pDir)) != NULL){
        if(strncmp(pDirent->d_name, ".", 1) == 0) continue;
        s32fd = atoi(pDirent->d_name);
        if(s32fd >= 3)
        {
            s32Value = fcntl(s32fd,F_GETFD);
            s32Value |= FD_CLOEXEC;
            fcntl(s32fd,F_SETFD,s32Value);
        }
    }
    closedir(pDir);
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */