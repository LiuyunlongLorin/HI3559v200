
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



 /**
 *   @file opt.h
 */




/**
* @defgroup Securelib_Interface Securelib Interface
* @ingroup System_interfaces
*
*/

#ifndef __LWIP_OPT_H__
#define __LWIP_OPT_H__

/*
 * Include user defined options first. Anything not defined in these files
 * will be set to standard values. Override anything you dont like!
 */
#include "lwipopts.h"
#include "lwip/debug.h"
#include "sys/select.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "asm/atomic.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include "checksum.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
   -----------------------------------------------
   ---------- Platform specific locking ----------
   -----------------------------------------------
*/

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#ifndef SYS_LIGHTWEIGHT_PROT
#define SYS_LIGHTWEIGHT_PROT            1
#endif

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#ifndef NO_SYS
#define NO_SYS                          0
#endif

/**
 * NO_SYS_NO_TIMERS==1: Drop support for sys_timeout when NO_SYS==1
 * Mainly for compatibility to old versions.
 */
#ifndef NO_SYS_NO_TIMERS
#define NO_SYS_NO_TIMERS                0
#endif


/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory setting.
*/
typedef int  (*PTlwIPFuncMemSet_s)(void *pvDest,
                                        size_t ulDestMax, int Char, size_t ulCount);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory copy. \n
*/
typedef int  (*PTlwIPFuncMemCpy_s)(void *pvDest,
                                  size_t ulDestMax, const void *Src, size_t ulCount);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory N bytes copy. \n
*/
typedef int  (*PTlwIPFuncStrNCpy_s)( char *pcDest, size_t ulDestMax,
                                                                     const char * pcSrc, size_t ulCount );
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory N bytes concatenation.
*/
typedef int (*PTlwIPFuncStrNCat_s)( char *pcDest, size_t ulDestMax,
                                                                    const char *pcSrc, size_t ulCount);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory concatenation.
*/
typedef int (*PTlwIPFuncStrCat_s)( char *pcDest, size_t ulDestMax,
                                                         const char *pcSrc);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for memory move.
*/
typedef int (*PTlwIPFuncMemMove_s)(void *pcDest,
                                 size_t ulDestMax, const void *pcSrc, size_t ulCount);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for string to buffer copy.
*/
typedef int (*PTlwIPFuncSnprintf_s) (char* pcStrDest, size_t ulDestMax,
    size_t ulCount, const char* pszFormat, ...);
/**
* @ingroup Securelib_Interface
* @brief
* Secure callback function for the random number generator.
*/
typedef int (*PTlwIPFuncRand_s)(void);

#define lwIP_TRUE 1
#define lwIP_FALSE 0

/**
* @ingroup Securelib_Interface
* @par Prototype
* @code
*    struct lwip_secure_impl_s
*    {
*      PTlwIPFuncMemSet_s     pfMemset_s;
*      PTlwIPFuncMemCpy_s    pfMemcpy_s;
*      PTlwIPFuncStrNCpy_s     pfStrNCpy_s;
*      PTlwIPFuncStrNCat_s      pfStrNCat_s;
*      PTlwIPFuncStrCat_s        pfStrCat_s;
*      PTlwIPFuncMemMove_s   pfMemMove_s;
*      PTlwIPFuncSnprintf_s      pfSnprintf_s;
*      PTlwIPFuncRand_s        pfRand;
*    };
*
* @endcode
*
* @par Description
* @datastruct PTlwIPFuncMemSet_s      Secure callback function for Memory Setting \n
* @datastruct PTlwIPFuncMemCpy_s     Secure callback function for Memory Copy \n
* @datastruct PTlwIPFuncStrNCpy_s     Secure callback function for Memory N bytes Copy \n
* @datastruct PTlwIPFuncStrNCat_s      Secure callback function for memory N  bytes concatenation \n
* @datastruct PTlwIPFuncStrCat_s        Secure callback function for memory concatenation \n
* @datastruct PTlwIPFuncMemMove_s   Secure callback function for memory move \n
* @datastruct PTlwIPFuncSnprintf_s      Secure callback function for string to buffer copy \n
* @datastruct PTlwIPFuncRand_s          Secure callback function for Random Number Generator
*/
typedef struct lwip_secure_impl_s
{
    PTlwIPFuncMemSet_s     pfMemset_s;  /**< Secure callback function for memory setting. */
    PTlwIPFuncMemCpy_s    pfMemcpy_s;      /**< Secure callback function for memory copy. */
    PTlwIPFuncStrNCpy_s     pfStrNCpy_s;    /**< Secure callback function for memory N bytes copy.  */
    PTlwIPFuncStrNCat_s      pfStrNCat_s;   /**< Secure callback function for memory N  bytes concatenation. */
    PTlwIPFuncStrCat_s        pfStrCat_s;    /**< Secure callback function for memory concatenation.  */
    PTlwIPFuncMemMove_s   pfMemMove_s;      /**< Secure callback function for memory move.  */
    PTlwIPFuncSnprintf_s      pfSnprintf_s;   /**< Secure callback function for string to buffer copy. */
    PTlwIPFuncRand_s        pfRand;            /**< Secure callback function for random number generator. */
}STlwIPSecFuncSsp,*PTlwIPSecFuncSsp;

extern STlwIPSecFuncSsp g_stlwIPSecSspCallbacks;
extern int g_IslwIPSecSspRegistered;
/*
Func Name:  lwIPRegSecSspCbk
*/
/**
* @ingroup Securelib_Interface
* @brief
*  Setting the register callback.
*
* @param[in]    pstSecureFuncSsp      Indicates a pointer to the structure callback.
*
* @returns
*  - ERR_OK: On success
*  - ERR_VAL: On failure due to Illegal value
*/
int lwIPRegSecSspCbk(const PTlwIPSecFuncSsp pstSecureFuncSsp);


/**
 * MEMCPY: Override this if you have a faster implementation at hand than the
 * one included in your C library.
 */

#ifndef MEMMOVE_S
#define MEMMOVE_S   g_stlwIPSecSspCallbacks.pfMemMove_s
#endif


//CSEC_FIX_2302
#ifndef MEMCPY_S
#define MEMCPY_S   g_stlwIPSecSspCallbacks.pfMemcpy_s
#endif

/**
 * SMEMCPY: Override this with care! Some compilers such as gcc can inline a
 * call to Memcpy() if the length is known at compile time and is small.
 */

//CSEC_FIX_2302
#ifndef SMEMCPY_S
#define SMEMCPY_S   g_stlwIPSecSspCallbacks.pfMemcpy_s
#endif

/**
 * STRCPY: Override this if you have a faster implementation at hand than the
 * one included in your C library.
 */

//CSEC_FIX_2302
#ifndef STRNCPY_S
#define STRNCPY_S g_stlwIPSecSspCallbacks.pfStrNCpy_s
#endif

/**
 * MEMSET: Override this if you have a faster implementation at hand than the
 * one included in your C library.
 */
//CSEC_FIX_2302
#ifndef MEMSET_S
#define MEMSET_S    g_stlwIPSecSspCallbacks.pfMemset_s
#endif

/**
 * STRLEN: override this if you have a faster implementation at hand than the
 * one included in your C library
 */
#ifndef STRLEN
#define STRLEN(src)                     strlen(src)
#endif

/**
 * STRNCMP: Override this if you have a faster implementation at hand than the
 * one included in your C library.
 */
#ifndef STRNCMP
#define STRNCMP(dst,src,len)            strncmp(dst,src,len)
#endif

/**
 * STRCMP: Override this if you have a faster implementation at hand than the
 * one included in your C library.
 */

//CSEC_FIX_2302
#ifndef STRCAT_S
#define STRCAT_S       g_stlwIPSecSspCallbacks.pfStrCat_s
#endif

//CSEC_FIX_2302
#ifndef STRNCAT_S
#define STRNCAT_S  g_stlwIPSecSspCallbacks.pfStrNCat_s
#endif

//CSEC_FIX_2302
#ifndef SNPRINTF_S
/* len here excludes NULL character. So pass len+1 to NonSecure fn and len to snprintf_s if added */
#define SNPRINTF_S  g_stlwIPSecSspCallbacks.pfSnprintf_s
#endif

#ifndef LWIP_CONFIG_PARAM
#define LWIP_CONFIG_PARAM 1
#endif

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/**
 * MEM_LIBC_MALLOC==1: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#ifndef MEM_LIBC_MALLOC
#define MEM_LIBC_MALLOC               0
#endif

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#ifndef MEM_SIZE
#define MEM_SIZE                        (512*1024)
#endif

#if MEM_LIBC_MALLOC
#ifndef MEM_PBUF_RAM_SIZE_LIMIT
/* Need to be enabled to enable RAM size limiting when LIBC_MALLOC is enabled */
#define MEM_PBUF_RAM_SIZE_LIMIT        0
#endif
#else
#define MEM_PBUF_RAM_SIZE_LIMIT        0
#endif


/**
* MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
* Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
* speed and usage from interrupts!
*/
#ifndef MEMP_MEM_MALLOC
#define MEMP_MEM_MALLOC                 0
#endif

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#ifndef MEM_ALIGNMENT
#define MEM_ALIGNMENT                   1
#endif

#ifndef PBUF_RAM_SIZE_MIN
#define PBUF_RAM_SIZE_MIN               1024
#endif

/**
 * MAX_PBUF_RAM_SIZE_TO_ALLOC: Maximum length to be passed to pbuf alloc for PBUF_RAM type
 */
#ifndef MAX_PBUF_RAM_SIZE_TO_ALLOC
#define MAX_PBUF_RAM_SIZE_TO_ALLOC      65535
#endif

/**
 * MEM_MALLOC_DMA_ALIGN: the align size of the DMA memory. If set to 1, then not using DMA.
 */
#ifndef MEM_MALLOC_DMA_ALIGN
#define MEM_MALLOC_DMA_ALIGN            64
#endif

/**
 * MEMP_OVERFLOW_CHECK: memp overflow protection reserves a configurable
 * amount of bytes before and after each memp element in every pool and fills
 * it with a prominent default value.
 *    MEMP_OVERFLOW_CHECK == 0 no checking
 *    MEMP_OVERFLOW_CHECK == 1 checks each element when it is freed
 *    MEMP_OVERFLOW_CHECK >= 2 checks each element in every pool every time
 *      memp_malloc() or memp_free() is called (useful but slow!)
 */
#ifndef MEMP_OVERFLOW_CHECK
#define MEMP_OVERFLOW_CHECK             0
#endif

/**
 * MEMP_SANITY_CHECK==1: run a sanity check after each memp_free() to make
 * sure that there are no cycles in the linked lists.
 */
#ifndef MEMP_SANITY_CHECK
#define MEMP_SANITY_CHECK               0
#endif

/**
 * MEM_USE_POOLS==1: Use an alternative to malloc() by allocating from a set
 * of memory pools of various sizes. When mem_malloc is called, an element of
 * the smallest pool that can provide the length needed is returned.
 * To use this, MEMP_USE_CUSTOM_POOLS also has to be enabled.
 */
#ifndef MEM_USE_POOLS
#define MEM_USE_POOLS                   0
#endif

/**
 * MEM_USE_POOLS_TRY_BIGGER_POOL==1: if one malloc-pool is empty, try the next
 * bigger pool - WARNING: THIS MIGHT WASTE MEMORY but it can make a system more
 * reliable. */
#ifndef MEM_USE_POOLS_TRY_BIGGER_POOL
#define MEM_USE_POOLS_TRY_BIGGER_POOL   0
#endif

/**
 * MEMP_USE_CUSTOM_POOLS==1: whether to include a user file lwippools.h
 * that defines additional pools beyond the "standard" ones required
 * by lwIP. If you set this to 1, you must have lwippools.h in your
 * inlude path somewhere.
 */
#ifndef MEMP_USE_CUSTOM_POOLS
#define MEMP_USE_CUSTOM_POOLS           0
#endif

/**
 * Set this to 1 if you want to free PBUF_RAM pbufs (or call mem_free()) from
 * interrupt context (or another context that doesn't allow waiting for a
 * semaphore).
 * If set to 1, mem_malloc will be protected by a semaphore and SYS_ARCH_PROTECT,
 * while mem_free will only use SYS_ARCH_PROTECT. mem_malloc SYS_ARCH_UNPROTECTs
 * with each loop so that mem_free can run.
 *
 * ATTENTION: As you can see from the above description, this leads to dis-/
 * enabling interrupts often, which can be slow! Also, on low memory, mem_malloc
 * can need longer.
 *
 * If you don't want that, at least for NO_SYS=0, you can still use the following
 * functions to enqueue a deallocation call which then runs in the tcpip_thread
 * context:
 * - pbuf_free_callback(p);
 * - mem_free_callback(m);
 */
#ifndef LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0
#endif

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
/**
 * MEMP_NUM_PBUF: the number of memp struct pbufs (used for PBUF_ROM and PBUF_REF).
 * If the application sends a lot of data out of ROM (or other static memory),
 * this should be set high.
 */
#ifndef MEMP_NUM_PBUF
#define MEMP_NUM_PBUF                   16
#endif

/**
 * MEMP_NUM_RAW_PCB: Number of raw connection PCBs
 * (requires the LWIP_RAW option)
 */
#ifndef MEMP_NUM_RAW_PCB
#define MEMP_NUM_RAW_PCB                10
#endif

/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#ifndef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB                16
#endif

/**
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#ifndef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB                16
#endif

/**
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections.
 * (requires the LWIP_TCP option)
 */
#ifndef MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_PCB_LISTEN         16
#endif

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#ifndef MEMP_NUM_TCP_SEG
#define MEMP_NUM_TCP_SEG                16
#endif

/**
 * MEMP_NUM_REASSDATA: the number of IP packets simultaneously queued for
 * reassembly (whole packets, not fragments!)
 */
#ifndef MEMP_NUM_REASSDATA
#define MEMP_NUM_REASSDATA              5
#endif

/**
 * MEMP_NUM_FRAG_PBUF: the number of IP fragments simultaneously sent
 * (fragments, not whole packets!).
 * This is only used with IP_FRAG_USES_STATIC_BUF==0 and
 * LWIP_NETIF_TX_SINGLE_PBUF==0 and only has to be > 1 with DMA-enabled MACs
 * where the packet is not yet sent when netif->output returns.
 */
#ifndef MEMP_NUM_FRAG_PBUF
#define MEMP_NUM_FRAG_PBUF              15
#endif

/**
 * MEMP_NUM_ARP_QUEUE: the number of simultaneously queued outgoing
 * packets (pbufs) that are waiting for an ARP request (to resolve
 * their destination address) to finish.
 * (requires the ARP_QUEUEING option)
 */
#ifndef MEMP_NUM_ARP_QUEUE
#define MEMP_NUM_ARP_QUEUE              30
#endif

/**
 * MEMP_NUM_IGMP_GROUP: The number of multicast groups whose network interfaces
 * can be members et the same time (one per netif - allsystems group -, plus one
 * per netif membership).
 * (requires the LWIP_IGMP option)
 */
#ifndef MEMP_NUM_IGMP_GROUP
#define MEMP_NUM_IGMP_GROUP             8
#endif

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simultaneously active timeouts.
 * (requires NO_SYS==0)
 * The default number of timeouts is calculated here for all enabled modules.
 * The formula expects settings to be either '0' or '1'.
 */
#ifndef MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_SYS_TIMEOUT            (LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS + PPP_SUPPORT + LWIP_TCPIP_TIMEOUT_NUM)
#endif

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#ifndef MEMP_NUM_NETBUF
#define MEMP_NUM_NETBUF                 200
#endif

#ifndef LWIP_NUM_SOCKETS
#define LWIP_NUM_SOCKETS                8
#endif

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#ifndef MEMP_NUM_NETCONN
#define MEMP_NUM_NETCONN                32
#endif

/**
 * MEMP_NUM_TCPIP_MSG_API: the number of struct tcpip_msg, which are used
 * for callback/timeout API communication.
 * (only needed if you use tcpip.c)
 */
#ifndef MEMP_NUM_TCPIP_MSG_API
#define MEMP_NUM_TCPIP_MSG_API          64
#endif

/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * (only needed if you use tcpip.c)
 */
#ifndef MEMP_NUM_TCPIP_MSG_INPKT
#define MEMP_NUM_TCPIP_MSG_INPKT        256
#endif

/**
 * MEMP_NUM_SNMP_NODE: the number of leafs in the SNMP tree.
 */
#ifndef MEMP_NUM_SNMP_NODE
#define MEMP_NUM_SNMP_NODE              50
#endif

/**
 * MEMP_NUM_SNMP_ROOTNODE: the number of branches in the SNMP tree.
 * Every branch has one leaf (MEMP_NUM_SNMP_NODE) at least!
 */
#ifndef MEMP_NUM_SNMP_ROOTNODE
#define MEMP_NUM_SNMP_ROOTNODE          30
#endif

/**
 * MEMP_NUM_SNMP_VARBIND: the number of concurrent requests (does not have to
 * be changed normally) - 2 of these are used per request (1 for input,
 * 1 for output)
 */
#ifndef MEMP_NUM_SNMP_VARBIND
#define MEMP_NUM_SNMP_VARBIND           2
#endif

/**
 * MEMP_NUM_SNMP_VALUE: the number of OID or values concurrently used
 * (does not have to be changed normally) - 3 of these are used per request
 * (1 for the value read and 2 for OIDs - input and output)
 */
#ifndef MEMP_NUM_SNMP_VALUE
#define MEMP_NUM_SNMP_VALUE             3
#endif

/**
 * MEMP_NUM_NETDB: the number of addrinfo resources in netdb mem poll
 */
#ifndef MEMP_NUM_NETDB
#define MEMP_NUM_NETDB                  8
#endif

/**
 * MEMP_NUM_LOCALHOSTLIST: the number of host entries in the local host list
 * if DNS_LOCAL_HOSTLIST_IS_DYNAMIC==1.
 */
#ifndef MEMP_NUM_LOCALHOSTLIST
#define MEMP_NUM_LOCALHOSTLIST          1
#endif

/**
 * MEMP_NUM_PPPOE_INTERFACES: the number of concurrently active PPPoE
 * interfaces (only used with PPPOE_SUPPORT==1)
 */
#ifndef MEMP_NUM_PPPOE_INTERFACES
#define MEMP_NUM_PPPOE_INTERFACES       1
#endif

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE                  16
#endif

#ifndef MAX_LWIP_SOCKET_NUM
#define MAX_LWIP_SOCKET_NUM (FD_SETSIZE - CONFIG_NFILE_DESCRIPTORS)
#endif

/*
   ---------------------------------
   ---------- ARP options ----------
   ---------------------------------
*/
/**
 * LWIP_ARP==1: Enable ARP functionality.
 */
#ifndef LWIP_ARP
#define LWIP_ARP                        1
#endif

#ifndef LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_ARP
#define LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_ARP 1
#endif

/**
 * ARP_TABLE_SIZE: Number of active MAC-IP_add pairs cached.
 */
#ifndef ARP_TABLE_SIZE
#define ARP_TABLE_SIZE                  10
#endif

/**
 * ARP_QUEUEING==1: Multiple outgoing packets are queued during hardware address
 * resolution. By default, only the most recent packet is queued per IP_add
 * This is sufficient for most protocols and mainly reduces TCP connection
 * startup time. Set this to 1 if you know your application sends more than one
 * packet in a row to an IP_add that is not in the ARP cache.
 */
#ifndef ARP_QUEUEING
#define ARP_QUEUEING                    1
#endif

/**
 * ETHARP_TRUST_IP_MAC==1: Incoming IP packets cause the ARP table to be
 * updated with the source MAC and IP_add supplied in the packet.
 * You may want to disable this if you do not trust LAN peers to have the
 * correct addresses, or as a limited approach to attempt to handle
 * spoofing. If disabled, lwIP will need to make a new ARP request if
 * the peer is not already in the ARP table, adding a little latency.
 * The peer *is* in the ARP table if it requested our address before.
 * Also notice that this slows down input processing of every IP packet!
 */
#ifndef ETHARP_TRUST_IP_MAC
#define ETHARP_TRUST_IP_MAC             1
#endif

/**
 * ETHARP_SUPPORT_VLAN==1: support receiving ethernet packets with VLAN header.
 * Additionally, you can define ETHARP_VLAN_CHECK to an u16_t VLAN ID to check.
 * If ETHARP_VLAN_CHECK is defined, only VLAN-traffic for this VLAN is accepted.
 * If ETHARP_VLAN_CHECK is not defined, all traffic is accepted.
 * Alternatively, define a function/define ETHARP_VLAN_CHECK_FN(eth_hdr, vlan)
 * that returns 1 to accept a packet or 0 to drop a packet.
 */
#ifndef ETHARP_SUPPORT_VLAN
#define ETHARP_SUPPORT_VLAN             0
#endif

/** LWIP_ETHERNET==1: enable ethernet support for PPPoE even though ARP
 * might be disabled
 */
#ifndef LWIP_ETHERNET
#define LWIP_ETHERNET                   (LWIP_ARP || PPPOE_SUPPORT)
#endif

/** ETH_PAD_SIZE: number of bytes added before the ethernet header to ensure
 * alignment of payload after that header. Since the header is 14 bytes long,
 * without this padding e.g. addresses in the IP header will not be aligned
 * on a 32-bit boundary, so setting this to 2 can speed up 32-bit-platforms.
 */
#ifndef ETH_PAD_SIZE
#define ETH_PAD_SIZE                    2
#endif

/** ETHARP_SUPPORT_STATIC_ENTRIES==1: enable code to support static ARP table
 * entries (using etharp_add_static_entry/etharp_remove_static_entry).
 */
#ifndef ETHARP_SUPPORT_STATIC_ENTRIES
#define ETHARP_SUPPORT_STATIC_ENTRIES   1
#endif


/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
*/

/**
 * LWIP_IPV4==1: Enable IPv4
 */
#ifndef LWIP_IPV4
#define LWIP_IPV4                       1
#endif


/**
 * IP_FORWARD==1: Enables the ability to forward IP packets across network
 * interfaces. If you are going to run lwIP on a device with only one network
 * interface, define this to 0.
 */
#ifndef IP_FORWARD
#define IP_FORWARD                      0
#endif

/**
 * IP_OPTIONS_ALLOWED: Defines the behavior for IP options.
 *      IP_OPTIONS_ALLOWED==0: All packets with IP options are dropped.
 *      IP_OPTIONS_ALLOWED==1: IP options are allowed (but not parsed).
 */
#ifndef IP_OPTIONS_ALLOWED
#define IP_OPTIONS_ALLOWED              1
#endif

/**
 * IP_REASSEMBLY==1: Reassemble incoming fragmented IP packets. Note that
 * this option does not affect outgoing packet sizes, which can be controlled
 * via IP_FRAG.
 */
#ifndef IP_REASSEMBLY
#define IP_REASSEMBLY                   1
#endif

/**
 * IP_FRAG==1: Fragment outgoing IP packets if their size exceeds MTU. Note
 * that this option does not affect incoming packet sizes, which can be
 * controlled via IP_REASSEMBLY.
 */
#ifndef IP_FRAG
#define IP_FRAG                         1
#endif

/**
 * IP_REASS_MAXAGE: Maximum time (in multiples of IP_TMR_INTERVAL - so seconds, normally)
 * a fragmented IP packet waits for all fragments to arrive. If not all fragments arrived
 * in this time, the whole packet is discarded.
 */
#ifndef IP_REASS_MAXAGE
#define IP_REASS_MAXAGE                 3
#endif

/**
 * IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.
 * Since the received pbufs are enqueued, be sure to configure
 * PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS so that the stack is still able to receive
 * packets even if the maximum amount of fragments is enqueued for reassembly!
 */


#ifndef IP_REASS_MAX_PBUFS
#define IP_REASS_MAX_PBUFS              1408
#endif

/**
 * IP_FRAG_USES_STATIC_BUF==1: Use a static MTU-sized buffer for IP
 * fragmentation. Otherwise, pbufs are allocated and reference the original
 * packet data to be fragmented.
 */
#ifndef IP_FRAG_USES_STATIC_BUF
#define IP_FRAG_USES_STATIC_BUF         0
#endif

/**
 * IP_FRAG_MAX_MTU: Assumed max MTU on any interface for IP frag buffer
 * (requires IP_FRAG_USES_STATIC_BUF==1)
 */
#if IP_FRAG_USES_STATIC_BUF && !defined(IP_FRAG_MAX_MTU)
#define IP_FRAG_MAX_MTU                 1500


#endif
/* As per RFC 791, "Every internet module must be able to forward a datagram of 68
 * octets without further fragmentation.  This is because an internet header
 * may be up to 60 octets, and the minimum fragment is 8 octets." */
#if IP_FRAG_USES_STATIC_BUF && !defined(IP_FRAG_MIN_MTU)
#define IP_FRAG_MIN_MTU                  68
#endif

/**
 * IP_DEFAULT_TTL: Default value for Time-To-Live used by transport layers.
 */
#ifndef IP_DEFAULT_TTL
#define IP_DEFAULT_TTL                  255
#endif

/**
 * IP_SOF_brcast=1: Use the SOF_brcast field to enable brcast
 * filter per pcb on udp and raw send operations. To enable brcast filter
 * on recv operations, you also have to set IP_SOF_brcast_RECV=1.
 */
#ifndef IP_SOF_BROADCAST
#define IP_SOF_BROADCAST                1
#endif

/**
 * IP_FORWARD_ALLOW_TX_ON_RX_NETIF==1: allow ip_forward() to send packets back
 * out on the netif where it was received. This should only be used for
 * wireless networks.
 * ATTENTION: When this is 1, make sure your netif driver correctly marks incoming
 * link-layer-brcast/multicast packets as such using the corresponding pbuf flags!
 */
#ifndef IP_FORWARD_ALLOW_TX_ON_RX_NETIF
#define IP_FORWARD_ALLOW_TX_ON_RX_NETIF 0
#endif

/*
   ----------------------------------
   ---------- ICMP options ----------
   ----------------------------------
*/
/**
 * LWIP_ICMP==1: Enable ICMP module inside the IP stack.
 * Be careful, disable that make your product non-compliant to RFC1122
 */
#ifndef LWIP_ICMP
#define LWIP_ICMP                       1
#endif

/**
 * ICMP_TTL: Default value for Time-To-Live used by ICMP packets.
 */
#ifndef ICMP_TTL
#define ICMP_TTL                       (IP_DEFAULT_TTL)
#endif

/**
 * LWIP_brcast_PING==1: respond to broadcast pings (default is unicast only)
 */
#ifndef LWIP_BROADCAST_PING
#define LWIP_BROADCAST_PING             0
#endif

/**
 * LWIP_MULTICAST_PING==1: respond to multicast pings (default is unicast only)
 */
#ifndef LWIP_MULTICAST_PING
#define LWIP_MULTICAST_PING             0
#endif

/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/
/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#ifndef LWIP_RAW
#define LWIP_RAW                        1
#endif

/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#ifndef RAW_TTL
#define RAW_TTL                        (IP_DEFAULT_TTL)
#endif

/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#ifndef LWIP_DHCP
#define LWIP_DHCP                       1
#endif

#ifndef LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_DHCP
#define LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_DHCP    1
#endif

/**
 * LWIP_TFTP==1: Enable TFTP module.
 */
#ifndef LWIP_TFTP
#define LWIP_TFTP                      1
#endif

#ifndef LWIP_DHCPS
#if LWIP_DHCP
#define LWIP_DHCPS                     1
#else
#define LWIP_DHCPS                     0
#endif
#endif

#ifndef LWIP_DHCPS_MAX_LEASE
#define LWIP_DHCPS_MAX_LEASE 30
#endif

#ifndef LWIP_DHCPS_LEASE_TIME
#define LWIP_DHCPS_LEASE_TIME  ~0
#endif

/* Offer time in seconds */
#ifndef LWIP_DHCPS_OFFER_TIME
#define LWIP_DHCPS_OFFER_TIME 300
#endif

#ifndef LWIP_DHCPS_DECLINE_TIME
#define LWIP_DHCPS_DECLINE_TIME 500
#endif


/**
 * DHCP_DOES_ARP_CHECK==1: Do an ARP check on the offered address.
 */
#ifndef DHCP_DOES_ARP_CHECK
#define DHCP_DOES_ARP_CHECK             ((LWIP_DHCP) && (LWIP_ARP))
#endif

#ifndef LWIP_DHCP_BOOTP_FILE
#define LWIP_DHCP_BOOTP_FILE                       0
#endif

/*
   ------------------------------------
   ---------- AUTOIP options ----------
   ------------------------------------
*/
/**
 * LWIP_AUTOIP==1: Enable AUTOIP module.
 */
#ifndef LWIP_AUTOIP
#define LWIP_AUTOIP                     0
#endif

/**
 * LWIP_DHCP_AUTOIP_COOP==1: Allow DHCP and AUTOIP to be both enabled on
 * the same interface at the same time.
 */
#ifndef LWIP_DHCP_AUTOIP_COOP
#define LWIP_DHCP_AUTOIP_COOP           0
#endif

/**
 * LWIP_DHCP_AUTOIP_COOP_TRIES: Set to the number of DHCP DISCOVER Signals
 * that should be sent before falling back on AUTOIP. This can be set
 * as low as 1 to get an AutoIP_add very quickly, but you should
 * be prepared to handle a changing IP_add when DHCP overrides
 * AutoIP.
 */
#ifndef LWIP_DHCP_AUTOIP_COOP_TRIES
#define LWIP_DHCP_AUTOIP_COOP_TRIES     64
#endif

/*
   ----------------------------------
   ---------- SNMP options ----------
   ----------------------------------
*/
/**
 * LWIP_SNMP==1: Turn on SNMP module. UDP must be available for SNMP
 * transport.
 */
#ifndef LWIP_SNMP
#define LWIP_SNMP                       0
#endif

/**
 * SNMP_CONCURRENT_REQUESTS: Number of concurrent requests the module will
 * allow. At least one request buffer is required.
 * Does not have to be changed unless external MIBs answer request asynchronously.
 */
#ifndef SNMP_CONCURRENT_REQUESTS
#define SNMP_CONCURRENT_REQUESTS        1
#endif

/**
 * SNMP_TRAP_DESTINATIONS: Number of trap destinations. At least one trap
 * destination is required.
 */
#ifndef SNMP_TRAP_DESTINATIONS
#define SNMP_TRAP_DESTINATIONS          1
#endif

/**
 * SNMP_PRIVATE_MIB:
 * When using a private MIB, you have to create a file 'private_mib.h' that contains
 * a 'struct mib_array_node mib_private' which contains your MIB.
 */
#ifndef SNMP_PRIVATE_MIB
#define SNMP_PRIVATE_MIB                0
#endif

/**
 * Only allow SNMP write actions that are 'safe' (e.g. disabling netifs is not
 * a safe action and disabled when SNMP_SAFE_REQUESTS = 1).
 * Unsafe requests are disabled by default!
 */
#ifndef SNMP_SAFE_REQUESTS
#define SNMP_SAFE_REQUESTS              1
#endif

/**
 * The maximum length of strings used. This affects the size of
 * MEMP_SNMP_VALUE elements.
 */
#ifndef SNMP_MAX_OCTET_STRING_LEN
#define SNMP_MAX_OCTET_STRING_LEN       127
#endif

/**
 * The maximum depth of the SNMP tree.
 * With private MIBs enabled, this depends on your MIB!
 * This affects the size of MEMP_SNMP_VALUE elements.
 */
#ifndef SNMP_MAX_TREE_DEPTH
#define SNMP_MAX_TREE_DEPTH             15
#endif

/**
 * The size of the MEMP_SNMP_VALUE elements, normally calculated from
 * SNMP_MAX_OCTET_STRING_LEN and SNMP_MAX_TREE_DEPTH.
 */
#ifndef SNMP_MAX_VALUE_SIZE
#define SNMP_MAX_VALUE_SIZE             LWIP_MAX((SNMP_MAX_OCTET_STRING_LEN)+1, sizeof(s32_t)*(SNMP_MAX_TREE_DEPTH))
#endif

/*
   ----------------------------------
   ---------- IGMP options ----------
   ----------------------------------
*/
/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#ifndef LWIP_IGMP
#define LWIP_IGMP                       0
#endif

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#ifndef LWIP_DNS
#define LWIP_DNS                        1
#endif

/** DNS maximum number of entries to maintain locally. */
#ifndef DNS_TABLE_SIZE
#define DNS_TABLE_SIZE                  4
#endif

/** DNS maximum host name length supported in the name table. */
#ifndef DNS_MAX_NAME_LENGTH
#define DNS_MAX_NAME_LENGTH             256
#endif

/** The maximum of DNS servers */
#ifndef DNS_MAX_SERVERS
#define DNS_MAX_SERVERS                 2
#endif

/** The maximum  */
#ifndef DNS_MAX_LABLE_LENGTH
#define DNS_MAX_LABLE_LENGTH                63
#endif

/**The maximum IP_add supported**/
#ifndef DNS_MAX_IPADDR
#define DNS_MAX_IPADDR                4
#endif

/** DNS do a name checking between the query and the response. */
#ifndef DNS_DOES_NAME_CHECK
#define DNS_DOES_NAME_CHECK             1
#endif

/** DNS use a local buffer if DNS_USES_STATIC_BUF=0, a static one if
    DNS_USES_STATIC_BUF=1, or a dynamic one if DNS_USES_STATIC_BUF=2.
    The buffer will be of size DNS_MSG_SIZE */
#ifndef DNS_USES_STATIC_BUF
#define DNS_USES_STATIC_BUF             1
#endif

/** DNS message max. size. Default value is RFC compliant. */
#ifndef DNS_MSG_SIZE
#define DNS_MSG_SIZE                    512
#endif

/** DNS_LOCAL_HOSTLIST: Implements a local host-to-address list. If enabled,
 *  you have to define
 *    #define DNS_LOCAL_HOSTLIST_INIT {{"host1", 0x123}, {"host2", 0x234}}
 *  (an array of structs name/address, where address is an u32_t in network
 *  byte order).
 *
 *  Instead, you can also use an external function:
 *  #define DNS_LOOKUP_LOCAL_EXTERN(x) extern u32_t my_lookup_function(const char *name)
 *  that returns the IP_add or INADDR_NONE if not found.
 */
#ifndef DNS_LOCAL_HOSTLIST
#define DNS_LOCAL_HOSTLIST              0
#endif /* DNS_LOCAL_HOSTLIST */

/** If this is turned on, the local host-list can be dynamically changed
 *  at runtime. */
#ifndef DNS_LOCAL_HOSTLIST_IS_DYNAMIC
#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC   0
#endif /* DNS_LOCAL_HOSTLIST_IS_DYNAMIC */

/*
   ----------------------------------
   ---------- SNTP options ----------
   ----------------------------------
*/
/**
 * LWIP_SNTP==1: Turn on SNTP module.
 */

#ifndef LWIP_SNTP
#define LWIP_SNTP                      0
#endif

/** SNTP server port */
#ifndef SNTP_PORT
#define SNTP_PORT                   123
#endif

/** Set this to 1 to allow SNTP_SERVER_ADDRESS as domain name. */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS             1
#endif

/** Set the number of SNTP server address need to configured */
#ifndef SNTP_NUM_SERVERS
#define SNTP_NUM_SERVERS             1
#endif

#ifndef SNTP_SERVER_ADDR_SIZE
#define SNTP_SERVER_ADDR_SIZE        255
#endif

/** \def SNTP_SERVER_ADDRESS
 * \brief SNTP server address:
 * - as IPv4 address in "u32_t" format
 * - as a DNS name if SNTP_SERVER_DNS is set to 1
 * May contain multiple server names (For example, "pool.ntp.org","second.time.server")
 */
#ifndef SNTP_SERVER_ADDRESS
#if SNTP_SERVER_DNS
#define SNTP_SERVER_ADDRESS         "pool.ntp.org"
#else
#define SNTP_SERVER_ADDRESS         "192.168.1.2"
#endif
#endif

/** According to the RFC, this shall be a random delay
 * between 1 and 5 minutes (in milliseconds) to prevent load peaks.
 * This can be defined to a random generation function,
 * which must return the delay in milliseconds as u32_t.
 * Turned off by default.
 */
#ifndef SNTP_STARTUP_DELAY
#define SNTP_STARTUP_DELAY          0
#endif

/** SNTP receive timeout - in milliseconds
 * Also used as retry timeout - this shouldn't be too low.
 * The default value is 3 seconds.
 */
#ifndef SNTP_RECV_TIMEOUT
#define SNTP_RECV_TIMEOUT           3000
#endif

/** SNTP update delay - in milliseconds
 * The default value is 1 hour.
 */
#ifndef SNTP_UPDATE_DELAY
#define SNTP_UPDATE_DELAY           3600000
#endif

/**  After SNTP send request to server, retry to server will happen SNTP_MAX_REQUEST_RETRANSMIT times
 * default is set to 3, retransmission will happen thrice after the original transmission.
 */
#ifndef SNTP_MAX_REQUEST_RETRANSMIT
#define SNTP_MAX_REQUEST_RETRANSMIT 3
#endif

/** SNTP macro to change system time and/or the update the RTC clock */
#ifndef SNTP_SET_SYSTEM_TIME
extern int stime(time_t *t);
#define SNTP_SET_SYSTEM_TIME(sec) ((void)stime(&sec))
#endif


/** SNTP macro to change system time including microseconds */
#ifdef SNTP_SET_SYSTEM_TIME_US
#define SNTP_CALC_TIME_US           1
#define SNTP_RECEIVE_TIME_SIZE      2
#else
#define SNTP_SET_SYSTEM_TIME_US(sec, us)
#define SNTP_CALC_TIME_US           0
#define SNTP_RECEIVE_TIME_SIZE      1
#endif

/** Default retry timeout (in milliseconds) if the response
 * received is invalid.
 * This is doubled with each retry until SNTP_RETRY_TIMEOUT_MAX is reached.
 */
#ifndef SNTP_RETRY_TIMEOUT
#define SNTP_RETRY_TIMEOUT          SNTP_RECV_TIMEOUT
#endif

/** Maximum retry timeout (in milliseconds). */
#ifndef SNTP_RETRY_TIMEOUT_MAX
#define SNTP_RETRY_TIMEOUT_MAX      (SNTP_RETRY_TIMEOUT * 10)
#endif

/** Increase retry timeout with every retry sent
 * Default is on to conform to RFC.
 */
#ifndef SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RETRY_TIMEOUT_EXP      1
#endif


/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/
/**
 * LWIP_UDP==1: Turn on UDP.
 */
#ifndef LWIP_UDP
#define LWIP_UDP                        1
#endif

/**
 * LWIP_UDPLITE==1: Turn on UDP-Lite. (Requires LWIP_UDP)
 */
#ifndef LWIP_UDPLITE
#define LWIP_UDPLITE                    0
#endif

/**
 * UDP_TTL: Default Time-To-Live value.
 */
#ifndef UDP_TTL
#define UDP_TTL                         (IP_DEFAULT_TTL)
#endif

/**
 * LWIP_NETBUF_RECVINFO==1: append destination addr and port to every netbuf.
 */
#ifndef LWIP_NETBUF_RECVINFO
#define LWIP_NETBUF_RECVINFO            0
#endif

/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
/**
 * LWIP_TCP==1: Turn on TCP.
 */
#ifndef LWIP_TCP
#define LWIP_TCP                        1
#endif

/**
 * TCP_TTL: Default Time-To-Live value.
 */
#ifndef TCP_TTL
#define TCP_TTL                         (IP_DEFAULT_TTL)
#endif

/**
 * TCP_WND: The size of a TCP window.  This must be at least
 * (2 * TCP_MSS) for things to work well
 * This must be less than 0xFFFF if window scaling is not enabled.
 * This must be less than 0xFFFFFFFF if window scaling is enabled.
 */
#ifndef TCP_WND
#define TCP_WND                         (2*TCP_MSS)
#endif

/**
 * TCP_MAXRTX: Maximum number of retransmissions of data segments.
 */
#ifndef TCP_MAXRTX
#define TCP_MAXRTX                      64
#endif

/**
 * TCP_SYNMAXRTX: Maximum number of retransmissions of SYN segments.
 */
#ifndef TCP_SYNMAXRTX
#define TCP_SYNMAXRTX                   6
#endif

/**
 * TCP_FW1MAXRTX: Maximum number of retransmissions of data segments in FIN_WAIT_1 or CLOSING.
 */
#ifndef TCP_FW1MAXRTX
#define TCP_FW1MAXRTX                   15
#endif

/**
 * TCP_QUEUE_OOSEQ==1: TCP will queue segments that arrive out of order.
 * Define to 0 if your device is low on memory.
 */
#ifndef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ                 (LWIP_TCP)
#endif

/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#ifndef TCP_MSS
#define TCP_MSS                         536
#endif

/**
 * TCP_CALCULATE_EFF_SEND_MSS: "The maximum size of a segment that TCP really
 * sends, the 'effective send MSS,' MUST be the smaller of the send MSS (which
 * reflects the available reassembly buffer size at the remote host) and the
 * largest size permitted by the IP layer" (RFC 1122)
 * Setting this to 1 enables code that checks TCP_MSS against the MTU of the
 * netif used for a connection and limits the MSS if it would be too big otherwise.
 */
#ifndef TCP_CALCULATE_EFF_SEND_MSS
#define TCP_CALCULATE_EFF_SEND_MSS      1
#endif


/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 */
#ifndef TCP_SND_BUF
#define TCP_SND_BUF                     (2 * TCP_MSS)
#endif

/**
 * TCP_OOSEQ_MAX_BYTES: The maximum number of bytes queued on ooseq per pcb.
 * Default is 0 (no limit). Only valid for TCP_QUEUE_OOSEQ==0.
 */
#ifndef TCP_OOSEQ_MAX_BYTES
#define TCP_OOSEQ_MAX_BYTES             0
#endif

/**
 * TCP_OOSEQ_MAX_PBUFS: The maximum number of pbufs queued on ooseq per pcb.
 * Default is 0 (no limit). Only valid for TCP_QUEUE_OOSEQ==0.
 */
#ifndef TCP_OOSEQ_MAX_PBUFS
#define TCP_OOSEQ_MAX_PBUFS             0
#endif

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#ifndef TCP_LISTEN_BACKLOG
#define TCP_LISTEN_BACKLOG              1
#endif

/**
 * The maximum allowed backlog for TCP listen netconns.
 * This backlog is used unless another is explicitly specified.
 */
#ifndef TCP_DEFAULT_LISTEN_BACKLOG
#define TCP_DEFAULT_LISTEN_BACKLOG      16
#endif

/**
 * TCP_OVERSIZE: The maximum number of bytes that tcp_write may
 * allocate ahead of time in an attempt to create shorter pbuf chains
 * for transmission. The meaningful range is 0 to TCP_MSS. Some
 * suggested values are:
 *
 * 0:         Disable oversized allocation. Each tcp_write() allocates a new
              pbuf (old behaviour).
 * 1:         Allocate size-aligned pbufs with minimal excess. Use this if your
 *            scatter-gather DMA requires aligned fragments.
 * 128:       Limit the pbuf/memory overhead to 20%.
 * TCP_MSS:   Try to create unfragmented TCP packets.
 * TCP_MSS/4: Try to create 4 fragments or less per TCP packet.
 */
#ifndef TCP_OVERSIZE
#define TCP_OVERSIZE                    TCP_MSS
#endif

/**
 * LWIP_TCP_TIMESTAMPS==1: support the TCP timestamp option.
 */
#ifndef LWIP_TCP_TIMESTAMPS
#define LWIP_TCP_TIMESTAMPS             0
#endif

/**
 * TCP_WND_UPDATE_THRESHOLD: difference in window to trigger an
 * explicit window update
 * Only dividing factor should be kept here.
 * (LWIP_WND/TCP_WND_UPDATE_THRESHOLD) to measure actual threshold value.
 */
#ifndef TCP_WND_UPDATE_THRESHOLD
#define TCP_WND_UPDATE_THRESHOLD   4
#endif

/**
 * LWIP_EVENT_API and LWIP_CALLBACK_API: Only one of these should be set to 1.
 *     LWIP_EVENT_API==1: The user defines lwip_tcp_event() to receive all
 *         events (accept, sent, etc) that happen in the system.
 *     LWIP_CALLBACK_API==1: The PCB callback function is called directly
 *         for the event. This is the default.
 */
#if !defined(LWIP_EVENT_API) && !defined(LWIP_CALLBACK_API)
#define LWIP_EVENT_API                  0
#define LWIP_CALLBACK_API               1
#endif

/*
 * LWIP_SACK==1: support the TCP SACK option.
 */
#ifndef LWIP_SACK
#define LWIP_SACK                   0
#endif


#ifndef LWIP_SACK_DATA_SEG_PIGGYBACK
#define LWIP_SACK_DATA_SEG_PIGGYBACK      0
#endif



/**
 * LWIP_WND_SCALE==1: support the TCP Window Scaling option.
 */
#ifndef LWIP_WND_SCALE
#define LWIP_WND_SCALE  1
#endif

#if LWIP_WND_SCALE

/**
 * TCP_RCV_SCALE - This holds the shift count value for Window scale
 */
#ifndef TCP_RCV_SCALE
#define TCP_RCV_SCALE   7
#endif

/**
 * TCP_WND_MIN  -If window scaling is enabled then this minimum receive window also
 * should be configured. If peer does not support the  window scaling
 * option, this minimum receive window will be considered.
 * This value should not be greater than 0XFFFF and this value should not
 * be more than TCP_WND.
 */
#ifndef TCP_WND_MIN
#define TCP_WND_MIN        32768
#endif

#endif

/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/
/**
 * PBUF_LINK_HLEN: the number of bytes that should be allocated for a
 * link level header. The default is 14, the standard value for
 * Ethernet.
 */
#ifndef PBUF_LINK_HLEN
#define PBUF_LINK_HLEN                  (14 + ETH_PAD_SIZE)
#endif

/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accomodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 */
#ifndef PBUF_POOL_BUFSIZE
#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_HLEN)
#endif

/**
* PBUF_MAC_HLEN: the maximum number of bytes that should be allocated for a
* MAC level header. The default is 64, the standard value for
* Wifi MAC header.
*/
#ifndef PBUF_MAC_HLEN
#define PBUF_MAC_HLEN                   (84 - PBUF_LINK_HLEN)
#endif


/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/
/**
 * LWIP_NETIF_HOSTNAME==1: use DHCP_OPTION_HOSTNAME with netif's hostname
 * field.
 */
#ifndef LWIP_NETIF_HOSTNAME
#define LWIP_NETIF_HOSTNAME             1
#endif

/**
 * LWIP_NETIF_HOSTNAME_DEFAULT: netif's hostname after init
 * field.
 */
#ifndef LWIP_NETIF_HOSTNAME_DEFAULT
#define LWIP_NETIF_HOSTNAME_DEFAULT     "DEFAULT"
#endif

/**
 * LWIP_NETIF_API==1: Support netif api (in netifapi.c)
 */
#ifndef LWIP_NETIF_API
#define LWIP_NETIF_API                  1
#endif

/**
 * LWIP_TCPIP_CALLBACKMSG==1: Support sending static msg from interrupt context (in tcpip.c)
 */
#ifndef LWIP_TCPIP_CALLBACKMSG
#define LWIP_TCPIP_CALLBACKMSG       0
#endif

/**
 * LWIP_NETIF_STATUS_CALLBACK==1: Support a callback function whenever an interface
 * changes its up/down status (That is, due to DHCP IP acquisition)
 */
#ifndef LWIP_NETIF_STATUS_CALLBACK
#define LWIP_NETIF_STATUS_CALLBACK      0
#endif

/**
 * LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
 * whenever the link changes (That is, link down)
 */
#ifndef LWIP_NETIF_LINK_CALLBACK
#define LWIP_NETIF_LINK_CALLBACK        1
#endif

/**
 * LWIP_NETIF_REMOVE_CALLBACK==1: Support a callback function that is called
 * when a netif has been removed
 */
#ifndef LWIP_NETIF_REMOVE_CALLBACK
#define LWIP_NETIF_REMOVE_CALLBACK      0
#endif

/**
 * LWIP_NETIF_HWADDRHINT==1: Cache link-layer-address hints (e.g. table
 * indices) in struct netif. TCP and UDP can make use of this to prevent
 * scanning the ARP table for every sent packet. While this is faster for big
 * ARP tables or many concurrent connections, it might be counter productive
 * if you have a tiny ARP table or if there never are concurrent connections.
 */
#ifndef LWIP_NETIF_HWADDRHINT
#define LWIP_NETIF_HWADDRHINT           0
#endif

/**
 * LWIP_NETIF_LOOPBACK==1: Support sending packets with a destination IP
 * address equal to the netif IP_add, looping them back up the stack.
 */
#ifndef LWIP_NETIF_LOOPBACK
#define LWIP_NETIF_LOOPBACK             0
#endif

/**
 * LWIP_NETIF_INDEX_MAX==10: Support sending packets with a destination IP
 * address equal to the netif IP_add, looping them back up the stack.
 */
#ifndef LWIP_NETIF_INDEX_MAX
#define LWIP_NETIF_INDEX_MAX            10
#endif

/**
 * LWIP_LOOPBACK_MAX_PBUFS: Maximum number of pbufs on queue for loopback
 * sending for each netif (0 = disabled)
 */
#ifndef LWIP_LOOPBACK_MAX_PBUFS
#define LWIP_LOOPBACK_MAX_PBUFS         0
#endif

/**
 * LWIP_NETIF_LOOPBACK_MULTITHREADING: Indicates whether threading is enabled in
 * the system, as netifs must change how they behave depending on this setting
 * for the LWIP_NETIF_LOOPBACK option to work.
 * Setting this is needed to avoid reentering non-reentrant functions like
 * tcp_input().
 *    LWIP_NETIF_LOOPBACK_MULTITHREADING==1: Indicates that the user is using a
 *       multithreaded environment like tcpip.c. In this case, netif->input()
 *       is called directly.
 *    LWIP_NETIF_LOOPBACK_MULTITHREADING==0: Indicates a polling (or NO_SYS) setup.
 *       The packets are put on a list and netif_poll() must be called in
 *       the main application loop.
 */
#ifndef LWIP_NETIF_LOOPBACK_MULTITHREADING
#define LWIP_NETIF_LOOPBACK_MULTITHREADING    (!NO_SYS)
#endif

/**
 * LWIP_NETIF_TX_SINGLE_PBUF: if this is set to 1, lwIP tries to put all data
 * to be sent into one single pbuf. This is for compatibility with DMA-enabled
 * MACs that do not support scatter-gather.
 * Beware that this might involve CPU-memcpy_s before transmitting that would not
 * be needed without this flag! Use this only if you need to!
 *
 * @todo: TCP and IP-frag do not work with this, yet:
 */
#ifndef LWIP_NETIF_TX_SINGLE_PBUF
#define LWIP_NETIF_TX_SINGLE_PBUF             1
#endif /* LWIP_NETIF_TX_SINGLE_PBUF */

/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/
/**
 * LWIP_HAVE_LOOPIF==1: Support loop interface (127.0.0.1) and loopif.c
 */
#ifndef LWIP_HAVE_LOOPIF
#define LWIP_HAVE_LOOPIF                0
#endif

/*
   ------------------------------------
   ---------- SLIPIF options ----------
   ------------------------------------
*/
/**
 * LWIP_HAVE_SLIPIF==1: Support slip interface and slipif.c
 */
#ifndef LWIP_HAVE_SLIPIF
#define LWIP_HAVE_SLIPIF                0
#endif

/*
   ------------------------------------
   ---------- Thread options ----------
   ------------------------------------
*/
/**
 * TCPIP_THREAD_NAME: The name assigned to the main tcpip thread.
 */
#ifndef TCPIP_THREAD_NAME
#define TCPIP_THREAD_NAME              "tcpip_thread"
#endif

/**
 * TCPIP_THREAD_STACKSIZE: The stack size used by the main tcpip thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef TCPIP_THREAD_STACKSIZE
#define TCPIP_THREAD_STACKSIZE          0
#endif

/**
 * TCPIP_THREAD_PRIO: The priority assigned to the main tcpip thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef TCPIP_THREAD_PRIO
#define TCPIP_THREAD_PRIO               1
#endif

/**
 * SLIPIF_THREAD_NAME: The name assigned to the slipif_loop thread.
 */
#ifndef SLIPIF_THREAD_NAME
#define SLIPIF_THREAD_NAME             "slipif_loop"
#endif

/**
 * SLIP_THREAD_STACKSIZE: The stack size used by the slipif_loop thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef SLIPIF_THREAD_STACKSIZE
#define SLIPIF_THREAD_STACKSIZE         0
#endif

/**
 * SLIPIF_THREAD_PRIO: The priority assigned to the slipif_loop thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef SLIPIF_THREAD_PRIO
#define SLIPIF_THREAD_PRIO              1
#endif

/**
 * PPP_THREAD_NAME: The name assigned to the pppInputThread.
 */
#ifndef PPP_THREAD_NAME
#define PPP_THREAD_NAME                "pppInputThread"
#endif

/**
 * PPP_THREAD_STACKSIZE: The stack size used by the pppInputThread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef PPP_THREAD_STACKSIZE
#define PPP_THREAD_STACKSIZE            0x1000
#endif

/**
 * PPP_THREAD_PRIO: The priority assigned to the pppInputThread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef PPP_THREAD_PRIO
#define PPP_THREAD_PRIO                 10
#endif

/**
 * DEFAULT_THREAD_NAME: The name assigned to any other lwIP thread.
 */
#ifndef DEFAULT_THREAD_NAME
#define DEFAULT_THREAD_NAME            "lwIP"
#endif

/**
 * DEFAULT_THREAD_STACKSIZE: The stack size used by any other lwIP thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef DEFAULT_THREAD_STACKSIZE
#define DEFAULT_THREAD_STACKSIZE        0
#endif

/**
 * DEFAULT_THREAD_PRIO: The priority assigned to any other lwIP thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef DEFAULT_THREAD_PRIO
#define DEFAULT_THREAD_PRIO             1
#endif

/**
 * DEFAULT_RAW_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_RAW/ NETCONN_PKT_RAW. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#ifndef DEFAULT_RAW_RECVMBOX_SIZE
#define DEFAULT_RAW_RECVMBOX_SIZE       0
#endif

/**
 * DEFAULT_UDP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_UDP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#ifndef DEFAULT_UDP_RECVMBOX_SIZE
#define DEFAULT_UDP_RECVMBOX_SIZE       0
#endif

/**
 * DEFAULT_TCP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_TCP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#ifndef DEFAULT_TCP_RECVMBOX_SIZE
#define DEFAULT_TCP_RECVMBOX_SIZE       0
#endif

/**
 * DEFAULT_ACCEPTMBOX_SIZE: The mailbox size for the incoming connections.
 * The queue size value itself is platform-dependent, but is passed to
 * sys_mbox_new() when the acceptmbox is created.
 */
#ifndef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         0
#endif

/**
 * MAX_RECVMBOX_SIZE: The max mailbox size for the incoming packets on a
 * NETCONN_*. make it small to save memory.
 */
#ifndef MAX_MBOX_SIZE
#define MAX_MBOX_SIZE                   2048
#endif

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_TCPIP_CORE_LOCKING: (EXPERIMENTAL!)
 * Don't use it if you're not an active lwIP project member
 */
#ifndef LWIP_TCPIP_CORE_LOCKING
#define LWIP_TCPIP_CORE_LOCKING         0
#endif

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT: (EXPERIMENTAL!)
 * Don't use it if you're not an active lwIP project member
 */
#ifndef LWIP_TCPIP_CORE_LOCKING_INPUT
#define LWIP_TCPIP_CORE_LOCKING_INPUT   0
#endif

/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#ifndef LWIP_NETCONN
#define LWIP_NETCONN                    1
#endif

/** LWIP_TCPIP_TIMEOUT==1: Enable sys_timeout/sys_untimeout to create
 * timers running in tcpip_thread from another thread.
 */
#ifndef LWIP_TCPIP_TIMEOUT
#define LWIP_TCPIP_TIMEOUT              1
#endif

/** LWIP_TCPIP_TIMEOUT_NUM==1: This is newly added to increase the memory pool count
 * for the application timer. This needs to be kept as the number of timer application wants to configure
 */

#ifndef LWIP_TCPIP_TIMEOUT_NUM
#define LWIP_TCPIP_TIMEOUT_NUM              1
#endif

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                     1
#endif

#ifndef LWIP_SOCKET_START_NUM
#define LWIP_SOCKET_START_NUM 0
#endif

/**
 * LWIP_COMPAT_SOCKETS==1: Enable BSD-style sockets functions names.
 * (only used if you use sockets.c)
 */
#ifndef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS             1
#endif

/**
 * LWIP_TCP_KEEPALIVE==1: Enable TCP_KEEPIDLE, TCP_KEEPINTVL and TCP_KEEPCNT
 * options processing. Note that TCP_KEEPIDLE and TCP_KEEPINTVL have to be set
 * in seconds. (does not require sockets.c, and will affect tcp.c)
 */
#ifndef LWIP_TCP_KEEPALIVE
#define LWIP_TCP_KEEPALIVE              1
#endif

/**
 * LWIP_SO_SNDTIMEO==1: Enable send timeout for sockets/netconns and
 * SO_SNDTIMEO processing.
 */
#ifndef LWIP_SO_SNDTIMEO
#define LWIP_SO_SNDTIMEO                0
#endif

/**
 * LWIP_SO_RCVTIMEO==1: Enable receive timeout for sockets/netconns and
 * SO_RCVTIMEO processing.
 */
#ifndef LWIP_SO_RCVTIMEO
#define LWIP_SO_RCVTIMEO                0
#endif

/**
 * LWIP_SO_RCVBUF==1: Enable SO_RCVBUF processing.
 */
#ifndef LWIP_SO_RCVBUF
#define LWIP_SO_RCVBUF                  1
#endif

/**
 * LWIP_SO_SNDBUF==1: Enable SO_SNDBUF processing.
 * only TCP socket in CLOSED or ESTABLISHED state supports settung sndbuf now.
 */
#ifndef LWIP_SO_SNDBUF
#define LWIP_SO_SNDBUF                  1
#endif

/**
 * If LWIP_SO_RCVBUF is used, this is the default value for recv_bufsize.
 */
#ifndef RECV_BUFSIZE_DEFAULT
#define RECV_BUFSIZE_DEFAULT            65535
#endif

#ifndef RECV_BUFSIZE_MIN
#define RECV_BUFSIZE_MIN                256
#endif

#ifndef SEND_BUFSIZE_MIN
#define SEND_BUFSIZE_MIN                (TCP_MSS*2)
#endif

#if LWIP_WND_SCALE
#ifndef SEND_BUFSIZE_MAX
#define SEND_BUFSIZE_MAX                0x40000
#endif
#else
#ifndef SEND_BUFSIZE_MAX
#define SEND_BUFSIZE_MAX                65535
#endif
#endif /* LWIP_WND_SCALE */

/**
 * SO_REUSE==1: Enable SO_REUSEADDR option.
 */
#ifndef SO_REUSE
#define SO_REUSE                        0
#endif

/**
 * SO_REUSE_RXTOALL==1: Pass a copy of incoming brcast/multicast packets
 * to all local matches if SO_REUSEADDR is turned on.
 * WARNING: Adds a MEMCPY_S for every packet if passing to more than one pcb!
 */
#ifndef SO_REUSE_RXTOALL
#define SO_REUSE_RXTOALL                1
#endif

/*
   ---------------------------------------
   ------------- Utitlity APIs ---------------
   ---------------------------------------
*/

#ifndef LWIP_ENABLE_LOS_SHELL_CMD
#define LWIP_ENABLE_LOS_SHELL_CMD       0
#endif


/* To configure how many times ping command has to send echo msg */
#ifndef LWIP_SHELL_CMD_PING_RETRY_TIMES
#define LWIP_SHELL_CMD_PING_RETRY_TIMES     4
#endif

/* Ping cmd waiting timeout (in millisec) to receive ping response */
#ifndef LWIP_SHELL_CMD_PING_TIMEOUT
#define LWIP_SHELL_CMD_PING_TIMEOUT     2000
#endif

/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/
/**
 * LWIP_STATS==1: Enable statistics gathering in lwip_stats.
 */
#ifndef LWIP_STATS
#define LWIP_STATS                      1
#endif

#if LWIP_STATS

/**
 * LWIP_STATS_DISPLAY==1: Compile in the statistics output functions.
 */
#ifndef LWIP_STATS_DISPLAY
#define LWIP_STATS_DISPLAY              1
#endif

/**
 * LINK_STATS==1: Enable link stats.
 */
#ifndef LINK_STATS
#define LINK_STATS                      1
#endif

/**
 * ETHARP_STATS==1: Enable etharp stats.
 */
#ifndef ETHARP_STATS
#define ETHARP_STATS                    (LWIP_ARP)
#endif

/**
 * IP_STATS==1: Enable IP stats.
 */
#ifndef IP_STATS
#define IP_STATS                        1
#endif

/**
 * IPFRAG_STATS==1: Enable IP fragmentation stats. Default is
 * on if using either frag or reass.
 */
#ifndef IPFRAG_STATS
#define IPFRAG_STATS                    (IP_REASSEMBLY || IP_FRAG)
#endif

/**
 * ICMP_STATS==1: Enable ICMP stats.
 */
#ifndef ICMP_STATS
#define ICMP_STATS                      1
#endif

/**
 * IGMP_STATS==1: Enable IGMP stats.
 */
#ifndef IGMP_STATS
#define IGMP_STATS                      (LWIP_IGMP)
#endif

/**
 * UDP_STATS==1: Enable UDP stats. Default is on if
 * UDP enabled, otherwise off.
 */
#ifndef UDP_STATS
#define UDP_STATS                       (LWIP_UDP)
#endif

/**
 * TCP_STATS==1: Enable TCP stats. Default is on if TCP
 * enabled, otherwise off.
 */
#ifndef TCP_STATS
#define TCP_STATS                       (LWIP_TCP)
#endif

/**
 * MEM_STATS==1: Enable mem.c stats.
 */
#ifndef MEM_STATS
#define MEM_STATS                       ((MEM_LIBC_MALLOC == 0) && (MEM_USE_POOLS == 0))
#endif

/**
 * MEMP_STATS==1: Enable memp.c pool stats.
 */
#ifndef MEMP_STATS
#define MEMP_STATS                      (MEMP_MEM_MALLOC == 0)
#endif

/**
 * SYS_STATS==1: Enable system stats (sem and mbox counts, etc).
 */
#ifndef SYS_STATS
#define SYS_STATS                       (NO_SYS == 0)
#endif

#else

#define LINK_STATS                      0
#define IP_STATS                        0
#define ETHARP_STATS                    0
#define IPFRAG_STATS                    0
#define ICMP_STATS                      0
#define IGMP_STATS                      0
#define UDP_STATS                       0
#define TCP_STATS                       0
#define MEM_STATS                       0
#define MEMP_STATS                      0
#define SYS_STATS                       0
#define LWIP_STATS_DISPLAY              0

#endif /* LWIP_STATS */

/*
   ----------------------------------
   ---------- BIRDGE options -----------
   ----------------------------------
*/
/**
 * BRIDGE_SUPPORT==1: Turn on BRIDGE module.
 */
#ifndef BRIDGE_SUPPORT
#define BRIDGE_SUPPORT                        1
#endif

/*
   ---------------------------------
   ---------- PPP options ----------
   ---------------------------------
*/
/**
 * LWIP_PPPOE==1: Turn on PPPoE module.
 */

#ifndef LWIP_PPPOE
#define LWIP_PPPOE                      1
#endif

/**
 * PPP_SUPPORT==1: Enable PPP.
 */
#ifndef PPP_SUPPORT
#define PPP_SUPPORT                     LWIP_PPPOE
#endif

/**
 * PPPOE_SUPPORT==1: Enable PPP Over Ethernet
 */
#ifndef PPPOE_SUPPORT
#define PPPOE_SUPPORT                   LWIP_PPPOE
#endif

/**
 * PPPOS_SUPPORT==1: Enable PPP Over Serial
 */
#ifndef PPPOS_SUPPORT
#define PPPOS_SUPPORT                   0
#endif

#if PPP_SUPPORT

/**
 * NUM_PPP: Max PPP sessions.
 */
#ifndef NUM_PPP
#define NUM_PPP                         1
#endif

/**
 * PAP_SUPPORT==1: Support PAP.
 * be aware that user and password was not encrypted in PAP Auth packet, so
 * this Auth machanism was not safe and LwIP disable it by default.
 */
#ifndef PAP_SUPPORT
#define PAP_SUPPORT                     0
#endif

/**
 * CHAP_SUPPORT==1: Support CHAP.
 */
#ifndef CHAP_SUPPORT
#define CHAP_SUPPORT                    1
#endif

/**
 * MSCHAP_SUPPORT==1: Support MSCHAP. CURRENTLY NOT SUPPORTED! DO NOT SET!
 */
#ifndef MSCHAP_SUPPORT
#define MSCHAP_SUPPORT                  0
#endif

/**
 * CBCP_SUPPORT==1: Support CBCP. CURRENTLY NOT SUPPORTED! DO NOT SET!
 */
#ifndef CBCP_SUPPORT
#define CBCP_SUPPORT                    0
#endif

/**
 * CCP_SUPPORT==1: Support CCP. CURRENTLY NOT SUPPORTED! DO NOT SET!
 */
#ifndef CCP_SUPPORT
#define CCP_SUPPORT                     0
#endif

/**
 * VJ_SUPPORT==1: Support VJ header compression.
 */
#ifndef VJ_SUPPORT
#define VJ_SUPPORT                      0
#endif

/**
 * M_D_5_SUPPORT==1: Support M_D_5 (see also CHAP).
 */
#ifndef MD5_SUPPORT
#define MD5_SUPPORT                     0
#endif

/*
 * Timeouts
 */
#ifndef FSM_DEFTIMEOUT
#define FSM_DEFTIMEOUT                  6       /* Timeout time in seconds */
#endif

#ifndef FSM_DEFMAXTERMREQS
#define FSM_DEFMAXTERMREQS              2       /* Maximum Terminate-Request transmissions */
#endif

#ifndef FSM_DEFMAXCONFREQS
#define FSM_DEFMAXCONFREQS              10      /* Maximum Configure-Request transmissions */
#endif

#ifndef FSM_DEFMAXNAKLOOPS
#define FSM_DEFMAXNAKLOOPS              5       /* Maximum number of nak loops */
#endif

#ifndef UPAP_DEFTIMEOUT
#define UPAP_DEFTIMEOUT                 6       /* Timeout (seconds) for retransmitting req */
#endif

#ifndef UPAP_DEFREQTIME
#define UPAP_DEFREQTIME                 30      /* Time to wait for auth-req from peer */
#endif

#ifndef CHAP_DEFTIMEOUT
#define CHAP_DEFTIMEOUT                 6       /* Timeout time in seconds */
#endif

#ifndef CHAP_DEFTRANSMITS
#define CHAP_DEFTRANSMITS               10      /* max # times to send challenge */
#endif

/* Interval in seconds between keepalive echo requests, 0 to disable. */
#ifndef LCP_ECHOINTERVAL
#define LCP_ECHOINTERVAL                0
#endif

/* Number of unanswered echo requests before failure. */
#ifndef LCP_MAXECHOFAILS
#define LCP_MAXECHOFAILS                3
#endif

/* Max Xmit idle time (in jiffies) before resend flag char. */
#ifndef PPP_MAXIDLEFLAG
#define PPP_MAXIDLEFLAG                 100
#endif

/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define PPP_MTU                         1500     /* Default MTU (size of Info field) */
#ifndef PPP_MAXMTU
/* #define PPP_MAXMTU  65535 - (PPP_HDRLEN + PPP_FCSLEN) */
#define PPP_MAXMTU                      1500 /* Largest MTU we allow */
#endif
#define PPP_MINMTU                      64
#define PPP_MRU                         1500     /* default MRU = max length of info field */
#define PPP_MAXMRU                      1500     /* Largest MRU we allow */
#ifndef PPP_DEFMRU
#define PPP_DEFMRU                      296             /* Try for this */
#endif
#define PPP_MINMRU                      128             /* No MRUs below this */

#ifndef MAXNAMELEN
#define MAXNAMELEN                      256     /* max length of hostname or name for auth */
#endif
#ifndef MAXSECRETLEN
#define MAXSECRETLEN                    256     /* max length of password or secret */
#endif

#endif /* PPP_SUPPORT */

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
/**
 * LWIP_CHKSUM_COPY_ALGORITHM==1: Choose checksum algorithm.
 */
#ifndef LWIP_CHKSUM_COPY_ALGORITHM
#define LWIP_CHKSUM_COPY_ALGORITHM  1
#endif

/**
 * CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.
 */
#ifndef CHECKSUM_GEN_IP
#define CHECKSUM_GEN_IP                 1
#endif

/**
 * CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.
 */
#ifndef CHECKSUM_GEN_UDP
#define CHECKSUM_GEN_UDP                1
#endif

/**
 * CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.
 */
#ifndef CHECKSUM_GEN_TCP
#define CHECKSUM_GEN_TCP                1
#endif

/**
 * CHECKSUM_GEN_ICMP==1: Generate checksums in software for outgoing ICMP packets.
 */
#ifndef CHECKSUM_GEN_ICMP
#define CHECKSUM_GEN_ICMP               1
#endif

/**
 * CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.
 */
#ifndef CHECKSUM_CHECK_IP
#define CHECKSUM_CHECK_IP               1
#endif

/**
 * CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.
 */
#ifndef CHECKSUM_CHECK_UDP
#define CHECKSUM_CHECK_UDP              1
#endif

/**
 * CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.
 */
#ifndef CHECKSUM_CHECK_TCP
#define CHECKSUM_CHECK_TCP              1
#endif

/**
 * LWIP_CHECKSUM_ON_COPY==1: Calculate checksum when copying data from
 * application buffers to pbufs.
 */
#ifndef LWIP_CHECKSUM_ON_COPY
#define LWIP_CHECKSUM_ON_COPY           0
#endif

/**
 * LWIP_TX_CSUM_OFFLOAD == 1: Enable TCP/UDP Tx checksum offload.
 */
#ifndef LWIP_TX_CSUM_OFFLOAD
#define LWIP_TX_CSUM_OFFLOAD           1
#endif

/* caculate checksum by software */
#define CHECKSUM_NONE                   0
/* calculate checksum by hardware, but software should calculate the pseduo header*/
#define CHECKSUM_PARTIAL                3

/*
   ---------------------------------------
   ---------- Hook options ---------------
   ---------------------------------------
*/

/* Hooks are undefined by default, define them to a function if you need them. */

/**
 * LWIP_HOOK_IP4_INPUT(pbuf, input_netif):
 * - called from ip_input() (IPv4)
 * - pbuf: received struct pbuf passed to ip_input()
 * - input_netif: struct netif on which the packet has been received
 * Return values:
 * - 0: Hook has not consumed the packet, packet is processed as normal
 * - != 0: Hook has consumed the packet.
 * If the hook consumed the packet, 'pbuf' is in the responsibility of the hook
 * (i.e. free it when done).
 */

/**
 * LWIP_HOOK_IP4_ROUTE(dest):
 * - called from ip_route() (IPv4)
 * - dest: destination IPv4 address
 * Returns the destination netif or NULL if no destination netif is found. In
 * that case, ip_route() continues as normal.
 */

/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/

/**
 * LWIP_DBG_MIN_LEVEL: After masking, the value of the dbg is
 * compared against this value. If it is smaller, then debugging
 * messages are written.
 */
#ifndef LWIP_DBG_MIN_LEVEL
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_OFF
#endif

/**
 * LWIP_DBG_TYPES_ON: A mask that can be used to globally enable/disable
 * dbg messages of certain types.
 */
#ifndef LWIP_DBG_TYPES_ON
#define LWIP_DBG_TYPES_ON               LWIP_DBG_OFF
#endif

/**
 * ETHARP_DEBUG: Enable debugging in etharp.c.
 */
#ifndef ETHARP_DEBUG
#define ETHARP_DEBUG                    LWIP_DBG_OFF
#endif

/**
 * NETIF_DEBUG: Enable debugging in netif.c.
 */
#ifndef NETIF_DEBUG
#define NETIF_DEBUG                     LWIP_DBG_OFF
#endif

/**
 * DRIVERIF_DEBUG: Enable debugging in driverif.c.
 */
#ifndef DRIVERIF_DEBUG
#define DRIVERIF_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * PBUF_DEBUG: Enable debugging in pbuf.c.
 */
#ifndef PBUF_DEBUG
#define PBUF_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * API_LIB_DEBUG: Enable debugging in api_lib.c.
 */
#ifndef API_LIB_DEBUG
#define API_LIB_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * API_MSG_DEBUG: Enable debugging in api_msg.c.
 */
#ifndef API_MSG_DEBUG
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * SOCKETS_DEBUG: Enable debugging in sockets.c.
 */
#ifndef SOCKETS_DEBUG
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * ICMP_DEBUG: Enable debugging in icmp.c.
 */
#ifndef ICMP_DEBUG
#define ICMP_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * IGMP_DEBUG: Enable debugging in igmp.c.
 */
#ifndef IGMP_DEBUG
#define IGMP_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * INET_DEBUG: Enable debugging in inet.c.
 */
#ifndef INET_DEBUG
#define INET_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * IP_DEBUG: Enable debugging for IP.
 */
#ifndef IP_DEBUG
#define IP_DEBUG                        LWIP_DBG_OFF
#endif

/**
 * IP_REASS_DEBUG: Enable debugging in ip_frag.c for both frag & reass.
 */
#ifndef IP_REASS_DEBUG
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * RAW_DEBUG: Enable debugging in raw.c.
 */
#ifndef RAW_DEBUG
#define RAW_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * MEM_DEBUG: Enable debugging in mem.c.
 */
#ifndef MEM_DEBUG
#define MEM_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * MEMP_DEBUG: Enable debugging in memp.c.
 */
#ifndef MEMP_DEBUG
#define MEMP_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * SYS_DEBUG: Enable debugging in sys.c.
 */
#ifndef SYS_DEBUG
#define SYS_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * TIMERS_DEBUG: Enable debugging in timers.c.
 */
#ifndef TIMERS_DEBUG
#define TIMERS_DEBUG                    LWIP_DBG_OFF
#endif

/**
 * TCP_DEBUG: Enable debugging for TCP.
 */
#ifndef TCP_DEBUG
#define TCP_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * TCP_INPUT_DEBUG: Enable debugging in tcp_in.c for incoming dbg.
 */
#ifndef TCP_INPUT_DEBUG
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#endif

/**
 * TCP_FR_DEBUG: Enable debugging in tcp_in.c for fast retransmit.
 */
#ifndef TCP_FR_DEBUG
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#endif

/**
 * TCP_RTO_DEBUG: Enable debugging in TCP for retransmit
 * timeout.
 */
#ifndef TCP_RTO_DEBUG
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * TCP_CWND_DEBUG: Enable debugging for TCP congestion window.
 */
#ifndef TCP_CWND_DEBUG
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * TCP_WND_DEBUG: Enable debugging in tcp_in.c for window updating.
 */
#ifndef TCP_WND_DEBUG
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * TCP_OUTPUT_DEBUG: Enable debugging in tcp_out.c output functions.
 */
#ifndef TCP_OUTPUT_DEBUG
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#endif

/**
 * TCP_RST_DEBUG: Enable debugging for TCP with the RST message.
 */
#ifndef TCP_RST_DEBUG
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * TCP_SACK_DEBUG: Enable debugging for TCP SACK and pipe based
 * loss recovery algorithm.
 */

#ifndef TCP_SACK_DEBUG
#define TCP_SACK_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * TCP_QLEN_DEBUG: Enable debugging for TCP queue lengths.
 */
#ifndef TCP_QLEN_DEBUG
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * UDP_DEBUG: Enable debugging in UDP.
 */
#ifndef UDP_DEBUG
#define UDP_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * TCPIP_DEBUG: Enable debugging in tcpip.c.
 */
#ifndef TCPIP_DEBUG
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#endif

/**
 * PPP_DEBUG: Enable debugging for PPP.
 */
#ifndef PPP_DEBUG
#define PPP_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * SLIP_DEBUG: Enable debugging in slipif.c.
 */
#ifndef SLIP_DEBUG
#define SLIP_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * DHCP_DEBUG: Enable debugging in dhcp.c.
 */
#ifndef DHCP_DEBUG
#define DHCP_DEBUG                      LWIP_DBG_OFF
#endif

/**
 * AUTOIP_DEBUG: Enable debugging in autoip.c.
 */
#ifndef AUTOIP_DEBUG
#define AUTOIP_DEBUG                    LWIP_DBG_OFF
#endif

/**
 * SNMP_MSG_DEBUG: Enable debugging for SNMP messages.
 */
#ifndef SNMP_MSG_DEBUG
#define SNMP_MSG_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * SNMP_MIB_DEBUG: Enable debugging for SNMP MIBs.
 */
#ifndef SNMP_MIB_DEBUG
#define SNMP_MIB_DEBUG                  LWIP_DBG_OFF
#endif

/**
 * DNS_DEBUG: Enable debugging for DNS.
 */
#ifndef DNS_DEBUG
#define DNS_DEBUG                       LWIP_DBG_OFF
#endif

/**
 * TFTP_DEBUG: Enable debugging for DNS.
 */
#ifndef TFTP_DEBUG
#define TFTP_DEBUG                       LWIP_DBG_OFF
#endif


/** * SYS_ARCH_DEBUG: Enable debugging for DNS. */

#ifndef SYS_ARCH_DEBUG
#define SYS_ARCH_DEBUG                   LWIP_DBG_OFF
#endif

/**
 * SNTP_DEBUG: Enable debugging for SNTP.
 */
#ifndef SNTP_DEBUG
#define SNTP_DEBUG                  LWIP_DBG_OFF
#endif



/**
 * DRV_STS_DEBUG: Enable debugging for  Driver Status
 */
#ifndef DRV_STS_DEBUG
#define DRV_STS_DEBUG           LWIP_DBG_OFF
#endif


/**
 * LWIP_PERF : Enable performance calculation APIs
 */

#ifndef LWIP_PERF
#define LWIP_PERF               0
#endif

/**
 * LWIP_DHCPS_DISCOVER_brcast : Enable DHCP server always using
 * brcast to send discover reply.
 */
#ifndef LWIP_DHCPS_DISCOVER_BROADCAST
#define LWIP_DHCPS_DISCOVER_BROADCAST    0
#endif

#ifndef LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_ARP
#define LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_ARP   1
#endif

#ifdef LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_DHCP
#define LWIP_ALWAYS_SEND_HWTYPE_AS_ETHER_IN_DHCP  1
#endif

#ifndef LWIP_USE_POSIX_COND_WAIT_WITH_ABS_TIMEOUT
#define LWIP_USE_POSIX_COND_WAIT_WITH_ABS_TIMEOUT    1
#endif

#ifndef LWIP_CONGCNTRL_DUPACK_THRESH
#define LWIP_CONGCNTRL_DUPACK_THRESH       3
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_CWND
#define LWIP_CONGCNTRL_INITIAL_CWND       20
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_SSTHRESH
#define LWIP_CONGCNTRL_INITIAL_SSTHRESH       200
#endif

/* NETIF DRIVER STATUS BEGIN */
#ifndef DRIVER_STATUS_CHECK
#define DRIVER_STATUS_CHECK       1
#endif

#if DRIVER_STATUS_CHECK
#ifndef DRIVER_WAKEUP_INTERVAL
#define DRIVER_WAKEUP_INTERVAL 120000
#endif
#endif
/* NETIF DRIVER STATUS END */

/* Declaring here as MEM_PBUF_RAM_SIZE_LIMIT is defined above */
extern int g_IslwIPInitialized;

/* netif ifindex MUST NOT start from 0 as this value means no netif was bond to sockets */
#ifndef LWIP_NETIF_IFINDEX_START
#define LWIP_NETIF_IFINDEX_START        1
#endif /* LWIP_NETIF_IFINDEX_START */

#ifndef LWIP_NETIF_IFINDEX_MAX
#define LWIP_NETIF_IFINDEX_MAX 0xFE
#endif

#if LWIP_RAW
#ifndef PF_PKT_SUPPORT
#define PF_PKT_SUPPORT  1
#endif /* PF_PKT_SUPPORT */
#else /* LWIP_RAW */
#define PF_PKT_SUPPORT 0
#define LWIP_NETIF_PROMISC 0
#endif /* LWIP_RAW */

#if PF_PKT_SUPPORT
#ifndef LWIP_NETIF_PROMISC
#define LWIP_NETIF_PROMISC 1
#endif /* LWIP_NETIF_PROMISC */
#else  /* PF_PKT_SUPPORT */
#define LWIP_NETIF_PROMISC 0
#endif /* PF_PKT_SUPPORT */

#ifndef LWIP_NETIF_ETHTOOL
#define LWIP_NETIF_ETHTOOL          1
#endif

/**
 * TCPIP_MBOX_SIZE: The mailbox size for the tcpip thread messages
 * The queue size value itself is platform-dependent, but is passed to
 * sys_mbox_new() when tcpip_init is called.
 */
#ifndef TCPIP_MBOX_SIZE
#if LWIP_NETIF_PROMISC
#define TCPIP_MBOX_SIZE                 512
#else
#define TCPIP_MBOX_SIZE                 320
#endif
#endif

#ifndef TCPIP_PRTY_MBOX_SIZE
#define TCPIP_PRTY_MBOX_SIZE 16
#endif

#ifndef PBUF_LINK_CHKSUM_LEN
#define PBUF_LINK_CHKSUM_LEN  0
#endif

#ifndef LWIP_SACK_PERF_OPT
#if LWIP_SACK
#define LWIP_SACK_PERF_OPT 0
#else
#define LWIP_SACK_PERF_OPT 0
#endif
#endif /* LWIP_SACK_PERF_OPT */

#ifndef LWIP_SACK_CWND_OPT
#define LWIP_SACK_CWND_OPT LWIP_SACK_PERF_OPT
#endif

#ifndef ENABLE_NETCONN_INTERFACE
#define ENABLE_NETCONN_INTERFACE 0
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LWIP_OPT_H__ */

