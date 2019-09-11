/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_adapt.c
 * @brief   server vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <sys/prctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "hi_mapi_sys.h"
#include "mapi_comm_inner.h"
#include "hi_mapi_log.h"
#include "sensor_interface_cfg_adapt.h"
#include "hi_comm_vi.h"
#include "hi_mipi.h"
#include "mapi_vcap_inner.h"
#include "mapi_vcap_arch.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_VCAP_EnableMipiClock(HI_S32 s32MipiFd, const combo_dev_attr_t *pstComboDevAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (pstComboDevAttr->input_mode != INPUT_MODE_MIPI && pstComboDevAttr->input_mode != INPUT_MODE_BT656) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "only suppport INPUT_MODE_MIPI or INPUT_MODE_BT656 \n");
        return HI_MAPI_VCAP_ENOT_PERM;
    } else {
        s32Ret = ioctl(s32MipiFd, HI_MIPI_ENABLE_MIPI_CLOCK, &pstComboDevAttr->devno);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "enable mipi failed\n");
            return s32Ret;
        }

        s32Ret = ioctl(s32MipiFd, HI_MIPI_RESET_MIPI, &pstComboDevAttr->devno);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "reset mipi failed\n");
            return s32Ret;
        }
    }

    return s32Ret;
}

HI_S32 HAI_MAPI_VCAP_DisableMipiClock(HI_HANDLE VcapDevHdl, HI_S32 s32MipiFd, input_mode_t enInputMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE devno = VcapDevHdl;

    if (enInputMode != INPUT_MODE_MIPI && enInputMode != INPUT_MODE_BT656) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "only suppport INPUT_MODE_MIPI or INPUT_MODE_BT656 \n");
        return HI_MAPI_VCAP_ENOT_PERM;
    } else {
        s32Ret = ioctl(s32MipiFd, HI_MIPI_RESET_MIPI, &devno);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "reset mipi failed\n");
            return s32Ret;
        }

        s32Ret = ioctl(s32MipiFd, HI_MIPI_DISABLE_MIPI_CLOCK, &devno);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "disable mipi clock failed\n");
            return s32Ret;
        }
    }

    return s32Ret;
}

HI_S32 HAL_MAPI_VCAP_ResetMipiClock(HI_S32 s32MipiFd, const combo_dev_attr_t *pstComboDevAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (pstComboDevAttr->input_mode != INPUT_MODE_MIPI && pstComboDevAttr->input_mode != INPUT_MODE_BT656) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "only suppport INPUT_MODE_MIPI or INPUT_MODE_BT656 \n");
        return HI_MAPI_VCAP_ENOT_PERM;
    } else {
        s32Ret = ioctl(s32MipiFd, HI_MIPI_UNRESET_MIPI, &pstComboDevAttr->devno);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ureset mipi failed\n");
            return s32Ret;
        }
    }
    return s32Ret;
}

data_type_t HAI_MAPI_VCAP_GetSnsDataType(const HI_MAPI_MIPI_INTF_S *pstSensorInf, input_mode_t enInputMode)
{
    data_type_t stSnsDataType = DATA_TYPE_BUTT;

    switch (enInputMode) {
        case INPUT_MODE_MIPI:
            stSnsDataType = pstSensorInf->mipi_attr.input_data_type;
            break;

        case INPUT_MODE_LVDS:
            stSnsDataType = pstSensorInf->lvds_attr.input_data_type;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "input mode:%d is not valid !\n", enInputMode);
            break;
    }
    return stSnsDataType;
}

HI_S32 HAI_MAPI_VCAP_LoadMipiConfig(combo_dev_attr_t *pComboDevAttr, const HI_MAPI_MIPI_INTF_S *pstSensorInf,
                                    const input_mode_t enInputMode, short aLaneId[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    switch (enInputMode) {
        case INPUT_MODE_LVDS:

            memcpy(&pComboDevAttr->lvds_attr, &pstSensorInf->lvds_attr, sizeof(lvds_dev_attr_t));

            for (i = 0; i < LVDS_LANE_NUM; i++) {
                pComboDevAttr->lvds_attr.lane_id[i] = aLaneId[i];
            }
            break;

        case INPUT_MODE_MIPI:

            memcpy(&pComboDevAttr->mipi_attr, &pstSensorInf->mipi_attr, sizeof(mipi_dev_attr_t));

            for (i = 0; i < MIPI_LANE_NUM; i++) {
                pComboDevAttr->mipi_attr.lane_id[i] = aLaneId[i];
            }
            break;
        case INPUT_MODE_BT656:
            break;

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sensor input mode error,please check!\n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }
    }

    return HI_SUCCESS;
}

VI_INTF_MODE_E HAI_MAPI_VCAP_GetIntfMode(input_mode_t enInputMode)
{
    VI_INTF_MODE_E enViIntfMode = VI_MODE_BUTT;

    switch (enInputMode) {
        case INPUT_MODE_MIPI:
            enViIntfMode = VI_MODE_MIPI;
            break;

        case INPUT_MODE_LVDS:
        case INPUT_MODE_SUBLVDS:
            enViIntfMode = VI_MODE_LVDS;
            break;
        case INPUT_MODE_HISPI:
            enViIntfMode = VI_MODE_HISPI;
            break;
        case INPUT_MODE_BT656:
            enViIntfMode = VI_MODE_BT656;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "input mode:%d is not valid !\n", enInputMode);
            break;
    }
    return enViIntfMode;
}

HI_S32 HAI_MAPI_VCAP_EnableSnapPipe(HI_HANDLE VcapPipeHdl, MAPI_VCAP_GLOB_ATTR_S *pstGobalAttr)
{
    VI_PIPE ViPipe;
    VI_DEV ViDev;
    HI_S32 s32Ret = HI_SUCCESS;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)pstGobalAttr->astGlobPipeAttr[ViPipe].BindDevHdl;

    /* in hi3559v200, suport single snap pipe */
    if ((pstGobalAttr->astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP)
        && (pstGobalAttr->astGlobPipeAttr[ViPipe].bSnapPipeEnable == HI_FALSE)
        && (pstGobalAttr->astGlobDevAttr[ViDev].u32PipeBindNum != 1)) {
        s32Ret = HI_MPI_SNAP_EnablePipe(ViPipe);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call HI_MPI_SNAP_EnablePipe fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            return s32Ret;
        }
    }
    pstGobalAttr->astGlobPipeAttr[ViPipe].bSnapPipeEnable = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HAI_MAPI_VCAP_DisableSnapPipe(HI_HANDLE VcapPipeHdl, MAPI_VCAP_GLOB_ATTR_S *pstGobalAttr)
{
    VI_PIPE ViPipe;
    VI_DEV ViDev;
    HI_S32 s32Ret = HI_SUCCESS;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)pstGobalAttr->astGlobPipeAttr[ViPipe].BindDevHdl;

    /* Hi3559V200 support single snap piper */
    if ((pstGobalAttr->astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP)
        && (pstGobalAttr->astGlobPipeAttr[ViPipe].bSnapPipeEnable == HI_TRUE)
        && (pstGobalAttr->astGlobDevAttr[ViDev].u32PipeBindNum != 1)) {
        s32Ret = HI_MPI_SNAP_DisablePipe(ViPipe);
        CHECK_MAPI_VCAP_RET(s32Ret, "disable snap fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }
    pstGobalAttr->astGlobPipeAttr[ViPipe].bSnapPipeEnable = HI_FALSE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
