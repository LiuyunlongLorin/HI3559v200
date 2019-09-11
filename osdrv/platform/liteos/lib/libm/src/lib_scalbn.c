/* @(#)s_scalbn.c 5.1 93/09/24 */
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

static const double two54   =  1.80143985094819840000e+16; /* 0x43500000, 0x00000000 */
static const double twom54  =  5.55111512312578270212e-17; /* 0x3C900000, 0x00000000 */
static const double huges   = 1.0e+300;
static const double tiny   = 1.0e-300;

double
scalbn (double x, int n)
{
    int k,hx,lx;
    EXTRACT_WORDS(hx,lx,x);
        k = (hx&0x7ff00000)>>20;        /* extract exponent */
        if (k==0) {                /* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
        x *= two54;
        GET_HIGH_WORD(hx,x);
        k = ((hx&0x7ff00000)>>20) - 54;
            if (n< -50000) return tiny*x;     /*underflow*/
        }
        if (k==0x7ff) return x+x;        /* NaN or Inf */
        k = k+n;
        if (k >  0x7fe) return huges*copysign(huges,x); /* overflow  */
        if (k > 0)                 /* normal result */
        {SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20)); return x;}
        if (k <= -54)
            if (n > 50000)  /* in case integer overflow in n+k */
                return huges*copysign(huges,x);    /*overflow*/
            else return tiny*copysign(tiny,x);     /*underflow*/
        k += 54;                /* subnormal result */
    SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20));
        return x*twom54;
}
