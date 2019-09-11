#include "pm_device.h"
#include "pm_chip_hal.h"

struct hpm_opp cpu_hpm_opp_table[6] = {
    {
        .freq = 594 * _1M,
        .vmin = 740000,
        .vmax = 1060000,
        .hpmrecord = 250,
        .div = 11
    },
    {
        .freq = 792 * _1M,
        .vmin = 740000,
        .vmax = 1060000,
        .hpmrecord = 250,
        .div = 15
    },
    {
        .freq = 930 * _1M,
        .vmin = 800000,
        .vmax = 1060000,
        .hpmrecord = 280,
        .div = 18
    },
    {
        .freq = 1000 * _1M,
        .vmin = 800000,
        .vmax = 1060000,
        .hpmrecord = 280,
        .div = 19
    },
    {
        .freq = 1150 * _1M,
        .vmin = 870000,
        .vmax = 1060000,
        .hpmrecord = 310,
        .div = 22
    },
    {
        .freq = 1250 * _1M,
        .vmin = 870000,
        .vmax = 1060000,
        .hpmrecord = 310,
        .div = 24
    },
};

int hal_cpu_get_avs_policy(void)
{
    return  (int)HI_CPU_AVS_POLICY_CLOSE;
}

void hal_cpu_set_hpm_div(unsigned int div)
{
    return ;
}

void hal_cpu_hpm_init(void)
{
    /* cpu HPM reset */
    /* cpu HPM limit*/
    /* set circularly check */
    /* enable */
    return ;
}

int hal_cpu_get_average_hpm(void)
{
    return 0;
}

