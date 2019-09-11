/*----------------------------------------------------------------------------
 *      Huawei - Huawei LiteOS
 *----------------------------------------------------------------------------
 *      Name:    Workqueue.h
 *      Purpose: High-precision-software-timer-related functions
 *      Rev.:    V1.0.0
 *----------------------------------------------------------------------------
 *

 * Copyright (c) 2014, Huawei Technologies Co., Ltd.
 * All rights reserved.
 *---------------------------------------------------------------------------*/

 /** @defgroup workqueue Workqueue
 *  @ingroup linux
*/
#ifndef __LITEOS_WORKQUEUE_H__
#define __LITEOS_WORKQUEUE_H__

#include "los_task.h"
#include <semaphore.h>
#include "los_memory.h"
#include "los_event.h"
#include "pthread.h"
#include "linux/list.h"
#include "linux/timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


struct lock_class_key {
};

/**
 * @ingroup los_task
 * Define a usable work priority.
 *
 * Highest task priority.
 */
#define OS_WORK_PRIORITY_HIGHEST                    0

/**
 * @ingroup los_task
 * Define a usable work priority.
 *
 * Lowest task priority.
 */
#define OS_WORK_PRIORITY_LOWEST                     31


/**
 * @ingroup los_task
 * Define a usable work priority.
 *
 * Default task priority.
 */
#define OS_WORK_PRIORITY_DEFAULT                     OS_WORK_PRIORITY_LOWEST

/**
 * @ingroup workqueue
 * System default workqueue.
 *
 */
extern struct workqueue_struct *g_pstSystemWq;

/**
 * @ingroup workqueue
 * Workqueue task control block structure.
 *
 */
typedef struct LOS_TASK_CB task_struct;

/**
 * @ingroup workqueue
 * Define atomic_long_t as a signed integer.
 *
 */
typedef long atomic_long_t;

struct work_struct;

/**
 * @ingroup workqueue
 * Workqueue handling function.
 *
 */
typedef void (*work_func_t)(struct work_struct *);


/**
 * @ingroup workqueue
 * Work structure.
 * A work is a node in a workqueue.
 *
 */
struct work_struct
{
    atomic_long_t data;        /**< Input parameter of the work handling function*/
    struct list_head entry;    /**< Pointer to a doubly linked list of a work*/
    work_func_t func;           /**< Work handling function*/
    UINT32 work_status;     /**< Work status*/
    UINT32 work_pri;
};

/**
 * @ingroup workqueue
 * delayed_work structure.
 * A delayed_work is a work that is delayed to be mounted to a workqueue.
 *
 */
struct delayed_work
{
    struct work_struct work;           /**< Work structure*/
    struct timer_list timer;              /**< Delay control block parameter structure*/
    struct workqueue_struct *wq;    /**< Workqueue that contains the delayed_work structure*/
    int cpu;                                   /**< Number of CPUs. Not in use temporarily.*/
};

/**
 * @ingroup workqueue
 * Workqueue control structure.
 * It awakes a workqueue or makes a workqueue sleep.
 *
 */
typedef struct tag_cpu_workqueue_struct
{
    struct list_head worklist;               /**< Pointer to a work doubly linked list*/
    struct work_struct *current_work; /**< Work that is being executed*/
    struct workqueue_struct *wq;        /**< Workqueue that contains the workqueue control structure*/
    task_struct *thread;                     /**< Workqueue handling thread*/
}cpu_workqueue_struct;

/**
 * @ingroup workqueue
 * Definition of a workqueue structure.
 *
 */
struct workqueue_struct
{
    cpu_workqueue_struct *cpu_wq;   /**< Workqueue control structure*/
    struct list_head list;                     /**< Pointer to a workqueue doubly linked list*/
    EVENT_CB_S wq_event;               /**< Event of a workqueue */
    unsigned int wq_id;                                   /**< Workqueue ID  */
    int delayed_work_count;             /**< Number of delayed works in a workqueue*/
    char *name;                                /**< Workqueue name*/
    int singlethread;                         /**< Whether to create a new working task. 0 indicates that the default working task will be used.*/
    int wq_status;                            /**< Workqueue status*/
    int freezeable;                            /**< Not in use temporarily*/
    int rt;                                         /**< Not in use temporarily*/
};

/**
 * @ingroup workqueue
 * Work status enumeration.
 *
 */
enum work_status
{
    WORK_BUSY_PENDING    = 1<<0,    /**< The status of work item is pending execution*/
    WORK_BUSY_RUNNING    = 1<<1,    /**< The status of work item is running*/
    WORK_STRUCT_PENDING = 1<<0,    /**< work item is pending execution*/
    WORK_STRUCT_RUNNING = 1<<1,   /**< work item is running*/
};

/**
 * @ingroup workqueue
 * Initialize a work.
 *
 */
#define INIT_WORK(_work, _func)          \
    do {      \
        INIT_LIST_HEAD(&((_work)->entry));    \
        (_work)->func = (_func);            \
        (_work)->data = (atomic_long_t)(0);     \
        (_work)->work_status = 0;            \
        (_work)->work_pri = OS_WORK_PRIORITY_DEFAULT; \
    } while (0)

/**
 * @ingroup  workqueue
 * @brief Initialize a delayed work.
 *
 *@par Description:
 * This API is used to initialize a delayed work.
 *
 * @attention
 * <ul>
 * <li>The parameter __work and _func shoud be valid memory, otherwise, the system may be abnormal. </li>
 * </ul>
 *
 * @param  __work   [IN] Work handle.
 * @param  _func    [IN] Executive function.
 *
 * @retval  None.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see none.
 * @since Huawei LiteOS V100R001C00
 */
 extern void init_delayed_work(struct delayed_work *__work,work_func_t _func);

/**
 * @ingroup workqueue
 * Initialize a delayed work.
 *
 */
#define INIT_DELAYED_WORK(_work, _func)   init_delayed_work(_work, _func)

/**
 * @ingroup  workqueue
 * @brief Create a workqueue.
 *
 *@par Description:
 * This API is used to create a workqueue that has a specified name.
 *
 * @attention
 * <ul>
 * <li> The passed-in workqueue name is a character string that cannot be null and is the only identifier of the workqueue, make sure it is unique. </li>
 * </ul>
 *
 * @param  name  [IN] Workqueue name.
 *
 * @retval  NULL                   The workqueue fails to be created.
 * @retval  workqueue_struct*      The workqueue is successfully created.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see destroy_workqueue
 * @since Huawei LiteOS V100R001C00
 */
extern struct workqueue_struct * create_singlethread_workqueue(char *name);
/**
 * @ingroup  workqueue
 * @brief Create a workqueue.
 *
 *@par Description:
 * This API is used to create a workqueue that has a specified name.
 *
 * @attention
 * <ul>
 * <li> The passed-in workqueue name is a character string that cannot be null and is the only identifier of the workqueue, make sure it is unique. </li>
 * </ul>
 *
 * @param  name  [IN] Workqueue name.
 *
 * @retval  NULL                   The workqueue fails to be created.
 * @retval  workqueue_struct*      The workqueue is successfully created.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see destroy_workqueue
 * @since Huawei LiteOS V100R001C00
 */
#define create_workqueue(name)      create_singlethread_workqueue(name)

/**
 * @ingroup  workqueue
 * @brief Delete a workqueue.
 *
 *@par Description:
 * This API is used to delete a workqueue that has a specified handle.
 *
 * @attention
 * <ul>
 * <li>The name of the workqueue will be null and the workqueue cannot be used again after the workqueue is deleted. </li>
 * </ul>
 *
 * @param  wq  [IN] Workqueue handle.
 *
 * @retval  None.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see create_workqueue
 * @since Huawei LiteOS V100R001C00
 */
void destroy_workqueue(struct workqueue_struct *wq);

/**
 * @ingroup  workqueue
 * @brief Queue a work on a workqueue.
 *
 *@par Description:
 * This API is used to queue a work on a specified workqueue.
 *
 * @attention
 * <ul>
 * <li>The parameter wq and work shoud be valid memory, otherwise, the system may be abnormal. </li>
 * <li>The work will be immediately queued on the workqueue. </li>
 * </ul>
 *
 * @param  wq     [IN] Workqueue handle.
 * @param  work  [IN] Work handle.
 *
 * @retval #TRUE      The work is successfully queued on the workqueue.
 * @retval #FALSE     The work fails to be queued on the workqueue.
 * @par Dependency:
 * <ul>
 * <li>This function should be used after create_singlethread_workqueue() or create_workqueue() has been called</li>
 * <li>workqueue.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see cancel_work_sync
 * @since Huawei LiteOS V100R001C00
 */
bool queue_work(struct workqueue_struct *wq, struct work_struct *work);

/**
 * @ingroup  workqueue
 * @brief Queue a work on a workqueue after delay.
 *
 *@par Description:
 * This API is used to queue a work on a specified workqueue after delay.
 *
 * @attention
 * <ul>
 * <li>The parameter wq and dwork shoud be valid memory, otherwise, the system may be abnormal. </li>
 * <li>The work will be queued on the workqueue in a delayed period of time. </li>
 * <li>The work will be queued on the workqueue immediately if delaytime is 0, it as same as queue_work(). </li>
 * </ul>
 *
 * @param  wq            [IN] Workqueue handle.
 * @param  dwork        [IN] Delayed work handle.
 * @param  delaytime  [IN] Delayed time, number of ticks to wait or 0 for immediate execution.
 *
 * @retval #TRUE      The work is successfully queued on the workqueue.
 * @retval #FALSE     The work fails to be queued on the workqueue.
 * @par Dependency:
 * <ul>
 * <li>This function should be used after create_singlethread_workqueue() or create_workqueue() has been called</li>
 * <li>workqueue.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see cancel_delayed_work
 * @since Huawei LiteOS V100R001C00
 */
bool queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned int delaytime);

/**
 * @ingroup  workqueue
 * @brief Put a work in a default workqueue.
 *
 *@par Description:
 * This API is used to put a work in the default workqueue that is created when Huawei LiteOS is initialized.
 *
 * @attention
 * <ul>
 * <li>The parameter work shoud be valid memory, otherwise, the system may be abnormal. </li>
 * <li>The default workqueue is g_pstSystemWq. </li>
 * </ul>
 *
 * @param  work        [IN] Work handle.
 *
 * @retval #TRUE      The work is successfully put in the workqueue.
 * @retval #FALSE     The work fails to be put in the workqueue.
 * @par Dependency:
 * <ul>
 * <li>This function should be used after create_singlethread_workqueue() or create_workqueue() has been called</li>
 * <li>workqueue.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see cancel_work_sync
 * @since Huawei LiteOS V100R001C00
 */
bool schedule_work(struct work_struct *work);

/**
 * @ingroup  workqueue
 * @brief Put a work in a default workqueue after delay.
 *
 *@par Description:
 * This API is used to put a work in the default workqueue that is created when Huawei LiteOS is initialized in a delayed period of time.
 *
 * @attention
 * <ul>
 * <li>The parameter dwork shoud be valid memory, otherwise, the system may be abnormal. </li>
 * <li>The default workqueue is g_pstSystemWq. </li>
 * <li>The dwork will be queued on the workqueue immediately if delaytime is 0. </li>
 * </ul>
 *
 * @param  dwork          [IN] Delayed work handle.
 * @param  delaytime     [IN] Delayed time, number of ticks to wait or 0 for immediate execution.
 *
 * @retval #TRUE      The work is successfully put in the workqueue.
 * @retval #FALSE     The work fails to be put in the workqueue.
 * @par Dependency:
 * <ul>
 * <li>This function should be used after create_singlethread_workqueue() or create_workqueue() has been called</li>
 * <li>workqueue.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see cancel_delayed_work
 * @since Huawei LiteOS V100R001C00
 */
bool schedule_delayed_work(struct delayed_work *dwork, unsigned int delaytime);

/**
 * @ingroup  workqueue
 * @brief Query the work status.
 *
 *@par Description:
 * This API is used to query the status of a work and a delayed work.
 *
 * @attention
 * <ul>
 * <li>The parameter work shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  work          [IN] Work handle.
 *
 * @retval #WORK_BUSY_PENDING      The work is pending.
 * @retval #WORK_BUSY_RUNNING      The work is running.
 * @retval #FALSE                  The value of the parameter work is NULL.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V100R001C00
 */
unsigned int work_busy(struct work_struct *work);

/**
 * @ingroup  workqueue
 * @brief Immediately execute a delayed work.
 *
 *@par Description:
 * This API is used to immediately put a delayed work in a workqueue and wait for the execution of the delayed work to end.
 *
 * @attention
 * <ul>
 * <li>flush_delayed_work() should be used after queue_delayed_work() has been called.</li>
 * <li>The parameter dwork shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  dwork          [IN] Delayed work handle.
 *
 * @retval #TRUE      The operation succeeds.
 * @retval #FALSE     The operation fails.
 * @par Dependency
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V100R001C00
 */
bool flush_delayed_work(struct delayed_work *dwork);

/**
 * @ingroup  workqueue
 * @brief Cancel a delayed work.
 *
 *@par Description:
 * This API is used to cancel a delayed work, which means that the work will not be executed regardless of whether the delayed time has expired.
 *
 * @attention
 * <ul>
 * <li>cancel_delayed_work() should be used after queue_delayed_work() has been called.</li>
 * <li>The parameter dwork shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  dwork          [IN] Delayed work handle.
 *
 * @retval #TRUE      The delayed work is successfully canceled.
 * @retval #FALSE     The delayed work fails to be canceled.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see queue_delayed_work
 * @since Huawei LiteOS V100R001C00
 */
bool cancel_delayed_work(struct delayed_work *dwork);
/**
 * @ingroup  workqueue
 * @brief Cancel a delayed work and wait for it to finish.
 *
 *@par Description:
 * This API is used to cancel a delayed work, which means that the work will not be executed regardless of whether the delayed time has expired.
 *
 * @attention
 * <ul>
 * <li>cancel_delayed_work_sync() should be used after queue_delayed_work() has been called.</li>
 * <li>The parameter dwork shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  dwork          [IN] Delayed work handle.
 *
 * @retval #TRUE      The delayed work is successfully canceled.
 * @retval #FALSE     The delayed work fails to be canceled.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see queue_delayed_work
 * @since Huawei LiteOS V100R001C00
 */
bool cancel_delayed_work_sync(struct delayed_work *dwork);

/**
 * @ingroup  workqueue
 * @brief Immediately execute a work.
 *
 *@par Description:
 * This API is used to immediately execute a specified work and wait for the execution to end.
 *
 * @attention
 * <ul>
 * <li>flush_work() should be used after queue_work() has been called.</li>
 * <li>The parameter work shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  work          [IN] Work handle.
 *
 * @retval #TRUE      The operation succeeds.
 * @retval #FALSE     The operation fails.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V100R001C00
 */
bool flush_work(struct work_struct *work);

/**
 * @ingroup  workqueue
 * @brief Cancel a work.
 *
 *@par Description:
 * This API is used to cancel a work that is pending or running.
 *
 * @attention
 * <ul>
 * <li>cancel_work_sync() should be used after queue_work() has been called.</li>
 * <li>The parameter work shoud be valid memory, otherwise, the system may be abnormal.</li>
 * </ul>
 *
 * @param  work          [IN] Work handle.
 *
 * @retval #TRUE      The work is successfully canceled.
 * @retval #FALSE     The work fails to be canceled.
 * @par Dependency:
 * <ul><li>workqueue.h: the header file that contain the API declaration.</li></ul>
 * @see queue_work
 * @since Huawei LiteOS V100R001C00
 */
bool cancel_work_sync(struct work_struct *work);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LITEOS_WORKQUEUE_H__ */
