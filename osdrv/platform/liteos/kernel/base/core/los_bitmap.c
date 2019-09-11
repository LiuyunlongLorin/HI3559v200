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

#include "los_task.ph"
#include "los_bitmap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define OS_BITMAP_MASK 0x1f

VOID LOS_BitMapSet(UINT32 *puwBitmap, UINT16 usPos)
{
    *puwBitmap |= (1U << (usPos & OS_BITMAP_MASK));
}

VOID LOS_BitMapClr(UINT32 *puwBitmap, UINT16 usPos)
{
    *puwBitmap &= ~(1U << (usPos & OS_BITMAP_MASK));
}

UINT16 LOS_HighBitGet(UINT32 *puwBitmap)
{
    UINT16 usBitNum;
    UINT32 uwVal = *puwBitmap;

    if (uwVal == 0)
    {
        return LOS_INVALID_BIT_INDEX;
    }

    usBitNum = 1;
    if ((uwVal >> 16) == 0)
    {
        usBitNum += 16;
        uwVal <<= 16;
    }
    if ((uwVal >> 24) == 0)
    {
        usBitNum += 8;
        uwVal <<= 8;
    }
    if ((uwVal >> 28) == 0)
    {
        usBitNum += 4;
        uwVal <<= 4;
    }
    if ((uwVal >> 30) == 0)
    {
        usBitNum += 2;
        uwVal <<= 2;
    }
    usBitNum -= uwVal >> 31;

    return 31 - usBitNum;
}

UINT16 LOS_LowBitGet(UINT32 *puwBitmap)
{
    UINT16 usBitNum;
    UINT32 uwVal = *puwBitmap;

    if (uwVal == 0)
    {
        return LOS_INVALID_BIT_INDEX;
    }

    usBitNum = 1;
    if ((uwVal << 16) == 0)
    {
        usBitNum += 16;
        uwVal >>= 16;
    }
    if ((uwVal << 24) == 0)
    {
        usBitNum += 8;
        uwVal >>= 8;
    }
    if ((uwVal << 28) == 0)
    {
        usBitNum += 4;
        uwVal >>= 4;
    }
    if ((uwVal << 30) == 0)
    {
        usBitNum += 2;
        uwVal >>= 2;
    }

    if ((uwVal << 31) & 0x80000000)
    {
        return usBitNum - 1;
    }
    else
    {
        return usBitNum;
    }
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
