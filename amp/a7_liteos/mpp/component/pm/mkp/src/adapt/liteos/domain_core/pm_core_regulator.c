#include "pm_device.h"
#include "pm_core.h"
#include "pm_core_regulator.h"
#include "hal_core_regulator.h"
#include "pm_core.h"

extern struct regulator_device core_regulator_dev;
static int core_domain_set_voltage(struct regulator_device* regulator, int set_volt)
{
    hal_core_domain_set_voltage(regulator->step_uV, regulator->max_uV, set_volt);
    return 0;
}

static int core_domain_get_voltage(struct regulator_device* regulator)
{
    return hal_core_domain_get_voltage(regulator->step_uV, regulator->max_uV);
}

static int core_domain_get_regulator_step(struct regulator_device* regulator)
{
    return hal_core_domain_get_regulator_step(regulator->min_uV, regulator->max_uV);
}

int core_domain_regulator_init(struct pm_domain_device* pm_domain_dev)
{
    core_regulator_dev.step_uV         = core_domain_get_regulator_step(&core_regulator_dev);
    core_regulator_dev.set_voltage     = core_domain_set_voltage;
    core_regulator_dev.get_voltage     = core_domain_get_voltage;
    core_regulator_dev.pm_domain_dev   = pm_domain_dev;
    pm_domain_dev->regulator_dev        = &core_regulator_dev;
    return 0;
}

int core_domain_regulator_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0;
}


