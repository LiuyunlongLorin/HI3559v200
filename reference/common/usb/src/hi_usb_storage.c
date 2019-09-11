/**
 * @file      hi_usb_storage.c
 * @brief     usb storage interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#ifdef __HuaweiLite__
#include "implementation/usb_init.h"
#endif
#include "hi_usb_storage.h"
#include "hi_uvc.h"
#include "hi_appcomm_util.h"
#include "hi_mapi_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_BOOL g_bInited = HI_FALSE;
static pthread_mutex_t g_Mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef __HuaweiLite__
extern int fmass_register_notify(void(*notify)(void *context, int status), void *context);
extern int fmass_partition_startup(char *path);
HI_S32 g_fmass_status = 0;
static HI_VOID fmass_notify(HI_VOID *conext, HI_S32 status)
{
    if (status == 1) {
        MLOGI("usb device connect.\n");
    } else {
        MLOGI("usb device disconnect.\n");
    }
    g_fmass_status = status;
}
#endif

static HI_S32 USB_STORAGE_LoadMod(HI_VOID)
{
#ifndef __HuaweiLite__
    HI_insmod(HI_APPFS_KOMOD_PATH"/libcomposite.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/usb_f_mass_storage.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/g_mass_storage.ko", "luns=1 stall=0 removable=1");
#else
    usb_init(DEVICE, DEV_MASS);
    g_fmass_status = 0;
    fmass_register_notify(fmass_notify, NULL);
#endif
    return HI_SUCCESS;
}

static HI_VOID USB_STORAGE_UnloadMod(HI_VOID)
{
#ifndef __HuaweiLite__
    HI_rmmod(HI_APPFS_KOMOD_PATH"/g_mass_storage.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/usb_f_mass_storage.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/libcomposite.ko");
#else
    g_fmass_status = 0;
    usb_deinit();
#endif
}

HI_S32 USB_STORAGE_Init(HI_VOID)
{
    MUTEX_LOCK(g_Mutex);
    if(HI_TRUE == g_bInited)
    {
        MLOGD(YELLOW"already inited\n"NONE);
        MUTEX_UNLOCK(g_Mutex);
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = USB_STORAGE_LoadMod();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "LoadKo");
    g_bInited = HI_TRUE;
    MUTEX_UNLOCK(g_Mutex);
    return HI_SUCCESS;
}

HI_S32 USB_STORAGE_Deinit(HI_VOID)
{
    MUTEX_LOCK(g_Mutex);
    if(HI_FALSE == g_bInited)
    {
        MLOGD(YELLOW"already deinited\n"NONE);
        MUTEX_UNLOCK(g_Mutex);
        return HI_SUCCESS;
    }

    USB_STORAGE_UnloadMod();
    g_bInited = HI_FALSE;
    MUTEX_UNLOCK(g_Mutex);
    return HI_SUCCESS;
}

HI_S32 USB_STORAGE_PrepareDev(const HI_USB_STORAGE_CFG_S* pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);
#ifndef __HuaweiLite__
    /* Read file content */
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szBuf[HI_APPCOMM_MAX_PATH_LEN] = {0};
    HI_S32  s32Fd = open(pstCfg->szSysFile, O_CREAT | O_RDWR, 0644);
    if(s32Fd < 0)
    {
        MLOGE("open %s failed\n", pstCfg->szSysFile);
        return HI_FAILURE;
    }
    s32Ret = read(s32Fd, szBuf, HI_APPCOMM_MAX_PATH_LEN);
    if(s32Ret < 0)
    {
        MLOGE("read %s failed\n", pstCfg->szSysFile);
        close(s32Fd);
        return HI_FAILURE;
    }

    ftruncate(s32Fd, 0);
    write(s32Fd, pstCfg->szDevPath, strnlen(pstCfg->szDevPath, HI_APPCOMM_MAX_PATH_LEN));
    close(s32Fd);
#else
    fmass_partition_startup((HI_CHAR*)pstCfg->szDevPath);
#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

