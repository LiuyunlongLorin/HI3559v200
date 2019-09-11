#ifndef  __PM_DEVICE_ADAPT_H__
#define  __PM_DEVICE_ADAPT_H__

#include "los_typedef.h"
#include "hi_comm_pm.h"
#include "pm_drv_comm.h"


struct pm_profile;
struct freq_device;
struct regulator_device;
struct pm_domain_device;


typedef int  domain_regulator_init(struct pm_domain_device*);
typedef int  domain_regulator_exit(struct pm_domain_device*);

struct pm_profile {
    int freq;   /* unit:Hz */
    int volt;   /* unit:uV */
    struct freq_device* freq_dev;
};

struct freq_device {
    struct pm_domain_device* pm_domain_dev;
    struct pm_profile* pm_profile_table;

    int profile_num;
    int cur_profile;
    int cur_freq;

    int max_freq;
    int min_freq;

    int (*get_freq_clk)(void);
    int (*set_freq_clk)(int freq);
    int (*handle)(void);

    osal_mutex_t lock;
    int dvfs_enable;
};

struct regulator_device {
    struct pm_domain_device* pm_domain_dev;

    int step_uV;
    int min_uV;
    int max_uV;

    int (*set_voltage) (struct regulator_device* regulator_dev, int set_volt);
    int (*get_voltage) (struct regulator_device* regulator_dev);
};


#endif/* End of #ifndef __PM_DEVICE_H__*/

