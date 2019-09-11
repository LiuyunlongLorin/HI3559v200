#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include "hisnet.h"
#include "hisnet_type_define.h"
#include "hisnet_argparser.h"
#include "hisnet_cmd_file.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 Hisnet_CMD_FILE_GetFileNum(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32FileNum = 0;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };
    HISNET_FILE_SVR_CB_S stSvrCB = {};
    memset(&stSvrCB, 0, sizeof(HISNET_FILE_SVR_CB_S));
    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pData, aszOut);
    }

    Hisnet_FILE_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnGetFileNum)
    {
        s32Ret = stSvrCB.pfnGetFileNum(&u32FileNum);

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "var count=\"%d\";\r\n", u32FileNum);
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_REGFUNC_NULL);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_CMD_FILE_GetFileList(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData)
{
    HI_S32 s32Ret = 0;
    HI_U32 i = 0;
    HI_U32 u32Start = 0;
    HI_U32 u32End = 0;
    HI_U32 u32Cnt = 0;
    HISNET_REMOTEFILENAME_S* pstFileList = NULL;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "start",   ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&u32Start), sizeof(HI_U32)
        },
        {
            "end",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&u32End), sizeof(HI_U32)
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pData, aszOut);
    }

    HISNET_FILE_SVR_CB_S stSvrCB = {};

    Hisnet_FILE_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnGetFileList)
    {
        if (u32End >= u32Start)
        {
            pstFileList = (HISNET_REMOTEFILENAME_S*)malloc((u32End - u32Start + 1) * sizeof(HISNET_REMOTEFILENAME_S));
        }
        else
        {
            pstFileList = (HISNET_REMOTEFILENAME_S*)malloc((u32Start - u32End + 1) * sizeof(HISNET_REMOTEFILENAME_S));
        }

        if (NULL == pstFileList)
        {
            MLOGE(" malloc the fileList size fail\n");
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }

        s32Ret = stSvrCB.pfnGetFileList(u32Start, u32End, pstFileList, &u32Cnt);

        if (HI_SUCCESS == s32Ret)
        {
            for (i = 0; i < u32Cnt; i++)
            {
                pfnCGIStringProc(pData, "%s;", pstFileList[i].aszName);
            }

            pfnCGIStringProc(pData, "\r\n");
            HI_APPCOMM_SAFE_FREE(pstFileList);
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            HI_APPCOMM_SAFE_FREE(pstFileList);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_REGFUNC_NULL);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_CMD_FILE_GetFileInfo(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData)
{
    HI_S32 s32Ret = 0;
    HISNET_REMOTEFILENAME_S stFileName;
    FILEMNG_REMOTEFILEINFO_S stFileInfo;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "name",    ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(stFileName.aszName), sizeof(stFileName.aszName)
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pData, aszOut);
    }

    HISNET_FILE_SVR_CB_S stSvrCB = {};

    Hisnet_FILE_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnGetFileInfo)
    {

        s32Ret = stSvrCB.pfnGetFileInfo(&stFileName, &stFileInfo);

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "var size=\"%u\";\r\n"
                             "var path=\"%s\";\r\n"
                             "var time=\"%u\";\r\n"
                             "var create=\"%s\";\r\n",
                             stFileInfo.u32Size,
                             stFileInfo.aszPath,
                             stFileInfo.u32Duration,
                             stFileInfo.aszCreate);
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_REGFUNC_NULL);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_CMD_FILE_DeleteFile(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData)
{
    HI_S32 s32Ret = 0;
    HISNET_REMOTEFILENAME_S stFileName;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "name",    ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(stFileName.aszName), sizeof(stFileName.aszName)
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pData, aszOut);
    }

    HISNET_FILE_SVR_CB_S stSvrCB = {};

    Hisnet_FILE_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnDeleteFile)
    {
        s32Ret = stSvrCB.pfnDeleteFile(&stFileName);

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "Success\r\n");
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_REGFUNC_NULL);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_CMD_FILE_DeleteAllFiles(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData)
{
    HI_S32 s32Ret = 0;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts);

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts))
    {
        Hisnet_Server_ARG_PrintHelp(aszOut, opts);
        pfnCGIStringProc(pData, aszOut);
    }

    HISNET_FILE_SVR_CB_S stSvrCB = {};

    Hisnet_FILE_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnDeleteAllFiles)
    {
        s32Ret = stSvrCB.pfnDeleteAllFiles();

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "Success\r\n");
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_RTN_ERROR);
            return HI_FAILURE;
        }
    }
    else
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_REGFUNC_NULL);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
