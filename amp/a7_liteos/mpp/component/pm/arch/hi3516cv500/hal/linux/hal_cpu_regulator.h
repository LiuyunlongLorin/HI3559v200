#ifndef  __HI_HAL_CPU_REGULATOR_H__
#define  __HI_HAL_CPU_REGULATOR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* Begin of #ifdef __cplusplus */

#define DELAY_TIME_MS   20
#define CPU_REGULATOR_MATCH_NAME    "a73_regulator"
#define CPU_REGULATOR_compatible    "hi3559a,regulators"
#define CPU_REGULATOR_NAME          "regulator-a73"

void hal_cpu_domain_set_voltage(int step, int regulator_max, int set_volt);
int hal_cpu_domain_get_voltage(int step, int regulator_max);
int hal_cpu_domain_get_regulator_step(int min_uV, int max_uV, void *reg);
int hal_cpu_domain_check_domain_valid(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef __HI_HAL_CPU_REGULATOR_H__*/


