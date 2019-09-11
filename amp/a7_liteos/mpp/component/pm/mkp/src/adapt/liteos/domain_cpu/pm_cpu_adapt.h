
#ifndef  __HI_PM_CPU_ADAPT_H__
#define  __HI_PM_CPU_ADAPT_H__


#include "pm_cpu_regulator.h"

//extern struct pm_domain_device cpu_domain_device;

int cpu_domain_cpufreq_init(struct pm_domain_device* pm_device);

int cpu_domain_cpufreq_exit(struct pm_domain_device* pm_device);


void cpu_domain_freeze(struct pm_domain_device* pm_domain_device);

void cpu_domain_restore(struct pm_domain_device* pm_domain_device);


#endif

