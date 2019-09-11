/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_aenc.c
 * @brief   msg aenc function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_aenc.h"
#include "hi_datafifo.h"
#include "hi_mapi_aenc.h"
#include "mapi_aenc_inner.h"
#include "msg_server.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 MSG_AENC_Init(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE ChnId;
    ChnId = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_MAPI_AENC_ATTR_S stAencAttr;
    MAPI_AENC_DATAFIFO_S stAencDatafifo;

    memcpy(&stAencAttr, pstMsg->as32PrivData, sizeof(HI_MAPI_AENC_ATTR_S));
    stAencAttr.pValue = pstMsg->pBody;

    s32Ret = HI_MAPI_AENC_Init(ChnId, &stAencAttr);
    if (s32Ret == HI_SUCCESS) {
        MAPI_AENC_OpenDatafifoWriter(ChnId, &stAencDatafifo);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stAencDatafifo, sizeof(MAPI_AENC_DATAFIFO_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_AENC_Deinit(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE ChnId;
    ChnId = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_AENC_Deinit(ChnId);
    if (s32Ret == HI_SUCCESS) {
        MAPI_AENC_CloseDatafifoWriter(ChnId);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_AENC_Start(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE ChnId;
    ChnId = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_AENC_Start(ChnId);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Start fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_AENC_Stop(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE hAencHandle;
    hAencHandle = GET_CHN_ID(pstMsg->u32Module);
    MAPI_AENC_LINKDATA_INFO_S stLinkDataInfo;
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_AENC_Stop(hAencHandle);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_Stop fail: %#x\n", s32Ret);
    }

    MAPI_AENC_GetLinkDataInfo(hAencHandle, &stLinkDataInfo);

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_AENC, "====== u32LastFrmSeq = %d , u32LinkFrmNum = %d \n",
                     stLinkDataInfo.u32LastFrmSeq, stLinkDataInfo.u32LinkFrmNum);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stLinkDataInfo, sizeof(MAPI_AENC_LINKDATA_INFO_S));
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_AENC_Stop_Finish(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE hAencHandle;
    hAencHandle = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;

    MAPI_AENC_ReleaseUnhandleStream(hAencHandle);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_AENC_BindCap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE hAencHandle;
    HI_HANDLE hAcapChnHandle;
    hAencHandle = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    hAcapChnHandle = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_AENC_BindACap(*(HI_HANDLE *)pstMsg->pBody, hAcapChnHandle, hAencHandle);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_BindACap fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_AENC_UnbindCap(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE hAencHandle;
    HI_HANDLE hAcapChnHandle;
    hAencHandle = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    hAcapChnHandle = pstMsg->as32PrivData[0];

    s32Ret = HI_MAPI_AENC_UnbindACap(*(HI_HANDLE *)pstMsg->pBody, hAcapChnHandle, hAencHandle);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnbindACap fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_AENC_RegisterEncoder(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_AENC_RegisterAudioEncoder(*((HI_MAPI_AUDIO_FORMAT_E *)pstMsg->pBody));
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_RegisterAudioEncoder fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);
    return HI_SUCCESS;
}

HI_S32 MSG_AENC_UnregisterEncoder(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_AENC_UnregisterAudioEncoder(*((HI_MAPI_AUDIO_FORMAT_E *)pstMsg->pBody));
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_UnregisterAudioEncoder fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_AENC_SendFrame(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_HANDLE ChnId;
    ChnId = GET_CHN_ID(pstMsg->u32Module);
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    AUDIO_FRAME_S *pstFrm;
    pstFrm = (AUDIO_FRAME_S *)pstMsg->pBody;

    s32Ret = HI_MAPI_AENC_SendFrame(ChnId, pstFrm);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_MAPI_AENC_SendFrame fail: %#x\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, HI_NULL, 0);
    if (respMsg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "HI_IPCMSG_CreateRespMessage fail\n");
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return s32Ret;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_AENC_INIT,                MSG_AENC_Init },
    { MSG_CMD_AENC_DEINIT,              MSG_AENC_Deinit },
    { MSG_CMD_AENC_START,               MSG_AENC_Start },
    { MSG_CMD_AENC_STOP,                MSG_AENC_Stop },
    { MSG_CMD_AENC_STOP_FINISH,         MSG_AENC_Stop_Finish },
    { MSG_CMD_AENC_BIND_ACAP,           MSG_AENC_BindCap },
    { MSG_CMD_AENC_UNBIND_ACAP,         MSG_AENC_UnbindCap },
    { MSG_CMD_AENC_REGISTER_AENCODER,   MSG_AENC_RegisterEncoder },
    { MSG_CMD_AENC_UNREGISTER_AENCODER, MSG_AENC_UnregisterEncoder },
    { MSG_CMD_AENC_SEND_FRAME,          MSG_AENC_SendFrame },
};

MSG_SERVER_MODULE_S g_stModuleAenc = {
    HI_MAPI_MOD_AENC,
    "aenc",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetAencMod(HI_VOID)
{
    return &g_stModuleAenc;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
