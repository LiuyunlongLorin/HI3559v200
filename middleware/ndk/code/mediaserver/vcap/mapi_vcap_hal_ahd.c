/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_hal_ahd.c
 * @brief   server vcap ahd module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include "mpi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "mapi_comm_inner.h"

static HI_HAL_AHD_DEV_S g_astHalAhdDev[HI_MAPI_VCAP_MAX_DEV_NUM] = {0};
HI_HAL_AHD_DEV_S *GetVcapAhdDevInfo(HI_HANDLE VcapDevHdl)
{
    return &(g_astHalAhdDev[VcapDevHdl]);
}

HI_S32 HI_MAPI_VCAP_RegAhdModule(HI_HANDLE VcapDevHdl, HI_HAL_AHD_DEV_S *pstHalDev)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstHalDev);
    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    MAPI_GLOB_DEV_ATTR_S *pstDevAtr = GetVcapDevAttr(VcapDevHdl);
    HI_U32 i;
    HI_U32 u32EnableChnCnt = 0;
    if (pstDevAtr->bSensorInit == HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP,
            "vcapdev:%d, if you want use ahd dev, then you should reg ahd info before sensor init, \n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (pstAhdInfo->pfnAhdInit != NULL) {
        /* consider the quick start mode */
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d has registered ahd dev info\n", VcapDevHdl);
        return HI_SUCCESS;
    }

    if (pstHalDev->u32ChnMax == 0 || pstHalDev->u32ChnMax > HI_HAL_AHD_CHN_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d ,u32ChnMax:%d should not be (0,%d] \n", VcapDevHdl,
                       pstAhdInfo->u32ChnMax, HI_HAL_AHD_CHN_MAX);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    for (i = 0; i < pstHalDev->u32ChnMax; i++) {
        if (pstHalDev->bUsedChn[i]) {
            u32EnableChnCnt++;
        }
    }

    if (pstHalDev->pfnAhdInit == NULL || pstHalDev->pfnAhdDeinit == NULL || pstHalDev->pfnAhdStart == NULL
        || pstHalDev->pfnAhdStop == NULL || pstHalDev->pfnAhdStartChn == NULL || pstHalDev->pfnAhdStopChn == NULL
        || pstHalDev->pfnAhdSetAttr == NULL || pstHalDev->pfnAhdGetAttr == NULL ||
        pstHalDev->pfnAhdGetStatus == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d ,callback function should not be null \n", VcapDevHdl);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    if (u32EnableChnCnt != 1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "not support ahd multi chn, used chn count:%d \n", u32EnableChnCnt);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }
    memcpy(pstAhdInfo, pstHalDev, sizeof(HI_HAL_AHD_DEV_S));

    s32Ret = pstAhdInfo->pfnAhdInit(pstAhdInfo);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d ,Peripheral exec pfnAhdInit failed, ret:%d\n", VcapDevHdl, s32Ret);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_AhdChangeChn(HI_HANDLE VcapDevHdl, HI_BOOL aszChn[], HI_U32 u32Cnt)
{
    HI_S32 s32Ret;
    HI_U32 i;
    HI_U32 u32EnableChnCnt = 0;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(aszChn);

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    if (u32Cnt < pstAhdInfo->u32ChnMax) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "aszChn channel array count should be equal to u32ChnMax:%d \n",
                       pstAhdInfo->u32ChnMax);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    if (pstAhdInfo->pfnAhdStartChn == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d has not register ahd dev info\n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    for (i = 0; i < pstAhdInfo->u32ChnMax; i++) {
        if (aszChn[i]) {
            u32EnableChnCnt++;
        }
    }

    if (u32EnableChnCnt != 1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "not support ahd multi chn, used chn count:%d \n", u32EnableChnCnt);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    for (i = 0; i < pstAhdInfo->u32ChnMax; i++) {
        if (pstAhdInfo->bUsedChn[i] != aszChn[i]) {
            if (aszChn[i]) {
                s32Ret = pstAhdInfo->pfnAhdStartChn(pstAhdInfo, i);
                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "exec pfnAhdStartChn failed, chn:%u, ret:%d\n", i, s32Ret);
                    return HI_MAPI_VCAP_EOPERATE_FAIL;
                }
            } else {
                s32Ret = pstAhdInfo->pfnAhdStopChn(pstAhdInfo, i);
                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "exec pfnAhdStopChn failed, chn:%u, ret:%d\n", i, s32Ret);
                    return HI_MAPI_VCAP_EOPERATE_FAIL;
                }
            }
            /* only change chn can update bUsedChn array */
            pstAhdInfo->bUsedChn[i] = aszChn[i];
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_AhdGetStatus(HI_HANDLE VcapDevHdl, HI_HAL_AHD_STATUS_E astAhdStatus[], HI_U32 u32Cnt)
{
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(astAhdStatus);

    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    MAPI_GLOB_DEV_ATTR_S *pstDevAtr = GetVcapDevAttr(VcapDevHdl);

    if (u32Cnt == 0 || u32Cnt > HI_HAL_AHD_CHN_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d ,u32Cnt:%u is illegal \n", VcapDevHdl, u32Cnt);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    if (pstAhdInfo->pfnAhdGetStatus == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d has not register ahd dev info\n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    HI_HAL_AHD_STATUS_E astAhdStatusTmp[HI_HAL_AHD_CHN_MAX];
    HI_S32 s32Ret = pstAhdInfo->pfnAhdGetStatus(pstAhdInfo, astAhdStatusTmp, HI_HAL_AHD_CHN_MAX);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d ahd get status failed:%d \n", VcapDevHdl, s32Ret);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    HI_U32 i = 0;
    for (; i < u32Cnt; i++) {
        astAhdStatus[i] = astAhdStatusTmp[i];
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_UnRegAhdModule(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret;
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    HI_HAL_AHD_DEV_S *pstAhdInfo = GetVcapAhdDevInfo(VcapDevHdl);
    MAPI_GLOB_DEV_ATTR_S *pstDevAtr = GetVcapDevAttr(VcapDevHdl);

    if (pstDevAtr->bSensorInit == HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d, you should unregister sensor before ahd unregister, \n",
            VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (pstAhdInfo->pfnAhdDeinit == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d has unregistered ahd dev info\n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = pstAhdInfo->pfnAhdDeinit(pstAhdInfo);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcapdev:%d, exec pfnAhdDeinit failed, ret:%d\n", VcapDevHdl, s32Ret);
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    memcpy(pstAhdInfo, 0, sizeof(HI_HAL_AHD_DEV_S));
    return HI_SUCCESS;
}
