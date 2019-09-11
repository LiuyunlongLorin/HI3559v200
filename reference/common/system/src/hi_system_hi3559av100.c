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

#define PWR_CTRL0 0x180c0028
#define PWR_CTRL1 0x180c002c
#define PWR_CTRL3 0x180c0034


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
    return;
}
#endif

HI_VOID HI_SYSTEM_SetSystemPwrEnable(HI_BOOL bEnable)
{
    return;
}


HI_S32 HI_SYSTEM_GetStartupWakeupSource(HI_SYSTEM_STARTUP_SRC_E* penStartupSrc)
{
    *penStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
