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
#include "los_binarytree.ph"
#include "stdio.h"
#include "los_memory.h"
#include "los_event.h"
#include "los_exc.h"
#include "los_task.ph"
#include "los_memrecord.ph"

enum enMemActype
{
    OS_MEM_VALIDFREE,
    OS_MEM_INVALIDFREE,
    OS_MEM_ALLOC,
};

MEM_RECODRD_INFO  g_stMemRecord1[RECORD_LEN] = {0};
MEM_RECODRD_INFO  g_stMemRecord2[RECORD_LEN] = {0};
MEM_RECODRD_INFO *g_pSaveMemRecord = g_stMemRecord1;
MEM_RECODRD_INFO *g_pPrintMemRecord = NULL;
MEM_RECODRD_INFO *g_pstPtr = NULL;

volatile INT32    g_swMemRecordIndex = -1;
volatile UINT32   g_uwMemLastEndIndex = 0;
EVENT_CB_S        g_stMemshowEvent;

extern ST_LOS_TASK  g_stLosTask;
extern UINT64       g_ullTickCount;
UINT32 g_uwMemRecordShowEnable = 1;
UINT32 g_uwLastAddrNodeIndex = 0;
UINT32 g_uwLastReqSizeNodeIndex = 0;
UINT32 g_uwLastLRNodeIndex = 0;

static VOID osMemRecordCompressInfo(VOID)
{
    UINT32 uwCount = 0;
    static CHAR infoStr[20];

    UINT32 uwCurrentIndex = g_uwAddrNodeIndex;
    for (uwCount = g_uwLastAddrNodeIndex; uwCount < uwCurrentIndex; uwCount++)
    {
        DecTo64F(g_aAddrNode[uwCount].leaf.uwNodeID, infoStr,3);
        printf("~^%s%x^~\n",infoStr,g_aAddrNode[uwCount].addr);
    }
    g_uwLastAddrNodeIndex = uwCurrentIndex;

    uwCurrentIndex = g_uwReqSizeNodeIndex;
    for (uwCount = g_uwLastReqSizeNodeIndex; uwCount < uwCurrentIndex; uwCount++)
    {
        DecTo64F( g_aReqSizeNode[uwCount].leaf.uwNodeID, infoStr,2);
        printf("*^%s%d^*\n",infoStr,g_aReqSizeNode[uwCount].reqSize);
    }
    g_uwLastReqSizeNodeIndex = uwCurrentIndex;

    uwCurrentIndex = g_uwLRNodeIndex;
    for (uwCount = g_uwLastLRNodeIndex; uwCount < uwCurrentIndex; uwCount++)
    {
        DecTo64F( g_aLRNode[uwCount].leaf.uwNodeID, infoStr,2);
        printf("$^%s%x%x%x^$\n",infoStr,g_aLRNode[uwCount].LR1,g_aLRNode[uwCount].LR2,g_aLRNode[uwCount].LR3);
    }
    g_uwLastLRNodeIndex = uwCurrentIndex;

    return;
}

static VOID osMemRecordShow(VOID)
{
    UINT32 uwStart = 0;
    UINT32 uwEnd = 0;
    MEM_RECODRD_INFO *pPrintMemRecord;
    MEM_RECODRD_INFO *pSaveMemRecord;
    static CHAR printStr[32];
    static CHAR nodeValue[7];
    UINT32 uwIndex = 0;
MEM_REC_PRINT_RESET:
    pPrintMemRecord = g_pPrintMemRecord;
    pSaveMemRecord = g_pSaveMemRecord;
    uwStart = g_uwMemLastEndIndex;
    uwEnd = g_swMemRecordIndex;
    if (NULL == g_pstPtr )
    {
        g_pstPtr = g_pPrintMemRecord;
    }

    osMemRecordCompressInfo();

    if (g_pstPtr)
    {
        printStr[0]='~';
        printStr[1]='!';
        while ((g_pstPtr == pPrintMemRecord && uwStart < RECORD_LEN)||(g_pstPtr == pSaveMemRecord && uwStart <= uwEnd))
        {
            uwIndex = 2;
            DecTo64F(uwStart,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].uwAddrID,nodeValue,3);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];

            DecTo64F(g_pstPtr[uwStart].uwOriginSizeID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].swActType,nodeValue,4);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];
            printStr[uwIndex++] = nodeValue[3];

            DecTo64F(g_pstPtr[uwStart].uwTaskID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].ullSysTick,nodeValue,6);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];
            printStr[uwIndex++] = nodeValue[3];
            printStr[uwIndex++] = nodeValue[4];
            printStr[uwIndex++] = nodeValue[5];

            DecTo64F(g_pstPtr[uwStart].uwLrID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            printStr[uwIndex++] = '!';
            printStr[uwIndex++] = '~';
            printStr[uwIndex++] = '\0';

            printf("%s\n",printStr);
            uwStart++;

        };

    }
    if (uwStart == RECORD_LEN || NULL == g_pstPtr)
    {
        uwStart = 0;
        if (g_pstPtr == g_pSaveMemRecord)
        {
            printf("Rec:error:the printf speed is low,Rnd\n");
            g_uwMemLastEndIndex = 0;
            goto MEM_REC_PRINT_RESET;
        }
        g_pstPtr = g_pSaveMemRecord;
        printStr[0]='~';
        printStr[1]='!';
        while ((uwStart < (uwEnd + 1)))
        {
            uwIndex = 2;
            DecTo64F(uwStart,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].uwAddrID,nodeValue,3);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];

            DecTo64F(g_pstPtr[uwStart].uwOriginSizeID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].swActType,nodeValue,4);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];
            printStr[uwIndex++] = nodeValue[3];

            DecTo64F(g_pstPtr[uwStart].uwTaskID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            DecTo64F(g_pstPtr[uwStart].ullSysTick,nodeValue,6);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];
            printStr[uwIndex++] = nodeValue[2];
            printStr[uwIndex++] = nodeValue[3];
            printStr[uwIndex++] = nodeValue[4];
            printStr[uwIndex++] = nodeValue[5];

            DecTo64F(g_pstPtr[uwStart].uwLrID,nodeValue,2);
            printStr[uwIndex++] = nodeValue[0];
            printStr[uwIndex++] = nodeValue[1];

            printStr[uwIndex++] = '!';
            printStr[uwIndex++] = '~';
            printStr[uwIndex++] = '\0';

            printf("%s\n",printStr);

            uwStart++;
        };
    }
    g_uwMemLastEndIndex = uwEnd + 1;
}

static VOID osMemRecordExchange(VOID)
{
    if(g_pSaveMemRecord == g_stMemRecord1)
    {
        g_pSaveMemRecord = g_stMemRecord2;
        g_pPrintMemRecord = g_stMemRecord1;
    }
    else
    {
        g_pSaveMemRecord = g_stMemRecord1;
        g_pPrintMemRecord = g_stMemRecord2;
    }
    g_swMemRecordIndex = -1;
    (VOID)LOS_EventWrite(&g_stMemshowEvent, 0x112);
}

static inline VOID osMemRecordLR(LRNode *stLRNode)
{
    AARCHPTR uwFp, uwtmpFP;
    UINT32 uwIndex = 0;

    stLRNode->LR1 = 0;
    stLRNode->LR2 = 0;
    stLRNode->LR3 = 0;

    uwFp = Get_Fp();
    while( uwFp > OS_SYS_FUNC_ADDR_START && uwFp < OS_SYS_FUNC_ADDR_END && (uwFp % sizeof(AARCHPTR *)) == 0)
    {
        uwtmpFP = uwFp;
        if(uwIndex == 1)
        {
            stLRNode->LR1 = *((AARCHPTR *)(uwtmpFP));
        }
        else if (uwIndex == 2)
        {
            stLRNode->LR2 = *((AARCHPTR *)(uwtmpFP));
        }
        else if (uwIndex == 3)
        {
            stLRNode->LR3 = *((AARCHPTR *)(uwtmpFP));
        }
        uwFp = *((AARCHPTR *)(uwtmpFP - sizeof(AARCHPTR *)));
        uwIndex++;
        if(uwIndex == 4)
        {
            break;
        }
    }
}

static VOID osMemRecordTaskID(VOID)
{
    if (g_stLosTask.pstRunTask)
    {
        g_pSaveMemRecord[g_swMemRecordIndex].uwTaskID = LOS_CurTaskIDGet();
    }
    else
    {
        g_pSaveMemRecord[g_swMemRecordIndex].uwTaskID = 0;
    }
}

static inline VOID osMemRecord(VOID *pPtr, UINT32 uwSize)
{
    UINT64 ullTickCount = g_ullTickCount;
    INT32 swID = 0;
    LRNode stLRNode;
    AddrNode stAddrNode;
    ReqSizeNode stReqSizeNode;

    if(RECORD_LEN - 1 == g_swMemRecordIndex)
    {
        osMemRecordExchange();
    }
    g_swMemRecordIndex++;

    osMemRecordLR(&stLRNode);
    swID = BinTreeInsert(&stLRNode,sizeof(LRNode),(BinNode **)&g_pLRRoot,getLRBinNode,CompareLRNode);
    g_pSaveMemRecord[g_swMemRecordIndex].uwLrID= swID;

    stAddrNode.addr = (AARCHPTR)pPtr;
    swID = BinTreeInsert(&stAddrNode,sizeof(AddrNode),(BinNode **)&g_pAddrRoot,getAddrBinNode,CompareAddrNode);
    g_pSaveMemRecord[g_swMemRecordIndex].uwAddrID= swID;

    g_pSaveMemRecord[g_swMemRecordIndex].ullSysTick = ullTickCount;

    osMemRecordTaskID();

    stReqSizeNode.reqSize = uwSize;
    swID = BinTreeInsert(&stReqSizeNode,sizeof(ReqSizeNode),(BinNode **)&g_pReqSizeRoot,getReqSizeBinNode,CompareReqSizeNode);
    g_pSaveMemRecord[g_swMemRecordIndex].uwOriginSizeID = swID;


}

VOID osMemRecordMalloc(VOID *pPtr, UINT32 uwSize)
{
    if(0 == g_uwMemRecordShowEnable)
    {
        return;
    }
    osMemRecord(pPtr, uwSize);
    g_pSaveMemRecord[g_swMemRecordIndex].swActType = OS_MEM_ALLOC;
}

VOID osMemRecordFree(VOID *pPtr, UINT32 uwSize)
{
    INT32 swActtype = 0;
    if(0 == g_uwMemRecordShowEnable)
    {
        return;
    }
    swActtype = (0 == uwSize) ? OS_MEM_INVALIDFREE : OS_MEM_VALIDFREE;
    osMemRecord(pPtr, uwSize);
    g_pSaveMemRecord[g_swMemRecordIndex].swActType = swActtype;
}

VOID osMemRecordShowTask(VOID)
{
    (VOID)LOS_EventInit(&g_stMemshowEvent);
    while (1)
    {
        (VOID)LOS_EventRead(&g_stMemshowEvent, 0xFFF, LOS_WAITMODE_OR | LOS_WAITMODE_CLR, MEM_RECORDSHOW_TIMEOUT);
        if (g_uwMemRecordShowEnable)
        {
            osMemRecordShow();
        }
    }
}

