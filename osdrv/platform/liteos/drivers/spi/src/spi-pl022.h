/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
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

#ifndef __SPI_PL022_H__
#define __SPI_PL022_H__

/* ********** spi reg offset define *************** */
#define REG_SPI_CR0             0x00
#define SPI_CR0_SCR_SHIFT       (8)
#define SPI_CR0_SPH_SHIFT       (7)
#define SPI_CR0_SPO_SHIFT       (6)
#define SPI_CR0_FRF_SHIFT       (4)
#define SPI_CR0_DSS_SHIFT       (0)
#define SPI_CR0_SCR             (0xff << 8) /* clkout=clk/(cpsdvsr*(scr+1)) */
#define SPI_CR0_SPH             (0x1 << 7)  /* spi phase */
#define SPI_CR0_SPO             (0x1 << 6)  /* spi clk polarity */
#define SPI_CR0_FRF             (0x3 << 4)  /* frame format set */
#define SPI_CR0_DSS             (0xf << 0)  /* data bits width */

#define REG_SPI_CR1             0x04
#define SPI_CR1_WAIT_EN_SHIFT   (15)
#define SPI_CR1_WAIT_VAL_SHIFT  (8)
#define SPI_CR1_ALT_SHIFT       (6)
#define SPI_CR1_BIG_END_SHIFT   (4)
#define SPI_CR1_MS_SHIFT        (2)
#define SPI_CR1_SSE_SHIFT       (1)
#define SPI_CR1_LBN_SHIFT       (0)
#define SPI_CR1_WAIT_EN         (0x1 << 15)
#define SPI_CR1_WAIT_VAL        (0x7f << 8)

/* alt mode:spi enable csn is select; spi disable csn is cancel */
#define SPI_CR1_ALT             (0x1 << 6)
#define SPI_CR1_BIG_END         (0x1 << 4) /* big end or little */
#define SPI_CR1_MS              (0x1 << 2) /* master-slave mode */
#define SPI_CR1_SSE             (0x1 << 1) /* spi enable set */
#define SPI_CR1_LBN             (0x1 << 0) /* loopback mode */

#define REG_SPI_DR              0x08

#define REG_SPI_SR              0x0c
#define SPI_SR_BSY_SHIFT        (4)
#define SPI_SR_RFF_SHIFT        (3)
#define SPI_SR_RNE_SHIFT        (2)
#define SPI_SR_TNF_SHIFT        (1)
#define SPI_SR_TFE_SHIFT        (0)
#define SPI_SR_BSY              (0x1 << 4)/* spi busy flag */
#define SPI_SR_RFF              (0x1 << 3)/* Whether to send fifo is full */
#define SPI_SR_RNE              (0x1 << 2)/* Whether to send fifo is no empty */
#define SPI_SR_TNF              (0x1 << 1)/* Whether to send fifo is no full */
#define SPI_SR_TFE              (0x1 << 0)/* Whether to send fifo is empty */

#define REG_SPI_CPSR            0x10
#define SPI_CPSR_CPSDVSR_SHIFT  (0)
#define SPI_CPSR_CPSDVSR        (0xff << 0)  /* even 2~254 */

#define REG_SPI_IMSC            0x14
#define REG_SPI_RIS             0x18
#define REG_SPI_MIS             0x1c
#define REG_SPI_ICR             0x20
#define REG_SPI_DMA_CR          0x24
#define REG_SPI_TXFIFO_CR       0x28
#define REG_SPI_RXFIFO_CR       0x2C

#define DMA_ENABLE              0x3
#define DMA_DISABLE             (~0x3)
#define DEF_RX_FIFO_CFG         0x0
#define DEF_TX_FIFO_CFG         0x0

#define MAX_WAIT                1000

#endif /* __SPI_PL022_H_ */

