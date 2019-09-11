/***********************************************************************************
*             Copyright 2006 - 2006, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_init.c
* Description:the interface of init all module
*
* History:
* Version   Date         Author     DefectNum    Description
* 1.1       2006-04-07   qiubin  NULL         Create this file.
***********************************************************************************/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "mmc.h"
#include "libhttpd.h"
#include "hisnet.h"
#include "hisnet_icgi.h"
#include "hisnet_cmd_sys.h"
#include "hisnet_cmd_file.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */



#define MAX_HTTP_EXTRA_HEAD_LEN (64)
#define SENDBUFFER_SIZE (512)
#define HTTP_Path_Backup "/tmpfs/config_backup.bin"
#define MAX_ACCESSLOG_LEN (1024)
#define MAX_HTTP_STATUS_CODE_LEN (64)
#define MAX_HTTP_MIME_TYPE_LEN (64)
#define MAX_HTTP_METHOD_LEN (16)
#define MAX_CNT_CGI 3072 /* Maximum Length for CGI */


//static PTR_FUNC_USER_AUTH g_pfunUserAuthenticate = 0;
//static PTR_FUNC_ACCESS_LOG g_pfunAccessLog = 0 ;
//static PTR_FUNC_ICGI_COMMON_HISNET g_pfunFileUpload = 0;


extern int terminate;
pthread_t g_webserveru32Thread = 0;

static HI_S32 ICGI_Response(void* pData, char* pszfmt, ...)
{
    if (NULL == pData || NULL == pszfmt)
    {
        return -1;
    }

    int ret;
    char response[2048];
    va_list stVal;
    va_start(stVal, pszfmt);
    ret = vsprintf(response, pszfmt, stVal);
    va_end(stVal);
    add_response((httpd_conn*)pData, response);
    return ret;
}

static HI_S32 ICGI_CommandProc(httpd_conn* hc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR* pCommand = NULL;
    HI_CHAR szCommand[MAX_CGI_INTER] = {0};
    HI_CHAR aszMethod[MAX_HTTP_METHOD_LEN] = {0};
    HI_CHAR aszAccessLog[MAX_ACCESSLOG_LEN] = {0};

    if (hc == NULL)
    {
        MLOGE("pstRequest & pstReponse can not be null\n\n\n");
        return HI_FAILURE;
    }

    if (hc->origfilename == NULL)
    {
        MLOGE("origfilename can not be null\n\n\n");
        return HI_FAILURE;
    }

    if ( hc->method == METHOD_GET )
    {
        strncpy(aszMethod, "GET", MAX_HTTP_METHOD_LEN - 1);
    }
    else
    {
        strncpy(aszMethod, "POST", MAX_HTTP_METHOD_LEN - 1);
    }

    pCommand = strrchr(hc->origfilename, '/');

    if ((pCommand != NULL) && (strlen(pCommand) < MAX_CGI_INTER))
    {
        strncpy(szCommand, pCommand + 1, MAX_CGI_INTER - 1);
        szCommand[MAX_CGI_INTER - 1] = '\0';
    }
    else
    {
        MLOGE("the command is invalide:\n%s\n", hc->origfilename);
        return HI_FAILURE;
    }

#if 0

    if (0 == strcmp(szCommand, HI_CGI_FILEUPLOAD))
    {
        HISNET_HTTPD_CONN stFileupdata;
        stFileupdata.conn_fd = hc->conn_fd;
        stFileupdata.contenttype = hc->contenttype;
        stFileupdata.contentlength = hc->contentlength;
        stFileupdata.checked_idx = hc->checked_idx;
        stFileupdata.read_idx = hc->read_idx;
        stFileupdata.read_buf = hc->read_buf;

        if (g_pfunFileUpload == NULL)
        {
            return HI_SUCCESS;
        }

        return g_pfunFileUpload(&stFileupdata);
    }
    else if (0 == strcmp(szCommand, HI_CGI_UserAuthenticate))
    {
        s32Ret = 0;

        if (NULL != g_pfunUserAuthenticate)
        {
            s32Ret =  g_pfunUserAuthenticate(&(hc->authorization[6]));

            if (s32Ret != 0)
            {
                ICGI_Response(hc, "no authentication");
            }
            else
            {
                ICGI_Response(hc, "authentication");
            }
        }

        return s32Ret;
    }
    else
#endif
    {
        HI_CHAR query[MAX_CNT_CGI] = {0};
        HI_CHAR pszRequest[MAX_CNT_CMD] = {0};
        strncpy(pszRequest, szCommand, strlen(szCommand));

        if (hc->method == METHOD_GET)
        {
            if (hc->query[0] != '\0')
            {
                if (strlen(hc->query) >= MAX_CNT_CGI)
                {
                    *(hc->query + MAX_CNT_CGI) = '\0';
                }

                strdecode( query, hc->query );

                if (strlen(query)  > (sizeof(pszRequest) - strlen(pszRequest)) - 1)
                {
                    MLOGE("HI_CGI_CommandProc pszRequest len is too short\n");
                    return HI_FAILURE;
                }

                strncat(pszRequest, query, strlen(query));
            }

            strncpy(aszMethod, "GET", MAX_HTTP_METHOD_LEN - 1);
        }
        else if (hc->method == METHOD_POST)
        {
            strncpy(aszMethod, "POST", MAX_HTTP_METHOD_LEN - 1);
            HI_S32 c, retnum, cpynum = 0, r;
            HI_CHAR buf[MAX_CNT_CGI] = {0};
            c = hc->read_idx - hc->checked_idx;

            if (c > 0)
            {
                strncpy(buf, &(hc->read_buf[hc->checked_idx]), MIN(c, MAX_CNT_CGI - 1));
                cpynum = MIN(c, MAX_CNT_CGI - 1);
            }

            while ((c < hc->contentlength ) && ( c < MAX_CNT_CGI - 1))
            {
                r = read(hc->conn_fd, buf + c, MIN( hc->contentlength - c, MAX_CNT_CGI - 1 - c));

                if (r < 0 && (errno == EINTR || errno == EAGAIN ))
                {
                    sleep( 1 );
                    continue;
                }

                if ( r <= 0 )
                {
                    break;
                }

                c += r;
                cpynum += r;
            }

            buf[cpynum] = '\0';
            retnum = strlen(buf);

            if (retnum > 0)
            {
                strdecode(query, buf);

                if (strlen(query)  > (sizeof(pszRequest) - strlen(pszRequest)) - 1)
                {
                    MLOGE("HI_CGI_CommandProc pszRequest len is too short\n");
                    return HI_FAILURE;
                }

                strncat(pszRequest, query, strlen(query));
            }
        }
        else
        {
            MLOGE("invalide request method \n" );
            return HI_FAILURE;
        }

        if (pszRequest[0] != '\0')
        {
            memset(aszAccessLog, 0, sizeof(aszAccessLog));
            snprintf(aszAccessLog, sizeof(aszAccessLog), "[%s] %s %s \n", httpd_ntoa(&hc->client_addr), aszMethod, pszRequest);
            MLOGD("Webserver Accesslog:\n%s\n", aszAccessLog);
        }
        else
        {
            memset(aszAccessLog, 0, sizeof(aszAccessLog));
            snprintf(aszAccessLog, sizeof(aszAccessLog), "[%s] %s %s \n", httpd_ntoa(&hc->client_addr), aszMethod, szCommand);
            MLOGD("Webserver Accesslog:\n%s\n", aszAccessLog);
        }

        s32Ret = HI_CGI_CommandProc(pszRequest, ICGI_Response, hc);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("HI_CGI_CommandProc fail\n");
            return s32Ret;
        }

    }

    return HI_SUCCESS;
}


HI_S32 HISNET_Init(HI_VOID)
{
    HI_S32 s32Rtn;

    if (0 != g_webserveru32Thread)
    {
        MLOGD("WebServer is running!\n");
        return HI_SUCCESS;
    }

    terminate = 0;

#ifdef __HuaweiLite__
    //#define PTHREAD_EXPLICIT_SCHED          2
    pthread_attr_t user_attr;
    pthread_attr_init( &user_attr );
    pthread_attr_setinheritsched (&user_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize (&user_attr, 128 * 1024);
    s32Rtn = pthread_create(&g_webserveru32Thread, &user_attr, thttpd_start_main, NULL);
    pthread_attr_destroy(&user_attr);
#else
    s32Rtn = pthread_create(&g_webserveru32Thread, NULL, thttpd_start_main, NULL);
#endif //#ifdef __LITEOS__


    HI_THTTPD_RegisterInternalCgiProc(ICGI_CommandProc);
    MLOGD("INIT WEB SERVER OK \n");
    return s32Rtn;
}

HI_S32 HISNET_Deinit(HI_VOID)
{
    if (0 == g_webserveru32Thread)
    {
        MLOGD("WebServer is not running!\n");
        return HI_SUCCESS;
    }

    terminate = 1;
    (void)pthread_join(g_webserveru32Thread, NULL);
    g_webserveru32Thread = 0;
    MLOGD("WebServer is stopped !");
    return HI_SUCCESS;
}

HI_S32 HISNET_CleanMmapCache(HI_VOID)
{
    mmc_term();
    return HI_SUCCESS;
}

HI_S32 HISNET_RegisterDistribLink(PTR_FUNC_DistribLink pfunDistribLink)
{
    return HI_THTTPD_RegisterDistribLink(pfunDistribLink);
}

HI_S32 HISNET_RegisterDefaultCmd(const HISNET_DEFAULT_SVR_CB_S* pstDefaultCmdProc)
{
    HI_S32 s32Ret;

    if (NULL == pstDefaultCmdProc)
    {
        MLOGE("pstDefaultCmdProc is NULL\n");
        return HI_FAILURE;
    }

    Hisnet_SYS_RegCallBackFunction(&pstDefaultCmdProc->stSysSvrCB);
    Hisnet_FILE_RegCallBackFunction(&pstDefaultCmdProc->stFileSvrCB);
    s32Ret = HI_CGI_RegisterDefaultCmd();

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_CGI_RegisterDefaultCmd fail\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HISNET_RegisterCmd(cgi_cmd_trans_s* pstCgiCmd)
{
    HI_S32 s32Ret;

    if (NULL == pstCgiCmd->cmdProc)
    {
        MLOGE("stCgiCmd.cmdProc is NULL\n");
        return HI_FAILURE;
    }

    s32Ret = HI_CGI_RegisterCgi(pstCgiCmd);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_CGI_RegisterCgi fail\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}


//HI_S32 HISNET_RegisterInternalCgiProc(HISNET_ICGI_PROC_S stfunIcgiProc)
//{
//    if (stfunIcgiProc.pfunAccessLog != NULL)
//    {
//        g_pfunAccessLog = stfunIcgiProc.pfunAccessLog;
//    }
//
//    if (stfunIcgiProc.pfunFileUpload != NULL)
//    {
//        g_pfunFileUpload = stfunIcgiProc.pfunFileUpload;
//    }
//
//    if (stfunIcgiProc.pfunUserAuthenticate != NULL)
//    {
//        g_pfunUserAuthenticate = stfunIcgiProc.pfunUserAuthenticate;
//    }
//
//
//    return 0;
//}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
