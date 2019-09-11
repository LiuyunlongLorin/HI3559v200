#include "pm_cpu_regulator.h"
#include "pm_cpu_freq.h"
#include "pm_cpu_avs.h"
#include "pm_cpu_adapt.h"

struct pm_domain_device cpu_domain_device = {
    .domain_regulator_init  = cpu_domain_regulator_init,
    .domain_regulator_exit  = cpu_domain_regulator_exit,
    .domain_avs_init        = cpu_domain_avs_init,
    .domain_avs_exit        = cpu_domain_avs_exit,
    .domain_freq_init    = cpu_domain_cpufreq_init,
    .domain_freq_exit    = cpu_domain_cpufreq_exit,
    .domain_freeze          = cpu_domain_freeze,
    .domain_restore         = cpu_domain_restore,
};


