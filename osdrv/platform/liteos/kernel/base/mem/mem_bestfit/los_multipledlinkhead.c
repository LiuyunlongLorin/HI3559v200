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

#include "los_multipledlinkhead.inc"

STATIC INLINE UINT32 osLog2(UINT32 uwSize)
{
    return uwSize?(sizeof(uwSize) * 8 - __builtin_clz(uwSize) - 1):0;
}

LITE_OS_SEC_TEXT_INIT VOID osDLnkInitMultiHead(VOID *pHeadAddr)
{
    LOS_MULTIPLE_DLNK_HEAD *pstHead = (LOS_MULTIPLE_DLNK_HEAD *)pHeadAddr;
    LOS_DL_LIST *pstListHead = pstHead->stListHead;
    UINT32 uwIndex;

    for (uwIndex = 0; uwIndex < OS_MULTI_DLNK_NUM; ++uwIndex, ++pstListHead)
    {
        LOS_ListInit(pstListHead);
    }
}

LITE_OS_SEC_TEXT_MINOR LOS_DL_LIST *osDLnkMultiHead(VOID *pHeadAddr, UINT32 uwSize)
{
    LOS_MULTIPLE_DLNK_HEAD *pstHead = (LOS_MULTIPLE_DLNK_HEAD *)pHeadAddr;
    UINT32 uwIndex =  osLog2(uwSize);

    if(uwIndex > OS_MAX_MULTI_DLNK_LOG2)
    {
        return (LOS_DL_LIST *)NULL;
    }

    if(uwIndex <= OS_MIN_MULTI_DLNK_LOG2)
    {
        uwIndex = OS_MIN_MULTI_DLNK_LOG2;
    }

    return pstHead->stListHead + (uwIndex - OS_MIN_MULTI_DLNK_LOG2);
}
