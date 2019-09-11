/*
 * lwip DNS resolver header file.

 * Author: Jim Pettinato
 *   April 2007

 * ported from uIP resolv.c Copyright (c) 2002-2003, Adam Dunkels.
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#ifndef __LWIP_DNS_H__
#define __LWIP_DNS_H__

#include "lwip/opt.h"

#if LWIP_DNS /* don't build if not configured for use in lwipopts.h */

#ifdef __cplusplus
extern "C" {
#endif




/* DNS timer period */
#define DNS_TMR_INTERVAL          1000

/* DNS field TYPE used for "Resource rcrds" */
#define DNS_RRTYPE_A              1     /* a host address */
#define DNS_RRTYPE_NS             2     /* an authoritative name server */
#define DNS_RRTYPE_MD             3     /* a mail destination (Obsolete - use MX) */
#define DNS_RRTYPE_MF             4     /* a mail forwarder (Obsolete - use MX) */
#define DNS_RRTYPE_CNAME          5     /* the canonical name for an alias */
#define DNS_RRTYPE_SOA            6     /* marks the start of a zone of authority */
#define DNS_RRTYPE_MB             7     /* a mailbox domain name (EXPERIMENTAL) */
#define DNS_RRTYPE_MG             8     /* a mail group member (EXPERIMENTAL) */
#define DNS_RRTYPE_MR             9     /* a mail rename domain name (EXPERIMENTAL) */
#define DNS_RRTYPE_NULL           10    /* a null RR (EXPERIMENTAL) */
#define DNS_RRTYPE_WKS            11    /* a well known service description */
#define DNS_RRTYPE_PTR            12    /* a domain name pointer */
#define DNS_RRTYPE_HINFO          13    /* host information */
#define DNS_RRTYPE_MINFO          14    /* mailbox or mail list information */
#define DNS_RRTYPE_MX             15    /* mail exchange */
#define DNS_RRTYPE_TXT            16    /* text strings */

/* DNS field CLASS used for "Resource rcrds" */
#define DNS_RRCLASS_IN            1     /* the Internet */
#define DNS_RRCLASS_CS            2     /* the CSNET class (Obsolete - used only for examples in some obsolete RFCs) */
#define DNS_RRCLASS_CH            3     /* the CHAOS class */
#define DNS_RRCLASS_HS            4     /* Hesiod [Dyer 87] */
#define DNS_RRCLASS_FLUSH         0x800 /* Flush bit */

/* The size used for the next line is rather a hack, but it prevents including socket.h in all files
   that include memp.h, and that would possibly break portability (since socket.h defines some types
   and constants possibly already define by the OS).
   Calculation rule:
   sizeof(struct addrinfo) + sizeof(struct sockaddr_in) + DNS_MAX_NAME_LENGTH + 1 byte zero-termination */
#define NETDB_ELEM_SIZE           (32 + 16 + DNS_MAX_NAME_LENGTH + 1)

#if DNS_LOCAL_HOSTLIST
/* struct used for local host-list */
struct local_hostlist_entry {
  /* static hostname */
  const char *name;
  /* static host address in network byteorder */
  ip_addr_t addr;
  struct local_hostlist_entry *next;
};
#if DNS_LOCAL_HOSTLIST_IS_DYNAMIC
#ifndef DNS_LOCAL_HOSTLIST_MAX_NAMELEN
#define DNS_LOCAL_HOSTLIST_MAX_NAMELEN  DNS_MAX_NAME_LENGTH
#endif
#define LOCALHOSTLIST_ELEM_SIZE ((sizeof(struct local_hostlist_entry) + DNS_LOCAL_HOSTLIST_MAX_NAMELEN + 1))
#endif /* DNS_LOCAL_HOSTLIST_IS_DYNAMIC */
#endif /* DNS_LOCAL_HOSTLIST */


typedef void (*dns_found_callback)(const char *name, ip_addr_t *ipaddr, u32_t count, void *callback_arg);

void           dns_init(void);
void           dns_tmr(void);

void           dns_setserver(u8_t numdns, ip_addr_t *dnsserver);

ip_addr_t      dns_getserver(u8_t numdns);

err_t          dns_gethostbyname(const char *hostname, ip_addr_t *addr, u32_t *count,
                                 dns_found_callback found, void *callback_arg);

#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
int            dns_local_removehost(const char *hostname, const ip_addr_t *addr);
err_t          dns_local_addhost(const char *hostname, const ip_addr_t *addr);
#endif /* DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC */



/** @defgroup DNS_Interfaces
* This section contains the DNS Interfaces.
*/


/*
Func Name:  lwip_dns_setserver
*/
/**
* @defgroup lwip_dns_setserver
* @ingroup DNS_Interfaces
* @par Prototype
* @code
* err_t lwip_dns_setserver(u8_t numdns, ip_addr_t *dnsserver);
* @endcode
*
* @par Purpose
*  This API is used to initialize one of the DNS servers.
*
* @par Description
*  This API is used to set the DNS server IP.
*
* @param[in]    numdns      Index of the DNS server to set [NA]
* @param[in]    dnsserver   IP_add of the DNS server to set [NA]
*
* @par Return values
*  - ERR_OK: On success
*  - ERR_MEM: On failure due to memory
*  - ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* dns.h
*
* @par Note
* 1. Index of the dns server array should be passed as numdns, it should
* be in the range from 0 to (DNS_MAX_SERVERS-1). If its not in the range
* then this API fails with return value ERR_VAL.\n
* 2. If NULL is passed to dnsserver, then IPADDR_ANY is updated.\n
* 3. This is a Thread safe API, this needs to be used in application instead
* of dns_setserver.\n
*
* @par Related Topics
* \n
* N/A
*/

err_t
lwip_dns_setserver(u8_t numdns, ip_addr_t *dnsserver);


/*
Func Name:  lwip_dns_getserver
*/

/**
* @defgroup lwip_dns_getserver
* @ingroup DNS_Interfaces
* @par Prototype
* @code
* err_t lwip_dns_getserver(u8_t numdns, ip_addr_t *dnsserver);
* @endcode
*
* @par Purpose
*  This API is used to obtain one of the configured DNS server.
*
* @par Description
*  This API is used to obtain one of the configured DNS server. This API
*  Gets the DNS server address on the index numdns and updates dnsserver.
*  Application should pass valid pointer of type ip_addr_t as dnsserver.
*
* @param[in]    numdns   index of the DNS server to set [NA]
* @param[out]  dnsserver pointer to IP_add varaible.
*
* @par Return values
*  - ERR_OK: On success
*  - ERR_MEM: On failure due to memory
*  - ERR_VAL: On failure due to Illegal value
*
* @par Required Header File
* dns.h
*
* @par Note
* 1. Index of the dns server array should be passed as numdns, it should
* be in the range from 0 to (DNS_MAX_SERVERS-1). If its not in the range
* then it updates IPADDR_ANY in dnsserver.\n
* 2. This is a Thread safe API, this needs to be used in application instead
* of dns_getserver.\n
*
* @par Related Topics
* \n
* N/A
*/
err_t
lwip_dns_getserver(u8_t numdns, ip_addr_t *dnsserver);



#ifdef __cplusplus
}
#endif

#endif /* LWIP_DNS */

#endif /* __LWIP_DNS_H__ */
