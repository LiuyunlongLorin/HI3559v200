#ifndef __PM_CHIP_HAL_H__
#define __PM_CHIP_HAL_H__


//============================hi3559v200===============================================
void pm_media_pwm_set_voltage(int set_volt_uv);
int  pm_media_pwm_get_voltage(void);
void pm_core_pwm_set_voltage(int set_volt_uv);
int  pm_core_pwm_get_voltage(void);
void pm_cpu_pwm_set_voltage( int set_volt_uv);
int  pm_cpu_pwm_get_voltage(void);



#endif

