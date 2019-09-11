
/**
 * @file    hi_product_media_comm.c
 * @brief   media comm source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2019/4/1
 * @version   1.0

 */

#include "hi_product_media.h"
#include "hi_appcomm_log.h"

#include "hi_mapi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_aenc.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_hdmi.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define PDT_MEDIA_CHECK_INIT(init)\
    do {\
        if (!init) {\
            MLOGE("module not init yet\n");\
            return HI_FAILURE;\
        }\
    }while(0);


typedef struct tagMEDIA_Context {
    HI_BOOL mediaInit;
    HI_BOOL videoInit;
    HI_BOOL audioInit;
    HI_BOOL videoOutInit;
    HI_BOOL audioOutInit;
    HI_BOOL dispStart;
    HI_BOOL hdmiInit;
    HI_BOOL hdmiStart;
    MEDIA_Cfg mediaCfg;
} MEDIA_Context;
static MEDIA_Context s_MediaCtx = {
    .mediaInit = HI_FALSE,
    .videoInit = HI_FALSE,
    .audioInit = HI_FALSE,
    .videoOutInit = HI_FALSE,
    .audioOutInit = HI_FALSE,
    .dispStart = HI_FALSE,
};


HI_S32 HI_PDT_MEDIA_Init(const MEDIA_ViVpssMode *viVpssMode, const MEDIA_VbCfg *vbCfg)
{
    HI_S32 ret = HI_SUCCESS;
    HI_MAPI_MEDIA_ATTR_S mediaAttr;
    memset(&mediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));
    HI_S32 i, j;
    HI_U32 s32Idx = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j) {
            mediaAttr.stMediaConfig.stVIVPSSMode.aenMode[s32Idx] = viVpssMode->mode[i][j].enMode;
            MLOGI("ViVpssMode[%d] %d\n", s32Idx, mediaAttr.stMediaConfig.stVIVPSSMode.aenMode[s32Idx]);
            s32Idx++;
        }
    }
    mediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = vbCfg->maxPoolCnt;
    for (i = 0; i < mediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt; ++i) {
        mediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u64BlkSize = vbCfg->commPool[i].blkSize;
        mediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u32BlkCnt  = vbCfg->commPool[i].blkCnt;
        MLOGI("VBPool[%d] BlkSize[%u] BlkCnt[%u]\n", i, vbCfg->commPool[i].blkSize,
              vbCfg->commPool[i].blkCnt);
    }
    mediaAttr.stVencModPara.u32H264eLowPowerMode = 1;
    mediaAttr.stVencModPara.u32H265eLowPowerMode = 1;
    ret = HI_MAPI_Media_Init(&mediaAttr);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    s_MediaCtx.mediaInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Deinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    HI_S32 ret = HI_SUCCESS;
    ret = HI_MAPI_Media_Deinit();
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    s_MediaCtx.mediaInit = HI_FALSE;
    return HI_SUCCESS;
}

static HI_S32 InitSensor(const MEDIA_VcapDevAttr *vcapDevAttr)
{
    if (!vcapDevAttr->enable) {
        MLOGD("VcapDev[%d] Disable\n", vcapDevAttr->devHdl);
        return HI_SUCCESS;
    }
    MLOGI("VcapDevHdl[%d] Size[%ux%u] SnsMode[%u] WdrMode[%d]\n",
          vcapDevAttr->devHdl, vcapDevAttr->snsAttr.stSize.u32Width,
          vcapDevAttr->snsAttr.stSize.u32Height, vcapDevAttr->snsAttr.u8SnsMode,
          vcapDevAttr->snsAttr.enWdrMode);
    HI_S32 ret = HI_SUCCESS;
    HI_MAPI_SENSOR_ATTR_S snsAttr;
    memcpy(&snsAttr, &vcapDevAttr->snsAttr, sizeof(HI_MAPI_SENSOR_ATTR_S));
    ret = HI_MAPI_VCAP_InitSensor(vcapDevAttr->devHdl, &snsAttr);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    return HI_SUCCESS;
}

static HI_S32 SwitchPixelFormat(HI_MEDIA_PIXEL_FMT_E mediaPixelFmt, PIXEL_FORMAT_E *destPixelFmt)
{
    switch (mediaPixelFmt) {
        case HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420:
            *destPixelFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            break;
        case HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_422:
            *destPixelFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_422;
            break;
        case HI_MEDIA_PIXEL_FMT_ABGR_1555:
            *destPixelFmt = PIXEL_FORMAT_ABGR_1555;
            break;
        case HI_MEDIA_PIXEL_FMT_ABGR_8888:
            *destPixelFmt = PIXEL_FORMAT_ABGR_8888;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_8BPP:
            *destPixelFmt = PIXEL_FORMAT_RGB_BAYER_8BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_10BPP:
            *destPixelFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_12BPP:
            *destPixelFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_14BPP:
            *destPixelFmt = PIXEL_FORMAT_RGB_BAYER_14BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_16BPP:
            *destPixelFmt = PIXEL_FORMAT_RGB_BAYER_16BPP;
            break;
        default:
            MLOGD("Invalid PixelFormat[%d]\n", mediaPixelFmt);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}

static HI_VOID SwitchVcapAttr(const MEDIA_VcapDevAttr *vcapDevAttr, HI_MAPI_VCAP_ATTR_S *destVcapAttr)
{
    memset(destVcapAttr, 0, sizeof(HI_MAPI_VCAP_ATTR_S));
    memcpy(&destVcapAttr->stVcapDevAttr, &vcapDevAttr->devAttr, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    HI_S32 i, j;
    HI_MAPI_VCAP_PIPE_ATTR_S *destPipeAttr = NULL;
    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++i) {
        const MEDIA_VcapPipeAttr *pipeAttr = &vcapDevAttr->vcapPipeAttr[i];
        MLOGI("Pipe[%d] Enable[%d]\n", i, pipeAttr->enable);
        if (!pipeAttr->enable) {
            continue;
        }
        destVcapAttr->aPipeIdBind[destVcapAttr->u32PipeBindNum] = pipeAttr->pipeHdl;
        destPipeAttr = &destVcapAttr->astVcapPipeAttr[pipeAttr->pipeHdl];
        memset(destPipeAttr, 0, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
        destPipeAttr->enCompressMode   = pipeAttr->compressMode;
        destPipeAttr->bIspBypass       = pipeAttr->ispBypass;
        destPipeAttr->enPipeBypassMode = pipeAttr->ispBypass ? VI_PIPE_BYPASS_BE : VI_PIPE_BYPASS_NONE;
        destPipeAttr->enPipeType       = pipeAttr->piepType;
        destPipeAttr->stFrameRate      = pipeAttr->frameRate;
        destPipeAttr->stIspPubAttr.stSize          = pipeAttr->ispPubAttr.stSize;
        destPipeAttr->stIspPubAttr.enBayer         = BAYER_BUTT;
        destPipeAttr->stIspPubAttr.f32FrameRate    = pipeAttr->ispPubAttr.f32FrameRate;
        destPipeAttr->stIspPubAttr.enSnsMirrorFlip = pipeAttr->ispPubAttr.enSnsMirrorFlip;
        destPipeAttr->stFrameIntAttr.enIntType     = FRAME_INTERRUPT_START;
        destPipeAttr->stFrameIntAttr.u32EarlyLine  = 0;
        MLOGI("  IspBypass[%d] PipeBypassMode[%d] PipeType[%d]\n",
              destPipeAttr->bIspBypass, destPipeAttr->enPipeBypassMode, destPipeAttr->enPipeType);
        HI_MAPI_PIPE_CHN_ATTR_S *destPipeChnAttr = NULL;
        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++j) {
            const MEDIA_VcapPipeChnAttr *pipeChnAttr = &pipeAttr->pipeChnAttr[j];
            if (!pipeChnAttr->enable) {
                continue;
            }
            destPipeChnAttr = &destPipeAttr->astPipeChnAttr[j];
            destPipeChnAttr->enCompressMode = pipeChnAttr->compressMode;
            SwitchPixelFormat(pipeChnAttr->pixelFormat, &destPipeChnAttr->enPixelFormat);
            destPipeChnAttr->stDestSize = pipeChnAttr->destSize;
            destPipeChnAttr->stFrameRate = pipeChnAttr->frameRate;
        }
        destVcapAttr->u32PipeBindNum++;
    }
}

static HI_S32 StartVcapDev(const MEDIA_VcapDevAttr *vcapDevAttr)
{
    if (!vcapDevAttr->enable) {
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VCAP_ATTR_S destVcapAttr;
    SwitchVcapAttr(vcapDevAttr, &destVcapAttr);
    s32Ret = HI_MAPI_VCAP_SetAttr(vcapDevAttr->devHdl, &destVcapAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_VCAP_StartDev(vcapDevAttr->devHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++i) {
        const MEDIA_VcapPipeAttr *pipeAttr = &vcapDevAttr->vcapPipeAttr[i];
        if (!pipeAttr->enable) {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++j) {
            const MEDIA_VcapPipeChnAttr *pipeChnAttr = &pipeAttr->pipeChnAttr[j];
            if (!pipeChnAttr->enable) {
                continue;
            }
            MLOGI("StartChn: DevHdl[%d] PipeHdl[%d] PipeChnHdl[%d]\n",
                  vcapDevAttr->devHdl, pipeAttr->pipeHdl, pipeAttr->pipeChnAttr[j].pipeChnHdl);
            MLOGI("  destSize[%ux%u] framerate[%d,%d] pixelfmt[%d]\n",
                  pipeAttr->pipeChnAttr[j].destSize.u32Width, pipeAttr->pipeChnAttr[j].destSize.u32Height,
                  pipeAttr->pipeChnAttr[j].frameRate.s32SrcFrameRate, pipeAttr->pipeChnAttr[j].frameRate.s32DstFrameRate,
                  pipeAttr->pipeChnAttr[j].pixelFormat);
            s32Ret = HI_MAPI_VCAP_StartChn(pipeAttr->pipeHdl, pipeAttr->pipeChnAttr[j].pipeChnHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        if (HI_TRUE != pipeAttr->ispBypass) {
            s32Ret = HI_MAPI_VCAP_StartISP(pipeAttr->pipeHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

static HI_S32 StopVcapDev(const MEDIA_VcapDevAttr *vcapDevAttr)
{
    if (!vcapDevAttr->enable) {
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++i) {
        const MEDIA_VcapPipeAttr *pipeAttr = &vcapDevAttr->vcapPipeAttr[i];
        if (!pipeAttr->enable) {
            continue;
        }
        if (HI_TRUE != pipeAttr->ispBypass) {
            s32Ret = HI_MAPI_VCAP_StopISP(pipeAttr->pipeHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++j) {
            const MEDIA_VcapPipeChnAttr *pipeChnAttr = &pipeAttr->pipeChnAttr[j];
            if (!pipeChnAttr->enable) {
                continue;
            }
            s32Ret = HI_MAPI_VCAP_StopChn(pipeAttr->pipeHdl, pipeAttr->pipeChnAttr[j].pipeChnHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    s32Ret = HI_MAPI_VCAP_StopDev(vcapDevAttr->devHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

static HI_S32 SwitchVpssAttr(const MEDIA_VpssAttr *vpssAttr, HI_MAPI_VPSS_ATTR_S *destVpssAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    memset(destVpssAttr, 0, sizeof(HI_MAPI_VPSS_ATTR_S));
    destVpssAttr->u32MaxW = vpssAttr->vpssAttr.u32MaxW;
    destVpssAttr->u32MaxH = vpssAttr->vpssAttr.u32MaxH;
    destVpssAttr->stFrameRate.s32SrcFrameRate = vpssAttr->vpssAttr.stFrameRate.s32SrcFrameRate;
    destVpssAttr->stFrameRate.s32DstFrameRate = vpssAttr->vpssAttr.stFrameRate.s32DstFrameRate;
    s32Ret = SwitchPixelFormat(vpssAttr->vpssAttr.enPixelFormat, &destVpssAttr->enPixelFormat);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    destVpssAttr->bNrEn = vpssAttr->vpssAttr.bNrEn;
    destVpssAttr->stNrAttr = vpssAttr->vpssAttr.stNrAttr;
    destVpssAttr->stNrAttr.enCompressMode = COMPRESS_MODE_NONE;
#if defined(HI3559V200)
    destVpssAttr->stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO_SPATIAL;
#endif
    MLOGI(YELLOW" VpssHdl[%d] VcapPipeHdl[%d] PipeChnHdl[%d]"NONE"\n",
          vpssAttr->vpssHdl, vpssAttr->vcapPipeHdl, vpssAttr->vcapPipeChnHdl);
    MLOGI(" MaxW[%u] MaxH[%u] srcFrmRate[%d] dstFrmRate[%d] PixedFmt[%d] bNr[%d] NrType[%d]\n",
          destVpssAttr->u32MaxW, destVpssAttr->u32MaxH, destVpssAttr->stFrameRate.s32SrcFrameRate,
          destVpssAttr->stFrameRate.s32DstFrameRate, destVpssAttr->enPixelFormat,
          destVpssAttr->bNrEn, destVpssAttr->stNrAttr.enNrType);
    return HI_SUCCESS;
}

static HI_S32 SwitchVpssPortAttr(const MEDIA_VpssPortAttr *vpssPortAttr,
                                 HI_MAPI_VPORT_ATTR_S *destVpssPortAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    destVpssPortAttr->u32Width      = vpssPortAttr->resolution.u32Width;
    destVpssPortAttr->u32Height     = vpssPortAttr->resolution.u32Height;
    destVpssPortAttr->stFrameRate   = vpssPortAttr->frameRate;
    destVpssPortAttr->enVideoFormat = vpssPortAttr->videoFormat;
    s32Ret = SwitchPixelFormat(vpssPortAttr->pixelFormat, &destVpssPortAttr->enPixelFormat);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    destVpssPortAttr->enCompressMode = COMPRESS_MODE_NONE;
    MLOGI("  Resoluton[%ux%u] srcFrmRate[%d] dstFrmRate[%d]\n",
          destVpssPortAttr->u32Width, destVpssPortAttr->u32Height,
          destVpssPortAttr->stFrameRate.s32SrcFrameRate, destVpssPortAttr->stFrameRate.s32DstFrameRate);
    MLOGI("  VideoFmt[%d] PixelFmt[%d]\n",
          destVpssPortAttr->enVideoFormat, destVpssPortAttr->enPixelFormat);
    return HI_SUCCESS;
}

static HI_S32 SetVpssPort(HI_S32 vpssHdl, const MEDIA_VpssPortAttr *vpssPortAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGI("VpssHdl[%d] portHdl[%d]\n", vpssHdl, vpssPortAttr->portHdl);
    HI_MAPI_VPORT_ATTR_S destVportAttr;
    memset(&destVportAttr, 0, sizeof(HI_MAPI_VPORT_ATTR_S));
    s32Ret = SwitchVpssPortAttr(vpssPortAttr, &destVportAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s32Ret = HI_MAPI_VPROC_SetPortAttr(vpssHdl, vpssPortAttr->portHdl, &destVportAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

static HI_S32 BindVpss(const MEDIA_VpssAttr *vpssAttr,
                       const MEDIA_VcapCfg *vcapCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        if (!vcapCfg->devAttr[i].enable) {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j) {
            if (!vcapCfg->devAttr[i].vcapPipeAttr[j].enable ||
                    (vpssAttr->vcapPipeHdl != vcapCfg->devAttr[i].vcapPipeAttr[j].pipeHdl)) {
                continue;
            }
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k) {
                const MEDIA_VcapPipeChnAttr *pipeChnAttr =
                &vcapCfg->devAttr[i].vcapPipeAttr[j].pipeChnAttr[k];
                if (pipeChnAttr->enable &&
                        (vpssAttr->vcapPipeChnHdl == pipeChnAttr->pipeChnHdl)) {
                    s32Ret = HI_MAPI_VPROC_BindVcap(vpssAttr->vcapPipeHdl,
                                                    vpssAttr->vcapPipeChnHdl, vpssAttr->vpssHdl);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 InitVpss(const MEDIA_VideoCfg *videoCfg, HI_S32 vpssIdx)
{
    const MEDIA_VpssAttr *vpssAttr = &videoCfg->vprocCfg.vpssAttr[vpssIdx];
    if (!vpssAttr->enable) {
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    /* Vpss Init */
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    s32Ret = SwitchVpssAttr(vpssAttr, &stVpssAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    /**if vpss binded vcap is not ispBypass, the vpss nr type is video*/
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++) {
        if (!videoCfg->vcapCfg.devAttr[i].enable) {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++) {
            if (!videoCfg->vcapCfg.devAttr[i].vcapPipeAttr[j].enable ||
                    (vpssAttr->vcapPipeHdl != videoCfg->vcapCfg.devAttr[i].vcapPipeAttr[j].pipeHdl)) {
                continue;
            }
            if (!videoCfg->vcapCfg.devAttr[i].vcapPipeAttr[j].ispBypass) {
                stVpssAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
            }
        }
    }
    stVpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
    MLOGI(LIGHT_BLUE"Vpss VpssHdl[%d] Nr enCompressMode[%d]\n"NONE,
          vpssAttr->vpssHdl, stVpssAttr.stNrAttr.enCompressMode);
    s32Ret = HI_MAPI_VPROC_InitVpss(vpssAttr->vpssHdl, &stVpssAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    /* Vpss Bind */
    s32Ret = BindVpss(vpssAttr, &videoCfg->vcapCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    /* VpssPort Init */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i) {
        if (!vpssAttr->portAttr[i].enable) {
            continue;
        }
        s32Ret = SetVpssPort(vpssAttr->vpssHdl, &vpssAttr->portAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s32Ret = HI_MAPI_VPROC_StartPort(vpssAttr->vpssHdl, vpssAttr->portAttr[i].portHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

static HI_S32 DeinitVpss(const MEDIA_VideoCfg *videoCfg, HI_S32 vpssIdx)
{
    const MEDIA_VpssAttr *vpssAttr = &videoCfg->vprocCfg.vpssAttr[vpssIdx];
    if (!vpssAttr->enable) {
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i) {
        if (!vpssAttr->portAttr[i].enable) {
            continue;
        }
        s32Ret = HI_MAPI_VPROC_StopPort(vpssAttr->vpssHdl, vpssAttr->portAttr[i].portHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s32Ret = HI_MAPI_VPROC_UnBindVCap(vpssAttr->vcapPipeHdl,
                                      vpssAttr->vcapPipeChnHdl, vpssAttr->vpssHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_VPROC_DeinitVpss(vpssAttr->vpssHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoInit(const MEDIA_VideoCfg *videoCfg)
{
    HI_APPCOMM_CHECK_POINTER(videoCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    if (s_MediaCtx.videoInit) {
        MLOGE("Video already Init\n");
        return HI_SUCCESS;
    }
    memcpy(&s_MediaCtx.mediaCfg.videoCfg, videoCfg, sizeof(MEDIA_VideoCfg));
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        /* Sensor */
        ret = InitSensor(&videoCfg->vcapCfg.devAttr[i]);
        HI_APPCOMM_CHECK_RETURN(ret, ret);
        /* VcapDev */
        ret = StartVcapDev(&videoCfg->vcapCfg.devAttr[i]);
        HI_APPCOMM_CHECK_RETURN(ret, ret);
    }
    /* Vproc */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i) {
        ret = InitVpss(videoCfg, i);
        HI_APPCOMM_CHECK_RETURN(ret, ret);
    }
    s_MediaCtx.videoInit = HI_TRUE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoDeinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.videoInit);
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i) {
        DeinitVpss(&s_MediaCtx.mediaCfg.videoCfg, i);
    }
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i) {
        StopVcapDev(&s_MediaCtx.mediaCfg.videoCfg.vcapCfg.devAttr[i]);
    }
    s_MediaCtx.videoInit = HI_FALSE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutInit(const MEDIA_VideoOutCfg *videoOutCfg)
{
    HI_APPCOMM_CHECK_POINTER(videoOutCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    memcpy(&s_MediaCtx.mediaCfg.videoOutCfg, videoOutCfg, sizeof(MEDIA_VideoOutCfg));
    const MEDIA_DispCfg *dispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i) {
        dispCfg = &videoOutCfg->dispCfg[i];
        if (!dispCfg->enable) {
            continue;
        }
        s32Ret = HI_MAPI_DISP_Init(dispCfg->dispHdl, (HI_MAPI_DISP_ATTR_S *)&dispCfg->dispAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_MediaCtx.videoOutInit = HI_TRUE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutDeinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.videoOutInit);
    const MEDIA_DispCfg *dispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i) {
        dispCfg = &s_MediaCtx.mediaCfg.videoOutCfg.dispCfg[i];
        if (!dispCfg->enable) {
            continue;
        }
        s32Ret = HI_MAPI_DISP_Deinit(dispCfg->dispHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_MediaCtx.videoOutInit = HI_FALSE;
    return HI_SUCCESS;
}

static HI_S32 DispWndStart(const MEDIA_DispCfg *dispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++i) {
        if (!dispCfg->wndCfg[i].enable) {
            continue;
        }
        HI_MAPI_DISP_WINDOW_ROTATE_S dispRotate = {0};
        dispRotate.enDispRorate = dispCfg->wndCfg[i].rotation;
        s32Ret = HI_MAPI_DISP_SetWindowAttrEx(dispCfg->dispHdl,
                                              dispCfg->wndCfg[i].wndHdl, HI_MAPI_DISP_WINDOW_CMD_ROTATE,
                                              (HI_VOID *)&dispRotate, sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        s32Ret = HI_MAPI_DISP_SetWindowAttr(dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl,
                                            (HI_MAPI_DISP_WINDOW_ATTR_S *)&dispCfg->wndCfg[i].wndAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        if (MEDIA_VIDEOMOD_VPSS == dispCfg->wndCfg[i].bindedMod) {
            s32Ret = HI_MAPI_DISP_Bind_VProc(dispCfg->wndCfg[i].modHdl, dispCfg->wndCfg[i].chnHdl,
                                             dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        } else if (MEDIA_VIDEOMOD_VCAP == dispCfg->wndCfg[i].bindedMod) {
            s32Ret = HI_MAPI_DISP_Bind_VCap(dispCfg->wndCfg[i].modHdl, dispCfg->wndCfg[i].chnHdl,
                                            dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        s32Ret = HI_MAPI_DISP_StartWindow(dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

static HI_S32 DispWndStop(const MEDIA_DispCfg *dispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++i) {
        if (!dispCfg->wndCfg[i].enable) {
            continue;
        }
        s32Ret = HI_MAPI_DISP_StopWindow(dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        if (MEDIA_VIDEOMOD_VPSS == dispCfg->wndCfg[i].bindedMod) {
            s32Ret = HI_MAPI_DISP_UnBind_VProc(dispCfg->wndCfg[i].modHdl, dispCfg->wndCfg[i].chnHdl,
                                               dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        } else if (MEDIA_VIDEOMOD_VCAP == dispCfg->wndCfg[i].bindedMod) {
            s32Ret = HI_MAPI_DISP_UnBind_VCap(dispCfg->wndCfg[i].modHdl, dispCfg->wndCfg[i].chnHdl,
                                              dispCfg->dispHdl, dispCfg->wndCfg[i].wndHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

static HI_VOID HdmiEventProc(HI_HDMI_EVENT_TYPE_E event, HI_VOID *privData)
{
    switch (event) {
        case HI_HDMI_EVENT_HOTPLUG:
            MLOGD("HDMI PlugIn\n");
            break;
        case HI_HDMI_EVENT_NO_PLUG:
            MLOGD("HDMI PlugOut\n");
            break;
        default:
            MLOGW("unsupport hdmi event[%x]\n", event);
            return;
    }
}

static HI_S32 InitHdmi(HI_VOID)
{
    if (s_MediaCtx.hdmiInit) {
        MLOGW("HDMI already Init\n");
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_CALLBACK_FUNC_S stHDMICb;
    stHDMICb.pfnHdmiEventCallback = HdmiEventProc;
    stHDMICb.pPrivateData = NULL;
    s32Ret = HI_MAPI_HDMI_Init(HI_HDMI_ID_0, &stHDMICb);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_MediaCtx.hdmiInit = HI_TRUE;
    return HI_SUCCESS;
}

static HI_S32 DeinitHdmi(HI_VOID)
{
    if (!s_MediaCtx.hdmiInit) {
        MLOGW("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MAPI_HDMI_Deinit(HI_HDMI_ID_0);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_MediaCtx.hdmiInit = HI_FALSE;
    return HI_SUCCESS;
}

static HI_S32 StartHdmi(HI_VOID)
{
    if (!s_MediaCtx.hdmiInit) {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_HDMI_ATTR_S hdmiAttr;
    memset(&hdmiAttr, 0, sizeof(HI_MAPI_HDMI_ATTR_S));
    hdmiAttr.bEnableHdmi = HI_TRUE;
    hdmiAttr.bEnableVideo = HI_TRUE;
    hdmiAttr.enHdmiSync = VO_OUTPUT_1080P30;
    hdmiAttr.bEnableAudio = HI_FALSE;
    hdmiAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    s32Ret = HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_0, &hdmiAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_HDMI_Start(HI_HDMI_ID_0);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_MediaCtx.hdmiStart = HI_TRUE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

static HI_S32 StopHdmi(HI_VOID)
{
    if (!s_MediaCtx.hdmiInit) {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MAPI_HDMI_Stop(HI_HDMI_ID_0);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_MediaCtx.hdmiStart = HI_FALSE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutStart(const MEDIA_VideoOutCfg *videoOutCfg)
{
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.videoOutInit);
    if (NULL == videoOutCfg) {
        videoOutCfg = &s_MediaCtx.mediaCfg.videoOutCfg;
    }
    const MEDIA_DispCfg *dispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i) {
        dispCfg = &videoOutCfg->dispCfg[i];
        if (!dispCfg->enable) {
            continue;
        }
        if (dispCfg->cscAttr.enable) {
            HI_MAPI_DISP_CSCATTREX_S dispCscAttr = {0};
            memcpy(&dispCscAttr.stVoCSC, &dispCfg->cscAttr.cscAttr.stVoCSC, sizeof(VO_CSC_S));
            s32Ret = HI_MAPI_DISP_SetAttrEx(dispCfg->dispHdl, HI_MAPI_DISP_CMD_VIDEO_CSC,
                                            (HI_VOID *)&dispCscAttr, sizeof(HI_MAPI_DISP_CSCATTREX_S));
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        s32Ret = HI_MAPI_DISP_Start(dispCfg->dispHdl,
                                    (HI_MAPI_DISP_VIDEOLAYER_ATTR_S *)&dispCfg->videoLayerAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        if (VO_INTF_HDMI == dispCfg->dispAttr.stPubAttr.enIntfType) {
            s32Ret = InitHdmi();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s32Ret = StartHdmi();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        s32Ret = DispWndStart(dispCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_MediaCtx.dispStart = HI_TRUE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutStop(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.mediaInit);
    PDT_MEDIA_CHECK_INIT(s_MediaCtx.videoOutInit);
    const MEDIA_DispCfg *dispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++) {
        dispCfg = &s_MediaCtx.mediaCfg.videoOutCfg.dispCfg[i];
        if (!dispCfg->enable) {
            continue;
        }
        s32Ret = DispWndStop(dispCfg);
        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);
        if (s_MediaCtx.hdmiStart) {
            s32Ret = StopHdmi();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s32Ret = DeinitHdmi();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        s32Ret = HI_MAPI_DISP_Stop(dispCfg->dispHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_MediaCtx.dispStart = HI_FALSE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

