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

#include "stdint.h"
#include "math.h"
#include "float.h"

static const float huges = 1.0e30F;

float
truncf(float x)
{
    int i0 = 0, j0 = 0; /*lint !e578*/
    unsigned int i = 0;
    GET_FLOAT_WORD(i0,x);
    j0 = ((i0>>23)&0xff)-0x7f;
    if(j0<23) {
        if(j0<0) {     /* raise inexact if x != 0 */
        if((huges+x)>0.0F)        /* |x|<1, so return 0*sign(x) */
            i0 &= 0x80000000;
        } else {
        i = (0x007fffff)>>j0;
        if((i0&i)==0) return x; /* x is integral */
        if((huges+x)>0.0F)        /* raise inexact flag */
            i0 &= (~i);
        }
    } else {
        if(j0==0x80) return x+x;    /* inf or NaN */
        else return x;        /* x is integral */
    }
    SET_FLOAT_WORD(x,i0);
    return x;
}


