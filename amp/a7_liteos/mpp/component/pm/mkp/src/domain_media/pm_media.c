#include "pm_media.h"
#include "pm_media_regulator.h"
#include "pm_media_avs.h"


struct pm_domain_device media_domain_dev = {
    .domain_regulator_init  = media_domain_regulator_init,
    .domain_regulator_exit  = media_domain_regulator_exit,
    .domain_avs_init        = media_domain_avs_init,
    .domain_avs_exit        = media_domain_avs_exit,
    .domain_freq_init    = NULL,
    .domain_freq_exit    = NULL,
    .domain_freeze          = NULL,
    .domain_restore         = NULL,
};


