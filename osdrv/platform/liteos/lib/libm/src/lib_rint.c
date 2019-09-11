/*-
 * Copyright (c) 2008 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "float.h"
#include "math.h"

static const double
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

double
rint(double x)
{
    int i0,j0,sx; /*lint !e578*/
    unsigned int i,i1;
    double w,t;
    EXTRACT_WORDS(i0,i1,x);
    sx = (i0>>31)&1;
    j0 = ((i0>>20)&0x7ff)-0x3ff;
    if(j0<20) {
        if(j0<0) {
        if(((i0&0x7fffffff)|i1)==0) return x;
        i1 |= (i0&0x0fffff);
        i0 &= 0xfffe0000;
        i0 |= ((i1|-i1)>>12)&0x80000; /*lint !e501*/
        SET_HIGH_WORD(x,i0);
            STRICT_ASSIGN(double,w,TWO52[sx]+x);
            t =  w-TWO52[sx];
        GET_HIGH_WORD(i0,t);
        SET_HIGH_WORD(t,(i0&0x7fffffff)|(sx<<31));
            return t;
        } else {
        i = (0x000fffff)>>j0;
        if(((i0&i)|i1)==0) return x; /* x is integral */
        i>>=1;
        if(((i0&i)|i1)!=0) {
            /*
             * Some bit is set after the 0.5 bit.  To avoid the
             * possibility of errors from double rounding in
             * w = TWO52[sx]+x, adjust the 0.25 bit to a lower
             * guard bit.  We do this for all j0<=51.  The
             * adjustment is trickiest for j0==18 and j0==19
             * since then it spans the word boundary.
             */
            if(j0==19) i1 = 0x40000000; else
            if(j0==18) i1 = 0x80000000; else
            i0 = (i0&(~i))|((0x20000)>>j0);
        }
        }
    } else if (j0>51) {
        if(j0==0x400) return x+x;    /* inf or NaN */
        else return x;        /* x is integral */
    } else {
        i = ((unsigned int)(0xffffffff))>>(j0-20);
        if((i1&i)==0) return x;    /* x is integral */
        i>>=1;
        if((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(j0-20));
    }
    INSERT_WORDS(x,i0,i1);
    STRICT_ASSIGN(double,w,TWO52[sx]+x);
    return w-TWO52[sx];
}

/*added by 10.15 */
long long llrint(double x)
{
    return (long long)rint(x);
}
