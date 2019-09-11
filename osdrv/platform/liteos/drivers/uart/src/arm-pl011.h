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
#ifndef __UART_HISILICON_H__
#define __UART_HISILICON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define DEFAULT_UART0_BAUDRATE        115200
#define DEFAULT_UART1_BAUDRATE        9600
#define DEFAULT_UART2_BAUDRATE        115200
#define DEFAULT_UART3_BAUDRATE        9600
#define DEFAULT_UART4_BAUDRATE        9600
#define CONFIG_MAX_BAUDRATE           921600

#define RX_DMA_BUF_SIZE               0x1000          /*4K*/

/* register define */
#define UART_IFLS_RX1_8               (0x00 << 3)
#define UART_IFLS_RX4_8               (0x02 << 3)
#define UART_IFLS_RX7_8               (0x04 << 3)
#define UART_IFLS_TX1_8               (0x00 << 0)
#define UART_IFLS_TX4_8               (0x02 << 0)
#define UART_IFLS_TX7_8               (0x04 << 0)

#define UART_CR_CTS                   (0x01 <<15)
#define UART_CR_RTS                   (0x01 <<14)
#define UART_CR_RX_EN                 (0x01 << 9)
#define UART_CR_TX_EN                 (0x01 << 8)
#define UART_CR_LOOPBACK              (0x01 << 7)
#define UART_CR_EN                    (0x01 << 0)

#define UART_FR_TXFE                  (0x01 << 7)
#define UART_FR_RXFF                  (0x01 << 6)
#define UART_FR_TXFF                  (0x01 << 5)
#define UART_FR_RXFE                  (0x01 << 4)
#define UART_FR_BUSY                  (0x01 << 3)

#define UART_LCR_H_BREAK               (0x01 << 0)
#define UART_LCR_H_PEN                 (0x01 << 1)
#define UART_LCR_H_EPS                 (0x01 << 2)
#define UART_LCR_H_STP2                (0x01 << 3)
#define UART_LCR_H_FIFO_EN             (0x01 << 4)
#define UART_LCR_H_8_BIT               (0x03 << 5)
#define UART_LCR_H_7_BIT               (0x02 << 5)
#define UART_LCR_H_6_BIT               (0x01 << 5)
#define UART_LCR_H_5_BIT               (0x00 << 5)
#define UART_LCR_H_SPS                 (0x01 << 7)

#define UART_RXDMAE                   (0x01 << 0)
#define UART_TXDMAE                   (0x01 << 1)

#define UART_MIS_TIMEOUT              (0x01 << 6)
#define UART_MIS_TX                   (0x01 << 5)
#define UART_MIS_RX                   (0x01 << 4)

#define UART_IMSC_OVER                (0x01 << 10)
#define UART_IMSC_BREAK               (0x01 << 9)
#define UART_IMSC_CHK                 (0x01 << 8)
#define UART_IMSC_ERR                 (0x01 << 7)
#define UART_IMSC_TIMEOUT             (0x01 << 6)
#define UART_IMSC_TX                  (0x01 << 5)
#define UART_IMSC_RX                  (0x01 << 4)

#define UART_DMACR_RX                 (0x01 << 0)
#define UART_DMACR_TX                 (0x01 << 1)
#define UART_DMACR_ONERR              (0x01 << 2)
#define UART_INFO                   (0x01<<1)


struct uart_dma_transfer {
    /*dma alloced channel*/
    unsigned int channel;
    /*dma created task id*/
    unsigned int thread_id;
    /*dma receive buf  head*/
    unsigned int head;
    /*dma receive buf  tail*/
    unsigned int tail;

    /*dma receive buf  cycled flag*/
    unsigned int flags;
#define BUF_CIRCLED    (1 << 0)

    /*dma receive buf, shoud be cache aligned*/
    char *buf;
};

struct arm_pl011_port {
    int enable;
    unsigned long phys_base;
    unsigned int irq_num;
    unsigned int default_baudrate;
    unsigned int flags;
#define PL011_FLG_IRQ_REQUESTED    (1 << 0)
#define PL011_FLG_DMA_RX_REQUESTED    (1 << 1)
#define PL011_FLG_DMA_TX_REQUESTED    (1 << 2)

    struct uart_dma_transfer *rx_udt;
    struct uart_driver_data *udd;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __UART_HISILICON_H__ */
