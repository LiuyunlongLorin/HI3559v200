/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
#ifndef __LWIP_ICMP_H__
#define __LWIP_ICMP_H__

#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include <netinet/ip_icmp.h>
#include <liteos/icmp.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ICMP_ER   ICMP_ECHOREPLY      /* echo reply */
#define ICMP_DUR  ICMP_DEST_UNREACH   /* destination unreachable */
#define ICMP_SQ   ICMP_SOURCE_QUENCH  /* source quench */
#define ICMP_RD   ICMP_REDIRECT       /* redirect */
#define ICMP_TE   ICMP_TIME_EXCEEDED  /* time exceeded */
#define ICMP_PP   ICMP_PARAMETERPROB  /* parameter problem */
#define ICMP_TS   ICMP_TIMESTAMP      /* timestamp */
#define ICMP_TSR  ICMP_TIMESTAMPREPLY /* timestamp reply */
#define ICMP_IRQ  ICMP_INFO_REQUEST   /* information request */
#define ICMP_IR   ICMP_INFO_REPLY     /* information reply */

enum icmp_dur_type {
  ICMP_DUR_NET   = ICMP_UNREACH_NET,        /* net unreachable */
  ICMP_DUR_HOST  = ICMP_UNREACH_HOST,       /* host unreachable */
  ICMP_DUR_PROTO = ICMP_UNREACH_PROTOCOL,   /* protocol unreachable */
  ICMP_DUR_PORT  = ICMP_UNREACH_PORT,       /* port unreachable */
  ICMP_DUR_FRAG  = ICMP_UNREACH_NEEDFRAG,   /* fragmentation needed and DF set */
  ICMP_DUR_SR    = ICMP_UNREACH_SRCFAIL     /* source route failed */
};

enum icmp_te_type {
  ICMP_TE_TTL  = ICMP_TIMXCEED_INTRANS,     /* time to live exceeded in transit */
  ICMP_TE_FRAG = ICMP_TIMXCEED_REASS        /* fragment reassembly time exceeded */
};

/** This is the standard ICMP header only that the u32_t data
 *  is splitted to two u16_t like ICMP echo needs it.
 *  This header is also used for other ICMP types that do not
 *  use the data part.
 */
PACK_STRUCT_BEGIN
struct icmp_echo_hdr {
  PACK_STRUCT_FIELD(u8_t type);
  PACK_STRUCT_FIELD(u8_t code);
  PACK_STRUCT_FIELD(u16_t chksum);
  PACK_STRUCT_FIELD(u16_t id);
  PACK_STRUCT_FIELD(u16_t seqno);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

#define ICMPH_TYPE(hdr) ((hdr)->type)
#define ICMPH_CODE(hdr) ((hdr)->code)

/** Combines type and code to an u16_t */
#define ICMPH_TYPE_SET(hdr, t) ((hdr)->type = (t))
#define ICMPH_CODE_SET(hdr, c) ((hdr)->code = (c))


#if LWIP_ICMP /* don't build if not configured for use in lwipopts.h */

void icmp_input(struct pbuf *p, struct netif *inp);
void icmp_dest_unreach(struct pbuf *p, enum icmp_dur_type t);
void icmp_time_exceeded(struct pbuf *p, enum icmp_te_type t);

#endif /* LWIP_ICMP */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_ICMP_H__ */
