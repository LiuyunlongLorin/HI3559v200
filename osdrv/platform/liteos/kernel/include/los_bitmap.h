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

/**@defgroup los_bitmap Bitmap
 * @ingroup kernel
 */

#ifndef _LOS_BITMAP_H
#define _LOS_BITMAP_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_bitmap
 * Flag that indicates the invalid bit index.
 *
 * The effective bit index is from 0 to 31.
 */
#define LOS_INVALID_BIT_INDEX 32

/**
 *@ingroup los_bitmap
 *@brief Set one bit.
 *
 *@par Description:
 *This API is used to set one bit of variable according to the parameter.
 *@attention
 *<ul>
 *<li>When the value of usPos is greater than 31, the bit (usPos & 0x1f) of puwBitmap will be set.</li>
 *</ul>
 *@param puwBitmap   [IN] The bitmap variable pointer.
 *@param usPos       [IN] The number bit to be set.
 *
 *@retval None
 *@par Dependency:
 *<ul><li>los_bitmap.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_BitMapClr
 *@since Huawei LiteOS V100R001C00
 */
VOID LOS_BitMapSet(UINT32 *puwBitmap, UINT16 usPos);

/**
 *@ingroup los_bitmap
 *@brief Clear one bit.
 *
 *@par Description:
 *This API is used to clear one bit of variable according to the parameter.
 *@attention
 *<ul>
 *<li>When the value of usPos is greater than 31, the bit (usPos & 0x1f) of puwBitmap will be clear.</li>
 *</ul>
 *@param puwBitmap   [IN] The bitmap variable pointer.
 *@param usPos          [IN] The number bit to be cleared.
 *
 *@retval none.
 *@par Dependency:
 *<ul><li>los_bitmap.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_BitMapSet.
 *@since Huawei LiteOS V100R001C00
 */
VOID LOS_BitMapClr(UINT32 *puwBitmap, UINT16 usPos);

/**
 *@ingroup los_bitmap
 *@brief Find the lowest one bit that is set.
 *
 *@par Description:
 *This API is used to find the lowest one bit that is set and return the bit index.
 *@attention
 *<ul>
 *<li>None</li>
 *</ul>
 *@param puwBitmap   [IN] The bitmap variable pointer.
 *
 *@retval UINT16 The bit index of the lowest one bit that is set.
 *@par Dependency:
 *<ul><li>los_bitmap.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_HighBitGet
 *@since Huawei LiteOS V100R001C00
 */
UINT16 LOS_LowBitGet(UINT32 *puwBitmap);

/**
 *@ingroup los_bitmap
 *@brief Find the highest one bit that is set.
 *
 *@par Description:
 *This API is used to find the highest one bit that is set and return the bit index.
 *@attention
 *<ul>
 *<li>None</li>
 *</ul>
 *@param puwBitmap   [IN] The bitmap variable pointer.
 *
 *@retval UINT16 The bit index of the highest one bit that is set.
 *@par Dependency:
 *<ul><li>los_bitmap.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_LowBitGet
 *@since Huawei LiteOS V100R001C00
 */
UINT16 LOS_HighBitGet(UINT32 *puwBitmap);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_BITMAP_H */

