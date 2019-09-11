#include "pm_regulator.h"
#include "pm_regulator_adapt.h"


void pm_regulator_init(struct pm_device* pm_dev)
{
    pm_regulator_init_adapt(pm_dev);
    return;
}


void pm_regulator_exit(struct pm_device* pm_dev)
{
    pm_regulator_exit_adapt(pm_dev);
    return ;
}


