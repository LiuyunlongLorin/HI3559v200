/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_file_exec.c
* @brief     oscserver file exec src file
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
static HI_S32 OSCSVR_Send_ProcessPicture_Response(HI_CHAR* pszCommandID,  HI_S32* ps32SocketFd)
{
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszCommandID);

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_ProcessPicture_JsonPack(pszCommandID, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture generate json packs error!!\n");
        return HI_FAILURE;
    }

    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture generate response fail!!\n");
        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture response send fail!!\n");
            return HI_FAILURE;
        }
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camear.processPicture*/
static HI_S32 OSCSVR_ProcessPicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_OSC_PATH_TYPE_E enType, HI_CHAR* pszCommandID, HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN])
{
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_BOOL bInvaild = HI_FALSE;
    HI_BOOL bDisabled = HI_FALSE;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszCommandID);

    s32Ret = pstServerCtx->stRegCommandCB.pfnProcessPitcure(pOSC, enType, aszFileUrl, pszCommandID, &bDisabled, &bInvaild);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture perform callback fail!!\n");
        return HI_FAILURE;
    }

    if (bInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Some of input url is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Any URL in previewFileUrls is invalid");

        goto Invaild;
    }

    if (bDisabled)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC not support delayprocess!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_DISABLED_COMMAND;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Command is currently disabled");

        goto Invaild;
    }

    s32Ret = OSCSVR_Send_ProcessPicture_Response(pszCommandID, ps32SocketFd);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture send the response fail!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_PROCESS_PICTURE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.getImages*/
HI_S32 OSCSVR_Process_GetImage(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32InvaildLen = 0;
    HI_S32  s32HeaderLen = 0;
    HI_S32  s32MaxSize = 0;
    FILE*   ImageFile = NULL;
    HI_S32  s32FileSize = 0;
    HI_S32  s32BinaryLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszRealPath[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszHeader[HI_OSC_HTTP_REPLY_HEADER_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_LEN] = {0};
    HI_CHAR aszBinaryData[OSC_IMAGE_BINARY_DATA_LEN + 1] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage get http req body error!!\n");
        return HI_FAILURE;
    }

    /*judge whether the clinet is invaild*/
    s32Ret = OSCSVR_Parser_GetImage_JsonPack(aszJsonPack, ps32SocketFd, aszFileUrl, &s32MaxSize);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage parser json error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    if (NULL == realpath(aszFileUrl, aszRealPath))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage have no direct access to open the file!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the fileUri value doesn't exist");

        goto Invaild;
    }

    /*open image file!!*/
    ImageFile = fopen(aszRealPath, "rb");

    if (NULL == ImageFile)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage fopen the file fail!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the fileUri value doesn't exist");

        goto Invaild;
    }

    (HI_VOID)fseek(ImageFile, 0, SEEK_END);
    s32FileSize = ftell(ImageFile);
    (HI_VOID)fseek(ImageFile, 0, SEEK_SET);

    s32StatCode = HI_OSC_STATUS_OK;
    OSCSVR_Generate_GetImage_HTTPResponse(s32StatCode, aszHeader, s32FileSize, HI_OSC_HTTP_REPLY_HEADER_LEN);
    /*Send osc response*/
    s32HeaderLen = strlen(aszHeader);

    if (s32HeaderLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszHeader, s32HeaderLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage http header response send fail!!\n");
            fclose(ImageFile);
            ImageFile = NULL;
            return HI_FAILURE;
        }
    }

    while (!feof(ImageFile))
    {
        memset_s(aszBinaryData, OSC_IMAGE_BINARY_DATA_LEN + 1, 0x00, OSC_IMAGE_BINARY_DATA_LEN + 1);
        s32BinaryLen = fread(aszBinaryData, 1,  OSC_IMAGE_BINARY_DATA_LEN , ImageFile);

        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszBinaryData, s32BinaryLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImage binary data send fail!!\n");
            fclose(ImageFile);
            ImageFile = NULL;
            return HI_FAILURE;
        }
    }

    fclose(ImageFile);
    ImageFile = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_IMAGE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.listFiles*/
HI_S32 OSCSVR_Process_ListFiles(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32   s32JsonLen = 0;
    HI_CHAR* pcJsonReply = NULL;      /*use the addr to memory the reply*/
    HI_S32   s32JsonReplyLen = 0;
    HI_S32   s32Size = 0;
    HI_S32   s32InvaildLen = 0;
    HI_S32   s32ListNum = 0;
    HI_S32   s32Ret = HI_SUCCESS;
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_INIT_PATH;
    HI_OSCSERVER_LIST_FILE_S* pstListFiles = NULL;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32ListNum = pstServerCtx->s32ListFiles;
    pstListFiles = (HI_OSCSERVER_LIST_FILE_S*)malloc(sizeof(HI_OSCSERVER_LIST_FILE_S));

    if (NULL == pstListFiles)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles malloc fail !!\n");
        return HI_FAILURE;
    }

    memset_s(pstListFiles, sizeof(HI_OSCSERVER_LIST_FILE_S), 0x00, sizeof(HI_OSCSERVER_LIST_FILE_S));

    enPathType = HI_OSC_ABSOLUTE_PATH;

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles the client IP is Invaild in level2\n");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles get http req body error!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_ListFiles_JsonPack(aszJsonPack, ps32SocketFd, s32ListNum, pstListFiles);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles parser body error!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    /*get the registered command*/
    if (!OSCSVR_Search_CommandCB(pstServerCtx, HI_OSC_COMMAND_LIST_FILE))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles check and get command fail!!\n");
        free(pstListFiles);
        pstListFiles = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnListFiles(enPathType, pstListFiles))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles cb perform fail!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    if (pstListFiles->bInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles input param is invaild!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "The input parameter name is recognized, but its value is invalid");
        free(pstListFiles);
        pstListFiles = NULL;

        goto Invaild;
    }

    /*send response*/
    s32Size = (OSC_LIST_FILE_MIN_LEN + sizeof(HI_OSC_FILE_PROPERTY_S)) * pstListFiles->s32EntryCount + OSC_LIST_FILE_MIN_LEN;

    if (s32Size <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles malloc len is wrong!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    pcJsonReply = (HI_CHAR*)malloc(s32Size);

    if (NULL == pcJsonReply)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles malloc fail!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        return HI_FAILURE;
    }

    memset_s(pcJsonReply, s32Size, 0x00, s32Size);

    s32JsonReplyLen = 0;
    s32Ret = OSCSVR_Generate_ListFile_JsonPack(pstListFiles, s32Size, pcJsonReply, &s32JsonReplyLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles generate json packs error!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        free(pcJsonReply);
        pcJsonReply = NULL;

        return HI_FAILURE;
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles is %s!!\n", pcJsonReply);

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(pcJsonReply, s32JsonReplyLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles send http response error!!\n");
        free(pstListFiles);
        pstListFiles = NULL;
        free(pcJsonReply);
        pcJsonReply = NULL;
        return HI_FAILURE;
    }

    free(pcJsonReply);
    pcJsonReply = NULL;
    free(pstListFiles);
    pstListFiles = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);

    return HI_SUCCESS;
Invaild:
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_FILE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.listImages*/
HI_S32 OSCSVR_Process_ListImages(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32   s32Size = 0;
    HI_S32   s32InvaildLen = 0;
    HI_S32   s32JsonLen = 0;
    HI_S32   s32ListImages = 0;
    HI_CHAR* pcJsonReply = NULL;      /*use the addr to memory the reply*/
    HI_S32   s32JsonReplyLen = 0;
    HI_S32   s32Ret = HI_SUCCESS;
    HI_S32   s32StatCode = HI_OSC_STATUS_OK;
    HI_OSCSERVER_LIST_IMAGE_S* pstListImage = NULL;
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_ABSOLUTE_PATH;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    s32ListImages = pstServerCtx->s32ListFiles;
    pstListImage = (HI_OSCSERVER_LIST_IMAGE_S*)malloc(sizeof(HI_OSCSERVER_LIST_IMAGE_S));

    if (NULL == pstListImage)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstListImage, sizeof(HI_OSCSERVER_LIST_IMAGE_S), 0x00, sizeof(HI_OSCSERVER_LIST_IMAGE_S));

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage get http req body error!!\n");
        free(pstListImage);
        pstListImage = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_ListImage_JsonPack(aszJsonPack, ps32SocketFd, s32ListImages, pstListImage);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage parser body error!!\n");
        free(pstListImage);
        pstListImage = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    enOSCCommandKey = HI_OSC_COMMAND_LIST_IMAGE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage check and get command fail!!\n");
        free(pstListImage);
        pstListImage = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnListImage(enPathType, pstListImage))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage cb perform fail!!\n");
        free(pstListImage);
        pstListImage = NULL;
        return HI_FAILURE;
    }

    if (pstListImage->bInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage input param is invaild!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "The input parameter name is recognized, but its value is invalid");
        free(pstListImage);
        pstListImage = NULL;

        goto Invaild;
    }

    /*send response*/
    s32Size = (OSC_LIST_FILE_MIN_LEN + sizeof(HI_OSC_FILE_PROPERTY_S)) * pstListImage->s32EntryCounts + OSC_LIST_FILE_MIN_LEN;

    if (s32Size <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage malloc len is wrong!!\n");
        free(pstListImage);
        pstListImage = NULL;
        return HI_FAILURE;
    }

    pcJsonReply = (HI_CHAR*)malloc(s32Size);

    if (NULL == pcJsonReply)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage malloc fail!!\n");
        free(pstListImage);
        pstListImage = NULL;
        return HI_FAILURE;
    }

    memset_s(pcJsonReply, s32Size, 0x00, s32Size);
    s32JsonReplyLen = 0;

    s32Ret = OSCSVR_Generate_ListImage_JsonPack(pstListImage, s32Size, pcJsonReply, &s32JsonReplyLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage generate json packs error!!\n");
        free(pstListImage);
        pstListImage = NULL;
        free(pcJsonReply);
        pcJsonReply = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(pcJsonReply, s32JsonReplyLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImage send http response error!!\n");
        free(pstListImage);
        pstListImage = NULL;
        free(pcJsonReply);
        pcJsonReply = NULL;
        return HI_FAILURE;
    }

    free(pcJsonReply);
    pcJsonReply = NULL;
    free(pstListImage);
    pstListImage = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_IMAGE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.getMetaData*/
HI_S32 OSCSVR_Process_GetMetadata(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_OSCSERVER_METADATA_S* pstMetaData = NULL;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_ABSOLUTE_PATH;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    pstMetaData = (HI_OSCSERVER_METADATA_S*)malloc(sizeof(HI_OSCSERVER_METADATA_S));

    if (NULL == pstMetaData)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstMetaData, sizeof(HI_OSCSERVER_METADATA_S), 0x00, sizeof(HI_OSCSERVER_METADATA_S));

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata get http req body error!!\n");
        free(pstMetaData);
        pstMetaData = NULL;
        return  HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_GetMetaData_JsonPack(aszJsonPack, ps32SocketFd, pstMetaData);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata parser body error!!\n");
        free(pstMetaData);
        pstMetaData = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return  HI_SUCCESS;
    }

    /*get the registered command*/
    enOSCCommandKey = HI_OSC_COMMAND_GET_METADATA;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata check and get command fail!!\n");
        free(pstMetaData);
        pstMetaData = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnGetMetaData(enPathType, pstMetaData))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata cb perform fail!!\n");
        free(pstMetaData);
        pstMetaData = NULL;

        return HI_FAILURE;
    }

    if (pstMetaData->bUrlInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata URL is Invaild!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "The input parameter is recognized, but its value is invalid");
        free(pstMetaData);
        pstMetaData = NULL;

        goto Invaild;
    }

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_GetMetaData_JsonPack(pstMetaData, OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata generate json packs error!!\n");
        free(pstMetaData);
        pstMetaData = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata send http response error!!\n");
        free(pstMetaData);
        pstMetaData = NULL;

        return HI_FAILURE;
    }

    free(pstMetaData);
    pstMetaData = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_METADATA, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.delete*/
HI_S32 OSCSVR_Process_Delete(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32Looper = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_BOOL bInvaild = HI_FALSE;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_OSC_PATH_TYPE_E enPathType = HI_OSC_INIT_PATH;
    HI_OSC_SERVER_LEVEL_E enLevel = HI_OSC_SERVER_LEVEL_BUT;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_CHAR aszDeleteUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];
    HI_CHAR aszReultsUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszDeleteUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
        memset_s(aszReultsUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    enPathType = HI_OSC_ABSOLUTE_PATH;
    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete get http req body error!!\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Parser_Delete_JsonPack(aszJsonPack, ps32SocketFd, &enLevel, aszDeleteUrl);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        /*no need to send response again*/
        return HI_SUCCESS;
    }

    /*get the registered command*/
    enOSCCommandKey = HI_OSC_COMMAND_DELETE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete check and get command fail!!\n");
        return HI_FAILURE;
    }

    enPathType = HI_OSC_ABSOLUTE_PATH;

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnDelete(enPathType, aszDeleteUrl, &bInvaild, aszReultsUrl))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete cb perform fail!!\n");
        return HI_FAILURE;
    }

    if (bInvaild)
    {
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        memset_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, 0x00, OSC_INVAILD_MESSAGE_MAX_LEN);
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "any of the URLs is invalid, without deleting!");
        goto Error;
    }

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_Delete_JsonPack(enLevel, OSC_MAX_PROTOCOL_BUFFER, aszReultsUrl, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete generate json packs error!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetadata send http response error!!\n");
        return HI_FAILURE;
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Error:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_DELETE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}

/*camera.switchWifi*/
HI_S32 OSCSVR_Process_SwitchWifi(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_OSCSERVER_SWITCH_WIFI_S* pstSwitchWifi = NULL;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    pstSwitchWifi = (HI_OSCSERVER_SWITCH_WIFI_S*)malloc(sizeof(HI_OSCSERVER_SWITCH_WIFI_S));

    if (NULL == pstSwitchWifi)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi malloc fail!\n");
        return HI_FAILURE;
    }

    memset_s(pstSwitchWifi, sizeof(HI_OSCSERVER_SWITCH_WIFI_S), 0x00, sizeof(HI_OSCSERVER_SWITCH_WIFI_S));

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi the client IP is Invaild in level2\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi get http req body error!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    /*judge whether the clinet is invaild*/
    s32Ret = OSCSVR_Parser_SwitchWifi_JsonPack(aszJsonPack, ps32SocketFd, pstSwitchWifi);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi parser json error!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    enOSCCommandKey = HI_OSC_COMMAND_SWITCH_WIFI;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi check and get command fail!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnSwitchWifi(pstSwitchWifi))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "perform SwitchWifi callback error!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    if (pstSwitchWifi->bInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi input is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Input parameter names are recognized, but one or more passed value are invalid");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        goto Invaild;
    }

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_SwitchWifi_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi generate json packs error!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi send http response error!!\n");
        free(pstSwitchWifi);
        pstSwitchWifi = NULL;

        return HI_FAILURE;
    }

    free(pstSwitchWifi);
    pstSwitchWifi = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_SWITCH_WIFI, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.uploadFile*/
HI_S32 OSCSVR_Process_UploadFile(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32InvaildLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_OSCSERVER_UPLOAD_FILE_S* pstUploadFile = NULL;
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

    pstUploadFile = (HI_OSCSERVER_UPLOAD_FILE_S*)malloc(sizeof(HI_OSCSERVER_UPLOAD_FILE_S));

    if (NULL == pstUploadFile)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pstUploadFile, sizeof(HI_OSCSERVER_UPLOAD_FILE_S), 0x00, sizeof(HI_OSCSERVER_UPLOAD_FILE_S));

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile the client IP is Invaild in level2!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile get http req body error!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        return HI_FAILURE;
    }

    /*judge whether the clinet is invaild*/
    s32Ret = OSCSVR_Parser_UploadFile_JsonPack(aszJsonPack, ps32SocketFd, pstUploadFile);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile parser body error!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    enOSCCommandKey = HI_OSC_COMMAND_UPLOAD_FILE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile check and get command fail!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != pstServerCtx->stRegCommandCB.pfnUploadFile(pstUploadFile))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "perform UploadFile callback error!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        return HI_FAILURE;
    }

    if (pstUploadFile->bInvaild)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile input is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_FORBIDDEN;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Input parameter names are recognized, but one or more passed value are invalid");
        free(pstUploadFile);
        pstUploadFile = NULL;

        goto Invaild;
    }

    /*send response*/
    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_UploafFile_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile generate json packs error!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Send_HTTPResponse(aszJsonPack, s32JsonLen, ps32SocketFd))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile send http response error!!\n");
        free(pstUploadFile);
        pstUploadFile = NULL;

        return HI_FAILURE;
    }

    free(pstUploadFile);
    pstUploadFile = NULL;
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_UPLOAD_FILE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszInvaildReply, OSC_MAX_PROTOCOL_BUFFER);
    s32InvaildLen = strlen(aszInvaildReply);

    if (s32InvaildLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszInvaildReply, s32InvaildLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_SUCCESS;
}
/*camera.processPicture*/
HI_S32 OSCSVR_Process_ProcessPicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen)
{
    HI_S32  s32Looper = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR aszComName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszCommandID[OSC_SVR_COMMAND_ID_LEN] = {0};
    HI_OSC_PATH_TYPE_E enType = HI_OSC_INIT_PATH;
    HI_OSC_API_COMMAND_E enOSCCommandKey = HI_OSC_COMMAND_INVAILD;
    HI_CHAR aszPreviewFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN];

    OSCSVR_CHECK_NULL_ERROR(pOSC);
    HI_OSC_SERVER_CTX_S* pstServerCtx = (HI_OSC_SERVER_CTX_S*)pOSC;

    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        memset_s(aszPreviewFileUrl[s32Looper], HI_OSC_FILEURL_MAX_LEN, 0x00, HI_OSC_FILEURL_MAX_LEN);
    }

    if (HI_TRUE != OSCSVR_CheckClientIP_IsInvaild(pstServerCtx, *ps32SocketFd, HI_OSC_SERVER_LEVEL_2))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture the client IP is Invaild in level2\n");
        return HI_FAILURE;
    }

    s32Ret = OSCSVR_Get_HTTP_Body(pClientReq, u32ReqLen, aszJsonPack, &s32JsonLen);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture get http req body error!!\n");
        return HI_FAILURE;
    }

    /*judge whether the client is invaild*/
    s32Ret = OSCSVR_Parser_ProcessPicture_JsonPack(aszJsonPack, ps32SocketFd, aszComName, aszPreviewFileUrl);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture parser body error!!\n");
        HI_OSCSVR_CloseSocket(ps32SocketFd);
        return HI_SUCCESS;
    }

    s32Ret = OSCSVR_Get_CommandID(pstServerCtx, aszCommandID);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture get command id fail!!\n");
        return HI_FAILURE;
    }

    /*create command status*/
    s32Ret = OSCSVR_Create_CommandStatus(pstServerCtx, HI_OSC_SERVER_LEVEL_2, aszComName, aszCommandID, HIOSC_COMM_STATUS_INPROGRESS, 0.00f);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture create command status fail\n");
        return HI_FAILURE;
    }

    enType = HI_OSC_ABSOLUTE_PATH;
    enOSCCommandKey = HI_OSC_COMMAND_PROCESS_PICTURE;

    if (!OSCSVR_Search_CommandCB(pstServerCtx, enOSCCommandKey))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture search command Callback fail!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_ProcessPicture(pOSC, ps32SocketFd, enType, aszCommandID, aszPreviewFileUrl))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture record status fail!!\n");
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
