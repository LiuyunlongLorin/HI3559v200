/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_sys.c
 * @brief   msg sys function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "msg_sys.h"
#include "msg_server.h"
#include "msg_define.h"
#include "hi_ipcmsg.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_log.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 MSG_MEDIA_SYS_Init(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_Media_Init((const HI_MAPI_MEDIA_ATTR_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_Media_Init Failed %x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_MEDIA_SYS_Deinit(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_Media_Deinit();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_Media_Deinit Failed %x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_MEDIA_SYS_AllocBuffer(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;
    MSG_ALLOCBUFFER_S *pstAllocBuffer = pstMsg->pBody;
    HI_U64 u64PhyBuffAddr = 0;
    HI_U8 *pVirBuffAddr;

    s32Ret = HI_MAPI_AllocBuffer(&u64PhyBuffAddr, (HI_VOID **)&pVirBuffAddr, pstAllocBuffer->u32Len,
                                 pstAllocBuffer->aszstrName);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_AllocBuffer Failed %x!\n", s32Ret);
    }

    pstAllocBuffer->u64PhyAddr = u64PhyBuffAddr;
    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstMsg->pBody, sizeof(MSG_ALLOCBUFFER_S));
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_MEDIA_SYS_FreeBuffer(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    MSG_FREEBUFFER_S *pstFreeBuffer = pstMsg->pBody;

    s32Ret = HI_MAPI_FreeBuffer(pstFreeBuffer->u64PhyAddr, pstFreeBuffer->pVirtAddr, pstFreeBuffer->u32Len);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_FreeBuffer Failed %x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_MEDIA_SYS_EnableDsp(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_Sys_EnableDsp((const HI_MAPI_Sys_DspAttr_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_Sys_EnableDsp Failed %x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_MEDIA_SYS_DiableDsp(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = HI_MAPI_Sys_DisableDsp((const HI_MAPI_Sys_DspAttr_S *)pstMsg->pBody);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_MAPI_Sys_DisableDsp Failed %x!\n", s32Ret);
    }

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    if (respMsg == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateRespMessage fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendAsync(siId, respMsg, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "SendAsync fail,ret:[%#x]\n", s32Ret);
        HI_IPCMSG_DestroyMessage(respMsg);
        return HI_FAILURE;
    }

    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stModuleCmdTable[] = {
    { MSG_CMD_MEDIA_SYS_INIT,         MSG_MEDIA_SYS_Init },
    { MSG_CMD_MEDIA_SYS_DEINIT,       MSG_MEDIA_SYS_Deinit },
    { MSG_CMD_MEDIA_SYS_ALLOC_BUFFER, MSG_MEDIA_SYS_AllocBuffer },
    { MSG_CMD_MEDIA_SYS_FREE_BUFFER,  MSG_MEDIA_SYS_FreeBuffer },
    { MSG_CMD_MEDIA_SYS_ENABLE_DSP,   MSG_MEDIA_SYS_EnableDsp },
    { MSG_CMD_MEDIA_SYS_DISABLE_DSP,  MSG_MEDIA_SYS_DiableDsp },
};

MSG_SERVER_MODULE_S g_stModuleSys = {
    HI_MAPI_MOD_SYS,
    "sys",
    sizeof(g_stModuleCmdTable) / sizeof(MSG_MODULE_CMD_S),
    &g_stModuleCmdTable[0]
};

MSG_SERVER_MODULE_S *MAPI_MSG_GetSysMod(HI_VOID)
{
    return &g_stModuleSys;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
