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

#ifndef __SPI_NAND_IDS_H__
#define __SPI_NAND_IDS_H__

#include "spi_common.h"

/*****************************************************************************/
#define SPI_NAND_SUPPORT_READ    (SPI_IF_READ_STD \
                    | SPI_IF_READ_FAST \
                    | SPI_IF_READ_DUAL \
                    | SPI_IF_READ_DUAL_ADDR \
                    | SPI_IF_READ_QUAD \
                    | SPI_IF_READ_QUAD_ADDR)

#define SPI_NAND_SUPPORT_WRITE    (SPI_IF_WRITE_STD | SPI_IF_WRITE_QUAD)

#define SPI_NAND_SUPPORT_MAX_DUMMY    8

/*****************************************************************************/
#define READ_ID_ADDR            0x00
#define PROTECT_ADDR            0xA0
#define FEATURE_ADDR            0xB0
#define STATUS_ADDR             0xC0

#define SPINAND_ERASE_FAIL      0x04
#define SPINAND_PROG_FAIL       0x08

#define FEATURES_DATA_LEN       1

/*****************************************************************************/
#define PROTECT_BP3_MASK        (1 << 6)
#define PROTECT_BP2_MASK        (1 << 5)
#define PROTECT_BP1_MASK        (1 << 4)
#define PROTECT_BP0_MASK        (1 << 3)
#define ANY_BP_ENABLE(_val)     ((PROTECT_BP3_MASK & _val) \
                                || (PROTECT_BP2_MASK & _val) \
                                || (PROTECT_BP1_MASK & _val) \
                                || (PROTECT_BP0_MASK & _val))
#define ALL_BP_MASK             (PROTECT_BP3_MASK \
                                | PROTECT_BP2_MASK \
                                | PROTECT_BP1_MASK \
                                | PROTECT_BP0_MASK)

/*****************************************************************************/
#define MAX_SPI_NAND_ID_LEN     3

struct spi_nand_info {
    char *name;
    unsigned char id[MAX_SPI_NAND_ID_LEN];
    unsigned char id_len;
    unsigned long long chipsize;
    unsigned int blocksize;
    unsigned int pagesize;
    unsigned int oobsize;
    unsigned int badblock_pos;
    struct spi_op *read[MAX_SPI_OP];
    struct spi_op *write[MAX_SPI_OP];
    struct spi_op *erase[MAX_SPI_OP];
    struct spi_drv *driver;
};

extern void spi_nand_get_erase(struct spi_nand_info *spiinfo,
        struct spi_op *spiop_erase);
extern void spi_nand_search_rw(struct spi_nand_info *spiinfo,
    struct spi_op *spiop_rw, u_int iftype, u_int max_dummy, int rw_type);

#endif /* End of __SPI_NAND_IDS_H__ */

