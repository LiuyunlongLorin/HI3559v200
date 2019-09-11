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

#ifndef __HISOC_SPI_H__
#define __HISOC_SPI_H__

#include "asm/platform.h"
#include "hisoc/clock.h"
#include "asm/io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define SPI_CLK_RATE   100000000

#define SPI0_FIFO_SIZE 256
#define SPI1_FIFO_SIZE 256
#define SPI2_FIFO_SIZE 256
#define SPI3_FIFO_SIZE 256
#define SPI4_FIFO_SIZE 256

#define SPI0_ENABLE 1
#define SPI1_ENABLE 1
#define SPI2_ENABLE 1
#define SPI3_ENABLE 0
#define SPI4_ENABLE 0

#define SPI0_NUM_CS     1
#define SPI1_NUM_CS     1
#define SPI2_NUM_CS     2
#define SPI3_NUM_CS     2
#define SPI4_NUM_CS     4

#define TOTAL_SPI_NUM (!!SPI0_NUM_CS + !!SPI1_NUM_CS + !!SPI2_NUM_CS + !!SPI3_NUM_CS + !!SPI4_NUM_CS)
#define TOTAL_SPI_CS_NUM (SPI0_NUM_CS + SPI1_NUM_CS + SPI2_NUM_CS + SPI3_NUM_CS + SPI4_NUM_CS)

#define REG_CRG102_MAP           (CRG_REG_BASE + 0x0e4)
#define CRG102_SSP4_CKEN         (0x1 << 20) /* 0:close clk, 1:open clk */
#define CRG102_SSP3_CKEN         (0x1 << 19) /* 0:close clk, 1:open clk */
#define CRG102_SSP2_CKEN         (0x1 << 18) /* 0:close clk, 1:open clk */
#define CRG102_SSP1_CKEN         (0x1 << 17) /* 0:close clk, 1:open clk */
#define CRG102_SSP0_CKEN         (0x1 << 16) /* 0:close clk, 1:open clk */

#define CRG102_SSP4_RST          (0x1 << 4)/* 0:cancel reset, 1:reset */
#define CRG102_SSP3_RST          (0x1 << 3)/* 0:cancel reset, 1:reset */
#define CRG102_SSP2_RST          (0x1 << 2) /* 0:cancel reset, 1:reset */
#define CRG102_SSP1_RST          (0x1 << 1) /* 0:cancel reset, 1:reset */
#define CRG102_SSP0_RST          (0x1 << 0) /* 0:cancel reset, 1:reset */

#define REG_MISC_CTRL1_MAP      (MISC_REG_BASE + 0x24)

#define MISC_CTRL1_SSP2_CS_SHIFT        (0)
#define MISC_CTRL1_SSP2_CS              (0x1 << MISC_CTRL1_SSP2_CS_SHIFT) /* 00:cs0, 01:cs1 */
#define MISC_CTRL1_SSP3_CS_SHIFT        (1)
#define MISC_CTRL1_SSP3_CS              (0x1 << MISC_CTRL1_SSP3_CS_SHIFT) /* 00:cs0, 01:cs1 */
#define MISC_CTRL1_SSP4_CS_SHIFT        (2)
#define MISC_CTRL1_SSP4_CS              (0x3 << MISC_CTRL1_SSP4_CS_SHIFT) /* 00:cs0, 01:cs1 */


static int spi_cfg_cs(uint16_t bus_num, uint8_t csn)
{
    unsigned int value;
    switch (bus_num) {
    case 0:
        if ((csn + 1) > SPI0_NUM_CS) {
            PRINT_ERR("%s, %s, %d line: error\n",
                    __FILE__, __func__, __LINE__);
            return -1;
        }
        break;

    case 1:
        if ((csn + 1) > SPI1_NUM_CS) {
            PRINT_ERR("%s, %s, %d line: error\n",
                    __FILE__, __func__, __LINE__);
            return -1;
        }
        break;
    case 2:
        if (csn < SPI2_NUM_CS) {
            value = readl(REG_MISC_CTRL1_MAP);
            value &= ~MISC_CTRL1_SSP2_CS;
            value |= (csn << MISC_CTRL1_SSP2_CS_SHIFT);
            writel(value, REG_MISC_CTRL1_MAP); /* set spi2 csn */
        } else {
            PRINT_ERR("%s, %s, %d line: error\n",
                    __FILE__, __func__, __LINE__);
            return -1;
        }
        break;
    case 3:
        if (csn < SPI3_NUM_CS) {
            value = readl(REG_MISC_CTRL1_MAP);
            value &= ~MISC_CTRL1_SSP3_CS;
            value |= (csn << MISC_CTRL1_SSP3_CS_SHIFT);
            writel(value, REG_MISC_CTRL1_MAP); /* set spi3 csn */
        } else {
            PRINT_ERR("%s, %s, %d line: error\n",
                    __FILE__, __func__, __LINE__);
            return -1;
        }
        break;
    case 4:
        if (csn < SPI4_NUM_CS) {
            value = readl(REG_MISC_CTRL1_MAP);
            value &= ~MISC_CTRL1_SSP4_CS;
            value |= (csn << MISC_CTRL1_SSP4_CS_SHIFT);
            writel(value, REG_MISC_CTRL1_MAP); /* set spi4 csn */
        } else {
            PRINT_ERR("%s, %s, %d line: error\n",
                    __FILE__, __func__, __LINE__);
            return -1;
        }
        break;
    default:
        PRINT_ERR("%s, %s, %d line: error\n",
                __FILE__, __func__, __LINE__);
        return -1;

    }
    return 0;
}

static int spi_hw_init_cfg(uint16_t bus_num)
{
    unsigned int value;

    switch (bus_num) {
    case 0:
        value = readl(REG_CRG102_MAP);
        value &= ~CRG102_SSP0_RST;
        value |= CRG102_SSP0_CKEN;
        writel(value, REG_CRG102_MAP); /* open spi0 clk */
        break;

    case 1:
        value = readl(REG_CRG102_MAP);
        value &= ~CRG102_SSP1_RST;
        value |= CRG102_SSP1_CKEN;
        writel(value, REG_CRG102_MAP); /* open spi1 clk */
        break;

    case 2:
        value = readl(REG_CRG102_MAP);
        value &= ~CRG102_SSP2_RST;
        value |= CRG102_SSP2_CKEN;
        writel(value, REG_CRG102_MAP); /* open spi2 clk */
        break;

    case 3:
        value = readl(REG_CRG102_MAP);
        value &= ~CRG102_SSP3_RST;
        value |= CRG102_SSP3_CKEN;
        writel(value, REG_CRG102_MAP); /* open spi3 clk */
        break;

    case 4:
        value = readl(REG_CRG102_MAP);
        value &= ~CRG102_SSP4_RST;
        value |= CRG102_SSP4_CKEN;
        writel(value, REG_CRG102_MAP); /* open spi3 clk */
        break;

    default:
        PRINT_ERR("%s, %s, %d line: error\n",
                __FILE__, __func__, __LINE__);
        return -1;
    }
    return 0;
}

static int spi_hw_exit_cfg(uint16_t bus_num)
{
    unsigned int value;

    switch (bus_num) {
    case 0:
        value = readl(REG_CRG102_MAP);
        value |= CRG102_SSP0_RST;
        value &= ~CRG102_SSP0_CKEN;
        writel(value, REG_CRG102_MAP); /* close spi0 clk */
        break;

    case 1:
        value = readl(REG_CRG102_MAP);
        value |= CRG102_SSP1_RST;
        value &= ~CRG102_SSP1_CKEN;
        writel(value, REG_CRG102_MAP);/* close spi1 clk */
        break;

    case 2:
        value = readl(REG_CRG102_MAP);
        value |= CRG102_SSP2_RST;
        value &= ~CRG102_SSP2_CKEN;
        writel(value, REG_CRG102_MAP);/* close spi2 clk */
        break;

    case 3:
        value = readl(REG_CRG102_MAP);
        value |= CRG102_SSP3_RST;
        value &= ~CRG102_SSP3_CKEN;
        writel(value, REG_CRG102_MAP);/* close spi3 clk */
        break;

    case 4:
        value = readl(REG_CRG102_MAP);
        value |= CRG102_SSP4_RST;
        value &= ~CRG102_SSP4_CKEN;
        writel(value, REG_CRG102_MAP);/* close spi4 clk */
        break;

    default:
        PRINT_ERR("%s, %s, %d line: error\n",
                __FILE__, __func__, __LINE__);
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

