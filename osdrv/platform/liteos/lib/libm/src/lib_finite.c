/* @(#)s_finite.c 5.1 93/09/24 */
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

int finite(double x)
{
    int hx;
    GET_HIGH_WORD(hx,x);
    return (int)((unsigned int)((hx&0x7fffffff)-0x7ff00000)>>31);
}

