/****************************************************************************
 * include/ctype.h
 *
 *   Copyright (C) 2007-2009, 2011, 2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
 /*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "ctype.h"

const char _nl_C_LC_CTYPE_class[2] = {"\002\000"};
const char _nl_C_LC_CTYPE_class32[4] ={"\000\000\002\000"};
const unsigned int _nl_C_LC_CTYPE_toupper[1] = {0x00};
const unsigned int _nl_C_LC_CTYPE_tolower[1] = {0x00};

#define b(t, x, o) (((const t *) _nl_C_LC_CTYPE_##x) + o)

const unsigned short int *__ctype_b = b(unsigned short int, class, 0);
const unsigned int *__ctype32_b = b(unsigned int, class32, 0);
const unsigned int *__ctype_tolower = b(unsigned int, tolower, 0);
const unsigned int *__ctype_toupper = b(unsigned int, toupper, 0);
const unsigned int *__ctype32_tolower = b(unsigned int, tolower, 0);
const unsigned int *__ctype32_toupper = b(unsigned int, toupper, 0);

/****************************************************************************
 * Name: tolower
 *
 * Description:
 *   tolower() converts the letter c to lower case, if possible.
 *
 ****************************************************************************/
int tolower(int c)
{
    return (((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c));
}

/****************************************************************************
 * Name: toupper
 *
 * Description:
 *   toupper() converts the letter c to upper case, if possible.
 *
 ****************************************************************************/
int toupper(int c)
{
    return  (((c) >= 'a' && (c) <= 'z') ? ((c) - 'a' + 'A') : (c));
}
