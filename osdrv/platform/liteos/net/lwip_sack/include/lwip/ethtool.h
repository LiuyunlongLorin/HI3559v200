/*
 *********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 *********************************************************************************
 */

#ifndef __ETHTOOL_H
#define __ETHTOOL_H


#include "lwip/opt.h"

#if LWIP_NETIF_ETHTOOL/* don't build if not configured for use in lwipopts.h */
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "arch/cc.h"
#include <netif/ifaddrs.h>
#include <liteos/ethtool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct ethtool_ops - optional netdev operations
 * @get_link: Report whether physical link is up.  Will only be called if
 *  the netdev is up.  Should usually be set to ethtool_op_get_link(),
 *  which uses netif_carrier_ok().
 * @begin: Function to be called before any other operation.  Returns a
 *  negative error code or zero.
 * @complete: Function to be called after any other operation except
 *  @begin.  Will be called even if the other operation failed.
 *
 * All operations are optional (i.e. the function pointer may be set
 * to %NULL) and callers must take this into account.  Callers must
 * hold the RTNL lock.
 *
 * See the structures used by these operations for further documentation.
 *
 * See &struct net_device and &struct net_device_ops for documentation
 * of the generic netdev features interface.
 */
struct ethtool_ops {
  u32_t (*get_link)(struct netif *netif);
  int (*begin)(struct netif *netif);
  void (*complete)(struct netif *netif);
};

s32_t dev_ethtool(struct netif *netif, struct ifreq *ifr);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_NETIF_ETHTOOL */

#endif /* __ETHTOOL_H */

