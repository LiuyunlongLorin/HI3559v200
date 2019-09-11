/**
* Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file    sensor_interface_cfg_params.h
* @brief   sensor interface cfg params  struct declaration
* @author  HiMobileCam NDK develop team
* @date    2018/2/6
*/

#ifndef __SENSOR_INTERFACE_CFG_PARAMS_H__
#define __SENSOR_INTERFACE_CFG_PARAMS_H__

#include "hi_mipi.h"
#include "hi_type.h"
#include "linux_list.h"
#include "hi_mapi_vcap_define.h"
#include "hi_comm_vi.h"
#include "mapi_vcap_inner.h"
#include "hi_comm_isp.h"
#include "sensor_interface_cfg_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if defined (__HI3556AV100__) || defined (__HI3519AV100__)
#define HI_MAPI_SENSOR_MAX_LANE_NUM (12)
#endif

#ifdef __HI3559AV100__
#define HI_MAPI_SENSOR_MAX_LANE_NUM (16)
#endif

#ifdef __HI3559V200__
#define HI_MAPI_SENSOR_MAX_LANE_NUM (4)
#endif

#ifdef __HI3518EV300__
#define HI_MAPI_SENSOR_MAX_LANE_NUM (4)
#endif

#define HI_MAPI_SENSOR_MAX_MODE_NUM (7)

/* sensor info */
#define UNUSED 0

#define IMX117      1
#define IMX277_SLVS 2
#define IMX377      3
#define IMX477      4
#define IMX290      5
#define IMX335      6
#define IMX458      7
#define IMX307      8
#define OS05A       9
#define OS08A10     10
#define GC2053      11
#define OS04B10     12
#define OV12870     13
#define BT656       20

typedef struct hiMAPI_SENSOR_COMM_CFG_S {
    lane_divide_mode_t enLaneIdMode;
    sns_clk_source_t aSnsClkSource[HI_MAPI_VCAP_MAX_DEV_NUM];
    sns_rst_source_t aSnsRstSource[HI_MAPI_VCAP_MAX_DEV_NUM];
    HI_S32 aBusId[HI_MAPI_VCAP_MAX_DEV_NUM];
} HI_MAPI_SENSOR_COMM_CFG_S;

typedef enum hiMAPI_SENSOR_COMMBUS_TYPE_E {
    HI_SENSOR_COMMBUS_TYPE_I2C = 0,
    HI_SENSOR_COMMBUS_TYPE_SPI,
    HI_SENSOR_COMMBUS_TYPE_BUTT
} HI_MAPI_SENSOR_COMMBUS_TYPE_E;

typedef struct hiMAPI_SENSOR_INPUT_ATTR_S {
    ISP_BAYER_FORMAT_E enBayerFormat;
    HI_MAPI_SENSOR_COMMBUS_TYPE_E enSensorCommBusType;
} HI_MAPI_SENSOR_INPUT_ATTR_S;

typedef struct hiMAPI_SENSOR_MODE_INFO_S {
    HI_S32 s32SensorModeCnt;
    const HI_MAPI_SENSOR_MODE_S *pSensorMode;
    const HI_MAPI_MIPI_INTF_S *pstIntf;
} HI_MAPI_SENSOR_MODE_INFO_S;

/* sensor interface description */
typedef struct hiMAPI_COMBO_DEV_ATTR_S {
    void *pSensorObj;
    HI_S32 s32SensorType;
    input_mode_t input_mode;
    mipi_data_rate_t data_rate;
    HI_MAPI_SENSOR_INPUT_ATTR_S stSensorInputAttr;
    HI_MAPI_SENSOR_MODE_INFO_S stSensorMode;
    short aLaneId[HI_MAPI_VCAP_MAX_DEV_NUM][HI_MAPI_SENSOR_MAX_MODE_NUM][HI_MAPI_SENSOR_MAX_LANE_NUM];
    struct cbb_list_head list;
} HI_MAPI_COMBO_DEV_ATTR_S;

HI_U32 SensorImx277Init(void);
HI_U32 SensorImx477Init(void);
HI_U32 SensorImx290Init(void);
HI_U32 SensorImx377Init(void);
HI_U32 SensorImx335Init(void);
HI_U32 SensorImx307Init(void);
HI_U32 SensorOs05aInit(void);
HI_U32 SensorOs08a10Init(void);
HI_U32 SensorGc2053Init(void);
HI_U32 SensorOs04b10Init(void);
HI_U32 SensorOV12870Init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* ARGPARSER_H__ */
