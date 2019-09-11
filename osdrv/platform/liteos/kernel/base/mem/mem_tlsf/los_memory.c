/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
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
#include "los_memory.inc"
#include "los_task.ph"
#ifdef LOS_MEM_SLAB
#include "los_slabmem.ph"
#endif
#ifdef LOSCFG_KERNEL_MEM_SPEC
#include "los_specmem.ph"
#endif
#include "los_exc.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**********************************************************************/
static inline INT32 __log2(UINT32 uwWord)
{
    return uwWord ? (sizeof(uwWord) * 8 - CLZ(uwWord) - 1) : 0;
}

static inline INT32 tlsf_ffs(UINT32 uwWord)
{
    return __log2(uwWord & (UINT32)(-(UINT32)uwWord)); /*lint !e501*/
}

static inline INT32 tlsf_fls(UINT32 uwWord)
{
    const INT32 swBit = uwWord ? 32 - CLZ(uwWord) : 0;
    return swBit - 1;
}

#if defined(__LP64__)
static inline INT32 tlsf_fls_sizet(AARCHPTR uwSize)
{
    INT32 swHigh = (INT32)(uwSize >> 32);
    INT32 swBits = 0;
    if (swHigh) {
        swBits = 32 + tlsf_fls(swHigh);
    } else {
        swBits = tlsf_fls((INT32)uwSize & 0xffffffff);
    }
    return swBits;
}
#else
#define tlsf_fls_sizet tlsf_fls
#endif


#if __GNUC__ || __INTEL_COMPILER
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

/*
 * Data structures and associated constants.
 */

/*
 * Block header structure.
 *
 * There are several implementation subtleties involved:
 * - The prev_phys_block field is only valid if the previous pNode is free.
 * - The prev_phys_block field is actually stored at the end of the
 *   previous pNode. It appears at the beginning of this structure only to
 *   simplify the implementation.
 * - The pNext_free / pPrev_free fields are only valid if the pNode is free.
 */
typedef struct LOS_MEM_DYN_NODE {
    /* Points to the previous physical pNode. */
    struct LOS_MEM_DYN_NODE *pstPreNode;

#if defined(__LP64__)
    UINT32 uwReserve;
#endif
    /* The size of this pNode, excluding the pNode header. */
    UINT32 uwSize;

    /* Next and previous free blocks. */
    struct LOS_MEM_DYN_NODE *pNext_free;
    struct LOS_MEM_DYN_NODE *pPrev_free;
} LOS_MEM_DYN_NODE;

enum tlsf_public {
    SL_INDEX_COUNT_LOG2 = 2,
};

enum tlsf_private {
#if defined(__LP64__)
    /* All allocation sizes and addresses are aligned to 8 bytes. */
    ALIGN_SIZE_LOG2 = 3,
#else
    /* All allocation sizes and addresses are aligned to 4 bytes. */
    ALIGN_SIZE_LOG2 = 2,
#endif
    ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

    FL_INDEX_MAX = 30,

    SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
    FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
    FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

typedef struct LOS_MEM_POOL_INFO {
    LOS_MEM_DYN_NODE stBlock_null;
#ifdef LOS_MEM_SLAB
    struct LOS_SLAB_CONTROL_HEADER stSlabCtrlHdr;
#endif
#ifdef LOSCFG_KERNEL_MEM_SPEC
    struct LOS_SPEC_CONTROL_HEADER stSpecCtrlHdr;
#endif
    UINT32 uwPoolSize;

    UINT32 fl_bitmap;
    UINT32 sl_bitmap[FL_INDEX_COUNT];

    LOS_MEM_DYN_NODE *pstBlocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    UINT32 uwPoolWaterLine;                 /**<Maximum usage size in a memory  pool  */
    UINT32 uwPoolCurUsedSize;               /**<Current usage size in a memory  pool  */
#endif
#ifdef LOSCFG_MEM_MUL_POOL
    VOID *pNextPool;
#endif
} LOS_MEM_POOL_INFO;

#ifdef LOSCFG_MEM_MUL_POOL
VOID *pool_head = NULL;
#endif

/* pool_t: a pNode of memory that TLSF can manage. */
typedef VOID *pool_t;

/* Add/remove memory pools. */
pool_t osMemPoolAdd(VOID *tlsf, VOID *pPool, UINT32 uwBytes);

/* Returns internal pNode size, not original request size */
UINT32 LOS_MemPtrSize(VOID *pPtr);

/* Overheads/limits of internal structures. */
UINT32 LOS_MemHeadSize(VOID);

/* Debugging. */
 #define TSLF_CONFIG_DEBUG

#ifdef TLSF_CONFIG_ASSERT
#include <assert.h>
#define tlsf_assert(expr) assert(expr)
#define   alignCheck(align)  tlsf_assert(0 == ((align) & ((align) - 1)) && "must align to a power of two");
#else
#define tlsf_assert(expr) (VOID)(0)
#define   alignCheck(align)
#endif

/*
 * Cast and min/max macros.
 */
#define tlsf_cast(t, exp) ((t)(exp))
#define tlsf_min(a, b) ((a) < (b) ? (a) : (b))
#define tlsf_max(a, b) ((a) > (b) ? (a) : (b))

#define tlsf_offset(a,b) LOS_OFF_SET_OF(a, b)

/*
 * Since pNode sizes are always at least a multiple of 4, the two least
 * significant bits of the size field are used to store the pNode status:
 * - bit 0: whether pNode is busy or free
 * - bit 1: whether previous pNode is busy or free
 */
static const UINT32 block_header_free_bit = 1 << 0;
static const UINT32 block_header_prev_free_bit = 1 << 1;

#if defined(__LP64__)
#define DEBUG_SPACE   8
#else
#define DEBUG_SPACE   4
#endif

/*
 * The size of the pNode header exposed to used blocks is the size field.
 * The pstPreNode field is stored *inside* the previous free pNode.
 */
 #if defined(__LP64__)
static const UINT32 block_header_overhead = tlsf_offset(LOS_MEM_DYN_NODE, uwSize) - sizeof(UINT32);
#else
static const UINT32 block_header_overhead = tlsf_offset(LOS_MEM_DYN_NODE, uwSize);
#endif
/* User data starts directly after the size field in a used pNode. */
static const UINT32 block_start_offset =
    tlsf_offset(LOS_MEM_DYN_NODE, uwSize) + sizeof(UINT32) + DEBUG_SPACE; /*lint !e413*/

/*
 * A free pNode must be large enough to store its header minus the size of
 * the pstPreNode field, and no larger than the number of addressable
 * bits for FL_INDEX.
 */
static const UINT32 block_size_min =
    sizeof(LOS_MEM_DYN_NODE) - sizeof(LOS_MEM_DYN_NODE *);
static const UINT32 block_size_max = tlsf_cast(UINT32, 1) << FL_INDEX_MAX;

#define control_t LOS_MEM_POOL_INFO

#define OS_MEM_TASKID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->pNext_free); \
                uwTmp &= 0xffff0000; \
                uwTmp |= ID; \
                ((LOS_MEM_DYN_NODE *)node)->pNext_free = (LOS_MEM_DYN_NODE *)uwTmp; \
            }while(0)

#define OS_MEM_TASKID_GET(node)  ((AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->pNext_free) & 0xffff)

#define OS_MEM_MODID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->pNext_free); \
                uwTmp &= 0xffff; \
                uwTmp |= ID << 16; \
                ((LOS_MEM_DYN_NODE *)node)->pNext_free = (LOS_MEM_DYN_NODE *)uwTmp; \
            }while(0)
#define OS_MEM_MODID_GET(node)  ((AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->pNext_free) >> 16)
VOID osMemInfoPrint(pool_t pPool);
#define IS_ALIGNED(value, alignSize)  (0 == (((AARCHPTR)(value)) & ((AARCHPTR)(alignSize - 1))))
#define IS_POW_TWO(value) (0 == (((AARCHPTR)value) & ((AARCHPTR)value - 1)))
/*
 * LOS_MEM_DYN_NODE member functions.
 */
static VOID osMemNodeSizeSet(LOS_MEM_DYN_NODE *pNode, UINT32 uwSize)
{
    pNode->uwSize = uwSize | (pNode->uwSize & (block_header_free_bit | block_header_prev_free_bit));
}

static UINT32 osMemNodeSizeGet(const LOS_MEM_DYN_NODE *pNode)
{
    return pNode->uwSize & ~(block_header_free_bit | block_header_prev_free_bit);
}

static INT32 osMemEndCheck(const LOS_MEM_DYN_NODE *pNode)
{
    return osMemNodeSizeGet(pNode) == 0;
}

static INT32 osMemFreeCheck(const LOS_MEM_DYN_NODE *pNode)
{
    return tlsf_cast(INT32, pNode->uwSize & block_header_free_bit);
}

static VOID osMemFreeMark(LOS_MEM_DYN_NODE *pNode)
{
    pNode->uwSize |= block_header_free_bit;
}

static VOID osMemUsedMark(LOS_MEM_DYN_NODE *pNode)
{
    pNode->uwSize &= ~block_header_free_bit;
}

static INT32 osMemPreFreeCheck(const LOS_MEM_DYN_NODE *pNode)
{
    return tlsf_cast(INT32, pNode->uwSize & block_header_prev_free_bit);
}

static VOID osMemPreFreeMark(LOS_MEM_DYN_NODE *pNode)
{
    pNode->uwSize |= block_header_prev_free_bit;
}

static VOID osMemPreUsedMark(LOS_MEM_DYN_NODE *pNode)
{
    pNode->uwSize &= ~block_header_prev_free_bit;
}

LOS_MEM_DYN_NODE *osMemPtrToNode(const VOID *pPtr)
{
    return tlsf_cast(LOS_MEM_DYN_NODE *,
                     tlsf_cast(UINT8*, pPtr) - block_start_offset);
}

static VOID *osMemNodeToPtr(const LOS_MEM_DYN_NODE *pNode)
{
    return tlsf_cast(VOID *,
                     tlsf_cast(UINT8 *, pNode) + block_start_offset);
}

/* Return location of next pNode after pNode of given size. */
static LOS_MEM_DYN_NODE *osMemPtrOffset(const VOID *pPtr, ssize_t swSize)
{
    return tlsf_cast(LOS_MEM_DYN_NODE *, tlsf_cast(AARCHPTR, pPtr) + swSize);
}

static LOS_MEM_DYN_NODE * osMemFirstNode(VOID *pPool)
{
    return osMemPtrOffset((const VOID *)((AARCHPTR)pPool + LOS_MemHeadSize()), -(ssize_t)block_header_overhead); /*lint !e570*/
}

static LOS_MEM_DYN_NODE *osMemEndNode(pool_t pPool)
{
    return (LOS_MEM_DYN_NODE *)((AARCHPTR)pPool + ((control_t *)pPool)->uwPoolSize - block_start_offset);
}

static AARCHPTR osMemEndPtr(pool_t pPool)
{
    return (AARCHPTR)pPool + ((control_t *)pPool)->uwPoolSize;
}

/* Return location of previous pNode. */
static LOS_MEM_DYN_NODE *osMemNodePre(const LOS_MEM_DYN_NODE *pNode)
{
    tlsf_assert(osMemPreFreeCheck(pNode) && "previous pNode must be free");
    return pNode->pstPreNode;
}

/* Return location of next existing pNode. */
static LOS_MEM_DYN_NODE *osMemNodeNext(const LOS_MEM_DYN_NODE *pNode)
{
    LOS_MEM_DYN_NODE *pNext = osMemPtrOffset(
        tlsf_cast(VOID *, pNode), (ssize_t)osMemNodeSizeGet(pNode));
    tlsf_assert(!osMemEndCheck(pNode));
    return pNext;
}

static LOS_MEM_DYN_NODE *osMemNextLink(LOS_MEM_DYN_NODE *pNode)
{
    LOS_MEM_DYN_NODE *pNext = osMemNodeNext(pNode);
    pNext->pstPreNode = pNode;
    return pNext;
}

static VOID osMemFreeSet(LOS_MEM_DYN_NODE *pNode)
{
    /* Link the pNode to the next pNode, first. */
    LOS_MEM_DYN_NODE *pNext = osMemNextLink(pNode);
    osMemPreFreeMark(pNext);
    osMemFreeMark(pNode);
}

static VOID osMemUsedSet(LOS_MEM_DYN_NODE *pNode)
{
    LOS_MEM_DYN_NODE *pNext = osMemNodeNext(pNode);
    osMemPreUsedMark(pNext);
    osMemUsedMark(pNode);
}

static AARCHPTR osMemAlignDown(AARCHPTR uwData, UINT32 uwAlign)
{
    alignCheck(uwAlign);
    return uwData - (uwData & (uwAlign - 1));
}

static AARCHPTR osMemAlignUp(AARCHPTR uwData, UINT32 uwAlign)
{
    alignCheck(uwAlign);
    return (uwData + (AARCHPTR)(uwAlign - 1)) & ~((AARCHPTR)(uwAlign - 1));
}

static VOID *osMemAlignPtr(const VOID *pPtr, UINT32 uwAlign)
{
    const AARCHPTR uwAligned = (tlsf_cast(AARCHPTR, pPtr) + (AARCHPTR)(uwAlign - 1)) & ~((AARCHPTR)(uwAlign - 1));
    alignCheck(uwAlign);
    return tlsf_cast(VOID *, uwAligned);
}

static UINT32 osMemReqSizeAdjust(UINT32 uwSize, UINT32 uwAlign)
{
    UINT32 uwAdjust = 0;
    if (uwSize && uwSize < block_size_max)
    {
        const UINT32 uwAligned = osMemAlignUp(uwSize, uwAlign) + block_start_offset;
        uwAdjust = tlsf_max(uwAligned, block_size_min);
    }
    return uwAdjust;
}

static VOID osMemMapInsert(UINT32 uwSize, INT32 *pFli, INT32 *pSli)
{
    INT32 swFl, swSl;
    if (uwSize < SMALL_BLOCK_SIZE) {
        /* Store small blocks in first list. */
        swFl = 0;
        swSl = tlsf_cast(INT32, uwSize) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    } else {
        swFl = tlsf_fls_sizet(uwSize);
        swSl = tlsf_cast(INT32, uwSize >> (swFl - SL_INDEX_COUNT_LOG2)) ^
             (1 << SL_INDEX_COUNT_LOG2);
        swFl -= (FL_INDEX_SHIFT - 1);
    }
    *pFli = swFl;
    *pSli = swSl;
}

static VOID osMemMapSearch(UINT32 uwSize, INT32 *pFli, INT32 *pSli)
{
    if (uwSize >= SMALL_BLOCK_SIZE) {
        const UINT32 uwRound = (1 << (tlsf_fls_sizet(uwSize) - SL_INDEX_COUNT_LOG2)) - 1;
        uwSize += uwRound;
    }
    osMemMapInsert(uwSize, pFli, pSli);
}

static LOS_MEM_DYN_NODE *osMemSuitableNodeSearch(control_t *control,
                                             INT32 *pFli, INT32 *pSli)
{
    INT32 swFl = *pFli;
    INT32 swSl = *pSli;
    UINT32 swSl_map = control->sl_bitmap[swFl] & (((UINT32)~0) << swSl);
    if (!swSl_map) {
        /* No pNode exists. Search in the next largest first-level list. */
        const UINT32 swFl_map = control->fl_bitmap & (((UINT32)~0) << (swFl + 1));
        if (!swFl_map) {
            /* No free blocks available, memory has been exhausted. */
            return NULL;
        }

        swFl = tlsf_ffs(swFl_map);
        *pFli = swFl;
        swSl_map = control->sl_bitmap[swFl];
    }
    tlsf_assert(swSl_map && "internal error - second level bitmap is null");
    swSl = tlsf_ffs(swSl_map);
    *pSli = swSl;

    /* Return the first pNode in the free list. */
    return control->pstBlocks[swFl][swSl];
}

static VOID osMemFreeNodeRemove(control_t *control,
                              LOS_MEM_DYN_NODE *pNode,
                              INT32 swFl, INT32 swSl)
{
    LOS_MEM_DYN_NODE *pPrev = pNode->pPrev_free;
    LOS_MEM_DYN_NODE *pNext = pNode->pNext_free;
    tlsf_assert(pPrev && "pPrev_free field can not be null");
    tlsf_assert(pNext && "pNext_free field can not be null");
    pNext->pPrev_free = pPrev;
    pPrev->pNext_free = pNext;
    /* If this pNode is the head of the free list, set new head. */
    if (control->pstBlocks[swFl][swSl] == pNode) {
        control->pstBlocks[swFl][swSl] = pNext;
        /* If the new head is null, clear the bitmap. */
        if (pNext == &control->stBlock_null) {
            control->sl_bitmap[swFl] &= ~(1 << swSl); /*lint !e502*/

            /* If the second bitmap is now empty, clear the fl bitmap. */
            if (!control->sl_bitmap[swFl]) {
                control->fl_bitmap &= ~(1 << swFl); /*lint !e502*/
            }
        }
    }
}

/* Insert a free pNode into the free pNode list. */
static VOID osMemFreeNodeInsert(control_t *control,
                              LOS_MEM_DYN_NODE *pNode,
                              INT32 swFl, INT32 swSl)
{
    LOS_MEM_DYN_NODE *pstCurrent = control->pstBlocks[swFl][swSl];
    tlsf_assert(pstCurrent && "free list cannot have a null entry");
    tlsf_assert(pNode && "cannot insert a null entry into the free list");
    pNode->pNext_free = pstCurrent;
    pNode->pPrev_free = &control->stBlock_null;
    pstCurrent->pPrev_free = pNode;

    tlsf_assert(osMemNodeToPtr(pNode) ==
                    osMemAlignPtr(osMemNodeToPtr(pNode), ALIGN_SIZE) &&
                "pNode not aligned properly");
    /*
     * Insert the new pNode at the head of the list, and mark the first-
     * and second-level bitmaps appropriately.
     */
    control->pstBlocks[swFl][swSl] = pNode;
    control->fl_bitmap |= (1 << swFl);
    control->sl_bitmap[swFl] |= (1 << swSl);
}

/* Remove a given pNode from the free list. */
static VOID osMemNodeRemove(control_t *control, LOS_MEM_DYN_NODE *pNode)
{
    INT32 swFl, swSl;
    osMemMapInsert(osMemNodeSizeGet(pNode), &swFl, &swSl);
    osMemFreeNodeRemove(control, pNode, swFl, swSl);
}

/* Insert a given pNode into the free list. */
static VOID osMemNodeInsert(control_t *control, LOS_MEM_DYN_NODE *pNode)
{
    INT32 swFl, swSl;
    osMemMapInsert(osMemNodeSizeGet(pNode), &swFl, &swSl);
    osMemFreeNodeInsert(control, pNode, swFl, swSl);
}

static INT32 osMemNodeSpitCheck(LOS_MEM_DYN_NODE *pNode, UINT32 uwSize)
{
    return osMemNodeSizeGet(pNode) >= sizeof(LOS_MEM_DYN_NODE) + uwSize;
}

/* Split a pNode into two, the second of which is free. */
static LOS_MEM_DYN_NODE *osMemNodeSpit(LOS_MEM_DYN_NODE *pNode, UINT32 uwSize)
{
    /* Calculate the amount of space left in the remaining pNode. */
    LOS_MEM_DYN_NODE *remaining = osMemPtrOffset((VOID *)pNode, (ssize_t)uwSize);

    const UINT32 remain_size = osMemNodeSizeGet(pNode) - (uwSize);

    tlsf_assert(osMemNodeToPtr(remaining) == osMemAlignPtr(osMemNodeToPtr(remaining),
                                                     ALIGN_SIZE) &&
                "remaining pNode not aligned properly");

    tlsf_assert(osMemNodeSizeGet(pNode) ==
                remain_size + uwSize);
    osMemNodeSizeSet(remaining, remain_size);
    tlsf_assert(osMemNodeSizeGet(remaining) >= block_size_min &&
                "pNode split with invalid size");

    osMemNodeSizeSet(pNode, uwSize);
    osMemFreeSet(remaining);

    return remaining;
}

/* Absorb a free pNode's storage into an adjacent previous free pNode. */
static LOS_MEM_DYN_NODE *osMemNodeAbsorb(LOS_MEM_DYN_NODE *pPrev, LOS_MEM_DYN_NODE *pNode)
{
    tlsf_assert(!osMemEndCheck(pPrev) && "previous pNode can't be last");
    /* Note: Leaves flags untouched. */
    pPrev->uwSize += osMemNodeSizeGet(pNode);
    (VOID)osMemNextLink(pPrev);
    return pPrev;
}

/* Merge a just-freed pNode with an adjacent previous free pNode. */
static LOS_MEM_DYN_NODE *osMemPreNodeMerge(control_t *control,
                                        LOS_MEM_DYN_NODE *pNode)
{
    if (osMemPreFreeCheck(pNode)) {
        LOS_MEM_DYN_NODE *pPrev = osMemNodePre(pNode);
        tlsf_assert(pPrev && "prev physical pNode can't be null");
        tlsf_assert(osMemFreeCheck(pPrev) &&
                    "prev pNode is not free though marked as such");
        osMemNodeRemove(control, pPrev);
        pNode = osMemNodeAbsorb(pPrev, pNode);
    }

    return pNode;
}

/* Merge a just-freed pNode with an adjacent free pNode. */
static LOS_MEM_DYN_NODE *osMemNextNodeMerge(control_t *control,
                                        LOS_MEM_DYN_NODE *pNode)
{
    LOS_MEM_DYN_NODE *pNext = osMemNodeNext(pNode);
    tlsf_assert(pNext && "next physical pNode can't be null");

    if (osMemFreeCheck(pNext)) {
        tlsf_assert(!osMemEndCheck(pNode) && "previous pNode can't be last");
        osMemNodeRemove(control, pNext);
        pNode = osMemNodeAbsorb(pNode, pNext);
    }

    return pNode;
}

/* Trim any trailing pNode space off the end of a pNode, return to pool. */
static VOID osMemFreeNodeTrim(control_t *control,
                            LOS_MEM_DYN_NODE *pNode,
                            UINT32 uwSize)
{
    tlsf_assert(osMemFreeCheck(pNode) && "pNode must be free");
    if (osMemNodeSpitCheck(pNode, uwSize))
    {
        LOS_MEM_DYN_NODE *remaining_block = osMemNodeSpit(pNode, uwSize);
        (VOID)osMemNextLink(pNode);
        osMemPreFreeMark(remaining_block);
        osMemNodeInsert(control, remaining_block);
    }
}

/* Trim any trailing pNode space off the end of a used pNode, return to pool. */
static VOID osMemUsedNodeTrim(control_t *control,
                            LOS_MEM_DYN_NODE *pNode,
                            UINT32 uwSize)
{
    tlsf_assert(!osMemFreeCheck(pNode) && "pNode must be used");
    if (osMemNodeSpitCheck(pNode, uwSize)) {
        /* If the next pNode is free, we must coalesce. */
        LOS_MEM_DYN_NODE *remaining_block = osMemNodeSpit(pNode, uwSize);
        osMemPreUsedMark(remaining_block);
        remaining_block->pstPreNode = pNode;

        remaining_block = osMemNextNodeMerge(control, remaining_block);
        osMemNodeInsert(control, remaining_block);
    }
}

static LOS_MEM_DYN_NODE *osMemFreeNodeTrimLeading(control_t *control,
                                               LOS_MEM_DYN_NODE *pNode,
                                               UINT32 uwSize)
{
    LOS_MEM_DYN_NODE *remaining_block = pNode;
    if (osMemNodeSpitCheck(pNode, uwSize)) {
        /* We want the 2nd pNode. */
        remaining_block = osMemNodeSpit(pNode, uwSize);
        osMemPreFreeMark(remaining_block);

        (VOID)osMemNextLink(pNode);
        osMemNodeInsert(control, pNode);
    }

    return remaining_block;
}

static LOS_MEM_DYN_NODE *osMemFreeNodeLocate(control_t *control, UINT32 uwSize)
{
    INT32 swFl = 0, swSl = 0;
    LOS_MEM_DYN_NODE *pNode = NULL;

    if (uwSize) {
        osMemMapInsert(uwSize, &swFl, &swSl);
        pNode = osMemSuitableNodeSearch(control, &swFl, &swSl);
    }

    if (pNode) {
        if (osMemNodeSizeGet(pNode) >= uwSize)
            goto EXIT;
        while (pNode->pNext_free != &control->stBlock_null)
        {
            pNode = pNode->pNext_free;
            if (osMemNodeSizeGet(pNode) >= uwSize)
                goto EXIT;
        }
        osMemMapSearch(uwSize, &swFl, &swSl);
        pNode = osMemSuitableNodeSearch(control, &swFl, &swSl);
        if (pNode == NULL || osMemNodeSizeGet(pNode) < uwSize)
            return NULL;
EXIT:
        osMemFreeNodeRemove(control, pNode, swFl, swSl);
    }

    if (pNode && !pNode->uwSize)
        pNode = NULL;

    return pNode;
}

static VOID *osMemUsedNodePrepare(control_t *control, LOS_MEM_DYN_NODE *pNode, UINT32 uwSize)
{
    VOID *pPtr = NULL;
    if (pNode)
    {
        tlsf_assert(uwSize && "size must be non-zero");
        osMemFreeNodeTrim(control, pNode, uwSize);
        osMemUsedSet(pNode);
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
        control->uwPoolCurUsedSize += osMemNodeSizeGet(pNode);
        if (control->uwPoolCurUsedSize > control->uwPoolWaterLine)
        {
            control->uwPoolWaterLine = control->uwPoolCurUsedSize;
        }
#endif
        /* If the operation occured before task initialization(g_stLosTask.pstRunTask was not assigned)
           or in interrupt,make the value of taskid of pNode to oxffffffff*/
        if (g_stLosTask.pstRunTask != NULL && OS_INT_INACTIVE)
        {
            OS_MEM_TASKID_SET(pNode, g_stLosTask.pstRunTask->uwTaskID);
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
            if (control == (control_t *)OS_SYS_MEM_ADDR || control == (control_t *)m_aucSysMem0)
            {
                OS_MEM_ADD_USED(osMemNodeSizeGet(pNode), OS_MEM_TASKID_GET(pNode));
            }
#endif
        }
        else
        {
            /* If the task mode does not initialize, the field is the 0xffffffff */
            OS_MEM_TASKID_SET(pNode, OS_NULL_INT);
            /* TODO: the commend task-MEMUSE is not include system initialization malloc */
        }
        pPtr = osMemNodeToPtr(pNode);
    }
    else
    {
        PRINT_ERR("-----------------------------------------------------------------------------------------------------------\n");
        osMemInfoPrint((pool_t)control);
        PRINT_ERR("[%s] No suitable free block, require free node size: 0x%x\n", __FUNCTION__, uwSize);
        PRINT_ERR("-----------------------------------------------------------------------------------------------------------\n");
        return NULL;
    }
    return pPtr;
}

/* Clear structure and point all empty lists at the null pNode. */
static VOID osMemControlClear(control_t *control, UINT32 uwBytes)
{
    UINT32 uwFlIndex, uwSlIndex;

    control->stBlock_null.pNext_free = &control->stBlock_null;
    control->stBlock_null.pPrev_free = &control->stBlock_null;

    control->uwPoolSize = uwBytes;

    control->fl_bitmap = 0;
    for (uwFlIndex = 0; uwFlIndex < FL_INDEX_COUNT; ++uwFlIndex) {
        control->sl_bitmap[uwFlIndex] = 0;
        for (uwSlIndex = 0; uwSlIndex < SL_INDEX_COUNT; ++uwSlIndex) {
            control->pstBlocks[uwFlIndex][uwSlIndex] = &control->stBlock_null;
        }
    }
}

/*
 * Debugging utilities.
 */
#ifdef TSLF_CONFIG_DEBUG
INT32 osMemMagicCheck(pool_t pPool, VOID *pPtr);

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemTaskIdGet(VOID *pPtr)
{
    LOS_MEM_DYN_NODE *pNode;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)m_aucSysMem1;
#ifdef LOSCFG_KERNEL_MEM_SPEC
    VOID *pSpecPtr = pPtr;
    LOS_SPEC_BLOCK_NODE *pstSpecNode = NULL;
#endif

    if(pPtr == NULL || !IS_ALIGNED(pPtr, sizeof(VOID *)))
    {
        return OS_INVALID;
    }
#ifdef LOSCFG_EXC_INTERACTION
    if (pPtr < (VOID *)m_aucSysMem1)
    {
        pstPoolInfo = (LOS_MEM_POOL_INFO *)m_aucSysMem0;
    }
#endif
#ifdef LOS_MEM_SLAB
    LOS_SLAB_BLOCK_NODE *pstSlabNode = OS_SLAB_BLOCK_HEAD_GET(pPtr);

    if (pstSlabNode == NULL)
    {
        return OS_INVALID;
    }

    if (OS_ALLOC_FROM_SLAB_CHECK(pstSlabNode))
    {
        if(OS_NULL_BYTE != OS_SLAB_BLOCK_ID_GET(pstSlabNode))
        {
            return OS_SLAB_BLOCK_ID_GET(pstSlabNode);
        }
        else
        {
            return OS_INVALID;
        }
    }
#endif

#ifdef LOSCFG_KERNEL_MEM_SPEC
    pSpecPtr = osSpecGetOriginPtr((VOID *)pstPoolInfo, pSpecPtr);
    pstSpecNode = OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
    if (pstSpecNode == NULL)
    {
        return OS_INVALID;
    }

    if (OS_ALLOC_FROM_SPEC_CHECK(pstSpecNode))
    {
        if(OS_NULL_SHORT != OS_SPEC_BLOCK_ID_GET(pstSpecNode))
        {
            return OS_SPEC_BLOCK_ID_GET(pstSpecNode);
        }
        else
        {
            return OS_INVALID;
        }
    }
#endif

    if (osMemMagicCheck(pstPoolInfo, pPtr) == LOS_NOK)
    {
        return OS_INVALID;
    }

    pNode = osMemPtrToNode(pPtr);

    if (osMemFreeCheck(pNode))
    {
        PRINTK("pNode already marked as free\n");
        return OS_INVALID;
    }

    return (UINT32)OS_MEM_TASKID_GET(osMemPtrToNode(pPtr));
}

UINT32 LOS_MemTotalUsedGet(pool_t pPool)
{
    LOS_MEM_DYN_NODE *pNode = osMemFirstNode(pPool);
    UINT32 uwSize = 0;

    if (pPool == NULL)
        return LOS_NOK;

    while (pNode && !osMemEndCheck(pNode)) {
        if (!osMemFreeCheck(pNode))
        {
            uwSize += osMemNodeSizeGet(pNode); /*lint !e413*/

        }
        pNode = osMemNodeNext(pNode);
    }
    uwSize += block_start_offset; // for end node
    return uwSize;
}

UINT32 LOS_MemUsedBlksGet(pool_t pPool)
{
    LOS_MEM_DYN_NODE *pNode = osMemFirstNode(pPool);
    UINT32 uwNum = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
        return LOS_NOK;

    uvIntSave = LOS_IntLock();

    while (pNode && !osMemEndCheck(pNode)) {
        if (!osMemFreeCheck(pNode))
            uwNum ++;
        pNode = osMemNodeNext(pNode);
    }
    ++uwNum;

    LOS_IntRestore(uvIntSave);

    return uwNum;
}

UINT32 LOS_MemFreeBlksGet(pool_t pPool)
{
    LOS_MEM_DYN_NODE *pNode = osMemFirstNode(pPool);
    UINT32 uwNum = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
        return LOS_NOK;

    uvIntSave = LOS_IntLock();

    while (pNode && !osMemEndCheck(pNode)) {
        if (osMemFreeCheck(pNode))
            uwNum ++;
        pNode = osMemNodeNext(pNode);
    }

    LOS_IntRestore(uvIntSave);

    return uwNum;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemIntegrityCheck(pool_t pPool)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_DYN_NODE *pstPreNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_TASK_CB *pstTaskCB;
    UINT32 uwTaskID;
    UINTPTR uvIntSave;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();
    pstPreNode = osMemFirstNode(pPool);
    for (pstTmpNode = pstPreNode; pstTmpNode < osMemEndNode(pPool);
        pstTmpNode = osMemNodeNext(pstTmpNode))
    {
        if (LOS_NOK == osMemMagicCheck(pPool, osMemNodeToPtr(pstTmpNode)))
        {
            PRINT_ERR("[%s], %d, pool[%p] check error!broken node:%p pstPreNode:%p\n",
                __FUNCTION__, __LINE__, pPool, pstTmpNode, pstPreNode);
            goto errout;
        }
        pstPreNode = pstTmpNode;
    }
    LOS_IntRestore(uvIntSave);
    return LOS_OK;

errout:
    PRINTK("broken node head: %p    0x%x  %p    %p\n",
                    pstTmpNode->pstPreNode, pstTmpNode->uwSize, pstTmpNode->pNext_free, pstTmpNode->pPrev_free);
    LOS_IntRestore(uvIntSave);
    uwTaskID = (UINT32)OS_MEM_TASKID_GET(pstPreNode);
    if (uwTaskID >= g_uwTskMaxNum)
    {
        LOS_Panic("Task ID %d in pstPreNode is invalid!\n", uwTaskID);
        return LOS_NOK;
    }
    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

    if ((pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED) ||
        (pstTaskCB->pfnTaskEntry == NULL) ||
        (pstTaskCB->pcTaskName == NULL))
    {
        LOS_Panic("\r\nTask ID %d in pstPreNode is not created!\n", uwTaskID);
        return LOS_NOK;
    }
    LOS_Panic("cur node: %p\n"
               "pre node: %p\n"
               "pre node was allocated by task:%s\n",
               pstTmpNode, pstPreNode,  pstTaskCB->pcTaskName); /*lint !e515*/
    return LOS_NOK;
}

UINT32 LOS_MemPtrSize(VOID *pPtr)
{
    UINT32 uwSize = 0;
    if (pPtr) {
        const LOS_MEM_DYN_NODE *pNode = osMemPtrToNode(pPtr);
        uwSize = osMemNodeSizeGet(pNode);
    }
    return uwSize;
}

static UINT32 osMemNodeInsideCheck(pool_t pPool, VOID *pPtr)
{
    if (((AARCHPTR)pPtr < (AARCHPTR)osMemFirstNode(pPool))  ||
        ((AARCHPTR)pPtr > (AARCHPTR)osMemEndNode(pPool)))
        return LOS_NOK;
    else
        return LOS_OK;
}

INT32 osMemMagicCheck(pool_t pPool, VOID *pPtr)
{
    const LOS_MEM_DYN_NODE *pNode = NULL;

    if(pPtr == NULL || !IS_ALIGNED(pPtr, sizeof(VOID *)))
    {
        return LOS_NOK;
    }
    else
    {
        pNode = osMemPtrToNode(pPtr);
    }

    if (pNode == osMemFirstNode(pPool))
        return LOS_OK;
    else if ((AARCHPTR)pNode->pstPreNode & (ALIGN_SIZE - 1))
        return LOS_NOK;
    else if(osMemNodeInsideCheck(pPool, pNode->pstPreNode) == LOS_NOK)
        return LOS_NOK;
    else if ((AARCHPTR)pNode == (AARCHPTR)osMemNodeNext(pNode->pstPreNode))
        return LOS_OK;
    else
        return LOS_NOK;
}

UINT32 LOS_MemInfoGet(pool_t pPool, LOS_MEM_POOL_STATUS *pstStatus)
{
    LOS_MEM_DYN_NODE *pNode = osMemFirstNode(pPool);
    UINT32 uwTotalUsedSize = 0, uwTotalFreeSize = 0, uwMaxFreeNodeSize = 0;
    UINT32 uwUsedNodeNum = 0, uwFreeNodeNum = 0;
    UINT32 uwTmpSize = 0;
    UINTPTR uvIntSave;

    if (pstStatus == NULL)
    {
        PRINT_ERR("can't use NULL addr to save info\n");
        return LOS_NOK;
    }

    if (pPool == NULL)
    {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", pPool, __LINE__);
        return LOS_NOK;
    }

    if (osMemMagicCheck(pPool, (VOID *)osMemEndPtr(pPool)) == LOS_NOK)
    {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", pPool, __LINE__);
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    while (pNode && !osMemEndCheck(pNode)) {
        uwTmpSize = osMemNodeSizeGet(pNode);
        if (osMemFreeCheck(pNode))
        {
            ++uwFreeNodeNum;
            uwTotalFreeSize += uwTmpSize;
            if (uwTmpSize > uwMaxFreeNodeSize)
                uwMaxFreeNodeSize = uwTmpSize;
        }
        else
        {
            ++uwUsedNodeNum;
            uwTotalUsedSize += uwTmpSize;
        }
        pNode = osMemNodeNext(pNode);
    }

    pstStatus->uwTotalUsedSize = uwTotalUsedSize + block_start_offset;
    pstStatus->uwTotalFreeSize = uwTotalFreeSize;
    pstStatus->uwMaxFreeNodeSize = uwMaxFreeNodeSize;
    pstStatus->uwUsedNodeNum = uwUsedNodeNum + 1;
    pstStatus->uwFreeNodeNum = uwFreeNodeNum;
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pstStatus->uwUsageWaterLine = ((LOS_MEM_POOL_INFO *)pPool)->uwPoolWaterLine;
#endif

    LOS_IntRestore(uvIntSave);

    return LOS_OK;

}

VOID osMemInfoPrint(pool_t pPool)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_POOL_STATUS stStatus = {0};
    if (LOS_NOK == LOS_MemInfoGet(pPool, &stStatus))
        return;

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    PRINTK("pool addr    pool size    used size     free size    max free node size   used node num     free node num      UsageWaterLine\n"
                 "---------    --------     -------       --------     --------------       -------------      ------------      ------------\n"
                 "%-8p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x    0x%-13x\n",
                        pPool, pstPoolInfo->uwPoolSize, stStatus.uwTotalUsedSize, stStatus.uwTotalFreeSize,
                        stStatus.uwMaxFreeNodeSize, stStatus.uwUsedNodeNum, stStatus.uwFreeNodeNum, stStatus.uwUsageWaterLine);

#else
    PRINTK("pool addr    pool size    used size     free size    max free node size   used node num     free node num\n"
                 "---------    --------     -------       --------     --------------       -------------      ------------\n"
                 "%-8p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x\n",
                        pPool, pstPoolInfo->uwPoolSize, stStatus.uwTotalUsedSize, stStatus.uwTotalFreeSize,
                        stStatus.uwMaxFreeNodeSize, stStatus.uwUsedNodeNum, stStatus.uwFreeNodeNum);
#endif

    return;
}

UINT32 LOS_MemPoolSizeGet(VOID * tlsf)
{
    control_t *control = tlsf_cast(control_t *, tlsf);
    if (tlsf == NULL)
        return LOS_NOK;

    return control->uwPoolSize;
}

UINT32 LOS_MemFreeNodeShow(VOID *pPool)
{
    LOS_MEM_DYN_NODE *pNode = osMemFirstNode(pPool);
    UINT32 uwNest[FL_INDEX_MAX + 1] = {0};
    UINT32 uwTmp = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
        return LOS_NOK;

    if (osMemMagicCheck(pPool, (VOID *)osMemEndPtr(pPool)) == LOS_NOK)
    {
        PRINT_ERR("wrong mem pool addr: %p line:%d\n", pPool, __LINE__);
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    while (pNode && !osMemEndCheck(pNode))
    {
        if (osMemFreeCheck(pNode))
            uwNest[__log2(osMemNodeSizeGet(pNode))]++; /*lint !e413*/
        pNode = osMemNodeNext(pNode);
    }

    PRINTK("\n   ************************ left free node number**********************\n");
    for (uwTmp = FL_INDEX_SHIFT; uwTmp <= FL_INDEX_MAX; uwTmp += 5)
    {
        UINT32 uwI;
        PRINTK("block size:    ");
        for (uwI = uwTmp; uwI < uwTmp + 5 && uwI <= FL_INDEX_MAX; uwI++)
            PRINTK("2^%-2d   ", uwI);
        PRINTK("\nnode number:    ");
        for (uwI = uwTmp; uwI < uwTmp + 5 && uwI <= FL_INDEX_MAX; uwI++)
            PRINTK("%d      ", uwNest[uwI]);
        PRINTK("\n");
    }
    PRINTK("\n   ********************************************************************\n\n");

    LOS_IntRestore(uvIntSave);

    return LOS_OK;
}

#endif /* TLSF_CONFIG_DEBUG */

UINT32 LOS_MemHeadSize(VOID)
{
    return sizeof(control_t);
}

pool_t osMemPoolAdd(VOID *tlsf, VOID *pPool, UINT32 uwBytes)
{
    LOS_MEM_DYN_NODE *pNode;
    LOS_MEM_DYN_NODE *pNext;

    const UINT32 uwPoolBytes = osMemAlignDown(uwBytes - block_start_offset, ALIGN_SIZE) + block_header_overhead;

    if (((AARCHPTR)pPool % ALIGN_SIZE) != 0) {
        PRINT_ERR("%s: Memory must be aligned by %u bytes.\n",
                 __FUNCTION__, (INT32)ALIGN_SIZE);
        return NULL;
    }

    if (uwPoolBytes < block_size_min || uwPoolBytes > block_size_max) {
#if defined(__LP64__)
        PRINT_ERR("%s: Memory size must be between 0x%x and 0x%x00 "
                "bytes.\n",
                __FUNCTION__,
                (INT32)(block_start_offset + block_size_min),
                (INT32)((block_start_offset + block_size_max) / 256));
#else
        PRINT_ERR("%s: Memory size must be between %u and %u bytes.\n",
                __FUNCTION__,
               (INT32)(block_start_offset + block_size_min),
               (INT32)(block_start_offset + block_size_max));
#endif
        return NULL;
    }

    /*
     * Create the main free pNode. Offset the start of the pNode slightly
     * so that the pstPreNode field falls outside of the pool -
     * it will never be used.
    */
    pNode = osMemPtrOffset(pPool, -(ssize_t)block_header_overhead); /*lint !e570*/
    osMemNodeSizeSet(pNode, uwPoolBytes);
    osMemFreeMark(pNode);
    osMemPreUsedMark(pNode);
    osMemNodeInsert(tlsf_cast(control_t *, tlsf), pNode);
    /* Split the pNode to create a zero-size sentinel pNode. */
    pNext = osMemNextLink(pNode);
    osMemNodeSizeSet(pNext, 0);
    osMemUsedMark(pNext);
    osMemPreFreeMark(pNext);

    return pPool;
}

/*
 * TLSF main interface.
 */

VOID * osMemPoolCreat(VOID *pPool, UINT32 uwBytes)
{
    if (((AARCHPTR)pPool % ALIGN_SIZE) != 0) {
        PRINTK("%s: Memory must be aligned to %u bytes.\n",
                __FUNCTION__,
               (UINT32)ALIGN_SIZE);
        return NULL;
    }

    osMemControlClear(tlsf_cast(control_t *, pPool), uwBytes);

    return tlsf_cast(VOID *, pPool);
}

UINT32 LOS_MemInit(VOID *pPool, UINT32 uwBytes)
{
    VOID *tlsf;
#ifdef LOSCFG_MEM_MUL_POOL
    VOID *pNext = pool_head;
    VOID * pCur;
    UINT32 uwPoolEnd;
    UINTPTR uvIntSave;
#endif

    if (!pPool || uwBytes <= LOS_MemHeadSize())
        return OS_ERROR;
    if(!IS_ALIGNED(uwBytes, ALIGN_SIZE))
    {
        PRINT_WARN("pool [%p, %p) size 0x%x sholud be aligned with ALIGN_SIZE\n",
                  pPool, (AARCHPTR)pPool + uwBytes, uwBytes);
        uwBytes = osMemAlignDown(uwBytes, ALIGN_SIZE);
    }
#ifdef LOSCFG_MEM_MUL_POOL
    uvIntSave = LOS_IntLock();
    while (pNext != NULL)
    {
        uwPoolEnd = (AARCHPTR)pNext + LOS_MemPoolSizeGet(pNext);
        if ((pPool <= pNext && ((AARCHPTR)pPool + uwBytes) > (AARCHPTR)pNext) ||
            ((AARCHPTR)pPool < uwPoolEnd && ((AARCHPTR)pPool + uwBytes) >= uwPoolEnd))
        {
            LOS_IntRestore(uvIntSave);
            PRINT_ERR("pool [%p, %p) conflict with pool [%p, %p)\n",
                          pPool, (AARCHPTR)pPool + uwBytes,
                          pNext, (AARCHPTR)pNext + LOS_MemPoolSizeGet(pNext));
            return OS_ERROR;
        }
        pCur = pNext;
        pNext = ((LOS_MEM_POOL_INFO *)pNext)->pNextPool;
    }
    LOS_IntRestore(uvIntSave);
#endif

    tlsf = osMemPoolCreat(pPool, uwBytes);
    if (osMemPoolAdd(tlsf, (UINT8 *)pPool + LOS_MemHeadSize(), uwBytes - LOS_MemHeadSize()) == 0)
    {

        return OS_ERROR;
    }

#ifdef LOS_MEM_SLAB
    if (osSlabMemInit(pPool) == OS_ERROR)
    {
        return OS_ERROR;
    }
#endif
#ifdef LOSCFG_MEM_MUL_POOL
    uvIntSave = LOS_IntLock();
    if (pool_head == NULL)
    {
        pool_head = pPool;
    }
    else
    {
        ((LOS_MEM_POOL_INFO *)pCur)->pNextPool = pPool;
    }

    ((LOS_MEM_POOL_INFO *)pPool)->pNextPool = NULL;
    LOS_IntRestore(uvIntSave);
#endif

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
        ((control_t *)tlsf)->uwPoolCurUsedSize = sizeof(LOS_MEM_POOL_INFO) + block_start_offset;
        ((control_t *)tlsf)->uwPoolWaterLine = ((control_t *)tlsf)->uwPoolCurUsedSize;
#endif

    return LOS_OK;
}

#ifdef LOSCFG_MEM_MUL_POOL
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemDeInit(VOID *pPool)
{
    UINTPTR uvIntSave, uwRet = LOS_NOK;
    VOID *pNext, *pCur;

    uvIntSave = LOS_IntLock();
#ifdef LOS_MEM_SLAB
    osSlabMemDeInit(pPool);
#endif
    do
    {
        if (pPool == NULL)
            break;

        if (pPool == pool_head)
        {
            pool_head = ((LOS_MEM_POOL_INFO *)pool_head)->pNextPool;
            uwRet = LOS_OK;
            break;
        }

        pCur = pool_head;
        pNext = pool_head;

        while (pNext != NULL)
        {
            if (pPool == pNext)
            {
                ((LOS_MEM_POOL_INFO *)pCur)->pNextPool = ((LOS_MEM_POOL_INFO *)pNext)->pNextPool;
                uwRet = LOS_OK;
                break;
            }
            pCur = pNext;
            pNext = ((LOS_MEM_POOL_INFO *)pNext)->pNextPool;
        }
    }while(0);

    LOS_IntRestore(uvIntSave);
    return uwRet;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemPoolList(VOID)
{
    VOID *pNext = pool_head;
    UINT32 uwIndex = 0;
    while (pNext != NULL)
    {
        PRINTK("pool%d :\n", uwIndex++);
        osMemInfoPrint(pNext);
        pNext = ((LOS_MEM_POOL_INFO *)pNext)->pNextPool;
    }
    return uwIndex;
}
#endif

VOID *osHeapAlloc(VOID *tlsf, UINT32 uwSize)
{
    UINT32 adjust;
    LOS_MEM_DYN_NODE *pNode;
    VOID *pPtr = NULL;
    control_t *control = tlsf_cast(control_t *, tlsf);

    adjust = osMemReqSizeAdjust(uwSize, ALIGN_SIZE);
    if(0 == adjust)
    {
        PRINT_ERR("require node size 0x%x is too large\n",uwSize);
        return NULL;
    }
    pNode = osMemFreeNodeLocate(control, adjust);
    pPtr = osMemUsedNodePrepare(control, pNode, adjust);

    return pPtr;
}

VOID *LOS_MemAlloc(VOID * tlsf, UINT32 uwSize)
{
    VOID *pPtr = NULL;
    UINTPTR uvIntSave;
#ifdef LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK
    (VOID)LOS_MemIntegrityCheck(tlsf);
#endif

    if (uwSize == 0 || !tlsf || uwSize > block_size_max)
        return NULL;

    uvIntSave = LOS_IntLock();

#ifdef LOS_MEM_SLAB
    if (uwSize <= LOS_SLAB_HEAP_BOUNDARY)
    {
        pPtr = osSlabMemAlloc(tlsf, uwSize);
    }

    if (pPtr == NULL)
#endif
    {
#ifdef LOSCFG_KERNEL_MEM_SPEC
        pPtr = osSpecMemAlloc(tlsf, uwSize);
        if (pPtr == NULL)
#endif
        {
            pPtr = osHeapAlloc(tlsf, uwSize);
        }
    }
    LOS_IntRestore(uvIntSave);
    return pPtr;
}

VOID *LOS_MemAllocAlign(VOID * tlsf, UINT32 uwSize, UINT32 uwAlign)
{
    control_t *control = tlsf_cast(control_t *, tlsf);
    UINT32 adjust, gap_minimum, size_with_gap, aligned_size;
    LOS_MEM_DYN_NODE *pNode;
    VOID *pPtr;
    UINTPTR uvIntSave;

    if (uwSize == 0 || !tlsf || uwSize > block_size_max || uwAlign == 0 || !IS_POW_TWO(uwAlign) || !IS_ALIGNED(uwAlign, sizeof(VOID *)))
    {
        return NULL;
    }

    uvIntSave = LOS_IntLock();
#ifdef LOSCFG_KERNEL_MEM_SPEC
    pPtr = osSpecMemAllocAlign(tlsf, uwSize, uwAlign);
    if(NULL == pPtr)
#endif
    {
        uwSize += 4;
        adjust = osMemReqSizeAdjust(uwSize, ALIGN_SIZE);
        if(0 == adjust)
        {
            PRINT_ERR("require node size 0x%x is too large\n",uwSize);
            LOS_IntRestore(uvIntSave);
            return NULL;
        }
        /*
         * We must allocate an additional minimum pNode size bytes so that if
         * our free pNode will leave an alignment gap which is smaller, we can
         * trim a leading free pNode and release it back to the pool. We must
         * do this because the previous physical pNode is in use, therefore
         * the pstPreNode field is not valid, and we can't simply adjust
         * the size of that pNode.
         */
        gap_minimum = sizeof(LOS_MEM_DYN_NODE);
        if (((adjust + gap_minimum) > (((UINT32)-1) - uwAlign)) || ((uwAlign + gap_minimum) > (((UINT32)-1) - adjust)))
        {
            LOS_IntRestore(uvIntSave);
            return NULL;
        }
        size_with_gap = osMemReqSizeAdjust(adjust + uwAlign + gap_minimum, ALIGN_SIZE);
        if(0 == size_with_gap)
        {
            PRINT_ERR("require node size 0x%x is too large\n",uwSize);
            LOS_IntRestore(uvIntSave);
            return NULL;
        }
        /*
         * If alignment is less than or equals base alignment, we're done.
         * If we requested 0 bytes, return null, as tlsf_malloc(0) does.
         */
        aligned_size = (adjust && uwAlign > ALIGN_SIZE) ? size_with_gap : adjust;

        pNode = osMemFreeNodeLocate(control, aligned_size);

        if (pNode) {
            VOID *aligned;
            UINT32 gap;
            pPtr = osMemNodeToPtr(pNode);
            aligned = osMemAlignPtr(pPtr, uwAlign);
            gap = tlsf_cast(
                UINT32, tlsf_cast(AARCHPTR, aligned) - tlsf_cast(AARCHPTR, pPtr));

            /* If gap size is too small, offset to next aligned boundary. */
            if (gap && gap < gap_minimum) {
                const UINT32 gap_remain = gap_minimum - gap;
                const UINT32 offset = tlsf_max(gap_remain, uwAlign);
                const VOID *next_aligned =
                    tlsf_cast(VOID *, tlsf_cast(AARCHPTR, aligned) + offset);

                aligned = osMemAlignPtr(next_aligned, uwAlign);
                gap = tlsf_cast(UINT32, tlsf_cast(AARCHPTR, aligned) -
                                            tlsf_cast(AARCHPTR, pPtr));
            }

            if (gap) {
                tlsf_assert(gap >= gap_minimum && "gap size too small");
                pNode = osMemFreeNodeTrimLeading(control, pNode, gap);
            }
        }

        pPtr = osMemUsedNodePrepare(control, pNode, adjust);
    }
    LOS_IntRestore(uvIntSave);
    return pPtr;
}

UINT32 osHeapFree(VOID * tlsf, VOID *pPtr)
{
    control_t *control;
    LOS_MEM_DYN_NODE *pNode;
    if ((!pPtr)) return LOS_NOK;
    control = (LOS_MEM_POOL_INFO *)tlsf;
    pNode = osMemPtrToNode(pPtr);

    if (osMemFreeCheck(pNode))
    {
        return LOS_NOK;
    }

    if (osMemMagicCheck(tlsf, pPtr) == LOS_NOK)
    {
        return LOS_NOK;
    }

#ifdef OS_MEM_ENABLE_MEM_STATISTICS
    if (tlsf == (VOID *)OS_SYS_MEM_ADDR || tlsf == (VOID *)m_aucSysMem0)
    {
        OS_MEM_REDUCE_USED(osMemNodeSizeGet(pNode), OS_MEM_TASKID_GET(pNode));
    }
#endif

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    control->uwPoolCurUsedSize -= osMemNodeSizeGet(pNode);
#endif

    osMemFreeSet(pNode);
    pNode = osMemPreNodeMerge(control, pNode);
    pNode = osMemNextNodeMerge(control, pNode);
    osMemNodeInsert(control, pNode);

    return LOS_OK;
}
UINT32 LOS_MemFree(VOID * tlsf, VOID *pPtr)
{
    UINTPTR uvIntSave;
    UINT32 uwRet;


    if (!tlsf || (!pPtr)) return LOS_NOK;
    uvIntSave = LOS_IntLock();
#ifdef LOS_MEM_SLAB
    uwRet = osSlabMemFree(tlsf, pPtr);
    if (uwRet == LOS_NOK)
#endif
    {
#ifdef LOSCFG_KERNEL_MEM_SPEC
        uwRet = osSpecMemFree(tlsf, pPtr);
        if (uwRet == LOS_NOK)
#endif
        {
            uwRet = osHeapFree(tlsf,pPtr);
        }
    }
    LOS_IntRestore(uvIntSave);
    return uwRet;

}

/*
 * The TLSF pNode information provides us with enough information to
 * provide a reasonably intelligent implementation of realloc, growing or
 * shrinking the currently allocated pNode as required.
 *
 * This routine handles the somewhat esoteric edge cases of realloc:
 * - a non-zero size with a null pointer will behave like malloc
 * - a zero size with a non-null pointer will behave like free
 * - a request that cannot be satisfied will leave the original buffer
 *   untouched
 * - an extended buffer size will leave the newly-allocated area with
 *   contents undefined
 */
VOID *osHeapRealloc(VOID * tlsf, VOID *pPtr, UINT32 uwSize)
{
    VOID *pNewPtr = NULL;
    control_t *control = tlsf_cast(control_t *, tlsf);
    LOS_MEM_DYN_NODE *pNode = osMemPtrToNode(pPtr);
    LOS_MEM_DYN_NODE *pNext = osMemNodeNext(pNode);

    const size_t cursize = osMemNodeSizeGet(pNode);
    const size_t combined = cursize + osMemNodeSizeGet(pNext);
    const size_t adjust = osMemReqSizeAdjust(uwSize, ALIGN_SIZE);
    if(0 == adjust)
    {
        PRINT_ERR("require node size 0x%x is too large\n",uwSize);
        return NULL;
    }
    if (osMemFreeCheck(pNode))
    {
        PRINTK("block already marked as free\n");
        return NULL;
    }

    /*
     * If the next block is used, or when combined with the current
     * block, does not offer enough space, we must reallocate and copy.
    */
    if (adjust > cursize && (!osMemFreeCheck(pNext) || adjust > combined))
    {
        pNewPtr = LOS_MemAlloc(tlsf, uwSize);
        if (pNewPtr)
        {
            const size_t minsize = tlsf_min((cursize - block_start_offset), uwSize);
            memcpy(pNewPtr, pPtr, minsize);
            (VOID)LOS_MemFree(tlsf, pPtr);
        }
    } else
    {
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
        if (control == (control_t *)OS_SYS_MEM_ADDR || control == (control_t *)m_aucSysMem0)
        {
            OS_MEM_REDUCE_USED(osMemNodeSizeGet(pNode), OS_MEM_TASKID_GET(pNode));
        }
#endif
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
        control->uwPoolCurUsedSize -= osMemNodeSizeGet(pNode);
#endif

        /* Do we need to expand to the next block? */
        if (adjust > cursize)
        {
            (VOID)osMemNextNodeMerge(control, pNode);
            osMemUsedSet(pNode);
        }

        /* Trim the resulting block and return the original pointer. */
        osMemUsedNodeTrim(control, pNode, adjust);
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
        if (control == (control_t *)OS_SYS_MEM_ADDR || control == (control_t *)m_aucSysMem0)
        {
            OS_MEM_ADD_USED(osMemNodeSizeGet(pNode), OS_MEM_TASKID_GET(pNode));
        }
#endif
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
            control->uwPoolCurUsedSize += osMemNodeSizeGet(pNode);
            if (control->uwPoolCurUsedSize > control->uwPoolWaterLine)
            {
                control->uwPoolWaterLine = control->uwPoolCurUsedSize;
            }
#endif

        pNewPtr = pPtr;
    }

    return pNewPtr;
}

VOID *LOS_MemRealloc(VOID * tlsf, VOID *pPtr, UINT32 uwSize)
{
    VOID *pRetPtr = NULL;
    UINTPTR uvIntSave;

#ifdef LOSCFG_KERNEL_MEM_SPEC
    BOOL bIsFromSpec = FALSE;
    LOS_SPEC_BLOCK_NODE *pstSpecNode;
#endif
#ifdef LOS_MEM_SLAB
    LOS_SLAB_BLOCK_NODE *pstSlabnode;
    BOOL bIsFromSlab = FALSE;
#endif
#if defined LOSCFG_KERNEL_MEM_SPEC || defined LOS_MEM_SLAB
    VOID *pAlignedPtr = pPtr;
    UINT32 uwMinSize = 0;
    UINT32 uwOrigSize = 0;
#endif
    if ((INT32)uwSize < 0)
    {
        return NULL;
    }

    uvIntSave = LOS_IntLock();

    /* Zero-size requests are treated as free. */
    if (pPtr && uwSize == 0)
    {
        (VOID)LOS_MemFree(tlsf, pPtr);
    }
    /* Requests with NULL pointers are treated as malloc. */
    else if (pPtr == NULL)
    {
        pRetPtr = LOS_MemAlloc(tlsf, uwSize);
    }
    else
    {
#ifdef LOS_MEM_SLAB
        pstSlabnode = OS_SLAB_BLOCK_HEAD_GET(pPtr);
        bIsFromSlab = OS_ALLOC_FROM_SLAB_CHECK(pstSlabnode);
        if(bIsFromSlab)
        {
            uwOrigSize = OS_SLAB_BLOCK_SIZE_GET(pstSlabnode);
        }
        else
#endif
        {
#ifdef LOSCFG_KERNEL_MEM_SPEC
            pAlignedPtr = osSpecGetOriginPtr(tlsf, pAlignedPtr);
            pstSpecNode = OS_SPEC_BLOCK_HEAD_GET(pAlignedPtr);
            bIsFromSpec = OS_ALLOC_FROM_SPEC_CHECK(pstSpecNode);
            if(bIsFromSpec)
            {
                uwOrigSize = OS_SPEC_BLOCK_SIZE_GET(pstSpecNode);
            }
            else
#endif
            {
#if defined LOSCFG_KERNEL_MEM_SPEC || defined LOS_MEM_SLAB
                uwOrigSize = osMemNodeSizeGet(osMemPtrToNode(pPtr))- block_start_offset;
#endif
            }
        }
#ifdef LOS_MEM_SLAB
        if (bIsFromSlab || (uwSize <= LOS_SLAB_HEAP_BOUNDARY))
        {
            /* if orig ptr is allocated from SLAB && realloc new size <= LOS_SLAB_HEAP_BOUNDARY */
            if (bIsFromSlab && (uwSize <= LOS_SLAB_HEAP_BOUNDARY))
            {
                if (OS_SLAB_CLASS_LEVEL_GET(uwOrigSize) == OS_SLAB_CLASS_LEVEL_GET(uwSize))
                {
                    LOS_IntRestore(uvIntSave);
                    return pPtr;
                }
            }

            pRetPtr = LOS_MemAlloc(tlsf, uwSize);
            if (pRetPtr)
            {
                uwMinSize = uwOrigSize < uwSize ? uwOrigSize : uwSize;
                memcpy(pRetPtr, pPtr, uwMinSize);
                (VOID)LOS_MemFree(tlsf, pPtr);
            }
        }
        else
#endif
        {
#ifdef LOSCFG_KERNEL_MEM_SPEC
            if(bIsFromSpec || osClassLevelGet(uwSize) != -1)
            {
                if(bIsFromSpec && osClassLevelGet(uwSize) != -1)
                {
                    if(osClassLevelGet(uwOrigSize) == osClassLevelGet(uwSize))
                    {
                        LOS_IntRestore(uvIntSave);
                        return pPtr;
                    }
                }
                pRetPtr = LOS_MemAlloc(tlsf, uwSize);

                if (pRetPtr)
                {
                    uwMinSize = uwOrigSize < uwSize ? uwOrigSize : uwSize;
                    memcpy(pRetPtr, pPtr, uwMinSize);
                    (VOID)LOS_MemFree(tlsf, pPtr);

                }
            }
            else
#endif
            /* if orig ptr is allocated from TLSF && realloc new size > LOS_SLAB_HEAP_BOUNDARY */
            {
                pRetPtr = osHeapRealloc(tlsf, pPtr, uwSize);
            }
        }
    }
    LOS_IntRestore(uvIntSave);
    return pRetPtr;
}

/**********************************************************************/
UINT8 *m_aucSysMem0 = (UINT8 *)NULL;
UINT8 *m_aucSysMem1 = (UINT8 *)NULL;

UINT8 *m_aucSysNoCacheMem0 = (UINT8 *)NULL;
__attribute__((section(".data.init"))) AARCHPTR g_sys_mem_addr_end;
__attribute__((section(".data.init"))) AARCHPTR g_exc_interact_mem_size = 0;
#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
static UINT8 ucCheckMemLevel = (UINT8)LOS_MEM_CHECK_LEVEL_DEFAULT;
#endif
MALLOC_HOOK g_MALLOC_HOOK = (MALLOC_HOOK)NULL; /*lint !e611*/

#ifdef LOSCFG_EXC_INTERACTION
LITE_OS_SEC_TEXT_INIT UINT32 osMemExcInteractionInit(AARCHPTR uwMemStart)
{
    UINT32 uwRet;
    UINT32 uwMemSize;
    m_aucSysMem0 = (UINT8 *)((uwMemStart + (64 - 1)) & ~((AARCHPTR)(64 - 1)));
    uwMemSize = OS_EXC_INTERACTMEM_SIZE;
    uwRet = LOS_MemInit(m_aucSysMem0, uwMemSize);
    PRINT_INFO("LiteOS kernel exc interaction memory address:%p,size:0x%x\n", m_aucSysMem0, uwMemSize);
    return uwRet;
}
#endif

LITE_OS_SEC_TEXT_INIT UINT32 osMemSystemInit(AARCHPTR uwMemStart)
{
    UINT32 uwRet;
    UINT32 uwMemSize;
#ifdef LOS_MEM_SLAB
    struct LOS_SLAB_ALLOCATOR_CFG_INFO stCfg;
#endif

    m_aucSysMem1 = (UINT8 *)((uwMemStart + (64 - 1)) & ~((AARCHPTR)(64 - 1)));
    uwMemSize = OS_SYS_MEM_SIZE;

#ifdef LOS_MEM_SLAB
    LOS_SlabCfgStructInit(&stCfg);
    LOS_SlabAllocatorCfgRegister(m_aucSysMem1, &stCfg);
#endif

    uwRet = LOS_MemInit(m_aucSysMem1, uwMemSize);
    PRINT_INFO("LiteOS system heap memory address:%p,size:0x%x\n",m_aucSysMem1, uwMemSize);
#ifndef LOSCFG_EXC_INTERACTION
    m_aucSysMem0 = m_aucSysMem1;
#endif
    return uwRet;
}

#if OS_SYS_NOCACHEMEM_SIZE
UINT32 osNocacheMemSystemInit(VOID)
{
    UINT32 uwRet;
#ifdef LOS_MEM_SLAB
    struct LOS_SLAB_ALLOCATOR_CFG_INFO stCfg;
#endif
    m_aucSysNoCacheMem0 = (g_sys_mem_addr_end - OS_SYS_NOCACHEMEM_SIZE);
#ifdef LOS_MEM_SLAB
    LOS_SlabCfgStructInit(&stCfg);
    LOS_SlabAllocatorCfgRegister(m_aucSysNoCacheMem0, &stCfg);
#endif

    uwRet = LOS_MemInit(m_aucSysNoCacheMem0, OS_SYS_NOCACHEMEM_SIZE);
    return uwRet;
}
#endif

VOID *osMemFindNodeCtrl(VOID* pPool,  VOID *pPtr);

LITE_OS_SEC_TEXT_MINOR AARCHPTR LOS_MemLastUsedGet(VOID * tlsf)
{
    control_t *control = tlsf_cast(control_t *, tlsf);
    LOS_MEM_DYN_NODE *pNode;

    if (tlsf == NULL)
    {
        return LOS_NOK;
    }

    pNode =(LOS_MEM_DYN_NODE *)((AARCHPTR)control + control->uwPoolSize - block_start_offset);

    if (osMemFreeCheck(pNode->pstPreNode))
    {
        return (AARCHPTR)(pNode->pstPreNode) + block_start_offset;
    }
    else
    {
        return (AARCHPTR)pNode + block_start_offset;
    }
}

/*****************************************************************************
 Function : osMemResetEndNode
 Description : reset "end node"
 Input       : pPool    --- Pointer to memory pool
 Input       : uwPreAddr    --- Pointer to the pre Pointer of end node
 Output      : endNode -- pointer to "end node"
 Return      : the number of free node
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID osMemResetEndNode(VOID *pPool, AARCHPTR uwPreAddr)
{
    LOS_MEM_DYN_NODE *pstEndNode = osMemEndNode(pPool);

    if (uwPreAddr != (AARCHPTR)NULL)
        pstEndNode->pstPreNode = (LOS_MEM_DYN_NODE *)(uwPreAddr - sizeof(LOS_MEM_DYN_NODE));
    pstEndNode->uwSize = 0;
}

#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK

/*****************************************************************************
  Function: LOS_MemNodeSizeCheck
  Description: get a pNode's(pPtr) size ,include total size and available size
  Input        :pPool --which pPool doesn't your pPtr belong to
                   pPtr --point to source node
  Output      :puwTotalSize -- save total size
                   puwAvailSize -- save availabe size
  Return : errorID or LOS_OK
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemNodeSizeCheck(VOID *pPool, VOID *pPtr, UINT32 *puwTotalSize, UINT32 *puwAvailSize)
{
    VOID *pHead = NULL;

    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_DISABLE)
    {
        return LOS_ERRNO_MEMCHECK_DISABLED;
    }

    if ((pPool == NULL) || (pPtr == NULL))
    {
        return LOS_ERRNO_MEMCHECK_PARA_NULL;
    }

    if ( osMemNodeInsideCheck(pPool, pPtr) == LOS_NOK)
    {
        return LOS_ERRNO_MEMCHECK_OUTSIDE;
    }

    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_HIGH)
    {
        pHead = osMemFindNodeCtrl(pPool, pPtr);
        if ((pHead == NULL) || (osMemNodeSizeGet(osMemPtrToNode(pHead)) - block_start_offset  < ((AARCHPTR)pPtr - (AARCHPTR)pHead)))
        {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        *puwTotalSize = osMemNodeSizeGet(osMemPtrToNode(pHead)) - block_start_offset;
        *puwAvailSize = osMemNodeSizeGet(osMemPtrToNode(pHead)) - block_start_offset - ((AARCHPTR)pPtr - (AARCHPTR)pHead);
        return LOS_OK;
    }
    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_LOW)
    {
        if ((AARCHPTR)pPtr & (ALIGN_SIZE - 1))
        {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        pHead = pPtr;
        if (osMemMagicCheck(pPool, pHead) == LOS_OK)
        {
            *puwTotalSize = osMemNodeSizeGet(osMemPtrToNode(pHead)) - block_start_offset;
            *puwAvailSize = osMemNodeSizeGet(osMemPtrToNode(pHead)) - block_start_offset;
            return LOS_OK;
        }
        else
        {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
    }

    return LOS_ERRNO_MEMCHECK_WRONG_LEVEL;
}

/*****************************************************************************
Function     : osMemFindNodeCtrl
Description : get a pool's memCtrl
Input         :pPtr -- point to source pPtr
Output       :None
Return        : search forward for pPtr's memCtrl or "NULL"
@attention : this func couldn't ensure the return memCtrl belongs to pPtr
it just find forward the most nearly one
*******************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID *osMemFindNodeCtrl(VOID* pPool, VOID *pPtr)
{
    UINT8 *pHead = (UINT8 *)pPtr;
    if( pPtr == NULL || pPool == NULL)
    {
        return NULL;
    }

    if (osMemNodeInsideCheck(pPool, pPtr) == LOS_NOK)
        return NULL;

    pHead = (UINT8 *)osMemAlignUp((AARCHPTR)pHead, ALIGN_SIZE);

    while (osMemMagicCheck(pPool, pHead) == LOS_NOK)
    {

        pHead -= ALIGN_SIZE;
    }
    return pHead;
}

/*****************************************************************************
 Function : LOS_MemCheckLevelSet
 Description : setting ucCheckMemLevel which decide the manner of memcheck
 Input       : ucLevel -- waht level want to set
 Output      : None
 Return      : LOS_OK -- setting succeed
                  OS_ERRNO_MEMCHECK_WRONG_LEVEL -- setting failed due to illegal parameter
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemCheckLevelSet(UINT8 ucLevel)
{
    if ( ucLevel == LOS_MEM_CHECK_LEVEL_LOW)
    {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_LOW \n", __FUNCTION__);
    }
    else if(ucLevel == LOS_MEM_CHECK_LEVEL_HIGH)
    {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_HIGH \n", __FUNCTION__);
    }
    else if (ucLevel == LOS_MEM_CHECK_LEVEL_DISABLE)
    {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_DISABLE \n", __FUNCTION__);
    }
    else
    {
        PRINTK("%s: wrong para, setting failed !! \n", __FUNCTION__);
        return LOS_ERRNO_MEMCHECK_WRONG_LEVEL;
    }
    ucCheckMemLevel = ucLevel;
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT8 LOS_MemCheckLevelGet(VOID)
{
    return ucCheckMemLevel;
}

#endif /* LOSCFG_BASE_MEM_NODE_SIZE_CHECK */

UINT32 osMemSysNodeCheck(VOID *pDst, VOID *pSrc, UINT32 uwLength, UINT8 ucPos)
{
#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
    UINT32 uwRet = 0;
    UINT32 uwTotalSize=0;
    UINT32 uwAvailSize=0;
    UINT8 *pPool = m_aucSysMem1;
#ifdef LOSCFG_EXC_INTERACTION
    if((AARCHPTR)pDst < (AARCHPTR)m_aucSysMem0 + g_exc_interact_mem_size)
    {
        pPool = m_aucSysMem0;
    }
#endif

    if (ucPos == 0) /* if this func was called by memset */
    {
        uwRet = LOS_MemNodeSizeCheck(pPool, pDst, &uwTotalSize, &uwAvailSize);
        if (uwRet == LOS_OK && uwLength > uwAvailSize)
        {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memset: dst inode uwAvailSize is not enough"
                                " uwAvailSize = 0x%x uwLength = 0x%x\n", uwAvailSize, uwLength);
            osBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return LOS_NOK;
        }
    }
    else if (ucPos == 1) /* if this func was called by memcpy */
    {
        uwRet = LOS_MemNodeSizeCheck(pPool, pDst, &uwTotalSize, &uwAvailSize);
        if (uwRet == LOS_OK && uwLength > uwAvailSize)
        {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memcpy: dst inode uwAvailSize is not enough"
                               " uwAvailSize = 0x%x uwLength = 0x%x\n", uwAvailSize, uwLength);
            osBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return LOS_NOK;
        }
#ifdef LOSCFG_EXC_INTERACTION
        if((AARCHPTR)pSrc < (AARCHPTR)m_aucSysMem0 + g_exc_interact_mem_size)
        {
            pPool = m_aucSysMem0;
        }
        else
        {
            pPool = m_aucSysMem1;
        }
#endif
        uwRet = LOS_MemNodeSizeCheck(pPool, pSrc, &uwTotalSize, &uwAvailSize);
        if (uwRet == LOS_OK && uwLength > uwAvailSize)
        {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memcpy: src inode uwAvailSize is not enough"
                               " uwAvailSize = 0x%x uwLength = 0x%x\n", uwAvailSize, uwLength);
            osBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return LOS_NOK;
        }
    }
#endif
    return LOS_OK;
}

#ifdef LOS_MEM_SLAB
VOID *osSlabCtrlHdrGet(VOID *pHeapAddr)
{
    return (&(tlsf_cast(control_t *, pHeapAddr)->stSlabCtrlHdr));
}
#endif

#ifdef LOSCFG_KERNEL_MEM_SPEC
VOID *osSpecCtrlHdrGet(VOID *pHeapAddr)
{
    if(pHeapAddr != (VOID *)OS_SYS_MEM_ADDR)
    {
        return NULL;
    }
    return (&(tlsf_cast(control_t *, pHeapAddr)->stSpecCtrlHdr));
}
#endif

#ifdef LOSCFG_MEM_MUL_MODULE
UINT32 g_uwMemInfo[MEM_MODULE_MAX + 1] = {0};

STATIC INLINE UINT32 osMemModCheck(UINT32 uwModule)
{
    if (uwModule > MEM_MODULE_MAX)
    {
        PRINT_ERR("error module ID input!\n");
        return LOS_NOK;
    }
    return LOS_OK;
}

VOID *LOS_MemMalloc(VOID *pPool, UINT32 uwSize, UINT32 uwModule)
{
    UINTPTR uvIntSave;
    VOID *pPtr, *pNode;
    if (osMemModCheck(uwModule) == LOS_NOK)
        return NULL;

    pPtr = LOS_MemAlloc(pPool, uwSize);
    if (pPtr != NULL)
    {
        uvIntSave = LOS_IntLock();
#ifdef LOS_MEM_SLAB
        pNode = (VOID *)OS_SLAB_BLOCK_HEAD_GET(pPtr);
        if (OS_ALLOC_FROM_SLAB_CHECK(pNode))
        {
            g_uwSlabmemInfo[uwModule] += (OS_SLAB_BLOCK_SIZE_GET(pNode) + sizeof(LOS_SLAB_BLOCK_NODE));
            OS_SLAB_MODID_SET(pNode, uwModule);
        }
        else
#endif
        {
#ifdef LOSCFG_KERNEL_MEM_SPEC
            pNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pPtr);
            if (OS_ALLOC_FROM_SPEC_CHECK(pNode))
            {
                g_uwMemInfo[uwModule] += (OS_SPEC_BLOCK_SIZE_GET(pNode) + sizeof(LOS_SPEC_BLOCK_NODE));
                OS_SPEC_MODID_SET(pNode, uwModule);
            }
            else
#endif
            {
                pNode = (VOID *)osMemPtrToNode(pPtr);
                g_uwMemInfo[uwModule] += osMemNodeSizeGet((LOS_MEM_DYN_NODE *)pNode);
                OS_MEM_MODID_SET(pNode, uwModule); /*lint !e613*/
            }
        }
        LOS_IntRestore(uvIntSave);

    }
    return pPtr;
}

VOID *LOS_MemMallocAlign(VOID *pPool, UINT32 uwSize, UINT32 uwBoundary, UINT32 uwModule)
{
    UINTPTR uvIntSave;
    VOID *pPtr, *pNode;
#ifdef LOSCFG_KERNEL_MEM_SPEC
    VOID *pSpecPtr = NULL;
#endif
    if (osMemModCheck(uwModule) == LOS_NOK)
        return NULL;
    pPtr = LOS_MemAllocAlign(pPool, uwSize, uwBoundary);
    if (pPtr != NULL)
    {
        uvIntSave = LOS_IntLock();
#ifdef LOSCFG_KERNEL_MEM_SPEC
        pSpecPtr = pPtr;
        pSpecPtr = osSpecGetOriginPtr(pPool, pSpecPtr);
        pNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
        if (OS_ALLOC_FROM_SPEC_CHECK(pNode))
        {
            g_uwMemInfo[uwModule] += (OS_SPEC_BLOCK_SIZE_GET(pNode) + sizeof(LOS_SPEC_BLOCK_NODE));
            OS_SPEC_MODID_SET(pNode, uwModule);
        }
        else
#endif
        {
            pNode = osMemPtrToNode(pPtr);
            g_uwMemInfo[uwModule] += osMemNodeSizeGet(pNode);
            OS_MEM_MODID_SET(pNode, uwModule); /*lint !e613*/
        }
        LOS_IntRestore(uvIntSave);
    }
    return pPtr;
}

UINT32 LOS_MemMfree(VOID *pPool, VOID *pPtr, UINT32 uwModule)
{
    UINTPTR uvIntSave;
    UINT32 uwRet = LOS_NOK;
    UINT32 uwSize;
    VOID *pstNode;
#ifdef LOSCFG_KERNEL_MEM_SPEC
    VOID *pSpecPtr = pPtr;
#endif
    if (osMemModCheck(uwModule) == LOS_NOK || pPtr == NULL)
        return LOS_NOK;
#ifdef LOS_MEM_SLAB
    pstNode = (VOID *)OS_SLAB_BLOCK_HEAD_GET(pPtr);
    if (pstNode == NULL)
    {
        return LOS_NOK;
    }
    if (OS_ALLOC_FROM_SLAB_CHECK(pstNode))
    {
        if (uwModule != OS_SLAB_MODID_GET(pstNode))
        {
            PRINT_ERR("node[%p] alloced in module %d, but free in module %d\n node's taskID: 0x%x\n",
                                pPtr, OS_SLAB_MODID_GET(pstNode), uwModule, OS_SLAB_BLOCK_ID_GET(pstNode));
            uwModule = OS_SLAB_MODID_GET(pstNode);
        }
        uwSize = OS_SLAB_BLOCK_SIZE_GET(pstNode);

        uwRet = osSlabMemFree(pPool, pPtr);
        if (uwRet == LOS_OK)
        {
            uvIntSave = LOS_IntLock();
            g_uwSlabmemInfo[uwModule] -= (uwSize + sizeof(LOS_SLAB_BLOCK_NODE));
            LOS_IntRestore(uvIntSave);
        }
    }
    else
#endif
    {
#ifdef LOSCFG_KERNEL_MEM_SPEC
        pSpecPtr = osSpecGetOriginPtr(pPool, pSpecPtr);
        pstNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
        if (pstNode == NULL)
        {
            return LOS_NOK;
        }

        if (OS_ALLOC_FROM_SPEC_CHECK(pstNode))
        {
            if (uwModule != OS_SPEC_MODID_GET(pstNode))
            {
                PRINT_ERR("node[%p] alloced in module %d, but free in module %d\n node's taskID: 0x%x\n",
                                    pPtr, OS_SPEC_MODID_GET(pstNode), uwModule, OS_SPEC_BLOCK_ID_GET(pstNode));
                uwModule = OS_SPEC_MODID_GET(pstNode);
            }
            uwSize = OS_SPEC_BLOCK_SIZE_GET(pstNode);
            uwRet = osSpecMemFree(pPool, pPtr);
            if (uwRet == LOS_OK)
            {
                uvIntSave = LOS_IntLock();
                g_uwMemInfo[uwModule] -= (uwSize + sizeof(LOS_SPEC_BLOCK_NODE));
                LOS_IntRestore(uvIntSave);
            }
        }
        else
#endif
        {
            pstNode = (VOID *)osMemPtrToNode(pPtr);
            if (pstNode == NULL)
            {
                return LOS_NOK;
            }

            uwSize = osMemNodeSizeGet(pstNode);

            if (uwModule != OS_MEM_MODID_GET(pstNode))
            {
                PRINT_ERR("node[%p] alloced in module %d, but free in module %d\n node's taskID: 0x%x\n",
                                    pPtr, OS_MEM_MODID_GET(pstNode), uwModule, OS_MEM_TASKID_GET(pstNode));
                uwModule = OS_MEM_MODID_GET(pstNode);
            }
            uwRet = osHeapFree(pPool, pPtr);
            if (uwRet == LOS_OK)
            {
                uvIntSave = LOS_IntLock();
                g_uwMemInfo[uwModule] -= uwSize;
                LOS_IntRestore(uvIntSave);
            }
        }
    }
    return uwRet;
}

VOID *LOS_MemMrealloc(VOID *pPool, VOID *pPtr, UINT32 uwSize, UINT32 uwModule)
{
    VOID *pNewPtr;
    UINT32 uwOldSize;
    UINTPTR uvIntSave;
    LOS_MEM_DYN_NODE * pstNode;
#ifdef LOSCFG_KERNEL_MEM_SPEC
    VOID *pSpecPtr = pPtr;
#endif
    UINT32 uwOldModule = uwModule;
    if (osMemModCheck(uwModule) == LOS_NOK)
        return NULL;

    if (pPtr == NULL)
    {
        return LOS_MemMalloc(pPool, uwSize, uwModule);
    }
#ifdef LOS_MEM_SLAB
    pstNode = (VOID *)OS_SLAB_BLOCK_HEAD_GET(pPtr);
    if (pstNode == NULL)
    {
        return NULL;
    }
    if (OS_ALLOC_FROM_SLAB_CHECK(pstNode))
    {

        if (uwModule != OS_SLAB_MODID_GET(pstNode))
        {
            PRINT_ERR("node[%p] alloced in module %d, but realloc in module %d\n node's taskID: 0x%x\n",
                                pPtr, OS_SLAB_MODID_GET(pstNode), uwModule, OS_SLAB_BLOCK_ID_GET(pstNode));
            uwOldModule = OS_SLAB_MODID_GET(pstNode);
        }
        uwOldSize = OS_SLAB_BLOCK_SIZE_GET(pstNode);

    }
    else
#endif
    {
#ifdef LOSCFG_KERNEL_MEM_SPEC
        pSpecPtr = osSpecGetOriginPtr(pPool, pSpecPtr);
        pstNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
        if (pstNode == NULL)
        {
            return NULL;
        }
        if (OS_ALLOC_FROM_SPEC_CHECK(pstNode))
        {

            if (uwModule != OS_SPEC_MODID_GET(pstNode))
            {
                PRINT_ERR("node[%p] alloced in module %d, but realloc in module %d\n node's taskID: 0x%x\n",
                                    pPtr, OS_SPEC_MODID_GET(pstNode), uwModule, OS_SPEC_BLOCK_ID_GET(pstNode));
                uwOldModule = OS_SPEC_MODID_GET(pstNode);
            }
            uwOldSize = OS_SPEC_BLOCK_SIZE_GET(pstNode);

        }
        else
#endif
        {
            pstNode = (VOID *)osMemPtrToNode(pPtr);
            if (pstNode == NULL)
            {
                return NULL;
            }

            if (uwModule != OS_MEM_MODID_GET(pstNode))
            {
                PRINT_ERR("a node[%p] alloced in module %d, but realloc in module %d\n node's taskID: %d\n",
                                    pPtr, OS_MEM_MODID_GET(pstNode), uwModule, OS_MEM_TASKID_GET(pstNode));
                uwOldModule = OS_MEM_MODID_GET(pstNode);
            }
            uwOldSize = osMemNodeSizeGet(pstNode);
        }
    }
    if (uwSize == 0)
    {
        (VOID)LOS_MemMfree(pPool, pPtr, uwOldModule);
        return NULL;
    }

    pNewPtr = LOS_MemRealloc(pPool, pPtr, uwSize);
    if (pNewPtr != NULL)
    {
        uvIntSave = LOS_IntLock();
#ifdef LOS_MEM_SLAB
        pstNode = (VOID *)OS_SLAB_BLOCK_HEAD_GET(pNewPtr);
        if (OS_ALLOC_FROM_SLAB_CHECK(pstNode))
        {
            g_uwSlabmemInfo[uwModule] += (OS_SLAB_BLOCK_SIZE_GET(pstNode) + sizeof(LOS_SLAB_BLOCK_NODE));
            OS_SLAB_MODID_SET(pstNode, uwModule);
        }
        else
#endif
        {
#ifdef LOSCFG_KERNEL_MEM_SPEC
            pSpecPtr = pNewPtr;
            pSpecPtr = osSpecGetOriginPtr(pPool, pSpecPtr);
            pstNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
            if (OS_ALLOC_FROM_SPEC_CHECK(pstNode))
            {
                g_uwMemInfo[uwModule] += ((OS_SPEC_BLOCK_SIZE_GET(pstNode) +sizeof(LOS_SPEC_BLOCK_NODE)));
                OS_SPEC_MODID_SET(pstNode, uwModule);
            }
            else
#endif
            {
                pstNode = (LOS_MEM_DYN_NODE *)osMemPtrToNode(pNewPtr);
                g_uwMemInfo[uwModule] += osMemNodeSizeGet(pstNode);
                OS_MEM_MODID_SET(pstNode, uwModule);
            }
        }
#ifdef LOS_MEM_SLAB
        pstNode = (VOID *)OS_SLAB_BLOCK_HEAD_GET(pPtr);
        if (OS_ALLOC_FROM_SLAB_CHECK(pstNode))
        {
            g_uwSlabmemInfo[uwOldModule] -= (uwOldSize + sizeof(LOS_SLAB_BLOCK_NODE));
        }
        else
#endif
        {

#ifdef LOSCFG_KERNEL_MEM_SPEC
            pSpecPtr = pPtr;
            pSpecPtr = osSpecGetOriginPtr(pPool, pSpecPtr);
            pstNode = (VOID *)OS_SPEC_BLOCK_HEAD_GET(pSpecPtr);
            if (OS_ALLOC_FROM_SPEC_CHECK(pstNode))
            {
                g_uwMemInfo[uwOldModule] -= (uwOldSize +sizeof(LOS_SPEC_BLOCK_NODE));
            }
            else
#endif
            {
                g_uwMemInfo[uwOldModule] -= uwOldSize;
            }

        }
        LOS_IntRestore(uvIntSave);
    }
    return pNewPtr;
}

UINT32 LOS_MemMusedGet(UINT32 uwModule)
{
    UINT32 uwMemMusedSize = 0;
    if (osMemModCheck(uwModule) == LOS_NOK)
        return OS_NULL_INT;
#ifdef LOS_MEM_SLAB
    uwMemMusedSize = g_uwMemInfo[uwModule] + g_uwSlabmemInfo[uwModule];
#else
    uwMemMusedSize = g_uwMemInfo[uwModule];
#endif
    return uwMemMusedSize;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

