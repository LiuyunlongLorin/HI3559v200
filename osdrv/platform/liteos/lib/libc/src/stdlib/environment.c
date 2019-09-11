/*    $OpenBSD: exit.c,v 1.12 2007/09/03 14:40:16 millert Exp $ */
/*-
 * Copyright (c) 1990 The Regents of the University of California.
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
 /*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "stdlib.h"
#include "string.h"
#include "los_exc.h"

int __isthreaded = 1;

void (*__cleanup)(void);

/****
not support
****/
int system(const char *string)
{
    return 0;
}

char *getenv(const char *name)
{
#ifdef LOSCFG_LLTREPORT
    if(strncmp(name, "GCOV_PREFIX", sizeof("GCOV_PREFIX")) == 0)
    {
#ifdef LOSCFG_LLTSER
        extern const char *gcov_dir;
        return (char *)gcov_dir;
#else
        return "/bin/vs/sd";
#endif
    }
    if(strncmp(name, "GCOV_PREFIX_STRIP", sizeof("GCOV_PREFIX_STRIP")) == 0)
    {
        return "6";
    }
#endif
    return (char *)NULL;
}

void abort(void)
{
    LOS_Panic("System was being aborted\n");
    while (1)
        ;
}

int atexit(void (*func)(void))
{
    return 0;
}

void _exit(int status)
{

}

void exit(int status)
{
    while (1)
        ;
}
