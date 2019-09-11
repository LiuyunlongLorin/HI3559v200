/*    $OpenBSD: citrus_utf8.c,v 1.6 2012/12/05 23:19:59 deraadt Exp $ */

/*-
 * Copyright (c) 2002-2004 Tim J. Robbins
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

#include "wchar.h"
#include "uchar.h"

extern size_t c32rtomb(char* s, char32_t c32, mbstate_t* ps);
extern size_t mbrtoc32(char32_t* pc32, const char* s, size_t n, mbstate_t* ps);

size_t wcrtomb(char* s, wchar_t wc, mbstate_t* ps)
{
  static mbstate_t __private_state;
  mbstate_t* state = (ps == NULL) ? &__private_state : ps;

  // Our wchar_t is UTF-32
  return c32rtomb(s, (char32_t)(wc), state); /*lint !e571*/
}

size_t mbrtowc(wchar_t* pwc, const char* s, size_t n, mbstate_t* ps)
{
  static mbstate_t __private_state;
  mbstate_t* state = (ps == NULL) ? &__private_state : ps;

  // Our wchar_t is UTF-32
  return mbrtoc32((char32_t*)(pwc), s, n, state);
}

int mbsinit(const mbstate_t* ps)
{
  return (ps == NULL || (*((const uint32_t*)(ps->__seq)) == 0));
}
