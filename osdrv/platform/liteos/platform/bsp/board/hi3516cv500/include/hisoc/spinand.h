
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

#ifndef __HISOC_SPINAND_H__
#define __HISOC_SPINAND_H__

#include "asm/platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
/*****************************************************************************/
#define CRG48                             0xc0
#define CRG48_SPI_NAND_CLK_SEL(_clk)      (((_clk) & 0x3) << 6)
#define CRG48_SPI_NAND_CLK_EN             (1 << 5)
#define CRG48_SPI_NAND_SOFT_RST_REQ       (1 << 4)

#define SPI_NAND_CLK_SEL_MASK       (0x3 << 6)
#define DEVICE_TYPE_SHIFT            1
#define DEVICE_TYPE_MASK            (0x1 << 1)

#define CLK_24M                         0
#define CLK_75M                         1
#define CLK_125M                        2

#define SPI_NAND_CLK_SEL_24M        CRG48_SPI_NAND_CLK_SEL(CLK_24M)
#define SPI_NAND_CLK_SEL_75M        CRG48_SPI_NAND_CLK_SEL(CLK_75M)
#define SPI_NAND_CLK_SEL_125M       CRG48_SPI_NAND_CLK_SEL(CLK_125M)

#define GET_CLK_TYPE(_reg)          (((_reg) >> 2) & 0x3)

/*****************************************************************************/
static void hisnfc100_set_system_clock(int clock, int clk_en)
{
    unsigned base = CRG_REG_BASE;
    unsigned regval = readl(base + CRG48);

    if(!clock)
        clock = SPI_NAND_CLK_SEL_75M;
    regval = (regval & SPI_NAND_CLK_SEL_MASK) | clock;

    if (clk_en)
        regval |= CRG48_SPI_NAND_CLK_EN;
    else
        regval &= ~CRG48_SPI_NAND_CLK_EN;

    if (readl(base + CRG48) != regval)
        writel(regval, (base + CRG48));
}

/*****************************************************************************/
static void hisnfc100_get_best_clock(unsigned int *clock)
{
    int ix;
    int clk_reg;
#define CLK_2X(_clk)    (((_clk) + 1) >> 1)
    unsigned int sysclk[] = {
        CLK_2X(24),    SPI_NAND_CLK_SEL_24M,
        CLK_2X(75),    SPI_NAND_CLK_SEL_75M,
        CLK_2X(125),    SPI_NAND_CLK_SEL_125M,
        0, 0,
    };
#undef CLK_2X

    clk_reg = SPI_NAND_CLK_SEL_24M;
    for (ix = 0; sysclk[ix]; ix += 2) {

        if (*clock < sysclk[ix])
            break;
        clk_reg = sysclk[ix + 1];
    }
    *clock = clk_reg;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

