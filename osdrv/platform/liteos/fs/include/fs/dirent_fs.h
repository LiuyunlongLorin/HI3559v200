/****************************************************************************
 * include/fs/dirent.h
 *
 *   Copyright (C) 2007, 2009, 2011-2013 Gregory Nutt. All rights reserved.
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

#ifndef __INCLUDE_FS_DIRENT_H
#define __INCLUDE_FS_DIRENT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"

#include "sys/types.h"
#include "stdint.h"
#include "dirent.h"

#include "fs/fs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/****************************************************************************
 * Public Types
 ****************************************************************************/

/* The internal representation of type DIR is just a container for an inode
 * reference, a position, a dirent structure, and file-system-specific
 * information.
 *
 * For the root pseudo-file system, we need retain only the 'next' inode
 * need for the next readdir() operation.  We hold a reference on this
 * inode so we know that it will persist until closedir is called.
 */

struct fs_pseudodir_s
{
    struct inode *fd_next;             /* The inode for the next call to readdir() */
};

typedef void *fs_dir_s;

#define DIRENT_MAGIC     0x11CBA828       /* Magic number to express the status of a dirent */
struct fs_dirent_s
{
    /* This is the node that was opened by opendir.  The type of the inode
     * determines the way that the readdir() operations are performed. For the
     * pseudo root pseudo-file system, it is also used to support rewind.
     *
     * We hold a reference on this inode so we know that it will persist until
     * closedir() is called (although inodes linked to this inode may change).
     */

    struct inode *fd_root;

    /* At present, only mountpoints require special handling flags */

#ifndef CONFIG_DISABLE_MOUNTPOINT
    unsigned int fd_flags;
#endif

    /* This keeps track of the current directory position for telldir */

    off_t fd_position;

    /* Retained control information depends on the type of file system that
     * provides is provides the mountpoint.  Ideally this information should
     * be hidden behind an opaque, file-system-dependent void *, but we put
     * the private definitions in line here for now to reduce allocations.
     */

    struct
    {
        /* Private data used by the built-in pseudo-file system */

        struct fs_pseudodir_s pseudo;

        /* Private data used by other file systems */
        fs_dir_s fs_dir;
    } u;

    /* In any event, this the actual struct dirent that is returned by readdir */

    struct dirent fd_dir;              /* Populated when readdir is called */
    int           fd_status;           /* Express the dirent is been opened or no */
};

/****************************************************************************
 * Global Variables
 ****************************************************************************/
extern DIR *fdopendir(int);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __INCLUDE_FS_DIRENT_H */
