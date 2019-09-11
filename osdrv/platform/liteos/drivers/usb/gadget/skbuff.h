
#ifndef _SKBUFF_H
#define _SKBUFF_H

#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>

#define USB_CHECKSUM_NONE   0
#define USB_CHECKSUM_UNNECESSARY    1
#define USB_CHECKSUM_COMPLETE   2
#define USB_CHECKSUM_PARTIAL    3

typedef unsigned int gfp_t;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

/* Maximum value in skb->csum_level */
#define SKB_MAX_CSUM_LEVEL  3
#define SKB_TRUESIZE(N) ((N) +                        \
             SKB_DATA_ALIGN(sizeof(struct sk_buff)))

typedef u8 * sk_buff_data_t;

typedef struct sk_buff {

    struct sk_buff *next;
    struct sk_buff *prev;

    u32 len, data_len;
    u16 mac_len, hdr_len;

    sk_buff_data_t tail;
    sk_buff_data_t end;
    u8 *head, *data;
    u32 truesize;
    atomic_t users;
}SK_BUFF_TYPE;

typedef struct sk_buff_head {
    SK_BUFF_TYPE  *next;
    SK_BUFF_TYPE  *prev;

    u32   qlen;
    spinlock_t  lock;
}SK_BUFF_HEAD_TYPE;

#define SKB_ALLOC_FCLONE 0x01

void usb_kfree_skb(struct sk_buff *skb);
void usb_kfree_skb_list(struct sk_buff *segs);
void usb_consume_skb(struct sk_buff *skb);

struct sk_buff *usb_alloc_skb(unsigned int size, gfp_t priority, int flags,
    int node);
static inline struct sk_buff *alloc_skb(u32 size, gfp_t priority)
{
    return usb_alloc_skb(size, priority, 0, -1);
}

static inline void __skb_insert(SK_BUFF_TYPE *pnewsk, SK_BUFF_TYPE *pprev, SK_BUFF_TYPE *pnext,
    SK_BUFF_HEAD_TYPE *plist)
{
    pnewsk->next = pnext;
    pnewsk->prev = pprev;
    pnext->prev  = pprev->next = pnewsk;
    plist->qlen++;
}

struct sk_buff *usb_skb_clone(struct sk_buff *skb, gfp_t priority);
int usb_pskb_expand_head(struct sk_buff *skb, int nhead, int ntail, gfp_t gfp_mask);
SK_BUFF_TYPE *usb_skb_realloc_headroom(SK_BUFF_TYPE *skb, u32 headroom);

#define usb_dev_kfree_skb(a) usb_consume_skb(a)
#define usb_dev_kfree_skb_any(a) usb_dev_kfree_skb(a)

static inline u8 *skb_end_pointer(const SK_BUFF_TYPE *pskb)
{
    return pskb->end;
}

static inline SK_BUFF_TYPE *skb_peek(const SK_BUFF_HEAD_TYPE *plist)
{
    struct sk_buff *pskb = plist->next;

    if (pskb == (struct sk_buff *)plist)
        pskb = NULL;
    return pskb;
}

static inline void __skb_unlink(SK_BUFF_TYPE *pskb, SK_BUFF_HEAD_TYPE *plist)
{
    SK_BUFF_TYPE *pnext, *pprev;

    plist->qlen--;
    pnext = pskb->next;
    pprev = pskb->prev;
    pskb->next  = pskb->prev = NULL;
    pnext->prev = pprev;
    pprev->next = pnext;
}

static inline u32 skb_end_offset(const SK_BUFF_TYPE *pskb)
{
    return pskb->end - pskb->head;
}

static inline void __skb_queue_before(SK_BUFF_HEAD_TYPE *plist,
    SK_BUFF_TYPE *pnext, SK_BUFF_TYPE *pnewsk)
{
    __skb_insert(pnewsk, pnext->prev, pnext, plist);
}

static inline u8 *skb_tail_pointer(const SK_BUFF_TYPE *pskb)
{
    return pskb->tail;
}

static inline void __skb_queue_tail(SK_BUFF_HEAD_TYPE *plist,
    SK_BUFF_TYPE *pnewsk)
{
    __skb_queue_before(plist, (struct sk_buff *)plist, pnewsk);
}

static inline void skb_set_tail_pointer(SK_BUFF_TYPE *pskb, const int offset)
{
    pskb->tail = pskb->data + offset;
}

static inline void skb_reset_tail_pointer(SK_BUFF_TYPE*pskb)
{
    pskb->tail = pskb->data;
}

static inline u8 *__skb_push(SK_BUFF_TYPE *pskb, u32 len)
{
    pskb->data -= len;
    pskb->len  += len;
    return pskb->data;
}

u8 *usb_skb_pull(SK_BUFF_TYPE *skb, u32 len);
static inline u8 *__skb_pull(SK_BUFF_TYPE *pskb, u32 len)
{
    pskb->len -= len;
    return pskb->data += len;
}

static inline u8 *skb_pull_inline(SK_BUFF_TYPE *pskb, u32 len)
{
    return unlikely(len > pskb->len) ? NULL : __skb_pull(pskb, len);
}

static inline u32 skb_headroom(const SK_BUFF_TYPE *pskb)
{
    return pskb->data - pskb->head;
}

static inline void skb_reserve(SK_BUFF_TYPE *pskb, int len)
{
    pskb->data += len;
    pskb->tail += len;
}

static inline void __skb_queue_head_init(SK_BUFF_HEAD_TYPE *plist)
{
    plist->prev = plist->next = (struct sk_buff *)plist;
    plist->qlen = 0;
}

static inline void skb_queue_head_init(SK_BUFF_HEAD_TYPE *plist)
{
    spin_lock_init(&plist->lock);
    __skb_queue_head_init(plist);
}


#ifndef NET_IP_ALIGN
#define USB_NET_IP_ALIGN    2
#define NET_IP_ALIGN  USB_NET_IP_ALIGN
#endif

static inline bool skb_is_nonlinear(const SK_BUFF_TYPE *pskb)
{
    return pskb->data_len;
}

static inline void __skb_trim(SK_BUFF_TYPE *pskb, u32 len)
{
    if (unlikely(skb_is_nonlinear(pskb))) {
        WARN_ON(1);
        PRINT_ERR("error!\n");
        return;
    }
    pskb->len = len;
    skb_set_tail_pointer(pskb, len);
}

void usb_skb_trim(struct sk_buff *skb, unsigned int len);
unsigned char *usb_skb_put(struct sk_buff *skb, unsigned int len);
struct sk_buff *usb_skb_dequeue(struct sk_buff_head *list);
unsigned char *usb_skb_push(struct sk_buff *skb, unsigned int len);
void usb_skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);

#endif  /* _SKBUFF_H */
