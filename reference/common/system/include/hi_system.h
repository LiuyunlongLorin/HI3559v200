/**
 * @file    hi_system.h
 * @brief   system module struct and interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/4/10
 * @version   1.0

 */
#ifndef __HI_SYSTEM_H__
#define __HI_SYSTEM_H__

#include "hi_mapi_ao_define.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**\addtogroup     SYSTEM */
/**@{ */  /**<!-- [SYSTEM] */

/** wakeup up source of startup Define */
typedef enum hiSYSTEM_STARTUP_SRC_E
{
    HI_SYSTEM_STARTUP_SRC_WAKEYP = 0,
    HI_SYSTEM_STARTUP_SRC_RTC,
    HI_SYSTEM_STARTUP_SRC_STARTUP,
    HI_SYSTEM_STARTUP_SRC_BUTT
} HI_SYSTEM_STARTUP_SRC_E;

/** wakeup up level Define */
typedef enum hiSYSTEM_WAKEUP_LEVEL_E
{
    HI_SYSTEM_WAKEUP_LEVEL_LOW = 0,
    HI_SYSTEM_WAKEUP_LEVEL_HIGH,
    HI_SYSTEM_WAKEUP_LEVEL_BUTT
} HI_SYSTEM_WAKEUP_LEVEL_E;

typedef struct hiSYSTEM_TM_S
{
    HI_S32 s32sec;         /**< seconds.range[0,59] */
    HI_S32 s32min;         /**< minutes.range[0,59] */
    HI_S32 s32hour;        /**< hours.range[0,23] */
    HI_S32 s32mday;        /**< day of the month.range[1,31] */
    HI_S32 s32mon;         /**< month.range[1,12] */
    HI_S32 s32year;        /**< year.>1970 */
} HI_SYSTEM_TM_S;

#if defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__)
/**
 * @brief    init system msg service
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_S32 HI_SYSTEM_ServiceInit(HI_VOID);
#endif

#if (defined(AMP_LINUX_HUAWEILITE)&& !defined(__HuaweiLite__))
/**
 * @brief     wait sharefs ready
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_BOOL HI_SYSTEM_WaitSharefsReady(HI_S32 s32Timeout_s);
#endif


#ifdef CFG_BOOT_SOUND_ON
/**
 * @brief    play boot sound.
 * @param[in] AoHdl:ao handle
 * @param[in] pu8PCMData:pcm_s16le data
 * @param[in] u32DataLen:pcm_s16le data lenght
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/8
 */
HI_S32 HI_SYSTEM_BootSound(HI_HANDLE AoHdl, const HI_U8* pu8PCMData, HI_U32 u32DataLen);
#endif

#ifdef CFG_BOOT_LOGO_ON
/**
 * @brief    show boot logo.
 * @param[in] DispHdl:diap handle
 * @param[in] WndHdl:window handle
 * @param[in] stRes:logo resolution
 * @param[in] pu8YUVData:YUV420 planer data
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/8
 */
HI_S32 HI_SYSTEM_BootLogo(HI_HANDLE DispHdl, HI_HANDLE WndHdl, SIZE_S stRes, const HI_U8* pu8YUVData);
#endif

/**
 * @brief    set system date time
 * @param[in] pstDateTime: date time
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_S32 HI_SYSTEM_SetDateTime(const HI_SYSTEM_TM_S* pstDateTime);

/**
 * @brief    get rtc date time
 * @param[out] pstDateTime: date time
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_S32 HI_SYSTEM_GetRTCDateTime(HI_SYSTEM_TM_S* pstDateTime);

/**
 * @brief    produce a system sound for key tone.
 * @param[in] AoHdl:ao handle
 * @param[in] pstAoAttr:ao attribute
 * @param[in] u64PhyAddr:pcm_s16le data phyaddr
 * @param[in] u32DataLen:pcm_s16le data lenght
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_S32 HI_SYSTEM_Sound(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S* pstAoAttr, HI_U64 u64PhyAddr, HI_U32 u32DataLen);

/**
 * @brief    halt system
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_VOID HI_SYSTEM_Poweroff(HI_VOID);

/**
 * @brief    reboot system.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_VOID HI_SYSTEM_Reboot(HI_VOID);


/**
 * @brief    set Wakeup enable.
 * @param[in] enable: Wakeup enable
 * @param[in] enLevel: Wakeup level
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_VOID HI_SYSTEM_SetWakeUpEnable(HI_BOOL bEnable,HI_SYSTEM_WAKEUP_LEVEL_E enLevel);

/**
 * @brief    set power_on enable.
 * @param[in] bEnable:
              HI_TRUE mean poweren_bit is true,and output high level
              HI_FALSE mean poweren_bit is false,and output low leve
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_VOID HI_SYSTEM_SetSystemPwrEnable(HI_BOOL bEnable);

/**
 * @brief    get Wakeup Source of Startup.
 * @param[in] penStartupSrc: Wakeup Source
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/4
 */
HI_S32 HI_SYSTEM_GetStartupWakeupSource(HI_SYSTEM_STARTUP_SRC_E* penStartupSrc);

/**@}*/  /**<!-- ==== SYSTEM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_SYSTEM_H__ */


