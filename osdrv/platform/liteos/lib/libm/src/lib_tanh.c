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

static const double one = 1.0;
static const double two = 2.0;
static const double tiny = 1.0e-300;
static const double huges = 1.0e300;
double
tanh(double x)
{
    double t = 0, z = 0;
    int jx = 0, ix = 0;

    GET_HIGH_WORD(jx,x);
    ix = jx&0x7fffffff;

    /* x is INF or NaN */
    if(ix>=0x7ff00000) {
        if (jx>=0) return one/x+one;    /* tanh(+-inf)=+-1 */
        else       return one/x-one;    /* tanh(NaN) = NaN */
    }

    /* |x| < 22 */
    if (ix < 0x40360000) {        /* |x|<22 */
        if (ix<0x3e300000) {    /* |x|<2**-28 */
        if((huges+x)>one) return x; /* tanh(tiny) = tiny with inexact */
        }
        if (ix>=0x3ff00000) {    /* |x|>=1  */
        t = expm1(two*fabs(x));
        z = one - two/(t+two);
        } else {
            t = expm1(-two*fabs(x));
            z= -t/(t+two);
        }
    /* |x| >= 22, return +-1 */
    } else {
        z = one - tiny;        /* raise inexact flag */
    }
    return (jx>=0)? z: -z;
}

