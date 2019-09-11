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

#include "stdio.h"
#include "stdlib.h"
#include "los_cpup.h"
#include "los_exc.h"
#include "show.h"
#include "string.h"

#include "los_memstat.ph"
#include "los_sem.ph"
#include "los_task.ph"

#include "shcmd.h"
#include "shell.h"

LITE_OS_SEC_TEXT_MINOR UINT8 *osShellCmdConvertTskStatus(UINT16 usTaskStatus)
{
    if (OS_TASK_STATUS_RUNNING & usTaskStatus)
    {
        {
            return (UINT8 *)"Running";
        }
    }
    else if (OS_TASK_STATUS_READY &  usTaskStatus)
    {
        return (UINT8 *)"Ready";
    }
    else
    {
        if (OS_TASK_STATUS_DELAY &  usTaskStatus)
        {
            return (UINT8 *)"Delay";
        }
        else if (OS_TASK_STATUS_PEND_TIME &  usTaskStatus)
        {
            if (OS_TASK_STATUS_SUSPEND & usTaskStatus)
            {
                return (UINT8 *)"SuspendTime";
            }
            else if (OS_TASK_STATUS_PEND & usTaskStatus)
            {
                return (UINT8 *)"PendTime";
            }
            else if (OS_TASK_STATUS_PEND_QUEUE & usTaskStatus)
            {
                return (UINT8 *)"QueuePendTime";
            }
        }
        else if (OS_TASK_STATUS_PEND & usTaskStatus)
        {
            return (UINT8 *)"Pend";
        }
        else if (OS_TASK_STATUS_PEND_QUEUE & usTaskStatus)
        {
            return (UINT8 *)"QueuePend";
        }
        else if (OS_TASK_STATUS_SUSPEND & usTaskStatus)
        {
            return (UINT8 *)"Suspend";
        }
    }

    return (UINT8 *)"Invalid";
}

UINT32 osShellCmdTaskWaterLineGet(UINT32 uwTaskID)
{
    AARCHPTR *puwStack;
    UINT32 uwPeakUsed;

    if (OS_TASK_MAGIC_WORD == *(AARCHPTR *)(((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->uwTopOfStack)
    {
        puwStack = (AARCHPTR *)((((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->uwTopOfStack + sizeof(AARCHPTR));
        while ((puwStack < (AARCHPTR *)(((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->pStackPointer) && (*puwStack == OS_TASK_STACK_INIT))
        {
            puwStack += 1;
        }
        uwPeakUsed = ((((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->uwStackSize - ((AARCHPTR)puwStack - (((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->uwTopOfStack));
    }
    else
    {
        PRINTK("CURRENT task %s stack overflow!\n", (((LOS_TASK_CB *)g_pstTaskCBArray) + uwTaskID)->pcTaskName);
        uwPeakUsed = 0xFFFFFFFF;
    }
    return uwPeakUsed;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdTskInfoGet(UINT32 uwTaskID, TRANSID_T TransId)
{
    LOS_TASK_CB *pstTaskCB;
    UINT32       uwLoop;
    UINT16       usSemID;

#ifdef LOSCFG_SHELL_EXCINFO
    extern UINT32 g_uwExcInfoIndex ;
    extern UINT32 g_uwRecordSpace;
    extern CHAR *g_pExcInfoBuf;
#endif

#ifdef LOSCFG_KERNEL_CPUP
    UINT32 uwMaxTaskNum = g_uwTskMaxNum;
    CPUP_INFO_S *pstCpu     = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *pstCpu10s  = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *pstCpu1s   = (CPUP_INFO_S *)NULL;
#ifdef LOSCFG_CPUP_INCLUDE_IRQ
    uwMaxTaskNum += OS_HWI_MAX_NUM;
#endif
#endif /* LOSCFG_KERNEL_CPUP */



    if (0xffffffff == uwTaskID)
    {
#ifdef LOSCFG_KERNEL_CPUP

        pstCpu = (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
        if (pstCpu == NULL)
        {
            PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                  g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                         "%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
            }
#endif
            return OS_ERROR;
        }
        memset((VOID *)pstCpu, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);

        pstCpu10s = (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
        if (pstCpu10s == NULL)
        {
            PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
            (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                  g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                         "%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
            }
#endif
            return OS_ERROR;
        }
        memset((VOID *)pstCpu10s, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);

        pstCpu1s= (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
        if (pstCpu1s == NULL)
        {
            PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
            (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
            (VOID)LOS_MemFree(m_aucSysMem0, pstCpu10s);
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                  g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                         "%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
            }
#endif
            return OS_ERROR;
        }
        memset((VOID *)pstCpu1s, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);

        LOS_TaskLock();
        (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu, 0xffff);
        (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu10s, 0);
        (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu1s, 1);
        LOS_TaskUnlock();
#endif

        PRINTK("\r\nName                          TID    Priority   Status       StackSize    WaterLine    StackPoint  TopOfStack   EventMask  SemID");
#ifdef LOSCFG_SHELL_EXCINFO
        if(g_uwRecordSpace > g_uwExcInfoIndex)
        {
              g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                     "\r\nName                          TID    Priority   Status       StackSize    WaterLine    StackPoint  TopOfStack   EventMask  SemID");
        }
#endif
#ifdef LOSCFG_KERNEL_CPUP
        PRINTK(" CPUUSE   CPUUSE10s  CPUUSE1s  ");
#ifdef LOSCFG_SHELL_EXCINFO
        if(g_uwRecordSpace > g_uwExcInfoIndex)
        {
              g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                     " CPUUSE   CPUUSE10s  CPUUSE1s  ");
        }
#endif
#endif /* LOSCFG_KERNEL_CPUP */
        PRINTK(" MEMUSE\n");
        PRINTK("----                          ---    --------   --------     ---------    ----------   ----------  ----------   ---------  -----");
#ifdef LOSCFG_SHELL_EXCINFO
        if(g_uwRecordSpace > g_uwExcInfoIndex)
        {
              g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                     " MEMUSE\n----                          ---    --------   --------     ---------    ----------   ----------  ----------   ---------  -----");
        }
#endif
#ifdef LOSCFG_KERNEL_CPUP
        PRINTK("  ------- ---------  ---------");
#ifdef LOSCFG_SHELL_EXCINFO
        if(g_uwRecordSpace > g_uwExcInfoIndex)
        {
              g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                     "  ------- ---------  ---------");
        }
#endif
#endif /* LOSCFG_KERNEL_CPUP */
        PRINTK("  -------\n");
#ifdef LOSCFG_SHELL_EXCINFO
        if(g_uwRecordSpace > g_uwExcInfoIndex)
        {
              g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                     "  -------\n");
        }
#endif
        for (uwLoop = 0; uwLoop < g_uwTskMaxNum; uwLoop++)
        {
            pstTaskCB = (((LOS_TASK_CB *)g_pstTaskCBArray) + uwLoop);
            if (pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED)
            {
                continue;
            }

            if (pstTaskCB->pTaskSem != NULL)
            {
                usSemID = ((SEM_CB_S *)pstTaskCB->pTaskSem)->usSemID;
            }
            else
            {
                usSemID = 0xFFFF;
            }

            PRINTK("%-30s"
                              "0x%-5x"
                              "%-11d"
                              "%-13s"
                              "0x%-11x"
                              "0x%-11x"
                              " %p "
                              " %p "
                              "0x%-9x"
                              "0x%-7hx",
                              pstTaskCB->pcTaskName,
                              pstTaskCB->uwTaskID,
                              pstTaskCB->usPriority,
                              osShellCmdConvertTskStatus(pstTaskCB->usTaskStatus),
                              pstTaskCB->uwStackSize,
                              osShellCmdTaskWaterLineGet(pstTaskCB->uwTaskID),
                              pstTaskCB->pStackPointer,
                              pstTaskCB->uwTopOfStack,
                              pstTaskCB->uwEventMask,
                              usSemID);
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                      "%-30s"
                      "0x%-5x"
                      "%-11d"
                      "%-13s"
                      "0x%-11x"
                      "0x%-11x"
                      " %p "
                      " %p "
                      "0x%-9x"
                      "0x%-7hx",
                      pstTaskCB->pcTaskName,
                      pstTaskCB->uwTaskID,
                      pstTaskCB->usPriority,
                      osShellCmdConvertTskStatus(pstTaskCB->usTaskStatus),
                      pstTaskCB->uwStackSize,
                      osShellCmdTaskWaterLineGet(pstTaskCB->uwTaskID),
                      pstTaskCB->pStackPointer,
                      pstTaskCB->uwTopOfStack,
                      pstTaskCB->uwEventMask,
                      usSemID);
            }
#endif
#ifdef LOSCFG_KERNEL_CPUP
            PRINTK("%2d.%-7d"
                         "%2d.%-9d"
                         "%2d.%-6d",
                         pstCpu[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                         pstCpu[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT,
                         pstCpu10s[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                         pstCpu10s[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT,
                         pstCpu1s[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                         pstCpu1s[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT);
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                       "%2d.%-7d"
                       "%2d.%-9d"
                       "%2d.%-6d",
                       pstCpu[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                       pstCpu[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT,
                       pstCpu10s[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                       pstCpu10s[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT,
                       pstCpu1s[pstTaskCB->uwTaskID].uwUsage / LOS_CPUP_PRECISION_MULT,
                       pstCpu1s[pstTaskCB->uwTaskID].uwUsage % LOS_CPUP_PRECISION_MULT);
            }
#endif
#endif /* LOSCFG_KERNEL_CPUP */
            PRINTK("%-11d\n",osTaskMemUsage(pstTaskCB->uwTaskID));
#ifdef LOSCFG_SHELL_EXCINFO
            if(g_uwRecordSpace > g_uwExcInfoIndex)
            {
                g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
                      "%-11d\n",osTaskMemUsage(pstTaskCB->uwTaskID));
            }
#endif
        }

#ifdef LOSCFG_KERNEL_CPUP
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu10s);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu1s);
#endif /* LOSCFG_KERNEL_CPUP */
    }
    else
    {
        osTaskBackTrace(uwTaskID);
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdDumpTask(INT32 argc, CHAR **argv)
{
    UINT32 uwTaskID;
    CHAR *pscPtr;

    if (argc < 2)
    {
        if (argc == 0)
        {
            uwTaskID = 0xffffffff;
        }
        else
        {
            uwTaskID = strtoul(argv[0], &pscPtr, 0);
            if (*pscPtr != 0 || (INT32)uwTaskID < 0)
            {
                PRINTK("\ntask ID can't access %s.\n", argv[0]);
                return (UINT32)-1;
            }
        }

        (void)osShellCmdTskInfoGet(uwTaskID,0);
    }
    else
    {
        PRINTK("\nUsage: task or task ID\n");
    }
    return 0;
}

SHELLCMD_ENTRY(task_shellcmd, CMD_TYPE_EX, "task", 1, (CMD_CBK_FUNC)osShellCmdDumpTask); /*lint !e19*/
#endif
