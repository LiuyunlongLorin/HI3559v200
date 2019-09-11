/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <limits.h>
#include <math.h>


#ifndef type
#define type        double
#define roundit     round
#define dtype       long
#define DTYPE_MIN   LONG_MIN
#define DTYPE_MAX   LONG_MAX
#define fn      lround
#endif
#define FE_INVALID 0x01

#ifdef __LP64__
#define FE_ALL_EXCEPT 0x9f

typedef __uint32_t fpu_control_t;   // FPCR, Floating-point Control Register.
typedef __uint32_t fpu_status_t;    // FPSR, Floating-point Status Register.

#define __get(REGISTER, __value) { \
      uint64_t __value64; \
      __asm__ __volatile__("mrs %0," REGISTER : "=r" (__value64)); \
      __value = (__uint32_t) __value64; \
}

#define __set(REGISTER, __value) { \
      uint64_t __value64 = __value; \
      __asm__ __volatile__("msr " REGISTER ",%0" : : "ri" (__value64)); \
}
#endif /*__LP64__*/

typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef __uint32_t fenv_t;
typedef __uint32_t fexcept_t;

static __inline int fegetenv(fenv_t* __envp) {
  fenv_t _fpscr;
#if !defined(__SOFTFP__)
  __asm__ __volatile__("vmrs %0,fpscr" : "=r" (_fpscr));
#else
  _fpscr = 0;
#endif
  *__envp = _fpscr;/*lint !e530*/
  return 0;
}

/*lint -e529*/
static __inline int fesetenv(const fenv_t* __envp) {
  fenv_t _fpscr = *__envp;
#if !defined(__SOFTFP__)
  __asm__ __volatile__("vmsr fpscr,%0" : :"ri" (_fpscr));
#else
  _fpscr = _fpscr;
#endif
  return 0;
}
/*lint +e529*/

static __inline int fesetexceptflag(const fexcept_t* __flagp, int __excepts) {
#ifdef  __LP64__
  fpu_status_t fpsr;
  __excepts &= FE_ALL_EXCEPT;
  __get("fpsr", fpsr);
  fpsr &= ~__excepts;/*lint !e502*/
  fpsr |= *__flagp & __excepts;
  __set("fpsr", fpsr);
#else
  fexcept_t __fpscr;
  (void)fegetenv(&__fpscr);
  __fpscr &= ~__excepts;/*lint !e502*/
  __fpscr |= *__flagp & __excepts;
  (void)fesetenv(&__fpscr);
#endif /*__LP64__*/
  return 0;
}

static __inline int feraiseexcept(int __excepts) {
  fexcept_t __ex = __excepts;
  (void)fesetexceptflag(&__ex, __excepts);
  return 0;
}
/*
 * If type has more precision than dtype, the endpoints dtype_(min|max) are
 * of the form xxx.5; they are "out of range" because lround() rounds away
 * from 0.  On the other hand, if type has less precision than dtype, then
 * all values that are out of range are integral, so we might as well assume
 * that everything is in range.  At compile time, INRANGE(x) should reduce to
 * two floating-point comparisons in the former case, or TRUE otherwise.
 */
static const type dtype_min = DTYPE_MIN - 0.5;
static const type dtype_max = DTYPE_MAX + 0.5;
#define INRANGE(x)  (dtype_max - DTYPE_MAX != 0.5 || \
             ((x) > dtype_min && (x) < dtype_max))

dtype
fn(type x)
{

    if (INRANGE(x)) {
        x = roundit(x);
        return ((dtype)x);
    } else {
        (void)feraiseexcept(FE_INVALID);
        return (DTYPE_MAX);
    }
}
