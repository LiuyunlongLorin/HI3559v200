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

#ifndef __LWIP_DEF_H__
#define __LWIP_DEF_H__

/* arch.h might define NULL already */
#include "lwip/arch.h"
#include "lwip/opt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LWIP_MAX(x , y)  (((x) > (y)) ? (x) : (y))
#define LWIP_MIN(x , y)  (((x) < (y)) ? (x) : (y))

/* Endianess-optimized shifting of two u8_t to create one u16_t */
#if BYTE_ORDER == LITTLE_ENDIAN
#define LWIP_MAKE_U16(a, b) ((a << 8) | b)
#else
#define LWIP_MAKE_U16(a, b) ((b << 8) | a)
#endif

#ifndef LWIP_PLATFORM_BYTESWAP
#define LWIP_PLATFORM_BYTESWAP 0
#endif

/* workaround for naming collisions on some platforms */
#ifdef htons
#define lwip_htons htons
#else
#if BYTE_ORDER == BIG_ENDIAN
#define lwip_htons(x) (x)
#else
u16_t lwip_htons(u16_t x);
#endif
#endif /* htons */
#ifdef htonl
#define lwip_htonl htonl
#else
#if BYTE_ORDER == BIG_ENDIAN
#define lwip_htonl(x) (x)
#else
u32_t lwip_htonl(u32_t x);
#endif
#endif /* htonl */
#ifdef ntohs
#define lwip_ntohs ntohs
#else
#if BYTE_ORDER == BIG_ENDIAN
#define lwip_ntohs(x) (x)
#else
u16_t lwip_ntohs(u16_t x);
#endif
#endif /* ntohs */

#ifdef ntohl
#define lwip_ntohl ntohl
#else
#if BYTE_ORDER == BIG_ENDIAN
#define lwip_ntohl(x) (x)
#else
u32_t lwip_ntohl(u32_t x);
#endif
#endif /* ntohl */

#if BYTE_ORDER == BIG_ENDIAN
#define PP_HTONS(x) (x)
#define PP_NTOHS(x) (x)
#define PP_HTONL(x) (x)
#define PP_NTOHL(x) (x)
#else /* BYTE_ORDER != BIG_ENDIAN */

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) /*lint -e778*/ ((((x) & 0xff) << 8) | \
                    /*lint -e572*/(((x) & 0xff00) >> 8))/*lint +e778*/ /*lint +e572*/
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) /*lint -e778*/((((x) & 0xff) << 24) | \
                        (((x) & 0xff00) << 8) | \
                    /*lint -e572*/(((x) & 0xff0000UL) >> 8) | \
                                  (((x) & 0xff000000UL) >> 24))/*lint +e572*/ /*lint +e778*/
#define PP_NTOHL(x) PP_HTONL(x)

#endif /* BYTE_ORDER == BIG_ENDIAN */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_DEF_H__ */

