#ifndef  __PM_REGULATOR_H__
#define  __PM_REGULATOR_H__

#include "pm_device.h"


void pm_regulator_init(struct pm_device* pm_dev);
void pm_regulator_exit(struct pm_device* pm_dev);

#endif

