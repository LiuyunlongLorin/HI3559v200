/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file    comm_sensor_intf_cfg_params.c
* @brief   sensor comm config params
* @author  HiMobileCam NDK develop team
* @date    2018-3-5
*/
#include "sensor_interface_cfg_params.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* sensor type */
static const HI_S32 g_as32AllSensorType[HI_MAPI_VCAP_MAX_DEV_NUM] = {
    CFG_SENSOR_TYPE0,
    CFG_SENSOR_TYPE1,
};

/* -1 is unused */
static const HI_MAPI_SENSOR_COMM_CFG_S g_stAllCommSnsIntfCfg = {
    .enLaneIdMode = LANE_DIVIDE_MODE,

    .aSnsClkSource = { /* Based on circuit pin connections  */
#if defined CFG_LANE_DIVIDE_MODE_0
        0, 1

#elif defined CFG_LANE_DIVIDE_MODE_1
        0, 1
#endif
    },

    .aSnsRstSource = { /* Based on circuit pin connections */
#if defined CFG_LANE_DIVIDE_MODE_0
        0, 1

#elif defined CFG_LANE_DIVIDE_MODE_1
        0, 1
#endif
    },

    .aBusId = { /* Based on circuit pin connections */
#if defined CFG_LANE_DIVIDE_MODE_0
        0, 1

#elif defined CFG_LANE_DIVIDE_MODE_1
        0, 1
#endif
    },
};

HI_VOID MAPI_SENSOR_LoadSnsType(HI_S32 *ps32SensorType)
{
    memcpy(ps32SensorType, g_as32AllSensorType, sizeof(HI_S32) * HI_MAPI_VCAP_MAX_DEV_NUM);

    return;
}

HI_VOID MAPI_SENSOR_LoadCommCfg(HI_MAPI_SENSOR_COMM_CFG_S *ps32SensorCommCfg)
{
    memcpy(ps32SensorCommCfg, &g_stAllCommSnsIntfCfg, sizeof(HI_MAPI_SENSOR_COMM_CFG_S));

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
