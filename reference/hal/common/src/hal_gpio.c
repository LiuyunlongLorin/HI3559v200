/**
* @file    hal_gpio.c
* @brief   product hal gpio implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#include <fcntl.h>
#include <string.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#ifdef __HuaweiLite__
#include <gpio.h>
#endif

#include "hi_type.h"
#include "hi_hal_gpio_inner.h"

#include "hi_hal_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#ifdef __HuaweiLite__
#define HAL_GPIO_DEV    "/dev/gpio"
#else
#define HAL_GPIO_DEV    "/dev/hi_gpio"
#endif

typedef struct tagHAL_GPIO_DATA_S
{
    HI_U8 u8Pin; /**< the pin's No */
    HI_U8 u8Dir;  /**< 0:input 1:output*/
    HI_S32 u8Value;
} HAL_GPIO_DATA_S;

/** macro define */
#define HAL_GPIO_SET_DIR      _IOWR('w', 4, HAL_GPIO_DATA_S)
#define HAL_GPIO_GET_DIR      _IOWR('r', 5, HAL_GPIO_DATA_S)
#define HAL_GPIO_READ_BIT      _IOWR('r', 6, HAL_GPIO_DATA_S)
#define HAL_GPIO_WRITE_BIT      _IOWR('w', 7, HAL_GPIO_DATA_S)

typedef struct tagHAL_GPIO_GROUPBIT_S
{
    HI_U32  u32GroupNum;
    HI_U32  u32BitNum;
    HI_U32  u32Value;
} HAL_GPIO_GROUPBIT_S;

HI_S32 HAL_GPIO_Init(HI_S32* ps32fd)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL == ps32fd)
    {
        MLOGE("[Error]input ps32fd is null\n");
        return HI_FAILURE;
    }

    /** Initial GPIO Device */
    s32Ret = open(HAL_GPIO_DEV, O_RDWR);

    if (HAL_FD_INITIALIZATION_VAL == s32Ret)
    {
        MLOGE("[Error] open gpiodev failed\n");
        return HI_FAILURE;
    }

    *ps32fd = s32Ret;
    return HI_SUCCESS;
}

HI_S32 HAL_GPIO_SetDir(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HAL_GPIO_DIR_E enDirVal)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HAL_FD_INITIALIZATION_VAL >= s32fd)
    {
        MLOGE("[Error]gpio(%d) fd is illegal\n", s32fd);
        return HI_FAILURE;
    }

    if ((HAL_GPIO_DIR_READ != enDirVal) && (HAL_GPIO_DIR_WRITE != enDirVal))
    {
        MLOGE("[Error]enDirVal(%d) illegal\n", enDirVal);
        return HI_FAILURE;
    }

#ifdef __HuaweiLite__
    gpio_groupbit_info stGpioData = {0};
    stGpioData.groupnumber = u32GrpNum;
    stGpioData.bitnumber   = u32BitNum;
    if (enDirVal == HAL_GPIO_DIR_READ)    /* read gpio to set input */
    {
        stGpioData.direction = GPIO_DIR_IN;
    }
    s32Ret = ioctl(s32fd, GPIO_SET_DIR, &stGpioData);
#else
    HAL_GPIO_GROUPBIT_S stGpioData;
    stGpioData.u32GroupNum = u32GrpNum;
    stGpioData.u32BitNum = u32BitNum;
    stGpioData.u32Value = enDirVal;
    s32Ret = ioctl(s32fd, HAL_GPIO_SET_DIR, &stGpioData);

#endif

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(" [Error] failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 HAL_GPIO_GetBitVal(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HI_U32* pu32BitVal)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HAL_FD_INITIALIZATION_VAL >= s32fd)
    {
        MLOGE("[Error]gpio(%d) fd is illegal\n", s32fd);
        return HI_FAILURE;
    }

#ifdef __HuaweiLite__
    gpio_groupbit_info stGpioData = {0};
    stGpioData.groupnumber = u32GrpNum;
    stGpioData.bitnumber   = u32BitNum;
    s32Ret = ioctl(s32fd, GPIO_READ_BIT, &stGpioData);
#else
    HAL_GPIO_GROUPBIT_S stGpioData;
    memset(&stGpioData, 0, sizeof(HAL_GPIO_GROUPBIT_S));
    stGpioData.u32GroupNum = u32GrpNum;
    stGpioData.u32BitNum = u32BitNum;
    s32Ret = ioctl(s32fd, HAL_GPIO_READ_BIT, &stGpioData);
#endif

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(" [Error] failed\n");
        return HI_FAILURE;
    }

#ifdef __HuaweiLite__
    *pu32BitVal = stGpioData.value;
#else
    *pu32BitVal = stGpioData.u32Value;
#endif
    return HI_SUCCESS;
}


HI_S32 HAL_GPIO_SetBitVal(HI_S32 s32fd, HI_U32 u32GrpNum, HI_U32 u32BitNum, HI_U32 u32BitVal)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HAL_FD_INITIALIZATION_VAL >= s32fd)
    {
        MLOGE("[Error]gpio(%d) fd is illegal\n", s32fd);
        return HI_FAILURE;
    }

#ifdef __HuaweiLite__
    gpio_groupbit_info stGpioData = {0};
    stGpioData.groupnumber = u32GrpNum;
    stGpioData.bitnumber   = u32BitNum;
    stGpioData.value       = (HI_U8)u32BitVal;
    s32Ret = ioctl(s32fd, GPIO_WRITE_BIT, &stGpioData);
#else
    HAL_GPIO_GROUPBIT_S stGpioData;
    memset(&stGpioData, 0, sizeof(HAL_GPIO_GROUPBIT_S));
    stGpioData.u32GroupNum = u32GrpNum;
    stGpioData.u32BitNum = u32BitNum;
    stGpioData.u32Value = u32BitVal;
    s32Ret = ioctl(s32fd, HAL_GPIO_WRITE_BIT, &stGpioData);
#endif

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(" [Error] failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_GPIO_Deinit(HI_S32 s32fd)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HAL_FD_INITIALIZATION_VAL >= s32fd)
    {
        MLOGE("[Error]gpio not init\n");
        return HI_FAILURE;
    }

    s32Ret = close(s32fd);

    if (0 == s32Ret)
    {
        s32Ret = HI_SUCCESS;
    }
    else
    {
        MLOGE("[Error]close s32fd(%d) fail,errno(%d)\n", s32fd,errno);
        s32Ret = HI_FAILURE;
    }
    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

