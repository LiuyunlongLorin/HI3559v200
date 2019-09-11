/****************************************************************************
 * fs/vfs/fs_statfs.c
 *
 *   Copyright (C) 2007-2009, 2012 Gregory Nutt. All rights reserved.
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

#include "sys/statfs.h"
#include "string.h"
#include "sched.h"

#include "inode/inode.h"
#include "errno.h"
#include "stdlib.h"
/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: statpseudo
 ****************************************************************************/

static inline int statpseudofs(FAR struct inode *inode_ptr, FAR struct statfs *buf)
{
  memset(buf, 0, sizeof(struct statfs));
  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stat
 *
 * Return: Zero on success; -1 on failure with errno set:
 *
 *   EACCES  Search permission is denied for one of the directories in the
 *           path prefix of path.
 *   EFAULT  Bad address.
 *   ENOENT  A component of the path path does not exist, or the path is an
 *           empty string.
 *   ENOMEM  Out of memory
 *   ENOTDIR A component of the path is not a directory.
 *   ENOSYS  The file system does not support this call.
 *
 ****************************************************************************/

int statfs(FAR const char *path, FAR struct statfs *buf)
{
    FAR struct inode *inode_ptr;
    FAR const char   *relpath = (const char *)NULL;
    int               ret     = OK;
    char *fullpath = (char *)NULL;

  /* Sanity checks */

  if (!path || !buf)
    {
      ret = EFAULT;
      goto errout;
    }

  if (!path[0])
    {
      ret = ENOENT;
      goto errout;
    }

  ret = vfs_normalize_path((const char *)NULL, path, &fullpath);

  if(ret < 0)
  {
    ret = -ret;
    goto errout;
  }

  /* Get an inode for this file */

  inode_ptr = inode_find(fullpath, &relpath);
  if (!inode_ptr)
    {
      /* This name does not refer to a psudeo-inode and there is no
       * mountpoint that includes in this path.
       */

      ret = ENOENT;
      free(fullpath);
      goto errout;
    }

  /* The way we handle the statfs depends on the type of inode that we
   * are dealing with.
   */

#ifndef CONFIG_DISABLE_MOUNTPOINT
  if (INODE_IS_MOUNTPT(inode_ptr))
    {
      /* The node is a file system mointpoint. Verify that the mountpoint
       * supports the statfs() method
       */

      if (inode_ptr->u.i_mops && inode_ptr->u.i_mops->statfs)
        {
          /* Perform the statfs() operation */

          ret = inode_ptr->u.i_mops->statfs(inode_ptr, buf);
        }
    }
  else
#endif
    {
      /* The node is part of the root pseudo file system */

      ret = statpseudofs(inode_ptr, buf);
    }

  /* Check if the statfs operation was successful */

  if (ret < 0)
    {
      ret = -ret;
      goto errout_with_inode;
    }

  /* Successfully statfs'ed the file */

  inode_release(inode_ptr);
  free(fullpath);
  return OK;

/* Failure conditions always set the errno appropriately */

errout_with_inode:
  inode_release(inode_ptr);
  free(fullpath);
errout:
  set_errno(ret);
  return VFS_ERROR;
}
