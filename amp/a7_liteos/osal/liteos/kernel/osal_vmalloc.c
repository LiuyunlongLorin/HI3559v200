#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "hi_osal.h"

void *osal_vmalloc(unsigned long size)
{
    return vmalloc(size);
}

void osal_vfree(const void *addr)
{
    vfree((void *)addr);
}

void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    return kmalloc(size, GFP_ATOMIC);
}

void osal_kfree(const void *addr)
{
    kfree((void *)addr);
}
