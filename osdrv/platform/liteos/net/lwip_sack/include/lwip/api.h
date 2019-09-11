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

#ifndef __LWIP_API_H__
#define __LWIP_API_H__

#include "lwip/opt.h"

#if LWIP_NETCONN /* don't build if not configured for use in lwipopts.h */

#include <stddef.h> /* for size_t */

#include "lwip/netbuf.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "asm/atomic.h"
#include "lwip/filter.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Throughout this file, IP_add and port numbers are expected to be in
 * the same byte order as in the corresponding pcb.
 */

/* Flags for netconn_write (u8_t) */
#define NETCONN_NOFLAG    0x00
#define NETCONN_NOCOPY    0x00 /* Only for source code compatibility */
#define NETCONN_COPY      0x01
#define NETCONN_MORE      0x02
#define NETCONN_DONTBLOCK 0x04

/* Flags for struct netconn.flags (u8_t) */
/** TCP: when data passed to netconn_write doesn't fit into the send buffer,
    this temporarily stores whether to wake up the original application task
    if data couldn't be sent in the first try. */
#define NETCONN_FLAG_WRITE_DELAYED            0x01
/** Should this netconn avoid blocking? */
#define NETCONN_FLAG_NON_BLOCKING             0x02
/** Was the last connect action a non-blocking one? */
#define NETCONN_FLAG_IN_NONBLOCKING_CONNECT   0x04
/** If this is set, a TCP netconn must call netconn_recved() to update
    the TCP receive window (done automatically if not set). */
#define NETCONN_FLAG_NO_AUTO_RECVED           0x08
/** If a nonblocking write has been rejected before, poll_tcp needs to
    check if the netconn is writable again */
#define NETCONN_FLAG_CHECK_WRITESPACE         0x10


/* Helpers to process several netconn_types by the same code */
#define NETCONNTYPE_GROUP(t)    (t&0xF0)
#define NETCONNTYPE_DATAGRAM(t) (t&0xE0)

/** Protocol family and type of the netconn */
enum netconn_type {
  NETCONN_INVALID    = 0,
  /* NETCONN_TCP Group */
  NETCONN_TCP        = 0x10,
  /* NETCONN_UDP Group */
  NETCONN_UDP        = 0x20,
  NETCONN_UDPLITE    = 0x21,
  NETCONN_UDPNOCHKSUM= 0x22,
  /* NETCONN_RAW Group */
  NETCONN_RAW        = 0x40,
#if PF_PKT_SUPPORT
  NETCONN_PKT_RAW    = 0x80
#endif
};

/** Current state of the netconn. **/
enum netconn_state {
  NETCONN_NONE,
  NETCONN_WRITE,
  NETCONN_LISTEN,
  NETCONN_CONNECTED,
  NETCONN_CONNECTING,
  NETCONN_CLOSE
};

/** Use to inform the callback function about changes */
enum netconn_evt {
  NETCONN_EVT_RCVPLUS,
  NETCONN_EVT_RCVMINUS,
  NETCONN_EVT_SENDPLUS,
  NETCONN_EVT_SENDMINUS,
  NETCONN_EVT_ERROR
};

#if LWIP_IGMP
/** Used for netconn_join_leave_group() */
enum netconn_igmp {
  NETCONN_JOIN,
  NETCONN_LEAVE
};
#endif /* LWIP_IGMP */

enum netconn_shutdown {
  NON_SHUTDOWN = 0,
  RCV_SHUTDOWN,
  SND_SHUTDOWN,
  SHUTDOWN_MASK
};

#define NETCONN_MBOX_ACTIVE       1
#define NETCONN_MBOX_DELETING   2

/* forward-declare some structs to avoid to include their headers */
struct ip_pcb;
struct tcp_pcb;
struct udp_pcb;
struct raw_pcb;
struct netconn;
struct api_msg_msg;

/** A callback prototype to inform about events for a netconn */
typedef void (* netconn_callback)(struct netconn *, enum netconn_evt, u16_t len);

/** A netconn descriptor */
struct netconn {
  /** the lwIP internal protocol control block */
  union {
    struct ip_pcb  *ip;
    struct tcp_pcb *tcp;
    struct udp_pcb *udp;
    struct raw_pcb *raw;
#if PF_PKT_SUPPORT
    struct raw_pcb *pkt_raw;
#endif
  } pcb;

#if LWIP_TCP
  /** TCP: when data passed to netconn_write doesn't fit into the send buffer,
      this temporarily stores how much is already sent. */
  size_t write_offset;
  /** TCP: when data passed to netconn_write doesn't fit into the send buffer,
      this temporarily stores the message.
      Also used during connect and close. */
  struct api_msg_msg *current_msg;
#endif /* LWIP_TCP */

  /** A callback function that is informed about events for this netconn */
  netconn_callback callback;

  /** conn_mutex for protection of connection parameters */
  sys_mutex_t conn_mutex;

  atomic_t  recv_count;
  /** mbox state, used for close event in parallel to recv and send events */
  atomic_t mbox_state;
  /** mbox where received packets are stored until they are fetched
      by the netconn application thread (can grow quite big) */
  sys_mbox_t recvmbox;

/** protection needed for multithread support over single socket for send.
        We do not support parallel process of recv from different thread for single socket. */
    sys_mutex_t send_mutex;

#if LWIP_TCP
  /** mbox where new connections are stored until processed
      by the application thread */
  sys_mbox_t acceptmbox;
#endif /* LWIP_TCP */
  /** only used for socket layer */
#if LWIP_SOCKET
  int socket;
#endif /* LWIP_SOCKET */
#if LWIP_SO_SNDTIMEO
  /** timeout to wait for sending data (which means enqueueing data for sending
      in internal buffers) */
  s32_t send_timeout;
#endif /* LWIP_SO_RCVTIMEO */
#if LWIP_SO_RCVTIMEO
  /** timeout to wait for new data to be received
      (or connections to arrive for listening netconns) */
  int recv_timeout;
#endif /* LWIP_SO_RCVTIMEO */
#if LWIP_SO_RCVBUF
  /** maximum amount of bytes queued in recvmbox
      not used for TCP: adjust TCP_WND instead! */
  int recv_bufsize;
  /** number of bytes currently in recvmbox to be received,
      tested against recv_bufsize to limit bytes on recvmbox
      for UDP and RAW, used for FIONREAD */
  s32_t recv_avail;
#endif /* LWIP_SO_RCVBUF */
  /** number of bytes left on last recv, for non-stream connections, this value was the buffer_len
      on last peek operation; for stream connection, this value was either the bytes count
      not copied to application on last recv or the buffer_len on last peek operation */
  u32_t lrcv_left;

  /** the last error this netconn had */
  volatile err_t last_err;
  /** the flag indicates whether last_err was overwrited by the callback api,
      this could happen only for nonblock TCP connecting, both the callback api(do_connected)
      and the calling api(netconn_connect) could change the last_err */
  s32_t last_err_overwrite;
  atomic_t is_err_fatal;

  /** flags holding more netconn-internal state, see NETCONN_FLAG_* defines */
  u32_t flags;
  u32_t flags_usr;

  atomic_t flags_blocking;

  /** sem that is used to synchroneously execute functions in the core context */
  sys_sem_t op_completed;
  sys_sem_t recv_completed;
  sys_sem_t send_completed;
  sys_sem_t close_completed;

  /** type of the netconn (TCP, UDP or RAW) */
  u32_t type;
  /** current state of the netconn */
  int state;
  enum netconn_shutdown shutdown;
#if LWIP_SOCK_FILTER
  struct sock_fprog sk_filter;
#endif
};

/** Register an Network connection event */
#define API_EVENT(c,e,l) if (c->callback) {         \
                           (*c->callback)(c, e, l); \
                         }

/** Set conn->last_err to err but don't overwrite fatal errors */
#define NETCONN_SET_SAFE_ERR(conn, err) do { \
  sys_mutex_lock(&(conn)->conn_mutex); \
  if (!atomic_read(&conn->is_err_fatal)) { \
    (conn)->last_err = err; \
    if (ERR_IS_FATAL((err))) \
    { \
      (void)atomic_set((void *)(&conn->is_err_fatal),1);\
    } \
  } \
  sys_mutex_unlock(&(conn)->conn_mutex); \
} while(0);

/** Set conn->last_err to err but don't overwrite fatal errors */
#define NETCONN_SET_SAFE_FATAL_ERR(conn, err) do { \
  sys_mutex_lock(&(conn)->conn_mutex); \
  (conn)->last_err = err; \
  (void)atomic_set((void *)(&conn->is_err_fatal),1);\
  sys_mutex_unlock(&(conn)->conn_mutex); \
} while(0);
/** Get conn->last_err to err */
#define NETCONN_GET_SAFE_ERR(conn, err) do { \
  sys_mutex_lock(&(conn)->conn_mutex); \
  err = (conn)->last_err; \
  sys_mutex_unlock(&(conn)->conn_mutex); \
} while(0)

/* Network connection functions: */
#define netconn_new(t)                  netconn_new_with_proto_and_callback(t, 0, NULL)
#define netconn_new_with_callback(t, c) netconn_new_with_proto_and_callback(t, 0, c)

#if PF_PKT_SUPPORT
struct
netconn *netconn_new_with_proto_and_callback(enum netconn_type t, u16_t proto,
                                             netconn_callback callback);
#else
struct
netconn *netconn_new_with_proto_and_callback(enum netconn_type t, u8_t proto,
                                             netconn_callback callback);
#endif
err_t netconn_delete(struct netconn *conn);
err_t netconn_initiate_delete(struct netconn *conn);
err_t netconn_finish_delete(struct netconn *conn);
/** Get the type of a netconn (as enum netconn_type). */
#define netconn_type(conn) (conn->type)

err_t   netconn_getaddr(struct netconn *conn, ip_addr_t *addr,
                        u16_t *port, u8_t local);
#define netconn_peer(c,i,p) netconn_getaddr(c,i,p,0)
#define netconn_addr(c,i,p) netconn_getaddr(c,i,p,1)

#if PF_PKT_SUPPORT
err_t netconn_bind(struct netconn *conn, ip_addr_t *addr, u16_t port, u8_t ifindex);
#else
err_t   netconn_bind(struct netconn *conn, ip_addr_t *addr, u16_t port);
#endif
err_t   netconn_connect(struct netconn *conn, ip_addr_t *addr, u16_t port);
err_t   netconn_listen_with_backlog(struct netconn *conn, u8_t backlog);
#define netconn_listen(conn) netconn_listen_with_backlog(conn, TCP_DEFAULT_LISTEN_BACKLOG)
err_t   netconn_accept(struct netconn *conn, struct netconn **new_conn);
err_t   netconn_recv(struct netconn *conn, struct netbuf **new_buf);
err_t   netconn_recv_tcp_pbuf(struct netconn *conn, struct pbuf **new_buf);
void    netconn_recved(struct netconn *conn, u32_t length);
err_t   netconn_send(struct netconn *conn, struct netbuf *buf);
err_t   netconn_write_partly(struct netconn *conn, const void *dataptr, size_t size,
                             u8_t apiflags, size_t *bytes_written);
#define netconn_write(conn, dataptr, size, apiflags) \
          netconn_write_partly(conn, dataptr, size, apiflags, NULL)
err_t   netconn_shutdown(struct netconn *conn, u8_t shut_rx, u8_t shut_tx);

#if LWIP_DNS
err_t   netconn_gethostbyname(const char *name, ip_addr_t *addr, u32_t *count);
#endif /* LWIP_DNS */

err_t netconn_getconninfo(struct netconn *conn, void  * conn_info);

int netconn_mbox_is_active(struct netconn *conn);


#define netconn_err(conn)               ((conn)->last_err)
#define netconn_recv_bufsize(conn)      ((conn)->recv_bufsize)

/** Set the blocking status of netconn calls (@todo: write/send is missing) */
#define netconn_set_nonblocking(conn, val)  do { if(val) { \
  (void)atomic_set(&((conn)->flags_blocking), NETCONN_FLAG_NON_BLOCKING); \
} else { \
  (void)atomic_set(&((conn)->flags_blocking),0);\
}} while(0)
/** Get the blocking status of netconn calls (@todo: write/send is missing) */
#define netconn_is_nonblocking(conn)        (atomic_read(&((conn)->flags_blocking)) == NETCONN_FLAG_NON_BLOCKING)
/** Set conn->mbox_state to state */
#define NETCONN_SET_SAFE_MBOX_STATE(conn, state) do { \
  (void)atomic_set((void *)(&(conn)->mbox_state),state); \
} while(0);

/** get blocking status from conn and apiflags by protecting */
#define NETCONN_APIFLAGS_IS_NONBLOCKING(conn, apiflags, dontblock) do { \
    dontblock = netconn_is_nonblocking(conn) || ((apiflags) & NETCONN_DONTBLOCK); \
  }while(0)

/** TCP: Set the no-auto-recved status of netconn calls (see NETCONN_FLAG_NO_AUTO_RECVED) */
#define netconn_set_noautorecved(conn)  do { \
    (conn)->flags_usr |= NETCONN_FLAG_NO_AUTO_RECVED; \
  }while(0)
#define netconn_clear_noautorecved(conn)  do { \
    (conn)->flags_usr &= ~ NETCONN_FLAG_NO_AUTO_RECVED; \
  }while(0)

/** TCP: Get the no-auto-recved status of netconn calls (see NETCONN_FLAG_NO_AUTO_RECVED) */
#define netconn_get_noautorecved(conn, noautorecved) do { \
    noautorecved = (((conn)->flags_usr & NETCONN_FLAG_NO_AUTO_RECVED) != 0); \
}while(0)

#if LWIP_SO_SNDTIMEO
/** Set the send timeout in milliseconds */
#define netconn_set_sendtimeout(conn, timeout)      ((conn)->send_timeout = (timeout))
/** Get the send timeout in milliseconds */
#define netconn_get_sendtimeout(conn)               ((conn)->send_timeout)
#endif /* LWIP_SO_SNDTIMEO */
#if LWIP_SO_RCVTIMEO
/** Set the receive timeout in milliseconds */
#define netconn_set_recvtimeout(conn, timeout)      ((conn)->recv_timeout = (timeout))
/** Get the receive timeout in milliseconds */
#define netconn_get_recvtimeout(conn)               ((conn)->recv_timeout)
#endif /* LWIP_SO_RCVTIMEO */
#if LWIP_SO_RCVBUF
/** Set the receive buffer in bytes */
#define netconn_set_recvbufsize(conn, recvbufsize)  ((conn)->recv_bufsize = (recvbufsize))
/** Get the receive buffer in bytes */
#define netconn_get_recvbufsize(conn)               ((conn)->recv_bufsize)
#endif /* LWIP_SO_RCVBUF*/

#ifdef __cplusplus
}
#endif

#endif /* LWIP_NETCONN */

#endif /* __LWIP_API_H__ */
