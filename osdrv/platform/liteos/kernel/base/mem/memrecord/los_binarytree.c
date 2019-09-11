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
#include "los_config.h"
#ifdef LOSCFG_MEM_RECORDINFO

#include "string.h"
#include "los_binarytree.ph"
#include "los_typedef.h"
#include "los_memory.h"
INT32 BinTreeInsert(VOID *pNode,UINT32 uwNodeLen, BinNode **pLeaf,
    BinNode *(*getMyBinNode)(UINT32 *pNodeID),INT32 (*CompareNode)(VOID *pNode1,VOID *pNode2))
{
    UINT32 uwNodeID;
    INT32 uwResult;
    BinNode **ppCurrentNode = pLeaf;
    if (pLeaf == NULL)
    {
        return -1;
    }
    do
    {
        if ( *ppCurrentNode != 0 )
        {
            uwResult = CompareNode(pNode,(*ppCurrentNode));
            if (0 == uwResult)
            {
                return (*ppCurrentNode)->uwNodeID;
            }
            else if (uwResult < 0)
            {
                ppCurrentNode = &((*ppCurrentNode)->left);
            }
            else
            {
                ppCurrentNode = &((*ppCurrentNode)->right);
            }
        }
        else
        {
            (*ppCurrentNode) = getMyBinNode(&uwNodeID);
            if (!(*ppCurrentNode))
            {
                return -1;
            }
            memcpy((*ppCurrentNode)->keyValue,((BinNode *)pNode)->keyValue,uwNodeLen - sizeof(BinNode));
            (*ppCurrentNode)->uwNodeID = uwNodeID;
            /* initialize the children to null */
            (*ppCurrentNode)->left = 0;
            (*ppCurrentNode)->right = 0;
            return (*ppCurrentNode)->uwNodeID;
        }
    }
    while (1);

}


//LRNODE
LRNode g_aLRNode[LR_COUNT];
UINT32 g_uwLRNodeIndex=0;
LRNode *g_pLRRoot = 0;
INT32 CompareLRNode(VOID *pNode1,VOID *pNode2)
{
    LRNode *pNd1 = (LRNode *)pNode1;
    LRNode *pNd2 = (LRNode *)pNode2;
    if (pNd1->LR1 < pNd2->LR1)
    {
        return -1;
    }
    else if ((pNd1->LR1 == pNd2->LR1) && (pNd1->LR2 < pNd2->LR2))
    {
        return -1;
    }
    else if ((pNd1->LR1 == pNd2->LR1) && (pNd1->LR2 == pNd2->LR2) && (pNd1->LR3 < pNd2->LR3))
    {
        return -1;
    }
    else if ((pNd1->LR1 == pNd2->LR1) && (pNd1->LR2 == pNd2->LR2) && (pNd1->LR3 == pNd2->LR3))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

BinNode *getLRBinNode(UINT32 *pNodeID)
{
    if (g_uwLRNodeIndex < LR_COUNT)
    {
        *pNodeID = g_uwLRNodeIndex;
        return (BinNode *)(&g_aLRNode[g_uwLRNodeIndex++]);
    }
    else
    {
        *pNodeID=-1;
        return (BinNode *)NULL;
    }
}

//ADDRNODE
AddrNode g_aAddrNode[ADDR_COUNT];
UINT32 g_uwAddrNodeIndex = 0;
AddrNode *g_pAddrRoot = 0;
INT32 CompareAddrNode(VOID *pNode1,VOID *pNode2)
{
    AddrNode *pNd1 = (AddrNode *)pNode1;
    AddrNode *pNd2 = (AddrNode *)pNode2;
    if (pNd1->addr < pNd2->addr)
        return -1;
    else if ((pNd1->addr == pNd2->addr) )
        return 0;
    else
        return 1;
}

BinNode *getAddrBinNode(UINT32 *pNodeID)
{
    if (g_uwAddrNodeIndex < ADDR_COUNT)
    {
        *pNodeID = g_uwAddrNodeIndex;
        return (BinNode *)(&g_aAddrNode[g_uwAddrNodeIndex++]);
    }
    else
    {
        *pNodeID = -1;
        return (BinNode *)NULL;
    }
}

//REQSIZENODE
ReqSizeNode g_aReqSizeNode[REQ_SIZE_COUNT];
UINT32 g_uwReqSizeNodeIndex=0;
ReqSizeNode *g_pReqSizeRoot = 0;

INT32 CompareReqSizeNode(VOID *pNode1,VOID *pNode2)
{
    ReqSizeNode *pNd1 = (ReqSizeNode *)pNode1;
    ReqSizeNode *pNd2 = (ReqSizeNode *)pNode2;
    if (pNd1->reqSize < pNd2->reqSize)
    {
        return -1;
    }
    else if ((pNd1->reqSize == pNd2->reqSize) )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

BinNode *getReqSizeBinNode(UINT32 *pNodeID)
{
    if (g_uwReqSizeNodeIndex < REQ_SIZE_COUNT)
    {
        *pNodeID = g_uwReqSizeNodeIndex;
        return (BinNode *)(&g_aReqSizeNode[g_uwReqSizeNodeIndex++]);
    }
    else
    {
        *pNodeID = -1;
        return (BinNode *)NULL;
    }
}


//TASKIDNODE
TaskIDNode g_aTaskIDNode[TASK_ID_COUNT];
UINT32 g_uwTaskIDNodeIndex = 0;
TaskIDNode *g_pTaskIDRoot = 0;
INT32 CompareTaskIDNode(VOID *pNode1,VOID *pNode2)
{
    TaskIDNode *pNd1 = (TaskIDNode *)pNode1;
    TaskIDNode *pNd2 = (TaskIDNode *)pNode2;
    if (pNd1->taskID < pNd2->taskID)
    {
        return -1;
    }
    else if ((pNd1->taskID == pNd2->taskID))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

BinNode *getTaskIDBinNode(UINT32 *pNodeID)
{
    if (g_uwTaskIDNodeIndex < TASK_ID_COUNT)
    {
        *pNodeID = g_uwTaskIDNodeIndex;
        return (BinNode *)(&g_aTaskIDNode[g_uwTaskIDNodeIndex++]);
    }
    else
    {
        *pNodeID = -1;
        return (BinNode *)NULL;
    }
}

UINT32 BinaryTreeTaskID[11] = {33, 10, 20, 9, 42, 34, 45, 47, 46, 50, 49};
UINT32 BinaryTreeReqSize[4] = {616, 136, 1708, 1580};

VOID osBinaryTreeInit(VOID)
{
    INT32 uwIndex = 0;
    LRNode stLRNode;
    extern CHAR __text_start, __text_end;

    stLRNode.LR1 = (AARCHPTR)((&__text_end - &__text_start) / 2 + &__text_start);
    stLRNode.LR2 = (AARCHPTR)((&__text_end - &__text_start) / 2 + &__text_start);
    stLRNode.LR3 = (AARCHPTR)((&__text_end - &__text_start) / 2 + &__text_start);
    BinTreeInsert(&stLRNode,sizeof(LRNode),(BinNode **)&g_pLRRoot,getLRBinNode,CompareLRNode);

    AddrNode stAddrNode;
    stAddrNode.addr = (g_sys_mem_addr_end - (AARCHPTR)&__bss_end) / 2 + (AARCHPTR)&__bss_end;
    BinTreeInsert(&stAddrNode,sizeof(AddrNode),(BinNode **)&g_pAddrRoot,getAddrBinNode,CompareAddrNode);

    TaskIDNode stTaskIDNode;
    for (uwIndex = 0; uwIndex < 11; uwIndex++)
    {
        stTaskIDNode.taskID = BinaryTreeTaskID[uwIndex];
        BinTreeInsert(&stTaskIDNode,sizeof(TaskIDNode),(BinNode **)&g_pTaskIDRoot,getTaskIDBinNode,CompareTaskIDNode);
    }

    ReqSizeNode stReqSizeNode;
    for (uwIndex = 0; uwIndex < 4; uwIndex++)
    {
        stReqSizeNode.reqSize = BinaryTreeReqSize[uwIndex];
        BinTreeInsert(&stReqSizeNode,sizeof(ReqSizeNode),(BinNode **)&g_pReqSizeRoot,getReqSizeBinNode,CompareReqSizeNode);
    }
}

#endif
