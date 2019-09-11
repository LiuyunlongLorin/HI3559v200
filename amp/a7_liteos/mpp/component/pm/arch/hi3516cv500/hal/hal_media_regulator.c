#include "linux/kernel.h"
#include "asm/delay.h"

#include "pm_device.h"
#include "pm_chip_hal.h"
#include "pm_drv_comm.h"

#ifndef PM_REGULATOR_TYPE_DCDC
#include "hal_regulator_i2c.h"
#endif

struct regulator_device media_regulator_dev;


void hal_media_domain_set_voltage(int step, int regulator_max, int set_volt)
{
#ifdef PM_REGULATOR_TYPE_DCDC
    pm_media_pwm_set_voltage(set_volt);
    PM_DEBUG(" set media domain voltage [ %d uv ] ok !\n", set_volt);
#else
    pm_media_pmu_set_voltage(step, regulator_max, set_volt);
#endif
}

int hal_media_domain_get_voltage(int step, int regulator_max)
{
#ifdef PM_REGULATOR_TYPE_DCDC
    PM_DEBUG("get media domain voltage ok !\n");
    return  pm_media_pwm_get_voltage();
#else
    return pm_media_pmu_get_voltage(step, regulator_max);
#endif
}

int hal_media_domain_get_regulator_step(int min_uV, int max_uV)
{
//    int step, period;
#ifdef PM_REGULATOR_TYPE_DCDC
    //    PM_DEBUG("get media domain voltage  step ok !\n");
    return 100;
#else
    //  pmu_get_regulator_step(min_uV,max_uV);
    return 0;
#endif
}

