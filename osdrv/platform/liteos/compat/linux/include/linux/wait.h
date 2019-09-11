/** @defgroup wait Wait
 *  @ingroup linux
*/
#ifndef __LINUX_WAIT_H__
#define __LINUX_WAIT_H__

#include "los_event.h"
#include "los_sys.h"
#include "pthread.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct wait_queue_head {
    EVENT_CB_S        stEvent;
    pthread_mutex_t    mutex;
    LOS_DL_LIST    poll_queue;
} wait_queue_head_t;

#define osWaitForever     0xFFFFFFFF     ///< wait forever timeout value
#define DECLARE_WAIT_QUEUE_HEAD(wq) wait_queue_head_t wq ={{0,0xFFFFFFFF,0xFFFFFFFF},PTHREAD_MUTEX_INITIALIZER}
/**
 * @ingroup  wait
 * @brief Initialize the waitqueue head.
 *
 * @par Description:
 * This API is used to initialize the waitqueue head.
 *
 * @attention
 * <ul>
 * <li>Please make sure the input parameter p_wait is valid, otherwise, the system would be crash.</li>
 * </ul>
 *
 * @param  p_wait [IN]  struct of the process that registered on the wait queue .
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wait.h: the header file that contains the API declaration.</li></ul>
 * @see none
 * @since Huawei LiteOS V100R001C00
 */
#define init_waitqueue_head(p_wait) __init_waitqueue_head(p_wait)

/**
 * @ingroup  wait
 * @brief wakeup the process that registered on the wait queue.
 *
 * @par Description:
 * This API is used to wakeup the process that registered on the wait queue.
 *
 * @attention
 * <ul>
 * <li>Please make sure the input parameter p_wait is valid, otherwise, the system would be crash.</li>
 * </ul>
 *
 * @param  p_wait [IN]  struct of the process that registered on the wait queue .
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wait.h: the header file that contains the API declaration.</li></ul>
 * @see none
 * @since Huawei LiteOS V100R001C00
 */

#define wake_up_interruptible(p_wait) __wake_up_interruptible(p_wait)
#define wake_up_interruptible_poll(p_wait, key) __wake_up_interruptible_poll(p_wait, key)

/**
 * @ingroup  wait
 * @brief wakeup the process that registered on the wait queue.
 *
 * @par Description:
 * This API is used to wakeup the process that registered on the wait queue.
 *
 * @attention
 * <ul>
 * <li>Please look up the function __wake_up_interruptible(p_wait).</li>
 * </ul>
 *
 * @param None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>Wait.h: the header file that contains the API declaration.</li></ul>
 * @see wake_up_interruptible
 * @since Huawei LiteOS V100R001C00
 */
#define wake_up        wake_up_interruptible

/**
 * @ingroup wait
 * @brief sleep until a condition gets true.
 *
 * @par Description:
 * This API is used to sleep  a process until the condition evaluates to true.
 * The condition is checked each time when the waitqueue wait is woken up.
 *
 * @attention
 * <ul>
 * <li>none.</li>
 * </ul>
 *
 * @param  wait [IN] the waitqueue to wait on.
 * @param  condition [IN] a condition evaluates to true or false.

 * @retval #0 always return 0

 * @par Dependency:
 * <ul><li>linux\wait.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
#define wait_event(wait, condition)  \
    ({ \
            int ret = 0; \
            \
            if (wait.stEvent.stEventList.pstPrev == (struct LOS_DL_LIST *)0xFFFFFFFF) \
            { \
                (void)LOS_EventInit(&wait.stEvent); \
            } \
            (void)pthread_mutex_lock(&wait.mutex);\
            while (!(condition)) \
            { \
                (void)LOS_EventRead(&wait.stEvent,0x1,LOS_WAITMODE_AND|LOS_WAITMODE_CLR,LOS_WAIT_FOREVER);\
            } \
            (void)pthread_mutex_unlock(&wait.mutex);\
            ret; \
    })

#define wait_event_interruptible      wait_event

//timeout in linux is jiffies
/**
 * @ingroup wait
 * @brief sleep until a condition gets true or a timeout elapses.
 *
 * @par Description:
 * This API is used to sleep  a process until the condition evaluates to true or a timeout elapses.
 * The condition is checked each time when the waitqueue wait is woken up.
 *
 * @attention
 * <ul>
 * <li>none.</li>
 * </ul>
 *
 * @param  wait [IN] the waitqueue to wait on.
 * @param  condition [IN] a condition evaluates to true or false.
 * @param  timeout [IN] the max sleep time (unit : Tick).
 *
 * @retval #0 return 0 if the condition evaluated to false after the timeout elapsed
 * @retval #1 return 1 if the condition evaluated to true after the timeout elapsed
 * @retval #2 return 2 if the condition evaluated to true and the timeout is osWaitForever
 *
 * @par Dependency:
 * <ul><li>linux\wait.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
#define wait_event_interruptible_timeout(wait, condition, timeout) \
    ({ \
    INT32 timeout_tmp; \
    UINT32 ret = 2; \
    UINT64 ticks_now; \
    \
    if(wait.stEvent.stEventList.pstPrev == (struct LOS_DL_LIST *)0xFFFFFFFF) \
    { \
        (void)LOS_EventInit(&wait.stEvent); \
    } \
    (void)pthread_mutex_lock(&wait.mutex);\
    while(!(condition)) \
    { \
        ticks_now = LOS_TickCountGet(); \
        ret = LOS_EventRead(&wait.stEvent,0x1,LOS_WAITMODE_AND|LOS_WAITMODE_CLR,timeout); \
        if(timeout == osWaitForever) {\
            if(condition) { \
                ret = 2;break; \
            } \
            else { \
                continue; \
            } \
        } \
        timeout_tmp = (INT32)(timeout - (UINT32)(LOS_TickCountGet() - ticks_now)); \
        if(timeout_tmp <= 0)\
        { \
             ret = condition ? 1 : 0; break;\
        } \
        else \
        { \
            if(ret == LOS_ERRNO_EVENT_READ_TIMEOUT) \
            { \
                if(condition){ \
                    ret = 1;break; \
                }else{ \
                    ret = 0;break; \
                } \
            } \
            else \
            { \
                \
                if(condition){ \
                   ret = 2;break; \
                }else{ \
                        /*TODO*/ \
                } \
            } \
        } \
    } \
    (void)pthread_mutex_unlock(&wait.mutex);\
    ret; \
    })


#define add_wait_queue(wait,new_wait) do{} while (0)
#define remove_wait_queue(wait,old_wait) do{} while (0)
#define DECLARE_WAITQUEUE(wait,current) do{} while (0)

static inline int waitqueue_active(wait_queue_head_t *q)
{
    return !LOS_ListEmpty(&(q->stEvent.stEventList));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_WAIT_H__ */
