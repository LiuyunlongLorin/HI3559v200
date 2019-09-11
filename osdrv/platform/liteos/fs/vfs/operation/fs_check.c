/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
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


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "sys/select.h"
#include "sys/stat.h"
#include "sys/prctl.h"
#include "fs/dirent_fs.h"
#include "inode/inode.h"

/****************************************************************************
 * Name: fscheck
 ****************************************************************************/
FAR int fscheck(FAR const char *path)
{
    FAR struct inode *inode_ptr = (struct inode *)NULL;
    FAR struct fs_dirent_s *dir;
    FAR const char *relpath;
    int ret;
    char *fullpath = (char *)NULL;
    char *fullpath_bak = (char *)NULL;

    ret = vfs_normalize_path((const char *)NULL, path, &fullpath);
    if (ret < 0)
    {
        ret = -ret;
        goto errout;
    }
    fullpath_bak = fullpath;

    inode_semtake();

    if (!fullpath || *fullpath == 0 || strcmp(fullpath, "/") == 0)
    {
        ret = EINVAL;
        goto errout_with_semaphore;
    }
    else
    {
        /* We don't know what to do with relative pathes */
        if (*fullpath != '/')
        {
            ret = ENOTDIR;
            goto errout_with_semaphore;
        }

        /* Find the node matching the path. */
        inode_ptr = inode_search((FAR const char **)&fullpath, (FAR struct inode**)NULL, (FAR struct inode**)NULL, &relpath);
    }

    if (!inode_ptr)
    {
        /* 'path' is not a directory.*/
        ret = ENOTDIR;
        goto errout_with_semaphore;
    }

    dir = (FAR struct fs_dirent_s *)zalloc(sizeof(struct fs_dirent_s));
    if (!dir)
    {
        /* Insufficient memory to complete the operation.*/
        ret = ENOMEM;
        goto errout_with_semaphore;
    }

#ifndef CONFIG_DISABLE_MOUNTPOINT
    if (INODE_IS_MOUNTPT(inode_ptr))
    {
        if (!inode_ptr->u.i_mops || !inode_ptr->u.i_mops->fscheck)
        {
            ret = ENOSYS;
            goto errout_with_direntry;
        }

        /* Perform the fscheck() operation */
        ret = inode_ptr->u.i_mops->fscheck(inode_ptr, relpath, dir);
        if (ret != OK)
        {
            ret = -ret;
            goto errout_with_direntry;
        }
    }
    else
#endif
    {
        ret = EINVAL;
        goto errout_with_direntry;
    }
    inode_semgive();
    free(dir);
    free((void*)fullpath_bak);
    return 0;

errout_with_direntry:
    free(dir);

errout_with_semaphore:
    inode_semgive();
    free((void*)fullpath_bak);
errout:
    set_errno(ret);
    return -1;
}
