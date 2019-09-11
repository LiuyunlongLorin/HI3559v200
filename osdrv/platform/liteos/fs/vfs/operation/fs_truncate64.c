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

#include "vfs_config.h"
#include "sys/types.h"
#include "unistd.h"
#include "fcntl.h"
#include "sched.h"
#include "assert.h"
#include "errno.h"

#include "inode/inode.h"

/****************************************************************************
 * Name: file_truncate
 ****************************************************************************/
int file_truncate64(FAR struct file *filep, off64_t length)
{
    FAR struct inode *inode_ptr;
    int ret;
    int err;

    /* Was this file opened for write access? */
    if (O_RDONLY == (filep->f_oflags & O_ACCMODE))
    {
        err = EACCES;
        goto errout;
    }

    /* Is a driver registered? Does it support the truncate method? */

    inode_ptr = filep->f_inode;
    if (!inode_ptr || !inode_ptr->u.i_mops || !inode_ptr->u.i_mops->truncate64)
    {
        err = EBADF;
        goto errout;
    }

    /* Yes, then let the driver perform the truncate */
    ret = inode_ptr->u.i_mops->truncate64(filep, length);
    if (ret < 0)
    {
        err = -ret;
        goto errout;
    }

    return ret;

    errout:
        set_errno(err);
        return VFS_ERROR;
}

/***************************************************************************
 * Name: ftruncate
 *
 * Description:
 * The ftruncate() function truncates the file size to the length bytes.
 * If  the  file  previously was larger than this size, the extra data is lost.
 * If the file previously was shorter, it is extended, and the extended part reads as disk data.
 * fp Pointer to the open file object.
 * Returned Value:
 * On success, 0.
 * On error, -1 is returned, and errno is set appro-priately:
 *
 *
 ********************************************************************************************/

int ftruncate64(int fd, off64_t length)
{
#if CONFIG_NFILE_DESCRIPTORS > 0
    FAR struct file *filep;
#endif

  /* Did we get a valid file descriptor? */

#if CONFIG_NFILE_DESCRIPTORS > 0
    if ((unsigned int)fd >= CONFIG_NFILE_DESCRIPTORS)
#endif
    {
        set_errno(EBADF);
        return VFS_ERROR;
    }

#if CONFIG_NFILE_DESCRIPTORS > 0
/* The descriptor is in the right range to be a file descriptor... write
* to the file.
*/
    filep = fs_getfilep(fd);
    if (!filep)
    {
    /* The errno value has already been set */
        return VFS_ERROR;
    }

/* Perform the truncate operation using the file descriptor as an index */
    return file_truncate64(filep, length);
#endif
}
