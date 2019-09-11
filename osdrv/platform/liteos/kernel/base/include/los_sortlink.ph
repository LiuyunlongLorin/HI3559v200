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

#ifndef _LOS_SORTLINK_PH
#define _LOS_SORTLINK_PH

#include "los_typedef.h"
#include "los_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

//  Sortlink Rollnum Structure:
//   ------------------------------------------
//  | 31 | 30 | 29 |.......| 4 | 3 | 2 | 1 | 0 |
//   ------------------------------------------
//  |<-High Bits->|<---------Low Bits--------->|
//
//  Low Bits  : circles
//  High Bits : sortlink index


#define OS_TSK_HIGH_BITS                        (3u)
#define OS_TSK_LOW_BITS                         (32u - OS_TSK_HIGH_BITS)
#define OS_TSK_SORTLINK_LOGLEN                  (OS_TSK_HIGH_BITS)
#define OS_TSK_SORTLINK_LEN                     (1u << OS_TSK_SORTLINK_LOGLEN)
#define OS_TSK_SORTLINK_MASK                    (OS_TSK_SORTLINK_LEN - 1u)
#define OS_TSK_MAX_ROLLNUM                      (0xFFFFFFFFu - OS_TSK_SORTLINK_LEN)
#define OS_TSK_HIGH_BITS_MASK                   (OS_TSK_SORTLINK_MASK << OS_TSK_LOW_BITS)
#define OS_TSK_LOW_BITS_MASK                    (~OS_TSK_HIGH_BITS_MASK)

#define EVALUATE_L(UWNUM, VALUE)        UWNUM = (((UWNUM) & OS_TSK_HIGH_BITS_MASK) | (VALUE))
#define EVALUATE_H(UWNUM, VALUE)        UWNUM = (((UWNUM) & OS_TSK_LOW_BITS_MASK) | ((VALUE) << OS_TSK_LOW_BITS))
#define UWROLLNUMSUB(UWNUM1,UWNUM2)     UWNUM1 = ((UWNUM1 & OS_TSK_HIGH_BITS_MASK) | (UWROLLNUM(UWNUM1) - UWROLLNUM(UWNUM2)))
#define UWROLLNUMADD(UWNUM1,UWNUM2)     UWNUM1 = ((UWNUM1 & OS_TSK_HIGH_BITS_MASK) | (UWROLLNUM(UWNUM1) + UWROLLNUM(UWNUM2)))
#define UWROLLNUMDEC(UWNUM)             UWNUM = (UWNUM - 1)
#define UWROLLNUM(UWNUM) ((UWNUM) & OS_TSK_LOW_BITS_MASK)
#define UWSORTINDEX(UWNUM) (UWNUM >> OS_TSK_LOW_BITS)

#define SET_SORTLIST_VALUE( pstSortList, uwValue)  (((SORTLINK_LIST *)pstSortList)->uwIdxRollNum = (uwValue))


typedef struct SORTLINK_LIST
{
    LOS_DL_LIST stList;
    UINT32      uwIdxRollNum;
} SORTLINK_LIST;

typedef struct tagSortLinkAttr
{
    LOS_DL_LIST    *pstSortLink;
    UINT16          usCursor;
    UINT16          usReserved;
} SORTLINK_ATTRIBUTE_S;

extern UINT32 osSortLinkInit(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader);
extern VOID osAdd2SortLink(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader,  SORTLINK_LIST *pstSortList);
extern VOID osDeleteSortLink(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader, SORTLINK_LIST *pstSortList);
extern UINT32 osSortLinkGetNextExpireTime(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader);
extern UINT32 osSortLinkGetTargetExpireTime(SORTLINK_ATTRIBUTE_S *pstSortLinkHeader, SORTLINK_LIST *pstTargetSortList);
extern VOID osSortLinkUpdateExpireTime(UINT32 uwSleepTicks, SORTLINK_ATTRIBUTE_S *pstSortLinkHeader);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SORTLINK_PH */


