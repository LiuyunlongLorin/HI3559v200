#ifndef  __HAL_CPU_REGULATOR_H__
#define  __HAL_CPU_REGULATOR_H__

extern struct regulator_device cpu_regulator_dev;

void hal_cpu_domain_set_voltage(int step, int regulator_max, int set_volt);
int hal_cpu_domain_get_voltage(int step, int regulator_max);
int hal_cpu_domain_get_regulator_step(int min_uV, int max_uV);


#endif

