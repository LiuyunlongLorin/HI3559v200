/**
* @file    hi_hal_gsensor.c
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

#include "hi_appcomm.h"
#include "hi_type.h"
#include "hi_appcomm_util.h"
#include "hi_hal_gsensor.h"
#include "hi_hal_common.h"



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define SENSITIVITY     _IOWR('w', 4, unsigned int)
#define SAMPLERATE      _IOWR('r', 5, unsigned int)
#define EXCEED_THRESHOLD_FLAG      _IOWR('r', 5, unsigned int)

#define HAL_GSENSOR_DEV "/dev/gsensor"
static HI_S32 s_s32HALGSENSORfd = HAL_FD_INITIALIZATION_VAL;

static HI_VOID HAL_GSENSOR_PinoutInit(HI_VOID)
{
#if defined(HI3556AV100) || defined(HI3519AV100)
#ifdef BOARD_DEMB
    himm(0x0405806c ,0x1073); /**< #pinout reuse as SDA*/
    himm(0x04058070 ,0x1073); /**< #pinout reuse as SCL*/
#else
    MLOGE("please point board type,fail\n\n");
#endif

#elif defined HI3559V200
#if defined(BOARD_DEMB) || defined(BOARD_DASHCAM_REFB)
    himm(0x112f0000 ,0x591); /**< #pinout reuse as SDA*/
    himm(0x112f0004 ,0x591); /**< #pinout reuse as SCL*/
#else
    MLOGE("please point board type,fail\n\n");
#endif

#elif defined HI3518EV300
#ifdef BOARD_DEMB
    himm(0x112c0038 ,0x1c21); /**< #pinout reuse as SDA*/
    himm(0x112c003c ,0x1c21); /**< #pinout reuse as SCL*/
#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");
#endif
}


HI_S32 HI_HAL_GSENSOR_Init(const HI_HAL_GSENSOR_CFG_S* pstCfg)
{
    HI_S32 s32Ret;
    if (s_s32HALGSENSORfd != HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("already init");
        return HI_HAL_EINITIALIZED;
    }
	HAL_GSENSOR_PinoutInit();
#if defined(HI3556AV100) || defined(HI3519AV100)
#ifdef BOARD_DEMB
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/mc_3416.ko","I2C_NUM=4");
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod 3c3416: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
#else
        MLOGE("please point board type,fail\n\n");
#endif

#elif defined HI3559V200
#if defined(BOARD_DEMB)
    MLOGI("HI3559V200 BOARD_DEMB\n");
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/mc_3416.ko","I2C_NUM=7");
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod 3c3416: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
#elif defined BOARD_DASHCAM_REFB
    MLOGI("BOARD_DASHCAM_REFB\n");
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/mc_3416.ko","I2C_NUM=7 LEVEL=1");
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod 3c3416: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
#else
        MLOGE("please point board type,fail\n\n");
#endif

#elif defined HI3518EV300
    #define I2C_NUM (2)
    #define LEVEL (0)
	extern int gsesnor_init(int i2c_num,int level);
    gsesnor_init(I2C_NUM,LEVEL);

#else
	MLOGE("please point chip type,fail\n\n");
#endif

    s_s32HALGSENSORfd = open(HAL_GSENSOR_DEV, O_RDWR);

    if (s_s32HALGSENSORfd < 0)
    {
        MLOGE("open [%s] failed\n",HAL_GSENSOR_DEV);
        return HI_HAL_EINVOKESYS;
    }
    s32Ret = ioctl(s_s32HALGSENSORfd, SENSITIVITY, &pstCfg->enSensitity);/**init gsensor sensitivity */
    if(-1 == s32Ret)
    {
        MLOGE("insmod 3c3416: failed, errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
    ioctl(s_s32HALGSENSORfd, EXCEED_THRESHOLD_FLAG, &s32Ret);/**skip first interupt flag */
    return HI_SUCCESS;
}


HI_S32 HI_HAL_GSENSOR_GetCollisionStatus(HI_BOOL * pbOnCollison)
{
    HI_S32 s32Ret;
    if (s_s32HALGSENSORfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    if (pbOnCollison == NULL)
    {
        MLOGE("not init");
        return HI_HAL_EINVAL;
    }

    s32Ret = ioctl(s_s32HALGSENSORfd, EXCEED_THRESHOLD_FLAG, pbOnCollison);
    if (s32Ret < 0)
    {
        MLOGE("ioctl error");
        return HI_HAL_EINVOKESYS;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GSENSOR_SetSensitity(HI_HAL_GSENSOR_SENSITITY_E enSensitity)
{
    HI_S32 s32Ret;
    if (s_s32HALGSENSORfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    if ((enSensitity < HI_HAL_GSENSOR_SENSITITY_OFF) ||
        (enSensitity >= HI_HAL_GSENSOR_SENSITITY_BUIT))
    {
        MLOGE("param valid");
        return HI_HAL_EINVAL;
    }
    s32Ret = ioctl(s_s32HALGSENSORfd, SENSITIVITY, &enSensitity);
    if (s32Ret < 0)
    {
        MLOGE("ioctl error");
        return HI_HAL_EINVOKESYS;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GSENSOR_SetAttr(const HI_HAL_GSENSOR_ATTR_S* pstAttr)
{
    return HI_SUCCESS;

}


HI_S32 HI_HAL_GSENSOR_Deinit()
{
    HI_S32 s32Ret;

    if (s_s32HALGSENSORfd == HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("gsensor not initialized,no need to close\n");
        return HI_HAL_ENOINIT;
    }
    s32Ret = close(s_s32HALGSENSORfd);
    if (0 > s32Ret)
    {
        MLOGE("gsesnorfd[%d] close,fail,errno(%d)\n",s_s32HALGSENSORfd,errno);
        return HI_HAL_EINVOKESYS;
    }
    s_s32HALGSENSORfd = HAL_FD_INITIALIZATION_VAL;
#ifndef __LITEOS__
    s32Ret = HI_rmmod(HI_APPFS_KOMOD_PATH"/mc_3416.ko");
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_rmmod failed\n");
        return HI_FAILURE;
    }
#else
    extern void gsesnor_exit(void);
    gsesnor_exit();
#endif
    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


