/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_venc.c
 * @brief   msg venc function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_venc.h"
#include "hi_datafifo.h"
#include "hi_comm_venc.h"
#include "mapi_venc_inner.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_venc_define.h"
#include "msg_server.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_BOOL g_abLinuxBootStarted[HI_MAPI_VENC_MAX_CHN_NUM] = { HI_FALSE };

HI_S32 MSG_VENC_Init(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    MAPI_VENC_DATAFIFO_S stVEncDatafifo;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    memset(&stVEncDatafifo, 0x0, sizeof(MAPI_VENC_DATAFIFO_S));
    s32Ret = HI_MAPI_VENC_Init(VencHdl, (HI_MAPI_VENC_ATTR_S *)pstMsg->pBody);

    if (s32Ret == HI_SUCCESS) {
        MAPI_VENC_OpenDatafifoWriter(VencHdl, (HI_U32)pstMsg->as32PrivData[0], &stVEncDatafifo);
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVEncDatafifo, sizeof(MAPI_VENC_DATAFIFO_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_OpenDatafifoStatus(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    /* In the quick start, both datafifo are initialized, open state */
    MAPI_VENC_OpenDatafifoStatus(VencHdl);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_Deinit(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_Deinit(VencHdl);

    if (s32Ret == HI_SUCCESS) {
        MAPI_VENC_CloseDatafifoWriter(VencHdl);
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_Start(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    if (g_abLinuxBootStarted[VencHdl] == HI_FALSE && MAPI_VENC_GetLiteBootStatus(VencHdl) == HI_TRUE) { /* Quick start */
        s32Ret = HI_SUCCESS;
    } else {
        s32Ret = HI_MAPI_VENC_Start(VencHdl, *(HI_S32 *)pstMsg->pBody);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
        }
    }

    MAPI_VENC_StartDataInfo(VencHdl);

    if (s32Ret == HI_SUCCESS) {
        g_abLinuxBootStarted[VencHdl] = HI_TRUE;
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_StopReady(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    MAPI_VENC_LINKDATA_INFO_S stLinkDataInfo;

    MAPI_VENC_StopDataInfo(VencHdl);
    MAPI_VENC_GetDataInfo(VencHdl, &stLinkDataInfo);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stLinkDataInfo, sizeof(MAPI_VENC_LINKDATA_INFO_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }
    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_Stop(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    MAPI_VENC_ClearDatafifoData(VencHdl);

    s32Ret = MAPI_VENC_ClearDatafifoFrame(VencHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "MAPI_VENC_ClearDatafifoFrame Failed : %#x!\n", s32Ret);
    }

    s32Ret = HI_MAPI_VENC_Stop(VencHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MAPI_VENC_Stop Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_BindVProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VProcHdl, VPortHdl;
    VProcHdl = GET_DEV_ID(pstMsg->u32Module);
    VPortHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_BindVProc(VProcHdl, VPortHdl, *(HI_HANDLE *)pstMsg->pBody, (HI_BOOL)pstMsg->as32PrivData[0]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_UnBindVProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VProcHdl, VPortHdl;
    VProcHdl = GET_DEV_ID(pstMsg->u32Module);
    VPortHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_UnBindVProc(VProcHdl, VPortHdl, *(HI_HANDLE *)pstMsg->pBody,
                                      (HI_BOOL)pstMsg->as32PrivData[0]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_BindVCap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl, PipeChnHdl;
    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_BindVCap(VcapPipeHdl, PipeChnHdl, *(HI_HANDLE *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_UnBindVCap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VcapPipeHdl, PipeChnHdl;
    VcapPipeHdl = GET_DEV_ID(pstMsg->u32Module);
    PipeChnHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_UnBindVCap(VcapPipeHdl, PipeChnHdl, *(HI_HANDLE *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_SetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_SetAttr(VencHdl, (HI_MAPI_VENC_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &s32Ret, sizeof(HI_S32));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    HI_MAPI_VENC_ATTR_S stAttr;

    memset(&stAttr, 0x0, sizeof(HI_MAPI_VENC_ATTR_S));
    s32Ret = HI_MAPI_VENC_GetAttr(VencHdl, &stAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stAttr, sizeof(HI_MAPI_VENC_ATTR_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_ReqIDR(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    s32Ret = HI_MAPI_VENC_RequestIDR(VencHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_SetExtendAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    HI_MAPI_VENC_CMD_E enCmd;

    memcpy(&enCmd, pstMsg->as32PrivData, sizeof(HI_MAPI_VENC_CMD_E));
    s32Ret = HI_MAPI_VENC_SetAttrEx(VencHdl, enCmd, pstMsg->pBody, pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetExtendAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl;
    VencHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    HI_MAPI_VENC_CMD_E enCmd;

    memcpy(&enCmd, pstMsg->as32PrivData, sizeof(HI_MAPI_VENC_CMD_E));
    s32Ret = HI_MAPI_VENC_GetAttrEx(VencHdl, enCmd, pstMsg->pBody, pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->u32BodyLen);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_SetOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl, OSDHdl;
    VencHdl = GET_DEV_ID(pstMsg->u32Module);
    OSDHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_SetOSDAttr(VencHdl, OSDHdl, (HI_MAPI_OSD_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->u32BodyLen);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl, OSDHdl;
    VencHdl = GET_DEV_ID(pstMsg->u32Module);
    OSDHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;
    HI_MAPI_OSD_ATTR_S stOSDAttr;

    memset(&stOSDAttr, 0x0, sizeof(HI_MAPI_OSD_ATTR_S));
    s32Ret = HI_MAPI_VENC_GetOSDAttr(VencHdl, OSDHdl, &stOSDAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_StartOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl, OSDHdl;
    HI_S32 s32Ret;
    VencHdl = GET_DEV_ID(pstMsg->u32Module);
    OSDHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_StartOSD(VencHdl, OSDHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_VENC_StopOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE VencHdl, OSDHdl;
    HI_S32 s32Ret;
    VencHdl = GET_DEV_ID(pstMsg->u32Module);
    OSDHdl = GET_CHN_ID(pstMsg->u32Module);
    HI_IPCMSG_MESSAGE_S *respMsg = HI_NULL;

    s32Ret = HI_MAPI_VENC_StopOSD(VencHdl, OSDHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "Failed : %#x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_IPCMSG_SendAsync fail,ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_VENC_INIT,                MSG_VENC_Init },
    { MSG_CMD_VENC_SET_DATAFIFO_STATUS, MSG_VENC_OpenDatafifoStatus },
    { MSG_CMD_VENC_DEINIT,              MSG_VENC_Deinit },
    { MSG_CMD_VENC_START,               MSG_VENC_Start },
    { MSG_CMD_VENC_STOP_READY,          MSG_VENC_StopReady },
    { MSG_CMD_VENC_STOP,                MSG_VENC_Stop },
    { MSG_CMD_VENC_SET_ATTR,            MSG_VENC_SetAttr },
    { MSG_CMD_VENC_GET_ATTR,            MSG_VENC_GetAttr },
    { MSG_CMD_VENC_BIND_VCAP,           MSG_VENC_BindVCap },
    { MSG_CMD_VENC_UNBIND_VCAP,         MSG_VENC_UnBindVCap },
    { MSG_CMD_VENC_BIND_VPROC,          MSG_VENC_BindVProc },
    { MSG_CMD_VENC_UNBIND_VPROC,        MSG_VENC_UnBindVProc },
    { MSG_CMD_VENC_REQ_IDR,             MSG_VENC_ReqIDR },
    { MSG_CMD_VENC_SET_EXTEND_ATTR,     MSG_VENC_SetExtendAttr },
    { MSG_CMD_VENC_GET_EXTEND_ATTR,     MSG_VENC_GetExtendAttr },
    { MSG_CMD_VENC_SET_OSD_ATTR,        MSG_VENC_SetOSDAttr },
    { MSG_CMD_VENC_GET_OSD_ATTR,        MSG_VENC_GetOSDAttr },
    { MSG_CMD_VENC_START_OSD,           MSG_VENC_StartOSD },
    { MSG_CMD_VENC_STOP_OSD,            MSG_VENC_StopOSD },
};

MSG_SERVER_MODULE_S g_stModuleVenc = {
    HI_MAPI_MOD_VENC,
    "venc",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetVencMod(HI_VOID)
{
    return &g_stModuleVenc;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
