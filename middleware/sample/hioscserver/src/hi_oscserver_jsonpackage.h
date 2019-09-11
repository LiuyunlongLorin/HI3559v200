/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_jsonpackage.h
* @brief     oscserver jsonparser head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/

#ifndef __HI_OSCSERVER_JSONPACKAGE_H__
#define __HI_OSCSERVER_JSONPACKAGE_H__

#include "hi_osc_server.h"
#include "hi_oscserver_http.h"
#include "hi_oscserver_jsonparser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 OSCSVR_Generate_Invaild_JsonPack(HI_S32 s32InvaildCode, HI_CHAR* pszComName, HI_CHAR* pszMessage, HI_CHAR* pszJsonPack, HI_S32* s32JsonLen);
HI_S32 OSCSVR_Generate_SVRInfo_JsonPack(HI_OSCSERVER_INFO_S* pstOSCSVRInfo, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_SVRState_JsonPack(HI_OSCSERVER_STATE_S* pstOSCSVRState, HI_S32 s32MaxLen, HI_CHAR* pszFingerPrint, HI_OSC_SERVER_LEVEL_E enLevel, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_CheckForUpdate_JsonPack(HI_U32 u32ThrottleTimeout, HI_S32 s32MaxLen, HI_CHAR* pszFingerPrint, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_StartSession_JsonPack(HI_S32 s32Timeout, HI_S32 s32MaxLen, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_CloseSession_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_UpdateSession_JsonPack(HI_S32 s32Timeout, HI_S32 s32MaxLen, HI_CHAR* pszSessionId, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_VideoCapture_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen, HI_CHAR* pszState);
HI_S32 OSCSVR_Generate_TakePicture_JsonPack(HI_CHAR* pszCommandId, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_SetOptions_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_GetOptions_JsonPack(HI_OSC_SERVER_LEVEL_E enLevel, HI_S32 s32MaxLen, HI_OSCSERVER_OPTIONS_S* pstOSCOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_ProcessPicture_JsonPack(HI_CHAR* pszCommandID, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_Reset_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_SwitchWifi_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_UploafFile_JsonPack(HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_IntervalCapture_JsonPack(HI_CHAR* pszCommandID, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_StopCapture_JsonPack(HI_CHAR pszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_ListFile_JsonPack(HI_OSCSERVER_LIST_FILE_S* pstListFile, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_ListImage_JsonPack(HI_OSCSERVER_LIST_IMAGE_S* pstListImage, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_GetMetaData_JsonPack(HI_OSCSERVER_METADATA_S* pstMetaData, HI_S32 s32MaxLen, HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_Delete_JsonPack(HI_OSC_SERVER_LEVEL_E enLevel, HI_S32 s32MaxLen, HI_CHAR aszReultsUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);
HI_S32 OSCSVR_Generate_CommandStatus_JsonPack(HI_CHAR* pszCommandName, HI_S32 s32MaxLen, HI_CHAR* pszCommandId, HI_OSC_SERVER_LEVEL_E enCliVersion, HI_OSC_COMM_STATUS_E enComStatus, HI_DOUBLE dCompletion, HI_CHAR pszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszJsonPack, HI_S32* ps32JsonLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_JSONPACKAGE_H__*/

