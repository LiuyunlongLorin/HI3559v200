/**
* @file    hi_hal_acc.h
* @brief   product hal acc interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2019/04/10
* @version

*/
#ifndef __HI_HAL_ACC_H__
#define __HI_HAL_ACC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_type.h"

typedef enum hiHAL_ACC_STATE {
    HI_HAL_ACC_STATE_IGNITION = 0,/**<dashcam ignition*/
    HI_HAL_ACC_STATE_FLAMEOUT, /**<dashcam flameout*/
    HI_HAL_ACC_STATE_BUIT
}
HI_HAL_ACC_STATE;

typedef struct hiACC_VOLTAGE_SUPPLY {
    HI_HAL_ACC_STATE usbVolSupply;   //usb voltage supply
    HI_HAL_ACC_STATE accDetect;   //acc detect
} HI_ACC_VOLTAGE_SUPPLY;

HI_S32 HI_HAL_ACC_GetState(HI_ACC_VOLTAGE_SUPPLY *accState);

HI_S32 HI_HAL_ACC_init(HI_VOID);

HI_S32 HI_HAL_ACC_deinit(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_H__*/
