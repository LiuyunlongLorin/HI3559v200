/* @(#)s_copysign.c 5.1 93/09/24 */
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

#include "math.h"
#include "float.h"

double
copysign(double x, double y)
{
    unsigned int hx,hy;
    GET_HIGH_WORD(hx,x);
    GET_HIGH_WORD(hy,y);
    SET_HIGH_WORD(x,(hx&0x7fffffff)|(hy&0x80000000));
    return x;
}

