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
#ifdef LOSCFG_EXC_INTERACTION
#include "los_task.ph"
#include "los_hw.h"
#include "hisoc/uart.h"
#include "console.h"
#ifdef LOSCFG_SHELL
#include "shell.h"
#endif

extern UINT32 osShellTask(UINT32 uwParam1,UINT32 uwParam2,UINT32 uwParam3,UINT32 uwParam4);
extern UINT32 shellEntry(void);

extern VOID hal_interrupt_acknowledge(unsigned int vector);
extern size_t g_curirqnum;

#define OS_EXCINTERACTIONTASK_ID_CHECK(uwTaskID)  (uwTaskID == g_uwIdleTaskID || uwTaskID == pStShell->uwShellTaskHandle || uwTaskID == pStShell->uwShellEntryHandle)

UINT32 osExcInteractionTaskCheck(TSK_INIT_PARAM_S *pstInitParam)
{
    if (pstInitParam->pfnTaskEntry == (TSK_ENTRY_FUNC)osIdleTask)
    {
        return  LOS_OK;
    }
    if (pstInitParam->pfnTaskEntry == (TSK_ENTRY_FUNC)osShellTask || pstInitParam->pfnTaskEntry == (TSK_ENTRY_FUNC)shellEntry)
    {
        return  LOS_OK;
    }
    return LOS_NOK;

}
VOID osExcInteractionTaskKeep(VOID)
{
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;
    UINT32 uwTaskID = 0;
    UINTPTR uvIntSave = LOS_IntLock();
    SHELL_CB * pStShell = (SHELL_CB *)(get_console_by_id(g_stLosTask.pstRunTask->usConsoleID))->pStShellHandle;


    g_usLosTaskLock = 0;
    g_vuwIntCount = 0;
    for (uwTaskID = 0; uwTaskID < g_uwTskMaxNum; uwTaskID++)
    {
        if (uwTaskID == g_stLosTask.pstRunTask->uwTaskID)
        {
           continue;
        }
        else if (OS_EXCINTERACTIONTASK_ID_CHECK(uwTaskID))
        {
            continue;
        }

        pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

        usTempStatus = pstTaskCB->usTaskStatus;

        if (OS_TASK_STATUS_UNUSED & usTempStatus)
        {
            continue;
        }
        (VOID)LOS_TaskDelete(uwTaskID);
    }
    hal_interrupt_init();
    uart_interrupt_unmask();
    hal_interrupt_acknowledge(g_curirqnum);
    (VOID)LOS_TaskDelete(g_stLosTask.pstRunTask->uwTaskID);
    (VOID)LOS_IntRestore(uvIntSave);

    LOS_Schedule();
}
#endif
