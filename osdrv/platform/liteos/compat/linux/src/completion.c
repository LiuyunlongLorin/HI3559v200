#include "linux/completion.h"
#include "los_task.ph"
#include "limits.h"

#include "los_err.ph"
#include "los_priqueue.ph"
#if (LOSCFG_BASE_CORE_SWTMR == YES)
#include "los_swtmr.ph"
#endif

extern VOID osBackTrace(VOID);
void init_completion(struct completion *x)
{
    if(!x)
        return;

    LOS_ListInit(&x->stComList);
    x->uwComCount = 0;
    return;
}

void complete(struct completion *x)
{
    UINTPTR       uwIntSave = 0;
    LOS_TASK_CB  *pstResumedTask;

    if(!x)
        return;

    uwIntSave = LOS_IntLock();
    if (!LOS_ListEmpty(&x->stComList))
    {
        pstResumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(x->stComList)));/*lint !e413*/
        LOS_ListDelete(LOS_DL_LIST_FIRST(&(x->stComList)));

        LOS_ASSERT_COND(pstResumedTask->usTaskStatus & OS_TASK_STATUS_PEND);
        pstResumedTask->usTaskStatus &= (~OS_TASK_STATUS_PEND);

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
        LOS_IntRestore(uwIntSave);
        LOS_Schedule();
    }
    else
    {
        x->uwComCount++;
        LOS_IntRestore(uwIntSave);
    }

    return;
}

void  wait_for_completion(struct completion *x)
{
    UINTPTR         uwIntSave = 0;
    LOS_TASK_CB     *pstRunTsk;
    LOS_DL_LIST     *pstPendObj;

    if(!x)
        return;

    uwIntSave = LOS_IntLock();

    if (OS_INT_ACTIVE)
    {
        PRINT_ERR("Calling %s in interrupt callback is not allowed.\n", __FUNCTION__);
        osBackTrace();
        LOS_IntRestore(uwIntSave);
        return;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
    /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    if (x->uwComCount > 0)
    {
        x->uwComCount--;
        LOS_IntRestore(uwIntSave);
        return;
    }

    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;
    osPriqueueDequeue(&pstRunTsk->stPendList);
    pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_READY);
    pstPendObj = &pstRunTsk->stPendList;

    pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND;
    LOS_ListTailInsert(&x->stComList, pstPendObj);
    LOS_IntRestore(uwIntSave);

    LOS_Schedule();
    return;
}

unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
    UINTPTR         uwIntSave = 0;
    UINT32          uwRetErr = 0;
    UINT64          uwTick_last = 0;
    LOS_TASK_CB    *pstRunTsk;
    LOS_DL_LIST    *pstPendObj;

    if(!x)
        return OS_WAIT_COMPLETION_ERROR;

    uwTick_last = LOS_TickCountGet();
    uwIntSave = LOS_IntLock();

    if (OS_INT_ACTIVE)
    {
        uwRetErr = OS_WAIT_COMPLETION_ERROR;
        PRINT_ERR("Calling %s in interrupt callback is not allowed.\n", __FUNCTION__);
        osBackTrace();
        goto error;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
     /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    if (x->uwComCount > 0)
    {
        x->uwComCount--;
        LOS_IntRestore(uwIntSave);
        return timeout;
    }

    if (!timeout || (LOS_WAIT_FOREVER == timeout))
    {
        uwRetErr = OS_WAIT_COMPLETION_ERROR;
        goto error;
    }

    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;
    osPriqueueDequeue(&pstRunTsk->stPendList);
    pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_READY);

    pstPendObj = &pstRunTsk->stPendList;
    pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND;
    LOS_ListTailInsert(&x->stComList, pstPendObj);

    pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND_TIME;
    osTaskAdd2TimerList((LOS_TASK_CB *)pstRunTsk, timeout);
    LOS_IntRestore(uwIntSave);

    LOS_Schedule();

    if (pstRunTsk->usTaskStatus & OS_TASK_STATUS_TIMEOUT)
    {
        uwIntSave = LOS_IntLock();
        pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        LOS_IntRestore(uwIntSave);
        return 0;
    }

    uwTick_last = LOS_TickCountGet() - uwTick_last;
    return (timeout - uwTick_last);

error:
    LOS_IntRestore(uwIntSave);
    OS_RETURN_ERROR(uwRetErr);
}

void complete_all(struct completion *x)
{
    UINTPTR      uwIntSave = 0;
    LOS_TASK_CB *pstResumedTask;

    if(!x)
        return;

    uwIntSave = LOS_IntLock();

    x->uwComCount += UINT_MAX/2;
    if (TRUE == LOS_ListEmpty(&x->stComList))
    {
        LOS_IntRestore(uwIntSave);
        return;
    }

    while (!LOS_ListEmpty(&x->stComList))
    {
        pstResumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(x->stComList)));/*lint !e413*/
        LOS_ListDelete(LOS_DL_LIST_FIRST(&(x->stComList)));

        LOS_ASSERT_COND(pstResumedTask->usTaskStatus & OS_TASK_STATUS_PEND);
        pstResumedTask->usTaskStatus &= (~OS_TASK_STATUS_PEND);

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
        x->uwComCount--;
    }
    LOS_IntRestore(uwIntSave);
    LOS_Schedule();

    return;
}
