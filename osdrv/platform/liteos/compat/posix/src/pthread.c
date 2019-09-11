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

#include "los_priqueue.ph"

#include "errno.h"
#include "sched.h"
#include "string.h"
#include "stdio.h"
#include "map_error.h"
#include "pprivate.h"
#include "pthread.h"
#include "limits.h"

//-----------------------------------------------------------------------------
// First check that the configuration contains the elements we need
//=============================================================================
// Internal data structures


// Array of pthread control structures. A pthread_t object is
// "just" an index into this array.
__attribute__((section(".pthdata")))_pthread_data g_pthread_data_0;
_pthread_data *PTHREADS_DATA;

// Count of number of threads that have exited and not been reaped.
static int pthreads_exited;

// Per-thread key allocation. This key map has a 1 bit set for each
// key that is free, zero if it is allocated.
//#define KEY_MAP_TYPE UINT32
//#define KEY_MAP_TYPE_SIZE   (sizeof(KEY_MAP_TYPE) * 8) /* in BITS! */
//static KEY_MAP_TYPE thread_key[PTHREAD_KEYS_MAX/KEY_MAP_TYPE_SIZE + 1];
//static void (*key_destructor[PTHREAD_KEYS_MAX])(void *);

pthread_mutex_t g_pthreads_data_mutex = PTHREAD_MUTEX_INITIALIZER;

//=============================================================================
// Exported variables

AARCHPTR pthread_canceled_dummy_var; /* pointed to by PTHREAD_CANCELED */

//=============================================================================
// Internal functions

//-----------------------------------------------------------------------------
// Private version of pthread_self() that returns a pointer to our internal
// control structure.
_pthread_data *pthread_get_self_data(void)
{
    unsigned int Running_TaskPID = (g_stLosTask.pstRunTask)->uwTaskID;
    _pthread_data *data = &PTHREADS_DATA[Running_TaskPID];

    return data;
}

_pthread_data *pthread_get_data(pthread_t id)
{
    _pthread_data *data = (_pthread_data *)NULL;

    if (id >= (pthread_t)g_uwTskMaxNum || id < 0) {
        return (_pthread_data *)NULL;
    }

    data = &PTHREADS_DATA[id];

    /* Check that this is a valid entry */
    if (data->state == PTHREAD_STATE_FREE || data->state == PTHREAD_STATE_EXITED) {
        return (_pthread_data *)NULL;
    }

    /* Check that the entry matches the id */
    if (data->id != id) {
        return (_pthread_data *)NULL;
    }

    /* Return the pointer */
    return data;
}

//-----------------------------------------------------------------------------
// Check whether there is a cancel pending and if so, whether
// cancellations are enabled. We do it in this order to reduce the
// number of tests in the common case - when no cancellations are
// pending.
// We make this inline so it can be called directly below for speed
static int checkforcancel(void)
{
    _pthread_data *self = pthread_get_self_data();

    if (self->canceled && self->cancelstate == PTHREAD_CANCEL_ENABLE) {
        return 1;
    } else {
        return 0;
    }
}

//-----------------------------------------------------------------------------
// The (Grim) Reaper.
// This function is called to tidy up and dispose of any threads that have
// exited. This work must be done from a thread other than the one exiting.
// Note: this function _must_ be called with pthread_mutex locked.

static void pthread_reap(void)
{
    unsigned int i;

    /* Loop over the thread table looking for exited threads. The
       pthreads_exited counter springs us out of this once we have
       found them all (and keeps us out if there are none to do).
       */

    for (i = 0; pthreads_exited && i < g_uwTskMaxNum; i++) {
        _pthread_data *data = &PTHREADS_DATA[i];

        if (data != NULL && data->state == PTHREAD_STATE_EXITED) {
            /* the Huawei LiteOS not delete the dead TCB,so need to delete the TCB */
            (void)LOS_TaskDelete(data->task->uwTaskID);

            if (data->task->usTaskStatus == OS_TASK_STATUS_UNUSED) {

                /* posix_thread_sigdestroy(thread);
                   release the sem
                   */
                if (data->joiner) {
                    if(LOS_SemDelete(data->joiner->usSemID) != LOS_OK){
                        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
                    }
                }

                data->state = PTHREAD_STATE_FREE;
                memset(data, 0, sizeof(_pthread_data));

                pthreads_exited--;
            }
        }
    }
}

//=============================================================================
// General thread operations
//-----------------------------------------------------------------------------
// Thread creation and management.
int pthread_create(pthread_t *thread,
        const pthread_attr_t *attr,
        void *(*start_routine)(void *),
        void *arg)
{
    pthread_attr_t user_attr;
    unsigned int uwRet;
    char name[NAME_MAX];
    static unsigned short pthread_number = 1;
    LOS_TASK_CB *pstTaskCB;
    TSK_INIT_PARAM_S stTskInitParam;
    unsigned int uwTskHandle, threadTemp;
    UINT32 uwSemHandle;
    _pthread_data *created, *self;

    if (thread == NULL || start_routine == NULL) {
        return EINVAL;
    }

    self = pthread_get_self_data();

    /* Set use_attr to the set of attributes we are going to
       actually use. Either those passed in, or the default set.
       */

    if (attr == NULL) {
        (void)pthread_attr_init(&user_attr);
    } else {
        user_attr = *attr;
    }

    /* If the stack size is not valid, we can assume that it is at
       least PTHREAD_STACK_MIN bytes.
       */
    if (user_attr.stacksize_set) {
        stTskInitParam.uwStackSize = user_attr.stacksize;
    } else {
        stTskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
        user_attr.stacksize        = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    }

    (void)snprintf(name, sizeof(name), "pth%02d", pthread_number);

    pthread_number++;
    stTskInitParam.pcName = name;

    stTskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)start_routine;
    stTskInitParam.auwArgs[0] = (AARCHPTR)arg;

    if (user_attr.inheritsched == PTHREAD_INHERIT_SCHED) {
        if (self->task == NULL)
        {
            user_attr.schedparam.sched_priority = g_stLosTask.pstRunTask->usPriority;
        }
        else
        {
            user_attr.schedpolicy     = self->attr.schedpolicy;
            user_attr.schedparam      = self->attr.schedparam;
            user_attr.scope           = self->attr.scope;
        }
    }

    stTskInitParam.usTaskPrio = (UINT16)user_attr.schedparam.sched_priority;

    /* set the pthread default joinable */
    stTskInitParam.uwResved = 0;
    pthread_reap();

    LOS_TaskLock();
    uwRet = LOS_TaskCreate(&uwTskHandle, &stTskInitParam);

    if (uwRet == LOS_OK) {
        threadTemp  = uwTskHandle;
        *thread     = threadTemp;
        pstTaskCB   = OS_TCB_FROM_TID(threadTemp);
        created     = (_pthread_data *)&PTHREADS_DATA[threadTemp];

        user_attr.stacksize     = pstTaskCB->uwStackSize;
        pstTaskCB->usTaskStatus = user_attr.detachstate == PTHREAD_CREATE_JOINABLE ? \
                                  (pstTaskCB->usTaskStatus & ~LOS_TASK_STATUS_DETACHED) : (pstTaskCB->usTaskStatus | LOS_TASK_STATUS_DETACHED);

        strncpy(created->name, name, sizeof(created->name) - 1);
        pstTaskCB->pcTaskName   = created->name;

        created->attr           = user_attr;

        created->id             = threadTemp;
        created->task           = pstTaskCB;
        created->state          = user_attr.detachstate == PTHREAD_CREATE_JOINABLE ? \
                                  PTHREAD_STATE_RUNNING : PTHREAD_STATE_DETACHED;  /* is it true? */
        /* need to confirmation */
        created->cancelstate    = PTHREAD_CANCEL_ENABLE;
        created->canceltype     = PTHREAD_CANCEL_DEFERRED;
        created->cancelbuffer   = (struct pthread_cleanup_buffer *)NULL;
        created->canceled       = 0;

        created->freestack      = 0; /* no use default : 0 */
        created->stackmem       = pstTaskCB->uwTopOfStack;

        created->thread_data    = (void **)NULL;

        if (created->state == PTHREAD_STATE_RUNNING) {
            uwRet = LOS_SemCreate(0, &uwSemHandle);
            if (uwRet) {
                goto _error_out_with_task;
            }
            created->joiner = GET_SEM(uwSemHandle);
            created->task->pThreadJoin = (void *)GET_SEM(uwSemHandle);
        } else {
            created->joiner = (SEM_CB_S *)NULL;
        }

        LOS_TaskUnlock();
    }

    if (uwRet == LOS_OK) {
        return ENOERR;
    } else {
        goto _error_out;
    }

_error_out_with_task:
    created->state = PTHREAD_STATE_FREE;
    (void)LOS_TaskDelete(uwTskHandle);
_error_out:
    *thread = (pthread_t)-1;
    LOS_TaskUnlock();

    return map_errno(uwRet);
}

void pthread_exit(void *retval)
{
    _pthread_data *self;
    UINTPTR uvIntSave;

    self = pthread_get_self_data();
    if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, (int *)0) != ENOERR){
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
    }

    if(pthread_mutex_lock(&g_pthreads_data_mutex) != ENOERR){
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
    }

    self->task->pThreadJoinRetval = retval;
    /* If we are already detached, go to EXITED state, otherwise
       go into JOIN state.
       */

    if (self->state == PTHREAD_STATE_DETACHED) {
        self->state = PTHREAD_STATE_EXITED;
        pthreads_exited++;
    } else {
        self->state = PTHREAD_STATE_JOIN;
    }

    if(pthread_mutex_unlock(&g_pthreads_data_mutex) != ENOERR){
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
    }
    /* Kick any waiting joiners */

    if (self->task->pThreadJoin) {
        if(LOS_SemPost(self->joiner->usSemID) != ENOERR){
            PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
        }
    }


    uvIntSave = LOS_IntLock();
    /* if the thread is the highest thread,it can't schedule in LOS_SemPost. */
    if (OS_TASK_STATUS_READY & self->task->usTaskStatus) {
        osPriqueueDequeue(&(self->task->stPendList));
        self->task->usTaskStatus &= (~OS_TASK_STATUS_READY);
    }
    (VOID)LOS_IntRestore(uvIntSave);

    LOS_Schedule();
}

int pthread_join(pthread_t thread, void **retval)
{ /*lint -e613*/
    UINT32 uwRet = ENOERR;
    int err = 0;
    UINT32 uwStatus = 0;
    _pthread_data *self = (_pthread_data *)NULL;
    _pthread_data *joined = (_pthread_data *)NULL;

    /* check for cancellation first. */
    pthread_testcancel();

    // Dispose of any dead threads
    (void)pthread_mutex_lock(&g_pthreads_data_mutex);
    pthread_reap();
    (void)pthread_mutex_unlock(&g_pthreads_data_mutex);


    self   = pthread_get_self_data();
    joined = pthread_get_data(thread);

    if (joined == NULL) {
        err = ESRCH;
        return err;
    }
    else
    {
        uwStatus = joined->state;
    }

    if (joined == self) {
        err = EDEADLK;
        return err;
    }

    if (!err) {

        switch (joined->state) {
        case PTHREAD_STATE_RUNNING:
            /* The thread is still running, we must wait for it. */
            if (joined->joiner) {
                joined->state = PTHREAD_STATE_ALRDY_JOIN;
                uwRet = LOS_SemPend(joined->joiner->usSemID, LOS_WAIT_FOREVER);
            }

            if (uwRet != LOS_OK) {
                err = ESRCH;
                break;
            }

            /* check that the thread is still joinable */
            if (joined->state == PTHREAD_STATE_JOIN) {
                break;
            }
            break;
            /* The thread has become unjoinable while we waited, so we
               fall through to complain.
               */

        case PTHREAD_STATE_FREE:
        case PTHREAD_STATE_DETACHED:
        case PTHREAD_STATE_EXITED:
            /* None of these may be joined. */
            err = EINVAL;
            break;
        case PTHREAD_STATE_ALRDY_JOIN:
            err = EINVAL;
            break;
        case PTHREAD_STATE_JOIN:
            break;
        }
    }

    (void)pthread_mutex_lock(&g_pthreads_data_mutex);

    if (!err) {
        /* here, we know that joinee is a thread that has exited and is
           ready to be joined.
           */

        /* Get the retval */
        if (retval != NULL) {
            *retval = joined->task->pThreadJoinRetval;
        }

        /* set state to exited. */
        joined->state = PTHREAD_STATE_EXITED;
        pthreads_exited++;

        /* Dispose of any dead threads */
        pthread_reap();
    }
    else if(joined != NULL)
    {
        joined->state = uwStatus;
    }

    (void)pthread_mutex_unlock(&g_pthreads_data_mutex);
    /* check for cancellation before returning */
    pthread_testcancel();

    return err;
} /*lint +e613*/

//-----------------------------------------------------------------------------
// Set the detachstate of the thread to "detached". The thread then does not
// need to be joined and its resources will be freed when it exits.
int pthread_detach(pthread_t thread)
{
    int ret = 0;
    LOS_TASK_CB *pstTaskCB;
    _pthread_data *detache = (_pthread_data *)NULL;

    if(pthread_mutex_lock(&g_pthreads_data_mutex) != ENOERR)
        ret = ESRCH;
    detache = pthread_get_data(thread);

    if (detache == NULL) {
        ret = ESRCH; /* No such thread */
    } else if (detache->state == PTHREAD_STATE_DETACHED) {
        ret = EINVAL; /* Already detached! */
    } else if (detache->state == PTHREAD_STATE_JOIN) {
        detache->state = PTHREAD_STATE_EXITED;
        pthreads_exited++;

    } else {
        /* Set state to detached and kick any joinees to
           make them return.
           */
        detache->state = PTHREAD_STATE_DETACHED;
        if (detache->joiner) {
            if(LOS_SemPost(detache->joiner->usSemID) != LOS_OK)
                ret = ESRCH;
            if(LOS_SemDelete(detache->joiner->usSemID) != LOS_OK)
                ret = ESRCH;
        }
        detache->joiner = (SEM_CB_S *)NULL;
        detache->task->pThreadJoin = NULL;

        pstTaskCB = OS_TCB_FROM_TID(thread);
        if (pstTaskCB->usTaskStatus == 0) {
            detache->state = PTHREAD_STATE_EXITED;
            pthreads_exited++;
        } else {
            pstTaskCB->usTaskStatus |= LOS_TASK_STATUS_DETACHED;
        }
    }

    /* Dispose of any dead threads */
    pthread_reap();
    if(pthread_mutex_unlock(&g_pthreads_data_mutex) != ENOERR)
        ret = ESRCH;

    return ret;
}

//-----------------------------------------------------------------------------
// Thread scheduling controls

//-----------------------------------------------------------------------------
// Set scheduling policy and parameters for the thread
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    _pthread_data *data = (_pthread_data *)NULL;
    int ret = ENOERR;

    if (param == NULL || param->sched_priority > OS_TASK_PRIORITY_LOWEST) {
        return EINVAL;
    }

    if (policy != SCHED_RR) {
        return EINVAL;
    }

    /* The parameters seem OK, change the thread... */
    ret = pthread_mutex_lock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret; /*lint !e454*/

    data = pthread_get_data(thread);
    if (data == NULL) {
        ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
        if(ret != ENOERR)
            return ret;
        return ESRCH;
    }

    /* Only support one policy now */
    data->attr.schedpolicy = SCHED_RR;
    data->attr.schedparam  = *param;

    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret;
    (void)LOS_TaskPriSet(thread, (UINT16)param->sched_priority);

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Get scheduling policy and parameters for the thread
int pthread_getschedparam (pthread_t thread, int *policy, struct sched_param *param)
{
    _pthread_data *data = (_pthread_data *)NULL;
    int ret = ENOERR;

    if (policy == NULL || param == NULL) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret; /*lint !e454*/

    data = pthread_get_data(thread);
    if (data == NULL) {
        goto errout;
    }

    *policy = data->attr.schedpolicy;
    *param = data->attr.schedparam;

    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    return ret;
errout:
    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret;
    return ESRCH;

}

//=============================================================================
// Dynamic package initialization
// Call init_routine just the once per control variable.
int pthread_once (pthread_once_t *once_control, void (*init_routine)(void))
{
    pthread_once_t old;
    int ret = ENOERR;

    if (once_control == NULL || init_routine == NULL) {
        return EINVAL;
    }

    /* Do a test and set on the once_control object. */
    ret = pthread_mutex_lock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret; /*lint !e454*/

    old = *once_control;
    *once_control = 1;

    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret;
    /* If the once_control was zero, call the init_routine(). */
    if (!old) {
        init_routine();
    }

    return ENOERR;
}

//=============================================================================
// Thread specific data
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    PRINT_ERR("[%s] is not support.\n", __FUNCTION__);
    return 0;
}

//-----------------------------------------------------------------------------
// Store the pointer value in the thread-specific data slot addressed
// by the key.
int pthread_setspecific(pthread_key_t key, const void *pointer)
{
    PRINT_ERR("[%s] is not support.\n", __FUNCTION__);
    return 0;
}

//-----------------------------------------------------------------------------
// Retrieve the pointer value in the thread-specific data slot addressed
// by the key.
void *pthread_getspecific(pthread_key_t key)
{
    PRINT_ERR("[%s] is not support.\n", __FUNCTION__);
    return NULL;
}

//=============================================================================
// Thread Cancellation Functions

//-----------------------------------------------------------------------------
// Set cancel state of current thread to ENABLE or DISABLE.
// Returns old state in *oldstate.
int pthread_setcancelstate(int state, int *oldstate)
{
    _pthread_data *self = (_pthread_data *)NULL;
    int ret = ENOERR;

    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret; /*lint !e454*/

    self = pthread_get_self_data();

    if (oldstate != NULL) {
        *oldstate = self->cancelstate;
    }

    self->cancelstate = (UINT8)state;

    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Set cancel type of current thread to ASYNCHRONOUS or DEFERRED.
// Returns old type in *oldtype.
int pthread_setcanceltype(int type, int *oldtype)
{
    _pthread_data *self = (_pthread_data *)NULL;
    int ret = ENOERR;

    if (type != PTHREAD_CANCEL_ASYNCHRONOUS && type != PTHREAD_CANCEL_DEFERRED) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret; /*lint !e454*/

    self = pthread_get_self_data();

    if (oldtype != NULL) {
        *oldtype = self->canceltype;
    }

    self->canceltype = (UINT8)type;

    ret = pthread_mutex_unlock(&g_pthreads_data_mutex);
    if(ret != ENOERR)
        return ret;

    return ENOERR;
}

//-----------------------------------------------------------------------------
// Cancel the thread.
int pthread_cancel(pthread_t thread)
{
    _pthread_data *data = (_pthread_data *)NULL;

    if(pthread_mutex_lock(&g_pthreads_data_mutex) != ENOERR)
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);

    data = pthread_get_data(thread);

    if (data == NULL) {
        if(pthread_mutex_unlock(&g_pthreads_data_mutex) != ENOERR)
            PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
        return ESRCH;
    }

    data->canceled = 1;

    if (data->cancelstate == PTHREAD_CANCEL_ENABLE) {
        if (data->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS) {
            /* If the thread has cancellation enabled, and it is in
               asynchronous mode, set the eCos thread's ASR pending to
               deal with it when the thread wakes up. We also release the
               thread out of any current wait to make it wake up.
               */

            LOS_TaskLock();
            data->canceled = 0;
            if((data->task->usTaskStatus == 0) || LOS_TaskSuspend(data->task->uwTaskID ) != ENOERR ){
                goto _error_out;
            }
            if (data->task->pThreadJoin) {
                if( LOS_SemPost(((SEM_CB_S *)(data->joiner))->usSemID) != ENOERR ){
                    goto _error_out;
                }
                data->task->pThreadJoin = NULL;
                pthread_canceled_dummy_var = (AARCHPTR)PTHREAD_CANCELED;
                data->task->pThreadJoinRetval = (VOID *)pthread_canceled_dummy_var;
            } else if (data->state && !(data->task->usTaskStatus & OS_TASK_STATUS_UNUSED)) {
                data->state = PTHREAD_STATE_EXITED;
                pthreads_exited++;
                pthread_reap();
            } else {
                goto _error_out;
            }
            LOS_TaskUnlock();
        }
    }

    /* Otherwise the thread has cancellation disabled, in which case
       it is up to the thread to enable cancellation
       */

    if(pthread_mutex_unlock(&g_pthreads_data_mutex) != ENOERR)
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);

    return ENOERR;
_error_out:
    LOS_TaskUnlock();
    if(pthread_mutex_unlock(&g_pthreads_data_mutex) != ENOERR)
        PRINT_ERR("%s is not support %d\n",__FUNCTION__,__LINE__);
    return ESRCH;
}

//-----------------------------------------------------------------------------
// Test for a pending cancellation for the current thread and terminate
// the thread if there is one.
void pthread_testcancel(void)
{
    if (checkforcancel()) {
        /* If we have cancellation enabled, and there is a cancellation
           pending, then go ahead and do the deed.
           */

        /* Exit now with special retval. pthread_exit() calls the
           cancellation handlers implicitly.
           */
        pthread_exit((void *)PTHREAD_CANCELED);
    }
}

//-----------------------------------------------------------------------------
// Get current thread id.
pthread_t pthread_self(void)
{
    _pthread_data *data = pthread_get_self_data();

    return data->id;
}

//-----------------------------------------------------------------------------
// Compare two thread identifiers.
int pthread_equal(pthread_t thread1, pthread_t thread2)
{
    return thread1 == thread2;
}

//-----------------------------------------------------------------------------
// These two functions actually implement the cleanup push and pop functionality.
void pthread_cleanup_push_inner(struct pthread_cleanup_buffer *buffer,
        void (*routine)(void *),
        void *arg)
{
    PRINT_ERR("[%s] is not support.\n", __FUNCTION__);
    return;
}

void pthread_cleanup_pop_inner(struct pthread_cleanup_buffer *buffer, int execute)
{
    PRINT_ERR("[%s] is not support.\n", __FUNCTION__);
    return;
}
