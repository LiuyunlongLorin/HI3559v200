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
    /* config 5M@30fps */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 2592,
            .height = 1944
        },
        .stSnsSize = {
            .u32Width = 2592,
            .u32Height = 1944
        },
        .mipi_attr =
        {
            .input_data_type = DATA_TYPE_RAW_12BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 4M@30fps */
    {
        .img_rect = {
            .x = 0,
            .y = 204,
            .width = 2592,
            .height = 1536
        },
        .stSnsSize = {
            .u32Width = 2592,
            .u32Height = 1944
        },
        .mipi_attr = {
            .input_data_type = DATA_TYPE_RAW_12BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 1440P@30fps */
    {
        .img_rect = {
            .x = 0,
            .y = 204,
            .width = 2560,
            .height = 1440
        },
        .stSnsSize = {
            .u32Width = 2592,
            .u32Height = 1944
        },
        .mipi_attr =
        {
            .input_data_type = DATA_TYPE_RAW_12BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_NONE,
        },
    },

    /* config 5M@30fps wdr */
    {
        .img_rect = {
            .x = 0,
            .y = 0,
            .width = 2592,
            .height = 1944
        },
        .stSnsSize = {
            .u32Width = 2592,
            .u32Height = 1944
        },
        .mipi_attr = {
            .input_data_type = DATA_TYPE_RAW_10BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_VC,
        },
    },

    /* config 4M@30fps wdr */
    {
        .img_rect = {
            .x = 0,
            .y = 204,
            .width = 2592,
            .height = 1536
        },
        .stSnsSize = {
            .u32Width = 2592,
            .u32Height = 1944
        },
        .mipi_attr = {
            .input_data_type = DATA_TYPE_RAW_10BIT,
            .wdr_mode = HI_MIPI_WDR_MODE_VC,
        },
    },
};

/* width and height must align with 4 bytes */
static const HI_MAPI_SENSOR_MODE_S g_Sensor_Mode[] = {
    /* SENSOR_MODE_5M_30FPS  frameRate:[1.0-30] */
    {
        .s32Width = 2592,
        .s32Height = 1944,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_NONE,
        .f32SnsMaxFrameRate = 30.0f
    },

    /* SENSOR_MODE_4M_30FPS  frameRate:[1.0-30] */
    {
        .s32Width = 2592,
        .s32Height = 1536,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_NONE,
        .f32SnsMaxFrameRate = 30.0f
    },

    /* SENSOR_MODE_1440P_30FPS  frameRate:[1.0-30] */
    {
        .s32Width = 2560,
        .s32Height = 1440,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_NONE,
        .f32SnsMaxFrameRate = 30.0f
    },

    /* SENSOR_MODE_5M_30FPS_WDR  frameRate:[1.0-30] */
    {
        .s32Width = 2592,
        .s32Height = 1944,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_2To1_LINE,
        .f32SnsMaxFrameRate = 30.0f
    },

    /* SENSOR_MODE_4M_30FPS_WDR  frameRate:[1.0-30] */
    {
        .s32Width = 2592,
        .s32Height = 1536,
        .u8SnsMode = 0,
        .enWdrMode = WDR_MODE_2To1_LINE,
        .f32SnsMaxFrameRate = 30.0f
    },
};

/* final interface here */
static HI_MAPI_COMBO_DEV_ATTR_S g_Sensor_Cfg = {
    /* config SENSOR */
    .pSensorObj = &stSnsImx335Obj,
    .s32SensorType = IMX335,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .stSensorInputAttr = {
        .enBayerFormat = BAYER_RGGB,
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
            { 0, 1, 2, 3 }, /* 5M@30FPS */
            { 0, 1, 2, 3 }, /* 4M@30FPS */
            { 0, 1, 2, 3 }, /* 1440P@30FPS */
            { 0, 1, 2, 3 }, /* 5M_30FPS_WDR */
            { 0, 1, 2, 3 }, /* 4M_30FPS_WDR */
        },

        /* No mipi_dev1 */
        {
            {}, {}, {}, {}, {}, {},

        }
#elif defined CFG_LANE_DIVIDE_MODE_1
        /* mipi_dev0 */
        {
            { 0, 2, -1, -1 }, /* 5M@30FPS */
            { 0, 2, -1, -1 }, /* 4M@30FPS */
            { 0, 2, -1, -1 }, /* 1440P@30FPS */
            { 0, 2, -1, -1 }, /* 5M_30FPS_WDR */
            { 0, 2, -1, -1 }, /* 4M_30FPS_WDR */
        },

        /* mipi_dev1 */
        {
            { 1, 3, -1, -1 }, /* 5M@30FPS */
            { 1, 3, -1, -1 }, /* 4M@30FPS */
            { 1, 3, -1, -1 }, /* 1440PM@30FPS */
            { 1, 3, -1, -1 }, /* 5M_30FPS_WDR */
            { 1, 3, -1, -1 }, /* 4M_30FPS_WDR */
        }
#endif
    },
};

HI_U32 SensorImx335Init(void)
{
    return MAPI_SENSOR_Register((HI_MAPI_COMBO_DEV_ATTR_S *)&g_Sensor_Cfg);
}
