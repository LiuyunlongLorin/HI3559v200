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

static const HI_MAPI_MIPI_INTF_S g_Sensor_MipiIntfCfg[] = {
    /* config 4M@30fps */
    {
        .img_rect = {
            .x = 16,
            .y = 204,
            .width = 2560,
            .height = 1600
        },
        .stSnsSize = {
            .u32Width = 2560,
            .u32Height = 1600
        },
        .mipi_attr =
        {
            .input_data_type = DATA_TYPE_RAW_12BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 4M@30fps wdr */
    {
        .img_rect = {
            .x = 16,
            .y = 204,
            .width = 2560,
            .height = 1600
        },
        .stSnsSize = {
            .u32Width = 2560,
            .u32Height = 1600
        },
        .mipi_attr = {
            .input_data_type = DATA_TYPE_RAW_10BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_VC,
        },
    },

};

/* width and height must align with 4 bytes */
static const HI_MAPI_SENSOR_MODE_S g_Sensor_Mode[] = {
    /* SENSOR_MODE_4M_30FPS  frameRate:[1.0-30] */
    {
        .s32Width = 2560,
        .s32Height = 1600,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_NONE,
        .f32SnsMaxFrameRate = 30.0f
    },

    /* SENSOR_MODE_4M_30FPS_WDR  frameRate:[1.0-30] */
    {
        .s32Width = 2560,
        .s32Height = 1600,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_2To1_LINE,
        .f32SnsMaxFrameRate = 30.0f
    },

};

/* final interface here */
static const HI_MAPI_COMBO_DEV_ATTR_S g_Sensor_Cfg = {
    /* config SENSOR */
    .pSensorObj = &stSnsOs05aObj,
    .s32SensorType = OS05A,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .stSensorInputAttr = {
        .enBayerFormat = BAYER_BGGR,
        .enSensorCommBusType = HI_SENSOR_COMMBUS_TYPE_I2C,
    },

    .stSensorMode = {
        .s32SensorModeCnt = sizeof(g_Sensor_Mode) / sizeof(HI_MAPI_SENSOR_MODE_S),
        .pSensorMode = g_Sensor_Mode,
        .pstIntf = g_Sensor_MipiIntfCfg
    },

    .aLaneId = {
#if defined CFG_LANE_DIVIDE_MODE_0
        /* mipi_dev0 */
        {
            { 0, 1, 2, 3 }, /* 4M@30FPS */
            { 0, 1, 2, 3 }, /* 4M_30FPS_WDR */
        },

        /* No mipi_dev1 */
        {
            {}, {}, {}, {}, {}, {},

        }
#elif defined CFG_LANE_DIVIDE_MODE_1
        /* mipi_dev0 */
        {
            { 0, 2, -1, -1 }, /* 4M@30FPS */
            { 0, 2, -1, -1 }, /* 4M_30FPS_WDR */
        },

        /* mipi_dev1 */
        {
            { 1, 3, -1, -1 }, /* 4M@30FPS */
            { 1, 3, -1, -1 }, /* 4M_30FPS_WDR */
        }
#endif
    },
};

HI_U32 SensorOs05aInit(void)
{
    return MAPI_SENSOR_Register((HI_MAPI_COMBO_DEV_ATTR_S *)&g_Sensor_Cfg);
}
