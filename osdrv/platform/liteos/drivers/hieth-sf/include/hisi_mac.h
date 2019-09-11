#ifndef    __HISOC_HISI_MAC_H__
#define    __HISOC_HISI_MAC_H__

#include "los_base.h"
#include "asm/platform.h"
#include "hieth.h"
#include "eth_phy.h"
#include "eth_drv.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PHY_ID_RTL8201  0x001cc816

#define LEN_MAGIC 0x5a5a

//#define isb() __asm__ __volatile__ ("isb" : : : "memory")
//#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
//#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define isb()
#define dsb()
#define dmb()

struct pbuf_info {
	struct pbuf_dma_info *dma_info[MAX_ETH_DRV_SG];
	u32 sg_len;
};

void net_dma_cache_inv(void *addr, unsigned int size);
void net_dma_cache_clean(void *addr, unsigned int size);

typedef struct eth_ram_cfg_st
{
    struct tx_pkt_info *txq_info;
    struct pbuf **rx_pbuf;
    struct dma_tx_desc *dma_tx;
    struct pbuf_info *pbuf_info;
} eth_ram_cfg_t;

typedef struct hieth_priv_s
{
   UINT32        intr_vector;
   UINT8        *enaddr;
   UINT32        base;      // Base address of device
   eth_phy_access_t    *phy;
   UINT32        total_len;
   UINT8        iterator;
   volatile        eth_ram_cfg_t *ram;
    unsigned int rx_feed;
    unsigned int rx_release;
#ifdef INT_IO_ETH_INT_SUPPORT_REQUIRED
   interrupt        intr;
   handle_t        intr_handle;
#endif
    struct timer_list phy_timer;

    UINT32        index;         //dev id
} hieth_priv_t;

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

