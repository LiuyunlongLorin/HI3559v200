
/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "math.h"
#include "float.h"


static const double xxx[] = {
         3.33333333333334091986e-01,    /* 3FD55555, 55555563 */
         1.33333333333201242699e-01,    /* 3FC11111, 1110FE7A */
         5.39682539762260521377e-02,    /* 3FABA1BA, 1BB341FE */
         2.18694882948595424599e-02,    /* 3F9664F4, 8406D637 */
         8.86323982359930005737e-03,    /* 3F8226E3, E96E8493 */
         3.59207910759131235356e-03,    /* 3F6D6D22, C9560328 */
         1.45620945432529025516e-03,    /* 3F57DBC8, FEE08315 */
         5.88041240820264096874e-04,    /* 3F4344D8, F2F26501 */
         2.46463134818469906812e-04,    /* 3F3026F7, 1A8D1068 */
         7.81794442939557092300e-05,    /* 3F147E88, A03792A6 */
         7.14072491382608190305e-05,    /* 3F12B80F, 32F0A7E9 */
        -1.85586374855275456654e-05,    /* BEF375CB, DB605373 */
         2.59073051863633712884e-05,    /* 3EFB2A70, 74BF7AD4 */
/* one */     1.00000000000000000000e+00,    /* 3FF00000, 00000000 */
/* pio4 */     7.85398163397448278999e-01,    /* 3FE921FB, 54442D18 */
/* pio4lo */     3.06161699786838301793e-17    /* 3C81A626, 33145C07 */
};
#define    one    xxx[13]
#define    pio4    xxx[14]
#define    pio4lo    xxx[15]
#define    T    xxx
/* INDENT ON */

static const double
zero =  0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
two24 =  1.67772160000000000000e+07, /* 0x41700000, 0x00000000 */
invpio2 =  6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
pio2_1  =  1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
pio2_1t =  6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
pio2_2  =  6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
pio2_2t =  2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
pio2_3  =  2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
pio2_3t =  8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

double
__kernel_tan(double x, double y, int iy) {
    double z, r, v, w, s;
    int ix, hx;

    GET_HIGH_WORD(hx,x);
    ix = hx & 0x7fffffff;            /* high word of |x| */
    if (ix >= 0x3FE59428) {    /* |x| >= 0.6744 */
        if (hx < 0) {
            x = -x;
            y = -y;
        }
        z = pio4 - x;
        w = pio4lo - y;
        x = z + w;
        y = 0.0;
    }
    z = x * x;
    w = z * z;
    r = T[1] + w * (T[3] + w * (T[5] + w * (T[7] + w * (T[9] +
        w * T[11]))));
    v = z * (T[2] + w * (T[4] + w * (T[6] + w * (T[8] + w * (T[10] +
        w * T[12])))));
    s = z * x;
    r = y + z * (s * (r + v) + y);
    r += T[0] * s;
    w = x + r;
    if (ix >= 0x3FE59428) {
        v = (double) iy;
        return (double) (1 - ((hx >> 30) & 2)) *
            (v - 2.0 * (x - (w * w / (w + v) - r)));
    }
    if (iy == 1)
        return w;
    else {
        /*
         * if allow error up to 2 ulp, simply return
         * -1.0 / (x+r) here
         */
        /* compute -1.0 / (x+r) accurately */
        double a, t;
        z = w;
        SET_LOW_WORD(z,0);
        v = r - (z - x);    /* z+v = r+x */
        t = a = -1.0 / w;    /* a = -1.0/w */
        SET_LOW_WORD(t,0);
        s = 1.0 + t * z;
        return t + a * (s + t * v);
    }
}

int
__ieee754_rem_pio2(double x, double *y)
{
    double z,w,t,r,fn;
    double tx[3],ty[2];
    int e0,i,j,nx,n,ix,hx;
    unsigned low;

    GET_HIGH_WORD(hx,x);        /* high word of x */
    ix = hx&0x7fffffff;
    if (ix <= 0x400f6a7a) {        /* |x| ~<= 5pi/4 */
        if ((ix & 0xfffff) == 0x921fb)  /* |x| ~= pi/2 or 2pi/2 */
        goto medium;        /* cancellation -- use medium case */
        if (ix <= 0x4002d97c) {    /* |x| ~<= 3pi/4 */
        if (hx > 0) {
            z = x - pio2_1;    /* one round good to 85 bits */
            y[0] = z - pio2_1t;
            y[1] = (z-y[0])-pio2_1t;
            return 1;
        } else {
            z = x + pio2_1;
            y[0] = z + pio2_1t;
            y[1] = (z-y[0])+pio2_1t;
            return -1;
        }
        } else {
        if (hx > 0) {
            z = x - 2*pio2_1;
            y[0] = z - 2*pio2_1t;
            y[1] = (z-y[0])-2*pio2_1t;
            return 2;
        } else {
            z = x + 2*pio2_1;
            y[0] = z + 2*pio2_1t;
            y[1] = (z-y[0])+2*pio2_1t;
            return -2;
        }
        }
    }
    if (ix <= 0x401c463b) {        /* |x| ~<= 9pi/4 */
        if (ix <= 0x4015fdbc) {    /* |x| ~<= 7pi/4 */
        if (ix == 0x4012d97c)    /* |x| ~= 3pi/2 */
            goto medium;
        if (hx > 0) {
            z = x - 3*pio2_1;
            y[0] = z - 3*pio2_1t;
            y[1] = (z-y[0])-3*pio2_1t;
            return 3;
        } else {
            z = x + 3*pio2_1;
            y[0] = z + 3*pio2_1t;
            y[1] = (z-y[0])+3*pio2_1t;
            return -3;
        }
        } else {
        if (ix == 0x401921fb)    /* |x| ~= 4pi/2 */
            goto medium;
        if (hx > 0) {
            z = x - 4*pio2_1;
            y[0] = z - 4*pio2_1t;
            y[1] = (z-y[0])-4*pio2_1t;
            return 4;
        } else {
            z = x + 4*pio2_1;
            y[0] = z + 4*pio2_1t;
            y[1] = (z-y[0])+4*pio2_1t;
            return -4;
        }
        }
    }
    if(ix<0x413921fb) {    /* |x| ~< 2^20*(pi/2), medium size */
medium:
        /* Use a specialized rint() to get fn.  Assume round-to-nearest. */
        STRICT_ASSIGN(double,fn,x*invpio2+0x1.8p52);
        fn = fn-0x1.8p52;
#ifdef HAVE_EFFICIENT_IRINT
        n  = irint(fn);
#else
        n  = (int)fn;
#endif
        r  = x-fn*pio2_1;
        w  = fn*pio2_1t;    /* 1st round good to 85 bit */
        {
            unsigned int high;
            j  = ix>>20;
            y[0] = r-w;
        GET_HIGH_WORD(high,y[0]);
            i = j-((high>>20)&0x7ff);
            if(i>16) {  /* 2nd iteration needed, good to 118 */
                t  = r;
                w  = fn*pio2_2;
                r  = t-w;
                w  = fn*pio2_2t-((t-r)-w);
                y[0] = r-w;
                GET_HIGH_WORD(high,y[0]);
                i = j-((high>>20)&0x7ff);
                if(i>49)  {    /* 3rd iteration need, 151 bits acc */
                    t  = r;    /* will cover all possible cases */
                    w  = fn*pio2_3;
                    r  = t-w;
                    w  = fn*pio2_3t-((t-r)-w);
                    y[0] = r-w;
                }
            }
        }
        y[1] = (r-y[0])-w;
        return n;
    }
    /*
     * all other (large) arguments
     */
    if(ix>=0x7ff00000) {        /* x is inf or NaN */
        y[0]=y[1]=x-x; return 0;
    }
    /* set z = scalbn(|x|,ilogb(x)-23) */
    GET_LOW_WORD(low,x);
    e0     = (ix>>20)-1046;    /* e0 = ilogb(z)-23; */
    INSERT_WORDS(z, ix - ((int)(e0<<20)), low);
    for(i=0;i<2;i++) {
        tx[i] = (double)((int)(z));
        z     = (z-tx[i])*two24;
    }
    tx[2] = z;
    nx = 3;
    while(tx[nx-1]==zero) nx--;    /* skip zero term */
    n  =  __kernel_rem_pio2(tx,ty,e0,nx,1);
    if(hx<0) {y[0] = -ty[0]; y[1] = -ty[1]; return -n;}
    y[0] = ty[0]; y[1] = ty[1]; return n;
}

double
tan(double x)
{
    double y[2],z=0.0;
    int n, ix;

    /* High word of x. */
    GET_HIGH_WORD(ix,x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix<0x3e400000)            /* x < 2**-27 */
        if((int)x==0) return x;        /* generate inexact */
        return __kernel_tan(x,z,1);
    }

    /* tan(Inf or NaN) is NaN */
    else if (ix>=0x7ff00000) return x-x;        /* NaN */

    /* argument reduction needed */
    else {
        n = __ieee754_rem_pio2(x,y);
        return __kernel_tan(y[0],y[1],1-((n&1)<<1)); /*   1 -- n even
                            -1 -- n odd */
    }
}
