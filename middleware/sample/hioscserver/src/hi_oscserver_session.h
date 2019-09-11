/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_session.h
* @brief     oscserver command process head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#ifndef __HI_OSCSERVER_SESSION_H__
#define __HI_OSCSERVER_SESSION_H__
#include "hi_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define OSC_CHECKFORUPDATE_CHECK_INTERVAL        (1)    /*in seconds*/
#define OSC_COMMAND_EXEC_MAX_NUM                 (19)   /*the num of command exec num*/

/*command status*/
#define HI_OSC_COMMAND_STATUS_COMPLETE          (1.00f)
#define HI_OSC_COMMAND_STATUS_PROGRESS          (0.00f)

/*battery level*/
#define HI_OSC_BATTERY_LEVEL                    (1.00f)

/*the callback fn use to storage the command fn*/
typedef HI_S32 (*HI_OSC_COMMAND_EXEC_FN_PTR)(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);

typedef struct hiOSC_COMMAND_EXEC_S
{
    HI_OSC_API_COMMAND_E enCommandKey;
    HI_OSC_COMMAND_EXEC_FN_PTR pfnCommandExec;
} HI_OSC_COMMAND_EXEC_S;

HI_S32 OSCSVR_GetClientIP(HI_SOCKET s32Sockfd, HI_CHAR* aszClientIP);
HI_BOOL OSCSVR_CheckClient_IsInvaild(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E enLevel);
HI_S32 OSCSVR_Get_CommandID(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszCommandID);
HI_BOOL OSCSVR_CheckClientIP_IsInvaild(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_SOCKET s32Sockfd, HI_OSC_SERVER_LEVEL_E enLevel);
HI_BOOL OSCSVR_Search_CommandCB(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey);
HI_S32 OSCSVR_GetSessionbyIPaddress(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszClientIP, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* enLevel);
HI_S32 OSCSVR_Create_CommandStatus(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SERVER_LEVEL_E enCliVersion, HI_CHAR* pszCommandName, HI_CHAR* pszCommandId, HI_OSC_COMM_STATUS_E enComStatus, HI_DOUBLE dCompletion);
HI_S32 OSCSVR_Delete_CommandStatus(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszCommandId);
HI_S32 HI_OSCSVR_Process_Info(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 HI_OSCSVR_Process_State(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 HI_OSCSVR_Process_CheckForUpdate(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 HI_OSCSVR_Process_Command_Exec(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 HI_OSCSVR_Process_Command_Status(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_SESSION_H__*/
