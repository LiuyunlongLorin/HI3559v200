#ifndef  __HAL_CPU_FREQ_H__
#define  __HAL_CPU_FREQ_H__

extern struct pm_profile pm_cpu_profile[6];


void hal_cpu_set_freq_clk(unsigned int freq);
int hal_cpu_get_freq_clk(void);


#endif

