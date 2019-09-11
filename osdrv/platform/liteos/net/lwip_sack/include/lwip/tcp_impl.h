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

#ifndef __LWIP_TCP_IMPL_H__
#define __LWIP_TCP_IMPL_H__

#include "lwip/opt.h"

#if LWIP_TCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/tcp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/ip.h"
#include "lwip/icmp.h"
#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tcp_seg;
struct tcp_sack_fast_rxmited;
/* Functions for interfacing with TCP: */

/* Lower layer interface to TCP: */
void             tcp_init    (void);  /* Initialize this module. */
void             tcp_tmr     (void);  /* Must be called every
                                         TCP_TMR_INTERVAL
                                         ms. (Typically 250 ms). */

/** External function (implemented in timers.c), called when TCP detects
 * that a timer is needed (i.e. active- or time-wait-pcb found). */
void tcp_timer_needed(void);


/* Only used by IP to pass a TCP segment to TCP: */
void             tcp_input   (struct pbuf *p, struct netif *inp);
void  tcp_segs_free(struct tcp_seg *seg);
#if LWIP_SACK_PERF_OPT
void tcp_fr_segs_free(struct tcp_sack_fast_rxmited *seg);
#endif

/* Adding for user configured stack parameters*/
/*
 * CFG_TCP_TTL: Configure  Time-To-Live value.
 */
#define CFG_TCP_TTL             (glwip_tcp_cfg_param.tcp_ttl)

/*
 * CFG_TCP_MAXRTX: Configure Maximum number of retransmissions of data segments.
 */
#define CFG_TCP_MAXRTX          (glwip_tcp_cfg_param.tcp_maxrtx)
/*
 * CFG_TCP_SYNMAXRTX: Configure  Maximum number of retransmissions of SYN segments.
 */
#define CFG_TCP_SYNMAXRTX       (glwip_tcp_cfg_param.tcp_synmaxrtx)

/*
 * CFG_TCP_FW1MAXRTX: Configure  Maximum number of retransmissions of data segments in FIN_WAIT_1 or CLOSING.
 */
#define CFG_TCP_FW1MAXRTX       (glwip_tcp_cfg_param.tcp_fw1maxrtx)

#if LWIP_WND_SCALE
#define CFG_TCP_RCV_SCALE       (glwip_tcp_cfg_param.tcp_rcv_scale)
/*
 * CFG_TCP_WND_MIN  -If window scaling is enabled then this minimum receive window also
 * should be configured. So that if peer is not supporting window scaling
 * option, then this minimum receive window will be considered.
 * This value should not be greater than 0XFFFF and this value should not
 * be more than TCP_WND.
 */
#define CFG_TCP_WND_MIN         (glwip_tcp_cfg_param.tcp_wnd_min)
#endif
/*
 * CFG_TCP_WND : Configure The size of a TCP window.  This must be at least
 * (2 * TCP_MSS) for things to work well
 * This must be less than 0xFFFF if window scaling is not enabled.
 * This must be less than 0xFFFFFFFF if window scaling is enabled.
 */

#define CFG_TCP_WND             (glwip_tcp_cfg_param.tcp_wnd)

/**
 * CFG_TCP_MSS: Configure TCP maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */


#define CFG_TCP_MSS             (glwip_tcp_cfg_param.tcp_mss)
/*
 * CFG_TCP_SND_BUF: Configure TCP sender buffer space (bytes).
 */
#define CFG_TCP_SND_BUF         (glwip_tcp_cfg_param.tcp_snd_buf)
/*
 * CFG_INITIAL_CWND: Configure TCP initial congestion window (bytes).
 */

#define CFG_INITIAL_CWND        (glwip_tcp_cfg_param.initial_cwnd)
/*
 * CFG_INITIAL_SSTHRESH: Configure TCP initial Threshold window (bytes).
 */
#define CFG_INITIAL_SSTHRESH    (glwip_tcp_cfg_param.inital_ssthresh)

#define CFG_TCP_OVERSIZE        (glwip_tcp_cfg_param.tcp_oversize)

#define TCP_SEQ_LT(a,b)     ((s32_t)((u32_t)(a) - (u32_t)(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((s32_t)((u32_t)(a) - (u32_t)(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((s32_t)((u32_t)(a) - (u32_t)(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((s32_t)((u32_t)(a) - (u32_t)(b)) >= 0)
/* is b<=a<=c? */
#define TCP_SEQ_BETWEEN(a,b,c) (TCP_SEQ_GEQ(a,b) && TCP_SEQ_LEQ(a,c))

/* Minshall's variant of the Nagle send check. */
#define tcp_nagle_minshall_check(tpcb) (TCP_SEQ_GT(tpcb->snd_sml, tpcb->lastack) && TCP_SEQ_LEQ(tpcb->snd_sml, tpcb->snd_nxt))

/**
 * This is the Nagle algorithm: try to combine usrdata to send as few TCP
 * segments as possible. Only send if
 * - no previously transmitted data on the connection remains unacknowledged or
 * - the TF_NODELAY flag is set (nagle algorithm turned off for this pcb) or
 * - the only unsent segment is at least pcb->mss bytes long (or there is more
 *   than one unsent segment - with lwIP, this can happen although unsent->len < mss)
 * - or if we are in fast-retransmit (TF_INFR)
 * - With Minshall's variant: all sent small packets are ACKed.
 */
#define tcp_do_output_nagle(tpcb) ((((tpcb)->unacked == NULL) || \
                            ((tpcb)->flags & (TF_NODELAY | TF_INFR)) || \
                            (((tpcb)->unsent != NULL) && (((tpcb)->unsent->next != NULL) || \
                              ((tpcb)->unsent->len >= (tpcb)->mss) || !tcp_nagle_minshall_check(tpcb)))  \
                            ) ? 1 : 0)
#define tcp_output_nagle(tpcb) (tcp_do_output_nagle(tpcb) ? tcp_output(tpcb) : ERR_OK)


#define TCP_SEQ_LT(a,b)     ((s32_t)((u32_t)(a) - (u32_t)(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((s32_t)((u32_t)(a) - (u32_t)(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((s32_t)((u32_t)(a) - (u32_t)(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((s32_t)((u32_t)(a) - (u32_t)(b)) >= 0)
/* is b<=a<=c? */
#define TCP_SEQ_BETWEEN(a,b,c) (TCP_SEQ_GEQ(a,b) && TCP_SEQ_LEQ(a,c))
#define TCP_FIN 0x01U
#define TCP_SYN 0x02U
#define TCP_RST 0x04U
#define TCP_PSH 0x08U
#define TCP_ACK 0x10U
#define TCP_URG 0x20U
#define TCP_ECE 0x40U
#define TCP_CWR 0x80U

#define TCP_FLAGS 0x3fU

/* Length of the TCP header, excluding options. */
#define TCP_HLEN 20

#ifndef TCP_TMR_INTERVAL
#define TCP_TMR_INTERVAL       100  /* The TCP timer interval in milliseconds. */
#endif /* TCP_TMR_INTERVAL */

#ifndef TCP_FAST_INTERVAL
#define TCP_FAST_INTERVAL      TCP_TMR_INTERVAL /* the fine grained timeout in milliseconds */
#endif /* TCP_FAST_INTERVAL */

#ifndef TCP_SLOW_INTERVAL
#define TCP_SLOW_INTERVAL_PERIOD  5
#define TCP_SLOW_INTERVAL      (TCP_SLOW_INTERVAL_PERIOD * TCP_TMR_INTERVAL)  /* the coarse grained timeout in milliseconds */
#endif /* TCP_SLOW_INTERVAL */

/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
/* will round off to the higher value in case it is not multiples of TCP_SLOW_INTERVAL */
#define DRIVER_WAKEUP_COUNT ((DRIVER_WAKEUP_INTERVAL + (TCP_SLOW_INTERVAL -1))/TCP_SLOW_INTERVAL)
#endif
/* NETIF DRIVER STATUS END */

#define TCP_FIN_WAIT_TIMEOUT 20000 /* milliseconds */
#define TCP_SYN_RCVD_TIMEOUT 20000 /* milliseconds */

#define TCP_OOSEQ_TIMEOUT        6 /* x RTO */

#ifndef TCP_MSL
#define TCP_MSL 60000UL /* The maximum segment lifetime in milliseconds */
#endif

/* Keepalive values, compliant with RFC 1122. Don't change this unless you know what you're doing */
#ifndef  TCP_KEEPIDLE_DEFAULT
#define  TCP_KEEPIDLE_DEFAULT     7200000UL /* Default KEEPALIVE timer in milliseconds */
#endif

#ifndef  TCP_KEEPINTVL_DEFAULT
#define  TCP_KEEPINTVL_DEFAULT    75000UL   /* Default Time between KEEPALIVE Signals in milliseconds */
#endif

#ifndef  TCP_KEEPCNT_DEFAULT
#define  TCP_KEEPCNT_DEFAULT      9U        /* Default Counter for KEEPALIVE Signals */
#endif

#define  TCP_MAXIDLE              (TCP_KEEPCNT_DEFAULT * TCP_KEEPINTVL_DEFAULT)  /* Maximum KEEPALIVE Signals time */

/* Fields are (of course) in network byte order.
 * Some fields are converted to host byte order in tcp_input().
 */
PACK_STRUCT_BEGIN
struct tcp_hdr {
  PACK_STRUCT_FIELD(u16_t src);
  PACK_STRUCT_FIELD(u16_t dest);
  PACK_STRUCT_FIELD(u32_t seqno);
  PACK_STRUCT_FIELD(u32_t ackno);
  PACK_STRUCT_FIELD(u16_t _hdrlen_rsvd_flags);
  PACK_STRUCT_FIELD(u16_t wnd);
  PACK_STRUCT_FIELD(u16_t chksum);
  PACK_STRUCT_FIELD(u16_t urgp);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

#define TCPH_HDRLEN(phdr) (ntohs((phdr)->_hdrlen_rsvd_flags) >> 12)
#define TCPH_FLAGS(phdr)  (ntohs((phdr)->_hdrlen_rsvd_flags) & TCP_FLAGS)

#define TCPH_HDRLEN_SET(phdr, len) ((phdr)->_hdrlen_rsvd_flags = htons(((len) << 12) | TCPH_FLAGS(phdr)))
#define TCPH_FLAGS_SET(phdr, flags) ((phdr)->_hdrlen_rsvd_flags = (u16_t)((((phdr)->_hdrlen_rsvd_flags & PP_HTONS((u16_t)(~(u16_t)(TCP_FLAGS)))) | htons(flags))))
#define TCPH_HDRLEN_FLAGS_SET(phdr, len, flags) ((phdr)->_hdrlen_rsvd_flags = htons((u16_t)(((len) << 12) | (flags))))

#define TCPH_SET_FLAG(phdr, flags ) ((phdr)->_hdrlen_rsvd_flags = ((phdr)->_hdrlen_rsvd_flags | htons(flags)))
#define TCPH_UNSET_FLAG(phdr, flags) ((phdr)->_hdrlen_rsvd_flags = htons(ntohs((phdr)->_hdrlen_rsvd_flags) | (TCPH_FLAGS(phdr) & ~(flags))))

#define TCP_TCPLEN(seg) ((seg)->len + (u16_t)((TCPH_FLAGS((seg)->tcphdr) & (TCP_FIN | TCP_SYN)) != 0))

/** Flags used on input processing, not on pcb->flags
*/
#define TF_RESET     ((u8_t)0x08U)   /* Connection was reset. */
#define TF_CLOSED    ((u8_t)0x10U)   /* Connection was sucessfully closed. */
#define TF_GOT_FIN   ((u8_t)0x20U)   /* Connection was closed by the remote end. */

/** Enabled extra-check for TCP_OVERSIZE if LWIP_DEBUG is enabled */
#if TCP_OVERSIZE
#define TCP_OVERSIZE_DBGCHECK 0
#else
#define TCP_OVERSIZE_DBGCHECK 0
#endif

/** Don't generate checksum on copy if CHECKSUM_GEN_TCP is disabled */
#define TCP_CHECKSUM_ON_COPY  (LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_TCP)

/* This structure represents a TCP segment on the unsent, unacked and ooseq queues */
struct tcp_seg {
  struct tcp_seg *next;    /* used when putting segements on a queue */
  struct pbuf *p;          /* buffer containing data + TCP header */
  u16_t len;               /* the TCP length of this segment */
#if TCP_OVERSIZE_DBGCHECK
  u16_t oversize_left;     /* Extra bytes available at the end of the last
                              pbuf in unsent (used for asserting vs.
                              tcp_pcb.unsent_oversized only) */
#else
  u16_t pad1;
#endif /* TCP_OVERSIZE_DBGCHECK */
#if TCP_CHECKSUM_ON_COPY
  u16_t chksum;
  u8_t  chksum_swapped;
#endif /* TCP_CHECKSUM_ON_COPY */
  u8_t  flags;
#define TF_SEG_OPTS_MSS         ((u8_t)0x01U) /* Include MSS option. */
#define TF_SEG_OPTS_TS          ((u8_t)0x02U) /* Include timestamp option. */
#define TF_SEG_DATA_CHECKSUMMED ((u8_t)0x04U) /* ALL data (not the header) is
                                               checksummed into 'chksum' */
  /* Adding for WINDOW SCALE */
#define TF_SEG_OPTS_WND_SCALE   ((u8_t)0x08U) /* Include WND SCALE option */
  /* Adding for SACK */
#define TF_SEG_OPTS_SACK        ((u8_t)0x10U) /* Include SACK option */
#define TF_SEG_OPTS_SACK_PERMITTED        ((u8_t)0x20U) /* Include SACK Permitted option */
#define TF_SEG_OPTS_SACK_OPTIONS          ((u8_t)0x40U) /* Include SACK Options */

  /* Adding for SACK */
#if LWIP_SACK
  u32_t  state;         /* This will be used to maintain SACK scoreboard as per RFC 6675 */
#define TF_SEG_NONE            0x0000U
#define TF_SEG_SACKED          0x0001U /* Segment Sacked */
#define TF_SEG_RETRANSMITTED   0x0002U /* Retransmitted as part of SACK based loss recovery alg */

  u32_t order; /* order of the segment in the ooseq q */
#if LWIP_SACK_PERF_OPT
  u32_t pkt_trans_seq_cntr;
#endif
#endif /* LWIP_SACK */
  u8_t  rexmit;
  u8_t pad[3];
/* NETIF DRIVER STATUS BEGIN */
#if DRIVER_STATUS_CHECK
  u32_t seg_type;
#endif
/* NETIF DRIVER STATUS END */

  struct tcp_hdr *tcphdr;  /* the TCP header */
};

/* Adding for WINDOW SCALE */
#define LWIP_TCP_OPT_EOL        0
#define LWIP_TCP_OPT_NOP        1
#define LWIP_TCP_OPT_MSS        2
#define LWIP_TCP_OPT_WS         3
#define LWIP_TCP_OPT_TS         8
/* Adding for SACK */
#define LWIP_TCP_OPT_SACK_PERMITTED   4
#define LWIP_TCP_OPT_SACK             5

#define LWIP_TCP_OPT_LEN_MSS    4
#if LWIP_TCP_TIMESTAMPS
#define LWIP_TCP_OPT_LEN_TS     10
#define LWIP_TCP_OPT_LEN_TS_OUT 12 /* aligned for output (includes NOP padding) */
#else
#define LWIP_TCP_OPT_LEN_TS_OUT 0
#endif
#if LWIP_WND_SCALE
#define LWIP_TCP_OPT_LEN_WS     3
#define LWIP_TCP_OPT_LEN_WS_OUT 4 /* aligned for output (includes NOP padding) */
#else
#define LWIP_TCP_OPT_LEN_WS_OUT 0
#endif
/* Adding for SACK */
#if LWIP_SACK
#define LWIP_TCP_OPT_LEN_SACK_PERMITTED     2
#define LWIP_TCP_OPT_LEN_SACK_PERMITTED_OUT 4 /* aligned for output (includes NOP padding) */
#else
#define LWIP_TCP_OPT_LEN_SACK_PERMITTED_OUT 0
#endif



/* Adding for SACK */
#define LWIP_TCP_OPT_LENGTH(flags) \
  ((flags & TF_SEG_OPTS_MSS       ? LWIP_TCP_OPT_LEN_MSS    : 0) + \
  (flags & TF_SEG_OPTS_TS        ? LWIP_TCP_OPT_LEN_TS_OUT : 0) + \
  (flags & TF_SEG_OPTS_WND_SCALE ? LWIP_TCP_OPT_LEN_WS_OUT : 0) + \
  (flags & TF_SEG_OPTS_SACK_PERMITTED ? LWIP_TCP_OPT_LEN_SACK_PERMITTED_OUT : 0))

/*4 -> padding + kind + length. 8 -> each SACK block */
#define LWIP_TCP_SACK_OPT_LENGTH(__cnt)    (__cnt > 0? (u8_t)(4 + (__cnt * 8)) : 0)

/** This returns a TCP header option for MSS in an u32_t */
#define TCP_BUILD_MSS_OPTION(mss) htonl(0x02040000 | ((mss) & 0xFFFF))

/* Adding for WINDOW SCALE */
#if LWIP_WND_SCALE
#define TCPWNDSIZE_F  U32_F
#define TCPWND_MAX    0xFFFFFFFFU
#else /* LWIP_WND_SCALE */
#define TCPWNDSIZE_F  U16_F
#define TCPWND_MAX    0xFFFFU
#endif /* LWIP_WND_SCALE */
/* Global variables: */
extern struct tcp_pcb *tcp_input_pcb;
extern u32_t tcp_ticks;
extern u8_t tcp_active_pcbs_changed;

/* The TCP PCB lists. */
union tcp_listen_pcbs_t { /* List of all TCP PCBs in LISTEN state. */
  struct tcp_pcb_listen *listen_pcbs;
  struct tcp_pcb *pcbs;
};
extern struct tcp_pcb *tcp_bound_pcbs;
extern union tcp_listen_pcbs_t tcp_listen_pcbs;
extern struct tcp_pcb *tcp_active_pcbs;  /* List of all TCP PCBs that are in a
              state in which they accept or send
              data. */
extern struct tcp_pcb *tcp_tw_pcbs;      /* List of all TCP PCBs in TIME-WAIT. */

extern struct tcp_pcb *tcp_tmp_pcb;      /* Only used for temporary storage. */

/* Axioms about the above lists:
   1) Every TCP PCB that is not CLOSED is in one of the lists.
   2) A PCB is only in one of the lists.
   3) All PCBs in the tcp_listen_pcbs list is in LISTEN state.
   4) All PCBs in the tcp_tw_pcbs list is in TIME-WAIT state.
*/
/* Define two macros, TCP_REG and TCP_RMV that registers a TCP PCB
   with a PCB list or removes a PCB from a list, respectively. */
#ifndef TCP_DEBUG_PCB_LISTS
#define TCP_DEBUG_PCB_LISTS 0
#endif
#if TCP_DEBUG_PCB_LISTS
#ifdef LWIP_DEV_DEBUG
#define TCP_REG(pcbs, npcb) do {\
                            LWIP_DEBUGF(TCP_DEBUG, ("TCP_REG %p local port %d\n", (npcb), (npcb)->local_port)); \
                            for(tcp_tmp_pcb = *(pcbs); \
          tcp_tmp_pcb != NULL; \
        tcp_tmp_pcb = tcp_tmp_pcb->next) { \
                                LWIP_ASSERT("TCP_REG: already registered\n", tcp_tmp_pcb != (npcb)); \
                            } \
                            LWIP_ASSERT("TCP_REG: pcb->state != CLOSED", ((pcbs) == &tcp_bound_pcbs) || ((npcb)->state != CLOSED)); \
                            (npcb)->next = *(pcbs); \
                            LWIP_ASSERT("TCP_REG: npcb->next != npcb", (npcb)->next != (npcb)); \
                            *(pcbs) = (npcb); \
                            LWIP_ASSERT("TCP_RMV: tcp_pcbs sane", tcp_pcbs_sane()); \
              tcp_timer_needed(); \
                            } while(0)
#else
#define TCP_REG(pcbs, npcb) do {\
                            for(tcp_tmp_pcb = *(pcbs); \
          tcp_tmp_pcb != NULL; \
        tcp_tmp_pcb = tcp_tmp_pcb->next) { \
                                LWIP_ASSERT("TCP_REG: already registered\n", tcp_tmp_pcb != (npcb)); \
                            } \
                            LWIP_ASSERT("TCP_REG: pcb->state != CLOSED", ((pcbs) == &tcp_bound_pcbs) || ((npcb)->state != CLOSED)); \
                            (npcb)->next = *(pcbs); \
                            LWIP_ASSERT("TCP_REG: npcb->next != npcb", (npcb)->next != (npcb)); \
                            *(pcbs) = (npcb); \
                            LWIP_ASSERT("TCP_RMV: tcp_pcbs sane", tcp_pcbs_sane()); \
              tcp_timer_needed(); \
                            } while(0)
#endif
#define TCP_RMV(pcbs, npcb) do { \
                            LWIP_ASSERT("TCP_RMV: pcbs != NULL", *(pcbs) != NULL); \
                            LWIP_DEBUGF(TCP_DEBUG, ("TCP_RMV: removing %p from %p\n", (npcb), *(pcbs))); \
                            if(*(pcbs) == (npcb)) { \
                               *(pcbs) = (*pcbs)->next; \
                            } else for(tcp_tmp_pcb = *(pcbs); tcp_tmp_pcb != NULL; tcp_tmp_pcb = tcp_tmp_pcb->next) { \
                               if(tcp_tmp_pcb->next == (npcb)) { \
                                  tcp_tmp_pcb->next = (npcb)->next; \
                                  break; \
                               } \
                            } \
                            (npcb)->next = NULL; \
                            LWIP_ASSERT("TCP_RMV: tcp_pcbs sane", tcp_pcbs_sane()); \
                            LWIP_DEBUGF(TCP_DEBUG, ("TCP_RMV: removed %p from %p\n", (npcb), *(pcbs))); \
                            } while(0)

#else /* LWIP_DEBUG */

#define TCP_REG(pcbs, npcb)                        \
  do {                                             \
    (npcb)->next = *pcbs;                          \
    *(pcbs) = (npcb);                              \
    tcp_timer_needed();                            \
  } while (0)

#define TCP_RMV(pcbs, npcb)                        \
  do {                                             \
    if(*(pcbs) == (npcb)) {                        \
      (*(pcbs)) = (*pcbs)->next;                   \
    }                                              \
    else {                                         \
      for(tcp_tmp_pcb = *pcbs;                     \
          tcp_tmp_pcb != NULL;                     \
          tcp_tmp_pcb = tcp_tmp_pcb->next) {       \
        if(tcp_tmp_pcb->next == (npcb)) {          \
          tcp_tmp_pcb->next = (npcb)->next;        \
          break;                                   \
        }                                          \
      }                                            \
    }                                              \
    (npcb)->next = NULL;                           \
  } while(0)

#endif /* LWIP_DEBUG */

#define TCP_REG_ACTIVE(npcb)                       \
  do {                                             \
    TCP_REG(&tcp_active_pcbs, npcb);               \
    tcp_active_pcbs_changed = 1;                   \
  } while (0)

#define TCP_RMV_ACTIVE(npcb)                       \
  do {                                             \
    TCP_RMV(&tcp_active_pcbs, npcb);               \
    tcp_active_pcbs_changed = 1;                   \
  } while (0)

#define TCP_PCB_REMOVE_ACTIVE(pcb)                 \
  do {                                             \
    tcp_pcb_remove(&tcp_active_pcbs, pcb);         \
    tcp_active_pcbs_changed = 1;                   \
  } while (0)


#define tcp_ack(pcb)                               \
  do {                                             \
    if((pcb)->flags & TF_ACK_DELAY) {              \
      (pcb)->flags = (tcpflags_t)((pcb)->flags & ~TF_ACK_DELAY);               \
      (pcb)->flags = (tcpflags_t)((pcb)->flags | TF_ACK_NOW);                  \
    }                                              \
    else {                                         \
      (pcb)->flags =(tcpflags_t)((pcb)->flags | TF_ACK_DELAY);                \
    }                                              \
  } while (0)

#define tcp_ack_now(pcb)                           \
  do {                                             \
    (pcb)->flags |= TF_ACK_NOW;                    \
  } while (0)


#ifdef __cplusplus
}
#endif

#endif /* LWIP_TCP */

#endif /* __LWIP_TCP_H__ */
