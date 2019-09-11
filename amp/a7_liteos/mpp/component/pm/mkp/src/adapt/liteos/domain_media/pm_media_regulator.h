#ifndef  __PM_MEDIA_REGULATOR_H__
#define  __PM_MEDIA_REGULATOR_H__

#include "pm_device.h"

int media_domain_regulator_init(struct pm_domain_device* pm_domain_dev);

int media_domain_regulator_exit(struct pm_domain_device* pm_domain_dev);


#endif

