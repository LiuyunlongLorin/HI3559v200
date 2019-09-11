#include "pm_core.h"
#include "pm_core_avs.h"
#include "hal_core_avs.h"
#include "pm_drv_comm.h"
#include "hal_media_avs.h"

static int reduce_volt_by_avs = 0;

struct avs_device core_avs_dev;

int core_get_average_temperature(void)
{
    return hal_core_get_average_temperature();
}

static void core_update_cur_avs_info(void)
{
    int i;
    int temperature = 0;
    int temperature_index = 0;
    struct avs_device* avs_dev = &core_avs_dev;
    /*Get current temperature and profile*/
    if (avs_dev->temp_num > 1) {
        temperature = core_get_average_temperature();
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
            /* core HPM freq */
            hal_core_set_hpm_div(avs_dev->div);
            break;
        }
    }
}

static void core_hpm_init(void)
{
    hal_core_hpm_init();
}

static int core_get_average_hpm(void)
{
    return hal_core_get_average_hpm();
}

static void core_set_volt_accord_hpm(int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    if (pm_domain_dev->cur_volt > volt_max) {
        pm_domain_dev->cur_volt -= CORE_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    } else if (pm_domain_dev->cur_volt < volt_min) {
        pm_domain_dev->cur_volt += CORE_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    } else if (hpm_delta < CORE_AVS_HPM_DELTA_MIN) {
        if (pm_domain_dev->cur_volt +  CORE_AVS_VOLT_STEP <= volt_max) {
            pm_domain_dev->cur_volt += CORE_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    } else if (hpm_delta >= CORE_AVS_HPM_DELTA_MAX) {
        if (pm_domain_dev->cur_volt - CORE_AVS_VOLT_STEP >= volt_min) {
            pm_domain_dev->cur_volt -= CORE_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    }
}

static void core_set_volt_accord_temp(void)
{
    int  temp = 0, diff_volt_uv = 0 ;
    struct avs_device* avs_dev = &core_avs_dev;
    struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    temp = core_get_average_temperature();
    PM_DEBUG(" read temp in core domain..:%d. \n", temp);

    diff_volt_uv=hal_media_get_voltage_accord_temp(temp);
    if(diff_volt_uv < 0) {
        diff_volt_uv = 0; /*Set the volt as power on volt */
    }
    if((reduce_volt_by_avs == 1) && (temp > 100)) {
        diff_volt_uv = 40 * 1000;
    }
    pm_domain_dev->cur_volt = avs_dev->poweron_volt - diff_volt_uv; /*step down*/
    PM_DEBUG("   ===========set Core volt ========>  [poweron_volt :%d uv], [diff_volt:%d uv]   [cur_volt:%d uv]  . ============\n", avs_dev->poweron_volt, diff_volt_uv, pm_domain_dev->cur_volt);
    regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
}


/*for debug core voltage */

#ifdef __PM_DEBUG_ON__
static void core_get_cur_volt(void)
{
    int  temp = 0;
    struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
//   regulator->set_voltage(regulator, 896053);
//   PM_DEBUG(" ============>  set core volt   [:896053 uv] fixly . \n" );
    temp = regulator->get_voltage(regulator);
    PM_DEBUG(" ============>  get core volt   [:%d uv]  . \n", temp);
}
#endif

int core_domain_get_poweron_volt(void)
{
    struct avs_device* avs_dev = &core_avs_dev;
    struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    if(0 == avs_dev->poweron_volt) {
        avs_dev->poweron_volt = regulator->get_voltage(regulator);
    }
    PM_DEBUG("================== core power on volt : %d uv==================\n", avs_dev->poweron_volt);
    return avs_dev->poweron_volt;
}

int core_domain_avs_handle(void)
{
    HI_CORE_AVS_POLICY_E coreAvsPolicy = (HI_CORE_AVS_POLICY_E)hal_core_get_avs_policy();
    if( HI_CORE_AVS_POLICY_CLOSE  == coreAvsPolicy || HI_CORE_AVS_POLICY_BUTT == coreAvsPolicy ) {
        return 0;
    } else {
        PM_DEBUG(" Core Avs Policy:  %0d\n", coreAvsPolicy);
    }
    if ( HI_CORE_AVS_POLICY_HPM == coreAvsPolicy ) {
        int cur_average_hpm = 0, delta = 0;
        struct avs_device* avs_dev = &core_avs_dev;
        struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
        struct regulator_device* regulator = pm_domain_dev->regulator_dev;
        pm_domain_dev->cur_volt = regulator->get_voltage(regulator);
        core_update_cur_avs_info();
        cur_average_hpm = core_get_average_hpm();
        delta = cur_average_hpm - avs_dev->cur_hpm;
        core_set_volt_accord_hpm(delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);
        /*
         PM_DEBUG("cur_average_hpm=%0d,avs_dev->cur_hpm=%0d,avs_dev->cur_volt_min=%0d,\
         avs_dev->cur_volt_max=%0d,pm_domain_dev->cur_volt=%0d\n",
            cur_average_hpm,avs_dev->cur_hpm,
            avs_dev->cur_volt_min,
            avs_dev->cur_volt_max,pm_domain_dev->cur_volt);
        */
    } else if( HI_CORE_AVS_POLICY_ONLY_TEMP == coreAvsPolicy ) {
#ifdef __PM_DEBUG_ON__
        core_get_cur_volt();  //for debug
#endif
        core_set_volt_accord_temp();
    }
    return 0;
}

int core_domain_avs_init(struct pm_domain_device* pm_domain_dev)
{
    HI_CORE_AVS_POLICY_E coreAvsPolicy = (HI_CORE_AVS_POLICY_E)hal_core_get_avs_policy();
    /*
        if( HI_CORE_AVS_POLICY_CLOSE  ==coreAvsPolicy || HI_CORE_AVS_POLICY_BUTT == coreAvsPolicy )
            return 0;
    */
    PM_DEBUG("core_domain_avs_init,in...\n");
    struct avs_device* avs_dev = &core_avs_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    pm_domain_dev->avs_dev  = avs_dev;
    pm_domain_dev->cur_volt = regulator->get_voltage(regulator);
    avs_dev->pm_domain_dev  = pm_domain_dev;
    avs_dev->hpm_opp_table  = core_hpm_opp_table;
    avs_dev->hpm_opp_num    = ARRAY_SIZE(core_hpm_opp_table);
    avs_dev->profile_num    = core_prof_num;
    avs_dev->cur_profile    = 0;
    avs_dev->temp_num       = core_temp_num;
    avs_dev->handle         = core_domain_avs_handle;
    if ( HI_CORE_AVS_POLICY_HPM == coreAvsPolicy  ) {
        core_hpm_init();
    } else if ( HI_CORE_AVS_POLICY_ONLY_TEMP == coreAvsPolicy  ) {
        if( 0 == pm_domain_dev->pm_dev->is_tsensor_init) {
            PM_DEBUG(" Media avs handle error:  Tsensor not inited !.   \n");
            return -1;
        }
        core_domain_get_poweron_volt();
    }
    return 0;
}


int core_domain_avs_exit(struct pm_domain_device* pm_domain_dev)
{
    return 0;
}

int core_domain_getprofile(void)
{
    return core_avs_dev.cur_profile;
}

void core_domain_setprofile(int profile)
{
    core_avs_dev.cur_profile = profile;
}

int core_domain_setvoltage(int set_volt)
{
    struct pm_domain_device* pm_domain_dev = core_avs_dev.pm_domain_dev;
    struct regulator_device* regulator = pm_domain_dev->regulator_dev;
    struct avs_device* avs_dev = &core_avs_dev;
    if(avs_dev->poweron_volt) {
        if(avs_dev->poweron_volt == set_volt) {
            /**Rest the core by poweron volt **/
            reduce_volt_by_avs = 0;
        }
    }
    if(regulator->set_voltage(regulator, set_volt)) {
        return -1;
    }
    return 0;
}

int core_domain_reduce_voltage(void)
{
    reduce_volt_by_avs = 1;
    return 0;
}

