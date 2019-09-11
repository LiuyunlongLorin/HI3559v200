/***********************************************************************************
*             Copyright 2006 - 2006, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_init.h
* Description:the interface of init all module
*
* History:
* Version   Date         Author     DefectNum    Description
* 1.1       2006-04-07   qiubin  NULL         Create this file.
***********************************************************************************/


#ifndef __HISNET_H__
#define __HISNET_H__

#include "thttpd.h"
#include "hi_appcomm.h"

#include "hisnet_sys.h"
#include "hisnet_file.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define WEBSERVER_NO_DISTRIBLINK 200

#define MAX_CGI_INTER 48
#define MAX_CMD_INTER 48
#define METHOD_LEN 4


#define CGI_METHOD_GET "get"
#define CGI_METHOD_SET "set"

typedef HI_S32 (*PfnCgiStringProc)(HI_VOID* pvData, HI_CHAR* pszfmt, ...);
typedef HI_S32 (*CGI_CMD_CmdProc)(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnStringProc, HI_VOID* pvData);

typedef struct cgi_cmd_trans
{
    HI_CHAR          cgi[MAX_CGI_INTER];
    HI_CHAR          cmd[MAX_CMD_INTER];
    HI_CHAR          method[METHOD_LEN];    /*get|set|*/
    CGI_CMD_CmdProc  cmdProc;               /*CallBack*/
} cgi_cmd_trans_s;

typedef struct hisnet_DEFAULT_SVR_CB_S
{
    HISNET_SYS_SVR_CB_S stSysSvrCB;
    HISNET_FILE_SVR_CB_S stFileSvrCB;
} HISNET_DEFAULT_SVR_CB_S;


HI_S32 HISNET_Init(HI_VOID);
HI_S32 HISNET_Deinit(HI_VOID);
HI_S32 HISNET_CleanMmapCache(HI_VOID);
HI_S32 HISNET_RegisterDistribLink(PTR_FUNC_DistribLink pfunDistribLink);
HI_S32 HISNET_RegisterDefaultCmd(const HISNET_DEFAULT_SVR_CB_S *pstDefaultCmdProc);
HI_S32 HISNET_RegisterCmd(cgi_cmd_trans_s* pstCgiCmd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HISNET_H__ */
