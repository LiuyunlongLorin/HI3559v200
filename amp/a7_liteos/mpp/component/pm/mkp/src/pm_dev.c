#include "asm/delay.h"
#include "hi_osal.h"

#include "pm_regulator.h"
#include "pm_dvfs.h"
#include "pm_avs.h"
#include "pm_proc.h"
#include "pm_device.h"

#ifdef PM_DOMAIN_CPU_ON
#include "pm_cpu.h"
#endif

#ifdef PM_DOMAIN_MEDIA_ON
#include "pm_media.h"
#endif

#ifdef PM_DOMAIN_CORE_ON
#include "pm_core.h"
#endif

#define PM_TIME_INTER  5  /** 5s **/


struct pm_device pm_dev;

static void pm_domain_register(void)
{
    pm_dev.domain_num = 0;
    pm_dev.is_tsensor_init = 0;
    pm_dev.is_init = 0;
#ifdef PM_DOMAIN_CPU_ON
    pm_dev.cpu_domain_device = &cpu_domain_device;
    cpu_domain_device.pm_dev = &pm_dev;
    pm_dev.domain_num++;
#else
    pm_dev.cpu_domain_device = NULL;
#endif
#ifdef PM_DOMAIN_MEDIA_ON
    pm_dev.media_domain_device = &media_domain_dev;
    media_domain_dev.pm_dev = &pm_dev;
    pm_dev.domain_num++;
#else
    pm_dev.media_domain_device = NULL;
#endif
#ifdef PM_DOMAIN_CORE_ON
    pm_dev.core_domain_device = &core_domain_dev;
    core_domain_dev.pm_dev = &pm_dev;
    pm_dev.domain_num++;
#else
    pm_dev.core_domain_device = NULL;
#endif
    PM_DEBUG("cpu_domain_device:   0x%x.\n", (unsigned int)pm_dev.cpu_domain_device);
    PM_DEBUG("media_domain_device:0x%x.\n", (unsigned int)pm_dev.media_domain_device);
    PM_DEBUG("core_domain_device:   0x%x.\n", (unsigned int)pm_dev.core_domain_device);
}

static void pm_handle(unsigned int uwPara)
{
    if (!pm_dev.pm_enable) {
        return;
    }
    osal_mutex_lock(&(pm_dev.lock));
#ifdef PM_DOMAIN_CPU_ON
    if (pm_dev.cpu_domain_device->freq_dev->handle()) {
        osal_mutex_unlock(&(pm_dev.lock));
        return;
    }
    pm_dev.cpu_domain_device->avs_dev->handle();
#endif
#ifdef PM_DOMAIN_MEDIA_ON
    pm_dev.media_domain_device->avs_dev->handle();
#endif
#ifdef PM_DOMAIN_CORE_ON
    pm_dev.core_domain_device->avs_dev->handle();
#endif
    osal_mutex_unlock(&(pm_dev.lock));
    return;
}


static int pm_task_handle(void* pdata)
{
    while ( pm_dev.is_init) {
        pm_handle(0);
        /* interval=5s */
        osal_msleep(PM_TIME_INTER * 1000);
    }
    osal_printk("pm task exit !\n");
    return 0;
}

void pm_timer_init(void)
{
    start_timer(pm_task_handle);
    return ;
}

void pm_tsensor_init(void)
{
    if(0 == pm_dev.is_tsensor_init) {
        pm_hal_tsensor_init();
        pm_dev.is_tsensor_init = 1;
    }
    return ;
}

/*
 * pm_init:init pm
 */
void pm_dev_init( bool bsvp, bool bavsp)
{
    pm_domain_register();
    pm_tsensor_init();
    pm_regulator_init(&pm_dev);
    pm_dvfs_init(&pm_dev);
    pm_avs_init(&pm_dev);
    osal_mutex_init(&(pm_dev.lock));
    pm_dev.pm_enable = 1;
    pm_dev.is_init = 1;
    pm_timer_init();
    PM_DEBUG("pm dev init ok !.\n");
}

/*
 * pm_init:deinit pm
 */
void pm_dev_deinit(void)
{
    if (0 == pm_dev.is_init) {
        return ;
    }
    pm_dev.pm_enable = 0;
    /*stop the time task*/
    pm_dev.is_init = 0;
    osal_mutex_destory(&(pm_dev.lock));
    PM_DEBUG("pm deinit ok!.\n");
}

/*
 * pm_init:get average temperature
 */
int get_average_temperature(void)
{
    return pm_hal_get_temperature();
}



