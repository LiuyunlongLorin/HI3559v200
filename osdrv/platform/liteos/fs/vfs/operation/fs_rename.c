/****************************************************************************
 * fs/vfs/fs_rename.c
 *
 *   Copyright (C) 2007-2009, 2014 Gregory Nutt. All rights reserved.
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

#include "stdio.h"
#include "errno.h"
#include "fs/fs.h"
#include "stdlib.h"
#include "inode/inode.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#undef FS_HAVE_WRITABLE_MOUNTPOINT
#if !defined(CONFIG_DISABLE_MOUNTPOINT) && defined(CONFIG_FS_WRITABLE) && \
    CONFIG_NFILE_STREAMS > 0
#  define FS_HAVE_WRITABLE_MOUNTPOINT 1
#endif

#undef FS_HAVE_PSEUDOFS_OPERATIONS
#if !defined(CONFIG_DISABLE_PSEUDOFS_OPERATIONS) && CONFIG_NFILE_STREAMS > 0
#  define FS_HAVE_PSEUDOFS_OPERATIONS 1
#endif

#undef FS_HAVE_RENAME
#if defined(FS_HAVE_WRITABLE_MOUNTPOINT) || defined(FS_HAVE_PSEUDOFS_OPERATIONS)
#  define FS_HAVE_RENAME 1
#endif

#ifdef FS_HAVE_RENAME

/****************************************************************************
 * Private Variables
 ****************************************************************************/

/****************************************************************************
 * Public Variables
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rename
 *
 * Description:  Remove a file managed a mountpoint
 *
 ****************************************************************************/

int rename(FAR const char *oldpath, FAR const char *newpath)
{
  FAR struct inode *oldinode;
  FAR struct inode *newinode;
  const char       *oldrelpath      = (const char *)NULL;
  char             *fulloldpath     = (char *)NULL;
  char             *fulloldpath_bak = (char *)NULL;
  char             *fullnewpath     = (char *)NULL;
  char             *fullnewpath_bak = (char *)NULL;
#ifndef CONFIG_DISABLE_MOUNTPOINT
  const char       *newrelpath      = (const char *)NULL;
#endif
  int               errcode         = ENOERR;
  int               ret;

  /* Ignore paths that are interpreted as the root directory which has no name
   * and cannot be moved
   */

  if (!oldpath || *oldpath == '\0' ||
      !newpath || *newpath == '\0' )
    {
      errcode = EINVAL;
      goto errout;
    }

  ret = vfs_normalize_path((const char *)NULL, oldpath, &fulloldpath);
  if (ret < 0)
   {
     errcode = -ret;
     goto errout;
   }
  fulloldpath_bak = fulloldpath;

  ret = vfs_normalize_path((const char *)NULL, newpath, &fullnewpath);
  if (ret < 0)
   {
     errcode = -ret;
     goto errout_with_path;
   }
  fullnewpath_bak = fullnewpath;

  /* Get an inode that includes the oldpath */

  oldinode = inode_find(fulloldpath, &oldrelpath);
  if (!oldinode)
    {
      /* There is no inode that includes in this path */

      errcode = ENOENT;
      free((void*)fullnewpath_bak);
      goto errout_with_path;
    }

#ifndef CONFIG_DISABLE_MOUNTPOINT
  /* Verify that the old inode is a valid mountpoint. */

  if (INODE_IS_MOUNTPT(oldinode) && oldinode->u.i_mops)
    {
      /* Get an inode for the new relpath -- it should like on the same
       * mountpoint
       */

      newinode = inode_find(fullnewpath, &newrelpath);
      if (!newinode)
        {
          /* There is no mountpoint that includes in this path */

          errcode = ENOENT;
          goto errout_with_oldinode;
        }

      /* Verify that the two paths lie on the same mountpoint inode */

      if (oldinode != newinode)
        {
          errcode = EXDEV;
          goto errout_with_newinode;
        }

      /* Perform the rename operation using the relative paths
       * at the common mountpoint.
       */

      if (oldinode->u.i_mops->rename)
        {
          ret = oldinode->u.i_mops->rename(oldinode, oldrelpath, newrelpath);
          if (ret < 0)
            {
              errcode = -ret;
              goto errout_with_newinode;
            }
        }
      else
        {
          errcode = ENOSYS;
          goto errout_with_newinode;
        }

      /* Successfully renamed */

      inode_release(newinode);
    }
  else
#endif
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
    {
      /* Create a new, empty inode at the destination location */

      inode_semtake();
      ret = inode_reserve(fullnewpath, &newinode);
      if (ret < 0)
        {
          /* It is an error if a node at newpath already exists in the tree
           * OR if we fail to allocate memory for the new inode (and possibly
           * any new intermediate path segments).
           */

          inode_semgive();
          errcode = EEXIST;
          goto errout_with_oldinode;
        }

      /* Copy the inode state from the old inode to the newly allocated inode */

      newinode->i_child   = oldinode->i_child;   /* Link to lower level inode */
      newinode->i_flags   = oldinode->i_flags;   /* Flags for inode */
      newinode->u.i_ops   = oldinode->u.i_ops;   /* Inode operations */
#ifdef CONFIG_FILE_MODE
      newinode->i_mode    = oldinode->i_mode;    /* Access mode flags */
#endif
      newinode->i_private = oldinode->i_private; /* Per inode driver private data */

      /* We now have two copies of the inode.  One with a reference count of
       * zero (the new one), and one that may have multiple references
       * including one by this logic (the old one)
       *
       * Remove the old inode.  Because we hold a reference count on the
       * inode, it will not be deleted now.  It will be deleted when all of
       * the references to to the inode have been released (perhaps when
       * inode_release() is called below).  inode_remove() should return
       * -EBUSY to indicate that the inode was not deleted now.
       */

      ret = inode_remove(fulloldpath);
      if (ret < 0 && ret != -EBUSY)
        {
          /* Remove the new node we just recreated */

          (void)inode_remove(fullnewpath);
          inode_semgive();

          errcode = -ret;
          goto errout_with_oldinode;
        }

      /* Remove all of the children from the unlinked inode */

      oldinode->i_child = (struct inode *)NULL;
      inode_semgive();
    }
#else
    {
      errcode = ENXIO;
      goto errout_with_oldinode;
    }
#endif

  /* Successfully renamed */

  inode_release(oldinode);
  free((void*)fulloldpath_bak);
  free((void*)fullnewpath_bak);

  return OK;

#ifndef CONFIG_DISABLE_MOUNTPOINT
 errout_with_newinode:
  inode_release(newinode);
#endif
 errout_with_oldinode:
  inode_release(oldinode);
  free((void*)fullnewpath_bak);
 errout_with_path:
  free((void*)fulloldpath_bak);
 errout:
  set_errno(errcode);
  return VFS_ERROR;
}

#endif /* FS_HAVE_RENAME */
