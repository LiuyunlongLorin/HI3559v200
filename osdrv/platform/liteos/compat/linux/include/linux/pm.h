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
#ifndef _PM_H
#define _PM_H

#include "linux/list.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct pm_message {
        int event;
} dev_pm_message_t;

struct dev_pm_info{
    dev_pm_message_t    power_state;
    LOS_DL_LIST    entry;
};

#define PM_EVENT_INVALID        (-1)
#define PM_EVENT_ON             0x0000
#define PM_EVENT_FREEZE         0x0001
#define PM_EVENT_SUSPEND        0x0002
#define PM_EVENT_HIBERNATE      0x0004
#define PM_EVENT_QUIESCE        0x0008
#define PM_EVENT_RESUME         0x0010

#define PMSG_INVALID    ((struct pm_message){ .event = PM_EVENT_INVALID, })
#define PMSG_ON         ((struct pm_message){ .event = PM_EVENT_ON, })
#define PMSG_FREEZE     ((struct pm_message){ .event = PM_EVENT_FREEZE, })
#define PMSG_QUIESCE    ((struct pm_message){ .event = PM_EVENT_QUIESCE, })
#define PMSG_SUSPEND    ((struct pm_message){ .event = PM_EVENT_SUSPEND, })
#define PMSG_HIBERNATE  ((struct pm_message){ .event = PM_EVENT_HIBERNATE, })
#define PMSG_RESUME     ((struct pm_message){ .event = PM_EVENT_RESUME, })

int dpm_suspend_start(dev_pm_message_t state);
void dpm_resume_end(dev_pm_message_t state);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

