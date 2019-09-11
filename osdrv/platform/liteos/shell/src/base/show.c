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

#include "show.h"
#include "shMsg.h"
#include "shcmd.h"
#include "shFifo.h"
#include "asm/hal_platform_ints.h"
#ifdef LOSCFG_DRIVERS_UART
#include "hisoc/uart.h"
#endif

#ifdef LOSCFG_SHELL

static BOOL g_ShellSourceFlag;

static unsigned int osShellSourceInit(void)
{

    (VOID)osCmdInit();

    return osShellSysCmdRegister();
}

unsigned int osShellInit(int console_id)
{
    UINT32 uwRet = LOS_OK;
    CONSOLE_CB * pStConsole;
    SHELL_CB * pStShell;

    if (g_ShellSourceFlag == 0)
    {
        uwRet = osShellSourceInit();
        if (uwRet == LOS_OK)
        {
            g_ShellSourceFlag = 1;
        }
        else
        {
            return uwRet;
        }
    }

    pStConsole = get_console_by_id(console_id);
    if (pStConsole == NULL)
    {
        return LOS_NOK;
    }

    pStConsole->pStShellHandle = LOS_MemAlloc((void *)m_aucSysMem0, sizeof(SHELL_CB));
    if (pStConsole->pStShellHandle == NULL)
    {
        return LOS_NOK;
    }

    pStShell = (SHELL_CB *)pStConsole->pStShellHandle;

    pStShell->uwConsoleID = console_id;
    shellKeyInit(pStShell);
    (void)strncpy(pStShell->cShellWorkingDirectory, "/", 2);

    uwRet = osShellTaskInit(pStShell);
    if (uwRet != LOS_OK)
    {
        (void)LOS_MemFree((void *)m_aucSysMem0, pStShell);
        return uwRet;
    }

    uwRet = osShellEntryInit(pStShell);
    if (uwRet != LOS_OK)
    {
        (void)LOS_MemFree((void *)m_aucSysMem0, pStShell);
        return uwRet;
    }

    return uwRet;
}

void osShellDeinit(int console_id)
{
    CONSOLE_CB * pStConsole;
    SHELL_CB * pStShell;

    pStConsole = get_console_by_id(console_id);
    if (pStConsole == NULL)
    {
        PRINT_ERR("shell deinit error.\n");
        return;
    }

    pStShell = (SHELL_CB *)pStConsole->pStShellHandle;

    (void)LOS_TaskDelete(pStShell->uwShellEntryHandle);
    (void)LOS_EventWrite(&pStShell->stShellEvent, CONSOLE_SHELL_KEY_EVENT);
}

char * osShellGetWorkingDirtectory(void)
{
    CONSOLE_CB *pStConsole;
    SHELL_CB * pStShell;

    pStConsole = get_console_by_id(g_stLosTask.pstRunTask->usConsoleID);
    pStShell = (SHELL_CB *)pStConsole->pStShellHandle;

    return pStShell->cShellWorkingDirectory;
}

#endif /* LOSCFG_SHELL */
