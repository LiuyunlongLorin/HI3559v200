/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_com_exec.c
* @brief     oscserver com exec src file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*osc module header file*/
#include "hi_osc_log.h"
#include "hi_osc_server.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_http.h"
#include "hi_oscserver_jsonparser.h"
#include "hi_oscserver_com_exec.h"
#include "hi_oscserver_jsonpackage.h"
#include "hi_oscserver_session.h"
#include "hi_oscserver_file_exec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
static HI_S32 OSCSVR_Send_TakePicture_Response(HI_CHAR* pszCommandID, HI_S32* ps32SocketFd)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszCommandID);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Generate_TakePicture_JsonPack(pszCommandID, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture generate json packs error!!\n");
        s32StatCode = HI_OSC_STATUS_INTERNAL_SERVER_ERROR;

        goto Failed;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
Failed:
    HI_OSCSVR_MSGParser_ConnectFailResponse(aszReply, s32StatCode, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Send_StartCapture_Response(HI_CHAR* pszCommandID,  HI_S32* ps32SocketFd)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszCommandID);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_IntervalCapture_JsonPack(pszCommandID, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture generate json packs error!!\n");

        return HI_FAILURE;
    }

    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture generate response fail!!\n");

        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

static HI_S32 OSCSVR_TakePicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSC_PATH_TYPE_E enType, HI_CHAR* pszCommandID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bDisabled = HI_FALSE;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszCommandID);

    s32Ret = pstServerCtx->stRegCommandCB.pfnTakePicture(pOSC, enType, pszCommandID, &bDisabled);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture perform callback fail!!\n");
        return HI_FAILURE;
    }

    if (bDisabled)
    {
        if (HI_OSC_SERVER_LEVEL_2 == enLevel)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture is busy currently!!\n");
            s32StatCode = HI_OSC_STATUS_FORBIDDEN;
            s32InvaildCode = HIOSC_INVAILD_DISABLED_COMMAND;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Command is currently disabled");

            goto Invaild;
        }

        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture is disabled currently!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Send_TakePicture_Response(pszCommandID, ps32SocketFd);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture send the response fail!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_TAKE_PICTURE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Session_TakePicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pszCommandName, HI_OSC_SERVER_LEVEL_E enLevel, HI_CHAR* pszSessionId)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_OSC_PATH_TYPE_E enType = HI_OSC_INIT_PATH;
    HI_CHAR aszCommandID[OSC_SVR_COMMAND_ID_LEN] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    enOSCCommandKey = HI_OSC_COMMAND_TAKE_PICTURE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture search command fail!!\n");
        return HI_FAILURE;
    }

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        enType = HI_OSC_ABSOLUTE_PATH;
        s32Ret = OSCSVR_Get_CommandID(pstServerCtx, aszCommandID);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture get command id fail!!\n");
            return HI_FAILURE;
        }

        if (HI_SUCCESS != OSCSVR_Create_CommandStatus(pstServerCtx, HI_OSC_SERVER_LEVEL_1, pszCommandName, aszCommandID, HIOSC_COMM_STATUS_INPROGRESS, 0.00f))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture create command status fail!!\n");
            return HI_FAILURE;
        }

        if (HI_SUCCESS != OSCSVR_TakePicture(pOSC, ps32SocketFd, enLevel, enType, aszCommandID))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "level1 Get Takepicture url fail!!\n");
            return HI_FAILURE;
        }
    }
    else if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        s32Ret = OSCSVR_Get_CommandID(pstServerCtx, aszCommandID);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture get command id fail!!\n");
            return HI_FAILURE;
        }

        if (HI_SUCCESS != OSCSVR_Create_CommandStatus(pstServerCtx, HI_OSC_SERVER_LEVEL_2, pszCommandName, aszCommandID, HIOSC_COMM_STATUS_INPROGRESS, 0.00f))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture create command status fail!!\n");
            return HI_FAILURE;
        }

        enType = HI_OSC_ABSOLUTE_PATH;

        if (HI_SUCCESS != OSCSVR_TakePicture(pOSC, ps32SocketFd, enLevel, enType, aszCommandID))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "level2 Get Takepicture url fail!!\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Create_ClientLevel1Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszClientIP, HI_S32 s32SessTimeOut, HI_OSC_SESS_LEVEL1_S* pstSessionInfo)
{
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pszClientIP);
    OSCSVR_CHECK_NULL_ERROR(pstSessionInfo);

    strncpy_s(pstSessionInfo->aszClientIP, OSC_IP_MAX_LEN, pszClientIP, OSC_IP_MAX_LEN - 1);

    if (OSC_IGNORE_SESS_TIMEOUT == s32SessTimeOut)
    {
        pstSessionInfo->s32SessTimeOut = pstServerCtx->s32TimeOut;
    }
    else
    {
        pstSessionInfo->s32SessTimeOut = s32SessTimeOut;
    }

    /*generate sessionid*/
    snprintf_s(aszSessionId, OSC_SESSIONID_MAX_LEN, OSC_SESSIONID_MAX_LEN - 1, "SID_%04d", pstServerCtx->u32SessionId);
    strncpy_s(pstSessionInfo->aszSessionId, OSC_SESSIONID_MAX_LEN, aszSessionId, strlen(aszSessionId) + 1);
    pstSessionInfo->aszSessionId[strlen(aszSessionId)] = '\0';
    pstSessionInfo->bCapture = HI_FALSE;
    /* get the first time!!*/
    (HI_VOID)time(&(pstSessionInfo->FirstTick));
    (HI_VOID)time(&(pstSessionInfo->CurrentTick));

    memset_s(pstSessionInfo->stOSCState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, 0x00, HI_OSCSVR_STORAGE_URL_MAX_LEN);
    pstSessionInfo->stOSCState.bStorageChanged = HI_TRUE;
    pstSessionInfo->stOSCState.dBatteryLevel = HI_OSC_BATTERY_LEVEL;
    pstSessionInfo->stOSCState.hVendorSpecific = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Create_ClientLevel2Session(HI_OSC_SERVER_CTX_S* pstServerCtx)
{
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    pstServerCtx->pstLevel2Sess = (HI_OSC_SESS_LEVEL2_S*)malloc(sizeof(HI_OSC_SESS_LEVEL2_S));

    if (NULL == pstServerCtx->pstLevel2Sess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC malloc level2 session info fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstServerCtx->pstLevel2Sess, sizeof(HI_OSC_SESS_LEVEL2_S), 0x00, sizeof(HI_OSC_SESS_LEVEL2_S));

    pstServerCtx->pstLevel2Sess->s32SessTimeOut = pstServerCtx->s32TimeOut;
    pstServerCtx->pstLevel2Sess->bCapture = HI_FALSE;
    memset_s(&(pstServerCtx->pstLevel2Sess->stOSCState), sizeof(HI_OSCSERVER_STATE_S), 0x00, sizeof(HI_OSCSERVER_STATE_S));

    /* get the first time!!*/
    (HI_VOID)time(&(pstServerCtx->pstLevel2Sess->FirstTick));
    (HI_VOID)time(&(pstServerCtx->pstLevel2Sess->CurrentTick));

    memset_s(pstServerCtx->pstLevel2Sess->stOSCState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, 0x00, HI_OSCSVR_STORAGE_URL_MAX_LEN);
    pstServerCtx->pstLevel2Sess->stOSCState.bStorageChanged = HI_TRUE;
    pstServerCtx->pstLevel2Sess->stOSCState.dBatteryLevel = 1.0f;
    pstServerCtx->pstLevel2Sess->stOSCState.hVendorSpecific = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Destroy_ClientLevel1Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SESS_LEVEL1_S* pstSessionInfo)
{
    OSCSVR_CHECK_NULL_ERROR(pstSessionInfo);
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    free(pstSessionInfo);
    pstSessionInfo = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Destroy_ClientLevel2Session(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SESS_LEVEL2_S* pstSessionInfo)
{
    OSCSVR_CHECK_NULL_ERROR(pstSessionInfo);
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    free(pstSessionInfo);
    pstSessionInfo = NULL;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_StartSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bFound = HI_FALSE;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_S32  s32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    OSC_LOCK(pstServerCtx->Level1SessLock);

    if ((HI_TRUE == pstServerCtx->bExclusiveUseFlag) && (NULL != pstServerCtx->pstLevel1Sess))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession camera already in exclusive use, new session can't be started.\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_IN_EXCLUSIVEUSE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Camera already in exclusive use, new session can't be started.");
        OSC_UNLOCK(pstServerCtx->Level1SessLock);

        goto Invaild;
    }

    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_StartSession_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, aszComName, &s32SessTimeOut);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    OSC_LOCK(pstServerCtx->Level1SessLock);

    if (NULL != pstServerCtx->pstLevel1Sess)
    {
        if (0 != strlen(pstServerCtx->pstLevel1Sess->aszSessionId))
        {
            bFound = HI_TRUE;
        }
    }


    if (bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "There is an existing session!\n");
        OSCSVR_Destroy_ClientLevel1Session(pstServerCtx, pstServerCtx->pstLevel1Sess);
        pstServerCtx->pstLevel1Sess = NULL;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession firstly start\n");
    }

    pstServerCtx->pstLevel1Sess = (HI_OSC_SESS_LEVEL1_S*)malloc(sizeof(HI_OSC_SESS_LEVEL1_S));

    if (NULL == pstServerCtx->pstLevel1Sess)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession malloc level1 session fail!!\n");
        OSC_UNLOCK(pstServerCtx->Level1SessLock);

        return HI_FAILURE;
    }

    memset_s(pstServerCtx->pstLevel1Sess, sizeof(HI_OSC_SESS_LEVEL1_S), 0x00, sizeof(HI_OSC_SESS_LEVEL1_S));

    if (HI_SUCCESS != OSCSVR_GetClientIP(*ps32SocketFd, aszClientIP))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession get client ip err!!\n");
        OSC_UNLOCK(pstServerCtx->Level1SessLock);
        return HI_FAILURE;
    }

    OSCSVR_Create_ClientLevel1Session(pstServerCtx, aszClientIP, s32SessTimeOut, pstServerCtx->pstLevel1Sess);
    pstServerCtx->u32SessionId ++;
    s32SessTimeOut = pstServerCtx->pstLevel1Sess->s32SessTimeOut;
    strncpy_s(aszSessionId, OSC_SESSIONID_MAX_LEN, pstServerCtx->pstLevel1Sess->aszSessionId, OSC_SESSIONID_MAX_LEN - 1);
    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_StartSession_JsonPack(s32SessTimeOut, OSC_MAX_PROTOCOL_BUFFER, aszSessionId, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession generate json packs error!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_START_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_CloseSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_BOOL bFound = HI_FALSE;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pOSC);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_CloseSession_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, aszComName, aszSessionId);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    OSC_LOCK(pstServerCtx->Level1SessLock);

    if (NULL != pstServerCtx->pstLevel1Sess)
    {
        if (0 == strncmp(pstServerCtx->pstLevel1Sess->aszSessionId, aszSessionId, OSC_SESSIONID_MAX_LEN))
        {
            bFound = HI_TRUE;
        }
    }


    if (bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession find the correspond session\n");

        if (NULL != pstServerCtx->pstLevel1Sess)
        {
            OSCSVR_Destroy_ClientLevel1Session(pstServerCtx, pstServerCtx->pstLevel1Sess);
            pstServerCtx->pstLevel1Sess = NULL;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the sessionid is invaild in CloseSession!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the sessionid is invaild");
        OSC_UNLOCK(pstServerCtx->Level1SessLock);

        goto Invaild;
    }

    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_CloseSession_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession generate json packs error!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ClosetSession send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_CLOSE_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_UpdateSession(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_BOOL bFound = HI_FALSE;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_S32  s32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_UpdateSession_JsonPack(aszJsonPack, aszComName, ps32SocketFd, aszSessionId, &s32SessTimeOut);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    OSC_LOCK(pstServerCtx->Level1SessLock);

    if (NULL != pstServerCtx->pstLevel1Sess)
    {
        if (0 == strncmp(pstServerCtx->pstLevel1Sess->aszSessionId, aszSessionId, OSC_SESSIONID_MAX_LEN))
        {
            bFound = HI_TRUE;
        }
    }

    if (bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession find the correspond session!\n");

        if ((0 == s32SessTimeOut) || (OSC_IGNORE_SESS_TIMEOUT == s32SessTimeOut))
        {
            pstServerCtx->pstLevel1Sess->s32SessTimeOut = pstServerCtx->s32TimeOut;
        }
        else
        {
            pstServerCtx->pstLevel1Sess->s32SessTimeOut = s32SessTimeOut;
        }

        s32SessTimeOut = pstServerCtx->pstLevel1Sess->s32SessTimeOut;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the sessionid is invaild in UpdateSession!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the sessionid is invaild");
        OSC_UNLOCK(pstServerCtx->Level1SessLock);

        goto Invaild;
    }

    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_UpdateSession_JsonPack(s32SessTimeOut, OSC_MAX_PROTOCOL_BUFFER, aszSessionId, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession generate json packs error!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_UPDATE_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_TakePicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32InvaildLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_SERVER_LEVEL_E enLevel = HI_OSC_SERVER_LEVEL_BUT;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_TakePicture_JsonPack(aszJsonPack, aszComName, ps32SocketFd, aszSessionId, &enLevel);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    if (!OSCSVR_CheckClient_IsInvaild(pstServerCtx, ps32SocketFd, aszSessionId, enLevel))
    {
        if (HI_OSC_SERVER_LEVEL_2 == enLevel)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture client is invaild!!\n");
            return HI_FAILURE;
        }

        memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
        s32JsonLen = 0;
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Sessionid is invaild");

        goto Invaild;
    }

    s32Ret = OSCSVR_Session_TakePicture(pOSC, ps32SocketFd, aszComName, enLevel, aszSessionId);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Takepicture session process error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_TAKE_PICTURE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Process_SetOption(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};
    HI_OSCSERVER_OPTIONS_S* pstSetOptions = NULL;
    HI_OSC_OPTIONS_TABLE_S* pstSetOptionsTab = NULL;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_OSC_SERVER_LEVEL_E enLevel = HI_OSC_SERVER_LEVEL_BUT;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions get http req body error!!\n");
        return HI_FAILURE;
    }

    /*malloc options data*/
    pstSetOptions = (HI_OSCSERVER_OPTIONS_S*)malloc(sizeof(HI_OSCSERVER_OPTIONS_S));

    if (NULL == pstSetOptions)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions malloc setoptions data fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstSetOptions, sizeof(HI_OSCSERVER_OPTIONS_S), 0x00, sizeof(HI_OSCSERVER_OPTIONS_S));
    pstSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INIT;

    /*malloc options table*/
    pstSetOptionsTab = (HI_OSC_OPTIONS_TABLE_S*)malloc(sizeof(HI_OSC_OPTIONS_TABLE_S));

    if (NULL == pstSetOptionsTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions malloc options table fail!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        return HI_FAILURE;
    }

    memset_s(pstSetOptionsTab, sizeof(HI_OSC_OPTIONS_TABLE_S), 0x00, sizeof(HI_OSC_OPTIONS_TABLE_S));

    if (HI_SUCCESS != OSCSVR_Parser_SetOptions_JsonPack(aszJsonPack, ps32SocketFd, aszSessionId, &enLevel, pstSetOptionsTab, pstSetOptions))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions the client is invaild!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_FAILURE;
    }

    /*check whether client is invaild*/
    if (HI_TRUE != OSCSVR_CheckClient_IsInvaild(pstServerCtx, ps32SocketFd, aszSessionId, enLevel))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions the client is invaild!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        return HI_FAILURE;
    }

    /*set the option after parser!!*/
    enOSCCommandKey = HI_OSC_COMMAND_SET_OPTIONS;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions check and get command fail!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnSetOption(enLevel, pstSetOptions, pstSetOptionsTab))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions perform callback fail!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        return HI_FAILURE;
    }

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_SetOptions_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions generate json packs error!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions send http response error!!\n");
        free(pstSetOptions);
        pstSetOptions = NULL;
        free(pstSetOptionsTab);
        pstSetOptionsTab = NULL;
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    free(pstSetOptions);
    pstSetOptions = NULL;
    free(pstSetOptionsTab);
    pstSetOptionsTab = NULL;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_GetOption(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};
    HI_OSCSERVER_OPTIONS_S* pstGetOptions = NULL;
    HI_OSC_OPTIONS_TABLE_S* pstGetOptionsTab = NULL;
    HI_OSC_SERVER_LEVEL_E enLevel = HI_OSC_SERVER_LEVEL_BUT;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions get http req body error!!\n");
        return HI_FAILURE;
    }

    /*malloc options data*/
    pstGetOptions = (HI_OSCSERVER_OPTIONS_S*)malloc(sizeof(HI_OSCSERVER_OPTIONS_S));

    if (NULL == pstGetOptions)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions malloc setoptions data fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstGetOptions, sizeof(HI_OSCSERVER_OPTIONS_S), 0x00, sizeof(HI_OSCSERVER_OPTIONS_S));
    pstGetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INIT;

    /*malloc options table*/
    pstGetOptionsTab = (HI_OSC_OPTIONS_TABLE_S*)malloc(sizeof(HI_OSC_OPTIONS_TABLE_S));

    if (NULL == pstGetOptionsTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions malloc options table fail!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        return HI_FAILURE;
    }

    memset_s(pstGetOptionsTab, sizeof(HI_OSC_OPTIONS_TABLE_S), 0x00, sizeof(HI_OSC_OPTIONS_TABLE_S));

    if (HI_SUCCESS != OSCSVR_Parser_GetOptions_JsonPack(aszJsonPack, ps32SocketFd, aszSessionId, &enLevel, pstGetOptionsTab, pstGetOptions))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions parser json package err!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_FAILURE;
    }

    /*check whether client is invaild*/
    if (HI_TRUE != OSCSVR_CheckClient_IsInvaild(pstServerCtx, ps32SocketFd, aszSessionId, enLevel))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions the client is invaild!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;

        return HI_FAILURE;
    }

    /*get the option after parser!!*/
    enOSCCommandKey = HI_OSC_COMMAND_GET_OPTIONS;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions check and get command fail!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnGetOption(enLevel, pstGetOptions, pstGetOptionsTab))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions perform callback fail!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;

        return HI_FAILURE;
    }

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_GetOptions_JsonPack(enLevel, OSC_MAX_PROTOCOL_BUFFER, pstGetOptions, pstGetOptionsTab, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions generate json packs error!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;

        return HI_FAILURE;
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions reply is %s\n", aszJsonPack);

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getoptions send http response error!!\n");
        free(pstGetOptions);
        pstGetOptions = NULL;
        free(pstGetOptionsTab);
        pstGetOptionsTab = NULL;

        return HI_FAILURE;
    }

    free(pstGetOptions);
    pstGetOptions = NULL;
    free(pstGetOptionsTab);
    pstGetOptionsTab = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_Reset(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client IP is Invaild in level2\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet get http req body error!!\n");
        return HI_FAILURE;
    }

    /*judge whether the clinet is invaild*/
    s32Ret = OSCSVR_Parser_Reset_JsonPack(aszJsonPack, ps32SocketFd, aszComName);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    enOSCCommandKey = HI_OSC_COMMAND_RESET;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet check and get command fail!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnReset())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "perform ReSet callback error!!\n");
        return HI_FAILURE;
    }

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_Reset_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet generate json packs error!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Reset send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_StartCapture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_BOOL bDisabled = HI_FALSE;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszCommandID[OSC_SVR_COMMAND_ID_LEN] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_INIT_PATH;
    HI_OSC_CAPTURE_MODE_E enCaptureMode = HI_OSC_CAPTURE_MODE_INIT;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    enPathType = HI_OSC_ABSOLUTE_PATH;
    enOSCCommandKey = HI_OSC_COMMAND_START_CAPTURE;

    if (u32ReqLen > strlen(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture input request len is wrong!!\n");
        return HI_FAILURE;
    }

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture search command fail!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Get_CommandID(pstServerCtx, aszCommandID))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture search command fail!!\n");
        return HI_FAILURE;
    }

    /*create command status*/
    if (HI_SUCCESS != OSCSVR_Create_CommandStatus(pstServerCtx, HI_OSC_SERVER_LEVEL_2, (HI_CHAR*)OSC_NAME_START_CAPTURE, aszCommandID, HIOSC_COMM_STATUS_INPROGRESS, 0.00f))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture create command status fail!!\n");
        return HI_FAILURE;
    }

    /*video capture*/
    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnStartCapture(pOSC, enPathType, aszCommandID, &bDisabled, &enCaptureMode))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture perform callback fail!!\n");
        OSCSVR_Delete_CommandStatus(pstServerCtx, aszCommandID);

        if (HI_SUCCESS != OSCSVR_Send_VideoCaptureResponse(ps32SocketFd, (HI_CHAR*)OSC_COMM_STATE_ERROR))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture send err response fail!!\n");
            return HI_FAILURE;
        }

        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    if (bDisabled)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture is busy currently!!\n");
        OSCSVR_Delete_CommandStatus(pstServerCtx, aszCommandID);
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_DISABLED_COMMAND;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Command is currently disabled");

        goto Invaild;
    }

    if (HI_OSC_CAPTURE_MODE_VIDEO == enCaptureMode)
    {
        OSCSVR_Delete_CommandStatus(pstServerCtx, aszCommandID);

        if (NULL != pstServerCtx->pstLevel2Sess)
        {
            pstServerCtx->pstLevel2Sess->bCapture = HI_TRUE;
        }

        if (HI_SUCCESS != OSCSVR_Send_VideoCaptureResponse(ps32SocketFd, (HI_CHAR*)OSC_COMM_STATE_DONE))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture send video response fail!!\n");
            return HI_FAILURE;
        }
    }
    else if (HI_OSC_CAPTURE_MODE_INTERVAL == enCaptureMode)
    {
        if (HI_SUCCESS != OSCSVR_Send_StartCapture_Response(aszCommandID, ps32SocketFd))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture send interval response fail!!\n");
            return HI_FAILURE;
        }

        return HI_SUCCESS;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_START_CAPTURE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_StopCapture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_S32 s32Looper = 0;
    HI_BOOL bDisabled = HI_FALSE;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_INIT_PATH;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszCaptureUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    if (u32ReqLen > strlen(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture input request len is wrong!!\n");
        return HI_FAILURE;
    }

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszCaptureUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    enPathType = HI_OSC_ABSOLUTE_PATH;

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture the client IP is Invaild in level2\n");

        return HI_FAILURE;
    }

    /*get the registered command*/
    enOSCCommandKey = HI_OSC_COMMAND_STOP_CAPTURE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture check and get command getoption fail!!\n");

        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnStopCapture(enPathType, aszCaptureUrl, &bDisabled))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture callback err!!\n");

        return HI_FAILURE;
    }

    if (bDisabled)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture is disbaled currently!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_DISABLED_COMMAND;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Command is currently disabled");

        goto Invaild;
    }

    pstServerCtx->pstLevel2Sess->bCapture = HI_FALSE;

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_StopCapture_JsonPack(aszCaptureUrl, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture generate json packs error!!\n");

        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_STOP_CAPTURE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_GetLivePreview(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_BOOL bDisabled = HI_FALSE;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    if (u32ReqLen > strlen(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview input request len is wrong!!\n");
        return HI_FAILURE;
    }

    if (!OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview command check client ip in level2 fail!!\n");
        return HI_FAILURE;
    }

    enOSCCommandKey = HI_OSC_COMMAND_GET_PRELIVE_VIEW;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview check and get command fail!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnGetLivePreview(pOSC, ps32SocketFd, &bDisabled))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview callback perform fail!!\n");
        return HI_FAILURE;
    }

    if (bDisabled)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview is disabled currently!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_UNABLE_SERVICE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Command is currently disabled");

        goto Invaild;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_LIVEPREVIEW, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetLivePreview callback invaild is\n%s\n", aszInvaildReply);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Process_PrivateCommand(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32JsonLen = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszCommName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_PRIVATE;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_Name_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, aszCommName);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Private command parser json err!!\n");

        return HI_FAILURE;
    }

    /*get the registered command*/
    enOSCCommandKey = HI_OSC_COMMAND_PRIVATE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Private command check and get command fail!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnPrivateCom(aszJsonPack, aszCommName, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Private command cb perform fail!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
