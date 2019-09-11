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

#ifndef __HISFC350_H__
#define __HISFC350_H__

#include "los_mux.h"

#include "asm/platform.h"

#include "spi_common.h"
#include "spinor_ids.h"

/*****************************************************************************/
#define HISFC350_SPI_NOR_CS_NUM                     1
#define THREE_BYTE_ADDR_BOOT                        0

/*****************************************************************************/
#define HISFC350_CMD_DATABUF0                       0x0400
#define HISFC350_CMD_DATABUF15                      0x043C
#define HISFC350_REG_BUF_SIZE \
    (HISFC350_CMD_DATABUF15 - HISFC350_CMD_DATABUF0 + 0x04)
#define HISFC350_REG_BUF_MASK                       (HISFC350_REG_BUF_SIZE - 1)

/*****************************************************************************/
#define HISFC350_DMA_ALIGN_SIZE            (256)
#define HISFC350_DMA_ALIGN_MASK                     (HISFC350_DMA_ALIGN_SIZE-1)
#define HISFC350_DMA_MAX_SIZE            (4096)
#define HISFC350_DMA_MAX_MASK                       (HISFC350_DMA_MAX_SIZE-1)

/*****************************************************************************/
#define HISFC350_GLOBAL_CONFIG                      0x0100
#define HISFC350_GLOBAL_CONFIG_READ_DELAY(_n)       (((_n) & 0x03) << 3)
#define HISFC350_GLOBAL_CONFIG_ADDR_MODE_4B         (1 << 2)
#define HISFC350_GLOBAL_CONFIG_ADDR_MODE_DIS_4B     (0 << 2)
#define HISFC350_GLOBAL_CONFIG_WRITE_PROTECT        (1 << 1)
#define HISFC350_GLOBAL_CONFIG_SPI_MODE3            (1 << 0)

/*****************************************************************************/
#define HISFC350_TIMING                             0x0110
#define HISFC350_TIMING_TSHSL(_n)                   ((_n) & 0xF)
#define HISFC350_TIMING_TCSS(_n)                    (((_n) & 0x7) << 8)
#define HISFC350_TIMING_TCSH(_n)                    (((_n) & 0x7) << 12)

/*****************************************************************************/
#define HISFC350_INT_RAW_STATUS                     0x0120
#define HISFC350_INT_RAW_STATUS_DMA_DONE            (1<<1)
#define HISFC350_INT_STATUS                         0x0124
#define HISFC350_INT_MASK                           0x0128
#define HISFC350_INT_CLEAR                          0x012C
#define HISFC350_INT_CLEAR_DMA_DONE                 (1<<1)

/*****************************************************************************/
#define HISFC350_BUS_CONFIG1                        0x0200
#define HISFC350_BUS_CONFIG1_READ_EN                (1<<31)
#define HISFC350_BUS_CONFIG1_WRITE_EN               (1<<30)
#define HISFC350_BUS_CONFIG1_WRITE_INS(_n)          ((_n & 0xFF) << 22)
#define HISFC350_BUS_CONFIG1_WRITE_DUMMY_CNT(_n)    ((_n & 0x7) << 19)
#define HISFC350_BUS_CONFIG1_WRITE_IF_TYPE(_n)      ((_n & 0x7) << 16)
#define HISFC350_BUS_CONFIG1_READ_INS(_n)           ((_n & 0xFF) << 8)
#define HISFC350_BUS_CONFIG1_READ_PREF_CNT(_n)      ((_n & 0x3) << 6)
#define HISFC350_BUS_CONFIG1_READ_DUMMY_CNT(_n)     ((_n & 0x7) << 3)
#define HISFC350_BUS_CONFIG1_READ_IF_TYPE(_n)       (_n & 0x7)

#define HISFC350_SPI_IF_TYPE_STD                    0x0
#define HISFC350_SPI_IF_TYPE_DUAL                   0x1
#define HISFC350_SPI_IF_TYPE_DIO                    0x2
#define HISFC350_SPI_IF_TYPE_QUAD                   0x5
#define HISFC350_SPI_IF_TYPE_QIO                    0x6

#define  hisfc350_spi_is_quad(spi) \
            ((( HISFC350_SPI_IF_TYPE_QUAD == spi->read->iftype) \
            || (HISFC350_SPI_IF_TYPE_QIO == spi->read->iftype) \
            || (HISFC350_SPI_IF_TYPE_QUAD == spi->write->iftype) \
            || (HISFC350_SPI_IF_TYPE_QIO == spi->write->iftype))?1:0)

#define HISFC350_BUS_CONFIG2                        0x0204
#define HISFC350_BUS_CONFIG2_WIP_LOCATE(_n)         (_n & 0x7)
#define HISFC350_BUS_FLASH_SIZE                     0x0210
#define HISFC350_BUS_FLASH_SIZE_CS0_MASK            0x0F
#define HISFC350_BUS_FLASH_SIZE_CS1_MASK            (0x0F << 8)
#define HISFC350_BUS_BASE_ADDR_CS0                  0x0214
#define HISFC350_BUS_BASE_ADDR_CS1                  0x0218
#define HISFC350_BUS_ALIAS_ADDR                     0x021C
#define HISFC350_BUS_ALIAS_CS                       0x0220

#define HISFC350_BUS_DMA_CTRL                       0x0240
#define HISFC350_BUS_DMA_CTRL_START                 (1 << 0)
#define HISFC350_BUS_DMA_CTRL_RW(_rw)               ((_rw)<<1)
#define HISFC350_BUS_DMA_CTRL_CS(_cs)               (((_cs) & 0x01) << 4)
#define HISFC350_BUS_DMA_MEM_SADDR                  0x0244
#define HISFC350_BUS_DMA_FLASH_SADDR                0x0248
#define HISFC350_BUS_DMA_LEN                        0x024C
#define HISFC350_BUS_DMA_LEN_DATA_CNT(_n)           ((_n - 1) & 0x0FFFFFFF)
#define HISFC350_BUS_DMA_AHB_CTRL                   0x0250
#define HISFC350_BUS_DMA_AHB_CTRL_INCR4_EN          (1<<0)
#define HISFC350_BUS_DMA_AHB_CTRL_INCR8_EN          (1<<1)
#define HISFC350_BUS_DMA_AHB_CTRL_INCR16_EN         (1<<2)

/*****************************************************************************/
#define HISFC350_CMD_CONFIG                         0x0300
#define HISFC350_CMD_CONFIG_MEM_IF_TYPE(_n)         (((_n) & 0x07) << 17)
#define HISFC350_CMD_CONFIG_DATA_CNT(_n)            (((_n-1) & 0x3F) << 9)
#define HISFC350_CMD_CONFIG_RW_READ                 (1<<8)
#define HISFC350_CMD_CONFIG_DATA_EN                 (1<<7)
#define HISFC350_CMD_CONFIG_DUMMY_CNT(_n)           (((_n) & 0x07) << 4)
#define HISFC350_CMD_CONFIG_ADDR_EN                 (1<<3)
#define HISFC350_CMD_CONFIG_SEL_CS(_cs)             (((_cs) & 0x01) << 1)
#define HISFC350_CMD_CONFIG_START                   (1<<0)

#define HISFC350_CMD_INS                            0x0308
#define HISFC350_CMD_ADDR                           0x030C
#define HISFC350_CMD_ADDR_MASK                      0x3FFFFFFF

/*****************************************************************************/
#define HISFC350_CMD_WAIT_CPU_FINISH(_host) do {\
    unsigned int timeout = 0x10000000; \
    while (((reg_read((_host), HISFC350_CMD_CONFIG) \
        & HISFC350_CMD_CONFIG_START)) && timeout) \
        --timeout; \
    if (!timeout) \
        ERR_MSG("cmd wait cpu finish timeout\n"); \
} while (0)

#define HISFC350_DMA_WAIT_CPU_FINISH(_host) do {\
    unsigned int timeout = 0xf0000000; \
    while (((reg_read((_host), HISFC350_BUS_DMA_CTRL) \
        & HISFC350_BUS_DMA_CTRL_START)) && timeout) { \
        --timeout; } \
    if (!timeout) \
        ERR_MSG("dma wait cpu finish timeout\n"); \
} while (0)

uint8_t hisfc350_read_reg(struct spi *spi, uint8_t cmd);
int spinor_general_qe_enable(struct spi *spi);

#define AC_DBG  0
#endif /* End of __HISFC350_H__ */

