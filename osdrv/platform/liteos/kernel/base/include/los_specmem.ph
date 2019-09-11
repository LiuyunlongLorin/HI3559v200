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

#ifndef _LOS_SPECMEM_PH
#define _LOS_SPECMEM_PH

#ifdef LOSCFG_KERNEL_MEM_SPEC

#include "los_memory.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct LOS_SPEC_ATOMIC_BITSET{
    UINT32 uwNumBits;
    UINT32 uwBitmap[];/*lint !e43*/
};

struct LOS_SPEC_ALLOCATOR_HEADER{
    UINT32 uwItemSz;                          //the size of each block
    UINT8 *ucDataChunks;                      //the start addr of data of class
    struct LOS_SPEC_ATOMIC_BITSET stBitset[0];/*lint !e43*/
};

struct LOS_SPEC_CLASS{
    UINT32 uwBlkSzMin;                           //the min size of class
    UINT32 uwBlkSzMax;                           //the max size of class
    UINT32 uwClassAllocNum;                      //the num of nodes of class
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    UINT32 uwClassCurUsedNum;                      //the num of nodes of class
    UINT32 uwClassWaterLine;                       //the num of nodes of class
#endif
    struct LOS_SPEC_ALLOCATOR_HEADER *pstSpecClassAllocator;  //the malloc addr of class
};
struct LOS_SPEC_CONTROL_HEADER{
    struct LOS_SPEC_CLASS stSpecMemClass[LOS_SPEC_CLASS_CNT];
};

typedef struct tagLOS_SPEC_BLOCK_NODE{
    UINT16 usMagic;
    UINT16 usRecordId;
    UINT32 uwBlkSz;
#if defined(__LP64__)
    UINT32 uwReserve;
#endif
#ifdef LOSCFG_MEM_MUL_MODULE
    UINT32 uwModuleId;
#endif
}LOS_SPEC_BLOCK_NODE;


#define OS_SPEC_ALIGN_SIZE   sizeof(VOID *)
#define OS_MEM_SPEC_ALIGN(p, alignSize)    (((AARCHPTR)(p) + alignSize -1) & (~ ((AARCHPTR)(alignSize -1))))

#define OS_MEM_NODE_USED_FLAG 0x80000000
#define OS_MEM_NODE_ALIGNED_FLAG 0x40000000
#define OS_MEM_NODE_USED_AND_ALIGNED_FLAG (OS_MEM_NODE_USED_FLAG | OS_MEM_NODE_ALIGNED_FLAG)

#define OS_SPEC_MAGIC (0xbaba)
#define OS_SPEC_BLOCK_HEAD_GET(pPtr)                                   ((LOS_SPEC_BLOCK_NODE *)((AARCHPTR)pPtr - sizeof(LOS_SPEC_BLOCK_NODE)))

#define OS_SPEC_BLOCK_MAGIC_SET(pstSpecNode)                           (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->usMagic = (UINT16)OS_SPEC_MAGIC)
#define OS_ALLOC_FROM_SPEC_CHECK(pstSpecNode)                          (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->usMagic == (UINT16)OS_SPEC_MAGIC)
#define OS_SPEC_BLOCK_MAGIC_GET(pstSpecNode)                           (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->usMagic)

#define OS_SPEC_BLOCK_SIZE_SET(pstSpecNode, uwSize)                    (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->uwBlkSz = uwSize)
#define OS_SPEC_BLOCK_SIZE_GET(pstSpecNode)                            (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->uwBlkSz)

#define OS_SPEC_BLOCK_ID_SET(pstSpecNode, usId)                        (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->usRecordId = usId)
#define OS_SPEC_BLOCK_ID_GET(pstSpecNode)                              (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->usRecordId)

#ifdef LOSCFG_MEM_MUL_MODULE
#define OS_SPEC_MODID_SET(pstSpecNode, uwId)                           (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->uwModuleId = uwId)
#define OS_SPEC_MODID_GET(pstSpecNode)                                 (((LOS_SPEC_BLOCK_NODE *)pstSpecNode)->uwModuleId)
#endif

#define OS_SPEC_BLOCK_SET_ALIGNED_FLAG(uwSizeAndFlag)                  (uwSizeAndFlag = ((uwSizeAndFlag) | OS_MEM_NODE_ALIGNED_FLAG))
#define OS_SPEC_BLOCK_GET_ALIGNED_FLAG(uwSizeAndFlag)                  ((uwSizeAndFlag) & OS_MEM_NODE_ALIGNED_FLAG)
#define OS_SPEC_BLOCK_GET_ALIGNED_GAPSIZE(uwSizeAndFlag)               ((uwSizeAndFlag) & (~OS_MEM_NODE_ALIGNED_FLAG))

#define OS_SPEC_ATOMIC_BITSET_SIZE_GET(numbits)                        (sizeof(struct LOS_SPEC_ATOMIC_BITSET) + ((numbits) + 31) / 8)

extern UINT32 g_uwSpecMemClassCnt;
extern UINT32 g_uwSpecMemClassMaxCnt;
extern VOID  *osSpecMemAlloc(VOID *pHeapAddr, UINT32 uwAllocSize);
extern VOID  *osSpecMemAllocAlign(VOID *pHeapAddr, UINT32 uwAllocSize, UINT32 uwBoundary);
extern UINT32 osSpecMemFree(VOID *pHeapAddr, VOID* pPtr);
extern INT32  osClassLevelGet(UINT32 allocSize);
extern VOID *osSpecGetOriginPtr(VOID *pHeapAddr, VOID* pPtr);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SPECMEM_PH */
