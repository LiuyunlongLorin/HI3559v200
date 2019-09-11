 /*  $NetBSD: strcoll.c,v 1.10 2012/06/25 22:32:46 abs Exp $ */

 /*-
  * Copyright (c) 1990, 1993
  *  The Regents of the University of California.  All rights reserved.
  *
  * This code is derived from software contributed to Berkeley by
  * Chris Torek.
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



 /****************************************************************************
  * Included Files
  ****************************************************************************/

 #include "string.h"

/*****************************************************************************
 Function         :  strcoll
 Description      :  Compares the string pointed to by str1 to the string pointed
                   to by str2.
 Input            :  [1] const char *str1, A pointer of string;
                   [2] const char *str2, A pointer of string.
 Output           :  nothing.
 Return           :  Returns zero if str1 and str2 are equal;
                   Returns less than zero or greater than zero if str1 is less
                   than or greater than str2 respectively.
 *****************************************************************************/
int strcoll(const char *str1, const char *str2)
{
    int ret = 0;

    ret = strcmp(str1, str2);

    return ret;
}

/* EOF strcoll.c */

