/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

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


 /**
 *   @file netif.h
 *
 */

#ifndef __LWIP_NETIF_H__
#define __LWIP_NETIF_H__

#include "lwip/opt.h"

#define ENABLE_LOOPBACK (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)

#include "lwip/err.h"

#include "lwip/ip_addr.h"

#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "netif/etharp.h"
#include "netif/driverif.h"
#if LWIP_NETIF_PROMISC
#include "lwip/sys.h"
#endif
#include "asm/atomic.h"

#if LWIP_DHCP
struct dhcp;
#endif
#if LWIP_AUTOIP
struct autoip;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IFNAMSIZ
/* Length of interface name.  */
#define IFNAMSIZ  16
#endif

#define ETHARP_HWADDR_LEN 6
/* Throughout this file, IP_add are expected to be in
 * the same byte order as in IP_PCB. */

/** The maximum of all used hardware address lengths
    across all types of interfaces in use. */
#define NETIF_MAX_HWADDR_LEN 6U

/** Max length for buffer store the hostname string */
#define NETIF_HOSTNAME_MAX_LEN 32U

/** Whether the network interface is 'up'. This is
 * a software flag used to control whether this network
 * interface is enabled and processes traffic.
 * It is set by the startup code (for static IP configuration) or
 * by dhcp/autoip when an address has been assigned.
 */
#define NETIF_FLAG_UP           0x01U
/** If set, the netif has broadcast capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_BROADCAST    0x02U
/** If set, the netif is one end of a point-to-point connection.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_POINTTOPOINT 0x04U
/** If set, the interface is configured using DHCP.
 * Set by the DHCP code when starting or stopping DHCP. */
#define NETIF_FLAG_DHCP         0x08U
/** If set, the interface has an active link
 *  (set by the network interface driver).
 * Either set by the netif driver in its init function (if the link
 * is up at that time) or at a later point when the link comes up
 * (if link detection is supported by the hardware). */
#define NETIF_FLAG_LINK_UP      0x10U
/** If set, the netif is an ethernet device using ARP.
 * Set by the netif driver in its init function.
 * Used to check input packet types and use of DHCP. */
#define NETIF_FLAG_ETHARP       0x20U
/** If set, the netif is an ethernet device. It might not use
 * ARP or TCP/IP if it is used for PPPoE only.
 */
#define NETIF_FLAG_ETHERNET     0x40U
/** If set, the netif has IGMP capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IGMP         0x80U
#if DRIVER_STATUS_CHECK
/** If set, the netif has send capability.
 * Set by the netif driver when its is ready to send. */
#define NETIF_FLAG_DRIVER_RDY   0x100U
#endif
#if LWIP_TX_CSUM_OFFLOAD
/* If set, the netif has checksum offload capability, Set by the netif driver */
#define NETIF_FLAG_DRIVER_CSUM_SUPPORT   0x200U
#endif
#if LWIP_NETIF_PROMISC

#define NETIF_FLAG_PROMISC 0x01U
#define NETIF_FLAG_PROMISC_RUNNING 0x02U
#endif  /* LWIP_NETIF_PROMISC */

/** Function prototype for netif init functions. Set up flags and output/linkoutput
 * callback functions in this function.
 *
 * @param netif Indicates the netif to initialize.
 */
typedef err_t (*netif_init_fn)(struct netif *netif);
/** Function prototype for netif->input functions. This function is saved as 'input'
 * callback function in the netif struct. Call this function when a packet has been received.
 *
 * @param p Indicates the received packet, copied into a pbuf.
 * @param inp Indicates the netif which received the packet.
 */
typedef err_t (*netif_input_fn)(struct pbuf *p, struct netif *inp);
/** Function prototype for netif->output functions. Called by lwIP when a packet
 * is sent. For ethernet netif, set this to 'etharp_output' and set
 * 'linkoutput'.
 *
 * @param netif Indicates the netif which sends a packet.
 * @param p Indicates the packet to send (p->payload points to IP header).
 * @param ipaddr Indicates the IP address to which the packet is sent.
 */
typedef err_t (*netif_output_fn)(struct netif *netif, struct pbuf *p,
       ip_addr_t *ipaddr);
/** Function prototype for netif->linkoutput functions. Only used for ethernet
 * netifs. This function is called by the ARP when a packet is sent.
 *
 * @param netif Indicates the netif which sends a packet.
 * @param p Indicates the packet to send (raw ethernet packet).
 */
typedef err_t (*netif_linkoutput_fn)(struct netif *netif, struct pbuf *p);
/** Function prototype for netif status- or link-callback functions. */
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Description
* Netif status callback.
*/
typedef void (*netif_status_callback_fn)(struct netif *netif);
/** Function prototype for netif igmp_mac_filter functions */
typedef err_t (*netif_igmp_mac_filter_fn)(struct netif *netif,
       ip_addr_t *group, u8_t action);

#if LWIP_NETIF_ETHTOOL
struct ethtool_ops;
#endif
/** Generic data structure used for all LwIP network interfaces.
 *  The following fields should be filled in by the initialization
 *  function for the device driver: hwaddr_len, hwaddr[], mtu, flags. */


/**
* @ingroup Network_Interfaces_Info
* @par Prototype
* @code
* struct netif {
*      struct netif *next;
*
*      ip_addr_t ip_addr;
*      ip_addr_t netmask;
*      ip_addr_t gw;
*
*      netif_input_fn input;
*      netif_output_fn output;
*      netif_linkoutput_fn linkoutput;
*
*      #if LWIP_NETIF_STATUS_CALLBACK
*        netif_status_callback_fn status_callback;
*      #endif
*
*      #if LWIP_NETIF_LINK_CALLBACK
*        netif_status_callback_fn link_callback;
*      #endif
*
*      #if LWIP_NETIF_REMOVE_CALLBACK
*        netif_status_callback_fn remove_callback;
*      #endif
*
*      void *state;
*
*      drv_send_fn drv_send;
*      drv_set_mac_fn drv_set_mac;
*      drv_get_mac_fn drv_get_mac;
*
*      #if LWIP_DHCP
*        struct dhcp *dhcp;
*         #if LWIP_DHCPS
*           struct dhcps *dhcps;
*         #endif
*      #endif
*
*      #if LWIP_AUTOIP
*        struct autoip *autoip;
*      #endif
*
*      #if LWIP_NETIF_HOSTNAME
*        char hostname[NETIF_HOSTNAME_MAX_LEN];
*      #endif
*
*      u16_t mtu;
*      u8_t hwaddr_len;
*      u8_t hwaddr[NETIF_MAX_HWADDR_LEN];
*      u16_t link_layer_type;
*      u8_t flags;
*      char name[2];
*      u8_t num;
*
*      #if LWIP_SNMP
*       u8_t link_type;
*       u32_t link_speed;
*       u32_t ts;
*       u32_t ifinoctets;
*       u32_t ifinucastpkts;
*       u32_t ifinnucastpkts;
*       u32_t ifindiscards;
*       u32_t ifoutoctets;
*       u32_t ifoutucastpkts;
*       u32_t ifoutnucastpkts;
*       u32_t ifoutdiscards;
*      #endif
*
*      #if LWIP_IGMP
*       netif_igmp_mac_filter_fn igmp_mac_filter;
*      #endif
*
*      #if LWIP_NETIF_HWADDRHINT
*       u8_t *addr_hint;
*      #endif
*
*      #if ENABLE_LOOPBACK
*       struct pbuf *loop_first;
*       struct pbuf *loop_last;
*        #if LWIP_LOOPBACK_MAX_PBUFS
*         u16_t loop_cnt_current;
*        #endif
*      #endif
*
*  };
* @endcode
*
* @par Description
* @datastruct  *next  Pointer to next in linked list \n
* @datastruct  ip_addr IP_add configuration in network byte order \n
* @datastruct  netmask Netmask for the IP \n
* @datastruct  gw Gateway
* @datastruct  input This function is called by the network device driver
*                    to pass a packet up the TCP/IP stack. \n
* @datastruct  output This function is called by the IP module when it wants
*                     to send a packet on the interface. This function typically
*                     first resolves the hardware address, then sends the packet. \n
* @datastruct  linkoutput This function is called by the ARP module when it wants
*                         to send a packet on the interface. This function outputs
*                         the pbuf as-is on the link medium. \n
* @datastruct  status_callback This function is called when the netif state
*                               is set to up or down \n
* @datastruct  link_callback This function is called when the netif link is set to up or down \n
* @datastruct  remove_callback This function is called when the netif has been removed \n
* @datastruct  *state  This field can be set by the device driver and could point
*  to state information for the device. \n
* @datastruct  drv_send  This function is called when lwIP want to send
*                    a packet to interface. \n
* @datastruct  drv_set_hwaddr  This function is called when lwIP want to set
*                    the mac_address of the interface. \n
* @datastruct  *dhcp The DHCP client state information for this netif \n
* @datastruct  *dhcps DHCP Server Informarion for this netif \n
* @datastruct  *autoip The AutoIP client state information for this netif \n
* @datastruct  hostname The hostname for this netif\n
* @datastruct  mtu   Maximum transfer unit (in bytes) \n
* @datastruct  hwaddr_len  Number of bytes used in hwaddr \n
* @datastruct  hwaddr[NETIF_MAX_HWADDR_LEN] Link level hardware address of this interface \n
* @datastruct  link_layer_type  Link layer type, ethernet or wifi \n
* @datastruct  flags flags (see NETIF_FLAG_ above) \n
* @datastruct  name[2] Descriptive abbreviation \n
* @datastruct  num  Number of this interface \n
* @datastruct  link_type  Link type (from "snmp_ifType" enum from snmp.h) \n
* @datastruct  link_speed (estimate) Link speed \n
* @datastruct  ts Timestamp at last change made (up/down) \n
* @datastruct  ifinoctets counters \n
* @datastruct  ifinucastpkts counters \n
* @datastruct  ifinnucastpkts counters \n
* @datastruct  ifindiscards counters \n
* @datastruct  ifoutoctets counters \n
* @datastruct  ifoutucastpkts counters \n
* @datastruct  ifoutnucastpkts counters \n
* @datastruct  ifoutdiscards counters \n
* @datastruct  igmp_mac_filter This function could be called to add or delete a entry in the multicast
*      filter table of the ethernet MAC \n
* @datastruct  *addr_hint Hardware type hint
* @datastruct  *loop_first  List of packets to be queued for ourselves. \n
* @datastruct  *loop_last  List of packets to be queued for ourselves. \n
* @datastruct  loop_cnt_current pbuf count \n
*/
struct netif {
  /** pointer to next in linked list */
  struct netif *next;  /**< Indicates a pointer to next in linked list. */

  /** IP_add configuration in network byte order */
  ip_addr_t ip_addr;  /**< Indicates the IP address configuration in network byte order. */
  ip_addr_t netmask;  /**< Indicates the netmask for the IP. */
  ip_addr_t gw;       /**< Indicates the gateway. */

  /** This function is called by the network device driver
   *  to pass a packet up the TCP/IP stack. */
  netif_input_fn input;  /**< Called by the network device driver to pass a packet up the TCP/IP stack. */
  /* This function is called by the IP module
   *  to send a packet on the interface. This function typically
   *  resolves the hardware address before sending the packet. */
  netif_output_fn output;  /**< Called by the IP module to send a packet on the interface. This function typically first resolves the hardware address, then sends the packet. */
  /* This function is called by the ARP module
   *  to send a packet on the interface. This function outputs
   *  the pbuf as-is on the link medium. */
  netif_linkoutput_fn linkoutput;  /**< Called by the ARP module
                        to send a packet on the interface. This function outputs
                         the pbuf as-is on the link medium. */
#if LWIP_NETIF_STATUS_CALLBACK
  /** This function is called when the netif state is set to up or down.
   */
  netif_status_callback_fn status_callback;  /**< Called when the netif state is set to up or down. */
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
  /** This function is called when the netif link is set to up or down.
   */
  netif_status_callback_fn link_callback;  /**< Called when the netif link is set to up or down. */
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_NETIF_REMOVE_CALLBACK
  /** This function is called when the netif has been removed. */
  netif_status_callback_fn remove_callback;  /**< Called when the netif is removed.  */
#endif /* LWIP_NETIF_REMOVE_CALLBACK */
  /** This field can be set by the device driver and could point
   *  to state information for the device. */
  void *state;      /**< This field can be set by the device driver and could point to state information for the device. */
  /** This function is called by lwIP to send a packet on the interface.*/
  drv_send_fn drv_send;  /**< This function is called when lwIP want to send a packet to interface. */
  /** This function is called by the lwIP when it wants
   *  to set the mac_address of the interface.*/
  drv_set_hwaddr_fn drv_set_hwaddr;  /**<  This function is called when lwIP want to set the mac_address of the interface.*/
#if LWIP_NETIF_PROMISC
  /** This function is called by  lwIP
   *  to set/unset the promiscuous mode of the interface.*/
  drv_config_fn drv_config;
#endif /*LWIP_NETIF_PROMISC*/
#if LWIP_NETIF_ETHTOOL
  struct ethtool_ops *ethtool_ops;
#endif

#if LWIP_DHCP
  /** the DHCP client state information for this netif */
  struct dhcp *dhcp;   /**<   DHCP Server Informarion for this netif. */

#if LWIP_DHCPS
 /* DHCP Server Informarion for this netif */
  struct dhcps *dhcps;
#endif

#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
  /** Indicates the AutoIP client state information for this netif. */
  struct autoip *autoip; /**< The AutoIP client state information for this netif . */
#endif
#if LWIP_NETIF_HOSTNAME
  /* the hostname for this netif, NULL is a valid value */
  char hostname[NETIF_HOSTNAME_MAX_LEN];  /**< The pointer to hostname for this netif, NULL is a valid value.  */
#endif /* LWIP_NETIF_HOSTNAME */
  /** maximum transfer unit (in bytes) */
  u16_t mtu;     /**< Maximum transfer unit (in bytes). */
  /** number of bytes used in hwaddr */
  u8_t hwaddr_len;   /**< Number of bytes used in hwaddr. \n */
  /** Indicates the link level hardware address of this interface. */
  u8_t hwaddr[NETIF_MAX_HWADDR_LEN];   /**<  Indicates the link level hardware address of this interface.*/
  /** link layer type, ethernet or wifi */
  u16_t link_layer_type;   /**< Indicates whether the link layer type is ethernet or wifi. */
  /** flags (see NETIF_FLAG_ above) */
  u16_t flags;  /**< Indicates flags (see NETIF_FLAG_ above). */
#if LWIP_NETIF_PROMISC
  atomic_t flags_ext;
  u32_t flags_ext1;
#endif
  /** descriptive abbreviation */
  char name[IFNAMSIZ];  /**< Descriptive abbreviation. */
  /** number of this interface */
  u8_t num;    /**< Indicates the number of this interface. */
  u8_t ifindex; /* Interface Index mapped to each netif. Starts from 1 */
#if LWIP_NETIF_PROMISC
  sys_mutex_t if_mutex;
#endif
#if LWIP_SNMP
  /** link type (from "snmp_ifType" enum from snmp.h) */
  u8_t link_type;  /**< Indicates the Link type (from "snmp_ifType" enum from snmp.h). */
  /** (estimate) link speed */
  u32_t link_speed;   /**< Indicates the (estimate) Link speed. */
  /** timestamp at last change made (up/down) */
  u32_t ts;  /**< Indicates the timestamp at last change made (up/down). */
  /** counters */
  u32_t ifinoctets;   /**< Indicates counters. */
  u32_t ifinucastpkts; /**< Indicates counters. */
  u32_t ifinnucastpkts; /**< Indicates counters. */
  u32_t ifindiscards; /**< Indicates counters. */
  u32_t ifoutoctets; /**< Indicates counters. */
  u32_t ifoutucastpkts; /**< Indicates counters. */
  u32_t ifoutnucastpkts; /**< Indicates counters. */
  u32_t ifoutdiscards; /**< Indicates counters. */
#endif /* LWIP_SNMP */
#if LWIP_IGMP
  /** This function must be called to add or delete a entry in the multicast
      filter table of the ethernet MAC.*/
  netif_igmp_mac_filter_fn igmp_mac_filter; /**< This function must be called to add or delete a entry in the multicast filter table of the ethernet MAC. */
#endif /* LWIP_IGMP */
#if LWIP_NETIF_HWADDRHINT
  u8_t *addr_hint;  /**< Indicates the hardware type hint. */
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK
  /* List of packets to be queued for ourselves. */
  struct pbuf *loop_first; /**< Indicates the list of packets to be queued for ourselves. */
  struct pbuf *loop_last; /**< Indicates the list of packets to be queued for ourselves. */
#if LWIP_LOOPBACK_MAX_PBUFS
  u16_t loop_cnt_current;  /**< Indicates the pbuf count. */
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif /* ENABLE_LOOPBACK */
/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
  s32_t waketime; /**< Started when netif_stop_queue is called from driver. */
#endif
/* NETIF DRIVER STATUS END*/
};

#if LWIP_SNMP
#define NETIF_INIT_SNMP(netif, type, speed) \
  /* use "snmp_ifType" enum from snmp.h for "type", snmp_ifType_ethernet_csmacd by example */ \
  (netif)->link_type = (type);    \
  /* your link speed here (units: bits per second) */  \
  (netif)->link_speed = (speed);  \
  (netif)->ts = 0;              \
  (netif)->ifinoctets = 0;      \
  (netif)->ifinucastpkts = 0;   \
  (netif)->ifinnucastpkts = 0;  \
  (netif)->ifindiscards = 0;    \
  (netif)->ifoutoctets = 0;     \
  (netif)->ifoutucastpkts = 0;  \
  (netif)->ifoutnucastpkts = 0; \
  (netif)->ifoutdiscards = 0
#else /* LWIP_SNMP */
#define NETIF_INIT_SNMP(netif, type, speed)
#endif /* LWIP_SNMP */


/** The list of network interfaces. */
extern struct netif *netif_list;
/** The default network interface. */
extern struct netif *netif_default;

#if LWIP_DHCP
err_t netif_dhcp_off(struct netif *netif);
#endif

u8_t netif_check_index_isusing(const char* ifname, const u8_t index);

void netif_init(void);

struct netif *netif_add(struct netif *netif, const ip_addr_t *ipaddr, const ip_addr_t *netmask, const ip_addr_t *gw);

struct netif *netif_add_secondary(struct netif *netif, const struct netif *main_netif, const u8_t add_if, const ip_addr_t *ipaddr, const ip_addr_t *netmask, const ip_addr_t *gw);

err_t
netif_set_addr(struct netif *netif, const ip_addr_t *ipaddr, const ip_addr_t *netmask,
      const ip_addr_t *gw);

void
netif_get_addr(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask,
      ip_addr_t *gw);

err_t netif_remove(struct netif * netif);

struct netif *netif_find(const char *name);



struct netif * netif_find_by_ipaddr(ip_addr_t *ipaddr);

#if PF_PKT_SUPPORT
struct netif *netif_find_by_ifindex(u8_t ifindex);
#endif

s8_t  netif_find_dst_ipaddr(ip_addr_t *ipaddr, ip_addr_t **dst_addr);
u8_t netif_ipaddr_isbrdcast(ip_addr_t *ipaddr);

void netif_set_default(struct netif *netif);

err_t netif_set_up(struct netif *netif);


err_t netif_set_down(struct netif *netif);


err_t netif_set_link_up(struct netif *netif);


err_t netif_set_link_down(struct netif *netif);

#if LWIP_NETIF_PROMISC
void netif_update_promiscuous_mode_status(struct netif *netif, u8_t set);
void netif_start_promisc_mode(u8_t ifindex);
void netif_stop_promisc_mode(u8_t ifindex);
#endif  /* LWIP_NETIF_PROMISC */

#if DRIVER_STATUS_CHECK
/* NETIF DRIVER STATUS BEGIN */
err_t
netif_wake_queue(struct netif *netif);
err_t
netif_stop_queue(struct netif *netif);
/* NETIF DRIVER STATUS END */
#endif

void netif_set_ipaddr(struct netif *netif, const ip_addr_t *ipaddr);
void netif_set_netmask(struct netif *netif, const ip_addr_t *netmask);
void netif_set_gw(struct netif *netif, const ip_addr_t *gw);
void netif_set_hwaddr(struct netif *netif, const unsigned char *hw_addr, int hw_len);


err_t netif_set_mtu(struct netif *netif, u16_t netif_mtu);

/** Ask if an interface is up */
/*lint --emacro( (774), netif_is_up)*/
#define netif_is_up(netif) (((netif)->flags & NETIF_FLAG_UP) ? (u8_t)1 : (u8_t)0)
/*
Func Name: netif_set_status_callback
*/




/**
* @ingroup  Driver_Interfaces
* @brief
*     Sets the callback to call when the interface is brought up/down.
*
* @param[in]   netif                       Indicates the netif structure.
* @param[in]   netif_status_callback_fn    Indicates the netif set status call function.
*/

#if LWIP_NETIF_STATUS_CALLBACK
void netif_set_status_callback(struct netif *netif, netif_status_callback_fn status_callback);
#endif /* LWIP_NETIF_STATUS_CALLBACK */
    /*
Func Name: netif_set_remove_callback
*/


/**
* @ingroup  Driver_Interfaces
* @brief
*   Sets the callback to call when the interface is removed.
*
* @param[in]   netif                               Indicates the netif structure.
* @param[in]   netif_status_callback_fn    Indicates the netif set status call function.
*/
#if LWIP_NETIF_REMOVE_CALLBACK
void netif_set_remove_callback(struct netif *netif, netif_status_callback_fn remove_callback);
#endif /* LWIP_NETIF_REMOVE_CALLBACK */

/* Ask if a link is up */
#define netif_is_link_up(netif) (((netif)->flags & NETIF_FLAG_LINK_UP) ? (u8_t)1 : (u8_t)0)

#if DRIVER_STATUS_CHECK
/** Ask if a driver is ready to send */
#define netif_is_ready(netif) (((netif)->flags & NETIF_FLAG_DRIVER_RDY) ? (u8_t)1 : (u8_t)0)
#endif

#if LWIP_NETIF_LINK_CALLBACK
err_t netif_set_link_callback(struct netif *netif, netif_status_callback_fn link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */

#if LWIP_IGMP
#define netif_set_igmp_mac_filter(netif, function) do { if((netif) != NULL) { (netif)->igmp_mac_filter = function; }}while(0)
#define netif_get_igmp_mac_filter(netif) (((netif) != NULL) ? ((netif)->igmp_mac_filter) : NULL)
#endif /* LWIP_IGMP */

#if ENABLE_LOOPBACK
err_t netif_loop_output(struct netif *netif, struct pbuf *p, ip_addr_t *dest_ip);
void netif_poll(struct netif *netif);
#if !LWIP_NETIF_LOOPBACK_MULTITHREADING
void netif_poll_all(void);
#endif /* !LWIP_NETIF_LOOPBACK_MULTITHREADING */
#endif /* ENABLE_LOOPBACK */

#if LWIP_NETIF_HWADDRHINT
/** LWIP_NETIF_HWADDRHINT */
#define NETIF_SET_HWADDRHINT(netif, hint) ((netif)->addr_hint = (hint))
#else /* LWIP_NETIF_HWADDRHINT */
#define NETIF_SET_HWADDRHINT(netif, hint)
#endif /* LWIP_NETIF_HWADDRHINT */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_NETIF_H__ */
