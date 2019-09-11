/* @(#)s_cos.c 5.1 93/09/24 */
/* @(#)k_cos.c 1.3 95/01/18 */
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

static const double
one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

double
__kernel_cos(double x, double y)
{
    double hz,z,r,w;

    z  = x*x;
    w  = z*z;
    r  = z*(C1+z*(C2+z*C3)) + w*w*(C4+z*(C5+z*C6));
    hz = 0.5*z;
    w  = one-hz;
    return w + (((one-w)-hz) + (z*r-x*y));
}


double
cos(double x)
{
    double y[2],z=0.0;
    int n, ix;

    /* High word of x. */
    GET_HIGH_WORD(ix,x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix<0x3e46a09e)            /* if x < 2**-27 * sqrt(2) */
        if(((int)x)==0) return 1.0;    /* generate inexact */
        return __kernel_cos(x,z);
    }

    /* cos(Inf or NaN) is NaN */
    else if (ix>=0x7ff00000) return x-x;

    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2(x,y);
        switch(n&3) {
        case 0: return  __kernel_cos(y[0],y[1]);
        case 1: return -__kernel_sin(y[0],y[1],1);
        case 2: return -__kernel_cos(y[0],y[1]);
        default:
                return  __kernel_sin(y[0],y[1],1);
        }
    }
}

