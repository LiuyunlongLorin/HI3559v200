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

#ifndef __SPINOR_IDS_H__
#define __SPINOR_IDS_H__

#include "spinor_common.h"
#include "spi_common.h"

/* Don't change the follow config */
#define SPI_NOR_SUPPORT_READ (SPI_IF_READ_STD \
            | SPI_IF_READ_FAST \
            | SPI_IF_READ_DUAL \
            | SPI_IF_READ_DUAL_ADDR \
            | SPI_IF_READ_QUAD \
            | SPI_IF_READ_QUAD_ADDR)

#define SPI_NOR_SUPPORT_WRITE (SPI_IF_WRITE_STD \
            | SPI_IF_WRITE_DUAL \
            | SPI_IF_WRITE_DUAL_ADDR \
            | SPI_IF_WRITE_QUAD \
            | SPI_IF_WRITE_QUAD_ADDR)

#define SPI_NOR_SUPPORT_MAX_DUMMY        (7)

struct spi_nor_info {
    char *name;
    unsigned char id[SPI_NOR_MAX_ID_LEN];
    unsigned int id_len;
    unsigned long chipsize;
    unsigned int erasesize;
    unsigned int addrcycle;
    struct spi_op *read[MAX_SPI_OP];
    struct spi_op *write[MAX_SPI_OP];
    struct spi_op *erase[MAX_SPI_OP];
    struct spi_drv *driver;
};

struct spi_nor_info *spi_nor_serach_id(char *id);

void spi_nor_search_rw(struct spi_nor_info *info,
    struct spi_op *spiop_rw, uint32_t iftype, uint32_t max_dummy, int rw_type);

void spi_nor_get_erase(struct spi_nor_info *info,
        struct spi_op *spiop_erase);

struct spinor_dev_info *spinor_get_dev_info_by_id(struct spinor_info *spinor);

#endif /* End of __SPINOR_IDS_H__ */

