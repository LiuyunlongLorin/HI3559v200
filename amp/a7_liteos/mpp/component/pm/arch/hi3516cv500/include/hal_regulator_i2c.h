#ifndef  __HAL_REGULATOR_I2C_H__
#define  __HAL_REGULATOR_I2C_H__


//extern struct hpm_opp cpu_hpm_opp_table[6];
void pm_core_pmu_set_voltage(int step, int regulator_max, int set_volt);
int  pm_core_pmu_get_voltage(int step, int regulator_max);
void pm_media_pmu_set_voltage(int step, int regulator_max, int set_volt);
int  pm_media_pmu_get_voltage(int step, int regulator_max);
void pm_cpu_pmu_set_voltage(int step, int regulator_max, int set_volt);
int  pm_cpu_pmu_get_voltage(int step, int regulator_max);

#endif

