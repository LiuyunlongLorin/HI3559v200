
#include "pm_device.h"
#include "pm_regulator.h"
#include "pm_dvfs.h"
#include "pm_avs.h"
#include "pm_avs.h"

#include "pm_dev_api.h"

#ifdef PM_DOMAIN_CORE_ON
#include "pm_core_avs.h"
#include "pm_core.h"
#endif


#ifdef PM_DOMAIN_MEDIA_ON
#include "hal_media_frq.h"
#include "pm_media_avs.h"
#include "pm_media.h"
#endif

#ifdef PM_DOMAIN_CPU_ON
#include "pm_cpu_freq.h"
#include "pm_cpu.h"
#endif

#include "pm_ext.h"


#define SUM_PIXEL_4K_30 (3840*2160*30)
#define SUM_PIXEL_4K_15 (3840*2160*15)

static HI_MEDIA_FREQ_S s_stMediaFreq;
static HI_U32 s_u32OldProfile = 3;
static HI_BOOL s_bsetMediaFreq_done = HI_FALSE;
static HI_MPI_PM_MEDIA_CFG_S g_pm_usrcfg = {};
extern struct pm_device pm_dev;


/*
 *  dvfs_ctrl:cpu domain dvfs ctrl interface
 *  parameter:0:disable dvfs;  others:enable dvfs
 *  return:-1:fail;   0:ok
 */
int dvfs_ctrl(int enable)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    osal_mutex_lock(&(pm_dev.cpu_domain_device->freq_dev->lock));
    if (enable) {
        pm_dev.cpu_domain_device->freq_dev->dvfs_enable = 1;
    } else {
        pm_dev.cpu_domain_device->freq_dev->dvfs_enable = 0;
    }
    osal_mutex_unlock(&(pm_dev.cpu_domain_device->freq_dev->lock));
    return 0;
}

/*
 *  svp_ctrl:samrt version proc ctrl interface
 *  parameter:0:disable svp;  others:enable svp
 *  return:-1:fail;   0:ok
 */
int svp_ctrl(int enable)
{
    HI_PM_MISC_POLICY_E miscPolicy = (HI_PM_MISC_POLICY_E)PM_HAL_GetMiscPolicy();
    if( HI_PM_MISC_POLICY_NONE  == miscPolicy || HI_PM_MISC_POLICY_BUTT  == miscPolicy  ) {
        osal_printk("  No misc policy used !.\n");
        return 0;
    }
    osal_printk("  svp_ctrl :%d ok!\n", enable);
    return 0;
}

/*
 *  pm_ctrl:power manager ctrl interface
 *  parameter:0:disable pm;  others:enable pm
 *  return:-1:fail;   0:ok
 */
int pm_ctrl(int enable)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    osal_mutex_lock(&(pm_dev.lock));
    if (enable) {
        pm_dev.pm_enable = 1;
    } else {
        pm_dev.pm_enable = 0;
    }
    osal_mutex_unlock(&(pm_dev.lock));
    return 0;
}


#ifdef PM_DOMAIN_CPU_ON

#ifdef __LITEOS__


/*
 * get_cpu_max_profile: get cpu domain max profile index
 * return:max profile index value
 */
int get_cpu_max_profile(void)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    return pm_dev.cpu_domain_device->freq_dev->profile_num - 1;
}

/*
 * get_cpu_profile: get cpu domain profile index
 * return:profile index value
 */
int get_cpu_profile(void)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    return cpu_domain_getprofile();
}

/*
 * set_cpu_profile: set cpu domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_cpu_profile(int profile)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    int max_profile = pm_dev.cpu_domain_device->freq_dev->profile_num - 1;
    if (pm_dev.pm_enable) {
        osal_printk("ERROR:please disable pm!\n");
        return -1;
    }
    if (profile < 0 || profile > max_profile) {
        osal_printk("ERROR: set_profile=%0d must be:[0~%0d]\n", profile, max_profile);
        return -1;
    }
    cpu_domain_setprofile(profile);
    return 0;
}
#endif

#endif


#ifdef PM_DOMAIN_MEDIA_ON

/*
 * get_media_max_profile: get media domain max profile index
 * return:max profile index value
 */
int get_media_max_profile(void)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    return 3;
}

/*
 * get_media_profile: get media domain profile index
 * return:profile index value
 */
int get_media_profile(void)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    int profile = media_domain_getprofile();
#if 0
    switch (profile) {
#ifdef HI3559
        case 0:
            profile = 2;
            break;
        case 1:
            profile = 3;
            break;
#endif
#ifdef HI3556
        case 0:
            profile = 1;
            break;
        case 1:
            profile = 2;
            break;
        case 2:
            profile = 3;
            break;
#endif
        default:
            profile = -1;
            osal_printk("ERROR: profile[%0d] is invalid!\n", profile);
    }
#endif
    return profile;
}

/*
 * set_media_profile: set media domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_media_profile(int profile)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    if (pm_dev.pm_enable) {
        osal_printk("ERROR:please disable pm!\n");
        return -1;
    }
#if 0
    switch (profile) {
#ifdef HI3559
        case 2:
            profile = 0;
            break;
        case 3:
            profile = 1;
            break;
#endif
#ifdef HI3556
        case 1:
            profile = 0;
            break;
        case 2:
            profile = 1;
            break;
        case 3:
            profile = 2;
            break;
#endif
        default:
            osal_printk("ERROR: profile[%0d] is invalid!\n", profile);
            return -1;
    }
#endif
    media_domain_setprofile(profile);
    return 0;
}


HI_S32 SetMediaProfileByMediaFreq(HI_MEDIA_FREQ_S* pstMediaFreq)
{
    HI_U32 u32Profile = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PM_HAL_GetMediaProfile(pstMediaFreq, &u32Profile);
    s_u32OldProfile = get_media_profile();
    if (u32Profile == s_u32OldProfile) {
        SetMediaFreqReg(pstMediaFreq);
        memcpy(&s_stMediaFreq, pstMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
    } else if (u32Profile > s_u32OldProfile) { //boosted voltage first ,then increase of frequency
        pm_ctrl(0);
        s32Ret = set_media_profile(u32Profile);
        if (HI_SUCCESS != s32Ret) {
            osal_printk( "set media profile %d error:%d.\n", u32Profile, s32Ret);
            pm_ctrl(1);
            return HI_FAILURE;
        }
        pm_ctrl(1);
        SetMediaFreqReg(pstMediaFreq);
        memcpy(&s_stMediaFreq, pstMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
    } else { //Reduce the frequency first, then reduction voltage
        SetMediaFreqReg(pstMediaFreq);
        memcpy(&s_stMediaFreq, pstMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
        pm_ctrl(0);
        s32Ret = set_media_profile(u32Profile);
        if (HI_SUCCESS != s32Ret) {
            osal_printk( "set media profile %d error:%d.\n", u32Profile, s32Ret);
            pm_ctrl(1);
            return HI_FAILURE;
        }
        pm_ctrl(1);
    }
    s_u32OldProfile = u32Profile;
    return HI_SUCCESS;
}



int get_media_usrcfg(HI_MPI_PM_MEDIA_CFG_S* pmedia_usrcfg)
{
    HI_MEDIA_AVS_POLICY_E mediaAvsPolicy = (HI_MEDIA_AVS_POLICY_E)hal_media_get_avs_policy();
    if( HI_MEDIA_AVS_POLICY_HPM  != mediaAvsPolicy ) {
        return 0;
    }
    if (NULL == pmedia_usrcfg) {
        osal_printk("Error: pmedia_usrcfgr is null!\n");
        return -1;
    }
    osal_memcpy(pmedia_usrcfg, &g_pm_usrcfg, sizeof(HI_MPI_PM_MEDIA_CFG_S));
    return HI_SUCCESS;
}

int set_media_usrcfg(HI_MPI_PM_MEDIA_CFG_S* pmedia_usrcfg)
{
    if (NULL == pmedia_usrcfg) {
        osal_printk("Error: pmedia_usrcfgr is null!\n");
        return -1;
    }
    osal_memcpy(&g_pm_usrcfg, pmedia_usrcfg, sizeof(HI_MPI_PM_MEDIA_CFG_S));
    return HI_SUCCESS;
}

HI_S32 SetMediaProfileByUsrCfg(HI_MPI_PM_MEDIA_CFG_S* pstUsrParam)
{
    HI_U32 u32Profile = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MEDIA_FREQ_S stMediaFreq = {};
    HI_MEDIA_AVS_POLICY_E mediaAvsPolicy = (HI_MEDIA_AVS_POLICY_E)hal_media_get_avs_policy();
    if( HI_MEDIA_AVS_POLICY_HPM  != mediaAvsPolicy ) {
        return 0;
    }
    PM_HAL_GetMediaFreqByUsrCfg(pstUsrParam, &stMediaFreq);
    PM_HAL_GetMediaProfile(&stMediaFreq, &u32Profile);
    s_u32OldProfile = get_media_profile();
    if (u32Profile == s_u32OldProfile) {
        SetMediaFreqReg(&stMediaFreq);
        memcpy(&s_stMediaFreq, &stMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
    } else if (u32Profile > s_u32OldProfile) { //boosted voltage first ,then increase of frequency
        pm_ctrl(0);
        s32Ret = set_media_profile(u32Profile);
        if (HI_SUCCESS != s32Ret) {
            osal_printk( "set media profile %d error:%d.\n", u32Profile, s32Ret);
            pm_ctrl(1);
            return HI_FAILURE;
        }
        pm_ctrl(1);
        SetMediaFreqReg(&stMediaFreq);
        memcpy(&s_stMediaFreq, &stMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
    } else { //Reduce the frequency first, then reduction voltage
        SetMediaFreqReg(&stMediaFreq);
        memcpy(&s_stMediaFreq, &stMediaFreq, sizeof(HI_MEDIA_FREQ_S));
        s_bsetMediaFreq_done = HI_TRUE;
        pm_ctrl(0);
        s32Ret = set_media_profile(u32Profile);
        if (HI_SUCCESS != s32Ret) {
            osal_printk( "set media profile %d error:%d.\n", u32Profile, s32Ret);
            pm_ctrl(1);
            return HI_FAILURE;
        }
        pm_ctrl(1);
    }
    set_media_usrcfg(pstUsrParam);
    s_u32OldProfile = u32Profile;
    return HI_SUCCESS;
}


HI_S32 SetSingleFreqByUsrCfg(HI_MPI_PM_SIGLE_MEDIA_CFG_S* pSingleModParam)
{
    /** Depend on core domain**/
#ifdef PM_DOMAIN_CORE_ON
    HI_MPI_PM_MEDIA_CFG_S media_usrcfg;
    if (NULL == pSingleModParam) {
        osal_printk("Error: pSingleModParam is null!\n");
        return HI_FAILURE;
    }
    if (0 == pm_dev.domain_num) {
        osal_printk("Error: no domain register!\n");
        return HI_FAILURE;
    }
    osal_memset(&media_usrcfg, 0, sizeof(HI_MPI_PM_MEDIA_CFG_S));
    switch (pSingleModParam->enMode) {
        case HI_PM_MEDIA_VI:
        case HI_PM_MEDIA_PIPEBE:
            osal_memcpy(&media_usrcfg.stViCfg, &pSingleModParam->unMediaCfg.stViCfg, sizeof(HI_MPI_PM_MEDIA_VI_CFG_S));
            break;
        case HI_PM_MEDIA_VPSS:
        case HI_PM_MEDIA_VEDU:
        case HI_PM_MEDIA_VDEC:
        case HI_PM_MEDIA_AVSP:
        default:
            osal_printk( "enModule is err\n");
            return HI_FAILURE;
    }
    int poweron_volt = 0;
    HI_S32 i = 0;
    pm_ctrl(0);
    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        if(SUM_PIXEL_4K_30 == media_usrcfg.stViCfg.au32VipipeSumPixel[i]) {
            poweron_volt = core_domain_get_poweron_volt();
            if(poweron_volt) {
                if(core_domain_setvoltage(poweron_volt)) {
                    osal_printk("Error: set poweron volt !\n");
                    pm_ctrl(1);
                    return HI_FAILURE;
                }
            } else {
                osal_printk("Error: get poweron volt !\n");
                pm_ctrl(1);
                return HI_FAILURE;
            }
            break;
        } else if(SUM_PIXEL_4K_15 == media_usrcfg.stViCfg.au32VipipeSumPixel[i]) {
            core_domain_reduce_voltage();
            break;
        }
    }
    pm_ctrl(1);
#endif
    return HI_SUCCESS;
}

/*
 * set_media_voltage: set media domain voltage
 * parameter:set_volt(uV)
 * return:0:ok;   -1:fail
 */
int set_media_voltage(int set_volt)
{
    if (pm_dev.domain_num == 0) {
        osal_printk("Error: no domain register!\n");
        return -1;
    }
    if (pm_dev.pm_enable) {
        osal_printk("ERROR:please disable pm!\n");
        return -1;
    }
    if (media_domain_setvoltage(set_volt)) {
        return -1;
    }
    return 0;
}

#endif



