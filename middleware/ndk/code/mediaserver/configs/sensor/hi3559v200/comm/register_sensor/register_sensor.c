/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file    register_sensor.c
* @brief   register sensor functions
* @author  HiMobileCam NDK develop team
* @date    2018-2-6
*/
#include <stdio.h>
#include <sensor_comm.h>
#include <sensor_interface_cfg_params.h>
#include "linux_list.h"
#include "mapi_vcap_inner.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static LIST_HEAD(s_stSensorCfgHead);

/* Register the sensor configuration, the sensor structure will be placed in the list */
HI_S32 MAPI_SENSOR_Register(HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg)
{
    cbb_list_add_tail(&pstSensorCfg->list, &s_stSensorCfgHead);

    return HI_SUCCESS;
}

HI_S32 MAPI_SENSOR_GetConfig(HI_S32 s32SensorType, HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg)
{
    HI_MAPI_COMBO_DEV_ATTR_S *pstTmp;

    cbb_list_for_each_entry(pstTmp, &s_stSensorCfgHead, list)
    {
        if (pstTmp->s32SensorType == s32SensorType) {
            *pstSensorCfg = *pstTmp;
            return HI_SUCCESS;
        }
    }

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "do not have this sensor type:%d !\n", s32SensorType);
    return HI_FAILURE;
}

HI_S32 MAPI_SENSOR_Load(HI_VOID)
{
    HI_S32 s32Ret;

    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "sensor param load begin...\n");

#ifdef SENSOR_IMX477
    s32Ret = SensorImx477Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx477 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx477 param load success!\n");
#endif

#ifdef SENSOR_IMX377
    s32Ret = SensorImx377Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx377 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx377 param load success!\n");
#endif

#ifdef SENSOR_IMX290
    s32Ret = SensorImx290Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx290 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx290 param load success!\n");
#endif

#ifdef SENSOR_IMX335
    s32Ret = SensorImx335Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx335 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx335 param load success!\n");
#endif

#ifdef SENSOR_IMX458
    s32Ret = SensorImx458Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx458 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx458 param load success!\n");
#endif

#ifdef SENSOR_IMX307
    s32Ret = SensorImx307Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorImx307 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorImx307 param load success!\n");
#endif

#ifdef SENSOR_OS05A
    s32Ret = SensorOs05aInit();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorOs05a param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorOs05a param load success!\n");
#endif

#ifdef SENSOR_OS08A10
    s32Ret = SensorOs08a10Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorOs08a10 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorOs08a10 param load success!\n");
#endif

#ifdef SENSOR_OS04B10
    s32Ret = SensorOs04b10Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorOs04b10 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorOs04b10 param load success!\n");
#endif

#ifdef SENSOR_OV12870
    s32Ret = SensorOV12870Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "SensorOV12870 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "SensorOV12870 param load success!\n");
#endif

#ifdef SENSOR_BT656
    s32Ret = SensorBT656Init();
    CHECK_MAPI_VCAP_RET(s32Ret, "BT656 param load fail!!!");
    MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "BT656 param load success!\n");
#endif
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
