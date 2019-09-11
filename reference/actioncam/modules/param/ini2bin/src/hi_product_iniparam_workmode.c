/**
 * @file      hi_product_iniparam_workmode.c
 * @brief     load workmode parameter
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hi_product_iniparam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


static HI_S32 PDT_INIPARAM_WORKMODE_LoadVideoProtuneCfg(const HI_CHAR *pszIniModule,
                HI_PDT_VIDEO_PROTUNE_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Value = 0;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:whitebalance");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WB);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:iso");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32ISO);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:ev");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enEV);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:metry_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enMetryType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:metry_average_param_idx");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u16MetryAverageParamIdx = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.video.protune:metry_center_param_idx");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u16MetryCenterParamIdx = s32Value;

    MLOGD("VideoProtune  WB[%u:0-auto], ISO[%u:0-auto]\n", pstParam->u32WB, pstParam->u32ISO);
    MLOGD("      EV[%d:0-B2,1-B1x5,2-B1,3-B0x5,4-0,5-0x5,6-1,7-1x5,8-2]\n", pstParam->enEV);
    MLOGD("      Metry[%d:0-Center,1-Average,2-Spot] AverageParamIdx[%u] CenterParamIdx[%u]\n",
        pstParam->enMetryType, pstParam->u16MetryAverageParamIdx, pstParam->u16MetryCenterParamIdx);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoProtuneCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PHOTO_PROTUNE_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Value = 0;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:whitebalance");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WB);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:iso");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32ISO);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:ev");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enEV);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:exposure_time");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32ExpTime_us);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:metry_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enMetryType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:metry_average_param_idx");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u16MetryAverageParamIdx = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.photo.protune:metry_center_param_idx");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u16MetryCenterParamIdx = s32Value;

    MLOGD("PhotoProtune  WB[%u:0-auto], ISO[%u:0-auto]\n", pstParam->u32WB, pstParam->u32ISO);
    MLOGD("      EV[%d:0-B2,1-B1x5,2-B1,3-B0x5,4-0,5-0x5,6-1,7-1x5,8-2], exptime[%uus:0-auto]\n",
        pstParam->enEV, pstParam->u32ExpTime_us);
    MLOGD("      Metry[%d:0-Center,1-Average,2-Spot] AverageParamIdx[%u] CenterParamIdx[%u]\n",
        pstParam->enMetryType, pstParam->u16MetryAverageParamIdx, pstParam->u16MetryCenterParamIdx);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_COMM_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:poweron_workmode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enPoweronWorkMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:poweron_action");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enPoweronAction);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("PowerOnWorkMode[%d:], PowerOnAction[%d:]\n", pstParam->enPoweronWorkMode, pstParam->enPoweronAction);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:usb_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enUsbMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("enUsbMode[%d:]\n", pstParam->enUsbMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.record:muxer_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRecMuxerType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.record:thumbnail_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRecThmType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("RecMuxerType[%d:0-MP4,1-MOV,2-TS], ThmType[%d:0-Independent,1-Embedded]\n",
        pstParam->enRecMuxerType, pstParam->enRecThmType);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.record:repair_unit");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32RepairUnit);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u32RepairUnit <<= 10;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.record:pre_alloc_unit");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocUnit);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u32PreAllocUnit <<= 10;
    MLOGD("FileRepairUnit[%uMB], PreAllocUnit[%uMB]\n", pstParam->u32RepairUnit >> 20, pstParam->u32PreAllocUnit >> 20);

    /* Load Video Protune Configure */
    s32Ret = PDT_INIPARAM_WORKMODE_LoadVideoProtuneCfg(pszIniModule, &pstParam->stVideoProTune);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VideoProtune");

    /* Load Photo Protune Configure */
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoProtuneCfg(pszIniModule, &pstParam->stPhotoProTune);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoProtune");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecDataSrcCfg(const HI_CHAR *pszIniModule,
                HI_PDT_REC_SRC_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Idx;

    for (s32Idx = 0; s32Idx < HI_PDT_REC_VSTREAM_MAX_CNT; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "recsource:venc_hdl%d", s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->aVencHdl[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("RecStream[%d] venchdl[%d]\n", s32Idx, pstParam->aVencHdl[s32Idx]);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "recsource:aenc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "recsource:thumbnail_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ThmHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("aenchdl[%d] thmhdl[%d]\n", pstParam->AencHdl, pstParam->ThmHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoDataSrcCfg(const HI_CHAR *pszIniModule,
                HI_PHOTOMNG_PHOTO_SRC_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:vcap_pipe_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:vcap_pipechn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:vproc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VprocHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:vport_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:venc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "photosource:thumbnail_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencThmHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("VcapPipeHdl[%d] VcapPipeChnHdl[%d] VprocHdl[%d] VportHdl[%d] venchdl[%d] thmhdl[%d]\n",
        pstParam->VcapPipeHdl, pstParam->VcapPipeChnHdl, pstParam->VprocHdl,
        pstParam->VportHdl, pstParam->VencHdl, pstParam->VencThmHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadTypeCommVcapDevAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_WORKMODE_COMM_VCAP_DEV_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32VcapPipeIdx, s32VcapPipeChnIdx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d:enable", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("vcapdev[%d] enable[%d]\n", s32Idx, pstParam->bEnable);

    PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_S* pstVcapPipeAttr = NULL;
    PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_CHN_S* pstVcapPipeChnAttr = NULL;
    for (s32VcapPipeIdx = 0; s32VcapPipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32VcapPipeIdx)
    {
        pstVcapPipeAttr = &pstParam->astVcapPipeAttr[s32VcapPipeIdx];

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d:enable",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->bEnable);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d:type",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->enPipeType);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d:isp_bypass",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->bIspBypass);
        MLOGD("    pipe[%d] enable[%d] pipeType[%d:0-video,1-snap] IspBypass[%d]\n", s32VcapPipeIdx,
            pstVcapPipeAttr->bEnable, pstVcapPipeAttr->enPipeType, pstVcapPipeAttr->bIspBypass);

        /* Scene Configure */
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:vpss_bypass",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.bVpssBypass);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:pipe_chn_hdl",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.PipeChnHdl);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:main_pipe_hdl",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.MainPipeHdl);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:pipe_vpss_hdl",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.VpssHdl);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:pipe_vport_hdl",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.VPortHdl);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.scene:pipe_venc_hdl",
            s32Idx, s32VcapPipeIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstVcapPipeAttr->stSceneCfg.VencHdl);

        MLOGD("    Scene: VpssBypass[%d] PipeChnHdl[%d] MainPipeHdl[%d] VpssHdl[%d] VportHdl[%d] VencHdl[%d]\n",
            pstVcapPipeAttr->stSceneCfg.bVpssBypass, pstVcapPipeAttr->stSceneCfg.PipeChnHdl,
            pstVcapPipeAttr->stSceneCfg.MainPipeHdl, pstVcapPipeAttr->stSceneCfg.VpssHdl,
            pstVcapPipeAttr->stSceneCfg.VPortHdl, pstVcapPipeAttr->stSceneCfg.VencHdl);

        for (s32VcapPipeChnIdx = 0; s32VcapPipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32VcapPipeChnIdx)
        {
            pstVcapPipeChnAttr = &pstVcapPipeAttr->astPipeChnAttr[s32VcapPipeChnIdx];

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                "vcap.%d.pipe.%d.chn.%d:enable",
                s32Idx, s32VcapPipeIdx, s32VcapPipeChnIdx);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstVcapPipeChnAttr->bEnable);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                "vcap.%d.pipe.%d.chn.%d:pixel_fmt",
                s32Idx, s32VcapPipeIdx, s32VcapPipeChnIdx);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstVcapPipeChnAttr->enPixelFormat);
            MLOGD("        pipechn[%d] enable[%d] pixelformat[%d:0-YUV420,1-YUV422]\n",
                s32VcapPipeChnIdx, pstVcapPipeChnAttr->bEnable, pstVcapPipeChnAttr->enPixelFormat);
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadTypeCommVpssAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_WORKMODE_COMM_VPSS_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32VportIdx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d:enable", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d:pixel_fmt", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enPixelFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vpss[%d] enable[%d] PixelFormat[%d:0-YUV420,1-YUV422]\n", s32Idx, pstParam->bEnable, pstParam->enPixelFormat);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.nr:enable", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bNrEn);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.nr:type", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stNrAttr.enNrType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.nr:compress_mode", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stNrAttr.enCompressMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.nr:motion_mode", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stNrAttr.enNrMotionMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD(" Nr: enable[%d] Type[%d:0-video,1-snap] compress_mode[%d:] motion_mode[%d:]\n", pstParam->bNrEn,
        pstParam->stNrAttr.enNrType, pstParam->stNrAttr.enCompressMode, pstParam->stNrAttr.enNrMotionMode);

    /* Load Vpss Port Configure */
    PDT_PARAM_WORKMODE_COMM_VPSS_PORT_ATTR_S* pstVpssPortAttr = NULL;
    for (s32VportIdx = 0; s32VportIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VportIdx)
    {
        pstVpssPortAttr = &pstParam->astPortAttr[s32VportIdx];
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
            "vpss.%d.vport.%d:enable", s32Idx, s32VportIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstVpssPortAttr->bEnable);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
            "vpss.%d.vport.%d:pixel_fmt", s32Idx, s32VportIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstVpssPortAttr->enPixelFormat);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
            "vpss.%d.vport.%d:video_fmt", s32Idx, s32VportIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstVpssPortAttr->enVideoFormat);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
            "vpss.%d.vport.%d:mode", s32Idx, s32VportIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstVpssPortAttr->stAspectRatio.enMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
            "vpss.%d.vport.%d:bg_color", s32Idx, s32VportIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstVpssPortAttr->stAspectRatio.u32BgColor);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("    port[%d] enable[%d] PixelFormat[%d:0-YUV420,1-YUV422] VideoFormat[%d:0-LINEAR] aspectMode[%d], aspectBgColor[%u]\n",
            s32VportIdx, pstVpssPortAttr->bEnable,
            pstVpssPortAttr->enPixelFormat, pstVpssPortAttr->enVideoFormat,
            pstVpssPortAttr->stAspectRatio.enMode, pstVpssPortAttr->stAspectRatio.u32BgColor);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadTypeCommVencCfg(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_WORKMODE_COMM_VENC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:enable", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:venc_hdl", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:binded_mod", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBindedMod);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:mod_hdl", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:chn_hdl", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d:type", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enVencType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("Venc[%d] enable[%d] venchdl[%d] bindedmod[%d:0-vcap,1-vpss] modhdl[%d] chnhdl[%d]\n",
        s32Idx, pstParam->bEnable, pstParam->VencHdl, pstParam->enBindedMod, pstParam->ModHdl, pstParam->ChnHdl);
    MLOGD("      VencType[%d:0-MainVideo,1-SubVideo,2-SingleSnap,3-MultiSnap,4-SubSnap]\n", pstParam->enVencType);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_GetCSCMatrixEnum(const HI_CHAR *pszEnumStr,
                VO_CSC_MATRIX_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_IDENTITY))
    {
        *penMode = VO_CSC_MATRIX_IDENTITY;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT601_TO_BT709))
    {
        *penMode = VO_CSC_MATRIX_BT601_TO_BT709;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT709_TO_BT601))
    {
        *penMode = VO_CSC_MATRIX_BT709_TO_BT601;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT601_TO_RGB_PC))
    {
        *penMode = VO_CSC_MATRIX_BT601_TO_RGB_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT709_TO_RGB_PC))
    {
        *penMode = VO_CSC_MATRIX_BT709_TO_RGB_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_RGB_TO_BT601_PC))
    {
        *penMode = VO_CSC_MATRIX_RGB_TO_BT601_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_RGB_TO_BT709_PC))
    {
        *penMode = VO_CSC_MATRIX_RGB_TO_BT709_PC;
    }
    else
    {
        MLOGE("Invalid CSCMatrix[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("    CSCMatrix[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadDispCSCAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_MAPI_DISP_CSCATTREX_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:cscmatrix", pszPrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_GetCSCMatrixEnum(pszString, &pstParam->stVoCSC.enCscMatrix);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:luma", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Luma);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:contrast", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Contrast);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:hue", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Hue);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:satuature", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Satuature);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("      luma[%u], contrast[%u], hue[%u], satuature[%u]\n", pstParam->stVoCSC.u32Luma,
        pstParam->stVoCSC.u32Contrast, pstParam->stVoCSC.u32Hue, pstParam->stVoCSC.u32Satuature);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadDispWndAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_DISP_WND_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:wnd_hdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->WndHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:binded_mod", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBindedMod);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mod_hdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:chn_hdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    Prefix[%s] enable[%d] wndhdl[%d] bindedmod[%d:0-vcap,1-vpss] modhdl[%d] chnhdl[%d]\n", pszPrefix,
        pstParam->bEnable, pstParam->WndHdl, pstParam->enBindedMod, pstParam->ModHdl, pstParam->ChnHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rotate_type", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    Rotate[%d, 0:0, 1:90, 2:180, 3:270]\n", pstParam->enRotate);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:priority", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stWndAttr.u32Priority);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    WndAttr priority[%u]\n", pstParam->stWndAttr.u32Priority);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadDispAspectAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, ASPECT_RATIO_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mode", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bg_color", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32BgColor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:videorect_x", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.s32X);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:videorect_y", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.s32Y);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:videorect_w", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:videorect_h", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("    aspect mode[%d] bgcolor[%d] videorect Start[%d,%d] size[%ux%u]\n",
        pstParam->enMode, pstParam->u32BgColor, pstParam->stVideoRect.s32X, pstParam->stVideoRect.s32Y,
        pstParam->stVideoRect.u32Width, pstParam->stVideoRect.u32Height);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadDispCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_DISP_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:disp_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VdispHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Display[%d]: enable[%d] disphdl[%d]\n", s32Idx, pstParam->bEnable, pstParam->VdispHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bg_color", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDispAttr.stPubAttr.u32BgColor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vdev_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDispAttr.stUserInfoAttr.u32DevFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD(" bg color[%08x] vdev framerate[%ufps]\n",
        pstParam->stDispAttr.stPubAttr.u32BgColor, pstParam->stDispAttr.stUserInfoAttr.u32DevFrameRate);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.videolayer:vl_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVideoLayerAttr.u32VLFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.videolayer:vl_bufflen", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVideoLayerAttr.u32BufLen);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD(" vl_framerate[%ufps] vl_bufflen[%ufps]\n",
        pstParam->stVideoLayerAttr.u32VLFrameRate, pstParam->stVideoLayerAttr.u32BufLen);

    /* Load CSC Attribute */
    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.csc", s32Idx);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stCscAttr.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    CSCEnable[%d]\n", pstParam->stCscAttr.bEnable);

    s32Ret = PDT_INIPARAM_WORKMODE_LoadDispCSCAttr(pszIniModule, szIniNodePrefix, &pstParam->stCscAttr.stAttrEx);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "CSCAttr");

    /* Load Window Attribute */
    HI_S32 s32WndIdx = 0;
    for (s32WndIdx = 0; s32WndIdx < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++s32WndIdx)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.window.%d", s32Idx, s32WndIdx);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadDispWndAttr(pszIniModule, szIniNodePrefix,
            &pstParam->astWndCfg[s32WndIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "WndAttr");

        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.window.%d.aspect.ratio", s32Idx, s32WndIdx);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadDispAspectAttr(pszIniModule, szIniNodePrefix,
            &pstParam->astWndCfg[s32WndIdx].stAspectRatio);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "AspectAttr");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadAoCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                PDT_PARAM_WORKMODE_COMM_AO_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "ao.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ao_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AoHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ao_chn_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AoChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:i2s_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enI2sType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Ao[%d]: aohdl[%d] enI2sType[%d]\n", s32Idx, pstParam->AoHdl, pstParam->enI2sType);

    return HI_SUCCESS;
}


static HI_S32 PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx;

    /* Load Vcapture Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeCommVcapDevAttr(pszIniModule, s32Idx, &pstParam->stVcapCfg.astVcapDevAttr[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapDev");
    }

    /* Load Vprocessor-Vpss Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VPSS_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeCommVpssAttr(pszIniModule, s32Idx, &pstParam->stVprocCfg.astVpssAttr[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VprocVpss");
    }

    /* Load Vencoder Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeCommVencCfg(pszIniModule, s32Idx, &pstParam->astVencCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Venc");
    }

    /* Load Display Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_DISP_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_LoadDispCfg(pszIniModule, s32Idx, &pstParam->astDispCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Vdisp");
    }

    /* Load Ao Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_AO_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_WORKMODE_LoadAoCfg(pszIniModule, s32Idx, &pstParam->astAoCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Ao");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* Load Record Data Source Configure */
    MLOGD("Record Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecDataSrcCfg(pszIniModule, &pstParam->stDataSrc);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DataSrc");

    /* Load Record Type Media Configure */
    MLOGD("Record Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;

    /* Load Photo Data Source Configure */
    MLOGD("Photo Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoDataSrcCfg(pszIniModule, &pstParam->stDataSrc);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DataSrc");

    /* Load Photo Type Media Configure */
    MLOGD("Photo Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadMPhotoMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_MPHOTO_MEDIA_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;

    /* Load Photo Data Source Configure */
    MLOGD("MPhoto Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoDataSrcCfg(pszIniModule, &pstParam->stDataSrc);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DataSrc");

    /* Load Photo Type Media Configure */
    MLOGD("MPhoto Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecSnapMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_RECSNAP_MEDIA_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;

    /* Load Record Data Source Configure */
    MLOGD("Record Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecDataSrcCfg(pszIniModule, &pstParam->stDataSrc.stRecSrc);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecDataSrc");

    /* Load Photo Data Source Configure */
    MLOGD("Photo Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoDataSrcCfg(pszIniModule, &pstParam->stDataSrc.stPhotoSrc);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoDataSrc");

    /* Load RecSnap Type Media Configure */
    MLOGD("MPhoto Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPlaybackMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_PLAYBACK_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* Load Playback Type Media Configure */
    MLOGD("Playback Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadHDMIPreviewMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_HDMIPREVIEW_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* Load HDMIPreview Type Media Configure */
    MLOGD("HDMIPreview Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadHDMIPlaybackMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_HDMIPLAYBACK_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* Load HDMIPlayback Type Media Configure */
    MLOGD("HDMIPlayback Type Common Media:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadTypeMediaCfg(pszIniModule, &pstParam->stMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TypeMediaCfg");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecCommCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, PDT_PARAM_REC_COMM_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Value;
    HI_S32  s32Idx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mediamode", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:split_type", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stSplitAttr.enSplitType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:split_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stSplitAttr.u32SplitTimeLenSec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:recbuffer_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32BufferTimeMSec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:prerec_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreRecTimeSec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("MediaMode[%d], SplitType[%d:0-none,1-time] SplitTime[%uS] RecBufTime[%uMS] PreRecTime[%uS]\n",
        pstParam->enMediaMode, pstParam->stSplitAttr.enSplitType,
        pstParam->stSplitAttr.u32SplitTimeLenSec, pstParam->u32BufferTimeMSec,
        pstParam->u32PreRecTimeSec);

    for (s32Idx = 0; s32Idx < HI_PDT_REC_VSTREAM_MAX_CNT; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:recvbuf_size%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->au32VBufSize[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->au32VBufSize[s32Idx] <<= 10;
        MLOGD("RecStream[%d] VBufSize[%dKB]\n", s32Idx, pstParam->au32VBufSize[s32Idx] >> 10);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:file_type_idx", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u8FileTypeIdx = s32Value;
    MLOGD("FileTypeIdx[%u]\n", pstParam->u8FileTypeIdx);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadNormRecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_NORM_REC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;

    /* Load Record Common Configure */
    MLOGD("Record Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(pszIniModule, "normal", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecordComm");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadLoopRecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_LOOP_REC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Record Common Configure */
    MLOGD("Record Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(pszIniModule, "loop", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecordComm");

    /* Load LoopTime */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "loop:loop_time");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32LoopTime_min);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Loop Time[%uMin]\n", pstParam->u32LoopTime_min);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadSlowRecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_SLOW_REC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Idx;

    /* Load Record Common Configure */
    MLOGD("Record Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(pszIniModule, "slow", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecordComm");

    /* Load Play FrameRate */
    for (s32Idx = 0; s32Idx < HI_PDT_REC_VSTREAM_MAX_CNT; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "slow:play_framerate%d", s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->au32PlayFrmRate[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Stream[%d] PlayFrameRate[%ufps]\n", s32Idx, pstParam->au32PlayFrmRate[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadLapseRecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_LAPSE_REC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Record Common Configure */
    MLOGD("Record Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(pszIniModule, "lapse", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecordComm");

    /* Load Lapse Interval */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "lapse:lapse_interval");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Interval_ms);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Lapse Interval[%ums]\n", pstParam->u32Interval_ms);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecSnapCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_RECSNAP_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Record Common Configure */
    MLOGD("Record Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(pszIniModule, "recsnap", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecordComm");

    /* Load Photo FileType Index */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "recsnap:snap_filetype_idx");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32SnapFileTypeIndex);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Photo FileTypeIdx[%u]\n", pstParam->u32SnapFileTypeIndex);

    /* Load Photo Interval */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "recsnap:lapse_interval");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32SnapInterval_ms);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Photo SnapInterval[%ums]\n", pstParam->u32SnapInterval_ms);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoProcAlgCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_PHOTO_PROCALG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Output Format Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:output_format");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enOutputFmt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Output Format[%d:0-JPG,1-DNG]\n", pstParam->enOutputFmt);

    /* Load Scene */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:scene");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enScene);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Scene[%d:0-NORMAL,1-HDR,2-LL]\n", pstParam->enScene);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoCommCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_PDT_PHOTO_COMM_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mediamode", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:file_type_idx_jpg", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u8JpgFileTypeIdx = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:file_type_idx_dng", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u8DngFileTypeIdx = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:file_type_idx_jpg_dng", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->u8JpgDngFileTypeIdx = s32Value;

    MLOGD("MediaMode[%d] JpgFileTypeIdx[%u] DngFileTypeIdx[%u] JpgDngFileTypeIdx[%u]\n",
        pstParam->enMediaMode, pstParam->u8JpgFileTypeIdx, pstParam->u8DngFileTypeIdx, pstParam->u8JpgDngFileTypeIdx);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadSingleCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_SINGLE_PHOTO_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;

    /* Load Photo Common Configure */
    MLOGD("Photo Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoCommCfg(pszIniModule, "single", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoComm");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadDelayCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_DELAY_PHOTO_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Photo Common Configure */
    MLOGD("Photo Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoCommCfg(pszIniModule, "delay", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoComm");

    /* Load Delay Time */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "delay:delay_time");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32DelayTime_s);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Delay Time[%uS]\n", pstParam->u32DelayTime_s);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadLapsePhotoCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_LAPSE_PHOTO_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Photo Common Configure */
    MLOGD("Photo Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoCommCfg(pszIniModule, "lapse", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoComm");

    /* Load Lapse Interval */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "lapse:lapse_interval");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Interval_ms);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Lapse Interval[%ums]\n", pstParam->u32Interval_ms);

    /* Load Exposure Time */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "lapse:exposure_time");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32ExpTime_us);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Exposure Time[%uus:0-auto]\n", pstParam->u32ExpTime_us);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadBurstCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_BURST_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  i = 0;

    /* Load Photo Common Configure */
    MLOGD("Photo Common:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoCommCfg(pszIniModule, "burst", &pstParam->stCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoComm");

    /* Load Jpeg BurstType */
    for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "burst:jpeg_burst_type%d", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->aenJpgBurstType[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("MediaMode[%d] Jpeg BurstType[%d:0-3_1,1-5_1,2-10_1,3-15_1,4-30_1,5-20_2,6-30_2,7-30_3]\n",
            pstParam->stCommAttr.enMediaMode, pstParam->aenJpgBurstType[i]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_UVC_FORMAT_CAP_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_U32  u32Idx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:count", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Cnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (HI_UVC_VIDEOMODE_BUTT < pstParam->u32Cnt)
    {
        MLOGE("Invalid Cap Count[%u]\n", pstParam->u32Cnt);
        return HI_FAILURE;
    }
    MLOGD("Prefix[%s] Cap Count[%u]\n", pszNodePrefix, pstParam->u32Cnt);

    for (u32Idx = 0; u32Idx < pstParam->u32Cnt; ++u32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_mode%u", pszNodePrefix, u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->astModes[u32Idx].enMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_bitrate%u", pszNodePrefix, u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->astModes[u32Idx].u32BitRate);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Cap[%u] Mode[%d] BitRate[%d]\n", u32Idx, pstParam->astModes[u32Idx].enMode,pstParam->astModes[u32Idx].u32BitRate);
    }

    /* default videomode */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_defmode", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enDefMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Default Mode[%d]\n", pstParam->enDefMode);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCDataSrcCfg(const HI_CHAR *pszIniModule,
                HI_UVC_DATA_SOURCE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:vproc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VprocHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:vport_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:venc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:acap_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vprochdl[%d] vporthdl[%d] venchdl[%d] acaphdl[%d]\n",
        pstParam->VprocHdl, pstParam->VportHdl, pstParam->VencHdl, pstParam->AcapHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PARAM_UVC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    /* Load MediaMode */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    /* Load Device Path */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc:dev_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stUvcCfg.szDevPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("DevPath[%s]\n", pstParam->stUvcCfg.szDevPath);

    /* Load YUV420 Cap Configure */
    MLOGD("YUV420 Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.yuv420",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_YUV420]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "YUV420Cap");

    /* Load MJPEG Cap Configure */
    MLOGD("MJPEG Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.mjpeg",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_MJPEG]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MJPEGCap");

    /* Load H264 Cap Configure */
    MLOGD("H264 Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.h264",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_H264]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264Cap");

    /* Load Data Source */
    MLOGD("Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCDataSrcCfg(pszIniModule, &pstParam->stUvcCfg.stDataSource);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DataSource");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUsbStorageCfg(const HI_CHAR *pszIniModule,
                HI_USB_STORAGE_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:dev_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szDevPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("DevPath[%s]\n", pstParam->szDevPath);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:sysfile");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szSysFile, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("SysFile[%s]\n", pstParam->szSysFile);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:usb_state_proc");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szProcFile, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("ProcFile[%s]\n", pstParam->szProcFile);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadHDMIPreviewCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.preview:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadHDMIPlaybackCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.player.config:interval_ms");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 500,
        (HI_S32*)&pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.player.config:video_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32VideoEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.player.config:audio_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32AudioEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.player.config:pause_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.bPauseMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("player config interval_ms[%d] video_buff[%d] audio_buff[%d] pause_mode[%d]\n",
        pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs,
        pstParam->stPlayerParam.u32VideoEsBufSize,
        pstParam->stPlayerParam.u32AudioEsBufSize,
        pstParam->stPlayerParam.bPauseMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.vout.opt:vout_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.enVoutType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.vout.opt:mod_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.vout.opt:chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstParam->stVoutOpt.enVoutType, pstParam->stVoutOpt.hModHdl, pstParam->stVoutOpt.hChnHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.aout.opt:ao_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudDevHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "hdmi.playback.aout.opt:ao_chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudTrackHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("AoutOpt AudTrackHdl[%d], AudDevHdl[%d]\n", pstParam->stAoutOpt.hAudTrackHdl, pstParam->stAoutOpt.hAudDevHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPlaybackCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PARAM_PLAYBACK_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:interval_ms");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 500,
        (HI_S32*)&pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:video_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32VideoEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:audio_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32AudioEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:pause_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.bPauseMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("player config interval_ms[%d] video_buff[%d] audio_buff[%d] pause_mode[%d]\n",
        pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs,
        pstParam->stPlayerParam.u32VideoEsBufSize,
        pstParam->stPlayerParam.u32AudioEsBufSize,
        pstParam->stPlayerParam.bPauseMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:vout_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.enVoutType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:mod_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstParam->stVoutOpt.enVoutType, pstParam->stVoutOpt.hModHdl, pstParam->stVoutOpt.hChnHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.aout.opt:ao_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudDevHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.aout.opt:ao_chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudTrackHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("AoutOpt AudTrackHdl[%d], AudDevHdl[%d]\n", pstParam->stAoutOpt.hAudTrackHdl, pstParam->stAoutOpt.hAudDevHdl);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadWorkModeConf(PDT_PARAM_WORKMODE_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};

    /* Load WorkMode Common Configure */
    MLOGD("WorkMode Common Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_COMM);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadCommCfg(szIniModuleName, &pstParam->stCommCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Common");
    MLOGD("\n\n");

    /* Load WorkMode Record Media Configure */
    MLOGD("WorkMode Record Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecMediaCfg(szIniModuleName, &pstParam->stRecMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecMedia");
    MLOGD("\n\n");

    /* Load WorkMode Photo Media Configure */
    MLOGD("WorkMode Photo Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoMediaCfg(szIniModuleName, &pstParam->stPhotoMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoMedia");
    MLOGD("\n\n");

    /* Load WorkMode MPhoto Media Configure */
    MLOGD("WorkMode MPhoto Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_MPHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadMPhotoMediaCfg(szIniModuleName, &pstParam->stMPhotoMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MPhotoMedia");
    MLOGD("\n\n");

    /* Load WorkMode RecSnap Media Configure */
    MLOGD("WorkMode RecSnap Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECSNAP);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecSnapMediaCfg(szIniModuleName, &pstParam->stRecSnapMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecSnapMedia");
    MLOGD("\n\n");

    /* Load WorkMode Playback Media Configure */
    MLOGD("WorkMode Playback Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PLAYBACK);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPlaybackMediaCfg(szIniModuleName, &pstParam->stPlayBackMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PlaybackMedia");
    MLOGD("\n\n");

    /* Load WorkMode HDMIPreview Media Configure */
    MLOGD("WorkMode HDMIPreview Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_HDMIPREVIEW);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadHDMIPreviewMediaCfg(szIniModuleName, &pstParam->stHDMIPreviewMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HDMIPreviewMedia");
    MLOGD("\n\n");

    /* Load WorkMode HDMIPlayback Media Configure */
    MLOGD("WorkMode HDMIPlayback Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_HDMIPLAYBACK);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadHDMIPlaybackMediaCfg(szIniModuleName, &pstParam->stHDMIPlaybackMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HDMIPlaybackMedia");
    MLOGD("\n\n");

    /* Load Normal Record Configure */
    MLOGD("Normal Record Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadNormRecCfg(szIniModuleName, &pstParam->stNormRecAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "NormRec");
    MLOGD("\n\n");

    /* Load Loop Record Configure */
    MLOGD("Loop Record Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadLoopRecCfg(szIniModuleName, &pstParam->stLoopRecAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LoopRec");
    MLOGD("\n\n");

    /* Load Slow Record Configure */
    MLOGD("Slow Record Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadSlowRecCfg(szIniModuleName, &pstParam->stSlowRecAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SlowRec");
    MLOGD("\n\n");

    /* Load Lapse Record Configure */
    MLOGD("Lapse Record Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadLapseRecCfg(szIniModuleName, &pstParam->stLpseRecAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LapseRec");
    MLOGD("\n\n");

    /* Load RecSnap Configure */
    MLOGD("RecSnap Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECSNAP);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecSnapCfg(szIniModuleName, &pstParam->stRecSnapAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecSnap");
    MLOGD("\n\n");

    /* Load PhotoProcAlg Configure */
    MLOGD("PhotoProcAlg Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoProcAlgCfg(szIniModuleName, &pstParam->stPhotoProcAlg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoProcAlg");
    MLOGD("\n\n");

    /* Load Single Configure */
    MLOGD("Single Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadSingleCfg(szIniModuleName, &pstParam->stSingPhotoAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Single");
    MLOGD("\n\n");

    /* Load Delay Configure */
    MLOGD("Delay Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadDelayCfg(szIniModuleName, &pstParam->stDelayPhotoAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Delay");
    MLOGD("\n\n");

    /* Load Lapse Photo Configure */
    MLOGD("Lapse Photo Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadLapsePhotoCfg(szIniModuleName, &pstParam->stLpsePhotoAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LapsePhoto");
    MLOGD("\n\n");

    /* Load Burst Configure */
    MLOGD("Burst Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_MPHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadBurstCfg(szIniModuleName, &pstParam->stBurstAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Burst");
    MLOGD("\n\n");

    /* Load UVC Configure */
    MLOGD("UVC Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_USB);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCfg(szIniModuleName, &pstParam->stUvcAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "UVC");
    MLOGD("\n\n");

    /* Load USB Storage Configure */
    MLOGD("USB Storage:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_USB);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUsbStorageCfg(szIniModuleName, &pstParam->stUsbStorageCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "UsbStorage");
    MLOGD("\n\n");

    /* Load HDMI Preview Configure */
    MLOGD("HDMI Preview:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_HDMIPREVIEW);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadHDMIPreviewCfg(szIniModuleName, &pstParam->stHDMIPreviewAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HDMIPreview");
    MLOGD("\n\n");

    /* Load HDMI Playback Configure */
    MLOGD("HDMI Playback:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_HDMIPLAYBACK);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadHDMIPlaybackCfg(szIniModuleName, &pstParam->stHDMIPlaybackAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HDMIPlayback");
    MLOGD("\n\n");

    /* Load Playback Configure */
    MLOGD("Playback:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PLAYBACK);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPlaybackCfg(szIniModuleName, &pstParam->stPlaybackAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Playback");
    MLOGD("\n\n");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

