/*----------------------------------------------------------------------------
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef __UART_DEV_H__
#define __UART_DEV_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef FALSE
#define FALSE    (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif




#define CONFIG_MAX_BAUDRATE    921600 /* max baud rate */


/* trace */
#define  TRACE_INIT    (1<<0)
#define  TRACE_DATA    (1<<2)
#define  TRACE_MSG     (1<<3)
#define  TRACE_ERR     (1<<4)

#define TRACE_MASK     (0xff)
/* messages print definitions, for debug ,err e.g. */
#ifndef LOSCFG_DEBUG_VERSION
#define LOSCFG_DEBUG_VERSION
#endif
#ifdef LOSCFG_DEBUG_VERSION
    #define uart_trace(mask, msg...) do { \
        if ((mask) & TRACE_MASK) { \
            dprintf("<uart>,%s:%d: ", __func__, __LINE__); \
            dprintf(msg); \
            dprintf("\n"); \
        } \
    } while (0)
#else
    #define uart_trace(mask, msg...)
#endif

#define uart_error(msg...) do { \
    dprintf("<uart,err>:%s,%d: ", __func__, __LINE__); \
    dprintf(msg); \
    dprintf("\n"); \
}while(0)



/*
 * uart core functions
 * */
/* read some data from rx_data buf in uart_ioc_transfer */
int uart_dev_read(struct uart_driver_data *udd, char *buf, size_t count);
/* check the buf is empty */
int uart_rx_buf_empty(struct uart_driver_data *udd);

int uart_recv_notify(struct uart_driver_data *udd, const char *buf, size_t count);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __UART_DEV_H__ */
