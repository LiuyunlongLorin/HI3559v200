
#ifndef __HAL_CORE_FREQ_H__
#define __HAL_CORE_FREQ_H__

extern struct pm_profile pm_core_profile[6];

void hal_core_set_freq_clk(unsigned int freq);
int hal_core_get_freq_clk(void);


#endif /*#ifndef __HAL_CORE_FREQ_H__*/


