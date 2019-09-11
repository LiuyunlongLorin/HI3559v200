/*-
 * Copyright (c) 2003 Mike Barcroft <mike@FreeBSD.org>
 * Copyright (c) 2002 David Schultz <das@FreeBSD.ORG>
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
 *
 * $FreeBSD$
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * from: @(#)fdlibm.h 5.1 93/09/24
 * $FreeBSD$
 */

/****************************************************************************
 * include/float.h
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Reference: http://pubs.opengroup.org/onlinepubs/009695399/basedefs/float.h.html
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************/
#ifndef __INCLUDE_FLOAT_H
#define __INCLUDE_FLOAT_H


/* TODO:  These values could vary with architectures toolchains.  This
 * logic should be move at least to the include/arch directory.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#include "sys/limits.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
/* Radix of exponent representation, b. */

#define FLT_RADIX 2

/* Number of base-FLT_RADIX digits in the floating-point significand, p. */

#define FLT_MANT_DIG 24

#ifndef CONFIG_HAVE_DOUBLE
#define CONFIG_HAVE_DOUBLE 1
#endif

#if CONFIG_HAVE_DOUBLE
#  define DBL_MANT_DIG 53
#else
#  define DBL_MANT_DIG FLT_MANT_DIG
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MANT_DIG DBL_MANT_DIG /* FIX ME */
#else
#  define LDBL_MANT_DIG DBL_MANT_DIG
#endif

/* Number of decimal digits, n, such that any floating-point number in the
 * widest supported floating type with pmax radix b digits can be rounded
 * to a floating-point number with n decimal digits and back again without
 * change to the value.
 */

#define DECIMAL_DIG 10

/* Number of decimal digits, q, such that any floating-point number with q
 * decimal digits can be rounded into a floating-point number with p radix
 * b digits and back again without change to the q decimal digits.
 */

#define FLT_DIG 6

#if CONFIG_HAVE_DOUBLE
#  define DBL_DIG 15  /* 10 */
#else
#  define DBL_DIG FLT_DIG
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_DIG DBL_DIG  /* FIX ME */
#else
#  define LDBL_DIG DBL_DIG
#endif

/* Minimum negative integer such that FLT_RADIX raised to that power minus
 * 1 is a normalized floating-point number, emin.
 */

#define FLT_MIN_EXP (-125)

#if CONFIG_HAVE_DOUBLE
#  define DBL_MIN_EXP (-1021)
#else
#  define DBL_MIN_EXP FLT_MIN_EXP
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MIN_EXP DBL_MIN_EXP /* FIX ME */
#else
#  define LDBL_MIN_EXP DBL_MIN_EXP
#endif

/* inimum negative integer such that 10 raised to that power is in the range
 * of normalized floating-point numbers.
 */

#define FLT_MIN_10_EXP (-37)

#if CONFIG_HAVE_DOUBLE
#  define DBL_MIN_10_EXP (-307)  /* -37 */
#else
#  define DBL_MIN_10_EXP FLT_MIN_10_EXP
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MIN_10_EXP DBL_MIN_10_EXP  /* FIX ME */
#else
#  define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#endif

/* Maximum integer such that FLT_RADIX raised to that power minus 1 is a
 * representable finite floating-point number, emax.
 */

#define FLT_MAX_EXP 128

#if CONFIG_HAVE_DOUBLE
#  define DBL_MAX_EXP 1024
#else
#  define DBL_MAX_EXP FLT_MAX_EXP
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MAX_EXP DBL_MAX_EXP /* FIX ME */
#else
#  define LDBL_MAX_EXP DBL_MAX_EXP
#endif

/* Maximum integer such that 10 raised to that power is in the range of
 * representable finite floating-point numbers.
 */

#define FLT_MAX_10_EXP 38  /* 37 */

#if CONFIG_HAVE_DOUBLE
#  define DBL_MAX_10_EXP 308  /* 37 */
#else
#  define DBL_MAX_10_EXP FLT_MAX_10_EXP
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MAX_10_EXP DBL_MAX_10_EXP  /* FIX ME */
#else
#  define LDBL_MAX_10_EXP DBL_MAX_10_EXP
#endif

/* Maximum representable finite floating-point number. */

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MAX DBL_MAX  /* FIX ME */
#else
#  define LDBL_MAX DBL_MAX
#endif

/* The difference between 1 and the least value greater than 1 that is
 * representable in the given floating-point type, b1-p.
 */

#define FLT_EPSILON 1.1920929e-07F  /* 1E-5 */

#if CONFIG_HAVE_DOUBLE
#  define DBL_EPSILON 2.2204460492503131e-16  /* 1E-9 */
#else
#  define DBL_EPSILON FLT_EPSILON
#endif

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_EPSILON DBL_EPSILON /* FIX ME */
#else
#  define LDBL_EPSILON DBL_EPSILON
#endif

/* Minimum normalized positive floating-point number, bemin -1. */

#ifdef CONFIG_HAVE_LONG_DOUBLE
#  define LDBL_MIN DBL_MIN /* FIX ME */
#else
#  define LDBL_MIN DBL_MIN
#endif


/************************add codes 2015.6.9****************************************************/
#define STRICT_ASSIGN(type, lval, rval) ((lval) = (rval))

typedef union
{
  double value;
  struct
  {
    unsigned int lsw;
    unsigned int msw;
  } parts;
  struct
  {
    unsigned long long w;
  } xparts;
} ieee_double_shape_type;

/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)                \
do {                                \
  ieee_double_shape_type ew_u;                    \
  ew_u.value = (d);                        \
  (ix0) = ew_u.parts.msw;                    \
  (ix1) = ew_u.parts.lsw;                    \
} while (0)

/* Get a 64-bit int from a double. */
#define EXTRACT_WORD64(ix,d)                    \
do {                                \
  ieee_double_shape_type ew_u;                    \
  ew_u.value = (d);                        \
  (ix) = ew_u.xparts.w;                        \
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)                    \
do {                                \
  ieee_double_shape_type gh_u;                    \
  gh_u.value = (d);                        \
  (i) = gh_u.parts.msw;                        \
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)                    \
do {                                \
  ieee_double_shape_type gl_u;                    \
  gl_u.value = (d);                        \
  (i) = gl_u.parts.lsw;                        \
} while (0)


/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)                    \
do {                                \
  ieee_double_shape_type sh_u;                    \
  sh_u.value = (d);                        \
  sh_u.parts.msw = (v);                        \
  (d) = sh_u.value;                        \
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)                    \
do {                                \
  ieee_double_shape_type sl_u;                    \
  sl_u.value = (d);                        \
  sl_u.parts.lsw = (v);                        \
  (d) = sl_u.value;                        \
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)                    \
do {                                \
  ieee_double_shape_type iw_u;                    \
  iw_u.parts.msw = (ix0);                    \
  iw_u.parts.lsw = (ix1);                    \
  (d) = iw_u.value;                        \
} while (0)

/* Set a double from a 64-bit int. */
#define INSERT_WORD64(d,ix)                    \
do {                                \
  ieee_double_shape_type iw_u;                    \
  iw_u.xparts.w = (ix);                        \
  (d) = iw_u.value;                        \
} while (0)

/************************float***************************************/
typedef union
{
  float value;
  /* FIXME: Assumes 32 bit int.  */
  unsigned int word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)                    \
do {                                \
  ieee_float_shape_type gf_u;                    \
  gf_u.value = (d);                        \
  (i) = gf_u.word;                        \
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)                    \
do {                                \
  ieee_float_shape_type sf_u;                    \
  sf_u.word = (i);                        \
  (d) = sf_u.value;                        \
} while (0)

union IEEEl2bits {
  long double e;
  struct {
    unsigned long long manl  :64;
    unsigned long long manh  :48;
    unsigned int  exp   :15;
    unsigned int  sign  :1;
  } bits;
  struct {
    unsigned long long manl     :64;
    unsigned long long manh     :48;
    unsigned int  expsign  :16;
  } xbits;
};

#define LDBL_NBIT       0
#define LDBL_IMPLICIT_NBIT
#define mask_nbit_l(u)  ((void)0)

#define LDBL_MANH_SIZE  20
#define LDBL_MANL_SIZE  32

#define LDBL_TO_ARRAY32(u, a) do {  \
    (a)[0] = (unsigned int)(u).bits.manl;   \
    (a)[1] = (unsigned int)(u).bits.manh;   \
} while(0)

extern int __kernel_rem_pio2(double*, double*, int, int, int);
extern int __ieee754_rem_pio2(double, double*);
extern double __kernel_sin(double, double, int);
extern double __kernel_cos(double, double);
extern double __kernel_tan(double, double, int);
extern float __kernel_tandf(double, int);
extern float __kernel_sindf(double);
extern float __kernel_cosdf(double);


extern double __ieee754_sqrt(double x);
extern float __ieee754_sqrtf(float x);

extern double log1p(double x);
extern float log1pf(float x);
extern float __ieee754_logf(float x);

extern double __ieee754_exp(double x);
extern float __ieee754_expf(float x);
extern double __ieee754_log(double x);
extern int __ieee754_rem_pio2f(float x, double *y);

extern float scalbnf(float x, int n);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __INCLUDE_FLOAT_H */
