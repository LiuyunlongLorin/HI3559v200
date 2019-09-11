/**
* @file    hi_hal_acc.c
* @brief   product acc interface
*
* Copyright (c) 2019 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2019/04/10
* @version

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "hi_hal_common_inner.h"
#include "hi_hal_gpio_inner.h"
#include "hi_appcomm_util.h"
#include "hi_appcomm_log.h"
#include "hi_hal_common.h"
#include "hi_hal_acc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define HAL_ACC_DET_GRP_NUM (2)            /**< acc detect gpio group number */
#define HAL_ACC_DET_BIT_NUM (4)            /**< acc detect gpio bit number */

#define HAL_USB_SUPPLY_DET_GRP_NUM (8)     /**< usb supply detect gpio group number */
#define HAL_USB_SUPPLY_DET_BIT_NUM (5)     /**< usb supply detect gpio bit number */

#define HAL_ACC_LEVEL_LOW   (0)            /**<low level*/
#define HAL_ACC_LEVEL_HIGH  (1)            /**<high level*/

static HI_S32 g_halAccfd = HAL_FD_INITIALIZATION_VAL;

HI_S32 HI_HAL_ACC_GetState(HI_ACC_VOLTAGE_SUPPLY *accState)
{
    HI_S32 ret = HI_SUCCESS;
    HI_U32 gpioGrpNum, gpioBitNum;
    HI_U32 value;
    if (g_halAccfd == HAL_FD_INITIALIZATION_VAL) {
        MLOGE("acc not initialized\n");
        return HI_HAL_ENOINIT;
    }
    if (accState == NULL) {
        MLOGE("pAccState is null\n");
        return HI_HAL_EINVAL;
    }
    memset(accState, 0, sizeof(HI_ACC_VOLTAGE_SUPPLY));
    gpioGrpNum = HAL_ACC_DET_GRP_NUM;
    gpioBitNum = HAL_ACC_DET_BIT_NUM;
    ret = HAL_GPIO_GetBitVal(g_halAccfd, gpioGrpNum, gpioBitNum, &value);
    if (HI_SUCCESS != ret) {
        MLOGE("[Error]read gpio data failed\n");
        return HI_HAL_EGPIO;
    }
    if (value == HAL_ACC_LEVEL_LOW) {
        accState->accDetect = HI_HAL_ACC_STATE_IGNITION;
    } else if (value == HAL_ACC_LEVEL_HIGH) {
        accState->accDetect = HI_HAL_ACC_STATE_FLAMEOUT;
    }
    gpioGrpNum = HAL_USB_SUPPLY_DET_GRP_NUM;
    gpioBitNum = HAL_USB_SUPPLY_DET_BIT_NUM;
    ret = HAL_GPIO_GetBitVal(g_halAccfd, gpioGrpNum, gpioBitNum, &value);
    if (HI_SUCCESS != ret) {
        MLOGE("[Error]read gpio data failed\n");
        return HI_HAL_EGPIO;
    }
    if (value == HAL_ACC_LEVEL_LOW) {
        accState->usbVolSupply = HI_HAL_ACC_STATE_FLAMEOUT;
    } else if (value == HAL_ACC_LEVEL_HIGH) {
        accState->usbVolSupply = HI_HAL_ACC_STATE_IGNITION;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_ACC_init(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    if (g_halAccfd != HAL_FD_INITIALIZATION_VAL) {
        MLOGI("hal_acc has already been init\n");
        return HI_SUCCESS;
    }
    himm(0x112F0038, 0x430);  /*usb voltage supply gpio init*/
    himm(0x114F0000, 0x430);  /*acc detect gpio init*/
    /** Initial GPIO Device */
    ret = HAL_GPIO_Init(&g_halAccfd);
    if (HI_FAILURE == ret) {
        MLOGE("[Error] open gpiodev failed,errno(%d)\n", errno);
        return HI_HAL_EINTER;
    }
    ret = HAL_GPIO_SetDir(g_halAccfd, HAL_ACC_DET_GRP_NUM, HAL_ACC_DET_BIT_NUM, HAL_GPIO_DIR_READ);
    if (HI_SUCCESS != ret) {
        MLOGE("[Error]set gpio dir failed\n");
        HAL_GPIO_Deinit(g_halAccfd);
        return HI_HAL_EGPIO;
    }
    ret = HAL_GPIO_SetDir(g_halAccfd, HAL_USB_SUPPLY_DET_GRP_NUM, HAL_USB_SUPPLY_DET_BIT_NUM, HAL_GPIO_DIR_READ);
    if (HI_SUCCESS != ret) {
        MLOGE("[Error]set gpio dir failed\n");
        HAL_GPIO_Deinit(g_halAccfd);
        return HI_HAL_EGPIO;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_ACC_deinit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    if (g_halAccfd == HAL_FD_INITIALIZATION_VAL) {
        MLOGE("Device has already deinit or Uninitialized!\n");
        return HI_SUCCESS;
    }
    ret = HAL_GPIO_Deinit(g_halAccfd);
    if (HI_SUCCESS != ret) {
        MLOGE("gpio deinit,errno(%d)\n", errno);
        return HI_HAL_EINVOKESYS;
    }
    g_halAccfd = HAL_FD_INITIALIZATION_VAL;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
