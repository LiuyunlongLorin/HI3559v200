/************************************************************************
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
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


/* implement flockfile(), ftrylockfile() and funlockfile()
 *
 * we can't use the OpenBSD implementation which uses kernel-specific
 * APIs not available on Linux.
 *
 * Instead, we use a pthread_mutex_t within the FILE* internal state.
 * See fileext.h for details.
 *
 * the behaviour, if fclose() is called while the corresponding
 * file is locked is totally undefined.
 */
#include "stdio.h"
#include "errno.h"
#include "pthread.h"

#include "local.h"

void
flockfile(FILE * fp) /*lint -e454 -e456*/
{
    if (fp != NULL) {
        (void)pthread_mutex_lock(&_FLOCK(fp));
    }
}

int
ftrylockfile(FILE *fp)
{
    /* The specification for ftrylockfile() says it returns 0 on success,
     * or non-zero on error. So return an errno code directly on error.
     */
    if (fp == NULL) {
        return EINVAL;
    }

    return pthread_mutex_trylock(&_FLOCK(fp));
}

void
funlockfile(FILE * fp)
{
    if (fp != NULL) {
        (void)pthread_mutex_unlock(&_FLOCK(fp)); /*lint !e455*/
    }
}
