/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_hdmi.c
 * @brief   msg hdmi function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_disp.h"
#include "msg_hdmi.h"
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_hdmi.h"
#include "mapi_hdmi_inner.h"
#include "msg_server.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 MSG_HDMI_Init(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_Init((HI_HDMI_ID_E)DevID, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_Init fail: %#x\n", s32Ret);
    }
    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_Deinit(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_Deinit((HI_HDMI_ID_E)DevID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_Deinit fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_GetSinkCapbility(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_MAPI_HDMI_SINKCAPABILITY_S stHdmiSinkCapbility;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_GetSinkCapability((HI_HDMI_ID_E)DevID, &stHdmiSinkCapbility);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_GetSinkCapability fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stHdmiSinkCapbility, sizeof(HI_MAPI_HDMI_SINKCAPABILITY_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_SetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_SetAttr((HI_HDMI_ID_E)DevID, (HI_MAPI_HDMI_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_SetAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_GetAttr(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    HI_MAPI_HDMI_ATTR_S stHdmiAttr;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_GetAttr((HI_HDMI_ID_E)DevID, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_GetAttr fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stHdmiAttr, sizeof(HI_MAPI_HDMI_ATTR_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_Start(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_Start((HI_HDMI_ID_E)DevID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_Start fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_HDMI_Stop(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE DevID;
    DevID = GET_DEV_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_HDMI_Stop((HI_HDMI_ID_E)DevID);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MAPI_HDMI_Stop fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_HDMI_INIT,              MSG_HDMI_Init },
    { MSG_CMD_HDMI_DEINIT,            MSG_HDMI_Deinit },
    { MSG_CMD_HDMI_GET_SINKCAPBILITY, MSG_HDMI_GetSinkCapbility },
    { MSG_CMD_HDMI_SET_ATTR,          MSG_HDMI_SetAttr },
    { MSG_CMD_HDMI_GET_ATTR,          MSG_HDMI_GetAttr },
    { MSG_CMD_HDMI_START,             MSG_HDMI_Start },
    { MSG_CMD_HDMI_STOP,              MSG_HDMI_Stop },
};

MSG_SERVER_MODULE_S g_stModuleHdmi = {
    HI_MAPI_MOD_HDMI,
    "hdmi",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetHdmiMod(HI_VOID)
{
    return &g_stModuleHdmi;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
