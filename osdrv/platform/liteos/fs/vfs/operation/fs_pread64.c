/****************************************************************************
 * fs/vfs/fs_pread64.c
 *
 *   Copyright (C) 2014 Gregory Nutt. All rights reserved.
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

#include "sys/types.h"
#include "unistd.h"
#include "errno.h"

#include "fs/fs.h"
#include "fs/file.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_seek64
 *
 * Description:
 *   Equivalent to the standard pread function except that is accepts a
 *   struct file instance instead of a file descriptor.  Currently used
 *   only by aio_read();
 *
 ****************************************************************************/

ssize_t file_pread64(FAR struct file *filep, FAR void *buf, size_t nbytes,
                   off64_t offset)
{
  off64_t savefilepos;
  off64_t pos;
  ssize_t ret;
  int errcode;

  /* Perform the seek to the current position.  This will not move the
   * file pointer, but will return its current setting
   */

  savefilepos = file_seek64(filep, 0, SEEK_CUR);
  if (savefilepos == (off64_t)-1)
    {
      /* file_seek64 might fail if this if the media is not seekable */

      return VFS_ERROR;
    }

  /* Then seek to the correct position in the file */

  pos = file_seek64(filep, offset, SEEK_SET);
  if (pos == (off64_t)-1)
    {
      /* This might fail is the offset is beyond the end of file */

      return VFS_ERROR;
    }

  /* Then perform the read operation */

  ret = file_read(filep, buf, nbytes);
  errcode = get_errno();

  /* Restore the file position */

  pos = file_seek64(filep, savefilepos, SEEK_SET);
  if (pos == (off64_t)-1 && ret >= 0)
    {
      /* This really should not fail */

      return VFS_ERROR;
    }

  set_errno(errcode);
  return ret;
}

/****************************************************************************
 * Name: pread64
 *
 * Description:
 *   The pread64() function performs the same action as read(), except that it
 *   reads from a given position in the file without changing the file
 *   pointer. The first three arguments to pread64() are the same as read()
 *   with the addition of a fourth argument offset for the desired position
 *   inside the file. An attempt to perform a pread64() on a file that is
 *   incapable of seeking results in an error.
 *
 *   NOTE: This function could have been wholly implemented within libc but
 *   it is not.  Why?  Because if pread64 were implemented in libc, it would
 *   require four system calls.  If it is implemented within the kernel,
 *   only three.
 *
 * Parameters:
 *   file     File structure instance
 *   buf      User-provided to save the data
 *   nbytes   The maximum size of the user-provided buffer
 *   offset   The file offset
 *
 * Return:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or -1 on failure with errno set appropriately.
 *   See read() return values
 *
 ****************************************************************************/

ssize_t pread64(int fd, void* buf, size_t nbytes, off64_t offset)
{
  FAR struct file *filep;

  /* Get the file structure corresponding to the file descriptor. */

  filep = fs_getfilep(fd);
  if (!filep)
    {
      /* The errno value has already been set */

      return (ssize_t)VFS_ERROR;
    }

  /* Let file_pread do the real work */

  return file_pread64(filep, buf, nbytes, offset);
}
