#ifndef  __HAL_CORE_REGULATOR_H__
#define  __HAL_CORE_REGULATOR_H__

extern struct regulator_device core_regulator_dev;

void hal_core_domain_set_voltage(int step, int regulator_max, int set_volt);
int hal_core_domain_get_regulator_step(int min_uV, int max_uV);
int hal_core_domain_get_voltage(int step, int regulator_max);

#endif

