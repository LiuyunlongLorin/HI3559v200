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
#include "vio_param.h"
#include "hi_hal_screen.h"

#include "hi_mapi_sys.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct tagPARAM_SensorConfig {
    SIZE_S          size;
    HI_FLOAT        maxFrameRate;
    WDR_MODE_E      wdrMode;
    HI_U32          snsMode;
    HI_CHAR*        description;
} PARAM_SensorConfig;

#if defined(HI3559V200)

#if defined(CONFIG_SNS0_IMX307) && defined(CONFIG_SNS1_IMX307)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    { {{1920, 1080},  30.0f, WDR_MODE_NONE,      0, "1080P"},
      {{1920, 1080},  30.0f, WDR_MODE_NONE,      0, "1080P"}
    },
    { {{1920, 1080},  30.0f, WDR_MODE_2To1_LINE, 0, "1080P"},
      {{1920, 1080},  30.0f, WDR_MODE_2To1_LINE, 0, "1080P"}
    },
};

#elif defined(CONFIG_SNS0_IMX377)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{3840, 2160},  30.0f, WDR_MODE_NONE,      1, "2160P"}},
    {{{1920, 1080}, 120.0f, WDR_MODE_NONE,      0, "1080P"}},
    {{{1280,  720}, 300.0f, WDR_MODE_NONE,      0, "720P"}},
    {{{1280,  720}, 240.0f, WDR_MODE_NONE,      1, "720P"}},
    {{{4000, 3000},  20.0f, WDR_MODE_NONE,      1, "12M"}},
};

#elif defined(CONFIG_SNS0_IMX458)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{3840, 2160},  30.0f, WDR_MODE_NONE,      0, "2160P"}},
    {{{3840, 2160},  30.0f, WDR_MODE_NONE,      2, "2160P"}},
    {{{1920, 1080},  90.0f, WDR_MODE_NONE,      0, "1080P"}},
    {{{1280,  720}, 129.0f, WDR_MODE_NONE,      0, "720P"}},
    {{{2716, 1524},  60.0f, WDR_MODE_NONE,      0, "2.7K"}},
    {{{2716, 1524},  40.0f, WDR_MODE_NONE,      1, "2.7K"}},
    {{{4000, 3000},  20.0f, WDR_MODE_NONE,      0, "12M"}},
};

#elif defined(CONFIG_SNS0_IMX335)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{2592, 1944},  30.0f, WDR_MODE_NONE,      0, "1944P"}},
    {{{2592, 1944},  30.0f, WDR_MODE_2To1_LINE, 0, "1944P"}},
    {{{2592, 1536},  30.0f, WDR_MODE_NONE,      0, "1536P"}},
    {{{2592, 1536},  30.0f, WDR_MODE_2To1_LINE, 0, "1536P"}},
    {{{2560, 1440},  30.0f, WDR_MODE_NONE,      0, "1440P"}},
};

#elif defined(CONFIG_SNS0_OS05A)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{2560, 1600},  30.0f, WDR_MODE_NONE,      0, "1600P"}},
    {{{2560, 1600},  30.0f, WDR_MODE_2To1_LINE, 0, "1600P"}},
};

#elif defined(CONFIG_SNS0_OS08A10)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{3840, 2160},  30.0f, WDR_MODE_NONE,      0, "2160P"}},
};

#elif defined(CONFIG_SNS0_IMX307)
static PARAM_SensorConfig s_snsCfgs[][CONFIG_SENSOR_CNT] = {
    {{{1920, 1080},  30.0f, WDR_MODE_NONE,      0, "1080P"}},
    {{{1920, 1080},  30.0f, WDR_MODE_2To1_LINE, 0, "1080P"}},
};
#endif

#endif


static HI_VOID GenerateViVpssMode(MEDIA_ViVpssMode* viVpssMode)
{
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j) {
            viVpssMode->mode[i][j].enMode = VI_OFFLINE_VPSS_OFFLINE;
        }
    }
}

static HI_VOID GenerateVbCfg(const MEDIA_Cfg* mediaCfg, MEDIA_VbCfg* vbCfg)
{
    HI_S32 i;
    HI_S32 vbIdx = 0;
    for (i = 0; i < MIN(CONFIG_SENSOR_CNT, HI_PDT_MEDIA_VCAP_DEV_MAX_CNT); ++i) {
        const HI_MAPI_SENSOR_ATTR_S* snsAttr = &mediaCfg->videoCfg.vcapCfg.devAttr[i].snsAttr;
        vbCfg->commPool[vbIdx].blkSize =
            HI_APPCOMM_ALIGN(snsAttr->stSize.u32Width, 128) * snsAttr->stSize.u32Height * 3 / 2 +
            HI_APPCOMM_ALIGN(snsAttr->stSize.u32Height * 24, 128);
        vbCfg->commPool[vbIdx].blkCnt  = 5 + ((snsAttr->enWdrMode == WDR_MODE_2To1_LINE) ? 2 : 0);
        vbIdx++;

        const HI_MAPI_DISP_WINDOW_ATTR_S* dispWndAttr = &mediaCfg->videoOutCfg.dispCfg[0].wndCfg[i].wndAttr;
        vbCfg->commPool[vbIdx].blkSize  =
            HI_APPCOMM_ALIGN(dispWndAttr->stRect.u32Width, 16) * dispWndAttr->stRect.u32Height * 3 / 2 +
            HI_APPCOMM_ALIGN(dispWndAttr->stRect.u32Height * 24, 16);
        vbCfg->commPool[vbIdx].blkCnt  = 3;
        vbIdx++;
    }
    vbCfg->maxPoolCnt = vbIdx;

    MLOGI("VBConfigure: \n");
    for (i = 0; i < vbCfg->maxPoolCnt; ++i) {
        MLOGI("  [%d] BlkSize[%u] BlkCnt[%u]\n", i, vbCfg->commPool[i].blkSize, vbCfg->commPool[i].blkCnt);
    }
}

static HI_VOID GenerateVcapAttr(const PARAM_SensorConfig* snsCfg, MEDIA_VcapDevAttr* devAttr)
{
    devAttr->enable = HI_TRUE;

    devAttr->snsAttr.enWdrMode = snsCfg->wdrMode;
    devAttr->snsAttr.stSize.u32Width  = snsCfg->size.u32Width;
    devAttr->snsAttr.stSize.u32Height = snsCfg->size.u32Height;
    devAttr->snsAttr.u8SnsMode = snsCfg->snsMode;

    devAttr->devAttr.enWdrMode = snsCfg->wdrMode;
    devAttr->devAttr.stBasSize.u32Width  = snsCfg->size.u32Width;
    devAttr->devAttr.stBasSize.u32Height = snsCfg->size.u32Height;
    devAttr->devAttr.u32CacheLine =
        ((devAttr->snsAttr.enWdrMode == WDR_MODE_2To1_LINE) ?
        (devAttr->devAttr.stBasSize.u32Height / 2) : devAttr->devAttr.stBasSize.u32Height);

    MEDIA_VcapPipeAttr* vcapPipeAttr = &devAttr->vcapPipeAttr[0];
    vcapPipeAttr->enable = HI_TRUE;
    vcapPipeAttr->piepType = HI_MAPI_PIPE_TYPE_VIDEO;
    vcapPipeAttr->frameRate.s32SrcFrameRate = -1;
    vcapPipeAttr->frameRate.s32DstFrameRate = -1;
    vcapPipeAttr->compressMode = COMPRESS_MODE_NONE;

    vcapPipeAttr->ispBypass = HI_FALSE;
    vcapPipeAttr->ispPubAttr.f32FrameRate = MIN(snsCfg->maxFrameRate, 30.0f);
    vcapPipeAttr->ispPubAttr.stSize.u32Width = snsCfg->size.u32Width;
    vcapPipeAttr->ispPubAttr.stSize.u32Height = snsCfg->size.u32Height;
    vcapPipeAttr->ispPubAttr.enSnsMirrorFlip = ISP_SNS_NORMAL;
    vcapPipeAttr->ispPubAttr.enBayer = BAYER_BUTT;

    MEDIA_VcapPipeChnAttr* pipeChnAttr = &vcapPipeAttr->pipeChnAttr[0];
    pipeChnAttr->enable = HI_TRUE;
    pipeChnAttr->pipeChnHdl = 0;
    pipeChnAttr->destSize.u32Width = snsCfg->size.u32Width;
    pipeChnAttr->destSize.u32Height = snsCfg->size.u32Height;
    pipeChnAttr->frameRate.s32SrcFrameRate = -1;
    pipeChnAttr->frameRate.s32DstFrameRate = -1;
    pipeChnAttr->compressMode = COMPRESS_MODE_NONE;
    pipeChnAttr->pixelFormat = HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420;
    MLOGI("Pipe[0]Chn[0] destSize[%ux%u]\n",
        pipeChnAttr->destSize.u32Width, pipeChnAttr->destSize.u32Height);

    if (devAttr->snsAttr.enWdrMode == WDR_MODE_2To1_LINE) {
        memcpy(&devAttr->vcapPipeAttr[1], &devAttr->vcapPipeAttr[0], sizeof(MEDIA_VcapPipeAttr));
        devAttr->vcapPipeAttr[1].pipeHdl = devAttr->vcapPipeAttr[0].pipeHdl + 1;
    }
}

static HI_VOID GenerateVpssCfg(const MEDIA_VcapDevAttr* vcapDevAttr, MEDIA_VpssAttr* vpssAttr)
{
    vpssAttr->enable = HI_TRUE;
    vpssAttr->vpssAttr.u32MaxW = vcapDevAttr->devAttr.stBasSize.u32Width;
    vpssAttr->vpssAttr.u32MaxH = vcapDevAttr->devAttr.stBasSize.u32Height;
    vpssAttr->vpssAttr.stFrameRate.s32SrcFrameRate = -1;
    vpssAttr->vpssAttr.stFrameRate.s32DstFrameRate = -1;
    vpssAttr->vpssAttr.enPixelFormat = HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420;
    vpssAttr->vpssAttr.bNrEn = HI_TRUE;
    vpssAttr->vpssAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
    vpssAttr->vpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_NONE;
    vpssAttr->vpssAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;

    MEDIA_VpssPortAttr* vpssPortAttr = &vpssAttr->portAttr[0];
    vpssPortAttr->enable = HI_TRUE;
    vpssPortAttr->portHdl = 0;
    vpssPortAttr->resolution.u32Width = vpssAttr->vpssAttr.u32MaxW;
    vpssPortAttr->resolution.u32Height = vpssAttr->vpssAttr.u32MaxH;
    vpssPortAttr->supportBufferShare = HI_FALSE;
    vpssPortAttr->frameRate.s32SrcFrameRate = -1;
    vpssPortAttr->frameRate.s32DstFrameRate = -1;
    vpssPortAttr->videoFormat = VIDEO_FORMAT_LINEAR;
    vpssPortAttr->pixelFormat = HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420;
}

static HI_VOID UpdateVpssPortAttr(PARAM_DisplayType dispType,
                                  const MEDIA_DispWndCfg* dispWndCfg,
                                  MEDIA_VpssPortAttr* vpssPortAttr)
{
    /* update vpsschn handle if scale down */
    if (dispWndCfg->wndAttr.stRect.u32Width < vpssPortAttr->resolution.u32Width) {
        vpssPortAttr->portHdl = 1;
    }
    if (dispType == PARAM_DISPLAY_TYPE_LCD) {
#if defined(CONFIG_SCREEN) && defined(CONFIG_VERTICAL_SCREEN)
        vpssPortAttr->resolution.u32Width  = dispWndCfg->wndAttr.stRect.u32Height;
        vpssPortAttr->resolution.u32Height = dispWndCfg->wndAttr.stRect.u32Width;
#else
        vpssPortAttr->resolution.u32Width  = dispWndCfg->wndAttr.stRect.u32Width;
        vpssPortAttr->resolution.u32Height = dispWndCfg->wndAttr.stRect.u32Height;
#endif
    } else {
        vpssPortAttr->resolution.u32Width  = dispWndCfg->wndAttr.stRect.u32Width;
        vpssPortAttr->resolution.u32Height = dispWndCfg->wndAttr.stRect.u32Height;
    }
}

static HI_VOID GenerateHdmiDisplayCfg(MEDIA_DispCfg *dispCfg, SIZE_S *dispSize)
{
    dispCfg->dispAttr.stPubAttr.u32BgColor = 0xff;
    dispCfg->dispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
    dispCfg->dispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080I60;
    dispSize->u32Width  = 1920;
    dispSize->u32Height = 1080;
}
#if defined(CONFIG_SCREEN)
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
#endif

static HI_S32 GenerateLcdDisplayCfg(MEDIA_DispCfg *dispCfg, SIZE_S *dispSize)
{
#if defined(CONFIG_SCREEN)
    HI_S32 ret = HI_SUCCESS;
    ret = GetLcdScreenAttr(&dispCfg->dispAttr, &dispCfg->videoLayerAttr);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    dispSize->u32Width  = dispCfg->videoLayerAttr.stImageSize.u32Width;
    dispSize->u32Height = dispCfg->videoLayerAttr.stImageSize.u32Height;
    dispCfg->videoLayerAttr.u32BufLen = 3;

    dispCfg->cscAttr.enable = HI_TRUE;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Contrast = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Hue = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Luma = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.u32Satuature = 50;
    dispCfg->cscAttr.cscAttr.stVoCSC.enCscMatrix = VO_CSC_MATRIX_BT601_TO_RGB_PC;
#endif
    return HI_SUCCESS;
}

static HI_VOID GenerateDisplayCfg(PARAM_DisplayType dispType, MEDIA_VideoCfg *videoCfg, MEDIA_DispCfg *dispCfg)
{
    SIZE_S dispSize;
    if (dispType == PARAM_DISPLAY_TYPE_HDMI) {
        GenerateHdmiDisplayCfg(dispCfg, &dispSize);
    } else if (dispType == PARAM_DISPLAY_TYPE_LCD) {
        GenerateLcdDisplayCfg(dispCfg, &dispSize);
    } else {
        return;
    }

    dispCfg->enable = HI_TRUE;
    dispCfg->dispHdl = 0;

    dispCfg->videoLayerAttr.u32BufLen = 3;
    HI_U32 dispW = dispSize.u32Width;
    HI_U32 dispH = dispSize.u32Height;
    MLOGI("disp size[%ux%u]\n", dispW, dispH);

    HI_S32 dispWndCnt = MIN(CONFIG_SENSOR_CNT, HI_PDT_MEDIA_VCAP_DEV_MAX_CNT);
    HI_S32 i;
    for (i = 0; i < dispWndCnt; ++i) {
        MEDIA_DispWndCfg* dispWndCfg = &dispCfg->wndCfg[i];
        dispWndCfg->enable = HI_TRUE;
        dispWndCfg->wndHdl = i;

        if (dispType == PARAM_DISPLAY_TYPE_LCD) {
#if defined(CONFIG_SCREEN) && defined(CONFIG_VERTICAL_SCREEN)
            dispWndCfg->rotation = ROTATION_90;
#else
            dispWndCfg->rotation = ROTATION_0;
#endif
        } else {
            dispWndCfg->rotation = ROTATION_0;
        }

        if (dispWndCnt == 1) {
            dispWndCfg->wndAttr.stRect.s32X = 0;
            dispWndCfg->wndAttr.stRect.s32Y = 0;
            dispWndCfg->wndAttr.stRect.u32Width = dispW;
            dispWndCfg->wndAttr.stRect.u32Height = dispH;
        } else if (dispWndCnt > 1 && dispWndCnt <= 4) {
            if (dispWndCfg->rotation == ROTATION_90) {
                dispWndCfg->wndAttr.stRect.s32X = (dispWndCnt == 2) ? HI_APPCOMM_ALIGN(dispW / 4, 2) : 0;
                dispWndCfg->wndAttr.stRect.s32Y = HI_APPCOMM_ALIGN(dispH / 2 * (i % 2), 2);
            } else {
                dispWndCfg->wndAttr.stRect.s32X = HI_APPCOMM_ALIGN(dispW / 2 * (i % 2), 2);
                dispWndCfg->wndAttr.stRect.s32Y = (dispWndCnt == 2) ? HI_APPCOMM_ALIGN(dispH / 4, 2) : 0;
            }
            dispWndCfg->wndAttr.stRect.u32Width = HI_APPCOMM_ALIGN(dispW / 2, 2);
            dispWndCfg->wndAttr.stRect.u32Height = HI_APPCOMM_ALIGN(dispH / 2, 2);
        }
        dispWndCfg->wndAttr.u32Priority = 0;

        dispWndCfg->bindedMod = MEDIA_VIDEOMOD_VPSS;
        dispWndCfg->modHdl = videoCfg->vprocCfg.vpssAttr[i].vpssHdl;
        UpdateVpssPortAttr(dispType, dispWndCfg, &videoCfg->vprocCfg.vpssAttr[i].portAttr[0]);
        dispWndCfg->chnHdl = videoCfg->vprocCfg.vpssAttr[i].portAttr[0].portHdl;
        MLOGI("  Wnd[%d] hdl[%d] Pos[%d,%d] Size[%ux%u] Rotation[%d]\n",
            i, dispWndCfg->wndHdl, dispWndCfg->wndAttr.stRect.s32X, dispWndCfg->wndAttr.stRect.s32Y,
            dispWndCfg->wndAttr.stRect.u32Width, dispWndCfg->wndAttr.stRect.u32Height,
            dispWndCfg->rotation);
    }
}

HI_S32 HI_PARAM_GetSnsSeqCnt(HI_VOID)
{
    return sizeof(s_snsCfgs) / sizeof(s_snsCfgs[0]);
}

HI_S32 HI_PARAM_GetSnsSeqDesc(HI_S32 index, HI_CHAR *buff, HI_U32 buffLen)
{
    HI_APPCOMM_CHECK_POINTER(buff, HI_FAILURE);
    if (index < 0 || index >= HI_PARAM_GetSnsSeqCnt()) {
        MLOGE("invalid sensor seq index[%d] out of range[0, %d]\n",
            index, HI_PARAM_GetSnsSeqCnt() - 1);
        return HI_FAILURE;
    }

    PARAM_SensorConfig* snsCfg = NULL;
    HI_S32 i;
    HI_S32 buffOffset = 0;
    for (i = 0; i < MIN(CONFIG_SENSOR_CNT, HI_PDT_MEDIA_VCAP_DEV_MAX_CNT); ++i) {
        snsCfg = &s_snsCfgs[index][i];
        snprintf(buff + buffOffset, buffLen, "%s%s[%ux%u(%u) %f%s]",
            (i == 0) ? "" : " / ", snsCfg->description,
            snsCfg->size.u32Width, snsCfg->size.u32Height, snsCfg->snsMode,
            snsCfg->maxFrameRate, (snsCfg->wdrMode == WDR_MODE_NONE) ? "" : " WDR");
        buffOffset = strnlen(buff, buffLen - 1);
    }
    return HI_SUCCESS;
}

HI_S32 HI_PARAM_GetMediaCfg(HI_S32 snsIdx, PARAM_DisplayType dispType, MEDIA_Cfg *mediaCfg)
{
    HI_APPCOMM_CHECK_POINTER(mediaCfg, HI_FAILURE);
    if (snsIdx < 0 || snsIdx >= HI_PARAM_GetSnsSeqCnt()) {
        MLOGE("invalid sensor seq index[%d] out of range[0, %d]\n",
            snsIdx, HI_PARAM_GetSnsSeqCnt() - 1);
        return HI_FAILURE;
    }
    memset(mediaCfg, 0, sizeof(MEDIA_Cfg));

    /* vi vpss mode */
    GenerateViVpssMode(&mediaCfg->viVpssMode);

    HI_S32 i;
    PARAM_SensorConfig* snsCfg = NULL;
    for (i = 0; i < MIN(CONFIG_SENSOR_CNT, HI_PDT_MEDIA_VCAP_DEV_MAX_CNT); ++i) {
        snsCfg = &s_snsCfgs[snsIdx][i];
        MLOGI("snsCfg[%d]: size[%ux%u] mode[%u] maxframerate[%f] wdrmode[%d]\n",
            i, snsCfg->size.u32Width, snsCfg->size.u32Height,
            snsCfg->snsMode, snsCfg->maxFrameRate, snsCfg->wdrMode);

        /* vcap */
        mediaCfg->videoCfg.vcapCfg.devAttr[i].devHdl = i;
        mediaCfg->videoCfg.vcapCfg.devAttr[i].vcapPipeAttr[0].pipeHdl = i * 2;
        GenerateVcapAttr(snsCfg, &mediaCfg->videoCfg.vcapCfg.devAttr[i]);

        /* vpss */
        MEDIA_VpssAttr* vpssAttr = &mediaCfg->videoCfg.vprocCfg.vpssAttr[i];
        vpssAttr->vpssHdl = i;
        vpssAttr->vcapPipeHdl = i * 2;
        vpssAttr->vcapPipeChnHdl = 0;
        GenerateVpssCfg(&mediaCfg->videoCfg.vcapCfg.devAttr[i], vpssAttr);
    }

    /* display */
    GenerateDisplayCfg(dispType, &mediaCfg->videoCfg, &mediaCfg->videoOutCfg.dispCfg[0]);

    /* vb configure */
    GenerateVbCfg(mediaCfg, &mediaCfg->vbCfg);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

