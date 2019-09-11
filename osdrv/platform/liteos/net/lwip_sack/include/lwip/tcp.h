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

 /**
 * @file tcp.h
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

#ifndef __LWIP_TCP_H__
#define __LWIP_TCP_H__

#include "lwip/opt.h"

#if LWIP_TCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/ip.h"
#include "lwip/icmp.h"
#include "lwip/err.h"
#include "arch/cc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_pcb;
struct tcp_info;

/**
 * @ingroup Config_TCP
 * Function prototype for tcp accept callback functions. Called when a new
 * connection can be accepted on a listening pcb.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param newpcb The new connection pcb
 * @param err An error code if there has been an error accepting.
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
typedef err_t (*tcp_accept_fn)(void *arg, struct tcp_pcb *newpcb, err_t err);

/**
 * @ingroup Config_TCP
 * Function prototype for tcp receive callback functions. Called when data has
 * been received.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb The connection pcb which received data
 * @param p The received data (or NULL when the connection has been closed!)
 * @param err An error code if there has been an error receiving
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
typedef err_t (*tcp_recv_fn)(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err);

/**
 * @ingroup Config_TCP
 * Function prototype for tcp sent callback functions. Called when sent data has
 * been acknowledged by the remote side. Use it to free corresponding resources.
 * This also means that the pcb has now space available to send new data.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb The connection pcb for which data has been acknowledged
 * @param len The amount of bytes acknowledged
 * @return ERR_OK: try to send some data by calling tcp_output
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
typedef err_t (*tcp_sent_fn)(void *arg, struct tcp_pcb *tpcb,
                              u16_t len);

/**
 * @ingroup Config_TCP
 * Function prototype for tcp poll callback functions. Called periodically as
 * specified by @see tcp_poll.
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb tcp pcb
 * @return ERR_OK: try to send some data by calling tcp_output
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 */
typedef err_t (*tcp_poll_fn)(void *arg, struct tcp_pcb *tpcb);

/**
 * @ingroup Config_TCP
 * Function prototype for tcp error callback functions. Called when the pcb
 * receives a RST or is unexpectedly closed for any other reason.
 *
 * @note The corresponding pcb is already freed when this callback is called!
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param err Error code to indicate why the pcb has been closed
 *            ERR_ABRT: aborted through tcp_abort or by a TCP timer
 *            ERR_RST: the connection was reset by the remote host
 */
typedef void  (*tcp_err_fn)(void *arg, err_t err);

/**
 * @ingroup Config_TCP
 * Function prototype for tcp connected callback functions. Called when a pcb
 * is connected to the remote side after initiating a connection attempt by
 * calling tcp_connect().
 *
 * @param arg Additional argument to pass to the callback function (@see tcp_arg())
 * @param tpcb The connection pcb which is connected
 * @param err An unused error code, always ERR_OK currently ;-) TODO!
 *            Only return ERR_ABRT if you have called tcp_abort from within the
 *            callback function!
 *
 * @note When a connection attempt fails, the error callback is currently called!
 */
typedef err_t (*tcp_connected_fn)(void *arg, struct tcp_pcb *tpcb, err_t err);

/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
typedef void (*tcp_event_sndplus_fn)(void *arg, struct tcp_pcb *tpcb);
#endif
/* NETIF DRIVER STATUS END */

/* Adding for WINDOW SCALE | SACK */
#if (LWIP_WND_SCALE || LWIP_SACK)
typedef u16_t tcpflags_t;
#else
typedef u8_t  tcpflags_t;
#endif

#if LWIP_WND_SCALE
typedef u32_t tcpwnd_size_t;
#define ACTUAL_WND_SIZE(pcb, wnd) ((tcpwnd_size_t)((wnd) >> (pcb)->rcv_scale) << (pcb)->rcv_scale)
#define RCV_WND_SCALE(pcb, wnd) (((wnd) >> (pcb)->rcv_scale))
#define SND_WND_SCALE(pcb, wnd) (((tcpwnd_size_t)(wnd) << (pcb)->snd_scale))
#else
typedef u16_t tcpwnd_size_t;
#define RCV_WND_SCALE(pcb, wnd) (wnd)
#define SND_WND_SCALE(pcb, wnd) (wnd)
#endif

#if LWIP_SACK
#ifndef LWIP_CONGCNTRL_DUPACK_THRESH
#define DUPACK_THRESH       3
#else
#define DUPACK_THRESH       LWIP_CONGCNTRL_DUPACK_THRESH
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_CWND
#define INITIAL_CWND       20
#else
#define INITIAL_CWND       LWIP_CONGCNTRL_INITIAL_CWND
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_SSTHRESH
#define INITIAL_SSTHRESH       200
#else
#define INITIAL_SSTHRESH       LWIP_CONGCNTRL_INITIAL_SSTHRESH
#endif

#else
#ifndef LWIP_CONGCNTRL_DUPACK_THRESH
#define DUPACK_THRESH       3
#else
#define DUPACK_THRESH       LWIP_CONGCNTRL_DUPACK_THRESH
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_CWND
#define INITIAL_CWND       2
#else
#define INITIAL_CWND       LWIP_CONGCNTRL_INITIAL_CWND
#endif

#ifndef LWIP_CONGCNTRL_INITIAL_SSTHRESH
#define INITIAL_SSTHRESH       10
#else
#define INITIAL_SSTHRESH       LWIP_CONGCNTRL_INITIAL_SSTHRESH
#endif
#endif

/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
#define DRV_READY         1
#define DRV_NOT_READY     0
#endif
/* NETIF DRIVER STATUS END */

enum tcp_state {
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  CLOSE_WAIT  = 5,
  FIN_WAIT_1  = 6,
  CLOSING     = 7,
  LAST_ACK    = 8,
  FIN_WAIT_2  = 9,
  TIME_WAIT   = 10
};

/* Adding for SACK */
#if LWIP_SACK
struct _sack_seq {
  struct _sack_seq *next;
  u32_t left;
  u32_t right;
  u32_t order;
};
#endif

#if LWIP_CALLBACK_API
  /* Function to call when a listener has been connected.
   * @param arg user-supplied argument (tcp_pcb.callback_arg)
   * @param pcb a new tcp_pcb that now is connected
   * @param err an error argument (TODO: that is current always ERR_OK?)
   * @return ERR_OK: accept the new connection,
   *                 any other err_t abortsthe new connection
   */
#define DEF_ACCEPT_CALLBACK  tcp_accept_fn accept;
#else /* LWIP_CALLBACK_API */
#define DEF_ACCEPT_CALLBACK
#endif /* LWIP_CALLBACK_API */

#if LWIP_SACK_PERF_OPT
struct tcp_sack_fast_rxmited {
  struct tcp_sack_fast_rxmited *next;
  struct tcp_seg *seg;
};
#endif

/**
 * members common to struct tcp_pcb and struct tcp_listen_pcb
 */
#define TCP_PCB_COMMON(type) \
  type *next; /* for the linked list */ \
  void *callback_arg; \
  /* the accept callback for listen- and normal pcbs, if LWIP_CALLBACK_API */ \
  DEF_ACCEPT_CALLBACK \
  /* ports are in host byte order */ \
  u16_t local_port;\
  u8_t state; /* TCP state -enum tcp_state*/ \
  u8_t prio; \
  tcpwnd_size_t snd_buf_static




/* the TCP protocol control block */
struct tcp_pcb {
/** common PCB members */
  IP_PCB;
/** protocol specific PCB members */
  TCP_PCB_COMMON(struct tcp_pcb);

  /* ports are in host byte order */
  u16_t remote_port;

  tcpflags_t flags;  //u16_t
#define TF_ACK_DELAY    ((tcpflags_t)0x0001U)   /* Delayed ACK. */
#define TF_ACK_NOW      ((tcpflags_t)0x0002U)   /* Immediate ACK. */
#define TF_INFR         ((tcpflags_t)0x0004U)   /* In fast recovery. */
#define TF_TIMESTAMP    ((tcpflags_t)0x0008U)   /* Timestamp option enabled */
#define TF_RXCLOSED     ((tcpflags_t)0x0010U)   /* rx closed by tcp_shutdown */
#define TF_FIN          ((tcpflags_t)0x0020U)   /* Connection was closed locally (FIN segment enqueued). */
#define TF_NODELAY      ((tcpflags_t)0x0040U)   /* Disable Nagle algorithm */
#define TF_NAGLEMEMERR  ((tcpflags_t)0x0080U)   /* nagle enabled, memerr, try to output to prevent delayed ACK to happen */
#if LWIP_WND_SCALE
#define TF_WND_SCALE    ((tcpflags_t)0x0100U)   /* Window Scale option enabled */
#endif

  /* Adding for SACK */
#if LWIP_SACK
#define TF_SACK         ((tcpflags_t)0x0200U)   /* TCP SACK option enabled */
#define TF_IN_SACK_FRLR ((tcpflags_t)0x0400U)   /* In SACK based Fast retransmit & loss recovery algorithm */
#define TF_IN_SACK_RTO  ((tcpflags_t)0x0800U)   /* From loss receovery it entered to Retrasnmit timeout state */
#endif
/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
#define TF_RST_ON_DRV_WAKE  ((tcpflags_t)0x1000U)   /* From loss receovery it entered to Retrasnmit timeout state */
#endif
/* NETIF DRIVER STATUS END */

  /* the rest of the fields are in host byte order
     as we have to do some math with them */

/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
  /* Netif driver status */
  u8_t drv_status; /* 0 - Driver not ready. 1- Driver is ready */
#endif
/* NETIF DRIVER STATUS END */

  /* Timers */
  u8_t polltmr, pollinterval;
  u8_t last_timer;
  u32_t tmr;

  /* receiver variables */
  u32_t rcv_nxt;   /* next seqno expected */
  tcpwnd_size_t rcv_wnd;   /* receiver window available */
  tcpwnd_size_t rcv_ann_wnd; /* receiver window to announce */
  u32_t rcv_ann_right_edge; /* announced right edge of window */

  /* Retransmission timer. */
  s16_t rtime;

  u16_t mss;   /* maximum segment size */
  u16_t snd_mss;   /* the send mss in tcp syn*/
  u16_t rcv_mss;   /* the recv mss in tcp syn*/
  /* RTT (round trip time) estimation variables */
  u32_t rttest; /* RTT estimate in 500ms ticks */
  u32_t rtseq;  /* sequence number being timed */
  s16_t sa, sv; /* @todo document this */

  s16_t rto;    /* retransmission time-out */
  u8_t nrtx;    /* number of retransmissions */

  /* fast retransmit/recovery */
  u8_t dupacks;
  u8_t pad_tcp1[2];
  /* Highest acknowledged seqno. */
  /* This will also be used as HighACK variable as per RFC 6675 */
  u32_t lastack;

  /* congestion avoidance/control variables */
  tcpwnd_size_t cwnd;
  tcpwnd_size_t ssthresh;

  /* sender variables */
  u32_t snd_nxt;   /* next new seqno to be sent */
  u32_t snd_sml;   /* The last byte of the most recently transmitted small packet */
  u32_t snd_wl1, snd_wl2; /* Sequence and acknowledgement numbers of last
                             window update. */
  u32_t snd_lbb;       /* Sequence number of next byte to be buffered. */
  /* Adding for WINDOW SCALE */
  tcpwnd_size_t snd_wnd;   /* sender window */
  tcpwnd_size_t snd_wnd_max; /* the maximum sender window announced by the remote host */
  tcpwnd_size_t acked;
  tcpwnd_size_t snd_buf;   /* Available buffer space for sending (in bytes). */

  /* These are ordered by sequence number: */
  struct tcp_seg *unsent;   /* Unsent (queued) segments. */
  struct tcp_seg *unacked;  /* Sent but unacknowledged segments. */
#if TCP_QUEUE_OOSEQ
  struct tcp_seg *ooseq;    /* Received out of sequence segments. */
#endif /* TCP_QUEUE_OOSEQ */

  struct pbuf *refused_data; /* Data previously received but not yet taken by upper layer */

#if LWIP_CALLBACK_API
  /* Function to be called when more send buffer space is available. */
  tcp_sent_fn sent;
  /* Function to be called when (in-sequence) data has arrived. */
  tcp_recv_fn recv;
  /* Function to be called when a connection has been set up. */
  tcp_connected_fn connected;
  /* Function which is called periodically. */
  tcp_poll_fn poll;
  /* Function to be called whenever a fatal error occurs. */
  tcp_err_fn errf;
#endif /* LWIP_CALLBACK_API */
/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
  tcp_event_sndplus_fn  sndplus;
#endif
/* NETIF DRIVER STATUS END */

#if LWIP_TCP_TIMESTAMPS
  u32_t ts_lastacksent;
  u32_t ts_recent;
#endif /* LWIP_TCP_TIMESTAMPS */

  /* idle time before KEEPALIVE is sent */
  u32_t keep_idle;
#if LWIP_TCP_KEEPALIVE
  u32_t keep_intvl;
  u32_t keep_cnt;
#endif /* LWIP_TCP_KEEPALIVE */

  /* Persist timer counter */
  u8_t persist_cnt;
  /* Persist timer back-off */
  u8_t persist_backoff;

  /* KEEPALIVE counter */
  u8_t keep_cnt_sent;
  u8_t pad_tcp3;


  u32_t tcpwnd;    /* TCP_WND value of each connection is stored here */
  u32_t   last_payload_len;

  /* Adding for SACK */
#if LWIP_SACK
  struct _sack_seq *sack_seq;
  /* Pipe to hold the number of octets available in network */
  u32_t pipe;
  /* Recovery point for stopping loss recovery phase */
  u32_t recovery_point;
  /* highest octet covered by any received SACK block */
  u32_t high_sacked;
  /* Loss recovery check needs to be started from this seg instead of pcb->unacked */
  /* This excludes the rexmited and sacked seg from pcb->unacked */
  /* That means, next_seg_for_lr points to first unsacked and  not retransmitted seg */
  struct tcp_seg *next_seg_for_lr;
  /* highest data trasnfered so far, equivalent to snd_nxt */
  u32_t high_data;
  /* highest octet retransmitted so far, as part of SACK based loss recovery algorithm */
  u32_t high_rxt;
  /* Rescure rxt as per loss recovery algorithm */
  u32_t rescue_rxt;

  u32_t num_sacks;
  u32_t ooseq_cnt;

#if LWIP_SACK_PERF_OPT
  u32_t pkt_seq_num;  // packet order on which they are transmitted out of stack
  u32_t high_sacked_pkt_seq;
  struct tcp_sack_fast_rxmited *fr_segs; /* List of fast retransmitted segments */
  struct tcp_sack_fast_rxmited *last_frseg; /* The latest fast retransmitted segment. This stores the latest
                                             * fast retransmitted segment so that when more segments are retransmitted,
                                             * it can be appended to this segmetn without iterating whole list */
#if LWIP_SACK_CWND_OPT
  tcpwnd_size_t recover_cwnd;
  tcpwnd_size_t recover_ssthresh;
#endif /* LWIP_SACK_CWND_OPT */
#endif /*LWIP_SACK_PERF_OPT*/
#endif /*LWIP_SACK*/

  /* Adding for WINDOW SCALE */
#if LWIP_WND_SCALE
#define TCP_SNDQUEUELEN_OVERFLOW (0x03ffffffU-3)
  u32_t snd_queuelen; /* Available buffer space for sending (in tcp_segs). */
  /* Window scale */
  u8_t snd_scale;
  u8_t rcv_scale;
#else /* LWIP_WND_SCALE */
#define TCP_SNDQUEUELEN_OVERFLOW (0xffffU-3)
  u16_t snd_queuelen; /* Available buffer space for sending (in tcp_segs). */
#endif /* LWIP_WND_SCALE */

  u16_t snd_queuelen_max;
  u16_t snd_queuelen_lowat;
  tcpwnd_size_t snd_buf_lowat;

#if TCP_OVERSIZE
  /* Extra bytes available at the end of the last pbuf in unsent. */
  u16_t unsent_oversize;
#else
  u16_t pad_tcp4;
#endif /* TCP_OVERSIZE */
};

struct tcp_pcb_listen {
/* Common members of all PCB types */
  IP_PCB;
/* Protocol specific PCB members */
  TCP_PCB_COMMON(struct tcp_pcb_listen);

#if TCP_LISTEN_BACKLOG
  u8_t backlog;
  u8_t accepts_pending;
  u8_t pad[2];
#endif /* TCP_LISTEN_BACKLOG */
};

/* This struct is used to configure the below structure members
 * and same  will assign to Macros defined in this file. */



/**
* @ingroup  Config_TCP
* @code
*struct lwip_tcp_cfg_param_
{
    u32_t tcp_wnd;
    tcpwnd_size_t tcp_snd_buf;
    tcpwnd_size_t initial_cwnd;
    tcpwnd_size_t inital_ssthresh;
    u16_t tcp_mss;
    u16_t tcp_oversize;
#if LWIP_WND_SCALE
    u16_t tcp_wnd_min;
    u8_t tcp_rcv_scale;
#endif
    u8_t tcp_ttl;
    u8_t tcp_maxrtx;
    u8_t tcp_synmaxrtx;
    u8_t tcp_fw1maxrtx;
#if LWIP_WND_SCALE
    u16_t pad;
 #else
    u8_t pad;
#endif
};
* @endcode
* @par Description
* This struct is used to configure the structure members.
*
* @datastruct      tcp_wnd            Size of a TCP window.\n
* @datastruct      tcp_snd_buf      TCP sender buffer space.\n
* @datastruct      initial_cwnd       TCP initial congestion window.\n
* @datastruct      inital_ssthresh   TCP initial Threshold window.\n
* @datastruct      tcp_mss            TCP Maximum segment size.\n
* @datastruct      tcp_oversize      Maximum number of bytes TCP write may allocate.\n
* @datastruct      tcp_wnd_min     TCP minimum window value.\n
* @datastruct      tcp_rcv_scale     Shift count value for Window scale.\n
* @datastruct      tcp_ttl               Time-To-Live value.\n
* @datastruct      tcp_maxrtx        Maximum number of retransmissions of data segments.\n
* @datastruct      tcp_synmaxrtx   Maximum number of retransmissions of SYN segments.\n
* @datastruct      tcp_fw1maxrtx   Maximum number of retransmissions of data segments when socket state was FIN_WAIT_1.\n
* @datastruct      pad                   used for structure 32 bit boundary aligment.\n
*
*/
struct lwip_tcp_cfg_param_
{
    u32_t tcp_wnd;  /**< Size of a TCP window.  */
    tcpwnd_size_t tcp_snd_buf; /**<  TCP sender buffer space. */
    tcpwnd_size_t initial_cwnd;  /**< TCP initial congestion window.    */
    tcpwnd_size_t inital_ssthresh; /**< TCP initial Threshold window. */
    u16_t tcp_mss;      /**<    TCP Maximum segment size. */
    u16_t tcp_oversize;   /**< Maximum number of bytes TCP write may allocate. */
#if LWIP_WND_SCALE
    u16_t tcp_wnd_min;   /**< TCP minimum window value. */
    u8_t tcp_rcv_scale;   /**< Shift count value for Window scale. */
#endif
    u8_t tcp_ttl;     /**< Time-To-Live value */
    u8_t tcp_maxrtx;  /**< Maximum number of retransmissions of data segments. */
    u8_t tcp_synmaxrtx;  /**< Maximum number of retransmissions of SYN segments. */
    u8_t tcp_fw1maxrtx;  /**< Maximum number of retransmissions of data segments when socket state was FIN_WAIT_1. */
#if LWIP_WND_SCALE
    u16_t pad;   /**<  Used for structure 32 bit boundary alignment. */
 #else
    u8_t pad;    /**<  Used for structure 16 bit boundary alignment. */
#endif
};

/** used to update TCP stack configurations by user */
extern struct lwip_tcp_cfg_param_ glwip_tcp_cfg_param;

/* Application program's interface: */
struct tcp_pcb * tcp_new     (void);

void             tcp_arg     (struct tcp_pcb *pcb, void *arg);
void             tcp_accept  (struct tcp_pcb *pcb, tcp_accept_fn accept);
void             tcp_recv    (struct tcp_pcb *pcb, tcp_recv_fn recv);
void             tcp_sent    (struct tcp_pcb *pcb, tcp_sent_fn sent);
void             tcp_poll    (struct tcp_pcb *pcb, tcp_poll_fn poll, u8_t interval);
void             tcp_err     (struct tcp_pcb *pcb, tcp_err_fn err);

#define          tcp_mss(pcb)             (((pcb)->flags & TF_TIMESTAMP) ? ((pcb)->mss - 12)  : (pcb)->mss)
#define          tcp_sndbuf(pcb)          ((pcb)->snd_buf)
#if LWIP_SO_SNDBUF
#define          tcp_set_sendbufsize(pcb, size)    ((pcb)->snd_buf_static = (size))
#define          tcp_get_sendbufsize(pcb)    ((pcb)->snd_buf_static)
#endif /* LWIP_SO_SNDBUF */
#define          tcp_sndqueuelen(pcb)     ((pcb)->snd_queuelen)
#define          tcp_nagle_disable(pcb)   ((pcb)->flags |= TF_NODELAY)
#define          tcp_nagle_enable(pcb)    ((pcb)->flags &= (tcpflags_t)~TF_NODELAY)
#define          tcp_nagle_disabled(pcb)  (((pcb)->flags & TF_NODELAY) != 0)

#if TCP_LISTEN_BACKLOG
#define          tcp_accepted(pcb) do { \
  LWIP_ASSERT("pcb->state == LISTEN (called for wrong pcb?)", pcb->state == LISTEN); \
  (((struct tcp_pcb_listen *)(pcb))->accepts_pending--); } while(0)
#else  /* TCP_LISTEN_BACKLOG */
#define          tcp_accepted(pcb) LWIP_ASSERT("pcb->state == LISTEN (called for wrong pcb?)", \
                                               (pcb)->state == LISTEN)
#endif /* TCP_LISTEN_BACKLOG */

void             tcp_recved  (struct tcp_pcb *pcb, u16_t len);
err_t            tcp_bind    (struct tcp_pcb *pcb, ip_addr_t *ipaddr,
                              u16_t port);
err_t            tcp_connect (struct tcp_pcb *pcb, ip_addr_t *ipaddr,
                              u16_t port, tcp_connected_fn connected);

struct tcp_pcb * tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);
#define          tcp_listen(pcb) tcp_listen_with_backlog(pcb, TCP_DEFAULT_LISTEN_BACKLOG)

void             tcp_abort (struct tcp_pcb *pcb);
err_t            tcp_close   (struct tcp_pcb *pcb);
err_t            tcp_shutdown(struct tcp_pcb *pcb, int shut_rx, int shut_tx);

/* Flags for "apiflags" parameter in tcp_write */
#define TCP_WRITE_FLAG_COPY 0x01
#define TCP_WRITE_FLAG_MORE 0x02

err_t            tcp_write   (struct tcp_pcb *pcb, const void *dataptr, tcpwnd_size_t len,
                              u8_t apiflags);
void             tcp_get_info(struct tcp_pcb *pcb, struct tcp_info *tcpinfo);

#define TCP_PRIO_MIN    1
#define TCP_PRIO_NORMAL 64
#define TCP_PRIO_MAX    127


err_t            tcp_output  (struct tcp_pcb *pcb);

char * lwip_version(void);

const char* tcp_debug_state_str(enum tcp_state s);

/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
void tcp_flush_pcb_on_wake_queue(struct tcp_pcb * pcb, u8_t status);
void tcpip_flush_on_wake_queue(struct netif *netif, u8_t status);
void tcp_ip_flush_pcblist_on_wake_queue(struct netif *netif, struct tcp_pcb * pcb_list, u8_t status);
void tcpip_upd_status_to_tcp_pcbs(struct netif *netif, u8_t status);
void tcp_ip_event_sendplus_on_wake_queue(struct netif *netif);
#endif
/* NETIF DRIVER STATUS END */

static inline void tcp_sndbuf_init(struct tcp_pcb *pcb)
{
  u32_t sndqueuemax;
  tcpwnd_size_t snd_buf = pcb->snd_buf_static;
  u16_t mss = pcb->mss;

  if ((snd_buf >> 1) > ((mss << 1) + 1)) {
    if ((snd_buf >> 1) < (snd_buf - 1)) {
      pcb->snd_buf_lowat = snd_buf >> 1;
    } else {
      pcb->snd_buf_lowat = snd_buf - 1;
    }
  } else {
    if (((mss << 1) + 1) < (snd_buf - 1)) {
      pcb->snd_buf_lowat = (mss << 1) + 1;
    } else {
      pcb->snd_buf_lowat = snd_buf - 1;
    }
  }

  sndqueuemax = ((snd_buf/mss) << 3);
  if (sndqueuemax > 0xFFFF) {
    sndqueuemax = 0xFFFF;
  }
  pcb->snd_queuelen_max = (u16_t)sndqueuemax;
  pcb->snd_queuelen_lowat = pcb->snd_queuelen_max >> 1;
  if (pcb->snd_queuelen_lowat < 5) {
    pcb->snd_queuelen_lowat = LWIP_MIN(5, pcb->snd_queuelen_max);
  }
}

#ifdef __cplusplus
}
#endif

#endif /* LWIP_TCP */

#endif /* __LWIP_TCP_H__ */
