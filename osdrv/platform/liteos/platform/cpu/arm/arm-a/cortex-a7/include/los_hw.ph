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

#ifndef _LOS_HW_PH
#define _LOS_HW_PH

#include "los_base.h"
#include "asm/dma.h"
#include "los_hw.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define  OS_MAIN_OSC_FREQ_HZ                      12000000uL
#define  OS_INTER_OSC_FREQ_HZ                     4000000uL
#define  OS_RTC_OSC_FREQ_HZ                       32768uL

/* ------- CLOCK SOURCE SELECT REGISTER DEFINE ------ */
#define  OS_REG_ADDR_CLKSRCSEL                    (*(UINT32 *)(0xE01FC10C))

/* ------------- PLL REGISTER DEFINES --------------- */
#define  OS_PLL_BASE_ADDR                         (0xE01FC080u)
#define  OS_REG_VAL_PLLCTRL                       (*(UINT32 *)(OS_PLL_BASE_ADDR + 0x00))
#define  OS_REG_VAL_PLLCFG                        (*(UINT32 *)(OS_PLL_BASE_ADDR + 0x04))
#define  OS_REG_VAL_PLLSTAT                       (*(UINT32 *)(OS_PLL_BASE_ADDR + 0x08))
#define  OS_REG_VAL_PLLFEED                       (*(UINT32 *)(OS_PLL_BASE_ADDR + 0x0C))

/* --------- POWER CONTROL REGISTER DEFINES --------- */
#define  OS_REG_VAL_PCON                          (*(UINT32 *)(0xE01FC0C0))
#define  OS_REG_VAL_PCONP                         (*(UINT32 *)(0xE01FC0C4))

/* -------- CLOCK DIVIDERS REGISTER DEFINITION ------ */
#define  OS_REG_VAL_CCLKCFG                       (*(UINT32 *)(0xE01FC104))
#define  OS_REG_VAL_USBCLKCFG                     (*(UINT32 *)(0xE01FC108))
#define  OS_REG_VAL_IRCTRIM                       (*(UINT32 *)(0xE01FC1A4))
#define  OS_REG_VAL_PCLKSEL0                      (*(UINT32 *)(0xE01FC1A8))
#define  OS_REG_VAL_PCLKSEL1                      (*(UINT32 *)(0xE01FC1AC))

/* ------------------ CLOCK DIVIDERS ---------------- */
#define  OS_CLK_DIV_1                             1
#define  OS_CLK_DIV_2                             2
#define  OS_CLK_DIV_4                             4
#define  OS_CLK_DIV_6                             6
#define  OS_CLK_DIV_8                             8

/* --------- PLL STATUS REGISTER BIT DEFINES -------- */
#define  OS_PLLSTAT_MSEL_MASK                     0x7FFF
#define  OS_PLLSTAT_NSEL_MASK                     0xFF0000
#define  OS_PLLSTAT_PLLE                          (1uL << 24)
#define  OS_PLLSTAT_PLLC                          (1uL << 25)
#define  OS_PLLSTAT_PLOCK                         (1uL << 26)

/* ------------- PERIPHEAL CLOCK DEFINES ------------ */
#define  OS_PCLKSEL_WDT                           0   /* WatchDog Timer. This clock can not be turned off   */
#define  OS_PCONP_RSV0                            0

#define  OS_PERIPH_CLK_TMR0                       1   /* Timer 0                                            */
#define  OS_PERIPH_CLK_TMR1                       2   /* Timer 1                                            */
#define  OS_PERIPH_CLK_UART0                      3   /* UART 0                                             */
#define  OS_PERIPH_CLK_UART1                      4   /* UART 1                                             */
#define  OS_PERIPH_CLK_PWM0                       5   /* PWM 0                                              */
#define  OS_PERIPH_CLK_PWM1                       6   /* PWM1                                               */
#define  OS_PERIPH_CLK_I2C0                       7   /* I2C 0                                              */
#define  OS_PERIPH_CLK_SPI                        8   /* SPI 0                                              */
#define  OS_PERIPH_CLK_RTC                        9   /* RTC                                                */
#define  OS_PERIPH_CLK_SSP1                      10   /* SSP 1                                              */

#define  OS_PCLKSEL_DAC                          11   /* D/A converter                                      */
#define  OS_PCONP_EMC                            11   /* External Memory Controller                         */

#define  OS_PERIPH_CLK_ADC                       12   /* A/D converter                                      */
#define  OS_PERIPH_CLK_CAN1                      13   /* CAN 1                                              */
#define  OS_PERIPH_CLK_CAN2                      14   /* CAN 2                                              */

#define  OS_PCLKSEL_ACF                          15   /* CAN filtering. This clock can not be turned off    */
#define  OS_PCONP_RSV1                           15
#define  OS_PCLKSEL_BAT_RAM                      16   /* Battery Supported RAM. It can not be turned off    */
#define  OS_PCONP_RSV2                           16
#define  OS_PCLKSEL_GPIO                         17   /* GPIOs. This clock can not be turned off            */
#define  OS_PCONP_RSV3                           17
#define  OS_PCLKSEL_PCB                          18   /* Pin conn block. This clock can not be turned off   */
#define  OS_PCONP_RSV4                           18

#define  OS_PERIPH_CLK_I2C1                      19   /* I2C1                                               */

#define  OS_PCONP_LCD                            20   /* LCD. LPC247x only                                  */
#define  OS_PCLKSEL_RSV0                         20

#define  OS_PERIPH_CLK_SSP0                      21   /* SSP0                                               */
#define  OS_PERIPH_CLK_TMR2                      22   /* TIMER 2                                            */
#define  OS_PERIPH_CLK_TMR3                      23   /* TIMER 3                                            */
#define  OS_PERIPH_CLK_UART2                     24   /* UART 2                                             */
#define  OS_PERIPH_CLK_UART3                     25   /* UART 3                                             */
#define  OS_PERIPH_CLK_I2C2                      26   /* I2C 2                                              */
#define  OS_PERIPH_CLK_I2S                       27   /* I2S                                                */

#define  OS_PCONP_SDC                            28   /* SD card interface                                  */
#define  OS_PCLKSEL_MCI                          28   /* MCI                                                */
#define  OS_PCONP_GPDMA                          29   /* GP DMA function                                    */
#define  OS_PCLKSEL_RSV1                         29
#define  OS_PCONP_ENET                           30   /* Ethernet Controller                                */
#define  OS_PCLKSEL_SYSCON                       30
#define  OS_PCONP_USB                            31   /* USB                                                */
#define  OS_PCLKSEL_RSV2                         31

/* --------- TIMER COUNTER BASE ADDR DEFINES -------- */
#define  OS_TMR0_BASE_REG                        ((OS_TMR_REG_S *)(0xE0004000))
#define  OS_TMR1_BASE_REG                        ((OS_TMR_REG_S *)(0xE0008000))
#define  OS_TMR2_BASE_REG                        ((OS_TMR_REG_S *)(0xE0070000))
#define  OS_TMR3_BASE_REG                        ((OS_TMR_REG_S *)(0xE0074000))

/* ------- TIMER CONTROL REGISTER BIT DEFINES ------- */
#define  OS_REG_TCR_EN                           (1uL << 0)
#define  OS_REG_TCR_RST                          (1uL << 1)

/* ------- TIMER INTERRUPT REGISTER BIT DEFINES ------- */
#define  OS_REG_IR_MR0                           (1uL << 0)
#define  OS_REG_IR_MR1                           (1uL << 1)
#define  OS_REG_IR_MR2                           (1uL << 2)
#define  OS_REG_IR_MR3                           (1uL << 3)
#define  OS_REG_IR_CR0                           (1uL << 4)
#define  OS_REG_IR_CR1                           (1uL << 5)
#define  OS_REG_IR_CR2                           (1uL << 6)
#define  OS_REG_IR_CR3                           (1uL << 7)

/* ------- MATCH CONTROL REGISTER BIT DEFINES ------- */
#define  OS_REG_TMCR_MR0I                        (1uL << 0)
#define  OS_REG_TMCR_MR0R                        (1uL << 1)
#define  OS_REG_TMCR_MR0S                        (1uL << 2)

#define  OS_REG_TMCR_MR1I                        (1uL << 3)
#define  OS_REG_TMCR_MR1R                        (1uL << 4)
#define  OS_REG_TMCR_MR1S                        (1uL << 5)

#define  OS_REG_TMCR_MR2I                        (1uL << 6)
#define  OS_REG_TMCR_MR2R                        (1uL << 7)
#define  OS_REG_TMCR_MR2S                        (1uL << 8)

#define  OS_REG_TMCR_MR3I                        (1uL << 9)
#define  OS_REG_TMCR_MR3R                        (1uL << 10)
#define  OS_REG_TMCR_MR3S                        (1uL << 11)

typedef  struct tagOsTmrRegs{
    UINT32  uwIR;
    UINT32  uwTCR;
    UINT32  uwTC;
    UINT32  uwPR;
    UINT32  uwPC;
    UINT32  uwMCR;
    UINT32  uwMR[4];
    UINT32  uwCCR;
    UINT32  uwCR[4];
    UINT32  uwEMR;
} OS_TMR_REG_S;

typedef struct tagPllRegs {
    UINT32  uwPllCtrl;
    UINT32  uwPllCfg;
    UINT32  uwPllStat;
    UINT32  uwPllFeed;
} OS_PLL_REG_S;

/*The size of this structure must be smaller than or equal to the size specified by OS_TSK_STACK_ALIGN (16 bytes).*/
typedef struct tagTskContext
{
    UINT64 D[32];        /*D0-D31*/
    UINT32 uwPSR;
    UINT32 auwR[13];     /*R0-R12*/
    UINT32 uwLR;         /*R14*/
    UINT32 uwPC;         /*R15*/
} TSK_CONTEXT_S;

extern VOID  *osTskStackInit(UINT32 uwTaskID, UINT32 uwStackSize, VOID *pTopStack);
extern VOID osSchedule(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LOS_HW_PH */

