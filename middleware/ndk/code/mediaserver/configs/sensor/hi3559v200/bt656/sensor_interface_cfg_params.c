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
    /* config 720p25 */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 1280,
            .height = 720
        },
        .stSnsSize = {
            .u32Width = 1280,
            .u32Height = 720
        },
        .mipi_attr = {
            .input_data_type = 0,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 720p30 */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 1280,
            .height = 720
        },
        .stSnsSize = {
            .u32Width = 1280,
            .u32Height = 720
        },
        .mipi_attr = {
            .input_data_type = 0,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 1080p25 */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 1920,
            .height = 1080
        },
        .stSnsSize = {
            .u32Width = 1920,
            .u32Height = 1080
        },
        .mipi_attr = {
            .input_data_type = 0,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 1080p30 */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 1920,
            .height = 1080
        },
        .stSnsSize = {
            .u32Width = 1920,
            .u32Height = 1080
        },
        .mipi_attr = {
            .input_data_type = 0,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },
};

/* width and height must align with 4 bytes */
static const HI_MAPI_SENSOR_MODE_S g_Sensor_Mode[] = {
    /* SENSOR_MODE_720p_25FPS  frameRate:[1.0-25] */
    {
        .s32Width = 1280,
        .s32Height = 720,
        .enWdrMode = WDR_MODE_NONE,
        .u8SnsMode = 0,
        .f32SnsMaxFrameRate = 25.0f,
    },

    /* SENSOR_MODE 720p_30FPS frameRate:[1.0-30] */
    {
        .s32Width = 1280,
        .s32Height = 720,
        .enWdrMode = WDR_MODE_NONE,
        .u8SnsMode = 1,
        .f32SnsMaxFrameRate = 30.0f,
    },

    /* SENSOR_MODE_1080p_25FPS frameRate:[1.0-25] */
    {
        .s32Width = 1920,
        .s32Height = 1080,
        .enWdrMode = WDR_MODE_NONE,
        .u8SnsMode = 0,
        .f32SnsMaxFrameRate = 25.0f,
    },
    /* SENSOR_MODE_1080p_30FPS frameRate:[1.0-30] */
    {
        .s32Width = 1920,
        .s32Height = 1080,
        .enWdrMode = WDR_MODE_NONE,
        .u8SnsMode = 1,
        .f32SnsMaxFrameRate = 30.0f,
    },
};

/* final interface here */
static HI_MAPI_COMBO_DEV_ATTR_S g_Sensor_Cfg = {
    /* config SENSOR */
    .pSensorObj = 0,
    .s32SensorType = BT656,
    .input_mode = INPUT_MODE_BT656,
    .data_rate = MIPI_DATA_RATE_X1,
    .stSensorInputAttr = {
        .enBayerFormat = 0,
        .enSensorCommBusType = 0,
    },

    .stSensorMode = {
        .s32SensorModeCnt = sizeof(g_Sensor_Mode) / sizeof(HI_MAPI_SENSOR_MODE_S),
        .pSensorMode = g_Sensor_Mode,
        .pstIntf = g_Sensor_MipiIntfCfg
    },

    .aLaneId = {

    },
};

HI_U32 SensorBT656Init(void)
{
    return MAPI_SENSOR_Register((HI_MAPI_COMBO_DEV_ATTR_S *)&g_Sensor_Cfg);
}
