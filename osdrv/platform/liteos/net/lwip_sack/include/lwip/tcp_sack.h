/**********************************************************************************
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************/

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

#ifndef __LWIP_TCP_SACK_CA__
#define __LWIP_TCP_SACK_CA__

#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NETIF DRIVER STATUS BEGIN */
#define UNSACKED_AND_LOST_SEG       0x0001U
#define UNSENT_SEG                  0x0002U
#define UNSACKED_SEG                0x0004U
#define RESCUE_RX_SEG               0x0008U
#if DRIVER_STATUS_CHECK
#define FAST_RETX_SEG               0x0010U /* First Segment retransmitted as part of Fast retransmit algorithm */
#define SEG_TYPE_NONE               0x0000U
#endif
/* NETIF DRIVER STATUS END */

extern u32_t
tcp_parseopt_sack(u8_t *opts, u16_t c);

extern u32_t
tcp_sack_update(struct tcp_pcb *pcb, u32_t ackno);

extern void
tcp_sack_based_fast_rexmit_alg(struct tcp_pcb *pcb);

extern void
tcp_sack_based_loss_recovery_alg(struct tcp_pcb *pcb);

#if LWIP_SACK_PERF_OPT
extern void
tcp_sack_rexmit_lost_rexmitted(struct tcp_pcb *pcb);
#endif

extern void
tcp_sack_set_pipe(struct tcp_pcb *pcb);

extern int
tcp_sack_is_lost(struct tcp_pcb *pcb, struct tcp_seg *seg);


#ifdef __cplusplus
}
#endif

#endif
