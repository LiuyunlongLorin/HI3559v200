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
#include "hi_math.h"
#include "hi_product_media.h"
#include "product_media_inner.h"
#include "hi_product_param.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_aenc.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_hdmi.h"
#include "hi_appcomm_util.h"
#include "hi_hal_screen.h"

#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
#include "hi_dsp.h"
#include "mpi_dsp.h"
#endif
#endif
#include "mpi_sys.h"

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
#include "hi_eventhub.h"
#endif
#include "hi_player.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_IMAGE_WIDTH    (4096)

/** media information physical address */
#define PDT_MEDIA_INFO_PHY_ADDR    (CFG_PARAM_PHY_ADDR + 0x40000)

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
    HI_BOOL bAudioOutStart;
} PDT_MEDIA_INFO_S;

/** Media Module Context */
typedef struct tagPDT_MEDIA_CONTEXT_S
{
    HI_BOOL bMediaInit;
    HI_BOOL bVideoInit;
    HI_BOOL bAudioInit;
    HI_BOOL bVideoOutInit;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    pthread_mutex_t CntMutex; /**<protect as32VencStartCnt/as32AencStartCnt */
    HI_S32  as32VpssVportStartCnt[HI_PDT_MEDIA_VPSS_MAX_CNT][HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];/**< vpss vport start count,
            only canbe modified by HI_PDT_MEDIA_VencStart/HI_PDT_MEDIA_VencStop */
    HI_S32  as32VencStartCnt[HI_PDT_MEDIA_VENC_MAX_CNT]; /**< venc start count,
            only canbe modified by HI_PDT_MEDIA_VencStart/HI_PDT_MEDIA_VencStop */
    HI_S32  as32AencStartCnt[HI_PDT_MEDIA_AENC_MAX_CNT]; /**< aenc start count,
            only canbe modified by HI_PDT_MEDIA_AencStart/HI_PDT_MEDIA_AencStop */

    HI_BOOL bHDMIStart;
#endif

    PDT_MEDIA_INFO_S* pstMediaInfo;
    PDT_MEDIA_VCAP_AE_INFO_S astVcapAEInfo[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_BOOL bCompress;
    HI_BOOL bLoadDsp;
} PDT_MEDIA_CONTEXT_S;
static PDT_MEDIA_CONTEXT_S s_stMEDIACtx =
{
    .bMediaInit = HI_FALSE,
    .bVideoInit = HI_FALSE,
    .bAudioInit = HI_FALSE,
    .bVideoOutInit = HI_FALSE,
    .bCompress = HI_TRUE,
    .bLoadDsp = HI_FALSE
};

#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
static HI_S32 PDT_LoadDspCoreBinary(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

#ifndef DPHOTO_POSTPROCESS_ARCH_DUAL
    HI_MAPI_Sys_DspAttr_S stDspAttr;
    stDspAttr.enDsp = SVP_DSP_ID_0;
    strcpy(stDspAttr.aszBinPath[SVP_DSP_ID_0*4+0], "/app/sharefs/hi_sram.bin");
    strcpy(stDspAttr.aszBinPath[SVP_DSP_ID_0*4+1], "/app/sharefs/hi_iram0.bin");
    strcpy(stDspAttr.aszBinPath[SVP_DSP_ID_0*4+2], "/app/sharefs/hi_dram0.bin");
    strcpy(stDspAttr.aszBinPath[SVP_DSP_ID_0*4+3], "/app/sharefs/hi_dram1.bin");
    HI_TIME_STAMP;
    s32Ret = HI_MAPI_Sys_EnableDsp(&stDspAttr);
    HI_TIME_STAMP;
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_Sys_EnableDsp failed with %#x!\n",  s32Ret);
        return s32Ret;
    }
#else
    SVP_DSP_ID_E enCoreId = SVP_DSP_ID_0;

    HI_CHAR * aszBin[4] =
    {"/app/dsp_bin/dsp0/hi_sram.bin",
     "/app/dsp_bin/dsp0/hi_iram0.bin",
     "/app/dsp_bin/dsp0/hi_dram0.bin",
     "/app/dsp_bin/dsp0/hi_dram1.bin"};

    s32Ret = HI_MPI_SVP_DSP_PowerOn(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_PowerOn failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[0], enCoreId * 4 + 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[1], enCoreId * 4 + 1);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[2], enCoreId * 4 + 2);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[3], enCoreId * 4 + 3);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_EnableCore(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_EnableCore failed with %#x!\n",  s32Ret);
        return s32Ret;
    }
#endif
    return HI_SUCCESS;
}

static HI_S32 PDT_UnloadDspCoreBinary(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
#ifndef DPHOTO_POSTPROCESS_ARCH_DUAL
    HI_MAPI_Sys_DspAttr_S stDspAttr;
    stDspAttr.enDsp = SVP_DSP_ID_0;

    s32Ret = HI_MAPI_Sys_DisableDsp(&stDspAttr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_Sys_DisableDsp failed with %#x!\n",  s32Ret);
        return s32Ret;
    }
#else
    SVP_DSP_ID_E enCoreId = SVP_DSP_ID_0;

    s32Ret = HI_MPI_SVP_DSP_DisableCore(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_DisableCore failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_PowerOff(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MPI_SVP_DSP_PowerOff failed with %#x!\n",  s32Ret);
        return s32Ret;
    }
#endif
    return HI_SUCCESS;
}
#endif
#endif

static HI_VOID PDT_MEDIA_SwitchRcTypeAttr(const HI_MEDIA_VENC_ATTR_S* pstVencAttr,
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
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_H265_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencAttr->stTypeAttr.enType)
        {
            const HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstAttrVbr = &pstVencAttr->stRcAttr.unAttr.stMjpegVbr;
            memcpy(&pstVencRcAttr->stAttrVbr, &pstAttrVbr->stAttr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
        }
        pstHierarchicalQp->bHierarchicalQpEn =
                ((PDT_MEDIA_HIGHFRAME_FPS <= pstVencRcAttr->stAttrVbr.fr32DstFrameRate) ? HI_TRUE : HI_FALSE);
        MLOGD("  VBR: MaxBitRate[%u] srcFrmRate[%ufps] dstFrmRate[%ufps] Gop[%u] StatTime[%uS]\n",
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

static HI_S32 PDT_MEDIA_InitSensor(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr)
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
    stSnsAttr.enWdrMode = pstVcapDevAttr->stSnsAttr.enWdrMode;
    MLOGD("VcapDevHdl[%d] Size[%ux%u] SnsMode[%u] WdrMode[%d]\n",
        pstVcapDevAttr->VcapDevHdl, stSnsAttr.stSize.u32Width, stSnsAttr.stSize.u32Height,
        stSnsAttr.u8SnsMode, stSnsAttr.enWdrMode);
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

static HI_VOID PDT_MEDIA_SwitchVcapAttr(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstMediaDevAttr,
                HI_MAPI_VCAP_ATTR_S* pstVcapAttr)
{
    memset(pstVcapAttr, 0x0, sizeof(HI_MAPI_VCAP_ATTR_S));
    pstVcapAttr->stVcapDevAttr.stBasSize = pstMediaDevAttr->stResolution;
    pstVcapAttr->stVcapDevAttr.enWdrMode = pstMediaDevAttr->enWdrMode;

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
        memset(pstMapiPipeAttr, 0, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));

        if(s_stMEDIACtx.bCompress)
        {
            pstMapiPipeAttr->enCompressMode = COMPRESS_MODE_LINE;
            MLOGD(LIGHT_BLUE"Vcap Pipe[%d] enCompressMode[%d]\n"NONE,
                s32PipeIdx, pstMapiPipeAttr->enCompressMode);
        }
        pstMapiPipeAttr->bIspBypass = pstPipeAttr->bIspBypass;
        pstMapiPipeAttr->enPipeType = pstPipeAttr->enPipeType;
        pstMapiPipeAttr->stFrameRate = pstPipeAttr->stFrameRate;
        pstMapiPipeAttr->stIspPubAttr.stSize = pstPipeAttr->stIspPubAttr.stSize;
        pstMapiPipeAttr->stIspPubAttr.enBayer = BAYER_BUTT;
        pstMapiPipeAttr->stIspPubAttr.f32FrameRate = pstPipeAttr->stIspPubAttr.f32FrameRate;

        if(0 == s32PipeIdx)
        {
            MLOGD("set stBasSize as pipe0 isp size\n");
            pstVcapAttr->stVcapDevAttr.stBasSize = pstMapiPipeAttr->stIspPubAttr.stSize;
        }

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
               (ROTATION_0 == pstPipeChnAttr->enRotate))
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

static HI_S32 PDT_MEDIA_StartVcapDev(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr,
    HI_BOOL bStartISP, HI_BOOL bSensorAttrChange)
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

        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChn = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            if (!pstPipeChn->bEnable)
            {
                continue;
            }

#ifdef CFG_MEDIA_VCAP_STANDBY
            if((0 == s32PipeIdx) &&
               (0 == s32PipeChnIdx) &&
               (HI_FALSE == bSensorAttrChange))
            {
               MLOGI(LIGHT_BLUE"Vcap Pipe[%d] Chn[%d] RESTART_SENSOR!\n"NONE, s32PipeIdx, s32PipeChnIdx);
               HI_PERFORMANCE_TIME_STAMP;
               s32Ret = HI_MAPI_VCAP_SetAttrEx(s32PipeIdx, s32PipeChnIdx, HI_MAPI_VCAP_CMD_RESTART_SENSOR, HI_NULL, 0);
               HI_PERFORMANCE_TIME_STAMP;
               HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
#endif
            MLOGD("StartChn: DevHdl[%d] PipeHdl[%d] PipeChnHdl[%d]\n",
                pstVcapDevAttr->VcapDevHdl, pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StartChn(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
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
            s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_ISP_Set3AInit,
                        &stIspInitAttr, sizeof(ISP_INIT_ATTR_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
        }

        ROTATION_E enRotate = pstPipeAttr->astPipeChnAttr[0].enRotate;
        MLOGI(LIGHT_BLUE"enRotate[%d]\n"NONE,enRotate);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_Rotate,
            &enRotate, sizeof(ROTATION_E));
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        if(!pstPipeAttr->astPipeChnAttr[0].stDISAttr.bEnable)
        {
            HI_MAPI_VCAP_MIRRORFLIP_ATTR_S stMirrorFlipAttr;
            stMirrorFlipAttr.bFlip   = pstPipeAttr->astPipeChnAttr[0].bFlip;
            stMirrorFlipAttr.bMirror = pstPipeAttr->astPipeChnAttr[0].bMirror;
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_SetAttrEx(pstPipeAttr->VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_MirrorFlip,
                &stMirrorFlipAttr, sizeof(HI_MAPI_VCAP_MIRRORFLIP_ATTR_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        if(bStartISP)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StartISP(pstPipeAttr->VcapPipeHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
#ifdef CFG_MEDIA_VCAP_STANDBY
        else
        {
            if(1 == s32PipeIdx)
            {
                MLOGI(LIGHT_BLUE"PipeIdx[%d] StartISP!\n"NONE,s32PipeIdx);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_StartISP(pstPipeAttr->VcapPipeHdl);
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            if(0 == s32PipeIdx)
            {
                if((pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx].bEnable) &&
                   (pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx].astPipeChnAttr[0].bEnable))
                {
                    MLOGI(LIGHT_BLUE"PipeIdx[%d] PipeChnIdx[0] EXIT_STANDBY!\n"NONE,s32PipeIdx);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(s32PipeIdx, 0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0);
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }
#endif

        if (0 < strnlen((const HI_CHAR*)pstVcapDevAttr->stExifInfo.au8ImageDescription, HI_MAPI_EXIF_DRSCRIPTION_LENGTH))
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_SetExifInfo(pstPipeAttr->VcapPipeHdl,(HI_MAPI_SNAP_EXIF_INFO_S*)&pstVcapDevAttr->stExifInfo);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
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
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        const HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstPipeAttr = &pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx];
        if (!pstPipeAttr->bEnable)
        {
            continue;
        }

        if (!s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet &&
            HI_MAPI_PIPE_TYPE_VIDEO == pstPipeAttr->enPipeType)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_GetAttrEx(pstPipeAttr->VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_ISP_GetExposureInfo,
                &s_stMEDIACtx.astVcapAEInfo[s32DevIdx].stExpInfo, sizeof(HI_MAPI_VCAP_EXPOSURE_INFO_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_GetAttrEx(pstPipeAttr->VcapPipeHdl, 0, HI_MAPI_VCAP_CMD_ISP_GetAwbInfo,
                &s_stMEDIACtx.astVcapAEInfo[s32DevIdx].stWBInfo, sizeof(ISP_WB_INFO_S));
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s_stMEDIACtx.astVcapAEInfo[s32DevIdx].bAESet = HI_TRUE;
        }

        if(bStopIsp)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StopISP(pstPipeAttr->VcapPipeHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
#ifdef CFG_MEDIA_VCAP_STANDBY
        else
        {
            if(1 == s32PipeIdx)
            {
                MLOGI(LIGHT_BLUE"PipeIdx[%d] StopISP!\n"NONE,s32PipeIdx);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_StopISP(pstPipeAttr->VcapPipeHdl);
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
#endif

        for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
        {
            const HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChn = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
            if (!pstPipeChn->bEnable)
            {
                continue;
            }

            if (pstVcapDevAttr->astVcapPipeAttr[s32PipeIdx].astPipeChnAttr[s32PipeChnIdx].stDISAttr.bEnable)
            {
                HI_MAPI_VCAP_DIS_ATTR_S stDisAttr;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_GetChnDISAttr(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl,
                    &stDisAttr);
                HI_PERFORMANCE_TIME_STAMP;

                stDisAttr.stDisAttr.bEnable = HI_FALSE;
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetChnDISAttr(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl,
                    &stDisAttr);
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_StopChn(pstPipeAttr->VcapPipeHdl, pstPipeChn->PipeChnHdl);
            HI_PERFORMANCE_TIME_STAMP;

#ifdef CFG_MEDIA_VCAP_STANDBY
            if(!bStopIsp)
            {
                if((0 == s32PipeIdx) && (0 == s32PipeChnIdx))
                {
                    MLOGI(LIGHT_BLUE"PipeIdx[%d] PipeChnIdx[%d] ENTER_STANDBY!\n"NONE,s32PipeIdx, s32PipeChnIdx);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(s32PipeIdx, s32PipeChnIdx,
                        HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0);
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
#endif

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
    MLOGD(YELLOW" VpssHdl[%d] VcapPipeHdl[%d] PipeChnHdl[%d]"NONE"\n",
        pstMediaVpssAttr->VpssHdl, pstMediaVpssAttr->VcapPipeHdl, pstMediaVpssAttr->VcapPipeChnHdl);
    MLOGD(" MaxW[%u] MaxH[%u] srcFrmRate[%d] dstFrmRate[%d] PixedFmt[%d] bNr[%d]\n",
        pstVpssAttr->u32MaxW, pstVpssAttr->u32MaxH, pstVpssAttr->stFrameRate.s32SrcFrameRate,
        pstVpssAttr->stFrameRate.s32DstFrameRate, pstVpssAttr->enPixelFormat, pstVpssAttr->bNrEn);
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

    memset(&pstVpssPortAttr->stAspectRatio, 0, sizeof(ASPECT_RATIO_S));
    pstVpssPortAttr->stAspectRatio.enMode = pstMediaVpssPortAttr->stAspectRatio.enMode;
    pstVpssPortAttr->stAspectRatio.u32BgColor = pstMediaVpssPortAttr->stAspectRatio.u32BgColor;

    MLOGD("  Resoluton[%ux%u] srcFrmRate[%d] dstFrmRate[%d]\n",
        pstVpssPortAttr->u32Width, pstVpssPortAttr->u32Height,
        pstVpssPortAttr->stFrameRate.s32SrcFrameRate, pstVpssPortAttr->stFrameRate.s32DstFrameRate);
    MLOGD("  VideoFmt[%d] PixelFmt[%d] aspectMode[%d] aspectBgColor[%u]\n",
        pstVpssPortAttr->enVideoFormat, pstVpssPortAttr->enPixelFormat,
        pstVpssPortAttr->stAspectRatio.enMode, pstVpssPortAttr->stAspectRatio.u32BgColor);
    return HI_SUCCESS;
}

static HI_VOID PDT_MEDIA_GetAspectRatio(const SIZE_S* pstDispSize,
    const SIZE_S* pstSrcSize, RECT_S* pstDispVideoRect);

static HI_S32 PDT_MEDIA_SetVport(HI_S32 VpssHdl, HI_U32 u32VpssInputWidth, HI_U32 u32VpssInputHeight,
                    const HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstMediaVpssPortAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MAPI_VPORT_ATTR_S stVportAttr;
    memset(&stVportAttr, 0, sizeof(HI_MAPI_VPORT_ATTR_S));
    MLOGD("VpssHdl[%d] portHdl[%d]\n", VpssHdl, pstMediaVpssPortAttr->VportHdl);
    s32Ret = PDT_MEDIA_SwitchVpssPortAttr(pstMediaVpssPortAttr, &stVportAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if((s_stMEDIACtx.bCompress) && (0 == pstMediaVpssPortAttr->VportHdl))
    {
#if defined(HI3559V200)
        if (u32VpssInputWidth > 3360)
        {
            stVportAttr.enCompressMode = COMPRESS_MODE_NONE;
        }
        else
        {
            stVportAttr.enCompressMode = COMPRESS_MODE_SEG;
        }
#else
        stVportAttr.enCompressMode = COMPRESS_MODE_SEG;
#endif
        MLOGD(LIGHT_BLUE"Vpss VpssHdl[%d] port[%d] enCompressMode[%d]\n"NONE,
            VpssHdl, pstMediaVpssPortAttr->VportHdl, stVportAttr.enCompressMode);
    }
            stVportAttr.stAspectRatio.enMode = 0;
            stVportAttr.stAspectRatio.u32BgColor = 0;
            stVportAttr.stAspectRatio.stVideoRect.s32X = 0;
            stVportAttr.stAspectRatio.stVideoRect.s32Y = 0;
            stVportAttr.stAspectRatio.stVideoRect.u32Width = 0;
            stVportAttr.stAspectRatio.stVideoRect.u32Height = 0;

    if(ASPECT_RATIO_MANUAL == stVportAttr.stAspectRatio.enMode) //设置副型比
    {
        /*Lorin Modity*/
        // MLOGI(LIGHT_BLUE"ini param: stVportAttr.u32Width %u, stVportAttr.u32Height %u.\n"NONE,
        //     stVportAttr.u32Width, stVportAttr.u32Height);

        // HI_HAL_SCREEN_ATTR_S stScreenAttr;
        // memset(&stScreenAttr, 0, sizeof(stScreenAttr));
        // s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
        // HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_GetAttr");

        // SIZE_S stScreenSize;
#if defined(CONFIG_VERTICAL_SCREEN)
        //Lorin Modity
        // stScreenSize.u32Width = stScreenAttr.stAttr.u32Height;
        // stScreenSize.u32Height = stScreenAttr.stAttr.u32Width;
        // MLOGI(LIGHT_BLUE"screen width %u, height %u.\n"NONE,
        //     stScreenSize.u32Width,
        //     stScreenSize.u32Height);
#else
        // stScreenSize.u32Width = stScreenAttr.stAttr.u32Width;
        // stScreenSize.u32Height = stScreenAttr.stAttr.u32Height;
        // MLOGI(LIGHT_BLUE"screen width %u, height %u.\n"NONE,
        //     stScreenSize.u32Width,
        //     stScreenSize.u32Height);
#endif

        // stVportAttr.u32Width = stScreenSize.u32Width;
        // stVportAttr.u32Height = stScreenSize.u32Height;
        // MLOGI(LIGHT_BLUE"reset param: stVportAttr.u32Width %u, stVportAttr.u32Height %u.\n"NONE,
        //     stVportAttr.u32Width, stVportAttr.u32Height);

        // SIZE_S stSrcSize;
        // stSrcSize.u32Width = u32VpssInputWidth;
        // stSrcSize.u32Height = u32VpssInputHeight;
        // MLOGI(LIGHT_BLUE"src width %d, height %d.\n"NONE, stSrcSize.u32Width, stSrcSize.u32Height);

        // PDT_MEDIA_GetAspectRatio(&stScreenSize, &stSrcSize, &stVportAttr.stAspectRatio.stVideoRect);
        // MLOGI(LIGHT_BLUE"x %d, y %d, width %u, height %u.\n"NONE,
        //     stVportAttr.stAspectRatio.stVideoRect.s32X,
        //     stVportAttr.stAspectRatio.stVideoRect.s32Y,
        //     stVportAttr.stAspectRatio.stVideoRect.u32Width,
        //     stVportAttr.stAspectRatio.stVideoRect.u32Height);
    }
    else
    {
       MLOGI(LIGHT_BLUE"mode %d, bgColor %u, x %d, y %d, width %u, height %u.\n"NONE,
            stVportAttr.stAspectRatio.enMode,
            stVportAttr.stAspectRatio.u32BgColor,
            stVportAttr.stAspectRatio.stVideoRect.s32X,
            stVportAttr.stAspectRatio.stVideoRect.s32Y,
            stVportAttr.stAspectRatio.stVideoRect.u32Width,
            stVportAttr.stAspectRatio.stVideoRect.u32Height);
    }

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_SetPortAttr(VpssHdl, pstMediaVpssPortAttr->VportHdl, &stVportAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("  Rotate[%d] Mirror[%d] Flip[%d]\n", pstMediaVpssPortAttr->enRotate,
        pstMediaVpssPortAttr->bMirror, pstMediaVpssPortAttr->bFlip);
    ROTATION_E enRotate = pstMediaVpssPortAttr->enRotate;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_SetPortAttrEx(VpssHdl, pstMediaVpssPortAttr->VportHdl,
        HI_VPROC_CMD_PortRotate, &enRotate, sizeof(ROTATION_E));
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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

    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_StartVport(HI_S32 VpssHdl, HI_S32 VpssPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    if(0 == s_stMEDIACtx.as32VpssVportStartCnt[VpssHdl][VpssPortHdl])
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_StartPort(VpssHdl, VpssPortHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("StartPort[%d,%d] failed[%x]\n", VpssHdl, VpssPortHdl, s32Ret);
            return HI_FAILURE;
        }
    }
    s_stMEDIACtx.as32VpssVportStartCnt[VpssHdl][VpssPortHdl]++;
#else
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_StartPort(VpssHdl, VpssPortHdl);
    HI_PERFORMANCE_TIME_STAMP;
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("StartPort[%d,%d] failed[%x]\n", VpssHdl, VpssPortHdl, s32Ret);
        return HI_FAILURE;
    }
#endif

    return s32Ret;
}

static HI_S32 PDT_MEDIA_StopVport(HI_S32 VpssHdl, HI_S32 VpssPortHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    MLOGI(YELLOW"StopPort[%d,%d] as32VpssVportStartCnt[%d]\n"NONE,
        VpssHdl, VpssPortHdl, s_stMEDIACtx.as32VpssVportStartCnt[VpssHdl][VpssPortHdl]);
    if(1 == s_stMEDIACtx.as32VpssVportStartCnt[VpssHdl][VpssPortHdl])
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VPROC_StopPort(VpssHdl, VpssPortHdl);
        HI_PERFORMANCE_TIME_STAMP;
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("StopPort[%d,%d] failed[%x]\n", VpssHdl, VpssPortHdl, s32Ret);
            return HI_FAILURE;
        }
    }
    s_stMEDIACtx.as32VpssVportStartCnt[VpssHdl][VpssPortHdl]--;
#else
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_StopPort(VpssHdl, VpssPortHdl);
    HI_PERFORMANCE_TIME_STAMP;
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("StopPort[%d,%d] failed[%x]\n", VpssHdl, VpssPortHdl, s32Ret);
        return HI_FAILURE;
    }
#endif

    return s32Ret;
}

static HI_S32 PDT_MEDIA_BindVpss(const HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr,
                    const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfg, HI_U32* pu32VpssInputWidth, HI_U32* pu32VpssInputHeight)
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
                    *pu32VpssInputHeight = pstPipeChnAttr->stDestResolution.u32Height;
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
    HI_U32 u32VpssInputHeight = 1080;
    HI_S32 i;

    /* Vpss Init */
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    s32Ret = PDT_MEDIA_SwitchVpssAttr(pstMediaVpssAttr, &stVpssAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stVpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
    MLOGD(LIGHT_BLUE"Vpss VpssHdl[%d] Nr enCompressMode[%d]\n"NONE,
        pstMediaVpssAttr->VpssHdl, stVpssAttr.stNrAttr.enCompressMode);
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_VPROC_InitVpss(pstMediaVpssAttr->VpssHdl, &stVpssAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /* reset vport start count */
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    HI_S32 s32PortIndex = 0;
    for(s32PortIndex = 0; s32PortIndex < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; s32PortIndex++)
    {
        s_stMEDIACtx.as32VpssVportStartCnt[pstMediaVpssAttr->VpssHdl][s32PortIndex] = 0;
    }
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

    /* Vpss Bind */
    s32Ret = PDT_MEDIA_BindVpss(pstMediaVpssAttr, &pstVideoCfg->stVcapCfg, &u32VpssInputWidth, &u32VpssInputHeight);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* VpssPort Init */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i)
    {
        if (!pstMediaVpssAttr->astVportAttr[i].bEnable)
        {
            continue;
        }
        s32Ret = PDT_MEDIA_SetVport(pstMediaVpssAttr->VpssHdl, u32VpssInputWidth, u32VpssInputHeight,
            &pstMediaVpssAttr->astVportAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

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

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        MUTEX_LOCK(s_stMEDIACtx.CntMutex);
#endif
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = PDT_MEDIA_StopVport(pstMediaVpssAttr->VpssHdl,
                pstMediaVpssAttr->astVportAttr[i].VportHdl);
        HI_PERFORMANCE_TIME_STAMP;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif
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

    /* reset vport start count */
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    MUTEX_LOCK(s_stMEDIACtx.CntMutex);
    HI_S32 s32PortIndex = 0;
    for(s32PortIndex = 0; s32PortIndex < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; s32PortIndex++)
    {
        s_stMEDIACtx.as32VpssVportStartCnt[pstMediaVpssAttr->VpssHdl][s32PortIndex] = 0;
    }
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif

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

static HI_S32 PDT_MEDIA_VoWndStart(const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

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
            s32Ret = HI_MAPI_DISP_Bind_VProc(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl,
                pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl, HI_FALSE);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
            MUTEX_LOCK(s_stMEDIACtx.CntMutex);
#endif
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = PDT_MEDIA_StartVport(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl);
            HI_PERFORMANCE_TIME_STAMP;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
            MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif
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
        else
        {
            MLOGE("Invalid Binded Module[%d]\n", pstDispCfg->astWndCfg[i].enBindedMod);
            return HI_PDT_MEDIA_EINVAL;
        }
        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++j)
        {
            if (ASPECT_RATIO_BUTT != pstDispCfg->astWndCfg[j].stAspectRatio.enMode)
            {
                HI_MAPI_DISP_WINDOW_ASPECTRATIO_S stAspectRatio = {};
                stAspectRatio.stAspectRatio.enMode = pstDispCfg->astWndCfg[j].stAspectRatio.enMode;
                stAspectRatio.stAspectRatio.u32BgColor = pstDispCfg->astWndCfg[j].stAspectRatio.u32BgColor;
                stAspectRatio.stAspectRatio.stVideoRect = pstDispCfg->astWndCfg[j].stAspectRatio.stVideoRect;
                MLOGI("[%d,%d] AspectMode[%d] BgColor[%#x]\n", pstDispCfg->VdispHdl,
                    pstDispCfg->astWndCfg[j].WndHdl,
                    stAspectRatio.stAspectRatio.enMode, stAspectRatio.stAspectRatio.u32BgColor);

                s32Ret = HI_MAPI_DISP_SetWindowAttrEx(pstDispCfg->VdispHdl,
                        pstDispCfg->astWndCfg[j].WndHdl, HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO,
                        (HI_VOID*)&stAspectRatio, sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_DISP_StartWindow(pstDispCfg->VdispHdl, pstDispCfg->astWndCfg[i].WndHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_VoWndStop(const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++i)
    {
        if (!pstDispCfg->astWndCfg[i].bEnable)
        {
            continue;
        }

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        MUTEX_LOCK(s_stMEDIACtx.CntMutex);
#endif
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = PDT_MEDIA_StopVport(pstDispCfg->astWndCfg[i].ModHdl, pstDispCfg->astWndCfg[i].ChnHdl);
        HI_PERFORMANCE_TIME_STAMP;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
#endif
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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
                && HI_FALSE == s_stMEDIACtx.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[0].stDISAttr.bEnable
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

#ifdef CONFIG_MOTIONSENSOR
                /* LDC */
                MLOGD(LIGHT_BLUE"vcap pipe[%d] chn[%d], stLDCAttr.bEnable(%d), stDISAttr.bEnable(%d)\n"NONE,
                    j, k, pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable);
                if((pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable) &&
                   !(pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable &&
                    HI_APPCOMM_CHECK_RANGE(pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate, 30, 60)))
                {
                    MLOGD(LIGHT_BLUE"set HI_MAPI_VCAP_CMD_LDC\n"NONE);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl, HI_MAPI_VCAP_CMD_LDC,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
#else
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
#endif
                /* DIS */
                if (pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable &&
                   (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate > 0) &&
                   (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate <= 60))
                {
                    HI_MAPI_VCAP_DIS_ATTR_S stDisAttr;
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_GetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                        &stDisAttr);
                    HI_PERFORMANCE_TIME_STAMP;
                    if(0 != memcmp(&stDisAttr,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr,
                        sizeof(HI_MAPI_VCAP_DIS_ATTR_S)))
                    {
                        stDisAttr.stDisAttr.bEnable = HI_FALSE;
                        HI_PERFORMANCE_TIME_STAMP;
                        s32Ret = HI_MAPI_VCAP_SetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                            pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                            &stDisAttr);
                        HI_PERFORMANCE_TIME_STAMP;
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                    }

                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stDisAttr.bEnable =
                        HI_TRUE;
                    MLOGD(LIGHT_BLUE"LDCv2 enable(%d)\n"NONE,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stLDCV2Attr.bEnable);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr);
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

static HI_BOOL PDT_MEDIA_CheckSensorAttrChange(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttrOld,
    const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttrNew)
{
    HI_MAPI_SENSOR_ATTR_S stSnsAttrOld;
    memset(&stSnsAttrOld, 0, sizeof(HI_MAPI_SENSOR_ATTR_S));
    stSnsAttrOld.stSize.u32Width  = pstVcapDevAttrOld->stSnsAttr.stResolution.u32Width;
    stSnsAttrOld.stSize.u32Height = pstVcapDevAttrOld->stSnsAttr.stResolution.u32Height;
    stSnsAttrOld.u8SnsMode = pstVcapDevAttrOld->stSnsAttr.u32SnsMode;
    stSnsAttrOld.enWdrMode = pstVcapDevAttrOld->stSnsAttr.enWdrMode;
    MLOGD("Old sensor attr: VcapDevHdl[%d] Size[%ux%u] SnsMode[%u] WdrMode[%d]\n",
        pstVcapDevAttrOld->VcapDevHdl, stSnsAttrOld.stSize.u32Width, stSnsAttrOld.stSize.u32Height,
        stSnsAttrOld.u8SnsMode, stSnsAttrOld.enWdrMode);

    HI_MAPI_SENSOR_ATTR_S stSnsAttrNew;
    memset(&stSnsAttrNew, 0, sizeof(HI_MAPI_SENSOR_ATTR_S));
    stSnsAttrNew.stSize.u32Width  = pstVcapDevAttrNew->stSnsAttr.stResolution.u32Width;
    stSnsAttrNew.stSize.u32Height = pstVcapDevAttrNew->stSnsAttr.stResolution.u32Height;
    stSnsAttrNew.u8SnsMode = pstVcapDevAttrNew->stSnsAttr.u32SnsMode;
    stSnsAttrNew.enWdrMode = pstVcapDevAttrNew->stSnsAttr.enWdrMode;
    MLOGD("New sensor attr: VcapDevHdl[%d] Size[%ux%u] SnsMode[%u] WdrMode[%d]\n",
        pstVcapDevAttrNew->VcapDevHdl, stSnsAttrNew.stSize.u32Width, stSnsAttrNew.stSize.u32Height,
        stSnsAttrNew.u8SnsMode, stSnsAttrNew.enWdrMode);

    HI_BOOL bRet = ((0 != memcmp(&stSnsAttrOld, &stSnsAttrNew, sizeof(HI_MAPI_SENSOR_ATTR_S))) ? HI_TRUE : HI_FALSE);
    return bRet;
}

static HI_S32 PDT_MEDIA_ResetVcap(const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgOld,
                const HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfgNew, HI_BOOL bCompressChange)
{
    if ((0 == memcmp(pstVcapCfgOld, pstVcapCfgNew, sizeof(HI_PDT_MEDIA_VCAP_CFG_S))) &&
        (HI_FALSE == bCompressChange))
    {
        MLOGD("Vcap Attr is not changed\n");
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DevIdx = 0;
    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttrOld = &pstVcapCfgOld->astVcapDevAttr[s32DevIdx];
        const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttrNew = &pstVcapCfgNew->astVcapDevAttr[s32DevIdx];
        HI_BOOL bSensorAttrChange = PDT_MEDIA_CheckSensorAttrChange(pstVcapDevAttrOld, pstVcapDevAttrNew);

        MLOGI(LIGHT_BLUE"bSensorAttrChange[%d]\n"NONE, bSensorAttrChange);
#ifdef CFG_MEDIA_VCAP_STANDBY
        s32Ret = PDT_MEDIA_StopVcapDev(pstVcapDevAttrOld, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#else
        s32Ret = PDT_MEDIA_StopVcapDev(pstVcapDevAttrOld, HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef CFG_MEDIA_VCAP_STANDBY
        if(bSensorAttrChange)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_VCAP_DeinitSensor(pstVcapCfgOld->astVcapDevAttr[s32DevIdx].VcapDevHdl);
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            /** set frequency */

            s32Ret = PDT_MEDIA_InitSensor(&pstVcapCfgNew->astVcapDevAttr[s32DevIdx]);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s32Ret = PDT_MEDIA_StartVcapDev(pstVcapDevAttrNew, HI_FALSE, bSensorAttrChange);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#else
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_VCAP_DeinitSensor(pstVcapCfgOld->astVcapDevAttr[s32DevIdx].VcapDevHdl);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        /** set frequency */

        s32Ret = PDT_MEDIA_InitSensor(&pstVcapCfgNew->astVcapDevAttr[s32DevIdx]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_MEDIA_StartVcapDev(pstVcapDevAttrNew, HI_TRUE, HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    }

    return HI_SUCCESS;
}

static HI_VOID PDT_MEDIA_GenerateOsdInfo(HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo)
{
    HI_S32 i, j, k;
    HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstOsdInfo->astVcapDevInfo[i].bEnable = pstVideoCfg->stVcapCfg.astVcapDevAttr[i].bEnable;
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            pstOsdInfo->astVcapDevInfo[i].astPipeInfo[j].bEnable =
                pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable;
            pstOsdInfo->astVcapDevInfo[i].astPipeInfo[j].VcapPipeHdl =
                pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl;
            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                pstOsdInfo->astVcapDevInfo[i].astPipeInfo[j].astPipeChnInfo[k].bEnable =
                    pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable;
                pstOsdInfo->astVcapDevInfo[i].astPipeInfo[j].astPipeChnInfo[k].PipeChnHdl =
                    pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl;
                pstOsdInfo->astVcapDevInfo[i].astPipeInfo[j].astPipeChnInfo[k].stDestResolution =
                    pstVideoCfg->stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].stDestResolution;
            }
        }
    }

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        pstOsdInfo->astVpssInfo[i].bEnable = pstVideoCfg->stVprocCfg.astVpssAttr[i].bEnable;
        pstOsdInfo->astVpssInfo[i].VpssHdl = pstVideoCfg->stVprocCfg.astVpssAttr[i].VpssHdl;
        pstOsdInfo->astVpssInfo[i].VcapPipeHdl = pstVideoCfg->stVprocCfg.astVpssAttr[i].VcapPipeHdl;
        pstOsdInfo->astVpssInfo[i].VcapPipeChnHdl = pstVideoCfg->stVprocCfg.astVpssAttr[i].VcapPipeChnHdl;
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            pstOsdInfo->astVpssInfo[i].astPortInfo[j].bEnable =
                pstVideoCfg->stVprocCfg.astVpssAttr[i].astVportAttr[j].bEnable;
            pstOsdInfo->astVpssInfo[i].astPortInfo[j].VportHdl =
                pstVideoCfg->stVprocCfg.astVpssAttr[i].astVportAttr[j].VportHdl;
            pstOsdInfo->astVpssInfo[i].astPortInfo[j].stResolution =
                pstVideoCfg->stVprocCfg.astVpssAttr[i].astVportAttr[j].stResolution;
            MLOGD(YELLOW"Vpss %d Port %d Res[%ux%u]"NONE"\n", i, j,
                pstOsdInfo->astVpssInfo[i].astPortInfo[j].stResolution.u32Width,
                pstOsdInfo->astVpssInfo[i].astPortInfo[j].stResolution.u32Height);
        }
    }

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstOsdInfo->astVencInfo[i].bEnable = pstVideoCfg->astVencCfg[i].bEnable;
        pstOsdInfo->astVencInfo[i].VencHdl = pstVideoCfg->astVencCfg[i].VencHdl;
        pstOsdInfo->astVencInfo[i].stResolution.u32Width =
            pstVideoCfg->astVencCfg[i].stVencAttr.stTypeAttr.u32Width;
        pstOsdInfo->astVencInfo[i].stResolution.u32Height =
            pstVideoCfg->astVencCfg[i].stVencAttr.stTypeAttr.u32Height;
        pstOsdInfo->astVencInfo[i].enBindedMod = pstVideoCfg->astVencCfg[i].enBindedMod;
        pstOsdInfo->astVencInfo[i].ModHdl = pstVideoCfg->astVencCfg[i].ModHdl;
        pstOsdInfo->astVencInfo[i].ChnHdl = pstVideoCfg->astVencCfg[i].ChnHdl;
    }
    memcpy(&pstOsdInfo->stOsdCfg, &pstVideoCfg->stOsdCfg, sizeof(HI_PDT_MEDIA_OSD_CFG_S));
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
    HI_BOOL bCompressChange, HI_BOOL bLoadDspChange)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgOld = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg;
    const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfgNew = &pstMediaCfg->stVideoCfg;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S stOsdInfo;

    /* init video if not init before */
    if (!s_stMEDIACtx.bVideoInit)
    {
        MLOGD("Video not init yet, init video\n");
        s32Ret = HI_PDT_MEDIA_VideoInit(&pstMediaCfg->stVideoCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_SUCCESS;
    }

    /* param not change: do nothing */
    if ((0 == memcmp(pstVideoCfgOld, pstVideoCfgNew, sizeof(HI_PDT_MEDIA_VIDEO_CFG_S))) &&
             (HI_FALSE == bCompressChange))
    {
        MLOGD(YELLOW"------------ Video Param Not Change ------------"NONE"\n");
    }
    /* param change */
    else
    {
        PDT_MEDIA_GenerateOsdInfo(&stOsdInfo);
        s32Ret = PDT_MEDIA_OSD_StopTimeOsd(&stOsdInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Stop TimeOsd OK!"NONE"\n");


#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    if(bLoadDspChange)
    {
       if(s_stMEDIACtx.bLoadDsp)
       {
            /** load DSP core binary */
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = PDT_LoadDspCoreBinary();
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
       }
       else
       {
            /** unload DSP core binary */
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = PDT_UnloadDspCoreBinary();
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
       }
    }
#endif
#endif

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

        PDT_MEDIA_GenerateOsdInfo(&stOsdInfo);
        s32Ret = PDT_MEDIA_OSD_StartTimeOsd(&stOsdInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        MLOGI(YELLOW"Start TimeOsd OK!"NONE"\n");

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

static HI_S32 PDT_MEDIA_ResetAudio(const HI_PDT_MEDIA_AUDIO_CFG_S *pstAudiofgOld,
                        const HI_PDT_MEDIA_AUDIO_CFG_S *pstAudiofgNew, HI_BOOL bRebuild)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* init audio if not init before */
    if (!s_stMEDIACtx.bAudioInit)
    {
        MLOGD("Audio not init yet, init audio\n");
        s32Ret = HI_PDT_MEDIA_AudioInit(pstAudiofgNew);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        return HI_SUCCESS;
    }

    /* param not change and neednot force rebuild: do nothing */
    if ( !bRebuild && (0 == memcmp(pstAudiofgOld, pstAudiofgNew, sizeof(HI_PDT_MEDIA_AUDIO_CFG_S))))
    {
        MLOGD(YELLOW"------------ Audio Param Not Change ------------"NONE"\n");
        return HI_SUCCESS;
    }

    /* deinit audio */
    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /** set frequency */

    /** init audio */
    s32Ret = HI_PDT_MEDIA_AudioInit(pstAudiofgNew);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
HI_S32 HI_PDT_MEDIA_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_EVTHUB_Register(HI_EVENT_PDT_MEDIA_HDMI_IN);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_PDT_MEDIA_HDMI_OUT);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}
#endif

HI_PDT_MEDIA_CFG_S* PDT_MEDIA_GetMediaCfg(HI_VOID)
{
    if (!s_stMEDIACtx.bMediaInit)
    {
        MLOGE("not init\n");
        return NULL;
    }
    return &s_stMEDIACtx.pstMediaInfo->stMediaCfg;
}

#if (defined(HI3559V200) && defined(DDR_BITWIDTH_16))
HI_VOID PDT_MEDIA_DDR_BITWIDTH_16(HI_VOID)
{
    /** WRITE */
    himm(0x12030080, 0x00566000); /** --NNIE_L--NNIE_H--GDC--VIPROC--VPSS--AIO--VDP--VICAP-- */
    himm(0x12030084, 0x30000071); /** --SCD--SPACC--FMC--EMMC--SDIO0--EDMA--USB--ETH-- */
    himm(0x12030088, 0x00040546); /** --DDRT--GZIP--TDE--IVE--JPGE--VGS--VEDU--CPU-- */
    himm(0x1203008c, 0x00000003); /** --SDIO1--JPGD-- */
    himm(0x1206408c, 0x90a10804); /** >=a, highest; >=8, 1th;  lower than 4, lowest */
    /** READ */
    himm(0x12030090, 0x00405560); /** --NNIE_L--NNIE_H--GDC--VIPROC--VPSS-AIO--VDP--VICAP-- */
    himm(0x12030094, 0x00507070); /** --SCD--SPACC--FMC--EMMC--SDIO0--EDMA--USB--ETH-- */
    himm(0x12030098, 0x00444447); /** --DDRT--GZIP--TDE--IVE--JPGE--VGS--VEDU--CPU-- */
    himm(0x1203009c, 0x00000003); /** --SDIO1--JPGD-- */
    himm(0x12064300, 0x30);
}
#endif

static HI_S32 PDT_MEDIA_Init(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode, const HI_PDT_MEDIA_VB_CFG_S* pstVBCfg, HI_BOOL bLoadDsp)
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
    if(VB_MAX_COMM_POOLS < stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt)
    {
        return HI_FAILURE;
    }
    for (i = 0; i < stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt; ++i)
    {
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u64BlkSize = pstVBCfg->astCommPool[i].u32BlkSize;
        stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].u32BlkCnt  = pstVBCfg->astCommPool[i].u32BlkCnt;
        if (HI_TRUE == bLoadDsp)
        {
            stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].enRemapMode = VB_REMAP_MODE_CACHED;
        }
        MLOGI(GREEN"VBPool[%d] BlkSize[%u] BlkCnt[%u] RemapMode[%d]\n"NONE, i, pstVBCfg->astCommPool[i].u32BlkSize,
            pstVBCfg->astCommPool[i].u32BlkCnt,
            stMediaAttr.stMediaConfig.stVbConfig.astCommPool[i].enRemapMode);
    }

    stMediaAttr.stVencModPara.u32H264eLowPowerMode = 1;
    stMediaAttr.stVencModPara.u32H265eLowPowerMode = 1;

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_Media_Init(&stMediaAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#if (defined(HI3559V200) && defined(DDR_BITWIDTH_16))
    PDT_MEDIA_DDR_BITWIDTH_16();
#endif

#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    /** load DSP core binary */
    if(HI_TRUE == bLoadDsp)
    {
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = PDT_LoadDspCoreBinary();
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
#endif
#endif

    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stViVpssMode, pstViVpssMode, sizeof(HI_PDT_MEDIA_VI_VPSS_MODE_S));
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVBCfg, pstVBCfg, sizeof(HI_PDT_MEDIA_VB_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Init(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode, const HI_PDT_MEDIA_VB_CFG_S* pstVBCfg, HI_BOOL bLoadDsp)
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
        s_stMEDIACtx.pstMediaInfo = (PDT_MEDIA_INFO_S*)malloc(sizeof(PDT_MEDIA_INFO_S));
        HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stMEDIACtx.pstMediaInfo, HI_FAILURE, "Malloc");
#if defined(AMP_LINUX)
        memset(s_stMEDIACtx.pstMediaInfo, 0, sizeof(PDT_MEDIA_INFO_S));
#else
        PDT_MEDIA_INFO_S* pstMediaInfo =
            (PDT_MEDIA_INFO_S*)HI_MemMap((HI_U32)PDT_MEDIA_INFO_PHY_ADDR, sizeof(PDT_MEDIA_INFO_S));
        HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(pstMediaInfo, HI_FAILURE, "MemMap");
        MLOGI("media info address[%#x][%p] size[%lu]\n", PDT_MEDIA_INFO_PHY_ADDR,
            pstMediaInfo, (HI_UL)sizeof(PDT_MEDIA_INFO_S));

        memcpy(s_stMEDIACtx.pstMediaInfo, pstMediaInfo, sizeof(PDT_MEDIA_INFO_S));

        HI_MemUnmap(pstMediaInfo);
#endif
#else
        s_stMEDIACtx.pstMediaInfo = (PDT_MEDIA_INFO_S*)PDT_MEDIA_INFO_PHY_ADDR;
        memset(s_stMEDIACtx.pstMediaInfo, 0, sizeof(PDT_MEDIA_INFO_S));
#endif

    /* Media Init */
    s32Ret = PDT_MEDIA_Init(pstViVpssMode, pstVBCfg, bLoadDsp);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* Update Media Context */
    s_stMEDIACtx.bMediaInit = HI_TRUE;

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_MUTEX_INIT_LOCK(s_stMEDIACtx.CntMutex);

    /* HDMI Init */
    s32Ret = PDT_MEDIA_HDMI_Init();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_stMEDIACtx.bHDMIStart = HI_FALSE;

    HI_PLAYER_RegAdec();
#endif

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Deinit(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_MUTEX_DESTROY(s_stMEDIACtx.CntMutex);

    HI_PLAYER_UnRegAdec();
    /* HDMI Deinit*/
    s32Ret = PDT_MEDIA_HDMI_Deinit();
#endif

#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    /** unload DSP core binary */
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = PDT_UnloadDspCoreBinary();
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_SUCCESS == s32Ret, "UnloadDspCoreBinary");
#endif
#endif

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret |= HI_MAPI_Media_Deinit();
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_LINUX)
    HI_APPCOMM_SAFE_FREE(s_stMEDIACtx.pstMediaInfo);
#endif
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

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        /* Sensor */
        s32Ret = PDT_MEDIA_InitSensor(&pstVideoCfg->stVcapCfg.astVcapDevAttr[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        /* VcapDev */
        s32Ret = PDT_MEDIA_StartVcapDev(&pstVideoCfg->stVcapCfg.astVcapDevAttr[i], HI_TRUE, HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* Vproc */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        s32Ret = PDT_MEDIA_InitVpss(pstVideoCfg, i);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* Venc */
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        s32Ret = PDT_MEDIA_InitVenc(pstVideoCfg, &pstVideoCfg->astVencCfg[i], i);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* OSD */
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S stOsdInfo;
    PDT_MEDIA_GenerateOsdInfo(&stOsdInfo);
    s32Ret = PDT_MEDIA_OSD_StartTimeOsd(&stOsdInfo);
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

    HI_PDT_MEDIA_OSD_VIDEO_INFO_S stOsdInfo;
    PDT_MEDIA_GenerateOsdInfo(&stOsdInfo);
    (HI_VOID)PDT_MEDIA_OSD_StopTimeOsd(&stOsdInfo);

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
        PDT_MEDIA_StopVcapDev(
            &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i],
            bStopIsp);
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
        pstDispCfg = &pstVideoOutCfg->astDispCfg[i];
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

HI_S32 HI_PDT_MEDIA_VideoOutStart(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

    const HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    static HI_BOOL bFirst = HI_TRUE;
    if (bFirst)
    {
        bFirst = HI_FALSE;
        MLOGD("disp start[%d]\n", s_stMEDIACtx.pstMediaInfo->bDispStart);
        if (s_stMEDIACtx.pstMediaInfo->bDispStart)
        {
            MLOGI("fastboot ignore first\n");

            /** refresh as32VpssVportStartCnt*/
            for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
            {
                pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
                if (!pstDispCfg->bEnable)
                {
                    continue;
                }

                for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++j)
                {
                    if (!pstDispCfg->astWndCfg[j].bEnable)
                    {
                        continue;
                    }

                    if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDispCfg->astWndCfg[j].enBindedMod)
                    {
                        MUTEX_LOCK(s_stMEDIACtx.CntMutex);
                        s_stMEDIACtx.as32VpssVportStartCnt[pstDispCfg->astWndCfg[j].ModHdl][pstDispCfg->astWndCfg[j].ChnHdl]++;
                        MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);
                    }
                }
            }
            return HI_SUCCESS;
        }
    }
#endif

    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstDispCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[i];
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
        s32Ret = HI_MAPI_DISP_Start(pstDispCfg->VdispHdl,
            (HI_MAPI_DISP_VIDEOLAYER_ATTR_S *)&pstDispCfg->stVideoLayerAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        s32Ret = PDT_MEDIA_VoWndStart(pstDispCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);



#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        if (VO_INTF_HDMI == pstDispCfg->stDispAttr.stPubAttr.enIntfType)
        {
            s32Ret = PDT_MEDIA_HDMI_Start();
            s_stMEDIACtx.bHDMIStart = ((HI_SUCCESS == s32Ret) ? HI_TRUE : HI_FALSE);
        }
#endif
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
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    MLOGI("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_VideoOutStop(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoOutInit);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    static HI_BOOL bFirst = HI_TRUE;
    if (bFirst)
    {
        MLOGI("ignore first\n");
        bFirst = HI_FALSE;
        return HI_SUCCESS;
    }
#endif

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
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        if (s_stMEDIACtx.bHDMIStart)
        {
            s32Ret = PDT_MEDIA_HDMI_Stop();
            s_stMEDIACtx.bHDMIStart = ((HI_SUCCESS == s32Ret) ? HI_FALSE : HI_TRUE);
        }
#endif
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
        return HI_PDT_MEDIA_EINITIALIZED;
    }
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg, pstAudioCfg, sizeof(HI_PDT_MEDIA_AUDIO_CFG_S));

    const HI_PDT_MEDIA_ACAP_CFG_S* pstAcapCfg = NULL;
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32AcapIdx, s32AcapChnlIdx, s32AencIdx;

    for (s32AcapIdx = 0; s32AcapIdx < HI_PDT_MEDIA_ACAP_MAX_CNT; ++s32AcapIdx)
    {
        pstAcapCfg = &pstAudioCfg->astAcapCfg[s32AcapIdx];
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

        for (s32AcapChnlIdx = 0; s32AcapChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32AcapChnlIdx)
        {
            if(HI_TRUE == pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].bEnable)
            {
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_ACAP_Start(pstAcapCfg->AcapHdl,
                    pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].AcapChnlHdl);
#if defined(HI3559V200)
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
                if(HI_TRUE == pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].bEnableVqe)
                {
                    s32Ret |= HI_MAPI_ACAP_EnableVqe(pstAcapCfg->AcapHdl,
                        pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].AcapChnlHdl);
                }
#endif
#endif
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }

        MLOGD("  AudioGain[%d]\n", pstAcapCfg->s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_ACAP_SetVolume(pstAcapCfg->AcapHdl, pstAcapCfg->s32AudioGain);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        for (s32AencIdx = 0; s32AencIdx < HI_PDT_MEDIA_AENC_MAX_CNT; ++s32AencIdx)
        {
            pstAencCfg = &pstAudioCfg->astAencCfg[s32AencIdx];
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioInit);

    const HI_PDT_MEDIA_ACAP_CFG_S* pstAcapCfg = NULL;
    const HI_PDT_MEDIA_AENC_CFG_S* pstAencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32AcapIdx, s32AcapChnlIdx, s32AencIdx;

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

        for (s32AcapChnlIdx = 0; s32AcapChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32AcapChnlIdx)
        {
            if(HI_TRUE == pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].bEnable)
            {
                HI_PERFORMANCE_TIME_STAMP;
#if defined(HI3559V200)
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
                if(HI_TRUE == pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].bEnableVqe)
                {
                    s32Ret |= HI_MAPI_ACAP_DisableVqe(pstAcapCfg->AcapHdl,
                        pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].AcapChnlHdl);
                }
#endif
#endif
                s32Ret = HI_MAPI_ACAP_Stop(pstAcapCfg->AcapHdl,
                    pstAcapCfg->AcapChnlCfg[s32AcapChnlIdx].AcapChnlHdl);
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

    if (s_stMEDIACtx.pstMediaInfo->bAudioOutStart)
    {
        MLOGD("already Start.\n");
        return HI_SUCCESS;
    }
    memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg, pstAudioOutCfg, sizeof(HI_PDT_MEDIA_AUDIOOUT_CFG_S));

    const HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = NULL;
    HI_S32 s32AudioGain;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        pstAoCfg = &pstAudioOutCfg->astAoCfg[i];
        if (!pstAoCfg->bEnable)
        {
            continue;
        }

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_AO_Init(pstAoCfg->AoHdl, &pstAoCfg->stAoAttr);
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

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
    s_stMEDIACtx.pstMediaInfo->bAudioOutStart = HI_TRUE;
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_AudioOutStop(HI_VOID)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.pstMediaInfo->bAudioOutStart);

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
    s_stMEDIACtx.pstMediaInfo->bAudioOutStart = HI_FALSE;

    MLOGD("Success\n");
    return HI_SUCCESS;
}

#if defined(CFG_LINUX_MMZ_HDMI_RELOAD) && (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))

static HI_VOID PDT_MEDIA_LoadSysDrv(HI_BOOL bMediaHDMI)
{
    MLOGI(YELLOW"load sys ko"NONE"\n");
    HI_S32 s32Ret;
    HI_U32 u32Fb0Size_KB;
    HI_CHAR szCmdParam[256] = {0,};
    if (bMediaHDMI)
    {
        u32Fb0Size_KB = (PDT_MEDIA_HDMI_MAX_WIDTH * PDT_MEDIA_HDMI_MAX_HEIGHT * 2 + 1023) / 1024;

        snprintf(szCmdParam, sizeof(szCmdParam), "anony=1 mmz_allocator=hisi "\
            "mmz=anonymous,0,0x%x,%dK:higo_mmz,1,0x%x,%dK map_mmz=0x%x,%dK",
            CFG_LINUX_MMZ_HDMI_START,
            CFG_LINUX_MMZ_HDMI_ANONYMOUS_LEN,
            CFG_LINUX_MMZ_HDMI_START + (CFG_LINUX_MMZ_HDMI_ANONYMOUS_LEN << 10),
            CFG_LINUX_MMZ_HDMI_HIGO_LEN,
            CFG_LITEOS_MMZ_START,
            (CFG_LINUX_MMZ_HDMI_START - CFG_LITEOS_MMZ_START) >> 10);
        MLOGI(GREEN"HDMI fbsize[%uKB] osalparam[%s]"NONE"\n", u32Fb0Size_KB, szCmdParam);
    }
    else
    {
        HI_HAL_SCREEN_ATTR_S stScreenAttr;
        memset(&stScreenAttr, 0, sizeof(stScreenAttr));
        s32Ret = HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_0, &stScreenAttr);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_HAL_SCREEN_GetAttr fail\n");
            return;
        }
        u32Fb0Size_KB = (stScreenAttr.stAttr.u32Width * stScreenAttr.stAttr.u32Height * 2 * 2 + 1023) / 1024;

        snprintf(szCmdParam, sizeof(szCmdParam), "anony=1 mmz_allocator=hisi "\
            "mmz=anonymous,0,0x%x,%dK:higo_mmz,1,0x%x,%dK map_mmz=0x%x,%dK",
            CFG_LINUX_MMZ_START, CFG_LINUX_MMZ_ANONYMOUS_LEN,
            CFG_LINUX_MMZ_START + (CFG_LINUX_MMZ_ANONYMOUS_LEN << 10), CFG_LINUX_MMZ_HIGO_LEN,
            CFG_LITEOS_MMZ_START, CFG_LITEOS_MMZ_LEN);
        MLOGI(GREEN"normal fbsize[%uKB] osalparam[%s]"NONE"\n", u32Fb0Size_KB, szCmdParam);
    }

    HI_insmod(HI_APPFS_KOMOD_PATH"/hi_osal.ko", szCmdParam);
    HI_insmod(HI_APPFS_KOMOD_PATH"/hi3559v200_base.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/hi3559v200_sys.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/hi3559v200_tde.ko", NULL);
    snprintf(szCmdParam, sizeof(szCmdParam), "video=\"hifb:vram0_size:%d\"", u32Fb0Size_KB);
    HI_insmod(HI_APPFS_KOMOD_PATH"/hifb.ko", szCmdParam);
    MLOGI(YELLOW"load sys ko finish"NONE"\n");
}

static HI_VOID PDT_MEDIA_UnloadSysDrv(HI_VOID)
{
    MLOGI(YELLOW"unload sys ko"NONE"\n");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/hifb.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/hi3559v200_tde.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/hi3559v200_sys.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/hi3559v200_base.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/hi_osal.ko");
}

static HI_BOOL PDT_MEDIA_IsHDMIRoute(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg)
{
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        if (!pstVideoOutCfg->astDispCfg[i].bEnable)
        {
            continue;
        }
        if (VO_INTF_HDMI == pstVideoOutCfg->astDispCfg[i].stDispAttr.stPubAttr.enIntfType)
        {
            MLOGI("HDMI media route\n");
            return HI_TRUE;
        }
    }
    return HI_FALSE;
}

#endif

static HI_S32 PDT_MEDIA_Rebuild(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bLoadDsp)
{
    HI_S32 s32Ret = HI_SUCCESS;

    const HI_MAPI_DISP_ATTR_S* pstOldVoAttr =
        &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg.astDispCfg[0].stDispAttr);
    const HI_MAPI_DISP_ATTR_S* pstNewVoAttr =
        &(pstMediaCfg->stVideoOutCfg.astDispCfg[0].stDispAttr);
    if ((s_stMEDIACtx.bVideoOutInit &&
        (0 != memcmp(pstOldVoAttr, pstNewVoAttr, sizeof(HI_MAPI_DISP_ATTR_S)))))
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
    if (s_stMEDIACtx.pstMediaInfo->bAudioOutStart)
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
#ifdef CFG_POST_PROCESS
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
        /** unload DSP core binary */
        if(HI_TRUE == bLoadDsp)
        {
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = PDT_UnloadDspCoreBinary();
            HI_PERFORMANCE_TIME_STAMP;
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
#endif
#endif

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        HI_PLAYER_UnRegAdec();
#endif

        HI_PERFORMANCE_TIME_STAMP;
        s32Ret = HI_MAPI_Media_Deinit();
        HI_PERFORMANCE_TIME_STAMP;
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        MLOGI(YELLOW"deinit media\n"NONE);
    }

#if defined(CFG_LINUX_MMZ_HDMI_RELOAD) && (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__))
    HI_BOOL bOldMediaHDMI = PDT_MEDIA_IsHDMIRoute(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg);
    HI_BOOL bNewMediaHDMI = PDT_MEDIA_IsHDMIRoute(&pstMediaCfg->stVideoOutCfg);
    if (bOldMediaHDMI != bNewMediaHDMI)
    {
        HI_MPI_SYS_CloseFd();
        PDT_MEDIA_UnloadSysDrv();
        PDT_MEDIA_LoadSysDrv(bNewMediaHDMI);
    }
#endif
    /** set media frequency */

#ifdef CONFIG_MOTIONSENSOR
    s32Ret = HI_PDT_MEDIA_DeinitMotionsensor();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"Deinit Motionsensor\n"NONE);
#endif

    /** init media */
    s32Ret = PDT_MEDIA_Init(&pstMediaCfg->stViVpssMode, &pstMediaCfg->stVBCfg, bLoadDsp);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"init media\n"NONE);

#ifdef CONFIG_MOTIONSENSOR
    s32Ret = HI_PDT_MEDIA_InitMotionsensor();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"Init Motionsensor\n"NONE);
#endif

    /** init video */
    s32Ret = HI_PDT_MEDIA_VideoInit(&pstMediaCfg->stVideoCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"init video\n"NONE);

    /** init audio */
    s32Ret = HI_PDT_MEDIA_AudioInit(&(pstMediaCfg->stAudioCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"init audio\n"NONE);

#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
        HI_PLAYER_RegAdec();
#endif

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

static HI_S32 PDT_MEDIA_Reset(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bCompressChage, HI_BOOL bLoadDspChange)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit video/audio out */

    HI_BOOL bAudioOutInit = HI_FALSE;
    HI_BOOL bVideoOutInit = HI_FALSE;

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
        bVideoOutInit = HI_TRUE;
    }
    else
    {
        memcpy(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoOutCfg, &pstMediaCfg->stVideoOutCfg,
            sizeof(HI_PDT_MEDIA_VIDEOOUT_CFG_S));
        MLOGI(YELLOW"update video out config\n"NONE);
    }
    /** deinit audio out */
    HI_PDT_MEDIA_AUDIOOUT_CFG_S* pOldstAudioOutCfg =
        &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioOutCfg);
    if ((s_stMEDIACtx.pstMediaInfo->bAudioOutStart) &&
        (0 != memcmp(&pstMediaCfg->stAudioOutCfg, pOldstAudioOutCfg, sizeof(HI_PDT_MEDIA_AUDIOOUT_CFG_S))))
    {
        MLOGI(YELLOW"deinit audio out\n"NONE);
        s32Ret = HI_PDT_MEDIA_AudioOutStop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        bAudioOutInit = HI_TRUE;
    }

    /** reset video pipe */
    s32Ret = PDT_MEDIA_ResetVideo(pstMediaCfg, bCompressChage, bLoadDspChange);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    MLOGI(YELLOW"reset video\n"NONE);

    /** reset audio */
    HI_PDT_MEDIA_AUDIO_CFG_S *pstAudiofgOld = &(s_stMEDIACtx.pstMediaInfo->stMediaCfg.stAudioCfg);
    HI_PDT_MEDIA_AUDIO_CFG_S *pstAudiofgNew = (HI_PDT_MEDIA_AUDIO_CFG_S *)&(pstMediaCfg->stAudioCfg);
    s32Ret = PDT_MEDIA_ResetAudio(pstAudiofgOld, pstAudiofgNew, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGI(YELLOW"reset audio\n"NONE);

    /** init audio out */
    if(bAudioOutInit)
    {
        s32Ret = HI_PDT_MEDIA_AudioOutStart(&pstMediaCfg->stAudioOutCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        MLOGI(YELLOW"init audio out\n"NONE);
    }

    /* init video out */
    if (bVideoOutInit)
    {
        s32Ret = HI_PDT_MEDIA_VideoOutInit(&pstMediaCfg->stVideoOutCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        MLOGI(YELLOW"init video out\n"NONE);
#ifdef CONFIG_SCREEN
        HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_HAL_SCREEN_SetDisplayState");
#endif
        MLOGI(YELLOW"set display state on\n"NONE);
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_Reset(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bCompress,
    HI_BOOL bLoadDsp, HI_BOOL bForceRebuild)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    HI_S32 s32Ret = HI_SUCCESS;
    MLOGI(LIGHT_BLUE"bCompress(%d), s_stMEDIACtx.bCompress(%d), bLoadDsp(%d), s_stMEDIACtx.bLoadDsp(%d)"NONE"\n",
        bCompress, s_stMEDIACtx.bCompress, bLoadDsp, s_stMEDIACtx.bLoadDsp);
    HI_BOOL bCompressChange = HI_FALSE;
    if(s_stMEDIACtx.bCompress != bCompress)
    {
        bCompressChange = HI_TRUE;
        s_stMEDIACtx.bCompress = bCompress;
    }

    HI_BOOL bLoadDspChange = HI_FALSE;
    if(s_stMEDIACtx.bLoadDsp != bLoadDsp)
    {
        bLoadDspChange = HI_TRUE;
        s_stMEDIACtx.bLoadDsp = bLoadDsp;
    }

    /** check rebuild all */
    if ((0 != memcmp(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVBCfg, &pstMediaCfg->stVBCfg, sizeof(HI_PDT_MEDIA_VB_CFG_S))) ||
        (0 != memcmp(&s_stMEDIACtx.pstMediaInfo->stMediaCfg.stViVpssMode, &pstMediaCfg->stViVpssMode, sizeof(HI_PDT_MEDIA_VI_VPSS_MODE_S))) ||
        (HI_TRUE == bForceRebuild))
    {
        MLOGI(YELLOW"rebuild all"NONE"\n");
        s32Ret = PDT_MEDIA_Rebuild(pstMediaCfg, bLoadDsp);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return s32Ret;
    }
    else
    {
        MLOGI(YELLOW"reset all, bCompressChange(%d) bLoadDspChange(%d)"NONE"\n", bCompressChange, bLoadDspChange);
        s32Ret = PDT_MEDIA_Reset(pstMediaCfg, bCompressChange, bLoadDspChange);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioInit);

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
        s32Ret = HI_MAPI_AENC_BindACap(pstAencCfg->AcapHdl, pstAencCfg->AcapChnHdl, AencHdl);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioInit);

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
        s32Ret = HI_MAPI_AENC_UnbindACap(pstAencCfg->AcapHdl, pstAencCfg->AcapChnHdl, AencHdl);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
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
                s32Ret = PDT_MEDIA_StartVport(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
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

        s_stMEDIACtx.as32VencStartCnt[i]++; /* update start count */
    }
    /* fixed framecount venc mode */
    else
    {
        MLOGD("framecnt %d\n", s32FrameCnt);
        if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            s32Ret = PDT_MEDIA_StartVport(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* find matched vencIdx */
    s32Ret = PDT_MEDIA_GetVencIdx(VencHdl, &i);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    pstVencCfg = &s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.astVencCfg[i];

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

        if(HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            s32Ret = PDT_MEDIA_StopVport(pstVencCfg->ModHdl, pstVencCfg->ChnHdl);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("StopPort[%d,%d] failed[%x]\n", pstVencCfg->ModHdl, pstVencCfg->ChnHdl, s32Ret);
            }
        }
    }

    s_stMEDIACtx.as32VencStartCnt[i]--; /* update start count */
    MLOGD("vencHdl[%d] startCnt[%d]\n", VencHdl, s_stMEDIACtx.as32VencStartCnt[i]);
    MUTEX_UNLOCK(s_stMEDIACtx.CntMutex);

    return HI_SUCCESS;
}
#endif

HI_S32 HI_PDT_MEDIA_GetVideoInfo(HI_HANDLE VencHdl, HI_MEDIA_VIDEOINFO_S* pstVideoInfo)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bAudioInit);
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

HI_S32 HI_PDT_MEDIA_SetTimeOSD(HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    if (s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.stTimeOsd.bShow == bEnable)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S stOsdInfo;
    PDT_MEDIA_GenerateOsdInfo(&stOsdInfo);
    if (bEnable)
    {
        stOsdInfo.stOsdCfg.stTimeOsd.bShow = HI_TRUE;
        s32Ret = PDT_MEDIA_OSD_StartTimeOsd(&stOsdInfo);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("Show OSD Error\n");
            return HI_FAILURE;
        }
        s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.stTimeOsd.bShow = HI_TRUE;
    }
    else
    {
        stOsdInfo.stOsdCfg.stTimeOsd.bShow = HI_FALSE;
        s32Ret = PDT_MEDIA_OSD_StopTimeOsd(&stOsdInfo);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("Stop OSD Error\n");
            return HI_FAILURE;
        }
        s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.stTimeOsd.bShow = HI_FALSE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetTimeOSD(HI_BOOL* pbEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    HI_APPCOMM_CHECK_POINTER(pbEnable, HI_PDT_MEDIA_EINVAL);
    *pbEnable = s_stMEDIACtx.pstMediaInfo->stMediaCfg.stVideoCfg.stOsdCfg.stTimeOsd.bShow;
    return HI_SUCCESS;
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
#ifdef CONFIG_MOTIONSENSOR
                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stLDCV2Attr.bEnable = bEnable;
#endif
                MLOGD(LIGHT_BLUE"s_stMEDIACtx.bCompress[%d]\n"NONE, s_stMEDIACtx.bCompress);

#ifdef CONFIG_MOTIONSENSOR
                if(!(pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable &&
                    (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate > 0) &&
                    (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate <= 60)))
                {
                    MLOGD("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,
                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_LDC,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
                else
                {
                    MLOGD("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                    HI_PERFORMANCE_TIME_STAMP;
                    s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_LDCV2,
                        &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr, sizeof(HI_MAPI_VCAP_DIS_ATTR_S));
                    HI_PERFORMANCE_TIME_STAMP;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
#else
                MLOGD("pipe[%d] pipechn[%d] enable[%d]\n", VcapPipeHdl, VcapPipeChnHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl, HI_MAPI_VCAP_CMD_LDC,
                    &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                HI_PERFORMANCE_TIME_STAMP;
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
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

HI_S32 HI_PDT_MEDIA_SetDIS(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL bEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);

    HI_S32 s32Ret = HI_SUCCESS;

    /* Check Enable Change or not */
    HI_BOOL bPrevEnable;
    s32Ret = HI_PDT_MEDIA_GetDIS(VcapPipeHdl, VcapPipeChnHdl, &bPrevEnable);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD(LIGHT_BLUE"pipe[%d] pipechn[%d] bPrevEnable[%d]\n"NONE,
        VcapPipeHdl, VcapPipeChnHdl, bPrevEnable);
    if (bPrevEnable == bEnable)
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

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (!pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].bEnable
                    || VcapPipeChnHdl != pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    continue;
                }

                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable = bEnable;

                MLOGD(LIGHT_BLUE"pipe[%d] pipechn[%d] enable[%d]\n"NONE, VcapPipeHdl, VcapPipeChnHdl,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable);
                MLOGD(LIGHT_BLUE"MotinLevel[%d] MotionType[%d]\n"NONE,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stDISConfig.enMotionLevel,
                    pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stDISConfig.enMode);

                if (HI_MAPI_PIPE_TYPE_VIDEO == pstDevAttr->astVcapPipeAttr[j].enPipeType)
                {
                    if (bEnable && (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate > 0) &&
                        (pstDevAttr->astVcapPipeAttr[j].stFrameRate.s32DstFrameRate <= 60))
                    {
                        MLOGD(LIGHT_BLUE"s_stMEDIACtx.bCompress[%d]\n"NONE,
                            s_stMEDIACtx.bCompress);
                        /** shutdown LDC before enable DIS */

#ifdef CONFIG_MOTIONSENSOR
                        if(bEnable && (0 == VcapPipeChnHdl) && pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable)
                        {
                            MLOGD(LIGHT_BLUE"pipe[%d] pipechn[%d] enable[%d], shut down LDC\n"NONE, VcapPipeHdl, VcapPipeChnHdl,
                                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                            VI_LDC_ATTR_S stLDCAttr;
                            memcpy(&stLDCAttr, &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                            stLDCAttr.bEnable = HI_FALSE;
                            HI_PERFORMANCE_TIME_STAMP;
                            s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl,
                                HI_MAPI_VCAP_CMD_LDC, &stLDCAttr, sizeof(VI_LDC_ATTR_S));
                            HI_PERFORMANCE_TIME_STAMP;
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                        }
#endif

                        HI_MAPI_VCAP_DIS_ATTR_S stDisAttr;
                        HI_PERFORMANCE_TIME_STAMP;
                        s32Ret = HI_MAPI_VCAP_GetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                            pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                            &stDisAttr);
                        HI_PERFORMANCE_TIME_STAMP;

                        if(0 != memcmp(&stDisAttr,
                            &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr,
                            sizeof(HI_MAPI_VCAP_DIS_ATTR_S)))
                        {
                            stDisAttr.stDisAttr.bEnable = HI_FALSE;
                            HI_PERFORMANCE_TIME_STAMP;
                            s32Ret = HI_MAPI_VCAP_SetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                                &stDisAttr);
                            HI_PERFORMANCE_TIME_STAMP;
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                        }

                        pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr.stDisAttr.bEnable =
                            HI_TRUE;
                        HI_PERFORMANCE_TIME_STAMP;
                        s32Ret = HI_MAPI_VCAP_SetChnDISAttr(pstDevAttr->astVcapPipeAttr[j].VcapPipeHdl,
                            pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl,
                            &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.stAttr);
                        HI_PERFORMANCE_TIME_STAMP;
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                    }

                    if(!bEnable)
                    {
                        HI_MAPI_VCAP_DIS_ATTR_S stDisAttr;
                        HI_PERFORMANCE_TIME_STAMP;
                        s32Ret = HI_MAPI_VCAP_GetChnDISAttr(VcapPipeHdl, VcapPipeChnHdl, &stDisAttr);
                        HI_PERFORMANCE_TIME_STAMP;

                        stDisAttr.stDisAttr.bEnable = HI_FALSE;
#ifdef CONFIG_MOTIONSENSOR
                        stDisAttr.stLDCV2Attr.bEnable = HI_FALSE;
#endif
                        HI_PERFORMANCE_TIME_STAMP;
                        s32Ret = HI_MAPI_VCAP_SetChnDISAttr(VcapPipeHdl, VcapPipeChnHdl, &stDisAttr);
                        HI_PERFORMANCE_TIME_STAMP;
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#ifdef CONFIG_MOTIONSENSOR
                        if(pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable)
                        {
                            MLOGD(LIGHT_BLUE"pipe[%d] pipechn[%d] enable[%d], turn on LDC\n"NONE, VcapPipeHdl, VcapPipeChnHdl,
                                pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr.bEnable);
                            VI_LDC_ATTR_S stLDCAttr;
                            memcpy(&stLDCAttr, &pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stLDCAttr, sizeof(VI_LDC_ATTR_S));
                            HI_PERFORMANCE_TIME_STAMP;
                            s32Ret = HI_MAPI_VCAP_SetAttrEx(VcapPipeHdl, VcapPipeChnHdl,
                                HI_MAPI_VCAP_CMD_LDC, &stLDCAttr, sizeof(VI_LDC_ATTR_S));
                            HI_PERFORMANCE_TIME_STAMP;
                            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                        }
#endif
                    }
                }

                return HI_SUCCESS;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetDIS(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL* pbEnable)
{
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bVideoInit);
    HI_APPCOMM_CHECK_POINTER(pbEnable, HI_PDT_MEDIA_EINVAL);

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
                    && HI_MAPI_PIPE_TYPE_VIDEO == pstDevAttr->astVcapPipeAttr[j].enPipeType
                    && VcapPipeChnHdl == pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl)
                {
                    *pbEnable = pstDevAttr->astVcapPipeAttr[j].astPipeChnAttr[k].stDISAttr.bEnable;
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.pstMediaInfo->bAudioOutStart);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.pstMediaInfo->bAudioOutStart);
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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.pstMediaInfo->bAudioOutStart);

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
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.pstMediaInfo->bAudioOutStart);

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

HI_S32 HI_PDT_MEDIA_InitOSD(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                            const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoAttr, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstOsdCfg, HI_PDT_MEDIA_EINVAL);
    PDT_MEDIA_CHECK_INIT(s_stMEDIACtx.bMediaInit);

    return PDT_MEDIA_OSD_Init(pstVideoAttr, pstOsdCfg);
}

HI_S32 HI_PDT_MEDIA_DeinitOSD(HI_VOID)
{
    return PDT_MEDIA_OSD_Deinit();
}

HI_S32 HI_PDT_MEDIA_GenerateHDMIMediaCfg(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_MEDIA_EINVAL);
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_MEDIA_HDMI_UpdateMediaCfg(pstMediaCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetHDMIState(HI_PDT_MEDIA_HDMI_STATE_E* penState)
{
    HI_APPCOMM_CHECK_POINTER(penState, HI_PDT_MEDIA_EINVAL);
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_MEDIA_HDMI_GetState(penState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 HI_PDT_MEDIA_GetHDMIDispSize(SIZE_S* pstDispSize)
{
    HI_APPCOMM_CHECK_POINTER(pstDispSize, HI_PDT_MEDIA_EINVAL);
#if (defined(AMP_LINUX_HUAWEILITE) && !defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_MEDIA_HDMI_GetDispSize(pstDispSize);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return HI_SUCCESS;
}

static HI_VOID PDT_MEDIA_GetAspectRatio(const SIZE_S* pstDispSize,
    const SIZE_S* pstSrcSize, RECT_S* pstDispVideoRect)
{
    /** Wsrc/Hsrc < Wdisp/Hdisp : Left/Right with Backgroud Edge */
    if (pstSrcSize->u32Width * pstDispSize->u32Height < pstSrcSize->u32Height * pstDispSize->u32Width
        && 0!=pstSrcSize->u32Height)
    {
        pstDispVideoRect->u32Width  = pstSrcSize->u32Width * pstDispSize->u32Height / pstSrcSize->u32Height;

        pstDispVideoRect->s32X =
            HI_APPCOMM_ALIGN(pstDispSize->u32Width - pstDispVideoRect->u32Width, 4) >> 1;
        pstDispVideoRect->s32Y = 0;
        pstDispVideoRect->u32Width = pstDispSize->u32Width - (pstDispVideoRect->s32X << 1);
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
        pstDispVideoRect->u32Height= pstDispSize->u32Height - (pstDispVideoRect->s32Y << 1);
    }
    else
    {
        pstDispVideoRect->s32X = 0;
        pstDispVideoRect->s32Y = 0;
        pstDispVideoRect->u32Width  = pstDispSize->u32Width;
        pstDispVideoRect->u32Height = pstDispSize->u32Height;
    }
}

static HI_S32 PDT_Media_UpdateAspecRatio(const HI_PDT_MEDIA_CFG_S* pstMediaCfg,
    const HI_HAL_SCREEN_ATTR_S* pstScreenAttr, HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_S32 s32DispIndex = 0;
    for(s32DispIndex = 0; s32DispIndex < HI_PDT_MEDIA_DISP_MAX_CNT; ++s32DispIndex)
    {
        if(HI_FALSE == pstDispCfg->bEnable)
        {
            continue;
        }

        HI_S32 s32WndIdx = 0;
        for (s32WndIdx = 0; s32WndIdx < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++s32WndIdx)
        {
            if(HI_FALSE == pstDispCfg->astWndCfg[s32WndIdx].bEnable)
            {
                continue;
            }

            if (ASPECT_RATIO_MANUAL == pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.enMode)
            {
                HI_U32 u32SrcWidth = 0;
                HI_U32 u32SrcHeight = 0;

                /** get source size */
                if(HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDispCfg->astWndCfg[s32WndIdx].enBindedMod)
                {
                    HI_S32 i = 0;
                    const HI_PDT_MEDIA_VPSS_ATTR_S *pstVpssAttr = NULL;
                    for(i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
                    {
                        pstVpssAttr = &pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i];
                        if((HI_TRUE != pstVpssAttr->bEnable) ||
                           (pstVpssAttr->VpssHdl != pstDispCfg->astWndCfg[s32WndIdx].ModHdl))
                        {
                            continue;
                        }

                        HI_S32 j = 0;
                        for(j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
                        {
                            if((HI_TRUE != pstVpssAttr->astVportAttr[j].bEnable) ||
                               (pstVpssAttr->astVportAttr[j].VportHdl != pstDispCfg->astWndCfg[s32WndIdx].ChnHdl))
                            {
                               continue;
                            }

                            u32SrcWidth = pstVpssAttr->astVportAttr[j].stResolution.u32Width;
                            u32SrcHeight = pstVpssAttr->astVportAttr[j].stResolution.u32Height;
                            MLOGI(YELLOW"SrcWidth(%d) SrcHeight(%d)\n"NONE, u32SrcWidth, u32SrcHeight);
                        }
                    }

                    if((0 == u32SrcWidth) || (0 == u32SrcHeight))
                    {
                        MLOGE(RED"source size err.\n"NONE);
                        return HI_FAILURE;
                    }
                }
                else
                {
                    MLOGI(RED"band mode(%d) not support.\n"NONE, pstDispCfg->astWndCfg[s32WndIdx].enBindedMod);
                    return HI_FAILURE;
                }

                SIZE_S stSrcSize;
                if((ROTATION_90 == pstDispCfg->astWndCfg[s32WndIdx].enRotate) ||
                   (ROTATION_270 == pstDispCfg->astWndCfg[s32WndIdx].enRotate))
                {
                    stSrcSize.u32Width = u32SrcHeight;
                    stSrcSize.u32Height = u32SrcWidth;
                }
                else
                {
                    stSrcSize.u32Width = u32SrcWidth;
                    stSrcSize.u32Height = u32SrcHeight;
                }

                SIZE_S stScreenSize;
                stScreenSize.u32Width = pstScreenAttr->stAttr.u32Width;
                stScreenSize.u32Height = pstScreenAttr->stAttr.u32Height;

                PDT_MEDIA_GetAspectRatio(&stScreenSize, &stSrcSize,
                    &pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect);

                pstDispCfg->stVideoLayerAttr.stImageSize.u32Width =
                    pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Width;
                pstDispCfg->stVideoLayerAttr.stImageSize.u32Height =
                    pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Height;

                pstDispCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.u32Width =
                    pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Width;
                pstDispCfg->astWndCfg[s32WndIdx].stWndAttr.stRect.u32Height =
                    pstDispCfg->astWndCfg[s32WndIdx].stAspectRatio.stVideoRect.u32Height;
            }
        }
    }
    return HI_SUCCESS;
}


HI_S32 HI_PDT_Media_UpdateDispCfg(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstDispCfg, HI_PDT_MEDIA_EINVAL);

    HI_HAL_SCREEN_ATTR_S stScreenAttr;
    memset(&stScreenAttr, 0, sizeof(stScreenAttr));

    HI_S32 s32Ret = HI_SUCCESS;
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

    pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Width  = stScreenAttr.stAttr.u32Width;
    pstDispCfg->astWndCfg[0].stWndAttr.stRect.u32Height = stScreenAttr.stAttr.u32Height;

    pstDispCfg->stDispAttr.stUserInfoAttr.u32DevFrameRate = stScreenAttr.stAttr.u32Framerate;

    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.bClkReverse = stScreenAttr.stAttr.stClkAttr.bClkReverse;
    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.u32DevDiv = stScreenAttr.stAttr.stClkAttr.u32DevDiv;
    pstDispCfg->stDispAttr.stUserInfoAttr.stUserInfo.u32PreDiv = 1;

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

    pstDispCfg->astWndCfg[0].stWndAttr.stRect.s32X = 0;
    pstDispCfg->astWndCfg[0].stWndAttr.stRect.s32Y = 0;
#if defined(CONFIG_VERTICAL_SCREEN)
    pstDispCfg->astWndCfg[0].enRotate = ROTATION_90;
#endif

    s32Ret = PDT_Media_UpdateAspecRatio(pstMediaCfg, &stScreenAttr, pstDispCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "UpdateAspecRatio");
    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONSENSOR
HI_S32 HI_PDT_MEDIA_InitMotionsensor(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MAPI_MOTIONSENSOR_INFO_S stMotionAttr;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_MOTIONSENSOR_INFO,
        (HI_VOID *)&stMotionAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_MAPI_VCAP_InitMotionSensor(&stMotionAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return s32Ret;
}

HI_S32 HI_PDT_MEDIA_DeinitMotionsensor(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_MAPI_MOTIONSENSOR_INFO_S stMotionAttr;
    stMotionAttr.stTempDrift.enMode = IMU_TEMP_DRIFT_LUT;
    s32Ret = HI_MAPI_VCAP_DeInitMotionSensor(&stMotionAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_MOTIONSENSOR_INFO,
        (HI_VOID *)&stMotionAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return s32Ret;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

