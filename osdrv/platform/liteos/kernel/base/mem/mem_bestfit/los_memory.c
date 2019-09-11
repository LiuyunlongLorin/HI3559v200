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

#include "los_memory.inc"
#ifdef LOS_MEM_LEAK_CHECK
#include "los_membox.ph"
#endif
#include "los_task.ph"
#include "los_exc.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif

#ifdef LOSCFG_MEM_RECORDINFO
#include "los_memrecord.ph"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_memory
 * Memory pool information structure
 */
typedef struct
{
    VOID *pPoolAddr;                        /**<Starting address of a memory pool  */
    UINT32 uwPoolSize;                      /**<Memory pool size    */
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    UINT32 uwPoolWaterLine;                 /**<Maximum usage size in a memory  pool  */
    UINT32 uwPoolCurUsedSize;               /**<Current usage size in a memory  pool  */
#endif
#ifdef LOSCFG_MEM_MUL_POOL
    VOID *pNextPool;
#endif
} LOS_MEM_POOL_INFO;

/**
 * @ingroup los_memory
 * Memory linked list control node structure
 */
typedef struct tagLOS_MEM_CTL_NODE
{
    LOS_DL_LIST stFreeNodeInfo; /**<Free memory node  */
    struct tagLOS_MEM_DYN_NODE *pstPreNode; /**<Pointer to the previous memory node*/

#ifdef LOSCFG_MEM_HEAD_BACKUP
    UINT32 uwGapSize;
    AARCHPTR uwChecksum;  /**<magic = xor checksum>*/
#endif

#ifdef LOSCFG_MEM_RECORDINFO
    UINT32 uwOriginSize;
#ifdef LOSCFG_AARCH64
    UINT32 uwRCDReserve;
#endif
#endif
#ifdef LOSCFG_AARCH64
    UINT32 uwReserve;
#endif
    UINT32 uwSizeAndFlag;   /**<Size and flag of the current node (the high two bits represent a flag, and the rest bits specify the size)*/
} LOS_MEM_CTL_NODE;

/**
 * @ingroup los_memory
 * Memory linked list node structure
 */
typedef struct tagLOS_MEM_DYN_NODE
{
#ifdef LOSCFG_MEM_HEAD_BACKUP
    LOS_MEM_CTL_NODE stBackupNode;
#endif
    LOS_MEM_CTL_NODE stSelfNode;
}LOS_MEM_DYN_NODE;

#ifdef LOSCFG_MEM_MUL_POOL
VOID *pool_head = NULL;
#endif

#ifdef LOSCFG_MEM_HEAD_BACKUP
static VOID osMemNodeSave(LOS_MEM_DYN_NODE *pstNode);
#define OS_MEM_TASKID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext); \
                uwTmp &= 0xffff0000; \
                uwTmp |= ID; \
                ((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext = (LOS_DL_LIST *)uwTmp; \
                osMemNodeSave((LOS_MEM_DYN_NODE *)node); \
            } while(0)
#else
#define OS_MEM_TASKID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext); \
                uwTmp &= 0xffff0000; \
                uwTmp |= ID; \
                ((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext = (LOS_DL_LIST *)uwTmp; \
            } while(0)
#endif
#define OS_MEM_TASKID_GET(node)  ((AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext) & 0xffff)

#ifdef LOSCFG_MEM_HEAD_BACKUP
#define OS_MEM_MODID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext); \
                uwTmp &= 0xffff; \
                uwTmp |= ID << 16; \
                ((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext = (LOS_DL_LIST *)uwTmp; \
                osMemNodeSave((LOS_MEM_DYN_NODE *)node); \
            } while(0)
#else
#define OS_MEM_MODID_SET(node, ID)  \
            do \
            { \
                AARCHPTR uwTmp = (AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext); \
                uwTmp &= 0xffff; \
                uwTmp |= ID << 16; \
                ((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext = (LOS_DL_LIST *)uwTmp; \
            } while(0)
#endif
#define OS_MEM_MODID_GET(node)  (((AARCHPTR)(((LOS_MEM_DYN_NODE *)node)->stSelfNode.stFreeNodeInfo.pstNext) >> 16) & 0xffff)

#define OS_MEM_ALIGN(p, alignSize)    (((AARCHPTR)(p) + alignSize -1) & (~ ((AARCHPTR)(alignSize -1))))
#define OS_MEM_NODE_HEAD_SIZE sizeof(LOS_MEM_DYN_NODE)
#define OS_MEM_MIN_POOL_SIZE (OS_DLNK_HEAD_SIZE + 2 * OS_MEM_NODE_HEAD_SIZE + sizeof(LOS_MEM_POOL_INFO))
#define IS_ALIGNED(value, alignSize)  (0 == (((AARCHPTR)(value)) & ((AARCHPTR)(alignSize -1))))
#define IS_POW_TWO(value) (0 == (((AARCHPTR)value) & ((AARCHPTR)value - 1)))

#ifdef LOSCFG_AARCH64
#define OS_MEM_ALIGN_SIZE 8
#else
#define OS_MEM_ALIGN_SIZE 4
#endif
#define OS_MEM_NODE_USED_FLAG 0x80000000
#define OS_MEM_NODE_ALIGNED_FLAG 0x40000000
#define OS_MEM_NODE_ALIGNED_AND_USED_FLAG (OS_MEM_NODE_USED_FLAG | OS_MEM_NODE_ALIGNED_FLAG)

#define OS_MEM_NODE_GET_ALIGNED_FLAG(uwSizeAndFlag) ((uwSizeAndFlag) & OS_MEM_NODE_ALIGNED_FLAG)
#define OS_MEM_NODE_SET_ALIGNED_FLAG(uwSizeAndFlag)  (uwSizeAndFlag = ((uwSizeAndFlag) | OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_ALIGNED_GAPSIZE(uwSizeAndFlag) ((uwSizeAndFlag) & (~OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_USED_FLAG(uwSizeAndFlag) ((uwSizeAndFlag) & OS_MEM_NODE_USED_FLAG)
#define OS_MEM_NODE_SET_USED_FLAG(uwSizeAndFlag) (uwSizeAndFlag = ((uwSizeAndFlag) | OS_MEM_NODE_USED_FLAG))
#define OS_MEM_NODE_GET_SIZE(uwSizeAndFlag) ((uwSizeAndFlag) & (~OS_MEM_NODE_ALIGNED_AND_USED_FLAG))
#define OS_MEM_IS_NODE_NEXT_EXIST(pstNode, pstPoolInfo) (((AARCHPTR)(pstNode) + (pstNode)->stSelfNode.uwSizeAndFlag) < ((AARCHPTR)(pstPoolInfo) + (pstPoolInfo)->uwPoolSize))
#define OS_MEM_HEAD(pPool, uwSize) OS_DLnkHead(OS_MEM_HEAD_ADDR(pPool), uwSize)
#define OS_MEM_HEAD_ADDR(pPool) ((VOID *)((AARCHPTR)(pPool) + sizeof(LOS_MEM_POOL_INFO)))
#define OS_MEM_NEXT_NODE(pstNode) ((LOS_MEM_DYN_NODE *)((UINT8 *)(pstNode) + OS_MEM_NODE_GET_SIZE((pstNode)->stSelfNode.uwSizeAndFlag)))
#define OS_MEM_FIRST_NODE(pPool) ((LOS_MEM_DYN_NODE *) ((UINT8 *)OS_MEM_HEAD_ADDR(pPool) + OS_DLNK_HEAD_SIZE))
#define OS_MEM_END_NODE(pPool, uwSize) ((LOS_MEM_DYN_NODE *)(((UINT8 *)(pPool) + (uwSize)) - OS_MEM_NODE_HEAD_SIZE))
#define OS_MEM_MIDDLE_ADDR_OPEN_END(startAddr, middleAddr, endAddr) (((UINT8 *)(startAddr) <= ((UINT8 *)(middleAddr))) && (((UINT8 *)(middleAddr)) < ((UINT8 *)(endAddr))))
#define OS_MEM_MIDDLE_ADDR(startAddr, middleAddr, endAddr) (((UINT8 *)(startAddr) <= ((UINT8 *)(middleAddr))) && (((UINT8 *)(middleAddr)) <= ((UINT8 *)(endAddr))))
#define OS_MEM_SET_MAGIC(value) (value) = (LOS_DL_LIST *)((AARCHPTR)(&(value)) ^ (AARCHPTR)(-1))
#define OS_MEM_MAGIC_VALID(value) ((((AARCHPTR)(value)) ^ ((AARCHPTR)(&(value)))) == (AARCHPTR)(-1))

UINT8 *m_aucSysMem0 = (UINT8 *)NULL;
UINT8 *m_aucSysMem1 = (UINT8 *)NULL;

UINT8 *m_aucSysNoCacheMem0 = (UINT8 *)NULL;
__attribute__((section(".data.init"))) AARCHPTR g_sys_mem_addr_end;
__attribute__((section(".data.init"))) AARCHPTR g_exc_interact_mem_size = 0;
#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
static UINT8 ucCheckMemLevel = (UINT8)LOS_MEM_CHECK_LEVEL_DEFAULT;
#endif

#ifdef LOS_MEM_LEAK_CHECK
MEM_CHECK_INFO *g_pMemCheckPool;
UINT32 g_uwMemCount = 0;
UINT32 g_uwPeakCount = 0;
UINT32 g_uwCheckFlag = 0;
#endif

#ifdef LOSCFG_MEM_MUL_MODULE
UINT32 g_uwMemInfo[MEM_MODULE_MAX + 1] = {0};
#endif

MALLOC_HOOK g_MALLOC_HOOK = (MALLOC_HOOK)NULL; /*lint !e611*/

#ifdef LOSCFG_MEM_HEAD_BACKUP
STATIC INLINE VOID osMemDispCtlNode(LOS_MEM_CTL_NODE *pCtlNode)
{
    AARCHPTR uwChecksum;

    uwChecksum = (((AARCHPTR)pCtlNode->stFreeNodeInfo.pstPrev) ^ \
                        ((AARCHPTR)pCtlNode->stFreeNodeInfo.pstNext) ^ \
                        ((AARCHPTR)pCtlNode->pstPreNode) ^ \
                        (UINT32)(pCtlNode->uwGapSize) ^ \
                        (UINT32)(pCtlNode->uwSizeAndFlag) ^ \
                        (UINT32)0xDEADBEEF);

    PRINT_ERR("node:%p checksum=%p[%p] stFreeNodeInfo.pstPrev=%p stFreeNodeInfo.pstNext=%p pstPreNode=%p uwGapSize=0x%x uwSizeAndFlag=0x%x\n",
        pCtlNode,
        pCtlNode->uwChecksum,
        uwChecksum,
        pCtlNode->stFreeNodeInfo.pstPrev,
        pCtlNode->stFreeNodeInfo.pstNext,
        pCtlNode->pstPreNode,
        pCtlNode->uwGapSize,
        pCtlNode->uwSizeAndFlag
    );
}

STATIC INLINE VOID osMemDispMoreDetails(LOS_MEM_DYN_NODE *pstNode)
{
    UINT32 uwTaskID;
    LOS_TASK_CB *pstTaskCB;

    if( NULL == pstNode)
    {
        PRINT_ERR("node's address is NULL\n");
        return;
    }

    PRINT_ERR("************************************************\n");
    osMemDispCtlNode(&pstNode->stSelfNode);
    PRINT_ERR("node's address: 0x%x\n", pstNode);

    if (!OS_MEM_NODE_GET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag)) {
        PRINT_ERR("this is a FREE node\n");
        PRINT_ERR("************************************************\n\n");
        return;
    }

    uwTaskID = OS_MEM_TASKID_GET(pstNode);
    if (uwTaskID >= g_uwTskMaxNum) {
        PRINT_ERR("The task [ID: 0x%x] is ILLEGAL\n", uwTaskID);
        if (uwTaskID == g_uwTskMaxNum) {
            PRINT_ERR("PROBABLY alloc by SYSTEM INIT, NOT IN ANY TASK\n");
        }
        PRINT_ERR("************************************************\n\n");
        return;
    }

    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);
    if ((pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED) ||
        (pstTaskCB->pfnTaskEntry == NULL) ||
        (pstTaskCB->pcTaskName == NULL)) {
        PRINT_ERR("The task [ID: 0x%x] is NOT CREATED(ILLEGAL)\n", uwTaskID);
        PRINT_ERR("************************************************\n\n");
        return;
    }

    PRINT_ERR("allocated by task: %s [ID = 0x%x]\n", pstTaskCB->pcTaskName, uwTaskID);
#ifdef LOSCFG_MEM_MUL_MODULE
    PRINT_ERR("allocated by moduleID: %d\n", OS_MEM_MODID_GET(pstNode));
#endif

    PRINT_ERR("************************************************\n\n");
}

STATIC INLINE VOID osMemDispWildPointerMsg(LOS_MEM_DYN_NODE *pstNode, VOID *pMem)
{
    PRINT_ERR("*****************************************************\n");
    PRINT_ERR("find an control block at: %p, gap size: 0x%x, sizeof(LOS_MEM_DYN_NODE): 0x%x\n", pstNode, pstNode->stSelfNode.uwGapSize, sizeof(LOS_MEM_DYN_NODE));
    PRINT_ERR("the pointer should be: %p\n",
            (AARCHPTR)pstNode + pstNode->stSelfNode.uwGapSize + sizeof(LOS_MEM_DYN_NODE));
    PRINT_ERR("the pointer given is: %p\n", pMem);
    PRINT_ERR("PROBABLY A WILD POINTER\n");
    osBackTrace();
    PRINT_ERR("*****************************************************\n\n");
}

STATIC INLINE VOID osMemChecksumSet(LOS_MEM_CTL_NODE *pCtlNode)
{
    pCtlNode->uwChecksum = (((AARCHPTR)pCtlNode->stFreeNodeInfo.pstNext) ^ \
        ((AARCHPTR)pCtlNode->stFreeNodeInfo.pstPrev) ^ \
        ((AARCHPTR)pCtlNode->pstPreNode) ^ \
        (UINT32)(pCtlNode->uwGapSize) ^ \
        (UINT32)(pCtlNode->uwSizeAndFlag) ^ \
        (UINT32)0xDEADBEEF);
}

STATIC INLINE BOOL osMemChecksumVerify(LOS_MEM_CTL_NODE *pCtlNode)
{
    return pCtlNode->uwChecksum == (((AARCHPTR)pCtlNode->stFreeNodeInfo.pstNext) ^ \
            ((AARCHPTR)pCtlNode->stFreeNodeInfo.pstPrev) ^ \
            ((AARCHPTR)pCtlNode->pstPreNode) ^ \
            (UINT32)(pCtlNode->uwGapSize) ^ \
            (UINT32)(pCtlNode->uwSizeAndFlag) ^ \
            (UINT32)0xDEADBEEF);

}

STATIC INLINE VOID osMemBackupSetup(LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pstNodePre = pstNode->stSelfNode.pstPreNode;

    if (pstNodePre == NULL) {
        PRINT_ERR("pstNodePre is NULL\n");
        osMemDispCtlNode(&(pstNode->stSelfNode));
        return;
    }

    pstNodePre->stBackupNode.stFreeNodeInfo.pstNext = pstNode->stSelfNode.stFreeNodeInfo.pstNext;
    pstNodePre->stBackupNode.stFreeNodeInfo.pstPrev = pstNode->stSelfNode.stFreeNodeInfo.pstPrev;
    pstNodePre->stBackupNode.pstPreNode             = pstNode->stSelfNode.pstPreNode;
    pstNodePre->stBackupNode.uwChecksum             = pstNode->stSelfNode.uwChecksum;
    pstNodePre->stBackupNode.uwGapSize              = pstNode->stSelfNode.uwGapSize;
#ifdef LOSCFG_MEM_RECORDINFO
    pstNodePre->stBackupNode.uwOriginSize           = pstNode->stSelfNode.uwOriginSize;
#endif
    pstNodePre->stBackupNode.uwSizeAndFlag          = pstNode->stSelfNode.uwSizeAndFlag;
}

LOS_MEM_DYN_NODE *osMemNodeNextGet(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;

    if (pstNode == OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize)) {
        return OS_MEM_FIRST_NODE(pPool);
    } else {
        return OS_MEM_NEXT_NODE(pstNode);
    }
}

STATIC INLINE UINT32 osMemBackupSetup4Next(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pNodeNext = osMemNodeNextGet(pPool, pstNode);

    if (!osMemChecksumVerify(&pNodeNext->stSelfNode)) {
        PRINT_ERR("bad next node's selfNode in [%s]\n", __FUNCTION__);
        osMemDispCtlNode(&(pNodeNext->stSelfNode));
        PRINT_ERR("preNode's details:\n");
        osMemDispMoreDetails(pstNode);

        return LOS_NOK;
    }

    if (!osMemChecksumVerify(&pstNode->stBackupNode)) {
        pstNode->stBackupNode.stFreeNodeInfo.pstNext    = pNodeNext->stSelfNode.stFreeNodeInfo.pstNext;
        pstNode->stBackupNode.stFreeNodeInfo.pstPrev    = pNodeNext->stSelfNode.stFreeNodeInfo.pstPrev;
        pstNode->stBackupNode.pstPreNode                = pNodeNext->stSelfNode.pstPreNode;
        pstNode->stBackupNode.uwChecksum                = pNodeNext->stSelfNode.uwChecksum;
        pstNode->stBackupNode.uwGapSize                 = pNodeNext->stSelfNode.uwGapSize;
#ifdef LOSCFG_MEM_RECORDINFO
        pstNode->stBackupNode.uwOriginSize              = pNodeNext->stSelfNode.uwOriginSize;
#endif
        pstNode->stBackupNode.uwSizeAndFlag             = pNodeNext->stSelfNode.uwSizeAndFlag;
    }
    return LOS_OK;
}

UINT32 osMemBackupDoRestore(VOID *pPool, LOS_MEM_DYN_NODE *pstNodePre, LOS_MEM_DYN_NODE *pstNode)
{
    if( NULL == pstNode || NULL == pstNodePre)
    {
        PRINT_ERR("node's address is NULL, pstNode:0x%x, pstNodePre:0x%x\n", pstNode, pstNodePre);
        return LOS_NOK;
    }
    PRINT_ERR("backup for this node:\n");
    osMemDispCtlNode(&pstNodePre->stBackupNode);
    PRINT_ERR("preNode's details:\n");
    osMemDispMoreDetails(pstNodePre);

    pstNode->stSelfNode.stFreeNodeInfo.pstNext  = pstNodePre->stBackupNode.stFreeNodeInfo.pstNext;
    pstNode->stSelfNode.stFreeNodeInfo.pstPrev  = pstNodePre->stBackupNode.stFreeNodeInfo.pstPrev;
    pstNode->stSelfNode.pstPreNode              = pstNodePre->stBackupNode.pstPreNode;
    pstNode->stSelfNode.uwChecksum              = pstNodePre->stBackupNode.uwChecksum;
    pstNode->stSelfNode.uwGapSize               = pstNodePre->stBackupNode.uwGapSize;
#ifdef LOSCFG_MEM_RECORDINFO
    pstNode->stSelfNode.uwOriginSize          = pstNodePre->stBackupNode.uwOriginSize;
#endif
    pstNode->stSelfNode.uwSizeAndFlag           = pstNodePre->stBackupNode.uwSizeAndFlag;

    /* we should re-setup next node's backup on current node*/
    return osMemBackupSetup4Next(pPool, pstNode);
}

LOS_MEM_DYN_NODE *osMemNodePrevGet(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pstNodeCur = NULL;
    LOS_MEM_DYN_NODE *pstNodePre = NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;

    if (pstNode == OS_MEM_FIRST_NODE(pPool)) {
        pstNodePre = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        if (!osMemChecksumVerify(&(pstNodePre->stSelfNode))) {
            PRINT_ERR("current node is THE FIRST NODE!\n");
            PRINT_ERR("bad preNode's selfNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodePre->stSelfNode));
            return pstNodePre;
        }
        if (!osMemChecksumVerify(&(pstNodePre->stBackupNode))) {
            PRINT_ERR("current node is THE FIRST NODE!\n");
            PRINT_ERR("bad preNode's backupNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodePre->stBackupNode));
            return pstNodePre;
        }
        return NULL;
    }

    for (pstNodeCur = OS_MEM_FIRST_NODE(pPool); \
        pstNodeCur < OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize); \
        pstNodeCur = OS_MEM_NEXT_NODE(pstNodeCur)) {
        if (!osMemChecksumVerify(&(pstNodeCur->stSelfNode))) {
            PRINT_ERR("bad node's selfNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodeCur->stSelfNode));

            if (NULL == pstNodePre) {
                return NULL;
            }

            PRINT_ERR("preNode's details:\n");
            osMemDispMoreDetails(pstNodePre);

            /* due to the every step's checksum verify, pstNodePre is trustful */
            if (osMemBackupDoRestore(pPool, pstNodePre, pstNodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (!osMemChecksumVerify(&(pstNodeCur->stBackupNode))) {
            PRINT_ERR("bad node's backupNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodeCur->stBackupNode));

            if (NULL != pstNodePre) {
                PRINT_ERR("preNode's details:\n");
                osMemDispMoreDetails(pstNodePre);
            }

            if (osMemBackupSetup4Next(pPool, pstNodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (OS_MEM_NEXT_NODE(pstNodeCur) == pstNode) {
            return pstNodeCur;
        }

        if (OS_MEM_NEXT_NODE(pstNodeCur) > pstNode) {
            break;
        }

        pstNodePre = pstNodeCur;
    }

    return NULL;
}

LOS_MEM_DYN_NODE *osMemNodePrevTryGet(VOID *pPool, LOS_MEM_DYN_NODE **ppstNode, VOID *pMem)
{
    UINT32 uwMemShoudBe = 0;
    LOS_MEM_DYN_NODE *pstNodeCur = NULL;
    LOS_MEM_DYN_NODE *pstNodePre = NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;

    if (pMem == OS_MEM_FIRST_NODE(pPool)) {
        pstNodePre = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        if (!osMemChecksumVerify(&(pstNodePre->stSelfNode))) {
            PRINT_ERR("current node is THE FIRST NODE!\n");
            PRINT_ERR("bad preNode's selfNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodePre->stSelfNode));
            return pstNodePre;
        }
        if (!osMemChecksumVerify(&(pstNodePre->stBackupNode))) {
            PRINT_ERR("current node is THE FIRST NODE!\n");
            PRINT_ERR("bad preNode's backupNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodePre->stBackupNode));
            return pstNodePre;
        }
        return NULL;
    }

    for (pstNodeCur = OS_MEM_FIRST_NODE(pPool); \
        pstNodeCur < OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize); \
        pstNodeCur = OS_MEM_NEXT_NODE(pstNodeCur)) {
        if (!osMemChecksumVerify(&(pstNodeCur->stSelfNode))) {
            PRINT_ERR("bad node's selfNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodeCur->stSelfNode));

            if (NULL == pstNodePre) {
                return NULL;
            }

            PRINT_ERR("preNode's details:\n");
            osMemDispMoreDetails(pstNodePre);

            /* due to the every step's checksum verify, pstNodePre is trustful */
            if (osMemBackupDoRestore(pPool, pstNodePre, pstNodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (!osMemChecksumVerify(&(pstNodeCur->stBackupNode))) {
            PRINT_ERR("bad node's backupNode in [%s]\n", __FUNCTION__);
            osMemDispCtlNode(&(pstNodeCur->stBackupNode));

            if (NULL != pstNodePre) {
                PRINT_ERR("preNode's details:\n");
                osMemDispMoreDetails(pstNodePre);
            }

            if (osMemBackupSetup4Next(pPool, pstNodeCur) != LOS_OK) {
                return NULL;
            }
        }

        uwMemShoudBe = (AARCHPTR)pstNodeCur + pstNodeCur->stSelfNode.uwGapSize + sizeof(LOS_MEM_DYN_NODE);
        if (uwMemShoudBe == (AARCHPTR)pMem) {
            *ppstNode = pstNodeCur;
            return pstNodePre;
        }

        if (OS_MEM_NEXT_NODE(pstNodeCur) > (LOS_MEM_DYN_NODE *)pMem) {
            osMemDispWildPointerMsg(pstNodeCur, pMem);
            break;
        }

        pstNodePre = pstNodeCur;
    }

    return NULL;
}

STATIC INLINE UINT32 osMemBackupTryRestore(VOID *pPool, LOS_MEM_DYN_NODE **ppstNode, VOID *pMem)
{
    LOS_MEM_DYN_NODE *pstNodeHead = NULL;
    LOS_MEM_DYN_NODE *pstNodePre = osMemNodePrevTryGet(pPool, &pstNodeHead, pMem);

    if (pstNodePre == NULL) {
        return LOS_NOK;
    }

    *ppstNode = pstNodeHead;
    return osMemBackupDoRestore(pPool, pstNodePre, *ppstNode);
}

STATIC INLINE UINT32 osMemBackupRestore(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pstNodePre = osMemNodePrevGet(pPool, pstNode);

    if (NULL == pstNodePre) {
        return LOS_NOK;
    }

    return osMemBackupDoRestore(pPool, pstNodePre, pstNode);
}

STATIC INLINE VOID osMemSetGapSize(LOS_MEM_CTL_NODE *pCtlNode, UINT32 uwGapSize)
{
    pCtlNode->uwGapSize = uwGapSize;
}

static VOID osMemNodeSave(LOS_MEM_DYN_NODE *pstNode)
{
    osMemSetGapSize(&(pstNode->stSelfNode), 0);
    osMemChecksumSet(&(pstNode->stSelfNode));
    osMemBackupSetup(pstNode);
}

static VOID osMemNodeSaveWithGapSize(LOS_MEM_DYN_NODE *pstNode, UINT32 uwGapSize)
{
    osMemSetGapSize(&(pstNode->stSelfNode), uwGapSize);
    osMemChecksumSet(&(pstNode->stSelfNode));
    osMemBackupSetup(pstNode);
}

static VOID osMemListDelete(LOS_DL_LIST *pstNode, VOID *pFirstNode)
{
    LOS_MEM_DYN_NODE *pstDynNode = NULL;

    pstNode->pstNext->pstPrev = pstNode->pstPrev;
    pstNode->pstPrev->pstNext = pstNode->pstNext;

    if ((VOID *)(pstNode->pstNext) >= (VOID *)pFirstNode) {
        pstDynNode = LOS_DL_LIST_ENTRY(pstNode->pstNext, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo);
        osMemNodeSave(pstDynNode);
    }

    if ((VOID *)(pstNode->pstPrev) >= (VOID *)pFirstNode) {
        pstDynNode = LOS_DL_LIST_ENTRY(pstNode->pstPrev, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo);
        osMemNodeSave(pstDynNode);
    }

    pstNode->pstNext = (LOS_DL_LIST *)NULL;
    pstNode->pstPrev = (LOS_DL_LIST *)NULL;

    pstDynNode = LOS_DL_LIST_ENTRY(pstNode, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo);
    osMemNodeSave(pstDynNode);
}

static VOID osMemListAdd(LOS_DL_LIST *pstList, LOS_DL_LIST *pstNode, VOID *pFirstNode)
{
    LOS_MEM_DYN_NODE *pstDynNode = NULL;

    pstNode->pstNext = pstList->pstNext;
    pstNode->pstPrev = pstList;

    pstDynNode = LOS_DL_LIST_ENTRY(pstNode, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo);
    osMemNodeSave(pstDynNode);

    pstList->pstNext->pstPrev = pstNode;
    if ((VOID *)(pstList->pstNext) >= pFirstNode) {
        pstDynNode = LOS_DL_LIST_ENTRY(pstList->pstNext, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo);
        osMemNodeSave(pstDynNode);
    }

    pstList->pstNext = pstNode;
}

VOID LOS_MemBadNodeShow(VOID *pPool)
{

    LOS_MEM_DYN_NODE *pstNodePre = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINTPTR uvIntSave;

    if (pPool == NULL) {
        return;
    }

    uvIntSave = LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode <= OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {
        osMemDispCtlNode(&pstTmpNode->stSelfNode);

        if (osMemChecksumVerify(&pstTmpNode->stSelfNode)) {
            continue;
        }

        pstNodePre = osMemNodePrevGet(pPool, pstTmpNode);
        if (NULL == pstNodePre) {
            PRINT_ERR("current node is invalid, but cannot find preNode\n");
            continue;
        }

        PRINT_ERR("preNode's details:\n");
        osMemDispMoreDetails(pstNodePre);
    }

    LOS_IntRestore(uvIntSave);
    PRINTK("check finish\n");
}

#else // without LOSCFG_MEM_HEAD_BACKUP

static VOID osMemListDelete(LOS_DL_LIST *pstNode, VOID *pFirstNode)
{
    pstNode->pstNext->pstPrev = pstNode->pstPrev;
    pstNode->pstPrev->pstNext = pstNode->pstNext;
    pstNode->pstNext = (LOS_DL_LIST *)NULL;
    pstNode->pstPrev = (LOS_DL_LIST *)NULL;
}

static VOID osMemListAdd(LOS_DL_LIST *pstList, LOS_DL_LIST *pstNode, VOID *pFirstNode)
{
    pstNode->pstNext = pstList->pstNext;
    pstNode->pstPrev = pstList;
    pstList->pstNext->pstPrev = pstNode;
    pstList->pstNext = pstNode;
}

#endif
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

    m_aucSysMem1 = (UINT8 *)((uwMemStart + (64 - 1)) & ~((AARCHPTR)(64 - 1)));
    uwMemSize = OS_SYS_MEM_SIZE;
    uwRet = LOS_MemInit(m_aucSysMem1, uwMemSize);
    PRINT_INFO("LiteOS system heap memory address:%p,size:0x%x\n", m_aucSysMem1, uwMemSize);
#ifndef LOSCFG_EXC_INTERACTION
    m_aucSysMem0 = m_aucSysMem1;
#endif
    return uwRet;
}


#if OS_SYS_NOCACHEMEM_SIZE
UINT32 osNocacheMemSystemInit(VOID)
{
    UINT32 uwRet;
    m_aucSysNoCacheMem0 = (g_sys_mem_addr_end - OS_SYS_NOCACHEMEM_SIZE);
    uwRet = LOS_MemInit(m_aucSysNoCacheMem0, OS_SYS_NOCACHEMEM_SIZE);
    return uwRet;
}
#endif

VOID osMemInfoPrint(VOID *pPool);
VOID *osMemFindNodeCtrl(VOID *pPtr);

#define osCheckNULL(para)  \
                    if (NULL == para) \
                    { \
                         PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__); \
                         return; \
                     }

/*****************************************************************************
 Function : osMemFindSuitableFreeBlock
 Description : find suitable free block use "best fit" algorithm
 Input       : pPool    --- Pointer to memory pool
                 uwAllocSize  --- Size of memory in bytes which note need allocate
 Output      : None
 Return      :NULL--no suitable block found
                 pstTem--pointer a suitable free block
*****************************************************************************/
STATIC INLINE LOS_MEM_DYN_NODE *osMemFindSuitableFreeBlock(VOID *pPool, UINT32 uwAllocSize)
{
    LOS_DL_LIST *pstListHead = (LOS_DL_LIST *)NULL;
    UINT32 uwMaxCount = (((LOS_MEM_POOL_INFO *)pPool)->uwPoolSize / uwAllocSize) << 1;
    UINT32 uwtCount;

    for (pstListHead = OS_MEM_HEAD(pPool, uwAllocSize); pstListHead != NULL; pstListHead = osDLnkNextMultiHead(OS_MEM_HEAD_ADDR(pPool), pstListHead))
    {
        LOS_MEM_DYN_NODE *pstTmp = (LOS_MEM_DYN_NODE *)NULL;
        uwtCount = 0;
        LOS_DL_LIST_FOR_EACH_ENTRY(pstTmp, pstListHead, LOS_MEM_DYN_NODE, stSelfNode.stFreeNodeInfo) /*lint !e413*/
        {
            if (uwtCount++ >= uwMaxCount)
            {
                PRINT_ERR("[%s:%d]node: %p execute too much time\n", __FILE__, __LINE__, pstTmp);
                break;
            }

#ifdef LOSCFG_MEM_HEAD_BACKUP
            if (!osMemChecksumVerify(&pstTmp->stSelfNode)) {
                PRINT_ERR("bad selfNode in [%s]\n", __FUNCTION__);
                osMemDispCtlNode(&pstTmp->stSelfNode);

                if (osMemBackupRestore(pPool, pstTmp) != LOS_OK) {
                    break;
                }
            }
#endif

            if ((AARCHPTR)pstTmp < (AARCHPTR)pPool || (AARCHPTR)pstTmp > ((AARCHPTR)pPool + ((LOS_MEM_POOL_INFO *)pPool)->uwPoolSize) ||
                0 != ((AARCHPTR)pstTmp & (OS_MEM_ALIGN_SIZE - 1)))
            {
                PRINT_ERR("[%s:%d]Mem node data error:%p, uwAllocSize=%d, pstTmp=%p\n", __FUNCTION__, __LINE__, pPool, uwAllocSize, pstTmp);
                break;
            }
            if (pstTmp->stSelfNode.uwSizeAndFlag >= uwAllocSize)
            {
                return pstTmp;
            }
        }
    }

    return (LOS_MEM_DYN_NODE *)NULL;
}

/*****************************************************************************
 Function : osMemClearNode
 Description : clear a mem Node , set every member to NULL
 Input       : pstNode    --- Pointer to the mem node which will be cleared up
 Output      : None
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemClearNode(LOS_MEM_DYN_NODE *pstNode)
{
    memset((VOID *)pstNode, 0, sizeof(LOS_MEM_DYN_NODE));
}

/*****************************************************************************
 Function : osMemMergeNode
 Description : merge this node and pre node ,then clear this node info
 Input       : pstNode    --- Pointer to node which will be merged
 Output      : None
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemMergeNode(LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pstNextNode = (LOS_MEM_DYN_NODE *)NULL;

    (pstNode->stSelfNode.pstPreNode)->stSelfNode.uwSizeAndFlag += pstNode->stSelfNode.uwSizeAndFlag;
    pstNextNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pstNode + pstNode->stSelfNode.uwSizeAndFlag);
    pstNextNode->stSelfNode.pstPreNode = pstNode->stSelfNode.pstPreNode;
#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstNode->stSelfNode.pstPreNode);
    osMemNodeSave(pstNextNode);
#endif
    osMemClearNode(pstNode);
}

/*****************************************************************************
 Function : osMemSpitNode
 Description : spit new node from pstAllocNode, and merge remainder mem if necessary
 Input       : pPool --Pointer to memory pool
                  pstAllocNode --the source node which new node be spit from to.
                                        After pick up it's node info, change to point the new node
                  uwAllocSize -- the size of new node
 Output      : pstAllocNode -- save new node addr
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemSpitNode(VOID *pPool,
                            LOS_MEM_DYN_NODE *pstAllocNode, UINT32 uwAllocSize)
{
    LOS_MEM_DYN_NODE *pstNewFreeNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_DYN_NODE *pstNextNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_DL_LIST *pstListHead = (LOS_DL_LIST *)NULL;
    VOID *pFirstNode = (VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pPool) + OS_DLNK_HEAD_SIZE);

    pstNewFreeNode = (LOS_MEM_DYN_NODE *)((UINT8 *)pstAllocNode + uwAllocSize);
    pstNewFreeNode->stSelfNode.pstPreNode = pstAllocNode;
    pstNewFreeNode->stSelfNode.uwSizeAndFlag = pstAllocNode->stSelfNode.uwSizeAndFlag - uwAllocSize;
    pstAllocNode->stSelfNode.uwSizeAndFlag = uwAllocSize;
    pstNextNode = OS_MEM_NEXT_NODE(pstNewFreeNode);
    pstNextNode->stSelfNode.pstPreNode = pstNewFreeNode;
    if (!OS_MEM_NODE_GET_USED_FLAG(pstNextNode->stSelfNode.uwSizeAndFlag))
    {
        osMemListDelete(&(pstNextNode->stSelfNode.stFreeNodeInfo), pFirstNode);
        osMemMergeNode(pstNextNode);
    }
#ifdef LOSCFG_MEM_HEAD_BACKUP
    else
    {
        osMemNodeSave(pstNextNode);
    }
#endif
    pstListHead = OS_MEM_HEAD(pPool, pstNewFreeNode->stSelfNode.uwSizeAndFlag);
    osCheckNULL(pstListHead);

    osMemListAdd(pstListHead,&(pstNewFreeNode->stSelfNode.stFreeNodeInfo), pFirstNode);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstNewFreeNode);
#endif
}

/*****************************************************************************
 Function : osMemFreeNode
 Description : free the node from memory & if there are free node beside, merger them.
                    at last update "pstListHead' which saved all free node control head
 Input       : pstNode -- the node which need be freed
                  pPool --Pointer to memory pool
 Output      : None
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemFreeNode(LOS_MEM_DYN_NODE *pstNode, VOID *pPool)
{
    LOS_MEM_DYN_NODE *pstNextNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_DL_LIST *pstListHead = (LOS_DL_LIST *)NULL;
    VOID *pFirstNode = (VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pPool) + OS_DLNK_HEAD_SIZE);

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    LOS_MEM_POOL_INFO *pPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    pPoolInfo->uwPoolCurUsedSize -= OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag);
#endif
    if (pPool == (VOID *)OS_SYS_MEM_ADDR || pPool == (VOID *)m_aucSysMem0)
        OS_MEM_REDUCE_USED(OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag), OS_MEM_TASKID_GET(pstNode));

    pstNode->stSelfNode.uwSizeAndFlag = OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstNode);
#endif
    if ((pstNode->stSelfNode.pstPreNode != NULL) &&
        (!OS_MEM_NODE_GET_USED_FLAG((pstNode->stSelfNode.pstPreNode)->stSelfNode.uwSizeAndFlag)))
    {
        LOS_MEM_DYN_NODE *pstPreNode = pstNode->stSelfNode.pstPreNode;
        osMemMergeNode(pstNode);
        pstNextNode = OS_MEM_NEXT_NODE(pstPreNode);
        if (!OS_MEM_NODE_GET_USED_FLAG(pstNextNode->stSelfNode.uwSizeAndFlag))
        {
            osMemListDelete(&(pstNextNode->stSelfNode.stFreeNodeInfo), pFirstNode);
            osMemMergeNode(pstNextNode);
        }

        osMemListDelete(&(pstPreNode->stSelfNode.stFreeNodeInfo), pFirstNode);
        pstListHead = OS_MEM_HEAD(pPool, pstPreNode->stSelfNode.uwSizeAndFlag);
        osCheckNULL(pstListHead);

        osMemListAdd(pstListHead,&(pstPreNode->stSelfNode.stFreeNodeInfo), pFirstNode);
    }
    else
    {
        pstNextNode = OS_MEM_NEXT_NODE(pstNode);
        if (!OS_MEM_NODE_GET_USED_FLAG(pstNextNode->stSelfNode.uwSizeAndFlag))
        {
            osMemListDelete(&(pstNextNode->stSelfNode.stFreeNodeInfo), pFirstNode);
            osMemMergeNode(pstNextNode);
        }

        pstListHead = OS_MEM_HEAD(pPool, pstNode->stSelfNode.uwSizeAndFlag);
        osCheckNULL(pstListHead);

        osMemListAdd(pstListHead,&(pstNode->stSelfNode.stFreeNodeInfo), pFirstNode);
    }
}

/*****************************************************************************
 Function : osMemCheckUsedNode
 Description : check the result if pointer memory node belongs to pointer memory pool
 Input       : pPool --Pointer to memory pool
                  pstNode -- the node which need be checked
 Output      : None
 Return      : LOS_OK or LOS_NOK
*****************************************************************************/
#ifdef LOS_DLNK_SAFE_CHECK
STATIC INLINE UINT32 osMemCheckUsedNode(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_DYN_NODE *pstTmp = NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstEnd = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);

    for (pstTmp = OS_MEM_FIRST_NODE(pPool); pstTmp < pstEnd; pstTmp = OS_MEM_NEXT_NODE(pstTmp ))
    {
        if ((pstTmp == pstNode) &&
            OS_MEM_NODE_GET_USED_FLAG(pstTmp->stSelfNode.uwSizeAndFlag))
        {
            return LOS_OK;
        }
        else if (pstTmp > pstNode)
        {
            return LOS_NOK;
        }
    }

    return LOS_NOK;
}

#elif defined(LOS_DLNK_SIMPLE_CHECK)
STATIC INLINE UINT32 osMemCheckUsedNode(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstStartNode = OS_MEM_FIRST_NODE(pPool);
    LOS_MEM_DYN_NODE *pstEndNode = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
    if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pstStartNode, pstNode, pstEndNode))
    {
        return LOS_NOK;
    }

    if (!OS_MEM_NODE_GET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag))
    {
        return LOS_NOK;
    }

    if ((!OS_MEM_MAGIC_VALID(pstNode->stSelfNode.stFreeNodeInfo.pstPrev)))
    {
        return LOS_NOK;
    }

    return LOS_OK;
}

#else
STATIC INLINE BOOL osMemIsNodeValid(const LOS_MEM_DYN_NODE *pstNode, const LOS_MEM_DYN_NODE *pstStartNode, const LOS_MEM_DYN_NODE *pstEndNode,
       const UINT8 *pucStartPool, const UINT8 *pucEndPool)
{
    if (!OS_MEM_MIDDLE_ADDR(pstStartNode, pstNode, pstEndNode))
    {
        return FALSE;
    }

    if (OS_MEM_NODE_GET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag))
    {
        if ((!OS_MEM_MAGIC_VALID(pstNode->stSelfNode.stFreeNodeInfo.pstPrev)))
        {
            return FALSE;
        }
        return TRUE;
    }

    if ((!OS_MEM_MIDDLE_ADDR_OPEN_END(pucStartPool, pstNode->stSelfNode.stFreeNodeInfo.pstPrev, pucEndPool)))
    {
        return FALSE;
    }

    return TRUE;
}

STATIC INLINE UINT32 osMemCheckUsedNode(VOID *pPool, LOS_MEM_DYN_NODE *pstNode)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstStartNode = OS_MEM_FIRST_NODE(pPool);
    LOS_MEM_DYN_NODE *pstEndNode = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
    UINT8 *pucEndPool = (UINT8 *)pPool + pstPoolInfo->uwPoolSize;
    const LOS_MEM_DYN_NODE *pstNextNode = (const LOS_MEM_DYN_NODE *)NULL;
    if (!osMemIsNodeValid(pstNode, pstStartNode, pstEndNode, (UINT8 *)pPool, pucEndPool))
    {
        return LOS_NOK;
    }

    if (!OS_MEM_NODE_GET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag))

    {
        return LOS_NOK;
    }

    pstNextNode = OS_MEM_NEXT_NODE(pstNode);
    if (!osMemIsNodeValid(pstNextNode, pstStartNode, pstEndNode, (UINT8 *)pPool, pucEndPool))
    {
        return LOS_NOK;
    }

    if (pstNextNode->stSelfNode.pstPreNode != pstNode)
    {
        return LOS_NOK;
    }

    if (pstNode != pstStartNode)
    {
        if (!osMemIsNodeValid(pstNode->stSelfNode.pstPreNode, pstStartNode, pstEndNode, (UINT8 *)pPool, pucEndPool))
        {
            return LOS_NOK;
        }

        if (OS_MEM_NEXT_NODE(pstNode->stSelfNode.pstPreNode) != pstNode)
        {
            return LOS_NOK;
        }
    }

    return LOS_OK;
}

#endif

/*****************************************************************************
 Function : osMemSetMagicNumAndTaskid
 Description : set magic & taskid
 Input       : pstNode -- the node which will be set magic &  taskid
 Output      : None
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemSetMagicNumAndTaskid(LOS_MEM_DYN_NODE *pstNode)
{
    OS_MEM_SET_MAGIC(pstNode->stSelfNode.stFreeNodeInfo.pstPrev);

    /* If the operation occured before task initialization(g_stLosTask.pstRunTask was not assigned)
       or in interrupt,make the value of taskid of pstNode to oxffffffff*/
    if (g_stLosTask.pstRunTask != NULL && OS_INT_INACTIVE)
    {
        OS_MEM_TASKID_SET(pstNode, g_stLosTask.pstRunTask->uwTaskID);
    }
    else
    {
        /* If the task mode does not initialize, the field is the 0xffffffff */
        pstNode->stSelfNode.stFreeNodeInfo.pstNext = (LOS_DL_LIST *)OS_NULL_INT;
        /* TODO: the commend task-MEMUSE is not include system initialization malloc */
    }
}

/*****************************************************************************
 Function : LOS_MemIntegrityCheck
 Description : memory pool integrity checking
 Input       : pPool --Pointer to memory pool
 Output      : None
 Return      : LOS_OK --memory pool integrate  or LOS_NOK--memory pool impaired
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemIntegrityCheck(VOID *pPool)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_DYN_NODE *pstPreNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINT8 *pEndPool;
    LOS_TASK_CB *pstTaskCB;
    UINT32 uwTaskID;
    UINTPTR uvIntSave;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    pEndPool = (UINT8 *)pPool + pstPoolInfo->uwPoolSize;

    uvIntSave = LOS_IntLock();
    pstPreNode = OS_MEM_FIRST_NODE(pPool);
    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode < OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {
        if (OS_MEM_NODE_GET_USED_FLAG(pstTmpNode->stSelfNode.uwSizeAndFlag))
        {
            if (!OS_MEM_MAGIC_VALID(pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev))
            {
                PRINT_ERR("[%s], %d, memory check error!\n"
                    "memory used but magic num wrong, stFreeNodeInfo.pstPrev(magic num):%p \n",
                    __FUNCTION__, __LINE__, pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev);
                goto errout;
            }
        }
        else //is free node, check free node range
        {
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pPool, pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev, pEndPool))
            {
                PRINT_ERR("[%s], %d, memory check error!\n"
                    "stFreeNodeInfo.pstPrev:%p is out of legal mem range[%p, %p]\n",
                    __FUNCTION__, __LINE__, pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev, pPool, pEndPool);
                goto errout;
            }
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pPool, pstTmpNode->stSelfNode.stFreeNodeInfo.pstNext, pEndPool))
            {
                PRINT_ERR("[%s], %d, memory check error!\n"
                    "stFreeNodeInfo.pstNext:%p is out of legal mem range[%p, %p]\n",
                    __FUNCTION__, __LINE__, pstTmpNode->stSelfNode.stFreeNodeInfo.pstNext, pPool, pEndPool);
                goto errout;
            }
        }

        pstPreNode = pstTmpNode;
    }
    LOS_IntRestore(uvIntSave);
    return LOS_OK;

errout:
    PRINTK("broken node head: %p  %p  %p  0x%x\n",
                    pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev, pstTmpNode->stSelfNode.stFreeNodeInfo.pstNext, pstTmpNode->stSelfNode.pstPreNode, pstTmpNode->stSelfNode.uwSizeAndFlag);
    LOS_IntRestore(uvIntSave);
    uwTaskID = OS_MEM_TASKID_GET(pstPreNode);
    if (uwTaskID >= g_uwTskMaxNum)
    {
        LOS_Panic("Task ID %d in pre node is invalid!\n", uwTaskID);
        return LOS_NOK;
    }
    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

    if ((pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED) ||
        (pstTaskCB->pfnTaskEntry == NULL) ||
        (pstTaskCB->pcTaskName == NULL))
    {
        LOS_Panic("\r\nTask ID %d in pre node is not created!\n", uwTaskID);
        return LOS_NOK;
    }
    LOS_Panic("cur node: %p\n"
               "pre node: %p\n"
               "pre node was allocated by task:%s\n",
               pstTmpNode, pstPreNode,  pstTaskCB->pcTaskName); /*lint !e515*/
    return LOS_NOK;
}

/*****************************************************************************
 Function : osMemAllocWithCheck
 Description : Allocate node from Memory pool
 Input       : pPool    --- Pointer to memory pool
                 uwSize  --- Size of memory in bytes to allocate
 Output      : None
 Return      : Pointer to allocated memory
*****************************************************************************/
STATIC INLINE VOID *osMemAllocWithCheck(VOID *pPool, UINT32  uwSize)
{
    LOS_MEM_DYN_NODE *pstAllocNode = (LOS_MEM_DYN_NODE *)NULL;
    UINT32 uwAllocSize;
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    LOS_MEM_POOL_INFO *pPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
#endif
    VOID *pFirstNode = (VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pPool) + OS_DLNK_HEAD_SIZE);

    if (g_MALLOC_HOOK != NULL)
        g_MALLOC_HOOK();

#ifdef LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK
    (VOID)LOS_MemIntegrityCheck(pPool);
#endif

    uwAllocSize = OS_MEM_ALIGN(uwSize + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);
    pstAllocNode = osMemFindSuitableFreeBlock(pPool, uwAllocSize);
    if (pstAllocNode == NULL)
    {
        PRINT_ERR("-----------------------------------------------------------------------------------------------------------\n");
        osMemInfoPrint(pPool);
        PRINT_ERR("[%s] No suitable free block, require free node size: 0x%x\n", __FUNCTION__, uwAllocSize);
        PRINT_ERR("-----------------------------------------------------------------------------------------------------------\n");
        return NULL;
    }
    if ((uwAllocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= pstAllocNode->stSelfNode.uwSizeAndFlag)
    {
        osMemSpitNode(pPool, pstAllocNode, uwAllocSize);
    }
    osMemListDelete(&(pstAllocNode->stSelfNode.stFreeNodeInfo), pFirstNode);
    osMemSetMagicNumAndTaskid(pstAllocNode);
    OS_MEM_NODE_SET_USED_FLAG(pstAllocNode->stSelfNode.uwSizeAndFlag);
    if (pPool == (VOID *)OS_SYS_MEM_ADDR || pPool == (VOID *)m_aucSysMem0)
        OS_MEM_ADD_USED(OS_MEM_NODE_GET_SIZE(pstAllocNode->stSelfNode.uwSizeAndFlag), OS_MEM_TASKID_GET(pstAllocNode));
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pPoolInfo->uwPoolCurUsedSize += OS_MEM_NODE_GET_SIZE(pstAllocNode->stSelfNode.uwSizeAndFlag);
    if (pPoolInfo->uwPoolCurUsedSize > pPoolInfo->uwPoolWaterLine)
    {
        pPoolInfo->uwPoolWaterLine = pPoolInfo->uwPoolCurUsedSize;
    }
#endif
#ifdef LOSCFG_MEM_RECORDINFO
    pstAllocNode->stSelfNode.uwOriginSize = uwSize;
#endif

#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstAllocNode);
#endif

    return (pstAllocNode + 1);
}

/*****************************************************************************
 Function : osMemReAllocSmaller
 Description : reAlloc a smaller memory node
 Input       : pPool    --- Pointer to memory pool
                 uwAllocSize  --- the size of new node which will be alloced
                 pstNode --the node which wille be realloced
                 uwNodeSize -- the size of old node
 Output      : pstNode -- pointer to the new node after realloc
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemReAllocSmaller(VOID *pPool, UINT32 uwAllocSize, LOS_MEM_DYN_NODE *pstNode, UINT32 uwNodeSize)
{
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    LOS_MEM_POOL_INFO *pPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
#endif
    if ((uwAllocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= uwNodeSize)
    {
        pstNode->stSelfNode.uwSizeAndFlag = uwNodeSize;
        osMemSpitNode(pPool, pstNode, uwAllocSize);
        OS_MEM_NODE_SET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
        osMemNodeSave(pstNode);
#endif
        if (pPool == (VOID *)OS_SYS_MEM_ADDR || pPool == (VOID *)m_aucSysMem0)
            OS_MEM_REDUCE_USED(uwNodeSize - uwAllocSize, OS_MEM_TASKID_GET(pstNode));
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
        pPoolInfo->uwPoolCurUsedSize -= (uwNodeSize - uwAllocSize);
#endif
     }
}

/*****************************************************************************
 Function : osMemMergeNodeForReAllocBigger
 Description : reAlloc a Bigger memory node after merge pstNode and nextNode
 Input       : pPool    --- Pointer to memory pool
                 uwAllocSize  --- the size of new node which will be alloced
                 pstNode --the node which wille be realloced
                 uwNodeSize -- the size of old node
                 pstNextNode -- pointer next node which will be merged
 Output      : pstNode -- pointer to the new node after realloc
 Return      : None
*****************************************************************************/
STATIC INLINE VOID osMemMergeNodeForReAllocBigger(VOID *pPool, UINT32 uwAllocSize, LOS_MEM_DYN_NODE *pstNode, UINT32 uwNodeSize, LOS_MEM_DYN_NODE *pstNextNode)
{
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    LOS_MEM_POOL_INFO *pPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
#endif
    VOID *pFirstNode = (VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pPool) + OS_DLNK_HEAD_SIZE);

    pstNode->stSelfNode.uwSizeAndFlag = uwNodeSize;
    osMemListDelete(&(pstNextNode->stSelfNode.stFreeNodeInfo), pFirstNode);
    osMemMergeNode(pstNextNode);
    if ((uwAllocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= pstNode->stSelfNode.uwSizeAndFlag)
    {
        osMemSpitNode(pPool, pstNode, uwAllocSize);
    }
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pPoolInfo->uwPoolCurUsedSize += (pstNode->stSelfNode.uwSizeAndFlag - uwNodeSize);
    if (pPoolInfo->uwPoolCurUsedSize > pPoolInfo->uwPoolWaterLine)
    {
        pPoolInfo->uwPoolWaterLine = pPoolInfo->uwPoolCurUsedSize;
    }
#endif
    if (pPool == (VOID *)OS_SYS_MEM_ADDR || pPool == (VOID *)m_aucSysMem0)
        OS_MEM_ADD_USED(pstNode->stSelfNode.uwSizeAndFlag - uwNodeSize, OS_MEM_TASKID_GET(pstNode));
    OS_MEM_NODE_SET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstNode);
#endif
}

/*****************************************************************************
 Function : LOS_MemInit
 Description : Initialize Dynamic Memory pool
 Input       : pPool    --- Pointer to memory pool
                 uwSize  --- Size of memory in bytes to allocate
 Output      : None
 Return      : LOS_OK - Ok, OS_ERROR - Error
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemInit(VOID *pPool, UINT32  uwSize)
{
    LOS_MEM_DYN_NODE *pstNewNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_DYN_NODE *pstEndNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)NULL;
    UINTPTR uvIntSave;
    LOS_DL_LIST *pstListHead = (LOS_DL_LIST *)NULL;
#ifdef LOSCFG_MEM_MUL_POOL
    VOID *pNext = pool_head;
    VOID * pCur;
    AARCHPTR uwPoolEnd;
#endif

    if ((pPool == NULL) || (uwSize < (OS_MEM_MIN_POOL_SIZE)))
    {
        return OS_ERROR;
    }

    if(!IS_ALIGNED(uwSize, OS_MEM_ALIGN_SIZE))
    {
        PRINT_WARN("pool [%p, %p) size 0x%x sholud be aligned with OS_MEM_ALIGN_SIZE\n",
                  pPool, (AARCHPTR)pPool + uwSize, uwSize);
        uwSize = OS_MEM_ALIGN(uwSize, OS_MEM_ALIGN_SIZE) - OS_MEM_ALIGN_SIZE;
    }
#ifdef LOSCFG_MEM_MUL_POOL
    while (pNext != NULL)
    {
        uwPoolEnd = (AARCHPTR)pNext + LOS_MemPoolSizeGet(pNext);
        if ((pPool <= pNext && ((AARCHPTR)pPool + uwSize) > (AARCHPTR)pNext) ||
            ((AARCHPTR)pPool < uwPoolEnd && ((AARCHPTR)pPool + uwSize) >= uwPoolEnd))
        {
            PRINT_ERR("pool [%p, %p) conflict with pool [%p, %p)\n",
                          pPool, (AARCHPTR)pPool + uwSize,
                          pNext, (AARCHPTR)pNext + LOS_MemPoolSizeGet(pNext));
            return OS_ERROR;
        }
        pCur = pNext;
        pNext = ((LOS_MEM_POOL_INFO *)pNext)->pNextPool;
    }
#endif

    uvIntSave = LOS_IntLock();

    pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    pstPoolInfo->pPoolAddr = pPool;
    pstPoolInfo->uwPoolSize = uwSize;
    osDLnkInitMultiHead(OS_MEM_HEAD_ADDR(pPool));
    pstNewNode = OS_MEM_FIRST_NODE(pPool);
    pstNewNode->stSelfNode.uwSizeAndFlag = ((uwSize - ((AARCHPTR)pstNewNode - (AARCHPTR)pPool)) - OS_MEM_NODE_HEAD_SIZE);
    pstNewNode->stSelfNode.pstPreNode = (LOS_MEM_DYN_NODE *)OS_MEM_END_NODE(pPool, uwSize);
    pstListHead = OS_MEM_HEAD(pPool, pstNewNode->stSelfNode.uwSizeAndFlag);
    if (NULL == pstListHead)
    {
        LOS_IntRestore(uvIntSave);
        return OS_ERROR;
    }

    LOS_ListTailInsert(pstListHead,&(pstNewNode->stSelfNode.stFreeNodeInfo));
    pstEndNode = (LOS_MEM_DYN_NODE *)OS_MEM_END_NODE(pPool, uwSize);
    (VOID)memset(pstEndNode, 0 ,sizeof(*pstEndNode));
    pstEndNode->stSelfNode.pstPreNode = pstNewNode;
    pstEndNode->stSelfNode.uwSizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    OS_MEM_NODE_SET_USED_FLAG(pstEndNode->stSelfNode.uwSizeAndFlag);
    osMemSetMagicNumAndTaskid(pstEndNode);
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pstPoolInfo->uwPoolCurUsedSize = sizeof(LOS_MEM_POOL_INFO) + OS_MULTI_DLNK_HEAD_SIZE + OS_MEM_NODE_GET_SIZE(pstEndNode->stSelfNode.uwSizeAndFlag);
    pstPoolInfo->uwPoolWaterLine = pstPoolInfo->uwPoolCurUsedSize;
#endif
#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstNewNode);
    osMemNodeSave(pstEndNode);
#endif
#ifdef LOSCFG_MEM_MUL_POOL
    if (pool_head == NULL)
    {
        pool_head = pPool;
    }
    else
    {
        ((LOS_MEM_POOL_INFO *)pCur)->pNextPool = pPool;
    }

    ((LOS_MEM_POOL_INFO *)pPool)->pNextPool = NULL;
#endif

    LOS_IntRestore(uvIntSave);

    return LOS_OK;
}

#ifdef LOSCFG_MEM_MUL_POOL
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemDeInit(VOID *pPool)
{
    UINTPTR uvIntSave, uwRet = LOS_NOK;
    VOID *pNext, *pCur;

    uvIntSave = LOS_IntLock();
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

#ifdef LOS_MEM_LEAK_CHECK
LITE_OS_SEC_TEXT VOID LOS_CheckMaxcount(VOID)
{
    if (g_uwCheckFlag == 1)
    {
        PRINTK("\nIt's checking the maxcount now, please waiting!\n");
        return;
    }
    PRINTK("\nthe max malloc count :%d\nplease set the CHECK_MAXCOUNT bigger than %d!\n", g_uwPeakCount, g_uwPeakCount);
}

LITE_OS_SEC_TEXT  VOID LOS_MemLeakCheckStart (VOID)
{
    UINT32 uwRet;
    UINTPTR uvIntSave;

    uvIntSave = LOS_IntLock();
    if (g_uwCheckFlag)
    {
        PRINT_ERR("\n%s was called twice !\n",__FUNCTION__);
        LOS_IntRestore(uvIntSave);
        return;
    }
    if (NULL == g_pMemCheckPool)
    {
        g_pMemCheckPool = LOS_MemAlloc((void *)OS_SYS_MEM_ADDR, MEM_CHECK_POOL_SIZE);
        if (NULL == g_pMemCheckPool)
        {
            PRINT_ERR("\n%s %d,mem check info pool malloc failed !\n",__FUNCTION__,__LINE__);
            LOS_IntRestore(uvIntSave);
            return;
        }
    }
    memset(g_pMemCheckPool, 0, MEM_CHECK_POOL_SIZE);
    uwRet = LOS_MemboxInit(g_pMemCheckPool, MEM_CHECK_POOL_SIZE, MEM_CHECK_SIZE);
    if (uwRet != LOS_OK)
    {
        LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, g_pMemCheckPool);
        PRINT_ERR("\n%s, %d,LOS_MemboxInit failed!\n",__FUNCTION__,__LINE__);
        LOS_IntRestore(uvIntSave);
        return;
    }
    g_uwMemCount = 0;
    g_uwPeakCount = 0;
    g_uwCheckFlag = 1;
    LOS_IntRestore(uvIntSave);
}

STATIC INLINE VOID osMemCheckinfoCreate(VOID *pPtr, UINT32  uwSize)
{
    UINT32 uwFp;
    UINT32 uwIndex = 0;
    UINT32 uwtmpFP;
    UINT32 uwBackLR;
    MEM_CHECK_INFO *pstCheck;
    UINTPTR uvIntSave;

    if (g_uwCheckFlag == 0)
        return;

    if (pPtr == NULL)
        return;

    uwFp = Get_Fp();
    uvIntSave = LOS_IntLock();
    g_uwMemCount++;
    if (g_uwMemCount > g_uwPeakCount)
    {
        g_uwPeakCount = g_uwMemCount;
    }
    LOS_IntRestore(uvIntSave);

    pstCheck = (MEM_CHECK_INFO *)LOS_MemboxAlloc(g_pMemCheckPool);
    if (NULL == pstCheck)
    {
            PRINT_ERR("%s, %d malloc membox failed!\n", __FUNCTION__,__LINE__);
            return;
    }
    pstCheck->pPoolAddr = pPtr;
    while ( uwFp > OS_SYS_FUNC_ADDR_START && uwFp < OS_SYS_FUNC_ADDR_END )
    {
        uwtmpFP = uwFp;
        uwBackLR = *((UINT32 *)(uwFp));
        uwFp = *((UINT32 *)(uwtmpFP - 4));
        if (uwIndex > 0)
        {
            pstCheck->uwFp[uwIndex - 1] = uwBackLR;
        }
        uwIndex++;
        if (uwIndex == 5)
            break;
    }
    pstCheck->Flag = 1;
    pstCheck->uwAddrSize = uwSize;

}

STATIC INLINE VOID osMemCheckinfoUpdate(VOID *pPtr, UINT32  uwSize)
{
    UINT32 uwFp;
    UINT32 uwIndex = 0;
    UINT32 uwtmpFP;
    UINT32 uwBackLR;
    UINT32 uwRet = 0;
    UINTPTR uvIntSave;
    LOS_MEMBOX_INFO *pstBoxInfo = (LOS_MEMBOX_INFO *)g_pMemCheckPool;
    LOS_MEMBOX_NODE *pstNode = (LOS_MEMBOX_NODE *)NULL;
    MEM_CHECK_INFO *pstMemCheckInfo = (MEM_CHECK_INFO *)NULL;

    if (g_uwCheckFlag == 0)
        return;

    uwFp = Get_Fp();
    uvIntSave = LOS_IntLock();
    pstNode = (LOS_MEMBOX_NODE *)(pstBoxInfo + 1);
    for (uwIndex = 0; uwIndex < pstBoxInfo->uwBlkNum; ++uwIndex, pstNode = OS_MEMBOX_NEXT(pstNode, pstBoxInfo->uwBlkSize))
    {
        pstMemCheckInfo = (MEM_CHECK_INFO *)((LOS_MEMBOX_NODE *)pstNode + 1);
        if ((*(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FLAG)) == 1)
        {
            if ((AARCHPTR)pPtr == *((AARCHPTR *)pstMemCheckInfo))
            {
                uwRet = 1;
                break;
            }
        }
    }

    if (uwRet == 0)
    {
        LOS_IntRestore(uvIntSave);
        return;
    }

    *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_ADDR_SIZE) = uwSize;

    uwIndex = 0;
    while ( uwFp > OS_SYS_FUNC_ADDR_START && uwFp < OS_SYS_FUNC_ADDR_END )
    {
        uwtmpFP = uwFp;
        uwBackLR = *((UINT32 *)(uwFp));
        uwFp = *((UINT32 *)(uwtmpFP - 4));
        if ( uwIndex > 1)
        {
            *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FP0 * (uwIndex - 1)) = uwBackLR;
        }
        uwIndex++;
        if (uwIndex == 6)
            break;
    }

    LOS_IntRestore(uvIntSave);

}

STATIC INLINE VOID osMemCheckinfoDel(VOID *pPtr)
{
    UINT32 uwIndex;
    UINT32 uwRet;
    UINTPTR uvIntSave;
    LOS_MEMBOX_INFO *pstBoxInfo = (LOS_MEMBOX_INFO *)g_pMemCheckPool;
    LOS_MEMBOX_NODE *pstNode = (LOS_MEMBOX_NODE *)NULL;
    MEM_CHECK_INFO *pstMemCheckInfo = (MEM_CHECK_INFO *)NULL;

    if (g_uwCheckFlag == 0)
        return;

    uvIntSave = LOS_IntLock();
    pstNode = (LOS_MEMBOX_NODE *)(pstBoxInfo + 1);
    for (uwIndex = 0; uwIndex < pstBoxInfo->uwBlkNum; ++uwIndex, pstNode = OS_MEMBOX_NEXT(pstNode, pstBoxInfo->uwBlkSize))
    {
        pstMemCheckInfo = (MEM_CHECK_INFO *)((LOS_MEMBOX_NODE *)pstNode + 1);
        if ((*(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FLAG)) == 1)
        {
            if ((AARCHPTR)pPtr == *((AARCHPTR *)pstMemCheckInfo))
            {
                g_uwMemCount--;
                uwRet = LOS_MemboxFree(g_pMemCheckPool, (VOID *)pstMemCheckInfo);
                if (uwRet != LOS_OK)
                {
                    PRINT_ERR("free membox failed!\n");
                }
                (*(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FLAG)) = 0;
                break;
            }
        }
    }

    LOS_IntRestore(uvIntSave);
}



LITE_OS_SEC_TEXT VOID LOS_MemLeakCheckEnd(VOID)
{
    UINTPTR uvIntSave;

    uvIntSave = LOS_IntLock();
    g_uwCheckFlag = 0;
    LOS_IntRestore(uvIntSave);
}

LITE_OS_SEC_TEXT VOID LOS_MemLeakCheckShow(VOID)
{
    UINT32 uwIndex ;
    UINT32 uwIndexTmp ;
    UINTPTR uvIntSave;
    LOS_MEMBOX_INFO *pstBoxInfo = (LOS_MEMBOX_INFO *)g_pMemCheckPool;
    LOS_MEMBOX_NODE *pstNode = (LOS_MEMBOX_NODE *)NULL;
    MEM_CHECK_INFO *pstMemCheckInfo = (MEM_CHECK_INFO *)NULL;
    uvIntSave = LOS_IntLock();

    if (g_uwCheckFlag == 1)
    {
        PRINTK("\nIt's checking mem used now, please waiting!\n");
        LOS_IntRestore(uvIntSave);
        return;
    }

    PRINTK("\n\n        **********************all mem used list**********************\r\n");
    PRINTK(" ID  lr0          lr1          lr2          lr3          MemSize      MemAddr\n");
    PRINTK("---------------------------------------------------------------------------------\n");
    pstNode = (LOS_MEMBOX_NODE *)(pstBoxInfo + 1);
    for (uwIndex = 0, uwIndexTmp = 0; uwIndex < pstBoxInfo->uwBlkNum; ++uwIndex, pstNode = OS_MEMBOX_NEXT(pstNode, pstBoxInfo->uwBlkSize))
    {
        pstMemCheckInfo = (MEM_CHECK_INFO *)((LOS_MEMBOX_NODE *)pstNode + 1);
        if ((*(AARCHPTR *)((AARCHPTR)((LOS_MEMBOX_NODE *)pstNode + 1) + OS_CHECK_FLAG)) == 1)
        {
            PRINTK(" %-3d 0x%-11x0x%-11x0x%-11x0x%-11x0x%-11x0x%-11x\n", uwIndexTmp,
                *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FP0), *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FP1),
                *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FP2), *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_CHECK_FP3),
                *(AARCHPTR *)((AARCHPTR)pstMemCheckInfo + OS_ADDR_SIZE), *(AARCHPTR *)(AARCHPTR)pstMemCheckInfo);
            uwIndexTmp++;
        }
    }
    LOS_IntRestore(uvIntSave);
    if (uwIndexTmp == 0)
    {
        PRINTK("************************no mem leak!!!************************\n");
        return;
    }
    PRINTK("---------------------------------------------------------------------------------\n");

}


#endif

/*****************************************************************************
 Function : LOS_MemAlloc
 Description : Allocate node from Memory pool
 Input       : pPool    --- Pointer to memory pool
                 uwSize  --- Size of memory in bytes to allocate
 Output      : None
 Return      : Pointer to allocated memory node
*****************************************************************************/
LITE_OS_SEC_TEXT VOID *LOS_MemAlloc (VOID *pPool, UINT32  uwSize)
{
    VOID *pPtr = NULL;
    UINTPTR uvIntSave = LOS_IntLock();

    do
    {
        if ((pPool == NULL) || (uwSize == 0))
        {
            break;
        }

        if (OS_MEM_NODE_GET_USED_FLAG(uwSize) || OS_MEM_NODE_GET_ALIGNED_FLAG(uwSize))
        {
            break;
        }

        pPtr = osMemAllocWithCheck(pPool, uwSize);

#ifdef LOS_MEM_LEAK_CHECK
        osMemCheckinfoCreate(pPtr, uwSize);
#endif
    } while (0);
#ifdef LOSCFG_MEM_RECORDINFO
    osMemRecordMalloc(pPtr, uwSize);
#endif
    LOS_IntRestore(uvIntSave);
    return pPtr;
}

/*****************************************************************************
 Function : LOS_MemAllocAlign
 Description : align size then allocate node from Memory pool
 Input       : pPool    --- Pointer to memory pool
                 uwSize  --- Size of memory in bytes to allocate
                 uwBoundary -- align form
 Output      : None
 Return      : Pointer to allocated memory node
*****************************************************************************/
LITE_OS_SEC_TEXT VOID *LOS_MemAllocAlign(VOID *pPool, UINT32 uwSize, UINT32 uwBoundary)
{
    UINT32 uwUseSize = 0;
    UINT32 uwGapSize = 0;
    VOID *pPtr = NULL;
    VOID *pAlignedPtr = NULL;
    UINTPTR uvIntSave = LOS_IntLock();
    LOS_MEM_DYN_NODE *pstAllocNode = NULL;
    do
    {
        if ((pPool == NULL) || (uwSize == 0) || uwBoundary == 0 || !IS_POW_TWO(uwBoundary) || !IS_ALIGNED(uwBoundary, sizeof(VOID *)))
        {
            break;
        }
        /*sizeof(uwGapSize) bytes stores offset between alignedPtr and ptr,
        the pPtr has been OS_MEM_ALIGN_SIZE(4 or 8) aligned, so maximum
        offset between alignedPtr and ptr is uwBoundary - OS_MEM_ALIGN_SIZE */
        if ((uwBoundary - sizeof(uwGapSize)) >(((UINT32) - 1) - uwSize))
        {
            break;
        }

        uwUseSize = uwSize + uwBoundary - sizeof(uwGapSize);

        if (OS_MEM_NODE_GET_USED_FLAG(uwUseSize) || OS_MEM_NODE_GET_ALIGNED_FLAG(uwUseSize))
        {
            break;
        }

        pPtr = osMemAllocWithCheck(pPool, uwUseSize);

        pAlignedPtr = (VOID *)OS_MEM_ALIGN(pPtr, uwBoundary);

        if (pPtr == pAlignedPtr)
        {
            break;
        }

        /* store gapSize in address (ptr -4), it will be checked while free */
        uwGapSize = (UINT32)((AARCHPTR)pAlignedPtr - (AARCHPTR)pPtr);
        pstAllocNode= (LOS_MEM_DYN_NODE *)pPtr - 1;
        OS_MEM_NODE_SET_ALIGNED_FLAG(pstAllocNode->stSelfNode.uwSizeAndFlag);
#ifdef LOSCFG_MEM_RECORDINFO
        pstAllocNode->stSelfNode.uwOriginSize = uwSize;
#endif
#ifdef LOSCFG_MEM_HEAD_BACKUP
        osMemNodeSaveWithGapSize(pstAllocNode, uwGapSize);
#endif

        OS_MEM_NODE_SET_ALIGNED_FLAG(uwGapSize);
        *((UINT32 *)((AARCHPTR)pAlignedPtr - sizeof(uwGapSize))) = uwGapSize;

        pPtr = pAlignedPtr;

#ifdef LOS_MEM_LEAK_CHECK
        osMemCheckinfoCreate(pPtr, uwSize);
#endif
    } while (0);
#ifdef LOSCFG_MEM_RECORDINFO
    osMemRecordMalloc(pPtr, uwSize);
#endif
     LOS_IntRestore(uvIntSave);

    return pPtr;
}

/*****************************************************************************
 Function : LOS_MemFree
 Description : free the node from memory & if there are free node beside, merger them.
                    at last update "pstListHead' which saved all free node control head
 Input       : pPool --Pointer to memory pool
                  pMem -- the node which need be freed
 Output      : None
 Return      : LOS_OK -Ok,  LOS_NOK -failed
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_MemFree(VOID *pPool, VOID *pPtr)
{
    UINT32 uwRet = LOS_NOK;
    UINT32 uwGapSize = 0;
    UINTPTR uvIntSave = LOS_IntLock();
#ifdef LOS_MEM_LEAK_CHECK
    VOID *pOriginPtr = pPtr;
#endif

    do
    {
        LOS_MEM_DYN_NODE *pstNode = (LOS_MEM_DYN_NODE *)NULL;

        if ((pPool == NULL) || (pPtr == NULL))
        {
            break;
        }

        pstNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE);

#ifdef LOSCFG_MEM_HEAD_BACKUP
        if (osMemChecksumVerify(&(pstNode->stSelfNode)) &&
            osMemChecksumVerify(&(pstNode->stBackupNode))) {
            goto DO_FREE;
        }
#endif

        uwGapSize = *((UINT32 *)((AARCHPTR)pPtr - sizeof(UINT32)));

        if(OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize) && OS_MEM_NODE_GET_USED_FLAG(uwGapSize))
        {
            PRINT_ERR("[%s:%d]uwGapSize:0x%x error\n", __FILE__, __LINE__, uwGapSize);
            break;
        }

        if (OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize))
        {
            uwGapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(uwGapSize);
            if ((uwGapSize & (OS_MEM_ALIGN_SIZE - 1)) || uwGapSize > (AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE)
            {
                PRINT_ERR("illegal uwGapSize: 0x%x\n", uwGapSize);

#ifdef LOSCFG_MEM_HEAD_BACKUP
                /* try to restore from a broken GapSize node */
                if (osMemBackupTryRestore(pPool, &pstNode, pPtr) == LOS_OK) {
                    goto DO_FREE;
                }
#endif

                break;
            }
            pstNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pPtr - uwGapSize - OS_MEM_NODE_HEAD_SIZE);

#ifdef LOSCFG_MEM_HEAD_BACKUP
            /* GapSize maybe legal, but node head is broken */
            if (!osMemChecksumVerify(&(pstNode->stSelfNode))) {
                pstNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE);
                if (osMemBackupTryRestore(pPool, &pstNode, pPtr) != LOS_OK) {
                    break;
                }
            }
#endif
        }
#ifdef LOSCFG_MEM_HEAD_BACKUP
        else {
            /* GapSize is bad, we need to verify & try to restore */
            if (!osMemChecksumVerify(&(pstNode->stSelfNode))) {
                pstNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE);
                if (osMemBackupTryRestore(pPool, &pstNode, pPtr) == LOS_OK) {
                    goto DO_FREE;
                }

                break;
            }
        }

DO_FREE:
#endif

        uwRet = osMemCheckUsedNode(pPool, pstNode);
        if (uwRet == LOS_OK)
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pPtr, pstNode->stSelfNode.uwOriginSize);
#endif
            osMemFreeNode(pstNode, pPool);
#ifdef LOS_MEM_LEAK_CHECK
            osMemCheckinfoDel(pOriginPtr);
#endif
        }

    } while(0);
#ifdef LOSCFG_MEM_RECORDINFO
    if(LOS_NOK == uwRet)
    {
       osMemRecordFree(pPtr, 0);
    }
#endif
    LOS_IntRestore(uvIntSave);
    return uwRet;
}

/*****************************************************************************
 Function : LOS_MemRealloc
 Description : reAlloc memory node
 Input       : pPool    --- Pointer to memory pool
                  pPtr  --- pointer to memory node which will be realloced
                 uwSize  --- the size of new node
 Output      : None
 Return      : Pointer to allocated memory
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID *LOS_MemRealloc (VOID *pPool,  VOID *pPtr, UINT32 uwSize)
{
    UINTPTR uvIntSave;
    UINT32 uwGapSize = 0;
    VOID *pNewPtr = NULL;
#if (defined LOS_MEM_LEAK_CHECK || defined LOSCFG_MEM_RECORDINFO)
    VOID *pOriginPtr = pPtr;
#endif

    if ((INT32)uwSize < 0)
    {
        return NULL;
    }

    uvIntSave = LOS_IntLock();

    do
    {
        LOS_MEM_DYN_NODE *pstNode = (LOS_MEM_DYN_NODE *)NULL;
        UINT32 uwRet;
        UINT32 uwAllocSize;
        UINT32 uwNodeSize;
        LOS_MEM_DYN_NODE *pstNextNode = (LOS_MEM_DYN_NODE *)NULL;

        if (pPtr == NULL)
        {
            pNewPtr = LOS_MemAlloc((VOID *)pPool, (UINT32)uwSize);
            break;
        }

        if (uwSize == 0)
        {
            (VOID)LOS_MemFree((VOID *)pPool, (VOID *)pPtr);
            break;
        }

        uwGapSize = *((UINT32 *)((AARCHPTR)pPtr - 4));
        if(OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize) && OS_MEM_NODE_GET_USED_FLAG(uwGapSize))
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pOriginPtr, 0);
#endif
            PRINT_ERR("[%s:%d]uwGapSize:0x%x error\n", __FILE__, __LINE__, uwGapSize);
            break;
        }
        if (OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize))
        {
            uwGapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(uwGapSize);
            if ((uwGapSize & (OS_MEM_ALIGN_SIZE - 1)) || uwGapSize > ((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE - (AARCHPTR)pPool) )
            {
                PRINT_ERR("[%s:%d]uwGapSize:0x%x error\n", __FILE__, __LINE__, uwGapSize);
#ifdef LOSCFG_MEM_RECORDINFO
                osMemRecordFree(pOriginPtr, 0);
#endif
                break;
            }
            pPtr = (VOID *)((AARCHPTR)pPtr - (AARCHPTR)uwGapSize);
        }
        pstNode = (LOS_MEM_DYN_NODE *)((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE);
        uwRet = osMemCheckUsedNode(pPool, pstNode);
        if (uwRet != LOS_OK)
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pOriginPtr, 0);
#endif
            break;
        }

        uwAllocSize = OS_MEM_ALIGN(uwSize + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);
        uwNodeSize = OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag);
        if (uwNodeSize >= uwAllocSize)
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pOriginPtr, pstNode->stSelfNode.uwOriginSize);
#endif
            osMemReAllocSmaller(pPool, uwAllocSize, pstNode, uwNodeSize);
#ifdef LOSCFG_MEM_RECORDINFO
            pstNode->stSelfNode.uwOriginSize = uwSize;
#ifdef LOSCFG_MEM_HEAD_BACKUP
            osMemNodeSave(pstNode);
#endif
            osMemRecordMalloc(pPtr, uwSize);
#endif
            pNewPtr = pPtr;
#ifdef LOS_MEM_LEAK_CHECK
            osMemCheckinfoUpdate(pNewPtr, uwSize);
#endif
            break;
        }

        pstNextNode = OS_MEM_NEXT_NODE(pstNode);
        if ((!OS_MEM_NODE_GET_USED_FLAG(pstNextNode->stSelfNode.uwSizeAndFlag)) &&
            ((pstNextNode->stSelfNode.uwSizeAndFlag + uwNodeSize) >= uwAllocSize))
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pOriginPtr, pstNode->stSelfNode.uwOriginSize);
#endif
            osMemMergeNodeForReAllocBigger(pPool, uwAllocSize, pstNode, uwNodeSize, pstNextNode);
#ifdef LOSCFG_MEM_RECORDINFO
            pstNode->stSelfNode.uwOriginSize = uwSize;
#ifdef LOSCFG_MEM_HEAD_BACKUP
            osMemNodeSave(pstNode);
#endif
            osMemRecordMalloc(pPtr, uwSize);
#endif
            pNewPtr = pPtr;
#ifdef LOS_MEM_LEAK_CHECK
            osMemCheckinfoUpdate(pNewPtr, uwSize);
#endif
            break;
        }

        pNewPtr = osMemAllocWithCheck(pPool, uwSize);
        if (pNewPtr != NULL)
        {
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordMalloc(pNewPtr, uwSize);
#endif
            (VOID)memcpy(pNewPtr, pPtr, uwNodeSize - OS_MEM_NODE_HEAD_SIZE);
#ifdef LOSCFG_MEM_RECORDINFO
            osMemRecordFree(pOriginPtr, pstNode->stSelfNode.uwOriginSize);
#endif
            osMemFreeNode(pstNode, pPool);
#ifdef LOS_MEM_LEAK_CHECK
            osMemCheckinfoDel(pOriginPtr);
#endif
        }
#ifdef LOS_MEM_LEAK_CHECK
        osMemCheckinfoCreate(pNewPtr, uwSize);
#endif
    } while (0);

    LOS_IntRestore(uvIntSave);
    return pNewPtr;
}

/*****************************************************************************
 Function : LOS_MemTotalUsedGet
 Description : figure the pointer memory pool for it's total mem used
 Input       : pPool    --- Pointer to memory pool
 Output      : None
 Return      : the size of the pool has been used
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemTotalUsedGet(VOID *pPool)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINT32 uwMemUsed = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode <= OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {
        if (OS_MEM_NODE_GET_USED_FLAG(pstTmpNode->stSelfNode.uwSizeAndFlag))
        {
            uwMemUsed += OS_MEM_NODE_GET_SIZE(pstTmpNode->stSelfNode.uwSizeAndFlag);
        }
    }

    LOS_IntRestore(uvIntSave);

    return uwMemUsed;
}

/*****************************************************************************
 Function : LOS_MemUsedBlksGet
 Description : get the number of used node
 Input       : pPool    --- Pointer to memory pool
 Output      : None
 Return      : the number of used node
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemUsedBlksGet(VOID *pPool)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINT32 blkNums = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode <= OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        pstTmpNode= OS_MEM_NEXT_NODE(pstTmpNode))
    {
        if (OS_MEM_NODE_GET_USED_FLAG(pstTmpNode->stSelfNode.uwSizeAndFlag))
        {
            blkNums++;
        }
    }

    LOS_IntRestore(uvIntSave);

    return blkNums;
}

/*****************************************************************************
 Function :LOS_MemTaskIdGet
 Description : get a memory node's taskID if pointer node is "used node"
 Input       : pPtr   --- pointer to aim node
 Output      : None
 Return      : taskID --Ok or OS_INVALID --pointer node is illegal or free node
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemTaskIdGet(VOID *pPtr)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)m_aucSysMem1;
    UINTPTR uvIntSave;
#ifdef LOSCFG_EXC_INTERACTION
    if (pPtr < (VOID *)m_aucSysMem1)
    {
        pstPoolInfo = (LOS_MEM_POOL_INFO *)m_aucSysMem0;
    }
#endif
    if (pPtr == NULL ||
        pPtr < (VOID *)OS_MEM_FIRST_NODE(pstPoolInfo) ||
        pPtr > (VOID *)OS_MEM_END_NODE(pstPoolInfo, pstPoolInfo->uwPoolSize))
    {
        PRINT_ERR("input ptr %p is out of system memory range[%p, %p]\n", pPtr, OS_MEM_FIRST_NODE(pstPoolInfo),
            OS_MEM_END_NODE(pstPoolInfo, pstPoolInfo->uwPoolSize));
        return OS_INVALID;
    }

    uvIntSave= LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pstPoolInfo); pstTmpNode <= OS_MEM_END_NODE(pstPoolInfo, pstPoolInfo->uwPoolSize);
        pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {

        if ((AARCHPTR)pPtr < (AARCHPTR)pstTmpNode)
        {
            if (OS_MEM_NODE_GET_USED_FLAG((pstTmpNode->stSelfNode.pstPreNode)->stSelfNode.uwSizeAndFlag))
            {
                LOS_IntRestore(uvIntSave);
                return (UINT32)((AARCHPTR)((pstTmpNode->stSelfNode.pstPreNode)->stSelfNode.stFreeNodeInfo.pstNext));
            }
            else
            {
                LOS_IntRestore(uvIntSave);
                PRINT_ERR("input ptr %p is belong to a free mem node\n", pPtr);
                return OS_INVALID;
            }
        }
    }

    LOS_IntRestore(uvIntSave);
    return OS_INVALID;
}

/*****************************************************************************
 Function : LOS_MemFreeBlksGet
 Description : get the number of free node
 Input       : pool    --- Pointer to memory pool
 Output      : None
 Return      : the number of free node
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemFreeBlksGet(VOID *pPool)
{
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINT32 blkNums = 0;
    UINTPTR uvIntSave;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode <= OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
        pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {
        if (!OS_MEM_NODE_GET_USED_FLAG(pstTmpNode->stSelfNode.uwSizeAndFlag))
        {
            blkNums++;
        }
    }

    LOS_IntRestore(uvIntSave);

    return blkNums;
}

/*****************************************************************************
 Function : LOS_MemLastUsedGet
 Description : get the address of last used node(except end node)
 Input       : pPool    --- Pointer to memory pool
 Output      : None
 Return      : address of last node offset sizeof(LOS_MEM_DYN_NODE), if last node is freeNode
                   address of the end node, if last node is usedNode
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR AARCHPTR LOS_MemLastUsedGet(VOID *pPool)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstNode;

    if (pPool == NULL)
    {
        return LOS_NOK;
    }

    pstNode = OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize)->stSelfNode.pstPreNode;

    if (OS_MEM_NODE_GET_USED_FLAG(pstNode->stSelfNode.uwSizeAndFlag) )
        return (AARCHPTR)((CHAR *)pstNode + OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag) + sizeof(LOS_MEM_DYN_NODE));
    else
        return (AARCHPTR)((CHAR *)pstNode + sizeof(LOS_MEM_DYN_NODE));
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
    LOS_MEM_DYN_NODE *pstEndNode = (LOS_MEM_DYN_NODE *)OS_MEM_END_NODE(pPool, ((LOS_MEM_POOL_INFO *)pPool)->uwPoolSize);
    pstEndNode->stSelfNode.uwSizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    if (uwPreAddr != (AARCHPTR)NULL)
        pstEndNode->stSelfNode.pstPreNode = (LOS_MEM_DYN_NODE *)(uwPreAddr - sizeof(LOS_MEM_DYN_NODE));
    OS_MEM_NODE_SET_USED_FLAG(pstEndNode->stSelfNode.uwSizeAndFlag);
    osMemSetMagicNumAndTaskid(pstEndNode);

#ifdef LOSCFG_MEM_HEAD_BACKUP
    osMemNodeSave(pstEndNode);
#endif
}

/*****************************************************************************
 Function : LOS_MemPoolSizeGet
 Description : get the memory pool's size
 Input       : pPool    --- Pointer to memory pool
 Output      : LOS_NOK & Other value -- The size of the memory pool.
 Return      : the size of the memory pool
*****************************************************************************/
 UINT32 LOS_MemPoolSizeGet(VOID *pPool)
{
    if (pPool == NULL)
    {
        return LOS_NOK;
    }
    return ((LOS_MEM_POOL_INFO *)pPool)->uwPoolSize;
}

LITE_OS_SEC_TEXT_MINOR VOID osMemInfoPrint(VOID *pPool)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_POOL_STATUS stStatus = {0};

    if (LOS_NOK == LOS_MemInfoGet(pPool, &stStatus))
        return;

#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    PRINTK("pool addr          pool size    used size     free size    max free node size   used node num     free node num      UsageWaterLine\n"
                 "---------------    --------     -------       --------     --------------       -------------      ------------      ------------\n"
                 "%-16p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x    0x%-13x\n",
                        pstPoolInfo->pPoolAddr, pstPoolInfo->uwPoolSize, stStatus.uwTotalUsedSize, stStatus.uwTotalFreeSize,
                        stStatus.uwMaxFreeNodeSize, stStatus.uwUsedNodeNum, stStatus.uwFreeNodeNum, stStatus.uwUsageWaterLine);

#else
    PRINTK("pool addr          pool size    used size     free size    max free node size   used node num     free node num\n"
                 "---------------    --------     -------       --------     --------------       -------------      ------------\n"
                 "%-16p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x\n",
                        pstPoolInfo->pPoolAddr, pstPoolInfo->uwPoolSize, stStatus.uwTotalUsedSize, stStatus.uwTotalFreeSize,
                        stStatus.uwMaxFreeNodeSize, stStatus.uwUsedNodeNum, stStatus.uwFreeNodeNum);
#endif
    return;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemInfoGet(VOID *pPool, LOS_MEM_POOL_STATUS *pstStatus)
{
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    UINT32 uwTotalUsedSize = 0, uwTotalFreeSize = 0, uwMaxFreeNodeSize = 0;
    UINT32 uwUsedNodeNum = 0, uwFreeNodeNum = 0;
    UINTPTR uvIntSave;

    if (pstStatus == NULL)
    {
        PRINT_ERR("can't use NULL addr to save info\n");
        return LOS_NOK;
    }

    if (pstPoolInfo == NULL || (AARCHPTR)pPool != (AARCHPTR)pstPoolInfo->pPoolAddr)
    {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", pstPoolInfo, __LINE__);
        return LOS_NOK;
    }

    pstTmpNode = (LOS_MEM_DYN_NODE *)OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
    pstTmpNode = (LOS_MEM_DYN_NODE *)OS_MEM_ALIGN(pstTmpNode, OS_MEM_ALIGN_SIZE);

    if (!OS_MEM_MAGIC_VALID(pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev))
    {
        PRINT_ERR("wrong mem pool addr: %p\n, line:%d", pstPoolInfo, __LINE__);
        return LOS_NOK;
    }

    uvIntSave = LOS_IntLock();

    for (pstTmpNode = OS_MEM_FIRST_NODE(pPool); pstTmpNode <= OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
            pstTmpNode = OS_MEM_NEXT_NODE(pstTmpNode))
    {
            if (!OS_MEM_NODE_GET_USED_FLAG(pstTmpNode->stSelfNode.uwSizeAndFlag))
            {
                ++uwFreeNodeNum;
                uwTotalFreeSize += OS_MEM_NODE_GET_SIZE(pstTmpNode->stSelfNode.uwSizeAndFlag);
                if (uwMaxFreeNodeSize< OS_MEM_NODE_GET_SIZE(pstTmpNode->stSelfNode.uwSizeAndFlag))
                {
                    uwMaxFreeNodeSize = OS_MEM_NODE_GET_SIZE(pstTmpNode->stSelfNode.uwSizeAndFlag);
                }
            }
            else
            {
                ++uwUsedNodeNum;
                uwTotalUsedSize += OS_MEM_NODE_GET_SIZE(pstTmpNode->stSelfNode.uwSizeAndFlag);
            }
    }

    LOS_IntRestore(uvIntSave);

    pstStatus->uwTotalUsedSize = uwTotalUsedSize;
    pstStatus->uwTotalFreeSize = uwTotalFreeSize;
    pstStatus->uwMaxFreeNodeSize = uwMaxFreeNodeSize;
    pstStatus->uwUsedNodeNum = uwUsedNodeNum;
    pstStatus->uwFreeNodeNum = uwFreeNodeNum;
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    pstStatus->uwUsageWaterLine = pstPoolInfo->uwPoolWaterLine;
#endif
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemFreeNodeShow(VOID *pPool)
{
    LOS_DL_LIST *pstListHead = (LOS_DL_LIST *)NULL;
    LOS_MULTIPLE_DLNK_HEAD *pHeadAddr = (LOS_MULTIPLE_DLNK_HEAD *)((AARCHPTR)pPool + sizeof(LOS_MEM_POOL_INFO));
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    LOS_MEM_DYN_NODE *pstTmpNode = (LOS_MEM_DYN_NODE *)NULL;
    UINT32 uwCount, uwIdx;
    UINT32 uwCountNum[8] = {0};
    UINTPTR uvIntSave;
    UINT32 uwIndex = 0;
    UINT32 uwIndexTmp = 0;

    if (pstPoolInfo == NULL || (AARCHPTR)pPool != (AARCHPTR)pstPoolInfo->pPoolAddr)
    {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", pstPoolInfo, __LINE__);
        return LOS_NOK;
    }

    pstTmpNode = (LOS_MEM_DYN_NODE *)OS_MEM_END_NODE(pPool, pstPoolInfo->uwPoolSize);
    pstTmpNode = (LOS_MEM_DYN_NODE *)OS_MEM_ALIGN(pstTmpNode, OS_MEM_ALIGN_SIZE);

    if (!OS_MEM_MAGIC_VALID(pstTmpNode->stSelfNode.stFreeNodeInfo.pstPrev))
    {
        PRINT_ERR("wrong mem pool addr: %p, line:%d\n", pstPoolInfo, __LINE__);
        return LOS_NOK;
    }
    PRINTK("\n   ************************ left free node number**********************\n");
    PRINTK("    block size:  ");
    uvIntSave = LOS_IntLock();

    for (uwIdx = 0; uwIdx <= OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2; uwIdx++, uwIndexTmp++)
    {
        uwCount = 0;
        pstListHead = pHeadAddr->stListHead[uwIdx].pstNext;
        while (pstListHead != &(pHeadAddr->stListHead[uwIdx]))
        {
            pstListHead = pstListHead->pstNext;
            ++uwCount;
        }
        PRINTK("2^%-5d", uwIdx + 4);
        if (uwIdx != 0 && ((uwIdx + 1) % 8 == 0 || uwIdx == 26))
        {
            PRINTK("\n    node number:");
            for (uwIndex = 0; uwIndex < 8; uwIndex++)
            {
                PRINTK("  %-5d",uwCountNum[uwIndex]);
                if (uwIndex == 2 && uwIdx == OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2)
                {
                    break;
                }
            }
            if (uwIdx != OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2)
                PRINTK("\n\n    block size:  ");
        }

        uwIndexTmp = uwIdx % 8;
        uwCountNum[uwIndexTmp] = uwCount;
    }
    LOS_IntRestore(uvIntSave);
    PRINTK("\n   ********************************************************************\n\n");

    return LOS_OK;
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
    LOS_MEM_POOL_INFO *pstPoolInfo = (LOS_MEM_POOL_INFO *)pPool;
    UINT8 *pEndPool;

    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_DISABLE)
    {
        return LOS_ERRNO_MEMCHECK_DISABLED;
    }

    if ((pPool == NULL) || (pPtr == NULL))
    {
        return LOS_ERRNO_MEMCHECK_PARA_NULL;
    }

    pEndPool = (UINT8 *)pPool + pstPoolInfo->uwPoolSize;
    if ( !( OS_MEM_MIDDLE_ADDR_OPEN_END(pPool, (UINT8 *)pPtr, pEndPool)))
    {
        return LOS_ERRNO_MEMCHECK_OUTSIDE;
    }

    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_HIGH)
    {
        pHead = osMemFindNodeCtrl(pPtr);
        if ((pHead == NULL) || (OS_MEM_NODE_GET_SIZE(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.uwSizeAndFlag) < ((AARCHPTR)pPtr - (AARCHPTR)pHead)))
        {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        *puwTotalSize = OS_MEM_NODE_GET_SIZE(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.uwSizeAndFlag - sizeof(LOS_MEM_DYN_NODE));
        *puwAvailSize = OS_MEM_NODE_GET_SIZE(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.uwSizeAndFlag - ((AARCHPTR)pPtr - (AARCHPTR)pHead));
        return LOS_OK;
    }
    if ( ucCheckMemLevel == LOS_MEM_CHECK_LEVEL_LOW)
    {
        if (pPtr != (VOID *)OS_MEM_ALIGN(pPtr, OS_MEM_ALIGN_SIZE))
        {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        pHead = (VOID *)((AARCHPTR)pPtr - sizeof(LOS_MEM_DYN_NODE));
        if (OS_MEM_MAGIC_VALID(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.stFreeNodeInfo.pstPrev))
        {
            *puwTotalSize = OS_MEM_NODE_GET_SIZE(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.uwSizeAndFlag - sizeof(LOS_MEM_DYN_NODE));
            *puwAvailSize = OS_MEM_NODE_GET_SIZE(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.uwSizeAndFlag - sizeof(LOS_MEM_DYN_NODE));
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
LITE_OS_SEC_TEXT_MINOR VOID *osMemFindNodeCtrl(VOID *pPtr)
{
    UINT8 *pHead = (UINT8 *)pPtr;

    if( pPtr == NULL )
    {
        return NULL;
    }

    pHead = (UINT8 *)OS_MEM_ALIGN((VOID *)pHead, OS_MEM_ALIGN_SIZE);
    while (!OS_MEM_MAGIC_VALID(((LOS_MEM_DYN_NODE *)pHead)->stSelfNode.stFreeNodeInfo.pstPrev))
    {
        pHead-= sizeof(AARCHPTR);
    }
    return pHead;
}

/*****************************************************************************
 Function : LOS_MemCheckLevelSet
 Description : setting ucCheckMemLevel which decide the manner of memcheck
 Input       : ucLevel -- what level want to set
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

#ifdef LOSCFG_MEM_MUL_MODULE
STATIC INLINE UINT32 osMemModCheck(UINT32 uwModule)
{
    if (uwModule > MEM_MODULE_MAX)
    {
        PRINT_ERR("error module ID input!\n");
        return LOS_NOK;
    }
    return LOS_OK;
}

STATIC INLINE VOID *osMemPtrToNode(VOID *pPtr)
{
    UINT32 uwGapSize;

    if ((AARCHPTR)pPtr & (OS_MEM_ALIGN_SIZE - 1))
    {
        PRINT_ERR("[%s:%d]pPtr:%p not align by 4byte\n", __FILE__, __LINE__, pPtr);
        return NULL;
    }

    uwGapSize = *((UINT32 *)((AARCHPTR)pPtr - sizeof(UINT32)));
    if(OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize) && OS_MEM_NODE_GET_USED_FLAG(uwGapSize))
    {
        PRINT_ERR("[%s:%d]uwGapSize:0x%x error\n", __FILE__, __LINE__, uwGapSize);
        return NULL;
    }
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(uwGapSize))
    {
        uwGapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(uwGapSize);
        if ((uwGapSize & (OS_MEM_ALIGN_SIZE - 1)) || uwGapSize > ((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE))
        {
            PRINT_ERR("[%s:%d]uwGapSize:0x%x error\n", __FILE__, __LINE__, uwGapSize);
            return NULL;
        }

        pPtr = (VOID *)((AARCHPTR)pPtr - uwGapSize);
    }

    return (VOID *)((AARCHPTR)pPtr - OS_MEM_NODE_HEAD_SIZE);
}

STATIC INLINE UINT32 osMemNodeSizeGet(VOID *pPtr)
{
    LOS_MEM_DYN_NODE * pstNode;
    pstNode = (LOS_MEM_DYN_NODE *)osMemPtrToNode(pPtr);
    if (pstNode == NULL)
    {
        return 0;
    }

    return OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag);
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
        g_uwMemInfo[uwModule] += osMemNodeSizeGet(pPtr);
        pNode = osMemPtrToNode(pPtr);
        OS_MEM_MODID_SET(pNode, uwModule); /*lint !e613*/
        LOS_IntRestore(uvIntSave);
    }
    return pPtr;
}

VOID *LOS_MemMallocAlign(VOID *pPool, UINT32 uwSize, UINT32 uwBoundary, UINT32 uwModule)
{
    UINTPTR uvIntSave;
    VOID *pPtr, *pNode;
    if (osMemModCheck(uwModule) == LOS_NOK)
        return NULL;
    pPtr = LOS_MemAllocAlign(pPool, uwSize, uwBoundary);
    if (pPtr != NULL)
    {
        uvIntSave = LOS_IntLock();
        g_uwMemInfo[uwModule] += osMemNodeSizeGet(pPtr);
        pNode = osMemPtrToNode(pPtr);
        OS_MEM_MODID_SET(pNode, uwModule); /*lint !e613*/
        LOS_IntRestore(uvIntSave);
    }
    return pPtr;
}

UINT32 LOS_MemMfree(VOID *pPool, VOID *pPtr, UINT32 uwModule)
{
    UINTPTR uvIntSave;
    UINT32 uwRet = LOS_NOK;
    UINT32 uwSize;
    LOS_MEM_DYN_NODE * pstNode;

    if (osMemModCheck(uwModule) == LOS_NOK || pPtr == NULL)
        return LOS_NOK;

    pstNode = (LOS_MEM_DYN_NODE *)osMemPtrToNode(pPtr);
    if (pstNode == NULL)
    {
        return LOS_NOK;
    }

    uwSize = OS_MEM_NODE_GET_SIZE(pstNode->stSelfNode.uwSizeAndFlag);

    if (uwModule != OS_MEM_MODID_GET(pstNode))
    {
        PRINT_ERR("node[%p] alloced in module %d, but free in module %d\n node's taskID: 0x%x\n",
                            pPtr, OS_MEM_MODID_GET(pstNode), uwModule, OS_MEM_TASKID_GET(pstNode));
        uwModule = OS_MEM_MODID_GET(pstNode);
    }
    uwRet = LOS_MemFree(pPool, pPtr);

    if (uwRet == LOS_OK)
    {
        uvIntSave = LOS_IntLock();
        g_uwMemInfo[uwModule] -= uwSize;
        LOS_IntRestore(uvIntSave);
    }
    return uwRet;
}

VOID *LOS_MemMrealloc(VOID *pPool, VOID *pPtr, UINT32 uwSize, UINT32 uwModule)
{
    VOID *pNewPtr;
    UINT32 uwOldSize;
    UINTPTR uvIntSave;
    LOS_MEM_DYN_NODE * pstNode;
    UINT32 uwOldModule = uwModule;

    if (osMemModCheck(uwModule) == LOS_NOK)
        return NULL;

    if (pPtr == NULL)
    {
        return LOS_MemMalloc(pPool, uwSize, uwModule);
    }

    pstNode = (LOS_MEM_DYN_NODE *)osMemPtrToNode(pPtr);
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

    if (uwSize == 0)
    {
        (VOID)LOS_MemMfree(pPool, pPtr, uwOldModule);
        return NULL;
    }

    uwOldSize = osMemNodeSizeGet(pPtr);
    pNewPtr = LOS_MemRealloc(pPool, pPtr, uwSize);
    if (pNewPtr != NULL)
    {
        uvIntSave = LOS_IntLock();
        g_uwMemInfo[uwModule] += osMemNodeSizeGet(pNewPtr);
        g_uwMemInfo[uwOldModule] -= uwOldSize;
        pstNode = (LOS_MEM_DYN_NODE *)osMemPtrToNode(pNewPtr);
        OS_MEM_MODID_SET(pstNode, uwModule);
        LOS_IntRestore(uvIntSave);
    }
    return pNewPtr;
}

UINT32 LOS_MemMusedGet(UINT32 uwModule)
{
    if (osMemModCheck(uwModule) == LOS_NOK)
        return OS_NULL_INT;
    return g_uwMemInfo[uwModule];
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
