#include "pm_device.h"
#include "pm_media.h"
#include "pm_media_regulator.h"
#include "hal_media_regulator.h"
#include "pm_media.h"

extern struct regulator_device media_regulator_dev;
static int media_domain_set_voltage(struct regulator_device* regulator, int set_volt)
{
    hal_media_domain_set_voltage(regulator->step_uV, regulator->max_uV, set_volt);
    return 0;
}

static int media_domain_get_voltage(struct regulator_device* regulator)
{
    return hal_media_domain_get_voltage(regulator->step_uV, regulator->max_uV);
}

static int media_domain_get_regulator_step(struct regulator_device* regulator)
{
    return hal_media_domain_get_regulator_step(regulator->min_uV, regulator->max_uV);
}

int media_domain_regulator_init(struct pm_domain_device* pm_domain_dev)
{
    media_regulator_dev.step_uV         = media_domain_get_regulator_step(&media_regulator_dev);
    media_regulator_dev.set_voltage     = media_domain_set_voltage;
    media_regulator_dev.get_voltage     = media_domain_get_voltage;
    media_regulator_dev.pm_domain_dev   = pm_domain_dev;
    pm_domain_dev->regulator_dev        = &media_regulator_dev;
    return 0;
}

int media_domain_regulator_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0;
}


