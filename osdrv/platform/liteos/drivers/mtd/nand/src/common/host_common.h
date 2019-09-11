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

#ifndef __HOST_COMMON_H__
#define __HOST_COMMON_H__

#include "mtd_common.h"

#define GET_OP                    0
#define SET_OP                    1

/* ECC type macros and enum  */
#define NAND_ECC_0BIT          0
#define NAND_ECC_8BIT_1K       1
#define NAND_ECC_16BIT_1K      2
#define NAND_ECC_24BIT_1K      3
#define NAND_ECC_28BIT_1K      4
#define NAND_ECC_40BIT_1K      5
#define NAND_ECC_64BIT_1K      6

#include "mtd_common.h"
#define reg_read(_host, _reg) \
        mtd_readl((AARCHPTR)((char *)_host->regbase + (_reg)))

#define reg_write(_host, _value, _reg) \
        mtd_writel((unsigned)(_value), (AARCHPTR)((char *)_host->regbase + (_reg)))

struct nand_host {
    struct nand_info *nand;

    void *priv;

    char *regbase;
    char *membase;

    unsigned int cfg;
    unsigned int cfg_ecc0;

    uint32_t pagesize;
    uint32_t oobsize;
    uint32_t ecctype;
    uint32_t addr_cycle;

    //use for dma transfer
    char *buforg;
    char *buffer;
    char *dma_data;
    char *dma_oob;

#define NAND_BAD_BLOCK_POS        0
    char *bbm;/* nand bad block mark */
#define NAND_EMPTY_BLOCK_POS        30
    char *ebm;/* nand empty block mark */

    uint32_t flags;
    uint16_t version;
};

const char *nand_ecc_name(int type);

#endif /* End of __HOST_COMMON_H__ */

