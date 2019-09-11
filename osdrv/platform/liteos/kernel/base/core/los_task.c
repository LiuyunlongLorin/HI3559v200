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

#include "los_task.inc"
#include "los_base.ph"
#include "los_memory.ph"
#include "los_priqueue.ph"
#include "los_sem.ph"
#include "los_hw.ph"
#include "los_exc.h"
#include "los_memstat.ph"
#ifdef LOSCFG_KERNEL_RUNSTOP
#include "los_runstop.ph"
#endif
#ifdef LOSCFG_KERNEL_TICKLESS
#include "los_tickless.h"
#include "los_tickless.ph"
#endif
#ifdef LOSCFG_KERNEL_CPUP
#include "los_cpup.ph"
#endif
#if (LOSCFG_BASE_CORE_SWTMR == YES)
#include "los_swtmr.ph"
#endif
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif
#ifdef LOSCFG_TASK_STACK_PROTECT
#include "hisoc/mmu_config.h"
extern SENCOND_PAGE stAppPage;
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

LITE_OS_SEC_BSS  LOS_TASK_CB                         *g_pstTaskCBArray;
__attribute__((section(".systcb")))LOS_TASK_CB     g_pstTaskCB_0;
LITE_OS_SEC_BSS  ST_LOS_TASK                         g_stLosTask;
LITE_OS_SEC_BSS  size_t                              g_usLosTaskLock;
LITE_OS_SEC_BSS  UINT32                                  g_uwTskMaxNum;
LITE_OS_SEC_BSS  UINT32                                  g_uwIdleTaskID;
LITE_OS_SEC_BSS  UINT32                                  g_uwSwtmrTaskID;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_stLosFreeTask;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_stTskRecyleList;
LITE_OS_SEC_BSS  SORTLINK_ATTRIBUTE_S               g_stTskSortLink;
LITE_OS_SEC_BSS  BOOL                                g_bTaskScheduled;

#if (LOSCFG_BASE_CORE_TSK_MONITOR == YES)
TSKSWITCHHOOK g_pfnTskSwitchHook = (TSKSWITCHHOOK)NULL; /*lint !e611*/
TSKSWITCHHOOK g_pfnUsrTskSwitchHook = (TSKSWITCHHOOK)NULL; /*lint !e611*/
#endif /* LOSCFG_BASE_CORE_TSK_MONITOR == YES */

#if (OS_PERF_TSK_FILTER == YES)
TSKSWITCHHOOK g_pfnPerfTskFilterHook = NULL;
#endif

extern CHAR LOS_BLOCK_START(systcb);
extern CHAR LOS_BLOCK_END(systcb);
/*****************************************************************************
 Function : osTskIdleBGD
 Description : Idle background.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT VOID osIdleTask(VOID)
{

    while (1)
    {
#ifdef LOSCFG_KERNEL_RUNSTOP
        if (TRUE == g_bSysdoneFlag)
        {
            osSystemSuspend();
        }
#endif

#ifdef LOSCFG_KERNEL_TICKLESS
        if (g_bTickIrqFlag)
        {
            g_bTickIrqFlag = 0;
            osTicklessStart(LOSCFG_BASE_CORE_TICK_PER_SECOND);
        }

#endif
        wfi();

    }
}

/*****************************************************************************
 Function : osTaskPriModify
 Description : Change task priority.
 Input       : pstTaskCB    --- task control block
                 usPriority      --- priority
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID osTaskPriModify(LOS_TASK_CB *pstTaskCB, UINT16 usPriority)
{
    if (pstTaskCB->usTaskStatus & OS_TASK_STATUS_READY)
    {
        osPriqueueDequeue(&pstTaskCB->stPendList);
        pstTaskCB->usPriority = usPriority;
        osPriqueueEnqueue(&pstTaskCB->stPendList, pstTaskCB->usPriority);
    }
    else
    {
        pstTaskCB->usPriority = usPriority;
    }
}

/*****************************************************************************
 Function : osTaskAdd2TimerList
 Description : Add task to sorted delay list.
 Input       : pstTaskCB    --- task control block
               uwTimeout    --- wait time, ticks
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT VOID osTaskAdd2TimerList(LOS_TASK_CB *pstTaskCB, UINT32 uwTimeout)
{
    SET_SORTLIST_VALUE(&(pstTaskCB->stSortList), uwTimeout);
    osAdd2SortLink(&g_stTskSortLink, &pstTaskCB->stSortList);
}


LITE_OS_SEC_TEXT VOID osTimerListDelete(LOS_TASK_CB *pstTaskCB)
{
    osDeleteSortLink(&g_stTskSortLink, &pstTaskCB->stSortList);
}

LITE_OS_SEC_TEXT VOID osTaskScan(VOID)
{
    SORTLINK_LIST *pstSortList;
    LOS_TASK_CB *pstTaskCB;
    BOOL bNeedSchedule = FALSE;
    UINT16 usTempStatus;
    LOS_DL_LIST *pstListObject;

    g_stTskSortLink.usCursor = (g_stTskSortLink.usCursor + 1) & OS_TSK_SORTLINK_MASK;
    pstListObject = g_stTskSortLink.pstSortLink + g_stTskSortLink.usCursor;

    if(LOS_ListEmpty(pstListObject))
    {
        return;
    }
    pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
    UWROLLNUMDEC(pstSortList->uwIdxRollNum);

    while (UWROLLNUM(pstSortList->uwIdxRollNum) == 0)
    {
        LOS_ListDelete(&pstSortList->stList);
        pstTaskCB = LOS_DL_LIST_ENTRY(pstSortList, LOS_TASK_CB, stSortList);
        pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_PEND_TIME);
        usTempStatus = pstTaskCB->usTaskStatus;
        if (OS_TASK_STATUS_PEND & usTempStatus)
        {
            pstTaskCB->usTaskStatus &= ~(OS_TASK_STATUS_PEND);
            pstTaskCB->usTaskStatus |= OS_TASK_STATUS_TIMEOUT;
            LOS_ListDelete(&pstTaskCB->stPendList);
            pstTaskCB->pTaskSem = NULL;
            pstTaskCB->pTaskMux = NULL;
        }
        else if (OS_TASK_STATUS_EVENT & usTempStatus)
        {
            pstTaskCB->usTaskStatus &= ~(OS_TASK_STATUS_EVENT);
            pstTaskCB->usTaskStatus |= OS_TASK_STATUS_TIMEOUT;
        }
        else if (OS_TASK_STATUS_PEND_QUEUE & usTempStatus)
        {
            LOS_ListDelete(&pstTaskCB->stPendList);
            pstTaskCB->usTaskStatus &= ~(OS_TASK_STATUS_PEND_QUEUE);
            pstTaskCB->usTaskStatus |= OS_TASK_STATUS_TIMEOUT;

        }
        else
        {
            pstTaskCB->usTaskStatus &= ~(OS_TASK_STATUS_DELAY);
        }

        if (!((OS_TASK_STATUS_SUSPEND) & usTempStatus))
        {
            pstTaskCB->usTaskStatus |= OS_TASK_STATUS_READY;
            osPriqueueEnqueue(&pstTaskCB->stPendList, pstTaskCB->usPriority);
            bNeedSchedule = TRUE;
        }

        pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
    }

    if (bNeedSchedule)
    {
        LOS_Schedule();
    }
}

/*****************************************************************************
 Function : osTaskInit
 Description : Task init function.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
extern CHAR LOS_CB_START(systcb);
extern CHAR LOS_CB_END(systcb);
LITE_OS_SEC_TEXT_INIT UINT32 osTaskInit(VOID) /*lint -e661 -e662 -e416*/
{
    UINT32 uwIndex;
    UINT32 uwRet;

    g_uwTskMaxNum = (&LOS_BLOCK_END(systcb) - &LOS_BLOCK_START(systcb)) / sizeof(LOS_TASK_CB) - 1; /*lint !e573*/
    g_pstTaskCBArray = (LOS_TASK_CB *)&LOS_BLOCK_START(systcb);
    (VOID)memset(g_pstTaskCBArray, 0, (&LOS_BLOCK_END(systcb) - &LOS_BLOCK_START(systcb)));

    LOS_ListInit(&g_stLosFreeTask);
    LOS_ListInit(&g_stTskRecyleList);
    for (uwIndex = 0; uwIndex < g_uwTskMaxNum; uwIndex++)
    {
        g_pstTaskCBArray[uwIndex].usTaskStatus = OS_TASK_STATUS_UNUSED;
        g_pstTaskCBArray[uwIndex].uwTaskID = uwIndex;
        LOS_ListTailInsert(&g_stLosFreeTask, &g_pstTaskCBArray[uwIndex].stPendList);
    }

    (VOID)memset((void *)(&g_stLosTask), 0, sizeof(g_stLosTask));
    g_stLosTask.pstRunTask = &g_pstTaskCBArray[g_uwTskMaxNum];
    g_stLosTask.pstRunTask->uwTaskID = uwIndex;
    g_stLosTask.pstRunTask->usTaskStatus = (OS_TASK_STATUS_UNUSED | OS_TASK_STATUS_RUNNING);
    g_stLosTask.pstRunTask->usPriority = OS_TASK_PRIORITY_LOWEST + 1;
    osPriqueueInit();

    uwRet = osSortLinkInit(&g_stTskSortLink);
    if(uwRet!= LOS_OK)
    {
        return LOS_ERRNO_TSK_NO_MEMORY;
    }

    return LOS_OK;
}/*lint +e661 +e662 +e416*/


/*****************************************************************************
 Function : osIdleTaskCreate
 Description : Create idle task.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 osIdleTaskCreate(VOID)
{
    UINT32 uwRet;
    TSK_INIT_PARAM_S stTaskInitParam;

    (VOID)memset((void *)(&stTaskInitParam), 0, sizeof(TSK_INIT_PARAM_S));
    stTaskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)osIdleTask;
    stTaskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
    stTaskInitParam.pcName = "IdleCore000";
    stTaskInitParam.usTaskPrio = OS_TASK_PRIORITY_LOWEST;
    uwRet = LOS_TaskCreate(&g_uwIdleTaskID, &stTaskInitParam);

    return uwRet;
}

/*****************************************************************************
 Function : LOS_CurTaskIDGet
 Description : get id of current running task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_CurTaskIDGet(VOID)
{
    if (NULL == g_stLosTask.pstRunTask)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }
    return g_stLosTask.pstRunTask->uwTaskID;
}

/*****************************************************************************
 Function : osTaskSelfDelete
 Description : Delete self task
 Input       : uwTaskID --- Task ID, uwIntSave
                 uvIntSave   --- interrupt flag
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 osTaskSelfDelete(UINT32 uwTaskID, UINTPTR uvIntSave)
{
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    usTempStatus = pstTaskCB->usTaskStatus;
    if (OS_TASK_STATUS_READY & usTempStatus)
    {
        osPriqueueDequeue(&pstTaskCB->stPendList);
        pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_READY);
    }
    else if ((OS_TASK_STATUS_PEND | OS_TASK_STATUS_PEND_QUEUE) & usTempStatus)
    {
        LOS_ListDelete(&pstTaskCB->stPendList);
        if ((OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME) & usTempStatus)
        {
            osTimerListDelete(pstTaskCB);
        }
    }

    pstTaskCB->usTaskStatus &= (~(OS_TASK_STATUS_SUSPEND));
    pstTaskCB->usTaskStatus |= OS_TASK_STATUS_UNUSED;
    pstTaskCB->uwEvent.uwEventID = 0xFFFFFFFF;
    pstTaskCB->uwEventMask = 0;
#ifdef LOSCFG_KERNEL_CPUP
    (VOID)memset((VOID *)&g_pstCpup[pstTaskCB->uwTaskID], 0, sizeof(OS_CPUP_S));
#endif
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
    OS_MEM_CLEAR(uwTaskID);
#endif
    g_stLosTask.pstNewTask = LOS_DL_LIST_ENTRY(osPriqueueTop(), LOS_TASK_CB, stPendList); /*lint !e413*/
    if (OS_TASK_STATUS_RUNNING & pstTaskCB->usTaskStatus)
    {
        LOS_ListTailInsert(&g_stTskRecyleList, &pstTaskCB->stPendList);
        g_stLosTask.pstRunTask = &g_pstTaskCBArray[g_uwTskMaxNum];
        g_stLosTask.pstRunTask->uwTaskID = uwTaskID;
        g_stLosTask.pstRunTask->usTaskStatus = pstTaskCB->usTaskStatus;
        g_stLosTask.pstRunTask->uwTopOfStack = pstTaskCB->uwTopOfStack;
        g_stLosTask.pstRunTask->pcTaskName = pstTaskCB->pcTaskName;
        pstTaskCB->usTaskStatus = OS_TASK_STATUS_UNUSED;
        (VOID)LOS_IntRestore(uvIntSave);
        osSchedule();
        return LOS_OK;
    }
    else if (OS_TASK_STATUS_UNUSED & pstTaskCB->usTaskStatus)
    {
        (VOID)LOS_IntRestore(uvIntSave);
        osSchedule();
        return LOS_OK;
    }

    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;
}

/*****************************************************************************
 Function : osTaskSwitchCheck
 Description : Check task switch
 Input       : Node
 Output      : None
 Return      : None
 *****************************************************************************/
#if (LOSCFG_BASE_CORE_TSK_MONITOR == YES)
LITE_OS_SEC_TEXT static VOID osTaskStackCheck(VOID)
{
    if ((*(AARCHPTR *)(g_stLosTask.pstRunTask->uwTopOfStack)) != OS_TASK_MAGIC_WORD)
    {
        PRINT_ERR("CURRENT task ID: %s:%d stack overflow!\n", g_stLosTask.pstRunTask->pcTaskName, g_stLosTask.pstRunTask->uwTaskID);
    }
    if (((AARCHPTR)(g_stLosTask.pstNewTask->pStackPointer) <= g_stLosTask.pstNewTask->uwTopOfStack) ||
        ((AARCHPTR)(g_stLosTask.pstNewTask->pStackPointer) > g_stLosTask.pstNewTask->uwTopOfStack + g_stLosTask.pstNewTask->uwStackSize))
    {
        PRINT_ERR("HIGHEST task ID: %s:%d SP error!\n", g_stLosTask.pstNewTask->pcTaskName, g_stLosTask.pstNewTask->uwTaskID);
        PRINT_ERR("HIGHEST task StackPointer: %p TopOfStack: %p\n",  g_stLosTask.pstNewTask->pStackPointer,  g_stLosTask.pstNewTask->uwTopOfStack);
    }

    if (g_pfnUsrTskSwitchHook != NULL)
    {
        g_pfnUsrTskSwitchHook();
    }
}

LITE_OS_SEC_TEXT_MINOR VOID osTaskMonInit(VOID)
{
    g_pfnTskSwitchHook = osTaskStackCheck;
    g_pfnUsrTskSwitchHook = (TSKSWITCHHOOK)NULL; /*lint !e611*/
    return ;
}
#endif

LITE_OS_SEC_TEXT_MINOR UINT32 osTaskSwitchCheck(VOID)
{
#if (LOSCFG_BASE_CORE_TSK_MONITOR == YES)
    if (g_pfnTskSwitchHook != NULL)
    {
        g_pfnTskSwitchHook();
    }
#endif /* LOSCFG_BASE_CORE_TSK_MONITOR == YES */

#if (OS_PERF_TSK_FILTER == YES)
    if (g_pfnPerfTskFilterHook != NULL)
    {
        g_pfnPerfTskFilterHook();
    }
#endif

#ifdef LOSCFG_KERNEL_CPUP
    osTskCycleEndStart();
#endif /* LOSCFG_KERNEL_CPUP */
    return LOS_OK;
}
/*****************************************************************************
 Function : osTaskEntry
 Description : All task entry
 Input       : uwTaskID     --- The ID of the task to be run
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT VOID osTaskEntry(UINT32 uwTaskID)
{
    LOS_TASK_CB *pstTaskCB;
    UINT32 uwIntSave;

    OS_TASK_ID_CHECK(uwTaskID);

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    if (pstTaskCB->pThreadJoin)
    {
        pstTaskCB->pThreadJoinRetval =  pstTaskCB->pfnTaskEntry( pstTaskCB->auwArgs[0],
                                        pstTaskCB->auwArgs[1],
                                        pstTaskCB->auwArgs[2],
                                        pstTaskCB->auwArgs[3]);
    }
    else
    {
        pstTaskCB->pfnTaskEntry( pstTaskCB->auwArgs[0],
                                 pstTaskCB->auwArgs[1],
                                 pstTaskCB->auwArgs[2],
                                 pstTaskCB->auwArgs[3]); /*lint !e534*/
    }

    if (pstTaskCB->usTaskStatus & LOS_TASK_STATUS_DETACHED)
    {
        uwIntSave = LOS_IntLock();
        g_usLosTaskLock = 0;
        (VOID)osTaskSelfDelete(pstTaskCB->uwTaskID, uwIntSave);
    }
    /* join mode: waiting for child task done */
    else
    {
        uwIntSave = LOS_IntLock();
        g_usLosTaskLock = 0;

        LOS_TaskLock();
        if (pstTaskCB->pThreadJoin)
        {
            if (LOS_SemPost((UINT32)(((SEM_CB_S *)pstTaskCB->pThreadJoin)->usSemID)) != LOS_OK)
            {
                PRINT_ERR("osTaskEntry LOS_SemPost fail!\n");
            }
            pstTaskCB->pThreadJoin = NULL;
        }

        if (OS_TASK_STATUS_READY & pstTaskCB->usTaskStatus)
        {
            osPriqueueDequeue(&pstTaskCB->stPendList);
            pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_READY);
        }
        LOS_TaskUnlock();
        (VOID)LOS_IntRestore(uwIntSave);
        LOS_Schedule();
    }

}

/*****************************************************************************
 Function : LOS_TaskCreateOnly
 Description : Create a task and suspend
 Input       : pstInitParam --- Task init parameters
 Output      : puwTaskID    --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateOnly(UINT32 *puwTaskID, TSK_INIT_PARAM_S *pstInitParam)
{
    UINT32 uwTaskID = 0;
    UINTPTR uvIntSave;
    VOID  *pTopStack;
    VOID  *pStackPtr;
    LOS_TASK_CB *pstTaskCB;
    UINT32 uwErrRet = OS_ERROR;
    VOID  *pPool = (VOID *)m_aucSysMem1;
    UINT32 uwPoolSize = OS_SYS_MEM_SIZE;
#ifdef LOSCFG_TASK_STACK_PROTECT
    AARCHPTR uwMMUProtectAddr;
    UINT32 uwAlignStackSize;
    MMU_PARAM mPara;
#endif

    if (NULL == puwTaskID)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    if (NULL == pstInitParam)
    {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (NULL == pstInitParam->pcName)
    {
        return LOS_ERRNO_TSK_NAME_EMPTY;
    }

    if (NULL == pstInitParam->pfnTaskEntry)
    {
        return LOS_ERRNO_TSK_ENTRY_NULL;
    }

    if ((pstInitParam->usTaskPrio) > OS_TASK_PRIORITY_LOWEST)
    {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }
#ifdef LOSCFG_EXC_INTERACTION
    if (!osExcInteractionTaskCheck(pstInitParam))
    {
        pPool = m_aucSysMem0;
        uwPoolSize = OS_EXC_INTERACTMEM_SIZE;
    }
#endif
#ifdef LOSCFG_TASK_STACK_PROTECT
    if (pstInitParam->uwStackSize > ((uwPoolSize > (2 * MMU_4K)) ? uwPoolSize - 2 * MMU_4K : 0))
#else
    if (pstInitParam->uwStackSize > uwPoolSize)
#endif
    {
        return LOS_ERRNO_TSK_STKSZ_TOO_LARGE;
    }

    if (0 == pstInitParam->uwStackSize)
    {
        pstInitParam->uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    }
    pstInitParam->uwStackSize = ALIGN(pstInitParam->uwStackSize , (sizeof(AARCHPTR) * 2));

    if (pstInitParam->uwStackSize < LOS_TASK_MIN_STACK_SIZE)
    {
        return LOS_ERRNO_TSK_STKSZ_TOO_SMALL;
    }

    uvIntSave = LOS_IntLock();
    while (!LOS_ListEmpty(&g_stTskRecyleList))
    {
        pstTaskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_stTskRecyleList)); /*lint !e413*/
        LOS_ListDelete(LOS_DL_LIST_FIRST(&g_stTskRecyleList));
        LOS_ListAdd(&g_stLosFreeTask, &pstTaskCB->stPendList);
#ifdef LOSCFG_TASK_STACK_PROTECT
        uwMMUProtectAddr = pstTaskCB->uwTopOfStack - MMU_4K;
        mPara.startAddr = uwMMUProtectAddr;
        mPara.endAddr = uwMMUProtectAddr + MMU_4K;
        mPara.uwFlag = BUFFER_ENABLE | CACHE_ENABLE | ACCESS_PERM_RW_RW;
        mPara.stPage = (SENCOND_PAGE *)&stAppPage;
        LOS_MMUParamSet(&mPara);
        (VOID)LOS_MemFree(pPool, (VOID *)uwMMUProtectAddr);
#else
        (VOID)LOS_MemFree(pPool, (VOID *)pstTaskCB->uwTopOfStack);
#endif
        pstTaskCB->uwTopOfStack = (AARCHPTR)NULL;
    }

    if (LOS_ListEmpty(&g_stLosFreeTask))
    {
        uwErrRet = LOS_ERRNO_TSK_TCB_UNAVAILABLE;
        OS_GOTO_ERREND();
    }

    pstTaskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_stLosFreeTask)); /*lint !e413*/
    LOS_ListDelete(LOS_DL_LIST_FIRST(&g_stLosFreeTask));
    (VOID)LOS_IntRestore(uvIntSave);
    uwTaskID = pstTaskCB->uwTaskID;

#ifdef LOSCFG_TASK_STACK_PROTECT
    uwAlignStackSize = ALIGN(pstInitParam->uwStackSize, MMU_4K);
    uwMMUProtectAddr = (AARCHPTR)LOS_MemAllocAlign(pPool, (uwAlignStackSize + MMU_4K), MMU_4K);
    if (uwMMUProtectAddr == 0)
    {
        pTopStack = (VOID *)NULL;
    }
    else
    {
        pTopStack = (VOID *)(uwMMUProtectAddr + MMU_4K);
        mPara.startAddr = uwMMUProtectAddr;
        mPara.endAddr = uwMMUProtectAddr + MMU_4K;
        mPara.uwFlag = BUFFER_ENABLE | CACHE_ENABLE | ACCESS_PERM_RO_RO;
        mPara.stPage = (SENCOND_PAGE *)&stAppPage;
        LOS_MMUParamSet(&mPara);
    }
#else
    pTopStack = (void *)LOS_MemAllocAlign(pPool, pstInitParam->uwStackSize, (sizeof(AARCHPTR) * 2));
#endif

    if (NULL == pTopStack)
    {
        uvIntSave = LOS_IntLock();
        LOS_ListAdd(&g_stLosFreeTask, &pstTaskCB->stPendList);
        uwErrRet = LOS_ERRNO_TSK_NO_MEMORY;
        OS_GOTO_ERREND();
    }

    pStackPtr = osTskStackInit(uwTaskID, pstInitParam->uwStackSize, pTopStack);
    pstTaskCB->pStackPointer = pStackPtr;
    pstTaskCB->auwArgs[0] = pstInitParam->auwArgs[0];
    pstTaskCB->auwArgs[1] = pstInitParam->auwArgs[1];
    pstTaskCB->auwArgs[2] = pstInitParam->auwArgs[2];
    pstTaskCB->auwArgs[3] = pstInitParam->auwArgs[3];
    pstTaskCB->uwTopOfStack = (AARCHPTR)pTopStack;
    pstTaskCB->uwStackSize = pstInitParam->uwStackSize;
    pstTaskCB->pTaskSem = NULL;
    pstTaskCB->pThreadJoin = NULL;
    pstTaskCB->pTaskMux = NULL;
    pstTaskCB->usTaskStatus = OS_TASK_STATUS_SUSPEND;
    pstTaskCB->usTaskStatus |= (pstInitParam->uwResved ? LOS_TASK_STATUS_DETACHED : 0);/*set the task is detached or joinable*/
    pstTaskCB->usPriority = pstInitParam->usTaskPrio;
    pstTaskCB->uwPriBitMap = 0;
    pstTaskCB->pfnTaskEntry = pstInitParam->pfnTaskEntry;
    pstTaskCB->uwEvent.uwEventID = 0xFFFFFFFF;
    pstTaskCB->uwEventMask = 0;
    pstTaskCB->pcTaskName   = pstInitParam->pcName;
    pstTaskCB->puwMsg = NULL;
    if (pstInitParam->usConsoleID == CONSOLE_SERIAL || pstInitParam->usConsoleID == CONSOLE_TELNET)
    {
        pstTaskCB->usConsoleID = g_stLosTask.pstRunTask->usConsoleID;
    }
    else
    {
        pstTaskCB->usConsoleID = 0;
    }

    *puwTaskID = uwTaskID;
    return LOS_OK;

LOS_ERREND:
    (VOID)LOS_IntRestore(uvIntSave);
    return uwErrRet;
}


/*****************************************************************************
 Function : LOS_TaskCreate
 Description : Create a task
 Input       : pstInitParam --- Task init parameters
 Output      : puwTaskID    --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreate(UINT32 *puwTaskID, TSK_INIT_PARAM_S *pstInitParam)
{
    UINT32 uwRet = LOS_OK;
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;

    uwRet = LOS_TaskCreateOnly(puwTaskID, pstInitParam);
    if (LOS_OK != uwRet)
    {
        return uwRet;
    }
    pstTaskCB = OS_TCB_FROM_TID(*puwTaskID);

    uvIntSave = LOS_IntLock();
    pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_SUSPEND);
    pstTaskCB->usTaskStatus |= OS_TASK_STATUS_READY;

#ifdef LOSCFG_KERNEL_CPUP
    g_pstCpup[pstTaskCB->uwTaskID].uwID = pstTaskCB->uwTaskID;
    g_pstCpup[pstTaskCB->uwTaskID].usStatus = pstTaskCB->usTaskStatus;
#endif

    osPriqueueEnqueue(&pstTaskCB->stPendList, pstTaskCB->usPriority);
    g_stLosTask.pstNewTask = LOS_DL_LIST_ENTRY(osPriqueueTop(), LOS_TASK_CB, stPendList); /*lint !e413*/

    if ((g_bTaskScheduled) && (g_usLosTaskLock == 0))
    {
        if (g_stLosTask.pstRunTask != g_stLosTask.pstNewTask)
        {
            if (LOS_CHECK_SCHEDULE)
            {
                (VOID)LOS_IntRestore(uvIntSave);
                osSchedule();
                return LOS_OK;
            }
        }
    }

    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;
}

/*****************************************************************************
 Function : LOS_TaskResume
 Description : Resume suspend task
 Input       : uwTaskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskResume(UINT32 uwTaskID)
{
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;
    UINT32 uwErrRet = OS_ERROR;

    if (uwTaskID >= g_uwTskMaxNum)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    uvIntSave = LOS_IntLock();
    usTempStatus = pstTaskCB->usTaskStatus;

    if (OS_TASK_STATUS_UNUSED & usTempStatus)
    {
        uwErrRet = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }
    else if (!(OS_TASK_STATUS_SUSPEND & usTempStatus))
    {
        uwErrRet = LOS_ERRNO_TSK_NOT_SUSPENDED;
        OS_GOTO_ERREND();
    }

    pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_SUSPEND);
    if (!(OS_CHECK_TASK_BLOCK & pstTaskCB->usTaskStatus))
    {
        pstTaskCB->usTaskStatus |= OS_TASK_STATUS_READY;
        osPriqueueEnqueue(&pstTaskCB->stPendList, pstTaskCB->usPriority);
        if (g_bTaskScheduled)
        {
            (VOID)LOS_IntRestore(uvIntSave);
            LOS_Schedule();
            return LOS_OK;
        }
        g_stLosTask.pstNewTask = LOS_DL_LIST_ENTRY(osPriqueueTop(), LOS_TASK_CB, stPendList); /*lint !e413*/
    }

    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;

LOS_ERREND:
    (VOID)LOS_IntRestore(uvIntSave);
    return uwErrRet;
}

/*****************************************************************************
 Function : LOS_TaskSuspend
 Description : Suspend task
 Input       : uwTaskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskSuspend(UINT32 uwTaskID)
{
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;
    UINT32 uwErrRet = OS_ERROR;

    if (uwTaskID == g_uwIdleTaskID)
    {
        return LOS_ERRNO_TSK_OPERATE_IDLE;
    }

    if (uwTaskID == g_uwSwtmrTaskID)
    {
        return LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED;
    }

    if (OS_TSK_GET_INDEX(uwTaskID) >= g_uwTskMaxNum)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    uvIntSave = LOS_IntLock();
    usTempStatus = pstTaskCB->usTaskStatus;
    if (OS_TASK_STATUS_UNUSED & usTempStatus)
    {
        uwErrRet = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }

    if (OS_TASK_STATUS_SUSPEND & usTempStatus)
    {
        uwErrRet = LOS_ERRNO_TSK_ALREADY_SUSPENDED;
        OS_GOTO_ERREND();
    }

    if ((OS_TASK_STATUS_RUNNING & usTempStatus) && (g_usLosTaskLock != 0))
    {
        uwErrRet = LOS_ERRNO_TSK_SUSPEND_LOCKED;
        OS_GOTO_ERREND();
    }

    if (OS_TASK_STATUS_READY & usTempStatus)
    {
        osPriqueueDequeue(&pstTaskCB->stPendList);
        pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_READY);
    }

    pstTaskCB->usTaskStatus |= OS_TASK_STATUS_SUSPEND;
    if (uwTaskID == g_stLosTask.pstRunTask->uwTaskID)
    {
        (VOID)LOS_IntRestore(uvIntSave);
        LOS_Schedule();
        return LOS_OK;
    }

    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;

LOS_ERREND:
    (VOID)LOS_IntRestore(uvIntSave);
    return uwErrRet;
}

/*****************************************************************************
 Function : LOS_TaskDelete
 Description : Delete a task
 Input       : uwTaskID --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDelete(UINT32 uwTaskID)
{
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;
    UINT32 uwErrRet = OS_ERROR;
#ifdef LOSCFG_TASK_STACK_PROTECT
    AARCHPTR uwMMUProtectAddr;
    MMU_PARAM mPara;
#endif

    if (uwTaskID == g_uwIdleTaskID)
    {
        return LOS_ERRNO_TSK_OPERATE_IDLE;
    }
#ifndef LOSCFG_EXC_INTERACTION
    if (uwTaskID == g_uwSwtmrTaskID)
    {
        return LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED;
    }
#endif
    if (OS_TSK_GET_INDEX(uwTaskID) >= g_uwTskMaxNum)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    uvIntSave = LOS_IntLock();

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

    usTempStatus = pstTaskCB->usTaskStatus;

    if (OS_TASK_STATUS_UNUSED & usTempStatus)
    {
        uwErrRet = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }

    /* If the task is running and scheduler is locked then you can not delete it */
    if ((OS_TASK_STATUS_RUNNING & usTempStatus) && (g_usLosTaskLock != 0))
    {
        uwErrRet = LOS_ERRNO_TSK_DELETE_LOCKED;
        OS_GOTO_ERREND();
    }

    if (OS_TASK_STATUS_READY & usTempStatus)
    {
        osPriqueueDequeue(&pstTaskCB->stPendList);
        pstTaskCB->usTaskStatus &= (~OS_TASK_STATUS_READY);
    }
    else if ((OS_TASK_STATUS_PEND & usTempStatus) || (OS_TASK_STATUS_PEND_QUEUE & usTempStatus))
    {
        LOS_ListDelete(&pstTaskCB->stPendList);
    }

    if ((OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME) & usTempStatus)
    {
        osTimerListDelete(pstTaskCB);
    }

    pstTaskCB->usTaskStatus &= (~(OS_TASK_STATUS_SUSPEND));
    pstTaskCB->usTaskStatus |= OS_TASK_STATUS_UNUSED;
    pstTaskCB->uwEvent.uwEventID = 0xFFFFFFFF;
    pstTaskCB->uwEventMask = 0;
#ifdef LOSCFG_KERNEL_CPUP
    (VOID)memset((VOID *)&g_pstCpup[pstTaskCB->uwTaskID], 0, sizeof(OS_CPUP_S));
#endif
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
    OS_MEM_CLEAR(uwTaskID);
#endif
    g_stLosTask.pstNewTask = LOS_DL_LIST_ENTRY(osPriqueueTop(), LOS_TASK_CB, stPendList); /*lint !e413*/
    if (OS_TASK_STATUS_RUNNING & pstTaskCB->usTaskStatus)
    {
        LOS_ListTailInsert(&g_stTskRecyleList, &pstTaskCB->stPendList);
        g_stLosTask.pstRunTask = &g_pstTaskCBArray[g_uwTskMaxNum];
        g_stLosTask.pstRunTask->uwTaskID = uwTaskID;
        g_stLosTask.pstRunTask->usTaskStatus = pstTaskCB->usTaskStatus;
        g_stLosTask.pstRunTask->uwTopOfStack = pstTaskCB->uwTopOfStack;
        g_stLosTask.pstRunTask->pcTaskName = pstTaskCB->pcTaskName;
        pstTaskCB->usTaskStatus = OS_TASK_STATUS_UNUSED;
        (VOID)LOS_IntRestore(uvIntSave);
        osSchedule();
        return LOS_OK;
    }
    else
    {
        pstTaskCB->usTaskStatus = OS_TASK_STATUS_UNUSED;
        LOS_ListAdd(&g_stLosFreeTask, &pstTaskCB->stPendList);
#ifdef LOSCFG_TASK_STACK_PROTECT
        uwMMUProtectAddr = pstTaskCB->uwTopOfStack - MMU_4K;
        mPara.startAddr = uwMMUProtectAddr;
        mPara.endAddr = uwMMUProtectAddr + MMU_4K;
        mPara.uwFlag = BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RW_RW;
        mPara.stPage = (SENCOND_PAGE *)&stAppPage;
        LOS_MMUParamSet(&mPara);
        (VOID)LOS_MemFree(m_aucSysMem1, (VOID *)uwMMUProtectAddr);
#else
        (VOID)LOS_MemFree(m_aucSysMem1, (VOID *)pstTaskCB->uwTopOfStack);
#endif
        pstTaskCB->uwTopOfStack = (AARCHPTR)NULL;
    }

    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;

LOS_ERREND:
    (VOID)LOS_IntRestore(uvIntSave);
    return uwErrRet;
}

/*****************************************************************************
 Function : LOS_TaskDelay
 Description : delay the current task
 Input       : uwTick    --- time
 Output      :None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_TaskDelay(UINT32 uwTick)
{
    UINTPTR uvIntSave;

    if (OS_INT_ACTIVE)
    {
        PRINT_ERR("!!!LOS_ERRNO_TSK_DELAY_IN_INT!!!\n");
        return LOS_ERRNO_TSK_DELAY_IN_INT;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
     /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    if (g_usLosTaskLock != 0)
    {
        return LOS_ERRNO_TSK_DELAY_IN_LOCK;
    }

    if (uwTick == 0)
    {
        return LOS_TaskYield();
    }
    else
    {
        uvIntSave = LOS_IntLock();
        osPriqueueDequeue(&(g_stLosTask.pstRunTask->stPendList));
        g_stLosTask.pstRunTask->usTaskStatus &= (~OS_TASK_STATUS_READY);
        osTaskAdd2TimerList((LOS_TASK_CB *)g_stLosTask.pstRunTask, uwTick);
        g_stLosTask.pstRunTask->usTaskStatus |= OS_TASK_STATUS_DELAY;
        (VOID)LOS_IntRestore(uvIntSave);
        LOS_Schedule();
    }

    return LOS_OK;
}

/*****************************************************************************
 Function : LOS_TaskPriGet
 Description : Get the priority of the task
 Input       : uwTaskID
 Output      : None
 Return      : TSK_PRIOR_T on success or OS_INVALID on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT16 LOS_TaskPriGet(UINT32 uwTaskID)
{
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;
    UINT16 usPriority;

    if (OS_CHECK_TSK_PID_NOIDLE(uwTaskID))
    {
       return (UINT16)OS_INVALID;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

    uvIntSave = LOS_IntLock();

    if (OS_TASK_STATUS_UNUSED & pstTaskCB->usTaskStatus)
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return (UINT16)OS_INVALID;
    }

    usPriority = pstTaskCB->usPriority;
    (VOID)LOS_IntRestore(uvIntSave);
    return usPriority;
}

/*****************************************************************************
 Function : LOS_TaskPriSet
 Description : Set the priority of the task
 Input       : uwTaskID
               usTaskPrio
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskPriSet(UINT32 uwTaskID, UINT16 usTaskPrio)
{
    BOOL   bIsReady;
    UINTPTR uvIntSave;
    LOS_TASK_CB *pstTaskCB;
    UINT16 usTempStatus;

    if (usTaskPrio > OS_TASK_PRIORITY_LOWEST)
    {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (uwTaskID == g_uwIdleTaskID)
    {
        return LOS_ERRNO_TSK_OPERATE_IDLE;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(uwTaskID))
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    uvIntSave = LOS_IntLock();
    usTempStatus = pstTaskCB->usTaskStatus;
    if (OS_TASK_STATUS_UNUSED & usTempStatus)
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }
    /* delete the task and insert with right priority into ready queue */
    bIsReady = (OS_TASK_STATUS_READY & usTempStatus);
    if (bIsReady)
    {
        osPriqueueDequeue(&pstTaskCB->stPendList);
        pstTaskCB->usPriority = usTaskPrio;
        osPriqueueEnqueue(&pstTaskCB->stPendList, pstTaskCB->usPriority);
    }
    else
    {
        pstTaskCB->usPriority = usTaskPrio;
    }
    (VOID)LOS_IntRestore(uvIntSave);
    /* delete the task and insert with right priority into ready queue */
    if (bIsReady)
    {
        LOS_Schedule();
    }
    return LOS_OK;
}

/*****************************************************************************
 Function : LOS_CurTaskPriSet
 Description : Set the priority of the current task
 Input       : usTaskPrio
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_CurTaskPriSet(UINT16 usTaskPrio)
{
    UINT32 uwRet;
    uwRet = LOS_TaskPriSet(g_stLosTask.pstRunTask->uwTaskID, usTaskPrio);
    return uwRet;
}

/**************************************************************************
 Function    : osTaskWait
 Description : pend a task in pstList
 Input       : pstList
               uwTimeOut -- Expiry time
 Output      : none
 Return      : LOS_OK on success or LOS_NOK on failure
**************************************************************************/
VOID osTaskWait(LOS_DL_LIST *pstList, UINT32 uwTaskStatus, UINT32 uwTimeOut)
{
    LOS_TASK_CB *pstRunTsk;
    LOS_DL_LIST *pstPendObj;

    pstRunTsk = g_stLosTask.pstRunTask;
    osPriqueueDequeue(&pstRunTsk->stPendList);
    pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_READY);
    pstPendObj = &pstRunTsk->stPendList;
    pstRunTsk->usTaskStatus |= uwTaskStatus;
    LOS_ListTailInsert(pstList,pstPendObj);
    if (uwTimeOut != LOS_WAIT_FOREVER)
    {
        pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND_TIME;
        osTaskAdd2TimerList((LOS_TASK_CB *)pstRunTsk, uwTimeOut);
    }
}

/**************************************************************************
 Function    : osTaskWake
 Description : delete the task from pendlist and also add to the priqueue
 Input       : pstResumedTask --> resumed task
 Output      : pstResumedTask
 Return      : none
**************************************************************************/
VOID osTaskWake(LOS_TASK_CB *pstResumedTask, UINT32 uwTaskStatus)
{
    LOS_ListDelete(&pstResumedTask->stPendList);
    pstResumedTask->usTaskStatus &= (~uwTaskStatus);

    if (pstResumedTask->usTaskStatus & OS_TASK_STATUS_PEND_TIME)
    {
        osTimerListDelete(pstResumedTask);
        pstResumedTask->usTaskStatus &= (~OS_TASK_STATUS_PEND_TIME);
    }
    if (!(pstResumedTask->usTaskStatus & OS_TASK_STATUS_SUSPEND))
    {
        pstResumedTask->usTaskStatus |= OS_TASK_STATUS_READY;
        osPriqueueEnqueue(&pstResumedTask->stPendList, pstResumedTask->usPriority);
    }
}

/*****************************************************************************
 Function : LOS_TaskYield
 Description : Adjust the procedure order of specified task
 Input       : usTaskPrio
               uwNextTask
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskYield(VOID)
{
    UINT32 uwTskCount = 0;
    UINTPTR uvIntSave;

    if(g_stLosTask.pstRunTask->uwTaskID >= g_uwTskMaxNum)
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }
    if(!(g_stLosTask.pstRunTask->usTaskStatus & OS_TASK_STATUS_READY))
    {
        return LOS_OK;
    }
    uvIntSave = LOS_IntLock();
    uwTskCount = osPriqueueSize(g_stLosTask.pstRunTask->usPriority);
    if (uwTskCount > 1)
    {

        LOS_ListDelete(&(g_stLosTask.pstRunTask->stPendList));
        g_stLosTask.pstRunTask->usTaskStatus |= OS_TASK_STATUS_READY;
        osPriqueueEnqueue(&(g_stLosTask.pstRunTask->stPendList), g_stLosTask.pstRunTask->usPriority);
    }
    else
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK;
    }
    (VOID)LOS_IntRestore(uvIntSave);
    LOS_Schedule();
    return LOS_OK;
}

/*****************************************************************************
 Function : LOS_TaskLock
 Description : Task lock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskLock(VOID)
{
    UINTPTR uvIntSave;

    uvIntSave = LOS_IntLock();
    g_usLosTaskLock++;
    (VOID)LOS_IntRestore(uvIntSave);
}

/*****************************************************************************
 Function : LOS_TaskUnlock
 Description : Task unlock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskUnlock(VOID)
{
    UINTPTR uvIntSave;

    uvIntSave = LOS_IntLock();
    if (g_usLosTaskLock > 0)
    {
        g_usLosTaskLock--;
        if (0 == g_usLosTaskLock)
        {
            (VOID)LOS_IntRestore(uvIntSave);
            LOS_Schedule();
            return;
        }
    }

    (VOID)LOS_IntRestore(uvIntSave);
}

/*****************************************************************************
 Function : LOS_TaskInfoGet
 Description : Get the information of the task
 Input       : uwTaskID
 Output      : pstTaskInfo
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskInfoGet(UINT32 uwTaskID, TSK_INFO_S *pstTaskInfo)
{
    UINT32    uwIntSave;
    LOS_TASK_CB *pstTaskCB;
    AARCHPTR *puwStack;

    if (NULL == pstTaskInfo)
    {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(uwTaskID))
    {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    uwIntSave = LOS_IntLock();

    if (OS_TASK_STATUS_UNUSED & pstTaskCB->usTaskStatus)
    {
        (VOID)LOS_IntRestore(uwIntSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (!(OS_TASK_STATUS_RUNNING & pstTaskCB->usTaskStatus) || (OS_INT_ACTIVE))
    {
        pstTaskInfo->uwSP = (AARCHPTR)pstTaskCB->pStackPointer;
    }
    else
    {
        __asm__ __volatile__("mov %0, sp" : "=r"(pstTaskInfo->uwSP));
    }

    pstTaskInfo->usTaskStatus = pstTaskCB->usTaskStatus;
    pstTaskInfo->usTaskPrio = pstTaskCB->usPriority;
    pstTaskInfo->uwStackSize  = pstTaskCB->uwStackSize;
    pstTaskInfo->uwTopOfStack = pstTaskCB->uwTopOfStack;
    pstTaskInfo->uwEvent = pstTaskCB->uwEvent;
    pstTaskInfo->uwEventMask = pstTaskCB->uwEventMask;
    pstTaskInfo->pTaskSem = pstTaskCB->pTaskSem;
    pstTaskInfo->pTaskMux = pstTaskCB->pTaskMux;
    pstTaskInfo->uwTaskID = uwTaskID;

    (VOID)strncpy(pstTaskInfo->acName, pstTaskCB->pcTaskName, LOS_TASK_NAMELEN - 1);
    pstTaskInfo->acName[LOS_TASK_NAMELEN - 1] = '\0';

    pstTaskInfo->uwBottomOfStack = TRUNCATE(((AARCHPTR)(pstTaskCB->uwTopOfStack) + (pstTaskCB->uwStackSize)), OS_TASK_STACK_ADDR_ALIGN);
    pstTaskInfo->uwCurrUsed = pstTaskInfo->uwBottomOfStack - pstTaskInfo->uwSP;

    if (OS_TASK_MAGIC_WORD == *(AARCHPTR *)pstTaskInfo->uwTopOfStack)
    {
        puwStack = (AARCHPTR *)(pstTaskInfo->uwTopOfStack + sizeof(AARCHPTR));
        while ((puwStack < (AARCHPTR *)pstTaskInfo->uwSP) && (*puwStack == OS_TASK_STACK_INIT))
        {
            puwStack += 1;
        }

        pstTaskInfo->uwPeakUsed = pstTaskCB->uwStackSize - ((AARCHPTR)puwStack - pstTaskInfo->uwTopOfStack);
        pstTaskInfo->bOvf = FALSE;
    }
    else
    {
        pstTaskInfo->uwPeakUsed = 0xFFFFFFFF;
        pstTaskInfo->bOvf = TRUE;
    }

    (VOID)LOS_IntRestore(uwIntSave);

    return LOS_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
