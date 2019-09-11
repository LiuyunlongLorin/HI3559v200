#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/cacheflush.h>
#include <linux/dma-direction.h>
#include "osal_mmz.h"

void osal_flush_cache_all(void)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return;
}

void osal_cpuc_flush_dcache_area(void *addr, int size)
{
    // dma_cache_clean(addr, addr + size);
    // dma_cache_inv((int)addr, addr + size);
    dprintf("Do not support in liteos\n");
}

void osal_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long length)
{
    hil_mmb_flush_dcache_byaddr(kvirt, phys_addr, length);
}

int osal_flush_dcache_all(void)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return 0;
}

