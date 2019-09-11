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
 ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "limits.h"
#include "errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef VFS_USING_WORKDIR
char working_directory[PATH_MAX] = {"/"};
#endif

unsigned int vfs_strnlen(const char *str, size_t maxlen)
{
    const char *p;

    for (p = str; maxlen-- != 0 && *p != '\0'; ++p) ;

    return p - str;
}

/* abandon the redundant '/' in the path, only keep one. */
static char * str_path(char *path)
{
    char *dst, *src;
    src = path;
    dst = path;
    while (*src != '\0')
    {
        if (*src == '/')
        {
            *dst++ = *src++;
            while(*src == '/')
                src++;
            continue;
        }
        *dst++ = *src++;
    }
    *dst = '\0';
    return path;
}

static char * str_normalize_path(char * fullpath)
{
    char *dst, *src;

    src = fullpath;
    dst = fullpath;

    while (*src != '\0')
    {
        if (*src == '.')
        {
            if(*(src+1) == '/')
            {
                src += 2;
                continue;
            }
            else if (*(src+1) == '.')
            {
                if (*(src+2) == '/' || *(src+2) == '\0')
                    src += 2;
                else
                {
                    while (*src != '\0' && *src != '/')
                        *dst++ = *src++;
                    continue;
                }
            }
            else
            {
                *dst++ = *src++;
                continue;
            }
        }
        else
        {
            *dst++ = *src++;
            continue;
        }

        if (dst-1 != fullpath)
            dst--;

        while (dst > fullpath && *(dst-1) != '/')
            dst--;

        if (*src == '/')
            src++;
    }

    *dst = '\0';
    /* remove '/' in the end of path if exist */
    dst --;

    if ( (*dst == '.') && (*(dst-1) == '/'))
    {
        *dst = '\0';
        dst --;
    }
    if ((dst != fullpath) && (*dst == '/'))
        *dst = '\0';

    return dst;
}

int vfs_normalize_path(const char *directory, const char *filename, char **pathname)
{
    char *fullpath = (char *)NULL;
    int nameLen;
    char *name;

    if (NULL == pathname)
    {
        return -EINVAL;
    }
    /* check parameters */
    if(!filename)
    {
        *pathname = (char *)NULL;
        return -EINVAL;
    }
    
    nameLen = vfs_strnlen(filename,PATH_MAX);
    
    if (!nameLen)
    {
        *pathname = (char *)NULL;
        return -EINVAL;
    }
    else if (nameLen >= PATH_MAX)
    {
        *pathname = (char *)NULL;
        return -ENAMETOOLONG;
    }

    for (name = (char *)filename + nameLen; name != filename && *name != '/'; name--);
    if (strlen(name) > NAME_MAX)
    {
        *pathname = (char *)NULL;
        return -ENAMETOOLONG;
    }

#ifdef VFS_USING_WORKDIR
    if (directory == NULL) /* shell use working directory */
        directory = &working_directory[0];
#else
    if ((directory == NULL) && (filename[0] != '/'))
    {
        PRINT_ERR("NO_WORKING_DIR\n");
        *pathname = (char *)NULL;
        return -EINVAL;
    }
#endif

    if (filename[0] != '/' && strlen(directory) + nameLen + 2 > PATH_MAX)
    {
        return -ENAMETOOLONG;
    }
    if (filename[0] != '/') /* not a absolute path */
    {
        if (nameLen > 1 && filename[0] == '.' && filename[1] == '/')
            filename += 2;

        fullpath = (char *)malloc(strlen(directory) + nameLen + 2);

        if (fullpath == NULL)
        {
            *pathname = (char *)NULL;
            return -ENOMEM;
        }
        /* join path and file name */
        (void)snprintf(fullpath, strlen(directory) + nameLen + 2,
            "%s/%s", directory, filename);
    }
    else /* it's a absolute path, use it directly */
    {
        fullpath = strdup(filename); /* copy string */

        if (fullpath == NULL)
        {
            *pathname = (char *)NULL;
            return -ENOMEM;
        }
        if (filename[1] == '/')
        {
            *pathname = (char *)NULL;
            free(fullpath);
            return -EINVAL;
        }
    }

    (void)str_path(fullpath);
    (void)str_normalize_path(fullpath);

    *pathname = fullpath;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

