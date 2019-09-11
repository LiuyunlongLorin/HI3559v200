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

#ifndef __NAND_COMMON_H__
#define __NAND_COMMON_H__

#include "los_typedef.h"

#define BBP_LAST_PAGE           0x01
#define BBP_FIRST_PAGE          0x02

#define GOOD_BLOCK              0
#define BAD_BLOCK               1

/* NAND features macros */
#define NAND_RANDOMIZER             0x01 /* nand chip need randomizer */
#define NAND_SYNCHRONOUS            0x02 /* nand chip support syncrono */
#define NAND_ASYNCHRONOUS           0x04 /* nand chip support asynchro */
#define NAND_SYNCHRONOUS_BOOT       0x08 /* nand boot from synchronous */
#define NAND_HW_AUTO                0x10 /* controller support hardware */
#define NAND_CONFIG_DONE            0x20 /* current controller config */

#define IS_RANDOMIZER(_dev)         ((_dev)->flags & NAND_RANDOMIZER)
#define IS_SYNCHRONOUS(_dev)        ((_dev)->flags & NAND_SYNCHRONOUS)
#define IS_ASYNCHRONOUS(_dev)       ((_dev)->flags & NAND_ASYNCHRONOUS)
#define IS_SYNCHRONOUS_BOOT(_dev)   ((_dev)->flags & NAND_SYNCHRONOUS_BOOT)
#define IS_HW_AUTO(_dev)            ((_dev)->flags & NAND_HW_AUTO)
#define IS_CONFIG_DONE(_dev)        ((_dev)->flags & NAND_CONFIG_DONE)

/* Max configuration */
#define NAND_BB_SIZE            2
#define NAND_MAX_ID_LEN         8

/* Status bits */
#define NAND_STATUS_FAIL        0x01
#define NAND_STATUS_FAIL_N1     0x02
#define NAND_STATUS_TRUE_READY  0x20
#define NAND_STATUS_READY       0x40
#define NAND_STATUS_WP          0x80

/* Standard NAND flash commands */
#define NAND_CMD_READ0          0x00
#define NAND_CMD_READ1          0x30
#define NAND_CMD_PROG0          0x80
#define NAND_CMD_PROG1          0x10
#define NAND_CMD_ERASE1         0x60
#define NAND_CMD_ERASE2         0xd0
#define NAND_CMD_STATUS         0x70
#define NAND_CMD_READID         0x90
#define NAND_CMD_RESET          0xff

/* Addr Cyecles configuration */
#define NAND_PROG_READ_ADDR_CYCLE   (5)
#define NAND_ERASE_ADDR_CYCLE       (3)

#define READ_ID_ADDR_NUM            1

/* NAND page size macros and emum */
#define NAND_PAGE_512B                   0
#define NAND_PAGE_2K                     1
#define NAND_PAGE_4K                     2
#define NAND_PAGE_8K                     3
#define NAND_PAGE_16K                    4

/*---------------------------------------------------------------------------*/
/* struct nand_config_info - Nand config information structure */
/*---------------------------------------------------------------------------*/
struct nand_config_info {
    unsigned int pagetype;
    unsigned int ecctype;
    unsigned int oobsize;
    unsigned int flag;
};

/*---------------------------------------------------------------------------*/
/* struct nand_status_info - Nand status information structure */
/*---------------------------------------------------------------------------*/
struct nand_status_info {
    char *start_type;/* Nand start type when boot init */
    uint16_t ecctype;/* Currently level of ECC */
    uint32_t flags;  /* Store various bit options for capability */
};

/*---------------------------------------------------------------------------*/
/* struct nand_dev_info - Nand device information structure */
/*---------------------------------------------------------------------------*/
struct nand_dev_info {
    char *name;/* Human-readable label */
    union {
        char id[NAND_MAX_ID_LEN];/* The full ID array */
        struct {
            uint8_t mfr_id;/* id[0]: Manufacturer ID */
            uint8_t dev_id;/* id[1]: Device ID */
        };
    };
    uint16_t id_len;/* The valid length of ID */
    uint16_t oobsize;/* Spare area(OOB) size */

    uint32_t pagesize;/* Size of a page area */
    uint32_t page_shift;
    uint32_t pagemask;

    uint32_t blocksize;/* Size of an erase block */
    uint32_t block_shift;
    uint32_t blockmask;

    uint64_t chipsize;/* Total size of the device */
    uint32_t chip_shift;

    struct nand_status_info status;

    void *priv;
};

/*---------------------------------------------------------------------------*/
/* struct nand_info - Nand various interface and information structure */
/*---------------------------------------------------------------------------*/
#include "linux/mtd/mtd.h"
#include "mtd_common.h"
struct nand_info {
    struct nand_dev_info dev;
    int numchips;

    void *priv;

    uint8_t (*erase)(struct nand_info *nand, uint32_t page);
    int (*program)(struct nand_info *nand, uint32_t page);
    int (*read)(struct nand_info *nand, uint32_t page);

    void (*read_id)(struct nand_info *nand, char *id);
    uint8_t (*read_status)(struct nand_info *nand);
    void (*reset)(struct nand_info *nand);
    uint8_t (*feature_op)(struct nand_info *nand, uint8_t op,
            uint8_t addr, uint8_t val);
    void (*ids_probe)(struct nand_info *nand);

    uint8_t (*read_byte)(struct nand_info *nand, int offset);
    uint16_t (*read_word)(struct nand_info *nand, int offset);
    void (*read_buf)(struct nand_info *nand, const char *buf,
            int len, int offset);
    void (*write_buf)(struct nand_info *nand, const char *buf,
            int len, int offset);

    struct nand_dev_info *(*get_dev_info_by_id)(struct nand_info *nand);
    int (*oob_resize)(struct nand_info *nand);

    int (*block_isbad) (struct nand_info *nand, loff_t ofs);
    int (*block_markbad) (struct nand_info *nand, loff_t ofs);

    uint8_t cur_cs;
    int (*get_device)(struct nand_info *nand);
    void (*put_device)(struct nand_info *nand);

#define NAND_READING    0x01
#define NAND_PROGING    0x02
#define NAND_ERASING    0x04
    int op_state;

    UINT32 lock;
    int (*resume)(struct nand_info *nand);
};

const char *nand_page_name(int type);
int nandpage_size2type(int size);
int nandpage_type2size(int size);

#endif /* End of __NAND_COMMON_H__ */

