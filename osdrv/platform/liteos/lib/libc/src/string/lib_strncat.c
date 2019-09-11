/*    $OpenBSD: strcat.c,v 1.8 2005/08/08 08:05:37 espie Exp $    */

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
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
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country
 * in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ************************************************************************/

/************************************************************
 * Included Files
 ************************************************************/

#include "string.h"

/*****************************************************************************
 Function         :  strncat
 Description      :  Appends the string pointed to by src to the end of the
                   string pointed to by dst up to n characters long. The
                   terminating null character of dst is overwritten. Copying
                   stops once n characters are copied or the terminating null
                   character of src is copied. A terminating null character
                   is always appended to dst. If overlapping occurs, the
                   result is undefined.
 Input            :  [1] char *dst, A pointer of string;
                   [2] const char *src, A pointer of string;
                   [3] size_t n, Number fo bytes need to copied.
 Output           :  nothing.
 Return           :  The argument dst is returned.
 *****************************************************************************/
char *strncat(char *dst, const char *src, size_t n)
{
    char *to = dst;

    while (*to)
        to++;

    while(n--)
    {
        if(!(*to++ = *src++))
            return dst;
    }

    *to = '\0';

    return dst;
}

/* EOF strncat.c */
