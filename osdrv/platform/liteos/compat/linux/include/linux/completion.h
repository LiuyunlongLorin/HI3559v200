/** @defgroup completion Completion
*   @ingroup linux
*/

#ifndef __LINUX_COMPLETION_H__
#define __LINUX_COMPLETION_H__

#include "los_sys.h"
#include "errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct completion{
    LOS_DL_LIST     stComList;
    UINT32          uwComCount;
};

#define OS_WAIT_COMPLETION_ERROR  0xFFFFFFFF

/**
 *@ingroup completion
 *@brief Initialize a completion.
 *
 *@par Description:
 *This API is used to initialize a specified completion.
 *@attention
 *<ul>
 *<li>The input parameter x must point to valid memory, otherwise, initilize a completion would failed.</li>
 *</ul>
 *
 *@param x   [IN] Pointer to the completion to be initialized,which must point to valid memory.
 *
 *@retval None.
 *@par Dependency: none
 *<ul><li>completion.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void init_completion(struct completion *x);

/**
 *@ingroup completion
 *@brief Wake up a task that is waiting on this completion.
 *
 *@par Description:
 *This API is used to wake up a task that is waiting on the completion.
 *@attention
 *<ul>
 *<li>The input parameter x must point to valid memory, otherwise, the system would be abnormal.</li>
 *<li>It suggested that calling complete() after wait_for_completion() or wait_for_completion_timeout(), otherwise, wait_for_completion() or wait_for_completion_timeout() would not block because there is already a completion completed.</li>
 *</ul>
 *
 *@param x [IN] Pointer to the completion on which the task to be woken up is waiting, which must point to valid memory.
 *
 *@retval None.
 *@par Dependency:
 *<ul>
 *<li>this function should be used after init_completion() be called.</li>
 *<li>completion.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void complete(struct completion *x);

/**
 *@ingroup completion
 *@brief Wait on a completion forever.
 *
 *@par Description:
 *This API is used to wait on a completion forever.
 *@attention
 *<ul>
 *<li>The input parameter x must point to valid memory, otherwise, the system would be abnormal.</li>
 *<li>Can not be used in interrupt.</li>
 *<li>DO NOT call this API in software timer callback. </li>
 *</ul>
 *
 *@param x [IN] Pointer to the completion to be waited on, which must point to valid memory.
 *
 *@retval None.
 *@par Dependency:
 *<ul>
 *<li>this function should be used after init_completion() be called.</li>
 *<li>completion.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void wait_for_completion(struct completion *x);

/**
 *@ingroup completion
 *@brief Wait on a completion within a certain time period.
 *
 *@par Description:
 * This API is used to wait on a completion within a certain time period (timeout).
 *@attention
 *<ul>
 *<li>The input parameter x must point to valid memory, otherwise, the system would be abnormal.</li>
 *<li>Timeout interval timeout should be in [1 , 0xFFFFFFFe], otherwise, it would return OS_WAIT_COMPLETION_ERROR but not wait for completion.</li>
 *<li>Can not be used in interrupt.</li>
 *<li>DO NOT call this API in software timer callback. </li>
 *</ul>
 *
 *@param x [IN] Pointer to the  completion to be waited on, which must point to valid memory.
 *@param timeout [IN] Timeout interval for waiting on the completion (unit: Tick).
 *
 *@retval 0 The timeout period expires before the task is blocked or scheduled.
 *@retval [1,0xFFFFFFFe] Remaining waiting time.
 *@retval OS_WAIT_COMPLETION_ERROR  failed to wait on the completion.
 *@par Dependency:
 *<ul>
 *<li>this function should be used after init_completion() be called.</li>
 *<li>completion.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout);


/**
 *@ingroup completion
 *@brief Wake up all tasks that are waiting on this completion.
 *
 *@par Description:
 *This API is used to wake up all tasks that are waiting on the completion.
 *@attention
 *<ul>
 *<li>The input parameter x must point to valid memory, otherwise, the system would be abnormal.</li>
 *<li>It suggested that calling complete_all() after wait_for_completion() or wait_for_completion_timeout(), otherwise, wait_for_completion() or wait_for_completion_timeout() would not block because there is already a completion completed.</li>
 *</ul>
 *
 *@param x [IN] Pointer to the completion on which the task to be woken up is waiting, which must point to valid memory.
 *
 *@retval None.
 *@par Dependency:
 *<ul>
 *<li>this function should be used after init_completion() be called.</li>
 *<li>completion.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void complete_all(struct completion *x);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_COMPLETION_H__ */
