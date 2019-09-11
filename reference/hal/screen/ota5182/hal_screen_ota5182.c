/**
* @file    hal_screen_ota5182.c
* @brief   hal screen implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "stdlib.h"

#include "hi_appcomm_util.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"
#include "hi_hal_screen.h"
#include "hi_hal_screen_inner.h"
#include "hi_hal_gpio_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE)

extern int  hi_ssp_lcd_init(void);
extern void  hi_ssp_lcd_exit(void);

#elif (defined(AMP_LINUX))

#define HAL_SCREEN_DEV       ("/dev/ssp_5182")
static HI_S32 s_s32HALSCREENFd = -1;

#endif


#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || !(defined(AMP_LINUX_HUAWEILITE)))

typedef struct tagHAL_SCREEN_OTA5182_CTX_S
{
    HI_HAL_SCREEN_STATE_E enSCREENDisplayState;
} HAL_SCREEN_OTA5182_CTX_S;
static HAL_SCREEN_OTA5182_CTX_S s_stHALSCREENOta5182Ctx = {HI_HAL_SCREEN_STATE_BUIT};

#endif


static HI_S32 HAL_SCREEN_OTA5182_Init(HI_VOID)
{
    /** Initial screen Device */
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE)

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = hi_ssp_lcd_init();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("init screen failed\n");
        return HI_HAL_EINTER;
    };

    s_stHALSCREENOta5182Ctx.enSCREENDisplayState = HI_TRUE;

#elif (defined(AMP_LINUX))

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/hi_ssp_ota5182.ko",NULL);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod hi_ssp_ota5182:failed\n");
        return HI_HAL_EINTER;
    }

    s_s32HALSCREENFd = open(HAL_SCREEN_DEV, O_RDWR);
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALSCREENFd)
    {
        HI_rmmod(HI_APPFS_KOMOD_PATH"/hi_ssp_ota5182.ko");
        return HI_HAL_EINTER;
    }

    s_stHALSCREENOta5182Ctx.enSCREENDisplayState = HI_TRUE;

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetAttr(HI_HAL_SCREEN_ATTR_S* pstAttr)
{

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

    //DO NOTHING

#else

    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    pstAttr->enType = HI_HAL_SCREEN_INTF_TYPE_LCD;
    pstAttr->unScreenAttr.stLcdAttr.enType = HI_HAL_SCREEN_LCD_INTF_8BIT;

    pstAttr->stAttr.u32OutClk = 0;

    pstAttr->stAttr.u32Width = 320;
    pstAttr->stAttr.u32Height = 240;
    pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stSynAttr.u16Vact = 0;
    pstAttr->stAttr.stSynAttr.u16Vbb = 0;
    pstAttr->stAttr.stSynAttr.u16Vfb = 0;
    pstAttr->stAttr.stSynAttr.u16Hact = 0;
    pstAttr->stAttr.stSynAttr.u16Hbb = 0;
    pstAttr->stAttr.stSynAttr.u16Hfb = 0;
    pstAttr->stAttr.stSynAttr.u16Hpw = 0;
    pstAttr->stAttr.stSynAttr.u16Vpw = 0;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->tAttr.stSynAttr.bIvs = 0;

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetDisplayState(HI_HAL_SCREEN_STATE_E* penDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

        //DO NOTHING

#else

    *penDisplayState = s_stHALSCREENOta5182Ctx.enSCREENDisplayState;

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_SetDisplayState(HI_HAL_SCREEN_STATE_E enDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

            //DO NOTHING

#else

    if (s_stHALSCREENOta5182Ctx.enSCREENDisplayState == enDisplayState)
    {
        return HI_SUCCESS;
    }

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetLuma(HI_U32* pu32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_SetLuma(HI_U32 u32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetSatuature(HI_U32* pu32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_SetSatuature(HI_U32 u32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetContrast(HI_U32* pu32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_SetContrast(HI_U32 u32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_SetBackLightState(HI_HAL_SCREEN_STATE_E enBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)))

    //DO NOTHING

#else

    // DO GPIO ACTION

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_GetBackLightState(HI_HAL_SCREEN_STATE_E* penBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)))

    //DO NOTHING

#else

    // DO GPIO ACTION

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA5182_Deinit(HI_VOID)
{
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))  || defined(AMP_HUAWEILITE)

    hi_ssp_lcd_exit();

#elif (defined(AMP_LINUX))

    HI_rmmod(HI_APPFS_KOMOD_PATH"/hi_ssp_ota5182.ko");
    close(s_s32HALSCREENFd);

#endif

    return HI_SUCCESS;
}

/** OTA5182 Object */
HI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
    .pfnInit = HAL_SCREEN_OTA5182_Init,
    .pfnGetAttr = HAL_SCREEN_OTA5182_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_OTA5182_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_OTA5182_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_OTA5182_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_OTA5182_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_OTA5182_SetLuma,
    .pfnGetLuma = HAL_SCREEN_OTA5182_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_OTA5182_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_OTA5182_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_OTA5182_SetContrast,
    .pfnGetContrast = HAL_SCREEN_OTA5182_GetContrast,
    .pfnDeinit = HAL_SCREEN_OTA5182_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

