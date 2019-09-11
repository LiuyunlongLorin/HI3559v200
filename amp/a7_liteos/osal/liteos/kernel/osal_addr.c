#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "hisoc/mmu_config.h"

void *osal_ioremap(unsigned long phys_addr, unsigned long size)
{
    return ioremap_nocache(phys_addr, size);
}

#define MEM_PAGE_ALIGN 0x1000
#define mem_ceil_align(x, g) ((((x) + (g)-1) / (g)) * (g))
#define mem_floor_align(x, g) ((x) / (g) * (g))

void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
    return ioremap_nocache(phys_addr, size);
}

void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size)
{
    return ioremap_cached(phys_addr, size);
}

void osal_iounmap(void *addr)
{
    return iounmap(addr);
}

unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
    return copy_from_user(to, from, n);
}

unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
    return copy_to_user(to, from, n);
}

int osal_access_ok(int type, const void *addr, unsigned long size)
{
    return 1;
}

