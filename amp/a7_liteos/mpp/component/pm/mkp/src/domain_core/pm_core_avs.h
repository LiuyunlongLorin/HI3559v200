#ifndef  __PM_CORE_AVS_H__
#define  __PM_CORE_AVS_H__

#include "pm_device.h"

int core_domain_avs_init(struct pm_domain_device* pm_domain_dev);
int core_domain_avs_exit(struct pm_domain_device* pm_domain_dev);
int core_domain_get_poweron_volt(void);
int core_domain_reduce_voltage(void);

int core_domain_getprofile(void);
void core_domain_setprofile(int profile);
int core_domain_setvoltage(int set_volt);

#endif

