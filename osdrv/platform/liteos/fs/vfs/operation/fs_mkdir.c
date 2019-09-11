/****************************************************************************
 * fs/vfs/fs_mkdir.c
 *
 *   Copyright (C) 2007, 2008, 2014 Gregory Nutt. All rights reserved.
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
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fs/fs.h"
#include "stdlib.h"
#include "inode/inode.h"
#include "string.h"

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

#undef FS_HAVE_MKDIR
#if defined(FS_HAVE_WRITABLE_MOUNTPOINT) || defined(FS_HAVE_PSEUDOFS_OPERATIONS)
#  define FS_HAVE_MKDIR 1
#endif

#ifdef FS_HAVE_MKDIR

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
 * Name: mkdir
 *
 * Description:  Create a directory
 *
 ****************************************************************************/

int mkdir(const char *pathname, mode_t mode)
{
  FAR struct inode *inode_ptr;
  const char        *relpath     = (char *)NULL;
  int               errcode;
  int               ret;
  char              *fullpath    = (char *)NULL;

  ret = vfs_normalize_path((const char *)NULL, pathname, &fullpath);

  if (ret < 0)
  {
    errcode = -ret;
    goto errout;
  }

  /* Find the inode that includes this path */

  inode_ptr = inode_find(fullpath, &relpath);
  if (inode_ptr)
    {
      /* An inode was found that includes this path and possibly refers to a
       * mountpoint.
       */

#ifndef CONFIG_DISABLE_MOUNTPOINT
      /* Check if the inode is a valid mountpoint. */

      if (!INODE_IS_MOUNTPT(inode_ptr) || !inode_ptr->u.i_mops)
        {
          /* The inode is not a mountpoint */

          errcode = EEXIST;
          goto errout_with_inode;
        }

      /* Perform the mkdir operation using the relative path
       * at the mountpoint.
       */

      if (inode_ptr->u.i_mops->mkdir)
        {
          ret = inode_ptr->u.i_mops->mkdir(inode_ptr, relpath, mode);
          if (ret < 0)
            {
              errcode = -ret;
              goto errout_with_inode;
            }
        }
      else
        {
          errcode = ENOSYS;
          goto errout_with_inode;
        }

      /* Release our reference on the inode */

      inode_release(inode_ptr);
#else
      /* But mountpoints are not supported in this configuration */

      errcode = EEXIST;
      goto errout_with_inode;
#endif
    }

#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
  /* No inode exists that contains this path.  Create a new inode in the
   * pseudo-filesystem at this location.
   */

  else
    {
      /* Create an inode in the pseudo-filesystem at this path */

      inode_semtake();
      ret = inode_reserve(fullpath, &inode_ptr);
      inode_semgive();

      if (ret < 0)
        {
          errcode = -ret;
          free(fullpath);
          goto errout;
        }
    }
#else
  else
    {
      errcode = ENXIO;
      free(fullpath);
      goto errout;
    }
#endif

  /* Directory successfully created */
  free(fullpath);
  return OK;

 errout_with_inode:
  inode_release(inode_ptr);
  free(fullpath);
 errout:
  set_errno(errcode);
  return VFS_ERROR;
}

#endif /* FS_HAVE_MKDIR */
