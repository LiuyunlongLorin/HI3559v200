/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
#include "los_config.h"

#ifdef LOSCFG_SHELL
#include "shcmd.h"
#include "shMsg.h"

#include "los_event.h"
#include "string.h"
#include "unistd.h"
#include "stdio.h"
#if defined(__LP64__)
#define  timeval64      timeval
#define  gettimeofday64 gettimeofday
#define  ctime64        ctime
#include "time.h"
#else
#include "time64.h"
#endif
#include "securec.h"

typedef struct
{
    BOOL   title;               /* whether to hide the timestamps */
    UINT32 count;               /* the total number of command executions */
    UINT32 interval;            /* running cycle of the command */
    EVENT_CB_S   watch_event;   /* event handle of the watch structure */
    char cmdbuf[CMD_MAX_LEN];   /* the command to watch */
} st_watch;

static st_watch* g_pStWatch;

#define RUN_FOREVER 0xFFFFFFFF

static void print_time(void)
{
    struct timeval64 stNowTime = {0};

    if(gettimeofday64(&stNowTime,NULL) == 0)
    {
       PRINTK("%s",ctime64(&(stNowTime.tv_sec)));
    }
}

static void osShellCmdDoWatch(AARCHPTR arg1)
{
    st_watch * pwatch = (st_watch *)arg1;
    UINT32 ret = 0;
    g_pStWatch = pwatch;

    while(pwatch->count--)
    {
        printf("\033[2J\n");
        if (pwatch->title)
        {
            print_time();
        }
        (void)osShMsgParse(0, pwatch->cmdbuf);
        (void)sleep(pwatch->interval);
        ret = LOS_EventRead(&pwatch->watch_event, 0x01, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, 1);
        if (ret == 0x01)
        {
            break;
        }
    }

    (void)LOS_EventDestroy(&pwatch->watch_event);
    free(g_pStWatch);
    g_pStWatch = NULL;
    PRINTK("\nHuawei LiteOS # ");
}

UINT32 osShellCmdWatch(UINT32 argc, CHAR ** argv)
{
    st_watch * pwatch = NULL;
    UINT32 argcount = 0;
    UINT32 argoff = 0;
    UINT32 ret = 0;
    TSK_INIT_PARAM_S stInitParam = {0};
    UINT32 WatchTaskId = 0;;

    if (argc <= 0)
    {
        PRINTK("\nUsage: watch\n");
        PRINTK("watch [parameter] [command]\n");
        return OS_ERROR;
    }

    if (argc == 1 && strncmp(argv[0], "--over", 6) == 0)
    {
        if (g_pStWatch)
        {
            ret = LOS_EventWrite(&g_pStWatch->watch_event, 0x01);
            if (ret != LOS_OK)
            {
                PRINT_ERR("Write event failed in %s,%d\n", __FUNCTION__, __LINE__);
                return OS_ERROR;
            }
            return LOS_OK;
        }
        else
        {
            PRINTK("No watch task to turn off.\n");
            return OS_ERROR;
        }
    }

    if (g_pStWatch)
    {
        PRINTK("Please turn off previous watch before to start a new watch.\n");
        return OS_ERROR;
    }

    pwatch = (st_watch *)malloc(sizeof(st_watch));
    if (pwatch == NULL)
    {
        PRINTK("Malloc error!\n");
        return OS_ERROR;
    }

    (void)memset_s(pwatch, sizeof(st_watch), 0, sizeof(st_watch));
    pwatch->title = TRUE;
    pwatch->count = RUN_FOREVER;
    pwatch->interval = 1;

    argcount = argc;
    while (argv[argoff][0] == '-')
    {
        if (argcount <= 1)
        {
            goto Watch_Error;
        }

        if (strncmp(argv[argoff], "-n", 2) == 0 || strncmp(argv[argoff], "--interval", 10) == 0)
        {
            if (argcount <= 2)
            {
                goto Watch_Error;
            }
            pwatch->interval = atoi(argv[argoff + 1]);
            if (pwatch->interval == 0)
            {
                PRINTK("\nUsage: watch\n");
                PRINTK("watch [parameter] [command]\n");
                goto Watch_Error;
            }
            argcount -= 2;
            argoff += 2;

        }
        else if (strncmp(argv[argoff], "-t", 2) == 0 || strncmp(argv[argoff], "-no-title", 9) == 0)
        {
            pwatch->title = FALSE;
            argcount--;
            argoff++;
        }
        else if (strncmp(argv[argoff], "-c", 2) == 0 || strncmp(argv[argoff], "--count", 7) == 0)
        {
            if (argcount <= 2)
            {
                goto Watch_Error;
            }
            pwatch->count = atoi(argv[argoff + 1]);
            if (pwatch->count == 0)
            {
                PRINTK("\nUsage: watch\n");
                PRINTK("watch [parameter] [command]\n");
                goto Watch_Error;
            }
            argcount -= 2;
            argoff += 2;
        }
        else
        {
            PRINTK("Unknown option.\n");
            goto Watch_Error;
        }

    }

    while(argc - argoff)
    {
        (void)strcat_s(pwatch->cmdbuf, sizeof(pwatch->cmdbuf), argv[argoff]);
        (void)strcat_s(pwatch->cmdbuf, sizeof(pwatch->cmdbuf), " ");
        argoff++;
    }

    ret = LOS_EventInit(&pwatch->watch_event);
    if (ret != 0)
    {
        PRINT_ERR("Watch event init failed in %s, %d\n", __FUNCTION__, __LINE__);
        goto Watch_Error;
    }

    stInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)osShellCmdDoWatch;
    stInitParam.usTaskPrio   = 10;
    stInitParam.auwArgs[0]   = (AARCHPTR)pwatch;
    stInitParam.uwStackSize  = 0x3000;
    stInitParam.pcName       = "shellcmd_watch";
    stInitParam.uwResved     = LOS_TASK_STATUS_DETACHED;

    ret = LOS_TaskCreate(&WatchTaskId, &stInitParam);
    if (ret != 0)
    {
        PRINT_ERR("Watch task init failed in %s, %d\n", __FUNCTION__, __LINE__);
        goto Watch_Error;
    }

    return LOS_OK;

Watch_Error:
    free(pwatch);
    return OS_ERROR;
}

SHELLCMD_ENTRY(watch_shellcmd, CMD_TYPE_EX, "watch", 0, (CMD_CBK_FUNC)osShellCmdWatch); /*lint !e19*/

#endif
