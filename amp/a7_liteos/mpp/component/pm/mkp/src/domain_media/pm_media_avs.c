#include "pm_media.h"
#include "pm_media_avs.h"
#include "hal_media_avs.h"
#include "pm_drv_comm.h"

struct avs_device media_avs_dev;

int media_get_average_temperature(void)
{
    return hal_media_get_average_temperature();
}

static void media_update_cur_avs_info(void)
{
    int i;
    int temperature = 0;
    int temperature_index = 0;
    struct avs_device* avs_dev = &media_avs_dev;
    /*Get current temperature and profile*/
    if (avs_dev->temp_num > 1) {
        temperature = media_get_average_temperature();
        for (i = 0; i < avs_dev->temp_num; i++) {
            if (temperature > avs_dev->hpm_opp_table[avs_dev->cur_profile * avs_dev->temp_num + i].temp) {
                temperature_index++;
            } else {
                break;
            }
        }
    }
    avs_dev->cur_freq =  avs_dev->cur_profile * avs_dev->temp_num + temperature_index;
    for (i = 0; i < avs_dev->hpm_opp_num; i++) {
        if (avs_dev->cur_freq  == avs_dev->hpm_opp_table[i].freq) {
            avs_dev->cur_hpm      = avs_dev->hpm_opp_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->hpm_opp_table[i].vmin;
            avs_dev->cur_volt_max = avs_dev->hpm_opp_table[i].vmax;
            avs_dev->div          = avs_dev->hpm_opp_table[i].div;
            /* media HPM freq */
            hal_media_set_hpm_div(avs_dev->div);
            break;
        }
    }
}

static void media_hpm_init(void)
{
    hal_media_hpm_init();
}

static int media_get_average_hpm(void)
{
    return hal_media_get_average_hpm();
}

static void media_set_volt_accord_hpm(int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    if (pm_domain_dev->cur_volt > volt_max) {
        pm_domain_dev->cur_volt -= MEDIA_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    } else if (pm_domain_dev->cur_volt < volt_min) {
        pm_domain_dev->cur_volt += MEDIA_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    } else if (hpm_delta < MEDIA_AVS_HPM_DELTA_MIN) {
        if (pm_domain_dev->cur_volt +  MEDIA_AVS_VOLT_STEP <= volt_max) {
            pm_domain_dev->cur_volt += MEDIA_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    } else if (hpm_delta >= MEDIA_AVS_HPM_DELTA_MAX) {
        if (pm_domain_dev->cur_volt - MEDIA_AVS_VOLT_STEP >= volt_min) {
            pm_domain_dev->cur_volt -= MEDIA_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    }
}

//#ifdef __HI3519AV100__

static void media_set_volt_accord_temp(void)
{
    int  temp = 0, diff_volt_uv = 0 ;
    struct avs_device* avs_dev = &media_avs_dev;
    struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    temp = media_get_average_temperature();
    PM_DEBUG(" read temp in media domain..:%d. \n", temp);
    if ( temp <= -20) {
        diff_volt_uv = 0;
    } else if ( -20 < temp && temp <= 50 ) {
        diff_volt_uv = (20 * 1000 / 70) * (temp + 20); /* 20*1000 =20mv,  70 = (50 - (-20))*/
    } else if  ( temp > 50) {
        diff_volt_uv = 20 * 1000;
    }
    pm_domain_dev->cur_volt = avs_dev->poweron_volt - diff_volt_uv; /*step down*/
    PM_DEBUG("   ===========set Media volt ========>  [poweron_volt :%d uv], [diff_volt:%d uv]   [cur_volt:%d uv]  . ============\n", avs_dev->poweron_volt, diff_volt_uv, pm_domain_dev->cur_volt);
    regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
}


/*for debug media voltage */
#ifdef __PM_DEBUG_ON__
static void media_get_cur_volt(void)
{
    int  temp = 0;
    struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
//   regulator->set_voltage(regulator, 896053);
//   PM_DEBUG(" ============>  set media volt   [:896053 uv] fixly . \n" );
    temp = regulator->get_voltage(regulator);
    PM_DEBUG(" ============>  get media volt   [:%d uv]  . \n", temp);
}
#endif


int media_domain_get_poweron_volt(void)
{
    struct avs_device* avs_dev = &media_avs_dev;
    struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    avs_dev->poweron_volt = regulator->get_voltage(regulator);
    PM_DEBUG("================== media power on volt : %d uv==================\n", avs_dev->poweron_volt);
    return 0;
}

//#endif

int media_domain_avs_handle(void)
{
    HI_MEDIA_AVS_POLICY_E mediaAvsPolicy = (HI_MEDIA_AVS_POLICY_E)hal_media_get_avs_policy();
    if( HI_MEDIA_AVS_POLICY_CLOSE  == mediaAvsPolicy || HI_MEDIA_AVS_POLICY_BUTT == mediaAvsPolicy ) {
        return 0;
    } else {
        PM_DEBUG(" Media Avs Policy:  %0d\n", mediaAvsPolicy);
    }
    if ( HI_MEDIA_AVS_POLICY_HPM == mediaAvsPolicy ) {
        int cur_average_hpm = 0, delta = 0;
        struct avs_device* avs_dev = &media_avs_dev;
        struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
        struct regulator_device* regulator = pm_domain_dev->regulator_dev;
        pm_domain_dev->cur_volt = regulator->get_voltage(regulator);
        media_update_cur_avs_info();
        cur_average_hpm = media_get_average_hpm();
        delta = cur_average_hpm - avs_dev->cur_hpm;
        media_set_volt_accord_hpm(delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);
        /*
         PM_DEBUG("cur_average_hpm=%0d,avs_dev->cur_hpm=%0d,avs_dev->cur_volt_min=%0d,\
         avs_dev->cur_volt_max=%0d,pm_domain_dev->cur_volt=%0d\n",
            cur_average_hpm,avs_dev->cur_hpm,
            avs_dev->cur_volt_min,
            avs_dev->cur_volt_max,pm_domain_dev->cur_volt);
        */
    } else if( HI_MEDIA_AVS_POLICY_ONLY_TEMP == mediaAvsPolicy ) {
#ifdef __PM_DEBUG_ON__
        media_get_cur_volt();  //for debug //for debug
#endif
        media_set_volt_accord_temp();
    }
    return 0;
}

int media_domain_avs_init(struct pm_domain_device* pm_domain_dev)
{
    HI_MEDIA_AVS_POLICY_E mediaAvsPolicy = (HI_MEDIA_AVS_POLICY_E)hal_media_get_avs_policy();
    /*
        if( HI_MEDIA_AVS_POLICY_CLOSE  ==mediaAvsPolicy || HI_MEDIA_AVS_POLICY_BUTT == mediaAvsPolicy )
            return 0;
    */
    struct avs_device* avs_dev = &media_avs_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    pm_domain_dev->avs_dev  = avs_dev;
    pm_domain_dev->cur_volt = regulator->get_voltage(regulator);
    avs_dev->pm_domain_dev  = pm_domain_dev;
    avs_dev->hpm_opp_table  = media_hpm_opp_table;
    avs_dev->hpm_opp_num    = ARRAY_SIZE(media_hpm_opp_table);
    avs_dev->profile_num    = media_prof_num;
    avs_dev->cur_profile    = 0;
    avs_dev->temp_num       = media_temp_num;
    avs_dev->handle         = media_domain_avs_handle;
    PM_DEBUG("media_domain_avs_init,in...\n");
    if ( HI_MEDIA_AVS_POLICY_HPM == mediaAvsPolicy  ) {
        media_hpm_init();
    } else if ( HI_MEDIA_AVS_POLICY_ONLY_TEMP == mediaAvsPolicy  ) {
        if( 0 == pm_domain_dev->pm_dev->is_tsensor_init) {
            PM_DEBUG(" Media avs handle error:  Tsensor not inited!.   \n");
            return -1;
        }
        media_domain_get_poweron_volt();
    }
    return 0;
}


int media_domain_avs_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0;
}

int media_domain_getprofile(void)
{
    return media_avs_dev.cur_profile;
}

void media_domain_setprofile(int profile)
{
    media_avs_dev.cur_profile = profile;
}

int media_domain_setvoltage(int set_volt)
{
    struct pm_domain_device* pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    if (regulator->set_voltage(regulator, set_volt)) {
        return -1;
    }
    return 0;
}

