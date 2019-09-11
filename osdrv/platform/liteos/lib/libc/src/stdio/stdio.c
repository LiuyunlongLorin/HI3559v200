/*    $OpenBSD: stdio.c,v 1.9 2005/08/08 08:05:36 espie Exp $ */
/************************************************************************
 * Copyright (c) 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
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


#include "fcntl.h"
#include "stdio.h"
#include "local.h"
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "fs/file.h"
#endif
#include "unistd.h"

#define _offset _unused_0
#define __SOFF  0x1000      /* set iff _offset is in fact correct */

/*
 * Small standard I/O/seek/close functions.
 * These maintain the `known seek offset' for seek optimisation.
 */
int
__sread(void *cookie, char *buf, int n)
{
    int ret = 0;
#ifdef LOSCFG_FS_VFS
    FILE *fp = (FILE *)cookie;

    ret = read(fp->_file, buf, n);
    /* if the read succeeded, update the current offset */
    if (ret >= 0)
        fp->_offset += ret;
    else
        fp->_flags &= ~__SOFF;    /* paranoia */
#endif
    return (ret);
}

int
__swrite(void *cookie, const char *buf, int n)
{
#ifdef LOSCFG_FS_VFS
    FILE *fp = (FILE *)cookie;

    if (fp->_flags & __SAPP)
        (void) lseek(fp->_file, (off_t)0, SEEK_END);
    fp->_flags &= ~__SOFF;    /* in case FAPPEND mode is set */
    return (write(fp->_file, buf, n));
#endif
}

fpos_t
__sseek(void *cookie, fpos_t offset, int whence)
{
    off_t ret = 0;
#ifdef LOSCFG_FS_VFS
    FILE *fp = (FILE *)cookie;
    FAR struct file *filep;

    if(whence == 0 && offset < 0)
    {
        filep = fs_getfilep(fp->_file);
        ret = (off_t)-1;
        fp->_offset = 0;
        if(filep != NULL)  filep->f_pos = ret;
        fp->_flags |= __SOFF;
        return (ret);
    }
    else
    ret = lseek(fp->_file, (off_t)offset, whence);
    if (ret == (off_t)-1)
        fp->_flags &= ~__SOFF;
    else {
        fp->_flags |= __SOFF;
        fp->_offset = ret;
    }
#endif
    return (ret);
}

off64_t __sseek64(void* cookie, off64_t offset, int whence) {
    PRINT_ERR("%s NOT support!\n", __FUNCTION__);
    return -1;
}

int
__sclose(void *cookie)
{
#ifdef LOSCFG_FS_VFS
    return (close(((FILE *)cookie)->_file));
#endif
}
