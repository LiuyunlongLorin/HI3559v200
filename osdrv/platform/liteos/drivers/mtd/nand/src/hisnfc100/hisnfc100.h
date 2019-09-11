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

#ifndef __HISNFC100_H__
#define __HISNFC100_H__

#include "mtd_common.h"
#include "nand_common.h"
#include "host_common.h"
#include "spi_common.h"

#define HISNFC100_SPI_NAND_CS_NUM 1

/******************************************************************************/
/* These macroes are for debug only, reg option is slower then dma option */
#undef HISNFC100_SUPPORT_REG_READ
/* #define HISNFC100_SUPPORT_REG_READ */
#undef HISNFC100_SUPPORT_REG_WRITE
/* #define HISNFC100_SUPPORT_REG_WRITE */

/*****************************************************************************/
#define HISNFC100_CFG                               0x00
#define HISNFC100_CFG_BOOT_MODE(_mode)              ((_mode) << 17)
#define HISNFC100_CFG_DEVICE_INTERNAL_ECC_ENABLE    (1 << 16)
#define HISNFC100_CFG_FLASH_SIZE_CS1(_size)         (((_size) & 0xf) << 10)
#define HISNFC100_CFG_FLASH_SIZE_CS0(_size)         (((_size) & 0xf) << 6)
#define HISNFC100_CFG_ADDR_MODE(_mode)              ((_mode) << 5)
#define HISNFC100_CFG_ECC_TYPE(type)                (((type) & 0x3) << 3)
#define HISNFC100_CFG_PAGE_SIZE(size)               ((size) << 2)
#define HISNFC100_CFG_DEVICE_TYPE(_type)            ((_type) << 1)
#define HISNFC100_CFG_OP_MODE(_mode)                ((_mode) << 0)

#define ECC_TYPE_MASK                   (0x3 << 3)
#define PAGE_SIZE_MASK                  (0x1 << 2)

#define DEVICE_TYPE_MASK                (0x1 << 1)
#define DEVICE_TYPE_NAND_FLASH          1

#define OP_MODE_MASK                    (0x1 << 0)
#define OP_MODE_BOOT                    0
#define OP_MODE_NORMAL                  1

/*****************************************************************************/
#define HISNFC100_GLOBAL_CFG                0x04
#define HISNFC100_GLOBAL_CFG_WP_ENABLE      (1 << 5)

/*****************************************************************************/
#define HISNFC100_TIMING_CFG                0x08
#define HISNFC100_TIMING_CFG_TCSH(_n)       (((_n) & 0xf) << 8)
#define HISNFC100_TIMING_CFG_TCSS(_n)       (((_n) & 0xf) << 4)
#define HISNFC100_TIMING_CFG_TSHSL(_n)      ((_n) & 0xf)

#define CS_HOLD_TIME                        0x6
#define CS_SETUP_TIME                       0x6
#define CS_DESELECT_TIME                    0xf

/*****************************************************************************/
#define HISNFC100_INT                       0x0c
#define HISNFC100_INT_OP_DONE               (1 << 0)

/*****************************************************************************/
#define HISNFC100_INT_CLR                   0x14
#define HISNFC100_INT_CLR_DMA_ERR           (1 << 5)
#define HISNFC100_INT_CLR_OP_FAIL           (1 << 2)
#define HISNFC100_INT_CLR_OP_DONE           (1 << 0)
#define HISNFC100_INT_CLR_ALL               0x7f

/*****************************************************************************/
#define HISNFC100_OPCODE                    0x18

/*****************************************************************************/
#define HISNFC100_OP_ADDR                   0x1c

/*****************************************************************************/
#define HISNFC100_OP                        0x20
#define HISNFC100_OP_SEL_CS(_cs)            ((_cs) << 11)
#define HISNFC100_OP_ADDR_NUM(_n)           (((_n) & 0x7) << 8)
#define HISNFC100_OP_DUMMY_CMD_EN(_en)      ((_en) << 7)
#define HISNFC100_OP_DUMMY_ADDR_EN(_en)     ((_en) << 6)
#define HISNFC100_OP_OPCODE_EN(_en)         ((_en) << 5)
#define HISNFC100_OP_ADDR_EN(_en)           ((_en) << 4)
#define HISNFC100_OP_DATE_WRITE_EN(_en)     ((_en) << 3)
#define HISNFC100_OP_DATE_READ_EN(_en)      ((_en) << 2)
#define HISNFC100_OP_STATUS_READ_EN(_en)    ((_en) << 1)
#define HISNFC100_OP_START                  (1 << 0)

/*****************************************************************************/
#define HISNFC100_DATA_NUM                  0x24
#define HISNFC100_DATA_NUM_CNT(_n)          ((_n) & 0x1fff)

/*****************************************************************************/
#define HISNFC100_OP_CFG                        0x28
#define HISNFC100_OP_CFG_DIR_TRANS_ENABLE       (1 << 11)
#define HISNFC100_OP_CFG_RD_OP_SEL(_type)       (((_type) & 0x3) << 9)
#define HISNFC100_OP_CFG_MEM_IF_TYPE(_type)     (((_type) & 0x7) << 6)
#define HISNFC100_OP_CFG_DUMMY_CMD_NUM(_no)     (((_no) & 0x7) << 3)
#define HISNFC100_OP_CFG_DUMMY_ADDR_NUM(_no)    (((_no) & 0x7) << 0)

#define RD_OP_READ_PAGE                 0x0

/*****************************************************************************/
#define HISNFC100_ADDRH                 0x2c
#define HISNFC100_ADDRH_SET(_addr)      ((_addr) & 0xff)

/*****************************************************************************/
#define HISNFC100_ADDRL                 0x30

/*****************************************************************************/
#define HISNFC100_OP_CTRL                       0x34
#define HISNFC100_OP_CTRL_RD_OPCODE(_code)      (((_code) & 0xff) << 16)
#define HISNFC100_OP_CTRL_WR_OPCODE(_code)      (((_code) & 0xff) << 8)
#define HISNFC100_OP_CTRL_CS_OP(_cs)            ((_cs) << 3)
#define HISNFC100_OP_CTRL_OP_TYPE(_type)        ((_type) << 2)
#define HISNFC100_OP_CTRL_RW_OP(_op)            ((_op) << 1)
#define HISNFC100_OP_CTRL_OP_READY              (1 << 0)

#define OP_TYPE_REG                 0
#define OP_TYPE_DMA                 1

#define RW_OP_READ                  0
#define RW_OP_WRITE                 1

/*****************************************************************************/
#define HISNFC100_DMA_CTRL                  0x3c
#define HISNFC100_DMA_CTRL_ALL_ENABLE       0x7

/*****************************************************************************/
#define HISNFC100_DMA_SADDR_D               0x40

/*****************************************************************************/
#define HISNFC100_DMA_SADDR_OOB             0x44

/*****************************************************************************/
#define HISNFC100_DMA_LEN                   0x48
#define HISNFC100_DMA_LEN_SET(_len)         ((_len) & 0xfffffff)

/*****************************************************************************/
#define HISNFC100_STATUS                    0x54

/*****************************************************************************/
#define HISNFC100_VERSION                   0x68

/*****************************************************************************/
struct hisnfc_host {
    struct nand_host *host;
    struct spi spi[1];
    void (*set_system_clock)(int clk, int clk_en);
};

#define HISNFC100_WAIT_TIMEOUT 10000000

#define HISNFC100_CMD_WAIT_CPU_FINISH(_host) do { \
    unsigned regval, timeout = HISNFC100_WAIT_TIMEOUT; \
    do { \
        regval = reg_read((_host), HISNFC100_OP); \
        --timeout; \
    } while ((regval & HISNFC100_OP_START) && timeout); \
    if (!timeout) \
        DBG_MSG("hisnfc100 wait cmd cpu finish timeout!\n"); \
} while (0)

/*****************************************************************************/
#define HISNFC100_DMA_WAIT_INT_FINISH(_host) do { \
    unsigned regval, timeout = HISNFC100_WAIT_TIMEOUT; \
    do { \
        regval = reg_read((_host), HISNFC100_INT); \
        --timeout; \
    } while ((!(regval & HISNFC100_INT_OP_DONE) && timeout)); \
    if (!timeout) \
        DBG_MSG("hisnfc100 wait dma int finish timeout!\n"); \
} while (0)

/*****************************************************************************/
#define HISNFC100_DMA_WAIT_CPU_FINISH(_host) do { \
    unsigned regval, timeout = HISNFC100_WAIT_TIMEOUT; \
    do { \
        regval = reg_read((_host), HISNFC100_OP_CTRL); \
        --timeout; \
    } while ((regval & HISNFC100_OP_CTRL_OP_READY) && timeout); \
    if (!timeout) \
        DBG_MSG("dma wait cpu finish timeout\n"); \
} while (0)

#define SR_DBG  0   /* read status  */
#define FT_DBG  0   /* feature op   */
#define WE_DBG  0   /* write en     */
#define QE_DBG  0   /* quad line en */


#endif /* End of __HISNFC100_H__ */

