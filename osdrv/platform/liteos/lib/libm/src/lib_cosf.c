/* s_cosf.c -- float version of s_cos.c.
 * k_cosf.c -- float version of k_cos.c
 * k_sinf.c -- float version of k_sin.c
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
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

/************************************************************************
 * Included Files
 ************************************************************************/

#include "math.h"
#include "float.h"

/************************************************************************
 * Public Functions
 ************************************************************************/

/* Small multiples of pi/2 rounded to double precision. */
static const double
c1pio2 = 1*M_PI_2,            /* 0x3FF921FB, 0x54442D18 */
c2pio2 = 2*M_PI_2,            /* 0x400921FB, 0x54442D18 */
c3pio2 = 3*M_PI_2,            /* 0x4012D97C, 0x7F3321D2 */
c4pio2 = 4*M_PI_2;            /* 0x401921FB, 0x54442D18 */

static const double
one =  1.0,
C0  = -0x1ffffffd0c5e81.0p-54,    /* -0.499999997251031003120 */
C1  =  0x155553e1053a42.0p-57,    /*  0.0416666233237390631894 */
C2  = -0x16c087e80f1e27.0p-62,    /* -0.00138867637746099294692 */
C3  =  0x199342e0ee5069.0p-68;    /*  0.0000243904487962774090654 */

/* |sin(x)/x - s(x)| < 2**-37.5 (~[-4.89e-12, 4.824e-12]). */
static const double
S1 = -0x15555554cbac77.0p-55,    /* -0.166666666416265235595 */
S2 =  0x111110896efbb2.0p-59,    /*  0.0083333293858894631756 */
S3 = -0x1a00f9e2cae774.0p-65,    /* -0.000198393348360966317347 */
S4 =  0x16cd878c3b46a7.0p-71;    /*  0.0000027183114939898219064 */


float
__kernel_cosdf(double x)
{
    double r, w, z;

    /* Try to optimize for parallel evaluation as in k_tanf.c. */
    z = x*x;
    w = z*z;
    r = C2+z*C3;
    return ((one+z*C0) + w*C1) + (w*z)*r;
}

float
__kernel_sindf(double x)
{
    double r, s, w, z;

    /* Try to optimize for parallel evaluation as in k_tanf.c. */
    z = x*x;
    w = z*z;
    r = S3+z*S4;
    s = z*x;
    return (x + s*(S1+z*S2)) + s*w*r;
}


float
cosf(float x)
{
    double y;
    int n, hx, ix;

    GET_FLOAT_WORD(hx,x);
    ix = hx & 0x7fffffff;

    if(ix <= 0x3f490fda) {        /* |x| ~<= pi/4 */
        if(ix<0x39800000)        /* |x| < 2**-12 */
        if(((int)x)==0) return 1.0;    /* 1 with inexact if x != 0 */
        return __kernel_cosdf(x);
    }
    if(ix<=0x407b53d1) {        /* |x| ~<= 5*pi/4 */
        if(ix>0x4016cbe3)        /* |x|  ~> 3*pi/4 */
        return -__kernel_cosdf(x + (hx > 0 ? -c2pio2 : c2pio2));
        else {
        if(hx>0)
            return __kernel_sindf(c1pio2 - x);
        else
            return __kernel_sindf(x + c1pio2);
        }
    }
    if(ix<=0x40e231d5) {        /* |x| ~<= 9*pi/4 */
        if(ix>0x40afeddf)        /* |x|  ~> 7*pi/4 */
        return __kernel_cosdf(x + (hx > 0 ? -c4pio2 : c4pio2));
        else {
        if(hx>0)
            return __kernel_sindf(x - c3pio2);
        else
            return __kernel_sindf(-c3pio2 - x);
        }
    }

    /* cos(Inf or NaN) is NaN */
    else if (ix>=0x7f800000) return x-x;

    /* general argument reduction needed */
    else {
        n = __ieee754_rem_pio2f(x,&y);
        switch(n&3) {
        case 0: return  __kernel_cosdf(y);
        case 1: return  __kernel_sindf(-y);
        case 2: return -__kernel_cosdf(y);
        default:
                return  __kernel_sindf(y);
        }
    }
}

