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
#ifndef __HISOC_MMC_H_
#define __HISOC_MMC_H_

/************************************************************************/

#include "asm/platform.h"
#include "mmc/mmc_os_adapt.h"
#include "mmc/host.h"
#include "sdhci_reg.h"
#include "sdhci.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MAX_MMC_NUM    3

#define MMC0    0
#define MMC1    1
#define MMC2    2
/* *
 * MMC HOST useable
 * Set to 1: useable
 * Set to 0: not useable
 * */
#define USE_MMC0    1
#define USE_MMC1    0
#define USE_MMC2    1

#define CONFIG_MMC0_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC0_CCLK_MAX    150000000  //150MHz
#define CONFIG_MMC1_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC1_CCLK_MAX    150000000  //150MHz
#define CONFIG_MMC2_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC2_CCLK_MAX    200000000  //200MHz

#define PERI_CRG86              (CRG_REG_BASE + 0x158)
#define PERI_CRG87              (CRG_REG_BASE + 0x15C)
#define PERI_CRG94              (CRG_REG_BASE + 0x178)
#define PERI_CRG95              (CRG_REG_BASE + 0x17C)
#define PERI_CRG96              (CRG_REG_BASE + 0x180)

#define EMMC_PHY_BASE           0x12160000

#define MMC_FREQ_100K       100000
#define MMC_FREQ_400K       400000
#define MMC_FREQ_25M        25000000
#define MMC_FREQ_50M        50000000
#define MMC_FREQ_100M       100000000
#define MMC_FREQ_125M       125000000
#define MMC_FREQ_150M       150000000
#define MMC_FREQ_200M       200000000


#define MMC_FREQ_MASK       0x7
#define MMC_FREQ_SHIFT      8
#define SDIO0_CLK_SEL_100K  (0)
#define SDIO0_CLK_SEL_400K  (7)
#define SDIO0_CLK_SEL_25M   (1)
#define SDIO0_CLK_SEL_50M   (2)
#define SDIO0_CLK_SEL_100M  (3)
#define SDIO0_CLK_SEL_125M  (4)
#define SDIO0_CLK_SEL_150M  (5)
#define SDIO0_CLK_SEL_200M  (6)


/************************************************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

