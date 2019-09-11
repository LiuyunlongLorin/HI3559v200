/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_osc_server.c
* @brief     oscserver
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
/*osc module header file*/
#include "hi_osc_log.h"
#include "hi_osc_server.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_http.h"
#include "hi_oscserver_jsonparser.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_com_exec.h"
#include "hi_oscserver_session.h"
#include "hi_oscserver_jsonpackage.h"
#include "hi_oscserver_file_exec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#ifndef MW_VERSION
#define MW_VERSION "0.0.0"
#endif
#define OSC_DEFAULT_TIMEOUT                     (300)
#define HI_OSC_MAX_LIVEPREVIEW_HEADER_LEN       (512)
#define HI_OSC_MAX_REQUEST_LEN                  (1024)

static HI_OSC_SERVER_CTX_S* s_pstOSCServerCtx = NULL;
pthread_mutex_t s_OSCServerCtxLock = PTHREAD_MUTEX_INITIALIZER;              /*the lock of OSCServer*/
HI_BOOL g_bExit = HI_FALSE;

#define OSCSVR_CHECK_NOT_CREATE(condition) \
    do \
    { \
        if(condition == NULL) \
        { \
            OSC_UNLOCK(s_OSCServerCtxLock); \
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer not create!\n"); \
            return HI_ERR_OSCSERVER_ERR_NOT_CREATE; \
        }\
    }while(0)

static HI_S32 OSCSVR_CheckOSCConfig(HI_OSC_CONFIG_S* pstOSCConfig)
{
    OSCSVR_CHECK_NULL_ERROR(pstOSCConfig);

    if ((pstOSCConfig->s32ListFiles < 0) || (pstOSCConfig->s32ListFiles > HI_OSC_LISTFILE_MAX_NUM))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, " param s32ListFiles not in the range  max:%d!\n", HI_OSC_LISTFILE_MAX_NUM);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Check_DistribLinkArgs(HI_MW_PTR hOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    OSCSVR_CHECK_NULL_ERROR(hOSC);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)hOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");

        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_FAILURE;
    }

    OSC_UNLOCK(s_OSCServerCtxLock);

    /*in case that the len is wrong*/
    if ((u32ReqLen > strlen(pClientReq)) || (u32ReqLen > HI_OSC_MAX_REQUEST_LEN))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC server input the len is invaild!!\n");
        return HI_FAILURE;
    }

    if (*ps32SocketFd == HI_OSC_INVALID_SOCKET)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC server input socketfd is invaild!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/**
 * @brief create server instance
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @param[in] pstOSCConfig HI_OSC_CONFIG_S : osc config info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_Create(HI_MW_PTR* ppOSC, HI_OSC_CONFIG_S* pstOSCConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_OSC_SERVER_CTX_S* pstServerCtx = NULL;

    OSCSVR_CHECK_NULL_ERROR(ppOSC);

    if (NULL == pstOSCConfig)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCconfig input parameters null !\n");
        return HI_ERR_OSCSERVER_NULL_PTR;
    }

    s32Ret = OSCSVR_CheckOSCConfig(pstOSCConfig);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCconfig input parameters err!\n");
        return HI_ERR_OSCSERVER_ILLEGAL_PARAM;
    }

    OSC_LOCK(s_OSCServerCtxLock);

    if (NULL != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCserver already created do not create again!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_ERR_CREATE_AGAIN;
    }

    pstServerCtx = (HI_OSC_SERVER_CTX_S*)malloc(sizeof(HI_OSC_SERVER_CTX_S));

    if (NULL == pstServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCserver malloc OSC server context failed\n");
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_ERR_MALLOC_FAIL;
    }

    memset_s(pstServerCtx, sizeof(HI_OSC_SERVER_CTX_S), 0x00, sizeof(HI_OSC_SERVER_CTX_S));

    pstServerCtx->s32ListFiles = pstOSCConfig->s32ListFiles;

    if (pstOSCConfig->s32TimeOut <= 0)
    {
        pstServerCtx->s32TimeOut = OSC_DEFAULT_TIMEOUT;
    }
    else
    {
        pstServerCtx->s32TimeOut = pstOSCConfig->s32TimeOut;
    }

    pstServerCtx->u32ThrottleTimeout = OSCSVR_CHECKFORUPDATE_WAIT_TIME;
    pstServerCtx->bExclusiveUseFlag = pstOSCConfig->bExclusiveUseFlag;
    pstServerCtx->u32SessionId = OSCSVR_INIT_SESSION_ID;
    pstServerCtx->u32CommandId = OSCSVR_INIT_COMMAND_ID;
    pstServerCtx->u32FingerId = OSCSVR_INIT_FINGERPRINT_ID;

    HI_LIST_INIT_HEAD_PTR(&pstServerCtx->pCommStatuslist);
    HI_LIST_INIT_HEAD_PTR(&pstServerCtx->pRegCommandlist);
    pthread_mutex_init(&pstServerCtx->CommStatusLock, NULL);
    pthread_mutex_init(&pstServerCtx->RegCommandlistLock, NULL);
    pthread_mutex_init(&pstServerCtx->Level1SessLock, NULL);
    pthread_mutex_init(&pstServerCtx->Level2SessLock, NULL);
    pthread_mutex_init(&pstServerCtx->StateLock, NULL);

    pstServerCtx->bExitLooper = HI_FALSE;
    *ppOSC = (HI_MW_PTR)pstServerCtx;
    s_pstOSCServerCtx = pstServerCtx;
    g_bExit = HI_FALSE;
    OSC_UNLOCK(s_OSCServerCtxLock);

    return HI_SUCCESS;
}

/**
 * @brief process osc http request, support multiple thread
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @param[in] ps32SocketFd  : socket fd
 * @param[in] pClientReq  : client request
 * @param[in] u32ReqLen  : request len
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_HTTP_DistribLink (HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_OSC_REQ_API_E enMethod = HI_OSC_INVAILD_API;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32RespLen = HI_SUCCESS;

    OSC_LOCK(s_OSCServerCtxLock);

    /*when the app need to exit, the osc mosdule not process the request again*/
    if (g_bExit == HI_TRUE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC server need to exit, no longer to process the request!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_ERR_NEED_EXIT;
    }

    OSC_UNLOCK(s_OSCServerCtxLock);

    OSCSVR_CHECK_NULL_ERROR(pOSC);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    if (HI_SUCCESS != OSCSVR_Check_DistribLinkArgs(pOSC, ps32SocketFd, pClientReq, u32ReqLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC server input args is invaild!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);

        return HI_ERR_OSCSERVER_ILLEGAL_PARAM;
    }

    if (!HI_OSCSVR_MSGParser_CheckRequest(pClientReq, u32ReqLen, &enMethod))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invalid http request, not procss the request!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);

        return HI_ERR_OSCSERVER_ERR_SESS_BAD_REQUEST;
    }

    /*judge which command need to perform*/
    switch (enMethod)
    {
        case HI_OSC_INFO_API:
            s32Ret = HI_OSCSVR_Process_Info(pOSC, ps32SocketFd, pClientReq, u32ReqLen);
            break;

        case HI_OSC_STATE_API:
            s32Ret = HI_OSCSVR_Process_State(pOSC, ps32SocketFd, pClientReq, u32ReqLen);
            break;

        case HI_OSC_CHECK_FORUPDATES_API:
            s32Ret = HI_OSCSVR_Process_CheckForUpdate(pOSC, ps32SocketFd, pClientReq, u32ReqLen);
            break;

        case HI_OSC_COMMAND_EXECUTE_API:
            s32Ret = HI_OSCSVR_Process_Command_Exec(pOSC, ps32SocketFd, pClientReq, u32ReqLen);
            break;

        case HI_OSC_COMMAND_STATUS_API:
            s32Ret = HI_OSCSVR_Process_Command_Status(pOSC, ps32SocketFd, pClientReq, u32ReqLen);
            break;

        default:
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invalid http request, just ignore this connection\n");
            s32Ret = HI_ERR_OSCSERVER_ERR_API_FAIL;
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            goto Failed;
    }

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Process osc API failed !!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);

        return HI_ERR_OSCSERVER_ERR_API_FAIL;
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

    /*inaild request not cause oscserver close*/
    return HI_SUCCESS;
}

/**
 * @brief register callback and private command, support multiple thread
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @param[in] HI_OSC_PRIVATE_COMMAND_S  : private command key
 * @param[in] pfnEventCallback  : callback fn address
 * @param[in] pszCommandName  : command name
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_RegisterEventCallback(HI_MW_PTR pOSC, HI_OSC_API_COMMAND_E enEvent, HI_VOID* pfnEventCallback, HI_CHAR* pszCommandName)
{
    List_Head_S* pstPosNode = NULL;
    HI_S32 s32CommandLen = 0;
    HI_OSC_REGCOMMAND_NODE_S* pstRegCommandNode = NULL;
    HI_OSC_REGCOMMAND_NODE_S* pstRegCommandTemp = NULL;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pfnEventCallback);
    OSCSVR_CHECK_NULL_ERROR(pszCommandName);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }


    s32CommandLen = strlen(pszCommandName);

    if ((s32CommandLen == 0) || (s32CommandLen >= HI_OSC_COMMAND_NAME_MAX_LEN))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Reg callback the input param len is err!!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_REGCOMMAND_INPUT_ERR;
    }

    OSC_LOCK(pstServerCtx->RegCommandlistLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pRegCommandlist)
    {
        pstRegCommandNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_REGCOMMAND_NODE_S, listPtr);

        if (NULL != pstRegCommandNode)
        {
            /*judge whether the callback has been registered, if registered need to cover*/
            if (pstRegCommandNode->enOSCRegKey == enEvent)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "The CallBack has already been Registered!!\n");
                pstRegCommandNode->pfnEventCB = pfnEventCallback;
                memset_s(pstRegCommandNode->aszCommandName, HI_OSC_COMMAND_NAME_MAX_LEN, 0x00, HI_OSC_COMMAND_NAME_MAX_LEN);
                strncpy_s(pstRegCommandNode->aszCommandName, HI_OSC_COMMAND_NAME_MAX_LEN, pszCommandName, strlen(pszCommandName) + 1);
                OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
                OSC_UNLOCK(s_OSCServerCtxLock);
                return HI_SUCCESS;
            }
        }
    }
    /*create new node to memory the callback*/
    pstRegCommandTemp = (HI_OSC_REGCOMMAND_NODE_S*)malloc(sizeof(HI_OSC_REGCOMMAND_NODE_S));

    if (NULL == pstRegCommandTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Malloc fail in Regging new command into osc module!!\n");
        OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_REGCOMMAND_MALLOC_FAIL;
    }

    memset_s((HI_VOID*)pstRegCommandTemp, sizeof(HI_OSC_REGCOMMAND_NODE_S), 0x00, sizeof(HI_OSC_REGCOMMAND_NODE_S));

    pstRegCommandTemp->enOSCRegKey = enEvent;
    pstRegCommandTemp->pfnEventCB = pfnEventCallback;
    strncpy_s(pstRegCommandTemp->aszCommandName, HI_OSC_COMMAND_NAME_MAX_LEN, pszCommandName, strlen(pszCommandName) + 1);
    HI_List_Add(&(pstRegCommandTemp->listPtr), &(pstServerCtx->pRegCommandlist));
    OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
    OSC_UNLOCK(s_OSCServerCtxLock);

    return HI_SUCCESS;
}

/**
 * @brief destroy server instance, support multiple thread
 * @param[in] handle HI_HANDLE : oscserver handle
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_Destroy(HI_MW_PTR* ppOSC)
{
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_OSC_REGCOMMAND_NODE_S* pstRegCommandNode = NULL;
    HI_OSC_COMM_STATUS_NODE_S* pstComStatusNode = NULL;

    OSCSVR_CHECK_NULL_ERROR(ppOSC);
    OSCSVR_CHECK_NULL_ERROR(*ppOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)(*ppOSC);

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }

    OSC_LOCK(pstServerCtx->RegCommandlistLock);

    /* delete each command list node*/
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pRegCommandlist)
    {
        pstRegCommandNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_REGCOMMAND_NODE_S, listPtr);

        if (NULL != pstRegCommandNode)
        {
            HI_List_Del(&(pstRegCommandNode->listPtr));
            free(pstRegCommandNode);
            pstRegCommandNode = NULL;
        }
    }
    OSC_UNLOCK(pstServerCtx->RegCommandlistLock);

    pstPosNode = NULL;
    pstTmpNode = NULL;
    OSC_LOCK(pstServerCtx->CommStatusLock);
    /* delete each command status node*/
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pCommStatuslist)
    {
        pstComStatusNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_COMM_STATUS_NODE_S, listPtr);

        if (NULL != pstComStatusNode)
        {
            HI_List_Del(&(pstComStatusNode->listPtr));
            free(pstComStatusNode);
            pstComStatusNode = NULL;
        }
    }
    OSC_UNLOCK(pstServerCtx->CommStatusLock);

    OSC_LOCK(pstServerCtx->Level1SessLock);

    /*destroy level1 session*/
    if (NULL != pstServerCtx->pstLevel1Sess)
    {
        OSCSVR_Destroy_ClientLevel1Session(pstServerCtx, pstServerCtx->pstLevel1Sess);
        pstServerCtx->pstLevel1Sess = NULL;
    }

    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    OSC_LOCK(pstServerCtx->Level2SessLock);

    /*destroy level2 session*/
    if (NULL != pstServerCtx->pstLevel2Sess)
    {
        OSCSVR_Destroy_ClientLevel2Session(pstServerCtx, pstServerCtx->pstLevel2Sess);
        pstServerCtx->pstLevel2Sess = NULL;
    }

    OSC_UNLOCK(pstServerCtx->Level2SessLock);

    /*destroy all the lock*/
    pthread_mutex_destroy(&pstServerCtx->RegCommandlistLock);
    pthread_mutex_destroy(&pstServerCtx->CommStatusLock);
    pthread_mutex_destroy(&pstServerCtx->Level1SessLock);
    pthread_mutex_destroy(&pstServerCtx->Level2SessLock);
    pthread_mutex_destroy(&pstServerCtx->StateLock);

    free(pstServerCtx);
    pstServerCtx = NULL;
    s_pstOSCServerCtx = NULL;
    *ppOSC = NULL;
    /*close and destroy gobal lock of OSCServer*/
    OSC_UNLOCK(s_OSCServerCtxLock);

    return HI_SUCCESS;
}

/**
 * @brief add command status into server instance, support multiple thread
 * @param[in] handle HI_HANDLE : oscserver handle
 * @param[in] pszCommandId HI_CHAR* : command id
 * @param[in] enComStatus HI_OSC_COMM_STATUS_E : the enum of command status
 * @param[in] aszResults HI_CHAR : the results of command
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_Update_CommandStatus(HI_MW_PTR pOSC, HI_CHAR* pszCommandId, HI_OSC_COMM_STATUS_E enComStatus, HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN])
{
    HI_BOOL bFound = HI_FALSE;
    HI_S32 s32Looper = 0;
    List_Head_S* pstPosNode = NULL;
    HI_OSC_COMM_STATUS_NODE_S* pstCommandStatusNode = NULL;

    OSCSVR_CHECK_NULL_ERROR(pszCommandId);
    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(s_pstOSCServerCtx);

    /*Lock the gobal lock*/
    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }

    if (HIOSC_COMM_STATUS_INIT == enComStatus)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer status invaild!!!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_ILLEGAL_PARAM;
    }

    OSC_LOCK(pstServerCtx->CommStatusLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pCommStatuslist)
    {
        pstCommandStatusNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_COMM_STATUS_NODE_S, listPtr);

        if (0 == strncmp(pstCommandStatusNode->aszCommandID, pszCommandId, OSC_SVR_COMMAND_ID_LEN))
        {
            bFound = HI_TRUE;
            break;
        }
    }

    if (!bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR not find the correspond id command status!\n");
        OSC_UNLOCK(pstServerCtx->CommStatusLock);
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_INVAILD_COMMANDID;
    }

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper++)
    {
        memset_s(pstCommandStatusNode->aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    /*update the command status*/
    pstCommandStatusNode->enComStatus = enComStatus;

    switch (enComStatus)
    {
        case HIOSC_COMM_STATUS_INPROGRESS:
            pstCommandStatusNode->dCompletion = HI_OSC_COMMAND_STATUS_PROGRESS;
            break;

        case HIOSC_COMM_STATUS_DONE:
            pstCommandStatusNode->dCompletion = HI_OSC_COMMAND_STATUS_COMPLETE;

            for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
            {
                strncpy_s(pstCommandStatusNode->aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, aszResults[s32Looper] , strlen(aszResults[s32Looper]) + 1);
                pstCommandStatusNode->aszResults[s32Looper][strlen(aszResults[s32Looper])] = '\0';
            }

            break;

        case HIOSC_COMM_STATUS_ERROR:
            pstCommandStatusNode->dCompletion = HI_OSC_COMMAND_STATUS_COMPLETE;

            for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
            {
                strncpy_s(pstCommandStatusNode->aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, aszResults[s32Looper] , strlen(aszResults[s32Looper]) + 1);
                pstCommandStatusNode->aszResults[s32Looper][strlen(aszResults[s32Looper])] = '\0';
            }

            break;

        default :
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR not find the correspond id command status!\n");

            OSC_UNLOCK(pstServerCtx->CommStatusLock);
            OSC_UNLOCK(s_OSCServerCtxLock);
            return HI_ERR_OSCSERVER_INVAILD_COMMAND_STATUS;
    }

    OSC_UNLOCK(pstServerCtx->CommStatusLock);
    OSC_UNLOCK(s_OSCServerCtxLock);

    return HI_SUCCESS;
}

/**
 * @brief send frame data, support multiple thread
 * @param[in] handle HI_HANDLE : oscserver handle
 * @param[in] bFirstFrame HI_BOOL : first frame flag
 * @param[in] s32SocketFd HI_S32 : the fd of socket
 * @param[in] pcFrameData HI_CHAR : the data of frame
 * @param[in] s32FrameLen HI_S32 : the len of frame
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_Send_FrameData(HI_MW_PTR pOSC, HI_BOOL bFirstFrame, HI_S32* ps32SocketFd, HI_CHAR* pcFrameData, HI_S32 s32FrameLen)
{
    HI_CHAR* pcResponse = NULL;
    HI_S32   s32RespLen = 0;
    HI_S32   s32BuffSize = 0;

    OSCSVR_CHECK_NULL_ERROR(pcFrameData);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }

    OSC_UNLOCK(s_OSCServerCtxLock);

    if (HI_OSC_INVALID_SOCKET == *ps32SocketFd)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Send frame data with invaild socketfd!\n");
        return HI_ERR_OSCSERVER_SOCKETFD_INVAILD;
    }

    if (s32FrameLen <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Frame Send input framelength is err!!len: %d\n", s32FrameLen);
        return HI_ERR_OSCSERVER_FRAME_INPUT_LENGTH_INVAILD;
    }

    s32BuffSize = HI_OSC_MAX_LIVEPREVIEW_HEADER_LEN + s32FrameLen;

    pcResponse = (HI_CHAR*)malloc(s32BuffSize);

    if (NULL == pcResponse)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Frame Send malloc fail!");
        return HI_ERR_OSCSERVER_FRAME_MALLOC_FAIL;
    }

    memset_s((HI_VOID*)pcResponse, s32BuffSize, 0x00, s32BuffSize);

    if (bFirstFrame)
    {
        OSCSVR_Generate_GetLivePreview_HTTPHeader(pcResponse, s32BuffSize, &s32RespLen);
    }

    s32RespLen += snprintf_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, s32BuffSize - s32RespLen, "%s: image/jpeg\r\n", OSC_HTTP_HEADER_CONTENTTYPE);
    s32RespLen += snprintf_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, s32BuffSize - s32RespLen, "%s: %d\r\n", OSC_HTTP_HEADER_CONTENTLENGTH, s32FrameLen);
    s32RespLen += snprintf_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, s32BuffSize - s32RespLen, "\r\n");
    memcpy_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, pcFrameData, s32FrameLen);
    s32RespLen = s32RespLen + s32FrameLen;
    s32RespLen += snprintf_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, s32BuffSize - s32RespLen, "\r\n\r\n");
    s32RespLen += snprintf_s(pcResponse + s32RespLen, s32BuffSize - s32RespLen, s32BuffSize - s32RespLen, "%s\r\n", OSC_GETLIVEPREVIEW_BOUNDARY);

    /*Send osc response*/
    if (HI_SUCCESS != HI_OSCSVR_Response_Send(*ps32SocketFd, pcResponse, s32RespLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Frame Send reponse fail!!\n");
        free(pcResponse);
        pcResponse = NULL;

        return HI_ERR_OSCSERVER_FRAME_FRAME_SEND_FAIL;
    }

    free(pcResponse);
    pcResponse = NULL;
    return HI_SUCCESS;
}

/**
 * @brief add the newest state into server instance.
 * @param[in] handle HI_HANDLE : oscserver handle
 * @param[in] dBatteryLevel HI_DOUBLE : the level of battery
 * @param[in] bStorageChange HI_BOOL : the storage change flag
 * @param[in] pszStorageUri HI_CHAR : the uri of storage
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_UpdateState(HI_MW_PTR pOSC, HI_OSC_DEV_STATE_E enState, HI_VOID* pUpdates)
{
    OSCSVR_CHECK_NULL_ERROR(pOSC);
    OSCSVR_CHECK_NULL_ERROR(pUpdates);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");
        OSC_UNLOCK(s_OSCServerCtxLock);

        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }

    switch (enState)
    {
        case HIOSC_DEV_STATE_BATTERY_LEVEL:
            OSC_LOCK(pstServerCtx->StateLock);
            pstServerCtx->stChangeState.dBatteryLevel = *((HI_DOUBLE*)pUpdates);
            OSC_UNLOCK(pstServerCtx->StateLock);
            break;

        case HIOSC_DEV_STATE_STORAGE_CHANGE:
            OSC_LOCK(pstServerCtx->StateLock);
            pstServerCtx->stChangeState.bStorageChanged = *((HI_BOOL*)pUpdates);
            OSC_UNLOCK(pstServerCtx->StateLock);
            break;

        case HIOSC_DEV_STATE_STORAGE_URL:
            if (strlen((HI_CHAR*)pUpdates) + 1 > HI_OSCSVR_STORAGE_URL_MAX_LEN)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC Module Change state input err! the uri len is long!!\n");
                OSC_UNLOCK(s_OSCServerCtxLock);

                return HI_ERR_OSCSERVER_ERR_CHANGE_STATE_FAIL;
            }

            OSC_LOCK(pstServerCtx->StateLock);
            strncpy_s(pstServerCtx->stChangeState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, (HI_CHAR*)pUpdates, strlen((HI_CHAR*)pUpdates) + 1);
            OSC_UNLOCK(pstServerCtx->StateLock);
            break;

        default :
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer enum value is invaild!!!\n");
            OSC_UNLOCK(s_OSCServerCtxLock);

            return HI_ERR_OSCSERVER_ILLEGAL_PARAM;
    }

    OSC_UNLOCK(s_OSCServerCtxLock);
    return HI_SUCCESS;
}

/**
 * @brief check whether the server can exit
 * @param[in] handle HI_HANDLE : oscserver handle
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_ProcessExit(HI_MW_PTR pOSC)
{
    OSCSVR_CHECK_NULL_ERROR(pOSC);

    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(s_OSCServerCtxLock);
    OSCSVR_CHECK_NOT_CREATE(s_pstOSCServerCtx);

    if (pstServerCtx != s_pstOSCServerCtx)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCServer address invaild!\n");

        OSC_UNLOCK(s_OSCServerCtxLock);
        return HI_ERR_OSCSERVER_HANDLE_INVALID;
    }

    /*when the osc module informed exited, no longer to process request again*/
    g_bExit = HI_TRUE;

    /*Make sure osc module not block the main thread*/
    pstServerCtx->bExitLooper = HI_TRUE;

    OSC_UNLOCK(s_OSCServerCtxLock);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
