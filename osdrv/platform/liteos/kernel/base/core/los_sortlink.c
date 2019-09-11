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

#include "los_base.h"
#include "los_memory.h"
#include "los_sortlink.ph"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


LITE_OS_SEC_TEXT_INIT UINT32 osSortLinkInit(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader)
{
    UINT32 uwSize;
    LOS_DL_LIST *pstListObject;
    UINT32 uwIndex;

    uwSize = sizeof(LOS_DL_LIST) << OS_TSK_SORTLINK_LOGLEN;
    pstListObject = (LOS_DL_LIST *)LOS_MemAlloc(m_aucSysMem0, uwSize);
    if (NULL == pstListObject)
    {
        return LOS_NOK;
    }

    (VOID)memset((void *)pstListObject, 0, uwSize);
    pstSortLinkHeader->pstSortLink = pstListObject;
    pstSortLinkHeader->usCursor = 0;
    for (uwIndex = 0; uwIndex < OS_TSK_SORTLINK_LEN; uwIndex++, pstListObject++)
    {
        LOS_ListInit(pstListObject);
    }
    return LOS_OK;
}

LITE_OS_SEC_TEXT VOID osAdd2SortLink(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader,  SORTLINK_LIST *pstSortList)
{
    SORTLINK_LIST *pstListSorted;
    LOS_DL_LIST *pstListObject;
    UINT32 uwSortIndex;
    UINT32 uwRollNum;
    UINT32 uwTimeout;

    /*  huge rollnum could cause carry to invalid high bit
        and eventually affect the calculation of sort index */
    if (pstSortList->uwIdxRollNum > OS_TSK_MAX_ROLLNUM)
    {
        SET_SORTLIST_VALUE(pstSortList, OS_TSK_MAX_ROLLNUM);
    }

    uwTimeout = pstSortList->uwIdxRollNum;

    uwSortIndex = uwTimeout & OS_TSK_SORTLINK_MASK;
    uwRollNum = (uwTimeout >> OS_TSK_SORTLINK_LOGLEN) + 1;
    (uwSortIndex > 0) ? 0 : (uwRollNum--);
    EVALUATE_L(pstSortList->uwIdxRollNum, uwRollNum);

    uwSortIndex = (uwSortIndex + pstSortLinkHeader->usCursor);
    uwSortIndex = uwSortIndex & OS_TSK_SORTLINK_MASK;
    EVALUATE_H(pstSortList->uwIdxRollNum, uwSortIndex);

    pstListObject = pstSortLinkHeader->pstSortLink + uwSortIndex;
    if (pstListObject->pstNext == pstListObject)
    {
        LOS_ListTailInsert(pstListObject, &pstSortList->stList);
    }
    else
    {
        pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
        do
        {
            if (UWROLLNUM(pstListSorted->uwIdxRollNum) <= UWROLLNUM(pstSortList->uwIdxRollNum))
            {
                UWROLLNUMSUB( pstSortList->uwIdxRollNum, pstListSorted->uwIdxRollNum);
            }
            else
            {
                UWROLLNUMSUB(pstListSorted->uwIdxRollNum, pstSortList->uwIdxRollNum);
                break;
            }

            pstListSorted = LOS_DL_LIST_ENTRY(pstListSorted->stList.pstNext, SORTLINK_LIST, stList); /*lint !e413*/
        } while (&pstListSorted->stList != (pstListObject));

        LOS_ListTailInsert(&pstListSorted->stList, &pstSortList->stList);
    }
}

LITE_OS_SEC_TEXT VOID osDeleteSortLink(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader, SORTLINK_LIST *pstSortList)
{
    LOS_DL_LIST  *pstListObject;
    SORTLINK_LIST  *pstNextSortList;
    UINT32 uwSortIndex;

    uwSortIndex = UWSORTINDEX(pstSortList->uwIdxRollNum);
    pstListObject = pstSortLinkHeader->pstSortLink + uwSortIndex;

    if (pstListObject != pstSortList->stList.pstNext)
    {
        pstNextSortList = LOS_DL_LIST_ENTRY(pstSortList->stList.pstNext, SORTLINK_LIST, stList);
        UWROLLNUMADD(pstNextSortList->uwIdxRollNum, pstSortList->uwIdxRollNum);
    }
    LOS_ListDelete(&pstSortList->stList);
}

LITE_OS_SEC_TEXT STATIC UINT32 osCalcExpierTime(UINT32 uwRollNum, UINT32 uwSortIndex, UINT16 usCurSortIndex)
{
    UINT32 uwExpireTime;

    if(uwSortIndex > usCurSortIndex)
    {
        uwSortIndex = (UINT32)(uwSortIndex - (UINT32)usCurSortIndex);
    }
    else
    {
        uwSortIndex = (UINT32)(OS_TSK_SORTLINK_LEN - (UINT32)usCurSortIndex + uwSortIndex);
    }
    uwExpireTime = ((uwRollNum - 1) << OS_TSK_SORTLINK_LOGLEN) + uwSortIndex;
    return uwExpireTime;
}

LITE_OS_SEC_TEXT UINT32 osSortLinkGetNextExpireTime(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader)
{
    UINT16 usCursor = 0xFFFF;
    UINT32 uwMinSortIndex = 0xFFFFFFFF;
    UINT32 uwMinRollNum = OS_TSK_LOW_BITS_MASK;
    UINT32 uwExpireTime = 0xFFFFFFFF;
    LOS_DL_LIST *pstListObject;
    SORTLINK_LIST *pstListSorted;
    UINT32 i;

    usCursor = (pstSortLinkHeader->usCursor + 1) & OS_TSK_SORTLINK_MASK;

    for(i = 0; i < OS_TSK_SORTLINK_LEN; i++)
    {
        pstListObject = pstSortLinkHeader->pstSortLink + ((usCursor + i) & OS_TSK_SORTLINK_MASK);
        if(!LOS_ListEmpty(pstListObject))
        {
            pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
            if(UWROLLNUM(pstListSorted->uwIdxRollNum) < uwMinRollNum)
            {
                uwMinRollNum = UWROLLNUM(pstListSorted->uwIdxRollNum);
                uwMinSortIndex = (usCursor + i) & OS_TSK_SORTLINK_MASK;
            }
        }
    }

    if(uwMinRollNum != OS_TSK_LOW_BITS_MASK)
    {
        uwExpireTime = osCalcExpierTime(uwMinRollNum, uwMinSortIndex, pstSortLinkHeader->usCursor);
    }

    return (UINT32)uwExpireTime;
}


LITE_OS_SEC_TEXT VOID osSortLinkUpdateExpireTime(UINT32 uwSleepTicks, SORTLINK_ATTRIBUTE_S *pstSortLinkHeader)
{
    SORTLINK_LIST *pstSortList;
    LOS_DL_LIST *pstListObject;
    UINT32 i =0;
    UINT32 uwSortIndex;
    UINT32 uwRollNum;
    if(0 == uwSleepTicks)
    {
        return;
    }
    uwSortIndex = uwSleepTicks & OS_TSK_SORTLINK_MASK;
    uwRollNum = (uwSleepTicks >> OS_TSK_SORTLINK_LOGLEN) + 1;
    (uwSortIndex > 0) ? 0 : (uwRollNum--);

    for (i = 0; i < OS_TSK_SORTLINK_LEN; i++)
    {
        pstListObject = pstSortLinkHeader->pstSortLink + ((pstSortLinkHeader->usCursor + i) & OS_TSK_SORTLINK_MASK);

        if (pstListObject->pstNext != pstListObject)
        {
            pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
            if (uwSortIndex ==  0)
            {
                uwSortIndex = OS_TSK_SORTLINK_LEN;
            }
            UWROLLNUMSUB(pstSortList->uwIdxRollNum, uwRollNum -1);
            if(i > 0 && i < uwSortIndex)
            {
                UWROLLNUMDEC(pstSortList->uwIdxRollNum);
            }
        }
    }
    pstSortLinkHeader->usCursor = (pstSortLinkHeader->usCursor + uwSleepTicks -1) % OS_TSK_SORTLINK_LEN;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osSortLinkGetTargetExpireTime(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader, SORTLINK_LIST *pstTargetSortList)
{
    SORTLINK_LIST *pstListSorted;
    LOS_DL_LIST *pstListObject;
    UINT32 uwSortIndex = UWSORTINDEX(pstTargetSortList->uwIdxRollNum);
    UINT32 uwRollNum = UWROLLNUM(pstTargetSortList->uwIdxRollNum);

    pstListObject = pstSortLinkHeader->pstSortLink + uwSortIndex;

    pstListSorted = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList); /*lint !e413*/
    while (pstListSorted != pstTargetSortList)
    {
        uwRollNum += UWROLLNUM(pstListSorted->uwIdxRollNum);
        pstListSorted = LOS_DL_LIST_ENTRY((pstListSorted->stList).pstNext, SORTLINK_LIST, stList);
    }
    return osCalcExpierTime(uwRollNum, uwSortIndex, pstSortLinkHeader->usCursor);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
