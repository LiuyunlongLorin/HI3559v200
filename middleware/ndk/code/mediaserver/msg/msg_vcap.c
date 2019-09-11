/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_vcap.c
 * @brief   msg vcap function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_vcap.h"
#include "hi_mapi_vcap_define.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "msg_server.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 MSG_VCAP_InitSensor(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_InitSensor(VcapDevHdl, (HI_MAPI_SENSOR_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_DeinitSensor(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_DeinitSensor(VcapDevHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSensorAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetSensorAttr(VcapDevHdl, (HI_MAPI_SENSOR_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_SENSOR_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetSensorFrameRate(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetSensorFrameRate(VcapDevHdl, (HI_FLOAT *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSensorFrameRate(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetSensorFrameRate(VcapDevHdl, (HI_FLOAT *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_S32));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSensorModesCnt(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetSensorModesCnt(VcapDevHdl, (HI_S32 *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_S32));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSensorAllModes(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);
    s32Ret = HI_MAPI_VCAP_GetSensorAllModes(VcapDevHdl, (HI_S32)pstMsg->as32PrivData[0],
                                            (HI_MAPI_SENSOR_MODE_S *)pstMsg->pBody);

    if (s32Ret == HI_SUCCESS) {
        respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody,
                                              sizeof(HI_MAPI_SENSOR_MODE_S) * pstMsg->as32PrivData[0]);
        CHECK_MAPI_VCAP_NULL_PTR(respMsg);

        s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
            HI_IPCMSG_DestroyMessage(respMsg);
            return s32Ret;
        }

        HI_IPCMSG_DestroyMessage(respMsg);
    } else {
        respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
        CHECK_MAPI_VCAP_NULL_PTR(respMsg);

        s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
            HI_IPCMSG_DestroyMessage(respMsg);
            return s32Ret;
        }

        HI_IPCMSG_DestroyMessage(respMsg);
    }

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetAttr(VcapDevHdl, (HI_MAPI_VCAP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetAttr(VcapDevHdl, (HI_MAPI_VCAP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_VCAP_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StartDev(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StartDev(VcapDevHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StopDev(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StopDev(VcapDevHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StartChn(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StartChn(VcapPipeHdl, PipeChnHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StopChn(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StopChn(VcapPipeHdl, PipeChnHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StartISP(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StartISP(VcapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StopISP(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StopISP(VcapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetSnapAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetSnapAttr(VcapPipeHdl, (HI_MAPI_VCAP_SNAP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSnapAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetSnapAttr(VcapPipeHdl, (HI_MAPI_VCAP_SNAP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StartTrigger(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StartTrigger(VcapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StopTrigger(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StopTrigger(VcapPipeHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetStitchAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapStitchHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapStitchHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetStitchAttr(VcapStitchHdl, (VI_STITCH_GRP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetStitchAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapStitchHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapStitchHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetStitchAttr(VcapStitchHdl, (VI_STITCH_GRP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(VI_STITCH_GRP_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StitchTrigger(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapStitchHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapStitchHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_StitchTrigger(VcapStitchHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetExifInfo(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetExifInfo(VcapPipeHdl, (HI_MAPI_SNAP_EXIF_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetExifInfo(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetExifInfo(VcapPipeHdl, (HI_MAPI_SNAP_EXIF_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_SNAP_EXIF_INFO_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, PipeChnHdl, pstMsg->as32PrivData[0], pstMsg->pBody,
                                    pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetAttrEx(VcapPipeHdl, PipeChnHdl, pstMsg->as32PrivData[0], pstMsg->pBody,
                                    pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->as32PrivData[1]);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetChnCropAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetChnCropAttr(VcapPipeHdl, PipeChnHdl, (VI_CROP_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetChnCropAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetChnCropAttr(VcapPipeHdl, PipeChnHdl, (VI_CROP_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(VI_CROP_INFO_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetDumpRawAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetDumpRawAttr(VcapPipeHdl, (HI_MAPI_DUMP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetDumpRawAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetDumpRawAttr(VcapPipeHdl, (HI_MAPI_DUMP_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_DUMP_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCap_GetRawFrame(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32DataNum = 0;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    /* pstMsg->as32PrivData[1] saves the bBNRraw parameter which is received from
           the client to determine whether it is BNR raw. */
    s32Ret = MAPI_VCAP_GetRawFrame(VcapPipeHdl, (HI_MAPI_FRAME_DATA_S *)pstMsg->pBody, &s32DataNum,
                                   pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody,
                                          (sizeof(HI_MAPI_FRAME_DATA_S) * HI_VCAP_RAW_EACHGROUP_MAX_NUM));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    /* respMsg->as32PrivData[0] saves the s32DataNum value obtained from the function and
           then send it to the client by IPCMSG */
    respMsg->as32PrivData[0] = s32DataNum;

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCap_ReleaseRawFrame(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    /* pstMsg->as32PrivData[1] saves the bBNRraw parameter which is received from the client to determine whether it is BNR raw.
       pstMsg->as32PrivData[0] saves the s32DataNum parameter which is received from the client to determine How many frames of
       raw data one group */
    s32Ret = MAPI_VCAP_ReleaseRawFrame(VcapPipeHdl, (HI_MAPI_FRAME_DATA_S *)pstMsg->pBody, pstMsg->as32PrivData[0],
                                       pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetChnOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_HANDLE OSDHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    OSDHdl = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_VCAP_SetChnOSDAttr(VcapPipeHdl, PipeChnHdl, OSDHdl, (HI_MAPI_OSD_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetChnOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_HANDLE OSDHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    OSDHdl = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_VCAP_GetChnOSDAttr(VcapPipeHdl, PipeChnHdl, OSDHdl, (HI_MAPI_OSD_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_OSD_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StartChnOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_HANDLE OSDHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    OSDHdl = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_VCAP_StartChnOSD(VcapPipeHdl, PipeChnHdl, OSDHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_StopChnOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_HANDLE OSDHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    OSDHdl = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_VCAP_StopChnOSD(VcapPipeHdl, PipeChnHdl, OSDHdl);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetSnapAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetSnapAttrEx(VcapPipeHdl, pstMsg->as32PrivData[0], pstMsg->pBody, pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetSnapAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetSnapAttrEx(VcapPipeHdl, pstMsg->as32PrivData[0], pstMsg->pBody, pstMsg->as32PrivData[1]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->as32PrivData[1]);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_SetChnDISAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_SetChnDISAttr(VcapPipeHdl, PipeChnHdl, (HI_MAPI_VCAP_DIS_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetChnDISAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_VCAP_GetChnDISAttr(VcapPipeHdl, PipeChnHdl, (HI_MAPI_VCAP_DIS_ATTR_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_VCAP_DIS_ATTR_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_InitMotionSensor(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VCAP_InitMotionSensor((HI_MAPI_MOTIONSENSOR_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_DeInitMotionSensor(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VCAP_DeInitMotionSensor((HI_MAPI_MOTIONSENSOR_INFO_S *)pstMsg->pBody);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_MAPI_MOTIONSENSOR_INFO_S));
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_GetAhdStatus(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    HI_U32 u32Cnt = (HI_U32)pstMsg->as32PrivData[0];
    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);
    s32Ret = HI_MAPI_VCAP_AhdGetStatus(VcapDevHdl, (HI_HAL_AHD_STATUS_E *)pstMsg->pBody, u32Cnt);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(HI_HAL_AHD_STATUS_E) * u32Cnt);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VCAP_ChangeChn(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapDevHdl;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    VcapDevHdl = GET_DEV_ID(pstMsg->u32Module);
    s32Ret = HI_MAPI_VCAP_AhdChangeChn(VcapDevHdl, (HI_BOOL *)pstMsg->pBody, (HI_U32)pstMsg->as32PrivData[0]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    CHECK_MAPI_VCAP_NULL_PTR(respMsg);

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_SENSOR_INIT,              MSG_VCAP_InitSensor },
    { MSG_CMD_SENSOR_DEINIT,            MSG_VCAP_DeinitSensor },
    { MSG_CMD_SENSOR_GETATTR,           MSG_VCAP_GetSensorAttr },
    { MSG_CMD_SENSOR_SET_FRAMERATE,     MSG_VCAP_SetSensorFrameRate },
    { MSG_CMD_SENSOR_GET_FRAMERATE,     MSG_VCAP_GetSensorFrameRate },
    { MSG_CMD_SENSOR_GET_MODESCNT,      MSG_VCAP_GetSensorModesCnt },
    { MSG_CMD_SENSOR_GET_ALLMODES,      MSG_VCAP_GetSensorAllModes },
    { MSG_CMD_VCAP_SET_ATTR,            MSG_VCAP_SetAttr },
    { MSG_CMD_VCAP_GET_ATTR,            MSG_VCAP_GetAttr },
    { MSG_CMD_VCAP_START_DEV,           MSG_VCAP_StartDev },
    { MSG_CMD_VCAP_STOP_DEV,            MSG_VCAP_StopDev },
    { MSG_CMD_VCAP_START_CHN,           MSG_VCAP_StartChn },
    { MSG_CMD_VCAP_STOP_CHN,            MSG_VCAP_StopChn },
    { MSG_CMD_VCAP_START_ISP,           MSG_VCAP_StartISP },
    { MSG_CMD_VCAP_STOP_ISP,            MSG_VCAP_StopISP },
    { MSG_CMD_VCAP_SET_SNAPATTR,        MSG_VCAP_SetSnapAttr },
    { MSG_CMD_VCAP_GET_SNAPATTR,        MSG_VCAP_GetSnapAttr },
    { MSG_CMD_VCAP_SET_SNAPATTR_EX,     MSG_VCAP_SetSnapAttrEx },
    { MSG_CMD_VCAP_GET_SNAPATTR_EX,     MSG_VCAP_GetSnapAttrEx },
    { MSG_CMD_VCAP_START_TRIGGER,       MSG_VCAP_StartTrigger },
    { MSG_CMD_VCAP_STOP_TRIGGER,        MSG_VCAP_StopTrigger },
    { MSG_CMD_VCAP_SET_STITCHATTR,      MSG_VCAP_SetStitchAttr },
    { MSG_CMD_VCAP_GET_STITCHATTR,      MSG_VCAP_GetStitchAttr },
    { MSG_CMD_VCAP_STITCH_TRIGGER,      MSG_VCAP_StitchTrigger },
    { MSG_CMD_VCAP_SET_ATTR_EX,         MSG_VCAP_SetAttrEx },
    { MSG_CMD_VCAP_GET_ATTR_EX,         MSG_VCAP_GetAttrEx },
    { MSG_CMD_VCAP_SET_CROPATTR,        MSG_VCAP_SetChnCropAttr },
    { MSG_CMD_VCAP_GET_CROPATTR,        MSG_VCAP_GetChnCropAttr },
    { MSG_CMD_VCAP_SETEXIF,             MSG_VCAP_SetExifInfo },
    { MSG_CMD_VCAP_GETEXIF,             MSG_VCAP_GetExifInfo },
    { MSG_CMD_VCAP_SETOSDATTR,          MSG_VCAP_SetChnOSDAttr },
    { MSG_CMD_VCAP_GETOSDATTR,          MSG_VCAP_GetChnOSDAttr },
    { MSG_CMD_VCAP_START_OSD,           MSG_VCAP_StartChnOSD },
    { MSG_CMD_VCAP_STOP_OSD,            MSG_VCAP_StopChnOSD },
    { MSG_CMD_VCAP_SET_DISATTR,         MSG_VCAP_SetChnDISAttr },
    { MSG_CMD_VCAP_GET_DISATTR,         MSG_VCAP_GetChnDISAttr },
    { MSG_CMD_VCAP_SET_DUMP_RAW,        MSG_VCAP_SetDumpRawAttr },
    { MSG_CMD_VCAP_GET_DUMP_RAW,        MSG_VCAP_GetDumpRawAttr },
    { MSG_CMD_VCAP_START_DUMP_RAW,      HI_NULL },
    { MSG_CMD_VCAP_SET_DUMP_BNRRAW,     HI_NULL },
    { MSG_CMD_VCAP_GET_DUMP_BNRRAW,     HI_NULL },
    { MSG_CMD_VCAP_START_DUMP_BNRRAW,   HI_NULL },
    { MSG_CMD_VCAP_GET_RAWFRAME,        MSG_VCap_GetRawFrame },
    { MSG_CMD_VCAP_RELEASE_RAWFRAME,    MSG_VCap_ReleaseRawFrame },
    { MSG_CMD_VCAP_INIT_MOTIONSENSOR,   MSG_VCAP_InitMotionSensor },
    { MSG_CMD_VCAP_DEINIT_MOTIONSENSOR, MSG_VCAP_DeInitMotionSensor },
    { MSG_CMD_VCAP_AHD_GET_STATUS,      MSG_VCAP_GetAhdStatus },
    { MSG_CMD_VCAP_AHD_CHANGE_CHN,      MSG_VCAP_ChangeChn },
};

MSG_SERVER_MODULE_S g_stModuleVcap = {
    HI_MAPI_MOD_VCAP,
    "vcap",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetVcapMod(HI_VOID)
{
    return &g_stModuleVcap;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
