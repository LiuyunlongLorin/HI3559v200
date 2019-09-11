#include "asm/io.h"
#include "asm/delay.h"
#include "pm_device.h"
#include "hal_media_avs.h"
#include "pm_chip_hal.h"

#define MEDIA_PROF_NUM 2
#define MEDIA_TEMP_NUM 2

#define MEDIA_SVB_NUM 4

#define HI3559_V200_CHIP                    0x35590200
#define HI3556_V200_CHIP                    0x35560200

#define REG_CHIP_ID_ADDRESS 0x12020ee0

#define READ_REG(reg_addr, pvalue) do {    \
        *(pvalue) = readl(reg_addr);    \
       } while (0)

int media_prof_num = MEDIA_PROF_NUM;
int media_temp_num = MEDIA_TEMP_NUM;

struct hpm_opp media_hpm_opp_table[MEDIA_PROF_NUM * MEDIA_TEMP_NUM] = {
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

struct hpm_opp media_svb_table[MEDIA_SVB_NUM][MEDIA_PROF_NUM * MEDIA_TEMP_NUM] = {
    /* <=195 */
    {   {.vmax = 950000,}, //profile0 low temp
        {.vmax  = 890000,}, //profile0 high temp
        {.vmax  = 950000,}, //profile1 low temp
        {.vmax  = 910000,}
    },//profile1 high temp
    /* <=220*/
    {   {.vmax = 920000,}, //profile0 low temp
        {.vmax  = 860000,}, //profile0 high temp
        {.vmax  = 920000,}, //profile1 low temp
        {.vmax  = 880000,}
    },//profile1 high temp
    /* <=245 */
    {   {.vmax = 870000,}, //profile0 low temp
        {.vmax  = 820000,}, //profile0 high temp
        {.vmax  = 870000,}, //profile1 low temp
        {.vmax  = 840000,}
    },//profile1 high temp
    /* others */
    {   {.vmax = 840000,}, //profile0 low temp
        {.vmax  = 800000,}, //profile0 high temp
        {.vmax  = 840000,}, //profile1 low temp
        {.vmax  = 820000,}
    },//profile1 high temp
};


int hal_media_get_avs_policy(void)
{
    return  (int)HI_MEDIA_AVS_POLICY_CLOSE;
}


void hal_media_set_hpm_div(unsigned int div)
{
    return;
}

void hal_media_update_vmax(unsigned int hpm_mda_value)
{
    int index;
    int i;
    if (hpm_mda_value <= 195) {
        index = 0;
    } else if (hpm_mda_value <= 220) {
        index = 1;
    } else if (hpm_mda_value <= 245) {
        index = 2;
    } else {
        index = 3;
    }
    //dprintf("index = %0d\n",index);
    for (i = 0; i < MEDIA_PROF_NUM * MEDIA_TEMP_NUM; i++) {
        media_hpm_opp_table[i].vmax = media_svb_table[index][i].vmax;
    }
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

#define OTP_MDA_HPM_IDDQ    0x12032004
#define HI_PMC_CTL_REG      0x120a0000
#define HPM_MDA_REG0    0x5c
#define HPM_MDA_REG1    0x60
#define HPM_MDA_VOL_REG     (HI_PMC_CTL_REG + 0xc)
static unsigned int hal_get_hpm_value(void)
{
    return 0;
}
#endif

void hal_media_hpm_init(void)
{
    /*set start voltage, CORE/MDA0 is 0.9v*/
    /* update vmax */
    /* media HPM reset */
    /* media HPM limit*/
    /* set circularly check */
    /* enable */
    return ;
}


int hal_media_get_average_hpm(void)
{
    return 0;
}

int hal_media_get_average_temperature(void)
{
    return pm_hal_get_temperature();
}

int  hal_media_get_voltage_accord_temp(int s32temp)
{
    int s32volt_uv=0;
    unsigned int u32Chip_ID = 0;
    READ_REG(REG_CHIP_ID_ADDRESS,&u32Chip_ID);

    if(u32Chip_ID == HI3559_V200_CHIP) {
      if ( s32temp <= -20) {
          s32volt_uv = 0;
      } else if ( -20 < s32temp && s32temp <= 70 ) {    /**Linear adjustment  5mv **/
          s32volt_uv = (5 * 1000 / 90) * (s32temp + 20);
      } else if  (100 >= s32temp && s32temp > 70) {     /**Linear adjustment 10mv **/
          s32volt_uv = (10 * 1000 / 30) * (s32temp - 70) + 5* 1000;
      } else if  (s32temp > 100) {
          s32volt_uv = 15 * 1000;    /**adjustment 15mv **/
      }
    }
    else if (u32Chip_ID == HI3556_V200_CHIP) {
        if ( s32temp <= -20) {
            s32volt_uv = 0;
        } else if ( -20 < s32temp && s32temp <= 70 ) {
            s32volt_uv = (10 * 1000 / 90) * (s32temp + 20);  /**Linear adjustment 10mv **/
        } else if  (100 >= s32temp && s32temp > 70) {
            s32volt_uv = (10 * 1000 / 30) * (s32temp - 70) + 10* 1000; /**Linear adjustment 10mv **/
        } else if  (s32temp > 100) {
            s32volt_uv = 20 * 1000;     /**adjustment 20mv **/
        }
    }
    else {
        s32volt_uv = -1;
        PM_DEBUG("==WARNING: Check the Chip[0x%08x] PM Strategy, voltage Seting by temp ?==\n",u32Chip_ID);
    }
    return s32volt_uv;
}
