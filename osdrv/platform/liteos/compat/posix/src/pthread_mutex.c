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

#include "stdlib.h"
#include "los_mux.h"
#include "los_task.ph"
#include "pthread.h"
#include "errno.h"
#include "string.h"
#include "los_priqueue.ph"
#include "map_error.h"
#include "los_bitmap.h"

#if (LOSCFG_BASE_CORE_SWTMR == YES)
#include "los_exc.h"
#include "los_swtmr.ph"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PTHREADS_MUTEX_CHECK(ptr)   if ((ptr) == NULL) return EINVAL
#define MUTEXATTR_TYPE_MASK 0x0f

static UINT32 osMuxPendForPosix(pthread_mutex_t *mutex, UINT32 uwTimeout);
static UINT32 osMuxPostForPosix(pthread_mutex_t *mutex);

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

#if defined POSIX_MUTEX_DEFAULT_INHERIT
    attr->protocol    = PTHREAD_PRIO_INHERIT;
#elif defined POSIX_MUTEX_DEFAULT_PROTECT
    attr->protocol    = PTHREAD_PRIO_PROTECT;
#else
    attr->protocol    = PTHREAD_PRIO_NONE;
#endif
    attr->prioceiling = OS_TASK_PRIORITY_LOWEST;
    attr->type        = PTHREAD_MUTEX_DEFAULT;
    return ENOERR;
}

//-----------------------------------------------------------------------------
// Destroy attribute object
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Optional functions depending on priority inversion protection options.

// Set priority inversion protection protocol
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    if (attr == NULL) {
        return EINVAL;
    }

    switch (protocol) {
    case PTHREAD_PRIO_NONE:
    case PTHREAD_PRIO_INHERIT:
    case PTHREAD_PRIO_PROTECT:
        attr->protocol = (unsigned char)protocol;
        return ENOERR;

    default:
        return EINVAL;
    }
}

// Get priority inversion protection protocol
int pthread_mutexattr_getprotocol(pthread_mutexattr_t *attr, int *protocol)
{
    if (attr != NULL && protocol != NULL) {
        *protocol = attr->protocol;
    } else {
        return EINVAL;
    }

    return ENOERR;
}

// Set priority for priority ceiling protocol
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
    if (attr == NULL ||
        prioceiling < OS_TASK_PRIORITY_HIGHEST ||
        prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }

    attr->prioceiling = (unsigned char)prioceiling;

    return ENOERR;
}

// Get priority for priority ceiling protocol
int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *attr, int *prioceiling)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (prioceiling != NULL) {
        *prioceiling = attr->prioceiling;
    }

    return ENOERR;
}

// Set priority ceiling of given mutex, returning old ceiling.
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling)
{
    int ret = ENOERR, retLock = ENOERR;
    if (mutex == NULL ||
        prioceiling < OS_TASK_PRIORITY_HIGHEST ||
        prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }

    retLock = pthread_mutex_lock(mutex);
    if(retLock != ENOERR)
        return retLock; /*lint !e454*/

    if (old_ceiling != NULL) {
        *old_ceiling = mutex->stAttr.prioceiling;
    }

    ret = pthread_mutexattr_setprioceiling(&mutex->stAttr, prioceiling);

    retLock = pthread_mutex_unlock(mutex);
    if (ret == ENOERR && retLock != ENOERR)
        return retLock;

    return ret;
}

// Get priority ceiling of given mutex
int pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *prioceiling)
{
    if (mutex != NULL && prioceiling != NULL) {
        *prioceiling = mutex->stAttr.prioceiling;
    } else {
       return EINVAL;
    }

    return ENOERR;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type_p)
{
    int type;

    if (attr == NULL || type_p == NULL) {
        return EINVAL;
    }

    type = (attr->type& MUTEXATTR_TYPE_MASK);

    if (type < PTHREAD_MUTEX_NORMAL || type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }

    *type_p = type;

    return ENOERR;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (attr == NULL ||
        type < PTHREAD_MUTEX_NORMAL ||
        type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }

    attr->type = (attr->type & ~MUTEXATTR_TYPE_MASK) | type;
    return ENOERR;
}


//-----------------------------------------------------------------------------
// Mutex functions

//-----------------------------------------------------------------------------
// Initialize mutex. If mutex_attr is NULL, use default attributes.
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutex_attr)
{
    pthread_mutexattr_t use_attr;
    MUX_CB_P *pstMuxCreated;

    if (mutex == NULL) {
        return EINVAL;
    }

    /* Set up the attributes we are going to use */
    if (mutex_attr == NULL) {
        if(pthread_mutexattr_init( &use_attr ) != ENOERR)
            return EINVAL;
    } else {
        use_attr = *mutex_attr;
    }

    if ((INT8)(use_attr.type) < PTHREAD_MUTEX_NORMAL ||
        use_attr.type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }
    if ((INT8)(use_attr.prioceiling) < OS_TASK_PRIORITY_HIGHEST ||
        use_attr.prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }
    if ((INT8)(use_attr.protocol) < PTHREAD_PRIO_NONE ||
        use_attr.protocol > PTHREAD_PRIO_PROTECT) {
        return EINVAL;
    }

    mutex->stAttr = use_attr;
    pstMuxCreated = &mutex->stLock;

    pstMuxCreated->usMuxCount  = 0;
    pstMuxCreated->pstOwner    = (LOS_TASK_CB *)NULL;
    LOS_ListInit(&pstMuxCreated->stMuxList);

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Destroy mutex.
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    MUX_CB_P *pstMuxDeleted;

    if (mutex == NULL) {
    	return EINVAL;
    }

    pstMuxDeleted = &mutex->stLock;

    if (pstMuxDeleted->usMuxCount != 0) {
        return EBUSY;
    }
    memset(mutex, 0, sizeof(pthread_mutex_t));

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Lock mutex, waiting for it if necessary.
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    MUX_CB_P *pstMuxPended;
    LOS_TASK_CB *pstRunTsk;
    UINT32 uwRet;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (OS_INT_ACTIVE) {
        return EPERM;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
     /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    pstMuxPended = &mutex->stLock;
    pstRunTsk = g_stLosTask.pstRunTask;

    if (pstMuxPended->stMuxList.pstPrev == NULL ||
        pstMuxPended->stMuxList.pstNext == NULL) {
        pstMuxPended->usMuxCount  = 0;
        pstMuxPended->pstOwner    = (LOS_TASK_CB *)NULL;
        LOS_ListInit(&pstMuxPended->stMuxList);
    }

    if (mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK &&
            pstMuxPended->usMuxCount != 0 &&
            pstMuxPended->pstOwner == pstRunTsk) {
        return EDEADLK;
    }
    if ((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL ||
        mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.prioceiling) < OS_TASK_PRIORITY_HIGHEST ||
        mutex->stAttr.prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.protocol) < PTHREAD_PRIO_NONE ||
        mutex->stAttr.protocol > PTHREAD_PRIO_PROTECT) {
        return EINVAL;
    }

    uwRet = osMuxPendForPosix(mutex, LOS_WAIT_FOREVER);

    return map_errno(uwRet);
}

//-----------------------------------------------------------------------------
// Try to lock mutex.
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    MUX_CB_P *pstMuxPended;
    LOS_TASK_CB *pstRunTsk;
    UINT32 uwRet;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (OS_INT_ACTIVE) {
        return EPERM;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
      /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    pstMuxPended = &mutex->stLock;
    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;

    if (pstMuxPended->stMuxList.pstPrev == NULL ||
        pstMuxPended->stMuxList.pstNext == NULL) {
        pstMuxPended->usMuxCount  = 0;
        pstMuxPended->pstOwner    = (LOS_TASK_CB *)NULL;
        LOS_ListInit(&pstMuxPended->stMuxList);
    }
    if (mutex->stLock.pstOwner != NULL && mutex->stLock.pstOwner != pstRunTsk) {
        return EBUSY;
    }
    if (mutex->stAttr.type != PTHREAD_MUTEX_RECURSIVE && pstMuxPended->usMuxCount != 0) {
        return EBUSY;
    }
    if ((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL ||
        mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.prioceiling) < OS_TASK_PRIORITY_HIGHEST ||
        mutex->stAttr.prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.protocol) < PTHREAD_PRIO_NONE ||
        mutex->stAttr.protocol > PTHREAD_PRIO_PROTECT) {
        return EINVAL;
    }

    uwRet = osMuxPendForPosix(mutex, 0);

    return map_errno(uwRet);
}


//-----------------------------------------------------------------------------
// Unlock mutex.
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    MUX_CB_P *pstMuxPosted;
    LOS_TASK_CB *pstRunTsk;
    UINT32 uwRet;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (OS_INT_ACTIVE) {
        return EPERM;
    }

#if (LOSCFG_BASE_CORE_SWTMR == YES)
     /* DO NOT Call blocking API in software timer task */
    if (g_stLosTask.pstRunTask->pfnTaskEntry == (TSK_ENTRY_FUNC)osSwTmrTask)
    {
        PRINTK("Warning: DO NOT call %s in software timer callback.\n", __FUNCTION__);
        osBackTrace();
    }
#endif

    pstMuxPosted = &mutex->stLock;
    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;

    if (pstMuxPosted->usMuxCount == 0) {
        return EPERM;
    }

    if (pstMuxPosted->pstOwner != pstRunTsk) {
        return EPERM;
    }
    if ((INT8)(mutex->stAttr.type) < PTHREAD_MUTEX_NORMAL ||
        mutex->stAttr.type > PTHREAD_MUTEX_ERRORCHECK) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.prioceiling) < OS_TASK_PRIORITY_HIGHEST ||
        mutex->stAttr.prioceiling > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }
    if ((INT8)(mutex->stAttr.protocol) < PTHREAD_PRIO_NONE ||
        mutex->stAttr.protocol > PTHREAD_PRIO_PROTECT) {
        return EINVAL;
    }

    uwRet = osMuxPostForPosix(mutex);

    return map_errno(uwRet);
}

static UINT32 osMuxPendForPosix(pthread_mutex_t *mutex, UINT32 uwTimeout)
{
    UINT32 uwIntSave;
    MUX_CB_P *pstMuxPended;
    UINT32 uwRetErr;
    LOS_TASK_CB *pstRunTsk;
    LOS_TASK_CB *pstPendedTask;
    LOS_DL_LIST *pstPendObj;
    UINT16 usBitMapPri;

    pstMuxPended = &mutex->stLock;
    uwIntSave = LOS_IntLock();

    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;

    if (pstMuxPended->usMuxCount == 0) {
        pstMuxPended->usMuxCount++;
        pstMuxPended->pstOwner = pstRunTsk;

        if (pstRunTsk->usPriority > mutex->stAttr.prioceiling
            && mutex->stAttr.protocol == PTHREAD_PRIO_PROTECT) {
            LOS_BitMapSet(&pstRunTsk->uwPriBitMap, pstRunTsk->usPriority);
            osTaskPriModify(pstRunTsk, mutex->stAttr.prioceiling);
        }

        LOS_IntRestore(uwIntSave);
        return LOS_OK;
    }

    if (pstMuxPended->pstOwner == pstRunTsk
        && mutex->stAttr.type == PTHREAD_MUTEX_RECURSIVE) {
        pstMuxPended->usMuxCount++;
        LOS_IntRestore(uwIntSave);
        return LOS_OK;
    }

    if (g_usLosTaskLock) {
        uwRetErr = LOS_ERRNO_MUX_PEND_IN_LOCK;
        goto errre_uniMuxPend;
    }

    /* while g_stLosTask.pstRunTask is not changed, we can use local variable 'pRunTsk' for performance */
    osPriqueueDequeue(&pstRunTsk->stPendList);
    pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_READY);
    pstRunTsk->pTaskMux = (VOID *)pstMuxPended;
    pstPendObj = &pstRunTsk->stPendList;
    pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND;

    if (pstMuxPended->pstOwner->usPriority > pstRunTsk->usPriority
        && mutex->stAttr.protocol == PTHREAD_PRIO_INHERIT) {
        LOS_BitMapSet(&(pstMuxPended->pstOwner->uwPriBitMap), pstMuxPended->pstOwner->usPriority);
        osTaskPriModify(pstMuxPended->pstOwner, pstRunTsk->usPriority);
    }

    if (LOS_ListEmpty(&pstMuxPended->stMuxList))
    {
        LOS_ListTailInsert(&pstMuxPended->stMuxList, pstPendObj);
    }
    else
    {
        if ((pstPendedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&pstMuxPended->stMuxList))) && /*lint !e413*/
            pstPendedTask->usPriority > pstRunTsk->usPriority)
        {
            LOS_ListHeadInsert(&pstMuxPended->stMuxList, pstPendObj);
        }
        else if ((pstPendedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_LAST(&pstMuxPended->stMuxList))) && /*lint !e413*/
            pstPendedTask->usPriority <= pstRunTsk->usPriority)
        {
            LOS_ListTailInsert(&pstMuxPended->stMuxList, pstPendObj);
        }
        else
        {
            LOS_DL_LIST_FOR_EACH_ENTRY(pstPendedTask, (&(pstMuxPended->stMuxList)), LOS_TASK_CB, stPendList) /*lint !e413*/
            {
                if (pstPendedTask->usPriority < pstRunTsk->usPriority)
                {
                    continue;
                }
                else if (pstPendedTask->usPriority > pstRunTsk->usPriority)
                {
                    LOS_ListAdd(pstPendedTask->stPendList.pstPrev, pstPendObj);
                    break;
                }
                else
                {
                    LOS_ListAdd(&pstPendedTask->stPendList, pstPendObj);
                    break;
                }
            }
        }
    }

    if (uwTimeout != LOS_WAIT_FOREVER) {
        pstRunTsk->usTaskStatus |= OS_TASK_STATUS_PEND_TIME;
        osTaskAdd2TimerList((LOS_TASK_CB *)pstRunTsk, uwTimeout);
        (VOID)LOS_IntRestore(uwIntSave);
        LOS_Schedule();

        uwIntSave = LOS_IntLock();
        if (pstMuxPended->pstOwner->usPriority == pstRunTsk->usPriority)
        {
            if ((usBitMapPri = LOS_LowBitGet(&(pstMuxPended->pstOwner->uwPriBitMap))) != LOS_INVALID_BIT_INDEX)
            {
                LOS_BitMapClr(&(pstMuxPended->pstOwner->uwPriBitMap), usBitMapPri);
                osTaskPriModify(pstMuxPended->pstOwner, usBitMapPri);
            }
        }
        else
        {
            if (LOS_HighBitGet(&(pstMuxPended->pstOwner->uwPriBitMap)) != pstRunTsk->usPriority)
                LOS_BitMapClr(&(pstMuxPended->pstOwner->uwPriBitMap), pstRunTsk->usPriority);
        }

        (VOID)LOS_IntRestore(uwIntSave);
    } else {
        LOS_IntRestore(uwIntSave);
        LOS_Schedule();

    }

    if (pstRunTsk->usTaskStatus & OS_TASK_STATUS_TIMEOUT) {
        uwIntSave = LOS_IntLock();
        pstRunTsk->usTaskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        (VOID)LOS_IntRestore(uwIntSave);
        uwRetErr = LOS_ERRNO_MUX_TIMEOUT;
        goto error_uniMuxPend;
    }

    return LOS_OK;

errre_uniMuxPend:
    (VOID)LOS_IntRestore(uwIntSave);
error_uniMuxPend:
    return uwRetErr;
}

static UINT32 osMuxPostForPosix(pthread_mutex_t *mutex)
{
    UINT32 uwIntSave;
    MUX_CB_P *pstMuxPosted;
    LOS_TASK_CB *pstResumedTask;
    LOS_TASK_CB *pstPendedTask;
    LOS_TASK_CB *pstRunTsk;
    UINT16 usBitMapPri;

    pstMuxPosted = &mutex->stLock;
    uwIntSave = LOS_IntLock();

    pstRunTsk = (LOS_TASK_CB *)g_stLosTask.pstRunTask;
    if (--(pstMuxPosted->usMuxCount) != 0
        && mutex->stAttr.type == PTHREAD_MUTEX_RECURSIVE) {
        LOS_IntRestore(uwIntSave);
        return LOS_OK;
    }

    if (mutex->stAttr.protocol == PTHREAD_PRIO_PROTECT)
    {
        if ((usBitMapPri = LOS_HighBitGet(&pstRunTsk->uwPriBitMap)) != LOS_INVALID_BIT_INDEX)
        {
            LOS_BitMapClr(&pstRunTsk->uwPriBitMap, usBitMapPri);
            osTaskPriModify(pstRunTsk, usBitMapPri);
        }
    }

    /* whether a task block the mutex lock*/
    if (!LOS_ListEmpty(&pstMuxPosted->stMuxList)) {
        pstResumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(pstMuxPosted->stMuxList)));/*lint !e413*/
        LOS_ListDelete(LOS_DL_LIST_FIRST(&(pstMuxPosted->stMuxList)));
        LOS_ASSERT_COND(pstResumedTask->usTaskStatus & OS_TASK_STATUS_PEND);

        pstResumedTask->usTaskStatus &= (~OS_TASK_STATUS_PEND);
        if (pstResumedTask->usTaskStatus & OS_TASK_STATUS_PEND_TIME) {
            osTimerListDelete(pstResumedTask);
            pstResumedTask->usTaskStatus &= (~OS_TASK_STATUS_PEND_TIME);

        }

        if (pstResumedTask->usPriority != pstRunTsk->usPriority)
        {
            if (LOS_HighBitGet(&pstRunTsk->uwPriBitMap) != pstResumedTask->usPriority)
                LOS_BitMapClr(&pstRunTsk->uwPriBitMap, pstResumedTask->usPriority);
        }
        else
        {
            if (pstRunTsk->uwPriBitMap != 0)
            {
                if (!LOS_ListEmpty(&pstMuxPosted->stMuxList))
                {
                    usBitMapPri = LOS_HighBitGet(&pstRunTsk->uwPriBitMap);
                    LOS_DL_LIST_FOR_EACH_ENTRY(pstPendedTask, (&(pstMuxPosted->stMuxList)), LOS_TASK_CB, stPendList) /*lint !e413*/
                    {
                        if (usBitMapPri != pstPendedTask->usPriority)
                            LOS_BitMapClr(&pstRunTsk->uwPriBitMap, pstPendedTask->usPriority);
                    }
                }

                usBitMapPri = LOS_LowBitGet(&pstRunTsk->uwPriBitMap);
                LOS_BitMapClr(&pstRunTsk->uwPriBitMap, usBitMapPri);
                osTaskPriModify(pstMuxPosted->pstOwner, usBitMapPri);
            }
        }

        pstMuxPosted->usMuxCount = 1;
        pstMuxPosted->pstOwner = pstResumedTask;
        pstResumedTask->pTaskMux = NULL;

        if (!(pstResumedTask->usTaskStatus & OS_TASK_STATUS_SUSPEND)) {
            pstResumedTask->usTaskStatus |= OS_TASK_STATUS_READY;
            osPriqueueEnqueue(&pstResumedTask->stPendList, pstResumedTask->usPriority);
        }

        (VOID)LOS_IntRestore(uwIntSave);
        LOS_Schedule();
    } else {
        pstMuxPosted->pstOwner = (LOS_TASK_CB *)NULL;
        (VOID)LOS_IntRestore(uwIntSave);
    }

    return LOS_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
