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

#ifndef __SPINOR_COMMON_H__
#define __SPINOR_COMMON_H__

#include "spinor_common.h"

/*****************************************************************************/
#define SPI_NOR_SR_WIP_MASK             (1 << 0)
#define SPI_NOR_SR_LEN                  1    /* Status Register length(byte) */

#define SPI_NOR_CR_LEN                  1    /* Config Register length(byte) */
#define SPI_NOR_CR_SHIFT                8    /* Config Register shift(bit) */
#define SPI_NOR_CR_4BYTE_SHIFT          5
#define SPI_NOR_CR_4BYTE_MASK           (1 << SPI_NOR_CR_4BYTE_SHIFT)
#define SPI_NOR_GET_4BYTE_BY_CR(cr)     (((cr) & SPI_NOR_CR_4BYTE_MASK) \
                                            >> SPI_NOR_CR_4BYTE_SHIFT)
#define SPI_NOR_CR_QE_SHIFT             1
#define SPI_NOR_CR_QE_MASK              (1 << SPI_NOR_CR_QE_SHIFT)
#define SPI_NOR_GET_QE_BY_CR(cr)        (((cr) & SPI_NOR_CR_QE_MASK) \
                                        >> SPI_NOR_CR_QE_SHIFT)

#define SPI_4BYTE_ADDR_LEN              (4)
#define SPI_3BYTE_ADDR_LEN              (3)

#define SPI_CMD_SR_WIP                  1    /* Write in Progress */
#define SPI_CMD_SR_WEL                  2    /* Write Enable Latch */

#define SPI_CMD_SR_QE                   (1 << 9)    /* quad enable */
#define SPI_CMD_SR_XQE                  (0 << 9)    /* quad disable */

#define SPI_NOR_SR_LEN                  1 /* Status Register length */
#define SPI_NOR_CR_LEN                  1 /* Config Register length */

#define SPI_NOR_MAX_ID_LEN 8
/*---------------------------------------------------------------------------*/
/* struct spinor_dev_info - spinor device information structure */
/*---------------------------------------------------------------------------*/
struct spinor_dev_info {
    char *name;/* Human-readable label */
    union {
        char id[SPI_NOR_MAX_ID_LEN];/* The full ID array */
        struct {
            uint8_t mfr_id;/* id[0]: Manufacturer ID */
            uint8_t dev_id;/* id[1]: Device ID */
        };
    };
    uint16_t id_len;/* The valid length of ID */

    uint32_t blocksize;/* Size of an erase block */

    uint64_t chipsize;/* Total size of the device */

    void *priv;
};

/*---------------------------------------------------------------------------*/
/* struct spinor_info - spinor various interface and information structure */
/*---------------------------------------------------------------------------*/
struct spinor_info {
    struct spinor_dev_info dev;
    int numchips;

    void *priv;

    int (*erase)(struct spinor_info *spinor, uint32_t addr, uint32_t len);
    int (*write)(struct spinor_info *spinor,
            uint32_t to, uint32_t len, const char *buf);
    int (*read)(struct spinor_info *spinor,
            uint32_t from, uint32_t len, const char *buf);
    void (*read_id)(struct spinor_info *spinor, char *id);
    void (*ids_probe)(struct spinor_info *spinor);
    int (*resume)(struct spinor_info *spinor);

    uint8_t cur_cs;
};

#endif /* End of __SPINOR_COMMON_H__ */

