/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_disp.c
 * @brief   msg disp function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_disp.h"
#include "hi_mapi_disp_define.h"
#include "hi_mapi_disp.h"
#include "mapi_disp_inner.h"
#include "msg_server.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_S32 MSG_DISP_Init(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_Init(DevID, (HI_MAPI_DISP_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Init fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_Deinit(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_Deinit(DevID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Deinit fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_Start(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_Start(DevID, (HI_MAPI_DISP_VIDEOLAYER_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Start fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_Stop(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_Stop(DevID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Stop fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_SetAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_SetAttrEx(DevID, pstMsg->as32PrivData[0], (HI_VOID *)pstMsg->pBody, pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_SetAttrEx fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_GetAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_MAPI_DISP_CMD_E enAttrExCmd;

    enAttrExCmd = pstMsg->as32PrivData[0];
    s32Ret = HI_MAPI_DISP_GetAttrEx(DevID, enAttrExCmd, pstMsg->pBody, pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_GetAttrEx fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->u32BodyLen);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_SetWindowAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_SetWindowAttr(DevID, ChnID, (HI_MAPI_DISP_WINDOW_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_SetWindowAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_GetWindowAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_GetWindowAttr(DevID, ChnID, &stWndAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_GetWindowAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stWndAttr, sizeof(HI_MAPI_DISP_WINDOW_ATTR_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_SetWindowAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);

    s32Ret = HI_MAPI_DISP_SetWindowAttrEx(DevID, ChnID, pstMsg->as32PrivData[0], (HI_VOID *)pstMsg->pBody,
                                          pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_SetWindowAttrEx fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_GetWindowAttrEx(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_MAPI_DISP_WINDOW_CMD_E enAttrExCmd;

    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    enAttrExCmd = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_DISP_GetWindowAttrEx(DevID, ChnID, enAttrExCmd, pstMsg->pBody, pstMsg->u32BodyLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_GetWindowAttrEx fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, pstMsg->u32BodyLen);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_StartWindow(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_StartWindow(DevID, ChnID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_StartWindow fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_StopWindow(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_StopWindow(DevID, ChnID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_StopWindow fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_ClearWindow(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_ClearWindow(DevID, ChnID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_ClearWindow fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_SetWindowOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_SetWindowOSDAttr(DevID, ChnID, pstMsg->as32PrivData[0], (HI_MAPI_OSD_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_SetWindowOSDAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_GetWindowOSDAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_MAPI_OSD_ATTR_S stOsdAttr;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_GetWindowOSDAttr(DevID, ChnID, pstMsg->as32PrivData[0], &stOsdAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_GetWindowOSDAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stOsdAttr, sizeof(HI_MAPI_OSD_ATTR_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_StartWindowOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_StartWindowOSD(DevID, ChnID, pstMsg->as32PrivData[0]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_StartWindowOSD fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_StopWindowOSD(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_DISP_StopWindowOSD(DevID, ChnID, pstMsg->as32PrivData[0]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_StopWindowOSD fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_Bind_VProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_HANDLE GrpHdl;
    HI_HANDLE VPortHdl;
    HI_BOOL bStitch;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    GrpHdl = pstMsg->as32PrivData[0];
    VPortHdl = pstMsg->as32PrivData[1];
    bStitch = pstMsg->as32PrivData[2];
    s32Ret = HI_MAPI_DISP_Bind_VProc(GrpHdl, VPortHdl, DevID, ChnID, bStitch);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Bind_VProc fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_UnBind_VProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_HANDLE GrpHdl;
    HI_HANDLE VPortHdl;
    HI_BOOL bStitch;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    GrpHdl = pstMsg->as32PrivData[0];
    VPortHdl = pstMsg->as32PrivData[1];
    bStitch = pstMsg->as32PrivData[2];
    s32Ret = HI_MAPI_DISP_UnBind_VProc(GrpHdl, VPortHdl, DevID, ChnID, bStitch);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_UnBind_VProc fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_Bind_Vcap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    VcapPipeHdl = pstMsg->as32PrivData[0];
    PipeChnHdl = pstMsg->as32PrivData[1];
    s32Ret = HI_MAPI_DISP_Bind_VCap(VcapPipeHdl, PipeChnHdl, DevID, ChnID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_Bind_VCap fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_DISP_UnBind_Vcap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_HANDLE ChnID;
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    ChnID = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    VcapPipeHdl = pstMsg->as32PrivData[0];
    PipeChnHdl = pstMsg->as32PrivData[1];
    s32Ret = HI_MAPI_DISP_UnBind_VCap(VcapPipeHdl, PipeChnHdl, DevID, ChnID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MAPI_DISP_UnBind_VCap fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_DISP_INIT,                MSG_DISP_Init },
    { MSG_CMD_DISP_DEINIT,              MSG_DISP_Deinit },
    { MSG_CMD_DISP_START,               MSG_DISP_Start },
    { MSG_CMD_DISP_STOP,                MSG_DISP_Stop },
    { MSG_CMD_DISP_SET_ATTREX,          MSG_DISP_SetAttrEx },
    { MSG_CMD_DISP_GET_ATTREX,          MSG_DISP_GetAttrEx },
    { MSG_CMD_DISP_SET_WINDOW_ATTR,     MSG_DISP_SetWindowAttr },
    { MSG_CMD_DISP_GET_WINDOW_ATTR,     MSG_DISP_GetWindowAttr },
    { MSG_CMD_DISP_SET_WINDOW_ATTREX,   MSG_DISP_SetWindowAttrEx },
    { MSG_CMD_DISP_GET_WINDOW_ATTREX,   MSG_DISP_GetWindowAttrEx },
    { MSG_CMD_DISP_START_WINDOW,        MSG_DISP_StartWindow },
    { MSG_CMD_DISP_STOP_WINDOW,         MSG_DISP_StopWindow },
    { MSG_CMD_DISP_CLEAR_WINDOW,        MSG_DISP_ClearWindow },
    { MSG_CMD_DISP_SET_WINDOW_OSD_ATTR, MSG_DISP_SetWindowOSDAttr },
    { MSG_CMD_DISP_GET_WINDOW_OSD_ATTR, MSG_DISP_GetWindowOSDAttr },
    { MSG_CMD_DISP_START_WINDOW_OSD,    MSG_DISP_StartWindowOSD },
    { MSG_CMD_DISP_STOP_WINDOW_OSD,     MSG_DISP_StopWindowOSD },
    { MSG_CMD_DISP_BIND_VPROC,          MSG_DISP_Bind_VProc },
    { MSG_CMD_DISP_UNBIND_VPROC,        MSG_DISP_UnBind_VProc },
    { MSG_CMD_DISP_BIND_VCAP,           MSG_DISP_Bind_Vcap },
    { MSG_CMD_DISP_UNBIND_VCAP,         MSG_DISP_UnBind_Vcap },
};

MSG_SERVER_MODULE_S s_stModuleDisp = {
    HI_MAPI_MOD_DISP,
    "disp",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetDispMod(HI_VOID)
{
    return &s_stModuleDisp;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
