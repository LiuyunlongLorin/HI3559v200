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

/**@defgroup pthread Thread
 * @ingroup posix
 */

/**
 *@defgroup posix POSIX
 */

#ifndef _HWLITEOS_POSIX_PTHREAD_H
#define _HWLITEOS_POSIX_PTHREAD_H

#include "los_task.ph"
#include "sys/types.h"
#include "sched.h"
#include "bits/pthread_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/**
 * @ingroup pthread
 * Define the default mutex protocol as POSIX_MUTEX_DEFAULT_INHERIT.
 */
#ifndef POSIX_MUTEX_DEFAULT_INHERIT
#define POSIX_MUTEX_DEFAULT_INHERIT
#endif

/**
 * @ingroup pthread
 * Mutex types
 */
enum {
    PTHREAD_MUTEX_NORMAL = 0,                                /**< Normal mutex*/
    PTHREAD_MUTEX_RECURSIVE = 1,                             /**< Recursive mutex*/
    PTHREAD_MUTEX_ERRORCHECK = 2,                            /**< Error checking mutex*/

    PTHREAD_MUTEX_ERRORCHECK_NP = PTHREAD_MUTEX_ERRORCHECK,  /**< Error checking mutex*/
    PTHREAD_MUTEX_RECURSIVE_NP  = PTHREAD_MUTEX_RECURSIVE,   /**< Recursive mutex*/

    PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL             /**< Default mutex*/
};

/**
 * @ingroup pthread
 * Statically initialize a condition variable.
 */
#define PTHREAD_COND_INITIALIZER    {-1, {0, {NULL, NULL}} , NULL, -1}

/**
 * @ingroup pthread
 * Define a condition variable to be shared between threads within the same process.
 */
#define PTHREAD_PROCESS_PRIVATE 0

/**
 * @ingroup pthread
 * Define a condition variable to be shared among multiple processes.
 */
#define PTHREAD_PROCESS_SHARED  1

/**
 * @ingroup pthread
 * Condition variable attribute.
 */
typedef int pthread_condattr_t;

/**
 * @ingroup pthread
 * Define the default mutex protocol as POSIX_MUTEX_DEFAULT_PROTECT.
 */

/**
 * @ingroup pthread
 * Mutex attributes object
 */
typedef struct pthread_mutexattr
{
    UINT8 protocol;     /**< Mutex protocol. The value range is [0, 2].*/
    UINT8 prioceiling;  /**< Upper priority limit of a mutex. The value range is [0, 31].*/
    UINT8 type;         /**< Mutex type. The value range is [0, 2].*/
    UINT8 reserved;     /**< Reserved.*/
} pthread_mutexattr_t;

/**
 * @ingroup pthread
 * Mutex operation object
 */
typedef struct
{
    LOS_DL_LIST     stMuxList;      /**< Mutex blocking linked list*/
    LOS_TASK_CB     *pstOwner;      /**< Task that involves the mutex*/
    UINT16          usMuxCount;     /**< Mutex counter value*/
} MUX_CB_P;

/**
 * @ingroup pthread
 * Mutex object
 */
typedef struct pthread_mutex
{
    pthread_mutexattr_t stAttr; /**< Mutex attributes object*/
    MUX_CB_P stLock;            /**< Mutex operation object*/
} pthread_mutex_t;

/**
 * @ingroup pthread
 * Condition variable structure.
 */
typedef struct pthread_cond
{
    volatile int count;    /**< The number of tasks blocked by condition */
    EVENT_CB_S event; /**< Event object*/
    pthread_mutex_t *mutex; /**< Mutex locker for condition variable protection */
    volatile int value; /**< Condition variable state value*/
} pthread_cond_t;

/**
 * @ingroup pthread
 *Mutex protocol
 */
enum
{
    PTHREAD_PRIO_NONE = 0,  /**< No protocol*/
    PTHREAD_PRIO_INHERIT,   /**< Priority inheritance protocol*/
    PTHREAD_PRIO_PROTECT   /**< Priority protection protocol*/
};


#if defined POSIX_MUTEX_DEFAULT_INHERIT
/**
 * @ingroup pthread
 * Statically initialize a recursive mutex that applies the priority inheritance protocol
 */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
    {{PTHREAD_PRIO_INHERIT, OS_TASK_PRIORITY_LOWEST, PTHREAD_MUTEX_RECURSIVE_NP, 0} , \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
/**
 * @ingroup pthread
 * Statically initialize a normal mutex that applies the priority inheritance protocol
 */
#define PTHREAD_MUTEX_INITIALIZER \
    {{PTHREAD_PRIO_INHERIT, OS_TASK_PRIORITY_LOWEST, 0, 0} , \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
#elif defined POSIX_MUTEX_DEFAULT_PROTECT
/**
 * @ingroup pthread
 * Statically initialize a recursive mutex that applies the priority protection protocol
 */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
{{PTHREAD_PRIO_PROTECT, OS_TASK_PRIORITY_LOWEST, PTHREAD_MUTEX_RECURSIVE_NP, 0} , \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
/**
 * @ingroup pthread
 * Statically initialize a normal mutex that applies the priority protection protocol
 */
#define PTHREAD_MUTEX_INITIALIZER {{PTHREAD_PRIO_PROTECT, OS_TASK_PRIORITY_LOWEST, 0, 0} ,  \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
#else
/**
 * @ingroup pthread
 * Statically initialize a recursive mutex that applies no priority protocol
 */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
{{PTHREAD_PRIO_NONE, OS_TASK_PRIORITY_LOWEST, PTHREAD_MUTEX_RECURSIVE_NP, 0} , \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
/**
 * @ingroup pthread
 * Statically initialize a normal mutex that applies no priority protocol
 */
#define PTHREAD_MUTEX_INITIALIZER {{PTHREAD_PRIO_NONE, OS_TASK_PRIORITY_LOWEST, 0, 0} , \
    {{(struct LOS_DL_LIST *)NULL, (struct LOS_DL_LIST *)NULL}, \
    (LOS_TASK_CB *)NULL, 0}}
#endif


typedef int pthread_key_t;

typedef int pthread_once_t;

#define PTHREAD_STACK_MIN    LOS_TASK_MIN_STACK_SIZE

#define PTHREAD_CREATE_DETACHED  0x00000001
#define PTHREAD_CREATE_JOINABLE  0x00000000

#define PTHREAD_SCOPE_SYSTEM     0
#define PTHREAD_SCOPE_PROCESS    1

int pthread_atfork(void (*)(void), void (*)(void), void(*)(void));


/**
 *@ingroup pthread
 *@brief Initialize a condition variable.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to initialize a condition variable.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>A condition variable cannot be initialized by multiple threads. When a condition variable needs to be reinitialized, it must not have been used.</li>
 *</ul>
 *
 *@param cond       [OUT] Condition variable object.
 *@param attr         [IN]    Condition variable attribute. The passed value and the default value of this parameter can be only set to PTHREAD_PROCESS_PRIVATE.
 *
 *@retval   #EINVAL      One or more parameters are invalid.
 *@retval   #ENOERR      The condition variable is successfully initialized.
 *@retval   #ENOMEM      Failed to allocate in-memory resources for the operation.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_destroy
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

/**
 *@ingroup pthread
 *@brief Destroy a condition variable.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to destroy a condition variable.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The condition variable is using memory and not destroyed.</li>
 *</ul>
 *
 *@param cond       [IN]    Condition variable object.
 *
 *@retval   #ENVAIL  The parameter is invalid.
 *@retval   #EBUSY   The condition variable is being in use.
 *@retval   #ENOERR  The condition variable is successfully destroyed.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_init
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_destroy(pthread_cond_t * cond);

/**
 *@ingroup pthread
 *@brief Unblock a thread blocked on a condition variable.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to unblock a thread blocked on a condition variable and wakes this thread.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param cond       [IN]    Condition variable object.
 *
 *@retval   #ENVAIL   The parameter is invalid.
 *@retval   #ENOERR   The thread is successfully unlocked.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_wait
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_signal(pthread_cond_t *cond);

/**
 *@ingroup pthread
 *@brief Unblock all threads blocked on a condition variable.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to unblock all threads blocked on a condition variable and wake all these threads.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param cond       [IN]    Condition variable object.
 *
 *@retval   #ENVAIL  The parameter is invalid.
 *@retval   #ENOERR  All threads blocked on this condition variable are successfully unblocked.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_wait
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_broadcast(pthread_cond_t *cond);

/**
 *@ingroup pthread
 *@brief Blocking of a condition variable.
 *
 *@par Description:
 *<ul>
 *<li> A thread has been being blocked on a condition variable and waits to be awoken by the condition variable.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param cond       [IN]    Condition variable object.
 *@param mutex     [IN]    Mutex object.
 *
 *@retval   #ENVAIL  One or more parameters are invalid.
 *@retval   #ENOERR  The thread is successfully awoken.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_signal pthread_cond_broadcast
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

/**
 *@ingroup pthread
 *@brief Time-out blocking of a condition variable.
 *
 *@par Description:
 *<ul>
 *<li> A thread has been being blocked on a condition variable and is awoken until the set relative time has passed or the thread obtains a condition variable.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The waiting time is a relative time.</li>
 *<li> Setting the timeout interval to a past time period is not supported.</li>
 *</ul>
 *
 *@param cond       [IN]    Condition variable object.
 *@param mutex     [IN]     Mutex object.
 *@param abstime    [IN]    Time object.
 *
 *@retval   #ENOERR     The thread receives a signal and is successfully awoken.
 *@retval   #ENVAIL     One or more parameters are invalid.
 *@retval   #ETIMEDOUT  The waiting time has passed.
 *@retval   #ENOMEM     Failed to allocate in-memory resources for the operation.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_cond_wait
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

/**
 *@ingroup pthread
 *@brief Obtain the condition variable attribute.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to obtain the condition variable attribute.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The condition variable attribute can be only set to PTHREAD_PROCESS_PRIVATE.</li>
 *<li>This API does not task effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param attr       [IN]    Condition variable attribute.
 *@param pshared   [OUT]    Obtained condition variable attribute, which is always PTHREAD_PROCESS_PRIVATE.
 *
 *@retval   #ENVAIL  One or more parameters are invalid.
 *@retval   #ENOERR  The condition variable attribute is successfully obtained.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_condattr_setpshared
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared);

/**
 *@ingroup pthread
 *@brief Set the condition variable attribute.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to set the condition variable attribute.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The condition variable attribute can be only set to PTHREAD_PROCESS_PRIVATE.</li>
 *<li>This API does not task effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param attr       [IN]    Condition variable attribute.
 *@param pshared  [OUT]    Condition variable attribute, which is always PTHREAD_PROCESS_PRIVATE.
 *
 *@retval   #ENVAIL  One or more parameters are invalid.
 *@retval   #ENOERR  The condition variable attribute is successfully set.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_condattr_getpshared
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);

/**
 *@ingroup pthread
 *@brief Destroy the condition variable attribute.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to destroy the condition variable attribute.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The condition variable attribute can be only set to PTHREAD_PROCESS_PRIVATE.</li>
 *<li>This API does not task effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param attr   [IN]    Condition variable attribute.
 *
 *@retval   #ENVAIL     The parameter is invalid.
 *@retval   #ENOERR     The condition variable attribute is successfully destroied.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_condattr_init
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_condattr_destroy(pthread_condattr_t *attr);

/**
 *@ingroup pthread
 *@brief Initialize the condition variable attribute.
 *
 *@par Description:
 *<ul>
 *<li> This API is used to initialize the condition variable attribute.</li>
 *<li>This API does not task effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The condition variable attribute can be only set to PTHREAD_PROCESS_PRIVATE.</li>
 *</ul>
 *
 *@param attr       [OUT]    Condition variable attribute.
 *
 *@retval   #ENVAIL  The parameter is invalid.
 *@retval   #ENOERR  The condition variable attribute is successfully initialized.
 *@par Dependency:
 *<ul><li>pthread.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_condattr_setpshared pthread_condattr_getpshared
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_condattr_init(pthread_condattr_t *attr);

/* General thread operations */

/* Thread creation and management. */

/* Create a thread. */
/**
 *@ingroup pthread
 *@brief Create a thread.
 *
 *@par Description:
 *This API is used to create a thread according to input parameters.
 *@attention
 *<ul>
 *<li>The parameter value and return value of the thread entrance function must be void.*</li>
 *<li>A smaller priority value indicates a higher priority.</li>
 *</ul>
 *
 *@param pthread          [OUT] ID of the created thread.
 *@param attr             [IN]  Pointer to a thread attributes object. If the parameter value is NULL, the default thread attributes are applied.
 *@param start_routine    [IN]  Function entry of the thread.
 *@param arg              [IN]  Parameter of the thread entrance function.
 *
 *@retval   #EINVAL    The thread fails to be created due to incorrect parameters.
 *@retval   #ENOSPC    The thread fails to be created because No free task control block is available ,or the SEM control block is not enough.
 *@retval   #ENOMEM    The thread fails to be created because the maximum number of threads is exceeded, or memory allocation fails, or an interrupt in underway.
 *@retval   #ENOERR    The thread is successfully created.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_exit
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_create(pthread_t *pthread,
        const pthread_attr_t *attr,
        void *(*start_routine)(void *),
        void *arg);

/* Terminate current thread. */
/**
 *@ingroup pthread
 *@brief Terminate the current thread.
 *
 *@par Description:
 *This API is used to terminate the current thread, and the return value contains only one parameter.
 *@attention
 *<ul>
 *<li></li>
 *</ul>
 *
 *@param   retval  [OUT] Pointer to the thread return value.
 *
 *@retval   None.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_creat
 *@since Huawei LiteOS V100R001C00
 */
extern void pthread_exit(void *retval);

/* Wait for the thread to terminate. If thread_return is not NULL then
   the retval from the thread's call to pthread_exit() is stored at thread_return.
   */
/**
 *@ingroup pthread
 *@brief Wait for a thread to terminate.
 *
 *@par Description:
 *This API is used to wait for a thread to terminate and reclaim its resources.
 *@attention
 *<ul>
 *<li>A thread cannot be waited for by multiple threads. If a thread is waited for by multiple threads, ESRCH will be returned.</li>
 *</ul>
 *
 *@param   pthread          [IN]  ID of the target thread (the waited thread).
 *@param   thread_return    [OUT] Return value sent to the waiting thread.
 *
 *@retval   #ESRCH      The target thread is not joinable (the thread does not exist, or has exited, or is waited for by another thread).
 *@retval   #EDEADLK    A deadlock results because a thread is waiting for itself to terminate.
 *@retval   #EINVAL     The target thread is not joinable.
 *@retval   #ENOERR     The target thread is successfully joined with.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_detach
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_join(pthread_t pthread, void **thread_return);

/* Set the detachstate of the thread to "detached". The thread then does not
   need to be joined and its resources will be freed when it exits.
   */
/**
 *@ingroup pthread
 *@brief Mark a thread as detached.
 *
 *@par Description:
 *This API is used to mark a thread as detached. When a detached thread terminates, its resources are automatically released back to Huawei LiteOS without the need for another thread to join with the terminated thread.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param    pthread   [IN]  ID of the thread to be marked as detached.
 *
 *@retval   #ESRCH      The target thread cannot be marked (the thread does not exist or has exited).
 *@retval   #EINVAL     The target thread is already marked as detached.
 *@retval   #ENOERR     The thread is successfully marked.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_join
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_detach(pthread_t pthread);

/* Get current thread id. */
/**
 *@ingroup pthread
 *@brief Acquire the ID of a calling thread.
 *
 *@par Description:
 *This API is used to acquire the thread identifier of the calling thread.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param  NULL.
 *
 *@retval pthread_t Thread ID [0,LOSCFG_BASE_CORE_TSK_LIMIT].
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern pthread_t pthread_self(void);

/* Compare two thread identifiers. */
/**
 *@ingroup pthread
 *@brief Compare two thread IDs.
 *
 *@par Description:
 *This API is used to compare whether two thread IDs belong to the same thread.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param thread1  [IN]  ID of the thread 1 being compared.
 *@param thread2  [IN]  ID of the thread 2 being compared.
 *
 *@retval   int Any value other than 0  mean that two threads are equal.
 *@retval   0  The two threads are unequal.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_equal(pthread_t thread1, pthread_t thread2);

/* Thread attribute handling. */

/* Initialize attributes object with default attributes:
   detachstate          == PTHREAD_JOINABLE
   scope                == PTHREAD_SCOPE_SYSTEM
   inheritsched         == PTHREAD_EXPLICIT_SCHED
   schedpolicy          == SCHED_OTHER
   schedparam           == unset
   stackaddr            == unset
   stacksize            == 0
   */
/**
 *@ingroup pthread
 *@brief Initialize a thread attributes object.
 *
 *@par Description:
 *This API is used to initialize a thread attributes object into default attributes.
 *@attention
 *<ul>
 *<li>Default thread attributes</li>
 *<li>detachstate          = PTHREAD_JOINABLE</li>
 *<li>scope                = PTHREAD_SCOPE_SYSTEM</li>
 *<li>inheritsched         = PTHREAD_EXPLICIT_SCHED</li>
 *<li>schedpolicy          = SCHED_OTHER</li>
 *<li>schedparam           = unset</li>
 *<li>stackaddr            = unset</li>
 *<li>stacksize            = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE</li>
 *</ul>
 *
 *@param   attr    [IN] Pointer to the thread attributes object to be initialized.
 *
 *@retval  #EINVAL    The input parameter is NULL.
 *@retval  #ENOERR    The thread attributes object is successfully initialized.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_destroy
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_init(pthread_attr_t *attr);

/* Destroy thread attributes object */
/**
 *@ingroup pthread
 *@brief Destroy a thread attributes object.
 *
 *@par Dependency:
 *This API is used to destroy a thread attributes object.
 *@attention
 *<ul>
 *<li>This API does not take effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param   attr    [IN] Pointer to the thread attributes object to be destroyed.
 *
 *@retval  #EINVAL    The input parameter is NULL.
 *@retval  #ENOERR    The thread attributes object is successfully destroyed.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_init
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_destroy(pthread_attr_t *attr);

/* Set the detachstate attribute */
/**
 *@ingroup pthread
 *@brief Set the detach state attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the detach state attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>The detach state attribute must be either PTHREAD_CREATE_JOINABLE or PTHREAD_CREATE_DETACHED. Otherwise, the attempt to set the detach state attribute will fail.</li>
 *</ul>
 *
 *@param   attr          [OUT] Pointer to the thread attributes object to be set.
 *@param   detachstate   [IN]  Detach state attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The detach state attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getdetachstate
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

/* Get the detachstate attribute */
/**
 *@ingroup pthread
 *@brief Acquire the detach state attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the detach state attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr          [IN]  Pointer to the thread attributes object to be read.
 *@param   detachstate   [OUT] Pointer to the acquired detach state attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The detach state attribute is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setdetachstate
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

/* Set scheduling contention scope */
/**
 *@ingroup pthread
 *@brief Set the contention scope attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the contention scope attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>The contention scope attribute can only be PTHREAD_SCOPE_SYSTEM. PTHREAD_SCOPE_PROCES is not supported.</li>
 *</ul>
 *
 *@param   attr      [OUT] Pointer to the thread attributes object to be set.
 *@param   scope     [IN]  Contention scope attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOTSUP   The specified contention scope attribute value is not supported.
 *@retval  #ENOERR    The contention scope attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getscope
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setscope(pthread_attr_t *attr, int scope);

/* Get scheduling contention scope */
/**
 *@ingroup pthread
 *@brief Acquire the contention scope attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the contention scope attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr    [IN]  Pointer to the thread attributes object to be read.
 *@param   scope   [OUT] Pointer to the acquired contention scope attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The contention scope attribute is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setscope
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);

/* Set scheduling inheritance attribute */
/**
 *@ingroup pthread
 *@brief Set the inherit scheduler attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the inherit scheduler attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>The inherit scheduler attribute must be either PTHREAD_INHERIT_SCHED or PTHREAD_EXPLICIT_SCHED.</li>
 *</ul>
 *
 *@param   attr       [OUT] Pointer to the thread attributes object to be set.
 *@param   inherit      [IN]  Inherit scheduler attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The inherit scheduler attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getinheritsched
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setinheritsched (pthread_attr_t *attr, int inherit);

/* Get scheduling inheritance attribute */
/**
 *@ingroup pthread
 *@brief Acquire the inherit scheduler attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the inherit scheduler attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr      [IN]  Pointer to the thread attributes object to be read.
 *@param   inherit   [OUT] Pointer to the acquired inherit scheduler attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The inherit scheduler attribute is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setinheritsched
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit);

/* Set scheduling policy */
/**
 *@ingroup pthread
 *@brief Set the scheduling policy attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the scheduling policy attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>The scheduling policy attribute is SCHED_OTHER, SCHED_FIFO, or SCHED_RR.</li>
 *<li>Only SCHED_RR is supported now. An error code will be returned if the value is set to the other two scheduling policies.</li>
 *</ul>
 *
 *@param   attr    [OUT] Pointer to the thread attributes object to be set.
 *@param   policy  [IN]  Scheduling policy attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The scheduling policy attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getschedpolicy
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);

/* Get scheduling policy */
/**
 *@ingroup pthread
 *@brief Acquire the scheduling policy attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the scheduling policy attribute of a thread attributes object.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr      [IN]  Pointer to the thread attributes object to be read.
 *@param   policy    [OUT] Pointer to the acquired scheduling policy attribute.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The scheduling policy attribute is successfully obtained.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setschedpolicy
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);

/* Set scheduling parameters */
/**
 *@ingroup pthread
 *@brief Set the scheduling parameter attributes of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the scheduling parameter attributes of a thread attributes object. The scheduling parameter attributes indicate thread priorities.
 *@attention
 *<ul>
 *<li>The priority of the scheduling parameter attributes must be in the range of [OS_TSK_PRIORITY_LOWEST,OS_TSK_PRIORITY_HIGHEST].</li>
 *</ul>
 *
 *@param   attr    [OUT] Pointer to the thread attributes object to be set.
 *@param   param   [IN]  Pointer to the scheduling parameter attributes object.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The scheduling parameter attributes are successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getschedparam
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);

/* Get scheduling parameters */
/**
 *@ingroup pthread
 *@brief Acquire the scheduling parameter attributes of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the scheduling parameter attributes of a thread attributes object. The scheduling parameter attributes indicate the thread priorities.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr    [IN]  Pointer to the thread attributes object to be read.
 *@param   param   [OUT] Pointer to the acquired scheduling parameter attributes object.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The scheduling parameter attributes are successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setschedparam
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);

/* Set starting address of stack. Whether this is at the start or end of
   the memory block allocated for the stack depends on whether the stack
   grows up or down.
   */
/**
 *@ingroup pthread
 *@brief Set the stack address attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to set the stack address attribute of a thread attributes object. This attribute specifies the start address of a stack.
 *@attention
 *<ul>
 *<li>Critical errors will occur if the stack address is incorrectly set. Call this API with caution.</li>
 *<li>This API does not take effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param   attr        [OUT] Pointer to the thread attributes object to be set.
 *@param   stackaddr   [IN]  Specified stack address.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The stack address attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getstackaddr
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

/* Get any previously set stack address. */
/**
 *@ingroup pthread
 *@brief Acquire the stack address attribute of a thread attributes object.
 *
 *@par Description:
 *This API is used to acquire the stack address attribute of a thread attributes object. This attribute specifies the starting address of a stack.
 *@attention
 *<ul>
  *<li>This API does not take effect on Huawei LiteOS Kernel.</li>
 *</ul>
 *
 *@param   attr        [IN]  Pointer to the thread attributes object to be read.
 *@param   stackaddr   [OUT] Pointer to the acquired starting address of stack.
 *
 *@retval  #EINVAL    invalid parameter.
 *@retval  #ENOERR    The stack address attribute is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setstackaddr
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);

/* Set minimum creation stack size. */
/**
 *@ingroup pthread
 *@brief Set the stack size attribute of a thread attribute object.
 *
 *@par Description:
 *This API is used to set the stack size attribute of a thread attribute object.
 *@attention
 *<ul>
 *<li>The stack size must fall within an appropriate range and be greater than PTHREAD_STACK_MIN. If the stack size attribute is left unspecified, the default stack size will be used.</li>
 *</ul>
 *
 *@param   attr       [OUT] Pointer to the thread attributes object to be set.
 *@param   stacksize  [IN]  Specified stack size.
 *
 *@retval  #EINVAL   invalid parameter.
 *@retval  #ENOERR   The stack size attribute is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_getstacksize
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);

/* Get current minimal stack size. */
/**
 *@ingroup pthread
 *@brief Acquire the stack size attribute of a thread attribute object.
 *
 *@par Description:
 *This API is used to acquire the stack size attribute of a thread attribute object.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param   attr        [IN]  Pointer to the thread attributes object to be read.
 *@param   stacksize   [OUT] Pointer to the acquired stack size.
 *
 *@retval  #EINVAL   invalid parameter, or the stack size left unspecified.
 *@retval  #ENOERR   The stack size attribute is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_attr_setstacksize
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

/* Thread scheduling controls */

/* Set scheduling policy and parameters for the thread */
/**
 *@ingroup pthread
 *@brief Set the scheduling policy and priority of a thread.
 *
 *@par Description:
 *This API is used to set the scheduling policy and priority of a thread.
 *@attention
 *<ul>
 *<li>The scheduling policy must be SCHED_OTHER, SCHED_FIFO, or SCHED_RR.</li>
 *<li>The content priority specified by the param parameter must fall within the OS priority range[OS_TSK_PRIORITY_LOWEST,OS_TSK_PRIORITY_HIGHEST].</li>
 *<li>A smaller priority value indicates a higher priority.</li>
 *</ul>
 *@param   pthread   [IN] ID of the thread whose scheduling policy is to be set.
 *@param   policy    [IN] Specified scheduling policy.
 *@param   param     [IN] Pointer to the scheduling priority.
 *
 *@retval  #EINVAL   invalid parameter.
 *@retval  #ENOERR   The scheduling policy and priority of the thread is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_getschedparam
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_setschedparam(pthread_t pthread, int policy, const struct sched_param *param);

/* Get scheduling policy and parameters for the thread */
/**
 *@ingroup pthread
 *@brief Acquire the scheduling policy and priority of a thread.
 *
 *@par Description:
 *This API is used to acquire the scheduling policy and priority of a thread.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *@param   pthread   [IN]  ID of the thread whose scheduling policy is to be read.
 *@param   policy    [OUT] Acquired scheduling policy.
 *@param   param     [OUT] Acquired scheduling priority.
 *
 *@retval  #EINVAL   invalid parameter.
 *@retval  #ENOERR   The scheduling policy and priority of the thread is successfully acquired.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_setschedparam
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_getschedparam(pthread_t pthread, int *policy, struct sched_param *param);

/* Dynamic package initialization */

/* Initializer for pthread_once_t instances */
/**
 * @ingroup pthread
 * Define the initial value of pthread_once_t.
 */
#define PTHREAD_ONCE_INIT   0

/* Call init_routine just the once per control variable. */
/**
 *@ingroup pthread
 *@brief Call the initialization routine at most once.
 *
 *@par  Decription:
 *This API is used to call the initialization routine at most once.
 *@attention
 *<ul>
 *<li>The control variable once_control must be statically initialized using PTHREAD_ONCE_INIT. Otherwise, this API will not work as expected.</li>
 *</ul>
 *@param   once_control    [IN/OUT]  Once-control parameter.
 *@param   init_routine    [IN]      Initialization routine function.
 *
 *@retval  #EINVAL  : One or more parameters are invalid.
 *@retval  #1 The thread has already been initialized.
 *@retval  #0 The initialization routine is successfully called.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

/* Thread specific data */

/* Create a key to identify a location in the thread specific data area.
   Each thread has its own distinct thread-specific data area but all are
   addressed by the same keys. The destructor function is called whenever a
   thread exits and the value associated with the key is non-NULL.
   */
/**
 *@ingroup pthread
 *@brief Create a thread-specific data key.
 *
 *@par Description:
 *This API is used to create a key that indentifies the thread-specific data.
 *@attention
 *<ul>
 *<li>Not support.</li>
 *</ul>
 *@param   key           [OUT]  Pointer to a key value.
 *@param   destructor    [IN]   Destructor function.
 *
 *@retval  #EINVAL         invalid parameter.
 *@retval  #ENOERR         The thread-specific data key is successfully created.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_key_delete
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

/* Delete key. */
/**
 *@ingroup pthread
 *@brief Delete a thread-specific data key.
 *
 *@par Description:
 *This API is used to delete the thread-specific key created by pthread_key_create.
 *@attention
 *<ul>
 *<li>Not support.</li>
 *</ul>
 *@param   key      [IN]  Key to be deleted.
 *
 *@retval  #ENOERR     The thread-specific data key is successfully deleted.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration</li>.
 *</ul>
 *@see pthread_key_create
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_key_delete(pthread_key_t key);

/* Store the pointer value in the thread-specific data slot addressed
   by the key.
   */
/**
 *@ingroup pthread
 *@brief Set the sharing of specific data.
 *
 *@par Description:
 *This API is used to set a key as the location of the shared memory.
 *@attention
 *<ul>
 *<li>Not support.</li>
 *</ul>
 *@param   key           [IN]  Key to be set.
 *@param   pointer       [IN]  Location of the shared memory.
 *
 *@retval  #EINVAL     invalid parameter.
 *@retval  #ENOERR     The location of the shared memory is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_getspecific
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_setspecific(pthread_key_t key, const void *pointer);

/* Retrieve the pointer value in the thread-specific data slot addressed
   by the key.
   */
/**
 *@ingroup pthread
 *@brief Acquire the shared memory addressed by a key.
 *
 *@par Description:
 *This API is used to acquire the shared memory addressed by a key.
 *@attention
 *<ul>
 *<li>Not support.</li>
 *</ul>
 *@param   key  [IN]  Key by which the share memory to be acquired is addressed.
 *
 *@retval void* Location of the shared memory.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_setspecific
 *@since Huawei LiteOS V100R001C00
 */
extern void *pthread_getspecific(pthread_key_t key);

/* Thread Cancellation */

/* Data structure used to manage cleanup functions */
/**
 * @ingroup pthread
 * Structure for managing cleanup functions
 */
struct pthread_cleanup_buffer
{
    struct pthread_cleanup_buffer *prev;        /**< Pointer to the buffer to be cleaned*/
    void (*routine)(void *);                 /**< General execution function*/
    void *arg;                        /**< Parameter of the execution function*/
};

/* Thread cancelled return value.
   This is a value returned as the retval in pthread_join() of a
   thread that has been cancelled. By making it the address of a
   location we define we can ensure that it differs from NULL and any
   other valid pointer (as required by the standard).
   */
/**
 * @ingroup pthread
 * Declare a global variable that will be used to store the return value of pthread_cancele.
 */
extern AARCHPTR pthread_canceled_dummy_var;

#define PTHREAD_INHERIT_SCHED           0

#define PTHREAD_EXPLICIT_SCHED          1

/**
 * @ingroup pthread
 * Define the return value of pthread_cancele.
 */
#define PTHREAD_CANCELED    -1

/* Cancelability enable and type */
/**
 * @ingroup pthread
 * Enable cancellation.
 */
#define PTHREAD_CANCEL_ENABLE   1

/**
 * @ingroup pthread
 * Disable cancellation.
 */
#define PTHREAD_CANCEL_DISABLE  2

/**
 * @ingroup pthread
 * Asynchronous cancellation.
 */
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

/**
 * @ingroup pthread
 * Deferred cancellation.
 */
#define PTHREAD_CANCEL_DEFERRED 2

/* Functions */

/* Set cancel state of current thread to ENABLE or DISABLE.
   Returns old state in *oldstate.
   */
/**
 *@ingroup pthread
 *@brief Set the cancellation state (PTHREAD_CANCEL_ENABLE or PTHREAD_CANCEL_DISABLE) of a thread.
 *
 *@par Description:
 *This API is used to set the cancellation state (PTHREAD_CANCEL_ENABLE or PTHREAD_CANCEL_DISABLE) of a thread.
 *@attention
 *<ul>
 *<li>The cancellation state of a thread must be set to PTHREAD_CANCEL_ENABLE or PTHREAD_CANCEL_DISABLE. If it is set to neither of the two states, EINVAL will be returned.</li>
 *<li>The value of the oldstate parameter can be NULL, indicating that the original cancellation state will not be stored.</li>
 *</ul>
 *@param   state     [IN]   Cancellation state to be set.
 *@param   oldstate  [OUT]  Pointer to the original cancellation state.
 *
 *@retval  #EINVAL      invalid parameter.
 *@retval  #ENOERR      The cancellation state is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_setcancelstate(int state, int *oldstate);

/* Set cancel type of current thread to ASYNCHRONOUS or DEFERRED.
   Returns old type in *oldtype.
   */
/**
 *@ingroup pthread
 *@brief Set the cancellation type of a thread.
 *
 *@par Description:
 *This API is used to set the cancellation type (PHREAD_CANCEL_ASYNCHRONOUS or PTHREAD_CANCEL_DEFERRED) of a thread.
 *@attention
 *<ul>
 *<li>The cancellation type of a thread must be set to PHREAD_CANCEL_ASYNCHRONOUS or PTHREAD_CANCEL_DEFERRED. If it is set to neither of the two types, EINVAL will be returned.</li>
 *<li>The value of the oldtype parameter can be NULL, indicating that the original cancellation type will not be stored.</li>
 *</ul>
 *@param   type      [IN]   Cancellation type to be set.
 *@param   oldtype   [OUT]  Pointer to the original cancellation type.
 *
 *@retval  #EINVAL      invalid parameter.
 *@retval  #ENOERR      The cancellation type is successfully set.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_setcanceltype(int type, int *oldtype);

/* Cancel the thread. */
/**
 *@ingroup pthread
 *@brief Cancel a thread.
 *
 *@par Description:
 *This API is used to cancel a thread.
 *@attention
 *<ul>
 *<li>Before the pthread_cancel API is called, the PTHREAD_CANCEL_ASYNCHRONOUS attribute must be set.</li>
 *<li>The input parameter must be a legal thread id.</li>
 *</ul>
 *@param   pthread  [IN]  Thread to be canceled.
 *
 *@retval  #ESRCH      The thread does not exist.
 *@retval  #ENOERR     The thread is successfully canceled.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern int pthread_cancel(pthread_t pthread);

/* Test for a pending cancellation for the current thread and terminate
   the thread if there is one.
   */
/**
 *@ingroup pthread
 *@brief Check whether the current thread is in the Canceled state. The thread will exit if it is in the Canceled state.
 *
 *@par Description:
 *This API is used to check whether the current thread is in the Canceled state. If in Canceled state, the thread will exit. If not in Canceled state, no further action will be taken.
 *@attention
 *<ul>
 *<li>The current system does not support the Canceled point.</li>
 *</ul>
 *@param   None.
 *
 *@retval  None.
 *@par Dependency:
 *<ul>
 *<li>pthread.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern void pthread_testcancel(void);

/* Install a cleanup routine.
   Note that pthread_cleanup_push() and pthread_cleanup_pop() are macros that
   must be used in matching pairs and at the same brace nesting level.
   */

/* NOTE: The involved function is not currently supported. So the macro become invalid too. */
#define pthread_cleanup_push(__routine, __arg)  \
{   \
    struct pthread_cleanup_buffer _buffer_; \
    pthread_cleanup_push_inner (&_buffer_, (__routine), (__arg));

/* NOTE: The involved function is not currently supported. So the macro become invalid too. */
#define pthread_cleanup_pop(__execute)  \
    pthread_cleanup_pop_inner (&_buffer_, (__execute)); \
}

/* NOTE: This function is not currently supported. */
extern void pthread_cleanup_push_inner(struct pthread_cleanup_buffer *buffer, void (*routine)(void *), void *arg);
/* NOTE: This function is not currently supported. */
extern void pthread_cleanup_pop_inner(struct pthread_cleanup_buffer *buffer, int execute);

/* NOTE: This function is not currently supported. */
extern int pthread_mutex_timedlock(pthread_mutex_t *__mutex, const struct timespec * __abstime);

/**
 *@ingroup mutex
 *@brief Initialize a mutex.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to initialize a mutex has a specified mutex attribute.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>Do not initialize a mutex that has been initialized or destroyed.</li>
 *</ul>
 *
 *@param mutex   [OUT] Mutex object.
 *@param attr    [IN] Mutex attributes object. A mutex attribute value can be the default value or a user-defined value.
 *
 *@retval #EINVAL   invalid parameter.
 *@retval #ENOERR   The mutex is successfully initialized.
 *@par Dependency:
 *<ul><li>mutex.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_mutex_destroy
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

/**
 *@ingroup mutex
 *@brief Destroy a mutex.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to destroy a mutex.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param mutex   [IN] Mutex object.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #EBUSY   The mutex is being in use.
 *@retval #ENOERR  The mutex is successfully destroyed.
 *@par Dependency:
 *<ul><li>mutex.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_mutex_init
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_destroy(pthread_mutex_t *mutex);

/**
 *@ingroup mutex
 *@brief Lock a mutex.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to lock a mutex. The lock operation varies with the mutex type and protocol.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>A recursive mutex can be locked more than once. A normal mutex does not check errors, so a deadlock will result when a normal mutex is relocked. An error checking mutex is able to check errors. Error code will be returned if a mutex is relocked.</li>
 *</ul>
 *
 *@param mutex   [IN] Mutex object.
 *
 *@retval #EINVAL   invalid parameter.
 *@retval #EPERM    The operation is not allowed during an interrupt.
 *@retval #EDEADLK  Relocking a mutex causes a deadlock.
 *@retval #ENOERR   The mutex is successfully locked.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see pthread_mutex_destroy
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_lock(pthread_mutex_t *mutex);

/**
 *@ingroup mutex
 *@brief Unlock a mutex.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to unlock a mutex.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param mutex   [IN] Mutex object.
 *
 *@retval #EINVAL invalid parameter.
 *@retval #EPERM  The operation is not allowed during an interrupt.
 *@retval #ENOERR The mutex is successfully unlocked, or the mutex being unlocked has never been locked before.
 *@par Dependency:
 *<ul><li>mutex.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_mutex_lock
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/**
 *@ingroup mutex
 *@brief Try to lock a mutex.
 *
 *@par Description:
 *<ul>
 *<li>If you try to lock a mutex when it is already locked, a deadlock will result. This API is used to try to lock the mutex.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param mutex   [IN] Mutex object.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #EPERM   The operation is not allowed during an interrupt.
 *@retval #EBUSY   The lock is already locked. In this case, EBUSY will be returned if you try to lock a non-recursive mutex. If you try to lock a recursive mutex, it will be locked.
 *@retval #ENOERR  The operation is successful.
 *@retval #EDEADLK The mutex is being locked during a mutex task.
 *@par Dependency:
 *<ul><li>mutex.h: the header file that contains the API declaration.</li></ul>
 *@see pthread_mutex_lock,pthread_mutex_unlock
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_trylock(pthread_mutex_t *mutex);

/**
 *@ingroup mutex
 *@brief Obtain the mutex type.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to obtain the mutex type.</li>
 *</ul>
 *@attention
 *<ul>
 *<li></li>
 *</ul>
 *
 *@param attr   [IN] Mutex attributes.
 *@param type   [OUT] Mutex type.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The mutex type is successfully obtained.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_settype
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);

/**
 *@ingroup mutex
 *@brief Set the mutex type.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to set the mutex type.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param attr   [OUT] Mutex attributes.
 *@param type   [IN] Mutex type, [0, 2].
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The mutex type is successfully set.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_gettype
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

/**
 *@ingroup mutex
 *@brief Initialize mutex attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to initialize mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The default mutex protocol is _POSIX_THREAD_PRIO_INHERITand is assigned the lowest mutex priority and the default mutex type.</li>
 *</ul>
 *
 *@param attr   [IN/OUT] Mutex attributes.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The mutex attributes are successfully initialized.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_destroy
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_init(pthread_mutexattr_t *attr);

/**
 *@ingroup mutex
 *@brief Destroy mutex attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to destroy mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param attr   [IN] Mutex attributes.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The mutex attributes are successfully destroyed.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_init
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

/**
 *@ingroup mutex
 *@brief Set the protocol of a mutex for defining mutex attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to set the protocol for defining mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param attr       [OUT] Mutex attributes.
 *@param protocol   [IN] Protocol for defining mutex attributes, and the value range is [0, 2].
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The protocol is successfully set.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_getprotocol
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr,int protocol);

/**
 *@ingroup mutex
 *@brief Obtain the mutex protocol in mutex attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to obtain the mutex protocol in mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param attr       [IN] Mutex attributes.
 *@param protocol   [OUT] Mutex protocol in mutex attributes.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The mutex protocol in mutex attributes is successfully obtained.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_setprotocol
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_getprotocol(pthread_mutexattr_t *attr, int *protocol);


/**
 *@ingroup mutex
 *@brief Set the upper priority limit in mutex attributes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to set the upper priority limit in mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The default priority is the lowest priority. If the upper priority limit is below the default priority, priority inversion will occur.</li>
 *</ul>
 *
 *@param attr          [OUT] Mutex attributes.
 *@param prioceiling   [IN] Upper priority limit in mutex attributes. The priority range is [0, 31].
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The upper priority limit in mutex attributes is successfully set.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_getprioceiling
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);

/**
 *@ingroup mutex
 *@brief Obtain the upper priority limit in mutex attributes.
 *
 *@par Decription:
 *<ul>
 *<li>This API is used to obtain the upper priority limit in mutex attributes.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param attr          [IN] Mutex attributes.
 *@param prioceiling   [OUT] Upper priority limit in mutex attributes.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The upper priority limit in mutex attributes is successfully obtained.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutexattr_setprioceiling
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *attr, int *prioceiling);

/**
*@ingroup mutex
*@brief Set the upper priority limit of a mutex.
*
*@par Description:
*<ul>
*<li>This API is used to set the upper priority limit of a mutex. The difference between this API and pthread_mutexattr_setprioceiling is that the mutex might be in use when this API is called.</li>
*</ul>
*@attention
*<ul>
*<li>The default priority is the lowest priority. If the upper priority limit is below the default priority, priority inversion will occur.</li>
*</ul>
*
*@param mutex [IN] Mutex.
*@param prioceiling [OUT] New upper priority limit of the mutex. The priority range is [0, 31].
*@param old_ceiling [OUT] Original upper priority limit of the mutex.
*
*@retval #ENOERR The upper priority limit of the mutex is successfully set.
*@retval #EINVAL Invalid parameter.
*@retval #EPERM The operation is not allowed during an interrupt.
*@retval #EDEADLK Relocking a mutex causes a deadlock.
*
*@par Dependency:
*<ul>
*<li>mutex.h: the header file that contains the API declaration.</li>
*<li>pthread.h: the header file that contains the mutex attributes.</li>
*</ul>
*@see pthread_mutex_getprioceiling
*@since Huawei LiteOS V100R001C00
*/
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling);

/**
 *@ingroup mutex
 *@brief Obtain the upper priority limit of a mutex.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to obtain the upper priority limit of a mutex.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param mutex         [IN] Mutex.
 *@param prioceiling   [OUT] Upper priority limit of the mutex.
 *
 *@retval #EINVAL  invalid parameter.
 *@retval #ENOERR  The upper priority limit of the mutex is successfully obtained.
 *@par Dependency:
 *<ul>
 *<li>mutex.h: the header file that contains the API declaration.</li>
 *<li>pthread.h: the header file that contains the mutex attributes.</li>
 *</ul>
 *@see pthread_mutex_setprioceiling
 *@since Huawei LiteOS V100R001C00
 */
int pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *prioceiling);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
