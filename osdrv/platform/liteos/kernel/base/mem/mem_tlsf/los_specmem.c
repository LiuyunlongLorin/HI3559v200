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
#include "los_task.ph"
#include "los_memstat.ph"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif
#ifdef LOSCFG_KERNEL_MEM_SPEC
#include "los_specmem.ph"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_MEM_SPEC

extern VOID *osSpecCtrlHdrGet(VOID *pHeapAddr);
#define OS_SPEC_CTRL_HEADER_GET(pHeapAddr)  (osSpecCtrlHdrGet(pHeapAddr))

struct LOS_SPEC_CONTROL_HEADER g_stAllocatorCfg;

UINT32 g_uwSpecMemClassCnt = 0;
UINT32 g_uwSpecMemClassMaxCnt = LOS_SPEC_CLASS_CNT;

/************************************************************************************************/
INT32 osClassLevelGet(UINT32 allocSize)
{
    UINT32 uwLevel = 0;
    UINTPTR uvIntSave = LOS_IntLock();

    while(uwLevel < g_uwSpecMemClassCnt)
    {
        if (allocSize > g_stAllocatorCfg.stSpecMemClass[uwLevel].uwBlkSzMin && allocSize <= g_stAllocatorCfg.stSpecMemClass[uwLevel].uwBlkSzMax)
        {
             break; /* found */
        }
        uwLevel++;
    }

    if(uwLevel == g_uwSpecMemClassCnt) /* not found */
    {
        LOS_IntRestore(uvIntSave);
        return -1;
    }
    LOS_IntRestore(uvIntSave);
    return uwLevel;
}


LITE_OS_SEC_TEXT_INIT STATIC VOID osAtomicBitsetInit(struct LOS_SPEC_ATOMIC_BITSET *pstSet, UINT32 uwNumBits)
{
    pstSet->uwNumBits = uwNumBits;
    memset(pstSet->uwBitmap, 0, (uwNumBits + 31) / 8);
    if (uwNumBits & 31) //mark all high bits so that osAtomicBitsetFindClearAndSet() is simpler
    {
        pstSet->uwBitmap[uwNumBits / 32] = ((UINT32)((INT32)-1LL)) << (uwNumBits & 31);
    }
}

LITE_OS_SEC_TEXT STATIC UINT32 osAtomicBitsetGetNumBits(const struct LOS_SPEC_ATOMIC_BITSET *pstSet)
{
    return pstSet->uwNumBits;
}

LITE_OS_SEC_TEXT STATIC UINT32 osAtomicGetAllSetbitNum(const struct LOS_SPEC_ATOMIC_BITSET *pstSet)
{
    UINT32 uwIdx, uwBits, uwCnt = 0;

    for (uwIdx = 0; uwIdx < pstSet->uwNumBits / 32;)
    {
        uwBits = pstSet->uwBitmap[uwIdx];
        for (; uwBits; ++uwCnt, uwBits &= (uwBits -1)); // clear the lowest set bit
        uwIdx++;
    }

    uwBits = pstSet->uwBitmap[uwIdx] & ~(0xFFFFFFFF << (pstSet->uwNumBits & 31));
    for (; uwBits; ++uwCnt, uwBits &= (uwBits -1)); // clear the lowest set bit

    return uwCnt;
}

LITE_OS_SEC_TEXT STATIC BOOL osAtomicBitsetGetBit(const struct LOS_SPEC_ATOMIC_BITSET *pstSet, UINT32 uwNum)
{
    if (uwNum >= pstSet->uwNumBits) /* any value is as good as the next */
    {
        return false;
    }
    return !!((pstSet->uwBitmap[uwNum / 32]) & (1UL << (uwNum & 31)));
}

LITE_OS_SEC_TEXT STATIC VOID osAtomicBitsetClearBit(struct LOS_SPEC_ATOMIC_BITSET *pstSet, UINT32 uwNum)
{
    UINT32 *pWordPtr = pstSet->uwBitmap + uwNum / 32;

    if (uwNum >= pstSet->uwNumBits)
    {
        return;
    }

    (*pWordPtr) &= ~(1UL << (uwNum & 31));
}

LITE_OS_SEC_TEXT STATIC INT32 osAtomicBitsetFindClearAndSet(struct LOS_SPEC_ATOMIC_BITSET *pstSet)
{
    UINT32 uwIdx, uwNumWords;
    UINT32 *puwWordPtr;
    UINT32 uwTmpWord;
    INT32  swCnt = 0;

    puwWordPtr = pstSet->uwBitmap;
    uwNumWords = (pstSet->uwNumBits + 31) / 32;
    for (uwIdx = 0; uwIdx < uwNumWords; uwIdx++, puwWordPtr++)
    {
        if (*puwWordPtr == 0xFFFFFFFF)
        {
            continue;
        }

        uwTmpWord = ~(*puwWordPtr);

        while(uwTmpWord)
        {
            uwTmpWord = uwTmpWord >> 1UL;
            swCnt++;
        }

        *puwWordPtr |= (1UL << (swCnt - 1));
        return (INT32)(uwIdx * 32 + swCnt - 1);
    }

    return -1;
}

/************************************************************************************************/
LITE_OS_SEC_TEXT STATIC struct LOS_SPEC_ALLOCATOR_HEADER* osSpecAllocatorNew(VOID *pHeapAddr, UINT32 uwItemSz, UINT32 uwItemAlign, UINT32 uwItemNum)
{
    struct LOS_SPEC_ALLOCATOR_HEADER *pstNewAllocator;
    UINT32 uwBitsetSz, uwDataSz;
    UINT32 uwItemSize;
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
    UINT32 uwTaskID;
#endif
    /* calcualte size */
    uwBitsetSz = OS_SPEC_ATOMIC_BITSET_SIZE_GET(uwItemNum);
    uwBitsetSz = (uwBitsetSz + (uwItemAlign - 1)) & ~(uwItemAlign - 1);
    uwItemSize = (uwItemSz + (uwItemAlign - 1)) & ~(uwItemAlign - 1);
    uwDataSz = uwItemSize * uwItemNum;

    /* allocate & init*/
    pstNewAllocator = (struct LOS_SPEC_ALLOCATOR_HEADER*)LOS_MemAlloc(pHeapAddr, sizeof(struct LOS_SPEC_ALLOCATOR_HEADER) + uwBitsetSz + uwDataSz);
    if (pstNewAllocator)
    {
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
        if (pHeapAddr == (VOID *)OS_SYS_MEM_ADDR)
        {
            if (g_stLosTask.pstRunTask != NULL && OS_INT_INACTIVE)
            {
                uwTaskID = g_stLosTask.pstRunTask->uwTaskID;
            }
            else
            {
                uwTaskID = OS_NULL_INT;
            }
            OS_MEM_REDUCE_USED(uwDataSz, uwTaskID);
        }
#endif
        pstNewAllocator->uwItemSz = uwItemSize;
        pstNewAllocator->ucDataChunks = ((UINT8*)pstNewAllocator->stBitset) + uwBitsetSz;
        osAtomicBitsetInit(pstNewAllocator->stBitset, uwItemNum);
    }

    return pstNewAllocator;
}

LITE_OS_SEC_TEXT STATIC VOID *osSpecAllocatorAlloc(struct LOS_SPEC_ALLOCATOR_HEADER *pstAllocator)
{
    VOID * pPtr = NULL;
    INT32 swItemIdx = osAtomicBitsetFindClearAndSet(pstAllocator->stBitset);
    if (swItemIdx < 0)
    {
        return NULL;
    }
    pPtr = pstAllocator->ucDataChunks + pstAllocator->uwItemSz * swItemIdx;
    return pPtr;
}

LITE_OS_SEC_TEXT STATIC UINT32 osSpecAllocatorFree(struct LOS_SPEC_ALLOCATOR_HEADER *pstAllocator, VOID *pPtr)
{
    UINT32 uwItemOffset = (UINT8*)pPtr - pstAllocator->ucDataChunks;
    UINT32 uwItemIdx = uwItemOffset / pstAllocator->uwItemSz;

    /* check for invalid input params */
    if ((uwItemOffset % pstAllocator->uwItemSz) || (uwItemIdx >= osAtomicBitsetGetNumBits(pstAllocator->stBitset)) || !osAtomicBitsetGetBit(pstAllocator->stBitset, uwItemIdx))
    {
        return LOS_NOK;
    }

    osAtomicBitsetClearBit(pstAllocator->stBitset, uwItemIdx);
    return LOS_OK;
}

/************************************************************************************************/

LITE_OS_SEC_TEXT STATIC VOID *osSpecBlockHeadFill(LOS_SPEC_BLOCK_NODE *pstSpecNode, UINT32 uwBlkSz, UINT32 uwOriginSize)
{
    UINT16 usTaskId;

    OS_SPEC_BLOCK_MAGIC_SET(pstSpecNode);
    OS_SPEC_BLOCK_SIZE_SET(pstSpecNode, uwBlkSz);
    /* If the operation occured before task initialization(g_stLosTask.pstRunTask was not assigned)
       or in interrupt,make the value of taskid of pstNode to 0xffff*/
    usTaskId = (g_stLosTask.pstRunTask != NULL && OS_INT_INACTIVE) ? (UINT16)(g_stLosTask.pstRunTask->uwTaskID) : OS_NULL_SHORT;

    OS_SPEC_BLOCK_ID_SET(pstSpecNode, usTaskId);
    return (VOID *)(pstSpecNode + 1);
}

UINT32 LOS_SpecMemCfgInit(VOID *pHeapAddr, UINT32 uwBlkSzMin, UINT32 uwBlkSzMax, UINT32 uwBlkNum)
{
    UINTPTR uvIntSave;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr;
    if(!pHeapAddr)
    {
        PRINT_ERR("%s failed, pHeapAddr %p is NULL\n",__FUNCTION__, pHeapAddr);
        return LOS_NOK;
    }

    if(uwBlkSzMin >= uwBlkSzMax)
    {
        PRINT_ERR("%s failed, uwBlkSzMax:0x%x should bigger than uwBlkSzMin:0x%x\n",__FUNCTION__, uwBlkSzMax, uwBlkSzMin);
        return LOS_NOK;
    }

    if(uwBlkNum == 0)
    {
        PRINT_ERR("%s failed, block num should be greater than zero\n",__FUNCTION__);
        return LOS_NOK;
    }

    pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);
    if(!pstSpecCtrlHdr)
    {
        PRINT_ERR("%s failed, pHeapAddr %p not the system mem pool\n",__FUNCTION__, pHeapAddr);
        return LOS_NOK; /*reach the max cnt */
    }
    uvIntSave = LOS_IntLock();
    if(g_uwSpecMemClassCnt >= g_uwSpecMemClassMaxCnt)
    {
        PRINT_ERR("%s failed, reach the max cnt %d\n",__FUNCTION__, LOS_SPEC_CLASS_CNT);
        LOS_IntRestore(uvIntSave);
        return LOS_NOK; /*reach the max cnt */
    }

    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].pstSpecClassAllocator = osSpecAllocatorNew(pHeapAddr, uwBlkSzMax + sizeof(LOS_SPEC_BLOCK_NODE), sizeof(VOID *), uwBlkNum);
    if(pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].pstSpecClassAllocator == NULL)
    {
        PRINT_ERR("%s (%d,%d] num:%d failed\n",__FUNCTION__,uwBlkSzMin, uwBlkSzMax ,uwBlkNum);
        LOS_IntRestore(uvIntSave);
        return LOS_NOK; /* do not alloc mem */
    }
    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].uwBlkSzMin = uwBlkSzMin;
    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].uwBlkSzMax = uwBlkSzMax;
    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].uwClassAllocNum = uwBlkNum;
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].uwClassCurUsedNum = 0;
    pstSpecCtrlHdr->stSpecMemClass[g_uwSpecMemClassCnt].uwClassWaterLine = 0;
#endif
    g_stAllocatorCfg = *pstSpecCtrlHdr;
    g_uwSpecMemClassCnt++;
    LOS_IntRestore(uvIntSave);
    return LOS_OK;
}

LITE_OS_SEC_TEXT VOID *osSpecMemAlloc(VOID *pHeapAddr, UINT32 uwAllocSize)
{

    VOID *pPtr = NULL;
    INT32 swLevel = -1;
    struct LOS_SPEC_CLASS *pstSpecClass;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);

    if(!pstSpecCtrlHdr)
    {
        return NULL;
    }
    swLevel = osClassLevelGet(uwAllocSize);
    if(swLevel == -1)
    {
        return NULL;
    }
    pstSpecClass = &(pstSpecCtrlHdr->stSpecMemClass[swLevel]);
    if (pstSpecClass->pstSpecClassAllocator)
    {

        pPtr = osSpecAllocatorAlloc(pstSpecClass->pstSpecClassAllocator);
        if (NULL != pPtr)
        {
            pPtr = osSpecBlockHeadFill((LOS_SPEC_BLOCK_NODE *)pPtr, pstSpecClass->uwBlkSzMax, uwAllocSize);
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
            pstSpecClass->uwClassCurUsedNum++;
            if(pstSpecClass->uwClassWaterLine < pstSpecClass->uwClassCurUsedNum)
            {
                pstSpecClass->uwClassWaterLine = pstSpecClass->uwClassCurUsedNum;
            }
#endif
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
            if (pHeapAddr == (VOID *)OS_SYS_MEM_ADDR)
            {
                OS_MEM_ADD_USED(pstSpecClass->pstSpecClassAllocator->uwItemSz, (UINT32)OS_SPEC_BLOCK_ID_GET((LOS_SPEC_BLOCK_NODE *)pPtr - 1));
            }
#endif
        }
    }
    return pPtr;
}

LITE_OS_SEC_TEXT VOID *osSpecMemAllocAlign(VOID *pHeapAddr, UINT32 uwAllocSize, UINT32 uwBoundary)
{

    VOID *pPtr = NULL;
    VOID *pAlignedPtr = NULL;
    UINT32 uwGapSize = 0;
    LOS_SPEC_BLOCK_NODE *pstNode = NULL;

    /*sizeof(uwGapSize) bytes stores offset between alignedPtr and ptr,
    the pPtr has been OS_MEM_ALIGN_SIZE(4 or 8) aligned, so maximum
    offset between alignedPtr and ptr is uwBoundary - OS_MEM_ALIGN_SIZE */
    if ((uwBoundary - sizeof(uwGapSize)) > (((UINT32)-1) - uwAllocSize))
    {
        goto OUTPUT;
    }
    uwAllocSize = uwAllocSize + uwBoundary - sizeof(UINT32);
    pPtr = osSpecMemAlloc(pHeapAddr, uwAllocSize);
    if(NULL == pPtr)
    {
        goto OUTPUT;
    }
    pAlignedPtr = (VOID *)OS_MEM_SPEC_ALIGN(pPtr, uwBoundary);
    if (pPtr == pAlignedPtr)
    {
        goto OUTPUT;
    }
    /* store gapSize in address (ptr -4), it will be checked while free */
    uwGapSize = (AARCHPTR)pAlignedPtr - (AARCHPTR)pPtr;
    OS_SPEC_BLOCK_SET_ALIGNED_FLAG(uwGapSize);
    pstNode = OS_SPEC_BLOCK_HEAD_GET(pPtr);
    *((UINT32 *)((AARCHPTR)pAlignedPtr - 4)) = uwGapSize;
    pPtr = pAlignedPtr;

OUTPUT:
    return pPtr;

}
LITE_OS_SEC_TEXT VOID *osSpecGetOriginPtr(VOID *pHeapAddr, VOID *pPtr)
{
    UINT32 uwGapSize = 0;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);
    if(!pstSpecCtrlHdr)
    {
        return pPtr;
    }
    if ((AARCHPTR)pPtr & (OS_SPEC_ALIGN_SIZE - 1))
    {
        return pPtr;
    }
    uwGapSize = *((UINT32 *)((AARCHPTR)pPtr - 4));
    if (OS_SPEC_BLOCK_GET_ALIGNED_FLAG(uwGapSize))
    {
        uwGapSize = OS_SPEC_BLOCK_GET_ALIGNED_GAPSIZE(uwGapSize);
        if (!(uwGapSize & (OS_SPEC_ALIGN_SIZE - 1)) && uwGapSize <= (AARCHPTR)pPtr - sizeof(LOS_SPEC_BLOCK_NODE) - (AARCHPTR)pHeapAddr)
        {

            pPtr = (VOID *)((AARCHPTR)pPtr - uwGapSize);
        }
    }
    return pPtr;
}
LITE_OS_SEC_TEXT UINT32 osSpecMemFree(VOID *pHeapAddr, VOID* pPtr)
{
    INT32 swLevel;
    UINT32 uwRet = 0;
    UINT32 uwGapSize = 0;
    struct LOS_SPEC_CLASS *pstSpecClass;
    LOS_SPEC_BLOCK_NODE *pstSpecNode;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);
    if(!pstSpecCtrlHdr)
    {
        return LOS_NOK;
    }
    if ((AARCHPTR)pPtr & (OS_SPEC_ALIGN_SIZE - 1))
    {
        return LOS_NOK;
    }
    uwGapSize = *((UINT32 *)((AARCHPTR)pPtr - 4));
    if (OS_SPEC_BLOCK_GET_ALIGNED_FLAG(uwGapSize))
    {
        uwGapSize = OS_SPEC_BLOCK_GET_ALIGNED_GAPSIZE(uwGapSize);
        if (!(uwGapSize & (OS_SPEC_ALIGN_SIZE - 1)) && uwGapSize <= (AARCHPTR)pPtr - sizeof(LOS_SPEC_BLOCK_NODE) - (AARCHPTR)pHeapAddr)
        {
            pPtr = (VOID *)((AARCHPTR)pPtr - uwGapSize);
        }
    }
    pstSpecNode = OS_SPEC_BLOCK_HEAD_GET(pPtr);
    if (!OS_ALLOC_FROM_SPEC_CHECK(pstSpecNode))
    {
        return LOS_NOK;
    }
    swLevel = osClassLevelGet(OS_SPEC_BLOCK_SIZE_GET(pstSpecNode));
    if (swLevel == -1)
    {
        return LOS_NOK;
    }
    pstSpecClass = &(pstSpecCtrlHdr->stSpecMemClass[swLevel]);
    uwRet = osSpecAllocatorFree(pstSpecClass->pstSpecClassAllocator, (VOID *)pstSpecNode);
    if(uwRet == LOS_OK)
    {
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
        pstSpecClass->uwClassCurUsedNum--;
#endif

#ifdef OS_MEM_ENABLE_MEM_STATISTICS
        if (pHeapAddr == (VOID *)OS_SYS_MEM_ADDR)
        {
            OS_MEM_REDUCE_USED(OS_SPEC_BLOCK_SIZE_GET(pstSpecNode) + sizeof(LOS_SPEC_BLOCK_NODE), (UINT32)OS_SPEC_BLOCK_ID_GET(pstSpecNode));
        }
#endif
        OS_SPEC_BLOCK_ID_SET(pstSpecNode, OS_NULL_SHORT);
    }
    return uwRet;
}

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
LITE_OS_SEC_TEXT INT32 LOS_SpecUsedBlksGet(VOID *pHeapAddr, UINT32 uwClassSize)
{
    INT32 swLevel = 0;
    UINT32 uwUsedBlks = 0;
    UINTPTR uvIntSave;
    struct LOS_SPEC_CLASS *pstSpecClass;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr;

    if (!pHeapAddr)
    {
        PRINT_ERR("%s failed, pHeapAddr %p is NULL\n",__FUNCTION__, pHeapAddr);
        return -1;
    }
    pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);
    if (!pstSpecCtrlHdr)
    {
        PRINT_ERR("%s failed, spec control header %p is NULL\n",__FUNCTION__, pstSpecCtrlHdr);
        return -1;
    }
    swLevel = osClassLevelGet(uwClassSize);

    if (swLevel == -1)
    {
        PRINT_ERR("%s failed, uwClassSize %d is not config\n",__FUNCTION__, uwClassSize);
        return -1;
    }

    uvIntSave = LOS_IntLock();
    pstSpecClass = &(pstSpecCtrlHdr->stSpecMemClass[swLevel]);
    uwUsedBlks = pstSpecClass->uwClassCurUsedNum;

    LOS_IntRestore(uvIntSave);
    return (INT32)uwUsedBlks;
}

LITE_OS_SEC_TEXT INT32 LOS_SpecFreeBlksGet(VOID *pHeapAddr, UINT32 uwClassSize)
{
    INT32 swLevel = 0;
    UINT32 uwUsedBlks = 0;
    UINT32 uwAllBlks = 0;
    UINTPTR uvIntSave;
    struct LOS_SPEC_CLASS *pstSpecClass;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr;

    if (!pHeapAddr)
    {
        PRINT_ERR("%s failed, pHeapAddr %p is NULL\n",__FUNCTION__, pHeapAddr);
        return -1;
    }
    pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(pHeapAddr);
    if (!pstSpecCtrlHdr)
    {
        PRINT_ERR("%s failed, spec control header %p is NULL\n",__FUNCTION__, pstSpecCtrlHdr);
        return -1;
    }
    swLevel = osClassLevelGet(uwClassSize);

    if (swLevel == -1)
    {
        PRINT_ERR("%s failed, uwClassSize %d is not config\n",__FUNCTION__, uwClassSize);
        return -1;
    }
    uvIntSave = LOS_IntLock();

    pstSpecClass = &(pstSpecCtrlHdr->stSpecMemClass[swLevel]);
    uwAllBlks  = pstSpecClass->uwClassAllocNum;
    uwUsedBlks = pstSpecClass->uwClassCurUsedNum;

    LOS_IntRestore(uvIntSave);

    return (INT32)(uwAllBlks - uwUsedBlks);
}
LITE_OS_SEC_TEXT VOID LOS_SpecInfoShow(VOID)
{
    UINT32 uwIndex;
    UINT32 uwClassSz = 0;
    UINTPTR uvIntSave;
    struct LOS_SPEC_CONTROL_HEADER *pstSpecCtrlHdr;

    uvIntSave = LOS_IntLock();

    PRINTK("\n-----------------------  ------------------------------------------------------------------------------------------------------------------\n");
    PRINTK("  heap (addr)                                               each class total blks/max blks used/free blks\n");
    PRINTK("-----------------------  ------------------------------------------------------------------------------------------------------------------\n");

    pstSpecCtrlHdr = OS_SPEC_CTRL_HEADER_GET(OS_SYS_MEM_ADDR);
    PRINTK("(%p)", OS_SYS_MEM_ADDR);
    PRINTK("total        ");
    for (uwIndex = 0; uwIndex < g_uwSpecMemClassCnt; uwIndex++)
    {
        PRINTK(" %d:%-2d ",
            pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwBlkSzMax, pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwClassAllocNum);
    }
    PRINTK("\n");
    PRINTK("(%p)", OS_SYS_MEM_ADDR);
    PRINTK("free         ");
    for (uwIndex = 0; uwIndex < g_uwSpecMemClassCnt; uwIndex++)
    {
        PRINTK(" %d:%-2d ",
            pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwBlkSzMax, pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwClassAllocNum - pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwClassCurUsedNum);
    }
    PRINTK("\n");
    PRINTK("(%p)", OS_SYS_MEM_ADDR);

    PRINTK("waterline    ");
    for (uwIndex = 0; uwIndex < g_uwSpecMemClassCnt; uwIndex++)
    {
        PRINTK(" %d:%-2d ",
            pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwBlkSzMax, pstSpecCtrlHdr->stSpecMemClass[uwIndex].uwClassWaterLine);
    }

    PRINTK("\n");

    PRINTK("------------------------  ------------------------------------------------------------------------------------------------------------------\n");

    LOS_IntRestore(uvIntSave);
}
#endif


#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

