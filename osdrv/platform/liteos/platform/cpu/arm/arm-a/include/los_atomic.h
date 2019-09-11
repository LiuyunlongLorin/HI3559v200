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

 /**@defgroup los_atomic Atomic
 * @ingroup kernel
 */

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_ARCH_CORTEX_A7
long __builtin_expect(long exp, long c);
#endif
/**
* @ingroup  los_atomic
* @brief Atomic addition.
*
* @par Description:
* This API is used to implement the atomic addition and return the result value of the augend.
* @attention
* <ul>
* <li>The pointer pswVal must not be NULL.</li>
* <li>If the addtion result is not in the range of representable values for 32-bit signed integer, an int integer overflow may occur to the return value</li>
* </ul>
*
* @param  pswVal      [IN] The augend pointer.
* @param  swAddVal    [IN] The addend.
*
* @retval #INT32  The result value of the augend.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
#ifdef LOSCFG_ARCH_ARM926
extern INT32 LOS_AtomicAdd(volatile INT32 *pswVal, INT32 swAddVal);

/**
* @ingroup  los_atomic
* @brief Atomic subtraction.
*
* @par Description:
* This API is used to implement the atomic subtraction and return the result value of the minuend.
* @attention
* <ul>
* <li>The pointer pswVal must not be NULL.</li>
* <li>If the subtraction result is not in the range of representable values for 32-bit signed integer, an int integer overflow may occur to the return value</li>
* </ul>
*
* @param  pswVal      [IN] The minuend pointer.
* @param  swAddVal    [IN] The subtrahend.
*
* @retval #INT32  The result value of the minuend.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
extern INT32 LOS_AtomicSub(volatile INT32 *pswVal, INT32 swAddVal);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE INT32 LOS_AtomicAdd(volatile INT32 *pswVal, INT32 swAddVal)
{
    INT32 swValue = 0;
    UINT32  uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %1, [%2]\n"
        "add   %1, %1, %3\n"
        "strex   %0, %1, [%2]"
        : "=&r" (uwStatus), "=&r" (swValue)
        : "r" (pswVal), "r" (swAddVal)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}

STATIC INLINE INT32 LOS_AtomicSub(volatile INT32 *pswVal, INT32 swAddVal)
{
    INT32 swValue = 0;
    UINT32  uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %1, [%2]\n"
        "sub   %1, %1, %3\n"
        "strex   %0, %1, [%2]"
        : "=&r" (uwStatus), "=&r" (swValue)
        : "r" (pswVal), "r" (swAddVal)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE INT64 LOS_AtomicAdd(volatile VOID *pswVal, UINT64 swAddVal)
{
    INT64 swValue = 0;
    UINT32  uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %1, %2\n"
        "add   %1, %1, %3\n"
        "stxr   %w0, %1, %2"
        : "=&r" (uwStatus), "=&r" (swValue), "+Q" (*(UINT64 *)pswVal)
        : "r" (swAddVal)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}

STATIC INLINE INT64 LOS_AtomicSub(volatile VOID *pswVal, UINT64 swAddVal)
{
    INT64 swValue = 0;
    UINT32  uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %1, %2\n"
        "sub   %1, %1, %3\n"
        "stxr   %w0, %1, %2"
        : "=&r" (uwStatus), "=&r" (swValue), "+Q" (*(UINT64 *)pswVal)
        : "r" (swAddVal)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}
#endif
/**
* @ingroup  los_atomic
* @brief Atomic addSelf.
*
* @par Description:
* This API is used to implement the atomic addSelf .
* @attention
* <ul>
* <li>The pointer pswAddr must not be NULL.</li>
* <li>The value which pswAddr point to must not be INT_MAX to avoid integer overflow after adding 1.</li>
* </ul>
*
* @param  pswAddr      [IN] The addSelf variable pointer.
*
* @retval none.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
#ifdef LOSCFG_ARCH_ARM926
extern VOID LOS_AtomicInc(volatile INT32 *pswAddr);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE VOID LOS_AtomicInc(volatile INT32 *pswAddr)
{
    INT32 swValue;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %0, [%3]\n"
        "add   %0, %0, #1\n"
        "strex   %1, %0, [%3]"
        : "=&r" (swValue), "=&r" (uwStatus), "+m" (*pswAddr)
        : "r" (pswAddr)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE VOID LOS_AtomicInc(volatile VOID *pswAddr)
{
    UINT64 swValue;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %0, %2\n"
        "add   %0, %0, #1\n"
        "stxr   %w1, %0, %2"
        : "=&r" (swValue), "=&r" (uwStatus), "+Q" (*(UINT64 *)pswAddr)
        );
    } while (__builtin_expect(uwStatus != 0, 0));
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic addSelf.
*
* @par Description:
* This API is used to implement the atomic addSelf and return the result of addSelf.
* @attention
* <ul>
* <li>The pointer pswAddr must not be NULL.</li>
* <li>The value which pswAddr point to must not be INT_MAX to avoid integer overflow after adding 1.</li>
* </ul>
*
* @param  pswAddr      [IN] The addSelf variable pointer.
*
* @retval #INT32 The return value of variable addSelf.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
#ifdef LOSCFG_ARCH_ARM926
extern INT32 LOS_AtomicIncRet(volatile INT32 *pswAddr);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE INT32 LOS_AtomicIncRet(volatile INT32 *pswAddr)
{
    INT32 swValue = 0;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldrex   %0, [%3]\n"
        "add   %0, %0, #1\n"
        "strex   %1, %0, [%3]"
        : "=&r" (swValue), "=&r" (uwStatus), "+m" (*pswAddr)
        : "r" (pswAddr)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE INT64 LOS_AtomicIncRet(volatile VOID *pswAddr)
{
    UINT64 swValue = 0;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %0, %2\n"
        "add   %0, %0, #1\n"
        "stxr   %w1, %0, %2"
        : "=&r" (swValue), "=&r" (uwStatus), "+Q" (*(UINT64 *)pswAddr)
        );
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic auto-decrement.
*
* @par Description:
* This API is used to implementating the atomic auto-decrement.
* @attention
* <ul>
* <li>The pointer pswAddr must not be NULL.</li>
* <li>The value which pswAddr point to must not be INT_MIN to avoid overflow after reducing 1.</li>
* </ul>
*
* @param  pswAddr      [IN] The auto-decrement variable pointer.
*
* @retval none.
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
#ifdef LOSCFG_ARCH_ARM926
extern VOID LOS_AtomicDec(volatile INT32 *pswAddr);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE VOID LOS_AtomicDec(volatile INT32 *pswAddr)
{
    INT32 swValue;
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
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE VOID LOS_AtomicDec(volatile VOID *pswAddr)
{
    UINT64 swValue;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %0, %2\n"
        "sub   %0, %0, #1\n"
        "stxr   %w1, %0, %2"
        : "=&r" (swValue), "=&r" (uwStatus), "+Q" (*(UINT64 *)pswAddr));
    } while (__builtin_expect(uwStatus != 0, 0));
}
#endif

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
#ifdef LOSCFG_ARCH_ARM926
extern INT32 LOS_AtomicDecRet(volatile INT32 *pswAddr);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
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
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE INT64 LOS_AtomicDecRet(volatile VOID *pswAddr)
{
    UINT64 swValue = 0;
    UINT32 uwStatus = 0;

    do
    {
        __asm__ __volatile__(
        "ldxr   %0, %2\n"
        "sub   %0, %0, #1\n"
        "stxr   %w1, %0, %2"
        : "=&r" (swValue), "=&r" (uwStatus), "+Q" (*(UINT64 *)pswAddr)
        );
    } while (__builtin_expect(uwStatus != 0, 0));

    return swValue;
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 8-bit variable.
*
* @par Description:
* This API is used to implement the atomic exchange for 8-bit variable and return the previous value of the atomic variable.
* @attention
* <ul>The pointer pucAddr must not be NULL.</ul>
*
* @param  pucAddr       [IN] The variable pointer.
* @param  uwValue       [IN] The exchange value.
*
* @retval #UINT32       The previous value of the atomic variable
* @par Dependency:
* <ul><li>los_atomic.h: the header file that contains the API declaration.</li></ul>
* @see
* @since Huawei LiteOS V100R001C00
*/
#ifdef LOSCFG_ARCH_ARM926
extern UINT32 LOS_AtomicXchgByte(volatile UINT8 *pucAddr, UINT32 uwValue);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE UINT32 LOS_AtomicXchgByte(volatile UINT8 *pucAddr, UINT32 uwValue)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldrexb   %0, [%3]\n"
        "strexb   %1, %4, [%3]"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m" (*pucAddr)
        : "r" (pucAddr), "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE UINT32 LOS_AtomicXchgByte(volatile VOID *pucAddr, UINT32 uwValue)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldxrb   %w0, %w2\n"
        "stxrb   %w1, %w3, %w2"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q" (*(UINT8 *)pucAddr)
        : "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 16-bit variable.
*
* @par Description:
* This API is used to implement the atomic exchange for 16-bit variable and return the previous value of the atomic variable.
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
#ifdef LOSCFG_ARCH_ARM926
extern UINT32 LOS_AtomicXchg16bits(volatile UINT16 *puwAddr, UINT32 uwValue);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE UINT32 LOS_AtomicXchg16bits(volatile UINT16 *puwAddr, UINT32 uwValue)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldrexh   %0, [%3]\n"
        "strexh   %1, %4, [%3]"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m" (*puwAddr)
        : "r" (puwAddr), "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE UINT32 LOS_AtomicXchg16bits(volatile VOID *puwAddr, UINT32 uwValue)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldxrh   %w0, %w2\n"
        "stxrh   %w1, %w3, %w2"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q" (*(UINT16 *)puwAddr)
        : "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}
#endif

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
#ifdef LOSCFG_ARCH_ARM926
extern UINT32 LOS_AtomicXchg32bits(volatile UINT32 *puwAddr, UINT32 uwValue);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
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
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE UINT32 LOS_AtomicXchg32bits(volatile VOID *puwAddr, UINT32 uwValue)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldxr   %w0, %w2\n"
        "stxr   %w1, %w3, %w2"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q" (*(UINT32 *)puwAddr)
        : "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}

STATIC INLINE UINT64 LOS_AtomicXchg64bits(volatile VOID *puwAddr, UINT64 uwValue)
{
     UINT64 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
        "ldxr   %0, %2\n"
        "stxr   %w1, %3, %2"
        : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q" (*(UINT64 *)puwAddr)
        : "r" (uwValue)
        : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal;
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 8-bit variable with compare.
*
* @par Description:
* This API is used to implement the atomic exchange for 8-bit variable, if the value of variable is equal to uwOldVal.
* @attention
* <ul>The pointer pucAddr must not be NULL.</ul>
*
* @param  pucAddr       [IN] The variable pointer.
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
#ifdef LOSCFG_ARCH_ARM926
extern BOOL LOS_AtomicCmpXchgByte(volatile UINT8 *pucAddr, UINT32 uwNewVal, UINT32 uwOldVal);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE BOOL LOS_AtomicCmpXchgByte(volatile UINT8 *pucAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "ldrexb %0, [%3]\n"
          "mov %1, #0\n"
          "teq %0, %4\n"
          "strexbeq %1, %5, [%3]"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m"(*pucAddr)
          : "r" (pucAddr), "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE BOOL LOS_AtomicCmpXchgByte(volatile VOID *pucAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "1: ldxrb %w0, %w2\n"
          "    mov %w1, #0\n"
          "    cmp %w0, %w3\n"
          "    b.ne 2f\n"
          "    stxrb %w1, %w4, %w2\n"
          "2:"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q"(*(UINT8 *)pucAddr)
          : "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#endif

/**
* @ingroup  los_atomic
* @brief Atomic exchange for 16-bit variable with compare.
*
* @par Description:
* This API is used to implement the atomic exchange for 16-bit variable, if the value of variable is equal to uwOldVal.
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
#ifdef LOSCFG_ARCH_ARM926
extern BOOL LOS_AtomicCmpXchg16bits(volatile UINT16 *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE BOOL LOS_AtomicCmpXchg16bits(volatile UINT16 *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "ldrexh %0, [%3]\n"
          "mov %1, #0\n"
          "teq %0, %4\n"
          "strexheq %1, %5, [%3]"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m"(*puwAddr)
          : "r" (puwAddr), "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE BOOL LOS_AtomicCmpXchg16bits(volatile VOID *pucAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "1: ldxrh %w0, %w2\n"
          "    mov %w1, #0\n"
          "    cmp %w0, %w3\n"
          "    b.ne 2f\n"
          "    stxrh %w1, %w4, %w2\n"
          "2:"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q"(*(UINT16 *)pucAddr)
          : "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#endif

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
#ifdef LOSCFG_ARCH_ARM926
extern BOOL LOS_AtomicCmpXchg32bits(volatile UINT32 *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal);
#elif defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) ||defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
STATIC INLINE BOOL LOS_AtomicCmpXchg32bits(volatile UINT32 *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "ldrex %0, [%3]\n"
          "mov %1, #0\n"
          "teq %0, %4\n"
          "strexeq %1, %5, [%3]"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+m"(*puwAddr)
          : "r" (puwAddr), "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#elif defined(LOSCFG_ARCH_CORTEX_A53_AARCH64)
STATIC INLINE BOOL LOS_AtomicCmpXchg32bits(volatile VOID *pucAddr, UINT32 uwNewVal, UINT32 uwOldVal)
{
     UINT32 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "1: ldxr %w0, %w2\n"
          "    mov %w1, #0\n"
          "    cmp %w0, %w3\n"
          "    b.ne 2f\n"
          "    stxr %w1, %w4, %w2\n"
          "2:"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q"(*(UINT32 *)pucAddr)
          : "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}

STATIC INLINE BOOL LOS_AtomicCmpXchg64bits(volatile VOID *puwAddr, UINT64 uwNewVal, UINT64 uwOldVal)
{
     UINT64 uwPrevVal = 0;
     UINT32 uwStatus = 0;

     do
    {
        __asm__ __volatile__(
          "1: ldxr %0, %2\n"
          "    mov %w1, #0\n"
          "    cmp %0, %3\n"
          "    b.ne 2f\n"
          "    stxr %w1, %4, %2\n"
          "2:"
          : "=&r" (uwPrevVal), "=&r" (uwStatus), "+Q"(*(UINT64 *)puwAddr)
          : "r" (uwOldVal), "r" (uwNewVal)
          : "cc");
    } while (__builtin_expect(uwStatus != 0, 0));

     return uwPrevVal != uwOldVal;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ATOMIC_H__ */
