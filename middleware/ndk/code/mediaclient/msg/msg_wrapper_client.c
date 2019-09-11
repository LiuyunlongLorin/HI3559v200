/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_wrapper_client.c
 * @brief   msg wrapper client function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "hi_ipcmsg.h"
#include "hi_mapi_log.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "msg_define.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 MAPI_SendSync(HI_U32 u32Module, HI_U32 u32CMD, HI_VOID *pBody, HI_U32 u32BodyLen,
                     MAPI_PRIV_DATA_S *pstPrivData)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    pReq = HI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
    if (pReq == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateMessage return NULL.\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }
    if (pstPrivData != NULL) {
        memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(HI_S32) * HI_IPCMSG_PRIVDATA_NUM);
    }
    s32Ret = HI_IPCMSG_SendSync(MAPI_Media_MSG_GetSiId(), pReq, &pResp, HI_IPCMSG_SEND_SYNC_TIMEOUT);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    if (s32Ret == HI_SUCCESS && (pResp->u32BodyLen > 0)) {
        memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

        if (pstPrivData != NULL) {
            memcpy(pstPrivData->as32PrivData, pResp->as32PrivData, sizeof(HI_S32) * HI_IPCMSG_PRIVDATA_NUM);
        }
    }
    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}

HI_S32 MAPI_SendSyncWithResp(HI_U32 u32Module, HI_U32 u32CMD, HI_VOID *pReqBody, HI_VOID *pRespBody,
                             HI_U32 u32ReqBodyLen, MAPI_PRIV_DATA_S *pstPrivData)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    pReq = HI_IPCMSG_CreateMessage(u32Module, u32CMD, pReqBody, u32ReqBodyLen);
    if (pReq == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_CreateMessage return NULL.\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }
    if (pstPrivData != NULL) {
        memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(HI_S32) * HI_IPCMSG_PRIVDATA_NUM);
    }
    s32Ret = HI_IPCMSG_SendSync(MAPI_Media_MSG_GetSiId(), pReq, &pResp, HI_IPCMSG_SEND_SYNC_TIMEOUT);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    if (s32Ret == HI_SUCCESS && (pResp->u32BodyLen > 0)) {
        memcpy(pRespBody, pResp->pBody, pResp->u32BodyLen);
    }
    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
