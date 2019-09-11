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
#ifdef LOSCFG_FS_FAT_VIRTUAL_PARTITION
int virstatfs(FAR const char *path, FAR struct statfs *buf)
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

        if (inode_ptr->u.i_mops && inode_ptr->u.i_mops->virstatfs)
        {
            /* Perform the statfs() operation */
            ret = inode_ptr->u.i_mops->virstatfs(inode_ptr, relpath, buf);
        }
        else
        {
            ret = EINVAL;
            goto errout_with_inode;
        }
    }
    else
#endif
    {
        ret = EINVAL;
        goto errout_with_inode;
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
#endif
