/**
 * @file      hi_product_param_comm.c
 * @brief     product parameter module interface implementatio
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/7
 * @version   1.0

 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "hi_product_param_inner.h"
#include "hi_product_param.h"
#include "hi_product_param_debug.h"
#ifndef __HuaweiLite__
#include "hi_eventhub.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(ret, errcode) \
    do { \
        if (HI_SUCCESS != ret){   \
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock); \
            MLOGE(RED" ret[%08x]"NONE"\n\n", ret); \
            return errcode;  \
        }   \
    }while(0)

#define PDT_PARAM_CHECK_EXPR_WITH_UNLOCK(expr, errcode) \
    do { \
        if (!(expr)){   \
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock); \
            MLOGE(RED" expr[%s] false"NONE"\n\n", #expr); \
            return errcode;  \
        }   \
    }while(0)



HI_S32 HI_PDT_PARAM_GetFileMngCfg(HI_PDT_FILEMNG_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    if(!memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stFileMngCfg, sizeof(HI_PDT_FILEMNG_CFG_S)))
    {
        MLOGE("memcpy err\n");
        HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
        return HI_FAILURE;
    }

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    PDT_PARAM_DebugFileMngCfg(pstCfg);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetStorageCfg(HI_STORAGEMNG_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stStorageCfg, sizeof(HI_STORAGEMNG_CFG_S));
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    MLOGD("MountPath[%s] DevPath[%s]\n\n", pstCfg->szMntPath, pstCfg->szDevPath);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetKeyMngCfg(HI_KEYMNG_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stkeyMngCfg, sizeof(HI_KEYMNG_CFG_S));
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    PDT_PARAM_DebugKeyMngCfg(pstCfg);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetGaugeMngCfg(HI_GAUGEMNG_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stGaugeMngCfg, sizeof(HI_GAUGEMNG_CFG_S));
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    MLOGD("LowLevel[%d] UltraLowLevel[%d]\n\n", pstCfg->s32LowLevel, pstCfg->s32UltraLowLevel);
    return HI_SUCCESS;
}

static HI_VOID PDT_PARAM_GetMediaModeVcapCfg(
                const PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaComm,
                const PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S *pstWorkModeTypeComm,
                const PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstMediaModeSpec,
                HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfg,
                HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode,
                HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_S32 s32DevIdx, s32PipeIdx, s32PipeChnIdx;
    HI_PDT_MEDIA_VCAP_PIPE_ATTR_S*     pstPipeAttr = NULL;
    HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
    const PDT_PARAM_MEDIA_COMM_VCAP_PIPE_S*     pstMediaCommPipe = NULL;
    const PDT_PARAM_MEDIA_COMM_VCAP_PIPE_CHN_S* pstMediaCommPipeChn = NULL;
    const PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_S*     pstWorkModeTypeCommPipe = NULL;
    const PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_CHN_S* pstWorkModeTypeCommPipeChn = NULL;
    const PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_S*     pstMediaModeSpecPipe = NULL;
    const PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_CHN_S* pstMediaModeSpecPipeChn = NULL;

    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        pstVcapCfg->astVcapDevAttr[s32DevIdx].bEnable = pstWorkModeTypeComm->stVcapCfg.astVcapDevAttr[s32DevIdx].bEnable;
        pstVcapCfg->astVcapDevAttr[s32DevIdx].VcapDevHdl = pstMediaComm->stVcapCfg.astVcapDevAttr[s32DevIdx].VcapDevHdl;
        memcpy(&pstVcapCfg->astVcapDevAttr[s32DevIdx].stSnsAttr,
            &pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].stSnsAttr,
            sizeof(HI_MEDIA_SENSOR_ATTR_S));
        pstVcapCfg->astVcapDevAttr[s32DevIdx].enWdrMode =
            pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].enWdrMode;
        pstVcapCfg->astVcapDevAttr[s32DevIdx].stResolution =
            pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].stResolution;

        for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
        {
            pstPipeAttr = &pstVcapCfg->astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx];
            pstMediaCommPipe = &pstMediaComm->stVcapCfg.astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx];
            pstWorkModeTypeCommPipe = &pstWorkModeTypeComm->stVcapCfg.astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx];
            pstMediaModeSpecPipe = &pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx];

            pstPipeAttr->bEnable = pstWorkModeTypeCommPipe->bEnable;
            pstPipeAttr->VcapPipeHdl = pstMediaCommPipe->VcapPipeHdl;
            pstPipeAttr->enPipeType  = pstWorkModeTypeCommPipe->enPipeType;
            pstPipeAttr->bIspBypass  = pstWorkModeTypeCommPipe->bIspBypass;
            pstPipeAttr->stFrameRate = pstMediaModeSpecPipe->stFrameRate;
            memcpy(&pstPipeAttr->stIspPubAttr, &pstMediaModeSpecPipe->stIspPubAttr, sizeof(HI_MAPI_PIPE_ISP_ATTR_S));

            pstViVpssMode->astMode[s32DevIdx][s32PipeIdx].VcapPipeHdl = pstPipeAttr->VcapPipeHdl;
            pstViVpssMode->astMode[s32DevIdx][s32PipeIdx].enMode = pstMediaModeSpecPipe->enViVpssMode;

            for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeChnIdx)
            {
                pstPipeChnAttr = &pstPipeAttr->astPipeChnAttr[s32PipeChnIdx];
                pstMediaCommPipeChn = &pstMediaCommPipe->astPipeChnAttr[s32PipeChnIdx];
                pstWorkModeTypeCommPipeChn = &pstWorkModeTypeCommPipe->astPipeChnAttr[s32PipeChnIdx];
                pstMediaModeSpecPipeChn = &pstMediaModeSpecPipe->astPipeChnAttr[s32PipeChnIdx];

                pstPipeChnAttr->bEnable = pstWorkModeTypeCommPipeChn->bEnable;
                pstPipeChnAttr->PipeChnHdl = pstMediaCommPipeChn->PipeChnHdl;
                pstPipeChnAttr->stDestResolution = pstMediaModeSpecPipeChn->stDestResolution;
                pstPipeChnAttr->stFrameRate = pstMediaModeSpecPipeChn->stFrameRate;
                pstPipeChnAttr->enPixelFormat = pstWorkModeTypeCommPipeChn->enPixelFormat;
                pstPipeChnAttr->enRotate = pstMediaCommPipeChn->enRotate;
                pstPipeChnAttr->bFlip = pstMediaCommPipeChn->bFlip;
                pstPipeChnAttr->bMirror = pstMediaCommPipeChn->bMirror;
                pstPipeChnAttr->s32Brightness = pstMediaCommPipeChn->s32Brightness;
                pstPipeChnAttr->s32Saturation = pstMediaCommPipeChn->s32Saturation;
                pstPipeChnAttr->stLDCAttr.bEnable =
                    ((pstMediaComm->bLDCEnable && pstMediaModeSpecPipeChn->bSupportLDC) ? HI_TRUE : HI_FALSE);
                memcpy(&pstPipeChnAttr->stLDCAttr.stAttr, &pstMediaCommPipeChn->stLDCAttr,
                    sizeof(pstPipeChnAttr->stLDCAttr.stAttr));
                pstPipeChnAttr->stDISAttr.bEnable =
                    ((pstMediaComm->bDISEnable && pstMediaModeSpecPipeChn->bSupportDIS) ? HI_TRUE : HI_FALSE);

                if(pstMediaModeSpecPipeChn->bSupportDIS)
                {
                    memcpy(&pstPipeChnAttr->stDISAttr.stAttr.stDisAttr, &pstMediaModeSpecPipeChn->stDisAttr,
                        sizeof(pstPipeChnAttr->stDISAttr.stAttr.stDisAttr));

                    memcpy(&pstPipeChnAttr->stDISAttr.stAttr.stMotionSensorInfo, &pstMediaComm->stMotionSensorInfo,
                        sizeof(pstPipeChnAttr->stDISAttr.stAttr.stMotionSensorInfo));

                    HI_S32 i;
                    for(i = 0; i < PDT_PARAM_LDCV2_CFG_CNT; i++)
                    {
                        if(pstMediaModeSpecPipeChn->enSensorTimeSequece !=
                           pstMediaComm->astLDCV2Cfg[i].enSensorTimeSequece)
                        {
                            continue;
                        }

                        memcpy(&pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr,
                            &pstMediaComm->astLDCV2Cfg[i].stLDCV2Attr,
                            sizeof(VI_LDCV2_ATTR_S));

#ifdef CONFIG_MOTIONSENSOR
                        pstPipeChnAttr->stDISAttr.stAttr.stLDCV2Attr.bEnable =
                            ((pstMediaComm->bLDCEnable && pstMediaModeSpecPipeChn->bSupportLDC) ? HI_TRUE : HI_FALSE);
#endif

                        memcpy(&pstPipeChnAttr->stDISAttr.stAttr.stDISConfig,
                            &pstMediaComm->astLDCV2Cfg[i].stDISConfig,
                            sizeof(DIS_CONFIG_S));
                        pstPipeChnAttr->stDISAttr.stAttr.stDISConfig.enMode =
                            pstMediaCommPipeChn->enDISMotionType;
                        pstPipeChnAttr->stDISAttr.stAttr.stDISConfig.enMotionLevel =
                            pstMediaModeSpecPipeChn->enDISAccuracy;
                        pstPipeChnAttr->stDISAttr.stAttr.stDISConfig.u32FrameRate =
                            pstMediaModeSpecPipeChn->stFrameRate.s32SrcFrameRate;
                    }
                }
            }

            pstSceneMode->astPipeAttr[s32PipeIdx].enPipeMode = HI_PDT_SCENE_PIPE_MODE_LINEAR;
            pstSceneMode->astPipeAttr[s32PipeIdx].bEnable = pstPipeAttr->bEnable;
            pstSceneMode->astPipeAttr[s32PipeIdx].bBypassIsp = pstPipeAttr->bIspBypass;
            pstSceneMode->astPipeAttr[s32PipeIdx].bBypassVpss = pstWorkModeTypeCommPipe->stSceneCfg.bVpssBypass;
            pstSceneMode->astPipeAttr[s32PipeIdx].VcapPipeHdl = pstPipeAttr->VcapPipeHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].PipeChnHdl = pstWorkModeTypeCommPipe->stSceneCfg.PipeChnHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].MainPipeHdl = pstWorkModeTypeCommPipe->stSceneCfg.MainPipeHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].VpssHdl = pstWorkModeTypeCommPipe->stSceneCfg.VpssHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].VPortHdl = pstWorkModeTypeCommPipe->stSceneCfg.VPortHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].VencHdl = pstWorkModeTypeCommPipe->stSceneCfg.VencHdl;
            pstSceneMode->astPipeAttr[s32PipeIdx].u8PipeParamIndex =
                pstMediaModeSpecPipe->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_LINEAR];
            pstSceneMode->astPipeAttr[s32PipeIdx].enPipeType =
                ((HI_MAPI_PIPE_TYPE_VIDEO == pstPipeAttr->enPipeType) ? HI_PDT_SCENE_PIPE_TYPE_VIDEO : HI_PDT_SCENE_PIPE_TYPE_SNAP);
        }
    }
}

static HI_VOID PDT_PARAM_GetMediaModeVprocCfg(
                const PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaComm,
                const PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S *pstWorkModeTypeComm,
                const PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstMediaModeSpec,
                HI_PDT_MEDIA_VPROC_CFG_S *pstVprocCfg)
{
    HI_S32 s32VpssIdx, s32VpssPortIdx;
    HI_PDT_MEDIA_VPSS_ATTR_S*      pstVpssAttr = NULL;
    HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVpssPortAttr = NULL;
    const PDT_PARAM_MEDIA_COMM_VPSS_ATTR_S*      pstMediaCommVpss = NULL;
    const PDT_PARAM_MEDIA_COMM_VPSS_PORT_ATTR_S* pstMediaCommVpssPort = NULL;
    const PDT_PARAM_WORKMODE_COMM_VPSS_ATTR_S*      pstTypeCommVpss = NULL;
    const PDT_PARAM_WORKMODE_COMM_VPSS_PORT_ATTR_S* pstTypeCommVpssPort = NULL;
    const PDT_PARAM_MEDIAMODE_SPEC_VPSS_ATTR_S*      pstMediaSpecVpss = NULL;
    const PDT_PARAM_MEDIAMODE_SPEC_VPSS_PORT_ATTR_S* pstMediaSpecVpssPort = NULL;

    for (s32VpssIdx = 0; s32VpssIdx < HI_PDT_MEDIA_VPSS_MAX_CNT; ++s32VpssIdx)
    {
        pstVpssAttr      = &pstVprocCfg->astVpssAttr[s32VpssIdx];
        pstMediaCommVpss = &pstMediaComm->stVprocCfg.astVpssAttr[s32VpssIdx];
        pstTypeCommVpss  = &pstWorkModeTypeComm->stVprocCfg.astVpssAttr[s32VpssIdx];
        pstMediaSpecVpss = &pstMediaModeSpec->stVprocCfg.astVpssAttr[s32VpssIdx];

        pstVpssAttr->bEnable = pstTypeCommVpss->bEnable;
        pstVpssAttr->VpssHdl = pstMediaCommVpss->VpssHdl;
        pstVpssAttr->VcapPipeHdl = pstMediaCommVpss->VcapPipeHdl;
        pstVpssAttr->VcapPipeChnHdl = pstMediaCommVpss->VcapPipeChnHdl;

        pstVpssAttr->stVpssAttr.u32MaxW = pstMediaSpecVpss->u32MaxW;
        pstVpssAttr->stVpssAttr.u32MaxH = pstMediaSpecVpss->u32MaxH;
        pstVpssAttr->stVpssAttr.stFrameRate.s32SrcFrameRate = pstMediaSpecVpss->stFrameRate.s32SrcFrameRate;
        pstVpssAttr->stVpssAttr.stFrameRate.s32DstFrameRate = pstMediaSpecVpss->stFrameRate.s32DstFrameRate;
        pstVpssAttr->stVpssAttr.enPixelFormat = pstTypeCommVpss->enPixelFormat;
        pstVpssAttr->stVpssAttr.bNrEn = pstTypeCommVpss->bNrEn;
        pstVpssAttr->stVpssAttr.stNrAttr = pstTypeCommVpss->stNrAttr;

        for (s32VpssPortIdx = 0; s32VpssPortIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VpssPortIdx)
        {
            pstVpssPortAttr      = &pstVpssAttr->astVportAttr[s32VpssPortIdx];
            pstMediaCommVpssPort = &pstMediaCommVpss->astPortAttr[s32VpssPortIdx];
            pstTypeCommVpssPort  = &pstTypeCommVpss->astPortAttr[s32VpssPortIdx];
            pstMediaSpecVpssPort = &pstMediaSpecVpss->astPortAttr[s32VpssPortIdx];

            pstVpssPortAttr->bEnable = pstTypeCommVpssPort->bEnable;
            pstVpssPortAttr->VportHdl = pstMediaCommVpssPort->VportHdl;
            pstVpssPortAttr->stResolution = pstMediaSpecVpssPort->stResolution;
            pstVpssPortAttr->stFrameRate.s32SrcFrameRate = pstMediaSpecVpssPort->stFrameRate.s32SrcFrameRate;
            pstVpssPortAttr->stFrameRate.s32DstFrameRate = pstMediaSpecVpssPort->stFrameRate.s32DstFrameRate;
            pstVpssPortAttr->enRotate = pstMediaCommVpssPort->enRotate;
            pstVpssPortAttr->bFlip = pstMediaCommVpssPort->bFlip;
            pstVpssPortAttr->bMirror = pstMediaCommVpssPort->bMirror;
            pstVpssPortAttr->enVideoFormat = pstTypeCommVpssPort->enVideoFormat;
            pstVpssPortAttr->enPixelFormat = pstTypeCommVpssPort->enPixelFormat;
            pstVpssPortAttr->stAspectRatio.enMode = pstTypeCommVpssPort->stAspectRatio.enMode;
            pstVpssPortAttr->stAspectRatio.u32BgColor = pstTypeCommVpssPort->stAspectRatio.u32BgColor;
        }
    }
}

/**
 * @brief     get video encoder attribute
 * @param[in] pstVencCommAttr : common venc attribute, main/sub video
 * @param[in] pstVencSpec : mediamode specified attribute
 * @param[in] penPayloadType : if NULL common payloadtype attribute will be used
 * @param[out]pstAttr : venc attribute
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/9
 */
static HI_VOID PDT_PARAM_GetMediaModeVideoEncCfg(
                const PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstVencCommAttr,
                const PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S* pstVencSpec,
                const HI_MAPI_PAYLOAD_TYPE_E *penPayloadType,
                HI_MEDIA_VENC_ATTR_S *pstAttr)
{
    pstAttr->stTypeAttr.enType = ((penPayloadType) ? *penPayloadType : pstVencCommAttr->enType);
    pstAttr->stTypeAttr.enSceneMode = pstVencCommAttr->enSceneMode;
    pstAttr->stRcAttr.enRcMode = pstVencCommAttr->enRcMode;
    pstAttr->stTypeAttr.u32Width = pstVencSpec->stResolution.u32Width;
    pstAttr->stTypeAttr.u32Height = pstVencSpec->stResolution.u32Height;
    pstAttr->stTypeAttr.u32BufSize = pstVencSpec->u32BufSize;
    if (HI_MAPI_PAYLOAD_TYPE_H264 == pstAttr->stTypeAttr.enType)
    {
        pstAttr->stTypeAttr.u32Profile = pstVencCommAttr->u32H264Profile;

        if (HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_CBR_S* pstH264Cbr = &pstAttr->stRcAttr.unAttr.stH264Cbr;
            pstH264Cbr->stAttr.u32BitRate = pstVencSpec->u32H264Bitrate_Kbps;
            pstH264Cbr->stAttr.u32SrcFrameRate  = pstVencSpec->u32SrcFrameRate;
            pstH264Cbr->stAttr.fr32DstFrameRate  = pstVencSpec->fr32DstFrameRate;
            pstH264Cbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstH264Cbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
            pstH264Cbr->u32MaxQp  = pstVencCommAttr->stH264Cbr.u32MaxQp;
            pstH264Cbr->u32MinQp  = pstVencCommAttr->stH264Cbr.u32MinQp;
            pstH264Cbr->u32MaxIQp = pstVencCommAttr->stH264Cbr.u32MaxIQp;
            pstH264Cbr->u32MinIQp = pstVencCommAttr->stH264Cbr.u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_VBR_S* pstH264Vbr = &pstAttr->stRcAttr.unAttr.stH264Vbr;
            pstH264Vbr->stAttr.u32MaxBitRate = pstVencSpec->u32H264Bitrate_Kbps;
            pstH264Vbr->stAttr.u32SrcFrameRate = pstVencSpec->u32SrcFrameRate;
            pstH264Vbr->stAttr.fr32DstFrameRate = pstVencSpec->fr32DstFrameRate;
            pstH264Vbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstH264Vbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
            pstH264Vbr->u32MaxQp  = pstVencCommAttr->stH264Vbr.u32MaxQp;
            pstH264Vbr->u32MinQp  = pstVencCommAttr->stH264Vbr.u32MinQp;
            pstH264Vbr->u32MaxIQp = pstVencCommAttr->stH264Vbr.u32MaxIQp;
            pstH264Vbr->u32MinIQp = pstVencCommAttr->stH264Vbr.u32MinIQp;
        }
    }
    else if (HI_MAPI_PAYLOAD_TYPE_H265 == pstAttr->stTypeAttr.enType)
    {
        pstAttr->stTypeAttr.u32Profile = pstVencCommAttr->u32H265Profile;

        if (HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_CBR_S* pstH265Cbr = &pstAttr->stRcAttr.unAttr.stH265Cbr;
            pstH265Cbr->stAttr.u32BitRate = pstVencSpec->u32H265Bitrate_Kbps;
            pstH265Cbr->stAttr.u32SrcFrameRate = pstVencSpec->u32SrcFrameRate;
            pstH265Cbr->stAttr.fr32DstFrameRate = pstVencSpec->fr32DstFrameRate;
            pstH265Cbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstH265Cbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
            pstH265Cbr->u32MaxQp  = pstVencCommAttr->stH265Cbr.u32MaxQp;
            pstH265Cbr->u32MinQp  = pstVencCommAttr->stH265Cbr.u32MinQp;
            pstH265Cbr->u32MaxIQp = pstVencCommAttr->stH265Cbr.u32MaxIQp;
            pstH265Cbr->u32MinIQp = pstVencCommAttr->stH265Cbr.u32MinIQp;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_VBR_S* pstH265Vbr = &pstAttr->stRcAttr.unAttr.stH265Vbr;
            pstH265Vbr->stAttr.u32MaxBitRate = pstVencSpec->u32H265Bitrate_Kbps;
            pstH265Vbr->stAttr.u32SrcFrameRate = pstVencSpec->u32SrcFrameRate;
            pstH265Vbr->stAttr.fr32DstFrameRate = pstVencSpec->fr32DstFrameRate;
            pstH265Vbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstH265Vbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
            pstH265Vbr->u32MaxQp  = pstVencCommAttr->stH265Vbr.u32MaxQp;
            pstH265Vbr->u32MinQp  = pstVencCommAttr->stH265Vbr.u32MinQp;
            pstH265Vbr->u32MaxIQp = pstVencCommAttr->stH265Vbr.u32MaxIQp;
            pstH265Vbr->u32MinIQp = pstVencCommAttr->stH265Vbr.u32MinIQp;
        }
    }
    else if (HI_MAPI_PAYLOAD_TYPE_MJPEG == pstAttr->stTypeAttr.enType)
    {
        if (HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_CBR_S* pstMjpegCbr = &pstAttr->stRcAttr.unAttr.stMjpegCbr;
            pstMjpegCbr->stAttr.u32BitRate = pstVencSpec->u32H264Bitrate_Kbps;
            pstMjpegCbr->stAttr.u32SrcFrameRate = pstVencSpec->u32SrcFrameRate;
            pstMjpegCbr->stAttr.fr32DstFrameRate = pstVencSpec->fr32DstFrameRate;
            pstMjpegCbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstMjpegCbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
            pstMjpegCbr->u32MaxQfactor = pstVencCommAttr->stMjpegCbr.u32MaxQfactor;
            pstMjpegCbr->u32MinQfactor = pstVencCommAttr->stMjpegCbr.u32MinQfactor;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_VBR_S* pstMjpegVbr = &pstAttr->stRcAttr.unAttr.stMjpegVbr;
            pstMjpegVbr->stAttr.u32MaxBitRate = pstVencSpec->u32H264Bitrate_Kbps;
            pstMjpegVbr->stAttr.u32SrcFrameRate = pstVencSpec->u32SrcFrameRate;
            pstMjpegVbr->stAttr.fr32DstFrameRate = pstVencSpec->fr32DstFrameRate;
            pstMjpegVbr->stAttr.u32Gop = pstVencSpec->u32Gop;
            pstMjpegVbr->stAttr.u32StatTime = pstVencSpec->u32StatTime;
        }
    }
}

static HI_VOID PDT_PARAM_GetMediaModeSnapEncCfg(
                const PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S *pstEncAttr,
                const PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S* pstVencSpec,
                HI_MEDIA_VENC_ATTR_S *pstAttr)
{
    pstAttr->stTypeAttr.enType = pstEncAttr->enType;
    pstAttr->stTypeAttr.enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV;
    pstAttr->stTypeAttr.u32Width = pstVencSpec->stResolution.u32Width;
    pstAttr->stTypeAttr.u32Height = pstVencSpec->stResolution.u32Height;
    pstAttr->stTypeAttr.u32BufSize = pstVencSpec->u32BufSize;
    if (HI_MAPI_PAYLOAD_TYPE_JPEG == pstEncAttr->enType)
    {
        pstAttr->stTypeAttr.stAttrJpege.bEnableDCF = pstEncAttr->bEnableDCF;
        pstAttr->stTypeAttr.stAttrJpege.u32Qfactor = pstEncAttr->u32Qfactor;
        pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum =
            (pstEncAttr->bMpfEnable ? 1 : 0);
        pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[0].u32Width =
            pstVencSpec->stMpfSize.u32Width;
        pstAttr->stTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[0].u32Height=
            pstVencSpec->stMpfSize.u32Height;
        pstAttr->stTypeAttr.stAttrJpege.enJpegEncodeMode = pstEncAttr->enEncodeMode;
    }
}

static HI_VOID PDT_PARAM_GetMediaModeVencCfg(
                const PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaComm,
                const PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S *pstWorkModeTypeComm,
                const PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstMediaModeSpec,
                HI_PDT_MEDIA_VENC_CFG_S *pstVencCfg)
{
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        pstVencCfg[s32Idx].bEnable = pstWorkModeTypeComm->astVencCfg[s32Idx].bEnable;
        pstVencCfg[s32Idx].VencHdl = pstWorkModeTypeComm->astVencCfg[s32Idx].VencHdl;
        pstVencCfg[s32Idx].enBindedMod = pstWorkModeTypeComm->astVencCfg[s32Idx].enBindedMod;
        pstVencCfg[s32Idx].ModHdl  = pstWorkModeTypeComm->astVencCfg[s32Idx].ModHdl;
        pstVencCfg[s32Idx].ChnHdl  = pstWorkModeTypeComm->astVencCfg[s32Idx].ChnHdl;

        if (PDT_PARAM_VENC_TYPE_MAIN_VIDEO == pstWorkModeTypeComm->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeVideoEncCfg(&pstMediaComm->stMainVideoVencAttr,
                &pstMediaModeSpec->astVencCfg[s32Idx], NULL, &pstVencCfg[s32Idx].stVencAttr);
        }
        else if (PDT_PARAM_VENC_TYPE_SUB_VIDEO == pstWorkModeTypeComm->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeVideoEncCfg(&pstMediaComm->stSubVideoVencAttr,
                &pstMediaModeSpec->astVencCfg[s32Idx], NULL, &pstVencCfg[s32Idx].stVencAttr);
        }
        else if (PDT_PARAM_VENC_TYPE_SINGLE_SNAP == pstWorkModeTypeComm->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeSnapEncCfg(&pstMediaComm->stSingleSnapVencAttr,
                &pstMediaModeSpec->astVencCfg[s32Idx], &pstVencCfg[s32Idx].stVencAttr);
        }
        else if (PDT_PARAM_VENC_TYPE_MULTI_SNAP == pstWorkModeTypeComm->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeSnapEncCfg(&pstMediaComm->stMultiSnapVencAttr,
                &pstMediaModeSpec->astVencCfg[s32Idx], &pstVencCfg[s32Idx].stVencAttr);
        }
        else if (PDT_PARAM_VENC_TYPE_SUB_SNAP == pstWorkModeTypeComm->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeSnapEncCfg(&pstMediaComm->stSubSnapVencAttr,
                &pstMediaModeSpec->astVencCfg[s32Idx], &pstVencCfg[s32Idx].stVencAttr);
        }
        memcpy(&pstVencCfg->stVencAttr.stRcAttr.stHierarchicalQp,
            &pstMediaComm->stHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));
    }
}

/**
 * @brief     find matched mediamode array index
 * @param[in] enMediaMode : mediamode enum
 * @param[in] ps32ModeIdx : mediamode array index
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/9
 */
static HI_S32 PDT_PARAM_GetMediaModeIdx(HI_PDT_MEDIAMODE_E enMediaMode, HI_S32* ps32ModeIdx)
{
    HI_S32 s32ModeIdx;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    for (s32ModeIdx = 0; s32ModeIdx < PDT_PARAM_MEDIAMODE_CNT; ++s32ModeIdx)
    {
        MLOGD("enMediaMode = %d\n", pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enMediaMode);
        if (enMediaMode == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enMediaMode)
        {
            MLOGD("Find MediaMode[%s] Index[%d]\n", PDT_PARAM_GetMediaModeStr(enMediaMode), s32ModeIdx);
            break;
        }
    }
    if (s32ModeIdx >= PDT_PARAM_MEDIAMODE_CNT)
    {
        MLOGE("Invalid MediaMode[%s]\n", PDT_PARAM_GetMediaModeStr(enMediaMode));
        return HI_PDT_PARAM_EINVAL;
    }
    *ps32ModeIdx = s32ModeIdx;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_MEDIA_CFG_S* pstCfg, HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstCfg, 0, sizeof(HI_PDT_MEDIA_CFG_S));
    memset(pstSceneMode, 0, sizeof(HI_PDT_SCENE_MODE_S));
    MLOGD("WorkMode[%s] MediaMode[%s]\n",
        PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetMediaModeStr(enMediaMode));

    HI_S32 s32Ret;
    HI_S32 s32ModeIdx = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    /* Find Matched MediaMode Index */
    s32Ret = PDT_PARAM_GetMediaModeIdx(enMediaMode, &s32ModeIdx);
    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

    /* ------ Fill MediaMode Configure ------ */

    /* VB Configure */
    memcpy(&pstCfg->stVBCfg, &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].stVBCfg,
                sizeof(HI_PDT_MEDIA_VB_CFG_S));

    /* Vcap/Vproc/Venc/Display */
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S* pstWorkModeTypeMediaCfg = NULL;
    if (PDT_PARAM_WORKMODE_TYPE_REC == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stRecMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_PHOTO == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stPhotoMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_MPHOTO == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stMPhotoMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_RECSNAP == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stRecSnapMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_PLAYBACK == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stPlayBackMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_HDMIPREVIEW == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stHDMIPreviewMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_HDMIPLAYBACK == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstWorkModeTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stHDMIPlaybackMediaCfg.stMediaCfg;
    }
    else
    {
        HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
        return HI_FAILURE;
    }
    PDT_PARAM_GetMediaModeVcapCfg(&pstParamCfg->stMediaCommCfg, pstWorkModeTypeMediaCfg,
        &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx],
        &pstCfg->stVideoCfg.stVcapCfg, &pstCfg->stViVpssMode, pstSceneMode);
    PDT_PARAM_GetMediaModeVprocCfg(&pstParamCfg->stMediaCommCfg, pstWorkModeTypeMediaCfg,
        &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx], &pstCfg->stVideoCfg.stVprocCfg);
    PDT_PARAM_GetMediaModeVencCfg(&pstParamCfg->stMediaCommCfg, pstWorkModeTypeMediaCfg,
        &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx], pstCfg->stVideoCfg.astVencCfg);
    memcpy(pstCfg->stVideoOutCfg.astDispCfg, pstWorkModeTypeMediaCfg->astDispCfg,
        sizeof(HI_PDT_MEDIA_DISP_CFG_S) * HI_PDT_MEDIA_DISP_MAX_CNT);

    memcpy(pstCfg->stAudioCfg.astAcapCfg, pstParamCfg->stMediaCommCfg.astAcapCfg,
        sizeof(HI_PDT_MEDIA_ACAP_CFG_S) * HI_PDT_MEDIA_ACAP_MAX_CNT);
    memcpy(pstCfg->stAudioCfg.astAencCfg, pstParamCfg->stMediaCommCfg.astAencCfg,
        sizeof(HI_PDT_MEDIA_AENC_CFG_S) * HI_PDT_MEDIA_AENC_MAX_CNT);
    memcpy(pstCfg->stAudioOutCfg.astAoCfg, pstParamCfg->stMediaCommCfg.astAoCfg,
        sizeof(HI_PDT_MEDIA_AO_CFG_S) * HI_PDT_MEDIA_AO_MAX_CNT);
    memcpy(&pstCfg->stVideoCfg.stOsdCfg, &pstParamCfg->stMediaCommCfg.stOsdCfg,
        sizeof(HI_PDT_MEDIA_OSD_CFG_S));

    HI_S32 i;
    PDT_PARAM_WORKMODE_COMM_AO_CFG_S* pstAoCfg = &pstWorkModeTypeMediaCfg->astAoCfg[0];

    /* update hdmi preview configure  */
    if (HI_PDT_WORKMODE_HDMI_PREVIEW == enWorkMode)
    {
        HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
        pstDispCfg = &pstParamCfg->stWorkModeCfg.stHDMIPreviewMediaCfg.stMediaCfg.astDispCfg[0];
        pstAoCfg = &pstParamCfg->stWorkModeCfg.stHDMIPreviewMediaCfg.stMediaCfg.astAoCfg[0];

        for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
        {
            pstCfg->stVideoOutCfg.astDispCfg[i].bEnable = pstDispCfg[i].bEnable;
            pstCfg->stVideoOutCfg.astDispCfg[i].VdispHdl = pstDispCfg[i].VdispHdl;
            pstCfg->stVideoOutCfg.astDispCfg[i].stCscAttr = pstDispCfg[i].stCscAttr;
        }
    }

    /* diable audio if global audio off */
    if (!pstParamCfg->stMediaCommCfg.bAudio)
    {
        for (i = 0; i < HI_PDT_MEDIA_ACAP_MAX_CNT; ++i)
        {
            pstCfg->stAudioCfg.astAcapCfg[i].bEnable = HI_FALSE;
        }
        for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; ++i)
        {
            pstCfg->stAudioCfg.astAencCfg[i].bEnable = HI_FALSE;
        }
    }

    /* update ao handle */
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        pstCfg->stAudioOutCfg.astAoCfg[i].AoHdl = pstAoCfg[i].AoHdl;
        pstCfg->stAudioOutCfg.astAoCfg[i].AoChnHdl = pstAoCfg[i].AoChnHdl;
        pstCfg->stAudioOutCfg.astAoCfg[i].stAoAttr.enI2sType = pstAoCfg[i].enI2sType;
    }

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    PDT_PARAM_DebugMediaCfg(pstCfg);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetSceneCfg(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_PARAM_SCENE_TYPE_E enSceneType,
                HI_PDT_PARAM_SCENE_CFG_S* pstSceneCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneCfg, HI_PDT_PARAM_EINVAL);
    if (enSceneType < HI_PDT_PARAM_SCENE_TYPE_LINEAR
        || enSceneType >= HI_PDT_PARAM_SCENE_TYPE_BUTT)
    {
        return HI_PDT_PARAM_EINVAL;
    }
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstSceneCfg, 0, sizeof(HI_PDT_PARAM_SCENE_CFG_S));

    MLOGD("SceneType[%d:0-linear,1-hdr,2-mfnr]\n", enSceneType);

    HI_S32 s32Ret;
    HI_S32 s32ModeIdx = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    /* Find Matched MediaMode Index */
    s32Ret = PDT_PARAM_GetMediaModeIdx(enMediaMode, &s32ModeIdx);
    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

    HI_S32 i, j;
    const PDT_PARAM_MEDIAMODE_SPEC_VCAP_DEV_S* pstVcapDev = NULL;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        pstVcapDev = &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].stVcapCfg.astVcapDevAttr[i];
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            pstSceneCfg->au8SceneParamIdx[j] =
                pstVcapDev->astVcapPipeAttr[j].au8SceneParamIdx[enSceneType];
            MLOGD("VcapDev[%d] Pipe[%d] Scene[%d] ParamIdx[%u]\n",
                i, j, enSceneType, pstSceneCfg->au8SceneParamIdx[j]);
        }
    }

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

static HI_VOID PDT_PARAM_GetMediaVcapDevCapbility(
                const PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstMediaModeSpec,
                HI_PDT_PARAM_MEDIA_VCAP_DEV_CAPABILITY_S* pstVcapDev)
{
    HI_S32 s32DevIdx, s32PipeIdx, s32PipeChnIdx;
    HI_PDT_PARAM_MEDIA_VCAP_PIPE_CHN_CAPABILITY_S* pstPipeChn = NULL;
    for (s32DevIdx = 0; s32DevIdx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32DevIdx)
    {
        for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
        {
            for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeChnIdx)
            {
                pstPipeChn = &pstVcapDev[s32DevIdx].astVcapPipe[s32PipeIdx].astPipeChn[s32PipeChnIdx];
                pstPipeChn->bSupportLDC =
                    pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx].astPipeChnAttr[s32PipeChnIdx].bSupportLDC;
                pstPipeChn->bSupportDIS =
                    pstMediaModeSpec->stVcapCfg.astVcapDevAttr[s32DevIdx].astVcapPipeAttr[s32PipeIdx].astPipeChnAttr[s32PipeChnIdx].bSupportDIS;
                MLOGD("VcapDev[%d] Pipe[%d] PipeChn[%d] %s LDC; %s DIS\n",
                    s32DevIdx, s32PipeIdx, s32PipeChnIdx,
                    (pstPipeChn->bSupportLDC) ? "Support" : "Not support",
                    (pstPipeChn->bSupportDIS) ? "Support" : "Not support");
            }
        }
    }
}

HI_S32 HI_PDT_PARAM_GetMediaCapability(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_PDT_PARAM_MEDIA_CAPABILITY_S* pstCapability)
{
    HI_APPCOMM_CHECK_POINTER(pstCapability, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstCapability, 0, sizeof(HI_PDT_PARAM_MEDIA_CAPABILITY_S));

    HI_S32 s32Ret;
    HI_S32 s32ModeIdx = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    /* Find Matched MediaMode Index */
    s32Ret = PDT_PARAM_GetMediaModeIdx(enMediaMode, &s32ModeIdx);
    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

    /* ------ Fill Media Capability ------ */
    PDT_PARAM_GetMediaVcapDevCapbility(&pstParamCfg->astMediaModeSpecCfg[s32ModeIdx], pstCapability->astVcapDev);

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetVencAttrByPayloadType(HI_PDT_MEDIAMODE_E enMediaMode,
                HI_MAPI_PAYLOAD_TYPE_E enPayload, HI_MEDIA_VENC_ATTR_S* pstAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstAttr, 0, sizeof(HI_MEDIA_VENC_ATTR_S));

    HI_S32 s32Ret;
    HI_S32 s32ModeIdx = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    /* Find Matched MediaMode Index */
    s32Ret = PDT_PARAM_GetMediaModeIdx(enMediaMode, &s32ModeIdx);
    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

    HI_S32 s32Idx;
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S* pstTypeMediaCfg = NULL;
    if (PDT_PARAM_WORKMODE_TYPE_REC == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stRecMediaCfg.stMediaCfg;
    }
    else if (PDT_PARAM_WORKMODE_TYPE_RECSNAP == pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].enWorkModeType)
    {
        pstTypeMediaCfg = &pstParamCfg->stWorkModeCfg.stRecSnapMediaCfg.stMediaCfg;
    }
    else
    {
        HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
        MLOGD("Invalid MediaMode[not REC/RECSNAP]\n");
        return HI_PDT_PARAM_EINVAL;
    }

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        if (PDT_PARAM_VENC_TYPE_MAIN_VIDEO == pstTypeMediaCfg->astVencCfg[s32Idx].enVencType)
        {
            PDT_PARAM_GetMediaModeVideoEncCfg(&pstParamCfg->stMediaCommCfg.stMainVideoVencAttr,
                &pstParamCfg->astMediaModeSpecCfg[s32ModeIdx].astVencCfg[s32Idx], &enPayload, pstAttr);
        }
    }
    PDT_PARAM_DebugVencAttr(pstAttr);

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetOSDVideoAttr(HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoAttr, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memcpy(pstVideoAttr, &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stOsdVideoAttr,
        sizeof(HI_PDT_MEDIA_OSD_VIDEO_ATTR_S));

#ifdef CFG_DEBUG_LOG_ON
    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                MLOGD("Vcap[%d] Pipe[%d] Chn[%d] MaxSize[%ux%u]\n", i, j, k,
                    pstVideoAttr->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Width,
                    pstVideoAttr->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Height);
            }
        }
    }
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            MLOGD("Vpss[%d] Port[%d] MaxSize[%ux%u]\n", i, j,
                    pstVideoAttr->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Width,
                    pstVideoAttr->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Height);
        }
    }
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        MLOGD("Venc[%d] MaxSize[%ux%u]\n", i,
                pstVideoAttr->astVencAttr[i].stMaxSize.u32Width,
                pstVideoAttr->astVencAttr[i].stMaxSize.u32Height);
    }
#endif
    return HI_SUCCESS;
}


static HI_VOID PDT_PARAM_GetRecCommAttr(HI_PDT_REC_COMM_ATTR_S *pstCommAttr,
                const PDT_PARAM_REC_COMM_ATTR_S *pstRecCommAttr)
{
    /* Record Common Configure */
    pstCommAttr->enMuxerType = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enRecMuxerType;
    pstCommAttr->enThmType = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enRecThmType;
    pstCommAttr->u32RepairUnit = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.u32RepairUnit;
    pstCommAttr->u32PreAllocUnit = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.u32PreAllocUnit;

    /* Record Mode Configure */
    HI_S32 s32Idx;
    pstCommAttr->enMediaMode = pstRecCommAttr->enMediaMode;
    pstCommAttr->stSplitAttr = pstRecCommAttr->stSplitAttr;
    pstCommAttr->u32BufferTimeMSec = pstRecCommAttr->u32BufferTimeMSec;
    pstCommAttr->u32PreRecTimeSec  = pstRecCommAttr->u32PreRecTimeSec;
    for (s32Idx = 0; s32Idx < HI_PDT_REC_VSTREAM_MAX_CNT; s32Idx++)
    {
        pstCommAttr->au32VBufSize[s32Idx] = pstRecCommAttr->au32VBufSize[s32Idx];
    }
    pstCommAttr->u8FileTypeIdx = pstRecCommAttr->u8FileTypeIdx;
}

static HI_VOID PDT_PARAM_GetNormRecCfg(HI_PDT_NORM_REC_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.stDataSrc, sizeof(HI_PDT_REC_SRC_S));

    PDT_PARAM_GetRecCommAttr(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stNormRecAttr.stCommAttr);

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugNormRecCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetLoopRecCfg(HI_PDT_LOOP_REC_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.stDataSrc, sizeof(HI_PDT_REC_SRC_S));

    PDT_PARAM_GetRecCommAttr(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.stCommAttr);
    pstCfg->u32LoopTime_min = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.u32LoopTime_min;

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugLoopRecCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetLapseRecCfg(HI_PDT_LAPSE_REC_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.stDataSrc, sizeof(HI_PDT_REC_SRC_S));

    PDT_PARAM_GetRecCommAttr(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.stCommAttr);
    pstCfg->u32Interval_ms = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.u32Interval_ms;

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugLapseRecCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetSlowRecCfg(HI_PDT_SLOW_REC_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.stDataSrc, sizeof(HI_PDT_REC_SRC_S));

    PDT_PARAM_GetRecCommAttr(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSlowRecAttr.stCommAttr);
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < HI_PDT_REC_VSTREAM_MAX_CNT; s32Idx++)
    {
        pstCfg->au32PlayFrmRate[s32Idx] = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSlowRecAttr.au32PlayFrmRate[s32Idx];
    }

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugSlowRecCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetSingPhotoCfg(HI_PDT_SINGLE_PHOTO_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.stDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));

    memcpy(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSingPhotoAttr.stCommAttr, sizeof(HI_PDT_PHOTO_COMM_ATTR_S));
    pstCfg->enOutputFmt = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enOutputFmt;
    pstCfg->enScene = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enScene;
    memcpy(&pstCfg->stProcAlg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.stProcAlg, sizeof(HI_PDT_PHOTO_PROCALG_S));

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugSinglePhotoCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetDelayPhotoCfg(HI_PDT_DELAY_PHOTO_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.stDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));

    memcpy(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.stCommAttr, sizeof(HI_PDT_PHOTO_COMM_ATTR_S));
    pstCfg->enOutputFmt = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enOutputFmt;
    pstCfg->enScene = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enScene;
    memcpy(&pstCfg->stProcAlg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.stProcAlg, sizeof(HI_PDT_PHOTO_PROCALG_S));
    pstCfg->u32DelayTime_s = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.u32DelayTime_s;

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugDelayPhotoCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetLapsePhotoCfg(HI_PDT_LAPSE_PHOTO_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.stDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));

    memcpy(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.stCommAttr, sizeof(HI_PDT_PHOTO_COMM_ATTR_S));
    pstCfg->u32Interval_ms = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32Interval_ms;

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
    pstCfg->stProTune.u32ExpTime_us = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32ExpTime_us;

    PDT_PARAM_DebugLapsePhotoCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetBurstCfg(HI_PDT_BURST_ATTR_S *pstCfg)
{
    HI_S32 s32Idx = 0;
    HI_S32 i;

    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stMPhotoMediaCfg.stDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));

    memcpy(&pstCfg->stCommAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr, sizeof(HI_PDT_PHOTO_COMM_ATTR_S));
    for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
    {
        if (PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode
            == PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.stMediaModeValues.astValues[i].s32Value)
        {
            s32Idx = i;
            break;
        }
    }
    pstCfg->enJpgBurstType = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.aenJpgBurstType[s32Idx];

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugBurstCfg(pstCfg);
}

static HI_VOID PDT_PARAM_GetRecSnapCfg(HI_PDT_RECSNAP_ATTR_S *pstCfg)
{
    memcpy(&pstCfg->stDataSrc, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapMediaCfg.stDataSrc, sizeof(HI_PDT_RECSNAP_SRC_S));

    PDT_PARAM_GetRecCommAttr(&pstCfg->stRecAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.stCommAttr);
    pstCfg->u32SnapFileTypeIndex = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.u32SnapFileTypeIndex;
    pstCfg->u32SnapInterval_ms = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.u32SnapInterval_ms;

    memcpy(&pstCfg->stProTune, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    PDT_PARAM_DebugRecSnapCfg(pstCfg);
}

HI_S32 HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_WORKMODE_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));

    pstCfg->enWorkMode = enWorkMode;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            PDT_PARAM_GetNormRecCfg(&pstCfg->unModeAttr.stNormRecAttr);
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            PDT_PARAM_GetLoopRecCfg(&pstCfg->unModeAttr.stLoopRecAttr);
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            PDT_PARAM_GetLapseRecCfg(&pstCfg->unModeAttr.stLapseRecAttr);
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            PDT_PARAM_GetSlowRecCfg(&pstCfg->unModeAttr.stSlowRecAttr);
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
            PDT_PARAM_GetSingPhotoCfg(&pstCfg->unModeAttr.stSinglePhotoAttr);
            break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetDelayPhotoCfg(&pstCfg->unModeAttr.stDelayPhotoAttr);
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetLapsePhotoCfg(&pstCfg->unModeAttr.stLapsePhotoAttr);
            break;
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetBurstCfg(&pstCfg->unModeAttr.stBurstAttr);
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            PDT_PARAM_GetRecSnapCfg(&pstCfg->unModeAttr.stRecSnapAttr);
            break;
        case HI_PDT_WORKMODE_UVC:
            memcpy(&pstCfg->unModeAttr.stUvcAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcAttr,
                sizeof(HI_PDT_PARAM_UVC_ATTR_S));
            PDT_PARAM_DebugUVCCfg(&pstCfg->unModeAttr.stUvcAttr);
            break;
        case HI_PDT_WORKMODE_USB_STORAGE:
            memcpy(&pstCfg->unModeAttr.stUsbStorageCfg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUsbStorageCfg,
                sizeof(HI_USB_STORAGE_CFG_S));
            PDT_PARAM_DebugUSBStorageCfg(&pstCfg->unModeAttr.stUsbStorageCfg);
            break;
        case HI_PDT_WORKMODE_HDMI_PREVIEW:
            memcpy(&pstCfg->unModeAttr.stHDMIPreviewAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stHDMIPreviewAttr,
                sizeof(HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S));
            PDT_PARAM_DebugHDMIPreviewCfg(&pstCfg->unModeAttr.stHDMIPreviewAttr);
            break;
        case HI_PDT_WORKMODE_HDMI_PLAYBACK:
            memcpy(&pstCfg->unModeAttr.stHDMIPlaybackAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stHDMIPlaybackAttr,
                sizeof(HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S));
            PDT_PARAM_DebugHDMIPlaybackCfg(&pstCfg->unModeAttr.stHDMIPlaybackAttr);
            break;
        case HI_PDT_WORKMODE_PLAYBACK:
            memcpy(&pstCfg->unModeAttr.stPlaybackAttr, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPlaybackAttr,
                sizeof(HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S));
            PDT_PARAM_DebugPlaybackCfg(&pstCfg->unModeAttr.stPlaybackAttr);
            break;
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGW("Unsupport workmode[%s]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_FAILURE;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMediaMode(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_MEDIAMODE_E *penMediaMode)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stNormRecAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSlowRecAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSingPhotoAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_BURST:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_UVC:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_HDMI_PREVIEW:
            *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stHDMIPreviewAttr.enMediaMode;
            break;
        case HI_PDT_WORKMODE_HDMI_PLAYBACK:
            *penMediaMode = HI_PDT_MEDIAMODE_HDMIPLAYBACK;
            break;
        case HI_PDT_WORKMODE_PLAYBACK:
            *penMediaMode = HI_PDT_MEDIAMODE_PLAYBACK;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[MediaMode]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] MediaMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetMediaModeStr(*penMediaMode));
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetPhotoScene(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PHOTO_SCENE_E *penScene)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            *penScene = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enScene;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[PhotoScene]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] PhotoScene[%d: 0-NORM,1-HDR,2-LL]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *penScene);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetPhotoOutputFmt(HI_PDT_WORKMODE_E enWorkMode, HI_PHOTOMNG_OUTPUT_FORMAT_E *penFormat)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            *penFormat = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enOutputFmt;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[PhotoOutputFormat]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] Photo OutputFormat[%d: 0-JPEG,1-DNG,2-DNG+JPEG]\n\n",
                PDT_PARAM_GetWorkModeStr(enWorkMode), *penFormat);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetDelayTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.u32DelayTime_s;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[DelayTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] DelayTime[%uS]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetLapseInterval(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LPSE_REC:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.u32Interval_ms;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32Interval_ms;
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.u32SnapInterval_ms;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[LapseInterval]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] LapseInterval[%ums]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetBurstType(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_MEDIAMODE_E enMediaMode,
                                     HI_PDT_JPG_BURST_TYPE_E* penType)
{
    HI_S32 i;
    if (HI_PDT_WORKMODE_BURST != enWorkMode)
    {
        MLOGE("Unsupport WorkMode[%s] With Param Type[BurstType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
        return HI_PDT_PARAM_EUNSUPPORT;
    }

    for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
    {
        if (enMediaMode == PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.stMediaModeValues.astValues[i].s32Value)
        {
            *penType = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.aenJpgBurstType[i];
            MLOGD("MediaMode[%s] BurstType[%d 0-3_1,1-5_1,2-10_1,3-15_1,4-30_1,5-20_2,6-30_2,7-30_3,]\n\n",
                PDT_PARAM_GetMediaModeStr(enMediaMode), *penType);
            return HI_SUCCESS;
        }
    }
    MLOGE("Unsupport MediaMode[%s]\n",
        PDT_PARAM_GetMediaModeStr(PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode));
    return HI_PDT_PARAM_EINVAL;
}

static HI_S32 PDT_PARAM_GetLoopTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LOOP_REC:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.u32LoopTime_min;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[LoopTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] LoopTime[%umin]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetEncPayloadType(HI_PDT_WORKMODE_E enWorkMode, HI_MAPI_PAYLOAD_TYPE_E *penParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMainVideoVencAttr.enType;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stSingleSnapVencAttr.enType;
            break;
        case HI_PDT_WORKMODE_BURST:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMultiSnapVencAttr.enType;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[EncPayloadType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] PayloadType[%d 0-H264,1-H265,2-MJPEG,3-JPEG]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *penParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetEV(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_SCENE_EV_E *penParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.enEV;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.enEV;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[EV]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] EV[%d 0-B2,1-B1_5,2-B1,3-B0_5,4-0,5-0_5,6-1,7-1_5,8-2]\n\n",
                PDT_PARAM_GetWorkModeStr(enWorkMode), *penParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetExpTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32ExpTime_us;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32ExpTime_us;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[ExpTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] ExpTime[%ums]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetISO(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.u32ISO;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32ISO;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[ISO]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] ISO[%u]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetWB(HI_PDT_WORKMODE_E enWorkMode, HI_U32 *pu32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.u32WB;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *pu32Param = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32WB;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[WB]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] WB[%u]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *pu32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMetry(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_SCENE_METRY_TYPE_E *penParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.enMetryType;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            *penParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.enMetryType;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[MetryType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] Metry[%d 0-CENTER,1-AVERAGE,2-SPOT]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *penParam);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetBurstType(HI_PDT_MEDIAMODE_E enMediaMode, HI_PDT_JPG_BURST_TYPE_E* penType)
{
    HI_APPCOMM_CHECK_POINTER(penType, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    s32Ret = PDT_PARAM_GetBurstType(HI_PDT_WORKMODE_BURST, enMediaMode, penType);
    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_PARAM_GetMediaMode(enWorkMode, (HI_PDT_MEDIAMODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
            s32Ret = PDT_PARAM_GetPhotoScene(enWorkMode, (HI_PDT_PHOTO_SCENE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
            s32Ret = PDT_PARAM_GetPhotoOutputFmt(enWorkMode, (HI_PHOTOMNG_OUTPUT_FORMAT_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
            s32Ret = PDT_PARAM_GetDelayTime(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_PARAM_GetLapseInterval(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
            s32Ret = PDT_PARAM_GetBurstType(enWorkMode,
                PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode,
                (HI_PDT_JPG_BURST_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
            s32Ret = PDT_PARAM_GetLoopTime(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_PARAM_GetEncPayloadType(enWorkMode, (HI_MAPI_PAYLOAD_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_PARAM_GetEV(enWorkMode, (HI_PDT_SCENE_EV_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            s32Ret = PDT_PARAM_GetExpTime(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_PARAM_GetISO(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_PARAM_GetWB(enWorkMode, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_PARAM_GetMetry(enWorkMode, (HI_PDT_SCENE_METRY_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stOsdCfg.stTimeOsd.bShow;
            MLOGD("WorkMode[%s] OSD Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
        {
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bAudio;
            MLOGD("WorkMode[%s] Audio Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_FLIP:
        {
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bFlip;
            MLOGD("WorkMode[%s] FLIP Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_DIS:
        {
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bDISEnable;
            MLOGD("WorkMode[%s] DIS Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_LDC:
        {
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bLDCEnable;
            MLOGD("WorkMode[%s] LDC Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
            *(HI_PDT_WORKMODE_E*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronWorkMode;
            MLOGD("PoweronWorkMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(*(HI_PDT_WORKMODE_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            *(HI_PDT_POWERON_ACTION_E*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronAction;
            MLOGD("PoweronAction[%s]\n\n", PDT_PARAM_GetPoweronActionStr(*(HI_PDT_POWERON_ACTION_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_WIFI_AP:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
            PDT_PARAM_DebugWiFiAPCfg((HI_HAL_WIFI_APMODE_CFG_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_DEV_INFO:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stDevInfo, sizeof(HI_UPGRADE_DEV_INFO_S));
            PDT_PARAM_DebugDevInfo((HI_UPGRADE_DEV_INFO_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_VOLUME:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SysVolume;
            MLOGD("Volume[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ScreenBrightness;
            MLOGD("ScreenBrightness[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stScreenDormantAttr, sizeof(HI_TIMEDTASK_ATTR_S));
            PDT_PARAM_DebugTimedTask("ScreenDormantMode", (HI_TIMEDTASK_ATTR_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stSystemDormantAttr, sizeof(HI_TIMEDTASK_ATTR_S));
            PDT_PARAM_DebugTimedTask("SystemDormantMode", (HI_TIMEDTASK_ATTR_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
            *(HI_BOOL *)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bBootSoundEnable;
            MLOGD("BootSound Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
         case HI_PDT_PARAM_TYPE_LANGUAGE:
            *(HI_S32 *)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SystemLanguage;
            MLOGD("SystemLanguage[%d]\n\n", *(HI_S32 *)pvParam);
            break;
         case HI_PDT_PARAM_TYPE_KEYTONE:
            *(HI_BOOL *)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bKeyToneEnable;
            MLOGD("KeyTone Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            *(HI_USB_MODE_E*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enUsbMode;
            MLOGD("UsbMode[%d], 0:Charge, 1:UVC, 2:Storage\n", *(HI_USB_MODE_E*)pvParam);
            break;
#ifdef CONFIG_MOTIONSENSOR
        case HI_PDT_PARAM_TYPE_MOTIONSENSOR_INFO:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMotionSensorInfo, sizeof(HI_MAPI_MOTIONSENSOR_INFO_S));

            HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionSensorInfo = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMotionSensorInfo;
            if(pstMotionSensorInfo->stTempDrift.enMode == IMU_TEMP_DRIFT_LUT)
            {
                MLOGD("get TempDrift:\n");
                MLOGD(" as32IMULut[%d][%d]:\n", TEMP_LUT_SAMPLES, AXIS_NUM);
                HI_S32 i = 0, j = 0;
                for(i = 0; i < TEMP_LUT_SAMPLES; i++)
                {
                    for(j = 0; j < AXIS_NUM; j++)
                    {
                        MLOGD("   as32IMULut[%d][%d]:%d\n",
                            i, j, pstMotionSensorInfo->stTempDrift.stTempLut.as32IMULut[i][j]);
                    }
                }
                for(i = 0; i < TEMP_LUT_SAMPLES; i++)
                {
                    for(j = 0; j < MFUSION_LUT_STATUS_NUM; j++)
                    {
                        MLOGD("    as32GyroLutStatus[%d][%d] = %d\n",
                            i, j, pstMotionSensorInfo->stTempDrift.stTempLut.as32GyroLutStatus[i][j]);
                    }
                }

                MLOGD(" s32RangeMin: %d\n",
                    pstMotionSensorInfo->stTempDrift.stTempLut.s32RangeMin);
                MLOGD(" s32RangeMax: %d\n",
                    pstMotionSensorInfo->stTempDrift.stTempLut.s32RangeMax);
                MLOGD(" u32Step: %u\n",
                    pstMotionSensorInfo->stTempDrift.stTempLut.u32Step);
            }

            break;
#endif
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport Common ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetMediaMode(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_MEDIAMODE_E enMediaMode)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stNormRecAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSlowRecAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stSingPhotoAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode = enMediaMode;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[MediaMode]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] MediaMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetMediaModeStr(enMediaMode));
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetPhotoScene(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PHOTO_SCENE_E enScene)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enScene = enScene;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[PhotoScene]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] PhotoScene[%d: 0-NORM,1-HDR,2-LL]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), enScene);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetPhotoOutputFmt(HI_PDT_WORKMODE_E enWorkMode, HI_PHOTOMNG_OUTPUT_FORMAT_E enFormat)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoProcAlg.enOutputFmt = enFormat;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[PhotoOutputFormat]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] Photo OutputFormat[%d: 0-JPEG,1-DNG,2-DNG+JPEG]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), enFormat);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetDelayTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stDelayPhotoAttr.u32DelayTime_s = u32Param;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[DelayTime]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] DelayTime[%uS]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetLapseInterval(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LPSE_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpseRecAttr.u32Interval_ms = u32Param;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32Interval_ms = u32Param;
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecSnapAttr.u32SnapInterval_ms = u32Param;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[LapseInterval]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] LapseInterval[%ums]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetBurstType(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_JPG_BURST_TYPE_E enParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_BURST:
        {
            HI_S32 s32Idx = 0;
            HI_S32 i;
            for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
            {
                if (PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode
                    == PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.stMediaModeValues.astValues[i].s32Value)
                {
                    s32Idx = i;
                    break;
                }
            }
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.aenJpgBurstType[s32Idx] = enParam;
            break;
        }
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[BurstType]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] MediaMode[%s] BurstType[%d 0-3_1,1-5_1,2-10_1,3-15_1,4-30_1,5-20_2,6-30_2,7-30_3,]\n\n",
                PDT_PARAM_GetWorkModeStr(enWorkMode),
                PDT_PARAM_GetMediaModeStr(PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode),
                enParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetLoopTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LOOP_REC:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLoopRecAttr.u32LoopTime_min = u32Param;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[LoopTime]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] LoopTime[%umin]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetEncPayloadType(HI_PDT_WORKMODE_E enWorkMode, HI_MAPI_PAYLOAD_TYPE_E enParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMainVideoVencAttr.enType = enParam;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[EncPayloadType]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] PayloadType[%d 0-H264,1-H265,2-MJPEG,3-JPEG]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), enParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetEV(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_SCENE_EV_E enParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.enEV = enParam;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.enEV = enParam;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[EV]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] EV[%d 0-B2,1-B1_5,2-B1,3-B0_5,4-0,5-0_5,6-1,7-1_5,8-2]\n\n",
                PDT_PARAM_GetWorkModeStr(enWorkMode), enParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetExpTime(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32ExpTime_us = u32Param;
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32ExpTime_us = u32Param;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[ExpTime]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] ExpTime[%ums]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetISO(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.u32ISO = u32Param;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32ISO = u32Param;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[ISO]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] ISO[%u]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetWB(HI_PDT_WORKMODE_E enWorkMode, HI_U32 u32Param)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.u32WB = u32Param;;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.u32WB = u32Param;;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[WB]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] WB[%u]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), u32Param);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetMetry(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_SCENE_METRY_TYPE_E enParam)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stVideoProTune.enMetryType = enParam;
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.stPhotoProTune.enMetryType = enParam;
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[MetryType]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] Metry[%d 0-CENTER,1-AVERAGE,2-SPOT]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), enParam);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_SetWorkModeParam(HI_PDT_WORKMODE_E enWorkMode,
                HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_PARAM_SetMediaMode(enWorkMode, *(HI_PDT_MEDIAMODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
            s32Ret = PDT_PARAM_SetPhotoScene(enWorkMode, *(HI_PDT_PHOTO_SCENE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
            s32Ret = PDT_PARAM_SetPhotoOutputFmt(enWorkMode, *(HI_PHOTOMNG_OUTPUT_FORMAT_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
            s32Ret = PDT_PARAM_SetDelayTime(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_PARAM_SetLapseInterval(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
            s32Ret = PDT_PARAM_SetBurstType(enWorkMode, *(HI_PDT_JPG_BURST_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
            s32Ret = PDT_PARAM_SetLoopTime(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_PARAM_SetEncPayloadType(enWorkMode, *(HI_MAPI_PAYLOAD_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_PARAM_SetEV(enWorkMode, *(HI_PDT_SCENE_EV_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            s32Ret = PDT_PARAM_SetExpTime(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_PARAM_SetISO(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_PARAM_SetWB(enWorkMode, *(HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_PARAM_SetMetry(enWorkMode, *(HI_PDT_SCENE_METRY_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stOsdCfg.stTimeOsd.bShow = *(HI_BOOL*)pvParam;
            MLOGD("WorkMode[%s] OSD Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
        {
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bAudio = *(HI_BOOL*)pvParam;
            MLOGD("WorkMode[%s] Audio Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_FLIP:
        {
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bFlip = *(HI_BOOL*)pvParam;
            MLOGD("WorkMode[%s] FLIP Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_DIS:
        {
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bDISEnable = *(HI_BOOL*)pvParam;
            MLOGD("WorkMode[%s] DIS Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_LDC:
        {
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.bLDCEnable = *(HI_BOOL*)pvParam;
            MLOGD("WorkMode[%s] LDC Enable[%d]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), *(HI_BOOL*)pvParam);
            break;
        }
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport WorkMode[%s] With ParamType[%s]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
#ifndef __HuaweiLite__
    PDT_PARAM_SetSaveFlg();
#endif
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronWorkMode = *(HI_PDT_WORKMODE_E*)pvParam;
            MLOGD("PoweronWorkMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(*(HI_PDT_WORKMODE_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronAction = *(HI_PDT_POWERON_ACTION_E*)pvParam;
            MLOGD("PoweronAction[%s]\n\n", PDT_PARAM_GetPoweronActionStr(*(HI_PDT_POWERON_ACTION_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_WIFI_AP:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiApCfg, pvParam, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
            PDT_PARAM_DebugWiFiAPCfg((HI_HAL_WIFI_APMODE_CFG_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_VOLUME:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SysVolume = *(HI_S32*)pvParam;
            MLOGD("Volume[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ScreenBrightness = *(HI_S32*)pvParam;
            MLOGD("ScreenBrightness[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_KEYTONE:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bKeyToneEnable = *(HI_BOOL *)pvParam;
            MLOGD("bKeyToneEnable[%d]\n\n", *(HI_BOOL *)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stScreenDormantAttr, pvParam, sizeof(HI_TIMEDTASK_ATTR_S));
            PDT_PARAM_DebugTimedTask("ScreenDormantMode", (HI_TIMEDTASK_ATTR_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stSystemDormantAttr, pvParam, sizeof(HI_TIMEDTASK_ATTR_S));
            PDT_PARAM_DebugTimedTask("SystemDormantMode", (HI_TIMEDTASK_ATTR_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enUsbMode = *(HI_USB_MODE_E*)pvParam;
            MLOGD("UsbMode[%d], 0:Charge, 1:UVC, 2:Storage\n", *(HI_USB_MODE_E*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SystemLanguage = *(HI_S32 *)pvParam;
            MLOGD("SystemLanguage[%d]\n\n", *(HI_S32 *)pvParam);
            break;
#ifdef CONFIG_MOTIONSENSOR
        case HI_PDT_PARAM_TYPE_MOTIONSENSOR_INFO:
            {
                MFUSION_TEMP_DRIFT_S *pstTempDrift =
                    &(((HI_MAPI_MOTIONSENSOR_INFO_S *)pvParam)->stTempDrift);
                memcpy(&PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMotionSensorInfo.stTempDrift, pstTempDrift, sizeof(MFUSION_TEMP_DRIFT_S));

                HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionSensorInfo = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.stMotionSensorInfo;
                if(pstMotionSensorInfo->stTempDrift.enMode == IMU_TEMP_DRIFT_LUT)
                {
                    MLOGD("set TempDrift:\n");
                    MLOGD(" as32IMULut[%d][%d]:\n", TEMP_LUT_SAMPLES, AXIS_NUM);
                    HI_S32 i = 0, j = 0;
                    for(i = 0; i < TEMP_LUT_SAMPLES; i++)
                    {
                        for(j = 0; j < AXIS_NUM; j++)
                        {
                            MLOGD("   as32IMULut[%d][%d]:%d\n",
                                i, j, pstMotionSensorInfo->stTempDrift.stTempLut.as32IMULut[i][j]);
                        }
                    }
                    for(i = 0; i < TEMP_LUT_SAMPLES; i++)
                    {
                        for(j = 0; j < MFUSION_LUT_STATUS_NUM; j++)
                        {
                            MLOGD("    as32GyroLutStatus[%d][%d] = %d\n",
                                i, j, pstMotionSensorInfo->stTempDrift.stTempLut.as32GyroLutStatus[i][j]);
                        }
                    }
                    MLOGD(" s32RangeMin: %d\n",
                        pstMotionSensorInfo->stTempDrift.stTempLut.s32RangeMin);
                    MLOGD(" s32RangeMax: %d\n",
                        pstMotionSensorInfo->stTempDrift.stTempLut.s32RangeMax);
                    MLOGD(" u32Step: %u\n",
                        pstMotionSensorInfo->stTempDrift.stTempLut.u32Step);
                }
            }
            break;
#endif
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport Common ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
#ifndef __HuaweiLite__
    PDT_PARAM_SetSaveFlg();

    /* publish event */
    HI_EVENT_S stEvent;
    stEvent.EventID = HI_EVENT_PDT_PARAM_CHANGE;
    stEvent.arg1 = enType;
    HI_EVTHUB_Publish(&stEvent);
#endif
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMediaModeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stNormRec.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLoopRec.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSlowRec.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLpseRec.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stRecSnap.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSingPhoto.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLpsePhoto.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_BURST:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.stMediaModeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[MediaMode]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetPhotoSceneValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSingPhoto.stSceneValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[PhotoScene]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetDelayTimeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSingPhoto.stDelayTimeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[DelayTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetOutputFormatValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSingPhoto.stOutputFormatValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[PhotoOutputFormat]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetLapseIntervalValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LPSE_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLpseRec.stLapseIntervalValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stRecSnap.stLapseIntervalValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLpsePhoto.stLapseIntervalValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[LapseInterval]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetBurstTypeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_BURST:
        {
            HI_S32 s32Idx = 0;
            HI_S32 i;
            for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
            {
                if (PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stBurstAttr.stCommAttr.enMediaMode
                    == PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.stMediaModeValues.astValues[i].s32Value)
                {
                    s32Idx = i;
                    break;
                }
            }
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stBurst.astBurstTypeValues[s32Idx], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        }
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[BurstType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetLoopTimeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_LOOP_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLoopRec.stLoopTimeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[LoopTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetEncPayloadTypeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVideoPayloadType, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[EncPayloadType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetEVValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVideoProtune.stEVValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPhotoProtune.stEVValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[EV]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetExpTimeValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPhotoProtune.stExpTimeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
        {
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLpsePhoto.stExpTimeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            HI_S32 i;
            HI_S32 s32LpseInterval_ms = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stLpsePhotoAttr.u32Interval_ms;
            for (i = 0; i < pstValueSet->s32Cnt; ++i)
            {
                if ((0 != pstValueSet->astValues[i].s32Value) &&
                    (s32LpseInterval_ms < pstValueSet->astValues[i].s32Value / 1000 + 1000))
                {
                    pstValueSet->s32Cnt = i;
                    break;
                }
            }
            break;
        }
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[ExpTime]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetISOValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVideoProtune.stISOValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPhotoProtune.stISOValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[ISO]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetWBValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVideoProtune.stWBValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPhotoProtune.stWBValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[WB]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMetryValues(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_BURST:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVideoProtune.stMetryTypeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPhotoProtune.stMetryTypeValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[MetryType]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetWorkModeItemValues(HI_PDT_WORKMODE_E enWorkMode,
                HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));

    HI_S32 s32Ret;
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_PARAM_GetMediaModeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
            s32Ret = PDT_PARAM_GetPhotoSceneValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
            s32Ret = PDT_PARAM_GetDelayTimeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
            s32Ret = PDT_PARAM_GetOutputFormatValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_PARAM_GetLapseIntervalValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
            s32Ret = PDT_PARAM_GetBurstTypeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
            s32Ret = PDT_PARAM_GetLoopTimeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_PARAM_GetEncPayloadTypeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_PARAM_GetEVValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            s32Ret = PDT_PARAM_GetExpTimeValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_PARAM_GetISOValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_PARAM_GetWBValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_PARAM_GetMetryValues(enWorkMode, pstValueSet);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[%s]\n",
                PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("WorkMode[%s] ParamType[%s]\n", PDT_PARAM_GetWorkModeStr(enWorkMode), PDT_PARAM_GetParamTypeStr(enType));
    PDT_PARAM_DebugItemValues(pstValueSet);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetCommItemValues(HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));

    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_ACTION:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stPowerOnActionValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stDevMng.stSysScreemDormantValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stDevMng.stSysScreemDormantValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stUsbMode, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stDevMng.stSystemLanguage, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport Common ParamType[%s]\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_FAILURE;
    }
    MLOGD("ParamType[%s]\n", PDT_PARAM_GetParamTypeStr(enType));
    PDT_PARAM_DebugItemValues(pstValueSet);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

