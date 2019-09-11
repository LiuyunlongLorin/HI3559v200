/**
* @file    hal_screen_ili8961.c
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
#if defined(AMP_LINUX_HUAWEILITE)
#include "hi_hal_common_inner.h"
#endif
#include "hi_hal_screen.h"
#include "hi_hal_screen_inner.h"
#include "hi_hal_gpio_inner.h"

#ifdef __HuaweiLite__
#include "asm/io.h"
#endif


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
extern void lcd_sleepin(void);
extern void lcd_sleepout(void);


#elif (defined(AMP_LINUX))

#define HAL_SCREEN_DEV       ("/dev/ssp_8961")
static HI_S32 s_s32HALSCREENFd = -1;

#endif


#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || !(defined(AMP_LINUX_HUAWEILITE)))

typedef struct tagHAL_SCREEN_ILI8961_CTX_S {
    HI_HAL_SCREEN_STATE_E enSCREENDisplayState;
} HAL_SCREEN_ILI8961_CTX_S;
static HAL_SCREEN_ILI8961_CTX_S s_stHALSCREENFt8961Ctx = {HI_HAL_SCREEN_STATE_BUIT};

#endif


static HI_S32 HAL_SCREEN_ILI8961_Init(HI_VOID)
{
    /** Initial screen Device */
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE)
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = hi_ssp_lcd_init();
    if (HI_SUCCESS != s32Ret) {
        MLOGE("init screen failed\n");
        return HI_HAL_EINTER;
    };
    s_stHALSCREENFt8961Ctx.enSCREENDisplayState = HI_TRUE;
#elif (defined(AMP_LINUX))
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/hi_ssp_ili8961.ko", NULL);
    if (HI_SUCCESS != s32Ret) {
        MLOGE("insmod hi_ssp_ili8961:failed\n");
        return HI_HAL_EINTER;
    }
    s_s32HALSCREENFd = open(HAL_SCREEN_DEV, O_RDWR);
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALSCREENFd) {
        HI_rmmod(HI_APPFS_KOMOD_PATH"/hi_ssp_ili8961.ko");
        return HI_HAL_EINTER;
    }
    s_stHALSCREENFt8961Ctx.enSCREENDisplayState = HI_TRUE;
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetAttr(HI_HAL_SCREEN_ATTR_S *pstAttr)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    //DO NOTHING
#else
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    pstAttr->enType = HI_HAL_SCREEN_INTF_TYPE_LCD;
    pstAttr->unScreenAttr.stLcdAttr.enType = HI_HAL_SCREEN_LCD_INTF_8BIT;
    pstAttr->stAttr.u32Width = 320;
    pstAttr->stAttr.u32Height = 240;
    pstAttr->stAttr.u32Framerate = 60;
    pstAttr->stAttr.stClkAttr.enClkType = HI_HAL_SCREEN_CLK_TYPE_LCDMCLK;
    pstAttr->stAttr.stClkAttr.u32OutClk = 0x295FAC;
    pstAttr->stAttr.stClkAttr.u32DevDiv = 4;
    pstAttr->stAttr.stClkAttr.bClkReverse = HI_TRUE;
    pstAttr->stAttr.stSynAttr.u16Vact = 240;
    pstAttr->stAttr.stSynAttr.u16Vbb = 21;
    pstAttr->stAttr.stSynAttr.u16Vfb = 2;
    pstAttr->stAttr.stSynAttr.u16Hact = 320;
    pstAttr->stAttr.stSynAttr.u16Hbb = 60;
    pstAttr->stAttr.stSynAttr.u16Hfb = 10;
    pstAttr->stAttr.stSynAttr.u16Hpw = 1;
    pstAttr->stAttr.stSynAttr.u16Vpw = 1;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 1;
    pstAttr->stAttr.stSynAttr.bIvs = 0;
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetDisplayState(HI_HAL_SCREEN_STATE_E *penDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    //DO NOTHING
#else
    *penDisplayState = s_stHALSCREENFt8961Ctx.enSCREENDisplayState;
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_SetDisplayState(HI_HAL_SCREEN_STATE_E enDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    //DO NOTHING
#else
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetLuma(HI_U32 *pu32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_SetLuma(HI_U32 u32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetSatuature(HI_U32 *pu32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_SetSatuature(HI_U32 u32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetContrast(HI_U32 *pu32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_SetContrast(HI_U32 u32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_SetBackLightState(HI_HAL_SCREEN_STATE_E enBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))
    //DO NOTHING
#else
#if defined(HI3518EV300)
    HI_U32 u32SetValue0 = 0x1 << 0;
    HI_U32 mask0 = 0x1 << 0;
    if (HI_HAL_SCREEN_STATE_OFF == enBackLightState) {
        writereg(u32SetValue0, mask0, 0x120B5400);
        writel(0x0, 0x120B5004);
    } else if (HI_HAL_SCREEN_STATE_ON == enBackLightState) {
        writereg(u32SetValue0, mask0, 0x120B5400);
        writel(0x1, 0x120B5004);
    }
#endif
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_GetBackLightState(HI_HAL_SCREEN_STATE_E *penBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))
    //DO NOTHING
#else
    // DO GPIO ACTION
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ILI8961_Deinit(HI_VOID)
{
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))  || defined(AMP_HUAWEILITE)
    hi_ssp_lcd_exit();
#elif (defined(AMP_LINUX))
    HI_rmmod("/komod/hi_ssp_ili8961.ko");
    close(s_s32HALSCREENFd);
#endif
    return HI_SUCCESS;
}

/** ILI8961 Object */
HI_HAL_SCREEN_OBJ_S stHALSCREENObj = {
    .pfnInit = HAL_SCREEN_ILI8961_Init,
    .pfnGetAttr = HAL_SCREEN_ILI8961_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_ILI8961_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_ILI8961_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_ILI8961_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_ILI8961_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_ILI8961_SetLuma,
    .pfnGetLuma = HAL_SCREEN_ILI8961_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_ILI8961_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_ILI8961_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_ILI8961_SetContrast,
    .pfnGetContrast = HAL_SCREEN_ILI8961_GetContrast,
    .pfnDeinit = HAL_SCREEN_ILI8961_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

