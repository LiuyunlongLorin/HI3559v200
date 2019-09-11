#include "asm/io.h"
#include "asm/delay.h"
#include "pm_device.h"
#include "hal_core_avs.h"
#include "pm_chip_hal.h"

#define CORE_PROF_NUM 2
#define CORE_TEMP_NUM 2

#define CORE_SVB_NUM 4

int core_prof_num = CORE_PROF_NUM;
int core_temp_num = CORE_TEMP_NUM;

struct hpm_opp core_hpm_opp_table[CORE_PROF_NUM * CORE_TEMP_NUM] = {
    {
        .freq = 0,
        .vmin = 770000,
        .vmax = 950000,
        .hpmrecord = 220,
        .div = 3,
        .temp = 50,
        .profile = 0,
    },
};

struct hpm_opp core_svb_table[CORE_SVB_NUM][CORE_PROF_NUM * CORE_TEMP_NUM] = {
    /* <=195 */
    {   {.vmax = 950000,}, //profile0 low temp
        {.vmax  = 890000,}, //profile0 high temp
        {.vmax  = 950000,}, //profile1 low temp
        {.vmax  = 910000,}
    },//profile1 high temp
};


int hal_core_get_avs_policy(void)
{
    return  (int)HI_CORE_AVS_POLICY_ONLY_TEMP;
}


void hal_core_set_hpm_div(unsigned int div)
{
    return;
}

void hal_core_update_vmax(unsigned int hpm_mda_value)
{
    return ;
}

#if 0
static unsigned hpm_value_avg(unsigned int* val)
{
    unsigned int i;
    unsigned tmp = 0;
    for (i = 0; i < 4; i++) {
        //dprintf("val[%0d]=%0d\n",i,val[i]);
        tmp += val[i] >> 2;
    }
    return tmp >> 2;
}
#endif

#if 0

static unsigned int hal_get_hpm_value(void)
{
    return 0;
}
#endif

void hal_core_hpm_init(void)
{
    /*set start voltage, CORE/MDA0 is 0.9v*/
    /* update vmax */
    /* core HPM reset */
    /* core HPM limit*/
    /* set circularly check */
    /* enable */
    return ;
}

int hal_core_get_average_hpm(void)
{
    return 0;
}

int hal_core_get_average_temperature(void)
{
    return pm_hal_get_temperature();
}


