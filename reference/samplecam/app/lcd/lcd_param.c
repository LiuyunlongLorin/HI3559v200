/**
 * @file    vio_main.c
 * @brief   vio sample
 *
 * Copyright (c) 2017 Huawei Tech.Co., Ltd.
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2019/4/2
 * @version   1.0

 */

#include "hi_appcomm_log.h"
#include "lcd_param.h"

#include "hi_mapi_sys.h"
#include "hi_hal_screen.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define LCD_MAX_SCRREN_WIDTH       (1920)
#define LCD_MAX_SCRREN_HEIGHT      (1080)

static HI_VOID GenerateViVpssMode(MEDIA_ViVpssMode* viVpssMode)
{
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j) {
            viVpssMode->mode[i][j].enMode = VI_OFFLINE_VPSS_OFFLINE;
        }
    }
}

static HI_VOID GenerateVbCfg(MEDIA_VbCfg* vbCfg)
{
    HI_S32 i;
    HI_S32 vbIdx = 0;
    for (i = 0; i < 1; ++i) {
        vbCfg->commPool[vbIdx].blkSize = LCD_MAX_SCRREN_WIDTH * LCD_MAX_SCRREN_HEIGHT * 3 / 2;
        vbCfg->commPool[vbIdx].blkCnt  = 6;
        vbIdx++;
    }
    vbCfg->maxPoolCnt = vbIdx;

    MLOGI("VBConfigure: \n");
    for (i = 0; i < vbCfg->maxPoolCnt; ++i) {
        MLOGI("  [%d] BlkSize[%u] BlkCnt[%u]\n", i, vbCfg->commPool[i].blkSize, vbCfg->commPool[i].blkCnt);
    }
}

static HI_S32 SwitchLcdIntfType(HI_HAL_SCREEN_LCD_INTF_TYPE_E lcdInftType,
                                VO_INTF_TYPE_E *voIntfType)
{
    switch(lcdInftType) {
        case HI_HAL_SCREEN_LCD_INTF_6BIT:
            *voIntfType = VO_INTF_LCD_6BIT;
            break;
        case HI_HAL_SCREEN_LCD_INTF_8BIT:
            *voIntfType = VO_INTF_LCD_8BIT;
            break;
        case HI_HAL_SCREEN_LCD_INTF_16BIT:
            *voIntfType = VO_INTF_LCD_16BIT;
            break;
        case HI_HAL_SCREEN_LCD_INTF_24BIT:
            *voIntfType = VO_INTF_LCD_24BIT;
            break;
        default:
            MLOGE("Invalid lcd type\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 GetLcdScreenAttr(HI_MAPI_DISP_ATTR_S *dispAttr, HI_MAPI_DISP_VIDEOLAYER_ATTR_S *videoLayerAttr)
{
    HI_S32 ret = HI_SUCCESS;
    HI_HAL_SCREEN_ATTR_S screenAttr = {0};
    ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &screenAttr);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);

    dispAttr->stPubAttr.u32BgColor = 0xff;
    dispAttr->stPubAttr.enIntfSync = VO_OUTPUT_USER;
    switch(screenAttr.enType) {
        case HI_HAL_SCREEN_INTF_TYPE_MIPI:
            dispAttr->stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case HI_HAL_SCREEN_INTF_TYPE_LCD:
            ret = SwitchLcdIntfType(screenAttr.unScreenAttr.stLcdAttr.enType,
                                    &dispAttr->stPubAttr.enIntfType);
            HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
            break;
        default:
            MLOGE("Invalid screen type\n");
            return HI_FAILURE;
    }

    dispAttr->stPubAttr.stSyncInfo.bSynm   = 1; /**<sync mode: signal */
    dispAttr->stPubAttr.stSyncInfo.bIop    = 1; /**<progressive display */
    dispAttr->stPubAttr.stSyncInfo.u16Vact = screenAttr.stAttr.stSynAttr.u16Vact;
    dispAttr->stPubAttr.stSyncInfo.u16Vbb  = screenAttr.stAttr.stSynAttr.u16Vbb;
    dispAttr->stPubAttr.stSyncInfo.u16Vfb  = screenAttr.stAttr.stSynAttr.u16Vfb;
    dispAttr->stPubAttr.stSyncInfo.u16Hact = screenAttr.stAttr.stSynAttr.u16Hact;
    dispAttr->stPubAttr.stSyncInfo.u16Hbb  = screenAttr.stAttr.stSynAttr.u16Hbb;
    dispAttr->stPubAttr.stSyncInfo.u16Hfb  = screenAttr.stAttr.stSynAttr.u16Hfb;
    dispAttr->stPubAttr.stSyncInfo.u16Hpw  = screenAttr.stAttr.stSynAttr.u16Hpw;
    dispAttr->stPubAttr.stSyncInfo.u16Vpw  = screenAttr.stAttr.stSynAttr.u16Vpw;
    dispAttr->stPubAttr.stSyncInfo.bIdv    = screenAttr.stAttr.stSynAttr.bIdv;
    dispAttr->stPubAttr.stSyncInfo.bIhs    = screenAttr.stAttr.stSynAttr.bIhs;
    dispAttr->stPubAttr.stSyncInfo.bIvs    = screenAttr.stAttr.stSynAttr.bIvs;

    dispAttr->stUserInfoAttr.stUserInfo.u32PreDiv   = 1;
    dispAttr->stUserInfoAttr.stUserInfo.bClkReverse = screenAttr.stAttr.stClkAttr.bClkReverse;
    dispAttr->stUserInfoAttr.stUserInfo.u32DevDiv   = screenAttr.stAttr.stClkAttr.u32DevDiv;
    dispAttr->stUserInfoAttr.u32DevFrameRate        = screenAttr.stAttr.u32Framerate;
    if (screenAttr.stAttr.stClkAttr.enClkType == HI_HAL_SCREEN_CLK_TYPE_LCDMCLK) {
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.enClkSource = VO_CLK_SOURCE_LCDMCLK;
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.u32LcdMClkDiv = screenAttr.stAttr.stClkAttr.u32OutClk;
    } else if (screenAttr.stAttr.stClkAttr.enClkType == HI_HAL_SCREEN_CLK_TYPE_PLL) {
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Fbdiv =
            screenAttr.stAttr.stClkAttr.stClkPll.u32Fbdiv;
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Frac =
            screenAttr.stAttr.stClkAttr.stClkPll.u32Frac;
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Refdiv =
            screenAttr.stAttr.stClkAttr.stClkPll.u32Refdiv;
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Postdiv1 =
            screenAttr.stAttr.stClkAttr.stClkPll.u32Postdiv1;
        dispAttr->stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Postdiv2 =
            screenAttr.stAttr.stClkAttr.stClkPll.u32Postdiv2;
    }

    videoLayerAttr->u32VLFrameRate = screenAttr.stAttr.u32Framerate;
    videoLayerAttr->stImageSize.u32Width  = screenAttr.stAttr.u32Width;
    videoLayerAttr->stImageSize.u32Height = screenAttr.stAttr.u32Height;
    return HI_SUCCESS;
}

static HI_S32 GenerateLcdDisplayCfg(MEDIA_DispCfg *dispCfg)
{
    HI_S32 ret = HI_SUCCESS;
    ret = GetLcdScreenAttr(&dispCfg->dispAttr, &dispCfg->videoLayerAttr);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    dispCfg->enable = HI_TRUE;
    dispCfg->dispHdl = 0;
    dispCfg->videoLayerAttr.u32BufLen = 3;

    dispCfg->cscAttr.enable = HI_TRUE;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Contrast = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Hue = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Luma = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Satuature = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.enCscMatrix = VO_CSC_MATRIX_BT601_TO_RGB_PC;

    dispCfg->wndCfg[0].enable = HI_TRUE;
    dispCfg->wndCfg[0].bindedMod = MEDIA_VIDEOMOD_VPSS;
    dispCfg->wndCfg[0].modHdl = 0;
    dispCfg->wndCfg[0].chnHdl = 0;
    dispCfg->wndCfg[0].wndHdl = 0;
    dispCfg->wndCfg[0].wndAttr.u32Priority = 0;
    dispCfg->wndCfg[0].wndAttr.stRect.s32X = 0;
    dispCfg->wndCfg[0].wndAttr.stRect.s32Y = 0;
    dispCfg->wndCfg[0].wndAttr.stRect.u32Width = dispCfg->videoLayerAttr.stImageSize.u32Width;
    dispCfg->wndCfg[0].wndAttr.stRect.u32Height = dispCfg->videoLayerAttr.stImageSize.u32Height;

#if defined(CONFIG_VERTICAL_SCREEN)
    dispCfg->wndCfg[0].rotation = ROTATION_90;
#else
    dispCfg->wndCfg[0].rotation = ROTATION_0;
#endif

    return HI_SUCCESS;
}

HI_S32 HI_PARAM_GetMediaCfg(MEDIA_Cfg* mediaCfg)
{
    HI_APPCOMM_CHECK_POINTER(mediaCfg, HI_FAILURE);
    memset(mediaCfg, 0, sizeof(MEDIA_Cfg));

    /* vi vpss mode */
    GenerateViVpssMode(&mediaCfg->viVpssMode);

    /* display */
    GenerateLcdDisplayCfg(&mediaCfg->videoOutCfg.dispCfg[0]);

    /* vb configure */
    GenerateVbCfg(&mediaCfg->vbCfg);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

