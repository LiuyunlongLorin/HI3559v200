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

#ifndef _REGULATOR_H
#define _REGULATOR_H

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


struct regulator_dev;

/*
 * Regulators can either control voltage or current.
 */
enum regulator_type {
    REGULATOR_VOLTAGE,
    REGULATOR_CURRENT,
};


#define REGULATOR_CHANGE_VOLTAGE 0x01
#define REGULATOR_CHANGE_CURRENT 0x02




struct regulation_constraints {
    char *name;
    /* voltage output range - for voltage control */
    int min_uV;
    int max_uV;
    int step_uV;
    /* current output range - for current control */
    int min_uA;
    int max_uA;
    int step_uA;
    /* valid operations for regulator */
    unsigned int valid_ops_mask;
};


struct regulator_ops {
    /* get/set regulator voltage */
    int (*set_voltage) (struct regulator_dev *, int set_uV);
    int (*get_voltage) (struct regulator_dev *);

    /* get/set regulator current  */
    int (*set_current_limit) (struct regulator_dev *, int set_uA);
    int (*get_current_limit) (struct regulator_dev *);

    /* enable/disable regulator */
    int (*enable) (struct regulator_dev *);
    int (*disable) (struct regulator_dev *);
    int (*is_enabled) (struct regulator_dev *);
};



struct regulator_desc {
    const char *name;
    unsigned n_voltages;
    const struct regulator_ops *ops;

    unsigned int uV_step;
    int fixed_uV;
    enum regulator_type type;

};




struct regulator_dev {
    const struct regulator_desc *desc;

    /* lists we belong to */
    LOS_DL_LIST list;/* list of all regulators */

    /* lists we own */
    LOS_DL_LIST consumer_list;/* consumers we supply */

    unsigned int lock;
    struct device dev;
    struct regulation_constraints *constraints;

    void *reg_data;     /* regulator_dev data */
};



struct regulator_config {
    struct device dev;
    struct regulation_constraints constraints;
    int (*regulator_init)(void *driver_data);
    void *driver_data;
};



struct regulator {
    struct device *dev;
    LOS_DL_LIST list;
    int min_uV;
    int max_uV;
    char *supply_name;
    struct regulator_dev *rdev;
};


extern struct regulator_dev *regulator_register(const struct regulator_desc *regulator_desc,
                            const struct regulator_config *cfg);
extern void regulator_unregister(struct regulator_dev *rdev);
extern int regulator_set_voltage(struct regulator *regulator, int min_uV, int max_uV, int set_volt);
extern int regulator_get_voltage(struct regulator *regulator);
extern int regulator_set_current_limit(struct regulator *regulator, int min_uA, int max_uA, int set_cur);
extern int regulator_get_current_limit(struct regulator *regulator);
extern struct regulator *regulator_get(const char *id);
extern void regulator_put(struct regulator *reg);




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _REGULATOR_H */

