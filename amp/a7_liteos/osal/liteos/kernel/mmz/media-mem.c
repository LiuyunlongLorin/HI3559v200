/* media-mem.c
*
* Copyright (c) 2006 Hisilicon Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*
*/

#include <linux/kernel.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
// #include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <linux/seq_file.h>

#include <linux/string.h>
#include <linux/list.h>

#include <linux/time.h>
#include <asm/outercache.h>
#include <linux/dma-mapping.h>

#include "osal_mmz.h"
#include "proc_fs.h"
#include "hi_osal.h"

#define MMZ_DBG_LEVEL  0x0
#define mmz_trace(level, s, params...)                                             \
    do {                                                                           \
        if (level & MMZ_DBG_LEVEL)                                                 \
            printk(KERN_INFO "[%s, %d]: " s "\n", __FUNCTION__, __LINE__, params); \
    } while (0)

#define mmz_trace_func() mmz_trace(0x02, "%s", __FILE__)

#define MMZ_GRAIN      PAGE_SIZE
#define mmz_bitmap_size(p) (mmz_align2(mmz_length2grain((p)->nbytes), 8) / 8)

#define mmz_get_bit(p, n) (((p)->bitmap[(n) / 8] >> ((n)&0x7)) & 0x1)
#define mmz_set_bit(p, n) (p)->bitmap[(n) / 8] |= 1 << ((n)&0x7)
#define mmz_clr_bit(p, n) (p)->bitmap[(n) / 8] &= ~(1 << ((n)&0x7))

#define mmz_pos2phy_addr(p, n) ((p)->phys_start + (n)*MMZ_GRAIN)
#define mmz_phy_addr2pos(p, a) (((a) - (p)->phys_start) / MMZ_GRAIN)

#define mmz_align2low(x, g) (((x) / (g)) * (g))
#define mmz_align2(x, g) ((((x) + (g)-1) / (g)) * (g))
#define mmz_grain_align(x) mmz_align2(x, MMZ_GRAIN)
#define mmz_length2grain(len) (mmz_grain_align(len) / MMZ_GRAIN)

static unsigned long _strtoul_ex(const char *s, char **ep, unsigned int base)
{
    char *__end_p = NULL;
    unsigned long __value;

    __value = strtoul(s, &__end_p, base);

    switch (*__end_p) {
        case 'm':
        case 'M':
            __value <<= 10;

        case 'k':
        case 'K':
            __value <<= 10;

            if (ep != NULL) {
                (*ep) = __end_p + 1;
            }

        default:
            break;
    }

    return __value;
}

static OSAL_LIST_HEAD(mmz_list);

extern struct osal_list_head map_mmz_list;
OSAL_LIST_HEAD(map_mmz_list);

static DEFINE_SEMAPHORE(mmz_lock);

static int anony = 0;
module_param(anony, int, S_IRUGO);
static int mmz_info_phys_start = -1;
int zone_number = 0;
int block_number = 0;
unsigned int mmb_number = 0; /* for mmb id */

hil_mmz_t *hil_mmz_create(const char *name, unsigned long gfp, unsigned long phys_start, unsigned long nbytes)
{
    hil_mmz_t *p = NULL;

    mmz_trace_func();

    if (name == NULL) {
        printk(KERN_ERR "%s: 'name' can not be zero!", __FUNCTION__);
        return NULL;
    }

    p = kmalloc(sizeof(hil_mmz_t) + 1, GFP_KERNEL);

    if (p == NULL) {
        return NULL;
    }

    memset(p, 0, sizeof(hil_mmz_t) + 1);
    strlcpy(p->name, name, HIL_MMZ_NAME_LEN);
    p->gfp = gfp;
    p->phys_start = phys_start;
    p->nbytes = nbytes;

    OSAL_INIT_LIST_HEAD(&p->list);
    OSAL_INIT_LIST_HEAD(&p->mmb_list);

    p->destructor = free;

    return p;
}
EXPORT_SYMBOL(hil_mmz_create);

hil_mmz_t *hil_mmz_create_v2(const char *name, unsigned long gfp, unsigned long phys_start,
                             unsigned long nbytes, unsigned int alloc_type, unsigned long block_align)
{
    hil_mmz_t *p = NULL;

    mmz_trace_func();

    if (name == NULL) {
        printk(KERN_ERR "%s: 'name' can not be zero!", __FUNCTION__);
        return NULL;
    }

    p = kmalloc(sizeof(hil_mmz_t), GFP_KERNEL);

    if (p == NULL) {
        return NULL;
    }

    memset(p, 0, sizeof(hil_mmz_t));
    strlcpy(p->name, name, HIL_MMZ_NAME_LEN);
    p->gfp = gfp;
    p->phys_start = phys_start;
    p->nbytes = nbytes;
    p->alloc_type = alloc_type;
    p->block_align = block_align;

    OSAL_INIT_LIST_HEAD(&p->list);
    OSAL_INIT_LIST_HEAD(&p->mmb_list);

    p->destructor = free;

    return p;
}

int hil_mmz_destroy(hil_mmz_t *zone)
{
    if (zone == NULL) {
        return -1;
    }

    if (zone->destructor) {
        zone->destructor(zone);
    }

    return 0;
}
EXPORT_SYMBOL(hil_mmz_destroy);

static int _check_mmz(hil_mmz_t *zone)
{
    hil_mmz_t *p = NULL;

    unsigned long new_start = zone->phys_start;
    unsigned long new_end = zone->phys_start + zone->nbytes;

    if (zone->nbytes == 0) {
        return -1;
    }

    osal_list_for_each_entry(p, &mmz_list, list) {
        unsigned long start, end;
        start = p->phys_start;
        end = p->phys_start + p->nbytes;

        if (new_start >= end) {
            continue;
        } else if ((new_start < start) && (new_end <= start)) {
            continue;
        } else
            ;

        printk(KERN_ERR "ERROR: Conflict MMZ:\n");
        printk(KERN_ERR "MMZ new:   " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(zone));
        printk(KERN_ERR "MMZ exist: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(p));
        printk(KERN_ERR "Add new MMZ failed!\n");
        return -1;
    }

    return 0;
}

int hil_mmz_register(hil_mmz_t *zone)
{
    int ret = 0;

    mmz_trace(1, HIL_MMZ_FMT_S, hil_mmz_fmt_arg(zone));

    if (zone == NULL) {
        return -1;
    }

    down(&mmz_lock);

    ret = _check_mmz(zone);

    if (ret) {
        up(&mmz_lock);
        return ret;
    }

    OSAL_INIT_LIST_HEAD(&zone->mmb_list);

    osal_list_add(&zone->list, &mmz_list);

    up(&mmz_lock);

    return 0;
}

int hil_mmz_unregister(hil_mmz_t *zone)
{
    int losts = 0;
    hil_mmb_t *p = NULL;

    if (zone == NULL) {
        return -1;
    }

    mmz_trace_func();

    down(&mmz_lock);
    osal_list_for_each_entry(p, &zone->mmb_list, list) {
        printk(KERN_WARNING "          MB Lost: " HIL_MMB_FMT_S "\n", hil_mmb_fmt_arg(p));
        losts++;
    }

    if (losts) {
        printk(KERN_ERR "%d mmbs not free, mmz<%s> can not be deregistered!\n", losts, zone->name);
        up(&mmz_lock);
        return -1;
    }

    osal_list_del(&zone->list);
    up(&mmz_lock);

    return 0;
}

static unsigned long _find_fixed_region(unsigned long *region_len, hil_mmz_t *mmz,
                                        unsigned long size, unsigned long align)
{
    unsigned long start;
    unsigned long fixed_start = 0;
    unsigned long fixed_len = -1;
    unsigned long len = 0;
    unsigned long blank_len = 0;
    hil_mmb_t *p = NULL;

    mmz_trace_func();
    align = mmz_grain_align(align);
    start = mmz_align2(mmz->phys_start, align);
    len = mmz_grain_align(size);

    osal_list_for_each_entry(p, &mmz->mmb_list, list) {
        hil_mmb_t *next;
        mmz_trace(4, "p->phys_addr=0x%08lX p->length = %luKB \t",
                  p->phys_addr, p->length / SZ_1K);
        next = osal_list_entry(p->list.next, __typeof__(*p), list);
        mmz_trace(4, ",next = 0x%08lX\n\n", next->phys_addr);

        /* if p is the first entry or not */
        if (osal_list_first_entry(&mmz->mmb_list, __typeof__(*p), list) == p) {
            blank_len = p->phys_addr - start;

            if ((blank_len < fixed_len) && (blank_len >= len)) {
                fixed_len = blank_len;
                fixed_start = start;
                mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                          __LINE__, fixed_start, fixed_len / SZ_1K);
            }
        }

        start = mmz_align2((p->phys_addr + p->length), align);
        BUG_ON((start < mmz->phys_start) || (start > (mmz->phys_start + mmz->nbytes)));

        /* if we have to alloc after the last node */
        if (osal_list_is_last(&p->list, &mmz->mmb_list)) {
            blank_len = mmz->phys_start + mmz->nbytes - start;

            if ((blank_len < fixed_len) && (blank_len >= len)) {
                fixed_len = blank_len;
                fixed_start = start;
                mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                          __LINE__, fixed_start, fixed_len / SZ_1K);
                break;
            } else {
                if (fixed_len != -1) {
                    goto out;
                }

                fixed_start = 0;
                mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                          __LINE__, fixed_start, fixed_len / SZ_1K);
                goto out;
            }
        }

        /* blank is too little */
        if ((start + len) > next->phys_addr) {
            mmz_trace(4, "start=0x%08lX ,len=%lu,next=0x%08lX\n",
                      start, len, next->phys_addr);
            continue;
        }

        blank_len = next->phys_addr - start;

        if ((blank_len < fixed_len) && (blank_len >= len)) {
            fixed_len = blank_len;
            fixed_start = start;
            mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                      __LINE__, fixed_start, fixed_len / SZ_1K);
        }
    }

    if ((mmz_grain_align(start + len) <= (mmz->phys_start + mmz->nbytes)) &&
        (start >= mmz->phys_start) &&
        (start < (mmz->phys_start + mmz->nbytes))) {
            fixed_len = len;
            fixed_start = start;
            mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                  __LINE__, fixed_start, fixed_len / SZ_1K);
        } else {
            fixed_start = 0;
            mmz_trace(4, "%d: fixed_region: start=0x%08lX, len=%luKB\n",
                  __LINE__, fixed_start, len / SZ_1K);
        }

out:
    *region_len = len;
    return fixed_start;
}

#if 0
static unsigned long _find_fixed_region_from_highaddr(unsigned long *region_len, hil_mmz_t *mmz,
                                                      unsigned long size, unsigned long align)
{
    int i, j;
    unsigned long fixed_start = 0;
    unsigned long fixed_len = ~1;

    mmz_trace_func();

    i = mmz_length2grain(mmz->nbytes);

    for (; i > 0; i--) {
        unsigned long start;
        unsigned long len;
        unsigned long start_highaddr;

        if (mmz_get_bit(mmz, i))
        { continue; }

        len = 0;
        start_highaddr = mmz_pos2phy_addr(mmz, i);

        for (; i > 0; i--) {
            if (mmz_get_bit(mmz, i)) {
                break;
            }

            len += MMZ_GRAIN;
        }

        if (len >= size) {
            j = mmz_phy_addr2pos(mmz, mmz_align2low(start_highaddr - size, align));
            // align = mmz_grain_align(align)/MMZ_GRAIN;
            // start = mmz_pos2phy_addr(mmz, j - align);
            start = mmz_pos2phy_addr(mmz, j);

            if ((start_highaddr - len <= start) && (start <= start_highaddr - size)) {
                fixed_len = len;
                fixed_start = start;
                break;
            }

            mmz_trace(1, "fixed_region: start=0x%08lX, len=%luKB", fixed_start, fixed_len / SZ_1K);
        }
    }

    *region_len = fixed_len;

    return fixed_start;
}
#endif

static int _do_mmb_alloc(hil_mmb_t *mmb)
{
    hil_mmb_t *p = NULL;
    mmz_trace_func();

    /* add mmb sorted */
    osal_list_for_each_entry(p, &mmb->zone->mmb_list, list) {
        if (mmb->phys_addr < p->phys_addr) {
            break;
        }

        if (mmb->phys_addr == p->phys_addr) {
            printk(KERN_ERR "ERROR: media-mem allocator bad in %s! (%s, %d)",
                   mmb->zone->name, __FUNCTION__, __LINE__);

            return -1;
        }
    }
    osal_list_add(&mmb->list, p->list.prev);

    mmz_trace(1, HIL_MMB_FMT_S, hil_mmb_fmt_arg(mmb));

    return 0;
}

static hil_mmb_t *__mmb_alloc(const char *name, unsigned long size, unsigned long align,
                              unsigned long gfp, const char *mmz_name, hil_mmz_t *_user_mmz)
{
    hil_mmz_t *mmz = NULL;
    hil_mmb_t *mmb = NULL;

    unsigned long start;
    unsigned long region_len;

    unsigned long fixed_start = 0;
    unsigned long fixed_len = ~1;
    hil_mmz_t *fixed_mmz = NULL;

    mmz_trace_func();

    if ((size == 0) || (size > 0x40000000UL)) {
        return NULL;
    }

    if (align == 0) {
        align = MMZ_GRAIN;
    }

    size = mmz_grain_align(size);

    mmz_trace(1, "size=%luKB, align=%lu", size / SZ_1K, align);

    begin_list_for_each_mmz(mmz, gfp, mmz_name)

    if ((_user_mmz != NULL) && (_user_mmz != mmz)) {
        continue;
    }

    start = _find_fixed_region(&region_len, mmz, size, align);

    if ((fixed_len > region_len) && (start != 0)) {
        fixed_len = region_len;
        fixed_start = start;
        fixed_mmz = mmz;
    }

    end_list_for_each_mmz()

    if (fixed_mmz == NULL) {
        return NULL;
    }

    mmb = kmalloc(sizeof(hil_mmb_t), GFP_KERNEL);

    if (mmb == NULL) {
        return NULL;
    }

    memset(mmb, 0, sizeof(hil_mmb_t));
    mmb->zone = fixed_mmz;
    mmb->phys_addr = fixed_start;
    mmb->length = size;
    mmb->id = ++mmb_number;

    if (name != NULL) {
        strlcpy(mmb->name, name, HIL_MMB_NAME_LEN);
    } else {
        strncpy(mmb->name, "<null>", HIL_MMB_NAME_LEN);
    }

    if (_do_mmb_alloc(mmb)) {
        kfree(mmb);
        mmb = NULL;
    }

    return mmb;
}

hil_mmb_t *hil_mmb_alloc(const char *name, unsigned long size, unsigned long align,
                         unsigned long gfp, const char *mmz_name)
{
    hil_mmb_t *mmb = NULL;

    down(&mmz_lock);
    mmb = __mmb_alloc(name, size, align, gfp, mmz_name, NULL);
    up(&mmz_lock);

    return mmb;
}
EXPORT_SYMBOL(hil_mmb_alloc);

hil_mmb_t *hil_mmb_alloc_in(const char *name, unsigned long size, unsigned long align,
                            hil_mmz_t *_user_mmz)
{
    hil_mmb_t *mmb = NULL;

    if (_user_mmz == NULL) {
        return NULL;
    }

    down(&mmz_lock);
    mmb = __mmb_alloc(name, size, align, _user_mmz->gfp, _user_mmz->name, _user_mmz);
    up(&mmz_lock);

    return mmb;
}

static void *_mmb_map2kern(hil_mmb_t *mmb, int cached)
{
    if (mmb->flags & HIL_MMB_MAP2KERN) {
        if ((cached * HIL_MMB_MAP2KERN_CACHED) != (mmb->flags & HIL_MMB_MAP2KERN_CACHED)) {
            printk(KERN_ERR "mmb<%s> already kernel-mapped %s, can not be re-mapped as %s.",
                   mmb->name,
                   (mmb->flags & HIL_MMB_MAP2KERN_CACHED) ? "cached" : "non-cached",
                   (cached) ? "cached" : "non-cached");
            return NULL;
        }

        mmb->map_ref++;

        return mmb->kvirt;
    }

    if (cached) {
        mmb->flags |= HIL_MMB_MAP2KERN_CACHED;

        mmb->kvirt = osal_ioremap_cached(mmb->phys_addr, mmb->length);

    } else {
        mmb->flags &= ~HIL_MMB_MAP2KERN_CACHED;

        mmb->kvirt = osal_ioremap_nocache(mmb->phys_addr, mmb->length);
    }

    if (mmb->kvirt) {
        mmb->flags |= HIL_MMB_MAP2KERN;
        mmb->map_ref++;
    }

    return mmb->kvirt;
}

void *hil_mmb_map2kern(hil_mmb_t *mmb)
{
    void *p = NULL;

    if (mmb == NULL) {
        return NULL;
    }

    down(&mmz_lock);
    p = _mmb_map2kern(mmb, 0);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(hil_mmb_map2kern);

void *hil_mmb_map2kern_cached(hil_mmb_t *mmb)
{
    void *p = NULL;

    if (mmb == NULL) {
        return NULL;
    }

    down(&mmz_lock);
    p = _mmb_map2kern(mmb, 1);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(hil_mmb_map2kern_cached);

static void *__mmf_map(unsigned long phys, int len, int cache)
{
    void *virt = NULL;

    if (cache) {
        virt = osal_ioremap_cached(phys, len);

    } else {
        virt = osal_ioremap_nocache(phys, len);
    }

    return virt;
}

void *hil_mmf_map2kern_nocache(unsigned long phys, int len)
{
    void *p = NULL;

    down(&mmz_lock);
    p = __mmf_map(phys, len, 0);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(hil_mmb_map2kern);

void *hil_mmf_map2kern_cache(unsigned long phys, int len)
{
    void *p = NULL;

    down(&mmz_lock);
    p = __mmf_map(phys, len, 1);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(hil_mmb_map2kern_cached);

void hil_mmf_unmap(void *virt)
{
    if (virt != NULL) {
        iounmap(virt);
    }
}
EXPORT_SYMBOL(hil_mmb_unmap);

int hil_mmb_flush_dcache_byaddr(void *kvirt, unsigned long phys_addr, unsigned long length)
{
    if (kvirt == NULL) {
        return -EINVAL;
    }

    /* Use flush range to instead flush_cache_all, because flush_cache_all only flush
      local cpu. And on_each_cpu macro cannot used to flush all cpus with irq disabled. */

    /* dmac_map_area is invalid in  hi3518ev200 kernel, arm9 is not supported yet */
    dma_cache_clean(phys_addr, phys_addr + length);
    dma_cache_inv(phys_addr, phys_addr + length);

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    /* if length > L2 cache size, then this interface will call <outer_flush_all> */
    outer_flush_range(phys_addr, phys_addr + length);
#endif

    return 0;
}
EXPORT_SYMBOL(hil_mmb_flush_dcache_byaddr);

int hil_mmb_invalid_cache_byaddr(void *kvirt, unsigned long phys_addr, unsigned long length)
{
    if (kvirt == NULL) {
        return -EINVAL;
    }

    /* dmac_map_area is invalid in  hi3518ev200 kernel, arm9 is not supported yet */
    dma_cache_clean(phys_addr, phys_addr + length);
    dma_cache_inv(phys_addr, phys_addr + length);

    return 0;
}
EXPORT_SYMBOL(hil_mmb_invalid_cache_byaddr);

static int _mmb_free(hil_mmb_t *mmb);

int hil_mmb_unmap(hil_mmb_t *mmb)
{
    int ref;

    if (mmb == NULL) {
        return -1;
    }

    down(&mmz_lock);

    if (mmb->flags & HIL_MMB_MAP2KERN_CACHED) {
        dma_cache_clean(mmb->phys_addr, mmb->phys_addr + mmb->length);
        dma_cache_inv(mmb->phys_addr, mmb->phys_addr + mmb->length);
    }

    if (mmb->flags & HIL_MMB_MAP2KERN) {
        ref = --mmb->map_ref;

        if (mmb->map_ref != 0) {
            up(&mmz_lock);
            return ref;
        }

        iounmap(mmb->kvirt);
    }

    mmb->kvirt = NULL;
    mmb->flags &= ~HIL_MMB_MAP2KERN;
    mmb->flags &= ~HIL_MMB_MAP2KERN_CACHED;

    if ((mmb->flags & HIL_MMB_RELEASED) && (mmb->phy_ref == 0)) {
        _mmb_free(mmb);
    }

    up(&mmz_lock);

    return 0;
}
EXPORT_SYMBOL(hil_mmb_unmap);

int hil_mmb_get(hil_mmb_t *mmb)
{
    int ref;

    if (mmb == NULL) {
        return -1;
    }

    down(&mmz_lock);

    if (mmb->flags & HIL_MMB_RELEASED) {
        printk(KERN_WARNING "hil_mmb_get: amazing, mmb<%s> is released!\n", mmb->name);
    }

    ref = ++mmb->phy_ref;

    up(&mmz_lock);

    return ref;
}

static int _mmb_free(hil_mmb_t *mmb)
{
    if (mmb->flags & HIL_MMB_MAP2KERN_CACHED) {
        dma_cache_clean(mmb->phys_addr, mmb->phys_addr + mmb->length);
        dma_cache_inv(mmb->phys_addr, mmb->phys_addr + mmb->length);
    }

    osal_list_del(&mmb->list);
    kfree(mmb);

    return 0;
}

int hil_mmb_put(hil_mmb_t *mmb)
{
    int ref;

    if (mmb == NULL) {
        return -1;
    }

    down(&mmz_lock);

    if (mmb->flags & HIL_MMB_RELEASED) {
        printk(KERN_WARNING "hil_mmb_put: amazing, mmb<%s> is released!\n", mmb->name);
    }

    ref = --mmb->phy_ref;

    if ((mmb->flags & HIL_MMB_RELEASED) && (mmb->phy_ref == 0) && (mmb->map_ref == 0)) {
        _mmb_free(mmb);
    }

    up(&mmz_lock);

    return ref;
}

int hil_mmb_free(hil_mmb_t *mmb)
{
    mmz_trace_func();

    if (mmb == NULL) {
        return -1;
    }

    mmz_trace(1, HIL_MMB_FMT_S, hil_mmb_fmt_arg(mmb));
    down(&mmz_lock);

    if (mmb->flags & HIL_MMB_RELEASED) {
        printk(KERN_WARNING "hil_mmb_free: amazing, mmb<%s> is released before, but still used!\n", mmb->name);

        up(&mmz_lock);
        return 0;
    }

    if (mmb->phy_ref > 0) {
        printk(KERN_WARNING "hil_mmb_free: free mmb<%s> delayed for which ref-count is %d!\n",
               mmb->name, mmb->map_ref);
        mmb->flags |= HIL_MMB_RELEASED;
        up(&mmz_lock);
        return 0;
    }

    if (mmb->flags & HIL_MMB_MAP2KERN) {
        printk(KERN_WARNING "hil_mmb_free: free mmb<%s> delayed for which is kernel-mapped to 0x%p with map_ref %d!\n",
               mmb->name, mmb->kvirt, mmb->map_ref);
        mmb->flags |= HIL_MMB_RELEASED;
        up(&mmz_lock);
        return 0;
    }

    _mmb_free(mmb);
    up(&mmz_lock);
    return 0;
}
EXPORT_SYMBOL(hil_mmb_free);

#define MACH_MMB(p, val, member)                                                       \
    do {                                                                               \
        hil_mmz_t* __mach_mmb_zone__ = NULL;                                           \
        (p) = NULL;                                                                    \
        osal_list_for_each_entry(__mach_mmb_zone__, &mmz_list, list)                   \
        {                                                                              \
            hil_mmb_t* __mach_mmb__ = NULL;                                            \
            osal_list_for_each_entry(__mach_mmb__, &__mach_mmb_zone__->mmb_list, list) \
            {                                                                          \
                if (__mach_mmb__->member == (val)) {                                   \
                    (p) = __mach_mmb__;                                                \
                    break;                                                             \
                }                                                                      \
            }                                                                          \
            if (p != NULL)                                                                     \
                break;                                                                 \
        }                                                                              \
    } while (0)

hil_mmb_t *hil_mmb_getby_phys(unsigned long addr)
{
    hil_mmb_t *p = NULL;
    down(&mmz_lock);
    MACH_MMB(p, addr, phys_addr);
    up(&mmz_lock);
    return p;
}
EXPORT_SYMBOL(hil_mmb_getby_phys);

#define MACH_MMB_2(p, val, member, Outoffset)                                                                                                     \
    do {                                                                                                                                          \
        hil_mmz_t* __mach_mmb_zone__ = NULL;                                                                                                      \
        (p) = NULL;                                                                                                                               \
        list_for_each_entry(__mach_mmb_zone__, &mmz_list, list)                                                                                   \
        {                                                                                                                                         \
            hil_mmb_t* __mach_mmb__ = NULL;                                                                                                       \
            list_for_each_entry(__mach_mmb__, &__mach_mmb_zone__->mmb_list, list)                                                                 \
            {                                                                                                                                     \
                if ((__mach_mmb__->member <= (val)) && ((__mach_mmb__->length + (unsigned char*)__mach_mmb__->member) > (unsigned char*)(uintptr_t)(val))) { \
                    (p) = __mach_mmb__;                                                                                                           \
                    Outoffset = (unsigned char*)(uintptr_t)val - (unsigned char*)(__mach_mmb__->member);                                                     \
                    break;                                                                                                                        \
                }                                                                                                                                 \
            }                                                                                                                                     \
            if (p != NULL)                                                                                                                                \
                break;                                                                                                                            \
        }                                                                                                                                         \
    } while (0)

hil_mmb_t *hil_mmb_getby_kvirt(void *virt)
{
    hil_mmb_t *p = NULL;
    unsigned long Outoffset;

    if (virt == NULL) {
        return NULL;
    }

    down(&mmz_lock);
    MACH_MMB_2(p, virt, kvirt, Outoffset);
    up(&mmz_lock);

    mmz_trace(1, "Outoffset %lu \n", Outoffset);

    return p;
}
EXPORT_SYMBOL(hil_mmb_getby_kvirt);

hil_mmb_t *hil_mmb_getby_phys_2(unsigned long addr, unsigned long *Outoffset)
{
    hil_mmb_t *p = NULL;

    down(&mmz_lock);
    MACH_MMB_2(p, addr, phys_addr, *Outoffset);
    up(&mmz_lock);
    return p;
}

hil_mmz_t *hil_mmz_find(unsigned long gfp, const char *mmz_name)
{
    hil_mmz_t *p = NULL;

    down(&mmz_lock);
    begin_list_for_each_mmz(p, gfp, mmz_name)
    up(&mmz_lock);
    return p;
    end_list_for_each_mmz()
    up(&mmz_lock);

    return NULL;
}

unsigned long hil_mmz_get_phys(const char *zone_name)
{
    hil_mmz_t *zone = NULL;

    zone = hil_mmz_find(0, zone_name);
    if (zone != NULL) {
        return zone->phys_start;
    }

    return 0;
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/*
 * name,gfp,phys_start,nbytes,alloc_type;...
 * All param in hex mode, except name.
 */
static int media_mem_parse_cmdline(char *s)
{
    hil_mmz_t *zone = NULL;
    char *line = NULL;
    unsigned long phys_end = 0;

    while ((line = strsep(&s, ":")) != NULL) {
        int i;
        char *argv[6];

        /*
         * FIXME: We got 4 args in "line", formated "argv[0],argv[1],argv[2],argv[3],argv[4]".
         * eg: "<mmz_name>,<gfp>,<phys_start_addr>,<size>,<alloc_type>"
         * For more convenient, "hard code" are used such as "arg[0]", i.e.
         */
        for (i = 0; (argv[i] = strsep(&line, ",")) != NULL;)
            if (++i == ARRAY_SIZE(argv)) {
                break;
            }

        if (i == 4) {
            zone = hil_mmz_create("null", 0, 0, 0);

            if (zone == NULL) {
                continue;
            }

            strlcpy(zone->name, argv[0], HIL_MMZ_NAME_LEN);
            zone->gfp = _strtoul_ex(argv[1], NULL, 0);
            zone->phys_start = _strtoul_ex(argv[2], NULL, 0);
            zone->nbytes = _strtoul_ex(argv[3], NULL, 0);
        } else if (i == 6) {
            zone = hil_mmz_create_v2("null", 0, 0, 0, 0, 0);

            if (zone == NULL) {
                continue;
            }

            strlcpy(zone->name, argv[0], HIL_MMZ_NAME_LEN);
            zone->gfp = _strtoul_ex(argv[1], NULL, 0);
            zone->phys_start = _strtoul_ex(argv[2], NULL, 0);
            zone->nbytes = _strtoul_ex(argv[3], NULL, 0);
            zone->alloc_type = _strtoul_ex(argv[4], NULL, 0);
            zone->block_align = _strtoul_ex(argv[5], NULL, 0);
        } else {
            printk(KERN_ERR "MMZ: your parameter num is not correct!\n");
            continue;
        }

        mmz_info_phys_start = zone->phys_start + zone->nbytes - 0x2000;

        if (hil_mmz_register(zone)) {
            printk(KERN_WARNING "Add MMZ failed: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(zone));
            hil_mmz_destroy(zone);
            return -1;
        }

        // if phys_end is 0xFFFFFFFF (32bit)
        phys_end = (zone->phys_start + zone->nbytes);

        if ((phys_end == 0) && (zone->nbytes >= PAGE_SIZE)) {
            // reserve last PAGE_SIZE memory
            zone->nbytes = zone->nbytes - PAGE_SIZE;
        }

        // if phys_end exceed 0xFFFFFFFF (32bit), wraping error
        if ((zone->phys_start > phys_end) && (phys_end != 0)) {
            printk(KERN_ERR "MMZ: parameter is not correct! Address exceeds 0xFFFFFFFF\n");
            hil_mmz_unregister(zone);
            hil_mmz_destroy(zone);
            return -1;
        }

        zone = NULL;
    }

    return 0;
}

static int _check_map_mmz(hil_mmz_t *zone)
{
    hil_mmz_t *p = NULL;

    unsigned long new_start = zone->phys_start;
    unsigned long new_end = zone->phys_start + zone->nbytes;

    if (zone->nbytes == 0) {
        return -1;
    }

    osal_list_for_each_entry(p, &map_mmz_list, list) {
        unsigned long start, end;
        start = p->phys_start;
        end = p->phys_start + p->nbytes;

        if (new_start >= end) {
            continue;
        } else if ((new_start < start) && (new_end <= start)) {
            continue;
        } else
            ;

        printk(KERN_ERR "ERROR: Conflict MMZ:\n");
        printk(KERN_ERR "MMZ new:   " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(zone));
        printk(KERN_ERR "MMZ exist: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(p));
        printk(KERN_ERR "Add new MMZ failed!\n");
        return -1;
    }

    return 0;
}

int hil_map_mmz_register(hil_mmz_t *zone)
{
    int ret = 0;

    mmz_trace(1, HIL_MMZ_FMT_S, hil_mmz_fmt_arg(zone));

    if (zone == NULL) {
        return -1;
    }

    down(&mmz_lock);

    ret = _check_mmz(zone);
    if (ret) {
        up(&mmz_lock);
        return ret;
    }

    ret = _check_map_mmz(zone);
    if (ret) {
        up(&mmz_lock);
        return ret;
    }

    OSAL_INIT_LIST_HEAD(&zone->mmb_list);

    osal_list_add(&zone->list, &map_mmz_list);

    up(&mmz_lock);

    return 0;
}

int hil_map_mmz_unregister(hil_mmz_t *zone)
{
    int losts = 0;
    hil_mmb_t *p = NULL;

    if (zone == NULL) {
        return -1;
    }

    mmz_trace_func();

    down(&mmz_lock);
    osal_list_for_each_entry(p, &zone->mmb_list, list) {
        printk(KERN_WARNING "MB Lost: " HIL_MMB_FMT_S "\n",
               hil_mmb_fmt_arg(p));
        losts++;
    }

    if (losts) {
        printk(KERN_ERR "%d mmbs not free, mmz<%s> can not be unregistered!\n",
               losts, zone->name);
        up(&mmz_lock);
        return -1;
    }

    osal_list_del(&zone->list);
    up(&mmz_lock);

    return 0;
}

static int map_mmz_init(char *s)
{
    hil_mmz_t *zone = NULL;
    char *line = NULL;

    if (s[0] == '\0') {
        return 0;
    }

    while ((line = strsep(&s, ":")) != NULL) {
        int i;
        char *argv[2];

        for (i = 0; (argv[i] = strsep(&line, ",")) != NULL;)
            if (++i == ARRAY_SIZE(argv)) {
                break;
            }

        if (i == 2) {
            zone = hil_mmz_create("null", 0, 0, 0);
            if (zone == NULL) {
                continue;
            }
            zone->phys_start = _strtoul_ex(argv[0], NULL, 0);
            zone->nbytes = _strtoul_ex(argv[1], NULL, 0);
        } else {
            printk(KERN_ERR "error parameters\n");
            return -EINVAL;
        }

        if (hil_map_mmz_register(zone)) {
            printk(KERN_WARNING "Add MMZ failed: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(zone));
            hil_mmz_destroy(zone);
        }
        zone = NULL;
    }

    return 0;
}

static void map_mmz_exit(void)
{
    hil_mmz_t *pmmz = NULL;
    struct osal_list_head *p = NULL, *n = NULL;

    mmz_trace_func();

    list_for_each_safe(p, n, &map_mmz_list) {
        pmmz = list_entry(p, hil_mmz_t, list);
        printk(KERN_WARNING "MMZ force removed: " HIL_MMZ_FMT_S "\n",
               hil_mmz_fmt_arg(pmmz));
        hil_map_mmz_unregister(pmmz);
        hil_mmz_destroy(pmmz);
    }
}

int hil_map_mmz_check_phys(unsigned long addr_start, unsigned long addr_len)
{
    hil_mmz_t *p = NULL;
    unsigned long addr_end = addr_start + addr_len;
    unsigned long temp_start, temp_end;

    osal_list_for_each_entry(p, &map_mmz_list, list) {
        temp_start = p->phys_start;
        temp_end = p->phys_start + p->nbytes;
        if ((addr_start >= temp_start) && (addr_end <= temp_end)) {
            return 0;
        }
    }

    printk(KERN_ERR "ERROR: MMAP ADDR: 0x%lx-0x%lx\n", addr_start, addr_end);

    return -1;
}

int hil_vma_check(unsigned long vm_start, unsigned long vm_end)
{
    return 0;
}

int hil_is_phys_in_mmz(unsigned long addr_start, unsigned long addr_len)
{
    hil_mmz_t *p = NULL;
    unsigned long addr_end = addr_start + addr_len;
    unsigned long temp_start, temp_end;

    osal_list_for_each_entry(p, &mmz_list, list) {
        temp_start = p->phys_start;
        temp_end = p->phys_start + p->nbytes;
        if ((addr_start >= temp_start) && (addr_end <= temp_end)) {
            return 0;
        }
    }
    // printk(KERN_ERR "ERROR: MMAP ADDR: 0x%lx-0x%lx\n", addr_start, addr_end);

    return -1;
}

int hil_mmb_flush_dcache_byaddr_safe(void *kvirt, unsigned long phys_addr, unsigned long length)
{
    return hil_mmb_flush_dcache_byaddr(kvirt, phys_addr, length);
}

#define MEDIA_MEM_NAME "media-mem"

#define CONFIG_PROC_FS

#ifdef CONFIG_PROC_FS

int get_mmz_info_phys_start(void)
{
    return mmz_info_phys_start;
}

int mmz_read_proc(struct osal_proc_dir_entry *entry)
{
    hil_mmz_t *p = NULL;
    int len = 0;
    unsigned int zone_number = 0;
    unsigned int block_number = 0;
    unsigned int used_size = 0;
    unsigned int free_size = 0;
    unsigned int mmz_total_size = 0;

    mmz_trace_func();

    down(&mmz_lock);
    osal_list_for_each_entry(p, &mmz_list, list) {
        hil_mmb_t *mmb = NULL;
        osal_seq_printf(entry, "+---ZONE: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(p));
        mmz_total_size += p->nbytes / 1024;
        ++zone_number;

        osal_list_for_each_entry(mmb, &p->mmb_list, list) {
            osal_seq_printf(entry, "   |-MMB: " HIL_MMB_FMT_S "\n", hil_mmb_fmt_arg(mmb));
            used_size += mmb->length / 1024;
            ++block_number;
        }
    }

    if (mmz_total_size != 0) {
        free_size = mmz_total_size - used_size;
        osal_seq_printf(entry, "\n---MMZ_USE_INFO:\n total size=%dKB(%dMB),"
                        "used=%dKB(%dMB + %dKB),remain=%dKB(%dMB + %dKB),"
                        "zone_number=%d,block_number=%d\n",
                        mmz_total_size, mmz_total_size / 1024,
                        used_size, used_size / 1024, used_size % 1024,
                        free_size, free_size / 1024, free_size % 1024,
                        zone_number, block_number);
    }

    up(&mmz_lock);

    return len;
}

#define MMZ_PROC_ROOT  NULL
static int __init media_mem_proc_init(void)
{
    osal_proc_entry_t *p = NULL;

    p = osal_create_proc_entry(MEDIA_MEM_NAME, MMZ_PROC_ROOT);

    if (p == NULL) {
        return -1;
    }

    p->read = mmz_read_proc;

    return 0;
}

static void __exit media_mem_proc_exit(void)
{
    osal_remove_proc_entry(MEDIA_MEM_NAME, MMZ_PROC_ROOT);
}

#else
static int __init media_mem_proc_init(void)
{
    return 0;
}
static void __exit media_mem_proc_exit(void) {}

#endif /* CONFIG_PROC_FS */

extern int __init mmz_userdev_init(void);

void mmz_exit_check(void)
{
    hil_mmz_t *pmmz = NULL;
    struct osal_list_head *p = NULL, *n = NULL;

    mmz_trace_func();

    osal_list_for_each_safe(p, n, &mmz_list) {
        pmmz = osal_list_entry(p, hil_mmz_t, list);
        printk(KERN_WARNING "MMZ force removed: " HIL_MMZ_FMT_S "\n", hil_mmz_fmt_arg(pmmz));
        hil_mmz_unregister(pmmz);
        hil_mmz_destroy(pmmz);
    }
}

int media_mem_init(void *pArgs)
{
    MMZ_MODULE_PARAMS_S *pstMmzModuleParam = (MMZ_MODULE_PARAMS_S *)pArgs;
    int ret = 0;

    printk(KERN_INFO "Hisilicon Media Memory Zone Manager\n");

    if (pstMmzModuleParam != NULL) {
        anony = pstMmzModuleParam->anony;

        if (anony != 1) {
            printk("The module param \"anony\" should only be 1 which is %d \n", anony);
            return -1;
        }

        ret = media_mem_parse_cmdline(pstMmzModuleParam->mmz);
        if (ret != 0) {
            mmz_exit_check();
            return ret;
        }

        ret = map_mmz_init(pstMmzModuleParam->map_mmz);
        if (ret != 0) {
            mmz_exit_check();
            return ret;
        }
    }

    media_mem_proc_init();

    // kcom_mmz_register();

    mmz_userdev_init();

    return 0;
}

extern void __exit mmz_userdev_exit(void);

void media_mem_exit(void)
{
    mmz_userdev_exit();

    // kcom_mmz_unregister();
    map_mmz_exit();

    mmz_exit_check();

    media_mem_proc_exit();
}


