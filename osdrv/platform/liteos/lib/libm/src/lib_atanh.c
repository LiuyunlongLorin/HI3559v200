/* @(#)e_atanh.c 1.3 95/01/18 */
/* @(#)s_log1p.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */

#include "math.h"
#include "float.h"


static const double one = 1.0;
static const double huges = 1e300;
static const double zero = 0.0;
static volatile double vzero = 0.0;

static const double ln2_hi  =  6.93147180369123816490e-01;    /* 3fe62e42 fee00000 */
static const double ln2_lo  =  1.90821492927058770002e-10;    /* 3dea39ef 35793c76 */
static const double two54   =  1.80143985094819840000e+16;  /* 43500000 00000000 */
static const double Lp1 = 6.666666666666735130e-01;  /* 3FE55555 55555593 */
static const double Lp2 = 3.999999999940941908e-01;  /* 3FD99999 9997FA04 */
static const double Lp3 = 2.857142874366239149e-01;  /* 3FD24924 94229359 */
static const double Lp4 = 2.222219843214978396e-01;  /* 3FCC71C5 1D8E78AF */
static const double Lp5 = 1.818357216161805012e-01;  /* 3FC74664 96CB03DE */
static const double Lp6 = 1.531383769920937332e-01;  /* 3FC39A09 D078C69F */
static const double Lp7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */


double
log1p(double x)
{
    double hfsq = 0, f = 0, c = 0, s = 0, z = 0, R = 0, u = 0;
    int k = 0, hx = 0, hu = 0, ax = 0;

    GET_HIGH_WORD(hx,x);
    ax = hx&0x7fffffff;

    k = 1;
    if (hx < 0x3FDA827A) {            /* 1+x < sqrt(2)+ */
        if(ax>=0x3ff00000) {        /* x <= -1.0 */
        if(x==-1.0) return -two54/vzero; /* log1p(-1)=+inf */
        else return (x-x)/(x-x);    /* log1p(x<-1)=NaN */
        }
        if(ax<0x3e200000) {            /* |x| < 2**-29 */
        if(two54+x>zero            /* raise inexact */
                &&ax<0x3c900000)         /* |x| < 2**-54 */
            return x;
        else
            return x - x*x*0.5;
        }
        if(hx>0||hx<=((int)0xbfd2bec4)) {
        k=0;f=x;hu=1;}        /* sqrt(2)/2- <= 1+x < sqrt(2)+ */
    }
    if (hx >= 0x7ff00000) return x+x;
    if(k!=0) {
        if(hx<0x43400000) {
        STRICT_ASSIGN(double,u,1.0+x);
        GET_HIGH_WORD(hu,u);
            k  = (hu>>20)-1023;
            c  = (k>0)? 1.0-(u-x):x-(u-1.0);/* correction term */
        c /= u;
        } else {
        u  = x;
        GET_HIGH_WORD(hu,u);
            k  = (hu>>20)-1023;
        c  = 0;
        }
        hu &= 0x000fffff;
        /*
         * The approximation to sqrt(2) used in thresholds is not
         * critical.  However, the ones used above must give less
         * strict bounds than the one here so that the k==0 case is
         * never reached from here, since here we have committed to
         * using the correction term but don't use it if k==0.
         */
        if(hu<0x6a09e) {            /* u ~< sqrt(2) */
            SET_HIGH_WORD(u,hu|0x3ff00000);    /* normalize u */
        } else {
            k += 1;
        SET_HIGH_WORD(u,hu|0x3fe00000);    /* normalize u/2 */
            hu = (0x00100000-hu)>>2;
        }
        f = u-1.0;
    }
    hfsq=0.5*f*f;
    if(hu==0) {    /* |f| < 2**-20 */
        if(f==zero) {
        if(k==0) {
            return zero;
        } else {
            c += k*ln2_lo;
            return k*ln2_hi+c;
        }
        }
        R = hfsq*(1.0-0.66666666666666666*f);
        if(k==0) return f-R; else
                 return k*ln2_hi-((R-(k*ln2_lo+c))-f);
    }
    s = f/(2.0+f);
    z = s*s;
    R = z*(Lp1+z*(Lp2+z*(Lp3+z*(Lp4+z*(Lp5+z*(Lp6+z*Lp7))))));
    if(k==0) return f-(hfsq-s*(hfsq+R)); else
         return k*ln2_hi-((hfsq-(s*(hfsq+R)+(k*ln2_lo+c)))-f);
}



double
__ieee754_atanh(double x)
{
    double t;
    int hx,ix;
    unsigned int lx;
    EXTRACT_WORDS(hx,lx,x);
    ix = hx&0x7fffffff;
    if ((ix|((lx|(-lx))>>31))>0x3ff00000) /* |x|>1 */ /*lint !e501*/
        return (x-x)/(x-x);
    if(ix==0x3ff00000)
        return x/zero; /*lint !e414*/
    if(ix<0x3e300000&&(huges+x)>zero) return x;    /* x<2**-28 */
    SET_HIGH_WORD(x,ix);
    if(ix<0x3fe00000) {        /* x < 0.5 */
        t = x+x;
        t = 0.5*log1p(t+t*x/(one-x));
    } else
        t = 0.5*log1p((x+x)/(one-x));
    if(hx>=0) return t; else return -t;
}

double atanh(double x)
{
    return __ieee754_atanh(x);
}
