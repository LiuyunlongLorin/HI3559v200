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
#include <sys/ioctl.h>

#include "hi_appcomm_util.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"
#include "hi_hal_screen.h"
#include "hi_hal_screen_inner.h"
#include "hi_hal_gpio_inner.h"

#if defined(HI3559V200)
#include "hi_mipi_tx.h"
#endif



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE)
    extern int  mipi_tx_module_init(void);
    extern void  mipi_tx_module_exit(void);

#if (defined(BOARD_DASHCAM_REFB) && defined(HI3559V200))
#define SCREENMAXLUMA (1000)
#endif

#elif (defined(AMP_LINUX))

static HI_S32 s_s32HALSCREENFd = -1;

#endif


#if ((defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || !(defined(AMP_LINUX_HUAWEILITE)))

typedef struct tagHAL_SCREEN_OTA7290B_CTX_S
{
    HI_HAL_SCREEN_STATE_E enSCREENDisplayState;
} HAL_SCREEN_OTA7290B_CTX_S;
static HAL_SCREEN_OTA7290B_CTX_S s_stHALSCREENOTA7290BCtx = {HI_HAL_SCREEN_STATE_BUIT};


#if defined(HI3559V200)
#define HAL_SCREEN_DEV       ("/dev/hi_mipi_tx")
static HI_S32 s_s32MipiFd = -1;

#endif


#endif


static HI_S32 HAL_SCREEN_OTA7290B_Init(HI_VOID)
{
    /** Initial screen Device */
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE)

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Value = 0;
    HI_S32 s32Setvalue = 0x1 << 5;
    HI_S32 s32mask = 0x1 << 5 ;
    himd(0x120d0400, &u32Value);  /**GPIO_0_5*/
    u32Value &= ~(s32mask);
    u32Value |= (s32Setvalue & s32mask);
    himm(0x120d0400, u32Value);

    himd(0x120d0400, &u32Value);
    printf("s32Value is %#x.\n",u32Value);

    himm(0x120D0080,0x20);
    himm(0x120D0080,0x00);
    himm(0x120D0080,0x20);

    s32Ret = mipi_tx_module_init();
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("init screen failed\n");
        return HI_HAL_EINTER;
    };

#if (defined(BOARD_DASHCAM_REFB) && defined(HI3559V200))
    MLOGI("init screen_pwm\n");
    /* set GPIO6_7 ---> PWM1 */
    himd(0x111f0028, &u32Value);
    u32Value |= 0x1;
    himm(0x111f0028,u32Value);

    /* set PWM clock */
    himd(0x120101bc, &u32Value);
    u32Value &= ~(0x11 << 8); /* [9:8]--> 00 :3MHz */
    himm(0x120101bc, u32Value);

    himd(0x120101bc, &u32Value);
    u32Value |= 0x1 << 7 ;    /* open PWM clock enable */
    himm(0x120101bc, u32Value);

    /*set PWM1 */
    himm(0x1207002c, 0x4); /* close PWM1 enable*/
    himm(0x12070020, 0x3e8); /* cycle 1K --> frequency 3KHz */
    himm(0x12070024, 0x3e8); /* default duty cycle is 100*/
    himm(0x1207002c, 0x5); /* open PWM1 enable*/
#endif

    s_s32MipiFd = open(HAL_SCREEN_DEV, O_RDWR);
    if (HAL_FD_INITIALIZATION_VAL == s_s32MipiFd)
    {
        mipi_tx_module_exit();
        return HI_HAL_EINTER;
    }

    /* ok */
    combo_dev_cfg_t MIPI_TX_CONFIG =
    {
        .devno = 0,
        .lane_id = {0, 1, 2, 3},
        .output_mode = OUTPUT_MODE_DSI_VIDEO,
        .output_format = OUT_FORMAT_RGB_24_BIT,
        .video_mode =  BURST_MODE,
        .sync_info = {
            .vid_pkt_size     = 400,
            .vid_hsa_pixels   = 22,
            .vid_hbp_pixels   = 38,
            .vid_hline_pixels = 470,
            .vid_vsa_lines    = 20,
            .vid_vbp_lines    = 30,
            .vid_vfp_lines    = 30,
            .vid_active_lines = 1600,
            .edpi_cmd_size = 0,
        },
        .phy_data_rate = 1000,
        .pixel_clk = 40623,
    };

    s32Ret = ioctl(s_s32MipiFd, HI_MIPI_TX_SET_DEV_CFG, &MIPI_TX_CONFIG);
    if (HI_SUCCESS != s32Ret)
    {
        printf("MIPI_TX SET_DEV_CONFIG failed\n");
        close(s_s32MipiFd);
        return HI_HAL_EINTER ;
    }

    s32Ret = ioctl(s_s32MipiFd, HI_MIPI_TX_ENABLE);
    if (HI_SUCCESS != s32Ret)
    {
        printf("MIPI_TX enable failed\n");
        close(s_s32MipiFd);
        return -1 ;
    }

    s_stHALSCREENOTA7290BCtx.enSCREENDisplayState = HI_TRUE;

#elif (defined(AMP_LINUX))

    s_stHALSCREENOTA7290BCtx.enSCREENDisplayState = HI_TRUE;

#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetAttr(HI_HAL_SCREEN_ATTR_S* pstAttr)
{

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

    //DO NOTHING

#else
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_HAL_EINVAL);
    pstAttr->enType = HI_HAL_SCREEN_INTF_TYPE_MIPI;

#ifdef CFG_LCD_UISRC_DEST_HEIGHT
    pstAttr->stAttr.u32Width = CFG_LCD_UISRC_DEST_HEIGHT;
#else
    pstAttr->stAttr.u32Width = 400;
#endif

#ifdef CFG_LCD_UISRC_DEST_WIDTH
    pstAttr->stAttr.u32Height = CFG_LCD_UISRC_DEST_WIDTH;
#else
    pstAttr->stAttr.u32Height = 1600;
#endif

    pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stClkAttr.u32DevDiv = 1;
    pstAttr->stAttr.stClkAttr.bClkReverse = HI_TRUE;
    pstAttr->stAttr.stClkAttr.enClkType = HI_HAL_SCREEN_CLK_TYPE_PLL;
    pstAttr->stAttr.stClkAttr.stClkPll.u32Fbdiv = 71;
    pstAttr->stAttr.stClkAttr.stClkPll.u32Frac = 0;
    pstAttr->stAttr.stClkAttr.stClkPll.u32Refdiv = 2;
    pstAttr->stAttr.stClkAttr.stClkPll.u32Postdiv1 = 4;
    pstAttr->stAttr.stClkAttr.stClkPll.u32Postdiv2 = 4;

    pstAttr->stAttr.stSynAttr.u16Vact = 1600;
    pstAttr->stAttr.stSynAttr.u16Vbb = 50;
    pstAttr->stAttr.stSynAttr.u16Vfb = 30;
    pstAttr->stAttr.stSynAttr.u16Hact = 400;
    pstAttr->stAttr.stSynAttr.u16Hbb = 80;
    pstAttr->stAttr.stSynAttr.u16Hfb = 50;
    pstAttr->stAttr.stSynAttr.u16Hpw = 30;
    pstAttr->stAttr.stSynAttr.u16Vpw = 20;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->stAttr.stSynAttr.bIvs = 0;
#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetDisplayState(HI_HAL_SCREEN_STATE_E* penDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

        //DO NOTHING

#else

    *penDisplayState = s_stHALSCREENOTA7290BCtx.enSCREENDisplayState;

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_SetDisplayState(HI_HAL_SCREEN_STATE_E enDisplayState)
{
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

            //DO NOTHING

#else


#endif

    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetLuma(HI_U32* pu32Luma)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

#else

#if ((defined(HI3559V200))&&(defined(BOARD_DASHCAM_REFB)))
    HI_U32 u32Value = 0;
    himd(0x12070024,&u32Value);
    *pu32Luma = u32Value * 100 / SCREENMAXLUMA;
#endif

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_SetLuma(HI_U32 u32Luma)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

#else

#if ((defined(HI3559V200))&&(defined(BOARD_DASHCAM_REFB)))
    HI_U32 u32Value = 0;
    if (u32Luma == 0)
    {
        u32Luma = 1;
    }
    u32Value = u32Luma * SCREENMAXLUMA / 100; /* conversion luma_value */
    himm(0x1207002c, 0x4); /* close PWM1 enable*/
    himm(0x12070024, u32Value); /* set duty cycle */
    himm(0x1207002c, 0x5); /* open PWM1 enable*/
#endif

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetSatuature(HI_U32* pu32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_SetSatuature(HI_U32 u32Satuature)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetContrast(HI_U32* pu32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_SetContrast(HI_U32 u32Contrast)
{
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_SetBackLightState(HI_HAL_SCREEN_STATE_E enBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

    //DO NOTHING

#else

#if defined(HI3559V200)

#ifndef BOARD_DASHCAM_REFB
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
    }
    else if (HI_HAL_SCREEN_STATE_ON == enBackLightState)
    {
        u32Vaule = readl(0x120D6400);
        u32Vaule &= ~(mask0);
        u32Vaule |= (u32SetValue0 & mask0);
        writel(u32Vaule, 0x120D6400);
        writel(0x80, 0x120D6200);
    }
#else
    MLOGI("set back light\n");
    if (HI_HAL_SCREEN_STATE_OFF == enBackLightState)
    {
        himm(0x1207002c, 0x4); /* close PWM1 enable*/
    }
    else if (HI_HAL_SCREEN_STATE_ON == enBackLightState)
    {
        himm(0x1207002c, 0x5); /* open PWM1 enable */
    }

#endif

#endif

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_GetBackLightState(HI_HAL_SCREEN_STATE_E* penBackLightState)
{
#if ((defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)))

    //DO NOTHING

#else
#if ((defined(HI3559V200))&&(defined(BOARD_DASHCAM_REFB)))
    HI_U32 u32Vaule = 0;
    himd(0x1207002c,&u32Vaule);
    if(u32Vaule & 0x1)
    {
        *penBackLightState = HI_HAL_SCREEN_STATE_ON;
    }
    else
    {
        *penBackLightState = HI_HAL_SCREEN_STATE_OFF;
    }
#endif

#endif
    return HI_SUCCESS;
}

static HI_S32 HAL_SCREEN_OTA7290B_Deinit(HI_VOID)
{
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))  || defined(AMP_HUAWEILITE)
    close(s_s32MipiFd);
    mipi_tx_module_exit();
#elif (defined(AMP_LINUX))


#endif

    return HI_SUCCESS;
}

/** OTA7290B Object */
HI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
    .pfnInit = HAL_SCREEN_OTA7290B_Init,
    .pfnGetAttr = HAL_SCREEN_OTA7290B_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_OTA7290B_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_OTA7290B_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_OTA7290B_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_OTA7290B_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_OTA7290B_SetLuma,
    .pfnGetLuma = HAL_SCREEN_OTA7290B_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_OTA7290B_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_OTA7290B_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_OTA7290B_SetContrast,
    .pfnGetContrast = HAL_SCREEN_OTA7290B_GetContrast,
    .pfnDeinit = HAL_SCREEN_OTA7290B_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

