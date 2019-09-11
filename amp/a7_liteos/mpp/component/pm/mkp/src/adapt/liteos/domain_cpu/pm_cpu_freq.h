#ifndef  __PM_CPU_FREQ_H__
#define  __PM_CPU_FREQ_H__

#include "pm_device.h"

int cpu_domain_freq_init(struct pm_domain_device* pm_domain_dev);

int cpu_domain_freq_exit(struct pm_domain_device* pm_domain_dev);

void cpu_domain_setprofile(int cur_profile);

int cpu_domain_getprofile(void);

#endif

