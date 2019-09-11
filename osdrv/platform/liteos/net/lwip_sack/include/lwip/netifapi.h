/*
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
 *   @file netifapi.h
 */

 /**
 *  @defgroup Threadsafe_Network_Interfaces Thread-safe Network Interfaces
 *  This section contains the thread safe network related interfaces.
 *  @ingroup System_interfaces
 */

/**
* @defgroup Threadsafe_DHCP_Interfaces Thread-safe DHCP Interfaces
* This section contains the thread safe DHCP interfaces.
* @ingroup System_interfaces
*/

#ifndef __LWIP_NETIFAPI_H__
#define __LWIP_NETIFAPI_H__

#include "lwip/opt.h"

#if LWIP_NETIF_API /* don't build if not configured for use in lwipopts.h */

#include "lwip/sys.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*netifapi_void_fn)(struct netif *netif);
typedef err_t (*netifapi_errt_fn)(struct netif *netif);

struct netifapi_msg_msg {
#if !LWIP_TCPIP_CORE_LOCKING
  sys_sem_t sem;
#endif /* !LWIP_TCPIP_CORE_LOCKING */
  err_t err;
  struct netif *netif;
  union {
    struct {
      ip_addr_t *ipaddr;
      ip_addr_t *netmask;
      ip_addr_t *gw;
    } add;
    struct {
      struct netif *netif;
      const char *name;
    } find_by_name;
    struct {
      struct netif *netif;
      unsigned char ifindex;
    } find_by_ifindex;
    struct {
      netifapi_void_fn voidfunc;
      netifapi_errt_fn errtfunc;
    } common;
    struct {
      struct dhcp *dhcp;
    } dhcp_struct;
#if LWIP_DHCPS
    struct {
      const char *start_ip;
      u16_t ip_num;
    } dhcps_struct;
#endif
    struct {
      netif_status_callback_fn link_callback;
    } netif_link_cb;
    struct {
      u16_t mtu;
    }netif_mtu;
    struct {
      struct netif *netif;
    } isdefault;
#if LWIP_NETIF_HOSTNAME
    struct {
      char *name;
      u8_t namelen;
    } hostname;
#endif /* LWIP_NETIF_HOSTNAME */
  } msg;
};

struct netifapi_msg {
  void (* function)(struct netifapi_msg_msg *msg);
  struct netifapi_msg_msg msg;
};

#if LWIP_NETIF_API /* don't build if not configured for use in lwipopts.h */

/*
Func Name:  netifapi_netif_add
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_add(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw);
* @endcode
*
* @par Purpose
*  This API is used to add a network interface to the list of LwIP netifs.
*
* @par Description
*  This is a thread safe API, used to add a network interface to the list of lwIP netifs. It is recommended
*  to use this API instead of netif_add().
*
* @param[in]    netif         Indicates the pre-allocated netif structure.
* @param[in]    ipaddr       Indicates the IP_add for the new netif.
* @param[in]    netmask    Indicates the network mask for the new netif.
* @param[in]    gw            Indicates the default gateway IP_add for the new netif.
*
* @par Return values
*  0 : On success \n
*  Negative value : On failure \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* The interface names are the format <ifname><num>
*
* @par Related Topics
* netif_add
*/

err_t netifapi_netif_add       ( struct netif *netif,
                                 ip_addr_t *ipaddr,
                                 ip_addr_t *netmask,
                                 ip_addr_t *gw);


/*
Func Name:  netifapi_netif_set_addr
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_addr(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw );
* @endcode
*
* @par Purpose
*  This API is used to change IP_add configuration for a network interface (including netmask and default gateway).
*
* @par Description
*  This is a thread safe API, used to change IP_add configuration for a network interface (including netmask and default gateway).
*  It is recommended to use this API instead of netif_set_addr().
*
* @param[in]    netif          Indicates the network interface to change.
* @param[in]    ipaddr       Indicates the new IP address.
* @param[in]    netmask    Indicates the new network mask.
* @param[in]    gw            Indicates the new default gateway IP address.
*
* @par Return values
*  0 : On success \n
*  Negative value : On failure \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* netif_set_addr
*/

err_t netifapi_netif_set_addr  ( struct netif *netif,
                                 ip_addr_t *ipaddr,
                                 ip_addr_t *netmask,
                                 ip_addr_t *gw );
/*
Func Name:  netifapi_netif_get_addr
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_get_addr(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw );
* @endcode
*
* @par Purpose
*  This API is used to get IP_add configuration for a network interface (including netmask and default gateway).
*
* @par Description
*
*  This is a thread safe API, used to get IP_add configuration for a network interface (including netmask and default gateway).
*  It is recommended to use this API instead of netif_get_addr()
*
* @param[in]    netif          Indicates the network interface to get.
* @param[in]    ipaddr       Indicates the IP address.
* @param[in]    netmask    Indicates the network mask.
* @param[in]    gw            Indicates the default gateway IP address.
*
* @par Return values
*  0 : On success \n
*  Negative value : On failure \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* netif_get_addr
*/

err_t netifapi_netif_get_addr  ( struct netif *netif,
                                 ip_addr_t *ipaddr,
                                 ip_addr_t *netmask,
                                 ip_addr_t *gw );

/*
Func Name:  netifapi_netif_common
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_common(struct netif *netif, netifapi_void_fn voidfunc, netifapi_errt_fn errtfunc);
* @endcode
*
* @par Purpose
*  This API is used to call all netif related APIs in a thread safe manner.
*
* @par Description
*
*  This API is used to call all netif related APIs in a thread safe manner. Those netif related APIs must be
*  of prototype to receive only struct netif* as argument and return type can of type err_t or void. You
*  must pass either viodfunc or errtfunc.
*
* @param[in]    netif          Indicates the network interface to be passed as argument.
* @param[in]    voidfunc    Indicates the netif API to call.
* @param[in]    errtfunc     Indicates the netif API to call.
*
* @par Return values
*  0 : On success \n
*  Negative value : On failure \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* The prototype for netifapi_void_fn and netifapi_errt_fn are as follows:\n
* typedef void (*netifapi_void_fn)(struct netif *netif); \n
* typedef err_t (*netifapi_errt_fn)(struct netif *netif); \n
*
* @par Related Topics
* N/A
*/
err_t netifapi_netif_common    ( struct netif *netif,
                                 netifapi_void_fn voidfunc,
                                 netifapi_errt_fn errtfunc);


/*
Func Name:  netifapi_netif_remove
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_remove(struct netif * netif);
* @endcode
*
* @par Purpose
*  This API is used to remove a network interface from the list of lwIP netifs
*  in a thread-safe way.
*
* @par Description
*  The Ethernet driver calls this API in a thread-safe way to remove a network
*  interface from the list of lwIP netifs.
*
* @param[in]    netif     Indicates network interface to be removed.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
 - Any data sent by the application will return success, but the data will not be sent if only netifapi_netif_remove() API is called and not the sockets.
 - Adaptor must close all the connections on socket before netifapi_netif_remove() API is called, as netifapi_netif_remove() does not close established connections.
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_netif_remove(struct netif *netif);

/*
Func Name:  netifapi_netif_set_up
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_up(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to bring an interface up in a thread-safe way.
*
* @par Description
*  This API is used to bring an interface up in a thread-safe way.
*  That is, available for processing traffic.
*
* @param[in]    netif     Indicates the network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* Enabling DHCP on a down interface will make it come up once configured.
*
* @par Related Topics
* netifapi_dhcp_start
*/
err_t netifapi_netif_set_up(struct netif *netif);

 /*
Func Name:  netifapi_netif_set_down
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_down(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to bring an interface down in a thread-safe way.
*
* @par Description
*  This API is used to bring an interface up in a thread-safe way.
*  That is, the interface disables any traffic processing.
*
* @param[in]    netif     Indicates the network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* Enabling DHCP on a down interface will make it come up once configured.
*
* @par Related Topics
* netifapi_dhcp_start
*/
err_t netifapi_netif_set_down(struct netif *netif);

/*
Func Name:  netifapi_netif_set_default
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_default(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to set a network interface as the default network interface
*  in a thread-safe way.
*
* @par Description
*  This API is used to set a network interface as the default network interface.
*  It is used to output all packets for which no specific route is found.
*
* @param[in]    netif     Indicates the network interface to be set as default.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_netif_set_default(struct netif *netif);

/*
Func Name:  netifapi_netif_get_default
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* struct netif* netifapi_netif_get_default(void);
* @endcode
*
* @par Purpose
*  This API is used to get the default network interface in a thread-safe way.
*
* @par Description
*  This API is used to get the default network interface.
*  It is used to output all packets for which no specific route is found.
*
* @param[in] [N/A]
*
* @par Return values
*  NULL: if either the default netif was NOT exist or the default netif was down \n
*  Others: the default netif \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
struct netif *netifapi_netif_get_default(void);

/*
Func Name:  netifapi_netif_set_link_up
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_link_up(struct netif *netif);
* @endcode
*
* @par Purpose
*  This thread-safe API is called by the driver when its link goes up.
*
* @par Description
*  This thread-safe API is called by the driver when its link goes up.
*
* @param[in]    netif     Indicates the network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_netif_set_link_up(struct netif *netif);

/*
Func Name:  netifapi_netif_set_link_down
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_link_down(struct netif *netif);
* @endcode
*
* @par Purpose
*  This thread-safe API is called by the driver when its link goes down.
*
* @par Description
*  This API is called by a driver when its link goes down in a thread-safe way.
*
* @param[in]    netif     Indicates the network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_netif_set_link_down(struct netif *netif);

/*
Func Name:  netifapi_dhcp_is_bound
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_is_bound(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to get the DHCP negotiation status for a network interface in a
*  thread-safe way.
*
* @par Description
*  This API is used to get the DHCP negotiation status for a network interface in a
*  thread-safe way.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: DHCP negotiation is done, network interface obtain new IP. \n
*  ERR_INPROGRESS: the network interface have not obtain new IP. \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcp_is_bound(struct netif *netif);

/*
Func Name:  netifapi_dhcp_start
*/

/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_start(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to start DHCP negotiation for a network interface in a
*  thread-safe way.
*
* @par Description
*  This API is used to start DHCP negotiation for a network interface. If no DHCP client instance
*  is attached to this interface, a new client is created first. If a DHCP client instance is already
*  present, it restarts negotiation. It is the thread-safe way for calling dhcp_start in the user space.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure \n
*  ERR_ARG:Invalid input parameter
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcp_start(struct netif *netif);

/*
Func Name:  netifapi_dhcp_stop
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_stop(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to remove the DHCP client from the interface in a
*  thread-safe way.
*
* @par Description
*  This API is used to remove the DHCP client from the interface. It stops DHCP configuration.
*  It is the thread-safe way for calling dhcp_stop in the user space.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcp_stop(struct netif *netif);

/*
Func Name:  netifapi_dhcp_cleanup
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_cleanup(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to free the memory allocated for DHCP during dhcp start.
*
* @par Description
*  This API is used to free the memory allocated for DHCP during dhcp start.
*  It is the thread-safe way for calling dhcp_cleanup in the user space.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcp_cleanup(struct netif *netif);


/*
Func Name:  netifapi_dhcp_inform
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_inform(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to inform a DHCP server of our manual configuration.
*
* @par Description
*  This API is used to inform a DHCP server of the manual configuration. This informs DHCP servers
*  of our fixed IP_add configuration by sending an INFORM message. It does not involve DHCP
*  address configuration.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/

err_t netifapi_dhcp_inform(struct netif *netif);

/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_set_struct(struct netif *netif, struct dhcp *dhcp);
* @endcode
*
* @par Purpose
*  This API is used to set a static DHCP structure to the netif.
*
* @par Description
*  This API is used to set a static DHCP structure to the netif.
*
* @param[in]    netif    Indicates the LwIP network interface.
* @param[in]    dhcp    Indicates the DHCP structure which needs to be set.
*
* @par Return values
*  ERR_OK: On success.
*
* @par Required Header File
* netifapi.h
*
* @par Note
* If this API is used before netifapi_dhcp_start, the application needs to use
* netifapi_dhcp_remove_struct  instead of netifapi_dhcp_cleanup to remove
* the struct dhcp.
*
* @par Related Topics
* netifapi_dhcp_remove_struct
*/

err_t netifapi_dhcp_set_struct(struct netif *netif, struct dhcp *dhcp);

/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcp_remove_struct(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to remove the static DHCP structure from netif.
*
* @par Description
*  This API is used to remove the static DHCP structure from netif, which was set
*  using the netifapi_dhcp_set_struct() API.
*
* @param[in]    netif    Indicates the LwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure \n
*  ERR_VAL: On Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* The application needs to use this API instead of netifapi_dhcp_cleanup if the
* dhcp structure is previously set on netif using netifapi_dhcp_set_struct().
*
* @par Related Topics
* netifapi_dhcp_set_struct
*/

err_t netifapi_dhcp_remove_struct(struct netif *netif);

/*
Func Name:  netifapi_dhcps_start
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcps_start(struct netif *netif, const char *start_ip, u16_t ip_num);
* @endcode
*
* @par Purpose
*  This API is used to start DHCP Server negotiation for a network interface.
*
* @par Description
*  This API is used to start DHCP Server negotiation for a network interface. If no DHCP server
*  was attached to this interface, a new server is created first. It is the
*  thread-safe way for calling dhcps_start in the user space.
*  If start_ip is set(not NULL) and ip_num is bigger than 0, then dhcp server will use
*  it as the start dhcp lease of dhcp server. the start_ip should in the same subnet of the netif.
*  ip_num shoud not bigger than LWIP_DHCPS_MAX_LEASE, and make sure the last dhcp lease
*  also in the same subnet of the netif.
*  If start_ip is not set, or ip_num is 0, the dhcp lease will set start ip lease
*  at subnet.1 and end at last valid ip addreess of the subnet(the ip lease number still
*  limited by LWIP_DHCPS_MAX_LEASE).
*
* @param[in]    netif    The LwIP network interface.
* @param[in]    start_ip The start of ip address of dhcp lease.
* @param[in]    ip_num   The size the dhcp lease pool.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure \n
*  ERR_ARG:Invalid input parameter
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcps_start(struct netif *netif, const char *start_ip, u16_t ip_num);

/*
Func Name:  netifapi_dhcps_stop
*/
/**
* @ingroup Threadsafe_DHCP_Interfaces
* @par Prototype
* @code
* err_t netifapi_dhcps_stop(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to remove the DHCP server from the interface.
*
* @par Description
*  This API is used to remove the DHCP server from the interface. It stops DHCP Server configuration.
*  It is the thread-safe way for calling dhcps_start in user space.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_dhcps_stop(struct netif *netif);

#if LWIP_AUTOIP
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Description
*   Call autoip_start() in a thread-safe way by running that function inside the tcpip_thread context.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
*  @par Note
*  Use only for functions where there is only "netif" parameter.
*/
err_t netifapi_autoip_start(struct netif *netif);

/**
* @ingroup Threadsafe_Network_Interfaces
* @par Purpose
* Thread-safe API to stop the  AutoIP client.
* @par Description
 * Call autoip_stop() in a thread-safe way by running that function inside the
 * tcpip_thread context.
 *
 * @param[in]    netif    Use only for functions where there is only "netif" parameter.
 *
 * @par Note
 * <ul><li>Use only for functions where there is only "netif" parameter.</li>
 * <li>Call the autoip_stop API to stop the AutoIP service.</li></ul>
 */
err_t netifapi_autoip_stop(struct netif *netif);
#endif /* LWIP_AUTOIP */
#endif /* LWIP_NETIF_API */

#if LWIP_NETIF_LINK_CALLBACK
/*
Func Name:  netifapi_netif_set_link_callback
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_link_callback(struct netif *netif, netif_status_callback_fn link_callback);
* @endcode
*
* @par Purpose
*  This API is used to set callback to netif. This API is called whenever link is brought up or down.
*
* @par Description
*  This API is used to set callback to netif. This API is called whenever link is brought up or down.
*
* @param[in]    netif    Indicates the lwIP network interface.
* @param[in]    link_callback    Indicates the function prototype for netif status- or link-callback functions.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_MEM: On failure due to memory \n
*  ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* netifapi.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/

err_t
netifapi_netif_set_link_callback(struct netif *netif, netif_status_callback_fn link_callback);
#endif /* LWIP_NETIF_LINK_CALLBACK */

/*
Func Name:  netifapi_netif_set_mtu
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_netif_set_mtu(struct netif *netif, u16_t mtu);
* @endcode
*
* @par Purpose
*  This API is used to set the MTU of the netif. Call this API whenever MTU needs to be changed.
*
* @par Description
*  This API is used to set the MTU of the netif. Call this API whenever MTU needs to be changed.
*
* @param[in]    netif    Indicates the lwIP network interface.
* @param[in]    mtu      Indicates the new MTU of the network interface. Valid values are 68 to 1500.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_ARG: On passing invalid arguments, \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* <ul><li>The value of the new mtu which is passed should be in the range 68 to 1500</li>
* <li>On modifying the MTU, the MTU change comes into effect immediately.</li>
* <li>IP packets for existing connections are also sent according to new MTU.</li>
* <li>Ideally, the application must ensure that connections are terminated before MTU modification or at
* init time to avoid side effects, since peer might be expecting a different MTU.</li>
* <li>Effective MSS for existing connection will not change, it might remain same. It is not
* suggested to change the MTU at runtime.</li>
* <li>Only for new connections, effective MSS shall be used for connection setup. </li></ul>
*
* @par Related Topics
* \n
* N/A
*/

err_t netifapi_netif_set_mtu(struct netif *netif, u16_t mtu);

/*NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
/*
Func Name: netifapi_stop_queue
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_stop_queue(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to set the netif driver status to the "Driver Not Ready" state.
*
* @par Description
*  This API is used to set the netif driver status to the "Driver Not Ready" state. The driver must call this API
*  to intimate the stack that the send buffer in the driver is full.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_ARG: On passing invalid arguments. \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_stop_queue(struct netif *netif);

/*
Func Name: netifapi_wake_queue
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_wake_queue(struct netif *netif);
* @endcode
*
* @par Purpose
*  This API is used to set the netif driver status to the "Driver Ready" state.
*
* @par Description
*  This API is used to set the netif driver status to the "Driver Ready" state. This API is called by the driver to
*  inform the stack that the driver send buffer is available to send after a netifapi_stop_queue was called
*  previously.
*
* @param[in]    netif    Indicates the lwIP network interface.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_ARG: On passing invalid arguments. \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_wake_queue(struct netif *netif);
#endif
/*NETIF DRIVER STATUS END */

#if LWIP_NETIF_HOSTNAME
/*
Func Name: netifapi_set_hostname
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_set_hostname(struct netif *netif, char *hostname, u8_t namelen);
* @endcode
*
* @par Purpose
*  This API is used to set the hostname of the netif.
*
* @par Description
*  This API is used to set the hostname of the netif, which is using in DHCP
*  message. The hostname string lenght should be less than NETIF_HOSTNAME_MAX_LEN,
*  otherwise the hostname will truncate to (NETIF_HOSTNAME_MAX_LEN-1).
*
* @param[in]    netif    Indicates the lwIP network interface.
* @param[in]    hostname The new hostname to use.
* @param[in]    namelen  The hostname string length, should be within the scope of 0~255.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_ARG: On passing invalid arguments. \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* N/A
*
* @par Related Topics
* N/A
*/
err_t netifapi_set_hostname(struct netif *netif, char *hostname, u8_t namelen);

/*
Func Name: netifapi_get_hostname
*/
/**
* @ingroup Threadsafe_Network_Interfaces
* @par Prototype
* @code
* err_t netifapi_get_hostname(struct netif *netif, char *hostname, u8_t namelen);
* @endcode
*
* @par Purpose
*  This API is used to get the hostname of the netif.
*
* @par Description
*  This API is used to get the hostname of the netif, which is using in DHCP
*  message. the hostname buffer length shoud not smaller than NETIF_HOSTNAME_MAX_LEN,
*  otherwise it will get a truncated hostname.
*
* @param[in]    netif    Indicates the lwIP network interface.
* @param[in]    hostname The buffer to stroe hostname string of the netif.
* @param[in]    namelen  The hostname string buffer length.
*
* @par Return values
*  ERR_OK: On success \n
*  ERR_ARG: On passing invalid arguments. \n
*
* @par Required Header File
* netifapi.h
*
* @par Note
* N/A
*
* @par Related Topics
* \n
* N/A
*/
err_t netifapi_get_hostname(struct netif *netif, char *hostname, u8_t namelen);

#endif /* LWIP_NETIF_HOSTNAME */


#ifdef __cplusplus
}
#endif

#endif /* LWIP_NETIF_API */

#endif /* __LWIP_NETIFAPI_H__ */
