#ifndef  __PM_DEVICE_API_H__
#define  __PM_DEVICE_API_H__

#include "pm_device.h"

/*
 *  dvfs_ctrl:cpu domain dvfs ctrl interface
 *  parameter:0:disable dvfs;  others:enable dvfs
 *  return:-1:fail;   0:ok
 */
int dvfs_ctrl(int enable);

/*
 *  svp_ctrl:samrt version proc ctrl interface
 *  parameter:0:disable svp;  others:enable svp
 *  return:-1:fail;   0:ok
 */
int svp_ctrl(int enable);

/*
 *  pm_ctrl:power manager ctrl interface
 *  parameter:0:disable pm;  others:enable pm
 *  return:-1:fail;   0:ok
 */
int pm_ctrl(int enable);

/*
 * get_cpu_max_profile: get cpu domain max profile index
 * return:max profile index value
 */
int get_cpu_max_profile(void);

/*
 * get_cpu_profile: get cpu domain profile index
 * return:profile index value
 */
int get_cpu_profile(void);

/*
 * set_cpu_profile: set cpu domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_cpu_profile(int profile);

/*
 * get_media_max_profile: get media domain max profile index
 * return:max profile index value
 */
int get_media_max_profile(void);

/*
 * get_media_profile: get media domain profile index
 * return:profile index value
 */
int get_media_profile(void);

/*
 * set_media_profile: set media domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_media_profile(int profile);

/*
 * set_media_voltage: set media domain voltage
 * parameter:set_volt(uV)
 * return:0:ok;   -1:fail
 */
int set_media_voltage(int set_volt);

/*
 * get_media_usrcfg: get usr media config info
 * parameter:pmedia_usrcfg
 * return:0:ok;   -1:fail
 */
int get_media_usrcfg(HI_MPI_PM_MEDIA_CFG_S* pmedia_usrcfg);

/*
 * SetMediaProfileByUsrCfg: set usr media config info
 * parameter:pmedia_usrcfg
 * return:0:ok;   -1:fail
 */
HI_S32 SetMediaProfileByUsrCfg(HI_MPI_PM_MEDIA_CFG_S* pstUsrParam);

/*
 * SetSingleFreqByUsrCfg: set usr media config info
 * parameter:pmedia_usrcfg
 * return:0:ok;   -1:fail
 */
HI_S32 SetSingleFreqByUsrCfg(HI_MPI_PM_SIGLE_MEDIA_CFG_S* pSingleModParam);

/*
 * pm_dev_init:init pm
 */
//void pm_dev_deinit(void);
void pm_dev_init(bool bsvp, bool bavsp);

/*
 * pm_dev_deinit:deinit pm
 */
void pm_dev_deinit(void);

/*
 * get_average_temperature: pm
 */

int get_average_temperature(void);


#endif/* End of #ifndef __PM_DEVICE_H__*/

