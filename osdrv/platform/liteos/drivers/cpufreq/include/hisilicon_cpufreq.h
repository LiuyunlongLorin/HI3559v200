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

#ifndef _HISILICON_CPUFREQ_H
#define _HISILICON_CPUFREQ_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include "cpufreq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct cpu_freq{
    int freq;
    int volt;
};


struct cpufreq_config {
    unsigned int *freq_table;
    unsigned int tbl_cnt;
    int (*set_voltage) (unsigned int index);
    int (*set_current) (unsigned int index);
};


/* Special Values of .frequency field */
#define CPUFREQ_ENTRY_INVALID   ~0u
#define CPUFREQ_TABLE_END   ~1u

static inline bool cpufreq_next_valid(struct cpu_freq **pos)
{
    while ((*pos)->freq != CPUFREQ_TABLE_END)
        if ((*pos)->freq != CPUFREQ_ENTRY_INVALID)
            return true;
        else
            (*pos)++;
    return false;
}


/*
 * cpufreq_for_each_entry - iterate over a cpufreq_frequency_table
 * @pos:    the cpufreq_frequency_table * to use as a loop cursor.
 * @table:  the cpufreq_frequency_table * to iterate over.
 */
#define cpufreq_for_each_entry(pos, table)  \
    for (pos = table; pos->freq != CPUFREQ_TABLE_END; pos++)



extern void pm_timer_init(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _HISILICON_CPUFREQ_H */


