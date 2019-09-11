/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_msgparser.h
* @brief     oscserver msgparser head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/

#ifndef __HI_OSCSERVER_HTTP_H__
#define __HI_OSCSERVER_HTTP_H__

#include "hi_osc_server.h"
#include "hi_oscserver_com_exec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define OSC_HTTP_INVALID_STATUS_CODE              (-1)
#define OSC_HTTP_INVALID_STATUS                   "Invalid Status"

#define OSC_HTTP_SEPARATOR_LEN                    (4)
#define HI_OSC_HTTP_REPLY_HEADER_LEN              (256)

/*HTTP start code*/
#define HI_OSC_BAD_STATUS_BEGIN                   (202)
#define HI_OSC_STATUS_CONTINUE                    (100)
#define HI_OSC_STATUS_OK                          (200)
#define HI_OSC_STATUS_ACCEPTED                    (202)
#define HI_OSC_STATUS_BAD_REQUEST                 (400)
#define HI_OSC_STATUS_UNAUTHORIZED                (401)
#define HI_OSC_STATUS_FORBIDDEN                   (403)
#define HI_OSC_STATUS_STREAM_NOT_FOUND            (404)
#define HI_OSC_STATUS_METHOD_NOT_ALLOWED          (405)
#define HI_OSC_STATUS_OVER_SUPPORTED_CONNECTION   (416)
#define HI_OSC_STATUS_SESSION_NOT_FOUND           (454)
#define HI_OSC_STATUS_UNSUPPORT_TRANSPORT         (461)
#define HI_OSC_STATUS_INTERNAL_SERVER_ERROR       (500)
#define HI_OSC_STATUS_SERVICE_UNAVAILIABLE        (503)

/*HTTP GET OR POST*/
#define HI_OSC_HTTP_REQ_GET_METHOD                "GET"
#define HI_OSC_HTTP_REQ_POST_METHOD               "POST"

#define OSC_HTTP_SERVER_DESCRIPTION               "Hisilicon HTTP Streaming Media Server"

/* message header keywords */
#define OSC_HTTP_HEADER_USER_AGENT                "User-Agent"
#define OSC_HTTP_HEADER_AUTHORIZATION             "Authorization"
/* message header keywords */
#define OSC_HTTP_HEADER_CONTENTLENGTH             "Content-Length"
#define OSC_HTTP_HEADER_ACCEPT                    "Accept"
#define OSC_HTTP_HEADER_CONTENTTYPE               "Content-Type"
#define OSC_HTTP_MIME_CONTENTTYPE                 "Content-type"

#define OSC_HTTP_HEADER_XSSOPTION                 "X-Content-Type-Options"
#define OSC_HTTP_VER_STR                          "HTTP/1.1"
/* message header keywords */
#define OSC_SSCANF_ASSIGN_NUMBER                 (3)

/*API supported by server*/
#define OSC_INFO_API                              "/osc/info"
#define OSC_STATE_API                             "/osc/state"
#define OSC_CHECK_FORUPDATES_API                  "/osc/checkForUpdates"
#define OSC_COMMAND_EXECUTE_API                   "/osc/commands/execute"
#define OSC_COMMAND_STATUS_API                    "/osc/commands/status"

/*Length*/
#define OSC_VER_MAX_LEN                           (16)
#define OSC_URL_MAX_LEN                           (64)
#define OSC_METHOD_MAX_LEN                        (32)
#define OSC_MAX_PROTOCOL_BUFFER                   (2048)

#define OSC_INVAILD_REPLY_MAX_LEN                 (512)
#define OSC_CLIENT_MSG_MAX_LEN                    (1024)
#define OSC_STATE_STRING_LEN                      (64)

#define HI_OSC_SUPPORT_COMMAND_NUM                (20)
/*API enum*/
typedef enum hiOSC_REQ_API_E
{
    /*api codes*/
    HI_OSC_INFO_API = 0,
    HI_OSC_STATE_API = 1,
    HI_OSC_CHECK_FORUPDATES_API = 2,
    HI_OSC_COMMAND_EXECUTE_API = 3,
    HI_OSC_COMMAND_STATUS_API = 4,
    HI_OSC_INVAILD_API
} HI_OSC_REQ_API_E;

typedef struct hiOSC_HTTP_STATUS
{
    HI_CHAR pszStatStr[OSC_STATE_STRING_LEN];
    HI_S32   s32StatCode;
} HI_OSC_HTTP_STATUS_S;

typedef struct hiOSC_COMMAND_S
{
    HI_CHAR pszCommand[OSC_COMM_NAME_MAX_LEN];
    HI_OSC_API_COMMAND_E   enCommandCode;
} HI_OSC_COMMAND_S;

/*options table*/
typedef struct hiOSC_OPTIONS_S
{
    HI_CHAR aszOptions[HI_OSC_OPTIONS_NAME_MAX_LEN];
    HI_BOOL bOptionsFlag;
} HI_OSC_OPTIONS_S;

/*options kinds number*/
#define HI_OSC_OPTIONS_IMAGE_MAX_NUM                  (26)
#define HI_OSC_OPTIONS_INTERVAL_MAX_NUM               (5)
#define HI_OSC_OPTIONS_FORMAT_MAX_NUM                 (4)
#define HI_OSC_OPTIONS_INFO_MAX_NUM                   (18)

HI_BOOL HI_OSCSVR_MSGParser_CheckRequest(const HI_CHAR* pszReq, HI_U32 u32ReqLen, HI_OSC_REQ_API_E* penMethod);
HI_S32  HI_OSCSVR_MSGParser_ParseAPI(const HI_CHAR* pszReq, HI_U32 u32ReqLen, HI_OSC_REQ_API_E* peMethod);
HI_BOOL HI_OSCSVR_CheckVaildMsg(const HI_CHAR* pszReq, HI_U32 u32Reqlen);
HI_CHAR* HI_OSCSVR_MSGParser_StatusCode2Str(HI_S32 s32Code);
HI_S32  HI_OSCSVR_MSGParser_ConnectFailResponse(HI_CHAR* pszReply, HI_S32 s32StateCode, HI_S32 s32MaxLen);
HI_S32  HI_OSCSVR_MSGParser_GetServerVersion(HI_CHAR* pszSvrVersion, HI_U32 u32VersionLen);
HI_S32 OSCSVR_Generate_HTTP_Response(HI_CHAR* pszJsonPack, HI_S32 s32JsonLen, HI_S32 s32StateCode, HI_CHAR* pszReply, HI_S32 s32MaxLen);
HI_S32 OSCSVR_Get_HTTP_Body(HI_CHAR* pClientReq, HI_U32 u32ReqLen, HI_CHAR* pszJsonPack, HI_S32* s32JsonLen);
HI_S32 HI_OSCSVR_MSGParser_ParseComm(const HI_CHAR* pszCommName, HI_OSC_API_COMMAND_E* penComAPI);
HI_S32 OSCSVR_Generate_GetLivePreview_HTTPHeader(HI_CHAR* pszReponse, HI_S32 s32BuffLen, HI_S32* s32RespLen);
HI_S32 OSCSVR_Generate_GetImage_HTTPResponse(HI_S32 s32StateCode, HI_CHAR* pszReply, HI_S32 s32BinaryLen, HI_S32 s32MaxLen);
HI_S32 OSCSVR_Send_VideoCaptureResponse(HI_S32* ps32SocketFd, HI_CHAR* pszState);
HI_S32 OSCSVR_Send_HTTPResponse(HI_CHAR* pszJsonPack, HI_S32 s32JsonLen, HI_S32* ps32SocketFd);
HI_S32 OSCSVR_Get_CommandApi(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey, HI_VOID* pfnEventCB);
HI_S32 OSCSVR_Get_FileCommandApi(HI_OSC_SERVER_CTX_S* pstServerCtx, HI_OSC_API_COMMAND_E enOSCCommandKey, HI_VOID* pfnEventCB);
HI_S32 HI_OSCSVR_Response_Send(HI_SOCKET s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen);
HI_S32 HI_OSCSVR_CloseSocket(HI_S32* ps32Socket);
HI_S32 HI_OSCSVR_Create_ImageTable(HI_OSC_OPTIONS_S* pstImageOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab, HI_OSC_SERVER_LEVEL_E enLevel);
HI_S32 HI_OSCSVR_Create_IntervalTable(HI_OSC_OPTIONS_S* pstIntervalOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab);
HI_S32 HI_OSCSVR_Create_FormatTable(HI_OSC_OPTIONS_S* pstFormatOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab);
HI_S32 HI_OSCSVR_Create_InfoTable(HI_OSC_OPTIONS_S* pstInfoOptions, HI_OSC_OPTIONS_TABLE_S* pstOptionsTab);
HI_S32 HI_OSCSVR_Frame_Send(HI_SOCKET s32WritSock, HI_CHAR* pszBuff, HI_U32 u32DataLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_HTTP_H__*/
