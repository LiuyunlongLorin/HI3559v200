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

#ifndef    __ASM_PLATFORM_H__
#define    __ASM_PLATFORM_H__

#include "asm/hal_platform_ints.h"
#include "hisoc/timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define DDR_MEM_BASE              0x80000000

#define FMC_MEM_BASE              0x10000000

#define GPIO11_REG_BASE           0x120db000
#define GPIO10_REG_BASE           0x120da000
#define GPIO9_REG_BASE            0x120d9000
#define GPIO8_REG_BASE            0x120d8000
#define GPIO7_REG_BASE            0x120d7000
#define GPIO6_REG_BASE            0x120d6000
#define GPIO5_REG_BASE            0x120d5000
#define GPIO4_REG_BASE            0x120d4000
#define GPIO3_REG_BASE            0x120d3000
#define GPIO2_REG_BASE            0x120d2000
#define GPIO1_REG_BASE            0x120d1000
#define GPIO0_REG_BASE            0x120d0000

#define SPI2_REG_BASE             0x120c2000
#define SPI1_REG_BASE             0x120c1000
#define SPI0_REG_BASE             0x120c0000

#define I2C7_REG_BASE             0x120b7000
#define I2C6_REG_BASE             0x120b6000
#define I2C5_REG_BASE             0x120b5000
#define I2C4_REG_BASE             0x120b4000
#define I2C3_REG_BASE             0x120b3000
#define I2C2_REG_BASE             0x120b2000
#define I2C1_REG_BASE             0x120b1000
#define I2C0_REG_BASE             0x120b0000

#define UART4_REG_BASE            0x120a4000
#define UART3_REG_BASE            0x120a3000
#define UART2_REG_BASE            0x120a2000
#define UART1_REG_BASE            0x120a1000
#define UART0_REG_BASE            0x120a0000

#define MISC_REG_BASE             0x12030000
#define SYS_CTRL_REG_BASE         0x12020000
#define CRG_REG_BASE              0x12010000

#define SDIO1_REG_BASE            0x10020000
#define SDIO0_REG_BASE            0x100f0000

#define ETH_REG_BASE              0x10010000

#define VIC_REG_BASE              0x10040000
#define IRQ_REG_BASE              VIC_REG_BASE

#define DMAC_REG_BASE             0x10060000
#define FMC_REG_BASE              0x10000000

#define BIT(n)                    (1U << (n))
#define TIMER0_ENABLE             BIT(16)
#define TIMER1_ENABLE             BIT(17)
#define TIMER2_ENABLE             BIT(18)
#define TIMER3_ENABLE             BIT(19)
#define TIMER4_ENABLE             BIT(20)
#define TIMER5_ENABLE             BIT(21)
#define TIMER6_ENABLE             BIT(22)
#define TIMER7_ENABLE             BIT(23)

#define TIMER7_REG_BASE           0x12003020
#define TIMER6_REG_BASE           0x12003000
#define TIMER5_REG_BASE           0x12002020
#define TIMER4_REG_BASE           0x12002000
#define TIMER3_REG_BASE           0x12001020
#define TIMER2_REG_BASE           0x12001000
#define TIMER1_REG_BASE           0x12000020
#define TIMER0_REG_BASE           0x12000000
#define TIMER_TICK_REG_BASE       TIMER4_REG_BASE   /* timer for tick */
#define TIMER_TICK_ENABLE         TIMER4_ENABLE
#define TIMER_TIME_REG_BASE       TIMER5_REG_BASE   /* timer for time */
#define TIMER_TIME_ENABLE         TIMER5_ENABLE
#define HRTIMER_TIMER_REG_BASE    TIMER7_REG_BASE /* timer for hrtimer */
#define HRTIMER_TIMER_ENABLE      TIMER7_ENABLE

#define CACHE_ALIGNED_SIZE        64

#define ARM_REG_BASE              0x10300000

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

