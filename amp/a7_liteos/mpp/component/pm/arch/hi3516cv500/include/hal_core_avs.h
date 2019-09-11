#ifndef  __HAL_CORE_AVS_H__
#define  __HAL_CORE_AVS_H__

#include "pm_device.h"

#define CORE_AVS_VOLT_STEP 10000 /*mv*/
#define CORE_AVS_HPM_DELTA_MIN 1
#define CORE_AVS_HPM_DELTA_MAX 6

extern int core_prof_num;
extern int core_temp_num;

extern struct hpm_opp core_hpm_opp_table[4];


void hal_core_set_hpm_div(unsigned int div);
void hal_core_hpm_init(void);
int hal_core_get_average_hpm(void);
int hal_core_get_average_temperature(void);

#endif/* End of #ifndef __HAL_CORE_AVS_H__*/

