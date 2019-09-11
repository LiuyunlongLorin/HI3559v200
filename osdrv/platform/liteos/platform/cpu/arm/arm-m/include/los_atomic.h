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

#ifndef LOS_ATOMIC_H
#define LOS_ATOMIC_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 32-bit variable.
*
* @par Description:
* This API is used to implement the atomic exchange for 32-bit variable and return the previous value of the atomic variable.
* @attention
* <ul>The pointer puwAddr must not be NULL.</ul>
*
* @param  puwAddr       [IN] The variable pointer.
* @param  uwValue       [IN] The exchange value.
*
* @retval #UINT32       The previous value of the atomic variable
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
STATIC INLINE UINT32 LOS_AtomicXchg32bits(volatile UINT32 *puwAddr, UINT32 uwValue)
{
    UINT32 uwPrevVal = 0;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %0, [%3]\n"
        "strex   %1, %4, [%3]"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m" (*puwAddr)
        : "r" (puwAddr), "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return uwPrevVal;
}

/**
* @ingroup  los_atomic
* @brief Atomic auto-decrement.
*
* @par Description:
* This API is used to implementating the atomic auto-decrement and return the result of auto-decrement.
* @attention
* <ul>
* <li>The pointer pswAddr must not be NULL.</li>
* <li>The value which pswAddr point to must not be INT_MIN to avoid overflow after reducing 1.</li>
* </ul>
*
* @param  pswAddr      [IN] The addSelf variable pointer.
*
* @retval #INT32  The return value of variable auto-decrement.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
STATIC INLINE INT32 LOS_AtomicDecRet(volatile INT32 *pswAddr)
{
    INT32 swValue = 0;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %0, [%3]\n"
        "sub   %0, %0, #1\n"
        "strex   %1, %0, [%3]"
        : "=&r" (swValue), "=&r" (uwStatus), "+m" (*pswAddr)
        : "r" (pswAddr)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 32-bit variable with compare.
*
* @par Description:
* This API is used to implement the atomic exchange for 32-bit variable, if the value of variable is equal to uwOldVal.
* @attention
* <ul>The pointer puwAddr must not be NULL.</ul>
*
* @param  puwAddr       [IN] The variable pointer.
* @param  uwNewVal      [IN] The new value.
* @param  uwOldVal      [IN] The old value.
*
* @retval TRUE  The previous value of the atomic variable is not equal to uwOldVal.
* @retval FALSE The previous value of the atomic variable is equal to uwOldVal.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
STATIC INLINE BOOL LOS_AtomicCmpXchg32bits(volatile VOID *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "1: ldrex %0, %2\n"
          "    mov %1, #0\n"
          "    cmp %0, %3\n"
          "    bne 2f\n"
          "    strex %1, %4, %2\n"
          "2:"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q"(*(UINT32 *)puwAddr)
          : "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* LOS_ATOMIC_H */
