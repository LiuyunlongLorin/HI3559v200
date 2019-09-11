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

#ifndef _LOS_SLABMEM_PH
#define _LOS_SLABMEM_PH

#include "los_memory.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOS_MEM_SLAB
struct LOS_ATOMIC_BITSET{
    UINT32 uwNumBits;
    UINT32 uwBitmap[];/*lint !e43*/
};

struct LOS_SLAB_ALLOCATOR_HEADER{
    UINT32 uwItemSz;
    UINT8 *ucDataChunks;
    struct LOS_ATOMIC_BITSET stBitset[0];/*lint !e43*/
};

struct LOS_SLAB_CLASS{
    UINT32 uwBlkSz;
    struct LOS_SLAB_ALLOCATOR_HEADER *pstSlabClassAllocator;
};

struct LOS_SLAB_CONTROL_HEADER{
#ifdef LOS_SLAB_INFO_STATISTIC
    struct LOS_SLAB_ALLOCATOR_CFG_INFO stAllocatorSizeCfg;
#endif
    struct LOS_SLAB_CLASS stSlabClass[LOS_SLAB_CLASS_CNT];
};

typedef struct tagLOS_SLAB_BLOCK_NODE{
    UINT16 usMagic;
    UINT8  ucBlkSz;
    UINT8  ucRecordId;
#ifdef LOSCFG_MEM_MUL_MODULE
    UINT32  ucModuleId;
#endif
}LOS_SLAB_BLOCK_NODE;

#define OS_SLAB_MAGIC (0xdede)
#define OS_SLAB_BLOCK_HEAD_GET(pPtr)                                   ((LOS_SLAB_BLOCK_NODE *)((UINT8 *)pPtr - sizeof(LOS_SLAB_BLOCK_NODE)))
#define OS_SLAB_BLOCK_MAGIC_SET(pstSlabNode)                           (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->usMagic = (UINT16)OS_SLAB_MAGIC)
#define OS_SLAB_BLOCK_MAGIC_GET(pstSlabNode)                           (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->usMagic)
#define OS_SLAB_BLOCK_SIZE_SET(pstSlabNode, uwSize)                    (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucBlkSz = uwSize)
#define OS_SLAB_BLOCK_SIZE_GET(pstSlabNode)                            (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucBlkSz)
#define OS_SLAB_BLOCK_ID_SET(pstSlabNode, uwId)                        (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucRecordId = uwId)
#define OS_SLAB_BLOCK_ID_GET(pstSlabNode)                              (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucRecordId)
#ifdef LOSCFG_MEM_MUL_MODULE
#define OS_SLAB_MODID_SET(pstSlabNode, uwId)                           (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucModuleId = uwId)
#define OS_SLAB_MODID_GET(pstSlabNode)                                 (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->ucModuleId)
#endif
#define OS_ALLOC_FROM_SLAB_CHECK(pstSlabNode)                          (((LOS_SLAB_BLOCK_NODE *)pstSlabNode)->usMagic == (UINT16)OS_SLAB_MAGIC)

#define OS_SLAB_LOG2(value)                                            (32 - CLZ(value) - 1)
#define OS_SLAB_CLASS_LEVEL_GET(uwSize)                                ((uwSize - 1) >>  OS_SLAB_LOG2(LOS_SLAB_HEAP_BOUNDARY / LOS_SLAB_CLASS_CNT))

#define OS_ATOMIC_BITSET_SIZE_GET(numbits)                             (sizeof(struct LOS_ATOMIC_BITSET) + ((numbits) + 31) / 8)

#define OS_SLAB_MIN_SIZE() (sizeof(struct LOS_SLAB_CONTROL_HEADER))

#ifdef LOS_SLAB_INFO_STATISTIC
#define OS_MEM_NUM_LIMIT (10)

struct LOS_SLAB_STATISTIC_INFO{
    VOID *pHeapAddr;
    UINT16 usClassSizeMaxUsed[LOS_SLAB_CLASS_CNT];
};
extern VOID osSlabInfoStatistic(VOID *pHeapAddr);
#endif

extern VOID *osSlabMemAlloc(VOID *pHeapAddr, UINT32 uwAllocSize);
extern UINT32 osSlabMemFree(VOID *pHeapAddr, VOID* pPtr);
extern UINT32 osSlabMemInit(VOID *pHeapAddr);

#ifdef LOSCFG_MEM_MUL_POOL
extern UINT32 osSlabMemDeInit(VOID *pHeapAddr);
#endif
#ifdef LOSCFG_MEM_MUL_MODULE
extern UINT32 g_uwSlabmemInfo[MEM_MODULE_MAX + 1];
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SLABMEM_PH */
