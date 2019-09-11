/* @(#)s_ceil.c 5.1 93/09/24 */
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

static const double huges = 1.0e300;

double
ceil(double x)
{
    int i0 = 0, i1 = 0, j0 = 0; /*lint !e578*/
    unsigned i = 0, j = 0;
    EXTRACT_WORDS(i0,i1,x);
    j0 = ((i0>>20)&0x7ff)-0x3ff;
    if(j0<20) {
        if(j0<0) {     /* raise inexact if x != 0 */
        if((huges+x)>0.0) {/* return 0*sign(x) if |x|<1 */
            if(i0<0) {i0=(int)0x80000000;i1=0;}
            else if((i0|i1)!=0) { i0=0x3ff00000;i1=0;}
        }
        } else {
        i = (0x000fffff)>>j0;
        if(((i0&i)|i1)==0) return x; /* x is integral */
        if((huges+x)>0.0) {    /* raise inexact flag */
            if(i0>0) i0 += (0x00100000)>>j0;
            i0 &= (~i); i1=0;
        }
        }
    } else if (j0>51) {
        if(j0==0x400) return x+x;    /* inf or NaN */
        else return x;        /* x is integral */
    } else {
        i = ((unsigned int)(0xffffffff))>>(j0-20);
        if((i1&i)==0) return x;    /* x is integral */
        if((huges+x)>0.0) {         /* raise inexact flag */
        if(i0>0) {
            if(j0==20) i0+=1;
            else {
            j = i1 + (1<<(52-j0));
            if(j<(unsigned)i1) i0+=1;    /* got a carry */
            i1 = j;
            }
        }
        i1 &= (~i);
        }
    }
    INSERT_WORDS(x,i0,i1);
    return x;
}
