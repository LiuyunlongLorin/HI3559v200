/* $OpenBSD: strerror_r.c,v 1.6 2005/08/08 08:05:37 espie Exp $ */
/* Public Domain <marc@snafu.org> */
/************************************************************************
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 * Copyright (C) 2007, 2009, 2011-2012 Gregory Nutt. All rights reserved.
 * Author: Gregory Nutt <gnutt@nuttx.org>
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


#include "errno.h"
#include "unistd.h"
#include "string.h"
#include "stdio.h"

struct Pair {
  int code;
  const char* msg;
};

static const char* __code_string_lookup(const struct Pair* strings, int code) {
    size_t i;
    for (i = 0; strings[i].msg != NULL; ++i) { /*lint !e520*/
        if (strings[i].code == code) {
            return strings[i].msg;
        }
    }
    return (char *)NULL;
} /*lint !e550*/

#ifdef CONFIG_STRERROR
static const struct Pair _sys_error_strings[] = {
#define  __BIONIC_ERRDEF(x,y,z)  { x, z },
#include <sys/_errdefs.h>
    { 0, NULL }
};
#endif

const char* __strerror_lookup(int error_number) {
#ifdef CONFIG_STRERROR
    return __code_string_lookup(_sys_error_strings, error_number);
#else
    return (char *)NULL;
#endif
}

int strerror_r(int error_number, char* buf, size_t buf_len) {
  //ErrnoRestorer errno_restorer;
  size_t length;

  const char* error_name = __strerror_lookup(error_number);
  if (error_name != NULL) {
    length = snprintf(buf, buf_len, "%s", error_name);
  } else {
    length = snprintf(buf, buf_len, "Unknown error %d", error_number);
  }
  if (length >= buf_len) {
    return -1;
  }

  return 0;
}

int __xpg_strerror_r(int error_number, char* buf, size_t buf_len)
{
    return strerror_r(error_number, buf, buf_len);
}


void perror(const char *prefix)
{
    char   buff[256];

    (void)strerror_r( errno, buff, sizeof(buff) );
#ifdef LOSCFG_FS_VFS

    if (prefix) {
        write( 2, prefix, strlen(prefix) );
        write( 2, ": ", 2 );
    }
    write( 2, buff, strlen(buff) );
    write( 2, "\n", 1 );
#else
    PRINT_ERR("%s:%s\n", prefix,buff);
#endif
}
