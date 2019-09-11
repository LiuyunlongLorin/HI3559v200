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
 *   @file pbuf.h
 */

 /**
  *   @defgroup Buffer_Interfaces Buffer Interfaces
  *   @ingroup System_interfaces
  */

#ifndef __LWIP_PBUF_H__
#define __LWIP_PBUF_H__

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "asm/atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Currently, the pbuf_custom code is only needed for one specific configuration
 * of IP_FRAG */
#define LWIP_SUPPORT_CUSTOM_PBUF (IP_FRAG && !IP_FRAG_USES_STATIC_BUF && !LWIP_NETIF_TX_SINGLE_PBUF)

#define PBUF_TRANSPORT_HLEN     20
#define PBUF_TRANSPORT_OPT_LEN  40
#define PBUF_IP_HLEN            20
#define PBUF_IP_OPT_LEN         40

typedef enum {
  PBUF_TRANSPORT,
  PBUF_IP,
  PBUF_LINK,
  PBUF_RAW
} pbuf_layer;

typedef enum {
  PBUF_RAM, /* pbuf data is stored in RAM */
  PBUF_ROM, /* pbuf data is stored in ROM */
  PBUF_REF, /* pbuf comes from the pbuf pool */
  PBUF_POOL, /* pbuf payload refers to RAM */
} pbuf_type;


/** indicates this packet's data should be immediately passed to the application */
/* #define PBUF_FLAG_PUSH      0x01U */ /* obsolete */
/** indicates this is a custom pbuf: pbuf_free and pbuf_header handle such a
    a pbuf differently */
#define PBUF_FLAG_IS_CUSTOM 0x02U
/** indicates this pbuf is UDP multicast to be looped back */
#define PBUF_FLAG_MCASTLOOP 0x04U
/** indicates this pbuf was received as link-level brcast */
#define PBUF_FLAG_LLBCAST   0x08U
/** indicates this pbuf was received as link-level multicast */
#define PBUF_FLAG_LLMCAST   0x10U
/** indicates this pbuf includes a TCP FIN flag */
#define PBUF_FLAG_TCP_FIN   0x20U
/** indicates this pbuf is send already */
#define PBUF_FLAG_IS_SNET   0x40U

/* NETIF DRIVER STATUS CHANGE BEGIN */
#if DRIVER_STATUS_CHECK
/** indicates this pbuf is from dhcp application */
#define PBUF_FLAG_DHCP_BUF  0x80U
#endif
/* NETIF DRIVER STATUS CHANGE END */
#if PF_PKT_SUPPORT
#define PBUF_FLAG_HOST      0x100U
#define PBUF_FLAG_OUTGOING  0x200U
#endif

/** indicates this pbuf is loopback */
#define PBUF_FLAG_IS_LOOPBACK   0x400U

/** indicates this pbuf is output and only pseduo header checksum was calculated  */
#if LWIP_TX_CSUM_OFFLOAD
#define PBUF_FLAG_TX_CSUM_PARTIAL 0x800U
#endif

/** indicates this pbuf never cross gateway (the destination is the nexthop) */
#define PBUF_FLAG_IS_LINK_ONLY   0x1000U

#if (MEM_MALLOC_DMA_ALIGN != 1)
struct pbuf_dma_info {
  void         *dma;
  /** pointer to the mac header in the buffer,used by driver */
  void *mac_header;

  atomic_t      dma_ref;
  u16_t         dma_len;
  /** link layer data totlen,used by driver */
  u16_t link_len;
};
#endif

struct pbuf {
  /** next pbuf in singly linked pbuf chain */
  struct pbuf *next;

  /** pointer to the actual data in the buffer */
  void *payload;

#if (MEM_MALLOC_DMA_ALIGN != 1)
  struct pbuf_dma_info *dma_info;
#endif
  /**
   * total length of this buffer and all next buffers in chain
   * belonging to the same packet.
   *
   * For non-queue packet chains this is the invariant:
   * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
   */
  u16_t tot_len;

  /** length of this buffer */
  u16_t len;

  /**
   * the reference count always equals the number of pointers
   * that refer to this pbuf. This can be pointers from an application,
   * the stack itself, or pbuf->next pointers from a chain.
   */
  atomic_t ref;

#if MEM_PBUF_RAM_SIZE_LIMIT
  u16_t malloc_len;
#endif

  /** misc flags */
  u16_t flags;

  /** pbuf_type as u8_t instead of enum to save space */
  u8_t  type;
#if MEM_PBUF_RAM_SIZE_LIMIT
  u8_t pad[3];
#else
  u8_t pad[1];
#endif
};

#if LWIP_SUPPORT_CUSTOM_PBUF
/** Prototype for a function to free a custom pbuf */
typedef void (*pbuf_free_custom_fn)(struct pbuf *p);

/** A custom pbuf: like a pbuf, but following a function pointer to free it. */
struct pbuf_custom {
  /** The actual pbuf */
  struct pbuf pbuf;
  /** This function is called when pbuf_free deallocates this pbuf(_custom) */
  pbuf_free_custom_fn custom_free_function;
};
#endif /* LWIP_SUPPORT_CUSTOM_PBUF */

#if LWIP_TCP && TCP_QUEUE_OOSEQ
/** Define this to 0 to prevent freeing ooseq pbufs when the PBUF_POOL is empty */
#ifndef PBUF_POOL_FREE_OOSEQ
#define PBUF_POOL_FREE_OOSEQ 1
#endif /* PBUF_POOL_FREE_OOSEQ */
#if NO_SYS && PBUF_POOL_FREE_OOSEQ
extern volatile u8_t pbuf_free_ooseq_pending;
void pbuf_free_ooseq();
/** When not using sys_check_timeouts(), call PBUF_CHECK_FREE_OOSEQ()
    at regular intervals from main level to check if ooseq pbufs need to be
    freed! */
#define PBUF_CHECK_FREE_OOSEQ() \
  do { \
    SYS_ARCH_DECL_PROTECT(level);\
    SYS_ARCH_PROTECT(level);\
    if(pbuf_free_ooseq_pending) { \
      SYS_ARCH_UNPROTECT(level);\
  /* pbuf_alloc() rprted PBUF_POOL to be empty -> try to free some \
     ooseq queued pbufs now */ \
    pbuf_free_ooseq();
  }\
  else{\
      SYS_ARCH_UNPROTECT(level);\
  }\
}while(0)

#endif /* NO_SYS && PBUF_POOL_FREE_OOSEQ*/

#if !NO_SYS
void pbuf_free_ooseq_callback(void *arg);
#endif

void pbuf_pool_is_empty(void);

#define PBUF_POOL_IS_EMPTY() pbuf_pool_is_empty()

#endif /* LWIP_TCP && TCP_QUEUE_OOSEQ */

/* Initializes the pbuf module. This call is empty for now, but may not be in future. */
#define pbuf_init()

struct pbuf *pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type);
#if LWIP_SUPPORT_CUSTOM_PBUF
struct pbuf *pbuf_alloced_custom(pbuf_layer l, u16_t length, pbuf_type type,
                                 struct pbuf_custom *p, void *payload_mem,
                                 u16_t payload_mem_len);
#endif /* LWIP_SUPPORT_CUSTOM_PBUF */
void pbuf_realloc(struct pbuf *p, u16_t size);
u8_t pbuf_header(struct pbuf *p, s16_t header_size);
void pbuf_ref(struct pbuf *p);
u8_t pbuf_free(struct pbuf *p);
u8_t pbuf_clen(struct pbuf *p);
void pbuf_cat(struct pbuf *head, struct pbuf *tail);
void pbuf_chain(struct pbuf *head, struct pbuf *tail);
struct pbuf *pbuf_dechain(struct pbuf *p);
err_t pbuf_copy(struct pbuf *p_to, struct pbuf *p_from);
u16_t pbuf_copy_partial(struct pbuf *p, void *dataptr, u16_t len, u16_t offset);
err_t pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len);
err_t pbuf_take_at(struct pbuf *buf, const void *dataptr, u16_t len, u16_t offset);
struct pbuf *pbuf_coalesce(struct pbuf *p, pbuf_layer layer);
#ifdef LWIP_DEV_DEBUG
void pbuf_debug_print(struct pbuf* p);
#endif
#if (MEM_MALLOC_DMA_ALIGN != 1)
struct pbuf *pbuf_dma_alloc(u16_t len);
err_t pbuf_dma_ref(struct pbuf_dma_info *dma_info);
void  pbuf_dma_free(struct pbuf_dma_info *dma_info);
#endif

#if LWIP_CHECKSUM_ON_COPY
err_t pbuf_fill_chksum(struct pbuf *p, u16_t start_offset, const void *dataptr,
                       u16_t len, u16_t *chksum);
#endif /* LWIP_CHECKSUM_ON_COPY */

/* Add for WINDOW SCALE */
#if LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE
void pbuf_split_64k(struct pbuf *p, struct pbuf **rest);
#endif /* LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE */
u8_t pbuf_get_at(struct pbuf* p, u16_t offset);
void pbuf_put_at(struct pbuf* p, u16_t offset, u8_t data);
u16_t pbuf_memcmp(struct pbuf* p, u16_t offset, const void* s2, u16_t n);
u16_t pbuf_memfind(struct pbuf* p, const void* mem, u16_t mem_len, u16_t start_offset);
u16_t pbuf_strstr(struct pbuf* p, const char* substr);
void pbuf_split_64k(struct pbuf *p, struct pbuf **rest); // windows scale needs large pbuf
#if MEM_PBUF_RAM_SIZE_LIMIT
 /*
Func Name: pbuf_ram_size_set
*/
/**
* @ingroup Buffer_Interfaces
* @par Prototype
* @code
* u32_t pbuf_ram_size_set(u32_t ram_max_size);
* @endcode
*
* @par Purpose
*  This API is used to set the max buffer size of PBUF_RAM allocation.
*
* @par Description
*  This API is used to set the max buffer size of PBUF_RAM allocation
*
* @param[in]    ram_max_size    maximum ram buffer size allowed [N/A]
*
* @par Return values
*  0: On failure
*  Non Zero value: Previous maximum ram buffer size
*
*
* @par Required Header File
* pbuf.h
*
* @par Note
*
* @par Related Topics
*/

u32_t pbuf_ram_size_set(u32_t ram_max_size);
#ifdef LWIP_DEBUG
/*
Func Name: pbuf_ram_display
*/
/**
* @ingroup Buffer_Interfaces
* @par Prototype
* @code
* void pbuf_ram_display(void);
* @endcode
*
* @par Purpose
*  This API is used to display the pbuf RAM details.
*
* @par Description
*  This API is used to display the pbuf RAM details.
*
* @param[in]
* void
*
* @par Return values
*  void
*
*
* @par Required Header File
* pbuf.h
*
* @par Note
* This API is enabled only if the LWIP_DEBUG macro is enabled.
*
* @par Related Topics
*/
void pbuf_ram_display(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_PBUF_H__ */
