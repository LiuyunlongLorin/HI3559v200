#include "pm_cpu_regulator.h"
#include "pm_cpu_freq.h"
#include "pm_cpu_avs.h"


int cpu_domain_cpufreq_init(struct pm_domain_device* pm_device)
{
    return cpu_domain_freq_init(pm_device);
}

int cpu_domain_cpufreq_exit(struct pm_domain_device* pm_device)
{
    return cpu_domain_freq_exit(pm_device);
}

void cpu_domain_freeze(struct pm_domain_device* pm_domain_device)
{
    /*if needed,this function can be reliazed at  pm_cpu.c where avs intf can be used */
    return ;
}

void cpu_domain_restore(struct pm_domain_device* pm_domain_device)
{
    /*if needed,this function can be reliazed at  pm_cpu.c where avs intf can be used */
    return ;
}



