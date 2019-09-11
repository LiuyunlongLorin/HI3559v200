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

#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern int g_cur_profile;
extern struct cpu_freq{
    int freq;
    int Volt;
}cpu_freq_table[4];
    /*
 * macro define
 */
/* opp table define */
#define HI_VDD_CPU_OPP0_MV              1020
#define HI_VDD_CPU_OPP0_FREQ            400
#define HI_VDD_CPU_OPP0_AVS_VMIN_MV     940
#define HI_VDD_CPU_OPP0_AVS_HPM         240
#define HI_VDD_CPU_OPP0_AVS_HPM_DIV     8


#define HI_VDD_CPU_OPP1_MV              1100
#define HI_VDD_CPU_OPP1_FREQ            600
#define HI_VDD_CPU_OPP1_AVS_VMIN_MV     920
#define HI_VDD_CPU_OPP1_AVS_HPM         270
#define HI_VDD_CPU_OPP1_AVS_HPM_DIV     11

#define HI_VDD_CPU_OPP2_MV              1200
#define HI_VDD_CPU_OPP2_FREQ            732
#define HI_VDD_CPU_OPP2_AVS_VMIN_MV     1000
#define HI_VDD_CPU_OPP2_AVS_HPM         325
#define HI_VDD_CPU_OPP2_AVS_HPM_DIV     14

#define HI_VDD_CPU_OPP3_MV              1300
#define HI_VDD_CPU_OPP3_FREQ            850
#define HI_VDD_CPU_OPP3_AVS_VMIN_MV     1070
#define HI_VDD_CPU_OPP3_AVS_HPM         365
#define HI_VDD_CPU_OPP3_AVS_HPM_DIV     16

#define MIN_CPU_FREQ                    HI_VDD_CPU_OPP1_FREQ
#define MAX_CPU_FREQ                    HI_VDD_CPU_OPP3_FREQ

#define CPU_VMAX                        1310 /*mv*/
#define CPU_VMIN                        800 /*mv*/

int cpu_VoltScale(int uvolt);

void cpu_AvsResume(void);
int avs_cpu_handler(void);
int cpu_AvsInit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
