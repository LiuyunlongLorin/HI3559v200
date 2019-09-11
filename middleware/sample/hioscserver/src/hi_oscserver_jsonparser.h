/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_jsonparser.h
* @brief     oscserver jsonparser head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/

#ifndef __HI_OSCSERVER_JSONPARSER_H__
#define __HI_OSCSERVER_JSONPARSER_H__

#include "hi_osc_server.h"
#include "hi_oscserver_http.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define OSC_STATE_WAITOUT_MAX_TIME         (900)
#define OSC_INVAILD_MESSAGE_MAX_LEN        (128)
#define OSC_INVAILD_CODE_MAX_LEN           (32)

/*osc response invaild code enum*/
typedef enum hi_OSC_INVAILD_CODE_E
{
    HIOSC_MISSING_PARAMETER = 1,
    HIOSC_INVAILD_PARAMETER_NAME,
    HIOSC_INVAILD_PARAMETER_VALUE,
    HIOSC_INVAILD_IN_EXCLUSIVEUSE,
    HIOSC_INVAILD_DISABLED_COMMAND,
    HIOSC_INVAILD_UNABLE_SERVICE,
    HIOSC_INVAILD_UNKNOWN_COMMAND,
    HIOSC_INVAILD_UPLOAD_ERROR,
    HIOSC_SERVER_ERROR,
    HIOSC_SERVER_ERROR_INIT
} HI_OSC_INVAILD_CODE_E;


/*invaild code string*/
#define OSC_INVAILD_STATE_MISSING_PARAM             "missingParameter"
#define OSC_INVAILD_STATE_INVAILD_PARAM_NAME        "invaildParametername"
#define OSC_INVAILD_STATE_INVAILD_PARAM_VALUE       "invaildParametervalue"
#define OSC_INVAILD_STATE_IN_EXCLUSIVEUSE           "cameraInExclusiveUse"
#define OSC_INVAILD_STATE_DISABLED_COMMAND          "disabledCommand"
#define OSC_INVAILD_STATE_UNKNOWN_COMMAND           "unknownCommand"
#define OSC_INVAILD_STATE_UNABLE_SERVICE            "serviceUnavailable"
#define OSC_INVAILD_STATE_UPLOAD_ERROR              "uploadError"

/*comm name*/
#define OSC_NAME_START_SESSION                      "camera.startSession"
#define OSC_NAME_UPDATE_SESSION                     "camera.updateSession"
#define OSC_NAME_CLOSE_SESSION                      "camera.closeSession"
#define OSC_NAME_TAKE_PICTURE                       "camera.takePicture"
#define OSC_NAME_SET_OPTIONS                        "camera.setOptions"
#define OSC_NAME_GET_OPTIONS                        "camera.getOptions"
#define OSC_NAME_PROCESS_PICTURE                    "camera.processPicture"
#define OSC_NAME_GET_IMAGE                          "camera.getImage"
#define OSC_NAME_RESET                              "camera.reset"
#define OSC_NAME_SWITCH_WIFI                        "camera.switchWifi"
#define OSC_NAME_UPLOAD_FILE                        "camera.uploadFile"
#define OSC_NAME_START_CAPTURE                      "camera.startCapture"
#define OSC_NAME_STOP_CAPTURE                       "camera.stopCapture"
#define OSC_NAME_LIST_FILE                          "camera.listFiles"
#define OSC_NAME_LIST_IMAGE                         "camera.listImages"
#define OSC_NAME_GET_METADATA                       "camera.getMetadata"
#define OSC_NAME_GET_LIVEPREVIEW                    "camera.getLivePreview"
#define OSC_NAME_DELETE                             "camera.delete"
#define OSC_NAME_PRIVATE_COMMAND                    "camera._"


/*osc comm state*/
#define OSC_COMM_STATE_DONE                         "done"
#define OSC_COMM_STATE_INPROGRESS                   "inProgress"
#define OSC_COMM_STATE_ERROR                        "error"


#define OSC_IGNORE_SESS_TIMEOUT                     (-1)

HI_S32 OSCSVR_Parser_CheckForUpdate_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_OSCSERVER_CHECKFORUPDATES_S* pstOSCCheckForUpdate);
HI_S32 OSCSVR_Parser_StartSession_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_S32* ps32SessTimeOut);
HI_S32 OSCSVR_Parser_Name_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName);
HI_S32 OSCSVR_Parser_CloseSession_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_CHAR* ps32SessId);
HI_S32 OSCSVR_Parser_UpdateSession_JsonPack(HI_CHAR* pszJsonPack,  HI_CHAR* pszComName, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_S32* ps32SessTimeOut);
HI_S32 OSCSVR_Parser_TakePicture_JsonPack(HI_CHAR* pszJsonPack,  HI_CHAR* pszComName, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* enLevel);
HI_S32 OSCSVR_Parser_SetOptions_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* penLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCSetOptions);
HI_S32 OSCSVR_Parser_GetOptions_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32SocketFd, HI_CHAR* pszSessionId, HI_OSC_SERVER_LEVEL_E* penLevel, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSCSERVER_OPTIONS_S* pstOSCGetOptions);
HI_S32 OSCSVR_Parser_ProcessPicture_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszComName, HI_CHAR aszPreviewFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN]);
HI_S32 OSCSVR_Parser_GetImage_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszFileUrl, HI_S32* ps32MaxSize);
HI_S32 OSCSVR_Parser_Reset_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_CHAR* pszComName);
HI_S32 OSCSVR_Parser_SwitchWifi_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_SWITCH_WIFI_S* pstSwitchWifi);
HI_S32 OSCSVR_Parser_UploadFile_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_UPLOAD_FILE_S* pstUploadFile);
HI_S32 OSCSVR_Parser_ListFiles_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_S32 s32ListFiles, HI_OSCSERVER_LIST_FILE_S* pstListFile);
HI_S32 OSCSVR_Parser_ListImage_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_S32 s32ListImages, HI_OSCSERVER_LIST_IMAGE_S* pstListImage);
HI_S32 OSCSVR_Parser_GetMetaData_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSCSERVER_METADATA_S* pstMetaData);
HI_S32 OSCSVR_Parser_Delete_JsonPack(HI_CHAR* pszJsonPack,  HI_S32* ps32SocketFd, HI_OSC_SERVER_LEVEL_E* enLevel, HI_CHAR aszDeleteUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN]);
HI_S32 OSCSVR_Parser_CommandStatus_JsonPack(HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_S32* ps32SocketFd, HI_CHAR* pszComId);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_JSONPARSER_H__*/
