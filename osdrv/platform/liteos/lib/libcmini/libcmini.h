
#ifndef __LIBCMINI_H
#define __LIBCMINI_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


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

struct ieee_double {
    unsigned int    dbl_fracl;
    unsigned int    dbl_frach:20;
    unsigned int    dbl_exp:11;
    unsigned int    dbl_sign:1;
};

#define    FP_ZERO        0x10
#define    FP_SUBNORMAL    0x08
#define    FP_SUBNORMAL    0x08
#define    FP_INFINITE    0x01
#define    FP_NAN        0x02
#define    DBL_EXP_INFNAN    2047
#define    FP_NORMAL    0x04

typedef union {
  double d;
  struct ieee_double bits;
}double_u;

#define EXTRACT_WORDS(ix0,ix1,d)                \
do {                                \
  ieee_double_shape_type ew_u;                    \
  ew_u.value = (d);                        \
  (ix0) = ew_u.parts.msw;                    \
  (ix1) = ew_u.parts.lsw;                    \
} while (0)

#define INSERT_WORDS(d,ix0,ix1)                    \
do {                                \
  ieee_double_shape_type iw_u;                    \
  iw_u.parts.msw = (ix0);                    \
  iw_u.parts.lsw = (ix1);                    \
  (d) = iw_u.value;                        \
} while (0)


#define GET_HIGH_WORD(i,d)                    \
do {                                \
  ieee_double_shape_type gh_u;                    \
  gh_u.value = (d);                        \
  (i) = gh_u.parts.msw;                        \
} while (0)




typedef __builtin_va_list  __gnuc_va_list;
typedef __gnuc_va_list    va_list;
#define va_start(v,l)     __builtin_va_start(v,l)
#define va_end(v)        __builtin_va_end(v)
#define va_arg(v,l)       __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)



extern void *memcpy(void *dst, const void *src, size_t n);
extern void *memmove(void *dst, const void *src, size_t n);
extern void *memset(void *dst, int c, size_t n);
extern char *strncpy(char *dst, const char *src, size_t n);
extern void *memchr(const void *src, int c, size_t n);
extern int finite(double x);
extern int __fpclassifyd(double d);
extern int __isnan(double d);
extern int isnan(double d);
extern size_t strlen(const char *str);
extern double modf(double x, double *iptr);
extern int raise(int sig);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LIBCMINI_H */


