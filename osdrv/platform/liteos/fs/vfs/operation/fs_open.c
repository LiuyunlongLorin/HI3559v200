/****************************************************************************
 * fs/vfs/fs_open.c
 *
 *   Copyright (C) 2007-2009, 2011-2012 Gregory Nutt. All rights reserved.
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

#include "errno.h"
#include "sys/types.h"
#include "fcntl.h"
#include "sched.h"
#include "assert.h"
#ifdef CONFIG_FILE_MODE
#include "stdarg.h"
#endif
#include "stdlib.h"
#include "fs/fs.h"

#include "inode/inode.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: open
 *
 * Description:
 *   Standard 'open' interface
 *
 ****************************************************************************/

int open(const char *path, int oflags, ...)
{
  FAR struct file *filep;
  FAR struct inode *inode_ptr;
  FAR const char *relpath = (const char *)NULL;
  char *fullpath = (char *)NULL;
#if defined(CONFIG_FILE_MODE) || !defined(CONFIG_DISABLE_MOUNTPOINT)
  mode_t mode = 0666;
#endif
  int ret;
  int fd;

#ifdef CONFIG_FILE_MODE
#  ifdef CONFIG_CPP_HAVE_WARNING
#    warning "File creation not implemented"
#  endif

  /* If the file is opened for creation, then get the mode bits */

  if ((oflags & (O_WRONLY|O_CREAT)) != 0)
    {
      va_list ap;
      va_start(ap, oflags);
      mode = va_arg(ap, mode_t);
      va_end(ap);
    }
#endif

  ret = vfs_normalize_path((const char *)NULL, path, &fullpath);
  if (ret < 0)
  {
    ret = -ret;
    goto errout;
  }

  /* Get an inode for this file */

  inode_ptr = inode_find(fullpath, &relpath);
  if (!inode_ptr)
    {
      /* "O_CREAT is not set and the named file does not exist.  Or, a
       * directory component in pathname does not exist or is a dangling
       * symbolic link."
       */

      ret = ENOENT;
      free(fullpath);
      goto errout;
    }

  /* Verify that the inode is valid and either a "normal" character driver or a
   * mountpoint.  We specifically exclude block drivers and and "special"
   * inodes (semaphores, message queues, shared memory).
   */

#ifndef CONFIG_DISABLE_MOUNTPOINT
  if ((!INODE_IS_DRIVER(inode_ptr) && !INODE_IS_MOUNTPT(inode_ptr)) || !inode_ptr->u.i_ops)
#else
  if (!INODE_IS_DRIVER(inode_ptr) || !inode_ptr->u.i_ops)
#endif
    {
      ret = ENXIO;
      goto errout_with_inode;
    }

  /* Associate the inode with a file structure */

  fd = files_allocate(inode_ptr, oflags, 0, NULL,3);
  if (fd < 0)
    {
      ret = EMFILE;
      goto errout_with_inode;
    }

  /* Get the file structure corresponding to the file descriptor. */

  filep = fs_getfilep(fd);
  if (!filep)
    {
      ret = EPERM;
      /* The errno value has already been set */
      goto errout_with_fd;
    }

  /* Perform the driver open operation.  NOTE that the open method may be
   * called many times.  The driver/mountpoint logic should handled this
   * because it may also be closed that many times.
   */

  ret = OK;
  filep->f_path = fullpath; //The mem will free in close(fd);
  filep->f_relpath = relpath;

  if (inode_ptr->u.i_ops->open)
    {
#ifndef CONFIG_DISABLE_MOUNTPOINT
      if (INODE_IS_MOUNTPT(inode_ptr))
        {
          ret = inode_ptr->u.i_mops->open(filep, relpath, oflags, mode);
        }
      else
#endif
        {
          ret = inode_ptr->u.i_ops->open(filep);
        }
    }

  if (ret < 0)
    {
      ret = -ret;
      goto errout_with_fd;
    }

  return fd;

 errout_with_fd:
  files_release(fd);
 errout_with_inode:
  inode_release(inode_ptr);
  free(fullpath);
 errout:
  set_errno(ret);
  return VFS_ERROR;
}

int
open64 (const char *__path, int __oflag, ...)
{
  mode_t mode = 0666;
#ifdef CONFIG_FILE_MODE
  va_list ap;
  va_start(ap, __oflag);
  mode = va_arg(ap, mode_t);
  va_end(ap);
#endif
  return open (__path, __oflag | O_LARGEFILE, mode);
}
