#include "pm_device.h"
#include "pm_cpu.h"
#include "pm_cpu_avs.h"

#include "hal_cpu_avs.h"
#include "pm_chip_hal.h"

struct avs_device cpu_avs_dev;

static void cpu_update_cur_avs_info(void)
{
    int i;
    struct avs_device* avs_dev = &cpu_avs_dev;
    struct pm_domain_device* pm_domain_dev = avs_dev->pm_domain_dev;
    avs_dev->cur_freq = pm_domain_dev->cur_freq;
    /* get default hpm record */
    for (i = 0; i < avs_dev->hpm_opp_num; i++) {
        if (avs_dev->cur_freq == avs_dev->hpm_opp_table[i].freq) {
            avs_dev->cur_hpm      = avs_dev->hpm_opp_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->hpm_opp_table[i].vmin;
            avs_dev->cur_volt_max = avs_dev->hpm_opp_table[i].vmax;
            avs_dev->div          = avs_dev->hpm_opp_table[i].div;
            /* cpu HPM freq */
            hal_cpu_set_hpm_div(avs_dev->div);
            break;
        }
    }
}

static void cpu_hpm_init(void)
{
    hal_cpu_hpm_init();
}

static int cpu_get_average_hpm(void)
{
    return hal_cpu_get_average_hpm();
}

static void cpu_set_volt_accord_hpm(int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    struct pm_domain_device* pm_domain_dev = cpu_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    //int flag=0;
    if (hpm_delta < CPU_AVS_HPM_DELTA_MIN) {
        /* up 10mv */
        if (pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP <= volt_max) {
            //flag = 1;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP);
            pm_domain_dev->cur_volt = pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP;
        }
    } else if (hpm_delta >= CPU_AVS_HPM_DELTA_MAX) {
        /*down 10mv */
        if (pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP >= volt_min) {
            //flag = 1;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP);
            pm_domain_dev->cur_volt = pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP;
        }
    }
    //if (flag) {
    //    dprintf("%s(%0d):hpm_delta=%0d,volt_min=%0d,volt_max=%0d,cur_freq=%0d,cur_volt=%0d\n",__func__,__LINE__,hpm_delta,volt_min,volt_max,pm_domain_dev->cur_freq,pm_domain_dev->cur_volt);
    //}
}

extern  int media_get_average_temperature(void);
static void cpu_set_volt_accord_temp(void)
{
    int  temp = 0, diff_volt_uv = 0 ;
    struct avs_device* avs_dev = &cpu_avs_dev;
    struct pm_domain_device* pm_domain_dev = cpu_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    temp = media_get_average_temperature();
    PM_DEBUG(" read temp in cpu domain..:%d. \n", temp);
    if ( temp <= -20) {
        diff_volt_uv = 0;
    } else if ( (-20 < temp ) && (temp <= 50 )) {
        diff_volt_uv = (20 * 1000 / 70) * (temp + 20);  /* 20*1000 =20mv,  70 = (50 - (-20))*/
    } else if  ( temp > 50) {
        diff_volt_uv = 20 * 1000;
    }
    pm_domain_dev->cur_volt = avs_dev->poweron_volt - diff_volt_uv; /*step down*/
    PM_DEBUG("  ==============set CPU volt ====>  [poweron_volt :%d uv], [diff_volt:%d uv]   [cur_volt:%d uv]  .=========== \n", avs_dev->poweron_volt, diff_volt_uv, pm_domain_dev->cur_volt);
    regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
}


int cpu_domain_get_poweron_volt(void)
{
    struct avs_device* avs_dev = &cpu_avs_dev;
    struct pm_domain_device* pm_domain_dev = cpu_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    avs_dev->poweron_volt = regulator->get_voltage(regulator);
    PM_DEBUG("==================   cpu power on voltage =%d=======================\n", avs_dev->poweron_volt);
    return 0;
}


/*  for debug */

#ifdef __PM_DEBUG_ON__
static int cpu_domain_get_cur_volt(void)
{
    struct avs_device* avs_dev = &cpu_avs_dev;
    struct pm_domain_device* pm_domain_dev = cpu_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    int value = 0;
    value = regulator->get_voltage(regulator);
    PM_DEBUG("================>  get  cpu current voltage =%d\n", value);
    return 0;
}
#endif

int cpu_domain_avs_handle(void)
{
    HI_CPU_AVS_POLICY_E cpuAvsPolicy = (HI_CPU_AVS_POLICY_E)hal_cpu_get_avs_policy();
    if( HI_CPU_AVS_POLICY_CLOSE  == cpuAvsPolicy || HI_CPU_AVS_POLICY_BUTT == cpuAvsPolicy ) {
        return 0;
    } else {
        PM_DEBUG(" Cpu Avs Policy:  %d\n", cpuAvsPolicy);
    }
    if ( HI_CPU_AVS_POLICY_HPM == cpuAvsPolicy  ) {
        int cur_average_hpm = 0, delta = 0;
        struct avs_device* avs_dev = &cpu_avs_dev;
        cpu_update_cur_avs_info();
        cur_average_hpm = cpu_get_average_hpm();
        delta = cur_average_hpm - avs_dev->cur_hpm;
        cpu_set_volt_accord_hpm(delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);
    } else if( HI_CPU_AVS_POLICY_ONLY_TEMP == cpuAvsPolicy ) {
#if 1
        cpu_set_volt_accord_temp();
#else
        cpu_domain_get_cur_volt(); //for debug
#endif
    }
    return 0;
}

int cpu_domain_avs_init(struct pm_domain_device* pm_domain_dev)
{
    HI_CPU_AVS_POLICY_E cpuAvsPolicy = (HI_CPU_AVS_POLICY_E)hal_cpu_get_avs_policy();
    /*
       if( HI_CPU_AVS_POLICY_CLOSE  ==cpuAvsPolicy || HI_CPU_AVS_POLICY_BUTT == cpuAvsPolicy )
           return 0;
    */
    cpu_avs_dev.hpm_opp_table       = cpu_hpm_opp_table;
    cpu_avs_dev.hpm_opp_num         = ARRAY_SIZE(cpu_hpm_opp_table);
    cpu_avs_dev.handle              = cpu_domain_avs_handle;
    cpu_avs_dev.pm_domain_dev       = pm_domain_dev;
    pm_domain_dev->avs_dev          = &cpu_avs_dev;
    if ( HI_CPU_AVS_POLICY_HPM == cpuAvsPolicy  ) {
        cpu_hpm_init();
    } else if( HI_CPU_AVS_POLICY_ONLY_TEMP == cpuAvsPolicy  ) {
        cpu_domain_get_poweron_volt();
    }
    return 0;
}

int cpu_domain_avs_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0 ;
}



