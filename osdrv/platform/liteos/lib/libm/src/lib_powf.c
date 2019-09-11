/* e_powf.c -- float version of e_pow.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
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

#include "math.h"
#include "float.h"

static const float bp[] = {1.0, 1.5,};
static const float dp_h[] = { 0.0, 5.84960938e-01,}; /* 0x3f15c000 */
static const float dp_l[] = { 0.0, 1.56322085e-06,}; /* 0x35d1cfdc */
static const float zero    =  0.0;
static const float one  =  1.0;
static const float two  =  2.0;
static const float two24    =  16777216.0;  /* 0x4b800000 */
static const float two25   =  3.355443200e+07;  /* 0x4c000000 */
static const float twom25  =  2.9802322388e-08; /* 0x33000000 */
static const float huges    =  1.0e+30;
static const float tiny    =  1.0e-30;
    /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
static const float L1  =  6.0000002384e-01; /* 0x3f19999a */
static const float L2  =  4.2857143283e-01; /* 0x3edb6db7 */
static const float L3  =  3.3333334327e-01; /* 0x3eaaaaab */
static const float L4  =  2.7272811532e-01; /* 0x3e8ba305 */
static const float L5  =  2.3066075146e-01; /* 0x3e6c3255 */
static const float L6  =  2.0697501302e-01; /* 0x3e53f142 */
static const float P1   =  1.6666667163e-01; /* 0x3e2aaaab */
static const float P2   = -2.7777778450e-03; /* 0xbb360b61 */
static const float P3   =  6.6137559770e-05; /* 0x388ab355 */
static const float P4   = -1.6533901999e-06; /* 0xb5ddea0e */
static const float P5   =  4.1381369442e-08; /* 0x3331bb4c */
static const float lg2  =  6.9314718246e-01; /* 0x3f317218 */
static const float lg2_h  =  6.93145752e-01; /* 0x3f317200 */
static const float lg2_l  =  1.42860654e-06; /* 0x35bfbe8c */
static const float ovt =  4.2995665694e-08; /* -(128-log2(ovfl+.5ulp)) */
static const float cp    =  9.6179670095e-01; /* 0x3f76384f =2/(3ln2) */
static const float cp_h  =  9.6191406250e-01; /* 0x3f764000 =12b cp */
static const float cp_l  = -1.1736857402e-04; /* 0xb8f623c6 =tail of cp_h */
static const float ivln2    =  1.4426950216e+00; /* 0x3fb8aa3b =1/ln2 */
static const float ivln2_h  =  1.4426879883e+00; /* 0x3fb8aa00 =16b 1/ln2*/
static const float ivln2_l  =  7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/


float
scalbnf (float x, int n)
{
    int k,ix;
    GET_FLOAT_WORD(ix,x);
    k = (ix&0x7f800000)>>23;        /* extract exponent */
    if (k==0) {             /* 0 or subnormal x */
        if ((ix&0x7fffffff)==0) return x; /* +-0 */
    x *= two25;
    GET_FLOAT_WORD(ix,x);
    k = ((ix&0x7f800000)>>23) - 25;
        if (n< -50000) return tiny*x;   /*underflow*/
    }
    if (k==0xff) return x+x;        /* NaN or Inf */
    k = k+n;
    if (k >  0xfe) return huges*copysignf(huges,x); /* overflow  */
    if (k > 0)              /* normal result */
    {SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23)); return x;}
    if (k <= -25)
        if (n > 50000)  /* in case integer overflow in n+k */
            return huges*copysignf(huges,x);    /*overflow*/
        else
            return tiny*copysignf(tiny,x); /*underflow*/
    k += 25;                /* subnormal result */
    SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23));
    return x*twom25;
}



float
__ieee754_powf(float x, float y)
{
    float z,ax,z_h,z_l,p_h,p_l;
    float y1,t1,t2,r,s,sn,t,u,v,w; /*lint !e578*/
    int i,j,k,yisint,n;
    int hx,hy,ix,iy,is;

    GET_FLOAT_WORD(hx,x);
    GET_FLOAT_WORD(hy,y);
    ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

    /* y==zero: x**0 = 1 */
    if(iy==0) return one;

    /* x==1: 1**y = 1, even if y is NaN */
    if (hx==0x3f800000) return one;

    /* y!=zero: result is NaN if either arg is NaN */
    if(ix > 0x7f800000 ||
       iy > 0x7f800000)
        return (x+0.0F)+(y+0.0F);

    /* determine if y is an odd int when x < 0
     * yisint = 0   ... y is not an integer
     * yisint = 1   ... y is an odd int
     * yisint = 2   ... y is an even int
     */
    yisint  = 0;
    if(hx<0) {
        if(iy>=0x4b800000) yisint = 2; /* even integer y */
        else if(iy>=0x3f800000) {
        k = (iy>>23)-0x7f;     /* exponent */
        j = iy>>(23-k);
        if((j<<(23-k))==iy) yisint = 2-(j&1);
        }
    }

    /* special value of y */
    if (iy==0x7f800000) {   /* y is +-inf */
        if (ix==0x3f800000)
            return  one;    /* (-1)**+-inf is NaN */
        else if (ix > 0x3f800000)/* (|x|>1)**+-inf = inf,0 */
            return (hy>=0)? y: zero;
        else            /* (|x|<1)**-,+inf = inf,0 */
            return (hy<0)?-y: zero;
    }
    if(iy==0x3f800000) {    /* y is  +-1 */
        if(hy<0) return one/x; else return x;
    }
    if(hy==0x40000000) return x*x; /* y is  2 */
    if(hy==0x3f000000) {    /* y is  0.5 */
        if(hx>=0)   /* x >= +0 */
        return __ieee754_sqrtf(x);
    }

    ax   = fabsf(x);
    /* special value of x */
    if(ix==0x7f800000||ix==0||ix==0x3f800000){
        z = ax;         /*x is +-0,+-inf,+-1*/
        if(hy<0) z = one/z; /* z = (1/|x|) */
        if(hx<0) {
        if(((ix-0x3f800000)|yisint)==0) {
            z = (z-z)/(z-z); /* (-1)**non-int is NaN */
        } else if(yisint==1)
            z = -z;     /* (x<0)**odd = -(|x|**odd) */
        }
        return z;
    }

    n = ((unsigned int)hx>>31)-1;

    /* (x<0)**(non-int) is NaN */
    if((n|yisint)==0) return (x-x)/(x-x);

    sn = one; /* s (sign of result -ve**odd) = -1 else = 1 */
    if((n|(yisint-1))==0) sn = -one;/* (-ve)**(odd int) */

    /* |y| is huges */
    if(iy>0x4d000000) { /* if |y| > 2**27 */
    /* over/underflow if x is not close to one */
        if(ix<0x3f7ffff8) return (hy<0)? sn*huges*huges:sn*tiny*tiny;
        if(ix>0x3f800007) return (hy>0)? sn*huges*huges:sn*tiny*tiny;
    /* now |1-x| is tiny <= 2**-20, suffice to compute
       log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = ax-1;       /* t has 20 trailing zeros */
        w = (t*t)*((float)0.5-t*((float)0.333333333333-t*(float)0.25));
        u = ivln2_h*t;  /* ivln2_h has 16 sig. bits */
        v = t*ivln2_l-w*ivln2;
        t1 = u+v;
        GET_FLOAT_WORD(is,t1);
        SET_FLOAT_WORD(t1,is&0xfffff000);
        t2 = v-(t1-u);
    } else {
        float s2,s_h,s_l,t_h,t_l;
        n = 0;
    /* take care subnormal number */
        if(ix<0x00800000)
        {ax *= two24; n -= 24; GET_FLOAT_WORD(ix,ax); }
        n  += ((ix)>>23)-0x7f;
        j  = ix&0x007fffff;
    /* determine interval */
        ix = j|0x3f800000;      /* normalize ix */
        if(j<=0x1cc471) k=0;    /* |x|<sqrt(3/2) */
        else if(j<0x5db3d7) k=1;    /* |x|<sqrt(3)   */
        else {k=0;n+=1;ix -= 0x00800000;}
        SET_FLOAT_WORD(ax,ix);

    /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax-bp[k];       /* bp[0]=1.0, bp[1]=1.5 */
        v = one/(ax+bp[k]);
        s = u*v;
        s_h = s;
        GET_FLOAT_WORD(is,s_h);
        SET_FLOAT_WORD(s_h,is&0xfffff000);
    /* t_h=ax+bp[k] High */
        is = ((ix>>1)&0xfffff000)|0x20000000;
        SET_FLOAT_WORD(t_h,is+0x00400000+(k<<21));
        t_l = ax - (t_h-bp[k]);
        s_l = v*((u-s_h*t_h)-s_h*t_l);
    /* compute log(ax) */
        s2 = s*s;
        r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
        r += s_l*(s_h+s);
        s2  = s_h*s_h;
        t_h = (float)3.0+s2+r;
        GET_FLOAT_WORD(is,t_h);
        SET_FLOAT_WORD(t_h,is&0xfffff000);
        t_l = r-((t_h-(float)3.0)-s2);
    /* u+v = s*(1+...) */
        u = s_h*t_h;
        v = s_l*t_h+t_l*s;
    /* 2/(3log2)*(s+...) */
        p_h = u+v;
        GET_FLOAT_WORD(is,p_h);
        SET_FLOAT_WORD(p_h,is&0xfffff000);
        p_l = v-(p_h-u);
        z_h = cp_h*p_h;     /* cp_h+cp_l = 2/(3*log2) */
        z_l = cp_l*p_h+p_l*cp+dp_l[k];
    /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
        t = (float)n;
        t1 = (((z_h+z_l)+dp_h[k])+t);
        GET_FLOAT_WORD(is,t1);
        SET_FLOAT_WORD(t1,is&0xfffff000);
        t2 = z_l-(((t1-t)-dp_h[k])-z_h);
    }

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
    GET_FLOAT_WORD(is,y);
    SET_FLOAT_WORD(y1,is&0xfffff000);
    p_l = (y-y1)*t1+y*t2;
    p_h = y1*t1;
    z = p_l+p_h;
    GET_FLOAT_WORD(j,z);
    if (j>0x43000000)               /* if z > 128 */
        return sn*huges*huges;          /* overflow */
    else if (j==0x43000000) {           /* if z == 128 */
        if(p_l+ovt>z-p_h) return sn*huges*huges;    /* overflow */
    }
    else if ((j&0x7fffffff)>0x43160000)     /* z <= -150 */
        return sn*tiny*tiny;            /* underflow */
    else if (j==(int)0xc3160000){           /* z == -150 */
        if(p_l<=z-p_h) return sn*tiny*tiny;     /* underflow */
    }
    /*
     * compute 2**(p_h+p_l)
     */
    i = j&0x7fffffff;
    k = (i>>23)-0x7f;
    n = 0;
    if(i>0x3f000000) {      /* if |z| > 0.5, set n = [z+0.5] */
        n = j+(0x00800000>>(k+1));
        k = ((n&0x7fffffff)>>23)-0x7f;  /* new k for n */
        SET_FLOAT_WORD(t,n&~(0x007fffff>>k));
        n = ((n&0x007fffff)|0x00800000)>>(23-k);
        if(j<0) n = -n;
        p_h -= t;
    }
    t = p_l+p_h;
    GET_FLOAT_WORD(is,t);
    SET_FLOAT_WORD(t,is&0xffff8000);
    u = t*lg2_h;
    v = (p_l-(t-p_h))*lg2+t*lg2_l;
    z = u+v;
    w = v-(z-u);
    t  = z*z;
    t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
    r  = (z*t1)/(t1-two)-(w+z*w);
    z  = one-(r-z);
    GET_FLOAT_WORD(j,z);
    j += (n<<23);
    if((j>>23)<=0) z = scalbnf(z,n);    /* subnormal output */
    else SET_FLOAT_WORD(z,j);
    return sn*z;
}

float powf  (float b, float e)
{
    return __ieee754_powf(b, e);
}
