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

#ifndef __AVS_PM_H__
#define __AVS_PM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef struct hiHPM_VF_S
{
    unsigned int freq;          /* unit: kHz */
    unsigned int vmin;
    unsigned int hpmrecord;     /* hpm record */
    unsigned int div;           /* frequency division ratio */
} HPM_VF_S;

struct avs_dev
{
    unsigned int avs_enable;
    unsigned int cur_volt;
    unsigned int cur_volt_max;
    unsigned int cur_volt_min;

    HPM_VF_S freq_hpm_table[5];
    unsigned int max_hpm_tale_index;
    /* hpm ctrl parameter */
    unsigned int cur_freq;
    unsigned int cur_hpm;
    unsigned int hpm_uplimit;
    unsigned int hpm_downlimit;
    unsigned int low_period;
    unsigned int high_period;
    unsigned int div;
    unsigned int shift;
    unsigned int offset;

    /* hw avs hpm parameter */
    unsigned int hpm_fsm_mode;
    unsigned int hpm_pwm_inc_step;
    unsigned int hpm_pwm_dec_step;

    /* timing sensor ctrl parameter */
    unsigned int ts_crazy_mode;
    unsigned int ts_crazy_path_dly;
    unsigned int ts_code_step;

    /* timing sensor hw avs parameter */
    unsigned int ts_avs_mode;
    unsigned int ts_avs_period;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
