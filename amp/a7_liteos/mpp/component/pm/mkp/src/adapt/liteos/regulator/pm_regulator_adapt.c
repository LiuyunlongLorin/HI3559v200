#include "pm_regulator.h"

int pm_regulator_init_adapt(struct pm_device* pm_dev)

{
#ifndef PM_REGULATOR_TYPE_DCDC
    /*i2c init */
    extern int pm_pmu_i2c_client_init(void);
    pm_pmu_i2c_client_init();
#endif
    if (pm_dev->cpu_domain_device && pm_dev->cpu_domain_device->domain_regulator_init) {
        pm_dev->cpu_domain_device->domain_regulator_init(pm_dev->cpu_domain_device);
    }
    if (pm_dev->media_domain_device && pm_dev->media_domain_device->domain_regulator_init) {
        pm_dev->media_domain_device->domain_regulator_init(pm_dev->media_domain_device);
    }
    if (pm_dev->core_domain_device && pm_dev->core_domain_device->domain_regulator_init) {
        pm_dev->core_domain_device->domain_regulator_init(pm_dev->core_domain_device);
    }
    return 0;
}

void pm_regulator_exit_adapt(struct pm_device* pm_dev)
{
    if (pm_dev->cpu_domain_device && pm_dev->cpu_domain_device->domain_regulator_exit) {
        pm_dev->cpu_domain_device->domain_regulator_exit(pm_dev->cpu_domain_device);
    }
    if (pm_dev->media_domain_device && pm_dev->media_domain_device->domain_regulator_exit) {
        pm_dev->media_domain_device->domain_regulator_exit(pm_dev->media_domain_device);
    }
#ifndef PM_REGULATOR_TYPE_DCDC
    /*i2c deinit */
    extern int pm_pmu_i2c_client_deinit(void);
    pm_pmu_i2c_client_deinit();
#endif
    return ;
}


