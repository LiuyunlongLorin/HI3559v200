/******************************************************************************
 Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : gdc_drv_comm.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/9/17
Last Modified :
Description   :
Function List :
******************************************************************************/


#ifndef __PM_DRV_COMM_H__
#define __PM_DRV_COMM_H__
#include "hi_osal.h"


/* module debug on/off  */

//#define __PM_DEBUG_ON__


#ifdef __PM_DEBUG_ON__

//#define PM_DEBUG(fmt,...) osal_printk( "[Func]:%s [Line]:%d [Info]:"fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)

#define PM_DEBUG(fmt,...)  osal_printk( "[Info]:"fmt,##__VA_ARGS__)

#else

#define PM_DEBUG(format,...)

#endif


typedef enum {
    HI_CPU_AVS_POLICY_CLOSE = 0,         /**<avs not used */
    HI_CPU_AVS_POLICY_HPM,                  /**<use the hpm table*/
    HI_CPU_AVS_POLICY_ONLY_TEMP,      /**only temperature used */
    HI_CPU_AVS_POLICY_BUTT

} HI_CPU_AVS_POLICY_E;


typedef enum {
    HI_CPU_DVFS_POLICY_CLOSE = 0,          /**<DVFS  not used */
    HI_CPU_DVFS_POLICY_OPEN,                  /**<dvfs used */
    HI_CPU_DVFS_POLICY_BUTT

} HI_CPU_DVFS_POLICY_E;


typedef enum {
    HI_MEDIA_AVS_POLICY_CLOSE = 0,        /**<avs not used */
    HI_MEDIA_AVS_POLICY_HPM,                  /**<use the hpm table */
    HI_MEDIA_AVS_POLICY_ONLY_TEMP,      /**only temperature used */
    HI_MEDIA_AVS_POLICY_BUTT

} HI_MEDIA_AVS_POLICY_E;


typedef enum {
    HI_CORE_AVS_POLICY_CLOSE = 0,        /**<avs not used */
    HI_CORE_AVS_POLICY_HPM,                  /**<use the hpm table */
    HI_CORE_AVS_POLICY_ONLY_TEMP,      /**only temperature used */
    HI_CORE_AVS_POLICY_BUTT

} HI_CORE_AVS_POLICY_E;


typedef enum {
    HI_PM_MISC_POLICY_NONE = 0,            /**<No misc policy*/
    HI_PM_MISC_POLICY_SVPCTRL = 1,        /**<ctrl svp enable or disable */
    HI_PM_MISC_POLICY_BUTT

} HI_PM_MISC_POLICY_E;


/* policy get interface */

int hal_cpu_get_avs_policy(void);

int hal_media_get_avs_policy(void);

int hal_core_get_avs_policy(void);

int hal_cpu_get_dvfs_policy(void);

/*tsensor  interface */

void pm_hal_tsensor_init(void);

int pm_hal_get_temperature(void);


#endif /* End of #ifndef __PM_DRV_COMM_H__ */



