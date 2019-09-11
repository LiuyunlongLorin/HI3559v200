#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hisnet.h"
#include "hisnet_type_define.h"
#include "hisnet_argparser.h"
#include "hisnet_cmd_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 Hisnet_PrintCmdParam_sys(int argc, const char* argv[])
{
    int i;
    printf("\n");

    for (i = 0; i < argc; i++)
    {
        printf("argv[%d]=%s\n", i, argv[i]);
    }

    printf("\n");
    return HI_SUCCESS;
}


HI_S32 Hisnet_CMD_SYS_DevAttr(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HI_CHAR _devname[HISNET_MAX_CAMERANAME] = {0};
    HI_U32 _devmodel = 0;
    HI_U32 _timeout = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, "get|set", HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            "name", ARG_TYPE_STRING, NULL, HI_FALSE, {0}, ARG_S4"Device name\r\n",
            (HI_VOID*)_devname, HISNET_MAX_CAMERANAME
        },

        {
            "model", ARG_TYPE_INT, NULL, HI_FALSE, {0}, ARG_S4"Device model\r\n",
            (HI_VOID*)& _devmodel, sizeof(_devmodel)
        },

        {
            "timeout", ARG_TYPE_INT, NULL, HI_FALSE, {0}, ARG_S4"Device timeout\r\n",
            (HI_VOID*)& _timeout, sizeof(_timeout)
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    HISNET_DEVICE_ATTR_S stDevAttr;
    HISNET_SYS_SVR_CB_S stSvrCB = {};

    memset(&stDevAttr, 0, sizeof(stDevAttr));
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));

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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("act", opts))
    {
        if (0 == strncmp(_act, "get", strlen("get")))
        {
            if (HI_NULL != stSvrCB.pfnGetDevAttr)
            {
                s32Ret = stSvrCB.pfnGetDevAttr(&stDevAttr);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "var model=\"%s\";\r\n"
                                     "var softversion=\"%s\";\r\n",
                                     stDevAttr.aszDevModel,
                                     stDevAttr.aszSoftwareVersion);
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
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 Hisnet_CMD_SYS_GetSDStatus(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, "get|set", HI_FALSE, {0},
            ARG_S4 "Must option!!! Open or close Audio and vedio encode channel.\r\n"
            ARG_S4 "Range of value,open|close.\r\n"
            ARG_S4 "Example : \r\n"
            ARG_S4 ARG_S4"> encode -chn 0 -act open -type both\r\n"
            ARG_S4 ARG_S4"> encode 0 open both\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    HISNET_SDSTATUS_S stSDStatus;
    HISNET_SYS_SVR_CB_S stSvrCB = {};

    memset(&stSDStatus, 0, sizeof(stSDStatus));
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));
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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("act", opts))
    {
        if (0 == strncmp(_act, "get", strlen("get")))
        {

            if (HI_NULL != stSvrCB.pfnGetSDStatus)
            {
                s32Ret = stSvrCB.pfnGetSDStatus(&stSDStatus);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "var sdstatus=\"%d\";\r\n"
                                     "var sdfreespace=\"%d\";\r\n"
                                     "var sdtotalspace=\"%d\";\r\n"
                                     "var partitionnum=\"%d\";\r\n",
                                     stSDStatus.enSdStatus, \
                                     stSDStatus.s32SDFreeSpace, \
                                     stSDStatus.s32TotalSpace, \
                                     stSDStatus.s32PartitionNum);
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
        }
        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}



HI_S32 Hisnet_CMD_SYS_SDCommand(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32Ret1 = 0;
    HISNET_SYS_SDCOMMAND_S stSDCommand;
    HI_S32 _partition = 0;
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts1[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "mount",     ARG_TYPE_NO_OPT, NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    ARG_OPT_S opts2[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "umount",     ARG_TYPE_NO_OPT, NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };
    ARG_OPT_S opts3[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "format",     ARG_TYPE_NO_OPT, NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },
        {
            "partition",   ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&_partition), sizeof(_partition)
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    HISNET_SYS_SVR_CB_S stSvrCB = {};
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));
    memset(&stSDCommand, 0, sizeof(HISNET_SYS_SDCOMMAND_S));
    s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts1);

    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts2);
        s32Ret1 = 1;

        if (HI_SUCCESS != s32Ret)
        {
            s32Ret = Hisnet_Server_ARG_Parser(argc, argv, opts3);
            s32Ret1 = 2;
        }
    }

    if (HI_SUCCESS != s32Ret)
    {
        pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_ARGPARSER_ERROR);
        return HI_FAILURE;
    }

    switch (s32Ret1)
    {
        case 0:
            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts1))
            {
                Hisnet_Server_ARG_PrintHelp(aszOut, opts1);
                pfnCGIStringProc(pData, aszOut);
            }

            Hisnet_SYS_GetCallBackFunction(&stSvrCB);

            stSDCommand.enSDCommand = HISNET_SDCOMMAND_MOUNT;

            if (HI_NULL != stSvrCB.pfnSDCommand)
            {
                s32Ret = stSvrCB.pfnSDCommand(&stSDCommand);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", s32Ret);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

            break;

        case 1:
            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts2))
            {
                Hisnet_Server_ARG_PrintHelp(aszOut, opts2);
                pfnCGIStringProc(pData, aszOut);
            }

            Hisnet_SYS_GetCallBackFunction(&stSvrCB);

            stSDCommand.enSDCommand = HISNET_SDCOMMAND_UMOUNT;

            if (HI_NULL != stSvrCB.pfnSDCommand)
            {
                s32Ret = stSvrCB.pfnSDCommand(&stSDCommand);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", s32Ret);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

            break;

        case 2:

            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("?", opts3))
            {
                Hisnet_Server_ARG_PrintHelp(aszOut, opts3);
                pfnCGIStringProc(pData, aszOut);
            }

            Hisnet_SYS_GetCallBackFunction(&stSvrCB);

            stSDCommand.enSDCommand = HISNET_SDCOMMAND_FORMAT;
            stSDCommand.s32Partition = _partition;

            if (HI_NULL != stSvrCB.pfnSDCommand)
            {
                s32Ret = stSvrCB.pfnSDCommand(&stSDCommand);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", s32Ret);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

            break;
    }

    /*for sd web refresh*/
    HISNET_SDSTATUS_S stSDStatus;

    if (NULL != stSvrCB.pfnGetSDStatus)
    {
        s32Ret = stSvrCB.pfnGetSDStatus(&stSDStatus);

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "var sdstatus=\"%d\";\r\n"
                             "var sdfreespace=\"%d\";\r\n"
                             "var sdtotalspace=\"%d\";\r\n"
                             "var partitionnum=\"%d\";\r\n",
                             stSDStatus.enSdStatus, \
                             stSDStatus.s32SDFreeSpace, \
                             stSDStatus.s32TotalSpace, \
                             stSDStatus.s32PartitionNum);
        }
    }

    return HI_SUCCESS;
}


HI_S32 Hisnet_CMD_SYS_SysTimeCfg(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HI_U32 _timeformat = 0;
    HI_CHAR _time[HISNET_MAX_TIMESIZE] = {0};
    HI_CHAR _timezone[HISNET_MAX_TIMESIZE] = {0};
    HI_CHAR _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, "get|set", HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },
        {
            "time",    ARG_TYPE_NO_OPT | ARG_TYPE_STRING, NULL, HI_FALSE, {0}, ARG_S4"print help msg\r\n",
            (HI_VOID*)_time, HISNET_MAX_TIMESIZE
        },
        {
            "timeformat",     ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&_timeformat), sizeof(_timeformat)
        },
        {
            "timezone",     ARG_TYPE_NO_OPT | ARG_TYPE_STRING, NULL, HI_FALSE, {0},
            ARG_S4"ptz prepoint name\r\n", (HI_VOID*)(_timezone), HISNET_MAX_TIMESIZE
        },

        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    HISNET_TIME_ATTR_S stSysTime;
    HISNET_SYS_SVR_CB_S stSvrCB = {};

    memset(&stSysTime, 0, sizeof(stSysTime));
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));

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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);


    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("act", opts))
    {
        if (0 == strncmp(_act, "set", strlen("set")))
        {
            stSvrCB.pfnGetSysTime(&stSysTime);
            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("time", opts))
            {
                snprintf(stSysTime.aszTime,HISNET_MAX_TIMESIZE,"%s",_time);
            }
            else
            {
                MLOGD("time is null\n");
            }

            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("timeformat", opts))
            {
                stSysTime.enTimeFormat = (HISNET_TIMEFORMAT_E)_timeformat;
            }
            else
            {
                MLOGD("timeformat is null\n");
            }

            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("timezone", opts))
            {
                snprintf(stSysTime.aszTimeZone,HISNET_MAX_TIMESIZE,"%s",_timezone);
            }
            else
            {
                MLOGD("timezone is null\n");
            }

            if (HI_NULL != stSvrCB.pfnSetSysTime)
            {
                s32Ret = stSvrCB.pfnSetSysTime(&stSysTime);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "Success\r\n");
                }
                else
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

        }
        else if (0 == strncmp(_act, "get", strlen("get")))
        {
            if (HI_NULL != stSvrCB.pfnGetSysTime)
            {
                s32Ret = stSvrCB.pfnGetSysTime(&stSysTime);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "var time=\"%s\";\r\n"
                                     "var timeformat=\"%d\";\r\n"
                                     "var timezone=\"%s\";\r\n",
                                     stSysTime.aszTime,
                                     stSysTime.enTimeFormat,
                                     stSysTime.aszTimeZone);
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

        }

        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


HI_S32 Hisnet_CMD_SYS_Reset(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
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

    HISNET_SYS_SVR_CB_S stSvrCB = {};
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));

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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnReset)
    {
        s32Ret = stSvrCB.pfnReset();

        if (HI_SUCCESS == s32Ret)
        {
            //pfnCGIStringProc(pData,"SvrFuncResult=\"%d\"\r\n",HISNET_SVR_RTN_SUCCESS);
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



HI_S32 Hisnet_CMD_SYS_GetBatteryStatus(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HISNET_BATTERY_STATUS_S stBatteryStatus;
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

    HISNET_SYS_SVR_CB_S stSvrCB = {};
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));

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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);

    if (HI_NULL != stSvrCB.pfnGetBatteryStatus)
    {
        s32Ret = stSvrCB.pfnGetBatteryStatus(&stBatteryStatus);

        if (HI_SUCCESS == s32Ret)
        {
            pfnCGIStringProc(pData, "var capacity=\"%d\";\r\n"
                             "var charge=\"%d\";\r\n"
                             "var ac=\"%d\";\r\n",
                             stBatteryStatus.s32Capacity,
                             stBatteryStatus.bCharge,
                             stBatteryStatus.bAC);
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

HI_S32 Hisnet_CMD_SYS_WifiAttrCfg(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData)
{
    HI_S32 s32Ret = 0;
    HI_U32   _wifiEnable = 0;
    HI_CHAR  _wifiSsid[HISNET_MAX_WIFI_SSID_LEN] = {0};
    HI_U32   _channel = 0;
    HI_CHAR  _act[HISNET_MAX_ACT_LEN] = {0};
    HI_CHAR aszOut[64] = {0};
    ARG_OPT_S opts[] =
    {
        {
            "?",       ARG_TYPE_NO_PARA | ARG_TYPE_SINGLE, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", NULL, 0
        },

        {
            "act",     ARG_TYPE_MUST | ARG_TYPE_NO_OPT | ARG_TYPE_STRING, "get|set", HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(_act), HISNET_MAX_ACT_LEN
        },

        {
            "enablewifi", ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&_wifiEnable), sizeof(_wifiEnable)
        },
        {
            "wifissid", ARG_TYPE_NO_OPT | ARG_TYPE_STRING, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(_wifiSsid), HISNET_MAX_WIFI_SSID_LEN
        },
        {
            "wifichannel",    ARG_TYPE_NO_OPT | ARG_TYPE_INT, HI_NULL, HI_FALSE, {0},
            ARG_S4"print help msg\r\n", (HI_VOID*)(&_channel), sizeof(_channel)
        },
        {
            "END",     ARG_TYPE_END, NULL, HI_FALSE, {0},
            ARG_S4"END\n", NULL, 0
        },
    };

    HISNET_WIFI_ATTR_S stWIFIAttr;
    HISNET_SYS_SVR_CB_S stSvrCB = {};

    Hisnet_PrintCmdParam_sys(argc, argv);
    memset(&stWIFIAttr, 0, sizeof(stWIFIAttr));
    memset(&stSvrCB, 0, sizeof(HISNET_SYS_SVR_CB_S));
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

    Hisnet_SYS_GetCallBackFunction(&stSvrCB);

    if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("act", opts))
    {
        if (0 == strncmp(_act, "set", strlen("set")))
        {
            if (NULL != stSvrCB.pfnGetWifiAttr)
            {
                s32Ret = stSvrCB.pfnGetWifiAttr(&stWIFIAttr);

                if (HI_SUCCESS != s32Ret)
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("enablewifi", opts))
            {
                stWIFIAttr.bEnable = (HI_BOOL)_wifiEnable;
            }
            else
            {
                MLOGD("enablewifi is null!!!\n");
            }

            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("wifissid", opts))
            {
                strncpy(stWIFIAttr.aszSSID, _wifiSsid, HISNET_MAX_WIFI_SSID_LEN - 1);
            }
            else
            {
                MLOGD("wifissid is null!!!\n");
            }
            if (HI_SUCCESS == Hisnet_Server_ARG_OptIsSet("wifichannel", opts))
            {
                stWIFIAttr.u32Channel = _channel;
            }
            else
            {
                MLOGD("wifichannel is null!!!\n");
            }

            if (HI_NULL != stSvrCB.pfnSetWifiAttr)
            {
                s32Ret = stSvrCB.pfnSetWifiAttr(&stWIFIAttr);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "Success\r\n");
                }
                else
                {
                    pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_RTN_ERROR);
                    return HI_FAILURE;
                }
            }
            else
            {
                pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"", HISNET_SVR_REGFUNC_NULL);
                return HI_FAILURE;
            }

        }

        else if (0 == strncmp(_act, "get", strlen("get")))
        {
            if (HI_NULL != stSvrCB.pfnGetWifiAttr)
            {
                s32Ret = stSvrCB.pfnGetWifiAttr(&stWIFIAttr);

                if (HI_SUCCESS == s32Ret)
                {
                    pfnCGIStringProc(pData, "var enablewifi=\"%d\";\r\n"
                                     "var wifissid=\"%s\";\r\n"
                                     "var wifichannel=\"%d\";\r\n"
                                     "var links=\"%d\";\r\n",
                                     stWIFIAttr.bEnable, \
                                     stWIFIAttr.aszSSID, \
                                     stWIFIAttr.u32Channel, \
                                     stWIFIAttr.u32Links);
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

        }

        else
        {
            pfnCGIStringProc(pData, "SvrFuncResult=\"%d\"\r\n", HISNET_SVR_CMD_INVALID);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
