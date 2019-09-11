/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2017>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef __NETIF_BRIDGE_H__
#define __NETIF_BRIDGE_H__

#include "lwip/opt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#if BRIDGE_SUPPORT/* don't build if not configured for use in lwipopts.h */

#include "asm/atomic.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"

//#define BRIDGE_PRINT
#ifdef BRIDGE_PRINT
#define bridge_print   dprintf
#else
#define bridge_print
#endif

#define BRIDGE_HASH_BITS        4
#define BRIDGE_HASH_SIZE        (1 << BRIDGE_HASH_BITS)
#define BRIDGE_AGEING_TIME    300

#define BRIDGE_IPV4        01
#define BRIDGE_IPV6        02
#define BRIDGE_MAC           03

#define MAX_DB_NUM        32

#define ARP_REQUEST 1
#define ARP_REPLY   2

#ifdef BRIDGE_IPV6_PASS
#define MAX_NETWORK_ADDR_LEN    17
#else
#define MAX_NETWORK_ADDR_LEN    11
#endif

#define MAX_SRC_NETWORK_NUM 2

struct network_db_entry
{
    struct network_db_entry *next_hash;
    struct network_db_entry **pprev_hash;
    atomic_t                        use_count;
    unsigned char                 macAddr[6];
    unsigned long                 ageing_timer;
    unsigned int                   used;
    unsigned char                networkAddr[MAX_NETWORK_ADDR_LEN];
    struct netif                   *port;
};

struct lwip_bridge
{
    struct network_db_entry    *nethash[BRIDGE_HASH_SIZE];
    struct network_db_entry    *porthash[BRIDGE_HASH_SIZE];
    struct network_db_entry    *arphash[BRIDGE_HASH_SIZE];
    unsigned char            br_mac[6];
    unsigned char            br_ip[4];
    struct netif *src[MAX_SRC_NETWORK_NUM];
    struct netif *dst;
    bool br_enable;
    bool init_flag;
};

extern struct lwip_bridge g_lwip_bridge;

enum BRIDGE_METHOD {
    BRIDGE_MIN,
    BRIDGE_CHECK,
    BRIDGE_INSERT,
    BRIDGE_LOOKUP,
    BRIDGE_PARSE,
    BRIDGE_MAX
};

enum BRIDGE_STATE {
    BRIDGE_LOCAL,
    BRIDGE_FORWARD,
    BRIDGE_FINISH,
    BRIDGE_ERROR
};

#define ETH_SENDER_IP_ADDR_LEN       4  /* length of an Ethernet send ip address */
#define ETH_TARGET_IP_ADDR_LEN       4  /* length of an Ethernet target ip address */
#define ETHER_ADDR_LEN  6   /* length of an Ethernet address */

#define SERVER_PORT            68
#define CLIENT_PORT            67
#define DHCP_MAGIC            0x63825363

struct eth_hdr_no_align {
  struct eth_addr dest;
  struct eth_addr src;
  u16_t type;
};

struct map_table {
  u8_t ipaddr[4];
  u8_t mac[ETHER_ADDR_LEN];
  struct netif *netif;
  u8_t state;
};

typedef struct eth_arphdr
{
    u16_t us_ar_hrd;   /* format of hardware address */
    u16_t us_ar_pro;   /* format of protocol address */

    u8_t  uc_ar_hln;   /* length of hardware address */
    u8_t  uc_ar_pln;   /* length of protocol address */
    u16_t us_ar_op;    /* ARP opcode (command) */

    u8_t  auc_ar_sha[ETHER_ADDR_LEN];           /* sender hardware address */
    u8_t  auc_ar_sip[ETH_SENDER_IP_ADDR_LEN];   /* sender IP address */
    u8_t  auc_ar_tha[ETHER_ADDR_LEN];           /* target hardware address */
    u8_t  auc_ar_tip[ETH_TARGET_IP_ADDR_LEN];   /* target IP address */
} eth_arphdr;

#define ETHER_TYPE_IP    0x0800  /* IP protocol */
#define ETHER_TYPE_ARP   0x0806  /* ARP protocol */

struct map_table g_map_table[16];

static inline int bridge_hash(unsigned char *netAddr)
{
    unsigned long x = 0;

    if(netAddr[0] == BRIDGE_MAC)
    {
        x = netAddr[1] ^ netAddr[2] ^ netAddr[3] ^ netAddr[4] ^ netAddr[5] ^
        netAddr[6];

        return x & (BRIDGE_HASH_SIZE - 1);
    }
    else if(netAddr[0] == BRIDGE_IPV4)
    {
        x = netAddr[7] ^ netAddr[8] ^ netAddr[9] ^ netAddr[10];

        return x & (BRIDGE_HASH_SIZE - 1);
    }
#ifdef BRIDGE_IPV6_PASS
    else if(netAddr[0] == BRIDGE_IPV6)
    {
        x = netAddr[1] ^ netAddr[2] ^ netAddr[3] ^ netAddr[4] ^ netAddr[5] ^
        netAddr[6] ^ netAddr[7] ^ netAddr[8] ^ netAddr[9] ^ netAddr[10] ^
        netAddr[11] ^ netAddr[12] ^ netAddr[13] ^ netAddr[14] ^ netAddr[15] ^
        netAddr[16];

        return x & (BRIDGE_HASH_SIZE - 1);
    }
#endif
    else
    {
        int i;

        for (i=0; i<MAX_NETWORK_ADDR_LEN; i++)
            x ^= netAddr[i];

        return x & (BRIDGE_HASH_SIZE - 1);
    }
}

static inline void bridge_hash_link(struct network_db_entry *nethash[],
    struct network_db_entry *ent, int hash)
{
    ent->next_hash = nethash[hash];
    if(ent->next_hash != NULL)
        ent->next_hash->pprev_hash = &ent->next_hash;
    nethash[hash] = ent;
    ent->pprev_hash = &nethash[hash];
}

static inline void bridge_hash_unlink(struct network_db_entry *ent)
{
    *(ent->pprev_hash) = ent->next_hash;
    if(ent->next_hash != NULL)
        ent->next_hash->pprev_hash = ent->pprev_hash;
    ent->next_hash = NULL;
    ent->pprev_hash = NULL;
}

static inline void generate_ipv4_network_addr(unsigned char *netAddr, unsigned char *ipAddr)
{
    (void)MEMSET_S(netAddr, MAX_NETWORK_ADDR_LEN, 0, MAX_NETWORK_ADDR_LEN);

    netAddr[0] = BRIDGE_IPV4;
    (void)MEMCPY_S(netAddr + 7, MAX_NETWORK_ADDR_LEN - 7, ipAddr, 4);
}

void bridge_init(void);
int bridge_input(struct netif *netif, struct pbuf *p);
int bridge_output(struct netif *netif, struct pbuf *p);
int bridge_set_forward_rule(char *src_ifn[], int src_count, char *dst_ifn);
void bridge_ip_forward(bool flag); /* TRUE: Enable the Bridge; FALSE: Disable the Bridge */
void bridge_ageing_time_set(unsigned int second);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* __NETIF_BRIDGE_H__ */
