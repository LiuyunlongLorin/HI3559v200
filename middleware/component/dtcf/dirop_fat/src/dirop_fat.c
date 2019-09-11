#ifndef __LITEOS__
#include "securec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/mount.h>
#include <syslog.h>
#include <signal.h>
#include <sys/prctl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <stdbool.h>



//#include "pthread.h"
#include "hi_type.h"

#include "dirop_fat.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


/*
    hi3516a/hi3518/hi3518ev200 linux内核版本，0x13命令可以使用
    hi3519 linux内核版本更新为3.18，0x13命令被占用，故后移一个
*/

#define VFAT_IOCTL_READDIR_ALL_0X13 (0x13)   /*hi3516a/hi3518/hi3518ev200*/
#define VFAT_IOCTL_READDIR_ALL_0X14 (0x14)   /*hi3519/3516cv300*/

#define OTHER_SIZE (sizeof(struct dirstream_fat)+4/*int d_count*/+4/*int d_usecount*/)

#define VFAT_IOCTL_READDIR_ALL      _IOR('r', IOCTL_CMD_READDIR_ALL, struct fat_direntall_buf)

DIR_FAT* opendir_fat(const char* name)
{
    DIR* dp = NULL;
    struct stat statbuf;
    DIR_FAT* dir_fat = NULL;

    dp = opendir (name);

    if (dp)
    {
        if (stat(name, &statbuf))
        {
            goto err;
        }

        if (statbuf.st_blksize < 512)
        {
            statbuf.st_blksize = 512;
        }

        dir_fat = (DIR_FAT*)malloc(statbuf.st_blksize + OTHER_SIZE);

        if (dir_fat)
        {
            memset_s((HI_U8*)dir_fat, statbuf.st_blksize + OTHER_SIZE, 0, (statbuf.st_blksize + OTHER_SIZE));
            //dir_fat->stDirStream.dp = dp;
            dir_fat->stDirStream.dd_nextloc = 0;
            dir_fat->stDirStream.dd_size = 0;
            //dir_fat->pstBuf = (struct fat_direntall_buf *)(((HI_U8*)dir_fat)+OTHER_SIZE);
            dir_fat->stBuf.d_count = statbuf.st_blksize;
            dir_fat->stBuf.d_usecount = 0;

            //       dir_fat->stDirStream.dd_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; /*g++ ok*/
            pthread_mutex_init(&(dir_fat->stDirStream.dd_lock), NULL);           /*gcc /g++ ok*/
        }
        else
        {
            goto err;
        }
    }

    //dp = opendir (name);
    if (dp)
    {
        dir_fat->stDirStream.dp = dp;
        return dir_fat;
    }

err:

    if (dp)
    {
        closedir (dp);
        dp = NULL;
    }

    return dir_fat;
}



int closedir_fat(DIR_FAT* dir_fat)
{
    int ret = 0;

    if (!dir_fat)
    {
        //__set_errno(EBADF);
        return -1;
    }

    pthread_mutex_destroy(&(dir_fat->stDirStream.dd_lock));
    ret = closedir (dir_fat->stDirStream.dp);

    free(dir_fat);
    dir_fat = NULL;

    return ret;
}


struct fat_direntall* readdir_fat(DIR_FAT* dir_fat)
{
    int fd = -1;
    int ret = 0;
    struct fat_direntall* de = NULL;

    if (!dir_fat)
    {
        //__set_errno(EBADF);
        return NULL;
    }

    (void)pthread_mutex_lock(&(dir_fat->stDirStream.dd_lock));

    fd = dirfd(dir_fat->stDirStream.dp);

    do
    {
        if (dir_fat->stDirStream.dd_size <= dir_fat->stDirStream.dd_nextloc)
        {
            /* read dir_fat->pstBuf->d_count bytes of directory entries. */
            ret = ioctl(fd, VFAT_IOCTL_READDIR_ALL, &(dir_fat->stBuf));
            if(0 != ret)
            {
                de = NULL;
                goto all_done;
            }

            if (dir_fat->stBuf.d_usecount <= 0)
            {
                de = NULL;
                goto all_done;
            }

            dir_fat->stDirStream.dd_size = dir_fat->stBuf.d_usecount;
            dir_fat->stDirStream.dd_nextloc = 0;
        }

        de = (struct fat_direntall*) (((char*)(&(dir_fat->stBuf.direntall))) + dir_fat->stDirStream.dd_nextloc);

        /* Am I right? H.J. */
        dir_fat->stDirStream.dd_nextloc += de->d_reclen;

        /* Skip deleted files.  */
    }
    while (de->d_ino == 0);

all_done:
    (void)pthread_mutex_unlock(&(dir_fat->stDirStream.dd_lock));
    return de;
}

#define _D_ALLOC_NAMLEN(d) (((char *) (d) + (d)->d_reclen) - &(d)->d_name[0])

/*this interface should ensure namelist is valid while return value is larger than 0 */
/*lint -e429 -e449*/
int scandir_fat(const char* dir, struct fat_direntall** *namelist,
                int (*selector) (const struct fat_direntall*),
                int (*compar) (const struct fat_direntall**, const struct fat_direntall**))
{
    DIR_FAT* dp = opendir_fat(dir);
    struct fat_direntall* current;
    struct fat_direntall** names = NULL;
    unsigned int names_size = 0, pos;
    bool bFailed = 0;
    int ret = 0;

    if (dp == NULL)
    { return -1; }

    pos = 0;

    while ((current = readdir_fat(dp)) != NULL)
    {
        int use_it = selector == NULL;

        if (! use_it)
        {
            use_it = (*selector) (current);
            /* The selector function might have changed errno.
             * It was zero before and it need to be again to make
             * the latter tests work.  */
            //if (! use_it)
            //__set_errno (0);
        }

        if (use_it)
        {
            struct fat_direntall* vnew;
            unsigned int dsize;

            if (pos == names_size)
            {
                struct fat_direntall** new_direntall;

                if (names_size == 0)
                { names_size = 10; }
                else
                { names_size *= 2; }

                new_direntall = (struct fat_direntall**) malloc (names_size * sizeof (struct fat_direntall*));

                if (new_direntall == NULL)
                {
                    bFailed = 1;
                    break;
                }
                memset_s(new_direntall, names_size * sizeof (struct fat_direntall*), 0, names_size * sizeof (struct fat_direntall*));

                if(HI_NULL != names)
                {
                    memcpy_s(new_direntall, names_size * sizeof (struct fat_direntall*), names, pos*sizeof (struct fat_direntall*));
                    free(names);
                }

                names = new_direntall;
            }

            dsize = &current->d_name[_D_ALLOC_NAMLEN(current)] - (char*)current;
            if (0 == dsize)
            {
                bFailed = 1;
                break;
            }

            vnew = (struct fat_direntall*) malloc (dsize);

            if (vnew == NULL)
            {
                bFailed = 1;
                break;
            }

            memcpy_s(vnew, dsize, current, dsize);
            names[pos++] = (struct fat_direntall*)vnew;
        }
    }

    if (1 == bFailed)
    {
        closedir_fat(dp);

        while (pos > 0 && NULL != names)
        {
            pos--;
            free (names[pos]);
            names[pos] = NULL;
        }

        free (names);
        return -1;
    }

    ret = closedir_fat (dp);
    if (ret != 0)
    {
        while (pos > 0 && NULL != names)
        {
            pos--;
            free (names[pos]);
            names[pos] = NULL;
        }

        free (names);
        return -1;
    }

    /* Sort the list if we have a comparison function to sort with.  */
    if ((NULL != compar) && (NULL != names))
    {
        /*if use g++, replace __compar_fn_t with  comparison_fn_t */
        qsort (names, pos, sizeof (struct fat_direntall*),  (__compar_fn_t)compar);
    }

    *namelist = names;
    return pos;
}
/*lint +e429 +e449*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
