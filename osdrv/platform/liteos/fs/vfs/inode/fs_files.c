/****************************************************************************
 * fs/inode/fs_files.c
 *
 *   Copyright (C) 2007-2009, 2011-2013 Gregory Nutt. All rights reserved.
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
#include "string.h"
#include "semaphore.h"
#include "assert.h"
#include "errno.h"

#include "fs/fs.h"
#include "stdio.h"
#include "stdlib.h"
#include "inode/inode.h"


#if CONFIG_NFILE_DESCRIPTORS > 0
struct filelist tg_filelist;
#endif

#if CONFIG_NFILE_STREAMS > 0
struct streamlist tg_streamlist;
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

struct filelist *sched_getfiles(void)
{
#if CONFIG_NFILE_DESCRIPTORS > 0
    return &tg_filelist;
#else
    return NULL;
#endif
}

static unsigned int bitmap[CONFIG_NFILE_DESCRIPTORS/32 + 1] = {0};

static void set_bit(int i, void *addr)
{
    unsigned int tem = (unsigned int)i >> 5;
    unsigned int *addri = (unsigned int *)addr + tem;
    unsigned int old = *addri;
    old = old | (1UL << (i & 0x1f));
    *addri = old;
}

static void clear_bit(int i, void *addr)
{
    unsigned int tem = (unsigned int)i >> 5;
    unsigned int *addri = (unsigned int *)addr + tem;
    unsigned int old = *addri;
    old = old & ~(1UL << (i & 0x1f));
    *addri = old;
}


/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Variables
 ****************************************************************************/

/****************************************************************************
 * Private Variables
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: _files_semtake
 ****************************************************************************/

static void _files_semtake(FAR struct filelist *list)
{
    /* Take the semaphore (perhaps waiting) */

    while (sem_wait(&list->fl_sem) != 0)
    {
        /* The only case that an error should occr here is if
         * the wait was awakened by a signal.
         */

        LOS_ASSERT(get_errno() == EINTR);
    }
}

/****************************************************************************
 * Name: _files_semgive
 ****************************************************************************/

#define _files_semgive(list)  (void)sem_post(&list->fl_sem)

/****************************************************************************
 * Name: _files_close
 *
 * Description:
 *   Close an inode (if open)
 *
 * Assumuptions:
 *   Caller holds the list semaphore because the file descriptor will be freed.
 *
 ****************************************************************************/

static int _files_close(FAR struct file *filep)
{
    struct inode *inode_ptr = filep->f_inode;
    int ret = OK;

    /* Check if the struct file is open (i.e., assigned an inode) */

    if (inode_ptr)
    {
        /* Close the file, driver, or mountpoint. */

        if (inode_ptr->u.i_ops && inode_ptr->u.i_ops->close)
        {
            /* Perform the close operation */

            ret = inode_ptr->u.i_ops->close(filep);
            if(ret != OK)
            {
                return ret;
            }
        }


        /* And release the inode */

        inode_release(inode_ptr);
        /* Release the path of file */
        free(filep->f_path);

        /* Release the file descriptor */
        filep->f_oflags  = 0;
        filep->f_pos     = 0;
        filep->f_path    = (char *)NULL;
        filep->f_priv    = NULL;
        filep->f_inode   = (struct inode *)NULL;
    }

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: files_initialize
 *
 * Description:
 *   This is called from the FS initialization logic to configure the files.
 *
 ****************************************************************************/

void files_initialize(void)
{
}

/****************************************************************************
 * Name: files_initlist
 *
 * Description: Initializes the list of files for a new task
 *
 ****************************************************************************/

void files_initlist(FAR struct filelist *list)
{
    DEBUGASSERT(list);

    /* Initialize the list access mutex */

    (void)sem_init(&list->fl_sem, 0, 1);
}

/****************************************************************************
 * Name: files_releaselist
 *
 * Description:
 *   Release a reference to the file list
 *
 ****************************************************************************/

void files_releaselist(FAR struct filelist *list)
{
    int i;

    DEBUGASSERT(list);

    /* Close each file descriptor .. Normally, you would need take the list
    * semaphore, but it is safe to ignore the semaphore in this context because
    * there should not be any references in this context.
    */

    for (i = 0; i < CONFIG_NFILE_DESCRIPTORS; i++)
    {
        (void)_files_close(&list->fl_files[i]);
    }

    /* Destroy the semaphore */

    (void)sem_destroy(&list->fl_sem);
}

/****************************************************************************
 * Name: file_dup2
 *
 * Description:
 *   Assign an inode to a specific files structure.  This is the heart of
 *   dup2.
 *
 ****************************************************************************/

int file_dup2(FAR struct file *filep1, FAR struct file *filep2)
{
    FAR struct filelist *list;
    FAR struct inode *inode_ptr;
    char *fullpath = (char *)NULL;
    const char *relpath = (const char *)NULL;
    int err;
    int ret;

    if (!filep1 || !filep1->f_inode || !filep2)
    {
        err = EBADF;
        goto errout;
    }

    list = sched_getfiles();
    DEBUGASSERT(list);

    _files_semtake(list);

  /* If there is already an inode contained in the new file structure,
   * close the file and release the inode.
   */

    ret = _files_close(filep2);
    if (ret < 0)
    {
        /* An error occurred while closing the driver */
        goto errout_with_ret;
    }

    fullpath = (char *)malloc(strlen(filep1->f_path)+1);
    if (NULL == fullpath)
    {
        ret = -ENOMEM;
        goto errout_with_ret;
    }

    /* Increment the reference count on the contained inode */

    inode_ptr = filep1->f_inode;

    /* Then clone the file structure */

    filep2->f_oflags = filep1->f_oflags;
    filep2->f_pos    = filep1->f_pos;
    filep2->f_inode  = inode_ptr;
    filep2->f_priv   = filep1->f_priv;

    strncpy(fullpath, filep1->f_path,strlen(filep1->f_path)+1);
    if (!inode_find(fullpath, &relpath))
    {
        ret = ENOENT;
        free(fullpath);
        goto errout_with_inode;
    }
    filep2->f_path = fullpath;
    filep2->f_relpath = relpath;

  /* Call the open method on the file, driver, mountpoint so that it
   * can maintain the correct open counts.
   */

    if (inode_ptr->u.i_ops )
    {
#ifndef CONFIG_DISABLE_MOUNTPOINT
        if (INODE_IS_MOUNTPT(inode_ptr))
        {
            /* Dup the open file on the in the new file structure */
            if(inode_ptr->u.i_mops->dup)
                ret = inode_ptr->u.i_mops->dup(filep1, filep2);
            else
                ret = -ENOSYS;
        }
        else
#endif
        {
            /* (Re-)open the pseudo file or device driver */
            if (inode_ptr->u.i_ops->open)
                ret = inode_ptr->u.i_ops->open(filep2);
            else
                ret = -ENOSYS;
        }

        /* Handle open failures */
        if (ret < 0)
        {
            goto errout_with_inode;
        }
    }

    _files_semgive(list);
    return OK;

/* Handler various error conditions */

errout_with_inode:
    inode_release(filep2->f_inode);
    free(filep2->f_path);
    filep2->f_oflags  = 0;
    filep2->f_pos     = 0;
    filep2->f_inode   = (struct inode *)NULL;
    filep2->f_priv    = NULL;
    filep2->f_path    = (char *)NULL;
    filep2->f_relpath = (const char *)NULL;

errout_with_ret:
    err               = -ret;
    _files_semgive(list);

errout:
    set_errno(err);
    return VFS_ERROR;
}

/****************************************************************************
 * Name: files_allocate
 *
 * Description:
 *   Allocate a struct files instance and associate it with an inode instance.
 *   Returns the file descriptor == index into the files array.
 *
 ****************************************************************************/

int files_allocate(FAR struct inode *inode_ptr, int oflags, off_t pos,void* priv, int minfd)
{
    FAR struct filelist *list;
    unsigned int *p;
    unsigned int mask;
    unsigned int i;

    /*minfd should be a positive number,and 0,1,2 had be distributed to stdin,stdout,stderr*/
    if (minfd < 3)
    {
        minfd = 3;
    }
    i = (unsigned int)minfd;

    list = sched_getfiles();
    DEBUGASSERT(list);

    _files_semtake(list);

    while (i < CONFIG_NFILE_DESCRIPTORS)
    {
        p = ((unsigned int *)bitmap) + (i >> 5);
        mask = 1 << (i & 0x1f);
        if ((~(*p) & mask))
        {
            set_bit(i, bitmap);
            list->fl_files[i].f_oflags = oflags;
            list->fl_files[i].f_pos    = pos;
            list->fl_files[i].f_inode  = inode_ptr;
            list->fl_files[i].f_priv   = priv;
            _files_semgive(list);
            return (int)i;
        }
        i++;
    }


    _files_semgive(list);
    return VFS_ERROR;
}

/****************************************************************************
 * Name: files_close
 *
 * Description:
 *   Close an inode (if open)
 *
 * Assumuptions:
 *   Caller holds the list semaphore because the file descriptor will be freed.
 *
 ****************************************************************************/

int files_close(int fd)
{
    FAR struct filelist *list;
    int                  ret;

    /* Get the thread-specific file list */

    list = sched_getfiles();
    DEBUGASSERT(list);

    /* If the file was properly opened, there should be an inode assigned */

    if (fd < 0 || fd >= CONFIG_NFILE_DESCRIPTORS || !list->fl_files[fd].f_inode)
    {
        return -EBADF;
    }
    /* Perform the protected close operation */

    _files_semtake(list);
    ret = _files_close(&list->fl_files[fd]);
    if(ret == OK)
        clear_bit(fd, bitmap);
    _files_semgive(list);
    return ret;
}

/****************************************************************************
 * Name: files_release
 *
 * Assumuptions:
 *   Similar to files_close().  Called only from open() logic on error
 *   conditions.
 *
 ****************************************************************************/

void files_release(int fd)
{
    FAR struct filelist *list;

    list = sched_getfiles();
    DEBUGASSERT(list);

    if (fd >=0 && fd < CONFIG_NFILE_DESCRIPTORS)
    {
        _files_semtake(list);
        list->fl_files[fd].f_oflags  = 0;
        list->fl_files[fd].f_pos     = 0;
        list->fl_files[fd].f_inode   = (struct inode *)NULL;
        clear_bit(fd, bitmap);
        _files_semgive(list);
    }
}

struct inode * files_get_openfile(int fd)
{
    FAR struct filelist *list;
    unsigned int *p;
    unsigned int mask;

    list = sched_getfiles();
    DEBUGASSERT(list);

    p = ((unsigned int *)bitmap) + ((unsigned int)fd >> 5);
    mask = 1 << (fd & 0x1f);
    if ((~(*p) & mask))
    {
        return NULL;
    }

    return list->fl_files[fd].f_inode;
}

void clear_fd(int fd)
{
    clear_bit(fd, bitmap);
}

#define FILE_START_FD 3

int close_files(struct inode *inode)
{
    int fd = 0, ret = 0;
    FAR struct inode *open_file_inode;

    for (fd = FILE_START_FD; fd < CONFIG_NFILE_DESCRIPTORS; fd++)
    {
        open_file_inode = files_get_openfile(fd);
        if (open_file_inode && (open_file_inode == inode))
        {
            ret = files_close(fd);
            if (ret < 0)
            {
                return -EBUSY;
            }
        }
    }

    return 0;
}


