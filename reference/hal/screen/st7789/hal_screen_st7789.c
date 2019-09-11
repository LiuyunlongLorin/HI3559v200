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

#define HAL_SCREEN_DEV       ("/dev/ssp_7789")
static HI_S32 s_s32HALSCREENFd = -1;

#endif


#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || !(defined(AMP_LINUX_HUAWEILITE)))

typedef struct tagHAL_SCREEN_ST7789_CTX_S
{
    HI_HAL_SCREEN_STATE_E enSCREENDisplayState;
} HAL_SCREEN_ST7789_CTX_S;
static HAL_SCREEN_ST7789_CTX_S s_stHALSCREENFt7789Ctx = {HI_HAL_SCREEN_STATE_BUIT};

#endif


static HI_S32 HAL_SCREEN_ST7789_Init(HI_VOID)
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

    s_stHALSCREENFt7789Ctx.enSCREENDisplayState = HI_TRUE;

#elif (defined(AMP_LINUX))

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_insmod(HI_APPFS_KOMOD_PATH"/hi_ssp_st7789.ko",NULL);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod hi_ssp_st7789:failed\n");
        return HI_HAL_EINTER;
    }

    s_s32HALSCREENFd = open(HAL_SCREEN_DEV, O_RDWR);
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALSCREENFd)
    {
        HI_rmmod(HI_APPFS_KOMOD_PATH"/hi_ssp_st7789.ko");
        return HI_HAL_EINTER;
    }

    s_stHALSCREENFt7789Ctx.enSCREENDisplayState = HI_TRUE;

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetAttr(HI_HAL_SCREEN_ATTR_S* pstAttr)
{

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

    //DO NOTHING

#else

    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    pstAttr->enType = HI_HAL_SCREEN_INTF_TYPE_LCD;
    pstAttr->unScreenAttr.stLcdAttr.enType = HI_HAL_SCREEN_LCD_INTF_6BIT;

    pstAttr->stAttr.u32Width = 240;
    pstAttr->stAttr.u32Height = 320;
    pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stClkAttr.enClkType = HI_HAL_SCREEN_CLK_TYPE_LCDMCLK;
    pstAttr->stAttr.stClkAttr.u32OutClk = 0x1CF62C;
    pstAttr->stAttr.stClkAttr.u32DevDiv = 3;
    pstAttr->stAttr.stClkAttr.bClkReverse = HI_TRUE;

    pstAttr->stAttr.stSynAttr.u16Vact = 320;
    pstAttr->stAttr.stSynAttr.u16Vbb = 10;
    pstAttr->stAttr.stSynAttr.u16Vfb = 4;
    pstAttr->stAttr.stSynAttr.u16Hact = 240;
    pstAttr->stAttr.stSynAttr.u16Hbb = 30;
    pstAttr->stAttr.stSynAttr.u16Hfb = 10;
    pstAttr->stAttr.stSynAttr.u16Hpw = 10;
    pstAttr->stAttr.stSynAttr.u16Vpw = 2;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->stAttr.stSynAttr.bIvs = 0;

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetDisplayState(HI_HAL_SCREEN_STATE_E* penDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

        //DO NOTHING

#else

    *penDisplayState = s_stHALSCREENFt7789Ctx.enSCREENDisplayState;

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_SetDisplayState(HI_HAL_SCREEN_STATE_E enDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

            //DO NOTHING

#else
#if 0
    if (HI_HAL_SCREEN_STATE_ON == enDisplayState)
    {
        HI_U32 u32Vaule = 0;
        HI_U32 u32SetValue0 = 0x6 << 18;
        HI_U32 mask0 = 0x7 << 18;

        HI_U32 u32SetValue1 = 0x227a65;
        HI_U32 mask1 = 0x07ffffff;

        HI_U32 u32SetValue2 = 0x2 << 14;
        HI_U32 mask2 = 0x3 << 14;

        u32Vaule = readl(0x04510124);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);

        u32Vaule &= ~(mask2);
        u32Vaule |= (u32SetValue2 & mask2);
        writel(u32Vaule, 0x04510124);

        u32Vaule = readl(0x04510120);
        u32Vaule &= ~(mask1);
        u32Vaule |= (u32SetValue1 & mask1);
        writel(u32Vaule, 0x04510120);
    }
    else if (HI_HAL_SCREEN_STATE_OFF == enDisplayState)
    {

    }
#endif
#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetLuma(HI_U32* pu32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_SetLuma(HI_U32 u32Luma)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetSatuature(HI_U32* pu32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_SetSatuature(HI_U32 u32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetContrast(HI_U32* pu32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_SetContrast(HI_U32 u32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_SetBackLightState(HI_HAL_SCREEN_STATE_E enBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

    //DO NOTHING

#else

#if defined(HI3556AV100) || defined(HI3519AV100)
    HI_U32 u32Vaule = 0;
    HI_U32 u32SetValue0 = 0x1 << 6;
    HI_U32 mask0 = 0x1 << 6;

    if (HI_HAL_SCREEN_STATE_OFF == enBackLightState)
    {
        u32Vaule = readl(0x045F1400);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);

        writel(u32Vaule, 0x045F1400);
        writel(0x0, 0x045F1100);
        lcd_sleepin();
    }
    else if (HI_HAL_SCREEN_STATE_ON == enBackLightState)
    {
        u32Vaule = readl(0x045F1400);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);
        lcd_sleepout();
        writel(u32Vaule, 0x045F1400);
        writel(0x40, 0x045F1100);
    }
#elif defined(HI3559V200)
        HI_U32 u32Vaule = 0;
    HI_U32 u32SetValue0 = 0x1 << 7;
    HI_U32 mask0 = 0x1 << 7;

    if (HI_HAL_SCREEN_STATE_OFF == enBackLightState)
    {
        u32Vaule = readl(0x120D6400);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);

        writel(u32Vaule, 0x120D6400);
        writel(0x0, 0x120D6200);
        lcd_sleepin();
    }
    else if (HI_HAL_SCREEN_STATE_ON == enBackLightState)
    {
        u32Vaule = readl(0x120D6400);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);
        lcd_sleepout();
        writel(u32Vaule, 0x120D6400);
        writel(0x80, 0x120D6200);
    }
#endif
#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_GetBackLightState(HI_HAL_SCREEN_STATE_E* penBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

    //DO NOTHING

#else

    // DO GPIO ACTION

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_ST7789_Deinit(HI_VOID)
{
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))  || defined(AMP_HUAWEILITE)

    hi_ssp_lcd_exit();

#elif (defined(AMP_LINUX))

    HI_rmmod("/komod/hi_ssp_st7789.ko");
    close(s_s32HALSCREENFd);

#endif

    return HI_SUCCESS;
}

/** OTA5182 Object */
HI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
    .pfnInit = HAL_SCREEN_ST7789_Init,
    .pfnGetAttr = HAL_SCREEN_ST7789_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_ST7789_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_ST7789_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_ST7789_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_ST7789_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_ST7789_SetLuma,
    .pfnGetLuma = HAL_SCREEN_ST7789_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_ST7789_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_ST7789_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_ST7789_SetContrast,
    .pfnGetContrast = HAL_SCREEN_ST7789_GetContrast,
    .pfnDeinit = HAL_SCREEN_ST7789_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

