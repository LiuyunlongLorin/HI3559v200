#include "los_cpup.h"
#include "los_mux.h"

#include "pm_device.h"
#include "pm_cpu_freq.h"
#include "hal_cpu_freq.h"
#include "pm_media_avs.h"

struct freq_device cpu_freq_dev;
extern struct pm_profile pm_cpu_profile[6];

static unsigned int get_cpu_load(void)
{
    /* return cpu usage in last 1s */
    return LOS_HistorySysCpuUsage(1) / LOS_CPUP_PRECISION_MULT;
}

static int cpu_set_freq_clk(int freq)
{
    hal_cpu_set_freq_clk(freq);
    return 0;
}

static int cpu_get_freq_clk(void)
{
    return hal_cpu_get_freq_clk();
}

static int cpu_get_dvfs_support(void)
{
    return hal_cpu_get_dvfs_policy();
}

void cpu_domain_setprofile(int cur_profile)
{
    int freq, volt;
    struct regulator_device* regulator;
    struct pm_domain_device* pm_domain_dev = cpu_freq_dev.pm_domain_dev;
    if (cur_profile < 0 || cur_profile > cpu_freq_dev.profile_num - 1) {
        dprintf("ERROR: set_profile=%0d must be:[0~%0d]\n", cur_profile, cpu_freq_dev.profile_num - 1);
        return;
    }
    if (cur_profile == cpu_freq_dev.cur_profile) {
        return;
    }
    freq = pm_cpu_profile[cur_profile].freq;
    volt = pm_cpu_profile[cur_profile].volt;
    //dprintf("%s(%0d):freq=%0dHz,volt=%0duV\n",__func__,__LINE__,freq,volt);
    regulator = cpu_freq_dev.pm_domain_dev->regulator_dev;
    if (cur_profile > cpu_freq_dev.cur_profile) {
        regulator->set_voltage(regulator, volt);
        (void)cpu_set_freq_clk(freq);
    } else {
        (void)cpu_set_freq_clk(freq);
        regulator->set_voltage(regulator, volt);
    }
    pm_domain_dev->cur_freq = cpu_freq_dev.cur_freq = freq;
    pm_domain_dev->cur_volt = volt;
    cpu_freq_dev.cur_profile = cur_profile;
}

int cpu_domain_getprofile(void)
{
    int i = 0;
    int freq;
    freq = cpu_freq_dev.cur_freq;
    for (i = 0; i < cpu_freq_dev.profile_num; i++) {
        if (freq == pm_cpu_profile[i].freq) {
            break;
        }
    }
    if (i != cpu_freq_dev.profile_num) {
        return i;
    } else {
        //   dprintf("Error:can not find valid profile!\n");
        return 0;
    }
}

static int cpu_freq_handle(void)
{
    int ret = 0;
    int swcpu_load = 0;
    if (!cpu_freq_dev.dvfs_enable) {
        goto out;
    }
    osal_mutex_lock(&(cpu_freq_dev.lock));
    swcpu_load = get_cpu_load();
    if (swcpu_load > 80) {
        if (cpu_freq_dev.cur_profile != cpu_freq_dev.profile_num - 1) {
            cpu_domain_setprofile(cpu_freq_dev.profile_num - 1);
            //dprintf("cpu freq=%0dHz volt=%0duV\n",cpu_freq_dev.pm_domain_dev->cur_freq,cpu_freq_dev.pm_domain_dev->cur_volt);
            ret = 1;
        }
    } else if (swcpu_load < 30) {
        if (cpu_freq_dev.cur_profile > 0) {
            cpu_domain_setprofile(cpu_freq_dev.cur_profile - 1);
            //dprintf("cpu freq=%0dHz volt=%0duV\n",cpu_freq_dev.pm_domain_dev->cur_freq,cpu_freq_dev.pm_domain_dev->cur_volt);
            ret = 1;
        }
    }
    osal_mutex_unlock(&(cpu_freq_dev.lock));
out:
    return ret;
}

int cpu_domain_freq_init(struct pm_domain_device* pm_domain_dev)
{
    cpu_freq_dev.dvfs_enable = cpu_get_dvfs_support();
    cpu_freq_dev.pm_profile_table   = pm_cpu_profile;
    cpu_freq_dev.profile_num        = ARRAY_SIZE(pm_cpu_profile);
    cpu_freq_dev.min_freq           = pm_cpu_profile[0].freq;
    cpu_freq_dev.max_freq           = pm_cpu_profile[cpu_freq_dev.profile_num - 1].freq;
    cpu_freq_dev.pm_domain_dev      = pm_domain_dev;
    cpu_freq_dev.get_freq_clk       = cpu_get_freq_clk;
    cpu_freq_dev.set_freq_clk       = cpu_set_freq_clk;
    cpu_freq_dev.handle             = cpu_freq_handle;
    cpu_freq_dev.cur_freq           = cpu_get_freq_clk();
    cpu_freq_dev.cur_profile        = cpu_domain_getprofile();
    pm_domain_dev->freq_dev         = &cpu_freq_dev;
    osal_mutex_init(&(cpu_freq_dev.lock));
    //dprintf("cpu_freq_dev.profile_num=%0d\n",cpu_freq_dev.profile_num);
    return 0;
}


int cpu_domain_freq_exit(struct pm_domain_device* pm_domain_dev)
{
    osal_mutex_destory(&(cpu_freq_dev.lock));
    return 0;
}


