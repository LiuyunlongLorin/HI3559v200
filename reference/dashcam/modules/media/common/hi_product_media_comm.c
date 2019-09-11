/**
 * @file    hi_product_media_comm.c
 * @brief   media comm source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "mpi_vpss.h"
#include "hi_product_media.h"
#include "product_media_inner.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_aenc.h"
#include "hi_mapi_ao.h"
#include "hi_appcomm_util.h"
#include "hi_hal_screen.h"
#if defined(AMP_LINUX_HUAWEILITE)
#include "product_media_msg.h"
#endif
#include "hi_timestamp.h"
#include "hi_player.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#if (defined(HI3556AV100))
#define MAX_COMPRESS_PIPE_NUM    (1)
#elif (defined(HI3559V200))
#define MAX_COMPRESS_PIPE_NUM    (2)
#else
#define MAX_COMPRESS_PIPE_NUM    (3)
#endif

#define VO_SD_DISP_MIN_BLK (3)
#define MAX_IMAGE_WIDTH    (4096)
#define MAIN_PREVIEW_VOWND (0)


#define MAX_VPSSPHYCHN_SCALE (15) /* the value of vpss phychn scale ability */

/** media information physical address */
#define PDT_MEDIA_INFO_PHY_ADDR    (CFG_PARAM_PHY_ADDR + 0x20000)

/** check init status */
#define PDT_MEDIA_CHECK_INIT(bInit)\
    do{\
        if (!bInit)\
        {\
            MLOGE("module not init yet\n");\
            return HI_PDT_MEDIA_ENOTINIT;\
        }\
    }while(0);

/** Media Vcap AE Information */
typedef struct tagPDT_MEDIA_VCAP_AE_INFO_S
{
    HI_MAPI_VCAP_EXPOSURE_INFO_S stExpInfo;
    ISP_WB_INFO_S stWBInfo;
    HI_BOOL bAESet;
} PDT_MEDIA_VCAP_AE_INFO_S;

/** Media Information, including media configure/ae info etc */
typedef struct tagPDT_MEDIA_INFO_S
{
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_BOOL bDispStart;
} PDT_MEDIA_INFO_S;

/** Media Module Context */
typedef struct tagPDT_MEDIA_CONTEXT_S
{
    HI_BOOL bMediaInit;
    HI_BOOL bVideoInit;
    HI_BOOL bAudioInit;
    HI_BOOL bVideoOutInit;
    HI_BOOL bAudioOutInit;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    pthread_mutex_t CntMutex; /**<protect as32VencStartCnt/as32AencStartCnt */
    HI_S32  as32VencStartCnt[HI_PDT_MEDIA_VENC_MAX_CNT]; /**< venc start count,
            only canbe modified by HI_PDT_MEDIA_VencStart/HI_PDT_MEDIA_VencStop */
    HI_S32  as32AencStartCnt[HI_PDT_MEDIA_AENC_MAX_CNT]; /**< aenc start count,
            only canbe modified by HI_PDT_MEDIA_AencStart/HI_PDT_MEDIA_AencStop */

    HI_BOOL bHDMIStart;
#endif

    PDT_MEDIA_INFO_S* pstMediaInfo;
    PDT_MEDIA_VCAP_AE_INFO_S astVcapAEInfo[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_BOOL bCompress;
} PDT_MEDIA_CONTEXT_S;
static PDT_MEDIA_CONTEXT_S s_stMEDIACtx =
{
    .bMediaInit = HI_FALSE,
    .bVideoInit = HI_FALSE,
    .bAudioInit = HI_FALSE,
    .bVideoOutInit = HI_FALSE,
    .bAudioOutInit = HI_FALSE,
    .bCompress = HI_TRUE
};


HI_VOID PDT_MEDIA_SwitchRcTypeAttr(const HI_MEDIA_VENC_ATTR_S* pstVencAttr,
                            HI_MAPI_VENC_RC_ATTR_S* pstVencRcAttr,
                            VENC_HIERARCHICAL_QP_S* pstHierarchicalQp)
{
#define PDT_MEDIA_HIGHFRAME_FPS (60)
    MLOGD("  Size[%ux%u] Type[%d:0-h264,1-h265,2-mjpeg,3-jpeg] BufSize[%u] Profile[%u]\n",
        pstVencAttr->stTypeAttr.u32Width, pstVencAttr->stTypeAttr.u32Height,
        pstVencAttr->stTypeAttr.enType, pstVencAttr->stTypeAttr.u32BufSize,
        pstVencAttr->stTypeAttr.u32Profile);
    memset(pstVencRcAttr, 0, sizeof(HI_MAPI_VENC_RC_ATTR_S));
    memcpy(pstHierarchicalQp, &pstVencAttr->stRcAttr.stHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));
    pstVencRcAttr->enRcMode = pstVencAttr->stRcAttr.enRcMode;
    if (HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H264_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH264Cbr;
            memcpy(&pstVencRcAttr->stAttrCbr, &pstAttrCbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H265_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH265Cbr;
            memcpy(&pstVencRcAttr->stAttrCbr, &pstAttrCbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stMjpegCbr;
            memcpy(&pstVencRcAttr->stAttrCbr, &pstAttrCbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        pstHierarchicalQp->bHierarchicalQpEn =
                ((PDT_MEDIA_HIGHFRAME_FPS <= pstVencRcAttr->stAttrCbr.fr32DstFrameRate) ? HI_TRUE : HI_FALSE);
        MLOGD("  CBR: BitRate[%u] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
            pstVencRcAttr->stAttrCbr.u32BitRate,
            pstVencRcAttr->stAttrCbr.u32SrcFrameRate, pstVencRcAttr->stAttrCbr.fr32DstFrameRate,
            pstVencRcAttr->stAttrCbr.u32Gop, pstVencRcAttr->stAttrCbr.u32StatTime);
    }
    else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H264_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH264Vbr;
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_VBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H265_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_VBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stMjpegVbr;
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_VBR_S));
        }
        pstHierarchicalQp->bHierarchicalQpEn =
                ((PDT_MEDIA_HIGHFRAME_FPS <= pstVencRcAttr->stAttrVbr.fr32DstFrameRate) ? HI_TRUE : HI_FALSE);
        MLOGD("  VBR: MaxBitRate[%u] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
            pstVencRcAttr->stAttrVbr.u32MaxBitRate,
            pstVencRcAttr->stAttrVbr.u32SrcFrameRate,pstVencRcAttr->stAttrVbr.fr32DstFrameRate,
            pstVencRcAttr->stAttrVbr.u32Gop, pstVencRcAttr->stAttrVbr.u32StatTime);
    }
    else if (HI_MAPI_VENC_RC_MODE_QVBR == pstVencAttr->stRcAttr.enRcMode)
    {
        if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H264_QVBR_S* pstAttrQVbr = &pstVencAttr->stRcAttr.unAttr.stH264QVbr;
            memcpy(&pstVencRcAttr->stAttrQVbr, &pstAttrQVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_QVBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H265_QVBR_S* pstAttrQVbr = &pstVencAttr->stRcAttr.unAttr.stH265QVbr;
            memcpy(&pstVencRcAttr->stAttrQVbr, &pstAttrQVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_QVBR_S));
        }
        pstHierarchicalQp->bHierarchicalQpEn =
                ((PDT_MEDIA_HIGHFRAME_FPS <= pstVencRcAttr->stAttrVbr.fr32DstFrameRate) ? HI_TRUE : HI_FALSE);
        MLOGD("  QVBR: MaxBitRate[%u] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
            pstVencRcAttr->stAttrVbr.u32MaxBitRate,
            pstVencRcAttr->stAttrVbr.u32SrcFrameRate,pstVencRcAttr->stAttrVbr.fr32DstFrameRate,
            pstVencRcAttr->stAttrVbr.u32Gop, pstVencRcAttr->stAttrVbr.u32StatTime);
    }
    MLOGD("HierarchicalQp: Enable[%d]\n", pstHierarchicalQp->bHierarchicalQpEn);
    MLOGD("  QPDelta[%d, %d, %d, %d]\n",
        pstHierarchicalQp->s32HierarchicalQpDelta[0], pstHierarchicalQp->s32HierarchicalQpDelta[1],
        pstHierarchicalQp->s32HierarchicalQpDelta[2], pstHierarchicalQp->s32HierarchicalQpDelta[3]);
    MLOGD("  FrameNum[%d, %d, %d, %d]\n",
        pstHierarchicalQp->s32HierarchicalFrameNum[0],pstHierarchicalQp->s32HierarchicalFrameNum[1],
        pstHierarchicalQp->s32HierarchicalFrameNum[2],pstHierarchicalQp->s32HierarchicalFrameNum[3]);
}

HI_S32 HI_PDT_MEDIA_InitSensor(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr)
{
    if (!pstVcapDevAttr->bEnable)
    {
        MLOGD("VcapDev Disable\n");
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_SENSOR_ATTR_S stSnsAttr;
    stSnsAttr.stSize.u32Width  = pstVcapDevAttr->stSnsAttr.stResolution.u32Width;
    stSnsAttr.stSize.u32Height = pstVcapDevAttr->stSnsAttr.stResolution.u32Height;
    stSnsAttr.u8SnsMode = pstVcapDevAttr->stSnsAttr.u32SnsMode;
    stSnsAttr.enWdrMode = pstVcapDevAttr->enWdrMode;
    MLOGD("VcapDevHdl[%d] Size[%ux%u] SnsMode[%u] WdrMode[%d]\n",
        pstVcapDevAttr->VcapDevHdl, stSnsAttr.stSize.u32Width, stSnsAttr.stSize.u32Height,
        stSnsAttr.u8SnsMode,  stSnsAttr.enWdrMode);
#if 0
    HI_S32 s32ModesCnt = 0;
    HI_S32 s32ModeIdx;
    HI_MAPI_SENSOR_ATTR_S* pstSensorModes = HI_NULL;

    /* Check SensorAttr Support or not */
    s32Ret = HI_MAPI_VCAP_GetSensorAllModes(VcapDevHdl, &pstSensorModes, &s32ModesCnt);
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && pstSensorModes, HI_FAILURE);
    MLOGD("ModeCnt = %d\n", s32ModesCnt);
    for (s32ModeIdx = 0; s32ModeIdx < s32ModesCnt; ++s32ModeIdx)
    {
        MLOGD(GREEN"SensorMode[%d]: Resolution[%ux%u] FrmRate[%dfps] WDR[%d:0-off] \n"NONE,
                s32ModeIdx, pstSensorModes[s32ModeIdx].stSize.u32Width, pstSensorModes[s32ModeIdx].stSize.u32Height,
                pstSensorModes[s32ModeIdx].s32FrameRate, pstSensorModes[s32ModeIdx].enWdrMode);
        if (0 == memcmp(pstSnsAttr, &pstSensorModes[s32ModeIdx], sizeof(HI_MAPI_SENSOR_ATTR_S)))
        {
            MLOGD(GREEN"SensorMode[%d]: Resolution[%ux%u] FrmRate[%dfps] WDR[%d:0-off] \n"NONE,
                s32ModeIdx, pstSnsAttr->stSize.u32Width, pstSnsAttr->stSize.u32Height,
                pstSnsAttr->s32FrameRate, pstSnsAttr->enWdrMode);
            break;
        }
    }
    HI_APPCOMM_SAFE_FREE(pstSensorModes);
    if (s32ModeIdx >= s32ModesCnt)
    {
        MLOGE(RED"Unsupport SensorMode: Resolution[%ux%u] FrmRate[%dfps] WDR[%d:0-off] "NONE,
            pstSnsAttr->stSize.u32Width, pstSnsAttr->stSize.u32Height,
            pstSnsAttr->s32FrameRate, pstSnsAttr->enWdrMode);
        return HI_PDT_MEDIA_EINVAL;
    }
#endif

    HI_PERFORMANCE_TIME_STAMP;
    /* Set Sensor Attribute */
    s32Ret = HI_MAPI_VCAP_InitSensor(pstVcapDevAttr->VcapDevHdl, &stSnsAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

static HI_VOID PDT_MEDIA_DebugVcapAttr(const HI_MAPI_VCAP_ATTR_S* pstVcapAttr)
{
    MLOGD("DevAttr: BasSize[%ux%u] WdrMode[%d] PipeBindNum[%d]\n",
        pstVcapAttr->stVcapDevAttr.stBasSize.u32Width,
        pstVcapAttr->stVcapDevAttr.stBasSize.u32Height,
        pstVcapAttr->stVcapDevAttr.enWdrMode, pstVcapAttr->u32PipeBindNum);
    HI_U32 s32PipeIdx, s32PipeChnIdx;
    for (s32PipeIdx = 0; s32PipeIdx < pstVcapAttr->u32PipeBindNum; ++s32PipeIdx)
    {
        const HI_MAPI_VCAP_PIPE_ATTR_S* pstPipeAttr =
            &pstVcapAttr->astVcapPipeAttr[pstVcapAttr->aPipeIdBind[s32PipeIdx]];
        MLOGD("Pipe[%d] Hdl[%d] CompMode[%d] IspByPass[%d] Type[%d:0-video,1-snap]\n",
            s32PipeIdx, pstVcapAttr->aPipeIdBind[s32PipeIdx],
            pstPipeAttr->enCompressMode, pstPipeAttr->bIspBypass, pstPipeAttr->enPipeType);
        MLOGD("  IspSize[%ux%u] IspFrmRate[%f] Bayer[%d:use default(%d)] FrameRate[%d->%d]\n",
            pstPipeAttr->stIspPubAttr.stSize.u32Width, pstPipeAttr->stIspPubAttr.stSize.u32Height,
            pstPipeAttr->stIspPubAttr.f32FrameRate, pstPipeAttr->stIspPubAttr.enBayer, BAYER_BUTT,
            pstPipeAttr->stFrameRate.s32SrcFrameRate, pstPipeAttr->stFrameRate.s32DstFrameRate);
        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_MAPI_PIPE_MAX_CHN_NUM; ++s32PipeChnIdx)
        {
            const HI_MAPI_PIPE_CHN_ATTR_S* pstPipeChn = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            MLOGD("  PipeChn[%d] Size[%ux%u] FrameRate[%d->%d]\n", s32PipeChnIdx,
                pstPipeChn->stDestSize.u32Width, pstPipeChn->stDestSize.u32Height,
                pstPipeChn->stFrameRate.s32SrcFrameRate, pstPipeChn->stFrameRate.s32DstFrameRate);
            MLOGD("    CompMode[%d] PixelFmt[%d]\n",
                pstPipeChn->enCompressMode, pstPipeChn->enPixelFormat);
        }
    }
}

static HI_S32 PDT_MEDIA_SwitchPixelFormat(HI_MEDIA_PIXEL_FMT_E enMediaPixelFmt, PIXEL_FORMAT_E* penPixelFmt)
{
    switch(enMediaPixelFmt)
    {
        case HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420:
            *penPixelFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            break;
        case HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_422:
            *penPixelFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_422;
            break;
        case HI_MEDIA_PIXEL_FMT_ABGR_1555:
            *penPixelFmt = PIXEL_FORMAT_ABGR_1555;
            break;
        case HI_MEDIA_PIXEL_FMT_ABGR_8888:
            *penPixelFmt = PIXEL_FORMAT_ABGR_8888;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_8BPP:
            *penPixelFmt = PIXEL_FORMAT_RGB_BAYER_8BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_10BPP:
            *penPixelFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_12BPP:
            *penPixelFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_14BPP:
            *penPixelFmt = PIXEL_FORMAT_RGB_BAYER_14BPP;
            break;
        case HI_MEDIA_PIXEL_FMT_RGB_BAYER_16BPP:
            *penPixelFmt = PIXEL_FORMAT_RGB_BAYER_16BPP;
            break;
        default:
            MLOGD("Invalid PixelFormat[%d]\n", enMediaPixelFmt);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_UpdateVcapDevAttr(HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstVcapDevAttr, HI_PDT_MEDIA_EINVAL);
    if(WDR_MODE_2To1_LINE==pstVcapDevAttr->enWdrMode)
    {
        pstVcapDevAttr->astVcapPipeAttr[1]=pstVcapDevAttr->astVcapPipeAttr[0];
        pstVcapDevAttr->astVcapPipeAttr[1].VcapPipeHdl=pstVcapDevAttr->astVcapPipeAttr[0].VcapPipeHdl+1;
        pstVcapDevAttr->astVcapPipeAttr[1].astPipeChnAttr[0].bEnable = HI_FALSE;
    }
    return HI_SUCCESS;
}

static HI_VOID PDT_MEDIA_SwitchVcapAttr(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstMediaDevAttr,
                HI_MAPI_VCAP_ATTR_S* pstVcapAttr)
{
    PDT_MEDIA_UpdateVcapDevAttr((HI_PDT_MEDIA_VCAP_DEV_ATTR_S*)pstMediaDevAttr);
    memset(pstVcapAttr, 0x0, sizeof(HI_MAPI_VCAP_ATTR_S));
    pstVcapAttr->stVcapDevAttr.stBasSize = pstMediaDevAttr->stResolution;
    pstVcapAttr->stVcapDevAttr.enWdrMode = pstMediaDevAttr->enWdrMode;
    pstVcapAttr->stVcapDevAttr.u32CacheLine = (WDR_MODE_2To1_LINE==pstMediaDevAttr->enWdrMode)?(pstMediaDevAttr->stResolution.u32Height/2):pstMediaDevAttr->stResolution.u32Height;

    HI_S32 s32PipeIdx, s32PipeChnIdx;
    HI_MAPI_VCAP_PIPE_ATTR_S* pstMapiPipeAttr = NULL;
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstPipeAttr = &pstMediaDevAttr->astVcapPipeAttr[s32PipeIdx];
        MLOGD("Pipe[%d] Enable[%d]\n", s32PipeIdx, pstPipeAttr->bEnable);
        if (!pstPipeAttr->bEnable)
        {
            continue;
        }

        pstVcapAttr->aPipeIdBind[pstVcapAttr->u32PipeBindNum] = pstPipeAttr->VcapPipeHdl;

        pstMapiPipeAttr = &pstVcapAttr->astVcapPipeAttr[pstPipeAttr->VcapPipeHdl];
        memset(pstMapiPipeAttr,0,sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
        pstMapiPipeAttr->enCompressMode = COMPRESS_MODE_NONE;
        if(s_stMEDIACtx.bCompress)
        {
            if((HI_TRUE != pstPipeAttr->bIspBypass) && (pstPipeAttr->VcapPipeHdl < MAX_COMPRESS_PIPE_NUM))
            {
                pstMapiPipeAttr->enCompressMode = COMPRESS_MODE_LINE;
                MLOGD(LIGHT_BLUE"Vcap Pipe[%d] enCompressMode[%d]\n"NONE,
                    pstPipeAttr->VcapPipeHdl, pstMapiPipeAttr->enCompressMode);
            }
        }
        pstMapiPipeAttr->bIspBypass = pstPipeAttr->bIspBypass;
        pstMapiPipeAttr->enPipeBypassMode = pstPipeAttr->bIspBypass?VI_PIPE_BYPASS_BE:VI_PIPE_BYPASS_NONE;
        pstMapiPipeAttr->enPipeType = pstPipeAttr->enPipeType;
        pstMapiPipeAttr->stFrameRate = pstPipeAttr->stFrameRate;
        pstMapiPipeAttr->stIspPubAttr.stSize = pstPipeAttr->stIspPubAttr.stSize;
        pstMapiPipeAttr->stIspPubAttr.enBayer = BAYER_BUTT;
        pstMapiPipeAttr->stIspPubAttr.f32FrameRate = pstPipeAttr->stIspPubAttr.f32FrameRate;
        pstMapiPipeAttr->stFrameIntAttr.enIntType = pstPipeAttr->stFrameIntAttr.enIntType;
        pstMapiPipeAttr->stFrameIntAttr.u32EarlyLine = pstPipeAttr->stFrameIntAttr.u32EarlyLine;
        HI_MAPI_PIPE_CHN_ATTR_S* pstMapiPipeChnAttr = NULL;
        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            if (!pstPipeChnAttr->bEnable)
            {
                continue;
            }
            pstMapiPipeChnAttr = &pstMapiPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            pstMapiPipeChnAttr->enCompressMode = COMPRESS_MODE_NONE;
            if((s_stMEDIACtx.bCompress) && (0 == s32PipeChnIdx) &&
               (ROTATION_0 == pstPipeChnAttr->enRotate)
#if defined(HI3559V200) || defined(HI3518EV300)
               /**for vpss in&out reuse*/
               && (pstPipeChnAttr->stDestResolution.u32Width > 2048)
#endif
               )
            {
                pstMapiPipeChnAttr->enCompressMode = COMPRESS_MODE_SEG;
                MLOGD(LIGHT_BLUE"Vcap Pipe[%d] Chn[%d] enCompressMode[%d]\n"NONE,
                    s32PipeIdx, s32PipeChnIdx, pstMapiPipeChnAttr->enCompressMode);
            }
            PDT_MEDIA_SwitchPixelFormat(pstPipeChnAttr->enPixelFormat, &pstMapiPipeChnAttr->enPixelFormat);
            pstMapiPipeChnAttr->stDestSize = pstPipeChnAttr->stDestResolution;
            pstMapiPipeChnAttr->stFrameRate = pstPipeChnAttr->stFrameRate;
        }

        pstVcapAttr->u32PipeBindNum++;
    }
    PDT_MEDIA_DebugVcapAttr(pstVcapAttr);
}

static HI_S32 PDT_MEDIA_StartVcapDev(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr)
{
    if (!pstVcapDevAttr->bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    PDT_MEDIA_SwitchVcapAttr(pstVcapDevAttr, &stVcapAttr);

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VCAP_SetAttr(pstVcapDevAttr->VcapDevHdl, &stVcapAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VCAP_StartDev(pstVcapDevAttr->VcapDevHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_S32 s32DevIdx = 0;
    HI_S32 s32PipeIdx, s32PipeChnIdx;
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstPipeAttr = &pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx];
        if (!pstPipeAttr->bEnable)
        {
            continue;
        }

        MLOGD("StartChn: DevHdl[%d] PipeHdl[%d] PipeChnHdl[%d]\n",
            pstVcapDevAttr->VcapDevHdl, pstPipeAttr->VcapPipeHdl, pstPipeAttr->astPipeChnAttr[0].PipeChnHdl);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VCAP_StartChn(pstPipeAttr->VcapPipeHdl, pstPipeAttr->astPipeChnAttr[0].PipeChnHdl);

        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChn = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            if (!pstPipeChn->bEnable)
            {
                continue;
            }

            if (s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet
                && HI_MAPI_PIPE_TYPE_VIDEO == pstPipeAttr->enPipeType)
            {
                PDT_MEDIA_VCAP_AE_INFO_S* pstAEInfo = &s_stMEDIACtx.astVcapAEInfo[s32DevIdx];
                ISP_INIT_ATTR_S stIspInitAttr;
                stIspInitAttr.u32Exposure      = pstAEInfo->stExpInfo.u32Exposure;
                stIspInitAttr.u32AGain         = pstAEInfo->stExpInfo.u32AGain;
                stIspInitAttr.u32DGain         = pstAEInfo->stExpInfo.u32DGain;
                stIspInitAttr.u32ISPDGain      = pstAEInfo->stExpInfo.u32ISPDGain;
                stIspInitAttr.u32ExpTime       = pstAEInfo->stExpInfo.u32ExpTime;
                stIspInitAttr.u32LinesPer500ms = pstAEInfo->stExpInfo.u32LinesPer500ms;
                stIspInitAttr.u32PirisFNO      = pstAEInfo->stExpInfo.u32PirisFNO;
                stIspInitAttr.u16WBBgain       = pstAEInfo->stWBInfo.u16Bgain;
                stIspInitAttr.u16WBRgain       = pstAEInfo->stWBInfo.u16Rgain;
                stIspInitAttr.u16WBGgain       =
                    (pstAEInfo->stWBInfo.u16Grgain + pstAEInfo->stWBInfo.u16Gbgain) >> 1;
                stIspInitAttr.u16SampleBgain   = 0;
                stIspInitAttr.u16SampleRgain   = 0;
                MLOGI(YELLOW"Pipe[%d] Exposure[%u] AGain[%u] DGain[%u] ISPDGain[%u] ExpTime[%u] LinesPer500ms[%u] PirisFNO[%u]"NONE"\n",
                    pstPipeAttr->VcapPipeHdl, stIspInitAttr.u32Exposure, stIspInitAttr.u32AGain,
                    stIspInitAttr.u32DGain, stIspInitAttr.u32ISPDGain,
                    stIspInitAttr.u32ExpTime, stIspInitAttr.u32LinesPer500ms,
                    stIspInitAttr.u32PirisFNO);
                MLOGI(YELLOW"WBBgain[%u] WBRgain[%u] WBGgain[%u]"NONE"\n", stIspInitAttr.u16WBBgain,
                    stIspInitAttr.u16WBRgain, stIspInitAttr.u16WBGgain);

                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl, HI_MAPI_VCAP_CMD_ISP_Set3AInit,
                            &stIspInitAttr, sizeof(ISP_INIT_ATTR_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
            }

            if (pstPipeChn->enRotate!=ROTATION_0)
            {
                ROTATION_E enRotate = pstPipeChn->enRotate;
                MLOGI(LIGHT_BLUE"enRotate[%d]\n"NONE,enRotate);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl, HI_MAPI_VCAP_CMD_Rotate,
                    &enRotate, sizeof(ROTATION_E));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            if (pstPipeChn->bFlip||pstPipeChn->bMirror)
            {
                HI_MAPI_VCAP_MIRRORFLIP_ATTR_S stMirrorFlipAttr;
                stMirrorFlipAttr.bFlip = pstPipeChn->bFlip;
                stMirrorFlipAttr.bMirror = pstPipeChn->bMirror;

                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl, HI_MAPI_VCAP_CMD_MirrorFlip,
                    &stMirrorFlipAttr, sizeof(HI_MAPI_VCAP_MIRRORFLIP_ATTR_S));
                    HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }

        if(HI_TRUE != pstPipeAttr->bIspBypass)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StartISP(pstPipeAttr->VcapPipeHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if (0 < strnlen((const HI_CHAR*)pstVcapDevAttr->stExifInfo.au8ImageDescription, HI_MAPI_EXIF_DRSCRIPTION_LENGTH))
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetExifInfo(pstPipeAttr->VcapPipeHdl,(HI_MAPI_SNAP_EXIF_INFO_S*)&pstVcapDevAttr->stExifInfo);
                HI_PERFORMANCE_TIME_STAMP;
                /**WDR 2to1 need not set pipe 1,ignore this fail*/
                //HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
    }
    s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet = HI_FALSE;

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_StopVcapDev(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr, HI_BOOL bStopIsp)
{
    if (!pstVcapDevAttr->bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DevIdx = 0;
    HI_S32 s32PipeIdx, s32PipeChnIdx;
    HI_HANDLE VcapPipeHdl_WdrMaster = 0;
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstPipeAttr = &pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx];
        if (!pstPipeAttr->bEnable)
        {
            continue;
        }
        if (HI_TRUE != pstPipeAttr->bIspBypass)
        {

            if (!s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet &&
                HI_MAPI_PIPE_TYPE_VIDEO == pstPipeAttr->enPipeType)
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_GetAttrEx(pstPipeAttr->VcapPipeHdl, pstPipeAttr->astPipeChnAttr[0].PipeChnHdl, HI_MAPI_VCAP_CMD_ISP_GetExposureInfo,
                    &s_stMEDIACtx.astVcapAEInfo[s32DevIdx].stExpInfo, sizeof(HI_MAPI_VCAP_EXPOSURE_INFO_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_GetAttrEx(pstPipeAttr->VcapPipeHdl, pstPipeAttr->astPipeChnAttr[0].PipeChnHdl, HI_MAPI_VCAP_CMD_ISP_GetAwbInfo,
                    &s_stMEDIACtx.astVcapAEInfo[s32DevIdx].stWBInfo, sizeof(ISP_WB_INFO_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet = HI_TRUE;
            }

            if (bStopIsp)
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_StopISP(pstPipeAttr->VcapPipeHdl);
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChn = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            if (!pstPipeChn->bEnable)
            {
                continue;
            }
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StopChn(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            VcapPipeHdl_WdrMaster = pstPipeAttr->VcapPipeHdl;
        }

        if (WDR_MODE_2To1_LINE==pstVcapDevAttr->enWdrMode&&VcapPipeHdl_WdrMaster!=pstPipeAttr->VcapPipeHdl)
        {
            MLOGI("HI_MAPI_VCAP_StopChn(%d,0) WDR slave pipe\n",pstPipeAttr->VcapPipeHdl);
            s32Ret = HI_MAPI_VCAP_StopChn(pstPipeAttr->VcapPipeHdl, 0);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VCAP_StopDev(pstVcapDevAttr->VcapDevHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_SwitchVpssAttr(const HI_PDT_MEDIA_VPSS_ATTR_S* pstMediaVpssAttr,
                        HI_MAPI_VPSS_ATTR_S* pstVpssAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    memset(pstVpssAttr, 0, sizeof(HI_MAPI_VPSS_ATTR_S));
    pstVpssAttr->u32MaxW = pstMediaVpssAttr->stVpssAttr.u32MaxW;
    pstVpssAttr->u32MaxH = pstMediaVpssAttr->stVpssAttr.u32MaxH;
    pstVpssAttr->stFrameRate.s32SrcFrameRate = pstMediaVpssAttr->stVpssAttr.stFrameRate.s32SrcFrameRate;
    pstVpssAttr->stFrameRate.s32DstFrameRate = pstMediaVpssAttr->stVpssAttr.stFrameRate.s32DstFrameRate;
    s32Ret = PDT_MEDIA_SwitchPixelFormat(pstMediaVpssAttr->stVpssAttr.enPixelFormat,
                &pstVpssAttr->enPixelFormat);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstVpssAttr->bNrEn = pstMediaVpssAttr->stVpssAttr.bNrEn;
    pstVpssAttr->stNrAttr = pstMediaVpssAttr->stVpssAttr.stNrAttr;
    pstVpssAttr->stNrAttr.enCompressMode = COMPRESS_MODE_NONE;
#if defined(HI3559V200) || defined(HI3518EV300)
    pstVpssAttr->stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO_SPATIAL;
#endif
    MLOGD(YELLOW" VpssHdl[%d] VcapPipeHdl[%d] PipeChnHdl[%d]"NONE"\n",
        pstMediaVpssAttr->VpssHdl, pstMediaVpssAttr->VcapPipeHdl, pstMediaVpssAttr->VcapPipeChnHdl);
    MLOGD(" MaxW[%u] MaxH[%u] srcFrmRate[%d] dstFrmRate[%d] PixedFmt[%d] bNr[%d] NrType[%d]\n",
        pstVpssAttr->u32MaxW, pstVpssAttr->u32MaxH, pstVpssAttr->stFrameRate.s32SrcFrameRate,
        pstVpssAttr->stFrameRate.s32DstFrameRate, pstVpssAttr->enPixelFormat, pstVpssAttr->bNrEn, pstVpssAttr->stNrAttr.enNrType);
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_SwitchVpssPortAttr(const HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstMediaVpssPortAttr,
                        HI_MAPI_VPORT_ATTR_S* pstVpssPortAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    pstVpssPortAttr->u32Width      = pstMediaVpssPortAttr->stResolution.u32Width;
    pstVpssPortAttr->u32Height     = pstMediaVpssPortAttr->stResolution.u32Height;
    pstVpssPortAttr->stFrameRate.s32SrcFrameRate = pstMediaVpssPortAttr->stFrameRate.s32SrcFrameRate;
    pstVpssPortAttr->stFrameRate.s32DstFrameRate = pstMediaVpssPortAttr->stFrameRate.s32DstFrameRate;
    pstVpssPortAttr->enVideoFormat = pstMediaVpssPortAttr->enVideoFormat;
    s32Ret = PDT_MEDIA_SwitchPixelFormat(pstMediaVpssPortAttr->enPixelFormat,
            &pstVpssPortAttr->enPixelFormat);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    pstVpssPortAttr->enCompressMode = COMPRESS_MODE_NONE;
    MLOGD("  Resoluton[%ux%u] srcFrmRate[%d] dstFrmRate[%d]\n",
        pstVpssPortAttr->u32Width, pstVpssPortAttr->u32Height,
        pstVpssPortAttr->stFrameRate.s32SrcFrameRate, pstVpssPortAttr->stFrameRate.s32DstFrameRate);
    MLOGD("  VideoFmt[%d] PixelFmt[%d]\n",
        pstVpssPortAttr->enVideoFormat, pstVpssPortAttr->enPixelFormat);
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_SetVport(HI_S32 VpssHdl, HI_U32 u32VpssInputWidth,HI_BOOL bNrEn,
                    const HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstMediaVpssPortAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MAPI_VPORT_ATTR_S stVportAttr;
    MLOGD("VpssHdl[%d] portHdl[%d]\n", VpssHdl, pstMediaVpssPortAttr->VportHdl);
    memset(&stVportAttr, 0, sizeof(HI_MAPI_VPORT_ATTR_S));
    s32Ret = PDT_MEDIA_SwitchVpssPortAttr(pstMediaVpssPortAttr, &stVportAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if((s_stMEDIACtx.bCompress) && (0 == pstMediaVpssPortAttr->VportHdl))
    {
#if defined(HI3559V200) || defined(HI3518EV300)
        if (u32VpssInputWidth > 3360)
        {
            stVportAttr.enCompressMode = COMPRESS_MODE_NONE;
        }
        else
        {
            stVportAttr.enCompressMode = COMPRESS_MODE_SEG;
        }

        if (stVportAttr.u32Width <= 2048)
        {
            stVportAttr.enCompressMode = COMPRESS_MODE_NONE;
        }
#else
        stVportAttr.enCompressMode = COMPRESS_MODE_SEG;
#endif

        MLOGD(LIGHT_BLUE"Vpss VpssHdl[%d] port[%d] enCompressMode[%d]\n"NONE,
            VpssHdl, pstMediaVpssPortAttr->VportHdl, stVportAttr.enCompressMode);
    }

    /**Vpss Port 0 can support Buffer share, if not meet requirements, SDK will block it*/
    if (pstMediaVpssPortAttr->bSupportBufferShare && bNrEn)
    {
        stVportAttr.bSupportBufferShare = 1;
    }
    else
    {
        stVportAttr.bSupportBufferShare = 0;
    }

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_SetPortAttr(VpssHdl, pstMediaVpssPortAttr->VportHdl, &stVportAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("  Rotate[%d] Mirror[%d] Flip[%d]\n", pstMediaVpssPortAttr->enRotate,
        pstMediaVpssPortAttr->bMirror, pstMediaVpssPortAttr->bFlip);
    ROTATION_E enRotate = pstMediaVpssPortAttr->enRotate;
    if (ROTATION_0!=enRotate)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, pstMediaVpssPortAttr->VportHdl,
            HI_VPROC_CMD_PortRotate, &enRotate, sizeof(ROTATION_E));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    if (pstMediaVpssPortAttr->bMirror && pstMediaVpssPortAttr->bFlip && stVportAttr.bSupportBufferShare)
    {
        ROTATION_E enRotate = ROTATION_180;
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, pstMediaVpssPortAttr->VportHdl,
            HI_VPROC_CMD_PortRotate, &enRotate, sizeof(ROTATION_E));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {

        HI_MAPI_VPROC_MIRROR_ATTR_S stVProcMirror;
        stVProcMirror.bEnable = pstMediaVpssPortAttr->bMirror;
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, pstMediaVpssPortAttr->VportHdl,
            HI_VPROC_CMD_PortMirror, &stVProcMirror, sizeof(HI_MAPI_VPROC_MIRROR_ATTR_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_MAPI_VPROC_FLIP_ATTR_S stVProcFlip;
    stVProcFlip.bEnable = pstMediaVpssPortAttr->bFlip;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, pstMediaVpssPortAttr->VportHdl,
        HI_VPROC_CMD_PortFlip, &stVProcFlip, sizeof(HI_MAPI_VPROC_FLIP_ATTR_S));
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_BindVpss(const HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr,
                    const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfg, HI_U32* pu32VpssInputWidth)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVcapCfg->astVcapDevAttr[i].bEnable)
        {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].bEnable
                || (pstVpssAttr->VcapPipeHdl != pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl))
            {
                continue;
            }
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr =
                    &pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k];
                if (pstPipeChnAttr->bEnable
                    && (pstVpssAttr->VcapPipeChnHdl == pstPipeChnAttr->PipeChnHdl))
                {
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VPROC_BindVcap(pstVpssAttr->VcapPipeHdl,
                        pstVpssAttr->VcapPipeChnHdl, pstVpssAttr->VpssHdl);
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                    *pu32VpssInputWidth = pstPipeChnAttr->stDestResolution.u32Width;
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_InitVpss(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg, HI_S32 s32VpssIdx)
{
    const HI_PDT_MEDIA_VPSS_ATTR_S* pstMediaVpssAttr = &pstVideoCfg->stVprocCfg.astVpssAttr[s32VpssIdx];
    if (!pstMediaVpssAttr->bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32VpssInputWidth = 1920;
    HI_S32 i, j;

    /* Vpss Init */
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    s32Ret = PDT_MEDIA_SwitchVpssAttr(pstMediaVpssAttr, &stVpssAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /**if vpss binded vcap is not ispBypass, the vpss nr type is video*/
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if (!pstVideoCfg->stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if (!pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable
                || (pstMediaVpssAttr->VcapPipeHdl != pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl))
            {
                continue;
            }

            if (!pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bIspBypass)
            {
                stVpssAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
            }
        }
    }

    stVpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
    MLOGD(LIGHT_BLUE"Vpss VpssHdl[%d] Nr enCompressMode[%d]\n"NONE,
        pstMediaVpssAttr->VpssHdl, stVpssAttr.stNrAttr.enCompressMode);
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_InitVpss(pstMediaVpssAttr->VpssHdl, &stVpssAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /* Vpss Bind */
    s32Ret = PDT_MEDIA_BindVpss(pstMediaVpssAttr, &pstVideoCfg->stVcapCfg, &u32VpssInputWidth);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* VpssPort Init */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i)
    {
        if (!pstMediaVpssAttr->astVportAttr[i].bEnable)
        {
            continue;
        }
        s32Ret = PDT_MEDIA_SetVport(pstMediaVpssAttr->VpssHdl, u32VpssInputWidth,stVpssAttr.bNrEn,
            &pstMediaVpssAttr->astVportAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#ifdef CONFIG_MOTIONDETECT_ON
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_StartPort(pstMediaVpssAttr->VpssHdl, pstMediaVpssAttr->astVportAttr[i].VportHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_DeinitVpss(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg, HI_S32 s32VpssIdx)
{
    const HI_PDT_MEDIA_VPSS_ATTR_S* pstMediaVpssAttr = &pstVideoCfg->stVprocCfg.astVpssAttr[s32VpssIdx];
    if (!pstMediaVpssAttr->bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i)
    {
        if (!pstMediaVpssAttr->astVportAttr[i].bEnable)
        {
            continue;
        }
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_StopPort(pstMediaVpssAttr->VpssHdl,
                pstMediaVpssAttr->astVportAttr[i].VportHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_UnBindVCap(pstMediaVpssAttr->VcapPipeHdl,
            pstMediaVpssAttr->VcapPipeChnHdl, pstMediaVpssAttr->VpssHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_DeinitVpss(pstMediaVpssAttr->VpssHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}


static HI_S32 PDT_MEDIA_BindVenc(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg,
                            const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_INVALID_HANDLE != pstVencCfg->ModHdl && HI_INVALID_HANDLE != pstVencCfg->ChnHdl)
    {
        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            /* Venc Bind */
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_BindVProc(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, pstVencCfg->VencHdl, HI_FALSE);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            /**start vpssport early to reduce vb use*/
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VPROC_StartPort(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_BindVCap(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, pstVencCfg->VencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            MLOGE("Invalid Binded Module[%d]\n", pstVencCfg->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_InitVenc(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg,
                                 const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg,
                                 HI_S32 s32VencIdx)
{
    if (!pstVencCfg->bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    VENC_HIERARCHICAL_QP_S stHierarchicalQp;

    /* Venc Init */
    MLOGD("Venc[%d] Hdl[%d]\n", s32VencIdx, pstVencCfg->VencHdl);
    memcpy(&stVencAttr.stVencPloadTypeAttr, &pstVencCfg->stVencAttr.stTypeAttr, sizeof(HI_MAPI_VENC_TYPE_ATTR_S));
    PDT_MEDIA_SwitchRcTypeAttr(&pstVencCfg->stVencAttr ,&stVencAttr.stRcAttr, &stHierarchicalQp);

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VENC_Init(pstVencCfg->VencHdl, &stVencAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /* Venc RcParam QP */
    VENC_RC_PARAM_S stRcParam;
    if (HI_MAPI_PAYLOAD_TYPE_H265 == stVencAttr.stVencPloadTypeAttr.enType)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_GetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_H265_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (HI_MAPI_VENC_RC_MODE_CBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H265_CBR_S* pstAttrCbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH265Cbr;
            stRcParam.stParamH265Cbr.u32MaxQp = pstAttrCbr->u32MaxQp;
            stRcParam.stParamH265Cbr.u32MinQp = pstAttrCbr->u32MinQp;
            stRcParam.stParamH265Cbr.u32MaxIQp = pstAttrCbr->u32MaxIQp;
            stRcParam.stParamH265Cbr.u32MinIQp = pstAttrCbr->u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H265_VBR_S* pstAttrVbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH265Vbr;
            stRcParam.stParamH265Vbr.u32MaxQp = pstAttrVbr->u32MaxQp;
            stRcParam.stParamH265Vbr.u32MinQp = pstAttrVbr->u32MinQp;
            stRcParam.stParamH265Vbr.u32MaxIQp = pstAttrVbr->u32MaxIQp;
            stRcParam.stParamH265Vbr.u32MinIQp = pstAttrVbr->u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_QVBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H265_QVBR_S* pstAttrQvbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH265QVbr;
            stRcParam.stParamH265QVbr.u32MaxQp = pstAttrQvbr->u32MaxQp;
            stRcParam.stParamH265QVbr.u32MinQp = pstAttrQvbr->u32MinQp;
            stRcParam.stParamH265QVbr.u32MaxIQp = pstAttrQvbr->u32MaxIQp;
            stRcParam.stParamH265QVbr.u32MinIQp = pstAttrQvbr->u32MinIQp;
            stRcParam.stParamH265QVbr.s32BitPercentUL = pstAttrQvbr->s32BitPercentUL;
            stRcParam.stParamH265QVbr.s32BitPercentLL = pstAttrQvbr->s32BitPercentLL;
            stRcParam.stParamH265QVbr.s32PsnrFluctuateUL = pstAttrQvbr->s32PsnrFluctuateUL;
            stRcParam.stParamH265QVbr.s32PsnrFluctuateLL = pstAttrQvbr->s32PsnrFluctuateLL;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_H265_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_HIERARCHICAL_QP,
            &stHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (HI_MAPI_PAYLOAD_TYPE_H264 == stVencAttr.stVencPloadTypeAttr.enType)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_GetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_H264_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (HI_MAPI_VENC_RC_MODE_CBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_CBR_S* pstAttrCbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH264Cbr;
            stRcParam.stParamH264Cbr.u32MaxQp = pstAttrCbr->u32MaxQp;
            stRcParam.stParamH264Cbr.u32MinQp = pstAttrCbr->u32MinQp;
            stRcParam.stParamH264Cbr.u32MaxIQp = pstAttrCbr->u32MaxIQp;
            stRcParam.stParamH264Cbr.u32MinIQp = pstAttrCbr->u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_VBR_S* pstAttrVbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH264Vbr;
            stRcParam.stParamH264Vbr.u32MaxQp = pstAttrVbr->u32MaxQp;
            stRcParam.stParamH264Vbr.u32MinQp = pstAttrVbr->u32MinQp;
            stRcParam.stParamH264Vbr.u32MaxIQp = pstAttrVbr->u32MaxIQp;
            stRcParam.stParamH264Vbr.u32MinIQp = pstAttrVbr->u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_QVBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_H264_QVBR_S* pstAttrQvbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stH264QVbr;
            stRcParam.stParamH264QVbr.u32MaxQp = pstAttrQvbr->u32MaxQp;
            stRcParam.stParamH264QVbr.u32MinQp = pstAttrQvbr->u32MinQp;
            stRcParam.stParamH264QVbr.u32MaxIQp = pstAttrQvbr->u32MaxIQp;
            stRcParam.stParamH264QVbr.u32MinIQp = pstAttrQvbr->u32MinIQp;
            stRcParam.stParamH264QVbr.s32BitPercentUL = pstAttrQvbr->s32BitPercentUL;
            stRcParam.stParamH264QVbr.s32BitPercentLL = pstAttrQvbr->s32BitPercentLL;
            stRcParam.stParamH264QVbr.s32PsnrFluctuateUL = pstAttrQvbr->s32PsnrFluctuateUL;
            stRcParam.stParamH264QVbr.s32PsnrFluctuateLL = pstAttrQvbr->s32PsnrFluctuateLL;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_H264_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_HIERARCHICAL_QP,
            &stHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == stVencAttr.stVencPloadTypeAttr.enType)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_GetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (HI_MAPI_VENC_RC_MODE_CBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_CBR_S* pstAttrCbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stMjpegCbr;
            stRcParam.stParamMjpegCbr.u32MaxQfactor = pstAttrCbr->u32MaxQfactor;
            stRcParam.stParamMjpegCbr.u32MinQfactor = pstAttrCbr->u32MinQfactor;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == stVencAttr.stRcAttr.enRcMode)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstAttrVbr =
                &pstVencCfg->stVencAttr.stRcAttr.unAttr.stMjpegVbr;
            stRcParam.stParamMjpegVbr.u32MaxQfactor = pstAttrVbr->u32MaxQfactor;
            stRcParam.stParamMjpegVbr.u32MinQfactor = pstAttrVbr->u32MinQfactor;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstVencCfg->VencHdl, HI_MAPI_VENC_CMD_H264_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    /* reset venc start count */
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    s_stMEDIACtx.as32VencStartCnt[s32VencIdx] = 0;
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

    s32Ret = PDT_MEDIA_BindVenc(pstVideoCfg, pstVencCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_DeinitVenc(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg, HI_S32 s32VencIdx)
{
    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = &pstVideoCfg->astVencCfg[s32VencIdx];
    if (!pstVencCfg->bEnable)
    {
        return HI_SUCCESS;
    }
    MLOGD("VencHdl[%d] deinit\n", pstVencCfg->VencHdl);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    s_stMEDIACtx.as32VencStartCnt[s32VencIdx] = 0;
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VENC_Stop(pstVencCfg->VencHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_UnBindVProc(pstVencCfg->ModHdl,
            pstVencCfg->ChnHdl, pstVencCfg->VencHdl, HI_FALSE);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_UnBindVCap(pstVencCfg->ModHdl,
            pstVencCfg->ChnHdl, pstVencCfg->VencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        MLOGE("Invalid Binded Module[%d]\n", pstVencCfg->enBindedMod);
        return HI_PDT_MEDIA_EINVAL;
    }

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VENC_Deinit(pstVencCfg->VencHdl);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

#ifdef CONFIG_SCREEN
static HI_S32 PDT_MEDIA_GetAspectRatio(const SIZE_S* pstDispSize, const SIZE_S* pstSrcSize, RECT_S* pstDispVideoRect)
{
    HI_APPCOMM_CHECK_POINTER(pstSrcSize, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDispSize, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDispVideoRect, HI_PDT_MEDIA_EINVAL);
    /** Wsrc/Hsrc < Wdisp/Hdisp : Left/Right with Backgroud Edge */
    if (pstSrcSize->u32Width * pstDispSize->u32Height < pstSrcSize->u32Height * pstDispSize->u32Width
        && 0!=pstSrcSize->u32Height)
    {
        pstDispVideoRect->u32Width  = pstSrcSize->u32Width * pstDispSize->u32Height / pstSrcSize->u32Height;

        pstDispVideoRect->s32X =
            HI_APPCOMM_ALIGN(pstDispSize->u32Width - pstDispVideoRect->u32Width, 4) >> 1;
        pstDispVideoRect->s32Y = 0;
        pstDispVideoRect->u32Width = pstDispSize->u32Width - ((HI_U32)pstDispVideoRect->s32X << 1);
        pstDispVideoRect->u32Height = pstDispSize->u32Height;
    }
    /** Wsrc/Hsrc > Wdisp/Hdisp : Top/Bottom with Backgroud Edge */
    else if (pstSrcSize->u32Width * pstDispSize->u32Height > pstSrcSize->u32Height * pstDispSize->u32Width
        && 0!=pstSrcSize->u32Width)
    {
        pstDispVideoRect->u32Height = pstSrcSize->u32Height * pstDispSize->u32Width / pstSrcSize->u32Width;

        pstDispVideoRect->s32X = 0;
        pstDispVideoRect->s32Y =
            HI_APPCOMM_ALIGN((pstDispSize->u32Height - pstDispVideoRect->u32Height), 4) >> 1;
        pstDispVideoRect->u32Width  = pstDispSize->u32Width;
        pstDispVideoRect->u32Height= pstDispSize->u32Height - ((HI_U32)pstDispVideoRect->s32Y << 1);
    }
    else
    {
        pstDispVideoRect->s32X = 0;
        pstDispVideoRect->s32Y = 0;
        pstDispVideoRect->u32Width  = pstDispSize->u32Width;
        pstDispVideoRect->u32Height = pstDispSize->u32Height;
    }
    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_MEDIA_GetVpssIndex(const HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg,
                                     HI_U32 VpssHdl, HI_U32 VportHdl, HI_U32* u32VpssHdlIndex, HI_U32* u32VportHdlIndex)
{
    HI_APPCOMM_CHECK_POINTER(pstVprocCfg, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(u32VpssHdlIndex, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(u32VportHdlIndex, HI_PDT_MEDIA_EINVAL);

    HI_S32 i,j;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        if (!pstVprocCfg->astVpssAttr[i].bEnable || VpssHdl != pstVprocCfg->astVpssAttr[i].VpssHdl)
        {
            continue;
        }

        *u32VpssHdlIndex = i;

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if ((pstVprocCfg->astVpssAttr[i].astVportAttr[j].bEnable)
                && (VportHdl == pstVprocCfg->astVpssAttr[i].astVportAttr[j].VportHdl))
            {
                *u32VportHdlIndex = j;

                return HI_SUCCESS;
            }
        }

        break;
    }

    MLOGE(" VpssHdl[%d] VportHdl[%d] is not existed\n", VpssHdl, VportHdl);
    return HI_FAILURE;
}


static HI_S32 PDT_MEDIA_UpdateVportAttr(HI_U32 VpssHdl,HI_U32 VportHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i,j,k;
    HI_U32 u32VpssInputWidth = 1920;
    HI_U32 u32VpssHdlIndex  = 0;
    HI_U32 u32VportHdlIndex = 0;

    HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg;
    HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVprocCfg;

    s32Ret = PDT_MEDIA_GetVpssIndex(pstVprocCfg, VpssHdl, VportHdl, &u32VpssHdlIndex, &u32VportHdlIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_HANDLE VcapPipeHdl    = pstVprocCfg->astVpssAttr[u32VpssHdlIndex].VcapPipeHdl;
    HI_HANDLE VcapPipeChnHdl = pstVprocCfg->astVpssAttr[u32VpssHdlIndex].VcapPipeChnHdl;

    s32Ret = HI_MAPI_VPROC_StopPort(VpssHdl,VportHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_BOOL bFind = HI_FALSE;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVcapCfg->astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].bEnable
                || (VcapPipeHdl != pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl))
            {
                continue;
            }

            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr =
                    &pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k];
                if ((pstPipeChnAttr->bEnable) && (VcapPipeChnHdl == pstPipeChnAttr->PipeChnHdl))
                {
                    u32VpssInputWidth = pstPipeChnAttr->stDestResolution.u32Width;
                    bFind = HI_TRUE;

                    break;
                }
            }

            break;
        }

        if (HI_TRUE == bFind)
        {
            break;
        }
    }

    s32Ret = PDT_MEDIA_SetVport(VpssHdl,u32VpssInputWidth, pstVprocCfg->astVpssAttr[u32VpssHdlIndex].stVpssAttr.bNrEn
                    ,&pstVprocCfg->astVpssAttr[u32VpssHdlIndex].astVportAttr[u32VportHdlIndex]);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}


static HI_S32 PDT_MEDIA_VoWndStart(const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++i)
    {
        if (!pstDispCfg->astWndCfg[i].bEnable)
        {
            continue;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_SetWindowAttr(pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl,
                    (HI_MAPI_DISP_WINDOW_ATTR_S*)&pstDispCfg->astWndCfg[i].stWndAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDispCfg->astWndCfg[i].enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = PDT_MEDIA_UpdateVportAttr(pstDispCfg->astWndCfg[i].ModHdl,pstDispCfg->astWndCfg[i].ChnHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_Bind_VProc(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl,
                pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl, HI_FALSE);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_PDT_MEDIA_DispWindowCrop(&pstDispCfg->astWndCfg[i].stCropCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VPROC_StartPort(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstDispCfg->astWndCfg[i].enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_Bind_VCap(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl,
                pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        HI_MAPI_DISP_WINDOW_ASPECTRATIO_S stAspectRatio = {};
        stAspectRatio.stAspectRatio = pstDispCfg->astWndCfg[i].stAspectRatio;
        if(ASPECT_RATIO_NONE!=stAspectRatio.stAspectRatio.enMode && pstDispCfg->astWndCfg[i].WndHdl == 0)
        {
            MLOGI("[%d,%d] AspectMode[%d] BgColor[%#x]\n", pstDispCfg->VdispHdl,
                pstDispCfg->astWndCfg[i].WndHdl,
                stAspectRatio.stAspectRatio.enMode, stAspectRatio.stAspectRatio.u32BgColor);
            s32Ret = HI_MAPI_DISP_SetWindowAttrEx(pstDispCfg->VdispHdl,
                    pstDispCfg->astWndCfg[i].WndHdl, HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO,
                    (HI_VOID*)&stAspectRatio, sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S));
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        s32Ret = HI_MAPI_DISP_StartWindow(pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_VoWndStop(const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    HI_BOOL chnStatus = HI_FALSE;

    for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++i)
    {
        if (!pstDispCfg->astWndCfg[i].bEnable)
        {
            continue;
        }

        /* TODO: stop vproc port, if port not used by other, eg thm venc */

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_StopWindow(pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDispCfg->astWndCfg[i].enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_UnBind_VProc(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl,
                pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl, HI_FALSE);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            for(j = 0; j < HI_PDT_MEDIA_VENC_MAX_CNT; j++)
            {
                if(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[j].ModHdl == pstDispCfg->astWndCfg[i].ModHdl &&
                    s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[j].ChnHdl ==  pstDispCfg->astWndCfg[i].ChnHdl &&
                    s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[j].bEnable)
                {
                    chnStatus = HI_TRUE;
                    MLOGI("can't stop channel%d!!\n",pstDispCfg->astWndCfg[i].ChnHdl);
                }
            }
            if(!chnStatus)
            {
                HI_MAPI_VPROC_StopPort(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl);
            }
        }
        else if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstDispCfg->astWndCfg[i].enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_UnBind_VCap(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl,
                pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            MLOGE("Invalid Binded Module[%d]\n", pstDispCfg->astWndCfg[i].enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_SetCompressMode(HI_BOOL bEnable)
{
#if 0
    HI_S32 i, j = 0;
    HI_S32 s32Ret = 0;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if (!s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if (!s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
            {
                continue;
            }
            HI_HANDLE VcapHdl = s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl;
            HI_MPP_VCAP_ATTR_S stVcapAttr;

            PDT_MEDIA_SwitchVcapAttr(i, j, &s_stMEDIACtx.stMediaCfg.stVideoCfg, &stVcapAttr);
            stVcapAttr.enCompressMode = HI_COMPRESS_MODE_NONE;

            if (HI_TRUE == bEnable
                && HI_FALSE == s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[0].stLDCAttr.bEnable
                && MAX_IMAGE_WIDTH > s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[0].stDestResolution.u32Width)
            {
                HI_MPP_ROTATE_E enRotate = HI_ROTATE_BUTT;

                s32Ret = HI_MAPI_VCap_GetAttrEx(VcapHdl, HI_MPP_VCAP_CMD_GetRotate, (HI_VOID*)&enRotate, sizeof(HI_MPP_ROTATE_E));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                if (HI_ROTATE_NONE == enRotate)
                {
                    stVcapAttr.enCompressMode = HI_COMPRESS_MODE_SEG;
                }
            }

            s32Ret = HI_MAPI_VCap_SetAttr(VcapHdl, (HI_MPP_VCAP_ATTR_S*)&stVcapAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    }

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
    {
        if (!s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].bEnable)
        {
            continue;
        }
        HI_VPROC_TYPE_E enProcType;

        HI_HANDLE VcapHdl = s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].VcapPipeHdl;
        HI_HANDLE VprocHdl = s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].VpssHdl;

        s32Ret = PDT_MEDIA_SwitchVpssType(&(s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].stVpssAttr.enVpssType), &enProcType);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
        {
            if (!s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].bEnable)
            {
                continue;
            }

            HI_HANDLE VportHdl = s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].VportHdl;
            HI_VPORT_ATTR_S stVportAttr;
            HI_MPP_VCAP_ATTR_S stVCapAttr;
            PDT_MEDIA_SwitchVportAttr(&s_stMEDIACtx.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j], &stVportAttr);
            stVportAttr.enCompressMode = HI_COMPRESS_MODE_NONE;

            s32Ret = HI_MAPI_VCap_GetAttr(VcapHdl, &stVCapAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if (HI_TRUE == bEnable
                && MAX_IMAGE_WIDTH > stVCapAttr.stResolution.u32Width
                && VPROC_TYPE_VIDEO == enProcType)
            {
                HI_MPP_ROTATE_E enRotate = HI_ROTATE_BUTT;

                s32Ret = HI_MAPI_VProc_GetAttrEx(VprocHdl, VportHdl, HI_VPROC_CMD_GetPortRotate, &enRotate, sizeof(HI_MPP_ROTATE_E));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                if (HI_ROTATE_NONE == enRotate)
                {
                    stVportAttr.enCompressMode = HI_COMPRESS_MODE_SEG;
                }
            }

            s32Ret = HI_MAPI_VProc_Port_SetAttr(VprocHdl, VportHdl, (HI_VPORT_ATTR_S*)&stVportAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
#endif
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_SetVideoExParam(HI_VOID)
{
    HI_S32 i, j, k;
    HI_S32 s32Ret = HI_SUCCESS;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstDevAttr =
            &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable)
            {
                continue;
            }
            if (HI_TRUE == pstDevAttr->astVcapPipeAttr[j].bIspBypass)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                /* Brightness */
                HI_U8 u8Brightness = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Brightness;
                MLOGD("Brightness[%u]\n", u8Brightness);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                    HI_MAPI_VCAP_CMD_ISP_Luma, &u8Brightness, sizeof(HI_U8));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                /* Saturation */
                HI_U8 u8Saturation = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Saturation;
                MLOGD("Saturation[%u]\n", u8Saturation);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                    HI_MAPI_VCAP_CMD_ISP_Saturation, &u8Saturation, sizeof(HI_U8));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                /* LDC */
                MLOGD(LIGHT_BLUE"vcap pipe[%d] chn[%d], stLDCAttr.bEnable(%d)\n"NONE,
                    j, k, pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                if(pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable)
                {
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl, HI_MAPI_VCAP_CMD_LDC,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }
    }

    s32Ret = PDT_MEDIA_SetCompressMode(HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_BOOL PDT_MEDIA_VcapNeedReset(const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgOld,
    const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgNew)
{
    HI_S32 s32DevIdx = 0;
    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        if(pstVcapCfgOld->astVcapDevAttr[s32DevIdx].bEnable != pstVcapCfgNew->astVcapDevAttr[s32DevIdx].bEnable
            || pstVcapCfgOld->astVcapDevAttr[s32DevIdx].VcapDevHdl != pstVcapCfgNew->astVcapDevAttr[s32DevIdx].VcapDevHdl
            || pstVcapCfgOld->astVcapDevAttr[s32DevIdx].enWdrMode != pstVcapCfgNew->astVcapDevAttr[s32DevIdx].enWdrMode
            || pstVcapCfgOld->astVcapDevAttr[s32DevIdx].u32VideoMode != pstVcapCfgNew->astVcapDevAttr[s32DevIdx].u32VideoMode)
        {
            return HI_TRUE;
        }
        if ((0 != memcmp(&pstVcapCfgOld->astVcapDevAttr[s32DevIdx].stResolution,
            &pstVcapCfgNew->astVcapDevAttr[s32DevIdx].stResolution, sizeof(SIZE_S))))
        {
            return HI_TRUE;
        }
        if ((0 != memcmp(&pstVcapCfgOld->astVcapDevAttr[s32DevIdx].stSnsAttr,
            &pstVcapCfgNew->astVcapDevAttr[s32DevIdx].stSnsAttr, sizeof(HI_MEDIA_SENSOR_ATTR_S))))
        {
            return HI_TRUE;
        }
        if ((0 != memcmp(pstVcapCfgOld->astVcapDevAttr[s32DevIdx].astVcapPipeAttr,
            pstVcapCfgNew->astVcapDevAttr[s32DevIdx].astVcapPipeAttr, sizeof(HI_PDT_MEDIA_VCAP_PIPE_ATTR_S) *
            HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT)))
        {
            return HI_TRUE;
        }

        if ((!pstVcapCfgOld->astVcapDevAttr[s32DevIdx].astVcapPipeAttr[0].bIspBypass) &&
            (0 != memcmp(&pstVcapCfgOld->astVcapDevAttr[s32DevIdx].stExifInfo,
            &pstVcapCfgNew->astVcapDevAttr[s32DevIdx].stExifInfo, sizeof(HI_MAPI_SNAP_EXIF_INFO_S))))
        {
            return HI_TRUE;
        }
    }
    return HI_FALSE;
}

static HI_S32 PDT_MEDIA_ResetVcap(const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgOld,
                const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgNew, HI_BOOL bCompressChange)
{
    PDT_MEDIA_UpdateVcapDevAttr((HI_PDT_MEDIA_VCAP_DEV_ATTR_S*)pstVcapCfgNew);
    if ((HI_FALSE == PDT_MEDIA_VcapNeedReset(pstVcapCfgOld, pstVcapCfgNew)) &&
        (HI_FALSE == bCompressChange))
    {
        MLOGI("Vcap Attr is not changed\n");
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DevIdx = 0;
    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        s32Ret = PDT_MEDIA_StopVcapDev(&pstVcapCfgOld->astVcapDevAttr[s32DevIdx], HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VCAP_DeinitSensor(pstVcapCfgOld->astVcapDevAttr[s32DevIdx].VcapDevHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /** set frequency */

    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        s32Ret = HI_PDT_MEDIA_InitSensor(&pstVcapCfgNew->astVcapDevAttr[s32DevIdx]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_MEDIA_StartVcapDev(&pstVcapCfgNew->astVcapDevAttr[s32DevIdx]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_ResetVproc(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgOld,
                            const HI_PDT_MEDIA_VIDEO_CFG_S *pstVideoCfgNew, HI_BOOL bCompressChange)
{
    const HI_PDT_MEDIA_VPSS_ATTR_S* pstMediaVpssOld = NULL;
    const HI_PDT_MEDIA_VPSS_ATTR_S* pstMediaVpssNew  = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
    {
        pstMediaVpssOld = &pstVideoCfgOld->stVprocCfg.astVpssAttr[i];
        pstMediaVpssNew = &pstVideoCfgNew->stVprocCfg.astVpssAttr[i];
        MLOGD("Vpss[%d] Hdl[%d]\n", i, pstMediaVpssNew->VpssHdl);

        if (!pstMediaVpssNew->bEnable)
        {
            if (pstMediaVpssOld->bEnable)
            {
                MLOGD("  enable -> disable: deinit vpss\n");
                s32Ret = PDT_MEDIA_DeinitVpss(pstVideoCfgOld, i);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            else
            {
                MLOGD("  disable -> disable: do nothing\n");
            }
        }
        else
        {
            if (pstMediaVpssOld->bEnable)
            {
                MLOGD("  enable -> enable:\n");
                if ((0 != memcmp(pstMediaVpssOld, pstMediaVpssNew, sizeof(HI_PDT_MEDIA_VPSS_ATTR_S))) ||
                    (HI_TRUE == bCompressChange))
                {
                    MLOGD("    Param Change: deinit and init\n");
                    s32Ret = PDT_MEDIA_DeinitVpss(pstVideoCfgOld, i);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                    s32Ret = PDT_MEDIA_InitVpss(pstVideoCfgNew, i);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
                else
                {
                    MLOGD("    Param Not Change: do nothing\n");
                }
            }
            else
            {
                /* Disable -> Enable */
                MLOGD("  disable -> enable: init vpss\n");
                s32Ret = PDT_MEDIA_InitVpss(pstVideoCfgNew, i);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_ResetVenc(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgOld,
                    const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgNew)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (0 != memcmp(&pstVideoCfgOld->astVencCfg, &pstVideoCfgNew->astVencCfg,
            sizeof(HI_PDT_MEDIA_VENC_CFG_S)*HI_PDT_MEDIA_VENC_MAX_CNT))
    {
        MLOGI("Venc Attr changed, rebuild all\n");

        HI_S32 i;
        for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
        {
            PDT_MEDIA_DeinitVenc(pstVideoCfgOld, i);
        }

        /** set frequency */

        /* Venc */
        for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
        {
            s32Ret = PDT_MEDIA_InitVenc(pstVideoCfgNew, &pstVideoCfgNew->astVencCfg[i], i);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        return HI_SUCCESS;
    }

    MLOGI("Venc Attr not changed, do nothing\n");
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_ResetVideo(const HI_PDT_MEDIA_CFG_S* pstMediaCfg,
    HI_BOOL bRebuild, HI_BOOL bCompressChange)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgOld = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg;
    const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgNew = &pstMediaCfg->stVideoCfg;
    HI_S32 s32Ret = HI_SUCCESS;

    /* init video if not init before */
    if (!s_stMEDIACtx.bVideoInit)
    {
        MLOGD("Video not init yet, init video\n");
        s32Ret = HI_PDT_MEDIA_VideoInit(&pstMediaCfg->stVideoCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_SUCCESS;
    }

    /* rebuild all */
    if (bRebuild)
    {
        s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_MEDIA_Deinit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_MEDIA_Init(&pstMediaCfg->stViVpssMode, &pstMediaCfg->stVBCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_PDT_MEDIA_VideoInit(pstVideoCfgNew);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGD(YELLOW"Rebuild Media OK!"NONE"\n");
    }
    /* neednot force rebuild and param not change: do nothing */
    else if ((0 == memcmp(pstVideoCfgOld, pstVideoCfgNew, sizeof(HI_PDT_MEDIA_VIDEO_CFG_S))) &&
             (HI_FALSE == bCompressChange))
    {
        MLOGD(YELLOW"------------ Video Param Not Change ------------"NONE"\n");
    }
    /* param change */
    else
    {
        s32Ret = PDT_MEDIA_StopOsd();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Stop Osd OK!"NONE"\n");

        s32Ret = PDT_MEDIA_ResetVcap(&pstVideoCfgOld->stVcapCfg, &pstVideoCfgNew->stVcapCfg, bCompressChange);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Reset VCAP OK!"NONE"\n");

        s32Ret = PDT_MEDIA_ResetVproc(pstVideoCfgOld, pstVideoCfgNew, bCompressChange);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Reset VPROC OK!"NONE"\n");

        s32Ret = PDT_MEDIA_ResetVenc(pstVideoCfgOld, pstVideoCfgNew);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Reset VENC OK!"NONE"\n");

        HI_BOOL bResetVideoExParam = HI_FALSE;
        if ((0 != memcmp(&pstVideoCfgOld->stVcapCfg, &pstVideoCfgNew->stVcapCfg, sizeof(HI_PDT_MEDIA_VCAP_CFG_S))) ||
            (HI_TRUE == bCompressChange))
        {
            bResetVideoExParam = HI_TRUE;
        }

        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg, &pstMediaCfg->stVideoCfg,
            sizeof(HI_PDT_MEDIA_VIDEO_CFG_S));

        s32Ret = PDT_MEDIA_StartOsd(&pstMediaCfg->stVideoCfg.stOsdCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Start Osd OK!"NONE"\n");

        if (bResetVideoExParam)
        {
            MLOGD("Vcap Attr changed\n");
            s32Ret = PDT_MEDIA_SetVideoExParam();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        MLOGD(YELLOW"Reset VIDEO OK!"NONE"\n");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_ResetAudio(const HI_PDT_MEDIA_AUDIO_CFG_S *pstAudiocfgOld,
                        const HI_PDT_MEDIA_AUDIO_CFG_S *pstAudioCfgNew, HI_BOOL bRebuild)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* init audio if not init before */
    if (!s_stMEDIACtx.bAudioInit)
    {
        MLOGD("Audio not init yet, init audio\n");
        s32Ret = HI_PDT_MEDIA_AudioInit(pstAudioCfgNew);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        return HI_SUCCESS;
    }

    /* param not change and neednot force rebuild: do nothing */
    if ( !bRebuild && (0 == memcmp(pstAudiocfgOld, pstAudioCfgNew, sizeof(HI_PDT_MEDIA_AUDIO_CFG_S))))
    {
        MLOGD(YELLOW"------------ Audio Param Not Change ------------"NONE"\n");
        return HI_SUCCESS;
    }

    /* deinit audio */
    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /** set frequency */

    /** init audio */
    s32Ret = HI_PDT_MEDIA_AudioInit(pstAudioCfgNew);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_ResetAudioOut(const HI_PDT_MEDIA_AUDIOOUT_CFG_S *pstAudioOutCfgOld,
                        const HI_PDT_MEDIA_AUDIOOUT_CFG_S *pstAudioOutCfgNew, HI_BOOL bRebuild)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* init audio out if not init before */
    if (!s_stMEDIACtx.bAudioOutInit)
    {
        MLOGD("AudioOut not init yet, init audio out\n");
        s32Ret = HI_PDT_MEDIA_AudioOutStart(pstAudioOutCfgNew);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        return HI_SUCCESS;
    }

    /* param not change and neednot force rebuild: do nothing */
    if ( !bRebuild && (0 == memcmp(pstAudioOutCfgOld, pstAudioOutCfgNew, sizeof(HI_PDT_MEDIA_AUDIOOUT_CFG_S))))
    {
        MLOGI(YELLOW"------------ Audio Out Param Not Change ------------"NONE"\n");
        return HI_SUCCESS;
    }

    /* deinit audio out*/
    s32Ret = HI_PDT_MEDIA_AudioOutStop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /** set frequency */

    /** init audio out*/
    s32Ret = HI_PDT_MEDIA_AudioOutStart(pstAudioOutCfgNew);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}



HI_PDT_MEDIA_CFG_S* PDT_MEDIA_GetMediaCfg(HI_VOID)
{
    if (!s_stMEDIACtx.bMediaInit)
    {
        MLOGE("not init\n");
        return NULL;
    }
    return &s_stMEDIACtx.pstMediaInfo->stMediaCfg;
}

static HI_S32 PDT_MEDIA_Init(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode, const HI_PDT_MEDIA_VB_CFG_S* pstVBCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));

    HI_S32 i, j;
    HI_U32 s32Idx = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[s32Idx] = pstViVpssMode->astMode[i][j].enMode;
            MLOGD("ViVpssMode[%d] %d\n", s32Idx, stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[s32Idx]);
            s32Idx++;
        }
    }

    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = pstVBCfg->u32MaxPoolCnt;
    for (i = 0; i < stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt; ++i)
    {
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u64BlkSize = pstVBCfg->astCommPool[i].u32BlkSize;
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u32BlkCnt  = pstVBCfg->astCommPool[i].u32BlkCnt;
        MLOGD("VBPool[%d] BlkSize[%u] BlkCnt[%u]\n", i, pstVBCfg->astCommPool[i].u32BlkSize,
            pstVBCfg->astCommPool[i].u32BlkCnt);
    }

    stMediaAttr.stVencModPara.u32H264eLowPowerMode = 1;
    stMediaAttr.stVencModPara.u32H265eLowPowerMode = 1;

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_Media_Init(&stMediaAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#if !(defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
    HI_PLAYER_RegAdec();
#endif
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stViVpssMode, pstViVpssMode, sizeof(HI_PDT_MEDIA_VI_VPSS_MODE_S));
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVBCfg, pstVBCfg, sizeof(HI_PDT_MEDIA_VB_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Init(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode, const HI_PDT_MEDIA_VB_CFG_S* pstVBCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstViVpssMode, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstVBCfg, HI_PDT_MEDIA_EINVAL);
    if (s_stMEDIACtx.bMediaInit)
    {
        MLOGD("Media already Init\n");
        return HI_PDT_MEDIA_EINITIALIZED;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    /* Map MediaInfo Memory */
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_LINUX)
    s_stMEDIACtx.pstMediaInfo = (PDT_MEDIA_INFO_S*)HI_MemMap((HI_U32)PDT_MEDIA_INFO_PHY_ADDR, sizeof(PDT_MEDIA_INFO_S));
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stMEDIACtx.pstMediaInfo, HI_FAILURE, "MemMap");
    MLOGI("media info address[%#x] size[%lu]\n", PDT_MEDIA_INFO_PHY_ADDR, (HI_UL)sizeof(PDT_MEDIA_INFO_S));
#if defined(AMP_LINUX)
    memset(s_stMEDIACtx.pstMediaInfo, 0, sizeof(PDT_MEDIA_INFO_S));
#endif
#else
    s_stMEDIACtx.pstMediaInfo = (PDT_MEDIA_INFO_S*)PDT_MEDIA_INFO_PHY_ADDR;
    memset(s_stMEDIACtx.pstMediaInfo, 0, sizeof(PDT_MEDIA_INFO_S));
#endif

    /* Media Init */
    s32Ret = PDT_MEDIA_Init(pstViVpssMode, pstVBCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* Update Media Context */
    s_stMEDIACtx.bMediaInit = HI_TRUE;

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_MUTEX_INIT_LOCK(s_stMEDIACtx.CntMutex);
#endif

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Deinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;
#if !(defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
    HI_MUTEX_DESTROY(s_stMEDIACtx.CntMutex);
    HI_PLAYER_UnRegAdec();
#endif

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_Media_Deinit();
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s_stMEDIACtx.bMediaInit = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoInit(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    if (s_stMEDIACtx.bVideoInit)
    {
        MLOGE("Video already Init\n");
        return HI_PDT_MEDIA_EINITIALIZED;
    }

    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg, pstVideoCfg, sizeof(HI_PDT_MEDIA_VIDEO_CFG_S));
    HI_PDT_MEDIA_VIDEO_CFG_S* pstCurrentVideoCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        /* Sensor */
        s32Ret = HI_PDT_MEDIA_InitSensor(&pstCurrentVideoCfg->stVcapCfg.astVcapDevAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        /* VcapDev */
        s32Ret = PDT_MEDIA_StartVcapDev(&pstCurrentVideoCfg->stVcapCfg.astVcapDevAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* Vproc */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        s32Ret = PDT_MEDIA_InitVpss(pstCurrentVideoCfg, i);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* Venc */
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        s32Ret = PDT_MEDIA_InitVenc(pstCurrentVideoCfg, &pstCurrentVideoCfg->astVencCfg[i], i);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* OSD */
    s32Ret = PDT_MEDIA_StartOsd(&pstCurrentVideoCfg->stOsdCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* ExParam */
    s32Ret = PDT_MEDIA_SetVideoExParam();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s_stMEDIACtx.bVideoInit = HI_TRUE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoDeinit(HI_BOOL bStopIsp)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_S32 i;

    /* OSD stop*/
    PDT_MEDIA_StopOsd();

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        PDT_MEDIA_DeinitVenc(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg, i);
    }

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        PDT_MEDIA_DeinitVpss(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg, i);
    }

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        PDT_MEDIA_StopVcapDev(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i], bStopIsp);
    }

    s_stMEDIACtx.bVideoInit = HI_FALSE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutInit(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoOutCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg, pstVideoOutCfg, sizeof(HI_PDT_MEDIA_VIDEOOUT_CFG_S));

    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
        if (!pstDispCfg->bEnable)
        {
            continue;
        }
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_Init(pstDispCfg->VdispHdl, (HI_MAPI_DISP_ATTR_S*)&pstDispCfg->stDispAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    s_stMEDIACtx.bVideoOutInit= HI_TRUE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutDeinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
        if (!pstDispCfg->bEnable)
        {
            continue;
        }
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_Deinit(pstDispCfg->VdispHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    s_stMEDIACtx.bVideoOutInit = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutStart(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg,const HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

    if(NULL==pstVideoOutCfg)
    {
        pstVideoOutCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg;
    }

    if(pstVprocCfg != NULL)
    {
        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVprocCfg,pstVprocCfg,sizeof(HI_PDT_MEDIA_VPROC_CFG_S));
    }

    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    HI_S32 s32WndCnt = 0;

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstDispCfg = &pstVideoOutCfg->astDispCfg[i];
        if (!pstDispCfg->bEnable)
        {
            continue;
        }

        if (pstDispCfg->stCscAttr.bEnable)
        {
            MLOGI(YELLOW"set csc in video out start\n"NONE);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_SetAttrEx(pstDispCfg->VdispHdl, HI_MAPI_DISP_CMD_VIDEO_CSC,
                (HI_VOID*)&pstDispCfg->stCscAttr.stAttrEx, sizeof(HI_MAPI_DISP_CSCATTREX_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++j)
        {
            if (!pstDispCfg->astWndCfg[j].bEnable)
            {
                continue;
            }
            HI_MAPI_DISP_WINDOW_ROTATE_S stDispRotate;
            stDispRotate.enDispRorate = pstDispCfg->astWndCfg[j].enRotate;
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_DISP_SetWindowAttrEx(pstDispCfg->VdispHdl,
                    pstDispCfg->astWndCfg[j].WndHdl, HI_MAPI_DISP_WINDOW_CMD_ROTATE,
                    (HI_VOID*)&stDispRotate, sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        HI_PERFORMANCE_TIME_STAMP;
        HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;
        stVideoLayerAttr.stImageSize = pstVideoOutCfg->astDispCfg[0].stImageSize;
        stVideoLayerAttr.u32VLFrameRate = 0;
        if(stVideoLayerAttr.stImageSize.u32Width==pstVideoOutCfg->astDispCfg[0].astWndCfg[0].stWndAttr.stRect.u32Width
          &&stVideoLayerAttr.stImageSize.u32Height==pstVideoOutCfg->astDispCfg[0].astWndCfg[0].stWndAttr.stRect.u32Height
          &&ASPECT_RATIO_NONE==pstVideoOutCfg->astDispCfg[0].astWndCfg[0].stAspectRatio.enMode)
        {
            for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; j++)
            {
                if(pstDispCfg->astWndCfg[j].bEnable)
                {
                    s32WndCnt++;
                }
            }

            /**if one DispDevice use more than 1 Wnd, The BufLen must not be 0*/
            if (s32WndCnt > 1)
            {
                stVideoLayerAttr.u32BufLen = pstDispCfg->u32BufferLen;
            }
            else
            {
                stVideoLayerAttr.u32BufLen = 0;
            }
        }
        else
        {
            stVideoLayerAttr.u32BufLen = pstDispCfg->u32BufferLen;
        }
        s32Ret = HI_MAPI_DISP_Start(pstDispCfg->VdispHdl,&stVideoLayerAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        s32Ret = PDT_MEDIA_VoWndStart(pstDispCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    }

    s_stMEDIACtx.pstMediaInfo->bDispStart = HI_TRUE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetDisplayGraphicCSC(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

    HI_S32 s32Ret = HI_SUCCESS;
    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
        if (!pstDispCfg->bEnable)
        {
            continue;
        }

        MLOGI(YELLOW"set graphic csc for graphic layer %d\n"NONE, pstDispCfg->VdispHdl);

        HI_MAPI_DISP_CSCATTREX_S stAttrEx;
        if (VO_INTF_HDMI == pstDispCfg->stDispAttr.stPubAttr.enIntfType)
        {
            stAttrEx.stVoCSC.enCscMatrix = VO_CSC_MATRIX_RGB_TO_BT601_TV;
        }
        else
        {
            stAttrEx.stVoCSC.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
        }
        stAttrEx.stVoCSC.u32Contrast = 50;
        stAttrEx.stVoCSC.u32Hue = 50;
        stAttrEx.stVoCSC.u32Luma = 50;
        stAttrEx.stVoCSC.u32Satuature = 50;

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_SetAttrEx(pstDispCfg->VdispHdl, HI_MAPI_DISP_CMD_GRAPHIC_CSC,
            (HI_VOID*)&stAttrEx, sizeof(HI_MAPI_DISP_CSCATTREX_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        HI_PERFORMANCE_TIME_STAMP;
    }
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_DispWindowCrop(const HI_PDT_MEDIA_DISP_WND_CROP_CFG_S* pstCropCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_POINTER(pstCropCfg, HI_PDT_MEDIA_EINVAL);
    if(pstCropCfg->bEnable)
    {
        HI_APPCOMM_CHECK_EXPR(2<=pstCropCfg->u32MaxH, HI_PDT_MEDIA_EINVAL);
        HI_APPCOMM_CHECK_EXPR(pstCropCfg->u32CurH<pstCropCfg->u32MaxH, HI_PDT_MEDIA_EINVAL);
    }
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__LINUX__))
    HI_S32 s32Result = 0;
    s32Ret = HI_MSG_SendSync(HI_MSG_MEDIA_SETCROP, pstCropCfg, sizeof(HI_PDT_MEDIA_DISP_WND_CROP_CFG_S), NULL,&s32Result, sizeof(s32Result));
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && (HI_SUCCESS == s32Result), HI_FAILURE);
#else

    HI_S32 i = 0, j = 0;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++)
    {
        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; j++)
        {
            HI_PDT_MEDIA_DISP_WND_CFG_S* pstDispWndCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i].astWndCfg[j];
            if(!pstDispWndCfg->bEnable)
            {
                MLOGI("disp[%d]_wnd[%d] is not enable\n",i,j);
                continue;
            }

            HI_HANDLE ModHdl = pstDispWndCfg->ModHdl;
            HI_HANDLE ChnHdl = pstDispWndCfg->ChnHdl;


            HI_MAPI_VPORT_ATTR_S stVPortAttr = {0};
            s32Ret = HI_MAPI_VPROC_GetPortAttr(ModHdl, ChnHdl, &stVPortAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_U32 u32MaxHeight = stVPortAttr.u32Height;
#if defined(CONFIG_VERTICAL_SCREEN)
            HI_U32 u32CropWidth  = pstDispWndCfg->stWndAttr.stRect.u32Height;
            HI_U32 u32CropHeight = pstDispWndCfg->stWndAttr.stRect.u32Width;
#else
            HI_U32 u32CropWidth  = pstDispWndCfg->stWndAttr.stRect.u32Width;
            HI_U32 u32CropHeight = pstDispWndCfg->stWndAttr.stRect.u32Height;
#endif

            VPSS_CROP_INFO_S stCropInfo = {0};
            if (pstDispWndCfg->WndHdl == MAIN_PREVIEW_VOWND)
            {
                stCropInfo.bEnable = pstCropCfg->bEnable;
            }
            else
            {
                stCropInfo.bEnable = HI_FALSE;
            }
            stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
            stCropInfo.stCropRect.s32X = 0;
            stCropInfo.stCropRect.u32Width = stVPortAttr.u32Width;
            stCropInfo.stCropRect.u32Height = HI_APPCOMM_ALIGN(stVPortAttr.u32Width * u32CropHeight / u32CropWidth, 2);

            if (stCropInfo.bEnable)
            {
                stCropInfo.stCropRect.s32Y = (pstCropCfg->u32CurH * (u32MaxHeight - stCropInfo.stCropRect.u32Height) / (pstCropCfg->u32MaxH - 1));
                stCropInfo.stCropRect.s32Y = HI_APPCOMM_ALIGN(stCropInfo.stCropRect.s32Y, 2);
            }
            else
            {
                stCropInfo.stCropRect.s32Y = 0;
            }
            MLOGI("set disp[%d]_wnd[%d]_modhdl[%d]_chnhdl[%d] crop \n",i,j,ModHdl,ChnHdl);
            s32Ret = HI_MPI_VPSS_SetChnCrop(ModHdl, ChnHdl, &stCropInfo);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
#endif
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutStop(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++)
    {
        pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
        if (!pstDispCfg->bEnable)
        {
            continue;
        }

        s32Ret = PDT_MEDIA_VoWndStop(pstDispCfg);
        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_Stop(pstDispCfg->VdispHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_stMEDIACtx.pstMediaInfo->bDispStart = HI_FALSE;
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AudioInit(const HI_PDT_MEDIA_AUDIO_CFG_S* pstAudioCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstAudioCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    if (HI_TRUE == s_stMEDIACtx.bAudioInit)
    {
        MLOGD("already Init.\n");
        return HI_SUCCESS;
    }
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg, pstAudioCfg, sizeof(HI_PDT_MEDIA_AUDIO_CFG_S));

    const HI_PDT_MEDIA_ACAP_CFG_S* pstAcapCfg = NULL;
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32AcapIdx, s32AencIdx;

    for (s32AcapIdx = 0; s32AcapIdx < HI_PDT_MEDIA_ACAP_MAX_CNT; ++s32AcapIdx)
    {
        pstAcapCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAcapCfg[s32AcapIdx];
        if (!pstAcapCfg->bEnable)
        {
            continue;
        }

        MLOGD("AcapHdl[%d] SampleRate[%d] BitWidth[%dbit] SoundMode[%d:0-left,1-right,3-stereo]\n",
            pstAcapCfg->AcapHdl, pstAcapCfg->stAcapAttr.enSampleRate,
            (pstAcapCfg->stAcapAttr.enBitwidth + 1) << 3, pstAcapCfg->stAcapAttr.enSoundMode);
        MLOGD("  WorkMode[%d:0-master,1-slave] PtNumPerFrm[%u]\n",
            pstAcapCfg->stAcapAttr.enWorkmode, pstAcapCfg->stAcapAttr.u32PtNumPerFrm);
        MLOGD("  MixerMicMode[%d:0-in0,1-in1,2-inD] ResampleRate[%d]\n",
            pstAcapCfg->stAcapAttr.enMixerMicMode, pstAcapCfg->stAcapAttr.enResampleRate);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_ACAP_Init(pstAcapCfg->AcapHdl, &pstAcapCfg->stAcapAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        HI_S32 s32ChnlIdx = 0;
        for (s32ChnlIdx = 0; s32ChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32ChnlIdx)
        {

            MLOGD("AcapChn%d :hadle[%d], enable[%d], enableVqe[%d]\n",s32ChnlIdx,
                pstAcapCfg->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl, pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnable,
                pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnableVqe);
            if(pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnable)
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_ACAP_Start(pstAcapCfg->AcapHdl,pstAcapCfg->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl);
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
                if(pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnableVqe)
                {
                    HI_MAPI_ACAP_EnableVqe(pstAcapCfg->AcapHdl, pstAcapCfg->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl);
                }
#endif
            }

        }

        MLOGD("  AudioGain[%d]\n", pstAcapCfg->s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_ACAP_SetVolume(pstAcapCfg->AcapHdl, pstAcapCfg->s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        for (s32AencIdx = 0; s32AencIdx < HI_PDT_MEDIA_AENC_MAX_CNT; ++s32AencIdx)
        {
            pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[s32AencIdx];
            if ( (pstAcapCfg->AcapHdl != pstAencCfg->AcapHdl) || (!pstAencCfg->bEnable))
            {
                continue;
            }

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
            MUTEX_LOCK(s_stMEDIACtx.CntMutex);
            s_stMEDIACtx.as32AencStartCnt[s32AencIdx] = 0;
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

            MLOGD("AencHdl[%d] AencFormat[%d:0-aaclc,1-opus]\n", pstAencCfg->AencHdl,pstAencCfg->enFormat);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_RegisterAudioEncoder(pstAencCfg->enFormat);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            /**need to be check*/
            HI_MAPI_AENC_ATTR_S stAencAttr;
            stAencAttr.enAencFormat   = HI_MAPI_AUDIO_FORMAT_AACLC;
            stAencAttr.u32PtNumPerFrm = pstAencCfg->u32PtNumPerFrm;
            stAencAttr.pValue         =
                &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[s32AencIdx].unAttr.stAACAttr;
            stAencAttr.u32Len         = sizeof(HI_MAPI_AENC_ATTR_AAC_S);

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_Init(pstAencCfg->AencHdl, &stAencAttr);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    s_stMEDIACtx.bAudioInit = HI_TRUE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AudioDeinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    if (HI_FALSE == s_stMEDIACtx.bAudioInit)
    {
        MLOGD("already Deinit.\n");
        return HI_PDT_MEDIA_EDEINITIALIZED;
    }

    const HI_PDT_MEDIA_ACAP_CFG_S* pstAcapCfg = NULL;
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32AcapIdx, s32AencIdx;

    for (s32AcapIdx = 0; s32AcapIdx < HI_PDT_MEDIA_ACAP_MAX_CNT; ++s32AcapIdx)
    {
        pstAcapCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAcapCfg[s32AcapIdx];
        if (!pstAcapCfg->bEnable)
        {
            continue;
        }

        for (s32AencIdx = 0; s32AencIdx < HI_PDT_MEDIA_AENC_MAX_CNT; ++s32AencIdx)
        {
            pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[s32AencIdx];
            if ((pstAencCfg->AcapHdl != pstAcapCfg->AcapHdl) || (!pstAencCfg->bEnable))
            {
                continue;
            }

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
            MUTEX_LOCK(s_stMEDIACtx.CntMutex);
            s_stMEDIACtx.as32AencStartCnt[s32AencIdx] = 0;
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_Deinit(pstAencCfg->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_UnregisterAudioEncoder(pstAencCfg->enFormat);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        HI_S32 s32ChnlIdx = 0;
        for (s32ChnlIdx = 0; s32ChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32ChnlIdx)
        {
            if(pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnable)
            {
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
                if(pstAcapCfg->AcapChnlCfg[s32ChnlIdx].bEnableVqe)
                {
                    HI_MAPI_ACAP_DisableVqe(pstAcapCfg->AcapHdl, pstAcapCfg->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl);
                }
#endif
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_ACAP_Stop(pstAcapCfg->AcapHdl,pstAcapCfg->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            }

        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_ACAP_Deinit(pstAcapCfg->AcapHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    MLOGD("Success\n");
    s_stMEDIACtx.bAudioInit = HI_FALSE;
    return HI_SUCCESS;
}

static inline HI_VOID PDT_MEDIA_SwitchAoGain(HI_S32 s32Volume, HI_S32* ps32Gain)
{
    if(s32Volume > 0)
    {
        *ps32Gain = (200 + s32Volume) / 3 * (HI_MAPI_AO_MAX_GAIN - HI_MAPI_AO_MIN_GAIN) / 100 + HI_MAPI_AO_MIN_GAIN;
    }
    else
    {
        *ps32Gain = HI_MAPI_AO_MIN_GAIN;
    }
}

HI_S32 HI_PDT_MEDIA_AudioOutStart(const HI_PDT_MEDIA_AUDIOOUT_CFG_S* pstAudioOutCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstAudioOutCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    if (s_stMEDIACtx.bAudioOutInit)
    {
        MLOGD("already Init.\n");
        return HI_PDT_MEDIA_EINITIALIZED;
    }
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg, pstAudioOutCfg, sizeof(HI_PDT_MEDIA_AUDIOOUT_CFG_S));

    const HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = NULL;
    HI_S32 s32AudioGain;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        pstAoCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i];
        if (!pstAoCfg->bEnable)
        {
            continue;
        }

        HI_PERFORMANCE_TIME_STAMP;
        HI_MAPI_AO_Init(pstAoCfg->AoHdl, &pstAoCfg->stAoAttr);
        HI_PERFORMANCE_TIME_STAMP;

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AO_Start(pstAoCfg->AoHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        PDT_MEDIA_SwitchAoGain(pstAoCfg->s32Volume, &s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AO_SetVolume(pstAoCfg->AoHdl, s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_stMEDIACtx.bAudioOutInit = HI_TRUE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AudioOutStop(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioOutInit);

    const HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        pstAoCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i];
        if (!pstAoCfg->bEnable)
        {
            continue;
        }
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AO_Stop(pstAoCfg->AoHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AO_Deinit(pstAoCfg->AoHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    s_stMEDIACtx.bAudioOutInit = HI_FALSE;

    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Rebuild(const HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    const VO_PUB_ATTR_S* pstOldVoPubAttr =
        &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[0].stDispAttr.stPubAttr);
    const VO_PUB_ATTR_S* pstNewVoPubAttr =
        &(pstMediaCfg->stVideoOutCfg.astDispCfg[0].stDispAttr.stPubAttr);
    if ((s_stMEDIACtx.bVideoOutInit &&
        (0 != memcmp(pstOldVoPubAttr, pstNewVoPubAttr, sizeof(VO_PUB_ATTR_S)))))
    {
        s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"deinit video out\n"NONE);
    }
    else
    {
        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg, &pstMediaCfg->stVideoOutCfg,
            sizeof(HI_PDT_MEDIA_VIDEOOUT_CFG_S));
        MLOGI(YELLOW"update video out config\n"NONE);
    }

    /** deinit audio out */
    if (s_stMEDIACtx.bAudioOutInit)
    {
        s32Ret = HI_PDT_MEDIA_AudioOutStop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"deinit audio out\n"NONE);
    }

    /** deinit video */
    if (s_stMEDIACtx.bVideoInit)
    {
        s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"deinit video\n"NONE);
    }

    /** deinit audio */
    if (s_stMEDIACtx.bAudioInit)
    {
        s32Ret = HI_PDT_MEDIA_AudioDeinit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"deinit audio\n"NONE);
    }

    /* deinit media */
    if (s_stMEDIACtx.bMediaInit)
    {
        HI_PERFORMANCE_TIME_STAMP;
#if !(defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
        HI_PLAYER_UnRegAdec();
#endif
        s32Ret = HI_MAPI_Media_Deinit();
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        MLOGI(YELLOW"deinit media\n"NONE);
    }

    /** set media frequency */

    /** init media */
    s32Ret = PDT_MEDIA_Init(&pstMediaCfg->stViVpssMode, &pstMediaCfg->stVBCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"init media\n"NONE);

    /** init video */
    s32Ret = HI_PDT_MEDIA_VideoInit(&pstMediaCfg->stVideoCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"init video\n"NONE);

    /** init audio */
    s32Ret = HI_PDT_MEDIA_AudioInit(&(pstMediaCfg->stAudioCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"init audio\n"NONE);

    /** init audio out */
    s32Ret = HI_PDT_MEDIA_AudioOutStart(&(pstMediaCfg->stAudioOutCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"init audio out\n"NONE);

    /** init video out */
    s32Ret = HI_PDT_MEDIA_VideoOutInit(&pstMediaCfg->stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"init video out\n"NONE);

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_Reset(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bCompressChage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    const HI_MAPI_DISP_ATTR_S* pstOldVoAttr =
        &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[0].stDispAttr);
    const HI_MAPI_DISP_ATTR_S* pstNewVoAttr =
        &(pstMediaCfg->stVideoOutCfg.astDispCfg[0].stDispAttr);
    if ((s_stMEDIACtx.bVideoOutInit &&
        (0 != memcmp(pstOldVoAttr, pstNewVoAttr, sizeof(HI_MAPI_DISP_ATTR_S)))))
    {
        HI_PDT_MEDIA_VideoOutStop();
        s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"deinit video out\n"NONE);
    }
    else
    {
        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg, &pstMediaCfg->stVideoOutCfg,
            sizeof(HI_PDT_MEDIA_VIDEOOUT_CFG_S));
        MLOGI(YELLOW"update video out config\n"NONE);
    }

    /** reset video pipe */
    s32Ret = PDT_MEDIA_ResetVideo(pstMediaCfg, HI_FALSE,bCompressChage);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"reset video\n"NONE);

    /** reset audio */
    HI_PDT_MEDIA_AUDIO_CFG_S *pstAudioCfgOld = &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg);
    HI_PDT_MEDIA_AUDIO_CFG_S *pstAudioCfgNew = (HI_PDT_MEDIA_AUDIO_CFG_S *)&(pstMediaCfg->stAudioCfg);
    s32Ret = PDT_MEDIA_ResetAudio(pstAudioCfgOld, pstAudioCfgNew, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"reset audio\n"NONE);


    /** reset audio out*/
    HI_PDT_MEDIA_AUDIOOUT_CFG_S *pstAoCfgOld = &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg);
    HI_PDT_MEDIA_AUDIOOUT_CFG_S *pstAoCfgNew = (HI_PDT_MEDIA_AUDIOOUT_CFG_S *)&(pstMediaCfg->stAudioOutCfg);
    s32Ret = PDT_MEDIA_ResetAudioOut(pstAoCfgOld,pstAoCfgNew,HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"reset audio out\n"NONE);

    /* init video out */
    s32Ret = HI_PDT_MEDIA_VideoOutInit(&pstMediaCfg->stVideoOutCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"init video out\n"NONE);
#ifdef CONFIG_SCREEN
    HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_SetDisplayState");
    MLOGI(YELLOW"set display state on\n"NONE);
#endif

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Reset(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bCompress)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;
    MLOGI(LIGHT_BLUE"bCompress(%d), s_stMEDIACtx.bCompress(%d)"NONE"\n", bCompress, s_stMEDIACtx.bCompress);
    HI_BOOL bCompressChange = HI_FALSE;
#if (!defined(HI3518EV300))
    if(s_stMEDIACtx.bCompress != bCompress)
    {
        bCompressChange = HI_TRUE;
        s_stMEDIACtx.bCompress = bCompress;
    }
#else
    bCompressChange = HI_FALSE;
    s_stMEDIACtx.bCompress = HI_FALSE;
#endif

    /** check rebuild all */
    if ((0 != memcmp(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVBCfg, &pstMediaCfg->stVBCfg, sizeof(HI_PDT_MEDIA_VB_CFG_S))) ||
        (0 != memcmp(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stViVpssMode, &pstMediaCfg->stViVpssMode, sizeof(HI_PDT_MEDIA_VI_VPSS_MODE_S))))
    {
        MLOGI(YELLOW"rebuild all"NONE"\n");
        s32Ret = HI_PDT_MEDIA_Rebuild(pstMediaCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return s32Ret;
    }
    else
    {
        MLOGI(YELLOW"reset all, bCompressChange(%d)"NONE"\n", bCompressChange);
        s32Ret = PDT_MEDIA_Reset(pstMediaCfg, bCompressChange);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
static HI_S32 PDT_MEDIA_GetAencIdx(HI_HANDLE AencHdl, HI_S32* ps32AencIdx)
{
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; ++i)
    {
        pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[i];
        if (AencHdl == pstAencCfg->AencHdl)
        {
            MLOGD("find matched aenc[%d] hdl[%d]\n", i, AencHdl);
            *ps32AencIdx = i;
            return HI_SUCCESS;
        }
    }
    MLOGE("Invalid aenc Hdl[%d]\n", AencHdl);
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_AencStart(HI_HANDLE AencHdl)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched aencIdx */
    s32Ret = PDT_MEDIA_GetAencIdx(AencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[i];

    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    if (0 == s_stMEDIACtx.as32AencStartCnt[i]) /* status:stop, action:start */
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AENC_BindACap(pstAencCfg->AcapHdl, pstAencCfg->AcapChnlHdl, AencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("BindACap failed[%x]\n", s32Ret);
            return HI_FAILURE;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AENC_Start(AencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("AencStart failed[%x]\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s_stMEDIACtx.as32AencStartCnt[i]++; /* update start count */
    MLOGD("aencHdl[%d] startCnt[%d]\n", AencHdl, s_stMEDIACtx.as32AencStartCnt[i]);
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AencStop(HI_HANDLE AencHdl)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched aencIdx */
    s32Ret = PDT_MEDIA_GetAencIdx(AencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[i];

    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    if (0 == s_stMEDIACtx.as32AencStartCnt[i])
    {
        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
        MLOGD("aencHdl[%d] has already stopped\n", AencHdl);
        return HI_SUCCESS;
    }
    else if (1 == s_stMEDIACtx.as32AencStartCnt[i])
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AENC_Stop(AencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("AencStop failed[%x]\n", s32Ret);
            return HI_FAILURE;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AENC_UnbindACap(pstAencCfg->AcapHdl, pstAencCfg->AcapChnlHdl, AencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("UnbindACap failed[%x]\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s_stMEDIACtx.as32AencStartCnt[i]--; /* update start count */
    MLOGD("aencHdl[%d] startCnt[%d]\n", AencHdl, s_stMEDIACtx.as32AencStartCnt[i]);
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);

    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_MEDIA_GetVencIdx(HI_HANDLE VencHdl, HI_S32* ps32VencIdx)
{
    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];
        if (VencHdl == pstVencCfg->VencHdl)
        {
            MLOGD("find matched venc[%d] hdl[%d]\n", i, VencHdl);
            *ps32VencIdx = i;
            return HI_SUCCESS;
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
HI_S32 HI_PDT_MEDIA_VencStart(HI_HANDLE VencHdl, HI_S32 s32FrameCnt)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    if(0 == s32FrameCnt)
    {
        MLOGW("request vencstart, but framecnt is 0, do nothing\n");
        return HI_SUCCESS;
    }

    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched vencIdx */
    s32Ret = PDT_MEDIA_GetVencIdx(VencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];

    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    /* continue venc mode */
    if (HI_MAPI_VENC_LIMITLESS_FRAME_COUNT == s32FrameCnt)
    {
        if (0 == s_stMEDIACtx.as32VencStartCnt[i]) /* status:stop, action:start */
        {
            if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VPROC_StartPort(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
                HI_PERFORMANCE_TIME_STAMP;
                if (HI_SUCCESS != s32Ret)
                {
                    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                    MLOGE("StartPort[%d,%d] failed[%x]\n", pstVencCfg->ModHdl, pstVencCfg->ChnHdl, s32Ret);
                    return HI_FAILURE;
                }
            }

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_Start(VencHdl, s32FrameCnt);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                MLOGE("StartVenc[%d] failed[%x]\n", VencHdl, s32Ret);
                return HI_FAILURE;
            }
        }
        else /* status:start, action:request IDR */
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_RequestIDR(VencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                MLOGE("RequestIDR[%d] failed[%x]\n", VencHdl, s32Ret);
                return HI_FAILURE;
            }
        }
        MLOGI("RequestIDR[%d]\n", VencHdl);
        s_stMEDIACtx.as32VencStartCnt[i]++; /* update start count */
    }
    /* fixed framecount venc mode */
    else
    {
        MLOGD("framecnt %d\n", s32FrameCnt);
        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VPROC_StartPort(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                MLOGE("StartPort[%d,%d] failed[%x]\n", pstVencCfg->ModHdl, pstVencCfg->ChnHdl, s32Ret);
                return s32Ret;
            }
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_Start(VencHdl, s32FrameCnt);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("StartVenc[%d] failed[%x]\n", VencHdl, s32Ret);
            return s32Ret;
        }
        s_stMEDIACtx.as32VencStartCnt[i] = 1;
    }
    MLOGD("vencHdl[%d] startCnt[%d]\n", VencHdl, s_stMEDIACtx.as32VencStartCnt[i]);
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VencStop(HI_HANDLE VencHdl)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    //const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched vencIdx */
    s32Ret = PDT_MEDIA_GetVencIdx(VencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    //pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];

    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    if (0 == s_stMEDIACtx.as32VencStartCnt[i])
    {
        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
        MLOGD("vencHdl[%d] has already stopped\n", VencHdl);
        return HI_SUCCESS;
    }
    else if (1 == s_stMEDIACtx.as32VencStartCnt[i])
    {
        MLOGD("vencHdl[%d] stop\n", VencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_Stop(VencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("VencStop[%d] failed[%x]\n", VencHdl, s32Ret);
            return HI_FAILURE;
        }

#if 0
        s32Ret = HI_MAPI_VPROC_StopPort(pstVencCfg->VprocHdl, pstVencCfg->VportHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGE("StopPort[%d,%d] failed[%x]\n", pstVencCfg->VprocHdl, pstVencCfg->VportHdl, s32Ret);
            return HI_FAILURE;
        }
#endif
    }

    s_stMEDIACtx.as32VencStartCnt[i]--; /* update start count */
    MLOGD("vencHdl[%d] startCnt[%d]\n", VencHdl, s_stMEDIACtx.as32VencStartCnt[i]);
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VencReBind(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstDstSrc)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pstDstSrc, HI_PDT_MEDIA_EINVAL);

    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];
        if (HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCfg->enVencType)
        {
            if ((pstVencCfg->enBindedMod != pstDstSrc->enBindedMod)
              ||(pstVencCfg->ModHdl != pstDstSrc->ModHdl)
              ||(pstVencCfg->ChnHdl != pstDstSrc->ChnHdl))
            {
                if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)
                {
                    s32Ret = HI_MAPI_VENC_UnBindVCap(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, pstVencCfg->VencHdl);
                }
                else if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
                {
                    s32Ret = HI_MAPI_VENC_UnBindVProc(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, pstVencCfg->VencHdl, HI_FALSE);
                }

                if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstDstSrc->enBindedMod)
                {
                    s32Ret = HI_MAPI_VENC_BindVCap(pstDstSrc->ModHdl, pstDstSrc->ChnHdl, pstVencCfg->VencHdl);
                }
                else if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDstSrc->enBindedMod)
                {
                    s32Ret = HI_MAPI_VENC_BindVProc(pstDstSrc->ModHdl, pstDstSrc->ChnHdl, pstVencCfg->VencHdl, HI_FALSE);
                }
                else
                {
                    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                    MLOGE("Invalid Binded Module[%d]\n", pstDstSrc->enBindedMod);
                    return HI_PDT_MEDIA_EINVAL;
                }
                pstVencCfg->enBindedMod = pstDstSrc->enBindedMod;
                pstVencCfg->ModHdl = pstDstSrc->ModHdl;
                pstVencCfg->ChnHdl = pstDstSrc->ChnHdl;
            }
            break;
        }
    }
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
    return s32Ret;
}
#endif

HI_S32 HI_PDT_MEDIA_GetVideoInfo(HI_HANDLE VencHdl, HI_MEDIA_VIDEOINFO_S* pstVideoInfo)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    HI_APPCOMM_CHECK_POINTER(pstVideoInfo, HI_PDT_MEDIA_EINVAL);

    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];
        if (pstVencCfg->bEnable && (VencHdl == pstVencCfg->VencHdl))
        {
            memcpy(&pstVideoInfo->stVencAttr, &pstVencCfg->stVencAttr, sizeof(HI_MEDIA_VENC_ATTR_S));
            return HI_SUCCESS;
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_GetAudioInfo(HI_HANDLE AencHdl, HI_MEDIA_AUDIOINFO_S* pstAudioInfo)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    HI_APPCOMM_CHECK_POINTER(pstAudioInfo, HI_PDT_MEDIA_EINVAL);

    const HI_PDT_MEDIA_ACAP_CFG_S* pstAcapCfg = NULL;
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; ++i)
    {
        pstAencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAencCfg[i];
        if (AencHdl == pstAencCfg->AencHdl)
        {
            for (j = 0; j < HI_PDT_MEDIA_ACAP_MAX_CNT; ++j)
            {
                pstAcapCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg.astAcapCfg[j];
                if (pstAencCfg->AcapHdl == pstAcapCfg->AcapHdl)
                {
                    pstAudioInfo->u32SampleRate = (HI_U32)pstAcapCfg->stAcapAttr.enSampleRate;
                    if (AUDIO_BIT_WIDTH_8 == pstAcapCfg->stAcapAttr.enBitwidth)
                    {
                        pstAudioInfo->u16SampleBitWidth = 8;
                    }
                    else if (AUDIO_BIT_WIDTH_16 == pstAcapCfg->stAcapAttr.enBitwidth)
                    {
                        pstAudioInfo->u16SampleBitWidth = 16;
                    }
                    else if (AUDIO_BIT_WIDTH_24 == pstAcapCfg->stAcapAttr.enBitwidth)
                    {
                        pstAudioInfo->u16SampleBitWidth = 24;
                    }
                    else
                    {
                        MLOGE("Invalid SampleBitWidth\n");
                        pstAudioInfo->u16SampleBitWidth = 0;
                    }
                    pstAudioInfo->enSoundMode = pstAcapCfg->stAcapAttr.enSoundMode;
                }
            }
            pstAudioInfo->u32AvgBytesPerSec = (HI_U32)pstAencCfg->unAttr.stAACAttr.enBitRate;
            pstAudioInfo->u32PtNumPerFrm = pstAencCfg->u32PtNumPerFrm;
            pstAudioInfo->enAencFormat = pstAencCfg->enFormat;
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetVencAttr(HI_HANDLE VencHdl, const HI_MEDIA_VENC_ATTR_S* pstVencAttr)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched vencIdx */
    s32Ret = PDT_MEDIA_GetVencIdx(VencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];

    if(pstVencCfg->bEnable)
    {
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        MUTEX_LOCK(s_stMEDIACtx.CntMutex);
        if (0 != s_stMEDIACtx.as32VencStartCnt[i])
        {
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
            MLOGD("Venc[%d] not stopped, unsupport setVencAttr\n", VencHdl);
            return HI_PDT_MEDIA_EUNSUPPORT;
        }
        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif
        /* deinit venc */
        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_UnBindVProc(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, VencHdl, HI_FALSE);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VENC_UnBindVCap(pstVencCfg->ModHdl, pstVencCfg->ChnHdl, pstVencCfg->VencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            MLOGE("Invalid Binded Module[%d]\n", pstVencCfg->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VENC_Deinit(VencHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        /* init venc with new venc attribute */
        HI_PDT_MEDIA_VENC_CFG_S stVencCfg;
        memcpy(&stVencCfg, &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i],
            sizeof(HI_PDT_MEDIA_VENC_CFG_S));
        memcpy(&stVencCfg.stVencAttr, pstVencAttr, sizeof(HI_MEDIA_VENC_ATTR_S));
        s32Ret = PDT_MEDIA_InitVenc(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg, &stVencCfg, i);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        /* update venc attribute context */
        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i].stVencAttr,
            pstVencAttr, sizeof(HI_MEDIA_VENC_ATTR_S));
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetVencAttr(HI_HANDLE VencHdl, HI_MEDIA_VENC_ATTR_S* pstVencAttr)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched vencIdx */
    s32Ret = PDT_MEDIA_GetVencIdx(VencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];

    memcpy(pstVencAttr, &pstVencCfg->stVencAttr, sizeof(HI_MEDIA_VENC_ATTR_S));
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetTimeOSD(HI_S32 s32VcapDevIdx,HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    HI_APPCOMM_CHECK_EXPR((s32VcapDevIdx >=0 &&(s32VcapDevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)),HI_PDT_MEDIA_EINVAL);
    HI_S32 s32Batch = s32VcapDevIdx*10+HI_OSD_TYPE_TIME;

    HI_S32 i,j = 0;
    for (i = 0; i < s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.s32OsdCnt; ++i)
    {
        HI_OSD_ATTR_S* pstOsdAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.astOsdAttr[i];
        for (j = 0; j < pstOsdAttr->u32DispNum; ++j)
        {
            if (pstOsdAttr->astDispAttr[j].s32Batch==s32Batch)
            {
                pstOsdAttr->astDispAttr[j].bShow = bEnable;

            }
        }
    }

    if (bEnable==HI_TRUE)
    {
        PDT_MEDIA_StartOsd(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg);
    }
    return HI_OSD_Batch(s32Batch, bEnable);
}

HI_S32 HI_PDT_MEDIA_SetLogoOSD(HI_S32 s32VcapDevIdx,HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    HI_APPCOMM_CHECK_EXPR((s32VcapDevIdx >=0 &&(s32VcapDevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)),HI_PDT_MEDIA_EINVAL);
    HI_S32 s32Batch = s32VcapDevIdx*10+HI_OSD_TYPE_BITMAP;

    HI_S32 i,j = 0;
    for (i = 0; i < s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.s32OsdCnt; ++i)
    {
        HI_OSD_ATTR_S* pstOsdAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.astOsdAttr[i];
        for (j = 0; j < pstOsdAttr->u32DispNum; ++j)
        {
            if (pstOsdAttr->astDispAttr[j].s32Batch==s32Batch)
            {
                pstOsdAttr->astDispAttr[j].bShow = bEnable;

            }
        }
    }

    return HI_OSD_Batch(s32Batch, bEnable);
}

HI_S32 HI_PDT_MEDIA_SetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable = HI_FALSE;
    s32Ret = HI_PDT_MEDIA_GetLDC(VcapPipeHdl, VcapPipeChnHdl, &bPrevEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (bPrevEnable == bEnable)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable
                || VcapPipeHdl != pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable = bEnable;

                MLOGD("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_LDC,
                    &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                return HI_SUCCESS;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL* pbEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pbEnable, HI_PDT_MEDIA_EINVAL);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (VcapPipeHdl == pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl
                    && VcapPipeChnHdl == pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    *pbEnable = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[VcapPipeChnHdl].stLDCAttr.bEnable;
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 PDT_MEDIA_GetPipeChnMirrorFilp(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL* pbMirrorEnable,
    HI_BOOL* pbFlipEnable)
{
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (VcapPipeHdl == pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl
                    && VcapPipeChnHdl == pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    *pbMirrorEnable = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[VcapPipeChnHdl].bMirror;
                    *pbFlipEnable = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[VcapPipeChnHdl].bFlip;
                    MLOGD("pipe[%d] pipechn[%d] Mirrorenable[%d] Flipenable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,*pbMirrorEnable,
                        *pbFlipEnable);
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 PDT_MEDIA_GetVpssPortMirrorFilp(HI_HANDLE VpssHdl, HI_HANDLE PortHdl, HI_BOOL* pbMirrorEnable,
    HI_BOOL* pbFlipEnable)
{
    HI_PDT_MEDIA_VPSS_ATTR_S *pstVpssAttr = NULL;
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        pstVpssAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if (VpssHdl == pstVpssAttr->VpssHdl
                && PortHdl == pstVpssAttr->astVportAttr[j].VportHdl)
            {
                *pbMirrorEnable = pstVpssAttr->astVportAttr[j].bMirror;
                *pbFlipEnable = pstVpssAttr->astVportAttr[j].bFlip;
                MLOGD("VpssHdl[%d] PortHdl[%d] Mirrorenable[%d] Flipenable[%d]\n", VpssHdl, PortHdl,*pbMirrorEnable,
                    *pbFlipEnable);
                return HI_SUCCESS;
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}


HI_S32 HI_PDT_MEDIA_GetMirror(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL* pbEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pstFlipSrc, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pbEnable, HI_PDT_MEDIA_EINVAL);
    HI_BOOL bFilp = HI_FALSE;

    switch(pstFlipSrc->enBindedMod)
    {
        case HI_PDT_MEDIA_VIDEOMOD_VCAP:
            s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,pbEnable,&bFilp);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        case HI_PDT_MEDIA_VIDEOMOD_VPSS:
            s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,pbEnable,&bFilp);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        default:
            MLOGE("enBindedMod[%d] not support\n",pstFlipSrc->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetFlip(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL* pbEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pstFlipSrc, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pbEnable, HI_PDT_MEDIA_EINVAL);
    HI_BOOL bMirror = HI_FALSE;

    switch(pstFlipSrc->enBindedMod)
    {
        case HI_PDT_MEDIA_VIDEOMOD_VCAP:
            s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,&bMirror,pbEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        case HI_PDT_MEDIA_VIDEOMOD_VPSS:
            s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,&bMirror,pbEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        default:
            MLOGE("enBindedMod[%d] not support\n",pstFlipSrc->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_SetPipeChnFlip(HI_HANDLE VcapPipeHdl,HI_HANDLE VcapPipeChnHdl, HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable = HI_FALSE;
    HI_BOOL bMirror = HI_FALSE;
    s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(VcapPipeHdl, VcapPipeChnHdl, &bMirror, &bPrevEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if (bPrevEnable == bEnable)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable
                || VcapPipeHdl != pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                HI_MAPI_VCAP_MIRRORFLIP_ATTR_S stMirrorFlipAttr;
                stMirrorFlipAttr.bFlip = bEnable;
                stMirrorFlipAttr.bMirror = bMirror;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_MirrorFlip,
                    &stMirrorFlipAttr, sizeof(HI_MAPI_VCAP_MIRRORFLIP_ATTR_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = PDT_MEDIA_SetCompressMode(HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bFlip = bEnable;
                MLOGI("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,bEnable);
                return HI_SUCCESS;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_SetVpssPortFlip(HI_HANDLE VpssHdl,HI_HANDLE PortHdl, HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_VPSS_ATTR_S *pstVpssAttr = NULL;
    HI_S32 i, j;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable = HI_FALSE;
    HI_BOOL bMirror = HI_FALSE;
    s32Ret = PDT_MEDIA_GetVpssPortMirrorFilp(VpssHdl, PortHdl, &bMirror, &bPrevEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (bPrevEnable == bEnable)
    {
        return HI_SUCCESS;
    }
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        pstVpssAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i];
        if ((!pstVpssAttr->bEnable) || (pstVpssAttr->VpssHdl != VpssHdl))
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if (!pstVpssAttr->astVportAttr[j].bEnable
                || PortHdl != pstVpssAttr->astVportAttr[j].VportHdl)
            {
                continue;
            }
#if 0
            if (pstVpssAttr->stVpssAttr.bNrEn && pstVpssAttr->astVportAttr[j].bSupportBufferShare)
            {
                ROTATION_E enRotate =  ROTATION_BUTT;
                if (bEnable)
                {
                    enRotate = ROTATION_180;
                }
                else
                {
                    enRotate= ROTATION_0;
                }
                s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, PortHdl,
                    HI_VPROC_CMD_PortRotate, &enRotate, sizeof(ROTATION_E));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            else
#endif
            HI_MAPI_VPROC_FLIP_ATTR_S stVProcFlip;
            stVProcFlip.bEnable = bEnable;
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, PortHdl,
                HI_VPROC_CMD_PortFlip, &stVProcFlip, sizeof(HI_MAPI_VPROC_FLIP_ATTR_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            pstVpssAttr->astVportAttr[j].bFlip = bEnable;
            MLOGI("VpssHdl[%d] PortHdl[%d] enable[%d]\n", VpssHdl, PortHdl,bEnable);
            return HI_SUCCESS;
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetFlip(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pstFlipSrc, HI_PDT_MEDIA_EINVAL);

    switch(pstFlipSrc->enBindedMod)
    {
        case HI_PDT_MEDIA_VIDEOMOD_VCAP:
            s32Ret = PDT_MEDIA_SetPipeChnFlip(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,bEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        case HI_PDT_MEDIA_VIDEOMOD_VPSS:
            s32Ret = PDT_MEDIA_SetVpssPortFlip(pstFlipSrc->ModHdl,pstFlipSrc->ChnHdl,bEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        default:
            MLOGE("enBindedMod[%d] not support\n",pstFlipSrc->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_SetPipeChnMirror(HI_HANDLE VcapPipeHdl,HI_HANDLE VcapPipeChnHdl, HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable = HI_FALSE;
    HI_BOOL bFlip = HI_FALSE;
    s32Ret = PDT_MEDIA_GetPipeChnMirrorFilp(VcapPipeHdl, VcapPipeChnHdl, &bPrevEnable, &bFlip);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (bPrevEnable == bEnable)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable
                || VcapPipeHdl != pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl)
            {
                continue;
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                HI_MAPI_VCAP_MIRRORFLIP_ATTR_S stMirrorFlipAttr;
                stMirrorFlipAttr.bMirror = bEnable;
                stMirrorFlipAttr.bFlip = bFlip;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_MirrorFlip,
                    &stMirrorFlipAttr, sizeof(HI_MAPI_VCAP_MIRRORFLIP_ATTR_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = PDT_MEDIA_SetCompressMode(HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bMirror = bEnable;
                MLOGI("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,bEnable);
                return HI_SUCCESS;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_SetVpssPortMirror(HI_HANDLE VpssHdl,HI_HANDLE PortHdl, HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_VPSS_ATTR_S *pstVpssAttr = NULL;
    HI_S32 i, j;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable = HI_FALSE;
    HI_BOOL bFlip = HI_FALSE;
    s32Ret = PDT_MEDIA_GetVpssPortMirrorFilp(VpssHdl, PortHdl, &bPrevEnable, &bFlip);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (bPrevEnable == bEnable)
    {
        return HI_SUCCESS;
    }
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        pstVpssAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i];
        if ((!pstVpssAttr->bEnable) || (pstVpssAttr->VpssHdl != VpssHdl))
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if (!pstVpssAttr->astVportAttr[j].bEnable
                || PortHdl != pstVpssAttr->astVportAttr[j].VportHdl)
            {
                continue;
            }

            HI_MAPI_VPROC_MIRROR_ATTR_S stVProcMirror;
            stVProcMirror.bEnable = bEnable;
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, PortHdl,
                HI_VPROC_CMD_PortMirror, &stVProcMirror, sizeof(HI_MAPI_VPROC_MIRROR_ATTR_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            pstVpssAttr->astVportAttr[j].bMirror = bEnable;
            MLOGI("VpssHdl[%d] PortHdl[%d] enable[%d]\n", VpssHdl, PortHdl,bEnable);
            return HI_SUCCESS;
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetMirror(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstMirrorSrc, HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pstMirrorSrc, HI_PDT_MEDIA_EINVAL);

    switch(pstMirrorSrc->enBindedMod)
    {
        case HI_PDT_MEDIA_VIDEOMOD_VCAP:
            s32Ret = PDT_MEDIA_SetPipeChnMirror(pstMirrorSrc->ModHdl,pstMirrorSrc->ChnHdl,bEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        case HI_PDT_MEDIA_VIDEOMOD_VPSS:
            s32Ret = PDT_MEDIA_SetVpssPortMirror(pstMirrorSrc->ModHdl,pstMirrorSrc->ChnHdl,bEnable);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HI_PDT_MEDIA_EINVAL);
            break;
        default:
            MLOGE("enBindedMod[%d] not support\n",pstMirrorSrc->enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
    }
    return HI_SUCCESS;
}


HI_S32 HI_PDT_MEDIA_SetBrightness(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32 s32Brightness)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(s32Brightness, 0, 100), HI_PDT_MEDIA_EINVAL);

    /* Check Value Change or not */
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32PrevBrightness = HI_FALSE;
    s32Ret = HI_PDT_MEDIA_GetBrightness(VcapPipeHdl, VcapPipeChnHdl, &s32PrevBrightness);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (s32PrevBrightness == s32Brightness)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable
                || VcapPipeHdl != pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl)
            {
                continue;
            }

            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                MLOGD("pipe[%d] pipechn[%d] value[%d]\n", VcapPipeHdl, VcapPipeChnHdl, s32Brightness);
                HI_U8 u8Brightness = s32Brightness;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl,
                    HI_MAPI_VCAP_CMD_ISP_Luma, &u8Brightness, sizeof(HI_U8));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Brightness = s32Brightness;
                return HI_SUCCESS;
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_GetBrightness(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32* ps32Brightness)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(ps32Brightness, HI_PDT_MEDIA_EINVAL);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (VcapPipeHdl == pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl
                    && VcapPipeChnHdl == pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    *ps32Brightness = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Brightness;
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_SetSaturation(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32 s32Saturation)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(s32Saturation, 0, 100), HI_PDT_MEDIA_EINVAL);

    /* Check Value Change or not */
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32PrevSaturation = HI_FALSE;
    s32Ret = HI_PDT_MEDIA_GetSaturation(VcapPipeHdl, VcapPipeChnHdl, &s32PrevSaturation);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    if (s32PrevSaturation == s32Saturation)
    {
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if (!pstDevAttr->astVcapPipeAttr[j].bEnable
                || VcapPipeHdl != pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl)
            {
                continue;
            }

            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                MLOGD("pipe[%d] pipechn[%d] value[%d]\n", VcapPipeHdl, VcapPipeChnHdl, s32Saturation);
                HI_U8 u8Saturation = s32Saturation;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl,
                    HI_MAPI_VCAP_CMD_ISP_Saturation, &u8Saturation, sizeof(HI_U8));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Saturation = s32Saturation;
                return HI_SUCCESS;
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_GetSaturation(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32* ps32Saturation)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(ps32Saturation, HI_PDT_MEDIA_EINVAL);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    HI_S32 i, j, k = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (VcapPipeHdl == pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl
                    && VcapPipeChnHdl == pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    *ps32Saturation = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].s32Saturation;
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

HI_S32 HI_PDT_MEDIA_SetAoVolume(HI_HANDLE AoHdl, HI_S32 s32Volume)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioOutInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(AoHdl, 0, HI_PDT_MEDIA_AO_MAX_CNT), HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(s32Volume, 0, 100), HI_PDT_MEDIA_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; i++)
    {
        if (s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].bEnable
            && AoHdl == s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].AoHdl)
        {
            HI_S32 s32AudioGain;
            PDT_MEDIA_SwitchAoGain(s32Volume,&s32AudioGain);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AO_SetVolume(AoHdl, s32AudioGain);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].s32Volume = s32Volume;
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetAoVolume(HI_HANDLE AoHdl, HI_S32* ps32Volume)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioOutInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(AoHdl, 0, HI_PDT_MEDIA_AO_MAX_CNT), HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(ps32Volume, HI_PDT_MEDIA_EINVAL);

    HI_S32 i = 0;
    *ps32Volume = 0;
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; i++)
    {
        if (s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].bEnable
            && AoHdl == s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].AoHdl)
        {
            *ps32Volume = s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].s32Volume;
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AoMute(HI_HANDLE AoHdl)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioOutInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(AoHdl, 0, HI_PDT_MEDIA_AO_MAX_CNT), HI_PDT_MEDIA_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; i++)
    {
        if (s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].bEnable
            && AoHdl == s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].AoHdl)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AO_Mute(AoHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AoUnmute(HI_HANDLE AoHdl)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioOutInit);
    HI_APPCOMM_CHECK_EXPR(HI_APPCOMM_CHECK_RANGE(AoHdl, 0, HI_PDT_MEDIA_AO_MAX_CNT), HI_PDT_MEDIA_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; i++)
    {
        if (s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].bEnable
            && AoHdl == s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg.astAoCfg[i].AoHdl)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AO_Unmute(AoHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VencInit(const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    VENC_HIERARCHICAL_QP_S stHierarchicalQp;
    HI_APPCOMM_CHECK_POINTER(pstVencCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    /* Venc Init */
    MLOGD("Venc_Hdl[%d]\n",pstVencCfg->VencHdl);
    memcpy(&stVencAttr.stVencPloadTypeAttr, &pstVencCfg->stVencAttr.stTypeAttr, sizeof(HI_MAPI_VENC_TYPE_ATTR_S));
    PDT_MEDIA_SwitchRcTypeAttr(&pstVencCfg->stVencAttr ,&stVencAttr.stRcAttr, &stHierarchicalQp);
    s32Ret = HI_MAPI_VENC_Init(pstVencCfg->VencHdl, &stVencAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_VENC_SetAttr(pstVencCfg->VencHdl, &stVencAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_SetFlickerFreq50HZ(HI_BOOL bEable)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i= 0;
    HI_S32 s32FrameRate = 0;
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDevAttr = NULL;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstDevAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i];
        if (!pstDevAttr->bEnable)
        {
            continue;
        }
        s32FrameRate = ((HI_TRUE == bEable)?((s32FrameRate/25)*25):s32FrameRate);
        if (s32FrameRate <= 0)
        {
            continue;
        }
        #if 0
        MLOGI("pstDevAttr->VcapDevHdl[%d],s32FrameRate[%d]\n",pstDevAttr->VcapDevHdl,s32FrameRate);
        s32Ret = HI_MAPI_VCAP_SetSensorFrameRate(pstDevAttr->VcapDevHdl,&s32FrameRate);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        #else

        HI_S32 j = 0, k = 0;
        HI_MAPI_VCAP_ATTR_S stVcapAttr;
        PDT_MEDIA_SwitchVcapAttr(pstDevAttr, &stVcapAttr);
        for (j = 0; j < stVcapAttr.u32PipeBindNum; ++j)
        {
            HI_HANDLE PipeHdl = stVcapAttr.aPipeIdBind[j];
            stVcapAttr.astVcapPipeAttr[PipeHdl].stIspPubAttr.f32FrameRate = s32FrameRate;
            stVcapAttr.astVcapPipeAttr[PipeHdl].stFrameRate.s32SrcFrameRate= s32FrameRate;
            stVcapAttr.astVcapPipeAttr[PipeHdl].stFrameRate.s32DstFrameRate= s32FrameRate;
            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                stVcapAttr.astVcapPipeAttr[PipeHdl].astPipeChnAttr[k].stFrameRate.s32SrcFrameRate = s32FrameRate;
                stVcapAttr.astVcapPipeAttr[PipeHdl].astPipeChnAttr[k].stFrameRate.s32DstFrameRate = s32FrameRate;

            }
        }
        s32Ret = HI_MAPI_VCAP_SetAttr(pstDevAttr->VcapDevHdl, &stVcapAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        #endif
    }
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        VENC_FRAME_RATE_S stVencCmdFps;
        HI_MEDIA_VENC_ATTR_S* pstVencAttr = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i].stVencAttr;

        if (HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
        {
            if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_H264_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH264Cbr;
                stVencCmdFps.s32SrcFrmRate= pstAttrCbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrCbr->stAttr.u32SrcFrameRate;
            }
            else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_H265_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stH265Cbr;
                stVencCmdFps.s32SrcFrmRate = pstAttrCbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrCbr->stAttr.u32SrcFrameRate;
            }
            else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_MJPEG_CBR_S* pstAttrCbr = &pstVencAttr->stRcAttr.unAttr.stMjpegCbr;
                stVencCmdFps.s32SrcFrmRate = pstAttrCbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrCbr->stAttr.u32SrcFrameRate;
            }
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencAttr->stRcAttr.enRcMode)
        {
            if (HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_H264_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH264Vbr;
                stVencCmdFps.s32SrcFrmRate = pstAttrVbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrVbr->stAttr.u32SrcFrameRate;
            }
            else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_H265_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
                stVencCmdFps.s32SrcFrmRate = pstAttrVbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrVbr->stAttr.u32SrcFrameRate;
            }
            else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencAttr->stTypeAttr.enType)
            {
                const HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stMjpegVbr;
                stVencCmdFps.s32SrcFrmRate = pstAttrVbr->stAttr.u32SrcFrameRate;
                stVencCmdFps.s32DstFrmRate= pstAttrVbr->stAttr.u32SrcFrameRate;
            }
        }

        s32FrameRate = stVencCmdFps.s32SrcFrmRate;
        stVencCmdFps.s32SrcFrmRate = ((HI_TRUE == bEable)?((s32FrameRate/25)*25):s32FrameRate);
        if (stVencCmdFps.s32SrcFrmRate <= 0)
        {
            continue;
        }
        /* set venc cmd frame date */
        if(HI_MAPI_PAYLOAD_TYPE_JPEG != pstVencAttr->stTypeAttr.enType)
        {
            HI_HANDLE VencHdl = s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i].VencHdl;
            MLOGD("Venc[%d] s32SrcFrmRate[%d] s32DstFrmRate[%d]\n",
                VencHdl, stVencCmdFps.s32SrcFrmRate,stVencCmdFps.s32DstFrmRate);
            s32Ret = HI_MAPI_VENC_SetAttrEx(VencHdl, HI_MAPI_VENC_CMD_FRAME_RATE,
                &stVencCmdFps, sizeof(VENC_FRAME_RATE_S));
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_InitOSD(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstOsdCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    return PDT_MEDIA_InitOsd(pstOsdCfg);
}

HI_S32 HI_PDT_MEDIA_DeinitOSD(HI_VOID)
{
    return PDT_MEDIA_DeinitOsd();
}

HI_S32 HI_PDT_Media_UpdateDispCfg(HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg,HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg,SIZE_S* pstSrcSize)
{
    HI_APPCOMM_CHECK_POINTER(pstDispCfg, HI_PDT_MEDIA_EINVAL);
    HI_S32 i = 0;
    HI_S32 s32Ret;
#ifdef CONFIG_SCREEN
    HI_HAL_SCREEN_ATTR_S stScreenAttr;
    memset(&stScreenAttr, 0, sizeof(stScreenAttr));
    s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_GetAttr");
#if defined(CONFIG_SCREEN_OTA5182)
    pstDispCfg->stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_320x240_60;
#else
    pstDispCfg->stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
#endif
    switch(stScreenAttr.enType)
    {
        case HI_HAL_SCREEN_INTF_TYPE_MIPI:
        {
            pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        }
        case HI_HAL_SCREEN_INTF_TYPE_LCD:
        {
            switch (stScreenAttr.unScreenAttr.stLcdAttr.enType)
            {
                case HI_HAL_SCREEN_LCD_INTF_6BIT:
                    pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_6BIT;
                    break;
                case HI_HAL_SCREEN_LCD_INTF_8BIT:
                    pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_8BIT;
                    break;
                case HI_HAL_SCREEN_LCD_INTF_16BIT:
                    pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_16BIT;
                    break;
                case HI_HAL_SCREEN_LCD_INTF_24BIT:
                    pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_24BIT;
                    break;
                default:
                    MLOGE("Invalid lcd type\n");
                    return HI_PDT_MEDIA_EUNSUPPORT;
            }
            break;
        }
        default:
            MLOGE("Invalid screen type\n");
            return HI_PDT_MEDIA_EUNSUPPORT;
    }

    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.bSynm = 1; /**<sync mode: signal */
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.bIop  = 1; /**<progressive display */
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Vact = stScreenAttr.stAttr.stSynAttr.u16Vact;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Vbb  = stScreenAttr.stAttr.stSynAttr.u16Vbb;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Vfb  = stScreenAttr.stAttr.stSynAttr.u16Vfb;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Hact = stScreenAttr.stAttr.stSynAttr.u16Hact;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Hbb  = stScreenAttr.stAttr.stSynAttr.u16Hbb;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Hfb  = stScreenAttr.stAttr.stSynAttr.u16Hfb;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Hpw  = stScreenAttr.stAttr.stSynAttr.u16Hpw;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.u16Vpw  = stScreenAttr.stAttr.stSynAttr.u16Vpw;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.bIdv = stScreenAttr.stAttr.stSynAttr.bIdv;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.bIhs = stScreenAttr.stAttr.stSynAttr.bIhs;
    pstDispCfg->stDispAttr.stPubAttr.stSyncInfo.bIvs = stScreenAttr.stAttr.stSynAttr.bIvs;


    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.bClkReverse = stScreenAttr.stAttr.stClkAttr.bClkReverse;
    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.u32DevDiv = stScreenAttr.stAttr.stClkAttr.u32DevDiv;
    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.u32PreDiv = 1;
    pstDispCfg->stDispAttr.stUserInfoAttr.u32DevFrameRate = stScreenAttr.stAttr.u32Framerate;

    if (HI_HAL_SCREEN_CLK_TYPE_LCDMCLK ==
        stScreenAttr.stAttr.stClkAttr.enClkType)
    {
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.enClkSource = VO_CLK_SOURCE_LCDMCLK;
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.u32LcdMClkDiv =        \
                                        stScreenAttr.stAttr.stClkAttr.u32OutClk;
    }
    else if (HI_HAL_SCREEN_CLK_TYPE_PLL ==
        stScreenAttr.stAttr.stClkAttr.enClkType)
    {
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Fbdiv =
            stScreenAttr.stAttr.stClkAttr.stClkPll.u32Fbdiv;
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Frac =
            stScreenAttr.stAttr.stClkAttr.stClkPll.u32Frac;
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Refdiv =
            stScreenAttr.stAttr.stClkAttr.stClkPll.u32Refdiv;
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Postdiv1 =
            stScreenAttr.stAttr.stClkAttr.stClkPll.u32Postdiv1;
        pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.stUserSyncPll.u32Postdiv2 =
            stScreenAttr.stAttr.stClkAttr.stClkPll.u32Postdiv2;
    }

    /* config AspectRatio Attr*/
    for(i=0;i<HI_PDT_MEDIA_DISP_WND_MAX_CNT;i++)
    {
        if(ASPECT_RATIO_MANUAL == pstDispCfg->astWndCfg[i].stAspectRatio.enMode && NULL != pstSrcSize)
        {
            SIZE_S stDispSize;
            stDispSize.u32Width = stScreenAttr.stAttr.u32Width;
            stDispSize.u32Height = stScreenAttr.stAttr.u32Height;
            SIZE_S stSrcSize = *pstSrcSize;
            if (ROTATION_90 == pstDispCfg->astWndCfg[i].enRotate || ROTATION_270 == pstDispCfg->astWndCfg[i].enRotate)
            {
                SWAP(stSrcSize.u32Width, stSrcSize.u32Height);
            }
            PDT_MEDIA_GetAspectRatio(&stDispSize, &stSrcSize, &pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect);
        }
        else
        {
            pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.s32X = 0;
            pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.s32Y = 0;
            pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.u32Width = stScreenAttr.stAttr.u32Width;
            pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.u32Height = stScreenAttr.stAttr.u32Height;
        }
#if defined(CONFIG_VERTICAL_SCREEN)
        if (ROTATION_0 == pstDispCfg->astWndCfg[i].enRotate)
        {
            pstDispCfg->astWndCfg[i].enRotate = ROTATION_90;
        }
#endif
        /* only change first vo_wndhdl Attr */
        if(i == 0)
        {
            pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Width  = pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.u32Width;
            pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Height = pstDispCfg->astWndCfg[i].stAspectRatio.stVideoRect.u32Height;
            pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X = 0;
            pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = 0;
        }
        else
        {
            /* change others vo_wndhdl Attr  */
            if (ROTATION_90 == pstDispCfg->astWndCfg[i].enRotate || ROTATION_270 == pstDispCfg->astWndCfg[i].enRotate)
            {
                SWAP(pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Width, pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Height);
                SWAP(pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X,pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y);
            }

            /*coordinate transformation */
            HI_S32 ImageWidht  = pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Width;
            HI_S32 ImageHeight = pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Height;
            HI_S32 x = pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X;
            HI_S32 y = pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y;

            switch (pstDispCfg->astWndCfg[i].enRotate)
            {
                case ROTATION_0:
                case ROTATION_BUTT:
                    break;

                case ROTATION_90:
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X = ImageWidht - pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Width - x;
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = y;
                    break;

                case ROTATION_180:
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X = ImageWidht - pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Width - x;
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = ImageHeight - pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Height - y;
                    break;

                case ROTATION_270:
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32X = x;
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = ImageHeight - pstDispCfg->astWndCfg[i].stWndAttr.stRect.u32Height - y;
                    break;
            }
            #ifdef REARVIEW
                if(pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y < UI_FILELIST_WINDOW_LEFT_LEN)
                {
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = UI_FILELIST_WINDOW_LEFT_LEN;
                }
                else if(ImageHeight - pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y < UI_FILELIST_WINDOW_LEFT_LEN)
                {
                    pstDispCfg->astWndCfg[i].stWndAttr.stRect.s32Y = ImageHeight - UI_FILELIST_WINDOW_LEFT_LEN;
                }
            #endif
        }
    }

#elif defined(HDMI_TEST)
    pstDispCfg->VdispHdl = 0;
    pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
    pstDispCfg->stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    pstDispCfg->stCscAttr.bEnable = HI_FALSE;
#endif
    pstDispCfg->stImageSize.u32Width = pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Width;
    pstDispCfg->stImageSize.u32Height= pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Height;

    /* update vport Attr which bind vo_wnd*/
    for (i = 0 ; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; i++)
    {
        HI_PDT_MEDIA_DISP_WND_CFG_S* pstWndCfg = &pstDispCfg->astWndCfg[i];
        if (pstWndCfg->enBindedMod != HI_PDT_MEDIA_VIDEOMOD_VPSS)
        {
            continue;
        }
        if (!pstWndCfg->bEnable)
        {
            continue;
        }
        if (i == MAIN_PREVIEW_VOWND)
        {
            continue;
        }
        else
        {
            HI_U32 u32VpssIndex,u32VportIndex;
            s32Ret = PDT_MEDIA_GetVpssIndex(pstVprocCfg, pstWndCfg->ModHdl, pstWndCfg->ChnHdl, &u32VpssIndex, &u32VportIndex);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_MEDIA_EINVAL);

            HI_U32 u32VpssWidth  = pstVprocCfg->astVpssAttr[u32VpssIndex].stVpssAttr.u32MaxW;
            HI_U32 u32VpssHeight = pstVprocCfg->astVpssAttr[u32VpssIndex].stVpssAttr.u32MaxH;
            HI_U32 u32VoWndWidth  = pstWndCfg->stWndAttr.stRect.u32Width;
            HI_U32 u32VoWndHeight = pstWndCfg->stWndAttr.stRect.u32Height;
            MLOGI("Vpss_w[%d]_h[%d],Vo_w[%d]_h[%d]\n", u32VpssWidth, u32VpssHeight, u32VoWndWidth, u32VoWndHeight);
#if defined(CONFIG_VERTICAL_SCREEN)
            if ((u32VpssWidth > (u32VoWndHeight * MAX_VPSSPHYCHN_SCALE)) || (u32VpssHeight > (u32VoWndWidth * MAX_VPSSPHYCHN_SCALE)))
            {
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Width  =
                    HI_APPCOMM_ALIGN(((u32VpssWidth + MAX_VPSSPHYCHN_SCALE - 1) / MAX_VPSSPHYCHN_SCALE), 4);
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Height =
                    HI_APPCOMM_ALIGN(((u32VpssHeight + MAX_VPSSPHYCHN_SCALE - 1) / MAX_VPSSPHYCHN_SCALE), 4);
            }
            else
            {
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Width =
                    pstWndCfg->stWndAttr.stRect.u32Height;
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Height =
                    pstWndCfg->stWndAttr.stRect.u32Width;
            }
#else
            if ((u32VpssWidth > (u32VoWndWidth * MAX_VPSSPHYCHN_SCALE)) || (u32VpssHeight > (u32VoWndHeight * MAX_VPSSPHYCHN_SCALE)))
            {
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Width  =
                    HI_APPCOMM_ALIGN(((u32VpssWidth + MAX_VPSSPHYCHN_SCALE - 1) / MAX_VPSSPHYCHN_SCALE), 4);
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Height =
                    HI_APPCOMM_ALIGN(((u32VpssHeight + MAX_VPSSPHYCHN_SCALE - 1) / MAX_VPSSPHYCHN_SCALE), 4);
            }
            else
            {
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Width =
                    pstWndCfg->stWndAttr.stRect.u32Width;
                pstVprocCfg->astVpssAttr[u32VpssIndex].astVportAttr[u32VportIndex].stResolution.u32Height =
                    pstWndCfg->stWndAttr.stRect.u32Height;
            }
#endif
        }
    }

    pstDispCfg->u32BufferLen = VO_SD_DISP_MIN_BLK;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

