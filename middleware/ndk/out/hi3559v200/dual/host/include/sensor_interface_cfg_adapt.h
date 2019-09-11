/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file    sensor_interface_cfg_adapt.h
* @brief   sensor interface cfg params  struct declaration
* @author  HiMobileCam NDK develop team
* @date    2018-9-12
*/

#ifndef __SENSOR_INTERFACE_CFG_ADAPT_H__
#define __SENSOR_INTERFACE_CFG_ADAPT_H__

#include "hi_mipi.h"
#include "hi_mapi_vcap_define.h"
#include "hi_comm_vi.h"
#include "hi_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct hiMAPI_MIPI_INTF_S {
    img_rect_t img_rect; /* it is related to mipi image_rect */
    SIZE_S stSnsSize;    /* it is related to sensor SIZE_S */
    union {
        mipi_dev_attr_t mipi_attr;
        lvds_dev_attr_t lvds_attr;
    };
} HI_MAPI_MIPI_INTF_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* ARGPARSER_H__ */
