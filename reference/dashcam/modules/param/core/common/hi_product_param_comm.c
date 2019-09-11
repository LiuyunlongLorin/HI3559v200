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
#ifdef AMP_HUAWEILITE
#include "lwip/netifapi.h"
#endif
#include "hi_product_param_inner.h"
#include "hi_product_param.h"
#include "hi_product_param_debug.h"
#include "hi_eventhub.h"

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

#define PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(ret, errcode,errstring) \
    do { \
        if (HI_SUCCESS != ret){   \
            MLOGE(RED" %s failed,ret[%08x]"NONE"\n\n",errstring, ret); \
            return errcode;  \
        }   \
    }while(0)

static HI_S32 PDT_PARAM_GetVcapPipeChnCfg(PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_CHN_S *pstVcapPipeChnCommCfg,
    PDT_PARAM_MEDIAMODE_VCAP_PIPE_CHN_ATTR_S *pstVcapPipeChnSpecAttr,
    HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S *pstVcapPipeChnAttr)
{
    pstVcapPipeChnAttr->bEnable = pstVcapPipeChnCommCfg->bEnable;
    pstVcapPipeChnAttr->PipeChnHdl = pstVcapPipeChnCommCfg->PipeChnHdl;
    pstVcapPipeChnAttr->enPixelFormat = pstVcapPipeChnCommCfg->enPixelFormat;
    pstVcapPipeChnAttr->enRotate = pstVcapPipeChnCommCfg->enRotate;
    pstVcapPipeChnAttr->bFlip = pstVcapPipeChnCommCfg->bFlip;
    pstVcapPipeChnAttr->bMirror = pstVcapPipeChnCommCfg->bMirror;
    pstVcapPipeChnAttr->s32Brightness = pstVcapPipeChnCommCfg->s32Brightness;
    pstVcapPipeChnAttr->s32Saturation = pstVcapPipeChnCommCfg->s32Saturation;
    memcpy(&pstVcapPipeChnAttr->stLDCAttr,&pstVcapPipeChnCommCfg->stLDCAttr,sizeof(VI_LDC_ATTR_S));

    MLOGD(YELLOW"--------------------s32Brightness[%d], s32Saturation[%d] ,bFlip[%d] bMirror[%d] \n"NONE
        ,pstVcapPipeChnCommCfg->s32Brightness,pstVcapPipeChnCommCfg->s32Saturation,
        pstVcapPipeChnCommCfg->bFlip,pstVcapPipeChnCommCfg->bMirror);
    pstVcapPipeChnAttr->stFrameRate.s32SrcFrameRate = pstVcapPipeChnSpecAttr->stFrameRate.s32SrcFrameRate;
    pstVcapPipeChnAttr->stFrameRate.s32DstFrameRate = pstVcapPipeChnSpecAttr->stFrameRate.s32DstFrameRate;
    memcpy(&pstVcapPipeChnAttr->stDestResolution,&pstVcapPipeChnSpecAttr->stDestResolution,sizeof(SIZE_S));

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVcapPipeCfg(PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_S *pstVcapPipeCommCfg,
    PDT_PARAM_MEDIAMODE_VCAP_PIPE_ATTR_S *pstVcapPipeSpecAttr,
    HI_PDT_MEDIA_VCAP_PIPE_ATTR_S *pstVcapPipeAttr)
{
    HI_S32 s32Idx = 0;

    pstVcapPipeAttr->bEnable = pstVcapPipeCommCfg->bEnable;
    pstVcapPipeAttr->VcapPipeHdl = pstVcapPipeCommCfg->VcapPipeHdl;
    pstVcapPipeAttr->enPipeType = pstVcapPipeCommCfg->enPipeType;

    pstVcapPipeAttr->stFrameRate.s32SrcFrameRate = pstVcapPipeSpecAttr->stFrameRate.s32SrcFrameRate;
    pstVcapPipeAttr->stFrameRate.s32DstFrameRate = pstVcapPipeSpecAttr->stFrameRate.s32DstFrameRate;
    pstVcapPipeAttr->bIspBypass = pstVcapPipeSpecAttr->bIspBypass;
    pstVcapPipeAttr->stFrameIntAttr.enIntType = pstVcapPipeSpecAttr->stFrameInterruptAttr.enIntType;
    pstVcapPipeAttr->stFrameIntAttr.u32EarlyLine = pstVcapPipeSpecAttr->stFrameInterruptAttr.u32EarlyLine;
    MLOGD("Enable[%d], VcapPipeHdl[%d] ,PipeType[%d] IspBypass[%d] \n"
        ,pstVcapPipeCommCfg->bEnable,pstVcapPipeCommCfg->VcapPipeHdl,pstVcapPipeCommCfg->enPipeType,pstVcapPipeSpecAttr->bIspBypass);
    memcpy(&pstVcapPipeAttr->stIspPubAttr,&pstVcapPipeSpecAttr->stIspPubAttr,sizeof(HI_MAPI_PIPE_ISP_ATTR_S));
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT;s32Idx++)
    {
        PDT_PARAM_GetVcapPipeChnCfg(&pstVcapPipeCommCfg->astPipeChnAttr[s32Idx],&pstVcapPipeSpecAttr->astPipeChnAttr[s32Idx],
            &pstVcapPipeAttr->astPipeChnAttr[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVcapDevCfg(PDT_PARAM_MEDIAMODE_COMM_VCAP_DEV_S *pstVcapCommCfg,
    PDT_PARAM_MEDIAMODE_VCAP_DEV_ATTR_S *pstVcapSpecAttr,
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstVcapDevAttr,
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S *pstSceneAttr)
{
    HI_S32 s32Idx = 0;

    pstVcapDevAttr->bEnable = pstVcapCommCfg->bEnable;
    pstVcapDevAttr->VcapDevHdl = pstVcapCommCfg->VcapDevHdl;
    memcpy(&pstVcapDevAttr->stSnsAttr,&pstVcapSpecAttr->stSnsAttr,sizeof(HI_MEDIA_SENSOR_ATTR_S));
    memcpy(&pstVcapDevAttr->stResolution,&pstVcapSpecAttr->stVcapDevAttr.stBasSize,sizeof(SIZE_S));
    pstVcapDevAttr->enWdrMode = pstVcapCommCfg->enWdrMode;
    pstVcapDevAttr->u32VideoMode = pstVcapCommCfg->u32VideoMode;

    pstSceneAttr->enPipeMode = HI_PDT_SCENE_PIPE_MODE_LINEAR;
    if((pstVcapDevAttr->enWdrMode >= WDR_MODE_2To1_LINE) && (pstVcapDevAttr->enWdrMode <= WDR_MODE_4To1_FRAME_FULL_RATE))
    {
        pstSceneAttr->enPipeMode = HI_PDT_SCENE_PIPE_MODE_WDR;
    }
    pstSceneAttr->bIspBypass = pstVcapSpecAttr->astVcapPipeAttr[0].bIspBypass;
    pstSceneAttr->MainPipeHdl = pstVcapCommCfg->astVcapPipeAttr[0].VcapPipeHdl;
    pstSceneAttr->VcapPipeHdl = pstVcapCommCfg->astVcapPipeAttr[0].VcapPipeHdl;
    pstSceneAttr->PipeChnHdl = pstVcapCommCfg->astVcapPipeAttr[0].astPipeChnAttr[0].PipeChnHdl;
    pstSceneAttr->u8PipeParamIndex = (HI_U8)pstVcapSpecAttr->astVcapPipeAttr[0].u32SceneParamLinearIdx;
    if(HI_PDT_SCENE_PIPE_MODE_WDR == pstSceneAttr->enPipeMode)
        pstSceneAttr->u8PipeParamIndex = (HI_U8)pstVcapSpecAttr->astVcapPipeAttr[0].u32SceneParamWdrIdx;
    pstSceneAttr->enPipeType = ((HI_MAPI_PIPE_TYPE_VIDEO == pstVcapCommCfg->astVcapPipeAttr[0].enPipeType) ? HI_PDT_SCENE_PIPE_TYPE_VIDEO : HI_PDT_SCENE_PIPE_TYPE_SNAP);

    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;s32Idx++)
    {
        MLOGD("pipe index [%d] \n",s32Idx);
        PDT_PARAM_GetVcapPipeCfg(&pstVcapCommCfg->astVcapPipeAttr[s32Idx],&pstVcapSpecAttr->astVcapPipeAttr[s32Idx],
            &pstVcapDevAttr->astVcapPipeAttr[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVportAttr(PDT_PARAM_MEDIAMODE_COMM_VPSS_PORT_ATTR_S *pstVportCommAttr,
    PDT_PARAM_MEDIAMODE_VPSS_PORT_ATTR_S *pstVportSpecAttr,
    HI_PDT_MEDIA_VPSS_PORT_ATTR_S *pstVportAttr)
{
    pstVportAttr->bEnable = pstVportCommAttr->bEnable;
    pstVportAttr->VportHdl = pstVportCommAttr->VportHdl;
    pstVportAttr->enRotate = pstVportCommAttr->enRotate;
    pstVportAttr->bFlip = pstVportCommAttr->bFlip;
    pstVportAttr->bMirror = pstVportCommAttr->bMirror;
    pstVportAttr->enVideoFormat = pstVportCommAttr->enVideoFormat;
    pstVportAttr->enPixelFormat = pstVportCommAttr->enPixelFormat;
    pstVportAttr->bSupportBufferShare = pstVportCommAttr->bSupportBufferShare;

    pstVportAttr->stResolution.u32Width = pstVportSpecAttr->stResolution.u32Width;
    pstVportAttr->stResolution.u32Height = pstVportSpecAttr->stResolution.u32Height;
    pstVportAttr->stFrameRate.s32SrcFrameRate = pstVportSpecAttr->stFrameRate.s32SrcFrameRate;
    pstVportAttr->stFrameRate.s32DstFrameRate = pstVportSpecAttr->stFrameRate.s32DstFrameRate;

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVpssAttr(PDT_PARAM_MEDIAMODE_COMM_VPSS_ATTR_S *pstVpssCommAttr,
    PDT_PARAM_MEDIAMODE_VPSS_ATTR_S *pstVpssSpecAttr,
    HI_PDT_MEDIA_VPSS_ATTR_S *pstVpssAttr)
{
    HI_S32 s32Idx = 0;

    pstVpssAttr->bEnable = pstVpssCommAttr->bEnable;
    pstVpssAttr->VpssHdl = pstVpssCommAttr->VpssHdl;
    pstVpssAttr->VcapPipeHdl = pstVpssCommAttr->VcapPipeHdl;
    pstVpssAttr->VcapPipeChnHdl = pstVpssCommAttr->PipeChnHdl;
    pstVpssAttr->stVpssAttr.enPixelFormat = pstVpssCommAttr->enPixelFormat;
    pstVpssAttr->stVpssAttr.bNrEn = pstVpssCommAttr->bNrEn;
    pstVpssAttr->stVpssAttr.stNrAttr.enCompressMode = pstVpssCommAttr->stNrAttr.enCompressMode;
    pstVpssAttr->stVpssAttr.stNrAttr.enNrMotionMode= pstVpssCommAttr->stNrAttr.enNrMotionMode;

    pstVpssAttr->stVpssAttr.u32MaxW = pstVpssSpecAttr->u32MaxW;
    pstVpssAttr->stVpssAttr.u32MaxH = pstVpssSpecAttr->u32MaxH;
    pstVpssAttr->stVpssAttr.stFrameRate.s32SrcFrameRate = pstVpssSpecAttr->stFrameRate.s32SrcFrameRate;
    pstVpssAttr->stVpssAttr.stFrameRate.s32DstFrameRate = pstVpssSpecAttr->stFrameRate.s32DstFrameRate;

    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;s32Idx++)
    {
        PDT_PARAM_GetVportAttr(&pstVpssCommAttr->astPortAttr[s32Idx],&pstVpssSpecAttr->astVportAttr[s32Idx],
            &pstVpssAttr->astVportAttr[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVideoVencCfg(PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstMediaVencComm,
    PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S *pstVencCommAttr,
    PDT_PARAM_MEDIAMODE_VENC_ATTR_S *pstVencSpecAttr,
    HI_MEDIA_VENC_ATTR_S *pstVencAttr)
{
    pstVencAttr->stTypeAttr.enType = pstVencCommAttr->enPayload;
    pstVencAttr->stRcAttr.enRcMode = pstVencCommAttr->enRcMode;
    pstVencAttr->stTypeAttr.enSceneMode = pstMediaVencComm->enSceneMode;
    pstVencAttr->stTypeAttr.u32Width = pstVencSpecAttr->u32Width;
    pstVencAttr->stTypeAttr.u32Height = pstVencSpecAttr->u32Height;
    pstVencAttr->stTypeAttr.u32BufSize = pstVencSpecAttr->u32BufSize;

    if(HI_MAPI_PAYLOAD_TYPE_H264 == pstVencCommAttr->enPayload)
    {
        pstVencAttr->stTypeAttr.u32Profile = pstMediaVencComm->u32H264Profile;
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_CBR_S *pstH264Cbr = &pstVencAttr->stRcAttr.unAttr.stH264Cbr;
            pstH264Cbr->u32MaxQp = pstMediaVencComm->stH264Cbr.u32MaxQp;
            pstH264Cbr->u32MinQp = pstMediaVencComm->stH264Cbr.u32MinQp;
            pstH264Cbr->u32MaxIQp = pstMediaVencComm->stH264Cbr.u32MaxIQp;
            pstH264Cbr->u32MinIQp = pstMediaVencComm->stH264Cbr.u32MinIQp;

            pstH264Cbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264Cbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264Cbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264Cbr->stAttr.u32BitRate = pstVencSpecAttr->u32H264_BitRate;
            pstH264Cbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_VBR_S *pstH264Vbr = &pstVencAttr->stRcAttr.unAttr.stH264Vbr;
            pstH264Vbr->u32MaxQp = pstMediaVencComm->stH264Vbr.u32MaxQp;
            pstH264Vbr->u32MinQp = pstMediaVencComm->stH264Vbr.u32MinQp;
            pstH264Vbr->u32MaxIQp = pstMediaVencComm->stH264Vbr.u32MaxIQp;
            pstH264Vbr->u32MinIQp = pstMediaVencComm->stH264Vbr.u32MinIQp;

            pstH264Vbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264Vbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264Vbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264Vbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H264_BitRate;
            pstH264Vbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else if(HI_MAPI_VENC_RC_MODE_QVBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_QVBR_S *pstH264QVbr = &pstVencAttr->stRcAttr.unAttr.stH264QVbr;
            pstH264QVbr->u32MaxQp = pstMediaVencComm->stH264QVbr.u32MaxQp;
            pstH264QVbr->u32MinQp = pstMediaVencComm->stH264QVbr.u32MinQp;
            pstH264QVbr->u32MaxIQp = pstMediaVencComm->stH264QVbr.u32MaxIQp;
            pstH264QVbr->u32MinIQp = pstMediaVencComm->stH264QVbr.u32MinIQp;
            pstH264QVbr->s32BitPercentUL = pstMediaVencComm->stH264QVbr.s32BitPercentUL;
            pstH264QVbr->s32BitPercentLL = pstMediaVencComm->stH264QVbr.s32BitPercentLL;
            pstH264QVbr->s32PsnrFluctuateUL = pstMediaVencComm->stH264QVbr.s32PsnrFluctuateUL;
            pstH264QVbr->s32PsnrFluctuateLL = pstMediaVencComm->stH264QVbr.s32PsnrFluctuateLL;

            pstH264QVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264QVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264QVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264QVbr->stAttr.u32TargetBitRate = pstVencSpecAttr->u32H264_BitRate;
            pstH264QVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }

    }
    else if(HI_MAPI_PAYLOAD_TYPE_H265 == pstVencCommAttr->enPayload)
    {
        pstVencAttr->stTypeAttr.u32Profile = pstMediaVencComm->u32H265Profile;
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_CBR_S *pstH265Cbr = &pstVencAttr->stRcAttr.unAttr.stH265Cbr;
            pstH265Cbr->u32MaxQp = pstMediaVencComm->stH265Cbr.u32MaxQp;
            pstH265Cbr->u32MinQp = pstMediaVencComm->stH265Cbr.u32MinQp;
            pstH265Cbr->u32MaxIQp = pstMediaVencComm->stH265Cbr.u32MaxIQp;
            pstH265Cbr->u32MinIQp = pstMediaVencComm->stH265Cbr.u32MinIQp;

            pstH265Cbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265Cbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265Cbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH265Cbr->stAttr.u32BitRate = pstVencSpecAttr->u32H265_BitRate;
            pstH265Cbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_VBR_S *pstH265Vbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
            pstH265Vbr->u32MaxQp = pstMediaVencComm->stH265Vbr.u32MaxQp;
            pstH265Vbr->u32MinQp = pstMediaVencComm->stH265Vbr.u32MinQp;
            pstH265Vbr->u32MaxIQp = pstMediaVencComm->stH265Vbr.u32MaxIQp;
            pstH265Vbr->u32MinIQp = pstMediaVencComm->stH265Vbr.u32MinIQp;

            pstH265Vbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265Vbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265Vbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH265Vbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H265_BitRate;
            pstH265Vbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else if(HI_MAPI_VENC_RC_MODE_QVBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_QVBR_S *pstH265QVbr = &pstVencAttr->stRcAttr.unAttr.stH265QVbr;
            pstH265QVbr->u32MaxQp = pstMediaVencComm->stH265QVbr.u32MaxQp;
            pstH265QVbr->u32MinQp = pstMediaVencComm->stH265QVbr.u32MinQp;
            pstH265QVbr->u32MaxIQp = pstMediaVencComm->stH265QVbr.u32MaxIQp;
            pstH265QVbr->u32MinIQp = pstMediaVencComm->stH265QVbr.u32MinIQp;
            pstH265QVbr->s32BitPercentUL = pstMediaVencComm->stH265QVbr.s32BitPercentUL;
            pstH265QVbr->s32BitPercentLL = pstMediaVencComm->stH265QVbr.s32BitPercentLL;
            pstH265QVbr->s32PsnrFluctuateUL = pstMediaVencComm->stH265QVbr.s32PsnrFluctuateUL;
            pstH265QVbr->s32PsnrFluctuateLL = pstMediaVencComm->stH265QVbr.s32PsnrFluctuateLL;

            pstH265QVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265QVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265QVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH265QVbr->stAttr.u32TargetBitRate = pstVencSpecAttr->u32H265_BitRate;
            pstH265QVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }

    }
    else if(HI_MAPI_PAYLOAD_TYPE_MJPEG == pstVencCommAttr->enPayload)
    {
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_CBR_S *pstMjpegCbr =  &pstVencAttr->stRcAttr.unAttr.stMjpegCbr;
            pstMjpegCbr->u32MaxQfactor = pstMediaVencComm->stMjpegCbr.u32MaxQfactor;
            pstMjpegCbr->u32MinQfactor = pstMediaVencComm->stMjpegCbr.u32MinQfactor;

            pstMjpegCbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstMjpegCbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstMjpegCbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstMjpegCbr->stAttr.u32BitRate= pstVencSpecAttr->u32H264_BitRate;
            pstMjpegCbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_VBR_S *pstMjpegVbr =  &pstVencAttr->stRcAttr.unAttr.stMjpegVbr;

            pstMjpegVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstMjpegVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstMjpegVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstMjpegVbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H264_BitRate;
            pstMjpegVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }
    }
    else
    {
        MLOGD("unsupport payload type [%d] \n",pstVencCommAttr->enPayload);
        return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetSnapVencCfg(HI_MAPI_VENC_ATTR_JPEG_S *pstMediaVencComm,
    PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S *pstVencCommAttr,
    PDT_PARAM_MEDIAMODE_VENC_ATTR_S *pstVencSpecAttr,
    HI_MEDIA_VENC_ATTR_S *pstVencAttr)
{
    pstVencAttr->stTypeAttr.enType = pstVencCommAttr->enPayload;
    pstVencAttr->stTypeAttr.enSceneMode = HI_MAPI_VENC_SCENE_MODE_CAR;
    pstVencAttr->stRcAttr.enRcMode = pstVencCommAttr->enRcMode;
    pstVencAttr->stTypeAttr.u32Width = pstVencSpecAttr->u32Width;
    pstVencAttr->stTypeAttr.u32Height = pstVencSpecAttr->u32Height;
    pstVencAttr->stTypeAttr.u32BufSize = pstVencSpecAttr->u32BufSize;
    pstVencAttr->stTypeAttr.u32Profile = 0;

    if(HI_MAPI_PAYLOAD_TYPE_JPEG == pstVencCommAttr->enPayload)
    {
        HI_MAPI_VENC_ATTR_JPEG_S *pstJpeg = &pstVencAttr->stTypeAttr.stAttrJpege;

        pstJpeg->bEnableDCF = pstMediaVencComm->bEnableDCF;
        pstJpeg->enJpegEncodeMode = pstMediaVencComm->enJpegEncodeMode;
        pstJpeg->u32Qfactor = pstMediaVencComm->u32Qfactor;
        pstJpeg->stAttrMPF = pstMediaVencComm->stAttrMPF;
    }
    else
    {
        MLOGD("unsupport payload type [%d] \n",pstVencCommAttr->enPayload);
        return HI_PDT_PARAM_EUNSUPPORT;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVencCfgByPayloadType(HI_MAPI_PAYLOAD_TYPE_E enPayloadType,
    PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstMediaVencComm,
    PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S *pstVencCommAttr,
    PDT_PARAM_MEDIAMODE_VENC_ATTR_S *pstVencSpecAttr,
    HI_PDT_MEDIA_VENC_CFG_S *pstVencCfg)
{
    pstVencCfg->bEnable = pstVencCommAttr->bEnable;
    pstVencCfg->VencHdl = pstVencCommAttr->VencHdl;
    pstVencCfg->enBindedMod = pstVencCommAttr->enBindedMod;
    pstVencCfg->ModHdl = pstVencCommAttr->ModHdl;
    pstVencCfg->ChnHdl = pstVencCommAttr->ChnHdl;
    HI_MEDIA_VENC_ATTR_S *pstVencAttr = &pstVencCfg->stVencAttr;
    pstVencAttr->stTypeAttr.enType = enPayloadType;
    pstVencAttr->stRcAttr.enRcMode = pstVencCommAttr->enRcMode;
    pstVencAttr->stTypeAttr.enSceneMode = pstMediaVencComm->enSceneMode;
    pstVencAttr->stTypeAttr.u32Width = pstVencSpecAttr->u32Width;
    pstVencAttr->stTypeAttr.u32Height = pstVencSpecAttr->u32Height;
    pstVencAttr->stTypeAttr.u32BufSize = pstVencSpecAttr->u32BufSize;

    if(HI_MAPI_PAYLOAD_TYPE_H264 == enPayloadType)
    {
        pstVencAttr->stTypeAttr.u32Profile = pstMediaVencComm->u32H264Profile;
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_CBR_S *pstH264Cbr = &pstVencAttr->stRcAttr.unAttr.stH264Cbr;
            pstH264Cbr->u32MaxQp = pstMediaVencComm->stH264Cbr.u32MaxQp;
            pstH264Cbr->u32MinQp = pstMediaVencComm->stH264Cbr.u32MinQp;
            pstH264Cbr->u32MaxIQp = pstMediaVencComm->stH264Cbr.u32MaxIQp;
            pstH264Cbr->u32MinIQp = pstMediaVencComm->stH264Cbr.u32MinIQp;

            pstH264Cbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264Cbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstH264Cbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264Cbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264Cbr->stAttr.u32BitRate = pstVencSpecAttr->u32H264_BitRate;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_VBR_S *pstH264Vbr = &pstVencAttr->stRcAttr.unAttr.stH264Vbr;
            pstH264Vbr->u32MaxQp = pstMediaVencComm->stH264Vbr.u32MaxQp;
            pstH264Vbr->u32MinQp = pstMediaVencComm->stH264Vbr.u32MinQp;
            pstH264Vbr->u32MaxIQp = pstMediaVencComm->stH264Vbr.u32MaxIQp;
            pstH264Vbr->u32MinIQp = pstMediaVencComm->stH264Vbr.u32MinIQp;

            pstH264Vbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264Vbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstH264Vbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264Vbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264Vbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H264_BitRate;
        }
        else if(HI_MAPI_VENC_RC_MODE_QVBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H264_QVBR_S *pstH264QVbr = &pstVencAttr->stRcAttr.unAttr.stH264QVbr;
            pstH264QVbr->u32MaxQp = pstMediaVencComm->stH264QVbr.u32MaxQp;
            pstH264QVbr->u32MinQp = pstMediaVencComm->stH264QVbr.u32MinQp;
            pstH264QVbr->u32MaxIQp = pstMediaVencComm->stH264QVbr.u32MaxIQp;
            pstH264QVbr->u32MinIQp = pstMediaVencComm->stH264QVbr.u32MinIQp;
            pstH264QVbr->s32BitPercentUL = pstMediaVencComm->stH264QVbr.s32BitPercentUL;
            pstH264QVbr->s32BitPercentLL = pstMediaVencComm->stH264QVbr.s32BitPercentLL;
            pstH264QVbr->s32PsnrFluctuateUL = pstMediaVencComm->stH264QVbr.s32PsnrFluctuateUL;
            pstH264QVbr->s32PsnrFluctuateLL = pstMediaVencComm->stH264QVbr.s32PsnrFluctuateLL;

            pstH264QVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH264QVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH264QVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH264QVbr->stAttr.u32TargetBitRate = pstVencSpecAttr->u32H264_BitRate;
            pstH264QVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }

    }
    else if(HI_MAPI_PAYLOAD_TYPE_H265 == enPayloadType)
    {
        pstVencAttr->stTypeAttr.u32Profile = pstMediaVencComm->u32H265Profile;
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_CBR_S *pstH265Cbr = &pstVencAttr->stRcAttr.unAttr.stH265Cbr;
            pstH265Cbr->u32MaxQp = pstMediaVencComm->stH265Cbr.u32MaxQp;
            pstH265Cbr->u32MinQp = pstMediaVencComm->stH265Cbr.u32MinQp;
            pstH265Cbr->u32MaxIQp = pstMediaVencComm->stH265Cbr.u32MaxIQp;
            pstH265Cbr->u32MinIQp = pstMediaVencComm->stH265Cbr.u32MinIQp;

            pstH265Cbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265Cbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstH265Cbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265Cbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH265Cbr->stAttr.u32BitRate = pstVencSpecAttr->u32H265_BitRate;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_VBR_S *pstH265Vbr = &pstVencAttr->stRcAttr.unAttr.stH265Vbr;
            pstH265Vbr->u32MaxQp = pstMediaVencComm->stH265Vbr.u32MaxQp;
            pstH265Vbr->u32MinQp = pstMediaVencComm->stH265Vbr.u32MinQp;
            pstH265Vbr->u32MaxIQp = pstMediaVencComm->stH265Vbr.u32MaxIQp;
            pstH265Vbr->u32MinIQp = pstMediaVencComm->stH265Vbr.u32MinIQp;

            pstH265Vbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265Vbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstH265Vbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265Vbr->stAttr.fr32DstFrameRate= pstVencSpecAttr->u32DstFrameRate;
            pstH265Vbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H265_BitRate;
        }
        else if(HI_MAPI_VENC_RC_MODE_QVBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_H265_QVBR_S *pstH265QVbr = &pstVencAttr->stRcAttr.unAttr.stH265QVbr;
            pstH265QVbr->u32MaxQp = pstMediaVencComm->stH265QVbr.u32MaxQp;
            pstH265QVbr->u32MinQp = pstMediaVencComm->stH265QVbr.u32MinQp;
            pstH265QVbr->u32MaxIQp = pstMediaVencComm->stH265QVbr.u32MaxIQp;
            pstH265QVbr->u32MinIQp = pstMediaVencComm->stH265QVbr.u32MinIQp;
            pstH265QVbr->s32BitPercentUL = pstMediaVencComm->stH265QVbr.s32BitPercentUL;
            pstH265QVbr->s32BitPercentLL = pstMediaVencComm->stH265QVbr.s32BitPercentLL;
            pstH265QVbr->s32PsnrFluctuateUL = pstMediaVencComm->stH265QVbr.s32PsnrFluctuateUL;
            pstH265QVbr->s32PsnrFluctuateLL = pstMediaVencComm->stH265QVbr.s32PsnrFluctuateLL;

            pstH265QVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstH265QVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstH265QVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstH265QVbr->stAttr.u32TargetBitRate = pstVencSpecAttr->u32H265_BitRate;
            pstH265QVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }

    }
    else if(HI_MAPI_PAYLOAD_TYPE_MJPEG == enPayloadType)
    {
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_CBR_S *pstMjpegCbr =  &pstVencAttr->stRcAttr.unAttr.stMjpegCbr;
            pstMjpegCbr->u32MaxQfactor = pstMediaVencComm->stMjpegCbr.u32MaxQfactor;
            pstMjpegCbr->u32MinQfactor = pstMediaVencComm->stMjpegCbr.u32MinQfactor;

            pstMjpegCbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstMjpegCbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstMjpegCbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstMjpegCbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstMjpegCbr->stAttr.u32BitRate= pstVencSpecAttr->u32H264_BitRate;
        }
        else if(HI_MAPI_VENC_RC_MODE_VBR == pstVencCommAttr->enRcMode)
        {
            HI_MEDIA_VENC_ATTR_MJPEG_VBR_S *pstMjpegVbr =  &pstVencAttr->stRcAttr.unAttr.stMjpegVbr;

            pstMjpegVbr->stAttr.u32Gop = pstVencSpecAttr->u32Gop;
            pstMjpegVbr->stAttr.u32StatTime = pstVencSpecAttr->u32StatTime;
            pstMjpegVbr->stAttr.u32SrcFrameRate = pstVencSpecAttr->u32SrcFrameRate;
            pstMjpegVbr->stAttr.fr32DstFrameRate = pstVencSpecAttr->u32DstFrameRate;
            pstMjpegVbr->stAttr.u32MaxBitRate = pstVencSpecAttr->u32H264_BitRate;
        }
        else
        {
            MLOGD("unsupport rc mode [%d] \n",pstVencCommAttr->enRcMode);
            return HI_PDT_PARAM_EUNSUPPORT;
        }
    }
    else
    {
        MLOGD("unsupport payload type [%d] \n",pstVencCommAttr->enPayload);
        return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_GetVencCfg(PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaComm,
    PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S *pstVencCommAttr,
    PDT_PARAM_MEDIAMODE_VENC_ATTR_S *pstVencSpecAttr,
    HI_PDT_MEDIA_VENC_CFG_S *pstVencCfg)
{
    HI_S32 s32Ret = HI_FAILURE;
    pstVencCfg->bEnable = pstVencCommAttr->bEnable;
    pstVencCfg->VencHdl = pstVencCommAttr->VencHdl;
    pstVencCfg->enBindedMod = pstVencCommAttr->enBindedMod;
    pstVencCfg->ModHdl = pstVencCommAttr->ModHdl;
    pstVencCfg->ChnHdl = pstVencCommAttr->ChnHdl;
    pstVencCfg->enVencType = pstVencCommAttr->enVencType;

    switch(pstVencCommAttr->enVencType)
    {
        case  HI_PDT_MEDIA_VENC_TYPE_REC:
            s32Ret = PDT_PARAM_GetVideoVencCfg(&pstMediaComm->stMainVideoVencAttr,pstVencCommAttr,pstVencSpecAttr,&pstVencCfg->stVencAttr);
            break;
        case HI_PDT_MEDIA_VENC_TYPE_LIVE:
            s32Ret = PDT_PARAM_GetVideoVencCfg(&pstMediaComm->stSubVideoVencAttr,pstVencCommAttr,pstVencSpecAttr,&pstVencCfg->stVencAttr);
            break;
        case HI_PDT_MEDIA_VENC_TYPE_SNAP:
            s32Ret = PDT_PARAM_GetSnapVencCfg(&pstMediaComm->stSingleSnapVencAttr,pstVencCommAttr,pstVencSpecAttr,&pstVencCfg->stVencAttr);
            break;
        case HI_PDT_MEDIA_VENC_TYPE_THM:
            s32Ret = PDT_PARAM_GetSnapVencCfg(&pstMediaComm->stSubSnapVencAttr,pstVencCommAttr,pstVencSpecAttr,&pstVencCfg->stVencAttr);
            break;
        default:
            MLOGE("unsupport venc type[%d] \n",pstVencCommAttr->enVencType);
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    memcpy(&pstVencCfg->stVencAttr.stRcAttr.stHierarchicalQp,
                &pstMediaComm->stHierarchicalQp, sizeof(VENC_HIERARCHICAL_QP_S));

    return s32Ret;
}


HI_S32 PDT_PARAM_GetDispCfg(PDT_PARAM_MEDIA_COMM_DISP_CFG_S *pstDispCommCfg, HI_PDT_PARAM_DISP_CFG_S *pstDispCfg)
{

    pstDispCfg->bEnable = pstDispCommCfg->bEnable;
    pstDispCfg->VdispHdl = pstDispCommCfg->VdispHdl;
    pstDispCfg->u32BgColor = pstDispCommCfg->u32BgColor;
    pstDispCfg->u32WindowCnt = pstDispCommCfg->u32WindowCnt;
    memcpy(&pstDispCfg->stCscAttr, &pstDispCommCfg->stCscAttr,sizeof(HI_PDT_MEDIA_DISP_CSC_ATTR_S));

    HI_S32 i = 0;
    for(i = 0;i < HI_PDT_MEDIA_DISP_WND_MAX_CNT;i++)
    {
        HI_PDT_MEDIA_DISP_WND_CFG_S *pstMediaDispWnd = &pstDispCfg->astWndCfg[i];
        PDT_PARAM_DISP_WND_CFG_S *pstParamDispWnd = &pstDispCommCfg->astWndCfg[i];

        pstMediaDispWnd->bEnable = pstParamDispWnd->bEnable;
        pstMediaDispWnd->WndHdl = pstParamDispWnd->WndHdl;
        pstMediaDispWnd->enBindedMod = pstDispCommCfg->astDispSrc[pstParamDispWnd->s32PreViewCamID].enBindedMod;
        pstMediaDispWnd->ModHdl = pstDispCommCfg->astDispSrc[pstParamDispWnd->s32PreViewCamID].ModHdl;
        pstMediaDispWnd->ChnHdl = pstDispCommCfg->astDispSrc[pstParamDispWnd->s32PreViewCamID].ChnHdl;
        pstMediaDispWnd->enRotate = pstParamDispWnd->enRotate;
        memcpy(&pstMediaDispWnd->stAspectRatio, &pstParamDispWnd->stAspectRatio,sizeof(ASPECT_RATIO_S));
        memcpy(&pstMediaDispWnd->stWndAttr, &pstParamDispWnd->stWndAttr,sizeof(HI_MAPI_DISP_WINDOW_ATTR_S));
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_GetRecFileAttr(PDT_PARAM_REC_COMM_ATTR_S *pstCommAttr,
                const PDT_PARAM_RECMNG_ATTR_S *pstRecMngAttr,
                const HI_S32 s32FileIdx,
                HI_RECMNG_FILE_ATTR_S *pstFileAttr)
{
    pstFileAttr->s32PlayFps = pstRecMngAttr->s32PlayFps;
    pstFileAttr->stDataSource = pstRecMngAttr->astRecFileDataSrc[s32FileIdx];
    pstFileAttr->stMuxerCfg.u32VBufSize = pstRecMngAttr->au32VBufSize[s32FileIdx];
    pstFileAttr->stMuxerCfg.u32PreAllocUnit = pstCommAttr->u32PreAllocUnit;
    pstFileAttr->stMuxerCfg.enMuxerType = pstCommAttr->enRecMuxerType;

    switch(pstFileAttr->stMuxerCfg.enMuxerType)
    {
        case HI_RECMNG_MUXER_MP4:
            pstFileAttr->stMuxerCfg.unMuxerCfg.stMp4Cfg.u32RepairUnit = pstCommAttr->u32RepairUnit;
            break;
        case HI_RECMNG_MUXER_MOV:
            pstFileAttr->stMuxerCfg.unMuxerCfg.stMovCfg.u32RepairUnit = pstCommAttr->u32RepairUnit;
            break;
        case HI_RECMNG_MUXER_TS:
            break;
        default:
            MLOGD("unsupport muxer type [%d] \n",pstFileAttr->stMuxerCfg.enMuxerType);
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;

}

static HI_VOID PDT_PARAM_GetNormRecCfg(HI_PDT_NORM_REC_ATTR_S *pstCfg)
{
    HI_S32 s32Idx = 0,s32FileIdx = 0;
    PDT_PARAM_WORKMODE_CFG_S *pstWorkModeCfg = &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg;
    memcpy(&pstCfg->stPhotoSrc,&pstWorkModeCfg->stRecMediaCfg.stPhotoSrc,sizeof(HI_PDT_PARAM_PHOTO_SRC_S) * HI_PDT_MEDIA_VCAP_DEV_MAX_CNT);

    for(s32Idx = 0;s32Idx < HI_PDT_RECTASK_MAX_CNT;s32Idx++)
    {
        pstCfg->astRecMngAttr[s32Idx].bEnable = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.bEnable;
        pstCfg->astRecMngAttr[s32Idx].stSplitAttr = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.stSplitAttr;
        pstCfg->astRecMngAttr[s32Idx].enRecThmType = pstWorkModeCfg->stRecCommAttr.enRecThmType;
        pstCfg->astRecMngAttr[s32Idx].stThmAttr = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.stThmAttr;
        pstCfg->astRecMngAttr[s32Idx].u32BufferTime_ms = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.u32BufferTime_ms;
        pstCfg->astRecMngAttr[s32Idx].u32PreRecTimeSec = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.u32PreRecTimeSec;

        pstCfg->astRecMngAttr[s32Idx].u32FileCnt = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.u32FileCnt;

        pstCfg->astRecMngAttr[s32Idx].u32Interval_ms = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.u32Interval_ms;
        pstCfg->astRecMngAttr[s32Idx].enRecType = pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr.enRecType;
        for(s32FileIdx = 0;s32FileIdx < pstCfg->astRecMngAttr[s32Idx].u32FileCnt && s32FileIdx < HI_PDT_RECTASK_FILE_MAX_CNT;s32FileIdx++)
        {
            PDT_PARAM_GetRecFileAttr(&pstWorkModeCfg->stRecCommAttr,&pstWorkModeCfg->astNormRecAttr[s32Idx].stRecMngAttr,
                s32FileIdx,&pstCfg->astRecMngAttr[s32Idx].astFileAttr[s32FileIdx]);
        }
    }

    PDT_PARAM_DebugNormRecCfg(pstCfg);
}

static HI_BOOL PDT_PARAM_IsRecStreamCheck(HI_HANDLE VencHdl)
{
    HI_PDT_NORM_REC_ATTR_S stRecAttr;
    HI_S32 i,j,k;
    memset(&stRecAttr,'\0',sizeof(HI_PDT_NORM_REC_ATTR_S));
    PDT_PARAM_GetNormRecCfg(&stRecAttr);
    for(i = 0;i < HI_PDT_RECTASK_MAX_CNT ; i++)
        for(j = 0; j < HI_PDT_RECTASK_FILE_MAX_CNT ; j++)
            for(k = 0; k < stRecAttr.astRecMngAttr[i].astFileAttr[j].stDataSource.u32VencCnt; k++)
            {
                if(stRecAttr.astRecMngAttr[i].astFileAttr[j].stDataSource.aVencHdl[k] == VencHdl)
                {
                    return HI_TRUE;
                }
            }
    return HI_FALSE;
}

static HI_VOID PDT_PARAM_GetPhotoCfg(HI_PDT_PARAM_PHOTO_CFG_S *pstCfg)
{
    PDT_PARAM_WORKMODE_CFG_S *pstWorkModeCfg = &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg;
    memcpy(&pstCfg->stPhotoSrc,&pstWorkModeCfg->stPhotoMediaCfg.stPhotoSrc,sizeof(HI_PDT_PARAM_PHOTO_SRC_S) * HI_PDT_MEDIA_VCAP_DEV_MAX_CNT);

    PDT_PARAM_DebugPhotoCfg(pstCfg);
}


static HI_S32 PDT_PARAM_GetMediaMode(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_MEDIAMODE_E *penMediaMode)
{
    HI_S32 s32Idx = 0;
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_PARKING_REC:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.astMediaModeCfg[s32Idx].enMediaMode;
                    break;
                }
            }
            break;
        case HI_PDT_WORKMODE_PHOTO:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.astMediaModeCfg[s32Idx].enMediaMode;
                    break;
                }
            }
            break;
        case HI_PDT_WORKMODE_UVC:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg.astMediaModeCfg[s32Idx].enMediaMode;
                    break;
                }
            }
            break;
        case HI_PDT_WORKMODE_PLAYBACK:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    *penMediaMode = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPlayBackCfg.astMediaModeCfg[s32Idx].enMediaMode;
                    break;
                }
            }
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[MediaMode]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }

    if(s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get mediamode WorkMode[%s] CAMID[%d] With Param Type[MediaMode] error \n",PDT_PARAM_GetWorkModeStr(enWorkMode),s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("get mediamode WorkMode[%s] CAMID[%d] MediaMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode),s32CamID, PDT_PARAM_GetMediaModeStr(*penMediaMode));
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetMediaMode(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_MEDIAMODE_E *penMediaMode)
{
    HI_S32 s32Idx = 0;
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stRecMediaCfg.astMediaModeCfg[s32Idx].enMediaMode = *penMediaMode;
                    break;
                }
            }
            break;
         case HI_PDT_WORKMODE_PHOTO:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPhotoMediaCfg.astMediaModeCfg[s32Idx].enMediaMode = *penMediaMode;
                    break;
                }
            }
            break;
        case HI_PDT_WORKMODE_UVC:
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
            {
                if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg.astMediaModeCfg[s32Idx].s32CamID)
                {
                    PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg.astMediaModeCfg[s32Idx].enMediaMode = *penMediaMode;
                    break;
                }
            }
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With Param Type[MediaMode]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }

    if(s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set mediamode WorkMode[%s] CAMID[%d] With Param Type[MediaMode] error \n",PDT_PARAM_GetWorkModeStr(enWorkMode),s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("set mediamode WorkMode[%s] CAMID[%d] MediaMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(enWorkMode),s32CamID, PDT_PARAM_GetMediaModeStr(*penMediaMode));
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetEncPayloadType(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_MAPI_PAYLOAD_TYPE_E *pPayload)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[PAYLOAD] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0,j = 0;

    *pPayload = HI_MAPI_PAYLOAD_TYPE_BUTT;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S *pstVencCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg;
            for(j = 0; j < pstVencCommCfg->u32VencCnt && j < HI_PDT_VCAPDEV_VENC_MAX_CNT;j++)
            {
                if(HI_PDT_MEDIA_VENC_TYPE_REC == pstVencCommCfg->astVencAttr[j].enVencType)
                {
                    *pPayload = pstVencCommCfg->astVencAttr[j].enPayload;
                    break;    /** the first venc found is main venc */
                }
            }
        }
    }
    if(HI_MAPI_PAYLOAD_TYPE_BUTT == *pPayload)
    {
        MLOGE("get CAMID[%d] With Param Type[PAYLOAD] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get enc payload CAMID[%d] Payload[%d] \n",s32CamID,*pPayload);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetApkVencId(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_HANDLE *VencId)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[APK_VENCID] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0,j = 0;

    *VencId = 1;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S *pstVencCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg;
        for(j = 0; j < pstVencCommCfg->u32VencCnt && j < HI_PDT_VCAPDEV_VENC_MAX_CNT;j++)
        {
            if(HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCommCfg->astVencAttr[j].enVencType)
            {
                *VencId = pstVencCommCfg->astVencAttr[j].VencHdl;
                MLOGD("get enc Apk VencId CAMID[%d] VencId[%d] \n",s32CamID,*VencId);
                return HI_SUCCESS;
            }
        }
    }
    MLOGE("get CAMID[%d] With Param Type[APK_VENCID] error \n",s32CamID);
    return HI_PDT_PARAM_EINVAL;
}

static HI_S32 PDT_PARAM_GetCamStatus(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pbEnable = PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].bEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CamStatus error CAMID[%d]  \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get CamStatus CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetCamStatus(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].bEnable = *pbEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CamStatus error CAMID[%d]  \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set CamStatus CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetEncPayloadType(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_MAPI_PAYLOAD_TYPE_E *pPayload)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[PAYLOAD] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0,j = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S *pstVencCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg;
            for(j = 0; j < pstVencCommCfg->u32VencCnt && j < HI_PDT_VCAPDEV_VENC_MAX_CNT;j++)
            {
                if(HI_PDT_MEDIA_VENC_TYPE_REC == pstVencCommCfg->astVencAttr[j].enVencType)
                {
                    pstVencCommCfg->astVencAttr[j].enPayload = *pPayload;
                    break;    /** the first venc found is main venc */
                }
            }
        }
    }

    MLOGD("set venc payload CAMID[%d] Payload[%d] \n",s32CamID,*pPayload);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetFlip(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[FLIP] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;
    PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S *pstMediaModeCommCfg = NULL;
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstSpecMediaCfg = NULL;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            pstMediaModeCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType];
            pstSpecMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].astMediaModeSpecCfg[0];

            if(HI_FALSE == pstSpecMediaCfg->stVcapAttr.astVcapPipeAttr[0].bIspBypass
              && VI_ONLINE_VPSS_ONLINE != pstSpecMediaCfg->astMode[0].enMode)
            {
                *pbEnable = pstMediaModeCommCfg->stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].bFlip;
            }
            else
            {
                *pbEnable = pstMediaModeCommCfg->stVprocCfg.astVpssAttr[0].astPortAttr[0].bFlip;
            }
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[FLIP] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get flip CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetFlip(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[FLIP] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;
    PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S *pstMediaModeCommCfg = NULL;
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstSpecMediaCfg = NULL;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            pstMediaModeCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType];
            pstSpecMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].astMediaModeSpecCfg[0];
            if(HI_FALSE == pstSpecMediaCfg->stVcapAttr.astVcapPipeAttr[0].bIspBypass
              && VI_ONLINE_VPSS_ONLINE != pstSpecMediaCfg->astMode[0].enMode)
            {
                pstMediaModeCommCfg->stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].bFlip = *pbEnable;
            }
            else
            {

                HI_S32 j = 0,k = 0;
                for(j = 0; j < HI_PDT_VCAPDEV_VPSS_MAX_CNT;j++)
                {
                    for(k = 0; k < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;k++)
                    {
                        pstMediaModeCommCfg->stVprocCfg.astVpssAttr[j].astPortAttr[k].bFlip = *pbEnable;

                    }
                }
            }
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[FLIP] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set flip CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMirror(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[MIRROR] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;
    PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S *pstMediaModeCommCfg = NULL;
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstSpecMediaCfg = NULL;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            pstMediaModeCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType];
            pstSpecMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].astMediaModeSpecCfg[0];

            if(HI_FALSE == pstSpecMediaCfg->stVcapAttr.astVcapPipeAttr[0].bIspBypass
              && VI_ONLINE_VPSS_ONLINE != pstSpecMediaCfg->astMode[0].enMode)
            {
                *pbEnable = pstMediaModeCommCfg->stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].bMirror;
            }
            else
            {
                *pbEnable = pstMediaModeCommCfg->stVprocCfg.astVpssAttr[0].astPortAttr[0].bMirror;
            }
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[MIRROR] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get mirror CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_SetMirror(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[MIRROR] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;
    PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S *pstMediaModeCommCfg = NULL;
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstSpecMediaCfg = NULL;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            pstMediaModeCommCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType];
            pstSpecMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].astMediaModeSpecCfg[0];
            if(HI_FALSE == pstSpecMediaCfg->stVcapAttr.astVcapPipeAttr[0].bIspBypass
              && VI_ONLINE_VPSS_ONLINE != pstSpecMediaCfg->astMode[0].enMode)
            {
                pstMediaModeCommCfg->stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].bMirror= *pbEnable;
            }
            else
            {

                HI_S32 j = 0,k = 0;
                for(j = 0; j < HI_PDT_VCAPDEV_VPSS_MAX_CNT;j++)
                {
                    for(k = 0; k < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;k++)
                    {
                        pstMediaModeCommCfg->stVprocCfg.astVpssAttr[j].astPortAttr[k].bMirror = *pbEnable;

                    }
                }
            }
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[MIRROR] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set mirror CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_GetLDC(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[LDC] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pbEnable = PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].stLDCAttr.bEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[LDC] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get LDC CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetLDC(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_BOOL *pbEnable)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[LDC] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.astVcapPipeAttr[0].astPipeChnAttr[0].stLDCAttr.bEnable = *pbEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[LDC] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set LDC CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetWDR(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, WDR_MODE_E *penWdrMode)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[WDR] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *penWdrMode = PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.enWdrMode;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[WDR] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get WDR CAMID[%d] enWdrMode[%d] \n",s32CamID,*penWdrMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetWDR(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, WDR_MODE_E *penWdrMode)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[WDR] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.enWdrMode = *penWdrMode;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[WDR] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set WDR CAMID[%d] enWdrMode[%d] \n",s32CamID,*penWdrMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVideoMode(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_U32 *pu32Mode)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[VideoMode] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pu32Mode = PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.u32VideoMode;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[VideoMode] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get CAMID[%d] u32VideoMode[%d] \n",s32CamID,*pu32Mode);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetVideoMode(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_U32 *pu32Mode)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[VideoMode] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev.u32VideoMode = *pu32Mode;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[VideoMode] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set CAMID[%d] u32VideoMode[%d] \n",s32CamID,*pu32Mode);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_GetCrop(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_PDT_MEDIA_DISP_WND_CROP_CFG_S *pstCropCfg)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, get CAMID[%d] With Param Type[crop] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            memcpy(pstCropCfg,&(PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stCropCfg),sizeof(HI_PDT_MEDIA_DISP_WND_CROP_CFG_S));
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[crop] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get crop CAMID[%d] Enable[%d] u32CurH[%d] u32MaxH[%d]\n",s32CamID,pstCropCfg->bEnable,pstCropCfg->u32CurH,pstCropCfg->u32MaxH);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetCrop(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, const HI_PDT_MEDIA_DISP_WND_CROP_CFG_S *pstCropCfg)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[crop] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            memcpy(&(PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stCropCfg),pstCropCfg,sizeof(HI_PDT_MEDIA_DISP_WND_CROP_CFG_S));
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[crop] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set crop CAMID[%d] Enable[%d] u32CurH[%d] u32MaxH[%d]\n",s32CamID,pstCropCfg->bEnable,pstCropCfg->u32CurH,pstCropCfg->u32MaxH);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetOSD(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        PDT_PARAM_CAM_MEDIA_CFG_S* pstCamMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i];
        if(s32CamID == pstCamMediaCfg->s32CamID)
        {
            *pbEnable = pstCamMediaCfg->bOsdShow;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get osd CAMID[%d] With Param Type[OSD] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get osd CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetOSD(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        PDT_PARAM_CAM_MEDIA_CFG_S* pstCamMediaCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i];
        if(s32CamID == pstCamMediaCfg->s32CamID)
        {
            pstCamMediaCfg->bOsdShow = *pbEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set osd CAMID[%d] With Param Type[OSD] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set osd CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetRecEnable(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_NORM_REC_ATTR_S *pstRecAttr = &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i];
            *pbEnable = pstRecAttr->stRecMngAttr.bEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("Get Rec enable CAMID[%d] With Param Type[rec enable] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("Get Rec enable CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_SetRecEnable(HI_S32 s32CamID, HI_BOOL *pbEnable)
{
    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_NORM_REC_ATTR_S *pstRecAttr = &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i];
            pstRecAttr->stRecMngAttr.bEnable = *pbEnable;
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set Rec enable CAMID[%d] With Param Type[rec enable] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set Rec enable CAMID[%d] Enable[%d] \n",s32CamID,*pbEnable);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_GetViVpssMode(HI_BOOL bEnable,
    HI_MEDIA_VI_VPSS_MODE_S *pstSrcViVpssMode,
    HI_MEDIA_VI_VPSS_MODE_S *pstDstViVpssMode)
{
    HI_S32 i = 0;
    if(bEnable)
    {
        for(i = 0;i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;i++)
        {
            pstDstViVpssMode[i].VcapPipeHdl = pstSrcViVpssMode[i].VcapPipeHdl;
            pstDstViVpssMode[i].enMode = pstSrcViVpssMode[i].enMode;
        }
    }
    return HI_SUCCESS;

}

static HI_S32 PDT_PARAM_GetRecSplitTime(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.stSplitAttr.u32SplitTimeLenSec;
            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("get [%d]_REC split time error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("get [%d]_REC split time[%d]\n", s32CamID, *pvParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetRecSplitTime(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.stSplitAttr.u32SplitTimeLenSec = *pvParam;
            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("set [%d]_REC split time error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("set [%d]_REC split time[%d]\n", s32CamID, *pvParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVBCfg(HI_PDT_MEDIA_VB_CFG_S *pstCamVbCfg,HI_PDT_MEDIA_VB_CFG_S *pstTotalVb)
{
    HI_S32 s32CamVbIdx = 0, s32TotalVbIdx = 0;
    HI_S32 s32TotalVbCnt = pstTotalVb->u32MaxPoolCnt;

    for(s32CamVbIdx= 0;s32CamVbIdx < pstCamVbCfg->u32MaxPoolCnt && s32CamVbIdx < HI_PDT_MEDIA_VB_MAX_POOLS; s32CamVbIdx++)
    {
        for(s32TotalVbIdx = 0;s32TotalVbIdx < s32TotalVbCnt;s32TotalVbIdx++)
        {
            if(pstCamVbCfg->astCommPool[s32CamVbIdx].u32BlkSize == pstTotalVb->astCommPool[s32TotalVbIdx].u32BlkSize)
            {
                /*the same size,just add the vb count*/
                pstTotalVb->astCommPool[s32TotalVbIdx].u32BlkCnt += pstCamVbCfg->astCommPool[s32CamVbIdx].u32BlkCnt;
                break;
            }
        }
        if(s32TotalVbIdx >= s32TotalVbCnt)
        {
            /*need add one pool to store*/
            if(pstTotalVb->u32MaxPoolCnt + 1 <= HI_PDT_MEDIA_VB_MAX_POOLS)
            {
                pstTotalVb->astCommPool[pstTotalVb->u32MaxPoolCnt].u32BlkSize = pstCamVbCfg->astCommPool[s32CamVbIdx].u32BlkSize;
                pstTotalVb->astCommPool[pstTotalVb->u32MaxPoolCnt].u32BlkCnt = pstCamVbCfg->astCommPool[s32CamVbIdx].u32BlkCnt;
                pstTotalVb->u32MaxPoolCnt++;
            }
            else
            {
                MLOGE("total vb count[%d] is full \n",pstTotalVb->u32MaxPoolCnt);
                return HI_PDT_PARAM_EINVAL;
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetVcapDevAttr(HI_BOOL bEnable,
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstSrcDevAttr,
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstDstDevAttr)
{
    if(bEnable)
    {
        memcpy(pstDstDevAttr,pstSrcDevAttr,sizeof(HI_PDT_MEDIA_VCAP_DEV_ATTR_S));
    }
    else
    {
        pstDstDevAttr->bEnable = HI_FALSE;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetLapseRecIntervalTime(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.u32Interval_ms = *pvParam;
            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("set [%d]_LAPSE_REC u32Interval_ms time error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("set [%d]LAPSE__REC split time[%d]\n", s32CamID, *pvParam);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetLapseRecIntervalTime(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.u32Interval_ms;

            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("get [%d]_REC split time error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("get [%d]_REC split time[%d]\n", s32CamID, *pvParam);
    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_SetRecordType(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.enRecType = *pvParam;
            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("set [%d]_LAPSE_REC u32Interval_ms time error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetRecordType(HI_S32 s32CamID, HI_S32* pvParam)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            *pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.astNormRecAttr[i].stRecMngAttr.enRecType;
            break;
        }
    }

    if (i >= HI_PDT_RECTASK_MAX_CNT)
    {
        MLOGE("get [%d]_REC type error\n", s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    MLOGD("get [%d]_REC type [%d]\n", s32CamID, *pvParam);
    return HI_SUCCESS;
}

static HI_U32 PDT_PARAM_GCD(HI_U32 x,HI_U32 y)
{
    HI_U32 u32Max = (x>y)?x:y;
    HI_U32 u32Min = (x>y)?y:x;
    HI_U32 z = u32Min;
    while(u32Max%u32Min!=0)
    {
        z = u32Max%u32Min;
        u32Max = u32Min;
        u32Min = z;
    }
    return z;
}

static HI_S32 PDT_PARAM_UpdateWifiApCfg(HI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
#define HI_PDT_LAST_MAC_ADDR_LEN (3)
#define HI_PDT_LAST_MAC_ADDR_STR_LEN (2*HI_PDT_LAST_MAC_ADDR_LEN)
#define HI_PDT_WIFI_INTERFACE_NAME "wlan0"

    HI_CHAR szMacAddrResult[HI_PDT_LAST_MAC_ADDR_STR_LEN + 1] = {0};

    if (0 == strcmp(pstApCfg->stCfg.szWiFiSSID, \
                    PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiApModeCfg.szFactroyWiFiSSID))
    {
        /* get mac Last six digits */
#ifndef __HuaweiLite__
        HI_CHAR szCmd[192] = {'\0'};
        snprintf(szCmd, sizeof(szCmd), "%s", "mac=`ifconfig "HI_PDT_WIFI_INTERFACE_NAME" | head -1 | awk '{ print $5 }' | tr -d ':' `;  echo ${mac:6:6}\n");
        FILE* fp = popen(szCmd, "r");

        if (NULL == fp)
        {
            perror("popen failed\n");
            return HI_SUCCESS;

        }else
        {
            fgets(szMacAddrResult, sizeof(szMacAddrResult), fp);
            pclose(fp);
        }
#elif defined(AMP_HUAWEILITE)
        struct netif *netif = netif_find(HI_PDT_WIFI_INTERFACE_NAME);
        if (NULL == netif)
        {
            perror("netif_find failed\n");
            return HI_SUCCESS;
        }
        else
        {
            if(netif->hwaddr_len<HI_PDT_LAST_MAC_ADDR_LEN)
            {
                MLOGE("HWaddr to short, Not added mac addr\n");
                return HI_SUCCESS;
            }
            HI_S32 ret=0;
            HI_S32 i=0;
            for(i = MIN(HI_PDT_LAST_MAC_ADDR_LEN,netif->hwaddr_len-HI_PDT_LAST_MAC_ADDR_LEN); i < netif->hwaddr_len ; i++)
            {
                ret += snprintf_s(&szMacAddrResult[ret], sizeof(szMacAddrResult)-ret, 2, "%02X", netif->hwaddr[i]);
                if(ret >= sizeof(szMacAddrResult)){
                    break;
                }
            }
        }
#endif
        if(((strlen(pstApCfg->stCfg.szWiFiSSID) - 1) + HI_PDT_LAST_MAC_ADDR_STR_LEN) >= HI_HAL_WIFI_SSID_LEN)
        {
            MLOGE("WiFiSSID to long, Not added mac addr\n");
        }else
        {
            strncat(pstApCfg->stCfg.szWiFiSSID, szMacAddrResult, HI_PDT_LAST_MAC_ADDR_STR_LEN);
        }
        MLOGD("szWiFiSSID:%s\n", pstApCfg->stCfg.szWiFiSSID);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_WorkMode2MediaModeType(HI_PDT_WORKMODE_E enWorkMode, PDT_PARAM_MEDIAMODE_TYPE_E *penMediaModeType)
{
    *penMediaModeType = PDT_PARAM_MEDIAMODE_TYPE_NORM_REC;
    if (HI_PDT_WORKMODE_PHOTO == enWorkMode)
    {
        *penMediaModeType = PDT_PARAM_MEDIAMODE_TYPE_PHOTO;
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_PARAM_UpdateLapseRecVencAttr(HI_PDT_PARAM_RECMNG_ATTR_S* pstRecMngAttr,
                                            HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_S32 s32VencIndex)
{
    HI_U32 u32Interval_ms = pstRecMngAttr->u32Interval_ms;
    HI_U32 u32GCD = PDT_PARAM_GCD(1000, u32Interval_ms);
    MLOGI(YELLOW"VencIndex(%d), IntervalMs(%d)\n"NONE, s32VencIndex, u32Interval_ms);

    HI_PDT_MEDIA_VENC_CFG_S* pstStreamVencCfg = &pstMediaCfg->stVideoCfg.astVencCfg[s32VencIndex];

    if (pstStreamVencCfg->bEnable)
    {
        HI_MAPI_VENC_ATTR_CBR_S* pstCBRAttr   = NULL;
        HI_MAPI_VENC_ATTR_VBR_S* pstVBRAttr   = NULL;
        HI_MAPI_VENC_ATTR_QVBR_S* pstQVBRAttr = NULL;
        HI_FR32* pu32srcFrameRate = NULL;
        HI_FR32* pu32dstFrameRate = NULL;
        HI_U32* pu32BitRate = NULL;
        HI_MAPI_PAYLOAD_TYPE_E enStreamType = pstStreamVencCfg->stVencAttr.stTypeAttr.enType;
        HI_MAPI_VENC_RC_MODE_E enRcMode     = pstStreamVencCfg->stVencAttr.stRcAttr.enRcMode;

        switch (enStreamType)
        {
            case HI_MAPI_PAYLOAD_TYPE_H264:
                switch (enRcMode)
                {
                    case HI_MAPI_VENC_RC_MODE_CBR:
                        pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr);
                        break;

                    case HI_MAPI_VENC_RC_MODE_VBR:
                        pstVBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr);
                        break;

                    case HI_MAPI_VENC_RC_MODE_QVBR:
                        pstQVBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr);
                        break;

                    default :
                        {
                            MLOGE("current VENC not support the RCmode[%d]\n", enRcMode);
                            return HI_FAILURE;
                        }
                        break;
                }
                break;

            case HI_MAPI_PAYLOAD_TYPE_H265:
                switch (enRcMode)
                {
                    case HI_MAPI_VENC_RC_MODE_CBR:
                        pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr);
                        break;

                    case HI_MAPI_VENC_RC_MODE_VBR:
                        pstVBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr);
                        break;

                    case HI_MAPI_VENC_RC_MODE_QVBR:
                        pstQVBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr);
                        break;

                    default :
                        {
                            MLOGE("current VENC not support the RCmode[%d]\n", enRcMode);
                            return HI_FAILURE;
                        }
                        break;
                }
                break;

            default:
                {
                    MLOGD(YELLOW"StreamType(%d) err\n"NONE, enStreamType);
                    return HI_SUCCESS;
                }
                break;
        }

        if (pstCBRAttr != NULL)
        {
            pu32srcFrameRate = &pstCBRAttr->u32SrcFrameRate;
            pu32dstFrameRate = &pstCBRAttr->fr32DstFrameRate;
            pu32BitRate      = &pstCBRAttr->u32BitRate;
        }
        else if (pstVBRAttr != NULL)
        {
            pu32srcFrameRate = &pstVBRAttr->u32SrcFrameRate;
            pu32dstFrameRate = &pstVBRAttr->fr32DstFrameRate;
            pu32BitRate      = &pstVBRAttr->u32MaxBitRate;
        }
        else if (pstQVBRAttr != NULL)
        {
            pu32srcFrameRate = &pstQVBRAttr->u32SrcFrameRate;
            pu32dstFrameRate = &pstQVBRAttr->fr32DstFrameRate;
            pu32BitRate      = &pstQVBRAttr->u32TargetBitRate;
        }
        else
        {
            return HI_FAILURE;
        }


        MLOGD(YELLOW"BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
              *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);

        *pu32BitRate = (*pu32BitRate)/(u32Interval_ms/1000.0 * 30);
        *pu32dstFrameRate = (1000/u32GCD)+((u32Interval_ms/u32GCD) << 16);

        if(*pu32BitRate < 2)
        {
            MLOGW(YELLOW"BitRate(%d) small than 2, set BitRate(2)\n"NONE, *pu32BitRate);
            *pu32BitRate = 2;

        }

        MLOGI(YELLOW"updated, BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
                *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);
    }

    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 PDT_PARAM_GetMdSensitivity(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_U32 *ps32MdSensitivity)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[MdSensitivity] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0, j = 0;
    HI_PDT_PARAM_VIDEODETECT_CFG_S *pstVideoDetectCfg =
        &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVideoDetectCfg;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            if(HI_TRUE == pstVideoDetectCfg->bEnable)
            {
                for(j = 0; j < pstVideoDetectCfg->algCnt; j++)
                {
                    if(HI_VIDEODETECT_ALG_TYPE_MD == pstVideoDetectCfg->algAttr[j].algType)
                    {
                        *ps32MdSensitivity = pstVideoDetectCfg->algAttr[j].algProcMdAttr.mdSensitivity;
                        break;
                    }
                }
                if(j >= pstVideoDetectCfg->algCnt)
                {
                    MLOGE("set CAMID[%d] With Param Type[MdSensitivity] error \n",s32CamID);
                    return HI_PDT_PARAM_EINVAL;
                }
            }
            else
            {
                MLOGE("CAMID[%d] VideoDetect not enable, get MdSensitivity faild! \n",s32CamID);
                return HI_PDT_PARAM_EINVAL;
            }

            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAMID[%d] With Param Type[MdSensitivity] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("get Md Sensitivity CAMID[%d] MdSensitivity[%d] \n",s32CamID, pstVideoDetectCfg->algAttr[0].algProcMdAttr.mdSensitivity);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SetMdSensitivity(HI_S32 s32CamID, PDT_PARAM_MEDIAMODE_TYPE_E enType, HI_U32 *pMdSensitivity)
{
    if (enType >= PDT_PARAM_MEDIAMODE_TYPE_BUTT)
    {
        MLOGE("Medie Mode type is invalid, set CAMID[%d] With Param Type[MdSensitivity] error\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }

    HI_S32 i = 0, j = 0;
    HI_PDT_PARAM_VIDEODETECT_CFG_S *pstVideoDetectCfg = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVideoDetectCfg;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            if(HI_TRUE == pstVideoDetectCfg->bEnable)
            {
                for(j = 0; j < pstVideoDetectCfg->algCnt; j++)
                {
                    if(HI_VIDEODETECT_ALG_TYPE_MD == pstVideoDetectCfg->algAttr[j].algType)
                    {
                        pstVideoDetectCfg->algAttr[j].algProcMdAttr.mdSensitivity = *pMdSensitivity;
                        break;
                    }
                }
                if(j >= pstVideoDetectCfg->algCnt)
                {
                    MLOGE("set CAMID[%d] With Param Type[MdSensitivity] error \n",s32CamID);
                    return HI_PDT_PARAM_EINVAL;
                }
            }
            else
            {
                MLOGE("CAMID[%d] VideoDetect not enable, get MdSensitivity faild! \n",s32CamID);
                return HI_PDT_PARAM_EINVAL;
            }

            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("set CAMID[%d] With Param Type[MdSensitivity] error \n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    MLOGD("set Md Sensitivity CAMID[%d] MdSensitivity[%d] \n",s32CamID,*pMdSensitivity);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetVideoDetectCfg(HI_S32 s32CamID, HI_PDT_PARAM_VIDEODETECT_CFG_S* pstVideoDetectCfg)
{
    HI_S32 i = 0;

    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(s32CamID == PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].s32CamID)
        {
            HI_PDT_PARAM_VIDEODETECT_CFG_S *pstVideoDetectAttr = &PDT_PARAM_GetCtx()->pstCfg->astCamMediaCfg[i].stMediaCommCfg[PDT_PARAM_MEDIAMODE_TYPE_NORM_REC].stVideoDetectCfg;
            memcpy(pstVideoDetectCfg, pstVideoDetectAttr, sizeof(HI_PDT_PARAM_VIDEODETECT_CFG_S));
            break;
        }
    }
    if(i >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("get CAM[%d] VideoDetect config failed!\n",s32CamID);
        return HI_PDT_PARAM_EINVAL;
    }
    PDT_PARAM_DebugVideoDetectCfg(pstVideoDetectCfg);

    return HI_SUCCESS;
}


#endif


HI_S32 HI_PDT_PARAM_GetFileMngCfg(HI_PDT_FILEMNG_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stFileMngCfg, sizeof(HI_PDT_FILEMNG_CFG_S));
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

HI_S32 HI_PDT_PARAM_GetGsensorCfg(HI_PDT_GSENSOR_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    memcpy(pstCfg, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stGsensorCfg, sizeof(HI_PDT_GSENSOR_CFG_S));
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    PDT_PARAM_DebugGsensorCfg(pstCfg);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_UpdateSceneMode(HI_PDT_WORKMODE_E enWorkMode,
    HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_S32 i = 0;
    PDT_PARAM_UVC_ATTR_S* pstUvcCfg = &(PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg);
    for (i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
    {
        if (HI_PDT_WORKMODE_UVC == enWorkMode ||
            HI_PDT_WORKMODE_PHOTO == enWorkMode)
        {
            pstSceneMode->astPipeAttr[i].bBypassVenc = HI_TRUE;
        }
        else
        {
            pstSceneMode->astPipeAttr[i].bBypassVenc = HI_FALSE;
        }

        if (HI_PDT_WORKMODE_UVC == enWorkMode)
        {
            if (pstUvcCfg->stUvcCfg.stDataSource.VprocHdl != pstSceneMode->astPipeAttr[i].VpssHdl)
            {
                pstSceneMode->astPipeAttr[i].bEnable = HI_FALSE;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 PDT_PARAM_UpdateFrmCfg(HI_S32 s32Frm, HI_S32 *pNewFrm)
{
    if((s32Frm % 30 == 0) && (s32Frm / 30 > 0))
    {
        *pNewFrm = (s32Frm /30) * 25;
    }
    else
    {
       *pNewFrm =  s32Frm;
    }
    return HI_SUCCESS;
}

HI_VOID PDT_PARAM_UpdateMediaOsdCfg(HI_U32 u32DevIdx, HI_BOOL  bOsdShow, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_U32 i, j;
    HI_U32 u32OsdIdx = 0;

    /*disable cam osd*/
    for(u32OsdIdx = 0;u32OsdIdx < MIN(pstMediaCfg->stVideoCfg.stOsdCfg.s32OsdCnt, HI_OSD_MAX_CNT);u32OsdIdx++)
    {
        HI_OSD_ATTR_S *pstOsdAttr = &pstMediaCfg->stVideoCfg.stOsdCfg.astOsdAttr[u32OsdIdx];
        for(i = 0;i < MIN(pstOsdAttr->u32DispNum, HI_OSD_MAX_DISP_CNT);i++)
        {
            if(bOsdShow == HI_FALSE)
            {
                if (u32DevIdx==pstOsdAttr->astDispAttr[i].s32Batch/10)
                {
                    pstOsdAttr->astDispAttr[i].bShow=HI_FALSE;
                }
            }
            if ( pstOsdAttr->astDispAttr[i].enBindedMod == HI_OSD_BINDMOD_VENC)
            {
                for(j = 0; j < HI_PDT_MEDIA_VENC_MAX_CNT; j++)
                {
                    if(pstOsdAttr->astDispAttr[i].ChnHdl == pstMediaCfg->stVideoCfg.astVencCfg[j].VencHdl &&
                        pstMediaCfg->stVideoCfg.astVencCfg[j].bEnable == HI_FALSE)
                    {
                        pstOsdAttr->astDispAttr[i].bShow=HI_FALSE;
                    }
                }
            }
        }
    }
}

HI_VOID PDT_PARAM_UpdateCamMediaCfg(HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCamCfg)
{
    HI_S32 i = 0,j = 0;
    HI_S32 u32NewFps;

    for(i = 0;i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;i++)
    {
        if(HI_FALSE == pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].bEnable)
        {
            continue;
        }
        PDT_PARAM_UpdateFrmCfg((HI_S32)pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].stIspPubAttr.f32FrameRate, &u32NewFps);
        pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].stIspPubAttr.f32FrameRate = u32NewFps;
        for(j = 0;j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT;j++)
        {
            if(HI_FALSE == pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].astPipeChnAttr[j].bEnable)
            {
                continue;
            }
            PDT_PARAM_UpdateFrmCfg(pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].astPipeChnAttr[j].stFrameRate.s32SrcFrameRate, &u32NewFps);
            pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].astPipeChnAttr[j].stFrameRate.s32SrcFrameRate
                = u32NewFps;
            PDT_PARAM_UpdateFrmCfg(pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].astPipeChnAttr[j].stFrameRate.s32DstFrameRate, &u32NewFps);
            pstCamCfg->stVcapDevAttr.astVcapPipeAttr[i].astPipeChnAttr[j].stFrameRate.s32DstFrameRate
                = u32NewFps;
        }
    }
    for(i = 0; i < pstCamCfg->u32VpssCnt && i < HI_PDT_VCAPDEV_VPSS_MAX_CNT;i++)
    {
        if(HI_FALSE == pstCamCfg->astVpssCfg[i].bEnable)
        {
            continue;
        }
        PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVpssCfg[i].stVpssAttr.stFrameRate.s32SrcFrameRate, &u32NewFps);
        pstCamCfg->astVpssCfg[i].stVpssAttr.stFrameRate.s32SrcFrameRate = u32NewFps;
        PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVpssCfg[i].stVpssAttr.stFrameRate.s32DstFrameRate, &u32NewFps);
        pstCamCfg->astVpssCfg[i].stVpssAttr.stFrameRate.s32DstFrameRate = u32NewFps;
        for(j = 0;j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;j++)
        {
            if(HI_FALSE == pstCamCfg->astVpssCfg[i].astVportAttr[j].bEnable)
            {
                continue;
            }
            PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVpssCfg[i].astVportAttr[j].stFrameRate.s32SrcFrameRate, &u32NewFps);
            pstCamCfg->astVpssCfg[i].astVportAttr[j].stFrameRate.s32SrcFrameRate = u32NewFps;
            PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVpssCfg[i].astVportAttr[j].stFrameRate.s32DstFrameRate, &u32NewFps);
            pstCamCfg->astVpssCfg[i].astVportAttr[j].stFrameRate.s32DstFrameRate = u32NewFps;
        }
    }
    for(i = 0;i < pstCamCfg->u32VencCnt && i < HI_PDT_VCAPDEV_VENC_MAX_CNT;i++)
    {
        if(HI_FALSE == pstCamCfg->astVencCfg[i].bEnable)
        {
            continue;
        }

        if(pstCamCfg->astVencCfg[i].stVencAttr.stTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H264)
        {
            if(HI_MAPI_VENC_RC_MODE_CBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr.u32Gop = u32NewFps;
            }
            if(HI_MAPI_VENC_RC_MODE_VBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264Vbr.stAttr.u32Gop = u32NewFps;
            }
            if(HI_MAPI_VENC_RC_MODE_QVBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH264QVbr.stAttr.u32Gop = u32NewFps;
            }
        }
        if(pstCamCfg->astVencCfg[i].stVencAttr.stTypeAttr.enType == HI_MAPI_PAYLOAD_TYPE_H265)
        {
            if(HI_MAPI_VENC_RC_MODE_CBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr.u32Gop = u32NewFps;
            }
            if(HI_MAPI_VENC_RC_MODE_VBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265Vbr.stAttr.u32Gop = u32NewFps;
            }
            if(HI_MAPI_VENC_RC_MODE_QVBR == pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.enRcMode)
            {
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr.u32SrcFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr.u32SrcFrameRate = u32NewFps;
                PDT_PARAM_UpdateFrmCfg(pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr.fr32DstFrameRate, &u32NewFps);
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr.fr32DstFrameRate = u32NewFps;
                pstCamCfg->astVencCfg[i].stVencAttr.stRcAttr.unAttr.stH265QVbr.stAttr.u32Gop = u32NewFps;
            }
        }
    }

}

HI_VOID PDT_PARAM_VideoModeUpdateCamMediaCfg(HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCamCfg)
{
    if(pstCamCfg->stVcapDevAttr.u32VideoMode == 0) /** PAL */
    {
        if((pstCamCfg->stVcapDevAttr.astVcapPipeAttr[0].stFrameRate.s32SrcFrameRate % 30 == 0)
            && (pstCamCfg->stVcapDevAttr.astVcapPipeAttr[0].stFrameRate.s32SrcFrameRate / 30 > 0))
        {
            PDT_PARAM_UpdateCamMediaCfg(pstCamCfg);
        }
    }
}

HI_S32 HI_PDT_PARAM_GetCamMediaCfg(HI_PDT_WORKMODE_E enWorkMode,
    HI_PDT_PARAM_MEDIAMODE_CFG_S *pstMediaModeCfg,
    HI_PDT_PARAM_CAM_MEDIA_CFG_S *pstCamCfg,
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S *pstSceneAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaModeCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstCamCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstSceneAttr, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 i = 0,j = 0,k = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    PDT_PARAM_MEDIAMODE_TYPE_E enType = PDT_PARAM_MEDIAMODE_TYPE_NORM_REC;
    s32Ret = PDT_PARAM_WorkMode2MediaModeType(enWorkMode, &enType);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(pstParamCfg->astCamMediaCfg[i].s32CamID == pstMediaModeCfg->s32CamID)
        {
            for(j = 0;j < pstParamCfg->astCamMediaCfg[i].u32MediaModeCnt && j < PDT_PARAM_MEDIAMODE_CNT;j++)
            {
                if(pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].enMediaMode == pstMediaModeCfg->enMediaMode)
                {
                     /*load vi-vpss mode*/
                     memcpy(&pstCamCfg->astMode,&pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].astMode,sizeof(HI_MEDIA_VI_VPSS_MODE_S)*HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT);

                    /*load vi-vpss mode*/
                     memcpy(&pstCamCfg->stVBCfg,&pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].stVBCfg,sizeof(HI_PDT_MEDIA_VB_CFG_S));

                    /*load vcap configure*/
                    s32Ret = PDT_PARAM_GetVcapDevCfg(&pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVcapDev,
                        &pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].stVcapAttr,
                        &pstCamCfg->stVcapDevAttr,pstSceneAttr);
                    PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

                    /*load vproc configure*/
                    pstCamCfg->u32VpssCnt = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.u32VpssCnt;
                    for(k = 0;k < pstCamCfg->u32VpssCnt && k < HI_PDT_VCAPDEV_VPSS_MAX_CNT;k++)
                    {
                        s32Ret = PDT_PARAM_GetVpssAttr(&pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k],
                            &pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].stVprocCfg.astVpssCfg[k],
                            &pstCamCfg->astVpssCfg[k]);
                        PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);

                        /**fill the scene param*/
                        if (pstSceneAttr->VcapPipeHdl == pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k].VcapPipeHdl
                           && pstSceneAttr->PipeChnHdl == pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k].PipeChnHdl
                           && pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k].bEnable)
                        {
                            pstSceneAttr->VpssHdl = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k].VpssHdl;
                            pstSceneAttr->VportHdl = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVprocCfg.astVpssAttr[k].astPortAttr[0].VportHdl;
                            MLOGI("scene param: VcapPipe[%d], VcapPipeChn[%d] VpssHdl[%d] Vport[%d]\n",pstSceneAttr->VcapPipeHdl,
                                pstSceneAttr->PipeChnHdl, pstSceneAttr->VpssHdl, pstSceneAttr->VportHdl);
                        }
                    }

                    /*load venc configure*/
                    HI_BOOL bFirst = HI_FALSE;
                    pstCamCfg->u32VencCnt = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.u32VencCnt;
                    for(k = 0;k < pstCamCfg->u32VencCnt && k < HI_PDT_VCAPDEV_VENC_MAX_CNT;k++)
                    {
                        MLOGD("venc index [%d] \n",k);
                        s32Ret = PDT_PARAM_GetVencCfg(&pstParamCfg->stMediaCommCfg,
                            &pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k],
                            &pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].stVencCfg.astVencAttr[k],
                            &pstCamCfg->astVencCfg[k]);
                        PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
                        if (pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k].enVencType == HI_PDT_MEDIA_VENC_TYPE_REC
                            && !bFirst && pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k].bEnable)
                        {
                            pstSceneAttr->VencHdl = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k].VencHdl;
                            bFirst = HI_TRUE;
                            MLOGI("scene param: VcapPipe[%d], VcapPipeChn[%d] Venc[%d]\n",pstSceneAttr->VcapPipeHdl,
                                pstSceneAttr->PipeChnHdl, pstSceneAttr->VencHdl);
                        }
                    }
                    PDT_PARAM_VideoModeUpdateCamMediaCfg(pstCamCfg);
                    if (pstParamCfg->astCamMediaCfg[i].bEnable)
                    {
                        pstCamCfg->bOsdShow = pstParamCfg->astCamMediaCfg[i].bOsdShow;
                    }
                    else
                    {
                        pstCamCfg->bOsdShow = HI_FALSE;
                    }
                }
            }

            /*load crop configure*/
            memcpy(&pstCamCfg->stCropCfg,&pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stCropCfg,sizeof(HI_PDT_MEDIA_DISP_WND_CROP_CFG_S));
        }
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    MLOGD("CamID[%d], MediaMode[%s \n",pstMediaModeCfg->s32CamID,PDT_PARAM_GetMediaModeStr(pstMediaModeCfg->enMediaMode));
    PDT_PARAM_DebugCamMediaCfg(pstCamCfg);
    PDT_PARAM_DebugSceneCamVcapCfg(pstSceneAttr);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetOsdCfg(HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstOsdCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;
    memcpy(pstOsdCfg, &pstParamCfg->stMediaCommCfg.stOsdCfg,sizeof(HI_PDT_MEDIA_OSD_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetMediaCommCfg(HI_PDT_PARAM_MEDIA_COMM_CFG_S *pstMediaCommCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaCommCfg, HI_PDT_PARAM_EINVAL);

    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;
    HI_S32 i = 0,s32Ret = HI_FAILURE;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);

    for(i = 0;i < HI_PDT_MEDIA_DISP_MAX_CNT;i++)
    {
        s32Ret = PDT_PARAM_GetDispCfg(&pstParamCfg->stMediaCommCfg.astDispCfg[i],&pstMediaCommCfg->stVideoOutCfg.astDispCfg[i]);
        PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
    }

    /*load acap configure*/
    memcpy(pstMediaCommCfg->stAudioCfg.astAcapCfg,pstParamCfg->stMediaCommCfg.astAcapCfg,sizeof(HI_PDT_MEDIA_ACAP_CFG_S)*HI_PDT_MEDIA_ACAP_MAX_CNT);

    /*load aenc configure*/
    memcpy(pstMediaCommCfg->stAudioCfg.astAencCfg,pstParamCfg->stMediaCommCfg.astAencCfg,sizeof(HI_PDT_MEDIA_AENC_CFG_S)*HI_PDT_MEDIA_AENC_MAX_CNT);

    /*load ao configure*/
    memcpy(pstMediaCommCfg->stAudioOutCfg.astAoCfg,pstParamCfg->stMediaCommCfg.astAoCfg,sizeof(HI_PDT_MEDIA_AO_CFG_S)*HI_PDT_MEDIA_AO_MAX_CNT);

    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    PDT_PARAM_DEBUGMediaCommCfg(pstMediaCommCfg);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetMainVencCfgByPayloadType(HI_PDT_WORKMODE_E enWorkMode,
    HI_PDT_PARAM_MEDIAMODE_CFG_S *pstMediaModeCfg,
    HI_MAPI_PAYLOAD_TYPE_E enPayload,
    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstMediaModeCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstVencCfg, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 i = 0,j = 0,k = 0;
    HI_U32 u32VencCnt = 0;
    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;

    PDT_PARAM_MEDIAMODE_TYPE_E enType = PDT_PARAM_MEDIAMODE_TYPE_NORM_REC;
    s32Ret = PDT_PARAM_WorkMode2MediaModeType(enWorkMode, &enType);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    for(i = HI_PDT_MEDIA_VCAP_DEV_MAX_CNT-1; i >= 0 ; i--)
    {
        /*find camera*/
        if(pstParamCfg->astCamMediaCfg[i].s32CamID == pstMediaModeCfg->s32CamID)
        {
            for(j = 0;j < pstParamCfg->astCamMediaCfg[i].u32MediaModeCnt && j < PDT_PARAM_MEDIAMODE_CNT;j++)
            {
                /*find mediamode*/
                if(pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].enMediaMode == pstMediaModeCfg->enMediaMode)
                {
                    u32VencCnt = pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.u32VencCnt;
                    for(k = 0;k < u32VencCnt && k < HI_PDT_VCAPDEV_VENC_MAX_CNT;k++)
                    {
                        /*find main stream venc*/
                        if(HI_PDT_MEDIA_VENC_TYPE_REC == pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k].enVencType)
                        {
                            MLOGD("venc index [%d] \n",k);
                            s32Ret = PDT_PARAM_GetVencCfgByPayloadType(enPayload,
                                &pstParamCfg->stMediaCommCfg.stMainVideoVencAttr,
                                &pstParamCfg->astCamMediaCfg[i].stMediaCommCfg[enType].stVencCfg.astVencAttr[k],
                                &pstParamCfg->astCamMediaCfg[i].astMediaModeSpecCfg[j].stVencCfg.astVencAttr[k],
                                pstVencCfg);
                            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
                            break; /** the first venc found is main venc */
                        }
                    }
                }
            }
        }
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);

    PDT_PARAM_DebugVencCfg(pstVencCfg);
    return HI_SUCCESS;
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
        case HI_PDT_WORKMODE_PARKING_REC:
            PDT_PARAM_GetNormRecCfg(&pstCfg->unModeCfg.stNormRecAttr);
            break;
        case HI_PDT_WORKMODE_PHOTO:
            PDT_PARAM_GetPhotoCfg(&pstCfg->unModeCfg.stPhotoCfg);
            break;
        case HI_PDT_WORKMODE_PLAYBACK:
            memcpy(&pstCfg->unModeCfg.stPlayBackCfg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stPlayBackCfg,
                sizeof(HI_PDT_PARAM_PLAYBACK_CFG_S));
            PDT_PARAM_DebugPlayBackCfg(&pstCfg->unModeCfg.stPlayBackCfg);
            break;
        case HI_PDT_WORKMODE_UVC:
            memcpy(&pstCfg->unModeCfg.stUvcCfg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUvcCfg.stUvcCfg,
                sizeof(HI_PDT_PARAM_UVC_CFG_S));
            PDT_PARAM_DebugUVCCfg(&pstCfg->unModeCfg.stUvcCfg);
            break;
        case HI_PDT_WORKMODE_USB_STORAGE:
            memcpy(&pstCfg->unModeCfg.stUsbStorageCfg, &PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUsbStorageCfg,
                sizeof(HI_USB_STORAGE_CFG_S));
            PDT_PARAM_DebugUSBStorageCfg(&pstCfg->unModeCfg.stUsbStorageCfg);
            break;
        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGW("Unsupport workmode[%s]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}


HI_S32 HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR((s32CamID >= 0)&&(s32CamID < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT) ,HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret;
    PDT_PARAM_MEDIAMODE_TYPE_E enMediaModeType = PDT_PARAM_MEDIAMODE_TYPE_NORM_REC;
    s32Ret = PDT_PARAM_WorkMode2MediaModeType(enWorkMode, &enMediaModeType);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_CAM_STATUS:
            s32Ret = PDT_PARAM_GetCamStatus(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_PARAM_GetMediaMode(enWorkMode, s32CamID, (HI_PDT_MEDIAMODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_PARAM_GetEncPayloadType(s32CamID, enMediaModeType, (HI_MAPI_PAYLOAD_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_APK_VENCID:
            s32Ret = PDT_PARAM_GetApkVencId(s32CamID, enMediaModeType, (HI_HANDLE*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_PARAM_GetFlip(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_MIRROR:
            s32Ret = PDT_PARAM_GetMirror(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_PARAM_GetLDC(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_REC:
            s32Ret = PDT_PARAM_GetRecEnable(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_PARAM_GetOSD(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_CROP:
            s32Ret = PDT_PARAM_GetCrop(s32CamID, enMediaModeType, (HI_PDT_MEDIA_DISP_WND_CROP_CFG_S*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_WDR:
            s32Ret = PDT_PARAM_GetWDR(s32CamID, enMediaModeType, (WDR_MODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            s32Ret = PDT_PARAM_GetVideoMode(s32CamID, enMediaModeType, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_SPLITTIME:
            s32Ret = PDT_PARAM_GetRecSplitTime(s32CamID,(HI_S32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret,s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret =  PDT_PARAM_GetLapseRecIntervalTime(s32CamID,(HI_S32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_RECORD_TYPE:
            s32Ret = PDT_PARAM_GetRecordType(s32CamID,(HI_S32*)pvParam);
            break;
#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
            s32Ret = PDT_PARAM_GetMdSensitivity(s32CamID, enMediaModeType, (HI_U32*)pvParam);
            break;
#endif

        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}


HI_S32 HI_PDT_PARAM_SetCamParam(HI_PDT_WORKMODE_E enWorkMode,HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR((s32CamID >= 0)&&(s32CamID < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT) ,HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_S32 s32Ret;
    PDT_PARAM_MEDIAMODE_TYPE_E enMediaModeType = PDT_PARAM_MEDIAMODE_TYPE_NORM_REC;
    s32Ret = PDT_PARAM_WorkMode2MediaModeType(enWorkMode, &enMediaModeType);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
         case HI_PDT_PARAM_TYPE_CAM_STATUS:
            s32Ret = PDT_PARAM_SetCamStatus(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_PARAM_SetMediaMode(enWorkMode, s32CamID, (HI_PDT_MEDIAMODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_PARAM_SetEncPayloadType(s32CamID, enMediaModeType, (HI_MAPI_PAYLOAD_TYPE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_PARAM_SetFlip(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_MIRROR:
            s32Ret = PDT_PARAM_SetMirror(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_PARAM_SetLDC(s32CamID, enMediaModeType, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_REC:
            s32Ret = PDT_PARAM_SetRecEnable(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_PARAM_SetOSD(s32CamID, (HI_BOOL*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_CROP:
            s32Ret = PDT_PARAM_SetCrop(s32CamID, enMediaModeType, (HI_PDT_MEDIA_DISP_WND_CROP_CFG_S*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_WDR:
            s32Ret = PDT_PARAM_SetWDR(s32CamID, enMediaModeType, (WDR_MODE_E*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            s32Ret = PDT_PARAM_SetVideoMode(s32CamID, enMediaModeType, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
         case HI_PDT_PARAM_TYPE_SPLITTIME:
            s32Ret = PDT_PARAM_SetRecSplitTime(s32CamID,(HI_S32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret,s32Ret);
            break;
         case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_PARAM_SetLapseRecIntervalTime(s32CamID,(HI_S32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;

        case HI_PDT_PARAM_RECORD_TYPE:
            s32Ret = PDT_PARAM_SetRecordType(s32CamID,(HI_S32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;

#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
            s32Ret = PDT_PARAM_SetMdSensitivity(s32CamID, enMediaModeType, (HI_U32*)pvParam);
            PDT_PARAM_CHECK_RETURN_WITH_UNLOCK(s32Ret, s32Ret);
            break;
#endif

        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
#if !(defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
    PDT_PARAM_SetSaveFlg();
#endif

    return HI_SUCCESS;
}


HI_S32 HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_E enType, HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
            *(HI_PDT_WORKMODE_E*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronWorkMode;
            MLOGD("PoweronWorkMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(*(HI_PDT_WORKMODE_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
            {
                HI_PDT_MEDIA_ACAP_CFG_S *pstAcapCfg = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAcapCfg[0];
                HI_PDT_MEDIA_AENC_CFG_S *pstAencCfg = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAencCfg[0];
                if(pstAcapCfg->bEnable == pstAencCfg->bEnable)
                {
                    *(HI_BOOL*)pvParam = pstAcapCfg->bEnable;
                }
                else
                {
                    MLOGE("acap Enable[%d] is not equal to aenc Enable[%d] \n",pstAcapCfg->bEnable,pstAencCfg->bEnable);
                    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
                    return HI_PDT_PARAM_EINVAL;
                }
                MLOGD("audio Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            }
            break;
        case HI_PDT_PARAM_TYPE_PREVIEW_CAMID:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astDispCfg[0].astWndCfg[0].s32PreViewCamID;
            MLOGD("PreViewCamID[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_WIFI:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiInfo, sizeof(HI_PDT_WIFI_INFO_S));
            PDT_PARAM_DebugWiFiInfo((HI_PDT_WIFI_INFO_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_WIFI_AP:
        {
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiApModeCfg.stWiFiApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
            PDT_PARAM_UpdateWifiApCfg((HI_HAL_WIFI_APMODE_CFG_S*)pvParam);
            PDT_PARAM_DebugWiFiAPCfg((HI_HAL_WIFI_APMODE_CFG_S*)pvParam);
            break;
        }
        case HI_PDT_PARAM_TYPE_DEV_INFO:
            memcpy(pvParam, &PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stDevInfo, sizeof(HI_PDT_DEV_INFO_S));
            PDT_PARAM_DebugDevInfo((HI_PDT_DEV_INFO_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_VOLUME:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAoCfg[0].s32Volume;
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
        case HI_PDT_PARAM_TYPE_KEYTONE:
            *(HI_BOOL *)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bKeyToneEnable;
            MLOGD("KeyTone Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
            *(HI_BOOL *)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bBootSoundEnable;
            MLOGD("BootSound Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32GsensorSensitivity;
            MLOGD("gsensor sensitivity[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ParkingLevel;
            MLOGD("gsensor parking sensitivity[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ParkingCollision;
            MLOGD("gsensor parking Collision count[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            *(HI_USB_MODE_E*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUsbCommAttr.enUsbMode;
            MLOGD("usb mode[%d]\n\n", *(HI_USB_MODE_E*)pvParam);
            break;

        case HI_PDT_PARAM_TYPE_LANGUAGE:
            *(HI_S32*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SystemLanguage;
            MLOGD("SystemLanguage[%d]\n\n", *(HI_S32 *)pvParam);
            break;

#ifndef ONE_SENSOR_CONNECT
        case HI_PDT_PARAM_TYPE_PREVIEWPIP:
            *(HI_BOOL*)pvParam = PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astDispCfg[0].astWndCfg[1].bEnable;
            MLOGD("PreviewPIP enable[%d]\n\n", *(HI_BOOL*)pvParam);
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

HI_S32 HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_E enType, const HI_VOID *pvParam)
{
    HI_APPCOMM_CHECK_POINTER(pvParam, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);

    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_POWERON_WORKMODE:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stCommCfg.enPoweronWorkMode = *(HI_PDT_WORKMODE_E*)pvParam;
            MLOGD("PoweronWorkMode[%s]\n\n", PDT_PARAM_GetWorkModeStr(*(HI_PDT_WORKMODE_E*)pvParam));
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
            {
                HI_PDT_MEDIA_ACAP_CFG_S *pstAcapCfg = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAcapCfg[0];
                HI_PDT_MEDIA_AENC_CFG_S *pstAencCfg = &PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAencCfg[0];
                pstAcapCfg->bEnable = *(HI_BOOL*)pvParam;
                pstAencCfg->bEnable = *(HI_BOOL*)pvParam;
                MLOGD("audio Enable[%d]\n\n", pstAcapCfg->bEnable);
            }
            break;
        case HI_PDT_PARAM_TYPE_WIFI:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiInfo, pvParam, sizeof(HI_PDT_WIFI_INFO_S));
            PDT_PARAM_DebugWiFiInfo((HI_PDT_WIFI_INFO_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_WIFI_AP:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stWiFiApModeCfg.stWiFiApCfg, pvParam, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
            PDT_PARAM_DebugWiFiAPCfg((HI_HAL_WIFI_APMODE_CFG_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_VOLUME:
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astAoCfg[0].s32Volume = *(HI_S32*)pvParam;
            MLOGD("Volume[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ScreenBrightness = *(HI_S32*)pvParam;
            MLOGD("ScreenBrightness[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            memcpy(&PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.stScreenDormantAttr, pvParam, sizeof(HI_TIMEDTASK_ATTR_S));
            PDT_PARAM_DebugTimedTask("ScreenDormantMode", (HI_TIMEDTASK_ATTR_S*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_KEYTONE:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bKeyToneEnable = *(HI_BOOL *)pvParam;
            MLOGD("KeyTone Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_BOOTSOUND:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.bBootSoundEnable = *(HI_BOOL *)pvParam;
            MLOGD("BootSound Enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32GsensorSensitivity = *(HI_S32*)pvParam;
            MLOGD("gsensor sensitivity[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ParkingLevel = *(HI_S32*)pvParam;
            MLOGD("gsensor parking level[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32ParkingCollision = *(HI_S32*)pvParam;
            MLOGD("gsensor parking collision count[%d]\n\n", *(HI_S32*)pvParam);
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            PDT_PARAM_GetCtx()->pstCfg->stWorkModeCfg.stUsbCommAttr.enUsbMode = *(HI_USB_MODE_E*)pvParam;
            MLOGD("usb mode[%d]\n\n", *(HI_USB_MODE_E*)pvParam);
            break;
         case HI_PDT_PARAM_TYPE_LANGUAGE:
            PDT_PARAM_GetCtx()->pstCfg->stDevMngCfg.s32SystemLanguage = *(HI_S32 *)pvParam;
            MLOGD("SystemLanguage[%d]\n\n", *(HI_S32 *)pvParam);
            break;

#ifndef ONE_SENSOR_CONNECT
         case HI_PDT_PARAM_TYPE_PREVIEWPIP:
            PDT_PARAM_GetCtx()->pstCfg->stMediaCommCfg.astDispCfg[0].astWndCfg[1].bEnable = *(HI_BOOL*)pvParam;
            MLOGD("PreviewPIP enable[%d]\n\n", *(HI_BOOL*)pvParam);
            break;
#endif

        default:
            HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
            MLOGE("Unsupport Common ParamType[%s]\n\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
#if !(defined(AMP_LINUX_HUAWEILITE)&&defined(__HuaweiLite__))
    PDT_PARAM_SetSaveFlg();

    /* publish event */
    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_PDT_PARAM_CHANGE;
    stEvent.arg1 = enType;
    HI_EVTHUB_Publish(&stEvent);
#endif
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_GetMediaModeValues(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    switch(enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astMediaModeValues[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_WORKMODE_PHOTO:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astPhotoMediaModeValues[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        default:
            MLOGE("Unsupport WorkMode[%s] With ParamType[MediaMode]\n", PDT_PARAM_GetWorkModeStr(enWorkMode));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetCamItemValues(HI_PDT_WORKMODE_E enWorkMode, HI_S32 s32CamID, HI_PDT_PARAM_TYPE_E enType, HI_PDT_ITEM_VALUESET_S *pstValueSet)
{
    HI_APPCOMM_CHECK_POINTER(pstValueSet, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_EXPR((s32CamID >=0) && (s32CamID < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT), HI_PDT_PARAM_ENOTINIT);
    HI_APPCOMM_CHECK_EXPR(PDT_PARAM_GetCtx()->bInit, HI_PDT_PARAM_ENOTINIT);
    memset(pstValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));

    switch(enType)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            PDT_PARAM_GetMediaModeValues(enWorkMode, s32CamID, pstValueSet);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astVideoPayloadType[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_WDR:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astWdrState[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astVideoMode[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astLdcState[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astFlipState[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_MIRROR:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.astMirrorState[s32CamID], sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_SPLITTIME:
            memcpy(pstValueSet,&PDT_PARAM_GetCtx()->pstCfg->stItemValues.stRecSplitTime,sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stLapseRecIntervalTime, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_RECORD_TYPE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stRecordType, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;

#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stMdSensitivity, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
#endif
        default:
            MLOGE("Unsupport CamID[%d] With ParamType[%s]\n",s32CamID, PDT_PARAM_GetParamTypeStr(enType));
            return HI_PDT_PARAM_EUNSUPPORT;
    }
    MLOGD("CamID[%d] ParamType[%s]\n", s32CamID, PDT_PARAM_GetParamTypeStr(enType));
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
        case HI_PDT_PARAM_TYPE_VOLUME:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stVolume, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stScreenDormantValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stScreenBrightnessValues, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSensitivityLevel, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_GSENSOR_PARKING:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stParkingLevel, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_USB_MODE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stUsbMode, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;
        case HI_PDT_PARAM_TYPE_LANGUAGE:
            memcpy(pstValueSet, &PDT_PARAM_GetCtx()->pstCfg->stItemValues.stSystemLanguage, sizeof(HI_PDT_ITEM_VALUESET_S));
            break;

        default:
            MLOGE("Unsupport Common ParamType[%s]\n", PDT_PARAM_GetParamTypeStr(enType));
            return HI_FAILURE;
    }
    MLOGD("ParamType[%s]\n", PDT_PARAM_GetParamTypeStr(enType));
    PDT_PARAM_DebugItemValues(pstValueSet);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetLiveSrcByCamID(HI_S32 s32CamID, HI_PDT_MEDIA_BIND_SRC_CFG_S *pstLiveSrc)
{
    HI_APPCOMM_CHECK_EXPR((s32CamID >= 0)&&(s32CamID < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT) ,HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstLiveSrc, HI_PDT_PARAM_EINVAL);

    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;
    HI_S32 i = 0;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    for(i = 0 ;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(i == s32CamID)
        {
            pstLiveSrc->enBindedMod = pstParamCfg->stMediaCommCfg.astLiveSrc[i].enBindedMod;
            pstLiveSrc->ModHdl = pstParamCfg->stMediaCommCfg.astLiveSrc[i].ModHdl;
            pstLiveSrc->ChnHdl = pstParamCfg->stMediaCommCfg.astLiveSrc[i].ChnHdl;
            break;
        }
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetDispSrcByCamID(HI_S32 s32CamID, HI_PDT_MEDIA_BIND_SRC_CFG_S *pstDispSrc)
{
    HI_APPCOMM_CHECK_EXPR((s32CamID >= 0)&&(s32CamID < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT) ,HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDispSrc, HI_PDT_PARAM_EINVAL);

    PDT_PARAM_CFG_S* pstParamCfg = PDT_PARAM_GetCtx()->pstCfg;
    HI_S32 i = 0;
    HI_MUTEX_LOCK(PDT_PARAM_GetCtx()->mutexLock);
    for(i = 0 ;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(i == s32CamID)
        {
            pstDispSrc->enBindedMod = pstParamCfg->stMediaCommCfg.astDispCfg[0].astDispSrc[i].enBindedMod;
            pstDispSrc->ModHdl = pstParamCfg->stMediaCommCfg.astDispCfg[0].astDispSrc[i].ModHdl;
            pstDispSrc->ChnHdl = pstParamCfg->stMediaCommCfg.astDispCfg[0].astDispSrc[i].ChnHdl;
            break;
        }
    }
    HI_MUTEX_UNLOCK(PDT_PARAM_GetCtx()->mutexLock);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetMediaModeCfg(HI_PDT_WORKMODE_E enWorkMode,HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S *pstCamCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCamCfg, HI_PDT_PARAM_EINVAL);
    HI_S32 s32Ret = HI_FAILURE,i = 0;
    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, i, HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);
        PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get mediamode configure");
        pstCamCfg->astMediaModeCfg[i].s32CamID = i;
        pstCamCfg->astMediaModeCfg[i].enMediaMode = enMediaMode;
    }
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetExifInfo(HI_MAPI_SNAP_EXIF_INFO_S* pstExifInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstExifInfo, HI_PDT_PARAM_EINVAL);
    memset(pstExifInfo, 0, sizeof(HI_MAPI_SNAP_EXIF_INFO_S));

    /*fill exif info*/
    snprintf((HI_CHAR*)pstExifInfo->au8ImageDescription,
             HI_MAPI_EXIF_DRSCRIPTION_LENGTH, "single Photo");
    snprintf((HI_CHAR*)pstExifInfo->au8Make,
             HI_MAPI_EXIF_DRSCRIPTION_LENGTH, "Hisilicon");
    snprintf((HI_CHAR*)pstExifInfo->au8Model,
             HI_MAPI_EXIF_DRSCRIPTION_LENGTH, "DCIM");
    snprintf((HI_CHAR*)pstExifInfo->au8Software,
             HI_MAPI_EXIF_DRSCRIPTION_LENGTH, "HiMobileCam");

    pstExifInfo->stApertureInfo.u32MinIrisFNOTarget = 1;
    pstExifInfo->stApertureInfo.u32MaxIrisFNOTarget = 1024;
    pstExifInfo->stGPSInfo.chGPSLatitudeRef = 'N';
    pstExifInfo->stGPSInfo.au32GPSLatitude[0][1] = 1;
    pstExifInfo->stGPSInfo.au32GPSLatitude[1][1] = 1;
    pstExifInfo->stGPSInfo.au32GPSLatitude[2][1] = 1;
    pstExifInfo->stGPSInfo.chGPSLongitudeRef = 'E';
    pstExifInfo->stGPSInfo.au32GPSLongitude[0][1] = 1;
    pstExifInfo->stGPSInfo.au32GPSLongitude[1][1] = 1;
    pstExifInfo->stGPSInfo.au32GPSLongitude[2][1] = 1;
    pstExifInfo->stGPSInfo.au32GPSAltitude[1] = 1;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetMediaCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_CAM_CONTEXT_S *pstCamCtx,HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCamCtx, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_PARAM_EINVAL);
    HI_PDT_PARAM_CAM_MEDIA_CFG_S astCamCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_MEDIA_COMM_CFG_S stCommCfg;
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S astSceneVcapAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];

    HI_S32 s32Ret = HI_FAILURE;
    HI_U32 i = 0,j = 0;
    HI_U32 u32VpssIdx = 0,u32VpssCnt = 0;
    HI_U32 u32VencIdx = 0,u32VencCnt = 0;
    memset(pstMediaCfg, 0, sizeof(HI_PDT_MEDIA_CFG_S));

    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        memset(&astCamCfg[i],'\0',sizeof(HI_PDT_PARAM_CAM_MEDIA_CFG_S));
        if(pstCamCtx[i].bEnable)
        {
            s32Ret = HI_PDT_PARAM_GetCamMediaCfg(enWorkMode, &pstCamCtx[i].stMediaModeCfg, &astCamCfg[i],&astSceneVcapAttr[i]);
            PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get cam media configure");
        }
    }

    s32Ret = HI_PDT_PARAM_GetOsdCfg(&pstMediaCfg->stVideoCfg.stOsdCfg);
    PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get osd configure");

    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        /*get vb configure*/
        if(pstCamCtx[i].bEnable)
        {
            /*get vi_vpss mode*/
            s32Ret = PDT_PARAM_GetViVpssMode(pstCamCtx[i].bEnable,astCamCfg[i].astMode,pstMediaCfg->stViVpssMode.astMode[i]);
            PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get vivpss mode");

            s32Ret = PDT_PARAM_GetVBCfg(&astCamCfg[i].stVBCfg, &pstMediaCfg->stVBCfg);
            PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get vb configure");
        }

        /*get vcap dev configure*/
        s32Ret = PDT_PARAM_GetVcapDevAttr(pstCamCtx[i].bEnable, &astCamCfg[i].stVcapDevAttr,&pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i]);
        PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,s32Ret,"get vcap dev attr");

        if (HI_FALSE == pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[0].bIspBypass)
        {
            /**load exif info*/
            HI_PDT_PARAM_GetExifInfo(&pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].stExifInfo);
        }

        if(pstCamCtx[i].bEnable)
        {
            /*load vpss attr*/
            u32VpssCnt += astCamCfg[i].u32VpssCnt;
            if(u32VpssCnt > HI_PDT_MEDIA_VPSS_MAX_CNT)
            {
                MLOGE("vpsscnt[%u] out of max[%d] \n",u32VpssCnt,HI_PDT_MEDIA_VPSS_MAX_CNT);
                return HI_PDT_PARAM_EINVAL;
            }
            memcpy(&pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[u32VpssIdx],&astCamCfg[i].astVpssCfg,sizeof(HI_PDT_MEDIA_VPSS_ATTR_S) * astCamCfg[i].u32VpssCnt);
            u32VpssIdx += astCamCfg[i].u32VpssCnt;

            /*load venc attr*/
            u32VencCnt += astCamCfg[i].u32VencCnt;
            if(u32VencCnt > HI_PDT_MEDIA_VENC_MAX_CNT)
            {
                MLOGE("venccnt[%u] out of max[%d] \n",u32VencCnt,HI_PDT_MEDIA_VENC_MAX_CNT);
                return HI_PDT_PARAM_EINVAL;
            }
            memcpy(&pstMediaCfg->stVideoCfg.astVencCfg[u32VencIdx],&astCamCfg[i].astVencCfg,sizeof(HI_PDT_MEDIA_VENC_CFG_S) * astCamCfg[i].u32VencCnt);
            u32VencIdx += astCamCfg[i].u32VencCnt;
        }
        PDT_PARAM_UpdateMediaOsdCfg(i, astCamCfg[i].bOsdShow, pstMediaCfg);
    }

    /*fill not used vpss*/
    while(u32VpssIdx < HI_PDT_MEDIA_VPSS_MAX_CNT)
    {
        pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[u32VpssIdx].bEnable = HI_FALSE;
        u32VpssIdx++;
    }
    /*fill not used venc*/
    while(u32VencIdx < HI_PDT_MEDIA_VENC_MAX_CNT)
    {
        pstMediaCfg->stVideoCfg.astVencCfg[u32VencIdx].bEnable = HI_FALSE;
        u32VencIdx++;
    }

    /*load media comm configure*/
    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stCommCfg);
    PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret,HI_PDT_PARAM_EINVAL,"get media comm configure");

    /*config vo_crop Attr */
    for(i=0;i<HI_PDT_MEDIA_DISP_MAX_CNT;i++)
    {
        for(j=0;j<HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;j++)
        {
            HI_PDT_MEDIA_DISP_WND_CFG_S *pstMediaDispWnd = &stCommCfg.stVideoOutCfg.astDispCfg[i].astWndCfg[j];
            if(!pstCamCtx[j].bEnable)
            {
                pstMediaDispWnd->bEnable = HI_FALSE;
                continue;
            }
            memcpy(&pstMediaDispWnd->stCropCfg,&astCamCfg[j].stCropCfg,sizeof(HI_PDT_MEDIA_DISP_WND_CROP_CFG_S));
        }
    }

    /*load disp configure*/
    for(i = 0;i < HI_PDT_MEDIA_DISP_MAX_CNT;i++)
    {
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].bEnable = stCommCfg.stVideoOutCfg.astDispCfg[i].bEnable;
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].VdispHdl = stCommCfg.stVideoOutCfg.astDispCfg[i].VdispHdl;
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].stDispAttr.stPubAttr.u32BgColor = stCommCfg.stVideoOutCfg.astDispCfg[i].u32BgColor;
        memset(&pstMediaCfg->stVideoOutCfg.astDispCfg[i].stDispAttr.stUserInfoAttr.stUserInfo,0,sizeof(VO_USER_INTFSYNC_INFO_S));
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].stDispAttr.stUserInfoAttr.u32DevFrameRate = 0;
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].stCscAttr = stCommCfg.stVideoOutCfg.astDispCfg[i].stCscAttr;
        memcpy(pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg,stCommCfg.stVideoOutCfg.astDispCfg[i].astWndCfg,sizeof(HI_PDT_MEDIA_DISP_WND_CFG_S)*HI_PDT_MEDIA_DISP_WND_MAX_CNT);
    }

    /*load audio configure*/
    memcpy(&pstMediaCfg->stAudioCfg,&stCommCfg.stAudioCfg,sizeof(HI_PDT_MEDIA_AUDIO_CFG_S));

    /*load audio out configure*/
    memcpy(&pstMediaCfg->stAudioOutCfg,&stCommCfg.stAudioOutCfg,sizeof(HI_PDT_MEDIA_AUDIOOUT_CFG_S));

    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_GetSceneCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_CAM_CONTEXT_S *pstCamCtx,HI_PDT_SCENE_MODE_S *pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstCamCtx, HI_PDT_PARAM_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_PARAM_EINVAL);

    HI_PDT_PARAM_CAM_MEDIA_CFG_S astCamCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S astSceneVcapAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];

    HI_S32 i = 0,j = 0,s32Ret = HI_FAILURE;

    /*load scenemode pipeattr*/
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        if(pstCamCtx[i].bEnable)
        {
            s32Ret = HI_PDT_PARAM_GetCamMediaCfg(enWorkMode, &pstCamCtx[i].stMediaModeCfg, &astCamCfg[i],&astSceneVcapAttr[i]);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("get cam[%d] media configure failed \n",i);
                return HI_PDT_PARAM_EINVAL;
            }

            if(j < HI_PDT_SCENE_PIPE_MAX_NUM)
            {
                pstSceneMode->astPipeAttr[j].bBypassIsp = astSceneVcapAttr[i].bIspBypass;
                if(HI_TRUE != pstSceneMode->astPipeAttr[j].bBypassIsp)
                    pstSceneMode->astPipeAttr[j].bEnable = HI_TRUE;
                else
                    pstSceneMode->astPipeAttr[j].bEnable = HI_FALSE;
                pstSceneMode->astPipeAttr[j].MainPipeHdl = astSceneVcapAttr[i].MainPipeHdl;
                pstSceneMode->astPipeAttr[j].VcapPipeHdl = astSceneVcapAttr[i].VcapPipeHdl;
                pstSceneMode->astPipeAttr[j].PipeChnHdl = astSceneVcapAttr[i].PipeChnHdl;
                pstSceneMode->astPipeAttr[j].u8PipeParamIndex = astSceneVcapAttr[i].u8PipeParamIndex;
                pstSceneMode->astPipeAttr[j].enPipeType = astSceneVcapAttr[i].enPipeType;
                pstSceneMode->astPipeAttr[j].enPipeMode = astSceneVcapAttr[i].enPipeMode;
                pstSceneMode->astPipeAttr[j].VpssHdl = astSceneVcapAttr[i].VpssHdl;
                pstSceneMode->astPipeAttr[j].VPortHdl = astSceneVcapAttr[i].VportHdl;
                pstSceneMode->astPipeAttr[j].VencHdl = astSceneVcapAttr[i].VencHdl;
                j++;
            }
            else
            {
                MLOGE(RED" index[%d] is equal or  larger than HI_PDT_SCENE_PIPE_MAX_NUM  \n\n"NONE,j);
                return HI_PDT_PARAM_EINVAL;
            }
        }
    }

    for(;j < HI_PDT_SCENE_PIPE_MAX_NUM;j++)
    {
        pstSceneMode->astPipeAttr[j].bEnable = HI_FALSE;
    }
    PDT_PARAM_UpdateSceneMode(enWorkMode,pstSceneMode);

    return HI_SUCCESS;
}


HI_S32 HI_PDT_PARAM_RecordUpdateMediaCfg(HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 i = 0, j = 0, s32Ret = HI_FAILURE;
    HI_BOOL bCheck = HI_FALSE;
    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    HI_PDT_PARAM_RECMNG_ATTR_S* pstRecMngAttr;
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_PARAM_EINVAL);
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_NORM_REC, &stEnterWorkModeCfg);
    PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");

    /*LAPSE: update venc attr */
   for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
   {
        pstRecMngAttr = &stEnterWorkModeCfg.unModeCfg.stNormRecAttr.astRecMngAttr[i];
        if (HI_REC_TYPE_LAPSE == pstRecMngAttr->enRecType)
        {
            if (pstRecMngAttr->u32Interval_ms < 64 * 100)
            {
                for (j = 0; j < HI_PDT_MEDIA_VENC_MAX_CNT; j++)
                {
                    if(HI_TRUE != pstMediaCfg->stVideoCfg.astVencCfg[j].bEnable)
                    {
                       continue;
                    }
                    if((pstMediaCfg->stVideoCfg.astVencCfg[j].ModHdl !=
                       pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].VcapDevHdl)  ||
                       (!pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable))
                    {
                       continue;
                    }
                    bCheck = PDT_PARAM_IsRecStreamCheck(pstMediaCfg->stVideoCfg.astVencCfg[j].VencHdl);
                    if(HI_FALSE == bCheck)
                    {
                        continue;
                    }
                   s32Ret = PDT_PARAM_UpdateLapseRecVencAttr(pstRecMngAttr,pstMediaCfg,j);
                   PDT_PARAM_CHECK_RETURN_WITH_ERRSTRING(s32Ret, HI_PDT_PARAM_EINVAL, "update venc attr");
               }
           }
        }
   }
   return HI_SUCCESS;

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

