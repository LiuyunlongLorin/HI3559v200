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

#include "los_event.inc"
#include "los_priqueue.ph"
#include "los_task.ph"

#if (LOSCFG_BASE_CORE_SWTMR == YES)
#include "los_exc.h"
#include "los_swtmr.ph"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


LITE_OS_SEC_TEXT_INIT UINT32 LOS_EventInit(PEVENT_CB_S pstEventCB)
{
    if (pstEventCB == NULL)
    {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    pstEventCB->uwEventID = 0;
    LOS_ListInit(&pstEventCB->stEventList);
    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_EventPoll(UINT32 *puwEventID, UINT32 uwEventMask, UINT32 uwMode)
{
     UINT32     uwRet = 0;
     UINTPTR    uvIntSave;

     uvIntSave = LOS_IntLock();
     if (uwMode & LOS_WAITMODE_OR)
     {
         if (0 != (*puwEventID & uwEventMask))
         {
            uwRet = *puwEventID & uwEventMask;
         }
     }
     else
     {
         if ((uwEventMask != 0) && (uwEventMask == (*puwEventID & uwEventMask)))
         {
            uwRet = *puwEventID & uwEventMask;
         }
     }

     if (uwRet && (LOS_WAITMODE_CLR & uwMode))
     {
        *puwEventID = *puwEventID & ~(uwRet);
     }

     LOS_IntRestore(uvIntSave);
     return uwRet;
}

LITE_OS_SEC_TEXT STATIC UINT32 osEventRead(PEVENT_CB_S pstEventCB, UINT32 uwEventMask, UINT32 uwMode, UINT32 uwTimeOut, BOOL bOnce)
{
    UINT32      uwRet = 0;
    UINTPTR     uvIntSave;
    LOS_TASK_CB *pstRunTsk;

    if (pstEventCB == NULL)
    {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }

    if (uwEventMask == 0)
    {
        return LOS_ERRNO_EVENT_EVENTMASK_INVALID;
    }

    if (uwEventMask & LOS_ERRTYPE_ERROR)
    {
        return LOS_ERRNO_EVENT_SETBIT_INVALID;
    }

    if (((uwMode & LOS_WAITMODE_OR) && (uwMode & LOS_WAITMODE_AND)) ||
        uwMode & ~(LOS_WAITMODE_OR | LOS_WAITMODE_AND | LOS_WAITMODE_CLR) ||
        !(uwMode & (LOS_WAITMODE_OR | LOS_WAITMODE_AND)))
    {
        return LOS_ERRNO_EVENT_FLAGS_INVALID;
    }

    if (OS_INT_ACTIVE)
    {
        return LOS_ERRNO_EVENT_READ_IN_INTERRUPT;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
     /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call LOS_EventRead in software timer callback.\n");
        osBackTrace();
    }
#endif

    uvIntSave = LOS_IntLock();

    if(bOnce == 0)
    {
        uwRet = LOS_EventPoll(&(pstEventCB->uwEventID), uwEventMask, uwMode);
    }

    if (uwRet == 0)
    {
        if (uwTimeOut == 0)
        {
            (VOID)LOS_IntRestore(uvIntSave);
            return uwRet;
        }

        if (g_usLosTaskLock)
        {
            (VOID)LOS_IntRestore(uvIntSave);
            return LOS_ERRNO_EVENT_READ_IN_LOCK;
        }

    pstRunTsk = g_stLosTask.pstRunTask;
    pstRunTsk->uwEventMask = uwEventMask;
    pstRunTsk->uwEventMode = uwMode;

    osTaskWait(&pstEventCB->stEventList, OS_TASK_STATUS_PEND, uwTimeOut);
    (VOID)LOS_IntRestore(uvIntSave);
    LOS_Schedule();

    if (pstRunTsk->usTaskStatus & OS_TASK_STATUS_TIMEOUT)
    {
        uvIntSave = LOS_IntLock();
        pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        (VOID)LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_EVENT_READ_TIMEOUT;
    }

    uvIntSave = LOS_IntLock();
    uwRet = LOS_EventPoll(&pstEventCB->uwEventID, uwEventMask, uwMode);
    (VOID)LOS_IntRestore(uvIntSave);
    }
    else
    {
        (VOID)LOS_IntRestore(uvIntSave);
    }

    return uwRet;
}

LITE_OS_SEC_TEXT STATIC UINT8 osEventResume(LOS_TASK_CB *pstResumedTask, PEVENT_CB_S pstEventCB, UINT32 uwEvents)
{
    UINT8 ucExitFlag = 0;

    if (((pstResumedTask->uwEventMode & LOS_WAITMODE_OR) && (pstResumedTask->uwEventMask & uwEvents) != 0) ||
        ((pstResumedTask->uwEventMode & LOS_WAITMODE_AND) && (pstResumedTask->uwEventMask & pstEventCB->uwEventID) == pstResumedTask->uwEventMask))
    {
        ucExitFlag = 1;

        osTaskWake(pstResumedTask, OS_TASK_STATUS_PEND);
    }

    return ucExitFlag;
}

LITE_OS_SEC_TEXT STATIC UINT32 osEventWrite(PEVENT_CB_S pstEventCB, UINT32 uwEvents, BOOL bOnce)
{
    LOS_TASK_CB *pstResumedTask;
    LOS_TASK_CB *pstNextTask = (LOS_TASK_CB *)NULL;
    UINTPTR     uvIntSave;
    UINT8       ucExitFlag = 0;

    if (pstEventCB == NULL)
    {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }

    if (uwEvents & LOS_ERRTYPE_ERROR)
    {
        return LOS_ERRNO_EVENT_SETBIT_INVALID;
    }

    uvIntSave = LOS_IntLock();

    pstEventCB->uwEventID |= uwEvents;
    if (!LOS_ListEmpty(&pstEventCB->stEventList))
    {
        for (pstResumedTask = LOS_DL_LIST_ENTRY((&pstEventCB->stEventList)->pstNext, LOS_TASK_CB, stPendList);/*lint !e413*/
            &pstResumedTask->stPendList != (&pstEventCB->stEventList);)
        {
            pstNextTask = LOS_DL_LIST_ENTRY(pstResumedTask->stPendList.pstNext, LOS_TASK_CB, stPendList); /*lint !e413*/
            if (osEventResume(pstResumedTask, pstEventCB, uwEvents))
            {
                ucExitFlag = 1;
            }
            if(bOnce == 1)
            {
                break;
            }
            pstResumedTask = pstNextTask;
        }

        if (ucExitFlag == 1)
        {
            (VOID)LOS_IntRestore(uvIntSave);
            LOS_Schedule();
            return LOS_OK;
        }
    }
    (VOID)LOS_IntRestore(uvIntSave);
    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_EventRead(PEVENT_CB_S pstEventCB, UINT32 uwEventMask, UINT32 uwMode, UINT32 uwTimeOut)
{
    return osEventRead(pstEventCB, uwEventMask, uwMode, uwTimeOut, 0);
}

LITE_OS_SEC_TEXT UINT32 LOS_EventWrite(PEVENT_CB_S pstEventCB, UINT32 uwEvents)
{
    return osEventWrite(pstEventCB, uwEvents, 0);
}

LITE_OS_SEC_TEXT_MINOR UINT32 osEventReadOnce(PEVENT_CB_S pstEventCB, UINT32 uwEventMask, UINT32 uwMode, UINT32 uwTimeOut)
{
    return osEventRead(pstEventCB, uwEventMask, uwMode, uwTimeOut, 1);
}

LITE_OS_SEC_TEXT_MINOR UINT32 osEventWriteOnce(PEVENT_CB_S pstEventCB, UINT32 uwEvents)
{
    return osEventWrite(pstEventCB, uwEvents, 1);
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_EventDestroy(PEVENT_CB_S pstEventCB)
{
    UINTPTR     uvIntSave;

    if (pstEventCB == NULL)
    {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }

    uvIntSave = LOS_IntLock();
    if (!LOS_ListEmpty(&pstEventCB->stEventList))
    {
        (VOID)LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_EVENT_SHOULD_NOT_DESTORY;
    }

    LOS_ListDelInit(&pstEventCB->stEventList);
    (VOID)LOS_IntRestore(uvIntSave);

    return LOS_OK; 
}
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_EventClear(PEVENT_CB_S pstEventCB, UINT32 uwEvents)
{
    UINTPTR uvIntSave;

    if (pstEventCB == NULL)
    {
        return LOS_ERRNO_EVENT_PTR_NULL;
    }
    uvIntSave = LOS_IntLock();
    pstEventCB->uwEventID &= uwEvents;
    (VOID)LOS_IntRestore(uvIntSave);

    return LOS_OK;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
