/*  $OpenBSD: gets.c,v 1.9 2005/08/08 08:05:36 espie Exp $ */
/************************************************************************
 * Copyright (c) 1990, 1993
 *  The Regents of the University of California.  All rights reserved.
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
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
 ************************************************************************/

/************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country
 * in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ************************************************************************/

#include "stdio.h"
#include "local.h"
#include "asm/hal_platform_ints.h"
#include "hisoc/uart.h"
//__warn_references(gets,
//    "warning: gets() is very unsafe; consider using fgets()");

char *
gets(char *buf)
{
    int c = 0;

#ifdef LOSCFG_FS_VFS
    char *s;

    FLOCKFILE(stdin);
    for (s = buf; c = getchar_unlocked(), (c != '\n') && (c != '\r');)
    {
        if (c == EOF)
            if (s == buf) {
                FUNLOCKFILE(stdin);
                return ((char *)NULL);
            } else
                break;
        else
            *s++ = c;
    }
    *s = '\0';
    FUNLOCKFILE(stdin);
#else
    hal_interrupt_mask(NUM_HAL_INTERRUPT_UART);
    while((buf[c] = uart_getc()) != 0x0d)
    {
        (void)uart_putc(buf[c]);
        c++;
    }
    buf[c] = '\0';
    (void)uart_putc('\r');
    (void)uart_putc('\n');
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_UART);
#endif
    return (buf);
}
