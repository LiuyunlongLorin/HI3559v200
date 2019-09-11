/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_arch.h
 * @brief   server vcap arch module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_VCAP_ARCH_H__
#define __MAPI_VCAP_ARCH_H__

#include "hi_mapi_sys.h"
#include "mapi_vcap_inner.h"
#ifdef SUPPORT_GYRO
#include "hi_comm_motionfusion.h"
#include "hi_comm_motionsensor.h"
#include "mpi_motionfusion.h"
#include "motionsensor_chip_cmd.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_VCAP_EnableMipiClock(HI_S32 s32MipiFd, const combo_dev_attr_t *pstComboDevAttr);

HI_S32 HAL_MAPI_VCAP_ResetMipiClock(HI_S32 s32MipiFd, const combo_dev_attr_t *pstComboDevAttr);

HI_S32 HAI_MAPI_VCAP_DisableMipiClock(HI_HANDLE VcapDevHdl, HI_S32 s32MipiFd, input_mode_t enInputMode);

data_type_t HAI_MAPI_VCAP_GetSnsDataType(const HI_MAPI_MIPI_INTF_S *pstSensorInf, input_mode_t enInputMode);

VI_INTF_MODE_E HAI_MAPI_VCAP_GetIntfMode(input_mode_t enInputMode);

HI_S32 HAI_MAPI_VCAP_LoadMipiConfig(combo_dev_attr_t *pComboDevAttr, const HI_MAPI_MIPI_INTF_S *pstSensorInf,
                                    const input_mode_t enInputMode, short aLaneId[]);

HI_S32 HAI_MAPI_VCAP_EnableSnapPipe(HI_HANDLE VcapPipeHdl, MAPI_VCAP_GLOB_ATTR_S *pstGobalAttr);

HI_S32 HAI_MAPI_VCAP_DisableSnapPipe(HI_HANDLE VcapPipeHdl, MAPI_VCAP_GLOB_ATTR_S *pstGobalAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_VCAP_ARCH_H__ */
