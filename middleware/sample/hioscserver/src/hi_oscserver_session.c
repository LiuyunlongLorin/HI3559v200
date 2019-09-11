/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_session.c
* @brief     oscserver session src file
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
#include "hi_oscserver_http.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_jsonparser.h"
#include "hi_oscserver_jsonpackage.h"
#include "hi_oscserver_com_exec.h"
#include "hi_oscserver_session.h"
#include "hi_oscserver_file_exec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*the command callback*/
static HI_OSC_COMMAND_EXEC_S s_stCommandExec[] =
{
    {HI_OSC_COMMAND_START_SESSION, OSCSVR_Process_StartSession},         /*1. startsession*/
    {HI_OSC_COMMAND_UPDATE_SESSION, OSCSVR_Process_UpdateSession},       /*2. updatesession*/
    {HI_OSC_COMMAND_CLOSE_SESSION, OSCSVR_Process_CloseSession},         /*3. closesession*/
    {HI_OSC_COMMAND_TAKE_PICTURE, OSCSVR_Process_TakePicture},           /*4. TakePicture*/
    {HI_OSC_COMMAND_GET_PRELIVE_VIEW, OSCSVR_Process_GetLivePreview},    /*5. GetLivePreview*/
    {HI_OSC_COMMAND_LIST_FILE, OSCSVR_Process_ListFiles},                 /*6. ListFiles*/
    {HI_OSC_COMMAND_LIST_IMAGE, OSCSVR_Process_ListImages},               /*7. ListImages*/
    {HI_OSC_COMMAND_GET_METADATA, OSCSVR_Process_GetMetadata},            /*8. GetMetadata*/
    {HI_OSC_COMMAND_GET_IMAGE, OSCSVR_Process_GetImage},                   /*9. GetImage*/
    {HI_OSC_COMMAND_DELETE, OSCSVR_Process_Delete},                        /*10. Delete*/
    {HI_OSC_COMMAND_PROCESS_PICTURE, OSCSVR_Process_ProcessPicture},     /*11. ProcessPicture*/
    {HI_OSC_COMMAND_SET_OPTIONS, OSCSVR_Process_SetOption},               /*12. SetOption*/
    {HI_OSC_COMMAND_GET_OPTIONS, OSCSVR_Process_GetOption},               /*13. GetOption*/
    {HI_OSC_COMMAND_RESET, OSCSVR_Process_Reset},                          /*14. Reset*/
    {HI_OSC_COMMAND_START_CAPTURE, OSCSVR_Process_StartCapture},          /*15. StartCapture*/
    {HI_OSC_COMMAND_STOP_CAPTURE, OSCSVR_Process_StopCapture},            /*16. StopCapture*/
    {HI_OSC_COMMAND_SWITCH_WIFI, OSCSVR_Process_SwitchWifi},              /*17. SwitchWifi*/
    {HI_OSC_COMMAND_UPLOAD_FILE, OSCSVR_Process_UploadFile},              /*18. UploadFile*/
    {HI_OSC_COMMAND_PRIVATE, OSCSVR_Process_PrivateCommand},             /*19. Private*/
    {HI_OSC_COMMAND_INVAILD, NULL}
};

HI_S32 OSCSVR_GetClientIP(HI_SOCKET s32Sockfd, HI_CHAR* aszClientIP)
{
    HI_CHAR* pcIPTemp = NULL;
    HI_S32 s32AddrLen  = 0;
    struct sockaddr_in stSockAddr;

    memset_s(&stSockAddr, sizeof(struct sockaddr_in), 0, sizeof(struct sockaddr_in));
    s32AddrLen = sizeof(stSockAddr);

    if (0 != getpeername(s32Sockfd, (struct sockaddr*)(&stSockAddr), (socklen_t*)&s32AddrLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR getpeername error.\n");
        return HI_FAILURE;
    }

    pcIPTemp = inet_ntoa(stSockAddr.sin_addr);

    if (NULL == pcIPTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR get ip address error.\n");
        return  HI_FAILURE;
    }

    strncpy_s(aszClientIP, OSC_IP_MAX_LEN, pcIPTemp, OSC_IP_MAX_LEN - 1);
    aszClientIP[OSC_IP_MAX_LEN - 1] = '\0';

    return HI_SUCCESS;
}

HI_BOOL OSCSVR_CheckClient_IsInvaild(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E enLevel)
{
    HI_BOOL bFound = HI_FALSE;
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN] = {0};

    if (HI_SUCCESS != OSCSVR_GetClientIP(*ps32SocketFd, aszClientIP))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get ip err!!\n");
        return HI_FALSE;
    }

    if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client IP is Invaild in level2\n");
            return HI_FALSE;
        }

        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client is level2\n");
        return HI_TRUE;
    }
    else if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        OSC_LOCK(pstServerCtx->Level1SessLock);

        /*check whether sessionid is invaild*/
        if (NULL != pstServerCtx->pstLevel1Sess)
        {
            if (0 == strncmp(pstServerCtx->pstLevel1Sess->aszSessionId, pszSessionId, OSC_SESSIONID_MAX_LEN))
            {
                bFound = HI_TRUE;
            }
        }

        OSC_UNLOCK(pstServerCtx->Level1SessLock);

        if (bFound)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "osc find the correspond session in level1\n");
            return HI_TRUE;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "osc not find the correspond session in level1\n");
            return HI_FALSE;
        }
    }
    else
    {
        /*version is err!!*/
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the level of version is err!!\n");
        return HI_FALSE;
    }

    return HI_TRUE;
}

HI_S32 OSCSVR_Delete_CommandStatus(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszCommandId)
{
    HI_BOOL bExist = HI_FALSE;
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_OSC_COMM_STATUS_NODE_S* pstCommandStatusInfo = NULL;

    OSCSVR_CHECK_NULL_ERROR(pszCommandId);
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(pstServerCtx->CommStatusLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pCommStatuslist)
    {
        pstCommandStatusInfo = HI_LIST_ENTRY(pstPosNode, HI_OSC_COMM_STATUS_NODE_S, listPtr);

        if (NULL != pstCommandStatusInfo)
        {
            if (0 == strncmp(pstCommandStatusInfo->aszCommandID, pszCommandId, strlen(pszCommandId) + 1))
            {
                HI_List_Del(&(pstCommandStatusInfo->listPtr));
                memset_s((HI_VOID*)pstCommandStatusInfo, sizeof(HI_OSC_COMM_STATUS_NODE_S), 0x00 , sizeof(HI_OSC_COMM_STATUS_NODE_S));
                pstServerCtx->u32CommandId --;
                free(pstCommandStatusInfo);
                pstCommandStatusInfo = NULL;
                bExist = HI_TRUE;
            }
        }
    }
    OSC_UNLOCK(pstServerCtx->CommStatusLock);

    if (bExist != HI_TRUE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete the commandstatus fail, for input the command id:%s is err!!\n", pszCommandId);
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Create_CommandStatus(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_SERVER_LEVEL_E enCliVersion, HI_CHAR* pszCommandName, HI_CHAR* pszCommandId, HI_OSC_COMM_STATUS_E enComStatus, HI_DOUBLE dCompletion)
{
    HI_S32 s32Looper = 0;
    HI_BOOL bExist = HI_FALSE;
    List_Head_S* pstPosNode = NULL;
    List_Head_S* pstTmpNode = NULL;
    HI_CHAR aszCommandId[OSC_SVR_COMMAND_ID_LEN] = {0};
    HI_OSC_COMM_STATUS_NODE_S* pstCommandStatusInfo = NULL;

    OSCSVR_CHECK_NULL_ERROR(pszCommandId);
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pszCommandName);

    OSC_LOCK(pstServerCtx->CommStatusLock);

    /*limit the command status storage number, max 3*/
    if (pstServerCtx->u32CommandId > OSC_COMMANDID_STORAGE_MAX_NUM)
    {
        snprintf_s(aszCommandId, OSC_SVR_COMMAND_ID_LEN, OSC_SVR_COMMAND_ID_LEN - 1, "%d", pstServerCtx->u32CommandId - OSC_COMMANDID_STORAGE_MAX_NUM);

        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Over the max ache num, ready to del %s com status!\n", aszCommandId);

        HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &pstServerCtx->pCommStatuslist)
        {
            pstCommandStatusInfo = HI_LIST_ENTRY(pstPosNode, HI_OSC_COMM_STATUS_NODE_S, listPtr);

            if (NULL != pstCommandStatusInfo)
            {
                if (0 == strncmp(pstCommandStatusInfo->aszCommandID, aszCommandId, strlen(aszCommandId) + 1))
                {
                    HI_List_Del(&(pstCommandStatusInfo->listPtr));
                    memset_s((HI_VOID*)pstCommandStatusInfo, sizeof(HI_OSC_COMM_STATUS_NODE_S), 0x00 , sizeof(HI_OSC_COMM_STATUS_NODE_S));
                    free(pstCommandStatusInfo);
                    pstCommandStatusInfo = NULL;
                    bExist = HI_TRUE;
                }
            }
        }

        if (bExist != HI_TRUE)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Fail to delete the commandstatus with the command id:%s!!\n", aszCommandId);
        }
    }

    /*begin to create the command status*/
    pstCommandStatusInfo = (HI_OSC_COMM_STATUS_NODE_S*)malloc(sizeof(HI_OSC_COMM_STATUS_NODE_S));

    if (NULL == pstCommandStatusInfo)
    {
        OSC_UNLOCK(pstServerCtx->CommStatusLock);
        return HI_FAILURE;
    }

    memset_s(pstCommandStatusInfo, sizeof(HI_OSC_COMM_STATUS_NODE_S), 0x00, sizeof(HI_OSC_COMM_STATUS_NODE_S));

    pstCommandStatusInfo->enComStatus = enComStatus;
    strncpy_s(pstCommandStatusInfo->aszCommandID, OSC_SVR_COMMAND_ID_LEN, pszCommandId, strlen(pszCommandId) + 1);
    pstCommandStatusInfo->aszCommandID[strlen(pszCommandId)] = '\0';
    strncpy_s(pstCommandStatusInfo->aszCommandName, OSC_COMM_NAME_MAX_LEN, pszCommandName, strlen(pszCommandName) + 1);
    pstCommandStatusInfo->aszCommandName[strlen(pszCommandName)] = '\0';
    pstCommandStatusInfo->enClientVersion = (HI_OSC_SERVER_LEVEL_E)enCliVersion;

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper++)
    {
        memset_s(pstCommandStatusInfo->aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    pstCommandStatusInfo->dCompletion = dCompletion;
    pstServerCtx->u32CommandId ++;
    HI_List_Add(&(pstCommandStatusInfo->listPtr), &(pstServerCtx->pCommStatuslist));
    OSC_UNLOCK(pstServerCtx->CommStatusLock);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Get_CommandID(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszCommandID)
{
    HI_U32 u32CommandId = 0;

    OSCSVR_CHECK_NULL_ERROR(pszCommandID);
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    OSC_LOCK(pstServerCtx->CommStatusLock);
    u32CommandId = pstServerCtx->u32CommandId;
    snprintf_s(pszCommandID, OSC_SVR_COMMAND_ID_LEN, OSC_SVR_COMMAND_ID_LEN - 1, "%d", u32CommandId);
    OSC_UNLOCK(pstServerCtx->CommStatusLock);

    return HI_SUCCESS;
}

HI_BOOL OSCSVR_Search_CommandCB(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bFound = HI_FALSE;
    List_Head_S* pstPosNode = NULL;
    HI_OSC_REGCOMMAND_NODE_S* pstRegCommandNode = NULL;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    /*check whether takepicture is realized*/
    OSC_LOCK(pstServerCtx->RegCommandlistLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pRegCommandlist)
    {
        pstRegCommandNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_REGCOMMAND_NODE_S, listPtr);

        if (NULL != pstRegCommandNode )
        {
            if (pstRegCommandNode->enOSCRegKey == enOSCCommandKey)
            {
                bFound = HI_TRUE;
                break;
            }
        }
    }

    if (!bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC not find the correspond RegCommand\n");
        OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
        return HI_FALSE;
    }

    if (NULL == pstRegCommandNode->pfnEventCB)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Command callback is not realized by outer!!\n");
        OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
        return HI_FALSE;
    }

    s32Ret = OSCSVR_Get_CommandApi(pstServerCtx, enOSCCommandKey, pstRegCommandNode->pfnEventCB);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "osc get command callback fail!\n");
        OSC_UNLOCK(pstServerCtx->RegCommandlistLock);
        return HI_FALSE;
    }

    OSC_UNLOCK(pstServerCtx->RegCommandlistLock);

    return HI_TRUE;
}

HI_BOOL OSCSVR_CheckClientIP_IsInvaild(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_SOCKET s32Sockfd, HI_OSC_SERVER_LEVEL_E enLevel)
{
    HI_BOOL bFound = HI_FALSE;
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);

    if (HI_SUCCESS != OSCSVR_GetClientIP(s32Sockfd, aszClientIP))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get ip err!!\n");
        return HI_FALSE;
    }

    if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        OSC_LOCK(pstServerCtx->Level2SessLock);

        if (NULL != pstServerCtx->pstLevel2Sess)
        {
            if (0 != strncmp(pstServerCtx->pstLevel2Sess->aszClientIP, aszClientIP, OSC_IP_MAX_LEN))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the level2 ip is invaild!!\n");
                OSC_UNLOCK(pstServerCtx->Level2SessLock);

                return HI_FALSE;
            }

            (HI_VOID)time(&(pstServerCtx->pstLevel2Sess->FirstTick));

            OSC_UNLOCK(pstServerCtx->Level2SessLock);
            return HI_TRUE;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the IP is firstly registered in level2\n");
            /*default to Level2*/
            OSCSVR_Create_ClientLevel2Session(pstServerCtx);
            strncpy_s(pstServerCtx->pstLevel2Sess->aszClientIP, OSC_IP_MAX_LEN, aszClientIP, strlen(aszClientIP) + 1);
            pstServerCtx->pstLevel2Sess->aszClientIP[OSC_IP_MAX_LEN - 1] = '\0';
            OSC_UNLOCK(pstServerCtx->Level2SessLock);

            return HI_TRUE;
        }

    }
    else if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        OSC_LOCK(pstServerCtx->Level1SessLock);

        /*check whether sessionid is invaild*/
        if (NULL != pstServerCtx->pstLevel1Sess)
        {
            if (0 == strncmp(pstServerCtx->pstLevel1Sess->aszClientIP, aszClientIP, OSC_IP_MAX_LEN))
            {
                bFound = HI_TRUE;
                (HI_VOID)time(&(pstServerCtx->pstLevel1Sess->FirstTick));
            }
        }

        if (bFound)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "find the correspond level1 session\n");
            OSC_UNLOCK(pstServerCtx->Level1SessLock);

            return HI_TRUE;
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "not find the correspond level1 session\n");
            OSC_UNLOCK(pstServerCtx->Level1SessLock);

            return HI_FALSE;
        }

        OSC_UNLOCK(pstServerCtx->Level1SessLock);
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the input version is err!!\n");
        return HI_FALSE;
    }

    return HI_TRUE;
}

static HI_S32 OSCSVR_CheckState(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszOldFingerPrint, HI_CHAR* pszNewFingerPrint)
{
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pszOldFingerPrint);
    OSCSVR_CHECK_NULL_ERROR(pszNewFingerPrint);

    OSC_LOCK(pstServerCtx->StateLock);

    if ((0 != strncmp(pstServerCtx->stChangeState.aszStorageUri, pstServerCtx->stPreState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN))
        || (pstServerCtx->stChangeState.bStorageChanged != pstServerCtx->stPreState.bStorageChanged)
        || (pstServerCtx->stChangeState.dBatteryLevel != pstServerCtx->stPreState.dBatteryLevel))
    {
        snprintf_s(pszNewFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, HI_OSC_FINGER_PRINT_MAX_LEN - 1, "FIG_%04d", pstServerCtx->u32FingerId);
        strncpy_s(pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pszNewFingerPrint, strlen(pszNewFingerPrint) + 1);
        pstServerCtx->u32FingerId ++;
    }
    else
    {
        strncpy_s(pszNewFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pszOldFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
        strncpy_s(pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pszNewFingerPrint, strlen(pszNewFingerPrint) + 1);
    }

    pstServerCtx->stPreState.dBatteryLevel = pstServerCtx->stChangeState.dBatteryLevel;
    pstServerCtx->stPreState.bStorageChanged = pstServerCtx->stChangeState.bStorageChanged;
    strncpy_s(pstServerCtx->stPreState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, pstServerCtx->stChangeState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN);
    strncpy_s(pstServerCtx->stPreState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
    OSC_UNLOCK(pstServerCtx->StateLock);

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_CompareState(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSCSERVER_STATE_S* pstOSCSVRState, HI_CHAR* pszFingerPrint)
{
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSVRState);
    OSCSVR_CHECK_NULL_ERROR(pszFingerPrint);

    OSC_LOCK(pstServerCtx->StateLock);

    if ((0 != strncmp(pstServerCtx->stPreState.aszStorageUri, pstOSCSVRState->aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN))
        || (pstServerCtx->stPreState.bStorageChanged != pstOSCSVRState->bStorageChanged)
        || (pstServerCtx->stPreState.dBatteryLevel != pstOSCSVRState->dBatteryLevel))
    {
        snprintf_s(pszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, HI_OSC_FINGER_PRINT_MAX_LEN - 1, "FIG_%04d", pstServerCtx->u32FingerId);
        strncpy_s(pstOSCSVRState->aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pszFingerPrint, strlen(pszFingerPrint) + 1);
        pstServerCtx->u32FingerId ++;
    }
    else
    {
        strncpy_s(pszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
        strncpy_s(pstOSCSVRState->aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
    }

    pstServerCtx->stPreState.dBatteryLevel = pstOSCSVRState->dBatteryLevel;
    pstServerCtx->stPreState.bStorageChanged = pstOSCSVRState->bStorageChanged;
    strncpy_s(pstServerCtx->stPreState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, pstOSCSVRState->aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN);
    strncpy_s(pstServerCtx->stPreState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstOSCSVRState->aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
    /*update the change state*/
    pstServerCtx->stChangeState.dBatteryLevel = pstServerCtx->stPreState.dBatteryLevel;
    pstServerCtx->stChangeState.bStorageChanged = pstServerCtx->stPreState.bStorageChanged;
    strncpy_s(pstServerCtx->stChangeState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN, pstServerCtx->stPreState.aszStorageUri, HI_OSCSVR_STORAGE_URL_MAX_LEN);
    strncpy_s(pstServerCtx->stChangeState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstServerCtx->stPreState.aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN);
    OSC_UNLOCK(pstServerCtx->StateLock);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_GetSessionbyIPaddress(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_CHAR* pszClientIP, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* enLevel)
{
    HI_BOOL bFound = HI_FALSE;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pszClientIP);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(enLevel);

    memset_s(pszSessionId, OSC_SESSIONID_MAX_LEN, 0x00, OSC_SESSIONID_MAX_LEN);

    OSC_LOCK(pstServerCtx->Level1SessLock);

    if (NULL != pstServerCtx->pstLevel1Sess)
    {
        if (0 == strncmp(pstServerCtx->pstLevel1Sess->aszClientIP, pszClientIP, OSC_IP_MAX_LEN))
        {
            if (0 != strlen(pstServerCtx->pstLevel1Sess->aszSessionId))
            {
                bFound = HI_TRUE;
                strncpy_s(pszSessionId, OSC_SESSIONID_MAX_LEN, pstServerCtx->pstLevel1Sess->aszSessionId, strlen(pstServerCtx->pstLevel1Sess->aszSessionId));
                pszSessionId[OSC_SESSIONID_MAX_LEN - 1] = '\0';
                *enLevel = HI_OSC_SERVER_LEVEL_1;
            }
            else
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the level1 client sessionid is 0 with the ip, Err!!\n");
                OSC_UNLOCK(pstServerCtx->Level1SessLock);
                return HI_FAILURE;
            }
        }
    }

    OSC_UNLOCK(pstServerCtx->Level1SessLock);

    if (bFound == HI_FALSE)
    {
        OSC_LOCK(pstServerCtx->Level2SessLock);

        if (NULL != pstServerCtx->pstLevel2Sess)
        {
            if (0 != strlen(pstServerCtx->pstLevel2Sess->aszClientIP))
            {
                if (0 != strncmp(pstServerCtx->pstLevel2Sess->aszClientIP, pszClientIP, OSC_IP_MAX_LEN) )
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client of ip is invaild in level2\n");
                    *enLevel = HI_OSC_SERVER_LEVEL_BUT;
                    OSC_UNLOCK(pstServerCtx->Level2SessLock);
                    return HI_FAILURE;
                }
                else
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client already registered in level2\n");
                    memset_s(pszSessionId, OSC_SESSIONID_MAX_LEN, 0x00, OSC_SESSIONID_MAX_LEN);
                    /*default to Level2*/
                    *enLevel = HI_OSC_SERVER_LEVEL_2;
                }
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the client firstly registered in level2\n");
            memset_s(pszSessionId, OSC_SESSIONID_MAX_LEN, 0x00, OSC_SESSIONID_MAX_LEN);
            /*default to Level2*/
            *enLevel = HI_OSC_SERVER_LEVEL_2;
            OSCSVR_Create_ClientLevel2Session(pstServerCtx);
            strncpy_s(pstServerCtx->pstLevel2Sess->aszClientIP, OSC_IP_MAX_LEN, pszClientIP, strlen(pszClientIP) + 1);
            pstServerCtx->pstLevel2Sess->aszClientIP[OSC_IP_MAX_LEN - 1] = '\0';
        }

        OSC_UNLOCK(pstServerCtx->Level2SessLock);
    }

    return HI_SUCCESS;
}
static HI_S32 OSCSVR_Command_Process(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_OSC_API_COMMAND_E enOSCCommand, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Looper = 0;
    HI_BOOL bFlag = HI_FALSE;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    for (s32Looper = 0; s32Looper < OSC_COMMAND_EXEC_MAX_NUM; s32Looper ++)
    {
        if (s_stCommandExec[s32Looper].enCommandKey == enOSCCommand)
        {
            bFlag = HI_TRUE;
            s32Ret = s_stCommandExec[s32Looper].pfnCommandExec(pOSC, ps32SocketFd, pClientReq, u32ReqLen);

            if (HI_SUCCESS != s32Ret)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR process command fail!!\n");
                return HI_FAILURE;
            }

            break;
        }
    }

    if (bFlag != HI_TRUE)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Not find the correspond command to exec!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Process_Info(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_OSCSERVER_INFO_S* pstOSCSVRInfo = NULL;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INFO;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    if (u32ReqLen > strlen(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Info input request len is wrong!!\n");
        return HI_FAILURE;
    }

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC search /osc/info callback fail!!\n");
        return HI_FAILURE;
    }

    pstOSCSVRInfo = (HI_OSCSERVER_INFO_S*)malloc(sizeof(HI_OSCSERVER_INFO_S));

    if (NULL == pstOSCSVRInfo)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC /osc/info malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstOSCSVRInfo, sizeof(HI_OSCSERVER_INFO_S), 0x00, sizeof(HI_OSCSERVER_INFO_S));

    s32Ret = pstServerCtx->stRegApiCB.pfnOSCSVRInfoCB(pstOSCSVRInfo);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC perform info callback fail!!\n");
        free(pstOSCSVRInfo);
        pstOSCSVRInfo = NULL;

        return HI_FAILURE;
    }

    /*CallBack success!!Start to pack json*/
    s32Ret = OSCSVR_Generate_SVRInfo_JsonPack(pstOSCSVRInfo, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/info pack json fail!!\n");
        free(pstOSCSVRInfo);
        pstOSCSVRInfo = NULL;

        return HI_FAILURE;
    }

    /*Set http state code*/
    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/info generate response fail!!\n");
        free(pstOSCSVRInfo);
        pstOSCSVRInfo = NULL;

        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/info response send fail!!\n");
            free(pstOSCSVRInfo);
            pstOSCSVRInfo = NULL;

            return HI_FAILURE;
        }
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    free(pstOSCSVRInfo);
    pstOSCSVRInfo = NULL;
    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Process_CheckForUpdate(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    static time_t FirstTick;
    static time_t LastTick;
    HI_CHAR aszFingerPrint[HI_OSC_FINGER_PRINT_MAX_LEN] = {0};             /*the value of the input fingerprint*/
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSCSERVER_CHECKFORUPDATES_S* pstCheckForUpdates = NULL;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate get http req body error!!\n");
        return HI_FAILURE;
    }

    pstCheckForUpdates = (HI_OSCSERVER_CHECKFORUPDATES_S*)malloc(sizeof(HI_OSCSERVER_CHECKFORUPDATES_S));

    if (NULL == pstCheckForUpdates)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate malloc error!!\n");
        return HI_FAILURE;
    }

    memset_s(pstCheckForUpdates, sizeof(HI_OSCSERVER_CHECKFORUPDATES_S), 0x00, sizeof(HI_OSCSERVER_CHECKFORUPDATES_S));

    s32Ret = OSCSVR_Parser_CheckForUpdate_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, pstCheckForUpdates);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate parser json pack error!!\n");
        free(pstCheckForUpdates);
        pstCheckForUpdates = NULL;

        return HI_FAILURE;
    }

    /* get the first time!!*/
    (HI_VOID)time(&FirstTick);
    (HI_VOID)time(&LastTick);

    while (pstServerCtx->bExitLooper != HI_TRUE)
    {
        if (HI_SUCCESS != OSCSVR_CheckState(pstServerCtx, pstCheckForUpdates->aszInFingerPrint, aszFingerPrint))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate get fingerprint fail!!\n");
            free(pstCheckForUpdates);
            pstCheckForUpdates = NULL;
            return HI_FAILURE;
        }

        /*get current time*/
        (HI_VOID)time(&LastTick);

        if ((0 != strncmp(aszFingerPrint, pstCheckForUpdates->aszInFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN))
            || (pstCheckForUpdates->s32WaitTimeout == -1)
            || (LastTick - FirstTick >= pstCheckForUpdates->s32WaitTimeout))
        {
            break;
        }

        /*in the waittime, need to decrease the pressure of cpu*/
        sleep(OSC_CHECKFORUPDATE_CHECK_INTERVAL);
    }

    if (0 == strlen(aszFingerPrint))
    {
        /*in case that the pstServerCtx->bExitFlag is false*/
        strncpy_s(aszFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, pstCheckForUpdates->aszInFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN - 1);
    }

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;

    /*CallBack success!!Start to pack json*/
    s32Ret = OSCSVR_Generate_CheckForUpdate_JsonPack(pstServerCtx->u32ThrottleTimeout, OSC_MAX_PROTOCOL_BUFFER, aszFingerPrint, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate pack json fail!!\n");
        free(pstCheckForUpdates);
        pstCheckForUpdates = NULL;
        return HI_FAILURE;
    }

    /*Set http state code*/
    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforUpdate send reponse fail!!\n");
        free(pstCheckForUpdates);
        pstCheckForUpdates = NULL;
        return HI_FAILURE;
    }

    free(pstCheckForUpdates);
    pstCheckForUpdates = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Process_State(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_OSCSERVER_STATE_S* pstOSCSVRState = NULL;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszFingerPrint[HI_OSC_FINGER_PRINT_MAX_LEN] = {0};             /*the value of the special fingerprint*/
    HI_CHAR aszSessionId[OSC_SESSIONID_MAX_LEN] = {0};
    HI_CHAR aszClientIP[OSC_IP_MAX_LEN] = {0};
    HI_OSC_SERVER_LEVEL_E enLevel = HI_OSC_SERVER_LEVEL_BUT;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_STATE;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    if (u32ReqLen > strlen(pClientReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "State input request len is wrong!!\n");
        return HI_FAILURE;
    }

    enOSCCommandKey = HI_OSC_COMMAND_STATE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC search /osc/state callback fail!!\n");
        return HI_FAILURE;
    }

    pstOSCSVRState = (HI_OSCSERVER_STATE_S*)malloc(sizeof(HI_OSCSERVER_STATE_S));

    if (NULL == pstOSCSVRState)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstOSCSVRState, sizeof(HI_OSCSERVER_STATE_S), 0x00, sizeof(HI_OSCSERVER_STATE_S));

    s32Ret = pstServerCtx->stRegApiCB.pfnOSCSVRStateCB(pstOSCSVRState);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC perform /osc/state callback fail!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_CompareState(pstServerCtx, pstOSCSVRState, aszFingerPrint))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state compare state error!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_GetClientIP(*ps32SocketFd, aszClientIP);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state get client ip error!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_GetSessionbyIPaddress(pstServerCtx, aszClientIP, aszSessionId, &enLevel);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state get session err!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    /*CallBack success!!Start to pack json*/
    s32Ret = OSCSVR_Generate_SVRState_JsonPack(pstOSCSVRState, OSC_MAX_PROTOCOL_BUFFER, aszFingerPrint, enLevel, aszSessionId, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state packge json fail!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    /*Set http state code*/
    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state generate response fail!!\n");
        free(pstOSCSVRState);
        pstOSCSVRState = NULL;
        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state response send fail!!\n");
            free(pstOSCSVRState);
            pstOSCSVRState = NULL;
            return HI_FAILURE;
        }
    }

    free(pstOSCSVRState);
    pstOSCSVRState = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Process_Command_Exec(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszCommName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSC_API_COMMAND_E enOSCCommand = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandexec get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_Name_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, aszCommName);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandexec parser Name err!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != HI_OSCSVR_MSGParser_ParseComm(aszCommName, &enOSCCommand))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandexec command name is unsupported!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Command_Process(pOSC, ps32SocketFd, enOSCCommand, pClientReq, u32ReqLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandexec process command fail\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Process_Command_Status(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32JsonLen = 0;
    HI_S32 s32Looper = 0;
    HI_DOUBLE  dCompletion = HI_OSC_COMMAND_STATUS_PROGRESS;
    HI_BOOL bFound = HI_FALSE;
    HI_S32  s32RespLen = 0;
    List_Head_S* pstPosNode = NULL;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszCommandID[OSC_SVR_COMMAND_ID_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSC_COMM_STATUS_NODE_S* pstCommandStatusNode = NULL;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_COMM_STATUS_E enComStatus = HIOSC_COMM_STATUS_INIT;
    HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];
    HI_CHAR aszCommandName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_OSC_SERVER_LEVEL_E enCliVersion = HI_OSC_SERVER_LEVEL_BUT;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_CommandStatus_JsonPack(aszJsonPack, &s32JsonLen, ps32SocketFd, aszCommandID);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus parser json error!!\n");
        return HI_FAILURE;
    }

    OSC_LOCK(pstServerCtx->CommStatusLock);
    HI_List_For_Each(pstPosNode, &pstServerCtx->pCommStatuslist)
    {
        pstCommandStatusNode = HI_LIST_ENTRY(pstPosNode, HI_OSC_COMM_STATUS_NODE_S, listPtr);

        if (0 == strncmp(pstCommandStatusNode->aszCommandID, aszCommandID, OSC_SVR_COMMAND_ID_LEN))
        {
            bFound = HI_TRUE;
            break;
        }
    }

    if (!bFound)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus the command id is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        OSC_UNLOCK(pstServerCtx->CommStatusLock);
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the input id is invaild");

        goto Invaild;
    }

    enComStatus = pstCommandStatusNode->enComStatus;
    dCompletion = pstCommandStatusNode->dCompletion;

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        strncpy_s(aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN, pstCommandStatusNode->aszResults[s32Looper], HI_OSC_FILEURL_MAX_LEN);
        aszResults[s32Looper][strlen(pstCommandStatusNode->aszResults[s32Looper])] = '\0';
    }

    strncpy_s(aszCommandName, OSC_COMM_NAME_MAX_LEN, pstCommandStatusNode->aszCommandName, OSC_COMM_NAME_MAX_LEN);
    aszCommandName[strlen(pstCommandStatusNode->aszCommandName)] = '\0';
    enCliVersion = pstCommandStatusNode->enClientVersion;
    OSC_UNLOCK(pstServerCtx->CommStatusLock);

    /*CallBack success!!Start to pack json*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_CommandStatus_JsonPack(aszCommandName, OSC_MAX_PROTOCOL_BUFFER, aszCommandID, enCliVersion, enComStatus, dCompletion, aszResults, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus  pack json fail!!\n");
        return HI_FAILURE;
    }

    /*Set http state code*/
    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus generate response fail!!\n");
        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus response send fail!!\n");
            return HI_FAILURE;
        }
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commandstatus response is\n%s!!\n", aszReply);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_COMMAND_STATUS_API, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
