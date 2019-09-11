/**
* @file    hi_hal_gauge.c
* @brief   hal gauge implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/17
* @version   1.0

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "hi_hal_common_inner.h"

#include "hi_type.h"
#include "hi_hal_gauge.h"
#include "hi_appcomm_log.h"
#include "hi_appcomm_util.h"
#include "hi_hal_common.h"
#include "bq27421.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

/** macro define */
#define HAL_GAUGE_DEV    ("/dev/bq27421_bat")
static HI_S32 s_s32HALGAUGEfd = HAL_FD_INITIALIZATION_VAL;


HI_S32 HI_HAL_GAUGE_Init(HI_VOID)
{
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALGAUGEfd)
    {
        /**First Load KO*/
        HI_S32 s32Ret = HI_SUCCESS;

        s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/bq27421.ko",NULL);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("insmod bq27421:failed\n");
            return HI_HAL_EINTER;
        }

        s_s32HALGAUGEfd = open(HAL_GAUGE_DEV, O_RDWR);
        if (HAL_FD_INITIALIZATION_VAL == s_s32HALGAUGEfd)
        {
            MLOGE("[Error] open gauge failed\n");
            return HI_HAL_EINVOKESYS;
        }
    }
    else
    {
        MLOGE("[Warning] gauge already be opened\n");
        return HI_HAL_EINITIALIZED;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GAUGE_GetLevel(HI_S32* ps32Level) /* percent*/
{
    HI_S32 s32Ret = 0;
    HI_S32 s32Level = 0;
    /* parm ps32Level check */
    if (NULL == ps32Level)
    {
        MLOGE("ps32Level is null\n");
        return HI_HAL_EINVAL;
    }

    /* init check */
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALGAUGEfd)
    {
        MLOGE("gauge not initialized\n");
        return HI_HAL_ENOINIT;
    }

    s32Ret = ioctl(s_s32HALGAUGEfd, IOCTL_CMD_READ_CAPACITY, &s32Level);
    if(HI_FAILURE == s32Ret)
    {
        MLOGE("get Level error\n");
        return HI_HAL_EINTER;
    }
    *ps32Level = s32Level;

    return HI_SUCCESS;
}

HI_S32 HI_HAL_GAUGE_GetChargeState(HI_BOOL* pbCharge)
{
    HI_BOOL bCharge = HI_FALSE;
    /* parm ps32Level check */
    if (NULL == pbCharge)
    {
        MLOGE("pbCharge is null\n");
        return HI_HAL_EINVAL;
    }

    /* init check */
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALGAUGEfd)
    {
        MLOGE("gauge not initialized\n");
        return HI_HAL_ENOINIT;
    }

    *pbCharge = bCharge;

    return HI_SUCCESS;
}


HI_S32 HI_HAL_GAUGE_Deinit(HI_VOID)
{
    /* Close gauge Device */
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALGAUGEfd)
    {
        MLOGW("gauge not initialized, no need to close\n");
        return HI_HAL_ENOINIT;
    }
    close(s_s32HALGAUGEfd);
    s_s32HALGAUGEfd = HAL_FD_INITIALIZATION_VAL;

    HI_rmmod(HI_APPFS_KOMOD_PATH"/bq27421.ko");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif  /* End of #ifdef __cplusplus */

