/* @(#)k_sin.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

//#include <sys/types.h>
#include "math.h"
#include "los_typedef.h"
#include "float.h"

static const double
half =  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

double
__kernel_sin(double x, double y, int iy)
{
#if defined(KRAIT_NEON_OPTIMIZATION)
    double z,zz,r,v;

    z    =  x*x;
    zz  =  z*z;
    v    =  z*x;
    r    =  S2+z*((S3+z*S4)+zz*(S5+z*S6));
    if(iy==0) return x+v*(S1+z*r);
    else      return x-((z*(half*y-v*r)-y)-v*S1);
#else
    double z,r,v,w;

    z    =  x*x;
    w    =  z*z;
    r    =  S2+z*(S3+z*S4) + z*w*(S5+z*S6);
    v    =  z*x;
    if(iy==0) return x+v*(S1+z*r);
    else      return x-((z*(half*y-v*r)-y)-v*S1);
#endif
}


double
sin(double x)
{
    double y[2],z=0.0;
    int n, ix;

    /* High word of x. */
    GET_HIGH_WORD(ix,x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix<0x3e500000)            /* |x| < 2**-26 */
           {if((int)x==0) return x;}    /* generate inexact */
        return __kernel_sin(x,z,0);
    }

    /* sin(Inf or NaN) is NaN */
    else if (ix>=0x7ff00000) return x-x;

    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2(x,y);
        switch(n&3) {
        case 0: return  __kernel_sin(y[0],y[1],1);
        case 1: return  __kernel_cos(y[0],y[1]);
        case 2: return -__kernel_sin(y[0],y[1],1);
        default:
            return -__kernel_cos(y[0],y[1]);
        }
    }
}
