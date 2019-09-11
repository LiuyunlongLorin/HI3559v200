#include "pm_device.h"
#include "pm_cpu.h"
#include "pm_cpu_regulator.h"
#include "hal_cpu_regulator.h"

extern struct regulator_device cpu_regulator_dev;
static int cpu_domain_set_voltage(struct regulator_device* regulator, int set_volt)
{
    hal_cpu_domain_set_voltage(regulator->step_uV, regulator->max_uV, set_volt);
    return 0;
}

static int cpu_domain_get_voltage(struct regulator_device* regulator)
{
    return hal_cpu_domain_get_voltage(regulator->step_uV, regulator->max_uV);
}

static int cpu_domain_get_regulator_step(struct regulator_device* regulator)
{
    return hal_cpu_domain_get_regulator_step(regulator->min_uV, regulator->max_uV);
}

int cpu_domain_regulator_init(struct pm_domain_device* pm_domain_dev)
{
    cpu_regulator_dev.pm_domain_dev = pm_domain_dev;
    cpu_regulator_dev.step_uV = cpu_domain_get_regulator_step(&cpu_regulator_dev);
    cpu_regulator_dev.set_voltage = cpu_domain_set_voltage;
    cpu_regulator_dev.get_voltage = cpu_domain_get_voltage;
    pm_domain_dev->regulator_dev = &cpu_regulator_dev;
    return 0;
}

int cpu_domain_regulator_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0;
}


