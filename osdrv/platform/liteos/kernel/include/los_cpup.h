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

 /** @defgroup los_cpup CPU usage
 * @ingroup kernel
 */

#ifndef _LOS_CPUP_H
#define _LOS_CPUP_H

#include "los_hwi.h"
#include "los_base.h"
#include "los_sys.h"
#include "los_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/**
* @ingroup los_cpup
* CPU usage error code: The request for memory fails.
*
* Value: 0x02001e00
*
* Solution: Decrease the maximum number of tasks.
*/
#define LOS_ERRNO_CPUP_NO_MEMORY             LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x00)

/**
* @ingroup los_cpup
* CPU usage error code: The pointer to an input parameter is NULL.
*
* Value: 0x02001e01
*
* Solution: Check whether the pointer to the input parameter is usable.
*/
#define LOS_ERRNO_CPUP_TASK_PTR_NULL         LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x01)

/**
* @ingroup los_cpup
* CPU usage error code: The CPU usage is not initialized.
*
* Value: 0x02001e02
*
* Solution: Check whether the CPU usage is initialized.
*/
#define LOS_ERRNO_CPUP_NO_INIT               LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x02)

/**
* @ingroup los_cpup
* CPU usage error code: The number of threads is invalid.
*
* Value: 0x02001e03
*
* Solution: Check whether the number of threads is applicable for the current operation.
*/
#define LOS_ERRNO_CPUP_MAXNUM_INVALID        LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x03)

/**
* @ingroup los_cpup
* CPU usage error code: The target thread is not created.
*
* Value: 0x02001e04
*
* Solution: Check whether the target thread is created.
*/
#define LOS_ERRNO_CPUP_THREAD_NO_CREATED     LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x04)

/**
* @ingroup los_cpup
* CPU usage error code: The target task ID is invalid.
*
* Value: 0x02001e05
*
* Solution: Check whether the target task ID is applicable for the current operation.
*/
#define LOS_ERRNO_CPUP_TSK_ID_INVALID        LOS_ERRNO_OS_ERROR(LOS_MOD_CPUP, 0x05)

/**
* @ingroup los_cpup
* Sum of cpup with all tasks. It means the value of cpup is a permillage.
*/
#define LOS_CPUP_PRECISION                   1000

/**
* @ingroup los_cpup
* Multiple of current cpup precision change to percent.
*/
#define LOS_CPUP_PRECISION_MULT              (LOS_CPUP_PRECISION / 100)

/**
 * @ingroup los_cpup
 * Count the CPU usage structures of all tasks.
 */
typedef struct tagCpupInfo
{
    UINT16 usStatus;            /**< save the cur task status     */
    UINT32 uwUsage;             /**< Usage. The value range is [0,1000].   */
} CPUP_INFO_S;

/**
 *@ingroup los_cpup
 *@brief Obtain the current CPU usage.
 *
 *@par Description:
 *This API is used to obtain the current CPU usage.
 *@attention
 *<ul>
 *<li>This API can be called only after the CPU usage is initialized. Otherwise, error codes will be returned.</li>
 *<li>The precision of the CPU usage can be adjusted by changing the value of the CPUP_PRECISION macro.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #LOS_ERRNO_CPUP_NO_INIT           The CPU usage is not initialized.
 *@retval #UINT32                           [0,100], current CPU usage, of which the precision is adjustable.
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_SysCpuUsage(VOID);

/**
 *@ingroup los_cpup
 *@brief Obtain the historical CPU usage.
 *
 *@par Description:
 *This API is used to obtain the historical CPU usage.
 *@attention
 *<ul>
 *<li>This API can be called only after the CPU usage is initialized. Otherwise, the CPU usage fails to be obtained.</li>
 *</ul>
 *
 *@param  usMode     [IN] UINT16. Task mode. The parameter value 0 indicates that the CPU usage within 10s will be obtained, and the parameter value 1 indicates that the CPU usage in the former 1s will be obtained. Other values indicate that the CPU usage in all time will be obtained.
 *
 *@retval #LOS_ERRNO_CPUP_NO_INIT           The CPU usage is not initialized.
 *@retval #UINT32                           [0,100], historical CPU usage, of which the precision is adjustable.
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_HistorySysCpuUsage(UINT16 usMode);

/**
 *@ingroup los_cpup
 *@brief Obtain the CPU usage of a specified task.
 *
 *@par Description:
 *This API is used to obtain the CPU usage of a task specified by a passed-in task ID.
 *@attention
 *<ul>
 *<li>This API can be called only after the CPU usage is initialized. Otherwise, the CPU usage fails to be obtained.</li>
 *<li>The passed-in task ID must be valid and the task specified by the task ID must be created. Otherwise, the CPU usage fails to be obtained.</li>
 *</ul>
 *
 *@param uwTaskID   [IN] UINT32. Task ID.
 *
 *@retval #LOS_ERRNO_CPUP_NO_INIT                  The CPU usage is not initialized.
 *@retval #LOS_ERRNO_CPUP_TSK_ID_INVALID           The target task ID is invalid.
 *@retval #LOS_ERRNO_CPUP_THREAD_NO_CREATED   The target thread is not created.
 *@retval #UINT32                                  [0,100], CPU usage of the specified task.
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_TaskCpuUsage(UINT32 uwTaskID);

/**
 *@ingroup los_cpup
 *@brief  Obtain the historical CPU usage of a specified task.
 *
 *@par Description:
 *This API is used to obtain the historical CPU usage of a task specified by a passed-in task ID.
 *@attention
 *<ul>
 *<li>This API can be called only after the CPU usage is initialized. Otherwise, the CPU usage fails to be obtained.</li>
 *<li>The passed-in task ID must be valid and the task specified by the task ID must be created. Otherwise, the CPU usage fails to be obtained.</li>
 *</ul>
 *
 *@param uwTaskID   [IN] UINT32. Task ID.
 *@param usMode     [IN] UINT16. Task mode. The parameter value 0 indicates that the CPU usage within 10s will be obtained, and the parameter value 1 indicates that the CPU usage in the former 1s will be obtained. Other values indicate that the CPU usage in all time will be obtained.
 *
 *@retval #LOS_ERRNO_CPUP_NO_INIT                  The CPU usage is not initialized.
 *@retval #LOS_ERRNO_CPUP_TSK_ID_INVALID           The target task ID is invalid.
 *@retval #LOS_ERRNO_CPUP_THREAD_NO_CREATED   The target thread is not created.
 *@retval #UINT32                                  [0,100], CPU usage of the specified task.
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_HistoryTaskCpuUsage(UINT32 uwTaskID, UINT16 usMode);

/**
 *@ingroup los_cpup
 *@brief Obtain the CPU usage of all tasks.
 *
 *@par Description:
 *This API is used to obtain the CPU usage of all tasks according to the passed-in maximum number of threads.
 *@attention
 *<ul>
 *<li>This API can be called only after the CPU usage is initialized. Otherwise, the CPU usage fails to be obtained.</li>
 *<li>The input parameter pointer must not be NULL, and the maximum number of threads must be usable. Otherwise, the CPU usage fails to be obtained.</li>
 *<li>The input parameter pointer should point to the structure array who's size be greater than (usMaxNum * sizeof (CPUP_INFO_S)).</li>
 *</ul>
 *
 *@param usMaxNum       [IN] UINT16. The Maximum number of threads.
 *@param pstCpupInfo    [OUT]Type.   CPUP_INFO_S* Pointer to the task CPUP information structure to be obtained.
 *@param usMode         [IN] UINT16. Task mode. The parameter value 0 indicates that the CPU usage within 10s will be obtained, and the parameter value 1 indicates that the CPU usage in the former 1s will be obtained. Other values indicate that the CPU usage in all time will be obtained.
 *
 *@retval #LOS_ERRNO_CPUP_NO_INIT                  The CPU usage is not initialized.
 *@retval #LOS_ERRNO_CPUP_TASK_PTR_NULL            The input parameter pointer is NULL.
 *@retval #LOS_ERRNO_CPUP_MAXNUM_INVALID           The maximum number of threads is invalid.
 *@retval #LOS_OK                                  The CPU usage of all tasks is successfully obtained.
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_AllTaskCpuUsage(UINT16 usMaxNum, CPUP_INFO_S *pstCpupInfo, UINT16 usMode);

/**
 *@ingroup los_cpup
 *@brief Reset the data of CPU usage.
 *
 *@par Description:
 *This API is used to reset the data of CPU usage.
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *<ul><li>los_cpup.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
extern VOID LOS_CpupReset(VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_CPUP_H */
