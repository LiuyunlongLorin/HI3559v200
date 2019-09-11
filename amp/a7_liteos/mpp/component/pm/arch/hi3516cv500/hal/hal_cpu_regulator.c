#include "linux/kernel.h"
#include "asm/delay.h"

#include "pm_device.h"
#include "pm_chip_hal.h"
#include "pm_drv_comm.h"

#ifndef PM_REGULATOR_TYPE_DCDC
#include "hal_regulator_i2c.h"
#endif

struct regulator_device cpu_regulator_dev;

void hal_cpu_domain_set_voltage(int step, int regulator_max, int set_volt)
{
#ifdef PM_REGULATOR_TYPE_DCDC
    /*--pwm way---*/
    pm_cpu_pwm_set_voltage(set_volt);
    PM_DEBUG("   set cpu domain voltage [ %d uv]  ok !\n", set_volt);
#else
    /*--i2c way---*/
    pm_cpu_pmu_set_voltage(step, regulator_max, set_volt);
#endif
    return ;
}

int hal_cpu_domain_get_voltage(int step, int regulator_max)
{
#ifdef PM_REGULATOR_TYPE_DCDC
    /*--pwm way---*/
    PM_DEBUG(" get cpu domain voltage  ok !\n");
    return   pm_cpu_pwm_get_voltage();
#else
    /*--i2c way---*/
    return pm_cpu_pmu_get_voltage(step, regulator_max);
#endif
}

int hal_cpu_domain_get_regulator_step(int min_uV, int max_uV)
{
#ifdef PM_REGULATOR_TYPE_DCDC
    /*--pwm way---*/
    //printf("get cpu domain voltage step ok !\n");
    return 100;
#else
    /*--i2c way---*/
    // pmu_get_regulator_step(min_uV,max_uV);
#endif
}

