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

/**@defgroup los_memory  Dynamic memory
 * @ingroup kernel
 */

#ifndef _LOS_MEMORY_H
#define _LOS_MEMORY_H

#include "los_config.h"
#include "los_base.h"
#include "los_toolchain.h"
#include "los_membox.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_MEM_TLSF
#define LOS_MEM_TLSF YES

#ifdef LOSCFG_KERNEL_MEM_SLAB
#define LOS_MEM_SLAB
#ifdef LOSCFG_SLAB_INFO_STATISTIC
#define LOS_SLAB_INFO_STATISTIC
#endif
#endif

#else
#define LOS_MEM_TLSF NO
#endif

#ifdef LOS_MEM_SLAB
#define LOS_SLAB_HEAP_BOUNDARY           (128)
#define LOS_SLAB_CLASS_CNT               (16)
#define LOS_SLAB_DEFAULT_ALLOCATOR_SIZE  (2 * 1024)

struct LOS_SLAB_ALLOCATOR_CFG_INFO{
    UINT16 usClassAllocatorSize[LOS_SLAB_CLASS_CNT];
};
#endif
#ifdef LOSCFG_KERNEL_MEM_SPEC
#define LOS_SPEC_CLASS_CNT    20
#endif
#ifdef LOS_MEM_LEAK_CHECK
typedef struct
{
    void *pPoolAddr;
    UINT32 uwFp[4];
    INT32 Flag;
    UINT32 uwAddrSize;
} MEM_CHECK_INFO;

typedef enum
{
    OS_CHECK_FP0 =  4,
    OS_CHECK_FP1 =  8,
    OS_CHECK_FP2 =  12,
    OS_CHECK_FP3 =  16,
    OS_CHECK_FLAG = 20,
    OS_ADDR_SIZE = 24,
}MemCheckInfo;

#define CHECK_MAXCOUNT 500
#define MEM_CHECK_SIZE sizeof(MEM_CHECK_INFO)
#define MEM_CHECK_POOL_SIZE  LOS_MEMBOX_SIZE(MEM_CHECK_SIZE, CHECK_MAXCOUNT)

extern UINT32 g_uwMemCount;
extern UINT32 g_uwPeakCount;
extern UINT32 g_uwCheckFlag;
extern MEM_CHECK_INFO *g_pMemCheckPool;
extern VOID LOS_CheckMaxcount(VOID);
extern VOID LOS_MemLeakCheckStart (VOID);
extern VOID LOS_MemLeakCheckEnd(VOID);
extern VOID LOS_MemLeakCheckShow(VOID);
#endif

typedef VOID (*MALLOC_HOOK)(VOID);

extern MALLOC_HOOK g_MALLOC_HOOK;

/**
 * @ingroup los_memory
 * The start address of exc interaction dynamic memory pool address, when the exc
 * interaction feature not support, m_aucSysMem0 equal to m_aucSysMem1.
 */
extern UINT8 *m_aucSysMem0;
/**
 * @ingroup los_memory
 * The start address of system dynamic memory pool address.
 */
extern UINT8 *m_aucSysMem1;


/**
 * @ingroup los_memory
 * The start address of system dynamic memory pool address(no cache).
 */
extern UINT8 *m_aucSysNoCacheMem0;

/**
 * @ingroup los_memory
 * The end address of system memory.
 */
extern AARCHPTR g_sys_mem_addr_end;
/**
 * @ingroup los_memory
 * The size of exc interaction memory.
 */
extern AARCHPTR g_exc_interact_mem_size;

/**
 * @ingroup los_memory
 * The memory Maximum memory usage statistics.
 * @attention
 * <ul> <li>If running as debug mode, it will affect the performance of memory malloc and free.</li>
 * <li> OS_MEM_WATERLINE=YES: open the function for Maximum memory usage statistics </li>
 * <li> OS_MEM_WATERLINE=NO: close the function for Maximum memory usage statistics, it set to NO as usual </li>
 * </ul>
 */
#ifdef LOSCFG_MEM_WATERLINE
#define OS_MEM_WATERLINE  NO
#endif

#ifdef LOSCFG_MEM_MUL_MODULE
/**
 * @ingroup los_memory
 * The memory usage statistics depend on module, this is the max module no.
 */
#define MEM_MODULE_MAX  0x20

/**
 *@ingroup los_memory
 *@brief Allocate dynamic memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate a memory block of which the size is specified and update module mem used.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize must be four byte-aligned.</li>
 *</ul>
 *
 *@param  pPool    [IN] Pointer to the memory pool that contains the memory block to be allocated.
 *@param  uwSize   [IN] Size of the memory block to be allocated (unit: byte).
 *@param  module   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #NULL          The memory fails to be allocated.
 *@retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory block returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemRealloc | LOS_MemAllocAlign | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemMalloc(VOID *pPool, UINT32 uwSize, UINT32 module);

/**
 *@ingroup los_memory
 *@brief Allocate aligned memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate memory blocks of specified size and of which the starting addresses are aligned on a specified boundary and update module mem used.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The alignment parameter value must be a power of 2 with the minimum value being 4.</li>
 *</ul>
 *
 *@param  pPool       [IN] Pointer to the memory pool that contains the memory blocks to be allocated.
 *@param  uwSize      [IN] Size of the memory to be allocated.
 *@param  uwBoundary  [IN] Boundary on which the memory is aligned.
 *@param  module   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #NULL          The memory fails to be allocated.
 *@retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemMallocAlign(VOID *pPool, UINT32 uwSize, UINT32 uwBoundary, UINT32 module);

/**
 *@ingroup los_memory
 *@brief Free dynamic memory.
 *
 *@par Description:
 *<li>This API is used to free specified dynamic memory that has been allocated and update module mem used.</li>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The input pMem parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign or LOS_MemRealloc.</li>
 *</ul>
 *
 *@param  pPool  [IN] Pointer to the memory pool that contains the dynamic memory block to be freed.
 *@param  pMem   [IN] Starting address of the memory block to be freed.
 *@param  module   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #LOS_NOK          The memory block fails to be freed because the starting address of the memory block is invalid, or the memory overwriting occurs.
 *@retval #LOS_OK           The memory block is successfully freed.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemRealloc | LOS_MemAllocAlign
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemMfree(VOID *pPool, VOID *pPtr, UINT32 module);

/**
 *@ingroup los_memory
 *@brief Re-allocate a memory block.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate a new memory block of which the size is specified by size if the original memory block size
 * is insufficient. The new memory block will copy the data in the original memory block of which the address is specified by ptr.
 * The size of the new memory block determines the maximum size of data to be copied. After the new memory block is created,
 * the original one is freed. And update module mem used</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The input pPtr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize must be aligned as follows: 1) if the pPtr is allocated by LOS_MemAlloc, it must be four byte-aligned; 2) if the pPtr is allocated by LOS_MemAllocAlign, it must be aligned with the size of the input parameter uwBoundary of LOS_MemAllocAlign.</li>
 *</ul>
 *
 *@param  pPool      [IN] Pointer to the memory pool that contains the original and new memory blocks.
 *@param  pPtr       [IN] Address of the original memory block.
 *@param  uwSize     [IN] Size of the new memory block.
 *@param  module   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #NULL          The memory fails to be re-allocated.
 *@retval #VOID*         The memory is successfully re-allocated with the starting address of the new memory block returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemAllocAlign | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemMrealloc(VOID *pPool, VOID *pPtr, UINT32 uwSize, UINT32 module);

/**
 *@ingroup los_memory
 *@brief get special module's mem consume size.
 *
 *@par Description:
 *<li>This API is used to get special module's mem consume size.</li>
 *@param  module   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #UINT32         The size of the special module's memory consumed.
 *@retval #OS_NULL_INT           The illegal module.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemMusedGet(UINT32 module);
#endif

#ifdef LOSCFG_MEM_MUL_POOL
/**
 *@ingroup los_memory
 *@brief Deinitialize dynamic memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to deinitialize the dynamic memory of a doubly linked list.</li>
 *</ul>
 *
 *@param pPool          [IN] Starting address of memory.
 *
 *@retval #OS_ERROR   The dynamic memory fails to be deinitialized.
 *@retval #LOS_OK     The dynamic memory is successfully deinitialized.
 *@par Dependency:
 *<ul>
 *<li>los_memory.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemDeInit(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Print infomation about all pools.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to print infomation about all pools.</li>
 *</ul>
 *
 *@retval #UINT32   The pool number.
 *@par Dependency:
 *<ul>
 *<li>los_memory.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemPoolList(VOID);
#endif

/**
 * @ingroup los_memory
 * Memory pool extern information structure
 */
typedef struct
{
    UINT32 uwTotalUsedSize;
    UINT32 uwTotalFreeSize;
    UINT32 uwMaxFreeNodeSize;
    UINT32 uwUsedNodeNum;
    UINT32 uwFreeNodeNum;
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    UINT32 uwUsageWaterLine;
#endif

#ifdef LOS_MEM_SLAB
    UINT32 uwSlabTotalUsedBuckets;
    UINT32 uwSlabTotalFreeBuckets;
#endif
} LOS_MEM_POOL_STATUS;

/**
 *@ingroup los_memory
 *@brief Initialize dynamic memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to initialize the dynamic memory of a doubly linked list.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The uwSize parameter value should match the following two conditions : 1) Be less than or equal to the Memory pool size; 2) Be greater than the size of OS_MEM_MIN_POOL_SIZE.</li>
 *<li>Call this API when dynamic memory needs to be initialized during the startup of Huawei LiteOS.</li>
 *<li>The parameter input must be four byte-aligned.</li>
 *<li>The init area [pPool, pPool + uwSize] should not conflict with other pools.</li>
 *</ul>
 *
 *@param pPool          [IN] Starting address of memory.
 *@param uwSize         [IN] Memory size.
 *
 *@retval #OS_ERROR   The dynamic memory fails to be initialized.
 *@retval #LOS_OK     The dynamic memory is successfully initialized.
 *@par Dependency:
 *<ul>
 *<li>los_memory.h: the header file that contains the API declaration.</li>
 *</ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemInit(VOID *pPool, UINT32 uwSize);

/**
 *@ingroup los_memory
 *@brief Allocate dynamic memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate a memory block of which the size is specified.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize must be four byte-aligned.</li>
 *</ul>
 *
 *@param  pPool    [IN] Pointer to the memory pool that contains the memory block to be allocated.
 *@param  uwSize   [IN] Size of the memory block to be allocated (unit: byte).
 *
 *@retval #NULL          The memory fails to be allocated.
 *@retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory block returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemRealloc | LOS_MemAllocAlign | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemAlloc(VOID *pPool, UINT32 uwSize);

/**
 *@ingroup los_memory
 *@brief Free dynamic memory.
 *
 *@par Description:
 *<li>This API is used to free specified dynamic memory that has been allocated.</li>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The input pMem parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign or LOS_MemRealloc.</li>
 *</ul>
 *
 *@param  pPool  [IN] Pointer to the memory pool that contains the dynamic memory block to be freed.
 *@param  pMem   [IN] Starting address of the memory block to be freed.
 *
 *@retval #LOS_NOK          The memory block fails to be freed because the starting address of the memory block is invalid, or the memory overwriting occurs.
 *@retval #LOS_OK           The memory block is successfully freed.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemRealloc | LOS_MemAllocAlign
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemFree(VOID *pPool, VOID *pPtr);

/**
 *@ingroup los_memory
 *@brief Re-allocate a memory block.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate a new memory block of which the size is specified by size if the original memory block size is insufficient. The new memory block will copy the data in the original memory block of which the address is specified by ptr. The size of the new memory block determines the maximum size of data to be copied. After the new memory block is created, the original one is freed.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The input pPtr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize must be aligned as follows: 1) if the pPtr is allocated by LOS_MemAlloc, it must be four byte-aligned; 2) if the pPtr is allocated by LOS_MemAllocAlign, it must be aligned with the size of the input parameter uwBoundary of LOS_MemAllocAlign.</li>
 *</ul>
 *
 *@param  pPool      [IN] Pointer to the memory pool that contains the original and new memory blocks.
 *@param  pPtr       [IN] Address of the original memory block.
 *@param  uwSize     [IN] Size of the new memory block.
 *
 *@retval #NULL          The memory fails to be re-allocated.
 *@retval #VOID*         The memory is successfully re-allocated with the starting address of the new memory block returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemAllocAlign | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemRealloc(VOID *pPool, VOID *pPtr, UINT32 uwSize);

/**
 *@ingroup los_memory
 *@brief Allocate aligned memory.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to allocate memory blocks of specified size and of which the starting addresses are aligned on a specified boundary.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The size of the input parameter uwSize can not be greater than the memory pool size that specified at the second input parameter of LOS_MemInit.</li>
 *<li>The alignment parameter value must be a power of 2 with the minimum value being 4.</li>
 *</ul>
 *
 *@param  pPool       [IN] Pointer to the memory pool that contains the memory blocks to be allocated.
 *@param  uwSize      [IN] Size of the memory to be allocated.
 *@param  uwBoundary  [IN] Boundary on which the memory is aligned.
 *
 *@retval #NULL          The memory fails to be allocated.
 *@retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory returned.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 *@since Huawei LiteOS V100R001C00
 */
extern VOID *LOS_MemAllocAlign(VOID *pPool, UINT32 uwSize, UINT32 uwBoundary);

/**
 *@ingroup los_memory
 *@brief Get the size of memory pool's size.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the size of memory pool' total size.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPool           [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK        The incoming parameter pPool is NULL.
 *@retval #UINT32         The size of the memory pool.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemPoolSizeGet(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Get the size of memory totally used.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the size of memory totally used in memory pool.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPool           [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK        The incoming parameter pPool is NULL.
 *@retval #UINT32         The size of the memory pool used.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemTotalUsedGet(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Get the number of free memory nodes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the number of free memory nodes in memory pool.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPool           [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK        The incoming parameter pPool is NULL.
 *@retval #UINT32         The number of free memory nodes.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemFreeBlksGet(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Get the number of used memory nodes.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the number of used memory nodes in memory pool.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPool           [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK        The incoming parameter pPool is NULL.
 *@retval #UINT32         The number of used memory nodes.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemUsedBlksGet(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Get the task ID of a used memory node.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the task ID of a used memory node.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPtr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 *<li>This interface only support obtain the task ID of a used memory node which is allocated from the system memory pool (OS_SYS_MEM_ADDR) at present.</li>
 *</ul>
 *
 *@param  pPtr               [IN] A used memory node.
 *
 *@retval #OS_INVALID        The incoming parameter pPtr is illegal.
 *@retval #UINT32            The task ID of used memory node pPtr.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemTaskIdGet(VOID *pPtr);

/**
 *@ingroup los_memory
 *@brief Get the address of last node.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the address of last node.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The last node of memory pool is not the end node.</li>
 *</ul>
 *
 *@param  pPtr               [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK           The incoming parameter pPool is NULL.
 *@retval #AARCHPTR            The address of the last used node that casts to UINT32.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern AARCHPTR LOS_MemLastUsedGet(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Get the infomation of memory pool.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the infomation of memory pool.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPool                 [IN] A pointer pointed to the memory pool.
 *@param  pStatus               [IN] A pointer for storage the pool status
 *
 *@retval #LOS_NOK           The incoming parameter pPool is NULL or invalid.
 *@retval #LOS_OK            Success to get memory infomation.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemInfoGet(VOID *pPool, LOS_MEM_POOL_STATUS *pStatus);

/**
 *@ingroup los_memory
 *@brief Get the number of free node in every size.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the number of free node in every size.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *</ul>
 *
 *@param  pPtr               [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK           The incoming parameter pPool is NULL.
 *@retval #UINT32            The address of the last used node that casts to UINT32.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemFreeNodeShow(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Check the memory pool Integrity.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to check the memory pool Integrity.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>LOS_MemIntegrityCheck will be called by malloc function when the macro of LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK is defined in LiteOS.</li>
 *<li>LOS_MemIntegrityCheck function can be called by user anytime.</li>
 *</ul>
 *
 *@param  pPool              [IN] A pointer pointed to the memory pool.
 *
 *@retval #LOS_NOK           The memory pool (pPool) is impaired.
 *@retval #LOS_OK            The memory pool (pPool) is integrated.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemIntegrityCheck(VOID *pPool);

/**
 *@ingroup los_memory
 *@brief Check the size of memory node specified.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to check the size of memory node.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input pPool parameter must be initialized via func LOS_MemInit.</li>
 *<li>The input pPtr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 *<li>The function will be called by function specified, such as memset or memcpy.</li>
 *<li>The feature can be enabled when you set the macro value of LOSCFG_BASE_MEM_NODE_SIZE_CHECK as YES.</li>
 *<li>You had better set memory check level as LOS_MEM_CHECK_LEVEL_DISABLE when copy bin file.</li>
 *</ul>
 *
 *@param  pPool              [IN]  A pointer pointed to the memory pool.
 *@param  pPtr               [IN]  A pointer pointed to the source node.
 *@param  puwTotalSize       [OUT] A pointer to save total size, must point to valid memory.
 *@param  puwAvailSize       [OUT] A pointer to save available size, must point to valid memory.
 *
 *@retval #LOS_ERRNO_MEMCHECK_DISABLED           Memcheck function does not open.
 *@retval #LOS_ERRNO_MEMCHECK_PARA_NULL          The pool or pPtr is NULL.
 *@retval #LOS_ERRNO_MEMCHECK_OUTSIDE            The pPtr address is not in the reasonable range.
 *@retval #LOS_ERRNO_MEMCHECK_NO_HEAD            Can't find the control head node from pPtr.
 *@retval #LOS_ERRNO_MEMCHECK_WRONG_LEVEL        The memory check level is illegal.
 *@retval #LOS_OK                               Success to get total size and available size of the memory node (pPtr).
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemCheckLevelSet | LOS_MemCheckLevelGet
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemNodeSizeCheck(VOID *pPool, VOID *pPtr, UINT32 *puwTotalSize, UINT32 *puwAvailSize);

/**
 *@ingroup los_memory
 *@brief Set the memory check level.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to set the memory check level.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>There are three level you can set.</li>
 *<li>The legal level are LOS_MEM_CHECK_LEVEL_LOW, LOS_MEM_CHECK_LEVEL_HIGH, LOS_MEM_CHECK_LEVEL_DISABLE.</li>
 *</ul>
 *
 *@param  ucLevel                                  [IN] The level what you want to set.
 *
 *@retval #LOS_ERRNO_MEMCHECK_WRONG_LEVEL           The memory check level what you want to set is illegal.
 *@retval #LOS_OK                                  Success to set the memory check level.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemNodeSizeCheck | LOS_MemCheckLevelGet
 *@since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_MemCheckLevelSet(UINT8 ucLevel);

/**
 *@ingroup los_memory
 *@brief Get the memory check level.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the current memory check level.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param  None
 *
 *@retval #UINT8           The current memory check level.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_MemNodeSizeCheck | LOS_MemCheckLevelSet
 *@since Huawei LiteOS V100R001C00
 */
extern UINT8 LOS_MemCheckLevelGet(VOID);

#ifdef LOS_MEM_SLAB
/**
 *@ingroup los_memory
 *@brief Get the SLAB used blocks number.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the SLAB used blocks number.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>if the input uwClassSize value is in the range of two continuous classes, the result will be the bigger class.</li>
 *<li>the input pool address pHeapAddr must be init by LOS_MemInit.</li>
 *</ul>
 *
 *@param  pHeapAddr          [IN] The heap address.
 *@param  uwClassSize        [IN] The SLAB class size.
 *
 *@retval #UINT32           The SLAB used blocks number.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SlabFreeBlksGet
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 LOS_SlabUsedBlksGet(VOID *pHeapAddr, UINT32 uwClassSize);

/**
 *@ingroup los_memory
 *@brief Get the SLAB free blocks number.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the SLAB free blocks number.</li>
 *<li>the input pool address pHeapAddr must be init by LOS_MemInit.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>if the input uwClassSize value is in the range of two continuous classes, the result will be the bigger class.</li>
 *</ul>
 *
 *@param  pHeapAddr          [IN] The heap address.
 *@param  uwClassSize        [IN] The SLAB class size.
 *
 *@retval #UINT32           The SLAB free blocks number.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SlabFreeBlksGet
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 LOS_SlabFreeBlksGet(VOID *pHeapAddr, UINT32 uwClassSize);

/**
 *@ingroup los_memory
 *@brief Init the SLAB allocator config info struct.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to init the SLAB allocator config info struct.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>All the allocator size will be set with LOS_SLAB_DEFAULT_ALLOCATOR_SIZE, when call this function.</li>
 *<li>If you will call the func LOS_SlabAllocatorCfgRegister, you should call this func first.</li>
 *</ul>
 *
 *@param  pstCfg      [IN] The SLAB allocator config info struct.
 *
 *@retval #VOID
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SlabAllocatorCfgRegister
 *@since Huawei LiteOS V200R001C00
 */
extern VOID LOS_SlabCfgStructInit(struct LOS_SLAB_ALLOCATOR_CFG_INFO *pstCfg);

/**
 *@ingroup los_memory
 *@brief Config all the SLAB class allocator size.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to config all the SLAB class allocator size.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>You should call LOS_SlabCfgStructInit first before call this func.</li>
 *<li>Each pool has its pstCfg, it should be set if necessary (the set value can refer to LOS_SlabInfoStatisticsPrint), otherwise, the SLAB class allocator size will use LOS_SLAB_DEFAULT_ALLOCATOR_SIZE.</li>
 *<li>If the same pool config more than once, the last config will take effect.</li>
 *<li>The sum of all the class allocator config size must be not greater than OS_SYS_MEM_SIZE.</li>
 *</ul>
 *
 *@param  pHeapAddr   [IN] The heap address.
 *@param  pstCfg      [IN] The SLAB allocator config info struct.
 *
 *@retval #VOID
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SlabCfgStructInit
 *@since Huawei LiteOS V200R001C00
 */
extern VOID LOS_SlabAllocatorCfgRegister(VOID * pHeapAddr, struct LOS_SLAB_ALLOCATOR_CFG_INFO *pstCfg);

#ifdef LOS_SLAB_INFO_STATISTIC
/**
 *@ingroup los_memory
 *@brief print the SLAB info.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to print the SLAB info.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>Print all SLAB class max block used of each config pool.</li>
 *<li>If you want to statistic SLAB info for some pools, you should register these pools first via LOS_SlabAllocatorCfgRegister.</li>
 *<li>If some SLAB class allocator size set 0, these class will not to be counted.</li>
 *</ul>
 *
 *@param  pstSlabCtrlHdr     [IN] The SLAB control header.
 *
 *@retval #UINT32           The SLAB free buckets number.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see None.
 *@since Huawei LiteOS V200R001C00
 */
extern VOID LOS_SlabInfoStatisticsPrint(VOID);
#endif

#ifdef LOSCFG_MEM_MUL_POOL

 /**
 *@ingroup los_memory
 *@brief get special module's SLAB consume size.
 *
 *@par Description:
 *<li>This API is used to get special module's SLAB consume size.</li>
 *@param  uwModule   [IN] module ID (0~MODULE_MAX).
 *
 *@retval #UINT32         The size of the special module's SLAB consumed.
 *@retval #OS_NULL_INT           The illegal module.
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 LOS_SlabMusedGet(UINT32 uwModule);
#endif

#endif

#ifdef LOSCFG_KERNEL_MEM_SPEC
 /**
 *@ingroup los_memory
 *@brief config the spec.
 *
 *@par Description:
 *<li>This API is used to config the class of spec .</li>
 *@param  pHeapAddr   [IN] The pool address.
 *@param  uwBlkSzMin    [IN] The min size of class.
 *@param  uwBlkSzMax    [IN] The max size of class.
 *@param  uwBlkNum      [IN] The num of blocks of class.
 *
 *@retval #LOS_NOK    config failed
 *@retval #LOS_OK    config success
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern UINT32 LOS_SpecMemCfgInit(VOID *pHeapAddr, UINT32 uwBlkSzMin, UINT32 uwBlkSzMax, UINT32 uwBlkNum);

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
 /**
 *@ingroup los_memory
 *@brief show the infomation of spec.
 *
 *@par Description:
 *<li>This API is used to show infomations of each class of spec .</li>
 *@param  None
 *
 *@retval None
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@since Huawei LiteOS V200R001C00
 */
extern VOID LOS_SpecInfoShow(VOID);
/**
 *@ingroup los_memory
 *@brief Get the spec used blocks number.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the spec used blocks number.</li>
 *<li>the input pool address pHeapAddr must be init by LOS_MemInit.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>if the input uwClassSize value is in the range of two continuous classes, the result will be the little class.</li>
 *</ul>
 *
 *@param  pHeapAddr          [IN] The heap address.
 *@param  uwClassSize        [IN] The spec class size.
 *
 *@retval #UINT32           The spec used blocks number.
 *@retval #-1               Get the spec used blocks number failed
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SpecFreeBlksGet
 *@since Huawei LiteOS V200R001C00
 */
extern INT32 LOS_SpecUsedBlksGet(VOID *pHeapAddr, UINT32 uwClassSize);
/**
 *@ingroup los_memory
 *@brief Get the spec free blocks number.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to get the spec free blocks number.</li>
 *<li>the input pool address pHeapAddr must be init by LOS_MemInit.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>if the input uwClassSize value is in the range of two continuous classes, the result will be the little class.</li>
 *</ul>
 *
 *@param  pHeapAddr          [IN] The heap address.
 *@param  uwClassSize        [IN] The spec class size.
 *
 *@retval #UINT32           The spec free blocks number.
 *@retval #-1               Get the spec free blocks number failed
 *@par Dependency:
 *<ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_SpecUsedBlksGet
 *@since Huawei LiteOS V200R001C00
 */
extern INT32 LOS_SpecFreeBlksGet(VOID *pHeapAddr, UINT32 uwClassSize);
#endif
#endif
/**
 *@ingroup los_memory
 * Define a mem size check intensity
 *
 * Lowest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_LOW      0

/**
 *@ingroup los_memory
 * Define a mem size check intensity
 *
 * Highest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_HIGH      1

/**
 *@ingroup los_memory
 * Define a mem size check intensity
 *
 * disable mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DISABLE       0xff

/**
 *@ingroup los_memory
 * Define a mem size check intensity
 *
 * default intensity set mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DEFAULT       LOS_MEM_CHECK_LEVEL_DISABLE

/**
*@ingroup los_memory
*  memcheck error code: the pPtr or pool is NULL
*  Value: 0x02000101
*  Solution: don't give a NULL parameter
*/
#define  LOS_ERRNO_MEMCHECK_PARA_NULL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x1)

/**
*@ingroup los_memory
*  memcheck error code: the pPtr addr not in the suit range
*  Value: 0x02000102
*  Solution: check pPtr and comfirm it included by stack
*/
#define  LOS_ERRNO_MEMCHECK_OUTSIDE      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x2)

/**
*@ingroup los_memory
*  memcheck error code: can't find the ctrl node
*  Value: 0x02000103
*  Solution: confirm the pPtr if this node has been freed or has not been alloced
*/
#define  LOS_ERRNO_MEMCHECK_NO_HEAD      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x3)

/**
*@ingroup los_memory
*  memcheck error code: the para level is wrong
*  Value: 0x02000104
*  Solution: checkout the memcheck level by the func "OS_GetMemCheck_Level"
*/
#define  LOS_ERRNO_MEMCHECK_WRONG_LEVEL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x4)

/**
*@ingroup los_memory
*  memcheck error code: memcheck func not open
*  Value: 0x02000105
*  Solution: enable memcheck by the func "OS_SetMemCheck_Level"
*/
#define  LOS_ERRNO_MEMCHECK_DISABLED      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x5)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_H */
