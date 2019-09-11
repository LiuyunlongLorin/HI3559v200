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

/**********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 **********************************************************************************/

#ifndef __LWIP_IP_H__
#define __LWIP_IP_H__

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Currently, the function ip_output_if_opt() is only used with IGMP */
#define IP_OPTIONS_SEND   LWIP_IGMP

#define IP_HLEN 20

#define IP_PROTO_ICMP    IPPROTO_ICMP
#define IP_PROTO_IGMP    IPPROTO_IGMP
#define IP_PROTO_UDP     IPPROTO_UDP
#define IP_PROTO_UDPLITE IPPROTO_UDPLITE
#define IP_PROTO_TCP     IPPROTO_TCP

/* This is passed as the destination address to ip_output_if (not
   to ip_output), meaning that an IP header already is constructed
   in the pbuf. This is used when TCP retransmits. */
#ifdef IP_NULL
#undef IP_NULL
#endif /* IP_NULL */
#define IP_NULL  NULL

#if LWIP_NETIF_HWADDRHINT
#define IP_PCB_ADDRHINT ;u8_t addr_hint
#define IP_PAD
#else
#define IP_PCB_ADDRHINT
#define IP_PAD ;u8_t pad_ip
#endif /* LWIP_NETIF_HWADDRHINT */

/* This is the common part of all PCB types. It needs to be at the
   beginning of a PCB type definition. It is located here so that
   changes to this common part are made in one location instead of
   having to change all PCB structs. */
#define IP_PCB \
  /* IP_add in network byte order */ \
  ip_addr_t local_ip; \
  ip_addr_t remote_ip; \
   /* Socket options */  \
   /* PC Lint Fix, u8_t to u32_t */  \
  u32_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* the netif set by bindtodevice */ \
  u8_t ifindex;              \
  /* Time To Live */     \
  u8_t ttl              \
  /* link layer address resolution hint */ \
  IP_PCB_ADDRHINT \
  IP_PAD

struct ip_pcb {
/* Common members of all PCB types */
  IP_PCB;
#ifdef IP_PCB_ADDRHINT
  u8_t pad[1];
#else
  u8_t pad[2];
#endif
};

/* route entry lookup scope. */
typedef enum rt_scope {
  RT_SCOPE_UNIVERSAL,
  RT_SCOPE_LINK,
  RT_SCOPE_HOST
} rt_scope_t;

/*
 * Option flags per-socket. These are the same like SO_XXX.
 */
#define SOF_ACCEPTCONN    0x02U  /* socket has had listen() */
#define SOF_REUSEADDR     0x04U  /* allow local address reuse */
#define SOF_KEEPALIVE     0x08U  /* keep connections alive */
#define SOF_BROADCAST     0x20U  /* permit to send and to receive brcast messages (see IP_SOF_brcast option) */
#define SOF_LINGER        0x80U  /* linger on close if data present */
#define SOF_REUSEPORT   0x0200U   /* Unimplemented: allow local address & port reuse */
#define SOF_DONTROUTE  0X0400U   /* all packets sent from this socket should only route with link scope entry */
#define SOF_BINDNONUNICAST  0x0800U   /* socket has bind to a non unicast */

/* These flags are inherited (e.g. from a listen-pcb to a connection-pcb): */
#define SOF_INHERITED   (SOF_REUSEADDR|SOF_KEEPALIVE|SOF_LINGER|SOF_DONTROUTE/*|SOF_DEBUG|SOF_OOBINLINE*/)

PACK_STRUCT_BEGIN
struct ip_hdr {
  /* version / header length */
  PACK_STRUCT_FIELD(u8_t _v_hl);
  /* type of service */
  PACK_STRUCT_FIELD(u8_t _tos);
  /* total length */
  PACK_STRUCT_FIELD(u16_t _len);
  /* identification */
  PACK_STRUCT_FIELD(u16_t _id);
  /* fragment offset field */
  PACK_STRUCT_FIELD(u16_t _offset);
  /* time to live */
  PACK_STRUCT_FIELD(u8_t _ttl);
  /* protocol*/
  PACK_STRUCT_FIELD(u8_t _proto);
  /* checksum */
  PACK_STRUCT_FIELD(u16_t _chksum);
  /* source and destination IP_add */
  PACK_STRUCT_FIELD(ip_addr_p_t src);
  PACK_STRUCT_FIELD(ip_addr_p_t dest);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

#define IPH_V(hdr)  ((hdr)->_v_hl >> 4)
#define IPH_HL(hdr) ((hdr)->_v_hl & 0x0f)
#define IPH_TOS(hdr) ((hdr)->_tos)
#define IPH_LEN(hdr) ((hdr)->_len)
#define IPH_ID(hdr) ((hdr)->_id)
#define IPH_OFFSET(hdr) ((hdr)->_offset)
#define IPH_TTL(hdr) ((hdr)->_ttl)
#define IPH_PROTO(hdr) ((hdr)->_proto)
#define IPH_CHKSUM(hdr) ((hdr)->_chksum)

#define IPH_VHL_SET(hdr, v, hl) ((hdr)->_v_hl = (((v) << 4) | (hl)))
#define IPH_TOS_SET(hdr, tos) ((hdr)->_tos = (tos))
#define IPH_LEN_SET(hdr, len) ((hdr)->_len = (len))
#define IPH_ID_SET(hdr, id) ((hdr)->_id = (id))
#define IPH_OFFSET_SET(hdr, off) ((hdr)->_offset = (off))
#define IPH_TTL_SET(hdr, ttl) ((hdr)->_ttl = (u8_t)(ttl))
#define IPH_PROTO_SET(hdr, proto) ((hdr)->_proto = (u8_t)(proto))
#define IPH_CHKSUM_SET(hdr, chksum) ((hdr)->_chksum = (chksum))
/** The interface that provided the packet for the current callback invocation. */
extern struct netif *current_netif;
/** Header of the input packet currently being processed. */
extern const struct ip_hdr *current_header;
/** Source IP_add of current_header */
extern ip_addr_t current_iphdr_src;
/** Destination IP_add of current_header */
extern ip_addr_t current_iphdr_dest;
extern u16_t ip_id;

#define ip_init() /* Compatibility define, not init needed. */
struct netif *ip_route(ip_addr_t *dest, rt_scope_t scope);
err_t ip_input(struct pbuf *p, struct netif *inp);
err_t ip_output(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto, struct ip_pcb *pcb);
err_t ip_output_if(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto,
       struct netif *netif);
err_t ip_output_if_src(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto,
       struct netif *netif);
#if LWIP_NETIF_HWADDRHINT
err_t ip_output_hinted(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto, u8_t *addr_hint, struct ip_pcb *pcb);
#endif /* LWIP_NETIF_HWADDRHINT */
#if IP_OPTIONS_SEND
err_t ip_output_if_opt(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto, struct netif *netif, void *ip_options,
       u16_t optlen);
err_t ip_output_if_opt_src(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto, struct netif *netif, void *ip_options,
       u16_t optlen);
#endif /* IP_OPTIONS_SEND */
/** Get the interface that received the current packet.
 * This function must only be called from a receive callback (udp_recv,
 * raw_recv, tcp_accept). It will return NULL otherwise. */
#define ip_current_netif()  (current_netif)
/** Get the IP header of the current packet.
 * This function must only be called from a receive callback (udp_recv,
 * raw_recv, tcp_accept). It will return NULL otherwise. */
#define ip_current_header() (current_header)
/** Source IP_add of current_header */
#define ip_current_src_addr()  (&current_iphdr_src)
/** Destination IP_add of current_header */
#define ip_current_dest_addr() (&current_iphdr_dest)

/** Gets an IP pcb option (SOF_* flags) */
#define ip_get_option(pcb, opt)   ((pcb)->so_options & (opt))
/** Sets an IP pcb option (SOF_* flags) */
#define ip_set_option(pcb, opt)   ((pcb)->so_options |= (opt))
/** Resets an IP pcb option (SOF_* flags) */
#define ip_reset_option(pcb, opt) ((pcb)->so_options &= ~(opt))

#ifdef  LWIP_DEV_DEBUG
#if IP_DEBUG
void ip_debug_print(struct pbuf *p);
#else
#define ip_debug_print(p)
#endif /* IP_DEBUG */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_IP_H__ */


