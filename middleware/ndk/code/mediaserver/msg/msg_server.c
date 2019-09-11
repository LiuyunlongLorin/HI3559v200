/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_server.c
 * @brief   msg server function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include "hi_type.h"
#include "hi_mapi_log.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "msg_server.h"
#include "hi_mapi_comm_define.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 g_s32MediaMsgId;

static MSG_SERVER_CONTEXT g_msgServerContext;
static HI_BOOL g_bMsgSvrStartFlg = HI_FALSE;
static pthread_t g_msgSvrReceiveThread = -1;

CMDPROC_FN MSG_SERVER_GetFunc(HI_U32 u32ModID, HI_U32 u32CmdID)
{
    HI_U32 i;
    MSG_SERVER_MODULE_S *pTmpServerModule =
        g_msgServerContext.pstServermodules[u32ModID];

    if (pTmpServerModule == HI_NULL) {
        return HI_NULL;
    }

    for (i = 0; i < pTmpServerModule->u32ModuleCmdAmount; i++) {
        if (u32CmdID == (pTmpServerModule->pstModuleCmdTable + i)->u32Cmd) {
            return (pTmpServerModule->pstModuleCmdTable + i)->CMDPROC_FN_PTR;
        }
    }
    return HI_NULL;
}

void MediaMsgReceiveProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *stMsg)
{
    HI_U32 u32ModID;
    CMDPROC_FN pProc = HI_NULL;

    u32ModID = GET_MOD_ID(stMsg->u32Module);

    switch (u32ModID) {
        case HI_MAPI_MOD_SYS:
        case HI_MAPI_MOD_VCAP:
        case HI_MAPI_MOD_VPROC:
        case HI_MAPI_MOD_VENC:
        case HI_MAPI_MOD_DISP:
        case HI_MAPI_MOD_HDMI:
        case HI_MAPI_MOD_ACAP:
        case HI_MAPI_MOD_AENC:
        case HI_MAPI_MOD_AO:
            pProc = MSG_SERVER_GetFunc(u32ModID, stMsg->u32CMD);
            if (pProc != HI_NULL) {
                HI_S32 s32Ret = pProc(siId, stMsg);
                if (s32Ret != HI_SUCCESS) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_SYS,
                                   "ModID:%u CMD:%u PROC return error:0x%x\n", u32ModID,
                                   stMsg->u32CMD, s32Ret);
                }
            } else {
                MAPI_ERR_TRACE(HI_MAPI_MOD_SYS,
                               "ModID:%u CMD:%u GetFunc return NULL\n", u32ModID,
                               stMsg->u32CMD);
            }
            break;
        case HI_MAPI_MOD_PM:
            break;

        default:
            printf("receive u32ModID:%d msg %d error.\n", u32ModID, stMsg->u32CMD);
            break;
    }
}

void *MediaMsgReceiveThread(void *arg)
{
    HI_S32 s32Ret;
    prctl(PR_SET_NAME, "Hi_pTMediaMsgRe", 0, 0, 0);
    MAPI_UNUSED(arg);

    /* first connect */
    s32Ret = HI_IPCMSG_Connect(&g_s32MediaMsgId, "HiMPP_MSG", MediaMsgReceiveProc);
    if (s32Ret != HI_SUCCESS) {
        printf("Connect fail\n");
        return NULL;
    }

    HI_IPCMSG_Run(g_s32MediaMsgId);

    /* reconnect detect and process */
    while (g_bMsgSvrStartFlg) {
        if (HI_IPCMSG_IsConnected(g_s32MediaMsgId) != HI_TRUE) {
            HI_IPCMSG_Disconnect(g_s32MediaMsgId);

            s32Ret = HI_IPCMSG_Connect(&g_s32MediaMsgId, "HiMPP_MSG", MediaMsgReceiveProc);
            if (s32Ret != HI_SUCCESS) {
                printf("Connect fail\n");
                return NULL;
            }
            HI_IPCMSG_Run(g_s32MediaMsgId);
        }
    }

    return NULL;
}

HI_S32 MAPI_Media_MSG_Init(HI_VOID)
{
    HI_S32 s32Ret;
    pthread_attr_t attr;
    HI_IPCMSG_CONNECT_S stConnectAttr = { 0, HI_IPCMSG_PORT_HIMPP, 1 };
    (HI_VOID)pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    s32Ret = HI_IPCMSG_AddService("HiMPP_MSG", &stConnectAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "HI_IPCMSG_AddService fail\n");
    }

#ifdef __HuaweiLite__
    (HI_VOID)pthread_attr_setstacksize(&attr, 0x10000);
    attr.inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr.schedparam.sched_priority = 6;
#endif

    g_bMsgSvrStartFlg = HI_TRUE;
    s32Ret = pthread_create(&g_msgSvrReceiveThread, &attr, MediaMsgReceiveThread, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pthread_create MediaMsgReceiveThread fail\n");
    }
    pthread_attr_destroy(&attr);
    MAPI_INFO_TRACE(HI_MAPI_MOD_SYS, "Media_MSG_Init successfull\n");

    g_msgServerContext.pstServermodules[HI_MAPI_MOD_SYS] = MAPI_MSG_GetSysMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_VCAP] = MAPI_MSG_GetVcapMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_VPROC] = MAPI_MSG_GetVProcMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_VENC] = MAPI_MSG_GetVencMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_DISP] = MAPI_MSG_GetDispMod();
#ifdef SUPPORT_HDMI
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_HDMI] = MAPI_MSG_GetHdmiMod();
#endif
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_ACAP] = MAPI_MSG_GetAcapMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_AENC] = MAPI_MSG_GetAencMod();
    g_msgServerContext.pstServermodules[HI_MAPI_MOD_AO] = MAPI_MSG_GetAoMod();
    return s32Ret;
}

HI_S32 MAPI_Media_MSG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;

    g_bMsgSvrStartFlg = HI_FALSE;
    s32Ret = HI_IPCMSG_Disconnect(g_s32MediaMsgId);
    if (s32Ret != HI_SUCCESS) {
        printf("Disconnect fail , ret:%d\n", s32Ret);
    }

    (HI_VOID)pthread_join(g_msgSvrReceiveThread, HI_NULL);

    HI_IPCMSG_DelService("HiMPP_MSG");
    return s32Ret;
}

HI_S32 MAPI_Media_MSG_GetSiId(HI_VOID)
{
    return g_s32MediaMsgId;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

