#ifndef  __PM_DVFS_H__
#define  __PM_DVFS_H__

#include "pm_device.h"

int pm_dvfs_init(struct pm_device* pm_device);
void pm_dvfs_exit(struct pm_device* pm_device);
#endif

