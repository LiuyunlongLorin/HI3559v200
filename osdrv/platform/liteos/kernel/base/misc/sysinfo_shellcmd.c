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
#include "los_sem.h"
#include "los_mux.h"
#include "los_queue.h"
#include "los_swtmr.h"
#include "string.h"
#include "unistd.h"
#include "los_sem.ph"
#include "los_mux.ph"
#include "los_queue.ph"
#include "los_swtmr.ph"

#include "shcmd.h"
#include "shell.h"

#define SYSINFO_ENABLED(x)  (((x) == YES) ? "YES" : "NO")

UINT32 osShellCmdTaskCntGet(VOID)
{
    UINT32 uwLoop, uwTaskCnt = 0;
    UINTPTR uwIntSave;
    LOS_TASK_CB *pstTaskCB = (LOS_TASK_CB *)NULL;

    uwIntSave = LOS_IntLock();
    for (uwLoop = 0; uwLoop < g_uwTskMaxNum; uwLoop++)
    {
        pstTaskCB = (((LOS_TASK_CB *)g_pstTaskCBArray) + uwLoop);
        if (pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED)
        {
            continue;
        }
        uwTaskCnt++;
    }
    (VOID)LOS_IntRestore(uwIntSave);
    return uwTaskCnt;
}

UINT32 osShellCmdSemCntGet(VOID)
{
    UINT32 uwLoop, uwSemCnt = 0;
    UINTPTR uwIntSave;
    SEM_CB_S *pstSemNode = (SEM_CB_S *)NULL;

    uwIntSave = LOS_IntLock();
    for (uwLoop = 0; uwLoop < LOSCFG_BASE_IPC_SEM_LIMIT; uwLoop++)
    {
        pstSemNode = GET_SEM(uwLoop);
        if (pstSemNode->usSemStat == OS_SEM_USED)
        {
            uwSemCnt++;
        }
    }
    (VOID)LOS_IntRestore(uwIntSave);
    return uwSemCnt;
}

UINT32 osShellCmdMuxCntGet(VOID)
{
    UINT32 uwLoop, uwMuxCnt = 0;
    UINTPTR uwIntSave;
    MUX_CB_S *pstMuxNode = (MUX_CB_S *)NULL;

    uwIntSave = LOS_IntLock();
    for (uwLoop = 0; uwLoop < LOSCFG_BASE_IPC_MUX_LIMIT; uwLoop++)
    {
        pstMuxNode = GET_MUX(uwLoop);
        if (pstMuxNode->ucMuxStat == OS_MUX_USED)
        {
            uwMuxCnt++;
        }
    }
    (VOID)LOS_IntRestore(uwIntSave);
    return uwMuxCnt;
}

UINT32 osShellCmdQueueCntGet(VOID)
{
    UINT32 uwLoop, uwQueueCnt = 0;
    UINTPTR uwIntSave;
    QUEUE_CB_S *pstQueueCB = (QUEUE_CB_S *)NULL;

    uwIntSave = LOS_IntLock();
    pstQueueCB = g_pstAllQueue;
    for (uwLoop = 0; uwLoop < LOSCFG_BASE_IPC_QUEUE_LIMIT; uwLoop++, pstQueueCB++)
    {
        if (pstQueueCB->usQueueState == OS_QUEUE_INUSED)
        {
            uwQueueCnt++;
        }
    }
    (VOID)LOS_IntRestore(uwIntSave);
    return uwQueueCnt;
}

UINT32 osShellCmdSwtmrCntGet(VOID)
{
    UINT32 uwLoop, uwSwTmrCnt = 0;
    UINTPTR uwIntSave;
    SWTMR_CTRL_S *pstSwTmrCB = (SWTMR_CTRL_S *)NULL;

    uwIntSave = LOS_IntLock();
    pstSwTmrCB = m_pstSwtmrCBArray;
    for (uwLoop = 0; uwLoop < LOSCFG_BASE_CORE_SWTMR_LIMIT; uwLoop++, pstSwTmrCB++)
    {
        if (pstSwTmrCB->ucState != OS_SWTMR_STATUS_UNUSED)
        {
            uwSwTmrCnt++;
        }
    }
    (VOID)LOS_IntRestore(uwIntSave);
    return uwSwTmrCnt;
}

LITE_OS_SEC_TEXT_MINOR VOID osShellCmdSystemInfoGet(VOID)
{
    PRINTK("\n   Module    Used      Total     Enabled\n");
    PRINTK("--------------------------------------------\n");
    PRINTK("   Task      %-10d%-10d%s\n",
                                            osShellCmdTaskCntGet(),
                                            LOSCFG_BASE_CORE_TSK_LIMIT,
                                            SYSINFO_ENABLED(YES)); /*lint !e506*/
    PRINTK("   Sem       %-10d%-10d%s\n",
                                            osShellCmdSemCntGet(),
                                            LOSCFG_BASE_IPC_SEM_LIMIT,
                                            SYSINFO_ENABLED(LOSCFG_BASE_IPC_SEM)); /*lint !e506*/
    PRINTK("   Mutex     %-10d%-10d%s\n",
                                            osShellCmdMuxCntGet(),
                                            LOSCFG_BASE_IPC_MUX_LIMIT,
                                            SYSINFO_ENABLED(LOSCFG_BASE_IPC_MUX)); /*lint !e506*/
    PRINTK("   Queue     %-10d%-10d%s\n",
                                            osShellCmdQueueCntGet(),
                                            LOSCFG_BASE_IPC_QUEUE_LIMIT,
                                            SYSINFO_ENABLED(LOSCFG_BASE_IPC_QUEUE)); /*lint !e506*/
    PRINTK("   SwTmr     %-10d%-10d%s\n",
                                            osShellCmdSwtmrCntGet(),
                                            LOSCFG_BASE_CORE_SWTMR_LIMIT,
                                            SYSINFO_ENABLED(LOSCFG_BASE_CORE_SWTMR)); /*lint !e506*/
}

INT32 osShellCmdSystemInfo(INT32 argc, CHAR **argv)
{
    if(argc == 0)
    {
        osShellCmdSystemInfoGet();
        return 0;
    }
    else
    {
        PRINTK("systeminfo: invalid option %s\n"
                "Systeminfo has NO ARGS.\n", argv[0]);
        return -1;
    }
}

SHELLCMD_ENTRY(systeminfo_shellcmd, CMD_TYPE_EX, "systeminfo", 1, (CMD_CBK_FUNC)osShellCmdSystemInfo); /*lint !e19*/
#endif
