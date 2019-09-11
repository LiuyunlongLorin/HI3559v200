/**
* Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file    sensor_comm.h
* @brief   register sensor functions declaration
* @author  HiMobileCam NDK develop team
* @date    2018/2/6
*/
#ifndef __REGISTER_SENSOR_H__
#define __REGISTER_SENSOR_H__

#include "hi_mipi.h"
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "sensor_interface_cfg_params.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 MAPI_SENSOR_Register(HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg);
HI_S32 MAPI_SENSOR_Load(HI_VOID);
HI_S32 MAPI_SENSOR_GetConfig(HI_S32 s32SnsType, HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg);
HI_S32 MAPI_SENSOR_LoadSnsType(HI_S32 *ps32SensorType);
HI_S32 MAPI_SENSOR_LoadCommCfg(HI_MAPI_SENSOR_COMM_CFG_S *ps32SensorCommCfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __REGISTER_ALL_SENSOR_H__ */

