
// Ethernet driver structure

#ifndef _ETH_DRV_H_
#define _ETH_DRV_H_

#define NETPKG_NET_LWIP
#ifdef NETPKG_NET_LWIP
#include "lwip/netif.h"
#include "los_mac.h"
#include "lwip/tcp_impl.h"
#endif

#define PHY_STATE_TIME    1000
#define NETNUM_IO_ETH_DRIVERS_SG_LIST_SIZE  18
#define MAX_ETH_DRV_SG NETNUM_IO_ETH_DRIVERS_SG_LIST_SIZE
#define MAX_ETH_MSG 1540

struct eth_drv_sg{
    UINT32  buf;
    UINT32 len;
};

struct eth_drv_sc{
    struct eth_hwr_funs *funs;
    void                *driver_private;
    const char          *dev_name;
    int                  state;
};

typedef void (*eth_start)(struct los_eth_driver *sc, unsigned char *enaddr, int flags);
typedef void (*eth_stop)(struct los_eth_driver *sc);
typedef int (*eth_control)(struct los_eth_driver *sc, unsigned long key, void *data, int data_length);
typedef int (*eth_can_send)(struct los_eth_driver *sc);
typedef void (*eth_send)(struct los_eth_driver *sc, struct pbuf *p);
typedef void (*eth_recv)(struct los_eth_driver *sc, struct pbuf *pbuf);
typedef void (*eth_deliver)(struct los_eth_driver *sc);
typedef void (*eth_poll)(struct los_eth_driver *sc);
typedef int (*eth_int_vector)(struct los_eth_driver *sc);

struct eth_hwr_funs {
    // Initialize hardware (including startup)
    eth_start start;
    // Shut down hardware
    eth_stop stop;
    // Device control (ioctl pass-thru)
    eth_control control;
    // Query - can a packet be sent?
    eth_can_send can_send;
    // Send a packet of data
    eth_send send;
    // Receive [unload] a packet of data
    eth_recv recv;
    // Deliver data to/from device from/to stack memory space
    // (moves lots of memcpy()s out of DSRs into thread)
    eth_deliver deliver;
    // Poll for interrupts/device service
    eth_poll poll;
    // Get interrupt information from hardware driver
    eth_int_vector int_vector;
    // Logical driver interface
    struct los_eth_funs *eth_drv, *eth_drv_old;
};

//void higmac_link_status_changed(struct netif *netif);
void hieth_link_status_changed(struct netif *netif);
void phy_state_machine(unsigned long arg);
u8_t hieth_set_hwaddr(struct netif *netif, u8_t *addr, u8_t len);
void hieth_config_mode(struct netif *netif, uint32_t config_flags, u8_t setBit);

int hisi_eth_set_phy_mode(const char *phy_mode);
int hisi_eth_set_phy_addr(unsigned int phy_addr);

// Control 'key's
#define ETH_DRV_SET_MAC_ADDRESS 0x0100

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#endif // _ETH_DRV_H_
