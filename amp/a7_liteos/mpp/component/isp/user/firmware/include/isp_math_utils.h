/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_math_utils.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2013/01/16
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __ISP_MATH_UTILS_H__
#define __ISP_MATH_UTILS_H__

#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ISP_BITMASK(bit)      ((1 << (bit)) - 1)
#define ISP_BITFIX(bit)       ((1 << (bit)))
#define ISP_SQR(x)            ((x) * (x))

HI_U8  sqrt16(HI_U32 arg);
HI_U8  log16(HI_U32 arg);
HI_U16 Sqrt32(HI_U32 u32Arg);
HI_S32 LinearInter(HI_S32 v, HI_S32 x0, HI_S32 y0, HI_S32 x1, HI_S32 y1);
HI_S32 CalcMulCoef(HI_S32 x0, HI_S32 y0, HI_S32 x1, HI_S32 y1, HI_U8 sft);
HI_U32 math_log2(const HI_U32 val, const HI_U8 out_precision, const HI_U8 shift_out);
HI_U32 log2_int_to_fixed(const HI_U32 val, const HI_U8 out_precision, const HI_U8 shift_out);
HI_U32 math_exp2(HI_U32 val, const unsigned char shift_in, const unsigned char shift_out);
HI_S32 solving_lin_equation_a(HI_S32 y1, HI_S32 y2, HI_S32 x1, HI_S32 x2, HI_S16 a_fraction_size);
HI_S32 solving_lin_equation_b(HI_S32 y1, HI_S32 a, HI_S32 x1, HI_S16 a_fraction_size);
HI_U32 div_fixed(HI_U32 a, HI_U32 b, const HI_U16 fraction_size);
HI_S32 solving_nth_root_045(HI_S32 x, const HI_U16 fraction_size);
HI_U32 transition(HI_U32 *lut_in, HI_U32 *lut_out, HI_U32 lut_size, HI_U32 value, HI_U32 value_fraction_size);
HI_S64 SignedRightShift(HI_S64 s64Value, HI_U8 u8BitShift);
HI_S64 SignedLeftShift(HI_S64 s64Value, HI_U8 u8BitShift);
void MemsetU16(HI_U16 *pVir, HI_U16 temp, HI_U32 size);
void MemsetU32(HI_U32 *pVir, HI_U32 temp, HI_U32 size);

HI_U8 GetIsoIndex(HI_U32 u32Iso);

void *ISP_MALLOC(unsigned long size);
#define ISP_FREE(ptr)         \
    do {                      \
        if (HI_NULL != ptr) { \
            free(ptr);        \
            ptr = HI_NULL;    \
        }                     \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // __ISP_MATH_UTILS_H__
