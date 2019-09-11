/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
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

/*lint -esym(459,shell_working_directory,errno_array,g_nfs_args)*/

#include "los_config.h"
#include "errno.h"
#include "sys/mount.h"
#ifdef LOSCFG_SHELL

#include "los_typedef.h"
#include "shell.h"
#include "fs/fs.h"
#include "sys/stat.h"
#include "inode/inode.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/statfs.h"
#include "stdio.h"
#include "pthread.h"

#include "shcmd.h"
#include "securec.h"
#include "show.h"

typedef enum
{
    RM_RECURSIVER,
    RM_FILE,
    RM_DIR
} RmType;

int osShellCmdDoChdir(const char *path)
{
    char *fullpath = (char *)NULL;
    DIR *pdirent;
    char *fullpath_bak = (char *)NULL;
    int ret;
    char * shell_working_directory = osShellGetWorkingDirtectory();


    if (path == NULL)
    {
        LOS_TaskLock();
        PRINTK("%s\n", shell_working_directory);
        LOS_TaskUnlock();

        return 0;
    }

    if (strlen(path) > PATH_MAX)
    {
        set_errno(ENOTDIR);
        perror("cd error");
        return -1;
    }

    ret = vfs_normalize_path(shell_working_directory, path, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        perror("cd error");
        return (int)-1; /* build path failed */
    }
    fullpath_bak = fullpath;

    pdirent = opendir(fullpath);
    if (pdirent == NULL)
    {
        free((void *)fullpath_bak);
        /* this is a not exist directory */
        PRINTK("no such file or directory\n");
        return -1;
    }

    /* close directory stream */
    (void)closedir(pdirent);
    /* copy full path to working directory */
    LOS_TaskLock();
    (VOID)strncpy_s(shell_working_directory, PATH_MAX, fullpath, PATH_MAX - 1);
    LOS_TaskUnlock();

    /* release normalize directory path name */
    free((void *)fullpath_bak);

    return 0;
}

int osShellCmdLs(int argc, char **argv)
{
    extern void ls(const char *pathname);
    char    *fullpath = (char *)NULL;
    char    *filename = (char *)NULL;
    int     ret;
    char * shell_working_directory = osShellGetWorkingDirtectory();


    if (argc == 0)
    {
        ls(shell_working_directory);
    }
    else
    {
        filename = argv[0];
        ret = vfs_normalize_path(shell_working_directory,filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("ls error");
            return (int)-1;
        }
        ls(fullpath);
        free(fullpath);
    }

    return 0;
}

int osShellCmdCd(int argc, char **argv)
{
    extern int chdir(const char * path);
    if (argc == 0)
    {
        (void)osShellCmdDoChdir("/");
    }
    else
    {
        (void)osShellCmdDoChdir(argv[0]);
    }

    return 0;
}

#define CAT_BUF_SIZE  512
pthread_mutex_t mutex_cat = PTHREAD_MUTEX_INITIALIZER;

int osShellCmdDoCatShow(AARCHPTR arg)
{
    char    buf[CAT_BUF_SIZE];
    size_t  uwSize;
    FILE    *ini;
    char    *fullpath = (char *)arg;

    (void)pthread_mutex_lock(&mutex_cat);

    if ((ini = fopen(fullpath, "r"))==NULL)
    {
        perror("cat error");
        (void)pthread_mutex_unlock(&mutex_cat);
        free(fullpath);
        return -1;
    }
    do
    {
        (void)memset_s(buf,sizeof(buf),0,CAT_BUF_SIZE);
        uwSize = fread(buf,1,CAT_BUF_SIZE,ini);
        if ((INT32)uwSize < 0)
        {
            perror("cat error");
            (void)pthread_mutex_unlock(&mutex_cat);
            free(fullpath);
            fclose(ini);
            return -1;
        }
        write(1,buf,uwSize);
        (void)LOS_TaskDelay(1);
    }
    while (uwSize == CAT_BUF_SIZE);

    free(fullpath);
    (VOID)fclose(ini);
    (void)pthread_mutex_unlock(&mutex_cat);
    return 0;
}

int osShellCmdCat(int argc, char **argv)
{
    char    *fullpath = (char *)NULL;
    char    *filename = (char *)NULL;
    char    *fullpath_bak = (char *)NULL;
    FAR const char *relpath;
    int     ret;
    UINT32  caTask;
    FAR struct inode *inode_ptr = (struct inode *)NULL;
     /*lint -e505*/
    TSK_INIT_PARAM_S stInitParam;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc != 1)
    {
        PRINTK("cat [FILE]\n");
        return -1;
    }

    filename = argv[0];
    ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        perror("cat error");
        return -1;
    }
    inode_semtake();
    fullpath_bak = fullpath;
    inode_ptr = inode_search((FAR const char **)&fullpath, (FAR struct inode**)NULL, (FAR struct inode**)NULL, &relpath);
    if(!inode_ptr)
    {
        set_errno(ENOENT);
        perror("cat error");
        inode_semgive();
        free(fullpath_bak);
        return -1;
    }
    if(INODE_IS_BLOCK(inode_ptr)||INODE_IS_DRIVER(inode_ptr))
    {
        set_errno(EPERM);
        perror("cat error");
        inode_semgive();
        free(fullpath_bak);
        return -1;
    }
    inode_semgive();
    (void)memset_s(&stInitParam, sizeof(stInitParam), 0, sizeof(TSK_INIT_PARAM_S));
    stInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)osShellCmdDoCatShow;
    stInitParam.usTaskPrio   = 10;
    stInitParam.auwArgs[0]   = (AARCHPTR)fullpath_bak;
    stInitParam.uwStackSize  = 0x3000;
    stInitParam.pcName       = "shellcmd_cat";
    stInitParam.uwResved     = LOS_TASK_STATUS_DETACHED;

    ret = (int)LOS_TaskCreate(&caTask, &stInitParam);

    return ret;
}

/*lint -e402*/
static int nfs_mount_ref(const char *server_ip_and_path, const char *mount_path,
                    unsigned int uid, unsigned int gid) __attribute__((weakref("nfs_mount")));

int osShellCmdMount(int argc, char **argv)
{
    int             ret;
    char            *fullpath = (char *)NULL;
    char            *filename;
    unsigned int    gid, uid;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc < 3)
    {
        PRINTK("mount [DEVICE] [PATH] [NAME]\n");
        return OS_FAIL;
    }

    if (0 == strncmp(argv[0], "-t", 2) || 0 == strncmp(argv[0], "-o", 2))
    {
        filename = argv[2];
        ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("mount error");
            return -1;
        }
        if (strncmp(argv[3], "nfs", 3) == 0)
        {
            if(argc <= 6)
            {
                if (argv[4] != NULL)
                {
                    uid = (unsigned int)strtoul(argv[4], (char **)NULL, 0);
                }
                else
                {
                    uid = 0;
                }

                if(argv[5] != NULL)
                {
                    gid = (unsigned int)strtoul(argv[5], (char **)NULL, 0);
                }
                else
                {
                    gid = 0;
                }

                if (nfs_mount_ref) /*lint !e506*/
                {
                    ret = nfs_mount_ref(argv[1], fullpath, uid, gid);
                    if(ret != LOS_OK)
                    {
                        PRINTK("mount -t [DEVICE] [PATH] [NAME]\n[DEVICE] format error, should be IP:PATH\n");
                    }
                }else{
                    PRINTK("can't find nfs_mount\n");
                }
                free(fullpath);
                return 0;
            }
        }

        if (strcmp(argv[1], "0") == 0)
        {
            ret = mount((const char *)NULL, fullpath, argv[3], 0, NULL);
        }
        else
        {
            ret = mount(argv[1], fullpath, argv[3], 0, NULL);
        }
        if (ret != LOS_OK)
        {
            perror("mount error");
        }
        else
        {
            PRINTK("mount ok\n");
        }
    }
    else
    {
        filename = argv[1];
        ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("mount error");
            return -1;
        }

        if (strncmp(argv[2], "nfs", 3) == 0)
        {
            if(argc <= 5)
            {
                if(argv[3] != NULL)
                {
                    uid = (unsigned int)strtoul(argv[3], (char **)NULL, 0);
                }
                else
                {
                    uid = 0;
                }

                if(argv[4] != NULL)
                {
                    gid = (unsigned int)strtoul(argv[4], (char **)NULL, 0);
                }
                else
                {
                    gid = 0;
                }

                if (nfs_mount_ref) /*lint !e506*/
                {
                    ret = nfs_mount_ref(argv[0], fullpath, uid, gid);
                    if(ret != LOS_OK)
                    {
                        PRINTK("mount [DEVICE] [PATH] [NAME]\n[DEVICE] format error, should be IP:PATH\n");
                    }
                }else{
                    PRINTK("can't find nfs_mount\n");
                }
                free(fullpath);
                return 0;
            }

            PRINTK("mount [DEVICE] [PATH] [NAME]\n");
            free(fullpath);
            return 0;
        }

        if (strcmp(argv[0], "0") == 0)
        {
            ret = mount((const char *)NULL, fullpath, argv[2], 0, NULL);
        }
        else
        {
            ret = mount(argv[0], fullpath, argv[2], 0, NULL);
        }
        if (ret != LOS_OK)
        {
            perror("mount error");
        }
        else
        {
            PRINTK("mount ok\n");
        }

    }

    free(fullpath);
    return 0;
}

int osShellCmdUmount(int argc, char ** argv)
{
    int     ret;
    char    *filename = (char *)NULL;
    char    *fullpath = (char *)NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();
    char    *work_path = shell_working_directory;
    char    *target_path = (char *)NULL;
    int     cmp_num = 0;
    if (argc == 0)
    {
        PRINTK("umount [PATH]\n");
    }
    else
    {
        filename = argv[0];
        ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("umount error");
            return -1;
        }

        target_path = fullpath;
        cmp_num = strlen(fullpath);
        ret = strncmp(work_path,target_path,cmp_num);
        if(0 == ret)
        {
            work_path += cmp_num;
            if(*work_path == '/' || *work_path == '\0')
            {
                set_errno(EBUSY);
                perror("umount error");
                free(fullpath);
                return -1;
            }
        }

        ret = umount(fullpath);
        free(fullpath);
        if (ret != LOS_OK)
        {
            perror("umount error");
        }
        else
        {
            PRINTK("umount ok\n");
        }
    }

    return 0;
}

int osShellCmdMkdir(int argc, char **argv)
{
    int     ret;
    char    *fullpath = (char *)NULL;
    char    *filename = (char *)NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc == 1)
    {
        filename = argv[0];
        ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("mkdir error");
            return -1;
        }
        ret = mkdir(fullpath, 0);
        if (ret == -1)
        {
            perror("mkdir error");
        }
        free(fullpath);
        return 0;
    }

    PRINTK("mkdir [DRECTORY]\n");
    return 0;
}

int osShellCmdPwd(int argc, char **argv)
{
    char    pcbuf[SHOW_MAX_LEN] = {0};
    DIR     *dir = NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc > 0)
    {
        PRINTK("\nUsage: pwd\n");
        return -1;
    }

    dir = opendir(shell_working_directory);
    if (dir == NULL)
    {
        perror("pwd error");
        return -1;
    }

    LOS_TaskLock();
    (VOID)strncpy_s(pcbuf, sizeof(pcbuf), shell_working_directory, SHOW_MAX_LEN - 1);
    LOS_TaskUnlock();

    PRINTK("%s\n",pcbuf);
    (VOID)closedir(dir);
    return 0;
}

int osShellCmdStatfs(int argc, char **argv)
{
    struct statfs       sfs;
    int                 result;
    unsigned long long  toatl_size, free_size;
    char                *fullpath = (char *)NULL;
    char                *filename = argv[0];
    char * shell_working_directory = osShellGetWorkingDirtectory();

    (void)memset_s(&sfs, sizeof(sfs), 0, sizeof(sfs));

    result = vfs_normalize_path(shell_working_directory, filename, &fullpath);
    if (result < 0)
    {
        set_errno(-result);
        goto errout_with_path;
    }
    result = statfs(fullpath, &sfs);
    free(fullpath);

    if (result)
    {
        goto errout_with_path;
    }

    if (!sfs.f_type)
    {
        goto errout_with_path;
    }

    toatl_size  = (unsigned long long)sfs.f_bsize * sfs.f_blocks; /*lint !e571*/
    free_size   = (unsigned long long)sfs.f_bsize * sfs.f_bfree; /*lint !e571*/

    PRINTK("statfs got:\n f_type         = %d\n cluster_size   = %d\n total_clusters = %llu\n free_clusters  = %llu\n avail_clusters = %llu\n f_namelen      = %d\n",
        sfs.f_type, sfs.f_bsize, sfs.f_blocks, sfs.f_bfree, sfs.f_bavail, sfs.f_namelen);
    PRINTK("\n%s\n total size: %4llu Bytes\n free  size: %4llu Bytes\n", argv[0], toatl_size, free_size);

    return 0;

errout_with_path:
    PRINTK("statfs failed! Invalid argument!\n");
    PRINTK("Usage  :\n");
    PRINTK("        statfs <path>\n");
    PRINTK("        path  : Mounted file system path that requires query information\n");
    PRINTK("Example:\n");
    PRINTK("        statfs /ramfs\n");

    return -1;
}

int osShellCmdTouch(int argc, char **argv)
{
    int     fd, ret;
    char    *fullpath = (char *)NULL;
    char    *filename = (char *)NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc == 0)
    {
        PRINTK("touch [FILE]\n");
        return -1;
    }
    filename = argv[0];
    ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        perror("touch error");
        return -1;
    }
    fd = open(fullpath, O_RDWR | O_CREAT, 0);
    free(fullpath);
    if (fd == -1)
    {
        perror("touch error");
        return -1;
    }

    close(fd);
    return 0;
}

#define CP_BUF_SIZE 512
pthread_mutex_t mutex_cp = PTHREAD_MUTEX_INITIALIZER;

int osShellCmdCp(int argc, char **argv)
{
    int     ret;
    char    *srcFilepath = (char *)NULL;
    char    *dstFilename = (char *)NULL;
    char    *srcFullpath = (char *)NULL;
    char    *dstFullpath = (char *)NULL;
    char    *srcFilename = (char *)NULL;
    char    *dstFilepath = (char *)NULL;
    char    buf[CP_BUF_SIZE];
    size_t  rSize, wSize;
    FILE    *srcFp = (FILE *)NULL;
    FILE    *dstFp = (FILE *)NULL;
    struct stat statBuf;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc < 2)
    {
        PRINTK("cp [SOURCEFILE] [DESTFILE]\n");
        goto errout;
    }
    srcFilepath = argv[0];
    dstFilename = argv[1];

    // Get source fullpath.
    ret = vfs_normalize_path(shell_working_directory, srcFilepath, &srcFullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        PRINTK("cp error: can't open %s.%s\n", srcFilepath, strerror(errno));
        goto errout;
    }
    // Is source path exist?
    ret = stat(srcFullpath, &statBuf);
    if (ret == -1)
    {
        PRINTK("cp error:%s\n", strerror(errno));
        goto errout_with_srcpath;
    }

    // Is source path a directory?
    if (srcFilepath[strlen(srcFilepath) - 1] == '/' || S_ISDIR(statBuf.st_mode))
    {
        PRINTK("cp error: Source file can't be a directory.\n");
        goto errout_with_srcpath;
    }

    // Get dest fullpath.
    ret = vfs_normalize_path(shell_working_directory, dstFilename, &dstFullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        PRINTK("cp error: can't open %s.%s\n", dstFilename, strerror(errno));
        goto errout_with_srcpath;
    }

    // Is dest path exist?
    ret = stat(dstFullpath, &statBuf);
    if (ret == -1)
    {
        // Is dest path a directory?
        if (dstFilename[strlen(dstFilename) - 1] == '/')
        {
            PRINTK("cp error: %s.\n", strerror(errno));
            goto errout_with_path;
        }
    }
    else
    {
        // Is dest path a directory?
        if (S_ISDIR(statBuf.st_mode))
        {
            // Get source file name without '/'.
            srcFilename = srcFilepath;
            while(strchr(srcFilename, '/'))
            {
                srcFilename = strchr(srcFilename, '/') + 1; /*lint !e613*/
            }
            // Add the source file after dest path.
            ret = vfs_normalize_path(dstFullpath, srcFilename, &dstFilepath);
            if (ret < 0)
            {
                set_errno(-ret);
                PRINTK("cp error.%s.\n", strerror(errno));
                goto errout_with_path;
            }
            free(dstFullpath);
            dstFullpath = dstFilepath;
        }
        else
        {
            // Is dest path a directory?
            if (dstFilename[strlen(dstFilename) - 1] == '/')
            {
                PRINTK("cp error: %s is not a directory.\n", dstFullpath);
                goto errout_with_path;
            }
        }
    }

    // Is dest file same as source file?
    if (0 == strcmp(srcFullpath, dstFullpath))
    {
        PRINTK("cp error: '%s' and '%s' are the same file\n",srcFullpath, dstFullpath);
        goto errout_with_path;
    }

    // Copy begins.
    (VOID)pthread_mutex_lock(&mutex_cp);
    srcFp = fopen(srcFullpath, "r");
    if (NULL == srcFp)
    {
        PRINTK("cp error: can't open %s.%s.\n", srcFullpath, strerror(errno));
        goto errout_with_mutex;
    }

    dstFp = fopen(dstFullpath, "w+");
    if (NULL == dstFp)
    {
        PRINTK("cp error: can't open %s.%s\n", dstFullpath, strerror(errno));
        goto errout_with_srcfd;
    }
    do
    {
        (void)memset_s(buf,sizeof(buf),0,CP_BUF_SIZE);
        rSize = fread(buf,1,CP_BUF_SIZE,srcFp);
        if (rSize == (size_t)EOF)
        {
            PRINTK("cp %s %s failed.%s.\n",srcFullpath, dstFullpath, strerror(errno));
            goto errout_with_fd;
        }
        wSize = fwrite(buf, 1, rSize, dstFp);
        if (wSize != rSize)
        {
            PRINTK("cp %s %s failed.Check space left on device.\n",srcFullpath, dstFullpath);
            goto errout_with_fd;
        }
    }
    while(rSize == CP_BUF_SIZE);

    // Release resource.
    free(srcFullpath);
    free(dstFullpath);
    (VOID)fclose(srcFp);
    (VOID)fclose(dstFp);
    (VOID)pthread_mutex_unlock(&mutex_cp);
    return LOS_OK;

errout_with_fd:
    (VOID)fclose(dstFp);
errout_with_srcfd:
    (VOID)fclose(srcFp);
errout_with_mutex:
    (VOID)pthread_mutex_unlock(&mutex_cp);
errout_with_path:
    free(dstFullpath);
errout_with_srcpath:
    free(srcFullpath);
errout:
    return -1;
}

static int osShellCmdDoRmdir(const char *pathname)
{
    struct dirent *pdirent;
    struct stat stat_info;
    DIR *d;
    char *fullpath = (char *)NULL;
    int ret = 0;

    (void)memset_s(&stat_info, sizeof(stat_info), 0, sizeof(struct stat));
    if (stat(pathname, &stat_info) == 0)
    {
        if (S_ISREG(stat_info.st_mode))
        {
            ret = remove(pathname);
            return ret;
        }
        d = opendir(pathname);
        while ((pdirent = readdir(d)))
        {
            if (strcmp(pdirent->d_name , "..") && strcmp(pdirent->d_name , "."))
            {
                size_t fullpath_buf_size = strlen(pathname) + strlen(pdirent->d_name) + 2;
                fullpath = (char *)malloc(fullpath_buf_size);
                if (fullpath == NULL)
                {
                    PRINTK("malloc failure!\n");
                    (void)closedir(d);
                    return -1;
                }
                (void)snprintf(fullpath, fullpath_buf_size, "%s/%s",pathname, pdirent->d_name);
                (void)osShellCmdDoRmdir(fullpath);
                free(fullpath);
            }
        }
        (void)closedir(d);
        ret = rmdir(pathname);
        return ret;
    }
    return -1;
}

/*  Wildcard matching operations  */
static int osWildcardMatch(char * src,char *pFileName)
{
    int ret;

    if (*src)
    {
        if (*pFileName == '*')
        {
            while((*pFileName == '*') || (*pFileName == '?'))
                pFileName++;

            if (!*pFileName)
                return 0;

            while (*src && !(*src == *pFileName))
                src++;

            if (!*src)
                return -1;

            ret = osWildcardMatch(src,pFileName);

            while(ret!=0 && *(++src))
            {
                if(*src == *pFileName)
                {
                    ret = osWildcardMatch(src,pFileName);
                }
            }
            return ret;
        }
        else
        {
            if((*src == *pFileName) || (*pFileName == '?'))
            {
                return osWildcardMatch(++src, ++pFileName);
            }
            return -1;
        }
    }

    while(*pFileName)
    {
        if(*pFileName != '*')
            return -1;
        pFileName++;
    }
    return 0;
}

/*   To determine whether a wildcard character exists in a path   */
static int osIsContainersWildcard(char * pFilename)
{
    while(*pFilename)
    {
        if ((*pFilename == '*') || (*pFilename == '?'))
        {
            return 1;
        }
        else
        {
            pFilename++;
        }
    }
    return 0;
}

/*  Delete a matching file or directory  */
static void osWildcardDeleteFileOrDir(char *pFullpath, RmType mark)
{
    int ret;

    switch (mark)
    {
        case RM_RECURSIVER:
            ret = osShellCmdDoRmdir(pFullpath);
            break;
        case RM_FILE:
            ret = unlink(pFullpath);
            break;
        case RM_DIR:
            ret = rmdir(pFullpath);
            break;
        default:
            return;
    }
    if (ret == -1)
    {
        PRINTK("%s  ", pFullpath);
        perror("rm/rmdir error!");
    }
    else
    {
        PRINTK("%s match successful!delete!\n", pFullpath);
    }

    return;
}

/*  Split the path with wildcard characters  */
static char* osWildcardSplitPath(char * pFullpath, char ** ppHandle, char ** ppWait)
{
    int n = 0, a = 0, b = 0;
    int len  = strlen(pFullpath);

    for (n = 0; n < len; n++)
    {
        if(pFullpath[n] == '/')
        {
            if (b != 0)
            {
                pFullpath[n] = '\0';
                *ppWait = pFullpath + n + 1;
                break;
            }
            a = n;
        }
        else if (pFullpath[n] == '*' || pFullpath[n] == '?')
        {
            b = n;
            pFullpath[a] = '\0';
            if (a == 0)
            {
                 *ppHandle = pFullpath + a + 1;
                 continue;
            }
            *ppHandle = pFullpath + a + 1;
        }
    }
    return pFullpath;
}

/*  Handling entry of the path with wildcard characters  */
static void osWildcardExtractDirectory(char *fullpath, RmType mark)
{
    char separator[] = "/";
    char rmpath[PATH_MAX] = {0};
    struct dirent *pdirent;
    char * f, *s, *t;
    int ret;
    DIR *d;

    f = s = t = NULL;
    f = osWildcardSplitPath(fullpath, &s, &t);

    if (s == NULL)
    {
        osWildcardDeleteFileOrDir(fullpath, mark);
        return;
    }

    if (!*f)
    {
        d = opendir("/");
    }
    else
    {
        d = opendir(f);
    }
    if (d == NULL)
    {
        perror("opendir error");
        return;
    }
    while ((pdirent = readdir(d)))
    {
        strcpy(rmpath, f);/*lint !e586*/
        ret = osWildcardMatch(pdirent ->d_name, s);

        if (ret == 0)
        {
            (void)strcat_s(rmpath, sizeof(rmpath), separator);
            (void)strcat_s(rmpath, sizeof(rmpath), pdirent ->d_name);

            if (t == NULL)
            {
                osWildcardDeleteFileOrDir(rmpath, mark);
            }
            else
            {
                (void)strcat_s(rmpath, sizeof(rmpath), separator);
                (void)strcat_s(rmpath, sizeof(rmpath),  t);
                osWildcardExtractDirectory(rmpath, mark);
            }
        }
    }
    (void)closedir(d);
}

int osShellCmdRm(int argc, char **argv)
{
    int ret = 0;
    char * fullpath = (char *)NULL;
    char * filename = (char *)NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc != 1 && argc != 2)
    {
        PRINTK("rm [FILE] or rm [-r/-R] [FILE]\n");
        return -1;
    }
    if (argc == 2)
    {
        if (!strcmp(argv[0],"-r") || !strcmp(argv[0],"-R"))
        {
            filename = argv[1];
            ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
            if (ret < 0)
            {
                set_errno(-ret);
                perror("rm error");
                return -1;
            }
            if (osIsContainersWildcard(fullpath))
            {
                osWildcardExtractDirectory(fullpath, RM_RECURSIVER);
            }
            else
            {
                ret = osShellCmdDoRmdir(fullpath);
            }
        }
        else
        {
            PRINTK("rm [FILE] or rm [-r/-R] [FILE]\n");
            return -1;
        }
    }
    else
    {
        filename = argv[0];
        ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
        if (ret < 0)
        {
            set_errno(-ret);
            perror("rm error");
            return -1;
        }
        if (osIsContainersWildcard(fullpath))
        {
            osWildcardExtractDirectory(fullpath, RM_FILE);
        }
        else
        {
            ret = unlink(fullpath);
        }
    }
    if (ret == -1)
    {
        perror("rm error");
    }
    free(fullpath);
    return 0;
}

int osShellCmdRmdir(int argc, char **argv)
{
    int ret = 0;
    char * fullpath = (char *)NULL;
    char * filename = (char *)NULL;
    char * shell_working_directory = osShellGetWorkingDirtectory();

    if (argc == 0)
    {
        PRINTK("rmdir [DRECTORY]\n");
        return -1;
    }
    filename = argv[0];
    ret = vfs_normalize_path(shell_working_directory, filename, &fullpath);
    if (ret < 0)
    {
        set_errno(-ret);
        perror("rmdir error");
        return -1;
    }
    if (osIsContainersWildcard(fullpath))
    {
        osWildcardExtractDirectory(fullpath, RM_DIR);
    }
    else
    {
        ret = rmdir(fullpath);
    }
    free(fullpath);

    if (ret == -1)
    {
        perror("rmdir error");
    }

    return 0;
}

int osShellCmdSync(int argc, char **argv)
{
    /*TODO:1.the standard return is int, but LiteOS return void, need modify. 2.the sync() is not statement.*/
    extern void sync(void);

    if (argc > 0)
    {
        PRINTK("\nUsage: sync\n");
        return -1;
    }

    sync();
    return 0;
}

int osShellCmdLsfd(int argc, char **argv)
{
    extern void lsfd(void);

    if (argc > 0)
    {
        PRINTK("\nUsage: lsfd\n");
        return -1;
    }

    lsfd();

    return 0;
}

SHELLCMD_ENTRY(lsfd_shellcmd, CMD_TYPE_EX, "lsfd", XARGS, (CMD_CBK_FUNC)osShellCmdLsfd); /*lint !e19*/
SHELLCMD_ENTRY(ls_shellcmd, CMD_TYPE_EX, "ls", XARGS, (CMD_CBK_FUNC)osShellCmdLs); /*lint !e19*/
SHELLCMD_ENTRY(pwd_shellcmd, CMD_TYPE_EX, "pwd", XARGS, (CMD_CBK_FUNC)osShellCmdPwd); /*lint !e19*/
SHELLCMD_ENTRY(cd_shellcmd, CMD_TYPE_EX, "cd", XARGS, (CMD_CBK_FUNC)osShellCmdCd); /*lint !e19*/
SHELLCMD_ENTRY(cat_shellcmd, CMD_TYPE_EX, "cat", XARGS, (CMD_CBK_FUNC)osShellCmdCat); /*lint !e19*/
SHELLCMD_ENTRY(rm_shellcmd, CMD_TYPE_EX, "rm", XARGS, (CMD_CBK_FUNC)osShellCmdRm); /*lint !e19*/
SHELLCMD_ENTRY(rmdir_shellcmd, CMD_TYPE_EX, "rmdir", XARGS, (CMD_CBK_FUNC)osShellCmdRmdir); /*lint !e19*/
SHELLCMD_ENTRY(mkdir_shellcmd, CMD_TYPE_EX, "mkdir", XARGS, (CMD_CBK_FUNC)osShellCmdMkdir); /*lint !e19*/
#if (defined(LOSCFG_FS_FAT))
SHELLCMD_ENTRY(sync_shellcmd, CMD_TYPE_EX, "sync", XARGS, (CMD_CBK_FUNC)osShellCmdSync); /*lint !e19*/
#endif
#if (defined(LOSCFG_FS_FAT)||defined(LOSCFG_FS_RAMFS)||defined(LOSCFG_FS_YAFFS)||defined(LOSCFG_FS_JFFS) || defined(LOSCFG_FS_NFS))
SHELLCMD_ENTRY(touch_shellcmd, CMD_TYPE_EX, "touch", XARGS, (CMD_CBK_FUNC)osShellCmdTouch); /*lint !e19*/
SHELLCMD_ENTRY(statfs_shellcmd, CMD_TYPE_EX, "statfs", XARGS, (CMD_CBK_FUNC)osShellCmdStatfs); /*lint !e19*/
SHELLCMD_ENTRY(mount_shellcmd, CMD_TYPE_EX, "mount", XARGS, (CMD_CBK_FUNC)osShellCmdMount); /*lint !e19*/
SHELLCMD_ENTRY(umount_shellcmd, CMD_TYPE_EX, "umount", XARGS, (CMD_CBK_FUNC)osShellCmdUmount); /*lint !e19*/
SHELLCMD_ENTRY(cp_shellcmd, CMD_TYPE_EX, "cp", XARGS, (CMD_CBK_FUNC)osShellCmdCp); /*lint !e19*/
#endif

#endif
