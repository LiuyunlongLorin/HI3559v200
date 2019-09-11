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

#ifndef _DEVFREQ_H
#define _DEVFREQ_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>

#include "linux/device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define TABLE_END   ~1u

struct devfreq_dev_profile {
    unsigned long init_freq;
    unsigned int polling_ms;

    int (*target)(struct device *dev, unsigned long *freq);  /* set */
    int (*get_target_freq)(struct device *dev, unsigned long *freq); /* get */
    void (*exit)(struct device *dev);

    unsigned int *freq_table;
    unsigned int max_state;
};

struct devfreq {
    LOS_DL_LIST list;

    unsigned int lock;
    struct device *dev;
    struct devfreq_dev_profile *profile;
    struct delayed_work work;

    void *data;/* private for user data */

    unsigned long prev_freq;

    unsigned long min_freq;
    unsigned long max_freq;
    bool stop_polling;
};

#define devfreq_for_each_entry(pos, table)  \
    for (pos = table; (unsigned int)(pos->freq) != TABLE_END; pos++)

extern int devfreq_suspend_device(struct device *dev);
extern int devfreq_resume_device(struct device *dev);

extern struct devfreq *devfreq_add_device(struct device *dev,
                   struct devfreq_dev_profile *profile,
                   void *data);
extern int devfreq_remove_device(struct devfreq *fdev);

extern void devfreq_store_load(unsigned int devload);
extern unsigned int devfreq_get_load(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _DEVFREQ_H */

