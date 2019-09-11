/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 * Copyright (c) <2013-2016>, <Huawei Technologies Co., Ltd>
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
* @defgroup Config_TCP  TCP Configuration APIs
* @ingroup  Configuration_Interfaces
*/

/**
* @defgroup Configuration_Interfaces Configuration Interfaces
* This contains all the configuration APIs
* 
*/

/**
 *   @file tcp_config.h
 */

#ifndef __LWIP_TCP_CONFIG_H__
#define __LWIP_TCP_CONFIG_H__


#include "opt.h"
#include "arch/cc.h"
#include "lwip/tcp_impl.h"

#ifdef LWIP_TCP
#ifdef __cplusplus
extern "C" {
#endif

#ifndef LWIP_DISABLE_TCP_SANITY_CHECKS
#define LWIP_DISABLE_TCP_SANITY_CHECKS  0
#endif

/**
 * TCP_TTL_MIN_VALUE: TTL value must can't be less than 8.
 */
#ifndef TCP_TTL_MIN_VALUE
#define TCP_TTL_MIN_VALUE 8
#endif

/**
 * TCP_MAXRTX_MIN_VALUE: Maximum number of retransmissions can't be less than 3.
 */
#ifndef TCP_MAXRTX_MIN_VALUE
#define TCP_MAXRTX_MIN_VALUE 3
#endif

 /*
Func Name: lwip_config_tcp_ttl
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_ttl(u8_t ttl);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP ttl value.
*
* @par Description
*    This API is used to configure TCP ttl value.
*
* @param[in]   ttl    Time to live. [N/A]
*
* @par Return values
*  0: On Success \n
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
*/
int lwip_config_tcp_ttl(u8_t ttl);
  /*
Func Name: lwip_config_tcp_maxrtx
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_maxrtx(u8_t maxrtx);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP maximum number of retransmissions of data segments.
*
* @par Description
*    This API is used to configure TCP maximum number of retransmissions of data segments.
*
* @param[in]   maxrtx    Maximum number of retransmissions of data segments. [N/A]
*
* @par Return values
*  0: On Success \n
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
*/
int lwip_config_tcp_maxrtx(u8_t maxrtx);
    /*
Func Name: lwip_config_tcp_synmaxrtx
*/
/**
* @ingroup  Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_synmaxrtx(u8_t maxrtx);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP maximum number of retransmissions of SYN segments.
*
* @par Description
*    This API is used to configure TCP maximum number of retransmissions of SYN segments.
*
* @param[in]   maxrtx    Maximum number of retransmissions of SYN segments. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_synmaxrtx(u8_t maxrtx);
    /*
Func Name: lwip_config_tcp_fw1maxrtx
*/
/**
* @ingroup  Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_fw1maxrtx(u8_t maxrtx);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP maximum number of retransmissions of data segments in FIN_WAIT_1 state.
*
* @par Description
*    This API is used to configure TCP maximum number of retransmissions of data segments in FIN_WAIT_1 state.
*
* @param[in]   maxrtx    Maximum number of retransmissions of data segments in FIN_WAIT_1 state. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_fw1maxrtx(u8_t maxrtx);
#if LWIP_WND_SCALE
    /*
Func Name: lwip_config_tcp_rcv_scale
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_rcv_scale(u8_t rcv_scale);
* @endcode
*
* @par Purpose
*    This API is used to configure  the shift count value for Window scale.
*
* @par Description
*    This API is used to configure  the shift count value for Window scale.
*    This API is available only if LWIP_WND_SCALE is enabled.
*
* @param[in]   rcv_scale    Shift count value for Window scale. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_rcv_scale(u8_t rcv_scale);
    /*
Func Name: lwip_config_tcp_wnd_min
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_wnd_min(u16_t wnd);
* @endcode
*
* @par Purpose
*    This API is used to configure the size of a TCP Minimum window.
*
* @par Description
*    This API is used to configure the size of a TCP Minimum window.
*    This API is available only if LWIP_WND_SCALE is enabled.
*
* @param[in]   wnd    Indicates the size of a TCP Minimum window.[N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_wnd_min(u16_t wnd);
#endif
    /*
Func Name: lwip_config_tcp_wnd
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_wnd(u32_t wnd);
* @endcode
*
* @par Purpose
*    This API is used to configure the size of a TCP  window.
*
* @par Description
*    This API is used to configure the size of a TCP  window.
*
* @param[in]   wnd    Size of a TCP  window. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_wnd(u32_t wnd);
    /*
Func Name: lwip_config_tcp_mss
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_mss(u8_t mss);
* @endcode
*
* @par Purpose
*    This API is used to configure the maximum segment size of TCP.
*
* @par Description
*    This API is used to configure the maximum segment size of TCP.
*
* @param[in]   wnd    Maximum segment size of TCP. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_mss(u16_t mss);
    /*
Func Name: lwip_config_tcp_snd_buf
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_tcp_snd_buf(tcpwnd_size_t snd_buf);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP sender buffer space (bytes).
*
* @par Description
*    This API is used to configure TCP sender buffer space (bytes).
*
* @param[in]   snd_buf    TCP sender buffer space. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_tcp_snd_buf(tcpwnd_size_t snd_buf);
   /*
Func Name: lwip_config_initial_cwd
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_initial_cwd(tcpwnd_size_t cwnd);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP initial congestion window (bytes).
*
* @par Description
*    This API is used to configure TCP initial congestion window (bytes).
*
* @param[in]   cwnd    TCP initial congestion window. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_initial_cwd(tcpwnd_size_t cwnd);
   /*
Func Name: lwip_config_initial_ssthresh
*/
/**
* @ingroup Config_TCP
* @par Prototype
* @code
* extern int lwip_config_initial_ssthresh(tcpwnd_size_t ssthresh);
* @endcode
*
* @par Purpose
*    This API is used to configure TCP initial threshold window (bytes).
*
* @par Description
*    This API is used to configure TCP initial threshold window (bytes).
*
* @param[in]   ssthresh    TCP initial Threshold window. [N/A]
*
* @par Return values
*  0: On Success
*  Non Zero value: On Failure
*
*
* @par Required Header File
* tcp_config.h
*
* @par Note
* \n
* This is not external interface, and does not carry any validation for access. Do not use this API.
* @par Related Topics
* \n
* None
*/
int lwip_config_initial_ssthresh(tcpwnd_size_t ssthresh);

#define LWIP_COINFIG_TCP_TTL() lwip_config_tcp_ttl(TCP_TTL)
#define LWIP_CONFIG_TCP_MAXRTX() lwip_config_tcp_maxrtx(TCP_MAXRTX)
#define LWIP_CONFIG_TCP_SYNMAXRTX() lwip_config_tcp_synmaxrtx(TCP_SYNMAXRTX)
#define LWIP_CONFIG_TCP_FW1MAXRTX() lwip_config_tcp_fw1maxrtx(TCP_FW1MAXRTX)
#if LWIP_WND_SCALE
#define LWIP_CONFIG_TCP_RCV_SCALE() lwip_config_tcp_rcv_scale(TCP_RCV_SCALE)
#define LWIP_CONFIG_TCP_WND_MIN() lwip_config_tcp_wnd_min(TCP_WND_MIN)
#endif
#define LWIP_CONFIG_TCP_WND() lwip_config_tcp_wnd(TCP_WND)
#define LWIP_CONFIG_TCP_MSS() lwip_config_tcp_mss(TCP_MSS)
#define LWIP_CONFIG_TCP_SND_BUF() lwip_config_tcp_snd_buf(TCP_SND_BUF)
#define LWIP_CONFIG_INITIAL_CWD() lwip_config_initial_cwd(LWIP_CONGCNTRL_INITIAL_CWND)
#define LWIP_CONFIG_INITIAL_SSTHRESH() lwip_config_initial_ssthresh(LWIP_CONGCNTRL_INITIAL_SSTHRESH)

#ifdef __cplusplus
}
#endif

#endif /* LWIP_TCP */
#endif /* __LWIP_TCP_CONFIG_H__ */
