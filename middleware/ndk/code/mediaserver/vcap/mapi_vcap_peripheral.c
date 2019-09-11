/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_peripheral.c
 * @brief   server vcap peripheral module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mpi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "sensor_interface_cfg_params.h"
#include "mapi_comm_inner.h"

HI_S32 PeripheralMngSetAttr(HI_HANDLE VcapDevHdl, HI_S32 u32SensorType, const HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    HI_S32 s32Ret;
    HI_S32 sensorType = u32SensorType;
    if (sensorType != BT656) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d used sensorType:%d \n", VcapDevHdl, sensorType);
        return HI_SUCCESS;
    }

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (pstAhdInfo->pfnAhdSetAttr == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " vcapdev:%d , ahd fun callback is null ,should reg ahd first \n", VcapDevHdl);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    HI_HAL_AHD_CFG_S ahdCfg;
    if (pstSensorAttr->stSize.u32Width == 1280 && pstSensorAttr->stSize.u32Height == 720) {
        ahdCfg.enRes = HI_HAL_AHD_RES_720P;
    } else if (pstSensorAttr->stSize.u32Width == 1920 && pstSensorAttr->stSize.u32Height == 1080) {
        ahdCfg.enRes = HI_HAL_AHD_RES_1080P;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d , width:%u, height:%u this resolution is not support for ahd\n",
                       VcapDevHdl, pstSensorAttr->stSize.u32Width, pstSensorAttr->stSize.u32Height);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    /* snsmode and frame rate's relation is defined in sensor_interface_cfg_params.c file */
    if (pstSensorAttr->u8SnsMode == 0) {
        ahdCfg.enFps = HI_HAL_AHD_FPS_25;
    } else if (pstSensorAttr->u8SnsMode == 1) {
        ahdCfg.enFps = HI_HAL_AHD_FPS_30;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sns mode:%u is not support for ahd\n", pstSensorAttr->u8SnsMode);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = pstAhdInfo->pfnAhdSetAttr(pstAhdInfo, &ahdCfg);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral exec pfnAhdSetAttr failed, ret:%d\n", s32Ret);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 PeripheralMngStart(HI_HANDLE VcapDevHdl, HI_S32 u32SensorType)
{
    HI_S32 s32Ret;
    HI_S32 sensorType = u32SensorType;
    if (sensorType != BT656) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d used sensorType:%d \n", VcapDevHdl, sensorType);
        return HI_SUCCESS;
    }

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (pstAhdInfo->pfnAhdStart == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " vcapdev:%d , ahd fun callback is null ,should reg ahd first \n", VcapDevHdl);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    s32Ret = pstAhdInfo->pfnAhdStart(pstAhdInfo);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral exec pfnAhdStart failed, ret:%d\n", s32Ret);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 PeripheralMngStartChn(HI_HANDLE VcapDevHdl, HI_S32 u32SensorType, HI_U32 u32Chn)
{
    /* todo: u32Chn is mean ahd chn ; it will add the relation checking about vi pipe and ahd chn */
    /* ahd start chn and stop chn will not change bUsedChn status */
    MAPI_UNUSED(u32Chn);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 sensorType = u32SensorType;
    HI_U32 i = 0;
    HI_U32 u32UsedChn = 0;
    HI_U32 u32EnableChnCnt = 0;
    if (sensorType != BT656) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d used sensorType:%d \n", VcapDevHdl, sensorType);
        return HI_SUCCESS;
    }

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (pstAhdInfo->pfnAhdStartChn == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " vcapdev:%d , ahd fun callback is null ,should reg ahd first \n", VcapDevHdl);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    for (i = 0; i < pstAhdInfo->u32ChnMax; i++) {
        if (pstAhdInfo->bUsedChn[i]) {
            u32EnableChnCnt++;
            u32UsedChn = i;
        }
    }

    if (u32EnableChnCnt == 1) {
        s32Ret = pstAhdInfo->pfnAhdStartChn(pstAhdInfo, u32UsedChn);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral exec pfnAhdStartChn failed, chn:%u, ret:%d\n", u32UsedChn,
                s32Ret);
            return HI_MAPI_VCAP_EOPERATE_FAIL;
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral not support multi chn \n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 PeripheralMngStopChn(HI_HANDLE VcapDevHdl, HI_S32 u32SensorType, HI_U32 u32Chn)
{
    /* todo: u32Chn is mean ahd chn ; it will add the relation checking about vi pipe and ahd chn */
    /* ahd start chn and stop chn will not change bUsedChn status */
    MAPI_UNUSED(u32Chn);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 sensorType = u32SensorType;
    HI_U32 i = 0;
    HI_U32 u32UsedChn = 0;
    HI_U32 u32EnableChnCnt = 0;
    if (sensorType != BT656) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d used sensorType:%d \n", VcapDevHdl, sensorType);
        return HI_SUCCESS;
    }

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (pstAhdInfo->pfnAhdStopChn == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " vcapdev:%d , ahd fun callback is null ,should reg ahd first \n", VcapDevHdl);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    for (i = 0; i < pstAhdInfo->u32ChnMax; i++) {
        if (pstAhdInfo->bUsedChn[i]) {
            u32EnableChnCnt++;
            u32UsedChn = i;
        }
    }

    if (u32EnableChnCnt == 1) {
        s32Ret = pstAhdInfo->pfnAhdStopChn(pstAhdInfo, u32UsedChn);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral exec pfnAhdStartChn failed, chn:%u, ret:%d\n", u32UsedChn,
                s32Ret);
            return HI_MAPI_VCAP_EOPERATE_FAIL;
        }
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral not support multi chn \n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 PeripheralMngStop(HI_HANDLE VcapDevHdl, HI_S32 u32SensorType)
{
    HI_S32 s32Ret;
    HI_S32 sensorType = u32SensorType;
    if (sensorType != BT656) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d used sensorType:%d \n", VcapDevHdl, sensorType);
        return HI_SUCCESS;
    }

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (pstAhdInfo->pfnAhdStop == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " vcapdev:%d , ahd fun callback is null ,should reg ahd first \n", VcapDevHdl);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    s32Ret = pstAhdInfo->pfnAhdStop(pstAhdInfo);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Peripheral exec pfnAhdStart failed, ret:%d\n", s32Ret);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

