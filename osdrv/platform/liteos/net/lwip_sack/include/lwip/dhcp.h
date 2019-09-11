/*  @file
 */

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

#ifndef __LWIP_DHCP_H__
#define __LWIP_DHCP_H__

#include "lwip/opt.h"

#if LWIP_DHCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/netif.h"
#include "lwip/udp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* period (in seconds) of the application calling dhcp_coarse_tmr() */
#define DHCP_COARSE_TIMER_SECS 60
/** period (in milliseconds) of the application calling dhcp_coarse_tmr() */
#define DHCP_COARSE_TIMER_MSECS (DHCP_COARSE_TIMER_SECS * 1000UL)
/** period (in milliseconds) of the application calling dhcp_fine_tmr() */
#define DHCP_FINE_TIMER_MSECS 500

#define DHCP_CHADDR_LEN 16U
#define DHCP_SNAME_LEN  64U
#define DHCP_FILE_LEN   128U

#define DHCP_BROADCAST_FLAG 0x8000

/** Minimum length for reply before packet is parsed */
#define DHCP_MIN_REPLY_LEN             44

/** DHCP_OPTION_MAX_MSG_SIZE is set to the MTU
 * MTU is checked to be big enough in dhcp_start */
#define DHCP_MAX_MSG_LEN(netif)        (netif->mtu)
#define DHCP_MAX_MSG_LEN_MIN_REQUIRED  576

struct dhcp
{
  /** transaction identifier of last sent request */
  u32_t xid;
  /** our connection to the DHCP server */
  struct udp_pcb *pcb;
  /** incoming msg */
  struct dhcp_msg *msg_in;
  /** retries of current request */
  u32_t tries;
  /** current DHCP state machine state */
  u8_t state;
#if LWIP_DHCP_AUTOIP_COOP
  u8_t autoip_coop_state;
#endif
  u8_t subnet_mask_given;

  struct pbuf *p_out; /* pbuf of outcoming msg */
  struct dhcp_msg *msg_out; /* outgoing msg */
  u16_t options_out_len; /* outgoing msg options length */
  u16_t request_timeout; /* #ticks with period DHCP_FINE_TIMER_SECS for request timeout */
  u16_t t1_timeout;  /* #ticks with period DHCP_COARSE_TIMER_SECS for renewal time */
  u16_t t2_timeout;  /* #ticks with period DHCP_COARSE_TIMER_SECS for rebind time */
  ip_addr_t server_ip_addr; /* dhcp server address that offered this lease */
  ip_addr_t offered_ip_addr;
  ip_addr_t offered_sn_mask;
  ip_addr_t offered_gw_addr;

  u32_t offered_t0_lease; /* lease period (in seconds) */
  u32_t offered_t1_renew; /* recommended renew time (usually 50% of lease period) */
  u32_t offered_t2_rebind; /* recommended rebind time (usually 66% of lease period)  */
  /*  LWIP_DHCP_BOOTP_FILE configuration option?
     integrate with possible TFTP-client for booting? */
#if LWIP_DHCP_BOOTP_FILE
  ip_addr_t offered_si_addr;
  char boot_file_name[DHCP_FILE_LEN];
#endif /* LWIP_DHCP_BOOTPFILE */
};

PACK_STRUCT_BEGIN
/* minimum set of fields of any DHCP message */
struct dhcp_msg
{
  PACK_STRUCT_FIELD(u8_t op);
  PACK_STRUCT_FIELD(u8_t htype);
  PACK_STRUCT_FIELD(u8_t hlen);
  PACK_STRUCT_FIELD(u8_t hops);
  PACK_STRUCT_FIELD(u32_t xid);
  PACK_STRUCT_FIELD(u16_t secs);
  PACK_STRUCT_FIELD(u16_t flags);
  PACK_STRUCT_FIELD(ip_addr_p_t ciaddr);
  PACK_STRUCT_FIELD(ip_addr_p_t yiaddr);
  PACK_STRUCT_FIELD(ip_addr_p_t siaddr);
  PACK_STRUCT_FIELD(ip_addr_p_t giaddr);
  PACK_STRUCT_FIELD(u8_t chaddr[DHCP_CHADDR_LEN]);
  PACK_STRUCT_FIELD(u8_t sname[DHCP_SNAME_LEN]);
  PACK_STRUCT_FIELD(u8_t file[DHCP_FILE_LEN]);
  PACK_STRUCT_FIELD(u32_t cookie);
#define DHCP_MIN_OPTIONS_LEN 68U
/* make sure user does not configure this too small */
#if ((defined(DHCP_OPTIONS_LEN)) && (DHCP_OPTIONS_LEN < DHCP_MIN_OPTIONS_LEN))
#  undef DHCP_OPTIONS_LEN
#endif
/* allow this to be configured in lwipopts.h, but not too small */
#if (!defined(DHCP_OPTIONS_LEN))
/* set this to be sufficient for your options in outgoing DHCP msgs */
#  define DHCP_OPTIONS_LEN DHCP_MIN_OPTIONS_LEN
#endif
  PACK_STRUCT_FIELD(u8_t options[DHCP_OPTIONS_LEN]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

void dhcp_set_struct(struct netif *netif, struct dhcp *dhcp);

#define dhcp_remove_struct(netif) do { (netif)->dhcp = NULL; } while(0)

void dhcp_cleanup(struct netif *netif);

err_t dhcp_is_bound(struct netif *netif);

err_t dhcp_start(struct netif *netif);

err_t dhcp_renew(struct netif *netif);


err_t dhcp_release(struct netif *netif);


void dhcp_stop(struct netif *netif);


void dhcp_inform(struct netif *netif);



/** Handle a possible change in the network configuration */
void dhcp_network_changed(struct netif *netif);

/** if enabled, check whether the offered IP_add is not in use, using ARP */
#if DHCP_DOES_ARP_CHECK
void dhcp_arp_reply(struct netif *netif, ip_addr_t *addr);
#endif

/** to be called every minute */
void dhcp_coarse_tmr(void);
/** to be called every half second */
void dhcp_fine_tmr(void);

/* Common Function for parsing Options for both Server and Client */
#if LWIP_DHCP_BOOTP_FILE
err_t dhcp_parse_options(struct pbuf *p, char *boot_file_name);
#else
err_t dhcp_parse_options(struct pbuf *p);
#endif

/* Common Function for option addition for both Server and Client */
void dhcp_common_option(struct dhcp_msg *msg_out, u8_t option_type, u8_t option_len, u16_t *options_out_len);
void dhcp_common_option_byte(struct dhcp_msg *msg_out, u8_t value, u16_t *options_out_len);
void dhcp_common_option_short(struct dhcp_msg *msg_out, u16_t value, u16_t *options_out_len);
void dhcp_common_option_long(struct dhcp_msg *msg_out, u32_t value, u16_t *options_out_len);
void dhcp_common_option_trailer(struct dhcp_msg *msg_out, u16_t *options_out_len);







/** DHCP message item offsets and length */
#define DHCP_OP_OFS       0
#define DHCP_HTYPE_OFS    1
#define DHCP_HLEN_OFS     2
#define DHCP_HOPS_OFS     3
#define DHCP_XID_OFS      4
#define DHCP_SECS_OFS     8
#define DHCP_FLAGS_OFS    10
#define DHCP_CIADDR_OFS   12
#define DHCP_YIADDR_OFS   16
#define DHCP_SIADDR_OFS   20
#define DHCP_GIADDR_OFS   24
#define DHCP_CHADDR_OFS   28
#define DHCP_SNAME_OFS    44
#define DHCP_FILE_OFS     108
#define DHCP_MSG_LEN      236

#define DHCP_COOKIE_OFS   DHCP_MSG_LEN
#define DHCP_OPTIONS_OFS  (DHCP_MSG_LEN + 4)

#define DHCP_CLIENT_PORT  68
#define DHCP_SERVER_PORT  67

/** DHCP client states */
#define DHCP_OFF          0
#define DHCP_REQUESTING   1
#define DHCP_INIT         2
#define DHCP_REBOOTING    3
#define DHCP_REBINDING    4
#define DHCP_RENEWING     5
#define DHCP_SELECTING    6
#define DHCP_INFORMING    7
#define DHCP_CHECKING     8
#define DHCP_PERMANENT    9
#define DHCP_BOUND        10
/** not yet implemented #define DHCP_RELEASING 11 */
#define DHCP_BACKING_OFF  12

/** AUTOIP cooperatation flags */
#define DHCP_AUTOIP_COOP_STATE_OFF  0
#define DHCP_AUTOIP_COOP_STATE_ON   1

#define DHCP_BOOTREQUEST  1
#define DHCP_BOOTREPLY    2

/** DHCP message types */
#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_DECLINE  4
#define DHCP_ACK      5
#define DHCP_NAK      6
#define DHCP_RELEASE  7
#define DHCP_INFORM   8

/** DHCP hardware type, currently only ethernet is supported */
#define DHCP_HTYPE_ETH 1
#define DHCP_HTYPE_WIFI 6

#define DHCP_MAGIC_COOKIE 0x63825363UL

/* This is a list of options for BOOTP and DHCP, see RFC 2132 for descriptions */

/** BootP options */
#define DHCP_OPTION_PAD 0
#define DHCP_OPTION_SUBNET_MASK 1 /* RFC 2132 3.3 */
#define DHCP_OPTION_SUBNET_MASK_SIZE 4
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_DNS_SERVER 6
#define DHCP_OPTION_HOSTNAME 12
#define DHCP_OPTION_IP_TTL 23
#define DHCP_OPTION_MTU 26
#define DHCP_OPTION_BROADCAST 28
#define DHCP_OPTION_TCP_TTL 37
#define DHCP_OPTION_END 255

/** DHCP options */
#define DHCP_OPTION_REQUESTED_IP 50 /* RFC 2132 9.1, requested IP_add */
#define DHCP_OPTION_LEASE_TIME 51 /* RFC 2132 9.2, time in seconds, in 4 bytes */
#define DHCP_OPTION_LEASE_TIME_SIZE 4

#define DHCP_OPTION_OVERLOAD 52 /* RFC2132 9.3, use file and/or sname field for options */

#define DHCP_OPTION_MESSAGE_TYPE 53 /* RFC 2132 9.6, important for DHCP */
#define DHCP_OPTION_MESSAGE_TYPE_LEN 1

#define DHCP_OPTION_SERVER_ID 54 /* RFC 2132 9.7, server IP_add */
#define DHCP_OPTION_SERVER_ID_LEN 4

#define DHCP_OPTION_PARAMETER_REQUEST_LIST 55 /* RFC 2132 9.8, requested option types */

#define DHCP_OPTION_MAX_MSG_SIZE 57 /* RFC 2132 9.10, message size accepted >= 576 */
#define DHCP_OPTION_MAX_MSG_SIZE_LEN 2

#define DHCP_OPTION_T1 58 /* T1 renewal time */
#define DHCP_OPTION_T1_LEN 4
#define DHCP_OPTION_T2 59 /* T2 rebinding time */
#define DHCP_OPTION_T2_LEN 4
#define DHCP_OPTION_US 60
#define DHCP_OPTION_CLIENT_ID 61
#define DHCP_OPTION_CLIENT_ID_LEN 7
#define DHCP_OPTION_TFTP_SERVERNAME 66
#define DHCP_OPTION_BOOTFILE 67

/** possible combinations of overloading the file and sname fields with options */
#define DHCP_OVERLOAD_NONE 0
#define DHCP_OVERLOAD_FILE 1
#define DHCP_OVERLOAD_SNAME  2
#define DHCP_OVERLOAD_SNAME_FILE 3

/** Option handling: options are parsed in dhcp_parse_reply
 * and saved in an array where other functions can load them from.
 * This might be moved into the struct dhcp (not necessarily since
 * lwIP is single-threaded and the array is only used while in recv
 * callback). */
#define DHCP_OPTION_IDX_OVERLOAD    0
#define DHCP_OPTION_IDX_MSG_TYPE    1
#define DHCP_OPTION_IDX_SERVER_ID   2
#define DHCP_OPTION_IDX_LEASE_TIME  3
#define DHCP_OPTION_IDX_T1          4
#define DHCP_OPTION_IDX_T2          5
#define DHCP_OPTION_IDX_SUBNET_MASK 6
#define DHCP_OPTION_IDX_ROUTER      7
#define DHCP_OPTION_IDX_DNS_SERVER  8
#define DHCP_OPTION_IDX_REQUESTED_IP 9
#define DHCP_OPTION_IDX_MAX         (DHCP_OPTION_IDX_DNS_SERVER + DNS_MAX_SERVERS)


#define dhcp_option_given(dhcp, idx)          (dhcp_rx_options_given[idx] != 0)
#define dhcp_got_option(dhcp, idx)            (dhcp_rx_options_given[idx] = 1)
#define dhcp_clear_option(dhcp, idx)          (dhcp_rx_options_given[idx] = 0)
#define dhcp_clear_all_options(dhcp)          (MEMSET_S(dhcp_rx_options_given, sizeof(dhcp_rx_options_given), \
						0, sizeof(dhcp_rx_options_given))) //CSEC_FIX_2302
#define dhcp_get_option_value(dhcp, idx)      (dhcp_rx_options_val[idx])
#define dhcp_set_option_value(dhcp, idx, val) (dhcp_rx_options_val[idx] = (val))

extern u32_t dhcp_rx_options_val[DHCP_OPTION_IDX_MAX];
extern u8_t  dhcp_rx_options_given[DHCP_OPTION_IDX_MAX];


#ifdef LWIP_DHCPS

struct dyn_lease_addr
{
	u8_t cli_hwaddr[DHCP_CHADDR_LEN];
    	u32_t flags;
       u32_t leasetime;
       u32_t proposed_leasetime;
       ip_addr_t cli_addr;
};



struct dhcps
{
  struct udp_pcb *pcb;
  struct dyn_lease_addr leasearr[LWIP_DHCPS_MAX_LEASE];
  u16_t lease_num;
  struct netif *netif;
  ip_addr_t start_addr;
  ip_addr_t end_addr;
};


err_t dhcps_start(struct netif *netif, const char *start_ip, u16_t ip_num);

void dhcps_stop(struct netif *netif);


#endif


#ifdef __cplusplus
}
#endif

#endif /* LWIP_DHCP */

#endif /*__LWIP_DHCP_H__*/
