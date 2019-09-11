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
#include "securec.h"
#ifdef LOSCFG_SHELL
#include "los_task.h"
#include "los_event.h"
#include "shcmd.h"
#include "los_list.h"
#include "los_printf.h"
#include "time.h"
#include "unistd.h"
#include "los_base.h"
#include "stdlib.h"
#include "stdio.h"
#include "hisoc/uart.h"
#include "shMsg.h"
#ifdef LOSCFG_FS_VFS
#include "console.h"
#endif

extern CMD_MOD_INFO_S m_stCmdInfo;


char* getInputBuf(SHELL_CB *pStShell)
{
    return pStShell->acShellbuf;
}

static void shellNotify(SHELL_CB *pStShell)
{
    (void)LOS_EventWrite(&pStShell->stShellEvent, SHELL_CMD_PARSE_EVENT);
}

void shellCmdLineParse(CHAR c, pf_OUTPUT pf_put, SHELL_CB *pStShell)
{
    CHAR                ch    = 0;
    INT32               swRet = 0;

    enum {
        STAT_NOMAL_KEY,
        STAT_ESC_KEY,
        STAT_MULTI_KEY
    };

#ifdef LOSCFG_FS_VFS
    ch = c;
#endif
#ifdef LOSCFG_PLATFORM_UART_WITHOUT_VFS
    ch = uart_getc();
#endif
    if (pStShell->uwShellBufOffset == 0 && ch != '\n' && ch != '\0')
    {
        (VOID)memset_s(pStShell->acShellbuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    }

    if (ch == '\r')
    {
        if (pStShell->uwShellBufOffset < SHOW_MAX_LEN -1)
        {
            pStShell->acShellbuf[pStShell->uwShellBufOffset] = '\0';
        }
        pStShell->uwShellBufOffset = 0;
        osShellCmdPush(pStShell->acShellbuf, pStShell->pStCmdKeyLink);
        pStShell->pStCmdMask = pStShell->pStCmdKeyLink;
        shellNotify(pStShell);
        return;
    }
    else if (ch == '\b') /* backspace */
    {
        if ((pStShell->uwShellBufOffset > 0) && (pStShell->uwShellBufOffset < SHOW_MAX_LEN - 1))
        {
            pStShell->acShellbuf[pStShell->uwShellBufOffset - 1] = '\0';
            pStShell->uwShellBufOffset--;
            pf_put("\b \b");
        }
        return;
    }
    else if (ch == 0x09)
    {
        if ((pStShell->uwShellBufOffset > 0) && (pStShell->uwShellBufOffset < SHOW_MAX_LEN - 1))
        {
            swRet = osTabCompletion(pStShell->acShellbuf, &pStShell->uwShellBufOffset);
            if (swRet > 1)
            {
                pf_put("Huawei LiteOS # %s", pStShell->acShellbuf);
            }
        }
        return;
    }
    else if (ch == 0x1b)
    {
        pStShell->uwShellKeyValue = STAT_ESC_KEY;
        return;
    }
    else if (ch == 0x5b)
    {
        if (pStShell->uwShellKeyValue == STAT_ESC_KEY)
        {
            pStShell->uwShellKeyValue = STAT_MULTI_KEY;
            return;
        }
    }
    else if (ch == 0x41)/*up*/
    {
        if (pStShell->uwShellKeyValue == STAT_MULTI_KEY)
        {
            shellHistoryShow(CMD_KEY_UP, pStShell);
            pStShell->uwShellKeyValue = STAT_NOMAL_KEY;
            return;
        }
    }
    else if (ch == 0x42)/*down*/
    {
        if (pStShell->uwShellKeyValue == STAT_MULTI_KEY)
        {
            pStShell->uwShellKeyValue = STAT_NOMAL_KEY;
            shellHistoryShow(CMD_KEY_DOWN, pStShell);
            return;
        }
    }
    else if (ch == 0x43)/*right*/
    {
        if (pStShell->uwShellKeyValue == STAT_MULTI_KEY)
        {
            pStShell->uwShellKeyValue = STAT_NOMAL_KEY;
            return;
        }
    }
    else if (ch == 0x44)/*left*/
    {
        if (pStShell->uwShellKeyValue == STAT_MULTI_KEY)
        {
            pStShell->uwShellKeyValue = STAT_NOMAL_KEY;
            return;
        }
    }

    if(ch != '\n' && ch != '\0')
    {
        if (pStShell->uwShellBufOffset < SHOW_MAX_LEN - 1)
        {
            pStShell->acShellbuf[pStShell->uwShellBufOffset] = ch;
        }
        else
        {
            pStShell->acShellbuf[SHOW_MAX_LEN - 1] = '\0';
        }
        pStShell->uwShellBufOffset++;
        pf_put("%c",ch);

    }

    pStShell->uwShellKeyValue = STAT_NOMAL_KEY;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShMsgTypeGet(CMD_PARSED_S *pstCmdParsed, CHAR *pcCmdType)
{
    CMD_ITEM_S *pstCurCmdItem = (CMD_ITEM_S *)NULL;
    UINT32 uwLen = strlen(pcCmdType);

    LOS_DL_LIST_FOR_EACH_ENTRY(pstCurCmdItem, &(m_stCmdInfo.stCmdList.list), CMD_ITEM_S, list)
    {
        if ((uwLen == strlen(pstCurCmdItem->pstCmd->pscCmdKey)) &&
                (0 == strncmp((char *)(pstCurCmdItem->pstCmd->pscCmdKey), pcCmdType, uwLen)) )
        {
            (VOID)memcpy_s((char *)(pstCmdParsed->ascCmdKey), CMD_KEY_LEN, pcCmdType, uwLen < CMD_KEY_LEN ? uwLen : CMD_KEY_LEN);
            pstCmdParsed->enCmdType = pstCurCmdItem->pstCmd->enCmdType;
            pstCmdParsed->bCmdType = FALSE;
            return LOS_OK;
        }
    }


    return OS_INVALID;
}

/***************************************************************************************
 Function:    osShMsgParse
 Description: Parse the string and try to operate the command
 Input:       pMsg : Pass in the original buff string, which is ready to be operated
***************************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 osShMsgParse(UINT32 TrasID,VOID *pMsg)
{
    CHAR *ptmp;
    CHAR *pscMsgName = NULL;
    CHAR *pscOutput = NULL;
    UINT32 uwLoop = 0;
    UINT32 uwLen = 0;
    CMD_PARSED_S stCmdParsed;
    UINT32 uwRet = LOS_OK;
    BOOL uwQuotes = FALSE;
    if (pMsg == NULL)
    {
        uwRet = OS_INVALID;
        goto END;
    }

    uwLen = strlen(pMsg);
    pscOutput = (CHAR *)LOS_MemAlloc(m_aucSysMem0, uwLen + 1);
    if (pscOutput == NULL)
    {
        PRINTK("malloc failure in %s[%d]\n", __FUNCTION__, __LINE__);
        uwRet = OS_INVALID;
        goto END;
    }
    /* Call function 'osCmdKeyShift' to squeeze and clear useless or overmuch space if string buffer */
    uwRet = osCmdKeyShift((char *)pMsg, pscOutput, uwLen + 1);
    if (LOS_OK != uwRet)
    {
        uwRet = OS_INVALID;
        goto END_FREE_OUTPUT;
    }
    if (strlen(pscOutput) == 0)
    {
        uwRet = OS_INVALID;
        goto END_FREE_OUTPUT;
    }

    (VOID)memset_s(&stCmdParsed,sizeof(CMD_PARSED_S), 0, sizeof(CMD_PARSED_S));
    stCmdParsed.bCmdType = TRUE;

    pscMsgName = (CHAR *)LOS_MemAlloc(m_aucSysMem0, uwLen + 1);
    if (pscMsgName == NULL)
    {
        PRINTK("malloc failure in %s[%d]\n", __FUNCTION__, __LINE__);
        uwRet = OS_INVALID;
        goto END_FREE_OUTPUT;
    }
    /* Scan the 'pscOutput' string for command */
    /* Notice: Command string must not have any special name */
    for (ptmp = pscOutput, uwLoop = 0; *ptmp != '\0' && uwLoop < uwLen; )
    {
        /* If reach a double quotes, switch the quotes matching status */
        if(*ptmp=='\"')
        {
            SWITCH_QUOTES_STATUS(uwQuotes);
            /* Ignore the double quote charactor itself */
            ptmp++;
            continue;
        }
        /* If detected a space which the quotes matching status is false */
        /* which said has detected the first space for seperator, finish this scan operation */
        if(*ptmp == ' ' && QUOTES_STATUS_CLOSE(uwQuotes))
            break;
        pscMsgName[uwLoop] = *ptmp++;
        uwLoop++;
    }
    pscMsgName[uwLoop] = '\0';
    /* Scan the command list to check whether the command can be found */
    uwRet = osShMsgTypeGet(&stCmdParsed, pscMsgName);
    PRINTK("\n");
    if (uwRet != LOS_OK)
    {
        PRINTK("%s:command not found", pscMsgName);
        goto END_FREE_MSG;
    }
    else if (uwRet == LOS_OK)
    {
        (VOID)osCmdExec(0, &stCmdParsed, pscOutput);
        goto END_FREE_MSG;
    }

END_FREE_MSG:
    (VOID)LOS_MemFree(m_aucSysMem0, pscMsgName);
END_FREE_OUTPUT:
    (VOID)LOS_MemFree(m_aucSysMem0, pscOutput);
END:
    return uwRet;
}


#ifdef LOSCFG_FS_VFS
LITE_OS_SEC_TEXT_MINOR UINT32 shellEntry(AARCHPTR uwParam)
{
    CHAR ch;
    UINT32 n = 0;
    SHELL_CB *pStShell = (SHELL_CB *)uwParam;

    CONSOLE_CB *pStConsole = get_console_by_id(pStShell->uwConsoleID);
    if (pStConsole == NULL)
    {
        PRINT_ERR("Shell task init error!\n");
        return 1;
    }

    memset(pStShell->acShellbuf, 0, sizeof(pStShell->acShellbuf));

    while(1)
    {
#ifdef LOSCFG_PLATFORM_CONSOLE
        if (!console_is_occupied(pStConsole))
        {
#endif
            /* is console ready for shell ? */
            n = read(pStConsole->fd, &ch, 1);
            if (n == 1){
                shellCmdLineParse(ch,(pf_OUTPUT)dprintf, pStShell);
            }
            if(is_nonblock((void *)pStConsole)){
                LOS_Msleep(50);
            }
#ifdef LOSCFG_PLATFORM_CONSOLE
        }
#endif
    }
}
#endif

LITE_OS_SEC_TEXT_MINOR UINT32 osShellTask(AARCHPTR uwParam1,
                                               AARCHPTR uwParam2,
                                               AARCHPTR uwParam3,
                                               AARCHPTR uwParam4)
{
    UINT32 uwRet;
    CHAR *pscBuf;
    SHELL_CB *pStShell = (SHELL_CB *)uwParam1;

    while (1)
    {
        PRINTK("\nHuawei LiteOS # ");
        uwRet = LOS_EventRead(&pStShell->stShellEvent,
            0xFFF, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, LOS_WAIT_FOREVER);
        if (SHELL_CMD_PARSE_EVENT == uwRet)
        {
            pscBuf = getInputBuf(pStShell);
            (void)osShMsgParse(0, pscBuf);
        }
        else if (CONSOLE_SHELL_KEY_EVENT == uwRet)
        {
            shellKeyDeInit((CMD_KEY_S *)pStShell->pStCmdKeyLink);
            break;
        }
    }
    (void)LOS_EventDestroy(&pStShell->stShellEvent);
    (void)LOS_MemFree((void *)m_aucSysMem0, pStShell);

    return 0;
}

static char *serialshelltask = "SerialShellTask";
static char *serialentrytask = "SerialEntryTask";
static char *telnetshelltask = "TelnetShellTask";
static char *telnetentrytask = "TelnetEntryTask";

LITE_OS_SEC_TEXT_MINOR UINT32 osShellTaskInit(SHELL_CB * pStShell)
{
    char *name;
    TSK_INIT_PARAM_S stInitParam = {0};

    if (pStShell->uwConsoleID == CONSOLE_SERIAL)
    {
        name = serialshelltask;
    }
    else if (pStShell->uwConsoleID == CONSOLE_TELNET)
    {
        name = telnetshelltask;
    }
    else
    {
        return LOS_NOK;
    }

    stInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)osShellTask;
    stInitParam.usTaskPrio   = 9;
    stInitParam.auwArgs[0]   = (AARCHPTR)pStShell;
    stInitParam.uwStackSize  = 0x3000;
    stInitParam.pcName       = name;
    stInitParam.uwResved     = LOS_TASK_STATUS_DETACHED;
    stInitParam.usConsoleID  = pStShell->uwConsoleID;

    (void)LOS_EventInit(&pStShell->stShellEvent);

    return LOS_TaskCreate(&pStShell->uwShellTaskHandle, &stInitParam);
}


LITE_OS_SEC_TEXT_MINOR UINT32 osShellEntryInit(SHELL_CB * pStShell)
{
    UINT32 uwRet = 0;
    char *name = NULL;
    TSK_INIT_PARAM_S stInitParam = {0};

    if (pStShell->uwConsoleID == CONSOLE_SERIAL)
    {
        name = serialentrytask;
    }
    else if (pStShell->uwConsoleID == CONSOLE_TELNET)
    {
        name = telnetentrytask;
    }
    else
    {
        return LOS_NOK;
    }

    stInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)shellEntry;
    stInitParam.usTaskPrio   = 30;
    stInitParam.auwArgs[0]   = (AARCHPTR)pStShell;
    stInitParam.uwStackSize  = 0x1000;
    stInitParam.pcName       = name;
    stInitParam.uwResved     = LOS_TASK_STATUS_DETACHED;
    stInitParam.usConsoleID  = pStShell->uwConsoleID;

    uwRet = LOS_TaskCreate(&pStShell->uwShellEntryHandle, &stInitParam);
#ifdef LOSCFG_PLATFORM_CONSOLE
    console_task_reg(pStShell->uwConsoleID, pStShell->uwShellEntryHandle);
#endif
    return uwRet;
}
#endif //LOSCFG_SHELL
