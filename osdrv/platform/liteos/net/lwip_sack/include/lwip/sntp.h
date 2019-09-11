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

#ifndef __SNTP_H__
#define __SNTP_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "lwip/opt.h"
#include "arch/sys_arch.h"
#include "lwip/err.h"


/**
* @defgroup SNTP_Interfaces SNTP Interfaces
* @ingroup User_interfaces
* This section contains the SNTP related interfaces.
*/

/*
Func Name:  lwip_sntp_start
*/
/**
* @ingroup SNTP_Interfaces
* @par Prototype
* @code
* int lwip_sntp_start(int server_num, char **sntp_server, struct timeval *time);
* @endcode
*
* @par Purpose
* This API is used to start the SNTP module.
*
* @par Description
* This API is used to start the SNTP query.
*
* @param[in]    server_num   Total servers in param sntp_server [N/A]
* @param[in]    sntp_server  Domain name or IP_add of SNTP server in string format [N/A]
* @param[out]   time         the time of server if lwip_sntp_start return ERR_OK. [N/A]
*
* @par Return values
* ERR_OK: On success\n
* ERR_MEM: On failure, due to memory error\n
* ERR_ARG: On failure, due to incorrect arg\n
* ERR_WOULDBLOCK: On failure, due to sntp server is not response\n
*
* @par Required Header File
* sntp.h
*
* @par Note
* 1. If Input parameter sntp_server passed as NULL or server_num assed as 0 to this API,
* in that case this API will fail with return value ERR_ARG.
* 2. If the string length of sntp_server passed to this API is greater than SNTP_SERVER_ADDR_SIZE
* then this API will fail with return value ERR_ARG.\n
* 3. This API will block for a few seconds waiting for sntp server replay.\n
* 4. If this API return ERR_OK, and param time is not NULL,
* Then time will be the time obtain form sntp server.\n
*
* @par Related Topics
* \n
*
*/

int lwip_sntp_start(int server_num, char **sntp_server, struct timeval *time);

#ifdef __cplusplus
}
#endif

#endif /* __SNTP_H__ */
