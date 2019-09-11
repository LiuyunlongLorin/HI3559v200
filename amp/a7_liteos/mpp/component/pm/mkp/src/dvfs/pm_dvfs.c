#include "pm_dvfs.h"


int pm_dvfs_init(struct pm_device* pm_device)
{
    if (pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_freq_init) {
        if (0 != pm_device->cpu_domain_device->domain_freq_init(pm_device->cpu_domain_device)) {
            pm_device->cpu_domain_device->domain_avs_init = NULL;//disable avs
        }
    }
    if (pm_device->media_domain_device && pm_device->media_domain_device->domain_freq_init) {
        if (0 != pm_device->media_domain_device->domain_freq_init(pm_device->media_domain_device)) {
            pm_device->media_domain_device->domain_avs_init = NULL;//disable avs
        }
    }
    return 0;
}

void pm_dvfs_exit(struct pm_device* pm_device)
{
    if (pm_device->cpu_domain_device && pm_device->cpu_domain_device->domain_freq_exit) {
        pm_device->cpu_domain_device->domain_freq_exit(pm_device->cpu_domain_device);
    }
    if (pm_device->media_domain_device && pm_device->media_domain_device->domain_freq_exit) {
        pm_device->media_domain_device->domain_freq_exit(pm_device->media_domain_device);
    }
}



