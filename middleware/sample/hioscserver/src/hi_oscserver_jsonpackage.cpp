/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_jsonpackage.cpp
* @brief     oscserver json package src file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <exception>
/*osc module header file*/
#include "hi_osc_log.h"
#include "hi_osc_server.h"
#include "hi_oscserver_http.h"
#include "hi_osc_server_err.h"
#include "hi_oscserver_jsonparser.h"
#include "hi_oscserver_jsonpackage.h"
#include "hi_oscserver_com_exec.h"
#include "hi_oscserver_file_exec.h"
#include "hi_oscserver_session.h"
/*rapid json thirdparty header file*/
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

HI_S32 OSCSVR_Generate_Invaild_JsonPack(HI_S32 s32InvaildCode, HI_CHAR* pszComName, HI_CHAR* pszMessage, HI_CHAR* pszJsonPack, HI_S32* s32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);
    HI_CHAR aszInavildCode[OSC_INVAILD_CODE_MAX_LEN] = {0};

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(s32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszComName);
    OSCSVR_CHECK_NULL_ERROR(pszMessage);

    switch (s32InvaildCode)
    {
        case HIOSC_MISSING_PARAMETER:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_MISSING_PARAM, strlen(OSC_INVAILD_STATE_MISSING_PARAM));
            aszInavildCode[strlen(OSC_INVAILD_STATE_MISSING_PARAM)] = '\0';
            break;

        case HIOSC_INVAILD_PARAMETER_NAME:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_INVAILD_PARAM_NAME, strlen(OSC_INVAILD_STATE_INVAILD_PARAM_NAME));
            aszInavildCode[strlen(OSC_INVAILD_STATE_INVAILD_PARAM_NAME)] = '\0';
            break;

        case HIOSC_INVAILD_PARAMETER_VALUE:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_INVAILD_PARAM_VALUE, strlen(OSC_INVAILD_STATE_INVAILD_PARAM_VALUE));
            aszInavildCode[strlen(OSC_INVAILD_STATE_INVAILD_PARAM_VALUE)] = '\0';
            break;

        case HIOSC_INVAILD_IN_EXCLUSIVEUSE:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_IN_EXCLUSIVEUSE, strlen(OSC_INVAILD_STATE_IN_EXCLUSIVEUSE));
            aszInavildCode[strlen(OSC_INVAILD_STATE_IN_EXCLUSIVEUSE)] = '\0';
            break;

        case HIOSC_INVAILD_DISABLED_COMMAND:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_DISABLED_COMMAND, strlen(OSC_INVAILD_STATE_DISABLED_COMMAND));
            aszInavildCode[strlen(OSC_INVAILD_STATE_DISABLED_COMMAND)] = '\0';
            break;

        case HIOSC_INVAILD_UNABLE_SERVICE:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_UNABLE_SERVICE, strlen(OSC_INVAILD_STATE_UNABLE_SERVICE));
            aszInavildCode[strlen(OSC_INVAILD_STATE_UNABLE_SERVICE)] = '\0';
            break;

        case HIOSC_INVAILD_UNKNOWN_COMMAND:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_UNKNOWN_COMMAND, strlen(OSC_INVAILD_STATE_UNKNOWN_COMMAND));
            aszInavildCode[strlen(OSC_INVAILD_STATE_UNKNOWN_COMMAND)] = '\0';
            break;

        case HIOSC_INVAILD_UPLOAD_ERROR:
            strncpy_s(aszInavildCode, OSC_INVAILD_CODE_MAX_LEN, OSC_INVAILD_STATE_UPLOAD_ERROR, strlen(OSC_INVAILD_STATE_UPLOAD_ERROR));
            aszInavildCode[strlen(OSC_INVAILD_STATE_UPLOAD_ERROR)] = '\0';
            break;

        default:
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "unsupport invaild code!!\n");
            return HI_FAILURE;
    }

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(pszComName);
    writer.Key("state");
    writer.String("error");
    writer.Key("error");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("code");
    writer.String(aszInavildCode);
    writer.Key("message");
    writer.String(pszMessage);
    writer.EndObject();
    writer.EndObject();

    if (OSC_MAX_PROTOCOL_BUFFER <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invaild response the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, OSC_MAX_PROTOCOL_BUFFER, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *s32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_SVRInfo_JsonPack(HI_OSCSERVER_INFO_S* pstOSCSVRInfo, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pstOSCSVRInfo);
    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("manufacturer");
    writer.String(pstOSCSVRInfo->aszManufacturer);
    writer.Key("model");
    writer.String(pstOSCSVRInfo->aszModel);
    writer.Key("serialNumber");
    writer.String(pstOSCSVRInfo->aszSerialNumber);
    writer.Key("firmwareVersion");
    writer.String(pstOSCSVRInfo->aszFirmwareVersion);
    writer.Key("supportUrl");
    writer.String(pstOSCSVRInfo->aszSupportURL);
    writer.Key("endpoints");
    writer.StartObject();                   /* Between StartObject()/EndObject()*/
    writer.Key("httpPort");
    writer.Uint(pstOSCSVRInfo->s32Httpport);
    writer.Key("httpUpdatesPort");
    writer.Uint(pstOSCSVRInfo->s32HttpUpdatesport);

    if (0 != pstOSCSVRInfo->s32Httpsport)
    {
        writer.Key("httpsPort");
        writer.Uint(pstOSCSVRInfo->s32Httpsport);
    }

    if (0 != pstOSCSVRInfo->s32HttpsUpdatesport)
    {
        writer.Key("httpsUpdatesPort");
        writer.Uint(pstOSCSVRInfo->s32HttpsUpdatesport);
    }

    writer.EndObject();
    writer.Key("gps");
    writer.Bool(pstOSCSVRInfo->bGps);
    writer.Key("gyro");
    writer.Bool(pstOSCSVRInfo->bGyro);
    writer.Key("uptime");
    writer.Uint(pstOSCSVRInfo->s32Uptime);
    writer.Key("api");
    writer.StartArray();                /*Between StartArray()/EndArray()*/
    writer.String("/osc/info");
    writer.String("/osc/state");
    writer.String("/osc/checkForUpdates");
    writer.String("/osc/commands/execute");
    writer.String("/osc/commands/status");
    writer.EndArray();
    writer.Key("apiLevel");
    writer.StartArray();                /*Between StartArray()/EndArray()*/
    writer.Uint(1);
    writer.Uint(2);
    writer.EndArray();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/info the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_SVRState_JsonPack(HI_OSCSERVER_STATE_S* pstOSCSVRState, HI_S32 s32MaxLen, HI_CHAR* pszFingerPrint, HI_OSC_SERVER_LEVEL_E enLevel, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pstOSCSVRState);
    OSCSVR_CHECK_NULL_ERROR(pszFingerPrint);
    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);

    if (HI_OSC_SERVER_LEVEL_BUT == enLevel)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state the input level is invaild!!\n");
        return HI_FAILURE;
    }

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("fingerprint");
    writer.String(pszFingerPrint);
    writer.Key("state");
    writer.StartObject();                   /* Between StartObject()/EndObject()*/

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        writer.Key("sessionId");
        writer.String(pszSessionId);
    }

    writer.Key("batteryLevel");
    writer.Double(pstOSCSVRState->dBatteryLevel);

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        writer.Key("storageChanged");
        writer.Bool(pstOSCSVRState->bStorageChanged);
    }

    if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        writer.Key("storageUri");
        writer.String(pstOSCSVRState->aszStorageUri);
    }

    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/state the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_CheckForUpdate_JsonPack(HI_U32 u32ThrottleTimeout, HI_S32 s32MaxLen, HI_CHAR* pszFingerPrint, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszFingerPrint);
    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("stateFingerprint");
    writer.String(pszFingerPrint);
    writer.Key("throttleTimeout");
    writer.Uint(u32ThrottleTimeout);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/checkforupdates the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_StartSession_JsonPack(HI_S32 s32Timeout, HI_S32 s32MaxLen, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_START_SESSION);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("sessionId");
    writer.String(pszSessionId);
    writer.Key("timeout");
    writer.Uint(s32Timeout);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartSession the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_CloseSession_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_CLOSE_SESSION);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "CloseSession the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_UpdateSession_JsonPack(HI_S32 s32Timeout, HI_S32 s32MaxLen, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszSessionId);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_UPDATE_SESSION);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("sessionId");
    writer.String(pszSessionId);
    writer.Key("timeout");
    writer.Uint(s32Timeout);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_VideoCapture_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_CHAR* pszState)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszState);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_START_CAPTURE);
    writer.Key("state");
    writer.String(pszState);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "VideoCapture the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_TakePicture_JsonPack(HI_CHAR* pszCommandId, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_DOUBLE dCompletion = HI_OSC_COMMAND_STATUS_PROGRESS;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszCommandId);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_TAKE_PICTURE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_INPROGRESS);
    writer.Key("id");
    writer.String(pszCommandId);
    writer.Key("progress");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("completion");
    writer.Double(dCompletion);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "UpdateSession the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_SetOptions_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_SET_OPTIONS);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SetOptions the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_Info_GpsAndGyo(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. gyro*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bGyro)
    {
        OptionsName.Key("gyro");
        OptionsName.Bool(pstOSCOptions->stInfo.bGyro);
    }

    /*2. gyroSupport*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bGyroSupport)
    {
        OptionsName.Key("gyroSupport");
        OptionsName.Bool(pstOSCOptions->stInfo.bGpsSupport);
    }

    /*3. gps*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bGps)
    {
        OptionsName.Key("gps");
        OptionsName.Bool(pstOSCOptions->stInfo.bGps);
    }

    /*4. gpsSupport*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bGpsSupport)
    {
        OptionsName.Key("gpsSupport");
        OptionsName.Bool(pstOSCOptions->stInfo.bGpsSupport);
    }

    /*5. delayProcessing*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bDelayProcessing)
    {
        OptionsName.Key("delayProcessing");
        OptionsName.Bool(pstOSCOptions->stInfo.bDelayProcessing);
    }

    /*6. delayProcessingSupport*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bDelayProcessingSupport)
    {
        OptionsName.Key("delayProcessingSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_DELAY_PROCESS_SUPPORT; s32Looper ++)
        {
            OptionsName.Bool(pstOSCOptions->stInfo.aszDelayProcessingSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*7. wifiPassword*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bWifiPassword)
    {
        OptionsName.Key("wifiPassword");
        OptionsName.String(pstOSCOptions->stInfo.aszWifiPassword);
    }

    /*8. gpsInfo*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bGPSInfo)
    {
        OptionsName.Key("gpsInfo");
        OptionsName.StartObject();
        OptionsName.Key("lat");
        OptionsName.Double(pstOSCOptions->stInfo.stGPSInfo.dLat);
        OptionsName.Key("lng");
        OptionsName.Double(pstOSCOptions->stInfo.stGPSInfo.dLng);
        OptionsName.EndObject();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_InfoOptions(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. sleepDelay*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bSleepDelay)
    {
        OptionsName.Key("sleepDelay");
        OptionsName.Uint(pstOSCOptions->stInfo.s32SleepDelay);
    }

    /*3. offDelay*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bOFFDelay)
    {
        OptionsName.Key("offDelay");
        OptionsName.Uint(pstOSCOptions->stInfo.s32OFFDelay);
    }

    /*4. offDelaySupport*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bOFFDelaySupport)
    {
        OptionsName.Key("offDelaySupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_OFF_DELAY_NUM; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->stInfo.aszOFFDelaySupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*5. totalSpace*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bTotalSpace)
    {
        OptionsName.Key("totalSpace");
        OptionsName.Uint(pstOSCOptions->stInfo.s32TotalSpace);
    }

    /*6. remainingSpace*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bRemainingSpace)
    {
        OptionsName.Key("remainingSpace");
        OptionsName.Uint(pstOSCOptions->stInfo.s32RemainingSpace);
    }

    /*7. remainingPictures*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bRemainingPictures)
    {
        OptionsName.Key("remainingPictures");
        OptionsName.Uint(pstOSCOptions->stInfo.s32RemainingPictures);
    }

    /*8. dateTimeZone*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bDateTimeZone)
    {
        OptionsName.Key("dateTimeZone");
        OptionsName.String(pstOSCOptions->stInfo.aszDateTimeZone);
    }

    /*9. dateTimeZone*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bDateTimeZone)
    {
        OptionsName.Key("dateTimeZone");
        OptionsName.String(pstOSCOptions->stInfo.aszDateTimeZone);
    }

    /*10. clientVersion*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bClientVersion)
    {
        OptionsName.Key("clientVersion");
        OptionsName.Uint(pstOSCOptions->stInfo.s32ClientVersion);
    }

    OSCSVR_Generate_Info_GpsAndGyo(OptionsName, pstOSCOptions, pstOptionsTab);
    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_FormatOptions(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. previewFormat*/
    if (HI_TRUE == pstOptionsTab->stFormatTable.bPreviewFormat)
    {
        OptionsName.Key("previewFormat");
        OptionsName.StartObject();
        OptionsName.Key("width");
        OptionsName.Uint(pstOSCOptions->stForMat.stPreviewFormat.u32Width);
        OptionsName.Key("height");
        OptionsName.Uint(pstOSCOptions->stForMat.stPreviewFormat.u32Height);
        OptionsName.Key("framerate");
        OptionsName.Uint(pstOSCOptions->stForMat.stPreviewFormat.u32FrameRate);
        OptionsName.EndObject();
    }

    /*2. previewFormatSupport*/
    if (HI_TRUE == pstOptionsTab->stFormatTable.bPreviewFormatSupport)
    {
        OptionsName.Key("previewFormatSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_PREVIEW_FORMAT_SUPPORT; s32Looper ++)
        {
            OptionsName.StartObject();
            OptionsName.Key("width");
            OptionsName.Uint(pstOSCOptions->stForMat.aszPreviewFormatSupport[s32Looper].u32Width);
            OptionsName.Key("height");
            OptionsName.Uint(pstOSCOptions->stForMat.aszPreviewFormatSupport[s32Looper].u32Height);
            OptionsName.Key("framerate");
            OptionsName.Uint(pstOSCOptions->stForMat.aszPreviewFormatSupport[s32Looper].u32FrameRate);
            OptionsName.EndObject();
        }

        OptionsName.EndArray();
    }

    /*3. fileFormat*/
    if (HI_TRUE == pstOptionsTab->stFormatTable.bFileFormat)
    {
        OptionsName.Key("fileFormat");
        OptionsName.StartObject();
        OptionsName.Key("type");
        OptionsName.String(pstOSCOptions->stForMat.stFileFormat.aszType);
        OptionsName.Key("width");
        OptionsName.Uint(pstOSCOptions->stForMat.stFileFormat.s32Width);
        OptionsName.Key("height");
        OptionsName.Uint(pstOSCOptions->stForMat.stFileFormat.s32Height);

        if (0 != pstOSCOptions->stForMat.stFileFormat.u32FrameRate)
        {
            OptionsName.Key("height");
            OptionsName.Uint(pstOSCOptions->stForMat.stFileFormat.u32FrameRate);
        }

        OptionsName.EndObject();
    }

    /*4. fileFormatSupport*/
    if (HI_TRUE == pstOptionsTab->stFormatTable.bFileFormatSupport)
    {
        OptionsName.Key("fileFormatSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_FORMAT_SUP_NUM; s32Looper ++)
        {
            OptionsName.StartObject();
            OptionsName.Key("type");
            OptionsName.String(pstOSCOptions->stForMat.aszFileFormatSupport[s32Looper].aszType);
            OptionsName.Key("width");
            OptionsName.Uint(pstOSCOptions->stForMat.aszFileFormatSupport[s32Looper].s32Width);
            OptionsName.Key("height");
            OptionsName.Uint(pstOSCOptions->stForMat.aszFileFormatSupport[s32Looper].s32Height);

            if (0 != pstOSCOptions->stForMat.aszFileFormatSupport[s32Looper].u32FrameRate)
            {
                OptionsName.Key("height");
                OptionsName.Uint(pstOSCOptions->stForMat.aszFileFormatSupport[s32Looper].u32FrameRate);
            }

            OptionsName.EndObject();
        }

        OptionsName.EndArray();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_IntervalOptions(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. remainingVideoSeconds*/
    if (HI_TRUE == pstOptionsTab->stVideoTable.bRemainingVideoSeconds)
    {
        OptionsName.Key("remainingVideoSeconds");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stVideo.s32RemainingVideoSeconds);
    }

    /*2. videoStitching*/
    if (HI_TRUE == pstOptionsTab->stVideoTable.bVideoStitching)
    {
        OptionsName.Key("videoStitching");
        OptionsName.String(pstOSCOptions->unModeStuct.stVideo.aszVideoStitching);
    }

    /*3. videoStitchingSupport*/
    if (HI_TRUE == pstOptionsTab->stVideoTable.bVideoStitchingSupport)
    {
        OptionsName.Key("videoStitchingSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_VIDEO_STITCHING_SUPPORT; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->unModeStuct.stVideo.aszVideoStitchingSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*4. videoGPS*/
    if (HI_TRUE == pstOptionsTab->stVideoTable.bVideoGPS)
    {
        OptionsName.Key("videoGPS");
        OptionsName.String(pstOSCOptions->unModeStuct.stVideo.aszVideoGPS);
    }

    /*5. videoGPSSupport*/
    if (HI_TRUE == pstOptionsTab->stVideoTable.bVideoGPSSupport)
    {
        OptionsName.Key("videoGPSSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_VIDEO_GPS_SUPPORT; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->unModeStuct.stVideo.aszVideoGPSSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_HdrSet(Writer<StringBuffer>& OptionsName, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        /*1. hdr*/
        if (HI_TRUE == pstOptionsTab->stImageTable.bLevel1hdr)
        {
            OptionsName.Key("hdr");
            OptionsName.Bool(pstOSCOptions->unModeStuct.stImage.bLevel1hdr);
        }

        /*2. hdrSupport*/
        if (HI_TRUE == pstOptionsTab->stImageTable.bLevel1hdrSupport)
        {
            OptionsName.Key("hdrSupport");
            OptionsName.Bool(pstOSCOptions->unModeStuct.stImage.bLevel1hdrSupport);
        }
    }
    else if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        /*1. hdr*/
        if (HI_TRUE == pstOptionsTab->stImageTable.bLevel2Hdr)
        {
            OptionsName.Key("hdr");
            OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszLevel2Hdr);
        }

        /*2. hdrSupport*/
        if (HI_TRUE == pstOptionsTab->stImageTable.bLevel2HdrSupport)
        {
            OptionsName.Key("hdrSupport");
            OptionsName.StartArray();

            for (s32Looper = 0; s32Looper < HI_OSC_OPTION_HDR_SUPPORT; s32Looper ++)
            {
                OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszLevel2HdrSupport[s32Looper]);
            }

            OptionsName.EndArray();
        }
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions generate the hdr iput level is unknown!!!\n");
    }

    /*3jiuzai. sleepDelaySupport*/
    if (HI_TRUE == pstOptionsTab->stInfoTable.bSleepDelaySupport)
    {
        OptionsName.Key("sleepDelaySupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_SLEEP_DELAY_NUM; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->stInfo.aszSleepDelaySupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ExposreBracket(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. exposureBracket*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureBracket)
    {
        OptionsName.Key("exposureBracket");
        OptionsName.StartObject();

        if (HI_TRUE == pstOSCOptions->unModeStuct.stImage.stExposureBracket.bAutoMode)
        {
            OptionsName.Key("autoMode");
            OptionsName.Bool(pstOSCOptions->unModeStuct.stImage.stExposureBracket.bAutoMode);
        }
        else
        {
            OptionsName.Key("shots");
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stExposureBracket.s32Shots);
            OptionsName.Key("increment");
            OptionsName.Double(pstOSCOptions->unModeStuct.stImage.stExposureBracket.dIncrement);
        }

        OptionsName.EndObject();
    }

    /*2. exposureBracketSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureBracketSupport)
    {
        OptionsName.Key("exposureBracketSupport");
        OptionsName.StartObject();
        OptionsName.Key("autoMode");
        OptionsName.Bool(pstOSCOptions->unModeStuct.stImage.stExposureBracketSupport.bAutoModeSupport);
        OptionsName.Key("shotsSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_SHOTS_SUPPORT; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stExposureBracketSupport.aszShotsSupport[s32Looper]);
        }

        OptionsName.EndArray();
        OptionsName.Key("incrementSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_INCREMENT_SUPPORT; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stExposureBracketSupport.aszIncrementSupport[s32Looper]);
        }

        OptionsName.EndArray();
        OptionsName.EndObject();
    }

    /*3. photoStitching*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bPhotoStitching)
    {
        OptionsName.Key("photoStitching");
        OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszPhotoStitching);
    }

    /*4. photoStitchingSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bPhotoStitchingSupport)
    {
        OptionsName.Key("photoStitchingSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_PHOTO_STITCHING_SUPPORT; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszPhotoStitchingSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ExposreSet(Writer<StringBuffer>& OptionsName, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*9. imageStabilization*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bImageStabilization)
    {
        OptionsName.Key("imageStabilization");
        OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszImageStabilization);
    }

    /*10. imageStabilizationSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bImageStabilizationSupport)
    {
        OptionsName.Key("imageStabilizationSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_IMAGE_STABILIZATION_SUPPORT; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszImageStabilizationSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*1. exposureProgram*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureProgram)
    {
        OptionsName.Key("exposureProgram");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.s32ExposureProgram);
    }

    /*2. exposureProgramSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureProgramSupport)
    {
        OptionsName.Key("exposureProgramSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_EXPOSE_SUPPORT_NUM; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.aszExposureProgramSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*3. exposureCompensation*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureCompensation)
    {
        OptionsName.Key("exposureCompensation");
        OptionsName.Double(pstOSCOptions->unModeStuct.stImage.dExposureCompensation);
    }

    /*4. exposureCompensationSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureCompensationSupport)
    {
        OptionsName.Key("exposureCompensationSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_EXPOSE_COMPENSATE_NUM; s32Looper ++)
        {
            OptionsName.Double(pstOSCOptions->unModeStuct.stImage.aszExposureCompensationSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*5. exposureDelay*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureDelay)
    {
        OptionsName.Key("exposureDelay");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.s32ExposureDelay);
    }

    /*6. exposureDelaySupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bExposureDelaySupport)
    {
        OptionsName.Key("exposureDelaySupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_EXPOSE_DELAY_NUM; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.aszExposureDelaySupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ImageOptions(Writer<StringBuffer>& OptionsName, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    /*1. iso*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bISO)
    {
        OptionsName.Key("iso");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.s32ISO);
    }

    /*2. isoSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bISOSupport)
    {
        OptionsName.Key("isoSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_IOS_SUPPORT_NUM; s32Looper ++)
        {
            OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.aszISOSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*3. shutterSpeed*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bShutterSpeed)
    {
        OptionsName.Key("shutterSpeed");
        OptionsName.Double(pstOSCOptions->unModeStuct.stImage.dShutterSpeed);
    }

    /*4. shutterSpeedSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bShutterSpeedSupport)
    {
        OptionsName.Key("shutterSpeedSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_SHUTTER_SUPPORT_NUM; s32Looper ++)
        {
            OptionsName.Double(pstOSCOptions->unModeStuct.stImage.aszShutterSpeedSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*5. aperture*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bAperture)
    {
        OptionsName.Key("aperture");
        OptionsName.Double(pstOSCOptions->unModeStuct.stImage.dAperture);
    }

    /*6. apertureSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bApertureSupport)
    {
        OptionsName.Key("apertureSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_APERTURE_SUPPORT_NUM; s32Looper ++)
        {
            OptionsName.Double(pstOSCOptions->unModeStuct.stImage.aszApertureSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*7. whiteBalance*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bWhiteBalance)
    {
        OptionsName.Key("whiteBalance");
        OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszWhiteBalance);
    }

    /*8. whiteBalanceSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bWhiteBalanceSupport)
    {
        OptionsName.Key("whiteBalanceSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_WHITE_BALANCE_NUM; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->unModeStuct.stImage.aszWhiteBalanceSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    /*Hdr*/
    OSCSVR_Generate_HdrSet(OptionsName, enLevel, pstOSCOptions, pstOptionsTab);
    /*Expourse*/
    OSCSVR_Generate_ExposreSet(OptionsName, pstOSCOptions, pstOptionsTab);
    /*Expourse bracket*/
    OSCSVR_Generate_ExposreBracket(OptionsName, pstOSCOptions, pstOptionsTab);

    /*6. captureInterval*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bCaptureInterval)
    {
        OptionsName.Key("captureInterval");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.s32CaptureInterval);
    }

    /*7. captureIntervalSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bCaptureIntervalSupport)
    {
        OptionsName.Key("captureIntervalSupport");
        OptionsName.StartObject();
        OptionsName.Key("minInterval");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stCaptureIntervalSupport.u32MinInterval);
        OptionsName.Key("maxInterval");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stCaptureIntervalSupport.u32MaxInterval);
        OptionsName.EndObject();
    }

    /*8. captureNumber*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bCaptureNumber)
    {
        OptionsName.Key("captureNumber");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.s32CaptureInterval);
    }

    /*9. captureNumberSupport*/
    if (HI_TRUE == pstOptionsTab->stImageTable.bCaptureNumberSupport)
    {
        OptionsName.Key("captureNumberSupport");
        OptionsName.StartObject();
        OptionsName.Key("minNumber");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stCaptureNumberSupport.u32MinNumber);
        OptionsName.Key("maxNumber");
        OptionsName.Uint(pstOSCOptions->unModeStuct.stImage.stCaptureNumberSupport.u32MaxNumber);
        OptionsName.EndObject();
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_GetOptions_Object(Writer<StringBuffer>& OptionsName, HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstOSCOptions);
    OSCSVR_CHECK_NULL_ERROR(pstOptionsTab);

    if (HI_TRUE == pstOptionsTab->bCaptureMode)
    {
        OptionsName.Key("captureMode");
        OptionsName.String(pstOSCOptions->aszCaptureMode);
    }

    if (HI_TRUE == pstOptionsTab->bCaptureModeSupport)
    {
        OptionsName.Key("captureModeSupport");
        OptionsName.StartArray();

        for (s32Looper = 0; s32Looper < HI_OSC_OPTION_CAPTURE_MODE_SUPPORT; s32Looper ++)
        {
            OptionsName.String(pstOSCOptions->aszCaptureModeSupport[s32Looper]);
        }

        OptionsName.EndArray();
    }

    if (HI_SUCCESS != OSCSVR_Generate_InfoOptions(OptionsName, pstOSCOptions, pstOptionsTab))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions generate info fail!!\n");
        return HI_FAILURE;
    }

    switch (pstOSCOptions->enOptionMode)
    {
        case HI_OSC_OPTIONS_MODE_IMAGE:
            if (HI_SUCCESS != OSCSVR_Generate_ImageOptions(OptionsName, enLevel, pstOSCOptions, pstOptionsTab))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions generate image fail!!\n");
                return HI_FAILURE;
            }

            break;

        case HI_OSC_OPTIONS_MODE_INTERVAL:
            if (HI_SUCCESS != OSCSVR_Generate_IntervalOptions(OptionsName, pstOSCOptions, pstOptionsTab))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions generate image fail!!\n");
                return HI_FAILURE;
            }

            break;

        case HI_OSC_OPTIONS_MODE_INIT:
        default :
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions not contain image or interval!!!\n");
            break;
    }

    if (HI_SUCCESS != OSCSVR_Generate_FormatOptions(OptionsName, pstOSCOptions, pstOptionsTab))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions generate format fail!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_GetOptions_JsonPack(HI_OSC_SERVER_LEVEL_E enLevel, HI_S32 s32MaxLen, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_GET_OPTIONS);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("options");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/

    s32Ret = OSCSVR_Generate_GetOptions_Object(writer, enLevel, pstOSCOptions, pstOptionsTab);

    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions write the option json end!!\n");
        return HI_FAILURE;
    }

    writer.EndObject();
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "GetOptions the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ProcessPicture_JsonPack(HI_CHAR* pszCommandID, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_DOUBLE dCompletion = HI_OSC_COMMAND_STATUS_PROGRESS;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_PROCESS_PICTURE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_INPROGRESS);
    writer.Key("id");
    writer.String(pszCommandID);
    writer.Key("progress");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("completion");
    writer.Double(dCompletion);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "ProcessPicture the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_Reset_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_RESET);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Reset the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_SwitchWifi_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_SWITCH_WIFI);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "SwitchWifi the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_UploafFile_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_UPLOAD_FILE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Uploadfile the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_IntervalCapture_JsonPack(HI_CHAR* pszCommandID, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_DOUBLE dCompletion = HI_OSC_COMMAND_STATUS_PROGRESS;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_START_CAPTURE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_INPROGRESS);
    writer.Key("id");
    writer.String(pszCommandID);
    writer.Key("progress");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("completion");
    writer.Double(dCompletion);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StartCapture the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_StopCapture_JsonPack(HI_CHAR pszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Looper = 0;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_STOP_CAPTURE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("fileUrls");
    writer.StartArray();

    for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
    {
        if (0 != strlen(pszFileUrl[s32Looper]))
        {
            writer.String(pszFileUrl[s32Looper]);
        }
    }

    writer.EndArray();
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "StopCapture the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ListFile_JsonPack(HI_OSCSERVER_LIST_FILE_S* pstListFile, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Looper = 0;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pstListFile);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_LIST_FILE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("entries");
    writer.StartArray();

    for (s32Looper = 0; s32Looper < pstListFile->s32EntryCount; s32Looper ++)
    {
        if (0 != strlen(pstListFile->aszEntries[s32Looper].aszFileName))
        {
            writer.StartObject();                     /* Between StartObject()/EndObject()*/
            writer.Key("name");
            writer.String(pstListFile->aszEntries[s32Looper].aszFileName);
            writer.Key("fileUrl");
            writer.String(pstListFile->aszEntries[s32Looper].aszFileURL);
            writer.Key("size");
            writer.Int(pstListFile->aszEntries[s32Looper].s32FileSize);
            writer.Key("dateTimeZone");
            writer.String(pstListFile->aszEntries[s32Looper].aszDateTimeZone);
            writer.Key("lat");
            writer.Double(pstListFile->aszEntries[s32Looper].dLat);
            writer.Key("lng");
            writer.Double(pstListFile->aszEntries[s32Looper].dLng);
            writer.Key("width");
            writer.Int(pstListFile->aszEntries[s32Looper].s32Width);
            writer.Key("height");
            writer.Int(pstListFile->aszEntries[s32Looper].s32Height);

            if (pstListFile->bIncludeThumb)
            {
                //writer.Key("thumbnail");
                //writer.String(pstListFile->aszEntries[s32Looper].aszThumbnail);
            }

            writer.Key("isProcessed");
            writer.Bool(pstListFile->aszEntries[s32Looper].bIsProcessed);
            writer.Key("previewUrl");
            writer.String(pstListFile->aszEntries[s32Looper].aszPreviewUrl);
            writer.EndObject();
        }
    }

    writer.EndArray();
    writer.Key("totalEntries");
    writer.Int(pstListFile->s32TotalEntries);
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Listfiles the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_ListImage_JsonPack(HI_OSCSERVER_LIST_IMAGE_S* pstListImage, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Looper = 0;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pstListImage);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_LIST_IMAGE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("entries");
    writer.StartArray();

    for (s32Looper = 0; s32Looper < pstListImage->s32EntryCounts; s32Looper ++)
    {
        if (0 != strlen(pstListImage->aszEntries[s32Looper].aszFileURL))
        {
            writer.StartObject();
            writer.Key("name");
            writer.String(pstListImage->aszEntries[s32Looper].aszFileName);
            writer.Key("fileURL");
            writer.String(pstListImage->aszEntries[s32Looper].aszFileURL);
            writer.Key("size");
            writer.Int(pstListImage->aszEntries[s32Looper].s32FileSize);
            writer.Key("dateTimeZone");
            writer.String(pstListImage->aszEntries[s32Looper].aszDateTimeZone);
            writer.Key("lat");
            writer.Double(pstListImage->aszEntries[s32Looper].dLat);
            writer.Key("lng");
            writer.Double(pstListImage->aszEntries[s32Looper].dLng);
            writer.Key("width");
            writer.Int(pstListImage->aszEntries[s32Looper].s32Width);
            writer.Key("height");
            writer.Int(pstListImage->aszEntries[s32Looper].s32Height);

            if (pstListImage->bIncludeThummb)
            {
                writer.Key("thumbnail");
                writer.String(pstListImage->aszEntries[s32Looper].aszThumbnail);
            }

            writer.EndObject();
        }
    }

    writer.EndArray();
    writer.Key("totalEntries");
    writer.Int(pstListImage->s32TotalEntries);

    if (0 != strlen(pstListImage->aszOutContinuationToken))
    {
        writer.Key("continuationToken");
        writer.String(pstListImage->aszOutContinuationToken);
    }

    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Listimages the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_GetMetaData_JsonPack(HI_OSCSERVER_METADATA_S* pstMetaData, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pstMetaData);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(OSC_NAME_GET_METADATA);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);
    writer.Key("results");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    /*write exif*/
    writer.Key("exif");
    writer.StartObject();
    writer.Key("ExifVersion");
    writer.String(pstMetaData->stExif.aszExifVersion);
    writer.Key("ImageDescription");
    writer.String(pstMetaData->stExif.aszImageDescription);
    writer.Key("DateTime");
    writer.String(pstMetaData->stExif.aszDateTime);
    writer.Key("ImageWidth");
    writer.Int(pstMetaData->stExif.s32ImageWidth);
    writer.Key("ImageLength");
    writer.Int(pstMetaData->stExif.s32ImageLength);
    writer.Key("ColorSpace");
    writer.Int(pstMetaData->stExif.s32ColorSpace);
    writer.Key("Compression");
    writer.Int(pstMetaData->stExif.s32Compression);
    writer.Key("Orientation");
    writer.Int(pstMetaData->stExif.s32Orientation);
    writer.Key("Flash");
    writer.Int(pstMetaData->stExif.s32Flash);
    writer.Key("FocalLength");
    writer.Int(pstMetaData->stExif.s32FocalLength);
    writer.Key("WhiteBalance");
    writer.Int(pstMetaData->stExif.s32WhiteBalance);
    writer.Key("ExposureTime");
    writer.Int(pstMetaData->stExif.s32ExposureTime);
    writer.Key("FNumber");
    writer.Int(pstMetaData->stExif.s32FNumber);
    writer.Key("ExposureProgram");
    writer.Int(pstMetaData->stExif.s32ExposureProgram);
    writer.Key("ISOSpeedRatings");
    writer.Int(pstMetaData->stExif.s32ISOSpeedRatings);
    writer.Key("ShutterSpeedValue");
    writer.Int(pstMetaData->stExif.s32ShutterSpeedValue);
    writer.Key("ApertureValue");
    writer.Int(pstMetaData->stExif.s32ApertureValue);
    writer.Key("BrightnessValue");
    writer.Int(pstMetaData->stExif.s32BrightnessValue);
    writer.Key("ExposureBiasValue");
    writer.Int(pstMetaData->stExif.s32ExposureBiasValue);
    writer.Key("GPSProcessingMethod");
    writer.String(pstMetaData->stExif.aszGPSProcessingMethod);
    writer.Key("GPSLatitudeRef");
    writer.String(pstMetaData->stExif.aszGPSLatitudeRef);
    writer.Key("GPSLatitude");
    writer.Double(pstMetaData->stExif.dGPSLatitude);
    writer.Key("GPSLongitudeRef");
    writer.String(pstMetaData->stExif.aszGPSLongitudeRef);
    writer.Key("GPSLongitude");
    writer.Double(pstMetaData->stExif.dGPSLongitude);
    writer.Key("Make");
    writer.String(pstMetaData->stExif.aszMake);
    writer.Key("Model");
    writer.String(pstMetaData->stExif.aszModel);
    writer.Key("Software");
    writer.String(pstMetaData->stExif.aszSoftware);
    writer.Key("Copyright");
    writer.String(pstMetaData->stExif.aszCopyright);
    writer.Key("MakerNote");
    writer.String(pstMetaData->stExif.aszMakerNote);
    writer.Key("ImageUniqueID");
    writer.String(pstMetaData->stExif.aszImageUniqueID);
    writer.EndObject();
    /*write xmp*/
    writer.Key("xmp");
    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("ProjectionType");
    writer.String(pstMetaData->stXMP.aszProjectionType);
    writer.Key("UsePanoramaViewer");
    writer.Bool(pstMetaData->stXMP.bUsePanoramaViewer);
    writer.Key("PoseHeadingDegrees");
    writer.Double(pstMetaData->stXMP.dPoseHeadingDegrees);
    writer.Key("CroppedAreaImageWidthPixels");
    writer.Int(pstMetaData->stXMP.s32CroppedAreaImageWidthPixels);
    writer.Key("CroppedAreaImageHeightPixels");
    writer.Int(pstMetaData->stXMP.s32CroppedAreaImageHeightPixels);
    writer.Key("FullPanoWidthPixels");
    writer.Int(pstMetaData->stXMP.s32FullPanoWidthPixels);
    writer.Key("FullPanoHeightPixels");
    writer.Int(pstMetaData->stXMP.s32FullPanoHeightPixels);
    writer.Key("CroppedAreaLeftPixels");
    writer.Int(pstMetaData->stXMP.s32CroppedAreaLeftPixels);
    writer.Key("CroppedAreaTopPixels");
    writer.Int(pstMetaData->stXMP.s32CroppedAreaTopPixels);
    writer.EndObject();
    writer.EndObject();
    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Getmetadata the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_Delete_JsonPack(HI_OSC_SERVER_LEVEL_E enLevel, HI_S32 s32MaxLen, HI_CHAR aszReultsUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Looper = 0;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);

    writer.StartObject();
    writer.Key("name");
    writer.String(OSC_NAME_DELETE);
    writer.Key("state");
    writer.String(OSC_COMM_STATE_DONE);

    if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        if (0 != strlen(aszReultsUrl[0]))
        {
            writer.Key("results");
            writer.StartObject();
            writer.Key("fileUrls");
            writer.StartArray();

            for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
            {
                if (0 != strlen(aszReultsUrl[s32Looper]))
                {
                    writer.String(aszReultsUrl[s32Looper]);
                }
            }

            writer.EndArray();
            writer.EndObject();
        }
    }

    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Delete the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_CommandStatus_JsonPack(HI_CHAR* pszCommandName, HI_S32 s32MaxLen, HI_CHAR* pszCommandId, HI_OSC_SERVER_LEVEL_E enCliVersion, HI_OSC_COMM_STATUS_E enComStatus, HI_DOUBLE dCompletion, HI_CHAR pszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen)
{
    HI_S32 s32Looper = 0;
    StringBuffer JsonBuffer;
    Writer<StringBuffer> writer(JsonBuffer);

    OSCSVR_CHECK_NULL_ERROR(pszCommandName);
    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(ps32JsonLen);
    OSCSVR_CHECK_NULL_ERROR(pszResults);

    writer.StartObject();                     /* Between StartObject()/EndObject()*/
    writer.Key("name");
    writer.String(pszCommandName);

    if (HIOSC_COMM_STATUS_DONE == enComStatus)
    {
        writer.Key("state");
        writer.String(OSC_COMM_STATE_DONE);
        writer.Key("results");
        writer.StartObject();

        if (0 == strncmp(pszCommandName, OSC_NAME_PROCESS_PICTURE, strlen(OSC_NAME_PROCESS_PICTURE)))
        {
            writer.Key("previewToFinalFileUrls");
            writer.StartArray();

            for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++ )
            {
                if (0 != strlen(pszResults[s32Looper]))
                {
                    writer.String(pszResults[s32Looper]);
                }
            }

            writer.EndArray();
        }
        else if (0 == strncmp(pszCommandName, OSC_NAME_START_CAPTURE, strlen(OSC_NAME_START_CAPTURE)))
        {
            writer.Key("fileUrls");
            writer.StartArray();

            for (s32Looper = 0; s32Looper < HI_OSC_FILEURL_MAX_NUM; s32Looper ++)
            {
                if (0 != strlen(pszResults[s32Looper]) )
                {
                    writer.String(pszResults[s32Looper]);
                }
            }

            writer.EndArray();
        }
        else
        {
            if (enCliVersion == HI_OSC_SERVER_LEVEL_1)
            {
                writer.Key("fileUri");
                writer.String(pszResults[0]);
            }
            else if (enCliVersion == HI_OSC_SERVER_LEVEL_2)
            {
                writer.Key("fileUrl");
                writer.String(pszResults[0]);
            }
            else
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status the input clientversion is err!!\n");
            }
        }

        writer.EndObject();
    }
    else if (HIOSC_COMM_STATUS_INPROGRESS == enComStatus)
    {
        writer.Key("state");
        writer.String(OSC_COMM_STATE_INPROGRESS);
        writer.Key("id");
        writer.String(pszCommandId);
        writer.Key("progress");
        writer.StartObject();
        writer.Key("completion");
        writer.Double(dCompletion);
        writer.EndObject();
    }
    else if (HIOSC_COMM_STATUS_ERROR == enComStatus)
    {
        writer.Key("state");
        writer.String(OSC_COMM_STATE_ERROR);
        writer.Key("error");
        writer.StartObject();
        writer.Key("code");
        writer.String(pszResults[0]);
        writer.EndObject();
    }

    writer.EndObject();

    if (s32MaxLen <= (HI_S32)JsonBuffer.GetSize())
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status the buff is too small to ache the json pack!!\n");
        return HI_FAILURE;
    }

    strncpy_s(pszJsonPack, s32MaxLen, JsonBuffer.GetString(), JsonBuffer.GetSize());
    pszJsonPack[JsonBuffer.GetSize()] = '\0';
    *ps32JsonLen = JsonBuffer.GetSize();
    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "/osc/commands/status the buff is %s\n", pszJsonPack);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
