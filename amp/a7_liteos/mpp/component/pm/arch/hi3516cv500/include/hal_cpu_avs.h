#ifndef  __HAL_CPU_AVS_H__
#define  __HAL_CPU_AVS_H__

#define CPU_AVS_VOLT_STEP 10000 /*mv*/
#define CPU_AVS_HPM_DELTA_MIN 1
#define CPU_AVS_HPM_DELTA_MAX 10

extern struct hpm_opp cpu_hpm_opp_table[6];

void hal_cpu_set_hpm_div(unsigned int div);
void hal_cpu_hpm_init(void);
int hal_cpu_get_average_hpm(void);

#endif

