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
#include "los_slabmem.ph"
#include "los_memstat.ph"
#include "los_task.ph"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOS_MEM_SLAB
extern VOID *osSlabCtrlHdrGet(VOID *pHeapAddr);
extern VOID *osHeapAlloc(VOID *tlsf, UINT32 uwSize);

#define OS_SLAB_CTRL_HEADER_GET(pHeapAddr)  (osSlabCtrlHdrGet(pHeapAddr))
struct LOS_SLAB_ALLOCATOR_CFG_INFO g_stAllocatorSizeCfg;

#ifdef LOS_SLAB_INFO_STATISTIC
UINT32 g_uwMemCfgNum = 0;
struct LOS_SLAB_STATISTIC_INFO g_stClassBlkMaxUsed[OS_MEM_NUM_LIMIT];
#endif
#ifdef LOSCFG_MEM_MUL_MODULE
UINT32 g_uwSlabmemInfo[MEM_MODULE_MAX + 1] = {0};
#endif
/************************************************************************************************/
LITE_OS_SEC_TEXT_INIT STATIC VOID osAtomicBitsetInit(struct LOS_ATOMIC_BITSET *pstSet, UINT32 uwNumBits)
{
    pstSet->uwNumBits = uwNumBits;
    memset(pstSet->uwBitmap, 0, (uwNumBits + 31) / 8);
    if (uwNumBits & 31) //mark all high bits so that osAtomicBitsetFindClearAndSet() is simpler
        pstSet->uwBitmap[uwNumBits / 32] = ((UINT32)((INT32)-1LL)) << (uwNumBits & 31);
}

LITE_OS_SEC_TEXT STATIC UINT32 osAtomicBitsetGetNumBits(const struct LOS_ATOMIC_BITSET *pstSet)
{
    return pstSet->uwNumBits;
}

LITE_OS_SEC_TEXT STATIC UINT32 osAtomicGetAllSetbitNum(const struct LOS_ATOMIC_BITSET *pstSet)
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

LITE_OS_SEC_TEXT STATIC BOOL osAtomicBitsetGetBit(const struct LOS_ATOMIC_BITSET *pstSet, UINT32 uwNum)
{
    if (uwNum >= pstSet->uwNumBits) /* any value is as good as the next */
        return false;

    return !!((pstSet->uwBitmap[uwNum / 32]) & (1UL << (uwNum & 31)));
}

LITE_OS_SEC_TEXT STATIC VOID osAtomicBitsetClearBit(struct LOS_ATOMIC_BITSET *pstSet, UINT32 uwNum)
{
    UINT32 *wordPtr = pstSet->uwBitmap + uwNum / 32;

    if (uwNum >= pstSet->uwNumBits)
        return;

    (*wordPtr) &= ~(1UL << (uwNum & 31));
}

LITE_OS_SEC_TEXT STATIC INT32 osAtomicBitsetFindClearAndSet(struct LOS_ATOMIC_BITSET *pstSet)
{
    UINT32 uwIdx, uwNumWords = (pstSet->uwNumBits + 31) / 32;
    UINT32 *puwWordPtr = pstSet->uwBitmap;
    UINT32 uwTmpWord;
    INT32 cnt = 0;

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
            cnt++;
        }

        *puwWordPtr |= (1UL << (cnt - 1));

        return (INT32)(uwIdx * 32 + cnt - 1);
    }

    return -1;
}

/************************************************************************************************/
LITE_OS_SEC_TEXT STATIC struct LOS_SLAB_ALLOCATOR_HEADER* osSlabAllocatorNew(VOID *pHeapAddr, UINT32 uwItemSz, UINT32 uwItemAlign, UINT32 uwNumItems)
{
    struct LOS_SLAB_ALLOCATOR_HEADER *pstNewAllocator;
    UINT32 uwBitsetSz, uwDataSz;
    UINT32 uwItemSize;
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
    UINT32 uwTaskID;
#endif
    /* calcualte size */
    uwBitsetSz = OS_ATOMIC_BITSET_SIZE_GET(uwNumItems);
    uwBitsetSz = (uwBitsetSz + (uwItemAlign - 1)) & ~(uwItemAlign - 1);
    uwItemSize = (uwItemSz + (uwItemAlign - 1)) & ~(uwItemAlign - 1);
    uwDataSz = uwItemSize * uwNumItems;

    /* allocate & init*/
    pstNewAllocator = (struct LOS_SLAB_ALLOCATOR_HEADER*)osHeapAlloc(pHeapAddr, sizeof(struct LOS_SLAB_ALLOCATOR_HEADER) + uwBitsetSz + uwDataSz);

    if (pstNewAllocator) {
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
        osAtomicBitsetInit(pstNewAllocator->stBitset, uwNumItems);
    }

    return pstNewAllocator;
}

LITE_OS_SEC_TEXT STATIC VOID *osSlabAllocatorAlloc(struct LOS_SLAB_ALLOCATOR_HEADER *pstAllocator)
{
    INT32 itemIdx = osAtomicBitsetFindClearAndSet(pstAllocator->stBitset);
    if (itemIdx < 0)
        return NULL;

    return pstAllocator->ucDataChunks + pstAllocator->uwItemSz * itemIdx;
}

LITE_OS_SEC_TEXT STATIC UINT32 osSlabAllocatorFree(struct LOS_SLAB_ALLOCATOR_HEADER *pstAllocator, VOID *pPtr)
{
    UINT32 uwItemOffset = (UINT8*)pPtr - pstAllocator->ucDataChunks;
    UINT32 uwItemIdx = uwItemOffset / pstAllocator->uwItemSz;
    /* check for invalid inputs */
    if ((uwItemOffset % pstAllocator->uwItemSz) || (uwItemIdx >= osAtomicBitsetGetNumBits(pstAllocator->stBitset)) || !osAtomicBitsetGetBit(pstAllocator->stBitset, uwItemIdx))
        return LOS_NOK;
    osAtomicBitsetClearBit(pstAllocator->stBitset, uwItemIdx);
    return LOS_OK;
}

/************************************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID *osSlabBlockHeadFill(LOS_SLAB_BLOCK_NODE *pstSlabNode, UINT32 uwBlkSz)
{
    UINT8 ucId;

    OS_SLAB_BLOCK_MAGIC_SET(pstSlabNode);
    OS_SLAB_BLOCK_SIZE_SET(pstSlabNode, uwBlkSz);

    /* If the operation occured before task initialization(g_stLosTask.pstRunTask was not assigned)
       or in interrupt,make the value of taskid of pstNode to 0xff*/

    ucId = (g_stLosTask.pstRunTask != NULL && OS_INT_INACTIVE) ? (UINT8)(g_stLosTask.pstRunTask->uwTaskID) : OS_NULL_BYTE;
    OS_SLAB_BLOCK_ID_SET(pstSlabNode, ucId);

    return (VOID *)(pstSlabNode + 1);
}

LITE_OS_SEC_TEXT_INIT VOID LOS_SlabCfgStructInit(struct LOS_SLAB_ALLOCATOR_CFG_INFO *pstCfg)
{
    INT32 uwIdx;

    if (pstCfg == NULL)
    {
        return;
    }

    for (uwIdx = 0; uwIdx < LOS_SLAB_CLASS_CNT; uwIdx++)
    {
        pstCfg->usClassAllocatorSize[uwIdx] = LOS_SLAB_DEFAULT_ALLOCATOR_SIZE;
    }
}


LITE_OS_SEC_TEXT VOID LOS_SlabAllocatorCfgRegister(VOID * pHeapAddr, struct LOS_SLAB_ALLOCATOR_CFG_INFO *pstCfg)
{
    if (pstCfg == NULL)
    {
        return;
    }

    g_stAllocatorSizeCfg = *pstCfg;
}

LITE_OS_SEC_TEXT UINT32 osSlabMemInit(VOID *pHeapAddr)
{
#ifdef LOS_SLAB_INFO_STATISTIC
    INT32 uwIndex;
    INT32 uwHeapIdx = 0;
#endif
    INT32 classLevel;
    UINT32 uwBlkSz;
    UINT32 uwNumItems;
    struct LOS_SLAB_CONTROL_HEADER *pstSlabCtrlHdr = OS_SLAB_CTRL_HEADER_GET(pHeapAddr);

#ifdef LOS_SLAB_INFO_STATISTIC
    /* set each class allocator size */
    pstSlabCtrlHdr->stAllocatorSizeCfg = g_stAllocatorSizeCfg;
    if (g_uwMemCfgNum >= OS_MEM_NUM_LIMIT)
    {
        PRINT_ERR("g_uwMemCfgNum(%d) > OS_MEM_NUM_LIMIT!\n", g_uwMemCfgNum);
        return (UINT32)OS_ERROR;
    }

    while (uwHeapIdx < g_uwMemCfgNum)
    {
        if (g_stClassBlkMaxUsed[uwHeapIdx++].pHeapAddr == pHeapAddr)
        {
            return (UINT32)OS_ERROR;
        }
    }

    g_stClassBlkMaxUsed[g_uwMemCfgNum++].pHeapAddr = pHeapAddr;
#endif

    for (classLevel = 0; classLevel < LOS_SLAB_CLASS_CNT; classLevel++)
    {
        /* set each class block size */
        uwBlkSz = (LOS_SLAB_HEAP_BOUNDARY / LOS_SLAB_CLASS_CNT) * (classLevel + 1);
        pstSlabCtrlHdr->stSlabClass[classLevel].uwBlkSz = uwBlkSz;

        /* init each class allocator */
        uwNumItems = (UINT32)(g_stAllocatorSizeCfg.usClassAllocatorSize[classLevel]) / uwBlkSz;
        if (uwNumItems == 0)
        {
            pstSlabCtrlHdr->stSlabClass[classLevel].pstSlabClassAllocator = NULL;
            PRINT_INFO("the slab class level %d do not cfg allocator size, try to reconfiguration if needed!\n", classLevel);
            continue;
        }

        pstSlabCtrlHdr->stSlabClass[classLevel].pstSlabClassAllocator = osSlabAllocatorNew(pHeapAddr, uwBlkSz + sizeof(LOS_SLAB_BLOCK_NODE), sizeof(VOID *), uwNumItems);
        if (pstSlabCtrlHdr->stSlabClass[classLevel].pstSlabClassAllocator == NULL)
        {
            return (UINT32)OS_ERROR;
        }
    }

    LOS_SlabCfgStructInit(&g_stAllocatorSizeCfg);

    return (UINT32)LOS_OK;
}
#ifdef LOSCFG_MEM_MUL_POOL
LITE_OS_SEC_TEXT UINT32 osSlabMemDeInit(VOID *pHeapAddr)
{
#ifdef LOS_SLAB_INFO_STATISTIC
    INT32 uwHeapIdx = 0;
    while (uwHeapIdx < g_uwMemCfgNum)
    {
        if (g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr == pHeapAddr)
        {
            g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr = NULL;
            g_uwMemCfgNum --;
            return (UINT32)LOS_OK;
        }
        uwHeapIdx++;
    }
#else
    return (UINT32)LOS_OK;
#endif
}
#endif

LITE_OS_SEC_TEXT VOID *osSlabMemAlloc(VOID *pHeapAddr, UINT32 uwAllocSize)
{
    VOID *pRet = NULL;
    INT32 classLevel = OS_SLAB_CLASS_LEVEL_GET(uwAllocSize);
    struct LOS_SLAB_CONTROL_HEADER *pstSlabCtrlHdr = OS_SLAB_CTRL_HEADER_GET(pHeapAddr);
    struct LOS_SLAB_CLASS *pstSlabClass;

    pstSlabClass = &(pstSlabCtrlHdr->stSlabClass[classLevel]);

    if (pstSlabClass->pstSlabClassAllocator)
    {
        pRet = osSlabAllocatorAlloc(pstSlabClass->pstSlabClassAllocator);
        if (NULL != pRet)
        {
            pRet = osSlabBlockHeadFill((LOS_SLAB_BLOCK_NODE *)pRet, pstSlabClass->uwBlkSz);
        }

#ifdef LOS_SLAB_INFO_STATISTIC
        if (pRet != NULL)
        {
            osSlabInfoStatistic(pHeapAddr);
        }
#endif
#ifdef OS_MEM_ENABLE_MEM_STATISTICS
        if (pRet != NULL && pHeapAddr == (VOID *)OS_SYS_MEM_ADDR)
        {
            OS_MEM_ADD_USED(pstSlabClass->pstSlabClassAllocator->uwItemSz, (UINT32)OS_SLAB_BLOCK_ID_GET((LOS_SLAB_BLOCK_NODE *)pRet - 1));
        }
#endif
    }
    if (NULL == pRet)
    {
        return NULL;
    }

    return pRet;
}

LITE_OS_SEC_TEXT UINT32 osSlabMemFree(VOID *pHeapAddr, VOID* pPtr)
{
    struct LOS_SLAB_CLASS *pstSlabClass;
    struct LOS_SLAB_CONTROL_HEADER *pstSlabCtrlHdr = OS_SLAB_CTRL_HEADER_GET(pHeapAddr);
    LOS_SLAB_BLOCK_NODE *pstSlabNode = OS_SLAB_BLOCK_HEAD_GET(pPtr);
    UINT32 uwRet = 0;

    if (!OS_ALLOC_FROM_SLAB_CHECK(pstSlabNode))
    {

        return LOS_NOK;
    }

    pstSlabClass = &(pstSlabCtrlHdr->stSlabClass[OS_SLAB_CLASS_LEVEL_GET(OS_SLAB_BLOCK_SIZE_GET(pstSlabNode))]);
    uwRet = osSlabAllocatorFree(pstSlabClass->pstSlabClassAllocator, (VOID *)pstSlabNode);
#ifdef OS_MEM_ENABLE_MEM_STATISTICS

    if (uwRet == LOS_OK && pHeapAddr == (VOID *)OS_SYS_MEM_ADDR)
    {
        OS_MEM_REDUCE_USED(pstSlabClass->pstSlabClassAllocator->uwItemSz, (UINT32)OS_SLAB_BLOCK_ID_GET(pstSlabNode));
    }
#endif
    if (uwRet == LOS_OK)
    {
        OS_SLAB_BLOCK_ID_SET(pstSlabNode, OS_NULL_BYTE);
    }
    return uwRet;
}

#ifdef LOS_SLAB_INFO_STATISTIC
LITE_OS_SEC_TEXT  VOID osSlabInfoStatistic(VOID *pHeapAddr)
{
    UINT32 uwIndex;
    UINT32 uwClassSz = 0;
    UINT32 uwHeapIdx = 0;
    UINT32 uwNum;

    while (uwHeapIdx < g_uwMemCfgNum)
    {
        if (g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr == pHeapAddr)
        {
            break;
        }

        uwHeapIdx++;
    }

    if (uwHeapIdx == g_uwMemCfgNum)
    {
        return;
    }

    for (uwIndex = 0; uwIndex < LOS_SLAB_CLASS_CNT; uwIndex++)
    {
        uwClassSz += LOS_SLAB_HEAP_BOUNDARY / LOS_SLAB_CLASS_CNT;
        uwNum = LOS_SlabUsedBlksGet(pHeapAddr, uwClassSz);
        if ((uwNum != (UINT32)-1) && (g_stClassBlkMaxUsed[uwHeapIdx].usClassSizeMaxUsed[uwIndex] < uwNum))
        {
            g_stClassBlkMaxUsed[uwHeapIdx].usClassSizeMaxUsed[uwIndex] = uwNum;
        }
    }
}

LITE_OS_SEC_TEXT VOID LOS_SlabInfoStatisticsPrint(VOID)
{
    INT32 uwIndex;
    UINT32 uwHeapIdx;
    UINT32 uwClassSz = 0;
    UINTPTR uvIntSave;

    uvIntSave = LOS_IntLock();

    PRINTK("\n-------------------  ------------------------------------------------------------------------------------------------------------------\n");
    PRINTK("  heap index(addr)                                               each class max blks used\n");
    PRINTK("-------------------  ------------------------------------------------------------------------------------------------------------------\n");

    for (uwHeapIdx = 0; uwHeapIdx < g_uwMemCfgNum; uwHeapIdx++)
    {
        PRINTK("   %d", uwHeapIdx);
        if (g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr == NULL)
        {
            PRINTK("\n");
            continue;
        }

        PRINTK("(%p)     ", g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr);

        for (uwIndex = 0; uwIndex < LOS_SLAB_CLASS_CNT; uwIndex++)
        {
            uwClassSz = LOS_SLAB_HEAP_BOUNDARY / LOS_SLAB_CLASS_CNT * (uwIndex + 1);
            PRINTK(" %d:%-2d ", uwClassSz, g_stClassBlkMaxUsed[uwHeapIdx].usClassSizeMaxUsed[uwIndex]);
        }

        PRINTK("\n");
    }

    PRINTK("-------------------  ------------------------------------------------------------------------------------------------------------------\n");

    LOS_IntRestore(uvIntSave);
}

LITE_OS_SEC_TEXT UINT32 LOS_SlabFreeNodeShow(VOID)
{
    UINTPTR uvIntSave;
    UINT32 uwIndex;
    UINT32 uwHeapIdx;
    UINT32 uwClassSz = 0;


    uvIntSave = LOS_IntLock();
    PRINTK("\n-------------------  ------------------------------------------------------------------------------------------------------------------\n");
    PRINTK("  heap index(addr)                                               each class left blks number(class level:num)\n");
    PRINTK("-------------------  ------------------------------------------------------------------------------------------------------------------\n");
    for (uwHeapIdx = 0; uwHeapIdx < g_uwMemCfgNum; uwHeapIdx++)
    {
        PRINTK("   %d", uwHeapIdx);
        if (g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr == NULL)
        {
            PRINTK("\n");
            continue;
        }

        PRINTK("(%p)     ", g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr);

        for (uwIndex = 0; uwIndex < LOS_SLAB_CLASS_CNT; uwIndex++)
        {
            uwClassSz = LOS_SLAB_HEAP_BOUNDARY / LOS_SLAB_CLASS_CNT * (uwIndex + 1);
            PRINTK(" %d:%-2d ", uwClassSz, LOS_SlabFreeBlksGet(g_stClassBlkMaxUsed[uwHeapIdx].pHeapAddr, uwClassSz));
        }

        PRINTK("\n");
    }
    PRINTK("-------------------  ------------------------------------------------------------------------------------------------------------------\n");
}

#endif

LITE_OS_SEC_TEXT UINT32 LOS_SlabUsedBlksGet(VOID *pHeapAddr, UINT32 uwClassSize)
{
    UINT32 uwUsedBlks = 0;
    UINTPTR uvIntSave;
    struct LOS_SLAB_CLASS *pstSlabClass;
    struct LOS_SLAB_CONTROL_HEADER *pstSlabCtrlHdr;

    if (!pHeapAddr || uwClassSize > LOS_SLAB_HEAP_BOUNDARY || uwClassSize == 0)
    {
        return (UINT32)-1;
    }

    uvIntSave = LOS_IntLock();
    pstSlabCtrlHdr = OS_SLAB_CTRL_HEADER_GET(pHeapAddr);

    pstSlabClass = &(pstSlabCtrlHdr->stSlabClass[OS_SLAB_CLASS_LEVEL_GET(uwClassSize)]);
    if ((pstSlabClass != NULL) && (pstSlabClass->pstSlabClassAllocator != NULL))
    {
        uwUsedBlks += osAtomicGetAllSetbitNum(pstSlabClass->pstSlabClassAllocator->stBitset);
    }

    LOS_IntRestore(uvIntSave);

    return uwUsedBlks;
}



LITE_OS_SEC_TEXT UINT32 LOS_SlabFreeBlksGet(VOID *pHeapAddr, UINT32 uwClassSize)
{
    UINT32 uwUsedBlks = 0;
    UINT32 uwAllBlks = 0;
    UINTPTR uvIntSave;
    struct LOS_SLAB_CLASS *pstSlabClass;
    struct LOS_SLAB_CONTROL_HEADER *pstSlabCtrlHdr;

    if (!pHeapAddr || uwClassSize > LOS_SLAB_HEAP_BOUNDARY || uwClassSize == 0 )
    {
        return (UINT32)-1;
    }

    uvIntSave = LOS_IntLock();

    pstSlabCtrlHdr = OS_SLAB_CTRL_HEADER_GET(pHeapAddr);

    pstSlabClass = &(pstSlabCtrlHdr->stSlabClass[OS_SLAB_CLASS_LEVEL_GET(uwClassSize)]);
    if ((pstSlabClass != NULL) && (pstSlabClass->pstSlabClassAllocator != NULL))
    {
        uwAllBlks  += osAtomicBitsetGetNumBits(pstSlabClass->pstSlabClassAllocator->stBitset);
        uwUsedBlks += osAtomicGetAllSetbitNum(pstSlabClass->pstSlabClassAllocator->stBitset);
    }

    LOS_IntRestore(uvIntSave);

    return (uwAllBlks - uwUsedBlks);
}

#ifdef LOSCFG_MEM_MUL_POOL
UINT32 LOS_SlabMusedGet(UINT32 uwModule)
{
    if (uwModule > MEM_MODULE_MAX)
    {
        PRINT_ERR("error module ID input!\n");
        return LOS_NOK;
    }
    return g_uwSlabmemInfo[uwModule];
}
#endif

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

