#ifndef  __HAL_MEDIA_AVS_H__
#define  __HAL_MEDIA_AVS_H__

#include "pm_device.h"

#define MEDIA_AVS_VOLT_STEP 10000 /*mv*/
#define MEDIA_AVS_HPM_DELTA_MIN 1
#define MEDIA_AVS_HPM_DELTA_MAX 6

extern int media_prof_num;
extern int media_temp_num;

extern struct hpm_opp media_hpm_opp_table[4];


void hal_media_set_hpm_div(unsigned int div);
void hal_media_hpm_init(void);
int hal_media_get_average_hpm(void);
int hal_media_get_average_temperature(void);
int hal_media_get_voltage_accord_temp(int s32temp);

#endif/* End of #ifndef __HAL_MEDIA_AVS_H__*/

