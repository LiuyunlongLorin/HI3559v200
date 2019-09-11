#ifndef _COMPAT_LINUX_SCATTERLIST_H
#define _COMPAT_LINUX_SCATTERLIST_H

#include <linux/string.h>
#include <linux/kernel.h>
#include "asm/bug.h"
#include "los_printf.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef unsigned long dma_addr_t;
typedef unsigned long ulong;
typedef unsigned int uint;

typedef struct scatterlist {
#ifdef CONFIG_DEBUG_SG
    ulong   sg_magic;
#endif
    ulong   page_link;
    uint    offset;
    uint    length;
    dma_addr_t      dma_address;
#ifdef CONFIG_NEED_SG_DMA_LENGTH
    uint    dma_length;
#endif
}scatterlist_t;

#define COMPAT_SG_MAGIC    0x87654321

/*
 * sg_mark_end - Mark the end of the scatterlist
 * Param:
 *    psg---SG entryScatterlist
 *
 * Description:
 *   Marks the passed in sg entry as the termination point for the psg table. A call to sg_next() on this entry will return NULL.
 *
 */
static inline void sg_mark_end(scatterlist_t *psg)
{
#ifdef CONFIG_DEBUG_SG
    BUG_ON(psg->sg_magic != COMPAT_SG_MAGIC);
#endif
    /* Set termination bit, clear potential chain bit*/
    psg->page_link |= 0x02;
    psg->page_link &= ~0x01;
}

static inline void sg_init_table(scatterlist_t *psgl, unsigned int nents)
{
    memset(psgl, 0, sizeof(*psgl) * nents);

    sg_mark_end(&psgl[nents - 1]);
}

static inline void sg_set_buf(scatterlist_t *psg, const void *buf,
        unsigned int buflen)
{
    psg->dma_address =(unsigned long) buf;
    psg->offset = 0;
    psg->length = buflen;
}

static inline void sg_init_one(scatterlist_t *psg, const void *buf, unsigned int buflen)
{
    sg_init_table(psg, 1);
    sg_set_buf(psg, buf, buflen);
}

#define SG_MAGIC    COMPAT_SG_MAGIC
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _COMPAT_LINUX_SCATTERLIST_H */
