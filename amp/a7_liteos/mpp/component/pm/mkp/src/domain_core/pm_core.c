#include "pm_core.h"
#include "pm_core_regulator.h"
#include "pm_core_avs.h"


struct pm_domain_device core_domain_dev = {
    .domain_regulator_init  = core_domain_regulator_init,
    .domain_regulator_exit  = core_domain_regulator_exit,
    .domain_avs_init        = core_domain_avs_init,
    .domain_avs_exit        = core_domain_avs_exit,
    .domain_freq_init    = NULL,
    .domain_freq_exit    = NULL,
    .domain_freeze          = NULL,
    .domain_restore         = NULL,
};


