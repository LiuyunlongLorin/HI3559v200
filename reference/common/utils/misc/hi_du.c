/**
 * @file    hi_du.c
 * @brief   du implementation from busybox-1.28.0.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/10
 * @version   1.0

 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>

#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))

struct globals
{
#if 0
#if ENABLE_FEATURE_HUMAN_READABLE
    unsigned long disp_unit;
#else
    unsigned disp_k;
#endif
    int max_print_depth;
    bool status;
#endif
    int slink_depth;
    int du_depth;
    dev_t dir_dev;
} FIX_ALIASING;

static struct globals G;

static char* last_char_is(const char* s, int c)
{
    if (s && *s)
    {
        size_t sz = strlen(s) - 1;
        s += sz;

        if ( (unsigned char)*s == c)
        {
            return (char*)s;
        }
    }

    return NULL;
}

static char* xasprintf(const char* format, ...)
{
    va_list p;
    char* string_ptr;
    va_start(p, format);
    vasprintf(&string_ptr, format, p);
    va_end(p);
    return string_ptr;
}

static char* concat_path_file(const char* path, const char* filename)
{
    char* lc;

    if (!path)
    {
        path = "";
    }

    lc = last_char_is(path, '/');

    while (*filename == '/')
    {
        filename++;
    }

    return xasprintf("%s%s%s", path, (lc == NULL ? "/" : ""), filename);
}

static char* concat_subpath_file(const char* path, const char* f)
{
    if (f && DOT_OR_DOTDOT(f))
    {
        return NULL;
    }

    return concat_path_file(path, f);
}
#if 0
static void print(unsigned long long size, const char* filename)
{
    size++;
    size >>= 1;
    printf("%llu\t%s\n", size, filename);
}
#endif
static unsigned long long du(const char* filename)
{
    G.slink_depth = INT_MAX;
    struct stat statbuf;
    unsigned long long sum;

    if (lstat(filename, &statbuf) != 0)
    {
        //G.status = EXIT_FAILURE;
        return 0;
    }

    if (G.du_depth == 0)
    {
        G.dir_dev = statbuf.st_dev;
    }
    else if (G.dir_dev != statbuf.st_dev)
    {
        return 0;
    }

    sum = statbuf.st_blocks;

    if (S_ISLNK(statbuf.st_mode))
    {
        if (G.slink_depth > G.du_depth)
        {
            /* -H or -L */
            if (stat(filename, &statbuf) != 0)
            {
                //G.status = EXIT_FAILURE;
                return 0;
            }

            sum = statbuf.st_blocks;

            if (G.slink_depth == 1)
            {
                /* Convert -H to -L */
                G.slink_depth = INT_MAX;
            }
        }
    }

    if (S_ISDIR(statbuf.st_mode))
    {
        DIR* dir;
        struct dirent* entry;
        char* newfile;
        dir = opendir(filename);

        if (!dir)
        {
            //G.status = EXIT_FAILURE;
            return sum;
        }

        while ((entry = readdir(dir)))
        {
            newfile = concat_subpath_file(filename, entry->d_name);

            if (newfile == NULL)
            {
                continue;
            }

            ++G.du_depth;
            sum += du(newfile);
            --G.du_depth;
            free(newfile);
        }

        closedir(dir);
    }
    else
    {
        if (G.du_depth != 0)
        {
            return sum;
        }
    }
#if 0
    if (G.du_depth <= G.max_print_depth)
    {
        print(sum, filename);
    }
#endif
    return sum;
}
/*********************************************************************************/

HI_S32 HI_du(const HI_CHAR* pszPath, HI_U64* pu64Size_KB)
{
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pu64Size_KB, HI_EINVAL);
    HI_S32 s32Ret = 0;
    *pu64Size_KB = 0;
    s32Ret = HI_PathIsDirectory(pszPath);

    if (-1 == s32Ret)
    {
        return HI_EINVAL;
    }
    else
    {
        HI_U64 u64Totalblocks = 0;
        u64Totalblocks = du(pszPath);
#ifndef __HuaweiLite__
        if (0 < u64Totalblocks)
        {
            u64Totalblocks++;
            u64Totalblocks >>= 1;
        }
        *pu64Size_KB = u64Totalblocks;
        return HI_SUCCESS;
#else
        struct statfs disk_statfs;
        if( statfs(pszPath, &disk_statfs) >= 0 )
        {
            *pu64Size_KB = u64Totalblocks*disk_statfs.f_bsize >> 10;
            return HI_SUCCESS;
        }
        return HI_EINTER;
#endif
    }
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
