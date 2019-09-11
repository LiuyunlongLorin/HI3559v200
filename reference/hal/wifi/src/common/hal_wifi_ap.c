/**
* @file    hal_wifi_ap.c
* @brief   hal wifi ap mode implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __LITEOS__
#include <linux/if.h>
#include <linux/sockios.h>
#else
#include "lwip/netifapi.h"
#include "hostapd_if.h"
#endif
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include "hi_hal_wifi_inner.h"

#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_hal_wifi.h"
#ifndef __UCLIBC__
#include <spawn.h>
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef __LITEOS__

#define HAL_WIFI_HOSTAPD_CONFIG_DIR   ("/dev/wifi/hostapd")
#define HAL_WIFI_HOSTAPD_CONFIG_FILE  ("/dev/wifi/hostapd/hostapd.conf")
#define HAL_WIFI_HOSTAPD_EXECUTE_FILE ("/app/bin/hostapd")
#define HAL_WIFI_ENTROPY_FILE ("/app/bin/entropy.bin")


#define HAL_WIFI_DHCPD_CONFIG_FILE    ("/app/udhcpd.conf")
#define HAL_WIFI_DHCPD_EXECUTE_FILE   ("/usr/sbin/udhcpd")
#define HAL_WIFI_IP ("192.168.0.1")



static HI_S32 HAL_WIFI_AP_UpdateConfig(HI_CHAR *ifname,  const HI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const HI_CHAR *pszConfigFile)
{
    HI_S32 s32Ret = HI_SUCCESS;
    DIR *pDir;

    /** ensure /dev/wifi exist */
    pDir = opendir(HAL_WIFI_DEV_DIR);
    if (!pDir) {
        if (mkdir(HAL_WIFI_DEV_DIR, 0666) < 0)
        {
            MLOGE("WiFi: Create '%s' fail,errno(%d)", HAL_WIFI_DEV_DIR,errno);
            return HI_FAILURE;
        }
    }
    else
    {
        closedir(pDir);
    }
    /** ensure hostapd configure file directory exist */
    pDir = opendir(HAL_WIFI_HOSTAPD_CONFIG_DIR);
    if (!pDir)
    {
        if (mkdir(HAL_WIFI_HOSTAPD_CONFIG_DIR, 0666) < 0)
        {
            MLOGE("WiFi: Create '%s' fail,errno(%d)", HAL_WIFI_HOSTAPD_CONFIG_DIR,errno);
            return HI_FAILURE;
        }
    }
    else
    {
        closedir(pDir);
    }
    s32Ret = HAL_WIFI_HAL_UpdateApConfig(ifname,pstApCfg,pszConfigFile);
    if(HI_SUCCESS != s32Ret )
    {
        MLOGE("hal update config  fail\n");
        return HI_FAILURE;

    }
    if (chmod(pszConfigFile, 0666) < 0)
    {
        MLOGE("WiFi: Failed to change '%s' to 0666 ,errno(%d)\n", pszConfigFile,errno);
        unlink(pszConfigFile);
        s32Ret = HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 HAL_WIFI_AP_HostapdStart()
{
#ifdef POSIX_SPAWN_FUNC
    pid_t pid = 0;
    HI_S32 s32Status = 0;
    HI_S32 s32Ret = 0;
    HI_CHAR* szArgs[] = {HAL_WIFI_HOSTAPD_EXECUTE_FILE,"-B",HAL_WIFI_HOSTAPD_CONFIG_FILE, NULL};
    HI_CHAR* szEnv[]  = {NULL};

    s32Ret = posix_spawn(&pid, szArgs[0], NULL, NULL, szArgs, szEnv);
    if(0 != s32Ret)
    {
        MLOGE("hostapd_start: posix_spawn failed,errno(%d)\n",errno);
        return HI_FAILURE;
    }
    s32Ret = waitpid(pid, &s32Status, 0);
    if (s32Ret == -1)
    {
        MLOGE("wait pid(%d) failed,errno(%d)\n",pid,errno);
        return HI_FAILURE;
    }
#else
    HI_S32 s32Ret = 0;
    HI_CHAR cmd[192]={'\0'};
    snprintf(cmd,sizeof(cmd),"%s -e %s -B %s",HAL_WIFI_HOSTAPD_EXECUTE_FILE,
        HAL_WIFI_ENTROPY_FILE,HAL_WIFI_HOSTAPD_CONFIG_FILE);
    s32Ret = HI_system(cmd);
    if(s32Ret == HI_FAILURE)
    {
        if(ECHILD != errno)
        {
            MLOGE("system errno[%d]\n",errno);
            //return HI_FAILURE;
        }
    }
#endif
    return HI_SUCCESS;
}

HI_S32 HAL_WIFI_AP_DhcpdStart()
{
#if defined(POSIX_SPAWN_FUNC)
    pid_t pid = 0;
    HI_S32 s32Status = 0;
    HI_S32 s32Ret = 0;
    HI_CHAR* szArgs[] = {HAL_WIFI_DHCPD_EXECUTE_FILE, HAL_WIFI_DHCPD_CONFIG_FILE, NULL};
    HI_CHAR* szEnv[]  = {NULL};

    s32Ret = posix_spawn(&pid, szArgs[0], NULL, NULL, szArgs, szEnv);
    if(0 != s32Ret)
    {
        MLOGE("udhcpd: posix_spawn failed,errno(%d)\n",errno);
        return HI_FAILURE;
    }
    s32Ret = waitpid(pid, &s32Status, 0);
    if (s32Ret == HI_FAILURE)
    {
        MLOGE("wait pid(%d) failed,errno(%d)\n",pid,errno);
        return HI_FAILURE;
    }
#else
    HI_S32 s32Ret = 0;
    HI_CHAR cmd[128]={'\0'};
    strncat(cmd,HAL_WIFI_DHCPD_EXECUTE_FILE,62);
    strcat(cmd," ");
    strncat(cmd,HAL_WIFI_DHCPD_CONFIG_FILE,62);
    s32Ret = HI_system(cmd);
    if(s32Ret == HI_FAILURE)
    {
        if(ECHILD != errno)
        {
            MLOGE("system errno[%d]\n",errno);
            //return HI_FAILURE;
        }
    }

#endif
    return HI_SUCCESS;
}

HI_S32 HAL_WIFI_AP_DhcpdStop()
{
#if defined(POSIX_SPAWN_FUNC)
    pid_t pid = 0;
    HI_S32 s32Status = 0;
    HI_S32 s32Ret = 0;
    HI_CHAR* szArgs[] = {HAL_WIFI_KILL_EXECUTE_FILE, "udhcpd", NULL};
    HI_CHAR* szEnv[]  = {NULL};

    s32Ret = posix_spawn(&pid, szArgs[0], NULL, NULL, szArgs, szEnv);
    if(0 != s32Ret)
    {
        MLOGE("dhcpd_stop: posix_spawn failed, %#x\n", s32Ret);
        return HI_FAILURE;
    }
    s32Ret = waitpid(pid, &s32Status, 0);
    if (s32Ret == HI_FAILURE)
    {
        MLOGE("wait pid(%d) failed,errno(%d)\n",pid,errno);
        return HI_FAILURE;
    }
#else

    HI_S32 s32Ret = 0;
    HI_CHAR cmd[128]={'\0'};
    strncat(cmd,HAL_WIFI_KILL_EXECUTE_FILE,63);
    strcat(cmd," ");
    strncat(cmd,"udhcpd",64);
    sighandler_t SignalPrev;
    SignalPrev = signal(SIGCHLD,SIG_DFL);
    s32Ret = HI_system(cmd);
    if (s32Ret == HI_FAILURE)
    {
        MLOGE("system errno[%d]\n",errno);
        //return HI_FAILURE;
    }
    signal(SIGCHLD,SignalPrev);

#endif
    return HI_SUCCESS;
}

HI_S32 HAL_WIFI_AP_HostapdStop()
{
#if defined(POSIX_SPAWN_FUNC)
    pid_t pid = 0;
    HI_S32 s32Status = 0;
    HI_S32 s32Ret = 0;
    HI_CHAR* szArgs[] = {HAL_WIFI_KILL_EXECUTE_FILE, "hostapd", NULL};
    HI_CHAR* szEnv[]  = {NULL};

    s32Ret = posix_spawn(&pid, szArgs[0], NULL, NULL, szArgs, szEnv);
    if(0 != s32Ret)
    {
        MLOGE("hostapd_stop: posix_spawn failed,errno(%d)\n",errno);
        return HI_FAILURE;
    }
    s32Ret = waitpid(pid, &s32Status, 0);
    if (s32Ret == -1)
    {
        MLOGE("wait pid(%d) failed,errno(%d)\n",pid,errno);
        return HI_FAILURE;
    }
#else

    HI_S32 s32Ret = 0;
    HI_CHAR cmd[128]={'\0'};
    strncat(cmd,HAL_WIFI_KILL_EXECUTE_FILE,63);
    strcat(cmd," ");
    strncat(cmd,"hostapd",64);
    sighandler_t SignalPrev;
    SignalPrev = signal(SIGCHLD,SIG_DFL);
    s32Ret = HI_system(cmd);
    if (s32Ret == HI_FAILURE)
    {
        MLOGE("system errno[%d]\n",errno);
        //return HI_FAILURE;
    }
    signal(SIGCHLD,SignalPrev);
#endif
    return HI_SUCCESS;
}


HI_S32 HAL_WIFI_AP_Start(HI_CHAR *ifname,const HI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if((NULL == ifname) || (*ifname == '\0') || (pstApCfg == NULL))
    {
        MLOGE("ifname is null or no content or stApCfg is null,error");
        return HI_FAILURE;
    }
    s32Ret = HAL_WIFI_AP_UpdateConfig(ifname,pstApCfg,HAL_WIFI_HOSTAPD_CONFIG_FILE);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("update config error\n");
        return HI_FAILURE;
    }
    s32Ret = HAL_WIFI_HAL_PowerOnReset();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("reg on pinout reset error\n");
        return HI_FAILURE;
    }
    s32Ret = HAL_WIFI_UTILS_Ifconfig(HAL_WIFI_INTERFACE_NAME,1);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("ifconfig error\n");
        return HI_FAILURE;
    }

    s32Ret = HAL_WIFI_HAL_GetInterface(HAL_WIFI_INTERFACE_NAME);
    if (HI_FAILURE == s32Ret)
    {
        MLOGE("found no wlan0,get wifi interface fail\n");
        return HI_FAILURE;
    }

    s32Ret = HAL_WIFI_UTILS_SetIp(HAL_WIFI_INTERFACE_NAME,HAL_WIFI_IP);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("ifconfig error\n");
        return HI_FAILURE;
    }
    s32Ret = HAL_WIFI_UTILS_AllFDClosexec();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGW("All fd Closexec error\n");
    }
    s32Ret = HAL_WIFI_AP_HostapdStart();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Hostapd Start error\n");
        return HI_FAILURE;
    }
    s32Ret = HAL_WIFI_AP_DhcpdStart();
    if(HI_SUCCESS != s32Ret)
    {
        HAL_WIFI_AP_HostapdStop();
        MLOGE("Dhcpd Start error");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


HI_S32 HAL_WIFI_AP_Stop(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HAL_WIFI_AP_DhcpdStop();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Dhcpd Stop error");
    }
    else
    {
        s32Ret = HAL_WIFI_AP_HostapdStop();
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("hostapd stop failed\n");
            return s32Ret;
        }
        s32Ret = HAL_WIFI_UTILS_Ifconfig(HAL_WIFI_INTERFACE_NAME,0);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("ifconfig error");
            return HI_FAILURE;
        }
    }
    return s32Ret;
}
#else
static struct netif *g_rtw_netif = NULL;
extern HI_S32 liteos_priority_task(HI_S32 priority,HI_VOID *(*start_routine) (HI_VOID *), HI_VOID *arg);

static HI_S32 liteos_softap_set_ip_addr(struct netif *pnetif)
{
/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   1
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    return netifapi_netif_set_addr(pnetif, &ipaddr, &netmask, &gw);
}

static HI_S32 liteos_stop_softap(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = netifapi_netif_set_down(&g_rtw_netif[0]);
    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(netifapi_netif_set_down,s32Ret);
    }
    s32Ret = netifapi_dhcps_stop(&g_rtw_netif[0]);
    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(netifapi_dhcps_stop,s32Ret);
    }
    return hostapd_stop();
}

static HI_S32 liteos_start_softap(const HI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    struct hostapd_conf hapd_conf;
    memset(&hapd_conf, 0, sizeof(struct hostapd_conf));
    /* set driver name */
    snprintf(hapd_conf.driver,sizeof(hapd_conf.driver),"realtek");
    /* set ssid */
    snprintf(hapd_conf.ssid,sizeof(hapd_conf.ssid),"%s",pstApCfg->stCfg.szWiFiSSID);
    /* set channel */
    hapd_conf.channel_num = pstApCfg->s32Channel;
    hapd_conf.authmode = HOSTAPD_SECURITY_OPEN;
    MLOGI("dump hostapd configuration\n");
    MLOGI("ssid: %s \n",hapd_conf.ssid);
    MLOGI("channel: %d \n",hapd_conf.channel_num);
    MLOGI("auth_algs: %d \n",hapd_conf.auth_algs);
    MLOGI("wpa: %d \n",hapd_conf.wpa);
    MLOGI("wpa_key_mgnt: %d \n",hapd_conf.wpa_key_mgmt);
    MLOGI("wpa_pairwise: %d \n",hapd_conf.wpa_pairwise);

    s32Ret = hostapd_start("rtl8188eus", &hapd_conf);/*init hostapd*/
    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = liteos_softap_set_ip_addr(&g_rtw_netif[0]);
        if (s32Ret != HI_SUCCESS)
        {
            HI_LOG_PrintFuncErr(liteos_softap_set_ip_addr,s32Ret);
            goto err;
        }
        s32Ret = netifapi_dhcps_start(&g_rtw_netif[0], NULL, 0);
        if (s32Ret != HI_SUCCESS)
        {
            HI_LOG_PrintFuncErr(netifapi_dhcps_start,s32Ret);
            goto err;
        }
        s32Ret = netifapi_netif_set_up(&g_rtw_netif[0]);
        if (s32Ret != HI_SUCCESS)
        {
            HI_LOG_PrintFuncErr(netifapi_netif_set_up,s32Ret);
            goto err;
        }
        return HI_SUCCESS;
    }
err:
    liteos_stop_softap();
    return s32Ret;
}

static HI_VOID* liteos_wifi_ap_start(HI_VOID* arg)
{
    HI_S32 s32Ret = liteos_start_softap((HI_HAL_WIFI_APMODE_CFG_S*)arg);
    if (s32Ret!=HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(liteos_start_softap,s32Ret);
    }
    return NULL;
}

static HI_VOID* liteos_wifi_ap_stop(HI_VOID* arg)
{
    HI_S32 s32Ret = liteos_stop_softap();
    if (s32Ret!=HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(liteos_stop_softap,s32Ret);
    }
    return NULL;
}

HI_S32 HAL_WIFI_AP_Start(HI_CHAR *ifname,const HI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    g_rtw_netif = netif_find(ifname);
    if (g_rtw_netif==NULL)
    {
        MLOGE("netif_find %s failed\n",ifname);
        return HI_FAILURE;
    }
    return liteos_priority_task(5, liteos_wifi_ap_start, (HI_VOID*)pstApCfg);
}

HI_S32 HAL_WIFI_AP_Stop(HI_VOID)
{
    if (g_rtw_netif!=NULL)
    {
        (HI_VOID)liteos_priority_task(5, liteos_wifi_ap_stop, NULL);
        g_rtw_netif = NULL;
    }
    return HI_SUCCESS;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
