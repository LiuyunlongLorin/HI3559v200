/**
* @file    hi_hal_watchdog.c
* @brief   hal gsensor implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/prctl.h>


#include "watchdog.h"

#include "hi_appcomm.h"
#include "hi_type.h"
#include "hi_appcomm_util.h"
#include "hi_hal_watchdog.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define HAL_WATCHDOG_DEV "/dev/watchdog"
static HI_S32 s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;

HI_S32 HI_HAL_WATCHDOG_Init(HI_S32 s32Time_s)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (s_s32HALWATCHDOGfd != HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("already init");
        return HI_HAL_EINITIALIZED;
    }

    if (s32Time_s < 2 || s32Time_s > 1000)
    {
        MLOGE("Interval time should not be less then two and bigger then 100. %d\n", s32Time_s);
        return HI_HAL_EINVAL;
    }

    HI_CHAR szWdtString[HI_APPCOMM_MAX_PATH_LEN] = {0};
    snprintf(szWdtString, HI_APPCOMM_MAX_PATH_LEN, " default_margin=%d nodeamon=1", s32Time_s);

#if defined(HI3559V200)
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/hi3559v200_wdt.ko",szWdtString);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod 3c3416: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
#endif

    s_s32HALWATCHDOGfd = open(HAL_WATCHDOG_DEV, O_RDWR);

    if (s_s32HALWATCHDOGfd < 0)
    {
        MLOGE("open [%s] failed\n",HAL_WATCHDOG_DEV);
        return HI_HAL_EINVOKESYS;
    }


    s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE);/**feed dog */
    if(-1 == s32Ret)
    {
        MLOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_WATCHDOG_Feed(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE);/**feed dog */
    if(-1 == s32Ret)
    {
        MLOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
    return HI_SUCCESS;
}


HI_S32 HI_HAL_WATCHDOG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;

    if (s_s32HALWATCHDOGfd == HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("watchdog not initialized,no need to close\n");
        return HI_HAL_ENOINIT;
    }
    s32Ret = close(s_s32HALWATCHDOGfd);
    if (0 > s32Ret)
    {
        MLOGE("wdrfd[%d] close,fail,errno(%d)\n",s_s32HALWATCHDOGfd,errno);
        return HI_HAL_EINVOKESYS;
    }
    s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


