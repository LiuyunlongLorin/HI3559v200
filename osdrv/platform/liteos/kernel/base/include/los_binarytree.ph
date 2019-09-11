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

#ifndef _LOS_BINARYTREE_PH
#define _LOS_BINARYTREE_PH

#include "los_typedef.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct BinNode
{
    struct BinNode *left;
    struct BinNode *right;
    UINT32 uwNodeID;
    CHAR keyValue[];  /*lint !e43*/
} BinNode;

typedef struct
{
    BinNode leaf;
    AARCHPTR LR1,LR2,LR3;
}LRNode;
#define LR_COUNT 4096
extern  LRNode g_aLRNode[LR_COUNT];
extern  UINT32 g_uwLRNodeIndex;
extern  LRNode *g_pLRRoot;

typedef struct
{
    BinNode leaf;
    AARCHPTR addr;
}AddrNode;

#define ADDR_COUNT 40960
extern  AddrNode g_aAddrNode[ADDR_COUNT];
extern  UINT32 g_uwAddrNodeIndex;
extern  AddrNode *g_pAddrRoot;


typedef struct
{
    BinNode leaf;
    INT32 reqSize;
}ReqSizeNode;

#define REQ_SIZE_COUNT 4096
extern  ReqSizeNode g_aReqSizeNode[REQ_SIZE_COUNT];
extern  UINT32 g_uwReqSizeNodeIndex;
extern  ReqSizeNode *g_pReqSizeRoot;

typedef struct
{
    BinNode leaf;
    UINT32 taskID;
}TaskIDNode;

#define TASK_ID_COUNT 1024
extern  TaskIDNode g_aTaskIDNode[TASK_ID_COUNT];
extern  UINT32 g_uwTaskIDNodeIndex;
extern  TaskIDNode *g_pTaskIDRoot;


extern INT32 BinTreeInsert(VOID *pNode,UINT32 uwNodeLen, BinNode **pLeaf,
BinNode *(*getMyBinNode)(UINT32 *pNodeID),INT32 (*CompareNode)(VOID *pNode1,VOID *pNode2));

extern INT32 CompareLRNode(VOID *pNode1,VOID *pNode2);
extern BinNode *getLRBinNode(UINT32 *pNodeID);

extern INT32 CompareAddrNode(VOID *pNode1,VOID *pNode2);
extern BinNode *getAddrBinNode(UINT32 *pNodeID);

extern INT32 CompareReqSizeNode(VOID *pNode1,VOID *pNode2);
extern BinNode *getReqSizeBinNode(UINT32 *pNodeID);

extern INT32 CompareTaskIDNode(VOID *pNode1,VOID *pNode2);
extern BinNode *getTaskIDBinNode(UINT32 *pNodeID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif