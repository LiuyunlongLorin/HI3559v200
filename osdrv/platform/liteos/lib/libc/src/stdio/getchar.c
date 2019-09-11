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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getchar.c   8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
//#include <sys/cdefs.h>
//__FBSDID("$FreeBSD$");

/*
 * A subroutine version of the macro getchar.
 */

#include "stdio.h"
#include "local.h"
#include "asm/hal_platform_ints.h"
#include "hisoc/uart.h"
#include "stdlib.h"

#define INPUT_BUF_SIZE 64 /* Keyboard input buffer size */
#undef getchar
#undef getchar_unlocked
int
getchar(void)
{
    int retval = 0;
#ifndef LOSCFG_FS_VFS
    static char *p_buf = NULL;
    static int num = 0, pos = 0;
    char ch = 0;
#endif
#ifdef LOSCFG_FS_VFS
    FLOCKFILE(stdin);
    /* Orientation set by __sgetc() when buffer is empty. */
    /* ORIENT(stdin, -1); */
    retval = __sgetc(stdin);
    FUNLOCKFILE(stdin);
#else
    hal_interrupt_mask(NUM_HAL_INTERRUPT_UART);
    if(p_buf == NULL)
    {
        if((p_buf = (char *)malloc(INPUT_BUF_SIZE)) == NULL)
        {
            return (EOF);
        }
        pos = 0;
    }

    /* refill buf */
    if(num == 0)
    {
        do{
            ch = uart_getc();
            if(ch == '\r' || ch == '\n')
            {
                (void)uart_putc('\r');
                (void)uart_putc('\n');
                ch = '\n';
            }
            else
            {
                (void)uart_putc(ch);
            }
            if(num <= INPUT_BUF_SIZE - 1)
            {
                p_buf[num] = ch;
                num++;
            }
        }while(ch != '\n');
        pos = 0;
    }

    num--;
    retval = p_buf[pos++];
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_UART);
#endif
    return (retval);
}

int
getchar_unlocked(void)
{

    return (__sgetc(stdin));
}
