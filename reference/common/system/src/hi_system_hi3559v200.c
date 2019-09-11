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

#define PWR_CTRL0 0x12090000
#define PWR_STATUS 0x12090008


#ifdef __LINUX__
HI_VOID HI_SYSTEM_Poweroff(HI_VOID)
{
    sync();
    HI_U32 value;
    himd(PWR_CTRL0, &value);
    value &= ~(0x1 << 16);   /**<button_adj_time enable*/
    value |= (0xA << 8);   /**<button_adj_time 10*100ms=1s*/
    value |= 0x2;  /**<shuntdown*/
    himm(PWR_CTRL0, value);
#ifdef CONFIG_RELEASE
    reboot(RB_POWER_OFF);
#endif
}


HI_VOID HI_SYSTEM_SetWakeUpEnable(HI_BOOL bEnable,HI_SYSTEM_WAKEUP_LEVEL_E enLevel)
{
    HI_U32 value = 0x0;
    himd(PWR_CTRL0, &value);
    if (bEnable)
    {
        value |= (0x1 << 2); /* open pwr_en , gsensor run when poweroff  */
        value &= ~(0x1 << 6); /*wakeup_en enable*/
        if(HI_SYSTEM_WAKEUP_LEVEL_HIGH == enLevel)
        {
            value &= ~(0x3 << 3);
            value |= (0x2 << 3); /*wakeup level mode,10 high level */
        }
        else
        {
            value |= (0x3 << 3); /*wakeup level mode,11 low level */
        }
    }
    else
    {
        value &= ~(0x1 << 2); /*pwr_en disable, gsensor stop when poweroff */
        value |= (0x1 << 6); /*wakeup_en disable*/
    }
    value &= ~(0x1 << 1);   /* clear poweroff_en bit*/
    himm(PWR_CTRL0, value);
}

#endif
HI_VOID HI_SYSTEM_SetSystemPwrEnable(HI_BOOL bEnable)
{
    HI_U32 value = 0x0;
    himd(PWR_CTRL0, &value);
    if (bEnable)
    {
        value |= (0x1 << 2); /* open pwr_en , gsensor run when poweroff  */
    }
    else
    {
        value &= ~(0x1 << 2); /*pwr_en disable, gsensor stop when poweroff */
    }
    value &= ~(0x1 << 1);   /* clear poweroff_en bit*/
    himm(PWR_CTRL0, value);
}

HI_S32 HI_SYSTEM_GetStartupWakeupSource(HI_SYSTEM_STARTUP_SRC_E* penStartupSrc)
{
    HI_APPCOMM_CHECK_POINTER(penStartupSrc, HI_EINVAL);
    HI_U32 value = 0x0;
    himd(PWR_STATUS,&value);
    value &= (0x1<<2);  /* read  wakeup spurce */
    if(value)
    {
        *penStartupSrc = HI_SYSTEM_STARTUP_SRC_WAKEYP;
    }
    else
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
