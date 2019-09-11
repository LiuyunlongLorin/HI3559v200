/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_jsonparser.cpp
* @brief     oscserver json parser src file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
/*osc module header file*/
#include "hi_osc_log.h"
#include "hi_osc_server.h"
#include "hi_oscserver_http.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_jsonparser.h"
#include "hi_oscserver_jsonpackage.h"
#include "hi_oscserver_com_exec.h"
#include "hi_oscserver_file_exec.h"
/*rapid json header file*/
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "reader.h"
#include "document.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

using namespace rapidjson;
using namespace std;

HI_S32 OSCSVR_Parser_Name_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName)
{
    Document document;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszInvaildName[OSC_COMM_NAME_MAX_LEN] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_OSC_API_COMMAND_E enCommand = HI_OSC_COMMAND_INVAILD;

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/exec ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/exec has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/exec has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter stateFingerprint is missing");
        snprintf_s(aszInvaildName, OSC_COMM_NAME_MAX_LEN, OSC_COMM_NAME_MAX_LEN - 1, OSC_NAME_START_SESSION);

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/exec json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");
        snprintf_s(aszInvaildName, OSC_COMM_NAME_MAX_LEN, OSC_COMM_NAME_MAX_LEN - 1, OSC_NAME_START_SESSION);

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (HI_SUCCESS != HI_OSCSVR_MSGParser_ParseComm(pszComName, &enCommand))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/exec parser comm name error!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_NAME;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "The input parameter name is unrecognized");
        snprintf_s(aszInvaildName, OSC_COMM_NAME_MAX_LEN, OSC_COMM_NAME_MAX_LEN - 1, OSC_NAME_START_SESSION);
        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, aszInvaildName, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}
/*
{
"stateFingerprint": "12EGA33",
"waitTimeout": 300
}
*/
HI_S32 OSCSVR_Parser_CheckForUpdate_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_OSCSERVER_CHECKFORUPDATES_S* pstOSCCheckForUpdate)
{
    Document document;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pstOSCCheckForUpdate);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates ParseInsitu error!%s\n", pszJsonPack);
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("stateFingerprint") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates has no member fingerprint!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter stateFingerprint is missing");

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["stateFingerprint"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

        goto Invaild;
    }

    if (strlen(document["stateFingerprint"].GetString()) > HI_OSC_FINGER_PRINT_MAX_LEN)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates json parse member stateFingerprint len is err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value len is invaild");

        goto Invaild;
    }

    strncpy_s(pstOSCCheckForUpdate->aszInFingerPrint, HI_OSC_FINGER_PRINT_MAX_LEN, document["stateFingerprint"].GetString(), strlen(document["stateFingerprint"].GetString()));
    pstOSCCheckForUpdate->aszInFingerPrint[strlen(document["stateFingerprint"].GetString())] = '\0';

    if (!document.HasMember("waitTimeout"))
    {
        /*body has no member*/
        pstOSCCheckForUpdate->s32WaitTimeout = -1;
    }
    else
    {
        if (!document["waitTimeout"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "waitTimeout value is invaild");
            goto Invaild;
        }

        pstOSCCheckForUpdate->s32WaitTimeout = document["waitTimeout"].GetInt();

        if (pstOSCCheckForUpdate->s32WaitTimeout > OSC_STATE_WAITOUT_MAX_TIME)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates json member unsuitable err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_CHECK_FORUPDATES_API, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_CommandStatus_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComId)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszComId);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status ParseInsitu error!%s\n", pszJsonPack);
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("id") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status has no member id!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter id is missing");

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["id"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value format is invaild");

        goto Invaild;
    }

    strncpy_s(pszComId, OSC_SVR_COMMAND_ID_LEN, document["id"].GetString(), strlen(document["id"].GetString()));
    pszComId[strlen(document["id"].GetString())] = '\0';
    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_START_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_StartSession_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_S32* ps32SessTimeOut)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(ps32SessTimeOut);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("timeout"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession timeout is ignored!\n");
            *ps32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;

            return HI_SUCCESS;
        }

        if (!parameters["timeout"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        *ps32SessTimeOut = parameters["timeout"].GetInt();

        if (*ps32SessTimeOut < 0)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession timeout wrong, set to default!!\n");
            *ps32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;
        }
    }
    else
    {
        *ps32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_START_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}


HI_S32 OSCSVR_Parser_CloseSession_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_CHAR* ps32SessId)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(ps32SessId);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "the name is missing");

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "Parameter name type is incorrect");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("sessionId"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession json parse para is missing!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "sessionId is missing");

            goto Invaild;
        }

        if (!parameters["sessionId"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter sessionId value is invaild");

            goto Invaild;
        }

        strncpy_s(ps32SessId, OSC_SESSIONID_MAX_LEN, parameters["sessionId"].GetString(), strlen(parameters["sessionId"].GetString()));
        ps32SessId[strlen(parameters["sessionId"].GetString())] = '\0';
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession json parse para is missing!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "sessionId is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_CLOSE_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}
HI_S32 OSCSVR_Parser_UpdateSession_JsonPack(HI_CHAR* pszJsonPack,  HI_CHAR* pszComName, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_S32* ps32SessTimeOut)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(ps32SessTimeOut);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("sessionId"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession has no member sessionId!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

            goto Invaild;
        }

        if (!parameters["sessionId"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        strncpy_s(pszSessionId, OSC_SESSIONID_MAX_LEN, parameters["sessionId"].GetString(), strlen(parameters["sessionId"].GetString()));
        pszSessionId[strlen(parameters["sessionId"].GetString())] = '\0';

        if (!parameters.HasMember("timeout"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession timeout is ignored!\n");
            *ps32SessTimeOut = OSC_IGNORE_SESS_TIMEOUT;

            return HI_SUCCESS;
        }

        if (!parameters["timeout"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        *ps32SessTimeOut = parameters["timeout"].GetInt();

        if (*ps32SessTimeOut < 0)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession has no member param!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");
        goto Invaild;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_UPDATE_SESSION, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}
HI_S32 OSCSVR_Parser_TakePicture_JsonPack(HI_CHAR* pszJsonPack,  HI_CHAR* pszComName, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* enLevel)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(enLevel);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter name is missing");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter object is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("sessionId"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture has no member sessionId!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter sessionId is missing");

            goto Invaild;
        }

        if (!parameters["sessionId"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "TakePicture json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter sessionId value is invaild");

            goto Invaild;
        }

        strncpy_s(pszSessionId, OSC_SESSIONID_MAX_LEN, parameters["sessionId"].GetString(), strlen(parameters["sessionId"].GetString()));
        pszSessionId[strlen(parameters["sessionId"].GetString())] = '\0';
        *enLevel = HI_OSC_SERVER_LEVEL_1;
    }
    else
    {
        *enLevel = HI_OSC_SERVER_LEVEL_2;
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
    return HI_FAILURE;
}

static HI_S32 OSCSVR_Parser_Info_Space(const Value& InfoOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions, HI_BOOL* pbFlag)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. totalSpace*/
    if (InfoOptions.HasMember("totalSpace"))
    {
        if (!InfoOptions["totalSpace"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options totalSpace format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.s32TotalSpace = InfoOptions["totalSpace"].GetInt();
        *pbFlag = HI_TRUE;
        pstOptionsTab->stInfoTable.bTotalSpace = HI_TRUE;
    }

    /*2. remainingSpace*/
    if (InfoOptions.HasMember("remainingSpace"))
    {
        if (!InfoOptions["remainingSpace"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options remainingSpace format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.s32RemainingSpace = InfoOptions["remainingSpace"].GetInt();
        *pbFlag = HI_TRUE;
        pstOptionsTab->stInfoTable.bRemainingSpace = HI_TRUE;
    }

    /*3. remainingPictures*/
    if (InfoOptions.HasMember("remainingPictures"))
    {
        if (!InfoOptions["remainingPictures"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options remainingPictures format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.s32RemainingPictures = InfoOptions["remainingPictures"].GetInt();
        *pbFlag = HI_TRUE;
        pstOptionsTab->stInfoTable.bRemainingPictures = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_Info_GpsAndGyo(const Value& InfoOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions, HI_BOOL* pbFlag)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. gyro*/
    if (InfoOptions.HasMember("gyro"))
    {
        if (!InfoOptions["gyro"].IsBool())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options gyro format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.bGyro = (HI_BOOL)InfoOptions["gyro"].GetBool();
        *pbFlag = HI_TRUE;
        pstOptionsTab->stInfoTable.bGyro = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
    }

    /*2. gps*/
    if (InfoOptions.HasMember("gps"))
    {
        if (!InfoOptions["gps"].IsBool())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options gps format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.bGps = (HI_BOOL)InfoOptions["gps"].GetBool();
        *pbFlag = HI_TRUE;
        pstOptionsTab->stInfoTable.bGps = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
    }

    /*3. gpsInfo*/
    if (InfoOptions.HasMember("gpsInfo"))
    {
        const Value& gpsInfo = InfoOptions["gpsInfo"];

        if ( !gpsInfo.IsObject() )
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member gpsInfo err!!!\n");
            return HI_FAILURE;
        }

        /*2. gpsInfo object.lat*/
        if (gpsInfo.HasMember("lat"))
        {
            if (!gpsInfo["lat"].IsDouble())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member lat err!!!\n");
                return HI_FAILURE;
            }

            pstOSCSetOptions->stInfo.stGPSInfo.dLat = gpsInfo["lat"].GetDouble();
        }

        /*3. gpsInfo object.lng*/
        if (gpsInfo.HasMember("lng"))
        {
            if (!gpsInfo["lng"].IsDouble())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member lng err!!!\n");
                return HI_FAILURE;
            }

            pstOSCSetOptions->stInfo.stGPSInfo.dLng = gpsInfo["lng"].GetDouble();
        }

        pstOptionsTab->stInfoTable.bGPSInfo = HI_TRUE;
        *pbFlag = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
    }

    return HI_SUCCESS;
}

/*Parser info options*/
static HI_S32 OSCSVR_Parser_InfoOptions_Value(const Value& InfoOptions, HI_BOOL* pbExist, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. sleepDelay*/
    if (InfoOptions.HasMember("sleepDelay"))
    {
        pstOSCSetOptions->stInfo.s32SleepDelay = InfoOptions["sleepDelay"].GetInt();
        pstOptionsTab->stInfoTable.bSleepDelay = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*2. offDelay*/
    if (InfoOptions.HasMember("offDelay"))
    {
        pstOSCSetOptions->stInfo.s32OFFDelay = InfoOptions["offDelay"].GetInt();
        pstOptionsTab->stInfoTable.bOFFDelay = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*3. dateTimeZone*/
    if (InfoOptions.HasMember("dateTimeZone"))
    {
        if (strlen(InfoOptions["dateTimeZone"].GetString()) > HI_OSC_OPTION_DATE_TIME_ZONE)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member dateTimeZone len is err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->stInfo.aszDateTimeZone, HI_OSC_OPTION_DATE_TIME_ZONE, InfoOptions["dateTimeZone"].GetString(), strlen(InfoOptions["dateTimeZone"].GetString()));
        pstOSCSetOptions->stInfo.aszDateTimeZone[strlen(InfoOptions["dateTimeZone"].GetString())] = '\0';
        pstOptionsTab->stInfoTable.bDateTimeZone = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*4. GPS and Gyro*/
    if (HI_SUCCESS != OSCSVR_Parser_Info_GpsAndGyo(InfoOptions, pstOptionsTab, pstOSCSetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member gps err!!!\n");
        return HI_FAILURE;
    }

    /*4. space*/
    if (HI_SUCCESS != OSCSVR_Parser_Info_Space(InfoOptions, pstOptionsTab, pstOSCSetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions json parse member space err!!!\n");
        return HI_FAILURE;
    }

    /*5. delayProcessing*/
    if (InfoOptions.HasMember("delayProcessing"))
    {
        if (!InfoOptions["delayProcessing"].IsBool())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member delayProcessing err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.bDelayProcessing = (HI_BOOL)InfoOptions["delayProcessing"].GetBool();
        pstOptionsTab->stInfoTable.bDelayProcessing = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*6. pollingDelay*/
    if (InfoOptions.HasMember("pollingDelay"))
    {
        if (!InfoOptions["pollingDelay"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member pollingDelay err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.s32PollingDelay = InfoOptions["pollingDelay"].GetInt();
        pstOptionsTab->stInfoTable.bPollingDelay = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*7. clientVersion*/
    if (InfoOptions.HasMember("clientVersion"))
    {
        if (!InfoOptions["clientVersion"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member clientVersion err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stInfo.s32ClientVersion = InfoOptions["clientVersion"].GetInt();
        pstOptionsTab->stInfoTable.bClientVersion = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*8. wifiPassword*/
    if (InfoOptions.HasMember("wifiPassword"))
    {
        if (!InfoOptions["wifiPassword"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member wifiPassword err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->stInfo.aszWifiPassword, HI_OSC_OPTION_WIFI_PASSWORD, InfoOptions["wifiPassword"].GetString(), strlen(InfoOptions["wifiPassword"].GetString()));
        pstOSCSetOptions->stInfo.aszWifiPassword[strlen(InfoOptions["wifiPassword"].GetString())] = '\0';
        pstOptionsTab->stInfoTable.bWifiPassword = HI_TRUE;
        pstOSCSetOptions->bInfo = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_PreviewFormat_Object(const Value& PreviewFormat, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    if (!PreviewFormat.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options previewFormat format err!!!\n");
        return HI_FAILURE;
    }

    /*1. previewFormat object.width*/
    if (PreviewFormat.HasMember("width"))
    {
        if (!PreviewFormat["width"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options width format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stPreviewFormat.u32Width = PreviewFormat["width"].GetInt();
    }

    /*2. previewFormat object.height*/
    if (PreviewFormat.HasMember("height"))
    {
        if (!PreviewFormat["height"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse format options height format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stPreviewFormat.u32Height = PreviewFormat["height"].GetInt();
    }

    /*3. previewFormat object.framerate*/
    if (PreviewFormat.HasMember("framerate"))
    {
        if (!PreviewFormat["framerate"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "setoptions json parse member framerate err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stPreviewFormat.u32FrameRate = PreviewFormat["framerate"].GetInt();
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_Format_Object(const Value& Format, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    if (!Format.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member Format object err!!!\n");
        return HI_FAILURE;
    }

    /*1. fileFormat object.type*/
    if (Format.HasMember("type"))
    {
        if (!Format["type"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member type err!!!\n");
            return HI_FAILURE;
        }

        if (strlen(Format["type"].GetString()) > HI_OSC_OPTION_FORMAT_TYPE_NAME)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member type len is err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->stForMat.stFileFormat.aszType, HI_OSC_OPTION_FORMAT_TYPE_NAME, Format["type"].GetString(), strlen(Format["type"].GetString()));
        pstOSCSetOptions->stForMat.stFileFormat.aszType[strlen(Format["type"].GetString())] = '\0';
    }

    /*2. fileFormat object.width*/
    if (Format.HasMember("width"))
    {
        if (!Format["width"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member width err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stFileFormat.s32Width = Format["width"].GetInt();
    }

    /*3. fileFormat object.height*/
    if (Format.HasMember("height"))
    {
        if (!Format["height"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member height err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stFileFormat.s32Height = Format["height"].GetInt();
    }

    /*4. fileFormat object.framerate, only in mode is video*/
    if (Format.HasMember("framerate"))
    {
        if (!Format["framerate"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member framerate err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->stForMat.stFileFormat.u32FrameRate = Format["framerate"].GetInt();
    }

    return HI_SUCCESS;
}

/*Parser format options*/
static HI_S32 OSCSVR_Parser_ForMatOptions_Value(const Value& ForMatOptions, HI_BOOL* pbExist, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. previewFormat options value*/
    if (ForMatOptions.HasMember("previewFormat"))
    {
        const Value& previewFormat = ForMatOptions["previewFormat"];

        if (HI_SUCCESS != OSCSVR_Parser_PreviewFormat_Object(previewFormat, pstOSCSetOptions))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member previewFormat err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->bForMat = HI_TRUE;
        pstOptionsTab->stFormatTable.bPreviewFormat = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    /*2. fileFormat options value*/
    if (ForMatOptions.HasMember("fileFormat"))
    {
        const Value& fileFormat = ForMatOptions["fileFormat"];

        if (HI_SUCCESS != OSCSVR_Parser_Format_Object(fileFormat, pstOSCSetOptions))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member fileFormat err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->bForMat = HI_TRUE;
        pstOptionsTab->stFormatTable.bFileFormat = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    return HI_SUCCESS;
}

/*Parser video options*/
static HI_S32 OSCSVR_Parser_IntervalOptions_Value(const Value& IntervalOptions, HI_BOOL* pbExist, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. remainingVideoSeconds options value*/
    if (IntervalOptions.HasMember("remainingVideoSeconds"))
    {
        if (!IntervalOptions["remainingVideoSeconds"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse video options remainingVideoSeconds format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stVideo.s32RemainingVideoSeconds = IntervalOptions["remainingVideoSeconds"].GetInt();
        pstOptionsTab->stVideoTable.bRemainingVideoSeconds = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INTERVAL;
        *pbExist = HI_TRUE;
    }

    /*2. videoStitching options value*/
    if (IntervalOptions.HasMember("videoStitching"))
    {
        if (!IntervalOptions["videoStitching"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse video options videoStitching format err!!!\n");
            return HI_FAILURE;
        }

        if (strlen(IntervalOptions["videoStitching"].GetString()) > HI_OSC_OPTION_VIDEO_STITCHING)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse video options videoStitching len err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->unModeStuct.stVideo.aszVideoStitching, HI_OSC_OPTION_VIDEO_STITCHING, IntervalOptions["videoStitching"].GetString(), strlen(IntervalOptions["videoStitching"].GetString()));
        pstOSCSetOptions->unModeStuct.stVideo.aszVideoStitching[strlen(IntervalOptions["videoStitching"].GetString())] = '\0';
        pstOptionsTab->stVideoTable.bVideoStitching = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INTERVAL;
        *pbExist = HI_TRUE;
    }

    /*3. videoGPS options value*/
    if (IntervalOptions.HasMember("videoGPS"))
    {
        if (!IntervalOptions["videoGPS"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse video options videoGPS format err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->unModeStuct.stVideo.aszVideoGPS, HI_OSC_OPTION_VIDEO_GPS, IntervalOptions["videoGPS"].GetString(), strlen(IntervalOptions["videoGPS"].GetString()));
        pstOSCSetOptions->unModeStuct.stVideo.aszVideoGPS[strlen(IntervalOptions["videoGPS"].GetString())] = '\0';
        pstOptionsTab->stVideoTable.bVideoGPS = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INTERVAL;
        *pbExist = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_ExposreSet(const Value& ImageOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions, HI_BOOL* pbFlag)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. exposureProgram options value*/
    if (ImageOptions.HasMember("exposureProgram"))
    {
        if (!ImageOptions["exposureProgram"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options exposureProgram err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stImage.s32ExposureProgram = ImageOptions["exposureProgram"].GetInt();
        pstOptionsTab->stImageTable.bExposureProgram = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbFlag = HI_TRUE;
    }

    /*2. exposureCompensation options value*/
    if (ImageOptions.HasMember("exposureCompensation"))
    {
        if (!ImageOptions["exposureCompensation"].IsDouble())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options exposureCompensation err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stImage.dExposureCompensation = ImageOptions["exposureCompensation"].GetDouble();
        pstOptionsTab->stImageTable.bExposureCompensation = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbFlag = HI_TRUE;
    }

    /*3. exposureDelay options value*/
    if (ImageOptions.HasMember("exposureDelay"))
    {
        if (!ImageOptions["exposureDelay"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options exposureDelay err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stImage.s32ExposureDelay = ImageOptions["exposureDelay"].GetInt();
        pstOptionsTab->stImageTable.bExposureDelay = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbFlag = HI_TRUE;
    }

    /*4. exposureBracket options value*/
    if (ImageOptions.HasMember("exposureBracket"))
    {
        const Value& exposureBracket = ImageOptions["exposureBracket"];

        if (!exposureBracket.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member fileFormat err!!!\n");
            return HI_FAILURE;
        }

        /*2. exposureBracket object.shots*/
        if (exposureBracket.HasMember("shots"))
        {
            pstOSCSetOptions->unModeStuct.stImage.stExposureBracket.s32Shots = exposureBracket["shots"].GetInt();
        }

        /*3. exposureBracket object.increment*/
        if (exposureBracket.HasMember("increment"))
        {
            pstOSCSetOptions->unModeStuct.stImage.stExposureBracket.dIncrement = exposureBracket["increment"].GetDouble();
        }

        /*4. exposureBracket object.autoMode*/
        if (exposureBracket.HasMember("autoMode"))
        {
            if (!exposureBracket["autoMode"].IsBool())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member shots err!!!\n");
                return HI_FAILURE;
            }

            pstOSCSetOptions->unModeStuct.stImage.stExposureBracket.bAutoMode = (HI_BOOL)exposureBracket["autoMode"].GetBool();
        }

        pstOptionsTab->stImageTable.bExposureBracket = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbFlag = HI_TRUE;
    }

    return HI_SUCCESS;
}

/*Parser image options*/
static HI_S32 OSCSVR_Parser_ImageOptions_Value(const Value& ImageOptions, HI_OSC_SERVER_LEVEL_E enLevel, HI_BOOL* pbExist, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. iso options value*/
    if (ImageOptions.HasMember("iso"))
    {
        pstOSCSetOptions->unModeStuct.stImage.s32ISO = ImageOptions["iso"].GetInt();
        pstOptionsTab->stImageTable.bISO = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*2. shutterSpeed options value*/
    if (ImageOptions.HasMember("shutterSpeed"))
    {
        pstOSCSetOptions->unModeStuct.stImage.dShutterSpeed = ImageOptions["shutterSpeed"].GetDouble();
        pstOptionsTab->stImageTable.bShutterSpeed = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*3. aperture options value*/
    if (ImageOptions.HasMember("aperture"))
    {
        pstOSCSetOptions->unModeStuct.stImage.dAperture = ImageOptions["aperture"].GetDouble();
        pstOptionsTab->stImageTable.bAperture = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*4. whiteBalance options value*/
    if (ImageOptions.HasMember("whiteBalance"))
    {
        if (strlen(ImageOptions["whiteBalance"].GetString()) > HI_OSC_OPTION_WHITE_BALANCE)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse whiteBalance options hdr len err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->unModeStuct.stImage.aszWhiteBalance, HI_OSC_OPTION_WHITE_BALANCE, ImageOptions["whiteBalance"].GetString(), strlen(ImageOptions["whiteBalance"].GetString()));
        pstOSCSetOptions->unModeStuct.stImage.aszWhiteBalance[strlen(ImageOptions["whiteBalance"].GetString())] = '\0';
        pstOptionsTab->stImageTable.bWhiteBalance = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*5. hdr*/
    if (ImageOptions.HasMember("hdr"))
    {
        if (HI_OSC_SERVER_LEVEL_1 == enLevel)
        {
            pstOSCSetOptions->unModeStuct.stImage.bLevel1hdr = (HI_BOOL)ImageOptions["hdr"].GetBool();
            pstOptionsTab->stImageTable.bLevel1hdr = HI_TRUE;
        }
        else if (HI_OSC_SERVER_LEVEL_2 == enLevel)
        {
            if (strlen(ImageOptions["hdr"].GetString()) > HI_OSC_OPTION_HDR)
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options hdr len err!!!\n");
                return HI_FAILURE;
            }

            strncpy_s(pstOSCSetOptions->unModeStuct.stImage.aszLevel2Hdr, HI_OSC_OPTION_HDR, ImageOptions["hdr"].GetString(), strlen(ImageOptions["hdr"].GetString()));
            pstOSCSetOptions->unModeStuct.stImage.aszLevel2Hdr[strlen(ImageOptions["hdr"].GetString())] = '\0';
            pstOptionsTab->stImageTable.bLevel2Hdr = HI_TRUE;
        }

        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*6. imageStabilization options value*/
    if (ImageOptions.HasMember("imageStabilization"))
    {
        if (strlen(ImageOptions["imageStabilization"].GetString()) > HI_OSC_OPTION_IMAGE_STABILIZATION)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options imageStabilization len err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->unModeStuct.stImage.aszImageStabilization, HI_OSC_OPTION_IMAGE_STABILIZATION, ImageOptions["imageStabilization"].GetString(), strlen(ImageOptions["imageStabilization"].GetString()));
        pstOSCSetOptions->unModeStuct.stImage.aszImageStabilization[strlen(ImageOptions["imageStabilization"].GetString())] = '\0';
        pstOptionsTab->stImageTable.bImageStabilization = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*7. photoStitching options value*/
    if (ImageOptions.HasMember("photoStitching"))
    {
        if (strlen(ImageOptions["photoStitching"].GetString()) > HI_OSC_OPTION_PHOTO_STITCHING)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options photoStitching len err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->unModeStuct.stImage.aszPhotoStitching, HI_OSC_OPTION_PHOTO_STITCHING, ImageOptions["photoStitching"].GetString(), strlen(ImageOptions["photoStitching"].GetString()));
        pstOSCSetOptions->unModeStuct.stImage.aszPhotoStitching[strlen(ImageOptions["photoStitching"].GetString())] = '\0';
        pstOptionsTab->stImageTable.bPhotoStitching = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    if (HI_SUCCESS !=  OSCSVR_Parser_ExposreSet(ImageOptions, pstOptionsTab, pstOSCSetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions parse image options expourse set err!!!\n");
        return HI_FAILURE;
    }

    /*4. captureInterval options value*/
    if (ImageOptions.HasMember("captureInterval"))
    {
        if (!ImageOptions["captureInterval"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options captureInterval format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stImage.s32CaptureInterval = ImageOptions["captureInterval"].GetInt();
        pstOptionsTab->stImageTable.bCaptureInterval = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    /*5. captureNumber options value*/
    if (ImageOptions.HasMember("captureNumber"))
    {
        if (!ImageOptions["captureNumber"].IsInt())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions parse image options captureNumber format err!!!\n");
            return HI_FAILURE;
        }

        pstOSCSetOptions->unModeStuct.stImage.u32CaptureNumber = ImageOptions["captureNumber"].GetInt();
        pstOptionsTab->stImageTable.bCaptureNumber = HI_TRUE;
        pstOSCSetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
        *pbExist = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_SetOptions_Value(const Value& SetOptions, HI_OSC_SERVER_LEVEL_E enlevel, HI_S32* ps32SocketFd, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    HI_BOOL bExist = HI_FALSE;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_CHAR aszInvaildName[OSC_COMM_NAME_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);

    /*1. captureMode options value*/
    if (SetOptions.HasMember("captureMode"))
    {
        if (!SetOptions["captureMode"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "setoptions json parse member captureMode err!!!\n");
            return HI_FAILURE;
        }

        if (strlen(SetOptions["captureMode"].GetString()) > HI_OSC_OPTION_CAPTURE_MODE_LEN)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "setoptions json parse member captureMode len err!!!\n");
            return HI_FAILURE;
        }

        strncpy_s(pstOSCSetOptions->aszCaptureMode, HI_OSC_OPTION_CAPTURE_MODE_LEN, SetOptions["captureMode"].GetString(), strlen(SetOptions["captureMode"].GetString()));
        pstOSCSetOptions->aszCaptureMode[strlen(SetOptions["captureMode"].GetString())] = '\0';
        pstOptionsTab->bCaptureMode = HI_TRUE;
        bExist = HI_TRUE;
    }

    /*2. Image options value*/
    if (HI_SUCCESS != OSCSVR_Parser_ImageOptions_Value(SetOptions, enlevel, &bExist, pstOptionsTab, pstOSCSetOptions))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Image Options parser err!!\n");
        return HI_FAILURE;
    }

    if (pstOSCSetOptions->enOptionMode == HI_OSC_OPTIONS_MODE_INIT)
    {
        /*3. interval options value*/
        if (HI_SUCCESS != OSCSVR_Parser_IntervalOptions_Value(SetOptions, &bExist, pstOptionsTab, pstOSCSetOptions))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Interval Options parser err!!\n");
            return HI_FAILURE;
        }
    }

    /*4. format options value*/
    if (HI_SUCCESS != OSCSVR_Parser_ForMatOptions_Value(SetOptions, &bExist, pstOptionsTab, pstOSCSetOptions))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Format Options parser err!!\n");
        return HI_FAILURE;
    }

    /*5. info options value*/
    if (HI_SUCCESS != OSCSVR_Parser_InfoOptions_Value(SetOptions, &bExist, pstOptionsTab, pstOSCSetOptions))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Info Options parser err!\n");
        return HI_FAILURE;
    }

    if (HI_FALSE == bExist)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "no recoregized options name!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_NAME;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "The input parameter name is unrecognized");
        snprintf_s(aszInvaildName, OSC_COMM_NAME_MAX_LEN, OSC_COMM_NAME_MAX_LEN - 1, OSC_NAME_SET_OPTIONS);
        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_SET_OPTIONS, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_SetOptions_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* penLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(pstOSCSetOptions);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(penLevel);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("sessionId"))
        {
            /*level 2 process*/
            *penLevel = HI_OSC_SERVER_LEVEL_2;
            memset_s(pszSessionId, OSC_SESSIONID_MAX_LEN, 0X00, OSC_SESSIONID_MAX_LEN);
        }
        else
        {
            /*level 1 process*/
            *penLevel = HI_OSC_SERVER_LEVEL_1;

            if (!parameters["sessionId"].IsString())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions sessionid formte err!!!\n");
                s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
                s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
                snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

                goto Invaild;
            }

            strncpy_s(pszSessionId, OSC_SESSIONID_MAX_LEN, parameters["sessionId"].GetString(), strlen(parameters["sessionId"].GetString()));
            pszSessionId[strlen(parameters["sessionId"].GetString())] = '\0';
        }

        /*parser option*/
        if (parameters.HasMember("options") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions has no object options!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");
            goto Invaild;
        }

        const Value& options = parameters["options"];

        if (HI_SUCCESS != OSCSVR_Parser_SetOptions_Value(options, *penLevel, ps32SocketFd, pstOptionsTab, pstOSCSetOptions))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions parser options err!!!\n");
            return HI_FAILURE;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Setoptions has no object options!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");
        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_SET_OPTIONS, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);

    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_InfoOptions_Name(HI_CHAR* pcOptionsName, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions, HI_BOOL* pbExist)
{
    HI_OSC_OPTIONS_S* pstInfoTab = NULL;
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pcOptionsName);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pbExist);

    pstInfoTab = (HI_OSC_OPTIONS_S*)malloc(sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INFO_MAX_NUM);

    if (NULL == pstInfoTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get info Options malloc err!!!\n");
        return HI_FAILURE;
    }

    memset_s(pstInfoTab, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INFO_MAX_NUM, 0x00, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INFO_MAX_NUM);
    HI_OSCSVR_Create_InfoTable(pstInfoTab, pstOptionsTab);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_INFO_MAX_NUM; s32Looper ++)
    {
        if (0 == strncmp(pcOptionsName, pstInfoTab[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
        {
            pstInfoTab[s32Looper].bOptionsFlag = HI_TRUE;
            *pbExist = HI_TRUE;
            pstOSCGetOptions->bInfo = HI_TRUE;
        }
    }

    pstOptionsTab->stInfoTable.bSleepDelay = pstInfoTab[0].bOptionsFlag;
    pstOptionsTab->stInfoTable.bSleepDelaySupport = pstInfoTab[1].bOptionsFlag;
    pstOptionsTab->stInfoTable.bOFFDelay = pstInfoTab[2].bOptionsFlag;
    pstOptionsTab->stInfoTable.bOFFDelaySupport = pstInfoTab[3].bOptionsFlag;
    pstOptionsTab->stInfoTable.bTotalSpace = pstInfoTab[4].bOptionsFlag;
    pstOptionsTab->stInfoTable.bRemainingSpace = pstInfoTab[5].bOptionsFlag;
    pstOptionsTab->stInfoTable.bRemainingPictures = pstInfoTab[6].bOptionsFlag;
    pstOptionsTab->stInfoTable.bDateTimeZone = pstInfoTab[7].bOptionsFlag;
    pstOptionsTab->stInfoTable.bGyro = pstInfoTab[8].bOptionsFlag;
    pstOptionsTab->stInfoTable.bGyroSupport = pstInfoTab[9].bOptionsFlag;
    pstOptionsTab->stInfoTable.bGps = pstInfoTab[10].bOptionsFlag;
    pstOptionsTab->stInfoTable.bGpsSupport = pstInfoTab[11].bOptionsFlag;
    pstOptionsTab->stInfoTable.bDelayProcessing = pstInfoTab[12].bOptionsFlag;
    pstOptionsTab->stInfoTable.bDelayProcessingSupport = pstInfoTab[13].bOptionsFlag;
    pstOptionsTab->stInfoTable.bPollingDelay = pstInfoTab[14].bOptionsFlag;
    pstOptionsTab->stInfoTable.bClientVersion = pstInfoTab[15].bOptionsFlag;
    pstOptionsTab->stInfoTable.bWifiPassword = pstInfoTab[16].bOptionsFlag;
    pstOptionsTab->stInfoTable.bGPSInfo = pstInfoTab[17].bOptionsFlag;

    free(pstInfoTab);
    pstInfoTab = NULL;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Parser_FormatOptions_Name(HI_CHAR* pcOptionsName, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions, HI_BOOL* pbExist)
{
    HI_OSC_OPTIONS_S* pstFormatTab = NULL;
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pcOptionsName);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pbExist);

    pstFormatTab = (HI_OSC_OPTIONS_S*)malloc(sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_FORMAT_MAX_NUM);

    if (NULL == pstFormatTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get format Options malloc err!!!\n");
        return HI_FAILURE;
    }

    memset_s(pstFormatTab, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_FORMAT_MAX_NUM, 0x00, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_FORMAT_MAX_NUM);

    HI_OSCSVR_Create_FormatTable(pstFormatTab, pstOptionsTab);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_FORMAT_MAX_NUM; s32Looper ++)
    {
        if (0 == strncmp(pcOptionsName, pstFormatTab[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
        {
            pstFormatTab[s32Looper].bOptionsFlag = HI_TRUE;
            *pbExist = HI_TRUE;
            pstOSCGetOptions->bForMat = HI_TRUE;
        }
    }

    pstOptionsTab->stFormatTable.bFileFormat = pstFormatTab[0].bOptionsFlag;
    pstOptionsTab->stFormatTable.bFileFormatSupport = pstFormatTab[1].bOptionsFlag;
    pstOptionsTab->stFormatTable.bPreviewFormat = pstFormatTab[2].bOptionsFlag;
    pstOptionsTab->stFormatTable.bPreviewFormatSupport = pstFormatTab[3].bOptionsFlag;

    free(pstFormatTab);
    pstFormatTab = NULL;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Parser_IntervalOptions_Name(HI_CHAR* pcOptionsName, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions, HI_BOOL* pbExist)
{
    HI_OSC_OPTIONS_S* pstIntervalTab = NULL;
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pcOptionsName);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pbExist);

    pstIntervalTab = (HI_OSC_OPTIONS_S*)malloc(sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INTERVAL_MAX_NUM);

    if (NULL == pstIntervalTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get Interval Options malloc err!!!\n");
        return HI_FAILURE;
    }

    memset_s(pstIntervalTab, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INTERVAL_MAX_NUM, 0x00, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_INTERVAL_MAX_NUM);
    HI_OSCSVR_Create_IntervalTable(pstIntervalTab, pstOptionsTab);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_INTERVAL_MAX_NUM; s32Looper ++)
    {
        if (0 == strncmp(pcOptionsName, pstIntervalTab[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
        {
            pstIntervalTab[s32Looper].bOptionsFlag = HI_TRUE;
            pstOSCGetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_INTERVAL;
            *pbExist = HI_TRUE;
        }
    }

    pstOptionsTab->stVideoTable.bRemainingVideoSeconds = pstIntervalTab[0].bOptionsFlag;
    pstOptionsTab->stVideoTable.bVideoStitching = pstIntervalTab[1].bOptionsFlag;
    pstOptionsTab->stVideoTable.bVideoStitchingSupport = pstIntervalTab[2].bOptionsFlag;
    pstOptionsTab->stVideoTable.bVideoGPS = pstIntervalTab[3].bOptionsFlag;
    pstOptionsTab->stVideoTable.bVideoGPSSupport = pstIntervalTab[4].bOptionsFlag;

    free(pstIntervalTab);
    pstIntervalTab = NULL;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Parser_ImageOptions_Name(HI_CHAR* pcOptionsName, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions, HI_BOOL* pbExist)
{
    HI_OSC_OPTIONS_S* pstImageTab = NULL;
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pcOptionsName);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pbExist);

    pstImageTab = (HI_OSC_OPTIONS_S*)malloc(sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_IMAGE_MAX_NUM);

    if (NULL == pstImageTab)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Get Image Options malloc err!!!\n");
        return HI_FAILURE;
    }

    memset_s(pstImageTab, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_IMAGE_MAX_NUM, 0x00, sizeof(HI_OSC_OPTIONS_S) * HI_OSC_OPTIONS_IMAGE_MAX_NUM);

    HI_OSCSVR_Create_ImageTable(pstImageTab, pstOptionsTab, enLevel);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_IMAGE_MAX_NUM; s32Looper ++)
    {
        if (0 == strncmp(pcOptionsName, pstImageTab[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
        {
            pstImageTab[s32Looper].bOptionsFlag = HI_TRUE;
            pstOSCGetOptions->enOptionMode = HI_OSC_OPTIONS_MODE_IMAGE;
            *pbExist = HI_TRUE;
        }
    }

    pstOptionsTab->stImageTable.bExposureProgram = pstImageTab[0].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureProgramSupport = pstImageTab[1].bOptionsFlag;
    pstOptionsTab->stImageTable.bISO = pstImageTab[2].bOptionsFlag;
    pstOptionsTab->stImageTable.bISOSupport = pstImageTab[3].bOptionsFlag;
    pstOptionsTab->stImageTable.bShutterSpeed = pstImageTab[4].bOptionsFlag;
    pstOptionsTab->stImageTable.bShutterSpeedSupport = pstImageTab[5].bOptionsFlag;
    pstOptionsTab->stImageTable.bAperture = pstImageTab[6].bOptionsFlag;
    pstOptionsTab->stImageTable.bApertureSupport = pstImageTab[7].bOptionsFlag;
    pstOptionsTab->stImageTable.bWhiteBalance = pstImageTab[8].bOptionsFlag;
    pstOptionsTab->stImageTable.bWhiteBalanceSupport = pstImageTab[9].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureCompensation = pstImageTab[10].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureCompensationSupport = pstImageTab[11].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureDelay = pstImageTab[12].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureDelaySupport = pstImageTab[13].bOptionsFlag;

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        pstOptionsTab->stImageTable.bLevel1hdr = pstImageTab[14].bOptionsFlag;
        pstOptionsTab->stImageTable.bLevel1hdrSupport = pstImageTab[15].bOptionsFlag;
    }
    else if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        pstOptionsTab->stImageTable.bLevel2Hdr = pstImageTab[14].bOptionsFlag;
        pstOptionsTab->stImageTable.bLevel2HdrSupport = pstImageTab[15].bOptionsFlag;
    }

    pstOptionsTab->stImageTable.bImageStabilization = pstImageTab[16].bOptionsFlag;
    pstOptionsTab->stImageTable.bImageStabilizationSupport = pstImageTab[17].bOptionsFlag;
    pstOptionsTab->stImageTable.bPhotoStitching = pstImageTab[18].bOptionsFlag;
    pstOptionsTab->stImageTable.bPhotoStitchingSupport = pstImageTab[19].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureBracket = pstImageTab[20].bOptionsFlag;
    pstOptionsTab->stImageTable.bExposureBracketSupport = pstImageTab[21].bOptionsFlag;
    pstOptionsTab->stImageTable.bCaptureInterval = pstImageTab[22].bOptionsFlag;
    pstOptionsTab->stImageTable.bCaptureIntervalSupport = pstImageTab[23].bOptionsFlag;
    pstOptionsTab->stImageTable.bCaptureNumber = pstImageTab[24].bOptionsFlag;
    pstOptionsTab->stImageTable.bCaptureNumberSupport = pstImageTab[25].bOptionsFlag;

    free(pstImageTab);
    pstImageTab = NULL;
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Parser_GetOptions(HI_CHAR* pcOptionsName, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions, HI_BOOL* pbExist)
{
    HI_CHAR aszOptions[HI_OSC_OPTIONS_NAME_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pcOptionsName);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    snprintf_s(aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, HI_OSC_OPTIONS_NAME_MAX_LEN - 1, "captureMode");

    if (0 == strncmp(pcOptionsName, aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
    {
        pstOptionsTab->bCaptureMode = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    memset_s(aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, 0x00, HI_OSC_OPTIONS_NAME_MAX_LEN);

    snprintf_s(aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, HI_OSC_OPTIONS_NAME_MAX_LEN - 1, "captureModeSupport");

    if (0 == strncmp(pcOptionsName, aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN))
    {
        pstOptionsTab->bCaptureModeSupport = HI_TRUE;
        *pbExist = HI_TRUE;
    }

    if (HI_SUCCESS != OSCSVR_Parser_ImageOptions_Name(pcOptionsName, enLevel, pstOptionsTab, pstOSCGetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions parser imageoptions name err!!!\n");
        return HI_FAILURE;
    }

    /*image or interval*/
    if (HI_SUCCESS != OSCSVR_Parser_IntervalOptions_Name(pcOptionsName, pstOptionsTab, pstOSCGetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions parser intervaloptions name err!!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Parser_FormatOptions_Name(pcOptionsName, pstOptionsTab, pstOSCGetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions parser Formatoptions name err!!!\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != OSCSVR_Parser_InfoOptions_Name(pcOptionsName, pstOptionsTab, pstOSCGetOptions, pbExist))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions parser infooptions name err!!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 OSCSVR_Parser_GetOptions_Name(const Value& Options, HI_S32* ps32SocketFd, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Looper = 0;
    HI_CHAR aszOptionName[HI_OSC_OPTIONS_NAME_MAX_LEN] = {0};
    HI_BOOL bExist = HI_FALSE;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32RespLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);
    OSCSVR_CHECK_NULL_ERROR(pstOSCGetOptions);

    if (!Options.IsArray())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions is not a array!!\n");
        return HI_FAILURE;
    }

    for (u32Looper = 0; u32Looper < Options.Size(); u32Looper ++)
    {
        if (!Options[u32Looper].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions array member formate err!!!\n");
            return HI_FAILURE;
        }

        memset_s(aszOptionName, HI_OSC_OPTIONS_NAME_MAX_LEN, 0x00, HI_OSC_OPTIONS_NAME_MAX_LEN);
        strncpy_s(aszOptionName, HI_OSC_OPTIONS_NAME_MAX_LEN, Options[u32Looper].GetString(), strlen(Options[u32Looper].GetString()) + 1);
        aszOptionName[strlen(Options[u32Looper].GetString())] = '\0';

        /*get each array member*/
        s32Ret = OSCSVR_Parser_GetOptions(aszOptionName, enLevel, pstOptionsTab, pstOSCGetOptions, &bExist);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions get options name member err!!!\n");
            return HI_FAILURE;
        }

        if (bExist == HI_FALSE)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions json parse member is invaild!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_NAME;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "One or more input parameter name or option name is unrecognized or unsupported");

            goto Invaild;
        }
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_OPTIONS, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_GetOptions_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* penLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions)
{
    Document document;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32RespLen = 0;
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32JsonLen = 0;
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(pstOSCGetOptions);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);
    OSCSVR_CHECK_NULL_ERROR(penLevel);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");
        goto Invaild;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");
            goto Invaild;
        }

        if (!parameters.HasMember("sessionId"))
        {
            /*level 2 process*/
            *penLevel = HI_OSC_SERVER_LEVEL_2;
        }
        else
        {
            /*level 1 process*/
            *penLevel = HI_OSC_SERVER_LEVEL_1;

            if (!parameters["sessionId"].IsString())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions json parse member formte err!!!\n");
                s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
                s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
                snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

                goto Invaild;
            }

            strncpy_s(pszSessionId, OSC_SESSIONID_MAX_LEN, parameters["sessionId"].GetString(), strlen(parameters["sessionId"].GetString()));
            pszSessionId[strlen(parameters["sessionId"].GetString())] = '\0';
        }

        /*parser option*/
        if (parameters.HasMember("optionNames") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions has no object options!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

            goto Invaild;
        }

        const Value& optionNames = parameters["optionNames"];

        if (HI_SUCCESS != OSCSVR_Parser_GetOptions_Name(optionNames, ps32SocketFd, *penLevel, pstOptionsTab, pstOSCGetOptions))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions parser optionsname err!!!\n");
            return HI_FAILURE;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_OPTIONS, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_ProcessPicture_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_CHAR aszPreviewFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN])
{
    Document document;
    HI_U32  u32Looper = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    /*added in level2*/
    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture has no member name!!!\n");
        return HI_FAILURE;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture json parse member formte err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.HasMember("previewFileUrls"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        const Value& previewFileUrls = parameters["previewFileUrls"];

        if (!previewFileUrls.IsArray())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture previewFileUrls is not a array!!\n");
            return HI_FAILURE;
        }

        if (HI_OSC_FILEURL_MAX_NUM <= previewFileUrls.Size())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture the number of previewFileUrls is over!!\n");
            return HI_FAILURE;
        }

        for (u32Looper = 0; u32Looper < previewFileUrls.Size(); u32Looper ++)
        {
            strncpy_s(*(aszPreviewFileUrl + u32Looper), HI_OSC_FILEURL_MAX_LEN, previewFileUrls[u32Looper].GetString(), strlen(previewFileUrls[u32Looper].GetString()));
            *(aszPreviewFileUrl + u32Looper)[strlen(previewFileUrls[u32Looper].GetString())] = '\0';
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is missing");
        goto Invaild;
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
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_Reset_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszComName)
{
    Document document;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    /*added in level2*/
    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet has no member name!!!\n");
        return HI_FAILURE;
    }

    /*start parser jsonpack*/
    if (!document["name"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ReSet json parse member format err!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter name type is wrong!");

        goto Invaild;
    }

    strncpy_s(pszComName, OSC_COMM_NAME_MAX_LEN, document["name"].GetString(), strlen(document["name"].GetString()));
    pszComName[strlen(document["name"].GetString())] = '\0';

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_RESET, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_SwitchWifi_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_SWITCH_WIFI_S* pstSwitchWifi)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pstSwitchWifi);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");
            goto Invaild;
        }

        if (!parameters.HasMember("preSharedKey"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has no member preSharedKey!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter preSharedKey is missing");

            goto Invaild;
        }

        if (!parameters["preSharedKey"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi json parse member preSharedKey format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter preSharedKey type is wrong!");

            goto Invaild;
        }

        strncpy_s(pstSwitchWifi->aszpreSharedKey, HI_OSCSVR_PRESHARED_KEY_MAX_LEN, parameters["preSharedKey"].GetString(), strlen(parameters["preSharedKey"].GetString()));
        pstSwitchWifi->aszpreSharedKey[strlen(parameters["preSharedKey"].GetString())] = '\0';

        /*parser option*/
        if (parameters.HasMember("wifiSsid") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has no wifiSsid!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter wifiSsid is missing");

            goto Invaild;
        }

        if (!parameters["wifiSsid"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi json parse member wifiSsid format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter wifiSsid type is wrong!");

            goto Invaild;
        }

        if (strlen(parameters["wifiSsid"].GetString()) > HI_OSCSVR_WIFISSID_MAX_LEN)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi json parse member wifiSsid len err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter wifiSsid len is wrong!");

            goto Invaild;
        }

        strncpy_s(pstSwitchWifi->aszWifiSSID, HI_OSCSVR_WIFISSID_MAX_LEN, parameters["wifiSsid"].GetString(), strlen(parameters["wifiSsid"].GetString()));
        pstSwitchWifi->aszWifiSSID[strlen(parameters["wifiSsid"].GetString())] = '\0';

        /*parser option*/
        if (parameters.HasMember("wifiPwd") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has no wifiPwd!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter wifiPwd is missing");

            goto Invaild;
        }

        if (!parameters["wifiPwd"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi json parse member wifiPwd format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter wifiPwd type is wrong!");

            goto Invaild;
        }

        strncpy_s(pstSwitchWifi->aszWifiPWD, HI_OSCSVR_WIFIPWD_MAX_LEN, parameters["wifiPwd"].GetString(), strlen(parameters["wifiPwd"].GetString()));
        pstSwitchWifi->aszWifiPWD[strlen(parameters["wifiPwd"].GetString())] = '\0';
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_SWITCH_WIFI, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_UploadFile_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_UPLOAD_FILE_S* pstUploadFile)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pstUploadFile);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("fileUrl"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has no member fileUrl!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter fileUrl is missing");
            goto Invaild;
        }

        if (!parameters["fileUrl"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile json parse member fileUrl format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter fileUrl type is wrong!");
            goto Invaild;
        }

        strncpy_s(pstUploadFile->aszFileUrl, HI_OSC_FILEURL_MAX_LEN, parameters["fileUrl"].GetString(), strlen(parameters["fileUrl"].GetString()));
        pstUploadFile->aszFileUrl[strlen(parameters["fileUrl"].GetString())] = '\0';

        /*parser option*/
        if (parameters.HasMember("uploadUrl") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has no uploadUrl!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter uploadUrl is missing");
            goto Invaild;
        }

        if (!parameters["uploadUrl"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile json parse member uploadUrl format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter uploadUrl type is wrong!");

            goto Invaild;
        }

        strncpy_s(pstUploadFile->aszUploadUrl, HI_OSCSVR_UPLOADURL_MAX_LEN, parameters["uploadUrl"].GetString(), strlen(parameters["uploadUrl"].GetString()));
        pstUploadFile->aszUploadUrl[strlen(parameters["uploadUrl"].GetString())] = '\0';

        /*parser option*/
        if (parameters.HasMember("accessToken") != 1)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has no accessToken!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter accessToken is missing");

            goto Invaild;
        }

        if (!parameters["accessToken"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile json parse member accessToken format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter accessToken type is wrong!");

            goto Invaild;
        }

        if (strlen(parameters["accessToken"].GetString()) > HI_OSCSVR_ACCESS_TOKEN_MAX_LEN)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile json parse member accessToken len err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter accessToken len is wrong!");

            goto Invaild;
        }

        strncpy_s(pstUploadFile->aszAccessToken, HI_OSCSVR_ACCESS_TOKEN_MAX_LEN, parameters["accessToken"].GetString(), strlen(parameters["accessToken"].GetString()));
        pstUploadFile->aszAccessToken[strlen(parameters["accessToken"].GetString())] = '\0';
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UploadFile has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_UPLOAD_FILE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_GetImage_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszFileUrl, HI_S32* ps32MaxSize)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pszFileUrl);
    OSCSVR_CHECK_NULL_ERROR(ps32MaxSize);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (!document.IsObject())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages has other content!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages has no member name!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages json parse member format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter object is invaild");

            goto Invaild;
        }

        if (!parameters.HasMember("fileUri"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages has no member fileUri!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter fileUri is missing");

            goto Invaild;
        }

        /*start parser jsonpack*/
        if (!parameters["fileUri"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages member fileUri type is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileUri data type is incorrect");

            goto Invaild;
        }

        strncpy_s(pszFileUrl, HI_OSC_FILEURL_MAX_LEN, parameters["fileUri"].GetString(), strlen(parameters["fileUri"].GetString()));
        pszFileUrl[strlen(parameters["fileUri"].GetString())] = '\0';

        if (NULL == strchr(pszFileUrl, '.'))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages member fileUri format is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileUri data type is incorrect");

            goto Invaild;
        }

        /*parser option*/
        if (parameters.HasMember("maxSize") == 1)
        {
            /*start parser jsonpack*/
            if (!parameters["maxSize"].IsInt())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages member maxSize type is wrong!!!\n");
                s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
                s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
                snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "maxSize data type is incorrect");

                goto Invaild;
            }

            *ps32MaxSize = parameters["maxSize"].GetInt();
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetImages has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_IMAGE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}
static HI_S32 OSCSVR_Parser_ListFilesParam(const Value& parameters, HI_S32* ps32SocketFd, HI_S32 s32ListFiles, HI_OSCSERVER_LIST_FILE_S* pstListFile)
{
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;

    OSCSVR_CHECK_NULL_ERROR(pstListFile);

    if (!parameters.HasMember("fileType"))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles has no member fileType!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter fileType is missing");

        goto Invaild;
    }

    if (!parameters["fileType"].IsString())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles member fileType type is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileType data type is incorrect");

        goto Invaild;
    }

    if (strlen(parameters["fileType"].GetString()) > HI_OSC_LISTFILE_FILETYPE_LEN)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles member fileType len is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileType data len is incorrect");

        goto Invaild;
    }

    strncpy_s(pstListFile->aszFileType, HI_OSC_LISTFILE_FILETYPE_LEN, parameters["fileType"].GetString(), strlen(parameters["fileType"].GetString()));
    pstListFile->aszFileType[strlen(parameters["fileType"].GetString())] = '\0';

    if ((0 != strncmp(pstListFile->aszFileType, "video", strlen("video")))
        && (0 != strncmp(pstListFile->aszFileType, "image", strlen("image")))
        && (0 != strncmp(pstListFile->aszFileType, "all", strlen("all"))))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles member fileType type is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileType data type is incorrect");

        goto Invaild;
    }

    /*startPosition is optional*/
    if (parameters.HasMember("startPosition") == 1)
    {
        pstListFile->s32StartPosition = parameters["startPosition"].GetInt();

        if (pstListFile->s32StartPosition < 0)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles member startPosition type is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "startPosition data type is incorrect");
            goto Invaild;
        }
    }
    else
    {
        pstListFile->s32StartPosition = 0;
    }

    /*parser entryCount*/
    if (parameters.HasMember("entryCount") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles has no entryCount!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter entryCount is missing");

        goto Invaild;
    }

    pstListFile->s32EntryCount = parameters["entryCount"].GetInt();

    if (pstListFile->s32EntryCount <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles value is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "entryCount value is invaild");
        goto Invaild;
    }
    else if (pstListFile->s32EntryCount >= s32ListFiles)
    {
        pstListFile->s32EntryCount = s32ListFiles;
    }

    /*parser entryCount*/
    if (parameters.HasMember("maxThumbSize") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles has no maxThumbSize!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter maxThumbSize is missing");
        goto Invaild;
    }

    if (parameters["maxThumbSize"].IsNull())
    {
        /*client no need thumbnail, "maxThumbSize" : null*/
        pstListFile->bIncludeThumb =  HI_FALSE;
        return HI_SUCCESS;
    }

    pstListFile->s32MaxThumbSize = parameters["maxThumbSize"].GetInt();
    pstListFile->bIncludeThumb =  HI_TRUE;

    if (pstListFile->s32MaxThumbSize < 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles s32MaxThumbSize value is invaild!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "MaxThumbSize value is invaild");

        goto Invaild;
    }

    return HI_SUCCESS;

Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_FILE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_ListFiles_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_S32 s32ListFiles, HI_OSCSERVER_LIST_FILE_S* pstListFile)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pstListFile);

    /*added in level2*/
    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles json parse member format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter object is invaild");

            goto Invaild;
        }

        if (HI_SUCCESS != OSCSVR_Parser_ListFilesParam(parameters, ps32SocketFd, s32ListFiles, pstListFile))
        {
            return HI_FAILURE;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListFiles has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_FILE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

static HI_S32 OSCSVR_Parser_ListImagesParam(const Value& parameters, HI_S32* ps32SocketFd, HI_S32 s32ListFiles, HI_OSCSERVER_LIST_IMAGE_S* pstListImage)
{
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    if (1 != parameters.HasMember("entryCount"))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages has no member entryCount!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter entryCount is missing");
        goto Invaild;
    }

    if (!parameters["entryCount"].IsInt())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages member entryCount type is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "entryCount data type is incorrect");
        goto Invaild;
    }

    pstListImage->s32EntryCounts = parameters["entryCount"].GetInt();

    if (pstListImage->s32EntryCounts <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages member entryCount type is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "entryCount data value is incorrect");

        goto Invaild;
    }
    else if (pstListImage->s32EntryCounts >= s32ListFiles)
    {
        pstListImage->s32EntryCounts = s32ListFiles;
    }

    if (parameters.HasMember("maxSize") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages has no maxSize member!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter maxSize is missing");
        goto Invaild;
    }

    pstListImage->s32MaxThumbSize = parameters["maxSize"].GetInt();

    if (pstListImage->s32MaxThumbSize < 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages member maxSize type is wrong!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "maxSize data value is incorrect");
        goto Invaild;
    }

    if (parameters.HasMember("continuationToken") == 1)
    {
        if (!parameters["continuationToken"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages member continuationToken type is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "continuationToken data type is incorrect");

            goto Invaild;
        }

        if (strlen(parameters["continuationToken"].GetString()) > HI_OSC_CONTINUDATION_TOKEN_MAX_LEN)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages member continuationToken len is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "continuationToken data len is incorrect");

            goto Invaild;
        }

        strncpy_s(pstListImage->aszInContinuationToken, HI_OSC_CONTINUDATION_TOKEN_MAX_LEN, parameters["continuationToken"].GetString(), strlen(parameters["continuationToken"].GetString()));
        pstListImage->aszInContinuationToken[strlen(parameters["continuationToken"].GetString())] = '\0';
    }

    if (parameters.HasMember("includeThumb") == 1)
    {
        if (1 == parameters["includeThumb"].GetBool())
        {
            pstListImage->bIncludeThummb = HI_TRUE;
        }
        else
        {
            pstListImage->bIncludeThummb = HI_FALSE;
        }
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_IMAGE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}


HI_S32 OSCSVR_Parser_ListImage_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_S32 s32ListImages, HI_OSCSERVER_LIST_IMAGE_S* pstListImage)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pstListImage);

    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages has no member name!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages json parse member format err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter object is invaild");

            goto Invaild;
        }

        if (HI_SUCCESS != OSCSVR_Parser_ListImagesParam(parameters, ps32SocketFd, s32ListImages, pstListImage))
        {
            return HI_FAILURE;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ListImages has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_LIST_IMAGE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_GetMetaData_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_METADATA_S* pstMetaData)
{
    Document document;
    HI_S32  s32RespLen = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);
    OSCSVR_CHECK_NULL_ERROR(pstMetaData);

    /*added in level2*/
    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData has no member name!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");
            goto Invaild;
        }

        if (!parameters.HasMember("fileUri"))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData has no member fileUri!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter fileUri is missing");
            goto Invaild;
        }

        if (!parameters["fileUri"].IsString())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData member fileUri type is wrong!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileUri data type is incorrect");
            goto Invaild;
        }

        strncpy_s(pstMetaData->aszFileUrl, HI_OSC_FILEURL_MAX_LEN, parameters["fileUri"].GetString(), strlen(parameters["fileUri"].GetString()));
        pstMetaData->aszFileUrl[strlen(parameters["fileUri"].GetString())] = '\0';
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetMetaData has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");
        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_GET_METADATA, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

HI_S32 OSCSVR_Parser_Delete_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSC_SERVER_LEVEL_E* enLevel, HI_CHAR aszDeleteUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN])
{
    Document document;
    HI_U32  u32Looper = 0;
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_S32  s32InvaildCode = HIOSC_SERVER_ERROR_INIT;
    HI_CHAR aszInvaildMessage[OSC_INVAILD_MESSAGE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32SocketFd);

    /*added in level2*/
    if (document.ParseInsitu(pszJsonPack).HasParseError())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete ParseInsitu error!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("name") != 1)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete has no member name!!!\n");
        return HI_FAILURE;
    }

    if (document.HasMember("parameters") == 1)
    {
        const Value& parameters = document["parameters"];

        if (!parameters.IsObject())
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete json parse member formte err!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter value is invaild");
            goto Invaild;
        }

        if (1 == parameters.HasMember("fileUri"))
        {
            if (!parameters["fileUri"].IsString())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete member fileUri type is wrong!!!\n");
                s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
                s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
                snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileUri data type is incorrect");
                goto Invaild;
            }

            *enLevel = HI_OSC_SERVER_LEVEL_1;
            strncpy_s(aszDeleteUrl[0], HI_OSC_FILEURL_MAX_LEN, parameters["fileUri"].GetString(), strlen(parameters["fileUri"].GetString()));
            aszDeleteUrl[0][strlen(parameters["fileUri"].GetString())] = '\0';
        }
        else if (1 == parameters.HasMember("fileUrls"))
        {
            *enLevel = HI_OSC_SERVER_LEVEL_2;
            const Value& fileUrls = parameters["fileUrls"];

            if (!fileUrls.IsArray())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete fileUrls is not a array!!\n");
                return HI_FAILURE;
            }

            if (HI_OSC_FILEURL_MAX_NUM <= fileUrls.Size())
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete the number of fileUrls is over!!\n");
                return HI_FAILURE;
            }

            for (u32Looper = 0; u32Looper < fileUrls.Size(); u32Looper ++)
            {
                if (!fileUrls[u32Looper].IsString())
                {
                    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete member fileUrls type is wrong!!!\n");
                    s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
                    s32InvaildCode = HIOSC_INVAILD_PARAMETER_VALUE;
                    snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "fileUrls data type is incorrect");

                    goto Invaild;
                }

                strncpy_s(aszDeleteUrl[u32Looper], HI_OSC_FILEURL_MAX_LEN, fileUrls[u32Looper].GetString(), strlen(fileUrls[u32Looper].GetString()));
                aszDeleteUrl[u32Looper][strlen(fileUrls[u32Looper].GetString())] = '\0';
            }
        }
        else
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete has no object parameters arguments!!!\n");
            s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
            s32InvaildCode = HIOSC_MISSING_PARAMETER;
            snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

            goto Invaild;
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete has no object parameters!!!\n");
        s32StatCode = HI_OSC_STATUS_BAD_REQUEST;
        s32InvaildCode = HIOSC_MISSING_PARAMETER;
        snprintf_s(aszInvaildMessage, OSC_INVAILD_MESSAGE_MAX_LEN, OSC_INVAILD_MESSAGE_MAX_LEN - 1, "parameter is missing");

        goto Invaild;
    }

    return HI_SUCCESS;
Invaild:
    OSCSVR_Generate_Invaild_JsonPack(s32InvaildCode, (HI_CHAR*)OSC_NAME_DELETE, aszInvaildMessage, aszJsonPack, &s32JsonLen);
    OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);
    }

    HI_OSCSVR_CloseSocket(ps32SocketFd);
    return HI_FAILURE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
