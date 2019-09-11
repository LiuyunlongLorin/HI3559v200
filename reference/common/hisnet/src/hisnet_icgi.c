#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>

//#include "hi_type.h"
#include "hisnet.h"
#include "hisnet_cmd_sys.h"
#include "hisnet_cmd_file.h"
#include "hisnet_icgi.h"

#define CGI_METHOD_CGI "cgi"
#define CGI_URL_PATH_FLAG "cururl"
#define CGI_POSTFIX ".cgi"
#define MAX_NUM_CMD 128  //command module function's max number of arguments
#define MAX_CNT_EVERYCMD 1024 //max length of every argument
#define MAX_LEN_URL 128
#define MAX_LEN 40
#define MAX_NUM_CMD 128  /* Maximum Param Number */
#define MAX_CNT_EVERYCMD 1024 /* Maximum Length for Parameter */




#define SENDBUFFER_SIZE (512)
#define DEVICEINFO_PAGE "/web/deviceinfo.html"

//in script boottab, if return value is 10, reboot system
#define EXIT_RESTART exit(10)

static cgi_cmd_trans_s astCgiCmd[MAX_LEN];
pthread_mutex_t g_stCgiLock = PTHREAD_MUTEX_INITIALIZER;

static cgi_cmd_trans_s s_stDefaultCGI [] =
{
    /*Hisnet_sys*/
    {"getdeviceattr.cgi",           "getdeviceattr&-act=get&",      CGI_METHOD_GET,     Hisnet_CMD_SYS_DevAttr},
    {"getsdstatus.cgi",             "getsdstatus&-act=get&",        CGI_METHOD_GET,     Hisnet_CMD_SYS_GetSDStatus},
    {"getsystime.cgi",              "getsystime&-act=get&",         CGI_METHOD_GET,     Hisnet_CMD_SYS_SysTimeCfg},
    {"setsystime.cgi",              "setsystime&-act=set&",         CGI_METHOD_SET,     Hisnet_CMD_SYS_SysTimeCfg},
    {"reset.cgi",                   "reset&",                       CGI_METHOD_GET,     Hisnet_CMD_SYS_Reset},
    {"sdcommand.cgi",               "sdcommand&",                   CGI_METHOD_GET,     Hisnet_CMD_SYS_SDCommand},
    {"getbatterycapacity.cgi",      "getbatterycapacity&",          CGI_METHOD_GET,     Hisnet_CMD_SYS_GetBatteryStatus},
    {"setwifi.cgi",                 "wifiattr&-act=set&",           CGI_METHOD_SET,     Hisnet_CMD_SYS_WifiAttrCfg},
    {"getwifi.cgi",                 "wifiattr&-act=get&",           CGI_METHOD_GET,     Hisnet_CMD_SYS_WifiAttrCfg},

    /*DV Filemng cgi*/
    {"getfilecount.cgi",            "getfilecount&",                CGI_METHOD_GET,     Hisnet_CMD_FILE_GetFileNum},
    {"getfilelist.cgi",             "getfilelist&",                 CGI_METHOD_GET,     Hisnet_CMD_FILE_GetFileList},
    {"getfileinfo.cgi",             "getfileinfo&",                 CGI_METHOD_GET,     Hisnet_CMD_FILE_GetFileInfo},
    {"deletefile.cgi",              "deletefile&",                  CGI_METHOD_GET,     Hisnet_CMD_FILE_DeleteFile},
    {"deleteallfiles.cgi",          "deleteallfiles&",              CGI_METHOD_GET,     Hisnet_CMD_FILE_DeleteAllFiles},
};


int CGI_StrCommandParse(char* strcmdin, int* argc, char** argv, int maxCnt, int maxLen)
{
    char* pszQuery;
    char* pszAndTemp = NULL;
    char* pszEqualTemp = NULL;
    char* pszMinTemp = NULL;
    int s32Num = *argc;
    int s32StrLen = 0;

    pszQuery = strcmdin;

    while ( (NULL != pszQuery) && (*pszQuery != '\0'))
    {
        pszAndTemp = strchr(pszQuery, '&');
        pszEqualTemp = strchr(pszQuery, '=');

        if ((pszAndTemp != NULL) && (pszEqualTemp != NULL))
        {
            if ( strlen(pszAndTemp) > strlen(pszEqualTemp) )
            {
                pszMinTemp = pszAndTemp;
            }
            else
            {
                pszMinTemp = pszEqualTemp;
            }
        }
        else if ((pszAndTemp != NULL))
        {
            pszMinTemp = pszAndTemp;
        }
        else if ((pszEqualTemp != NULL))
        {
            pszMinTemp = pszEqualTemp;
        }
        else if ((pszAndTemp == NULL) && (pszEqualTemp == NULL))
        {
            s32StrLen = MIN( strlen(pszQuery), maxLen - 1 );
            strncpy(argv[s32Num], pszQuery, s32StrLen);
            *(argv[s32Num] + s32StrLen) = '\0';
            s32Num++;
            break;
        }

        if ((strlen(pszQuery) - strlen(pszMinTemp)) > 0)
        {
            s32StrLen = MIN((strlen(pszQuery) - strlen(pszMinTemp)), maxLen - 1 );
            strncpy(argv[s32Num], pszQuery, s32StrLen);
            *(argv[s32Num] + s32StrLen ) = '\0';
            s32Num++;

            if ( s32Num >= maxCnt)
            {
                break;
            }
        }

        pszQuery = pszMinTemp + 1;
        pszAndTemp = NULL;
        pszEqualTemp = NULL;
        pszMinTemp = NULL;
    }

    (*argc) = s32Num;
    return 0;
}

int HI_CGI_CommandProc(char* strcmdin, PfnCgiStringProc pfnCgiStringGet, void* pData)
{
    int i, j;
    int ret;
    int argc = 0;
    char* argv[MAX_NUM_CMD] = {0};
    char* pszAllMalloc = NULL;
    CGI_CMD_CmdProc pFunCgiCmdProc = NULL;
    char szCommand[MAX_CGI_INTER] = {0};
    char* pCommand = NULL;
    char* pCgiLabel = NULL;     //'.cgi' symbol
    char* pAndLabel = NULL;     //'&' symbol
    char method[8] = {0};
    int commandFlag = 0;
    HI_CHAR aszRequest[MAX_CNT_CMD] = {0};

    if (strcmdin == NULL)
    {
        MLOGE("strcmdin is NULL\n");
        return -1;
    }

    //parse command from strcmdin, the substring before ".cgi"  is the right command.
    pCgiLabel = strstr(strcmdin, CGI_POSTFIX);

    if (NULL == pCgiLabel)
    {
        pAndLabel = strchr(strcmdin, '&');

        if (pAndLabel != NULL)
        {
            if ((pAndLabel - strcmdin) > (MAX_CGI_INTER - 1))
            {
                MLOGE("input cgi cmd length too long: %ld\n", (HI_UL)(pAndLabel - strcmdin));
                return -1;
            }

            strncpy(szCommand, strcmdin,  pAndLabel - strcmdin);
            pCommand = pAndLabel + 1;
        }
        else
        {
            MLOGE("invalid szCommand\n");
            return -1;
        }
    }
    else
    {
        if ((pCgiLabel - strcmdin) > (MAX_CGI_INTER - 1))
        {
            MLOGE("input cgi cmd length too long: %ld\n", (HI_UL)(pAndLabel - strcmdin));
            return -1;
        }

        strncpy(szCommand, strcmdin,  pCgiLabel - strcmdin);
        pCommand = pCgiLabel + sizeof(CGI_POSTFIX) - 1;
    }

    pszAllMalloc = (char*)malloc(MAX_NUM_CMD * MAX_CNT_EVERYCMD * sizeof(char));

    if (pszAllMalloc == NULL)
    {
        MLOGE("malloc the argv size is fail\n");
        return -1;
    }

    memset(pszAllMalloc, 0x00, (MAX_NUM_CMD * MAX_CNT_EVERYCMD * sizeof(char)));

    for (i = 0; i < MAX_NUM_CMD; i++)
    {
        argv[i] = pszAllMalloc + (i * MAX_CNT_EVERYCMD);
    }

    //find the match command in the global array cgi_cmd[]
    for (i = 0; (i < MAX_LEN) && (NULL != astCgiCmd[i].cmdProc); i++)
    {
        if (strstr(astCgiCmd[i].cgi, szCommand) != NULL)
        {
            commandFlag = 1;
            pFunCgiCmdProc = astCgiCmd[i].cmdProc;
            strncpy(aszRequest, astCgiCmd[i].cmd, strlen(astCgiCmd[i].cmd));
            strncpy(method, astCgiCmd[i].method, strlen(astCgiCmd[i].method));
            break;
        }
    }

    if (i >= MAX_LEN||commandFlag != 1)
    {
        MLOGE("can not find the cgi command[%s]\n", strcmdin);
        ret = -1;
        goto exit;
    }

    if (strlen(pCommand) > (sizeof(aszRequest) - strlen(aszRequest)) - 1)
    {
        MLOGE("pCommand len is too long: %lu\n", (HI_UL)strlen(pCommand));
        ret = -1;
        goto exit;
    }

    strncat(aszRequest, pCommand, strlen(pCommand));

#if 0
    //execute cgi by calling icgi function
    if (0 == strcmp(method, CGI_METHOD_CGI))
    {
        ret = CGI_StrCommandParse(aszRequest, &argc, argv, MAX_NUM_CMD, MAX_CNT_EVERYCMD);

        if ( ret == 0 )
        {
            ret = -1;
            goto exit;
        }

        if ((aszRequest[0] != '\0') && ( argc < MAX_NUM_CMD ))
        {
            snprintf(argv[argc], MAX_CNT_EVERYCMD, "\"%s\"", aszRequest);
            argc++;
        }

        printf(" the cgi request is ");

        for (i = 0; i < argc; i++)
        {
            printf(" %s \n", argv[i]);
        }

        if (pFunCgiCmdProc != NULL)
        {
            ret = pFunCgiCmdProc(argc, (const char**)argv, pfnCgiStringGet, pData);

            if (ret != 0)
            {
                ret = -1;
                goto exit;
            }

            ret = 0;
            goto exit;
        }
    }
#endif
    ret = CGI_StrCommandParse(aszRequest, &argc, argv, MAX_NUM_CMD, MAX_CNT_EVERYCMD);

    if (ret != 0)
    {
        ret = -1;
        goto exit;
    }

    for (i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], CGI_URL_PATH_FLAG))
        {
            for (j = i; j < argc; j++)
            {
                argv[j] = argv[j + 2];
            }

            argc = argc - 2;
            break;
        }
    }

    if (pFunCgiCmdProc != NULL)
    {
        ret = pFunCgiCmdProc(argc, (const char**)argv, pfnCgiStringGet, pData);

        if (ret != 0)
        {
            MLOGE("pFunCgiCmdProc fail \n" );
            ret = -1;
            goto exit;
        }
    }

    ret = HI_SUCCESS;
exit:

    if (pszAllMalloc != NULL)
    {
        free(pszAllMalloc);
    }

    pszAllMalloc = NULL;

    return ret;
}

HI_S32 HI_CGI_RegisterCgi(cgi_cmd_trans_s* pstCgiCmd)
{
    HI_S32 i;

    if (NULL == pstCgiCmd->cmdProc)
    {
        MLOGE("stCgiCmd.cmdProc is NULL\n");
        return -1;
    }

    HI_MUTEX_LOCK(g_stCgiLock);

    for (i = 0; i < MAX_LEN; i++)
    {
        if (NULL == astCgiCmd[i].cmdProc)
        {
            memcpy(&astCgiCmd[i], pstCgiCmd, sizeof(cgi_cmd_trans_s));
            break;
        }
    }

    if (i >= MAX_LEN)
    {
        HI_MUTEX_UNLOCK(g_stCgiLock);
        MLOGE("command list is full\n");
        return -1;
    }

    HI_MUTEX_UNLOCK(g_stCgiLock);
    return 0;
}

HI_S32 HI_CGI_RegisterDefaultCmd(HI_VOID)
{
    HI_S32 i;
    HI_MUTEX_LOCK(g_stCgiLock);
    memset(astCgiCmd, 0, MAX_LEN * sizeof(cgi_cmd_trans_s));

    for (i = 0; i < sizeof(s_stDefaultCGI)/sizeof(cgi_cmd_trans_s); i++)
    {
        memcpy(&astCgiCmd[i], &s_stDefaultCGI[i], sizeof(cgi_cmd_trans_s));
    }
    HI_MUTEX_UNLOCK(g_stCgiLock);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
