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

#ifndef __HISOC_SPINOR_H__
#define __HISOC_SPINOR_H__

#include "asm/platform.h"
#include "asm/io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PERI_CRG48                (CRG_REG_BASE + 0x00C0)
#define PERI_CRG48_RST            (1 << 0)
#define PERI_CRG48_CLKEN            (1 << 1)
#define PERI_CRG48_CLK_24M            (0 << 2)
#define PERI_CRG48_CLK_75M            ((0 << 3) | (1 << 2))
#define PERI_CRG48_CLK_125M            ((1 << 2) | (1 << 3))

#define SFC_ADDR_MODE_REG            (0x8C)
#define SFC_ADDR_MODE_MASK            (0x80)
#define SFC_CLSEL_MASK                (0xC)
#define SFC_PERI_CLKDIV1_SHIFT            (28)
#define SFC_PERI_CLKDIV1_MASK            (0xF)

/*****************************************************************************/
#undef GET_SFC_ADDR_MODE
#define GET_SFC_ADDR_MODE ({\
    int start_up_mode = 0;\
    start_up_mode = readl(IO_ADDRESS(SYS_CTRL_REG_BASE + SFC_ADDR_MODE_REG));\
    start_up_mode &= SFC_ADDR_MODE_MASK;\
    start_up_mode; })

/*****************************************************************************/
static void hisfc350_set_system_clock(unsigned clock, int clk_en)
{
    unsigned int regval = readl(PERI_CRG48);

    regval = regval & (~SFC_CLSEL_MASK);

    if (clock) {
        regval &= ~SFC_CLSEL_MASK;
        regval |= clock & SFC_CLSEL_MASK;
    } else {
        regval &= ~SFC_CLSEL_MASK;
        regval |= PERI_CRG48_CLK_24M; /* Default Clock */
    }

    if (clk_en)
        regval |= PERI_CRG48_CLKEN;

    if (regval != readl(PERI_CRG48))
        writel(regval, (PERI_CRG48));
}

/*****************************************************************************/
static void hisfc350_get_best_clock(unsigned int *clock)
{
    int ix;
    int clk_reg;

#define CLK_2X(_clk)    (((_clk) + 1) >> 1)
    unsigned int sysclk[] = {
        CLK_2X(24),    PERI_CRG48_CLK_24M,
        CLK_2X(75),    PERI_CRG48_CLK_75M,
        CLK_2X(125),    PERI_CRG48_CLK_125M,
        0, 0,
    };
#undef CLK_2X

    clk_reg = PERI_CRG48_CLK_24M;
    for (ix = 0; sysclk[ix]; ix += 2) {
        if (*clock < sysclk[ix])
            break;
        clk_reg = sysclk[ix + 1];
    }

    *clock = clk_reg;
}

/*****************************************************************************/
#ifdef CONFIG_HISFC350_SHOW_CYCLE_TIMING
static char * hisfc350_get_clock_str(unsigned int clk_reg)
{
    static char buffer[40];

    /* calculate reference PERI_CLKDIV1[31:28] */
    SFC_PR(BT_DBG, "clk_reg=0x%0x.\n",clk_reg);
    clk_reg = 216 / ((clk_reg >> SFC_PERI_CLKDIV1_SHIFT)
                & SFC_PERI_CLKDIV1_MASK);
    sprintf(buffer, "%dM", clk_reg);

    return buffer;
}
#endif /* CONFIG_PERI_SHOW_CYCLE_TIMING */


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif

