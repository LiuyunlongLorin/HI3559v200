#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

static int klib_fopen(const char *filename, int flags, int mode)
{
    return open(filename, flags, mode);
}

static void klib_fclose(int filp)
{
    if (filp != -1) {
        close(filp);
    }
    return;
}

static int klib_fwrite(const char *buf, int len, int filp)
{
    int writelen;

    writelen = write(filp, buf, len);

    return writelen;
}

static int klib_fread(char *buf, unsigned int len, int filp)
{
    int readlen;
    readlen = read(filp, buf, len);
    return readlen;
}

void *osal_klib_fopen(const char *filename, int flags, int mode)
{
    int file;
    int *pfilp = NULL;

    file = klib_fopen(filename, flags, mode);
    if (file == -1) {
        pfilp = NULL;
    } else {
        *pfilp = file;
    }

    return (void *)pfilp;
}
void osal_klib_fclose(void *filp)
{
    int u32Filp = *(int *)filp;

    klib_fclose(u32Filp);
    return;
}

int osal_klib_fwrite(const char *buf, int len, void *filp)
{
    int u32Filp = *(int *)filp;

    return klib_fwrite(buf, len, u32Filp);
}

int osal_klib_fread(char *buf, unsigned int len, void *filp)
{
    int u32Filp = *(int *)filp;

    return klib_fread(buf, len, u32Filp);
}

