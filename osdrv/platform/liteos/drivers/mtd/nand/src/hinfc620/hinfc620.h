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

#ifndef __HINFC620_H__
#define __HINFC620_H__

/******************************************************************************/
#include "nand.h"
#include "nand_common.h"
#include "asm/platform.h"

#define HINFC620_NAND_CS_NUM 0

/*****************************************************************************/
#define CONFIG_HINFC620_W_LATCH                     (2)
#define CONFIG_HINFC620_R_LATCH                     (3)
#define CONFIG_HINFC620_RW_LATCH                    (2)
/*****************************************************************************/
#define HINFC620_BUFFER_BASE_ADDRESS_LEN            (2048 + 128)
/*****************************************************************************/
#define HINFC620_CON                        0x00
#define HINFC620_CON_OP_MODE_NORMAL         (1U << 0)
#define HINFC620_CON_PAGEISZE_SHIFT         (1)
#define HINFC620_CON_PAGESIZE_MASK          (0x07)
#define HINFC620_CON_BUS_WIDTH              (1U << 4)
#define HINFC620_CON_READY_BUSY_SEL         (1U << 8)
#define HINFC620_CON_ECCTYPE_SHIFT          (9)
#define HINFC620_CON_ECCTYPE_MASK           (0x0f)
#define HINFC620_CON_RANDOMIZER_EN          (1<<14)
#define HINFC620_CON_SYNC_TYPE_SHIFT        15
#define HINFC620_CON_SYNC_TYPE_MASK         (3<<HINFC620_CON_SYNC_TYPE_SHIFT)
/*****************************************************************************/
#define HINFC620_OPIDLE                     0x08
#define CONFIG_HINFC620_OPIDLE              0x111f11
#define SET_HINFC620_OPIDLE(_wait_ready_wait, _cmd2_wait, _addr_wait, \
        _cmd1_wait, _frb_wait) \
        ((_wait_ready_wait) \
        | (((_cmd2_wait) & 0x0F) << 4)  \
        | (((0xf) & 0x0F) << 8)  \
        | (((_addr_wait) & 0x0F) << 12) \
        | (((_cmd1_wait) & 0x0F) << 16) \
        | (((_frb_wait) & 0x0F) << 20))
/*****************************************************************************/
#define HINFC620_PWIDTH         0x04
#define SET_HINFC620_PWIDTH(_w_lcnt, _r_lcnt, _rw_hcnt) \
    ((_w_lcnt) | (((_r_lcnt) & 0x0F) << 4) | (((_rw_hcnt) & 0x0F) << 8))
/*****************************************************************************/
#define HINFC620_CMD                    0x0C
#define SET_READ_STATUS_CMD(_cmd)       (((_cmd) & 0xFF) << 16)
#define SET_CMD2(_cmd)                  (((_cmd) & 0xFF) << 8)
#define SET_CMD1(_cmd)                  ((_cmd) & 0xFF)
#define SET_ALL_CMD(_cmd2, _cmd1)       (SET_CMD2(_cmd2) | SET_CMD1(_cmd1))
/*****************************************************************************/
#define HINFC620_ADDRL                  0x10
/*****************************************************************************/
#define HINFC620_ADDRH                  0x14
/*****************************************************************************/
#define HINFC620_DATA_NUM               0x18
/*****************************************************************************/
#define HINFC620_OP                     0x1C
#define HINFC620_OP_READ_STATUS_EN      (1U << 0)
#define HINFC620_OP_READ_DATA_EN        (1U << 1)
#define HINFC620_OP_WAIT_READY_EN       (1U << 2)
#define HINFC620_OP_CMD2_EN             (1U << 3)
#define HINFC620_OP_WRITE_DATA_EN       (1U << 4)
#define HINFC620_OP_ADDR_EN             (1U << 5)
#define HINFC620_OP_CMD1_EN             (1U << 6)
#define HINFC620_OP_NF_CS_SHIFT         (7)
#define HINFC620_OP_NF_CS_MASK          (3)
#define HINFC620_OP_ADDR_CYCLE_SHIFT    (9)
#define HINFC620_OP_ADDR_CYCLE_MASK     (7)
#define HINFC620_OP_READID_EN           (1U << 12)
#define HINFC620_OP_RW_REG_EN           (1U << 13)
/*****************************************************************************/
#define HINFC620_STATUS                 0x20
#define HINFC620_OP_READY               (1U << 0)
/*****************************************************************************/
#define HINFC620_INTEN                  0x24
#define HINFC620_INTEN_DMA_ERR_EN       (1U << 10)
#define HINFC620_INTEN_DMA_DONE_EN      (1U << 9)
#define HINFC620_INTEN_WR_LOCK_EN       (1U << 8)
#define HINFC620_INTEN_AHR_OP_EN        (1U << 7)
#define HINFC620_INTEN_ERR_INVALID_EN   (1U << 6)
#define HINFC620_INTEN_ERR_VALID_EN     (1U << 5)
#define HINFC620_INTEN_CS1_DONE_EN      (1U << 2)
#define HINFC620_INTEN_CS0_DONE_EN      (1U << 1)
#define HINFC620_INTEN_OP_DONE_EN       (1U << 0)
#define HINFC620_INTEN_ALL              0x7e7
/*****************************************************************************/
#define HINFC620_INTS                   0x28
#define HINFC620_INTS_UE                (1U << 6)
/*****************************************************************************/
#define HINFC620_INTCLR                 0x2C
#define HINFC620_INTCLR_UE              (1U << 6)
#define HINFC620_INTCLR_CE              (1U << 5)
#define HINFC620_INTCLR_ALL             0x7e7
/*****************************************************************************/
#define HINFC620_DMA_CTRL               0x60
#define SET_DMA_CTRL_CS(_cs)            (((_cs) & 0x3) << 8)
#define DMA_CTRL_ADDR_NUM_MASK          (1U << 7)
#define DMA_CTRL_BURST16_EN             (1U << 6)
#define DMA_CTRL_BURST8_EN              (1U << 5)
#define DMA_CTRL_BURST4_EN              (1U << 4)
#define DMA_CTRL_WR_SEL(_type)          (((_type) & 0x1) << 1)
#define HINFC620_DMA_CTRL_WE            (1U << 1)
#define DMA_CTRL_DMA_START              (1U << 0)

#define DMA_WR_TYPE_READ                0
#define DMA_WR_TYPE_WRITE               1

#define HINFC620_DMA_ADDR_OFFSET        4096
#define HINFC620_DMA_ALIGN              64

#define HINFC620_DMA_ADDR_DATA          0x64
#define HINFC620_DMA_ADDR_OOB           0x68
#define HINFC620_DMA_ADDR_DATA1         0xB4
#define HINFC620_DMA_ADDR_DATA2         0xB8
#define HINFC620_DMA_ADDR_DATA3         0xBC
#define HINFC620_DMA_ADDR_DATA4         0xEC
#define HINFC620_DMA_ADDR_DATA5         0xF0
#define HINFC620_DMA_ADDR_DATA6         0xF4
#define HINFC620_DMA_ADDR_DATA7         0xF8

#define HINFC620_DMA_LEN                0x6C
#define SET_DMA_OOB_LEN(_len)           (((_len) & 0x1FFF) << 16)

#define HINFC620_DMA_PARA                   0x70
#define HINFC620_DMA_PARA_DATA_RW_EN        (1U << 0)
#define HINFC620_DMA_PARA_OOB_RW_EN         (1U << 1)
#define HINFC620_DMA_PARA_DATA_EDC_EN       (1U << 2)
#define HINFC620_DMA_PARA_OOB_EDC_EN        (1U << 3)
#define HINFC620_DMA_PARA_EXT_LEN_SHIFT     (6)
#define HINFC620_DMA_PARA_EXT_LEN_MASK      (0x03)
/*****************************************************************************/
#define HINFC620_VERSION                    0x74
/*****************************************************************************/
#define HINFC620_LOG_READ_ADDR              0x7C
/*****************************************************************************/
#define HINFC620_LOG_READ_LEN               0x80
/*****************************************************************************/
#define HINFC620_FIFO_EMPTY                 0x90
#define HINFC620_FIFO_EMPTY_SHIFT           0xF
#define HINFC620_FIFO_EMPTY_MASK            (0xFF << HINFC620_FIFO_EMPTY_SHIFT)
/*****************************************************************************/
#define HINFC620_BOOT_SET                   0x94
#define HINFC620_BOOT_SET_PAGENUM_MASK      0x3
/*****************************************************************************/
#define HINFC620_ECC_REG0                   0xA0
#define HINFC620_ECC_REG1                   0xA4
#define HINFC620_ECC_REG2                   0xA8
#define HINFC620_ECC_REG3                   0xAC
/*****************************************************************************/
#define HINFC620_RANDOMIZER                 0xC0
#define HINFC620_RANDOMIZER_PAD             0x02
#define HINFC620_RANDOMIZER_ENABLE          0x01
/*****************************************************************************/
#define HINFC620_BOOT_CFG                       0xC4
#define HINFC620_BOOT_CFG_RANDOMIZER_PAD        0x01
#define HINFC620_BOOT_CFG_SAVE_PIN_MODE_SHIFT   13
#define HINFC620_BOOT_CFG_SAVE_PIN_MODE   \
    (1U << HINFC620_BOOT_CFG_SAVE_PIN_MODE_SHIFT)
#define HINFC620_BOOT_CFG_SYC_NAND_PAD_SHIFT    12
#define HINFC620_BOOT_CFG_SYC_NAND_PAD    \
    (1U << HINFC620_BOOT_CFG_SYC_NAND_PAD_SHIFT)

#define HINFC620_READ_1CMD_1ADD_DATA    \
    (HINFC620_OP_CMD1_EN \
    | HINFC620_OP_ADDR_EN \
    | HINFC620_OP_READ_DATA_EN \
    | HINFC620_OP_WAIT_READY_EN \
    | ((nand->cur_cs & HINFC620_OP_NF_CS_MASK) \
        << HINFC620_OP_NF_CS_SHIFT) \
    | (1 << HINFC620_OP_ADDR_CYCLE_SHIFT))

#define WAIT_TIMEOUT_COUNT 0x800000

#define HINFC620_WAIT_OP_FINISH(_host) \
    do { \
        uint32_t timeout = WAIT_TIMEOUT_COUNT; \
        while (!(reg_read((_host), HINFC620_STATUS) \
            & HINFC620_OP_READY) && timeout) \
            timeout--; \
        if (!timeout) \
            DBG_MSG("Wait Nand controller OP finish timeout.\n"); \
    } while (0)

#define HINFC620_WAIT_DMA_FINISH(_host) \
    do { \
        uint32_t timeout = WAIT_TIMEOUT_COUNT; \
        while ((reg_read(host, HINFC620_DMA_CTRL) \
            & DMA_CTRL_DMA_START) && timeout) \
            timeout--; \
        if (!timeout) \
            DBG_MSG("Wait Nand controller DMA finish timeout.\n"); \
    } while (0)

#endif /* End of __HINFC620_H__  */

