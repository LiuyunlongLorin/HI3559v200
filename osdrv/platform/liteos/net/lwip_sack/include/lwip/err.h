/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
#ifndef __LWIP_ERR_H__
#define __LWIP_ERR_H__

#include "lwip/opt.h"
#include "lwip/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Define LWIP_ERR_T in cc.h if you want to use
 *  a different type for your platform (must be signed). */
#ifdef LWIP_ERR_T
typedef LWIP_ERR_T err_t;
#else /* LWIP_ERR_T */
typedef s32_t err_t;
#endif /* LWIP_ERR_T*/

/* Definitions for error constants. */
/* If you modify error definitions for error constants here,
   must correspondently modify the definition of err_strerr[] in the err.c*/

#define ERR_OK          0    /* No error, everything OK.       */
#define ERR_MEM        -1    /* Out of memory error.           */
#define ERR_BUF        -2    /* Buffer error.                  */
#define ERR_RTE        -3    /* Routing problem.               */
#define ERR_INPROGRESS -4    /* Operation in progress          */
#define ERR_VAL        -5    /* Illegal value.                 */
#define ERR_WOULDBLOCK -6    /* Operation would block.         */
#define ERR_USE        -7    /* Address in use.                */
#define ERR_ISCONN     -8    /* Already connected.             */
#define ERR_MSGSIZE    -9    /* Message too long.              */
#define ERR_NODEV      -10   /* No such device.                */
#define ERR_NODEVADDR  -11   /* No such device or address.     */
#define ERR_NOADDR     -12   /* No such address.               */
#define ERR_OPNOTSUPP  -13   /* Operation not supported on transport endpoint. */
#define ERR_ALREADY    -14   /* Operation already in progress. */
#define ERR_ACCES      -15   /* Permission denied.             */
#define ERR_NOPROTOOPT -16   /* Protocol not available.        */
#define ERR_NODEST     -17   /* Destination address required   */
#define ERR_NETDOWN    -18   /* Network is down                */
#define ERR_CONN       -19   /* Not connected.                 */
#define ERR_AFNOSUPP   -20   /* Address family not supported by protocol */
#define ERR_IS_FATAL(e) ((e) < ERR_AFNOSUPP)

#define ERR_ABRT       -21   /* Connection aborted.            */
#define ERR_RST        -22   /* Connection reset.              */
#define ERR_CLSD       -23   /* Connection closed.             */
#define ERR_ARG        -24   /* Illegal argument.              */
#define ERR_REFUSED    -25   /* Connection refused.            */
#define ERR_TIMEDOUT   -26   /* Connection timed out.          */
#define ERR_IF         -27   /* Low-level netif error.         */


#ifdef LWIP_DEBUG
extern const char *lwip_strerr(err_t err);
#else
#define lwip_strerr(x) ""
#endif /* LWIP_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_ERR_H__ */
