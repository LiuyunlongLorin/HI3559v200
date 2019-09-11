#include <string.h>
#include <unistd.h>
#ifdef __LINUX__
#include <sys/ioctl.h>
#include <sys/reboot.h>
#endif

#include "hi_appcomm_util.h"
#include "hi_system.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define PWR_CTRL0 0x04590028
#define PWR_CTRL1 0x0459002c
#define PWR_CTRL3 0x04590034
#define PMC_WAKEUP0_IO_CTRL 0x04590000
#define PMC_EN0_IO_CTRL 0x0459001c


#ifdef __LINUX__
HI_VOID HI_SYSTEM_Poweroff(HI_VOID)
{
    sync();
    HI_U32 value;
    himd(PWR_CTRL1, &value);
    value |= 0xA;   /**<button_adj_time 10*100ms=1s*/
    himm(PWR_CTRL1, value);

    himd(PWR_CTRL0, &value);
    value |= 0x2;  /**<shuntdown*/
    himm(PWR_CTRL0, value);
#ifdef CONFIG_RELEASE
    reboot(RB_POWER_OFF);
#endif
}

HI_VOID HI_SYSTEM_SetWakeUpEnable(HI_BOOL bEnable,HI_SYSTEM_WAKEUP_LEVEL_E enLevel)
{
    HI_U32 value;
    himm(0x04590058,0x5A5AABCD);/** unlock pmc*/
    himd(PMC_WAKEUP0_IO_CTRL, &value);
    value |= (0x1 << 10); /**<wakeup0 as input*/
    himm(PMC_WAKEUP0_IO_CTRL, value);
    himd(PWR_CTRL0, &value);

    if (HI_TRUE == bEnable)
    {
        value &= ~(0x1 << 17);/**<wakeup0 enable*/
        if(HI_SYSTEM_WAKEUP_LEVEL_HIGH == enLevel)
        {
            value &= ~(0x3 << 8);
            value |= (0x2 << 8); /*wakeup level mode,10 high level */
        }
        else
        {
            value |= (0x3 << 8); /*wakeup level mode,11 low level */
        }
    }
    else
    {
        value |= (0x1 << 17);/**<wakeup0 disable*/
    }

    himm(PWR_CTRL0, value);
    himd(PMC_EN0_IO_CTRL, &value);
    value &= ~(0x1 << 10); /**<PWR_EN0 as output*/
    himm(PMC_EN0_IO_CTRL, value);
    himd(PWR_CTRL3, &value);

    if (HI_TRUE == bEnable)
    {
        value |= 0x1;/**<PWR_EN0 level high,gsensor run when poweroff*/
    }
    else
    {
        value &= ~0x1;/**<PWR_EN0 level low,gsensor stop when poweroff*/
    }
    himm(PWR_CTRL3, value);

}
#endif

HI_VOID HI_SYSTEM_SetSystemPwrEnable(HI_BOOL bEnable)
{
    HI_U32 value;
    himm(0x04590058,0x5A5AABCD);/** unlock pmc*/
    himd(PMC_EN0_IO_CTRL, &value);
    value &= ~(0x1 << 10); /**<PWR_EN0 as output*/
    himm(PMC_EN0_IO_CTRL, value);

    himd(PWR_CTRL3, &value);

    if (HI_TRUE == bEnable)
    {
        value |= 0x1;/**<PWR_EN0 level high,gsensor run when poweroff*/
    }
    else
    {
        value &= ~0x1;/**<PWR_EN0 level low,gsensor stop when poweroff*/
    }
    himm(PWR_CTRL3, value);
    MLOGI("PWR_CTRL3  value is %x\n",value);
    return;
}

HI_S32 HI_SYSTEM_GetStartupWakeupSource(HI_SYSTEM_STARTUP_SRC_E* penStartupSrc)
{
    HI_APPCOMM_CHECK_POINTER(penStartupSrc, HI_EINVAL);
#if defined(PMC_WAKEUP0_IO_CTRL)
    HI_U32 value = 0;
    himd(PWR_CTRL0, &value);
    if(value & (0x3 << 20))
    {
        *penStartupSrc = HI_SYSTEM_STARTUP_SRC_WAKEYP;
    }
    else
#endif
    {
        *penStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    }
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
