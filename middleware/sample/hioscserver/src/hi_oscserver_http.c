/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_http.c
* @brief     oscserver http src file
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
extern HI_BOOL g_bExit;

static HI_OSC_HTTP_STATUS_S s_astOSCHttpStatus[] =
{
    {"Continue", 100},
    {"OK", 200},
    {"Created", 201},
    {"Accepted", 202},
    {"Non-Authoritative Information", 203},
    {"No Content", 204},
    {"Reset Content", 205},
    {"Partial Content", 206},
    {"Multiple Choices", 300},
    {"Moved Permanently", 301},
    {"Moved Temporarily", 302},
    {"Bad Request", 400},
    {"Unauthorized", 401},
    {"Payment Required", 402},
    {"Forbidden", 403},
    {"Not Found", 404},
    {"Method Not Allowed", 405},
    {"Not Acceptable", 406},
    {"Proxy Authentication Required", 407},
    {"Request Time-out", 408},
    {"Conflict", 409},
    {"Gone", 410},
    {"Length Required", 411},
    {"Precondition Failed", 412},
    {"Request Entity Too Large", 413},
    {"Request-URI Too Large", 414},
    {"Unsupported Media Type", 415},
    {"Over Supported connection ", 416},
    {"Bad Extension", 420},
    {"Invalid Parameter", 450},
    {"Parameter Not Understood", 451},
    {"Conference Not Found", 452},
    {"Not Enough Bandwidth", 453},
    {"Session Not Found", 454},
    {"Method Not Valid In This State", 455},
    {"Header Field Not Valid for Resource", 456},
    {"Invalid Range", 457},
    {"Parameter Is Read-Only", 458},
    {"Internal Server Error", 500},
    {"Not Implemented", 501},
    {"Bad Gateway", 502},
    {"Service Unavailable", 503},
    {"Gateway Time-out", 504},
    {"HTTP Version Not Supported", 505},
    {"Extended Error:", 911},
    {{0}, OSC_HTTP_INVALID_STATUS_CODE}
};

static HI_OSC_COMMAND_S s_astOSCCommand[] =
{
    {OSC_NAME_START_SESSION, HI_OSC_COMMAND_START_SESSION},
    {OSC_NAME_CLOSE_SESSION, HI_OSC_COMMAND_CLOSE_SESSION},
    {OSC_NAME_UPDATE_SESSION, HI_OSC_COMMAND_UPDATE_SESSION},
    {OSC_NAME_TAKE_PICTURE, HI_OSC_COMMAND_TAKE_PICTURE},
    {OSC_NAME_SET_OPTIONS, HI_OSC_COMMAND_SET_OPTIONS},
    {OSC_NAME_GET_OPTIONS, HI_OSC_COMMAND_GET_OPTIONS},
    {OSC_NAME_PROCESS_PICTURE, HI_OSC_COMMAND_PROCESS_PICTURE},
    {OSC_NAME_GET_IMAGE, HI_OSC_COMMAND_GET_IMAGE},
    {OSC_NAME_RESET, HI_OSC_COMMAND_RESET},
    {OSC_NAME_SWITCH_WIFI, HI_OSC_COMMAND_SWITCH_WIFI},
    {OSC_NAME_UPLOAD_FILE, HI_OSC_COMMAND_UPLOAD_FILE},
    {OSC_NAME_START_CAPTURE, HI_OSC_COMMAND_START_CAPTURE},
    {OSC_NAME_STOP_CAPTURE, HI_OSC_COMMAND_STOP_CAPTURE},
    {OSC_NAME_LIST_FILE, HI_OSC_COMMAND_LIST_FILE},
    {OSC_NAME_LIST_IMAGE, HI_OSC_COMMAND_LIST_IMAGE},
    {OSC_NAME_GET_METADATA, HI_OSC_COMMAND_GET_METADATA},
    {OSC_NAME_GET_LIVEPREVIEW, HI_OSC_COMMAND_GET_PRELIVE_VIEW},
    {OSC_NAME_DELETE, HI_OSC_COMMAND_DELETE},
    {OSC_NAME_PRIVATE_COMMAND, HI_OSC_COMMAND_PRIVATE},
    {{0}, OSC_HTTP_INVALID_STATUS_CODE}
};

static HI_OSC_OPTIONS_S s_stImageOptions[] =
{
    {"exposureProgram", HI_FALSE},                /*1*/
    {"exposureProgramSupport", HI_FALSE},         /*2*/
    {"iso", HI_FALSE},                             /*3*/
    {"isoSupport", HI_FALSE},                     /*4*/
    {"shutterSpeed", HI_FALSE},                   /*5*/
    {"shutterSpeedSupport", HI_FALSE},            /*6*/
    {"aperture", HI_FALSE},                        /*7*/
    {"apertureSupport", HI_FALSE},                /*8*/
    {"whiteBalance", HI_FALSE},                   /*9*/
    {"whiteBalanceSupport", HI_FALSE},            /*10*/
    {"exposureCompensation", HI_FALSE},           /*11*/
    {"exposureCompensationSupport", HI_FALSE},    /*12*/
    {"exposureDelay", HI_FALSE},                  /*13*/
    {"exposureDelaySupport", HI_FALSE},           /*14*/
    {"hdr", HI_FALSE},                             /*15*/
    {"hdrSupport", HI_FALSE},                     /*16*/
    {"imageStabilization", HI_FALSE},             /*17*/
    {"imageStabilizationSupport", HI_FALSE},      /*18*/
    {"photoStitching", HI_FALSE},                 /*19*/
    {"photoStitchingSupport", HI_FALSE},          /*20*/
    {"exposureBracket", HI_FALSE},                /*21*/
    {"exposureBracketSupport", HI_FALSE},          /*22*/
    {"captureInterval", HI_FALSE},                /*23*/
    {"captureIntervalSupport", HI_FALSE},         /*24*/
    {"captureNumber", HI_FALSE},                  /*25*/
    {"captureNumberSupport", HI_FALSE}            /*26*/
};

static HI_OSC_OPTIONS_S s_stIntervalOptions[] =
{
    {"remainingVideoSeconds", HI_FALSE},         /*1*/
    {"videoStitching", HI_FALSE},                /*2*/
    {"videoStitchingSupport", HI_FALSE},         /*3*/
    {"videoGPS", HI_FALSE},                       /*4*/
    {"videoGPSSupport", HI_FALSE},               /*5*/
};

static HI_OSC_OPTIONS_S s_stFormatOptions[] =
{
    {"fileFormat", HI_FALSE},                    /*1*/
    {"fileFormatSupport", HI_FALSE},             /*2*/
    {"previewFormat", HI_FALSE},                 /*3*/
    {"previewFormatSupport", HI_FALSE}           /*4*/
};

static HI_OSC_OPTIONS_S s_stInfoOptions[] =
{
    {"sleepDelay", HI_FALSE},                    /*1*/
    {"sleepDelaySupport", HI_FALSE},             /*2*/
    {"offDelay", HI_FALSE},                      /*3*/
    {"offDelaySupport", HI_FALSE},               /*4*/
    {"totalSpace", HI_FALSE},                    /*5*/
    {"remainingSpace", HI_FALSE},                /*6*/
    {"remainingPictures", HI_FALSE},             /*7*/
    {"dateTimeZone", HI_FALSE},                  /*8*/
    {"gyro", HI_FALSE},                           /*9*/
    {"gyroSupport", HI_FALSE},                   /*10*/
    {"gps", HI_FALSE},                            /*11*/
    {"gpsSupport", HI_FALSE},                    /*12*/
    {"delayProcessing", HI_FALSE},               /*13*/
    {"delayProcessingSupport", HI_FALSE},        /*14*/
    {"pollingDelay", HI_FALSE},                  /*15*/
    {"clientVersion", HI_FALSE},                 /*16*/
    {"wifiPassword", HI_FALSE},                  /*17*/
    {"gpsInfo", HI_FALSE}                        /*18*/
};

HI_S32 HI_OSCSVR_Create_ImageTable(HI_OSC_OPTIONS_S* pstImageOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSC_SERVER_LEVEL_E enLevel)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstImageOptions);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_IMAGE_MAX_NUM; s32Looper ++)
    {
        pstImageOptions[s32Looper].bOptionsFlag = s_stImageOptions[s32Looper].bOptionsFlag;
        strncpy_s(pstImageOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, s_stImageOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN);
        pstImageOptions[s32Looper].aszOptions[strlen(s_stImageOptions[s32Looper].aszOptions)] = '\0';
    }

    pstImageOptions[0].bOptionsFlag = pstOptionsTab->stImageTable.bExposureProgram;
    pstImageOptions[1].bOptionsFlag = pstOptionsTab->stImageTable.bExposureProgramSupport;
    pstImageOptions[2].bOptionsFlag = pstOptionsTab->stImageTable.bISO;
    pstImageOptions[3].bOptionsFlag = pstOptionsTab->stImageTable.bISOSupport;
    pstImageOptions[4].bOptionsFlag = pstOptionsTab->stImageTable.bShutterSpeed;
    pstImageOptions[5].bOptionsFlag = pstOptionsTab->stImageTable.bShutterSpeedSupport;
    pstImageOptions[6].bOptionsFlag = pstOptionsTab->stImageTable.bAperture;
    pstImageOptions[7].bOptionsFlag = pstOptionsTab->stImageTable.bApertureSupport;
    pstImageOptions[8].bOptionsFlag = pstOptionsTab->stImageTable.bWhiteBalance;
    pstImageOptions[9].bOptionsFlag = pstOptionsTab->stImageTable.bWhiteBalanceSupport;
    pstImageOptions[10].bOptionsFlag = pstOptionsTab->stImageTable.bExposureCompensation;
    pstImageOptions[11].bOptionsFlag = pstOptionsTab->stImageTable.bExposureCompensationSupport;
    pstImageOptions[12].bOptionsFlag = pstOptionsTab->stImageTable.bExposureDelay;
    pstImageOptions[13].bOptionsFlag = pstOptionsTab->stImageTable.bExposureDelaySupport;

    if (HI_OSC_SERVER_LEVEL_1 == enLevel)
    {
        pstImageOptions[14].bOptionsFlag = pstOptionsTab->stImageTable.bLevel1hdr;
        pstImageOptions[15].bOptionsFlag = pstOptionsTab->stImageTable.bLevel1hdrSupport;
    }
    else if (HI_OSC_SERVER_LEVEL_2 == enLevel)
    {
        pstImageOptions[14].bOptionsFlag = pstOptionsTab->stImageTable.bLevel2Hdr;
        pstImageOptions[15].bOptionsFlag = pstOptionsTab->stImageTable.bLevel2HdrSupport;
    }

    pstImageOptions[16].bOptionsFlag = pstOptionsTab->stImageTable.bImageStabilization;
    pstImageOptions[17].bOptionsFlag = pstOptionsTab->stImageTable.bImageStabilizationSupport;
    pstImageOptions[18].bOptionsFlag = pstOptionsTab->stImageTable.bPhotoStitching;
    pstImageOptions[19].bOptionsFlag = pstOptionsTab->stImageTable.bPhotoStitchingSupport;
    pstImageOptions[20].bOptionsFlag = pstOptionsTab->stImageTable.bExposureBracket;
    pstImageOptions[21].bOptionsFlag = pstOptionsTab->stImageTable.bExposureBracketSupport;
    pstImageOptions[22].bOptionsFlag = pstOptionsTab->stImageTable.bCaptureInterval;
    pstImageOptions[23].bOptionsFlag = pstOptionsTab->stImageTable.bCaptureIntervalSupport;
    pstImageOptions[24].bOptionsFlag = pstOptionsTab->stImageTable.bCaptureNumber;
    pstImageOptions[25].bOptionsFlag = pstOptionsTab->stImageTable.bCaptureNumberSupport;

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Create_IntervalTable(HI_OSC_OPTIONS_S* pstIntervalOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstIntervalOptions);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_INTERVAL_MAX_NUM; s32Looper ++)
    {
        pstIntervalOptions[s32Looper].bOptionsFlag = s_stIntervalOptions[s32Looper].bOptionsFlag;
        strncpy_s(pstIntervalOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, s_stIntervalOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN);
        pstIntervalOptions[s32Looper].aszOptions[strlen(s_stIntervalOptions[s32Looper].aszOptions)] = '\0';
    }

    pstIntervalOptions[0].bOptionsFlag = pstOptionsTab->stVideoTable.bRemainingVideoSeconds;
    pstIntervalOptions[1].bOptionsFlag = pstOptionsTab->stVideoTable.bVideoStitching;
    pstIntervalOptions[2].bOptionsFlag = pstOptionsTab->stVideoTable.bVideoStitchingSupport;
    pstIntervalOptions[3].bOptionsFlag = pstOptionsTab->stVideoTable.bVideoGPS;
    pstIntervalOptions[4].bOptionsFlag = pstOptionsTab->stVideoTable.bVideoGPSSupport;

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Create_FormatTable(HI_OSC_OPTIONS_S* pstFormatOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstFormatOptions);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_FORMAT_MAX_NUM; s32Looper ++)
    {
        pstFormatOptions[s32Looper].bOptionsFlag = s_stFormatOptions[s32Looper].bOptionsFlag;
        strncpy_s(pstFormatOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, s_stFormatOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN);
        pstFormatOptions[s32Looper].aszOptions[strlen(s_stFormatOptions[s32Looper].aszOptions)] = '\0';
    }

    pstFormatOptions[0].bOptionsFlag = pstOptionsTab->stFormatTable.bFileFormat;
    pstFormatOptions[1].bOptionsFlag = pstOptionsTab->stFormatTable.bFileFormatSupport;
    pstFormatOptions[2].bOptionsFlag = pstOptionsTab->stFormatTable.bPreviewFormat;
    pstFormatOptions[3].bOptionsFlag = pstOptionsTab->stFormatTable.bPreviewFormatSupport;

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Create_InfoTable(HI_OSC_OPTIONS_S* pstInfoOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pstInfoOptions);

    for (s32Looper = 0; s32Looper < HI_OSC_OPTIONS_INFO_MAX_NUM; s32Looper ++)
    {
        pstInfoOptions[s32Looper].bOptionsFlag = s_stInfoOptions[s32Looper].bOptionsFlag;
        strncpy_s(pstInfoOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN, s_stInfoOptions[s32Looper].aszOptions, HI_OSC_OPTIONS_NAME_MAX_LEN);
        pstInfoOptions[s32Looper].aszOptions[strlen(s_stInfoOptions[s32Looper].aszOptions)] = '\0';
    }

    pstInfoOptions[0].bOptionsFlag = pstOptionsTab->stInfoTable.bSleepDelay;
    pstInfoOptions[1].bOptionsFlag = pstOptionsTab->stInfoTable.bSleepDelaySupport;
    pstInfoOptions[2].bOptionsFlag = pstOptionsTab->stInfoTable.bOFFDelay;
    pstInfoOptions[3].bOptionsFlag = pstOptionsTab->stInfoTable.bOFFDelaySupport;
    pstInfoOptions[4].bOptionsFlag = pstOptionsTab->stInfoTable.bTotalSpace;
    pstInfoOptions[5].bOptionsFlag = pstOptionsTab->stInfoTable.bRemainingSpace;
    pstInfoOptions[6].bOptionsFlag = pstOptionsTab->stInfoTable.bRemainingPictures;
    pstInfoOptions[7].bOptionsFlag = pstOptionsTab->stInfoTable.bDateTimeZone;
    pstInfoOptions[8].bOptionsFlag = pstOptionsTab->stInfoTable.bGyro;
    pstInfoOptions[9].bOptionsFlag = pstOptionsTab->stInfoTable.bGyroSupport;
    pstInfoOptions[10].bOptionsFlag = pstOptionsTab->stInfoTable.bGps;
    pstInfoOptions[11].bOptionsFlag = pstOptionsTab->stInfoTable.bGpsSupport;
    pstInfoOptions[12].bOptionsFlag = pstOptionsTab->stInfoTable.bDelayProcessing;
    pstInfoOptions[13].bOptionsFlag = pstOptionsTab->stInfoTable.bDelayProcessingSupport;
    pstInfoOptions[14].bOptionsFlag = pstOptionsTab->stInfoTable.bPollingDelay;
    pstInfoOptions[15].bOptionsFlag = pstOptionsTab->stInfoTable.bClientVersion;
    pstInfoOptions[16].bOptionsFlag = pstOptionsTab->stInfoTable.bWifiPassword;
    pstInfoOptions[17].bOptionsFlag = pstOptionsTab->stInfoTable.bGPSInfo;

    return HI_SUCCESS;
}

/*ex.
    GET /osc/info HTTP/1.1
    User-Agent: Dalvik/1.6.0 (Linux; U; Android 4.2.2; L39u Build/14.1.N.0.63)
    Host: 192.168.1.1
    Connection: Keep-Alive
    Accept-Encoding: gzip
*/
HI_BOOL HI_OSCSVR_CheckVaildMsg(const HI_CHAR* pszReq, HI_U32 u32Reqlen)
{
    HI_S32  s32Cnt = 0;
    HI_CHAR aszVersion[OSC_VER_MAX_LEN] = {0};
    HI_CHAR aszUrl[OSC_URL_MAX_LEN] = {0};
    HI_CHAR aszMethod[OSC_METHOD_MAX_LEN] = {0};

    if (NULL == pszReq)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC input param is null\n");
        return HI_FALSE;
    }

    if (u32Reqlen > strlen(pszReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC input Len is invaild\n");
        return HI_FALSE;
    }

    s32Cnt = sscanf(pszReq, "%31s %63s  %15s", aszMethod, aszUrl, aszVersion);

    if (OSC_SSCANF_ASSIGN_NUMBER != s32Cnt)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC sscanf err, %d, %s, %s, %s\n", s32Cnt, aszMethod, aszUrl, aszVersion);
        return HI_FALSE;
    }

    aszVersion[OSC_VER_MAX_LEN - 1] = '\0';
    aszUrl[OSC_URL_MAX_LEN - 1] = '\0';
    aszMethod[OSC_METHOD_MAX_LEN - 1] = '\0';

    /*check http req request*/
    if ((0 != strncmp(aszMethod, HI_OSC_HTTP_REQ_GET_METHOD, strlen(HI_OSC_HTTP_REQ_GET_METHOD))) && (0 != strncmp(aszMethod, HI_OSC_HTTP_REQ_POST_METHOD, strlen(HI_OSC_HTTP_REQ_POST_METHOD))))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "no Get or no POST, unsupport http req method\n");
        return HI_FALSE;
    }

    if (NULL == strstr(aszUrl, "/osc"))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the request no belong to OSC module!!\n");
        return HI_FALSE;
    }

    return HI_TRUE;
}
HI_S32 OSCSVR_Get_FileCommandApi(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey, HI_VOID* pfnEventCB)
{
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pfnEventCB);

    switch (enOSCCommandKey)
    {
        case HI_OSC_COMMAND_LIST_FILE:
            pstServerCtx->stRegCommandCB.pfnListFiles = (HI_OSC_LISTFILE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_LIST_IMAGE:
            pstServerCtx->stRegCommandCB.pfnListImage = (HI_OSC_LISTIMAGE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_DELETE:
            pstServerCtx->stRegCommandCB.pfnDelete = (HI_OSC_DELETE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_RESET:
            pstServerCtx->stRegCommandCB.pfnReset = (HI_OSC_RESET_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_SWITCH_WIFI:
            pstServerCtx->stRegCommandCB.pfnSwitchWifi = (HI_OSC_SWITCHWIFI_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_UPLOAD_FILE:
            pstServerCtx->stRegCommandCB.pfnUploadFile = (HI_OSC_UPLOADFILE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_GET_METADATA:
            pstServerCtx->stRegCommandCB.pfnGetMetaData = (HI_OSC_GETMETADATA_FN_PTR)pfnEventCB;
            break;

        default:
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Input key not match the RegCommand\n");
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}
HI_S32 OSCSVR_Get_CommandApi(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey, HI_VOID* pfnEventCB)
{
    OSCSVR_CHECK_NULL_ERROR(pstServerCtx);
    OSCSVR_CHECK_NULL_ERROR(pfnEventCB);

    switch (enOSCCommandKey)
    {
        case HI_OSC_COMMAND_INFO:
            pstServerCtx->stRegApiCB.pfnOSCSVRInfoCB = (HI_OSC_INFO_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_STATE:
            pstServerCtx->stRegApiCB.pfnOSCSVRStateCB = (HI_OSC_STATE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_TAKE_PICTURE:
            pstServerCtx->stRegCommandCB.pfnTakePicture = (HI_OSC_TAKEPICTURE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_GET_OPTIONS:
            pstServerCtx->stRegCommandCB.pfnGetOption = (HI_OSC_GETOPTION_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_SET_OPTIONS:
            pstServerCtx->stRegCommandCB.pfnSetOption = (HI_OSC_SETOPTION_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_PROCESS_PICTURE:
            pstServerCtx->stRegCommandCB.pfnProcessPitcure = (HI_OSC_PROCESSPICTURE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_START_CAPTURE:
            pstServerCtx->stRegCommandCB.pfnStartCapture = (HI_OSC_STARTCAPTURE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_STOP_CAPTURE:
            pstServerCtx->stRegCommandCB.pfnStopCapture = (HI_OSC_STOPCAPTURE_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_GET_PRELIVE_VIEW:
            pstServerCtx->stRegCommandCB.pfnGetLivePreview = (HI_OSC_GETLIVEPREVIEW_FN_PTR)pfnEventCB;
            break;

        case HI_OSC_COMMAND_PRIVATE:
            pstServerCtx->stRegCommandCB.pfnPrivateCom = (HI_OSC_PRIVATECOM_FN_PTR)pfnEventCB;
            break;

        default:
            if (HI_SUCCESS !=  OSCSVR_Get_FileCommandApi(pstServerCtx, enOSCCommandKey, pfnEventCB))
            {
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC input key is err!!!\n");
                return HI_FAILURE;
            }
    }

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_MSGParser_ParseAPI(const HI_CHAR* pszReq, HI_U32 u32ReqLen, HI_OSC_REQ_API_E* peMethod)
{
    HI_S32  s32Cnt = 0;
    HI_CHAR aszVersion[OSC_VER_MAX_LEN] = {0};
    HI_CHAR aszUrl[OSC_URL_MAX_LEN] = {0};
    HI_CHAR aszMethod[OSC_METHOD_MAX_LEN] = {0};

    if (NULL == pszReq)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC input param is NULL\n");
        return HI_FAILURE;
    }

    if (u32ReqLen > strlen(pszReq))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC input Len is invaild\n");
        return HI_FAILURE;
    }

    s32Cnt = sscanf(pszReq, "%31s %63s %15s", aszMethod, aszUrl, aszVersion);

    if (OSC_SSCANF_ASSIGN_NUMBER != s32Cnt)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC ParseAPI err\n");
        return HI_FALSE;
    }

    aszVersion[OSC_VER_MAX_LEN - 1] = '\0';
    aszUrl[OSC_URL_MAX_LEN - 1] = '\0';
    aszMethod[OSC_METHOD_MAX_LEN - 1] = '\0';

    if (0 == strncmp(aszUrl, OSC_INFO_API, strlen(OSC_INFO_API)))
    {
        *peMethod = HI_OSC_INFO_API;
    }
    else if (0 == strncmp(aszUrl, OSC_STATE_API, strlen(OSC_STATE_API)))
    {
        *peMethod = HI_OSC_STATE_API;
    }
    else if (0 == strncmp(aszUrl, OSC_CHECK_FORUPDATES_API, strlen(OSC_CHECK_FORUPDATES_API)))
    {
        *peMethod = HI_OSC_CHECK_FORUPDATES_API;
    }
    else if (0 == strncmp(aszUrl, OSC_COMMAND_EXECUTE_API, strlen(OSC_COMMAND_EXECUTE_API)))
    {
        *peMethod = HI_OSC_COMMAND_EXECUTE_API;
    }
    else if (0 == strncmp(aszUrl, OSC_COMMAND_STATUS_API, strlen(OSC_COMMAND_STATUS_API)))
    {
        *peMethod = HI_OSC_COMMAND_STATUS_API;
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "the url contains an invaild API!!\n");
        *peMethod = HI_OSC_INVAILD_API;

        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_MSGParser_ParseComm(const HI_CHAR* pszCommName, HI_OSC_API_COMMAND_E* penComAPI)
{
    HI_S32 s32Looper = 0;

    OSCSVR_CHECK_NULL_ERROR(pszCommName);
    OSCSVR_CHECK_NULL_ERROR(penComAPI);

    *penComAPI = HI_OSC_COMMAND_INVAILD;

    if ( NULL != strstr(pszCommName, OSC_NAME_PRIVATE_COMMAND) )
    {
        *penComAPI = HI_OSC_COMMAND_PRIVATE;
    }
    else
    {
        for (s32Looper = 0; s32Looper < HI_OSC_SUPPORT_COMMAND_NUM; s32Looper ++)
        {
            if (0 == strncmp(s_astOSCCommand[s32Looper].pszCommand, pszCommName, strlen(pszCommName)))
            {
                *penComAPI = s_astOSCCommand[s32Looper].enCommandCode;
                break;
            }
        }
    }

    if (*penComAPI == HI_OSC_COMMAND_INVAILD)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSC not support the command!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_BOOL HI_OSCSVR_MSGParser_CheckRequest(const HI_CHAR* pszReq, HI_U32 u32ReqLen, HI_OSC_REQ_API_E* penMethod)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* check for request message. */
    if (!HI_OSCSVR_CheckVaildMsg(pszReq, u32ReqLen))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "req msg formate error\n");
        return HI_FALSE;
    }

    /* not a response message, check for method request. */
    s32Ret = HI_OSCSVR_MSGParser_ParseAPI(pszReq, u32ReqLen, penMethod);

    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Method requested was invalid.  Message discarded. Method = %s\n", pszReq);
        return HI_FALSE;
    }

    return HI_TRUE;
}

HI_CHAR* HI_OSCSVR_MSGParser_StatusCode2Str(HI_S32 s32Code)
{
    HI_OSC_HTTP_STATUS_S*  pstStatus = NULL;

    for (pstStatus = s_astOSCHttpStatus; pstStatus->s32StatCode != OSC_HTTP_INVALID_STATUS_CODE; pstStatus++ )
    {
        if ( pstStatus->s32StatCode == s32Code )
        {
            return pstStatus->pszStatStr;
        }
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "Invalid status code.%d .\n", s32Code);

    return OSC_HTTP_INVALID_STATUS;
}

HI_S32 HI_OSCSVR_MSGParser_GetServerVersion(HI_CHAR* pszSvrVersion, HI_U32 u32VersionLen)
{
    if (NULL == pszSvrVersion || 0 == u32VersionLen)
    {
        return HI_FAILURE;
    }

    snprintf_s(pszSvrVersion, u32VersionLen, u32VersionLen - 1, "%s/1.0.0(%s)", OSC_HTTP_SERVER_DESCRIPTION, __DATE__);
    pszSvrVersion[u32VersionLen - 1] = '\0';

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_MSGParser_ConnectFailResponse(HI_CHAR* pszReply, HI_S32 s32StateCode, HI_S32 s32MaxLen)
{
    HI_S32  s32Temp = 0;
    HI_S32  s32ReplyLen = s32MaxLen;
    HI_CHAR au8SvrVersion[OSC_VER_MAX_LEN] = {0};

    s32Temp += snprintf_s(pszReply, s32ReplyLen, s32ReplyLen, "%s %d %s\r\n", OSC_HTTP_VER_STR, s32StateCode, HI_OSCSVR_MSGParser_StatusCode2Str(s32StateCode));
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "Cache-Control: no-cache\r\n");
    HI_OSCSVR_MSGParser_GetServerVersion(au8SvrVersion, OSC_VER_MAX_LEN);
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "%s: 0\r\n", OSC_HTTP_HEADER_CONTENTLENGTH);
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "%s: nosniff\r\n", OSC_HTTP_HEADER_XSSOPTION);
    /*Connection: close*/
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "Connection: close\r\n");
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "Server: %s\r\n", au8SvrVersion);
    s32Temp += snprintf_s(pszReply + s32Temp, s32ReplyLen - s32Temp, s32ReplyLen - s32Temp, "\r\n");

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_Response_Send(HI_SOCKET s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32RemSize = 0;
    HI_S32 s32Size    = 0;
    HI_S32 s32Errno = 0;
    fd_set write_fds;
    struct timeval TimeoutVal;  /* Timeout value */
    HI_CHAR*  ps8BufferPos = NULL;

    memset_s(&TimeoutVal, sizeof(struct timeval), 0, sizeof(struct timeval));
    u32RemSize = u32DataLen;
    ps8BufferPos = pszBuff;

    while (u32RemSize > 0)
    {
        FD_ZERO(&write_fds);
        FD_SET(s32WritSock, &write_fds);
        TimeoutVal.tv_sec = OSCSVR_HTTP_TRANS_TIMEVAL_SEC;
        TimeoutVal.tv_usec = OSCSVR_HTTP_TRANS_TIMEVAL_USEC;
        /*judge if it can send */
        s32Ret = select(s32WritSock + 1, NULL, &write_fds, NULL, &TimeoutVal);

        if (s32Ret > 0)
        {
            if (FD_ISSET(s32WritSock, &write_fds))
            {
                s32Size = send(s32WritSock, ps8BufferPos, u32RemSize, MSG_DONTWAIT);

                if (s32Size < 0)
                {
                    /*if it is not eagain error, means can not send*/
                    if (errno != EINTR && errno != EAGAIN)
                    {
                        s32Errno = errno;
                        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR response Send error, for:%s\n", strerror(s32Errno));
                        return HI_ERR_OSCSERVER_ERR_SESS_SEND_FAIL;
                    }

                    /*it is eagain error, means can try again*/
                    continue;
                }

                u32RemSize -= s32Size;
                ps8BufferPos += s32Size;

                if (u32RemSize <= 0)
                {
                    break;
                }
            }
            else
            {
                s32Errno = errno;
                HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR response Send error:%s fd not in fd_set\n", strerror(s32Errno));
                return HI_ERR_OSCSERVER_ERR_SESS_SEND_FAIL;
            }
        }
        /*select found over time or error happend*/
        else if (s32Ret == 0)
        {
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR response Send error:%s  select overtime %d.%ds\n", strerror(s32Errno),
                                 OSCSVR_HTTP_TRANS_TIMEVAL_SEC, OSCSVR_HTTP_TRANS_TIMEVAL_USEC);
            return HI_ERR_OSCSERVER_ERR_SESS_SEND_FAIL;
        }
        else if (s32Ret < 0)
        {
            s32Errno = errno;
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR response Send error:%X\n", strerror(s32Errno));
            return HI_ERR_OSCSERVER_ERR_SESS_SEND_FAIL;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_OSCSVR_CloseSocket(HI_S32* ps32Socket)
{
    if (*ps32Socket >= 0)
    {
        close(*ps32Socket);
        *ps32Socket = HI_OSC_INVALID_SOCKET;
        return HI_SUCCESS;
    }

    return HI_FAILURE;
}

HI_S32 OSCSVR_Generate_HTTP_Response(HI_CHAR* pszJsonPack, HI_S32 s32JsonLen, HI_S32 s32StateCode, HI_CHAR* pszReply, HI_S32 s32MaxLen)
{
    HI_S32  s32TempLen = 0;

    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);
    OSCSVR_CHECK_NULL_ERROR(pszReply);

    s32TempLen += snprintf_s(pszReply, s32MaxLen, s32MaxLen, "%s %d %s\r\n", OSC_HTTP_VER_STR, s32StateCode, HI_OSCSVR_MSGParser_StatusCode2Str(s32StateCode));
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Expires: 0\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Max-Age: 0\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s: application/json; charset=utf-8\r\n", OSC_HTTP_HEADER_CONTENTTYPE);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s: %d\r\n", OSC_HTTP_HEADER_CONTENTLENGTH, s32JsonLen);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s: nosniff\r\n", OSC_HTTP_HEADER_XSSOPTION);
    /*Connection: close*/
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Connection: close\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s\r\n", pszJsonPack);

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Get_HTTP_Body(HI_CHAR* pClientReq, HI_U32 u32ReqLen, HI_CHAR* pszJsonPack, HI_S32* s32JsonLen)
{
    HI_U32 u32MsgLen = 0;
    HI_CHAR* pszIndexTemp = NULL;
    HI_CHAR aszTemp[OSC_MAX_PROTOCOL_BUFFER] = {0};

    OSCSVR_CHECK_NULL_ERROR(pClientReq);
    OSCSVR_CHECK_NULL_ERROR(pszJsonPack);

    strncpy_s(aszTemp, OSC_MAX_PROTOCOL_BUFFER, pClientReq, OSC_MAX_PROTOCOL_BUFFER - 1);
    aszTemp[OSC_MAX_PROTOCOL_BUFFER - 1] = '\0';

    pszIndexTemp = strstr(aszTemp, "\r\n\r\n");

    if (HI_NULL == pszIndexTemp)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "[GET_HTTP_BODY]the http req formate is invaild!!\n");
        return HI_FAILURE;
    }

    memset_s(pszJsonPack, u32ReqLen, 0x00, u32ReqLen);
    u32MsgLen = (HI_U32)(pszIndexTemp - aszTemp) + OSC_HTTP_SEPARATOR_LEN;

    pszIndexTemp += OSC_HTTP_SEPARATOR_LEN;
    u32ReqLen = u32ReqLen - u32MsgLen;

    if (u32ReqLen > 0)
    {
        strncpy_s(pszJsonPack, OSC_MAX_PROTOCOL_BUFFER, pszIndexTemp, OSC_MAX_PROTOCOL_BUFFER - 1);
        pszJsonPack[OSC_MAX_PROTOCOL_BUFFER - 1] = '\0';
        *s32JsonLen = strlen(pszJsonPack);
    }
    else
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "[GET_HTTP_BODY]the http req not contain http body!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Generate_GetImage_HTTPResponse(HI_S32 s32StateCode, HI_CHAR* pszReply, HI_S32 s32BinaryLen, HI_S32 s32MaxLen)
{
    HI_S32  s32TempLen = 0;
    HI_S32  s32ReplyLen = s32MaxLen;

    OSCSVR_CHECK_NULL_ERROR(pszReply);

    s32TempLen += snprintf_s(pszReply, s32ReplyLen, s32ReplyLen, "%s %d %s\r\n", OSC_HTTP_VER_STR, s32StateCode, HI_OSCSVR_MSGParser_StatusCode2Str(s32StateCode));
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "%s: image/jpeg\r\n", OSC_HTTP_HEADER_CONTENTTYPE);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "%s: %d\r\n", OSC_HTTP_HEADER_CONTENTLENGTH, s32BinaryLen);
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "%s: nosniff\r\n", OSC_HTTP_HEADER_XSSOPTION);
    /*Connection: close*/
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "Connection: close\r\n");
    s32TempLen += snprintf_s(pszReply + s32TempLen, s32ReplyLen - s32TempLen, s32ReplyLen - s32TempLen, "\r\n");

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Send_VideoCaptureResponse(HI_S32* ps32SocketFd, HI_CHAR* pszState)
{
    HI_S32  s32JsonLen = 0;
    HI_S32  s32RespLen = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;
    HI_CHAR aszReply[OSC_MAX_PROTOCOL_BUFFER] = {0};
    HI_CHAR aszJsonPack[OSC_MAX_PROTOCOL_BUFFER] = {0};

    memset_s(aszJsonPack, OSC_MAX_PROTOCOL_BUFFER, 0x00, OSC_MAX_PROTOCOL_BUFFER);
    s32JsonLen = 0;
    s32Ret = OSCSVR_Generate_VideoCapture_JsonPack(OSC_MAX_PROTOCOL_BUFFER, aszJsonPack, &s32JsonLen, pszState);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR startcapture generate json packs error!!\n");
        s32StatCode = HI_OSC_STATUS_INTERNAL_SERVER_ERROR;

        goto Failed;
    }

    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(aszJsonPack, s32JsonLen, s32StatCode, aszReply, OSC_MAX_PROTOCOL_BUFFER);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR startcapture generate response fail!!\n");
        s32StatCode = HI_OSC_STATUS_INTERNAL_SERVER_ERROR;

        goto Failed;
    }

    /*Send osc response*/
    s32RespLen = strlen(aszReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, aszReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVR startcapture response send fail!!\n");
            s32StatCode = HI_OSC_STATUS_INTERNAL_SERVER_ERROR;

            goto Failed;
        }
    }

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

HI_S32 OSCSVR_Generate_GetLivePreview_HTTPHeader(HI_CHAR* pszReponse, HI_S32 s32BuffLen, HI_S32* s32RespLen)
{
    HI_S32  s32MaxLen = 0;
    HI_S32  s32TempLen = 0;
    HI_S32  s32StateCode = 0;

    s32StateCode = HI_OSC_STATUS_OK;
    OSCSVR_CHECK_NULL_ERROR(pszReponse);
    OSCSVR_CHECK_NULL_ERROR(s32RespLen);
    s32MaxLen = s32BuffLen;
    s32TempLen += snprintf_s(pszReponse, s32MaxLen, s32MaxLen, "%s %d %s\r\n", OSC_HTTP_VER_STR, s32StateCode, HI_OSCSVR_MSGParser_StatusCode2Str(s32StateCode));
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n");
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Pragma: no-cache\r\n");
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Expires: 0\r\n");
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "Max-Age: 0\r\n");
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s: multipart/x-mixed-replace; boundary=%s\r\n", OSC_HTTP_HEADER_CONTENTTYPE, OSC_GETLIVEPREVIEW_BOUNDARY);
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s: nosniff\r\n", OSC_HTTP_HEADER_XSSOPTION);
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "\r\n");
    s32TempLen += snprintf_s(pszReponse + s32TempLen, s32MaxLen - s32TempLen, s32MaxLen - s32TempLen, "%s\r\n", OSC_GETLIVEPREVIEW_BOUNDARY);

    *s32RespLen = s32TempLen;

    return HI_SUCCESS;
}

HI_S32 OSCSVR_Send_HTTPResponse(HI_CHAR* pszJsonPack, HI_S32 s32JsonLen, HI_S32* ps32SocketFd)
{
    HI_S32  s32RespLen = 0;
    HI_S32  s32MaxSize = 0;
    HI_CHAR* pcReply = NULL;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32StatCode = HI_OSC_STATUS_OK;

    s32MaxSize = s32JsonLen + HI_OSC_HTTP_REPLY_HEADER_LEN;

    if (s32MaxSize <= 0)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVER response send malloc len is wrong!!\n");
        return HI_FAILURE;
    }

    pcReply = (HI_CHAR*)malloc(s32MaxSize);

    if (NULL == pcReply)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVER response send malloc fail!!\n");
        return HI_FAILURE;
    }

    memset_s(pcReply, s32MaxSize, 0x00, s32MaxSize);

    s32StatCode = HI_OSC_STATUS_OK;
    s32Ret = OSCSVR_Generate_HTTP_Response(pszJsonPack, s32JsonLen, s32StatCode, pcReply, s32MaxSize);

    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVER generate response fail!!\n");
        free(pcReply);
        pcReply = NULL;
        return HI_FAILURE;
    }

    /*Send osc response*/
    s32RespLen = strlen(pcReply);

    if (s32RespLen > 0)
    {
        s32Ret = HI_OSCSVR_Response_Send(*ps32SocketFd, pcReply, s32RespLen);

        if (HI_SUCCESS != s32Ret)
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVER response send fail!!\n");
            free(pcReply);
            pcReply = NULL;
            return HI_FAILURE;
        }
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_OSCSERVER, HI_SERVER_LOG_LEVEL_ERR, "OSCSVER Response \n%s\n", pcReply);
    free(pcReply);
    pcReply = NULL;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
