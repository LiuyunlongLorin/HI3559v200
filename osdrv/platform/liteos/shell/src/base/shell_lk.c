/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
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

#include "shell_lk.h"
#include "unistd.h"
#include "shcmd.h"
#include "stdlib.h"
#include "stdio.h"
#include "securec.h"
#ifdef LOSCFG_SHELL_DMESG
#include "dmesg.ph"
#endif

#ifdef LOSCFG_SHELL_LK
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef enum tagTraceFlag{
    TRACE_EMG = 0,
    TRACE_COMMOM = 1,
    TRACE_ERROR = 2,
    TRACE_WARN = 3,
    TRACE_INFO = 4,
    TRACE_DEBUG = 5,
}TRACE_FLAG;

typedef struct taglogger{
    int module_level;
    int trace_level;
    int fmt_level;
    FILE *fp;
    void *private;
}Logger;

static int g_tracelevel;
static int g_modulelevel;

#define LOG_LIMIT_BUF    1024
static Logger g_Logger = { 0 };

static const char *g_logString[] = {
    "EMG",
    "COMMOM",
    "ERR",
    "WARN",
    "INFO",
    "DEBUG",
};

void osLkDefaultFunc(int l_evel, const char *file, const char *func, int line, const char *fmt, va_list ap);

LK_FUNC osLkHook = (LK_FUNC)osLkDefaultFunc;

static inline int osLkTraceLvGet(void)
{
    return g_tracelevel;
}

const char *osLkCurLogLvGet(void)
{
    return g_logString[g_tracelevel];
}

const char *osLkLogLvGet(int level)
{
    return g_logString[level];
}

void osLkTraceLvSet(int level)
{
    g_tracelevel = level;
    g_Logger.trace_level = level;
    return;
}

void osLkModuleLvSet(unsigned int level)
{
    g_modulelevel = level;
    g_Logger.module_level = level;
    return;
}


int osLkModuleLvGet(void)
{
    return g_modulelevel;
}

void osLkLogFileSet(const char *str)
{
    FILE *fp;

    if (str == NULL)
        return;
    fp = fopen(str, "w+");
    if (fp == NULL)
    {
        printf("Error can't open the %s file\n",str);
        return ;
    }
    else
    {
        printf("set path fp = %p\n", fp);
    }

    g_Logger.fp = fp;
}

FILE *osLogFpGet(void)
{
    return g_Logger.fp;
}

int cmd_log(int argc, char **argv)
{
    int level;
    char *p;

    if (argc < 2 || argc >= 4)
    {
        printf("Usage: %s level <num>\n",argv[0]);
        printf("Usage: %s module <num>\n", argv[0]);
        printf("Usage: %s path <PATH>\n", argv[0]);
        return -1;
    }

    if (!strncmp(argv[1], "level", strlen(argv[1]) + 1))
    {
        level = strtoul(argv[2], &p, 0);
        if (argc == 2 || *p != 0 || level >5 || level < 0)
        {
            printf("current log level %s\n",osLkCurLogLvGet());
            printf("%s %s [num] can access as 0:EMG 1:COMMOM 2:ERROR 3:WARN 4:INFO 5:DEBUG\n",argv[0], argv[1]);
        }
        else
        {
            osLkTraceLvSet(level);
            printf("Set current log level %s\n",osLkCurLogLvGet());
        }
    }
    else if (!strncmp(argv[1], "module", strlen(argv[1]) + 1))
    {
        level = strtoul(argv[2], &p, 0);
        if (argc == 2 || *p != 0 || level >4 || level < 0)
        {
            printf("%s %s can't access %s\n", argv[0], argv[1], argv[3]);
            printf("not support yet\n");
            return -1;
        }
        else
        {
            osLkModuleLvSet(level);
            printf("not support yet\n");
        }
    }
    else if(!strncmp(argv[1], "path", strlen(argv[1]) + 1))
    {
        osLkLogFileSet(argv[2]);
        printf("not support yet\n");
    }
    else
    {
        printf("Usage: %s level <num>\n",argv[0]);
        printf("Usage: %s module <num>\n", argv[0]);
        printf("Usage: %s path <PATH>\n", argv[0]);
        return -1;
    }

    return 0;
}

typedef struct taglogbuf{
    UINT32 wt;
    UINT32 rt;
    UINT32 flags;
    char *logbuf;
}LogBuf;

extern void lk_dprintf(const char *fmt, va_list ap);
#ifdef LOSCFG_SHELL_DMESG
extern void dmesg_dprintf(const char *fmt, va_list ap);
#endif

void osLkDefaultFunc(int l_evel, const char *file, const char *func, int line, const char *fmt, va_list ap)
{
    if (l_evel > osLkTraceLvGet())
    {
#ifdef LOSCFG_SHELL_DMESG
        if((UINT32)l_evel <= osDmesgLvGet())
        {
            if (l_evel != TRACE_COMMOM)
            {
                int tmp_len = strlen(g_logString[l_evel]);
                const char* tmp_ptr = g_logString[l_evel];
                while(tmp_len--)
                    (void)osLogRecord(*tmp_ptr++,NULL);
            }
            dmesg_dprintf(fmt, ap);
        }
#endif
        return;
    }
    if (l_evel != TRACE_COMMOM)
        dprintf("[%s]", g_logString[l_evel]);
    lk_dprintf(fmt, ap);
}

void LOS_LkPrint(int l_evel, const char *file, const char *func, int line, const char *fmt, ...)
{
    va_list ap;
    if (osLkHook != NULL)
    {
        va_start(ap, fmt);
        osLkHook(l_evel, file, func, line, fmt, ap);
        va_end(ap);
    }
}

void LOS_LkRegHook(LK_FUNC hook)
{
    osLkHook = hook;
}

void osLkLoggerInit(const char *str)
{
    (VOID)memset_s(&g_Logger, sizeof(Logger), 0,sizeof(Logger));
    osLkTraceLvSet(TRACE_DEFAULT);
    LOS_LkRegHook(osLkDefaultFunc);
#ifdef LOSCFG_SHELL_DMESG
    (void)LOS_DmesgLvSet(TRACE_DEFAULT);
#endif
}

SHELLCMD_ENTRY(log_shellcmd, CMD_TYPE_STD, "log", 1, (CMD_CBK_FUNC)cmd_log); /*lint !e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
