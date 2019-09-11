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

#ifndef __HIFMC_COMMON_H__
#define __HIFMC_COMMON_H__

/* HIFMC ECCTYPE REG CONFIG */
enum hifmc_ecc_reg {
    hifmc_ecc_0bit   = 0x00,
    hifmc_ecc_8bit   = 0x01,
    hifmc_ecc_16bit  = 0x02,
    hifmc_ecc_24bit  = 0x03,
    hifmc_ecc_28bit  = 0x04,
    hifmc_ecc_40bit  = 0x05,
    hifmc_ecc_64bit  = 0x06,
};

/* HIFMC PAGESIZE REG CONFIG */
enum hifmc_page_reg {
    hifmc_pagesize_2K    = 0x0,
    hifmc_pagesize_4K    = 0x1,
    hifmc_pagesize_8K    = 0x2,
    hifmc_pagesize_16K   = 0x3,
};

enum hifmc_page_reg hifmc_page_type2reg(int type);
int hifmc_page_reg2type(enum hifmc_page_reg reg);
enum hifmc_ecc_reg hifmc_ecc_type2reg(int type);
int hifmc_ecc_reg2type(enum hifmc_ecc_reg reg);

/* HIFMC REG */
/*****************************************************************************/
#define FMC_CFG                             0x00
#define FMC_CFG_SPI_NAND_SEL(_type)         (((_size) & 0x3) << 11)
#define FMC_CFG_SPI_NOR_ADDR_MODE(_mode)    ((_mode) << 10)
#define FMC_CFG_BLOCK_SIZE(_size)           (((_size) & 0x3) << 8)
#define FMC_CFG_ECC_TYPE(_type)             (((_type) & 0x7) << 5)
#define FMC_CFG_PAGE_SIZE(_size)            (((_size) & 0x3) << 3)
#define FMC_CFG_FLASH_SEL(_type)            (((_type) & 0x3) << 1)
#define FMC_CFG_OP_MODE(_mode)              ((_mode) & 0x1)

#define SPI_NOR_ADDR_MODE_3_BYTES           0x0
#define SPI_NOR_ADDR_MODE_4_BYTES           0x1

#define FMC_SPI_NOR_ADDR_MODE_SHIFT             10
#define FMC_SPI_NOR_ADDR_MODE_MASK              (0x1 << FMC_SPI_NOR_ADDR_MODE_SHIFT)

#define ECC_TYPE_SHIFT                      5
#define ECC_TYPE_MASK                       (0x7 << ECC_TYPE_SHIFT)

#define PAGE_SIZE_SHIFT                     3
#define PAGE_SIZE_MASK                      (0x3 << PAGE_SIZE_SHIFT)

#define FLASH_TYPE_SPI_NOR                  0x0
#define FLASH_TYPE_SPI_NAND                 0x1
#define FLASH_TYPE_NAND                     0x2
#define FLASH_TYPE_DEFAULT                  0x3
#define FLASH_SEL_SHIFT                     1
#define FLASH_SEL_MASK                      (0x3 << FLASH_SEL_SHIFT)

#define OP_MODE_BOOT                        0x0
#define OP_MODE_NORMAL                      0x1
#define OP_MODE_MASK                        0x1

/*****************************************************************************/
#define FMC_GLOBAL_CFG                      0x04
#define FMC_GLOBAL_CFG_WP_ENABLE            (1 << 6)
#define FMC_GLOBAL_CFG_RANDOMIZER_EN        (1 << 2)
#define FMC_GLOBAL_CFG_EDO_EN               (1 << 9)

/*****************************************************************************/
#define FMC_SPI_TIMING_CFG                  0x08
#define TIMING_CFG_TCSH(_n)                 (((_n) & 0xf) << 8)
#define TIMING_CFG_TCSS(_n)                 (((_n) & 0xf) << 4)
#define TIMING_CFG_TSHSL(_n)                ((_n) & 0xf)

#define CS_HOLD_TIME                        0x6
#define CS_SETUP_TIME                       0x6
#define CS_DESELECT_TIME                    0xf

/*****************************************************************************/
#define FMC_PND_PWIDTH_CFG                  0x0c
#define PWIDTH_CFG_RW_HCNT(_n)              (((_n) & 0xf) << 8)
#define PWIDTH_CFG_R_LCNT(_n)               (((_n) & 0xf) << 4)
#define PWIDTH_CFG_W_LCNT(_n)               ((_n) & 0xf)
#define RW_H_WIDTH                          (0x2)
#define R_L_WIDTH                           (0x3)
#define W_L_WIDTH                           (0x2)

/*****************************************************************************/
#define FMC_INT                             0x18
#define FMC_INT_AHB_OP                      (1 << 7)
#define FMC_INT_WR_LOCK                     (1 << 6)
#define FMC_INT_DMA_ERR                     (1 << 5)
#define FMC_INT_ERR_ALARM                   (1 << 4)
#define FMC_INT_ERR_INVALID                 (1 << 3)
#define FMC_INT_ERR_VALID                   (1 << 2)
#define FMC_INT_OP_FAIL                     (1 << 1)
#define FMC_INT_OP_DONE                     (1 << 0)

/*****************************************************************************/
#define FMC_INT_EN                          0x1c
#define FMC_INT_EN_AHB_OP                   (1 << 7)
#define FMC_INT_EN_WR_LOCK                  (1 << 6)
#define FMC_INT_EN_DMA_ERR                  (1 << 5)
#define FMC_INT_EN_ERR_ALARM                (1 << 4)
#define FMC_INT_EN_ERR_INVALID              (1 << 3)
#define FMC_INT_EN_ERR_VALID                (1 << 2)
#define FMC_INT_EN_OP_FAIL                  (1 << 1)
#define FMC_INT_EN_OP_DONE                  (1 << 0)

/*****************************************************************************/
#define FMC_INT_CLR                         0x20
#define FMC_INT_CLR_AHB_OP                  (1 << 7)
#define FMC_INT_CLR_WR_LOCK                 (1 << 6)
#define FMC_INT_CLR_DMA_ERR                 (1 << 5)
#define FMC_INT_CLR_ERR_ALARM               (1 << 4)
#define FMC_INT_CLR_ERR_INVALID             (1 << 3)
#define FMC_INT_CLR_ERR_VALID               (1 << 2)
#define FMC_INT_CLR_OP_FAIL                 (1 << 1)
#define FMC_INT_CLR_OP_DONE                 (1 << 0)
#define FMC_INT_CLR_ALL                     0xff

/*****************************************************************************/
#define FMC_CMD                             0x24
#define FMC_CMD_CMD2(_cmd)                  (((_cmd) & 0xff) << 8)
#define FMC_CMD_CMD1(_cmd)                  ((_cmd) & 0xff)

/*****************************************************************************/
#define FMC_ADDRH                           0x28
#define FMC_ADDRL                           0x2c

/*****************************************************************************/
#define FMC_OP_CFG                          0x30
#define OP_CFG_FM_CS(_cs)                   ((_cs) << 11)
#define OP_CFG_FORCE_CS_EN(_en)             ((_en) << 10)
#define OP_CFG_MEM_IF_TYPE(_type)           (((_type) & 0x7) << 7)
#define OP_CFG_ADDR_NUM(_addr)              (((_addr) & 0x7) << 4)
#define OP_CFG_DUMMY_NUM(_dummy)            ((_dummy) & 0xf)

/*****************************************************************************/
#define FMC_SPI_OP_ADDR                     0x34

/*****************************************************************************/
#define FMC_DATA_NUM                        0x38
#define FMC_DATA_NUM_CNT(_n)                ((_n) & 0x3fff)

/*****************************************************************************/
#define FMC_OP                              0x3c
#define FMC_OP_DUMMY_EN(_en)                ((_en) << 8)
#define FMC_OP_CMD1_EN(_en)                 ((_en) << 7)
#define FMC_OP_ADDR_EN(_en)                 ((_en) << 6)
#define FMC_OP_WRITE_DATA_EN(_en)           ((_en) << 5)
#define FMC_OP_CMD2_EN(_en)                 ((_en) << 4)
#define FMC_OP_WAIT_READY_EN(_en)           ((_en) << 3)
#define FMC_OP_READ_DATA_EN(_en)            ((_en) << 2)
#define FMC_OP_READ_STATUS_EN(_en)          ((_en) << 1)
#define FMC_OP_REG_OP_START                 1

/*****************************************************************************/
#define FMC_DMA_LEN                         0x40
#define FMC_DMA_LEN_SET(_len)               ((_len) & 0x0fffffff)

/*****************************************************************************/
#define FMC_DMA_AHB_CTRL                    0x48
#define FMC_DMA_AHB_CTRL_DMA_PP_EN          (1 << 3)
#define FMC_DMA_AHB_CTRL_BURST16_EN         (1 << 2)
#define FMC_DMA_AHB_CTRL_BURST8_EN          (1 << 1)
#define FMC_DMA_AHB_CTRL_BURST4_EN          1
#define ALL_BURST_ENABLE                    (FMC_DMA_AHB_CTRL_BURST16_EN \
                                            | FMC_DMA_AHB_CTRL_BURST8_EN \
                                            | FMC_DMA_AHB_CTRL_BURST4_EN)

/*****************************************************************************/
#define FMC_DMA_SADDR_D0                    0x4c
#define FMC_DMA_SADDRH_D0                   0x200
#define FMC_DMA_SADDR_D1                    0x50
#define FMC_DMA_SADDRH_D1                   0x204
#define FMC_DMA_SADDR_D2                    0x54
#define FMC_DMA_SADDRH_D2                   0x208
#define FMC_DMA_SADDR_D3                    0x58
#define FMC_DMA_SADDRH_D3                   0x20c
#define FMC_DMA_SADDR_OOB                   0x5c
#define FMC_DMA_SADDRH_OOB                  0x210
#define FMC_DMA_ADDR_OFFSET                 4096

/*****************************************************************************/
#define FMC_DMA_BLK_SADDR                   0x60
#define FMC_DMA_BLK_SADDR_SET(_addr)        ((_addr) & 0xffffff)

/*****************************************************************************/
#define FMC_DMA_BLK_LEN                     0x64
#define FMC_DMA_BLK_LEN_SET(_len)           ((_len) & 0xffff)

/*****************************************************************************/
#define FMC_OP_CTRL                         0x68
#define OP_CTRL_RD_OPCODE(_code)            (((_code) & 0xff) << 16)
#define OP_CTRL_WR_OPCODE(_code)            (((_code) & 0xff) << 8)
#define OP_CTRL_RD_OP_SEL(_op)              (((_op) & 0x3) << 4)
#define OP_CTRL_DMA_OP(_type)               ((_type) << 2)
#define OP_CTRL_RW_OP(_op)                  ((_op) << 1)
#define OP_CTRL_DMA_OP_READY                1

#define RD_OP_READ_ALL_PAGE                 0x0
#define RD_OP_READ_OOB                      0x1
#define RD_OP_BLOCK_READ                    0x2

#define RD_OP_SHIFT                         4
#define RD_OP_MASK                          (0x3 << RD_OP_SHIFT)

#define OP_TYPE_DMA                         0x0
#define OP_TYPE_REG                         0x1

#define RW_OP_READ                          0x0
#define RW_OP_WRITE                         0x1

/*****************************************************************************/
#define FMC_OP_PARA                         0x70
#define FMC_OP_PARA_RD_OOB_ONLY             (1 << 1)

/*****************************************************************************/
#define FMC_BOOT_SET                        0x74
#define FMC_BOOT_SET_DEVICE_ECC_EN          (1 << 3)
#define FMC_BOOT_SET_BOOT_QUAD_EN           (1 << 1)

/*****************************************************************************/
#define FMC_STATUS                          0xac

#define GET_OP                              0
#define SET_OP                              1

/*****************************************************************************/
#define FMC_VERSION                         0xbc

/*****************************************************************************/
#define FMC_CPU_WAIT_TIMEOUT        0x800000
#define FMC_DMA_WAIT_TIMEOUT        0xf0000000

#define FMC_CMD_WAIT_CPU_FINISH(_host) \
    do { \
        unsigned regval, timeout = FMC_CPU_WAIT_TIMEOUT; \
        do { \
            regval = reg_read((_host), FMC_OP); \
            --timeout; \
        } while ((regval & FMC_OP_REG_OP_START) && timeout); \
        if (!timeout) \
            ERR_MSG(" Wait cmd cpu finish timeout!\n"); \
    } while (0)

/*****************************************************************************/
#define FMC_DMA_WAIT_INT_FINISH(_host) \
    do { \
        unsigned regval, timeout = FMC_DMA_WAIT_TIMEOUT; \
        do { \
            regval = reg_read((_host), FMC_INT); \
            --timeout; \
        } while ((!(regval & FMC_INT_OP_DONE) && timeout)); \
        if (!timeout) { \
            ERR_MSG(" Wait dma int finish timeout!\n"); \
        } \
    } while (0)

/*****************************************************************************/
#define FMC_DMA_WAIT_CPU_FINISH(_host) \
    do { \
        unsigned regval, timeout = FMC_CPU_WAIT_TIMEOUT; \
        do { \
            regval = reg_read((_host), FMC_OP_CTRL); \
            --timeout; \
        } while ((regval & OP_CTRL_DMA_OP_READY) && timeout); \
        if (!timeout) { \
            ERR_MSG(" Wait dma cpu finish timeout!\n"); \
        } \
    } while (0)

/*****************************************************************************/
#endif /* End of __HIFMC_COMMON_H__ */

