/**
 * @file    hi_modutils.c
 * @brief   common modutils related functions from busybox-1.28.0.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/10
 * @version   1.0

 */
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <byteswap.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* linux/include/linux/module.h has limit of 64 chars on module names */
#define MODULE_NAME_LEN 64
#define SWAP_BE32(x) bswap_32(x)

#ifdef __UCLIBC__
extern int init_module(void* module_image, unsigned long len, const char* param_values);
extern int delete_module(const char* name, int flags);
#else
#include <sys/syscall.h>
#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
#define delete_module(mod, flags) syscall(__NR_delete_module, mod, flags)
#endif

static const char* moderror(int err)
{
    switch (err)
    {
        case -1: /* btw: it's -EPERM */
            return "no such module";

        case ENOEXEC:
            return "invalid module format";

        case ENOENT:
            return "unknown symbol in module, or unknown parameter";

        case ESRCH:
            return "module has wrong symbol version";

        case ENOSYS:
            return "kernel does not support requested operation";
    }

    if (err < 0) /* should always be */
    {
        err = -err;
    }

    return strerror(err);
}

static char* filename2modname(const char* filename, char* modname)
{
    int i;
    const char* from;

    if (filename == NULL)
    {
        return NULL;
    }

    if (modname != NULL)
    {
        from = filename;

        for (i = 0; i < (MODULE_NAME_LEN - 1) && from[i] != '\0' && from[i] != '.'; i++)
        {
            modname[i] = (from[i] == '-') ? '_' : from[i];
        }

        modname[i] = '\0';
    }

    return modname;
}

static void* try_to_mmap_module(const char* filename, size_t* image_size_p)
{
    /* We have user reports of failure to load 3MB module
     * on a 16MB RAM machine. Apparently even a transient
     * memory spike to 6MB during module load
     * is too big for that system. */
    void* image;
    struct stat st;
    int fd;

    fd = open(filename, O_RDONLY, 0666);
    fstat(fd, &st);
    image = NULL;

    /* st.st_size is off_t, we can't just pass it to mmap */
    if (st.st_size <= *image_size_p)
    {
        size_t image_size = st.st_size;
        image = mmap(NULL, image_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if (image == MAP_FAILED)
        {
            image = NULL;
        }
        else if (*(uint32_t*)image != SWAP_BE32(0x7f454C46))
        {
            /* No ELF signature. Compressed module? */
            munmap(image, image_size);
            image = NULL;
        }
        else
        {
            /* Success. Report the size */
            *image_size_p = image_size;
        }
    }

    close(fd);
    return image;
}

static int hi_init_module(const char* filename, const char* options)
{
    size_t image_size;
    char* image;
    int rc;

    if (!options)
    {
        options = "";
    }

    image_size = INT_MAX - 4095;
    image = try_to_mmap_module(filename, &image_size);

    if (!image)
    {
        errno = ENOMEM;
        return -errno;
    }

    errno = 0;
    init_module(image, image_size, options);
    rc = errno;
    munmap(image, image_size);
    return rc;
}

static int hi_delete_module(const char* module, unsigned int flags)
{
    errno = 0;
    delete_module(module, flags);
    return errno;
}

static HI_CHAR* local_basename (HI_CHAR *filename)
{
    HI_CHAR *p;

    if (filename == NULL || filename[0] == '\0')
    {
        /* We return a pointer to a static string containing ".".  */
        p = (HI_CHAR *) ".";
    }
    else
    {
        p = strrchr (filename, '/');

        if (p == NULL)
        {
            /* There is no slash in the filename.  Return the whole string.  */
            p = filename;
        }
        else
        {
            if (p[1] == '\0')
            {
                /* We must remove trailing '/'.  */
                while (p > filename && p[-1] == '/')
                    --p;

                /* Now we can be in two situations:
                   a) the string only contains '/' characters, so we return '/'
                   b) p points past the last component, but we have to remove the trailing slash.
                */
                if (p > filename)
                {
                    *p-- = '\0';
                    while (p > filename && p[-1] != '/')
                        --p;
                }
                else
                {
                    /* The last slash we already found is the right position to return.  */
                    while (p[1] != '\0')
                        ++p;
                }
            }
            else
            {
                /* Go to the first character of the name.  */
                ++p;
            }
        }
    }

    return p;
}
/*********************************************************************************/

HI_S32 HI_insmod(const HI_CHAR* pszPath, const HI_CHAR* pszOptions)
{
    HI_S32 rc = 0;
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_EINVAL);
    rc = hi_init_module(pszPath, pszOptions);

    if (rc)
    {
        MLOGE("can't insert '%s': %s\n", pszPath, moderror(rc));
    }

    return rc;
}

HI_S32 HI_rmmod(const HI_CHAR* pszPath)
{
    HI_U32 flags = O_NONBLOCK | O_EXCL;
    HI_CHAR modname[MODULE_NAME_LEN];
    HI_APPCOMM_CHECK_POINTER(pszPath, HI_EINVAL);
    const HI_CHAR* bname = local_basename((HI_CHAR*)pszPath);

    if (NULL == filename2modname(bname, modname))
    {
        return HI_FAILURE;
    }

    if (hi_delete_module(modname, flags))
    {
        MLOGE("can't unload '%s': %s\n", modname, moderror(errno));
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
