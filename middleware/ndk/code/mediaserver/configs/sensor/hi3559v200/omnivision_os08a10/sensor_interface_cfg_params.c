/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file    sensor_interface_cfg_params.c
* @brief   sensor dev interface config params:
* one type interface config params must be choiced at global var "user_dev_attr"
* @author  HiMobileCam NDK develop team
* @date    2018-2-7
*/

#include "mpi_vi.h"
#include "mpi_isp.h"
#include "hi_sns_ctrl.h"
#include "sensor_comm.h"
#include "sensor_interface_cfg_params.h"
#include "../include/sensor_interface_cfg_adapt.h"

extern ISP_SNS_OBJ_S stSnsOS08A10Obj;

static const HI_MAPI_MIPI_INTF_S g_Sensor_MipiIntfCfg[] = {
    /* config 4k@30fps */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 3840,
            .height = 2160
        },
        .stSnsSize = {
            .u32Width = 3840,
            .u32Height = 2160
        },
        .mipi_attr =
        {
            .input_data_type = DATA_TYPE_RAW_10BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },
};

/* width and height must align with 4 bytes */
static const HI_MAPI_SENSOR_MODE_S g_Sensor_Mode[] = {
    /* SENSOR_MODE_4K_30FPS  frameRate:[1.0-30] */
    {
        .s32Width = 3840,
        .s32Height = 2160,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_NONE,
        .f32SnsMaxFrameRate = 30.0f
    },
};

/* final interface here */
static const HI_MAPI_COMBO_DEV_ATTR_S g_Sensor_Cfg = {
    /* config SENSOR */
    .pSensorObj = &stSnsOS08A10Obj,
    .s32SensorType = OS08A10,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .stSensorInputAttr = {
        .enBayerFormat = BAYER_BGGR,
        .enSensorCommBusType = HI_SENSOR_COMMBUS_TYPE_I2C,
    },

    .stSensorMode = {
        .s32SensorModeCnt = sizeof(g_Sensor_Mode) / sizeof(HI_MAPI_SENSOR_MODE_S),
        .pSensorMode = (void*)g_Sensor_Mode,
        .pstIntf = (void*)g_Sensor_MipiIntfCfg
    },

    .aLaneId = {
#if defined CFG_LANE_DIVIDE_MODE_0
        /* mipi_dev0 */
        {
            { 0, 1, 2, 3 }, /* 4K30 */
            {},
            {},
            {},
            {},
            {},
        },

        /* No mipi_dev1 */
        {
            {}, {}, {}, {}, {}, {},

        }
#elif defined CFG_LANE_DIVIDE_MODE_1
        /* mipi_dev0 */
        {
            {}, {}, {}, {}, {}, {},
        },

        /* mipi_dev1 */
        {
            {}, {}, {}, {}, {}, {},
        }
#endif
    },
};

HI_U32 SensorOs08a10Init(void)
{
    return MAPI_SENSOR_Register((HI_MAPI_COMBO_DEV_ATTR_S *)&g_Sensor_Cfg);
}

