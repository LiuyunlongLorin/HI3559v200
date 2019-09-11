#ifndef  __PM_CORE_REGULATOR_H__
#define  __PM_CORE_REGULATOR_H__

#include "pm_device.h"

int core_domain_regulator_init(struct pm_domain_device* pm_domain_dev);

int core_domain_regulator_exit(struct pm_domain_device* pm_domain_dev);


#endif

