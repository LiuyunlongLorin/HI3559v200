/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_server.h
 * @brief   msg server header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_SERVER_H__
#define __MSG_SERVER_H__

#include "hi_mapi_comm_define.h"
#include "hi_comm_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MSG_SERVER_MODULE_NAME_LEN (16)
#define MSG_SERVER_MODULE_AMOUNT   (16)

typedef HI_S32 (*CMDPROC_FN)(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg);

typedef struct tagMSG_MODULE_CMD_S {
    HI_U32 u32Cmd;
    CMDPROC_FN CMDPROC_FN_PTR;
} MSG_MODULE_CMD_S;

typedef struct tagMSG_SERVER_MODULE_S {
    HI_U32 u32ModID;
    HI_CHAR saModuleName[MSG_SERVER_MODULE_NAME_LEN];
    HI_U32 u32ModuleCmdAmount;
    MSG_MODULE_CMD_S *pstModuleCmdTable;
} MSG_SERVER_MODULE_S;

typedef struct tagMSG_SERVER_CONTEXT {
    MSG_SERVER_MODULE_S *pstServermodules[HI_MAPI_MOD_BUTT];
} MSG_SERVER_CONTEXT;

HI_S32 MAPI_Media_MSG_Init(HI_VOID);
HI_S32 MAPI_Media_MSG_Deinit(HI_VOID);
HI_S32 MAPI_Media_MSG_GetSiId(HI_VOID);

HI_S32 PM_MSG_PROC(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg);

MSG_SERVER_MODULE_S *MAPI_MSG_GetSysMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetVcapMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetVProcMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetVencMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetDispMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetHdmiMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetAcapMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetAencMod(HI_VOID);
MSG_SERVER_MODULE_S *MAPI_MSG_GetAoMod(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_SERVER_H__ */
