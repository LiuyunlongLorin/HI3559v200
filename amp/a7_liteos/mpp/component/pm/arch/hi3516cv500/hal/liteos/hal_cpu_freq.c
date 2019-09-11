#include "asm/delay.h"

#include "pm_device.h"
#include "pm_chip_hal.h"

struct pm_profile pm_cpu_profile[6] = {
    {
        .freq = 594 * _1M,
        .volt = 940000,
    },

};

/*1: support,  0: not support */
int hal_cpu_get_dvfs_policy(void)
{
    return  (int)HI_CPU_DVFS_POLICY_CLOSE;
}

void hal_cpu_set_freq_clk(unsigned int freq)
{
    freq = freq / _1M;
    return;
}

int hal_cpu_get_freq_clk(void)
{
    int freq;
    //----- hi3556av100 'reg config is to be done..----------
    freq = 24 * _1M;
    //-----the following is hi3556v100 'reg config ----------
    //   dprintf("get  hi3556av100 's cpu freq ok !.\n");
    return freq;
}

