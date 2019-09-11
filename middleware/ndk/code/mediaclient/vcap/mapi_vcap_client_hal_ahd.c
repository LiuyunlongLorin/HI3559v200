/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_client_hal_ahd.c
 * @brief   vcap client ahd function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "hi_ipcmsg.h"
#include "hi_comm_ipcmsg.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "msg_vcap.h"
#include "msg_define.h"
#include "msg_wrapper.h"
#include "mpi_sys.h"
#include "mapi_comm_inner.h"

HI_S32 HI_MAPI_VCAP_RegAhdModule(HI_HANDLE VcapDevHdl, HI_HAL_AHD_DEV_S *pstHalDev)
{
    MAPI_UNUSED(VcapDevHdl);
    MAPI_UNUSED(pstHalDev);
    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s is not support in linux.\n", __FUNCTION__);

    return HI_MAPI_VCAP_ENOTSUPPORT;
}

HI_S32 HI_MAPI_VCAP_AhdChangeChn(HI_HANDLE VcapDevHdl, HI_BOOL aszChn[], HI_U32 u32Cnt)
{
    HI_S32 s32Ret;
    MAPI_PRIV_DATA_S stPrivData;
    HI_U32 u32ModFd;
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(aszChn);
    if (u32Cnt == 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "u32Cnt should not be zero \n");
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    stPrivData.as32PrivData[0] = u32Cnt;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_AHD_CHANGE_CHN, aszChn, sizeof(HI_BOOL) * u32Cnt, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_AHD_CHANGE_CHN fail\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_AhdGetStatus(HI_HANDLE VcapDevHdl, HI_HAL_AHD_STATUS_E astAhdStatus[], HI_U32 u32Cnt)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(astAhdStatus);

    if (u32Cnt == 0 || u32Cnt > HI_HAL_AHD_CHN_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "u32Cnt:%d should be in (0,%u] \n", u32Cnt, HI_HAL_AHD_CHN_MAX);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    stPrivData.as32PrivData[0] = u32Cnt;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_AHD_GET_STATUS, astAhdStatus, sizeof(HI_HAL_AHD_STATUS_E) * u32Cnt,
                           &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_AHD_GET_STATUS fail\n");
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_UnRegAhdModule(HI_HANDLE VcapDevHdl)
{
    MAPI_UNUSED(VcapDevHdl);
    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "%s is not support in linux.\n", __FUNCTION__);

    return HI_MAPI_VCAP_ENOTSUPPORT;
}



