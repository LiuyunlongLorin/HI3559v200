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
 *   @file tcpip.h
 */
/**
* @defgroup TCPIP_Interfaces TCP_IP Interface
* This section provides information about TCP/IP interfaces.
* @ingroup System_interfaces
*/
#ifndef __LWIP_TCPIP_H__
#define __LWIP_TCPIP_H__

#include "lwip/opt.h"

#if !NO_SYS /* don't build if not configured for use in lwipopts.h */

#include "lwip/api_msg.h"
#include "lwip/netifapi.h"
#include "lwip/pbuf.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/timers.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Define this to something that triggers a watchdog. This is called from
 * tcpip_thread after processing a message. */
#ifndef LWIP_TCPIP_THREAD_ALIVE
#define LWIP_TCPIP_THREAD_ALIVE()
#endif

#if LWIP_NETCONN
err_t tcpip_apimsg(struct api_msg *apimsg, sys_sem_t *sem);
#if LWIP_TCPIP_CORE_LOCKING
err_t tcpip_apimsg_lock(struct api_msg *apimsg);
#endif /* LWIP_TCPIP_CORE_LOCKING */
#endif /* LWIP_NETCONN */

#if LWIP_TCPIP_CORE_LOCKING
/** The global semaphore to lock the stack. */
extern sys_mutex_t lock_tcpip_core;
#define LOCK_TCPIP_CORE()     sys_mutex_lock(&lock_tcpip_core)
#define UNLOCK_TCPIP_CORE()   sys_mutex_unlock(&lock_tcpip_core)
#define TCPIP_APIMSG(m)       tcpip_apimsg_lock(m)
#define TCPIP_SEND_APIMSG(m, err)  (err = tcpip_apimsg_lock(m))
#define TCPIP_RECV_APIMSG(m, conn, err)  (err = tcpip_apimsg_lock(m))
#define TCPIP_APIMSG_ACK(m)
#define TCPIP_RECV_APIMSG_ACK(m)
#define TCPIP_SEND_APIMSG_ACK(m)
#define TCPIP_NETIFAPI(m)     tcpip_netifapi_lock(m)
#define TCPIP_NETIFAPI_PRIORITY(m)
#define TCPIP_NETIFAPI_PRIORITY_ACK(m)
#define TCPIP_NETIFAPI_ACK(m)
#else /* LWIP_TCPIP_CORE_LOCKING */
#define LOCK_TCPIP_CORE()
#define UNLOCK_TCPIP_CORE()
#define TCPIP_APIMSG(m)       tcpip_apimsg((m), (&(m)->msg.conn->op_completed))

#define TCPIP_RECV_APIMSG(m,conn, err)    \
do{\
  atomic_inc(&conn->recv_count);\
  err = tcpip_apimsg((m), (&(m)->msg.conn->recv_completed));\
}while(0)

 /** as per design requirement of multithread support over single socket,
    send is supported over multiple threads. To provide protection we are using send_mutex */
#define TCPIP_SEND_APIMSG(m, err)   \
do {\
    sys_mutex_lock(&(m)->msg.conn->send_mutex);\
    err = tcpip_apimsg((m), (&(m)->msg.conn->send_completed));\
    sys_mutex_unlock(&(m)->msg.conn->send_mutex);\
}while(0)

#define TCPIP_CLOSE_APIMSG_ACK(m)   sys_sem_signal(&m->conn->close_completed)
#define TCPIP_APIMSG_ACK(m)   sys_sem_signal(&m->conn->op_completed)
#define TCPIP_RECV_APIMSG_ACK(m) sys_sem_signal(&m->conn->recv_completed)
#define TCPIP_SEND_APIMSG_ACK(m) sys_sem_signal(&m->conn->send_completed)
#define TCPIP_NETIFAPI(m)     tcpip_netifapi(m)
#define TCPIP_NETIFAPI_PRIORITY(m) tcpip_netifapi_priority(m)
#define TCPIP_NETIFAPI_PRIORITY_ACK(m) sys_sem_signal(&m->sem)
#define TCPIP_NETIFAPI_ACK(m) sys_sem_signal(&m->sem)
#endif /* LWIP_TCPIP_CORE_LOCKING */

/**
* @ingroup TCPIP_Interfaces
* @brief
*  Function prototype for the init_done function passed to tcpip_init
*
* @param[in]    arg  Specifies the argument to pass to tcpip_init_done. 
*
* @par Return values
*  None
*/
typedef void (*tcpip_init_done_fn)(void *arg);
/** Function prototype for functions passed to tcpip_callback() */
typedef void (*tcpip_callback_fn)(void *ctx);

/* Forward declarations */
struct tcpip_callback_msg;

/*
Func Name:  tcpip_init
*/
/**
* @ingroup TCPIP_Interfaces
* @par Prototype
* @code
* void tcpip_init(tcpip_init_done_fn tcpip_init_done, void *arg);
* @endcode
*
* @par Purpose
*  This API initializes all sub modules and starts the tcpip_thread.
*
* @par Description
*  This API initializes all sub modules and starts the tcpip_thread.
*
* @param[in]    tcpip_init_done   function to call when tcpip_thread is running and finished initializing [N/A]
* @param[in]    arg                    argument to pass to tcpip_init_done [N/A]
*
* @par Return values
*  None
*
* @par Required Header File
* tcpip.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
void tcpip_init(tcpip_init_done_fn tcpip_init_done, void *arg);


/*
Func Name:  lwip_set_socket_num
*/
/**
* @defgroup lwip_set_socket_num
* @ingroup TCPIP_Interfaces
* @par Prototype
* @code
* void lwip_set_socket_num(int socketnum);
* @endcode
*
* @par Purpose
*    This API is used to configure max socket num in lwip.
*
* @par Description
*    This API is used to configure max socket num in lwip.
*
* @param[in]   socketnum    Maximum number sockets. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcpip.h
*
* @par Note
* \n
* Call this API before tcpip_init, or it will be failed.
* If this API is not invoked, max socket num is set to DEFAULT_LWIP_NUM_SOCKETS, which is defined in lwipopts.h.
* @par Related Topics
* \n
* None
*/
int lwip_set_socket_num(int socketnum);

/*
Func Name:  lwip_get_socket_num
*/
/**
* @defgroup lwip_get_socket_num
* @ingroup TCPIP_Interfaces
* @par Prototype
* @code
* void lwip_get_socket_num(void);
* @endcode
*
* @par Purpose
*    This API is used to get max socket num setting in lwip.
*
* @par Description
*    This API is used to get max socket num setting in lwip.
*
* @param[in]   void
*
* @par Return values
*  current max socket number
*
* @par Required Header File
* tcpip.h
*
* @par Note
* None
*/
int lwip_get_socket_num(void);

err_t tcpip_input(struct pbuf *p, struct netif *inp);

/* NETIF DRIVER STATUS BEGIN */
void tcpip_upd_status_to_tcp_pcbs(struct netif *netif, u8_t status);
/* NETIF DRIVER STATUS END */

#if LWIP_NETIF_API
err_t tcpip_netifapi(struct netifapi_msg *netifapimsg);
/* NETIF DRIVER STATUS BEGIN */
err_t
tcpip_netifapi_priority(struct netifapi_msg* netifapimsg);
/* NETIF DRIVER STATUS END */
#if LWIP_TCPIP_CORE_LOCKING
err_t tcpip_netifapi_lock(struct netifapi_msg *netifapimsg);
#endif /* LWIP_TCPIP_CORE_LOCKING */
#endif /* LWIP_NETIF_API */

err_t tcpip_callback_with_block(tcpip_callback_fn function, void *ctx, u8_t block);
#define tcpip_callback(f, ctx)              tcpip_callback_with_block(f, ctx, 1)

struct tcpip_callback_msg* tcpip_callbackmsg_new(tcpip_callback_fn function, void *ctx);
void   tcpip_callbackmsg_delete(struct tcpip_callback_msg* msg);
err_t  tcpip_trycallback(struct tcpip_callback_msg* msg);


enum tcpip_msg_type {
#if LWIP_NETCONN
  TCPIP_MSG_API,
#endif /* LWIP_NETCONN */
  TCPIP_MSG_INPKT,
#if LWIP_NETIF_API
  TCPIP_MSG_NETIFAPI,
#endif /* LWIP_NETIF_API */
#if LWIP_TCPIP_TIMEOUT
  TCPIP_MSG_TIMEOUT,
  TCPIP_MSG_UNTIMEOUT,
#endif /* LWIP_TCPIP_TIMEOUT */
  TCPIP_MSG_CALLBACK,
  TCPIP_MSG_CALLBACK_STATIC
};

struct tcpip_msg {
  enum tcpip_msg_type type;
  sys_sem_t *sem;
  union {
#if LWIP_NETCONN
    struct api_msg *apimsg;
#endif /* LWIP_NETCONN */
#if LWIP_NETIF_API
    struct netifapi_msg *netifapimsg;
#endif /* LWIP_NETIF_API */
    struct {
      struct pbuf *p;
      struct netif *netif;
    } inp;
    struct {
      tcpip_callback_fn function;
      void *ctx;
    } cb;
#if LWIP_TCPIP_TIMEOUT
    struct {
      u32_t msecs;
      sys_timeout_handler h;
      void *arg;
    } tmo;
#endif /* LWIP_TCPIP_TIMEOUT */
  } msg;
};

struct tcpip_conn {
  struct eth_addr dst_mac; /* Destinition Mac */
  ip_addr_t src_ip; /* Sorce IP */
  ip_addr_t dst_ip; /* Destinition IP */
  u16_t ipid;    /* Ip Identification */
  u16_t srcport; /* Source Port Address */
  u16_t dstport; /* Destination Port Address */
  u32_t tcpwin;  /* TCP window */
  u32_t seqnum;  /* TCP Sequence Number */
  u32_t acknum;  /* TCP Ack Number */
  u32_t last_payload_len;     /* TCP payload length of last packet */
  u32_t tsval;   /* Timestamp Value */
  u32_t tsecr;   /* Timestamp Echo Reply */
};

#ifdef __cplusplus
}
#endif

#endif /* !NO_SYS */

#endif /* __LWIP_TCPIP_H__ */
