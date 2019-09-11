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

/**@defgroup los_config System configuration items
 */

#ifndef _LOS_CONFIG_H
#define _LOS_CONFIG_H

#include "sys_config.h"
#include "los_typedef.h"
#include "hisoc/clock.h"
#include "board.h"
#include "platform_config.h"
#include "los_tick.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_config
 * int stack start addr
 */
extern char  __int_stack_start;

/****************************** System clock module configuration ****************************/

/**
 * @ingroup los_config
 * System clock (unit: HZ)
 */
#ifndef OS_SYS_CLOCK
#define OS_SYS_CLOCK                            (get_bus_clk())
#endif
/**
 * @ingroup los_config
 * time timer clock (unit: HZ)
 */
#ifndef OS_TIME_TIMER_CLOCK
#define OS_TIME_TIMER_CLOCK                     OS_SYS_CLOCK
#endif

/**
* @ingroup los_config
* limit addr range when search for  'func local(frame pointer)' or 'func name'
*/
#ifndef OS_SYS_FUNC_ADDR_START
#define OS_SYS_FUNC_ADDR_START      (AARCHPTR)&__int_stack_start
#endif
#ifndef OS_SYS_FUNC_ADDR_END
#define OS_SYS_FUNC_ADDR_END        g_sys_mem_addr_end
#endif

/**
* @ingroup los_config
* .int stack size
*/
#ifndef OS_UNDEF_STACK_SIZE
#define OS_UNDEF_STACK_SIZE     32
#endif
#ifndef OS_ABT_STACK_SIZE
#define OS_ABT_STACK_SIZE   32
#endif
#ifndef OS_IRQ_STACK_SIZE
#define OS_IRQ_STACK_SIZE   64
#endif
#ifndef OS_FIQ_STACK_SIZE
#define OS_FIQ_STACK_SIZE   64
#endif
#ifndef OS_SVC_STACK_SIZE
#define OS_SVC_STACK_SIZE   4096
#endif

/**
* @ingroup los_config
* .startup stack size
*/
#ifndef OS_STARTUP_STACK_SIZE
#define OS_STARTUP_STACK_SIZE   512
#endif

/**
 * @ingroup los_config
 * Number of Ticks in one second
 */
#ifndef LOSCFG_BASE_CORE_TICK_PER_SECOND
#define LOSCFG_BASE_CORE_TICK_PER_SECOND                     100
#endif

/**
 * @ingroup los_config
 * Microseconds of adjtime in one second
 */
#ifndef LOSCFG_BASE_CORE_ADJ_PER_SECOND
#define LOSCFG_BASE_CORE_ADJ_PER_SECOND            500
#endif

/**
 * @ingroup los_config
 * External configuration item for timer tailoring
 */
#ifndef LOSCFG_BASE_CORE_TICK_HW_TIME
#define LOSCFG_BASE_CORE_TICK_HW_TIME                    YES
#endif


/****************************** Hardware interrupt module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for hardware interrupt tailoring
 */
#ifndef LOSCFG_PLATFORM_HWI
#define LOSCFG_PLATFORM_HWI                         YES
#endif

/**
 * @ingroup los_config
 * Maximum number of used hardware interrupts, including Tick timer interrupts.
 */
#ifndef LOSCFG_PLATFORM_HWI_LIMIT
#define LOSCFG_PLATFORM_HWI_LIMIT                    96
#endif

/****************************** Task module configuration ********************************/

/**
 * @ingroup los_config
 * Minimum stack size.
 *
 * 0x400 bytes, aligned on a boundary of 4.
 */
#ifndef LOS_TASK_MIN_STACK_SIZE
#define LOS_TASK_MIN_STACK_SIZE                     (ALIGN(0x600, 8))
#endif

/**
 * @ingroup los_config
 * Default task priority
 */
#ifndef LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO
#define LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO                10
#endif

extern UINT32                                  g_uwTskMaxNum;
/**
 * @ingroup los_config
 * Maximum supported number of tasks except the idle task rather than the number of usable tasks
 */
#ifndef LOSCFG_BASE_CORE_TSK_LIMIT
#define LOSCFG_BASE_CORE_TSK_LIMIT                 g_uwTskMaxNum
#endif

/**
 * @ingroup los_config
 * Size of the idle task stack
 */
#ifndef LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE
#define LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE                 SIZE(0x800)
#endif

/**
 * @ingroup los_config
 * Default task stack size
 */
#ifndef LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE
#define LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE              SIZE(0x6000)
#endif

/**
 * @ingroup los_config
 * Configuration item for task Robin tailoring
 */
#ifndef LOSCFG_BASE_CORE_TIMESLICE
#define LOSCFG_BASE_CORE_TIMESLICE                        YES
#endif

/**
 * @ingroup los_config
 * Longest execution time of tasks with the same priorities
 */
#ifndef LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT
#define LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT                        2
#endif

/**
 * @ingroup los_config
 * Configuration item for task (stack) monitoring module tailoring
 */
#ifndef LOSCFG_BASE_CORE_TSK_MONITOR
#define LOSCFG_BASE_CORE_TSK_MONITOR                          YES
#endif

/**
 * @ingroup los_config
 * Configuration item for task perf task filter hook
 */
#ifndef OS_PERF_TSK_FILTER
#define OS_PERF_TSK_FILTER                      NO
#endif


/****************************** Semaphore module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for semaphore module tailoring
 */
#ifndef LOSCFG_BASE_IPC_SEM
#define LOSCFG_BASE_IPC_SEM                         YES
#endif

extern UINT32 g_uwSemNumCfg;

/**
 * @ingroup los_config
 * Maximum supported number of semaphores
 */
#ifndef LOSCFG_BASE_IPC_SEM_LIMIT
#define LOSCFG_BASE_IPC_SEM_LIMIT                  g_uwSemNumCfg
#endif

/****************************** mutex module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for mutex module tailoring
 */
#ifndef LOSCFG_BASE_IPC_MUX
#define LOSCFG_BASE_IPC_MUX                         YES
#endif

extern UINT32 g_uwMuxNumCfg;
/**
 * @ingroup los_config
 * Maximum supported number of mutexes
 */
#ifndef LOSCFG_BASE_IPC_MUX_LIMIT
#define LOSCFG_BASE_IPC_MUX_LIMIT                 g_uwMuxNumCfg
#endif
/****************************** Queue module configuration ********************************/

/**
 * @ingroup los_config
 * Configuration item for queue module tailoring
 */
#ifndef LOSCFG_BASE_IPC_QUEUE
#define LOSCFG_BASE_IPC_QUEUE                       YES
#endif

extern UINT32 g_uwQueueNumCfg;
/**
 * @ingroup los_config
 * Maximum supported number of queues rather than the number of usable queues
 */
#ifndef LOSCFG_BASE_IPC_QUEUE_LIMIT
#define LOSCFG_BASE_IPC_QUEUE_LIMIT               g_uwQueueNumCfg
#endif
/****************************** Software timer module configuration **************************/
#if (LOSCFG_BASE_IPC_QUEUE == YES)

/**
 * @ingroup los_config
 * Configuration item for software timer module tailoring
 */
#ifndef LOSCFG_BASE_CORE_SWTMR
#define LOSCFG_BASE_CORE_SWTMR                       YES
#endif

extern UINT32 g_uwSwtmrNumCfg;
/**
 * @ingroup los_config
 * Maximum supported number of software timers rather than the number of usable software timers
 */
#ifndef LOSCFG_BASE_CORE_SWTMR_LIMIT
#define LOSCFG_BASE_CORE_SWTMR_LIMIT               g_uwSwtmrNumCfg
#endif
/**
 * @ingroup los_config
 * Max number of software timers ID
 */
#ifndef OS_SWTMR_MAX_TIMERID
#define OS_SWTMR_MAX_TIMERID                         ((65535/LOSCFG_BASE_CORE_SWTMR_LIMIT) * LOSCFG_BASE_CORE_SWTMR_LIMIT)
#endif
/**
 * @ingroup los_config
 * Maximum size of a software timer queue
 */
#ifndef OS_SWTMR_HANDLE_QUEUE_SIZE
#define OS_SWTMR_HANDLE_QUEUE_SIZE             (LOSCFG_BASE_CORE_SWTMR_LIMIT + 0)
#endif
#endif

/****************************** Memory module configuration **************************/
#ifndef OS_EXC_INTERACTMEM_SIZE
#define OS_EXC_INTERACTMEM_SIZE                         g_exc_interact_mem_size
#endif
/**
 * @ingroup los_config
 * Starting address of the system memory
 */
#ifndef OS_SYS_MEM_ADDR
#define OS_SYS_MEM_ADDR                        &m_aucSysMem1[0]
#endif

/**
 * @ingroup los_config
 * Memory size
 */
#ifndef OS_SYS_MEM_SIZE
#define OS_SYS_MEM_SIZE                        ((g_sys_mem_addr_end ) - OS_SYS_NOCACHEMEM_SIZE  -((OS_EXC_INTERACTMEM_SIZE + ((unsigned long)&__bss_end) + (64 - 1)) & ~(64 - 1)))
#endif
/****************************** Memory module configuration **************************/

/**
 * @ingroup los_config
 * Size of unaligned memory
 */
#ifndef OS_SYS_NOCACHEMEM_SIZE
#define OS_SYS_NOCACHEMEM_SIZE                    0x0
#endif
#if OS_SYS_NOCACHEMEM_SIZE

/**
 * @ingroup los_config
 * Starting address of the unaligned memory
 */
#ifndef OS_SYS_NOCACHEMEM_ADDR
#define OS_SYS_NOCACHEMEM_ADDR                    &m_aucSysNoCacheMem0[0]
#endif
#endif

/****************************** fw Interface configuration **************************/

/**
 * @ingroup los_config
 * Configuration item for the monitoring of task communication
 */
#ifndef LOSCFG_COMPAT_CMSIS_FW
#define LOSCFG_COMPAT_CMSIS_FW                         YES
#endif

/**
 * @ingroup los_config
 * Version number
 */
#define _T(x)                                   x
#define HW_LITEOS_SYSNAME                       "Huawei LiteOS"
#define HW_LITEOS_SEP                           " "
#define _V(v)                                   _T(HW_LITEOS_SYSNAME)_T(HW_LITEOS_SEP)_T(v)

#define HW_LITEOS_VERSION                       "V200R002C00B063"
#define VER                                     _V(HW_LITEOS_VERSION)

/**
 * @ingroup los_config
 * The Version number of Public
 */

#define MAJ_V                            3
#define MIN_V                            0
#define REL_V                            7

#define VERSION_NUM(a,b,c) ((a) << 16|(b) << 8|(c))
#define HW_LITEOS_OPEN_VERSION_NUM VERSION_NUM(MAJ_V,MIN_V,REL_V)


/****************************** Dynamic loading module configuration **************************/
#ifndef OS_AUTOINIT_DYNLOADER
#define OS_AUTOINIT_DYNLOADER                    YES
#endif

/****************************** exception information  configuration ******************************/
#ifdef LOSCFG_SHELL_EXCINFO
/**
 * @ingroup los_config
 * the size of space for recording exception information
 */
#define EXCINFO_RECORD_BUF_SIZE    (4*1024)

/**
 * @ingroup los_config
 * the address of space for recording exception information
 */
#define EXCINFO_RECORD_ADDR    (0xa00000)

/**
 *@ingroup los_config
 *@brief  define the type of functions for reading or writing exception information .
 *
 *@par Description:
 *<ul>
 *<li>This defination is used to declare the type of functions for reading or writing exception information</li>
 *</ul>
 *@attention
 *<ul>
 *<li> "uwStartAddr" must be left to save the exception address space, the size of "pBuf" is "uwSpace"  </li>
 *</ul>
 *
 *@param uwStartAddr    [IN] Address of storage ,its must be left to save the exception address space
 *@param uwSpace          [IN] size of storage.its is also the size of "pBuf"
 *@param uwRWFlag       [IN] writer-read flag, 0 for writing,1 for reading, other number is to do nothing.
 *@param pBuf                [IN] the buffer of storing data.
 *
 *@retval none.
 *@par Dependency:
 *<ul><li>los_config.h: the header file that contains the type defination.</li></ul>
 *@see
 *@since Huawei LiteOS V200R002C00
 */
typedef VOID (*log_read_write_fn)( UINT32 uwStartAddr, UINT32 uwSpace, UINT32 uwRWFlag, CHAR * pBuf);

/**
 *@ingroup los_config
 *@brief Register recording exception information function.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to Register recording exception information function,and specify location and space and size</li>
 *</ul>
 *@attention
 *<ul>
 *<li> "uwStartAddr" must be left to save the exception address space, the size of "pBuf" is "uwSpace",the space of "pBuf" is malloc or free in user's code  </li>
 *</ul>
 *
 *@param uwStartAddr    [IN] Address of storage .its must be left to save the exception address space
 *@param uwSpace         [IN] size of storage space.its is also the size of "pBuf"
 *@param pBuf               [IN] the buffer of storing exception information.the space of "pBuf" is malloc or free in user's code
 *@param pfnHook         [IN] the function for reading or writing exception information .
 *
 *@retval none.
 *@par Dependency:
 *<ul><li>los_config.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V200R002C00
 */
VOID LOS_ExcInfoRegHook(UINT32 uwStartAddr, UINT32 uwSpace, CHAR * pBuf,  log_read_write_fn pfnHook);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LOS_CONFIG_H */
