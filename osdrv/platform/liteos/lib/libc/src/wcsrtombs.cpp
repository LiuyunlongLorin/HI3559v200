/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2018>, <Huawei Technologies Co., Ltd>
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

#include <errno.h>
#include <sys/param.h>
#include <string.h>
#include <wchar.h>
#include <uchar.h>
#include "private/bionic_mbstate.h"

size_t wcsnrtombs(char* dst, const wchar_t** src, size_t nwc, size_t len, mbstate_t* ps)
{
    static mbstate_t __private_state;
    char buf[MB_LEN_MAX];
    size_t i, o, r;
    mbstate_t* state = (ps == NULL) ? &__private_state : ps;

    if (!mbsinit(state) || src == NULL)
    {
        return -1;
    }

    if (dst == NULL)
    {
        for (i = o = 0; i < nwc; i++, o += r)
        {
            wchar_t wc = (*src)[i];
            if (static_cast<uint32_t>(wc) < 0x80)
            {
                if (wc == 0)
                {
                    return o;
                }
                r = 1;
            }
            else
            {
                r = wcrtomb(buf, wc, state);
                if (r == __MB_ERR_ILLEGAL_SEQUENCE)
                {
                    return r;
                }
            }
        }
        return o;
    }

    for (i = o = 0; i < nwc && o < len; i++, o += r)
    {
        wchar_t wc = (*src)[i];
        if (static_cast<uint32_t>(wc) < 0x80)
        {
            dst[o] = wc;
            if (wc == 0)
            {
                *src = (const wchar_t*)NULL;
                return o;
            }
            r = 1;
        }
        else if (len - o >= sizeof(buf))
        {
            r = wcrtomb(dst + o, wc, state);
            if (r == __MB_ERR_ILLEGAL_SEQUENCE)
            {
                *src += i;
                return r;
            }
        }
        else
        {
            r = wcrtomb(buf, wc, state);
            if (r == __MB_ERR_ILLEGAL_SEQUENCE)
            {
                *src += i;
                return r;
            }
            if (r > len - o)
            {
                break;
            }
            memcpy(dst + o, buf, r);
        }
    }
    *src += i;
    return o;
}

size_t wcsrtombs(char* dst, const wchar_t** src, size_t len, mbstate_t* ps)
{
    return wcsnrtombs(dst, src, SIZE_MAX, len, ps);
}
