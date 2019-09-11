#ifndef    __GMAC_H__
#define    __GMAC_H__

#include "los_base.h"
#include "asm/platform.h"
#include "higmac.h"
#include "eth_phy.h"
#include "linux/delay.h"
#include "eth_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define         HIGMAC_MDIO_IO_BASE             GSF_REG_BASE
#define         HIGMAC_MDIO_IO_SIZE             (0x1000)
#define         HIGMAC_MDIO_FRQDIV              (0)

#define         REG_MDIO_SINGLE_CMD             0x000003C0
#define         REG_MDIO_SINGLE_DATA            0x000003C4
#define         REG_MDIO_RDATA_STATUS           0x000003D0

#define PHY_ID_RTL8201  0x001cc816

#define isb()   __asm__ __volatile__ ("isb" : : : "memory")
#ifndef LOSCFG_ARCH_CORTEX_A53_AARCH64
#define dsb()   __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb()   __asm__ __volatile__ ("dmb" : : : "memory")
#else
#define dsb(opt)   __asm__ __volatile("dsb " #opt : : : "memory")
#define dmb(opt)   __asm__ __volatile__ ("dmb ishst" ::: "memory")
#endif

#define SKB_DATA_ALIGN(X)       ALIGN(X, CACHE_ALIGNED_SIZE)
#define NET_SKB_PAD             CACHE_ALIGNED_SIZE

#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN    2
#endif
struct los_sk_buff {
        unsigned int len;
        unsigned char           *head,
                                *data;
        unsigned int truesize;
};

struct los_sk_buff *los_alloc_skb(unsigned int length);
struct los_sk_buff *los_alloc_skb_ip_align(unsigned int length);

void los_free_skb(struct los_sk_buff *skb);

void net_dma_cache_inv(void *addr, unsigned int size);
void net_dma_cache_clean(void *addr, unsigned int size);

typedef struct higmac_priv_s
{
   UINT32        intr_vector;
   UINT8        *enaddr;
   UINT32        base;      // Base address of device
   eth_phy_access_t    *phy;
   UINT32        total_len;
   UINT8        iterator;
//   BOOL        tx_busy;
#ifdef INT_IO_ETH_INT_SUPPORT_REQUIRED
   interrupt        intr;
   handle_t        intr_handle;
#endif
    struct timer_list phy_timer;

    UINT32        index;         //dev id

} higmac_priv_t;

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline int is_multicast_ether_addr(const unsigned char *addr)
{
        return 0x01 & addr[0];
}

/**
 * eth_random_addr - Generate software assigned random Ethernet address
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Generate a random Ethernet address (MAC) that is not multicast
 * and has the local assigned bit set.
 */
static inline void eth_random_addr(unsigned char *addr)
{
        int rand_val;
    unsigned long long nowclocks;

    msleep(200);
    nowclocks = hi_sched_clock() & 0xffffffff;
        srand((unsigned)nowclocks);
        rand_val = rand();
        addr[0] = rand_val & 0xff;
        addr[1] = (rand_val >> 8) & 0xff;
        addr[2] = (rand_val >> 16) & 0xff;
        addr[3] = (rand_val >> 24) & 0xff;

    msleep(200);
    nowclocks = hi_sched_clock() & 0xffffffff;
        srand((unsigned)nowclocks);
        rand_val = rand();
        addr[4] = rand_val & 0xff;
        addr[5] = (rand_val >> 8) & 0xff;

    addr[0] &= 0xfe;        /* clear multicast bit */
        addr[0] |= 0x02;        /* set local assignment bit (IEEE802) */
}

//extern void los_net_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

