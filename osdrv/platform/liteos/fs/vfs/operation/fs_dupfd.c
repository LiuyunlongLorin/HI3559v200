/****************************************************************************
 * fs/vfs/fs_dupfd.c
 *
 *   Copyright (C) 2007-2009, 2011-2014 Gregory Nutt. All rights reserved.
 *   Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 *   All rights reserved.
 *
 *   Author: Gregory Nutt <gnutt@nuttx.org>
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
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"

#include "assert.h"
#include "errno.h"
#include "sched.h"

#include "fs/fs.h"
#include "fs/file.h"

#include "inode/inode.h"
#include "stdlib.h"
#include "string.h"
#if CONFIG_NFILE_DESCRIPTORS > 0

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_dup
 *
 * Description:
 *   Equivalent to the non-standard fs_dupfd() function except that it
 *   accepts a struct file instance instead of a file descriptor.  Currently
 *   used only by file_vfcntl();
 *
 ****************************************************************************/

int file_dup(FAR struct file *filep, int minfd)
{
    int fd2;
    int ret;
    int err;
    struct file *filep2;
    char * fullpath = (char *)NULL;
    const char *relpath = (const char *)NULL;

    /* Verify that fd is a valid, open file descriptor */

    if (NULL == filep->f_inode)
    {
        set_errno(EBADF);
        return VFS_ERROR;
    }

    fullpath = (char*)malloc(strlen(filep->f_path)+1);
    if (!fullpath)
    {
        set_errno(ENOMEM);
        return VFS_ERROR;
    }

    /* Then allocate a new file descriptor for the inode */

    fd2 = files_allocate(filep->f_inode, filep->f_oflags, filep->f_pos, filep->f_priv,minfd);
    if (fd2 < 0)
    {
        inode_release(filep->f_inode);
        free(fullpath);
        set_errno(EMFILE);
        return VFS_ERROR;
    }
    /*lint -e613*/
    filep2 = fs_getfilep(fd2);

    strncpy(fullpath, filep->f_path,strlen(filep->f_path) + 1);
    if (!inode_find(fullpath, &relpath))
    {
        ret = ENOENT;
        free(fullpath);
        goto errout_with_inode;
    }
    filep2->f_path = fullpath;
    filep2->f_relpath = relpath;

    if(filep->f_inode->u.i_mops && filep->f_inode->u.i_mops->dup)
        ret = filep->f_inode->u.i_mops->dup(filep, filep2);
    else
        ret = -ENOSYS;

    if(ret < 0)
        goto errout_with_inode;

    return fd2;

errout_with_inode:
    clear_fd(fd2);
    inode_release(filep2->f_inode);
    free(filep2->f_path);
    filep2->f_oflags  = 0;
    filep2->f_pos     = 0;
    filep2->f_inode   = (struct inode *)NULL;
    filep2->f_priv    = NULL;
    filep2->f_path    = (char *)NULL;
    filep2->f_relpath = (const char *)NULL;
    err               = -ret;
    set_errno(err);
    return VFS_ERROR;

}/*lint +e613*/

/****************************************************************************
 * Name: fs_dupfd OR dup
 *
 * Description:
 *   Clone a file descriptor 'fd' to an arbitray descriptor number (any value
 *   greater than or equal to 'minfd'). If socket descriptors are
 *   implemented, then this is called by dup() for the case of file
 *   descriptors.  If socket descriptors are not implemented, then this
 *   function IS dup().
 *
 ****************************************************************************/

int fs_dupfd(int fd, int minfd)
{
  FAR struct file *filep;

  /* Get the file structure corresponding to the file descriptor. */

  filep = fs_getfilep(fd);
  if (!filep)
    {
      /* The errno value has already been set */

      return VFS_ERROR;
    }

  /* Let file_dup() do the real work */

  return file_dup(filep, minfd);
}

#endif /* CONFIG_NFILE_DESCRIPTORS > 0 */
