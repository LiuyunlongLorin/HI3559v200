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

#ifndef _CPUFREQ_H
#define _CPUFREQ_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include "linux/device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct notifier_block;

#define CPUFREQ_NAME_LEN        16

struct cpufreq_freqs {
    unsigned int olds;
    unsigned int news;
};


typedef int (*notifier_fn_t)(struct notifier_block *nb,
            unsigned long action, void *data);

struct notifier_block {
    notifier_fn_t notifier_call;
    struct notifier_block *next;
};

struct notifier_block_head {
    unsigned int lock;
    struct notifier_block *head;
};

//cpufreq info
struct cpufreq_info {
    unsigned int    max_freq;
    unsigned int    min_freq;
    /* in 10^(-9) s = nanoseconds */
    unsigned int    transition_latency;
    unsigned int sampling_rate;           //sample rate

};


//cpufreq policy
struct cpufreq_policy {
    struct cpufreq_info cpuinfo;
    unsigned int        min;    /* in kHz */
    unsigned int        max;    /* in kHz */
    unsigned int        cur;    /* in kHz  */
    unsigned int        restore_freq; /* = policy->cur before transition */
    unsigned int        max_index;
    unsigned int       *freq_table;
    void               *driver_data;  /* For cpufreq driver's internal use */
};

//cpufreq driver
struct cpufreq_driver {
    char        name[CPUFREQ_NAME_LEN];
    unsigned char   flags;
    void        *driver_data;
    unsigned int        lock;
    struct device dev;
    /* needed by all drivers */
    int     (*init)(struct cpufreq_policy *policy);
    int     (*verify)(struct cpufreq_policy *policy);
    int     (*target_index)(struct cpufreq_policy *policy,unsigned int index);
    unsigned int    (*get)(void);
    int     (*exit)(struct cpufreq_policy *policy);
    int     (*suspend)(struct cpufreq_policy *policy);
    int     (*resume)(struct cpufreq_policy *policy);
};



extern int cpufreq_register_driver(struct cpufreq_driver *driver_data);
extern int cpufreq_unregister_driver(struct cpufreq_driver *driver);
extern int cpufreq_generic_freqtable_verify (struct cpufreq_policy *policy);
extern unsigned int cpufreq_generic_get(void);
extern int cpufreq_generic_init(struct cpufreq_policy *policy, unsigned int *table,
        unsigned int transition_latency);
extern int cpufreq_driver_target(struct cpufreq_policy *policy, unsigned int target_freq);
extern void cpufreq_suspend(void);
extern void cpufreq_resume(void);

extern int cpufreq_register_notifier(struct notifier_block *nb);
extern int cpufreq_unregister_notifier(struct notifier_block *nb);

extern struct cpufreq_policy *cpufreq_get_policy(void);
extern int cpufreq_freqtable_target(struct cpufreq_policy *policy,
                           unsigned int target_freq);

static inline void
cpufreq_verify_limits(struct cpufreq_policy *policy)
{
    if (policy->min < policy->cpuinfo.min_freq)
        policy->min = policy->cpuinfo.min_freq;
    if (policy->max < policy->cpuinfo.min_freq)
        policy->max = policy->cpuinfo.min_freq;
    if (policy->min > policy->cpuinfo.max_freq)
        policy->min = policy->cpuinfo.max_freq;
    if (policy->max > policy->cpuinfo.max_freq)
        policy->max = policy->cpuinfo.max_freq;
    if (policy->min > policy->max)
        policy->min = policy->max;

    return;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _CPUFREQ_H */

