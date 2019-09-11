/*-----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/
 * or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ------------------------------------------------------------------------------
 * Notice of Export Control Law
 ==============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "dirent.h"
#include "unistd.h"
#include "sys/select.h"
#include "sys/stat.h"
#include "sys/prctl.h"
#include "los_task.ph"

#include "inode/inode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MAX_DIR_ENT 1024
extern char working_directory[PATH_MAX];
#if defined(LOSCFG_FS_YAFFS) && defined(LOSCFG_FS_YAFFS_CACHE)
extern int nand_sync(void);
extern struct mtd_info *nand_mtd; /*lint !e565*/
#endif

extern struct inode * files_get_openfile(int fd);

int fstat(int fildes, struct stat *buf)
{
  struct file *filep;

  filep = fs_getfilep(fildes);
  if (!filep)
  {
    return VFS_ERROR;
  }

  return stat(filep->f_path, buf);
}

int fstat64(int fildes, struct stat64 *buf)
{
  struct file *filep;

  filep = fs_getfilep(fildes);
  if (!filep)
  {
    return VFS_ERROR;
  }

  return stat64(filep->f_path, buf);
}

int lstat(const char *path, struct stat *buffer)
{
    return stat(path, buffer);
}

int chdir(const char *path)
{
    int ret;
    char *fullpath;
    DIR *dirent_ptr;
    char *fullpath_bak = (char *)NULL;

    if (!path || !strlen(path))
    {
        set_errno(ENOENT);
        return -1;
    }

    if (strlen(path) > PATH_MAX)
    {
        set_errno(ENOTDIR);

        return -1;
    }

    ret = vfs_normalize_path((const char *)NULL, path, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        return -1; /* build path failed */
    }
    fullpath_bak = fullpath;

    dirent_ptr = opendir(fullpath);
    if (dirent_ptr == NULL)
    {
        free((void*)fullpath_bak);
        /* this is a not exist directory */
        return -1;
    }

    /* close directory stream */
    ret = closedir(dirent_ptr);
    if (ret < 0)
    {
        free((void*)fullpath_bak);
        return ret;
    }

    /* copy full path to working directory */
    LOS_TaskLock();
    strncpy(working_directory, fullpath, PATH_MAX - 1);
    LOS_TaskUnlock();

    /* release normalize directory path name */
    free((void*)fullpath_bak);

    return 0;
}

/**
 * this function is a POSIX compliant version, which will return current
 * working directory.
 *
 * @param buf the returned current directory.
 * @param size the buffer size.
 *
 * @return the returned current directory.
 */
char *getcwd(char *buf, size_t n)
{
    if (buf == NULL)
    {
        set_errno(EINVAL);
        return buf;
    }
#ifdef VFS_USING_WORKDIR
    if (n <= strlen(working_directory))
    {
        set_errno(ERANGE);
        return NULL;
    }
    LOS_TaskLock();
    strncpy(buf, working_directory, n);
    LOS_TaskUnlock();
#else
    PRINT_ERR("NO_WORKING_DIR\n");
#endif

    return buf;
}

int chmod(const char *path, mode_t mode)
{
    set_errno(ENOSUPP);
    return -1;
}

int access(const char *path, int amode)
{
    int result;
    mode_t mode;
    struct stat buf;

    result = stat(path, &buf);

    if (result != ENOERR)
    {
        return -1;
    }

    mode = buf.st_mode;
    if (amode & R_OK)
    {
        if ((mode & (S_IROTH|S_IRGRP|S_IRUSR)) == 0)
        {
            set_errno(EACCES);
            return -1;
        }
    }

    if (amode & W_OK)
    {
        if ((mode & (S_IWOTH|S_IWGRP|S_IWUSR)) == 0)
        {
            set_errno(EACCES);
            return -1;
        }
    }

    if (amode & X_OK)
    {
        if ((mode & (S_IXOTH|S_IXGRP|S_IXUSR)) == 0)
        {
            set_errno(EACCES);
            return -1;
        }
    }

    return 0;
}

bool IS_MOUNTPT(const char *dev)
{
    struct inode *node = (struct inode *)NULL;
    bool ret = 0;

    node = inode_find(dev, (const char **)NULL);
    if(node == NULL)
    {
        return 0;
    }

   ret = INODE_IS_MOUNTPT(node);
   inode_release(node);
   node = (struct inode *)NULL;
   return ret; /*lint !e438*/
}

int prctl(int  option, ...)
{
    unsigned long name;
    va_list ap;

    va_start(ap, option);

    if (PR_SET_NAME == option)
    {
        name = va_arg(ap, unsigned long);
        g_stLosTask.pstRunTask->pcTaskName = (char *) name;
    }

    va_end(ap);

    return 0; /*lint !e438*/
}

int scandir(const char* dir, struct dirent*** namelist,
                                 int(*filter)(const struct dirent*),
                                 int(*compar)(const struct dirent**,
                                              const struct dirent**))
{
    DIR * od;
    int n = 0;
    struct dirent ** list = (struct dirent **)NULL;
    struct dirent * ent ,* p;

    if ((dir == NULL) || (namelist == NULL))
        return -1;

    od = opendir(dir);
    if (od == NULL)
        return -1;

    /* Assign a maximum array */
    list = (struct dirent **)malloc(MAX_DIR_ENT*sizeof(struct dirent *));
    if (list == NULL)
    {
         (void)closedir(od);
         return -1;
    }

    while ((ent = readdir(od)) != NULL)
    {
       if (filter && !filter(ent))
          continue;

       p = (struct dirent *)malloc(sizeof(struct dirent));
       if (NULL == p)
            break;

       memcpy((void *)p,(void *)ent,sizeof(struct dirent));
       list[n] = p;

       n++;
       if(n >= MAX_DIR_ENT)
            break;

    }

    if (closedir(od) < 0)
    {
        while (n--)
            free(list[n]);
        free(list);
        return -1;
    }

    /* Change to return to the array size*/
    *namelist = (struct dirent **)realloc((void *)list,n*sizeof(struct dirent *)); /*lint !e586*/
     if (*namelist == NULL && n > 0)
        *namelist = list; /*lint !e449*/

    /* Sort array*/
    if (compar && *namelist)
       qsort((void *)*namelist,(size_t)n,sizeof(struct dirent *),(int(*)(const void *, const void *))*compar);

    return n;
}

int alphasort(const struct dirent** a, const struct dirent** b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

char *rindex( const char *s, int c )
{
    // Don't bother tracing - strrchr can do that
    return strrchr(s, c); /*lint !e605*/
}

int (*sd_sync_fn) (int) = NULL;
int (*nand_sync_fn) (void) = NULL;
void sync(void)
{
    int ret = 0;
#ifdef LOSCFG_FS_FAT_CACHE
    if(sd_sync_fn != NULL) {
        ret = sd_sync_fn(0);
        ret = sd_sync_fn(1);
    }
#endif
#if defined(LOSCFG_FS_YAFFS) && defined(LOSCFG_FS_YAFFS_CACHE)
    if (nand_mtd != NULL && nand_sync_fn != NULL)
    {
        ret = nand_sync_fn();
        if(ret)
            PRINT_ERR("Sync nand failed(%d)!\n", ret);
    }
#endif
}/*lint !e438 !e529 !e550*/

long rt_device_register(void *dev,
                            const char *name,
                            unsigned short flags)
{
    return 0;
}

void ls(const char *pathname)
{
    struct stat64 stat64_info;
    struct stat stat_info;
    struct dirent *pdirent;
    char *path = (char *)NULL;
    char *fullpath = (char *)NULL;
    char *fullpath_bak = (char *)NULL;
    int ret;
    DIR *d = (DIR *)NULL;

    if (pathname == NULL)
    {
        #ifdef VFS_USING_WORKDIR
        /* open current working directory */
        path = strdup(working_directory);
        #else
        path = strdup("/");
        #endif
        if (path == NULL)
            return ;
    }
    else
    {
        ret = vfs_normalize_path(NULL, pathname, &path);
        if (ret < 0)
        {
            set_errno(-ret);
            return;
        }
    }

    /* list all directory and file*/
    d = opendir(path);
    if (d == NULL)
    {
        PRINT_ERR("No such directory\n");
    }
    else
    {
        PRINTK("Directory %s:\n", path);
        do
        {
            pdirent = readdir(d);
            if (pdirent)
            {
                memset(&stat_info, 0, sizeof(struct stat));
                if (path[1] != '\0')
                {
                    fullpath = (char *)malloc(strlen(path) + strlen(pdirent->d_name) + 2);
                    if (!fullpath)
                        goto EXIT_WITH_NOMEM;

                    (void)snprintf(fullpath, strlen(path) + strlen(pdirent->d_name) + 2,
                    "%s/%s", path, pdirent->d_name);
                }
                else
                {
                    fullpath = (char *)malloc(strlen(pdirent->d_name) + 2);
                    if (!fullpath)
                        goto EXIT_WITH_NOMEM;

                    (void)snprintf(fullpath, strlen(pdirent->d_name) + 2,
                    "/%s", pdirent->d_name);
                }

                fullpath_bak = fullpath;

                if (stat64(fullpath, &stat64_info) == 0)
                {
                    PRINTK("%-20s", pdirent->d_name);
                    if (S_ISDIR(stat64_info.st_mode))
                    {
                        PRINTK(" %-25s\n", "<DIR>");
                    }
                    else
                    {
                        PRINTK(" %-25lld\n", stat64_info.st_size);
                    }
                }
                else if (stat(fullpath, &stat_info) == 0)
                {
                    PRINTK("%-20s", pdirent->d_name);
                    if (S_ISDIR(stat_info.st_mode))
                    {
                        PRINTK(" %-25s\n", "<DIR>");
                    }
                    else
                    {
                        PRINTK(" %-25lu\n", stat_info.st_size);
                    }
                }
                else
                    PRINTK("BAD file: %s\n", pdirent->d_name);
                free(fullpath_bak);
            }
        }while(pdirent);

    (void)closedir(d);
    }
    free(path);

    return ;
EXIT_WITH_NOMEM:
    free(path);
    set_errno(ENOSPC);
    (void)closedir(d);
    return;
}


char *realpath(const char *path, char *resolved_path)
{
    int ret,result;
    char *new_path;
    struct stat buf;

    ret = vfs_normalize_path(NULL, path, &new_path);
    if (ret < 0)
    {
        ret = -ret;
        set_errno(ret);
        return NULL;
    }

    result = stat(new_path, &buf);

    if(resolved_path == NULL)
    {
        if (result != ENOERR)
        {
            free(new_path);
            return NULL;
        }
        return new_path;
    }

    strcpy(resolved_path,new_path);/*lint !e586*/
    free(new_path);
    if (result != ENOERR)
    {
        return NULL;
    }
    return resolved_path;
}

void lsfd(void)
{
    FAR struct filelist *f_list;
    unsigned int i = 3;
    int ret;
    FAR struct inode *node;

    f_list = &tg_filelist;

    PRINTK("   fd    filename\n");
    ret = sem_wait(&f_list->fl_sem);
    if (ret < 0)
    {
        PRINTK("sem_wait error, ret=%d\n", ret);
        return;
    }

    while (i < CONFIG_NFILE_DESCRIPTORS)
    {
        node = files_get_openfile(i);
        if(node)
        {
            PRINTK("%5d   %s\n", i, f_list->fl_files[i].f_path);
        }
        i++;
    }
    (void)sem_post(&f_list->fl_sem);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

