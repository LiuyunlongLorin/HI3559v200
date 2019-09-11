#ifndef  __PM_CPU_REGULATOR_H__
#define  __PM_CPU_REGULATOR_H__

#include "pm_device.h"

int cpu_domain_regulator_init(struct pm_domain_device* pm_domain_dev);

int cpu_domain_regulator_exit(struct pm_domain_device* pm_domain_dev);


#endif

