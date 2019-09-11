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
#ifndef __SYS_PM_H__
#define __SYS_PM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MAX_FREQ_ADJUST_CNT 500
#define MAX_I2C_ADJUST_CNT 500

#define PERI_CRG0     (CRG_REG_BASE + 0x0)
#define PERI_CRG1     (CRG_REG_BASE + 0x4)

#define PERI_CRG2     (CRG_REG_BASE + 0x8)
#define PERI_CRG3     (CRG_REG_BASE + 0xC)
#define PERI_CRG11    (CRG_REG_BASE + 0x2C)
#define PERI_CRG12    (CRG_REG_BASE + 0x30)
#define PERI_CRG16    (CRG_REG_BASE + 0x40)
#define PERI_CRG18    (CRG_REG_BASE + 0x48)
#define PERI_CRG23    (CRG_REG_BASE + 0x5C)
#define PERI_CRG49    (CRG_REG_BASE + 0xC4)
#define PERI_CRG58    (CRG_REG_BASE + 0xE8)

#define PERI_PMC_0   (PMC_REG_BASE + 0x0)
#define PERI_PMC_1   (PMC_REG_BASE + 0x4)

#define PERI_PMC_6   (PMC_REG_BASE + 0x18)
#define PERI_PMC_7   (PMC_REG_BASE + 0x1C)
#define PERI_PMC_9   (PMC_REG_BASE + 0x24)
#define PERI_PMC_14  (PMC_REG_BASE + 0x38)
#define PERI_PMC_15  (PMC_REG_BASE + 0x3C)
#define PERI_PMC_16  (PMC_REG_BASE + 0x40)
#define PERI_PMC_17  (PMC_REG_BASE + 0x44)

#define PERI_PMC_19  (PMC_REG_BASE + 0x4C)
#define PERI_PMC_20  (PMC_REG_BASE + 0x50)


#define PERI_PMC_22  (PMC_REG_BASE + 0x58)
#define PERI_PMC_23  (PMC_REG_BASE + 0x5c)
#define PERI_PMC_24  (PMC_REG_BASE + 0x60)
#define PERI_PMC_25  (PMC_REG_BASE + 0x64)
#define PERI_PMC_26  (PMC_REG_BASE + 0x68)
#define PERI_PMC_29  (PMC_REG_BASE + 0x74)
#define PERI_PMC_31  (PMC_REG_BASE + 0x7c)
#define PERI_PMC_32  (PMC_REG_BASE + 0x80)
#define PERI_PMC_30  (PMC_REG_BASE + 0x78)
#define PERI_PMC_33  (PMC_REG_BASE + 0x84)

#define PERI_PMC_58  (PMC_REG_BASE + 0xE8)
#define PERI_PMC_65  (PMC_REG_BASE + 0x104)
#define PERI_PMC_67  (PMC_REG_BASE + 0x10C)

#define HPM_PC_RECORED_MASK 0x3ff

#define PERI_MUX83  (IO_MUX_REG_BASE + 0x14C)
#define PERI_MUX84  (IO_MUX_REG_BASE + 0x150)
#define PERI_MUX85  (IO_MUX_REG_BASE + 0x154)
#define PERI_MUX86  (IO_MUX_REG_BASE + 0x158)

#define PROFILE_REG  (SYS_CTRL_REG_BASE + 0x015C)

#define PM_CPU      (0x01L<<0)
#define PM_MEDIA    (0x01L<<1)
#define PM_R1    127  //12.7k|?
#define PM_R2    127  //12.7k|?
#define PM_R3    806  //80.6k|?
#define PM_R4    5    //0.5k|?
#define PM_R5    10   //1k|?
#define PM_VREF  608  //608mv


struct PM_DC_DC_ATTR_S
{
    int Vref;
    int R1;
    int R2;
    int R3;
    int R4;
    int PwmFreqN;
    int Vout_base;
    int VoltConfig;
    int Rconfig;
};


typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hw_profile            : 4   ; /* [3..0]  */
        unsigned int    volatge               : 4   ; /* [7..4]  */
        unsigned int    volatge_way           : 4   ; /* [11..8]  */
        unsigned int    hw_mda_profile        : 4   ; /* [15..12]  */
        unsigned int    sw_mda_profile        : 4   ; /* [19..16]  */
        unsigned int    sw_mda_available      : 1   ; /* [20]  */
        unsigned int    sw_mda_setting        : 1   ; /* [21]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned long    u32;

} U_PROFILE_REG;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
