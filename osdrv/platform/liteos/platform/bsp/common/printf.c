/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "los_config.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#include "stdlib.h"
#endif
#ifdef LOSCFG_LIB_LIBCMINI
#include "libcmini.h"
#endif
#include "hisoc/uart.h"
#ifdef LOSCFG_FS_VFS
#include "console.h"
#endif
#ifdef LOSCFG_SHELL_DMESG
#include "dmesg.ph"
#endif

/*lint -e42 -e40 -e52 -e529 -e611 -e578 -e539 -e63*/
extern int finite(double x);
extern ssize_t write(int fd, const void *buf, size_t nbytes);
typedef void (*PUTC_FUNC)(unsigned n, void *cookie);

#define quad_t long long
#define u_quad_t unsigned long long
#define  SIZEBUF  256

#define LIBC_STDIO_PRINTF_FLOATING_POINT
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
#ifdef LOSCFG_LIB_LIBM
#include <float.h>      // for DBL_DIG etc. below
#include <math.h>       // for modf()
#endif

/* NOTICE! FOR FULL FLOAT SUPPROT,
   delete or commente the following line
*/
#define SMALL_NUMBER_BUFFER

/* NOTICE! IF STACK IS SMALL, delete or commente the following line,
   then the system will use malloc memory instead
*/
#define USE_STACK_BUFFER

#ifndef USE_STACK_BUFFER
#include "stdlib.h"
#endif

#define MAXEXPINDEX 20
#if defined(SMALL_NUMBER_BUFFER) && defined(USE_STACK_BUFFER)
#define MAXFRACT  20
#define MAXEXP    20
#define BUF             (MAXEXP+MAXFRACT+1+1)     /* + decimal point */
#else
#define MAXFRACT  20
#define MAXEXP    309
#define BUF             (MAXEXP+MAXFRACT+1+1)     /* + decimal point */
#endif

#define DEFPREC         6
static int
cvt( double, int, int, char *, int, char *, char * );

typedef union
{
    INT32 asi32[2];

    INT64 asi64;

    double value;

    struct
    {
        unsigned int fraction3:16;
        unsigned int fraction2:16;
        unsigned int fraction1:16;
        unsigned int fraction0: 4;
        unsigned int exponent :11;
        unsigned int sign     : 1;
    }number;

    struct
    {
        unsigned int function3:16;
        unsigned int function2:16;
        unsigned int function1:16;
        unsigned int function0:3;
        unsigned int quiet:1;
        unsigned int exponent: 11;
        unsigned int sign : 1;
    }nan;

    struct
    {
        UINT32 lsw;
        UINT32 msw;
    }parts;

}libm_ieee_double_shape_type;

#else

#define BUF 40

#endif

/*
 * Flags used during conversion.
 */
#define ALT             0x001           /* alternate form */
#define HEXPREFIX       0x002           /* add 0x or 0X prefix */
#define LADJUST         0x004           /* left adjustment */
#define LONGDBL         0x008           /* long double; unimplemented */
#define LONGINT         0x010           /* long integer */
#define QUADINT         0x020           /* quad integer */
#define SHORTINT        0x040           /* short integer */
#define ZEROPAD         0x080           /* zero (as opposed to blank) pad */
#define FPT             0x100           /* Floating point number */
#define SIZET           0x200           /* size_t */

#define to_digit(c)     ((c) - '0')
#define is_digit(c)     ((unsigned)to_digit(c) <= 9)
#define to_char(n)      ((n) + '0')

#define PADSIZE 32             /* pad chunk size */

#define SARG() \
    (flags&QUADINT ? va_arg(arg, signed long long) : \
     flags&LONGINT ? va_arg(arg, long) : \
     flags&SHORTINT ? (long)(short)va_arg(arg, int) : \
     flags&SIZET ? (long)va_arg(arg, size_t) : \
     (long)va_arg(arg, int))
#define UARG() \
    (flags&QUADINT ? va_arg(arg, unsigned long long) : \
     flags&LONGINT ? va_arg(arg, unsigned long) : \
     flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(arg, int) : \
     flags&SIZET ? va_arg(arg, size_t) : \
     (unsigned long)va_arg(arg, unsigned int))

#define PRINT(ptr,len)            \
    do{                                \
        if (pFputc == NULL)             \
            break;                      \
        int tmp_len = (len);                \
        char* tmp_ptr = (ptr);                \
        while(tmp_len--)                    \
            pFputc(*tmp_ptr++,cookie);    \
    }while(0)                        \


#define PAD(howmany, with)                                                   \
    do{                                                              \
        if ((x = (howmany)) > 0) {                                               \
            while (x > PADSIZE) {                                                \
                PRINT(with, PADSIZE);                                            \
                x -= PADSIZE;                                                    \
            }                                                                    \
            PRINT(with, x);                                                      \
        }                                                                        \
    }while(0)

#define DMESG_PAD(howmany, with)                                        \
    do{                                                                 \
        if ((x = (howmany)) > 0) {                                      \
            while (x > PADSIZE) {                                       \
                (VOID)osLogMemcpyRecord(with, PADSIZE);                 \
                x -= PADSIZE;                                           \
            }                                                           \
            (VOID)osLogMemcpyRecord(with, x);                           \
        }                                                               \
    }while(0)

int __cp(char *dst, char *src, int len,int size, int wBufLen)
{
    char *temp_dst = dst;
    char *temp_src = src;
    int temp_len = len;
    int i= 0;

    if(len <= 0)
    {
        return 0;
    }

    while (temp_len--)
    {
        if(i >= wBufLen- size -2)
        {
            return temp_dst - dst;
        }

        if(*temp_src == '\n')
        {
            *temp_dst++ = '\r';
            i++;
            if(i >= wBufLen - size -2)
            {
                return temp_dst - dst;
            }
        }

        *temp_dst++ = *temp_src++;
        i++;
    }

    return temp_dst - dst;
}

int hex2asc(int n)
{
    n &= 15;
    if(n > 9){
        return ('a' - 10) + n;
    } else {
        return '0' + n;
    }
}

void dputs(const char *s, void (*pFputc)(unsigned n, void *cookie), void *cookie)
{
    while (*s) {
        pFputc(*s++, cookie);
    }
}

#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT

    static char *
_round(double fract, int *expo, char *start, char *end, char ch, char *signp)
{
    double tmp;

    if (!fract)
        tmp = to_digit(ch);
    else
        (void)modf(fract * 10, &tmp);

    if (tmp > 4)
        for (;; --end) {
            if (*end == '.')
                --end;
            if (++*end <= '9')
                break;
            *end = '0';
            if (end == start) {
                if (!expo){          /* f; add extra digit */
                    *--end = '1';
                    --start;
                }
                else {      /* e/E; increment exponent */
                    *end = '1';
                    ++*expo;
                }
                break;
            }
        }
    /* ``"%.3f", (double)-0.0004'' gives you a negative 0. */
    else if (*signp == '-')
        for (;; --end) {
            if (*end == '.')
                --end;
            if (*end != '0')
                break;
            if (end == start)
                *signp = 0;
        }
    return (start);
} // round()

    static char *
index(char *pbuf, int expo, int fmtch)
{
    char *t;
    char expbuf[MAXEXPINDEX];

    *pbuf++ = fmtch;
    if (expo >= 0)
        *pbuf++ = '+';
    else {
        expo = -expo;
        *pbuf++ = '-';
    }

    t = expbuf + MAXEXPINDEX;
    if (expo <= 9) {
        *pbuf++ = '0';
        *pbuf++ = to_char(expo);
    }
    else {
        do {
            *--t = to_char(expo % 10);
        } while ((expo /= 10) > 9);
        *--t = to_char(expo);
        for (; t < expbuf + MAXEXPINDEX; *pbuf++ = *t++);
    }
    return (pbuf);
} // exponent()


    static int
cvt(double number, int prec, int flags, char *signp, int fmtch, char *startp,
        char *endp)
{
    libm_ieee_double_shape_type ieeetype;
    char *pdata = startp;

    ieeetype.value = number;
    *signp = 0;
    if ( ieeetype.number.sign ){  // this checks for <0.0 and -0.0
        *signp = '-';
        number = -number;
    }

    if (!finite(number)) {
        unsigned case_adj;
        if (fmtch == 'f' || fmtch == 'g' || fmtch == 'e') {
            case_adj = 'a' - 'A';
        }
        else {
            case_adj = 0;
        }

        if (!isnan(number)) {  /*lint  !e506*/ // infinite
            *pdata++ = 'I' + case_adj;
            *pdata++ = 'N' + case_adj;
            *pdata++ = 'F' + case_adj;
        }
        else {
            *pdata++ = 'N' + case_adj;
            *pdata++ = 'A' + case_adj;
            *pdata++ = 'N' + case_adj;
        }
    }
    else {
        char *pbuf;
        double fract;
        int dotch, expcount, gfmt;
        double idx, tmp;

        fract = modf(number, &idx);
        dotch = expcount = gfmt = 0;

        /* get an extra slot for rounding. */
        pdata = ++startp;

        /*
         * get integer portion of number; put into the end of the buffer; the
         * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
         */
        pbuf = endp - 1;
        for (; (idx && (pbuf > startp)); ) {
            tmp = modf(idx / 10, &idx);
            *pbuf-- = to_char((int)((tmp + .01) * 10));
            ++expcount;
            if (expcount >= MAXEXP) break;
        }

        if (expcount >= MAXEXP && idx)
        {
            PRINT_ERR("%s, %d, Convert the number to string failed, don't use SMALL_NUMBER_BUFFER \n", __FUNCTION__, __LINE__);
        }

        switch (fmtch) {
            case 'e':
            case 'E':
efmt:
                if (expcount) {
                    *pdata++ = *++pbuf;
                    if (prec || flags&ALT)
                        *pdata++ = '.';

                    for (; prec && ++pbuf < endp; --prec)
                        *pdata++ = *pbuf;

                    if (!prec && ++pbuf < endp) { /*lint !e662*/
                        startp = _round((double)0, &expcount, startp,
                                pdata - 1, *pbuf, signp);
                        fract = 0;
                    }
                    --expcount;
                }
                else if (!fract) {
                    *pdata++ = '0';
                    if (prec || flags&ALT)
                        *pdata++ = '.';
                }
                else {
                    for (expcount = -1;; --expcount) {
                        fract = modf(fract * 10, &tmp);
                        if (tmp)
                            break;
                    }
                    *pdata++ = to_char((int)tmp);
                    if (prec || flags&ALT)
                        *pdata++ = '.';
                }

                /* if requires more precision and some fraction left */
                if (fract) {
                    if (prec)
                        do {
                            fract = modf(fract * 10, &tmp);
                            *pdata++ = to_char((int)tmp);
                        } while (--prec && fract);
                    if (fract)
                        startp = _round(fract, &expcount, startp,
                                pdata - 1, (char)0, signp);
                }
                /* if requires more precision */
                for (; prec-- && pdata < endp; *pdata++ = '0');

                /* unless alternate flag, trim any g/G format trailing 0's */
                if (gfmt && !(flags&ALT)) {
                    while (pdata > startp && *--pdata == '0');
                    if (*pdata == '.')
                        --pdata;
                    ++pdata;
                }
                pdata = index(pdata, expcount, fmtch);
                break;
            case 'f':
                /* reverse integer into beginning of buffer */
                if (!expcount)
                    *pdata++ = '0';
                else
                    for (; ++pbuf < endp; *pdata++ = *pbuf);

                /*
                 * if precision required or alternate flag set, add in a
                 * decimal point.
                 */
                if (prec || flags&ALT)
                    *pdata++ = '.';
                /* if requires more precision and some fraction left */
                if (fract) {
                    if (prec)
                        do {
                            fract = modf(fract * 10, &tmp);
                            *pdata++ = to_char((int)tmp);
                        } while (--prec && fract);
                    if (fract)
                        startp = _round(fract, (int *)NULL, startp,
                                pdata - 1, (char)0, signp);
                }
                for (; prec-- && pdata < endp; *pdata++ = '0');
                break;
            case 'g':
            case 'G':
                /* a precision of 0 is treated as a precision of 1. */
                if (!prec)
                    ++prec;

                if (expcount > prec || (!expcount && fract && fract < .0001)) {
                    --prec;
                    gfmt = 1;
                    fmtch -= 2;             /* G->E, g->e */
                    goto efmt;
                }

                if (!expcount)
                    *pdata++ = '0';
                else
                    for (; ++pbuf < endp; *pdata++ = *pbuf, --prec);

                if (prec || flags&ALT) {
                    *pdata++ = '.';
                    dotch = 1;
                }
                else
                    dotch = 0;

                if (fract) {
                    if (prec) {
                        do {
                            fract = modf(fract * 10, &tmp);
                            *pdata++ = to_char((int)tmp);
                        } while(!tmp);
                        while (--prec && fract) {
                            fract = modf(fract * 10, &tmp);
                            *pdata++ = to_char((int)tmp);
                        }
                    }
                    if (fract)
                        startp = _round(fract, (int *)NULL, startp,
                                pdata - 1, (char)0, signp);
                }
                /* alternate format, adds 0's for precision, else trim 0's */
                if (flags&ALT)
                    for (; prec-- && pdata < endp; *pdata++ = '0');
                else if (dotch) {
                    while (pdata > startp && *--pdata == '0');
                    if (*pdata != '.')
                        ++pdata;
                }
        }
    }
    return (pdata - startp);
} // cvt()

#endif
#ifdef LOSCFG_NET_TELNET
extern volatile UINT32 g_telnetMask; /*lint !e18*/
extern int telnet_client_fd;

int telnet_Mask(void)
{
    return g_telnetMask;
}
#endif

static int __dprintf(const char *format, va_list arg,
        PUTC_FUNC pFputc,char *cookie, int wBufLen)
{
    char *fmt;
    int ch;
    int x,y;
    char *cp;
    int flags;
    int ret;
    int width;
    int prec;
    char sign;
    UINT32 wc;

    u_quad_t _uquad;
    enum {OCT,DEC,HEX} base = OCT;
    int dprec;
    int fieldsz;
    int realsz;
    int size;
    char *xdigs;


#ifdef USE_STACK_BUFFER
    char buf[BUF];
#else
    char *buf = NULL;
#endif

    char ox[2];
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
    char softsign;
    double _double;
    int fpprec;
#endif

    char *str = cookie;
    int tp_len = 0;

    static char blanks[PADSIZE];
    static char zeroes[PADSIZE];

#ifndef USE_STACK_BUFFER
    buf = (char*)malloc(BUF*sizeof(char));
    if (NULL == buf)
    {
        return -1;
    }
#endif


    memset(blanks, ' ', PADSIZE);
    memset(zeroes, '0', PADSIZE);

    xdigs = (char *)NULL;
    fmt = (char*)format;
    ret = 0;

    for(;;){
        cp = (char*)fmt;
        while(x=((wc = *fmt) != 0)){
            fmt += x;
            if(wc == '%'){
                fmt--;
                break;
            }
        }//end while(x=..

        if ((y = fmt - cp) != 0) {
#ifdef LOSCFG_SHELL_DMESG
            if(!osCheckUartLock())
                PRINT(cp,y);/*lint -e42 -e40 -e52*/
            (void)osLogMemcpyRecord(cp,y);
#else
            PRINT(cp,y);/*lint -e42 -e40 -e52*/
#endif
            tp_len = __cp(str,cp,y,ret, wBufLen);
            str += tp_len;
            ret += tp_len;
        }

        if((x <= 0))
        {
            *++str = '\0';   /*lint !e661*/
#ifndef USE_STACK_BUFFER
            free(buf);
#endif
            return ret+1;
        }

        fmt++;
        flags = 0;
        dprec = 0;
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
        fpprec = 0;
#endif
        sign = '\0';
        prec = -1;
        width = 0;

rflag:
        ch = *fmt++;
reswitch:
        switch (ch) {
                case '*':
                    if ((width = va_arg(arg, int)) >= 0)
                        goto rflag;
                    width = -width;
                    /*lint -fallthrough */
                case '-':
                    flags |= LADJUST;
                    goto rflag;
                case ' ':
                    if(!sign)
                        sign = ' ';
                    goto rflag;
                case '.':
                    if ((ch = *fmt++) == '*') {
                        x = va_arg(arg, int);
                        prec = x < 0 ? -1 : x;
                        goto rflag;
                    }
                    x = 0;
                    while (is_digit(ch)) {
                        x = 10 * x + to_digit(ch);
                        ch = *fmt++;
                    }
                    prec = x < 0 ? -1 : x;
                    goto reswitch;
                case '#':
                    flags |= ALT;
                    goto rflag;
                case '+':
                    sign = '+';
                    goto rflag;
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
                case 'L':
                    flags |= LONGDBL;
                    goto rflag;
#endif
                case '0':
                    flags |= ZEROPAD;
                    goto rflag;
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    x = 0;
                    do {
                        x = 10 * x + to_digit(ch);
                        ch = *fmt++;
                    } while (is_digit(ch));
                    width = x;
                    goto reswitch;
                case 'h':
                    flags |= SHORTINT;
                    goto rflag;
                case 'l':
                    if (*fmt == 'l') {
                        fmt++;
                        flags |= QUADINT;
                    } else {
                        flags |= LONGINT;
                    }
                    goto rflag;
                case 'q':
                    flags |= QUADINT;
                    goto rflag;
                case 'c':
                    *(cp = buf) = va_arg(arg, int);
                    size = 1;
                    sign = '\0';
                    break;
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
                case 'e':
                case 'E':
                case 'f':
                case 'g':
                case 'G':
                    _double = va_arg(arg, double);
                    if (prec > MAXFRACT) {
                        if ((ch != 'g' && ch != 'G' && ch != 'e' && ch != 'E') || (flags&ALT))
                        {
                            if (!finite(_double)) {
                                fpprec = 0;
                            }
                            else {
                                fpprec = prec - MAXFRACT;
                            }
                        }
                        prec = MAXFRACT;
                    } else if (prec == -1)
                        prec = DEFPREC;
                    cp = buf;
                    *cp = '\0';
#ifndef USE_STACK_BUFFER
                    size = cvt(_double, prec, flags, &softsign, ch,cp, buf + BUF);
#else
                    size = cvt(_double, prec, flags, &softsign, ch,cp, buf + sizeof(buf));
#endif

                    if (softsign)
                        sign = '-';
                    if (*cp == '\0')
                        cp++;
                    break;
#else
                case 'e':
                case 'E':
                case 'f':
                case 'g':
                case 'G':
                    (void) va_arg(arg, double);
                    cp = "";
                    size = 0;
                    sign = '\0';
                    break;
#endif
                case 'O':
                    flags |= LONGINT;
                    /*lint -fallthrough */
                case 'o':
                    _uquad = UARG();
                    base = OCT;
                    goto nosign;
                case 'D':
                    flags |= LONGINT;
                    /*lint -fallthrough */
                case 'd':
                case 'i':
                    _uquad = SARG();
#ifndef _NO_LONGLONG
                    if ((quad_t)_uquad < 0)
#else
                        if ((long) _uquad < 0)
#endif
                        {
                            _uquad = (~_uquad + 1);
                            sign = '-';
                        }
                    base = DEC;
                    goto number;
                case 'U':
                    flags |= LONGINT;
                    /*lint -fallthrough */
                case 'u':
                    _uquad = UARG();
                    base = DEC;
                    goto nosign;

                /*lint -e48*/
                case 'n':
#ifndef ___NO_LONGLONG
                    if (flags & QUADINT)
                        *va_arg(arg, quad_t *) = ret;
                    else
#endif
                        if (flags & LONGINT)
                            *va_arg(arg, long *) = ret;
                        else if (flags & SHORTINT)
                            *va_arg(arg, short *) = ret;
                        else if (flags & SIZET)
                            *va_arg(arg, size_t *) = ret;
                        else
                            *va_arg(arg, int *) = ret;
                    continue;
                /*lint +e48*/
                case 's':
                    if ((cp = va_arg(arg, char *)) == NULL) /*lint !e64*/
                        cp = (char *)"(null)";
                    if (prec >= 0) {
                        char *p = (char *)memchr(cp, 0, prec);
                        if (p != NULL) {
                            size = p - cp;
                            if (size > prec)
                                size = prec;
                        } else
                            size = prec;
                    } else
                        size = strlen(cp);
                    sign = '\0';
                    break;
                case 'p':
                    _uquad = (unsigned long)va_arg(arg, void *);
                    base = HEX;
                    xdigs = (char *)"0123456789abcdef";
                    flags |= HEXPREFIX;
                    ch = 'x';
                    goto nosign;
                case 'z':
                    flags |= SIZET;
                    goto rflag;
                case 'X':
                    xdigs = (char *)"0123456789ABCDEF";
                    goto hex;
                case 'x':
                    xdigs = (char *)"0123456789abcdef";
hex:            _uquad = UARG();
                base = HEX;
                if (flags & ALT && _uquad != 0)
                    flags |= HEXPREFIX;
nosign:         sign = '\0';

number:         if ((dprec = prec) >= 0)
                    flags &= ~ZEROPAD;
                cp = buf + BUF;
                if (_uquad != 0 || prec != 0) {
                    switch(base){
                        case DEC:
                            if(flags & QUADINT) {
                                while (_uquad >= 10) {
                                    u_quad_t next = _uquad / 10;
                                    *--cp = to_char(_uquad - (next * 10));
                                    _uquad = next;
                                }
                                *--cp = to_char(_uquad);
                            }
                            else {
                                unsigned long v = (unsigned long)_uquad;
                                while (v >= 10) {
                                    unsigned long next = v / 10;
                                    *--cp = to_char(v - (next * 10));
                                    v = next;
                                }
                                *--cp = to_char(v);
                            }
                            break;
                        case HEX:
                            do {
                                *--cp = xdigs[_uquad & 15]; /*lint !e613*/
                                _uquad >>= 4;
                            } while (_uquad);
                            break;
                        case OCT:
                            do{
                                *--cp = to_char(_uquad & 7);
                                _uquad >>= 3;
                            }while(_uquad);
                            if (flags & ALT && *cp != '0')
                                *--cp = '0';
                            break;
                        default:
                            cp = (char *)"bug in __dprintf: bad base";
                            size = strlen(cp);
                            goto skipsize;
                    }
                }//end if(_uquad
                size = buf + BUF - cp;
skipsize:
                break;
                default:
                if (ch == '\0')
                {
#ifndef USE_STACK_BUFFER
                    free(buf);
#endif
                    return ret;
                }
                cp = buf;
                *cp = ch;
                size = 1;
                sign = '\0';
                break;
            }//end switch(ch)..
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
            fieldsz = size + fpprec;
#else
            fieldsz = size;
#endif
            if (sign)
                fieldsz++;
            else if (flags & HEXPREFIX)
                fieldsz+= 2;
            realsz = dprec > fieldsz ? dprec : fieldsz;

            if ((flags & (LADJUST|ZEROPAD)) == 0) {
                if (width - realsz > 0) {
#ifdef LOSCFG_SHELL_DMESG
                    if(!osCheckUartLock())
                        PAD(width - realsz, blanks);/*lint -e42 -e40 -e52*/
                    DMESG_PAD(width - realsz, blanks);
#else
                    PAD(width - realsz, blanks);/*lint -e42 -e40 -e52*/
#endif
                    tp_len = __cp(str,blanks, width - realsz,ret, wBufLen);
                    str += tp_len;
                    ret += tp_len;
                }
            }

            if (sign) {
                {
#ifdef LOSCFG_SHELL_DMESG
                    if(!osCheckUartLock())
                        PRINT(&sign, 1);/*lint -e42 -e40 -e52*/
                    (void)osLogMemcpyRecord(&sign, 1);
#else
                    PRINT(&sign, 1);/*lint -e42 -e40 -e52*/
#endif
                }
                tp_len = __cp(str,&sign,1,ret, wBufLen);
                str += tp_len;
                ret+=tp_len;
            } else if (flags & HEXPREFIX) {
                ox[0] = '0';
                ox[1] = ch;
#ifdef LOSCFG_SHELL_DMESG
                if(!osCheckUartLock())
                    PRINT(ox, 2);/*lint -e42 -e40 -e52*/
                (void)osLogMemcpyRecord(ox, 2);
#else
                PRINT(ox, 2);/*lint -e42 -e40 -e52*/
#endif
                tp_len = __cp(str, ox, 2, ret, wBufLen);
                str += tp_len;
                ret += tp_len;
            }

            if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD) {
                if (width - realsz > 0) {
#ifdef LOSCFG_SHELL_DMESG
                    if(!osCheckUartLock())
                        PAD(width - realsz, zeroes);/*lint -e42 -e40 -e52*/
                    DMESG_PAD(width - realsz, zeroes);
#else
                    PAD(width - realsz, zeroes);/*lint -e42 -e40 -e52*/
#endif
                    tp_len = __cp(str,zeroes, width - realsz,ret, wBufLen);
                    str +=  tp_len;
                    ret += tp_len;
                }
            }

            if (dprec - fieldsz > 0) {
#ifdef LOSCFG_SHELL_DMESG
                if(!osCheckUartLock())
                    PAD(dprec - fieldsz, zeroes);/*lint -e42 -e40 -e52*/
                DMESG_PAD(dprec - fieldsz, zeroes);
#else
                PAD(dprec - fieldsz, zeroes);/*lint -e42 -e40 -e52*/
#endif
                tp_len = __cp(str,zeroes, dprec - fieldsz,ret, wBufLen);
                str += tp_len;
                ret += tp_len;

            }
#ifdef LOSCFG_SHELL_DMESG
            if(!osCheckUartLock())
                PRINT(cp, size);/*lint -e42 -e40 -e52*/
            (void)osLogMemcpyRecord(cp, size);
#else
            PRINT(cp, size);/*lint -e42 -e40 -e52*/
#endif
            tp_len = __cp(str,cp,size,ret, wBufLen);
            ret += tp_len;
            str += tp_len;
#ifdef LIBC_STDIO_PRINTF_FLOATING_POINT
            if(fpprec > 0)
            {
#ifdef LOSCFG_SHELL_DMESG
                if(!osCheckUartLock())
                    PAD(fpprec, zeroes);/*lint -e42 -e40 -e52*/
                DMESG_PAD(fpprec, zeroes);
#else
                PAD(fpprec, zeroes);/*lint -e42 -e40 -e52*/
#endif
                tp_len = __cp(str,zeroes,fpprec,ret, wBufLen);
                str += tp_len;
                ret += tp_len;
            }
#endif
            if (flags & LADJUST) {
                if (width - realsz > 0) {
#ifdef LOSCFG_SHELL_DMESG
                    if(!osCheckUartLock())
                        PAD(width - realsz, blanks);/*lint -e42 -e40 -e52*/
                    DMESG_PAD(width - realsz, blanks);
#else
                    PAD(width - realsz, blanks);/*lint -e42 -e40 -e52*/
#endif
                    tp_len = __cp(str,blanks,width - realsz,ret, wBufLen);
                    str += tp_len;
                    ret += tp_len;
                }
            }
    }//end for(;;)
}


void _dprintf(const char *fmt, va_list ap)
{
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
#else
    char *abuf = NULL;
#endif

#ifndef USE_STACK_BUFFER
    abuf = (char*)malloc(SIZEBUF*sizeof(char));
    if (NULL == abuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif

    (void)__dprintf(fmt, ap, (PUTC_FUNC)uart_fputc, abuf, SIZEBUF);

#ifndef USE_STACK_BUFFER
    free(abuf);
#endif

}

__attribute__ ((noinline)) void dprintf(const char *fmt, ...)
{
    void *pfn_put = (PUTC_FUNC)uart_fputc;
    va_list ap;
    int wBufLen;
    char *bbuf;
#ifdef LOSCFG_PLATFORM_CONSOLE
    int wLen;
#endif
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
    bbuf = abuf;
#else
    bbuf = (char*)malloc(SIZEBUF);
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    wBufLen = SIZEBUF;

    va_start(ap, fmt); /*lint !e530*/

#ifdef LOSCFG_PLATFORM_CONSOLE
    if (console_enable())
    {
        pfn_put = NULL;
    }
    wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
    if (pfn_put == NULL)
    {
        while (wLen >= wBufLen - 1)
        {
#ifdef USE_STACK_BUFFER
            if (wBufLen != SIZEBUF)
                free(bbuf);  /*lint !e424*/
#else
            free(bbuf);
#endif

            wBufLen <<= 1;
            if (wBufLen <= 0)
            {
                PRINT_ERR("%s, %d, length overflow!\n", __FUNCTION__, __LINE__);
                va_end(ap);
                return;
            }

            bbuf = (char*)malloc(wBufLen);
            if (NULL == bbuf)
            {
                PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
                va_end(ap);
                return;
            }
            wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
        }
        *(bbuf + wLen - 1) = 0;
        (VOID)write(STDOUT, bbuf, wLen);
    }
#else
    (VOID)__dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
#endif
#ifdef USE_STACK_BUFFER
    if (wBufLen != SIZEBUF)
        free(bbuf); /*lint !e673*/
#else
    free(bbuf);
#endif


    va_end(ap);
} /*lint !e438 !e593*/

void lk_dprintf(const char *fmt, va_list ap)
{
    void *pfn_put = (PUTC_FUNC)uart_fputc;
#ifdef LOSCFG_PLATFORM_CONSOLE
    int wLen;
#endif
    int wBufLen;
    char *bbuf;
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
    bbuf = abuf;
#else
    bbuf = (char*)malloc(SIZEBUF);
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    wBufLen = SIZEBUF;

#ifdef LOSCFG_PLATFORM_CONSOLE
    if (console_enable() == TRUE)
    {
        pfn_put = NULL;
    }
    wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
    if (pfn_put == NULL)
    {
        while (wLen >= wBufLen - 1)
        {
#ifdef USE_STACK_BUFFER
            if (wBufLen != SIZEBUF)
                free(bbuf);  /*lint !e424*/
#else
            free(bbuf);
#endif
            wBufLen <<= 1;
            if (wBufLen <= 0)
            {
                PRINT_ERR("%s, %d, length overflow!\n", __FUNCTION__, __LINE__);
                return;
            }
            bbuf = (char*)malloc(wBufLen);
            if (NULL == bbuf)
            {
                PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
                return;
            }
            wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
        }
        *(bbuf + wLen - 1) = 0;
        (VOID)write(STDOUT, bbuf, wLen);
    }
#else
    (VOID)__dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
#endif
#ifdef USE_STACK_BUFFER
    if (wBufLen != SIZEBUF)
        free(bbuf);  /*lint !e673*/
#else
    free(bbuf);
#endif
} /*lint !e438 !e593*/

#ifdef LOSCFG_SHELL_DMESG
void dmesg_dprintf(const char *fmt, va_list ap)
{
    void *pfn_put = (PUTC_FUNC)NULL;
#ifdef LOSCFG_PLATFORM_CONSOLE
    int wLen;
#endif
    int wBufLen;
    char *bbuf;
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
    bbuf = abuf;
#else
    bbuf = (char*)malloc(SIZEBUF);
    if (NULL == bbuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    wBufLen = SIZEBUF;

#ifdef LOSCFG_PLATFORM_CONSOLE
    if (console_enable() == TRUE)
    {
        pfn_put = NULL;
    }
    wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
    if (pfn_put == NULL)
    {
        while (wLen >= wBufLen - 1)
        {
#ifdef USE_STACK_BUFFER
            if (wBufLen != SIZEBUF)
                free(bbuf);  /*lint !e424*/
#else
            free(bbuf);
#endif
            wBufLen <<= 1;
            if (wBufLen <= 0)
            {
                PRINT_ERR("%s, %d, length overflow!\n", __FUNCTION__, __LINE__);
                return;
            }
            bbuf = (char*)malloc(wBufLen);
            if (NULL == bbuf)
            {
                PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
                return;
            }
            wLen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
        }
        *(bbuf + wLen - 1) = 0;
        (VOID)write(STDOUT, bbuf, wLen);
    }
#else
    (VOID)__dprintf(fmt, ap, (PUTC_FUNC)pfn_put, bbuf, wBufLen);
#endif
#ifdef USE_STACK_BUFFER
    if (wBufLen != SIZEBUF)
        free(bbuf);  /*lint !e673*/
#else
    free(bbuf);
#endif
} /*lint !e438 !e593*/
#endif


#ifdef LOSCFG_PLATFORM_UART_WITHOUT_VFS
__attribute__ ((noinline)) int printf(const char *fmt, ...)
{
    int uwlen;
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
#else
    char *abuf = NULL;
#endif
    va_list ap;

    va_start(ap, fmt);

#ifndef USE_STACK_BUFFER
    abuf = (char*)malloc(SIZEBUF*sizeof(char));
    if (NULL == abuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    memset(abuf,0 , SIZEBUF);
    uwlen = __dprintf(fmt, ap, (PUTC_FUNC)uart_fputc, abuf, SIZEBUF);

#ifndef USE_STACK_BUFFER
    free(abuf);
#endif
    va_end(ap);

} /*lint !e438*/
#endif

__attribute__ ((noinline)) void los_printf(const char *fmt, ...)
{
    void *pfn_put = (PUTC_FUNC)uart_fputc;
#ifdef LOSCFG_PLATFORM_CONSOLE
    int uwlen;
#endif
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
#else
    char *abuf = NULL;
    abuf = (char*)malloc(SIZEBUF*sizeof(char));
    if (NULL == abuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    va_list ap;
    va_start(ap, fmt); /*lint !e530*/
    memset(abuf,0 , SIZEBUF);

#ifdef LOSCFG_PLATFORM_CONSOLE
    if (console_enable() == TRUE)
    {
        pfn_put = NULL;
    }
    uwlen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, abuf, SIZEBUF);
    if (pfn_put == NULL){
        (VOID)write(STDOUT, abuf, uwlen);
    }
#else
    (VOID)__dprintf(fmt, ap, (PUTC_FUNC)pfn_put, abuf, SIZEBUF);
#endif


#ifndef USE_STACK_BUFFER
    free(abuf);
#endif

    va_end(ap);
} /*lint !e438*/

__attribute__ ((noinline)) void start_printf(const char *fmt, ...)
{
    void *pfn_put = (PUTC_FUNC)uart_fputc;
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
#else
    char *abuf = NULL;
    abuf = (char*)malloc(SIZEBUF*sizeof(char));
    if (NULL == abuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    va_list ap;
    va_start(ap, fmt); /*lint !e530*/
    memset(abuf,0 , SIZEBUF);

    (VOID) __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, abuf, SIZEBUF);

#ifndef USE_STACK_BUFFER
    free(abuf);
#endif

    va_end(ap);
} /*lint !e438*/

__attribute__ ((noinline)) void syslog( int level, char *fmt, ... )
{
    void *pfn_put = (PUTC_FUNC)uart_fputc;
#ifdef LOSCFG_PLATFORM_CONSOLE
    int uwlen;
#endif
#ifdef USE_STACK_BUFFER
    char abuf[SIZEBUF];
#else
    char *abuf = NULL;
    abuf = (char*)malloc(SIZEBUF*sizeof(char));
    if (NULL == abuf)
    {
        PRINT_ERR("%s, %d, malloc failed!\n", __FUNCTION__, __LINE__);
        return;
    }
#endif
    va_list ap;
    va_start(ap, fmt); /*lint !e530*/
    memset(abuf,0 , SIZEBUF);

#ifdef LOSCFG_PLATFORM_CONSOLE
    if (console_enable() == TRUE)
    {
        pfn_put = NULL;
    }
    uwlen = __dprintf(fmt, ap, (PUTC_FUNC)pfn_put, abuf, SIZEBUF);
    if (pfn_put == NULL){
        (VOID)write(STDOUT, abuf, uwlen);
    }
#else
    (VOID)__dprintf(fmt, ap, (PUTC_FUNC)pfn_put, abuf, SIZEBUF);
#endif

#ifndef USE_STACK_BUFFER
    free(abuf);
#endif

    va_end(ap);
} /*lint !e438*/
/*lint -e42 -e40 -e52 -e529 -e611 -e578 -e539 -e63*/
