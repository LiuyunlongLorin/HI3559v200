/**
 * @file      hi_product_iniparam_mediacomm.c
 * @brief     load media common parameter
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

HI_S32 HI_PDT_INIPARAM_MEDIAMODE_GetSensorTimeSequeceEnum(const HI_CHAR *pszEnumStr,
    PDT_PARAM_SENSOR_TIME_SEQUENCE_E  *penSensorTimeSequece)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_4K30))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_4K30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_4K60))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_4K60;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_1080P90))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_1080P90;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_1080P120))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_1080P120;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_1080P240))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_1080P240;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_720P120))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_720P120;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, PDT_SENSOR_TIME_SEQUENCE_720P240))
    {
        *penSensorTimeSequece = PDT_SENSOR_TIME_SEQUENCE_720P240;
    }
    else
    {
        MLOGE("Invalid SensorTimeSequece[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("SensorTimeSequece[%s] Enum[%d]\n", pszEnumStr, *penSensorTimeSequece);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIA_COMM_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "comm:ldc_enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bLDCEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "comm:dis_enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bDISEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "comm:flip_enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bFlip);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "comm:audio_enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bAudio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Global LDCEnable[%d] DISEnable[%d] FlipEnable[%d] AudioEnable[%d]\n",
        pstParam->bLDCEnable, pstParam->bDISEnable, pstParam->bFlip, pstParam->bAudio);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadLDCAttr(const HI_CHAR* pszIniModule,
                const HI_CHAR* pszNodePrefix, LDC_ATTR_S* pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:aspect", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bAspect);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:xoffset", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32CenterXOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:yoffset", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32CenterYOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:xratio", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32XRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:yratio", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32YRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:xyratio", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32XYRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.ldc:distortion_ratio", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32DistortionRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    LDC aspect[%d], offset[%d,%d], ratio[x:%d,y:%d,xy:%d,distortion:%d]\n",
        pstParam->bAspect, pstParam->s32CenterXOffset, pstParam->s32CenterYOffset,
        pstParam->s32XRatio, pstParam->s32YRatio, pstParam->s32XYRatio, pstParam->s32DistortionRatio);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_GetDISTypeEnum(const HI_CHAR *pszEnumStr,
                DIS_MODE_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MODE_4_DOF_GME))
    {
        *penMode = DIS_MODE_4_DOF_GME;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MODE_6_DOF_GME))
    {
        *penMode = DIS_MODE_6_DOF_GME;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MODE_GYRO))
    {
        *penMode = DIS_MODE_GYRO;
    }
    else
    {
        MLOGE("Invalid DISType[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("    DISType[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVcapPipeChnAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VcapIdx, HI_S32 s32VcapPipeIdx, HI_S32 s32VcapPipeChnIdx,
                PDT_PARAM_MEDIA_COMM_VCAP_PIPE_CHN_S* pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.chn.%d",
        s32VcapIdx, s32VcapPipeIdx, s32VcapPipeChnIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vcap_pipechn_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->PipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vcap[%d] Pipe[%d] PipeChn[%d] handle[%d]\n", s32VcapIdx, s32VcapPipeIdx,
        s32VcapPipeChnIdx, pstParam->PipeChnHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rotate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mirror", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bMirror);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:flip", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bFlip);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    Rotate[%d, 0:0, 1:90, 2:180, 3:270] Mirror[%d] Flip[%d]\n",
        pstParam->enRotate, pstParam->bMirror, pstParam->bFlip);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:brightness", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32Brightness);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:saturation", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32Saturation);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    Brightness[%d] Saturation[%d]\n", pstParam->s32Brightness, pstParam->s32Saturation);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadLDCAttr(pszIniModule, szIniNodePrefix, &pstParam->stLDCAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LDCAttr");

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dis_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_GetDISTypeEnum(pszString, &pstParam->enDISMotionType);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVcapPipeAttr(const HI_CHAR *pszIniModule, HI_S32 s32VcapIdx,
                HI_S32 s32VcapPipeIdx, PDT_PARAM_MEDIA_COMM_VCAP_PIPE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d",
        s32VcapIdx, s32VcapPipeIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vcap_pipe_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vcap[%d] Pipe[%d] handle[%d]\n", s32VcapIdx, s32VcapPipeIdx, pstParam->VcapPipeHdl);

    HI_S32  s32PipeChnIdx;
    for (s32PipeChnIdx = 0; s32PipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32PipeChnIdx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVcapPipeChnAttr(pszIniModule, s32VcapIdx,
            s32VcapPipeIdx, s32PipeChnIdx, &pstParam->astPipeChnAttr[s32PipeChnIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeChn");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVcapDevAttr(const HI_CHAR *pszIniModule, HI_S32 s32VcapIdx,
                PDT_PARAM_MEDIA_COMM_VCAP_DEV_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d:vcap_dev_hdl", s32VcapIdx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapDevHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("vcap device handle[%d]\n", pstParam->VcapDevHdl);

    HI_S32  s32PipeIdx;
    for (s32PipeIdx = 0; s32PipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32PipeIdx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVcapPipeAttr(pszIniModule, s32VcapIdx,
            s32PipeIdx, &pstParam->astVcapPipeAttr[s32PipeIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipe");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVpssPortAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VpssIdx, HI_S32 s32VpssPortIdx,
                PDT_PARAM_MEDIA_COMM_VPSS_PORT_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.vport.%d", s32VpssIdx, s32VpssPortIdx);
    MLOGD("    Vpss[%d] port[%d]\n", s32VpssIdx, s32VpssPortIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vport_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rotate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mirror", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bMirror);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:flip", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bFlip);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    handle[%d] Rotate[%d, 0:0, 1:90, 2:180, 3:270] Mirror[%d] Flip[%d]\n",
        pstParam->VportHdl, pstParam->enRotate, pstParam->bMirror, pstParam->bFlip);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVpssAttr(const HI_CHAR *pszIniModule, HI_S32 s32VpssIdx,
                PDT_PARAM_MEDIA_COMM_VPSS_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d", s32VpssIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vpss_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VpssHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vcap_pipe_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vcap_pipechn_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vpss[%d] handle[%d] vcap pipehdl[%d] vcap pipechnhdl[%d]\n", s32VpssIdx,
        pstParam->VpssHdl, pstParam->VcapPipeHdl, pstParam->VcapPipeChnHdl);

    HI_S32  s32VportIdx = 0;
    for (s32VportIdx = 0; s32VportIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VportIdx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVpssPortAttr(pszIniModule, s32VpssIdx, s32VportIdx,
            &pstParam->astPortAttr[s32VportIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VpssPort");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVprocCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIA_COMM_VPROC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32VpssIdx = 0;
    for (s32VpssIdx = 0; s32VpssIdx < HI_PDT_MEDIA_VPSS_MAX_CNT; ++s32VpssIdx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVpssAttr(pszIniModule, s32VpssIdx,
            &pstParam->astVpssAttr[s32VpssIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Vpss");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH264CbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_COMM_VENC_ATTR_H264_CBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H264CBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH264VbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_COMM_VENC_ATTR_H264_VBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H264VBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencH264Attr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.h264", pszPrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:profile", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32H264Profile);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    profile[%u:0-baseline,1-main,2-high]\n", pstParam->u32H264Profile);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH264CbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH264Cbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264CBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH264VbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH264Vbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264VBRAttr");
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH265CbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_COMM_VENC_ATTR_H265_CBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H265CBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH265VbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_COMM_VENC_ATTR_H265_VBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H265VBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencH265Attr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.h265", pszPrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:profile", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32H265Profile);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    profile[%u:0-main]\n", pstParam->u32H265Profile);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH265CbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH265Cbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265CBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH265VbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH265Vbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265VBRAttr");
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszEncType, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.video.%s", pszEncType);
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:payload_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:scene_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSceneMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("[%s] payload[%d:0-H264,1-H265,2-MJPEG] scene[%d:0-normal,1-dv,2-car]\n",
        szIniNodePrefix, pstParam->enType, pstParam->enSceneMode);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencH264Attr(pszIniModule, szIniNodePrefix, pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264Attr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencH265Attr(pszIniModule, szIniNodePrefix, pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265Attr");

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rc_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRcMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    RC Mode[%d:0-CBR,1-VBR]\n", pstParam->enRcMode);

    pstParam->stMjpegCbr.u32MinQfactor = 75;
    pstParam->stMjpegCbr.u32MaxQfactor = 85;
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszEncType, PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.snap.%s", pszEncType);
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:payload_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:encode_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enEncodeMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable_dcf", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnableDCF);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:quality_factor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Qfactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mpf_enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bMpfEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("[%s] payload[%d:3:JPEG] encodeMode[%d] bDCF[%d], QualityFactor[%u], bMPF[%d]\n",
        szIniNodePrefix, pstParam->enType, pstParam->enEncodeMode, pstParam->bEnableDCF,
        pstParam->u32Qfactor, pstParam->bMpfEnable);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencHierarchicalQp(const HI_CHAR *pszIniModule,
                VENC_HIERARCHICAL_QP_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 s32Idx;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.hierarchical.qp");
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    for (s32Idx = 0; s32Idx < 4; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:qp_delta%d", szIniNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->s32HierarchicalQpDelta[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("  QpDelta%d[%d]\n", s32Idx, pstParam->s32HierarchicalQpDelta[s32Idx]);
    }

    for (s32Idx = 0; s32Idx < 4; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:frame_num%d", szIniNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->s32HierarchicalFrameNum[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("  FrameNum%d[%d]\n", s32Idx, pstParam->s32HierarchicalFrameNum[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_GetAIOModeEnum(const HI_CHAR *pszEnumStr,
                AIO_MODE_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_I2S_MASTER))
    {
        *penMode = AIO_MODE_I2S_MASTER;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_I2S_SLAVE))
    {
        *penMode = AIO_MODE_I2S_SLAVE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_PCM_SLAVE_STD))
    {
        *penMode = AIO_MODE_PCM_SLAVE_STD;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_PCM_SLAVE_NSTD))
    {
        *penMode = AIO_MODE_PCM_SLAVE_NSTD;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_PCM_MASTER_STD))
    {
        *penMode = AIO_MODE_PCM_MASTER_STD;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, AIO_MODE_PCM_MASTER_NSTD))
    {
        *penMode = AIO_MODE_PCM_MASTER_NSTD;
    }
    else
    {
        MLOGE("Invalid AIOMode[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("    AIOMode[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAcapCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_ACAP_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "acapture.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acap_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:audio_gain", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32AudioGain);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Acap[%d] enable[%d] hdl[%d] audio gain[%d]\n", s32Idx, pstParam->bEnable,
        pstParam->AcapHdl, pstParam->s32AudioGain);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enSampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:resample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enResampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enBitwidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:work_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_GetAIOModeEnum(pszString, &pstParam->stAcapAttr.enWorkmode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acodec_mixer", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enMixerMicMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("    samplerate[%d],bitwidth[%d:0-8,1-16,2-24] soundMode[%d:0-left,1-right,2-stereo]\n",
        pstParam->stAcapAttr.enSampleRate, pstParam->stAcapAttr.enBitwidth,
        pstParam->stAcapAttr.enSoundMode);
    MLOGD("    resamplerate[%d] acodec_mixer[%d:0-IN0,1-IN1,2-IN_D] ptNumPerFrm[%u]\n",
        pstParam->stAcapAttr.enResampleRate, pstParam->stAcapAttr.enMixerMicMode,
        pstParam->stAcapAttr.u32PtNumPerFrm);

    HI_S32 s32AcapChnlIdx = 0;
    for(s32AcapChnlIdx = 0; s32AcapChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; s32AcapChnlIdx++)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "acapture.%d.chnl.%d", s32Idx, s32AcapChnlIdx);

        pstParam->AcapChnlCfg[s32AcapChnlIdx].AcapChnlHdl = s32AcapChnlIdx;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlCfg[s32AcapChnlIdx].bEnable);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable_vqe", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlCfg[s32AcapChnlIdx].bEnableVqe);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAencAACCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_MAPI_AENC_ATTR_AAC_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Value;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "aenc.%d.aac", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aac_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enAACType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBitRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSmpRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBitWidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:trans_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enTransType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:band_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    pstParam->s16BandWidth = s32Value;
    MLOGD("    AAC Type[%d:0-AACLC],bitrate[%d],samplerate[%d]\n",
        pstParam->enAACType, pstParam->enBitRate, pstParam->enSmpRate);
    MLOGD("    bitwidth[%d:0-8,1-16,2-24],soundMode[%d:0-mono,1-stereo] bandwidth[%d]\n",
        pstParam->enBitWidth, pstParam->enSoundMode,pstParam->s16BandWidth);
    MLOGD("    transType[%d:0-ADTS,1-LOAS,2-LATM_MCP1]\n", pstParam->enTransType);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAencCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_AENC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "aenc.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aenc_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acap_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acap_chnl_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:audio_format", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Aenc[%d] enable[%d] aenchdl[%d] acaphdl[%d] format[%d:0-aac,1-opus] ptNumPerFrm[%u]\n",
        s32Idx, pstParam->bEnable, pstParam->AencHdl, pstParam->AcapHdl,
        pstParam->enFormat, pstParam->u32PtNumPerFrm);

    /* Load Aenc AAC Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAencAACCfg(pszIniModule, s32Idx, &pstParam->unAttr.stAACAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "AencAAC");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAoCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_AO_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "ao.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ao_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AoHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ao_chn_hdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AoChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enSampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->stAoAttr.enInSampleRate = pstParam->stAoAttr.enSampleRate;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enBitwidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Ao[%d] enable[%d] hdl[%d]\n", s32Idx, pstParam->bEnable, pstParam->AoHdl);
    MLOGD("    samplerate[%d],bitwidth[%d:0-8,1-16,2-24] soundMode[%d:0-left,1-right,2-stereo]\n",
        pstParam->stAoAttr.enSampleRate, pstParam->stAoAttr.enBitwidth, pstParam->stAoAttr.enSoundMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:work_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_GetAIOModeEnum(pszString, &pstParam->stAoAttr.enWorkmode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:volume", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->s32Volume);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    ptNumPerFrm[%u] volume[%d]\n", pstParam->stAoAttr.u32PtNumPerFrm, pstParam->s32Volume);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadTimeOSDCfg(const HI_CHAR *pszIniModule,
                HI_PDT_MEDIA_TIME_OSD_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:show");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bShow);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:time_format");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:color");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Color);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:bg_color");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32BgColor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:font_width");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stFontSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:font_height");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stFontSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:start_x");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stStartPos.s32X);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:start_y");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stStartPos.s32Y);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:fg_alpha");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32FgAlpha);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.time:bg_alpha");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32BgAlpha);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("TimeOSD show[%d],timeformat[%d:0-24H],color[%08x],bgcolor[%08x]\n",
        pstParam->bShow, pstParam->enFormat, pstParam->u32Color, pstParam->u32BgColor);
    MLOGD("  fontsize[%ux%u],startpos[%ux%u],fg alpha[%#x], bg alpha[%#x]\n",
        pstParam->stFontSize.u32Width, pstParam->stFontSize.u32Height,
        pstParam->stStartPos.s32X, pstParam->stStartPos.s32Y, pstParam->u32FgAlpha, pstParam->u32BgAlpha);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadOSDCfg(const HI_CHAR *pszIniModule,
                HI_PDT_MEDIA_OSD_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.common:bind_module");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBindMod);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("OSD binded module[%d:0-vcap,1-venc]\n", pstParam->enBindMod);

    /* Load TImeOSD Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadTimeOSDCfg(pszIniModule, &pstParam->stTimeOsd);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "TimeOSD");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadOSDVideoAttr(const HI_CHAR* pszIniModule,
                HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 i, j, k;

    /* Load Vcap Attribute */
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                    "osd.video.vcap.%d.pipe.%d.chn.%d:max_width", i, j, k);
                s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                    (HI_S32*)&pstParam->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Width);
                PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

                snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                    "osd.video.vcap.%d.pipe.%d.chn.%d:max_height", i, j, k);
                s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                    (HI_S32*)&pstParam->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Height);
                PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
                MLOGD("Vcap[%d] Pipe[%d] Chn[%d] MaxSize[%ux%u]\n", i, j, k,
                    pstParam->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Width,
                    pstParam->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize.u32Height);
            }
        }
    }

    /* Load Vpss Attribute */
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                "osd.video.vpss.%d.vport.%d:max_width", i, j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Width);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,
                "osd.video.vpss.%d.vport.%d:max_height", i, j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Height);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("Vpss[%d] Port[%d] MaxSize[%ux%u]\n", i, j,
                    pstParam->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Width,
                    pstParam->astVpssAttr[i].astPortAttr[j].stMaxSize.u32Height);
        }
    }

    /* Load Venc Attribute */
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.video.venc.%d:max_width", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astVencAttr[i].stMaxSize.u32Width);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "osd.video.venc.%d:max_height", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astVencAttr[i].stMaxSize.u32Height);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Venc[%d] MaxSize[%ux%u]\n", i,
                pstParam->astVencAttr[i].stMaxSize.u32Width,
                pstParam->astVencAttr[i].stMaxSize.u32Height);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDISLdcv2Attr(const HI_CHAR *pszIniModule,
    PDT_PARAM_LDCV2_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "common");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:SensorTimeSequence", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetSensorTimeSequeceEnum(pszString, &pstParam->enSensorTimeSequece);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }
    MLOGD("    SensorTimeSequence[%d]\n", pstParam->enSensorTimeSequece);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "LDCV2Attr");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.bEnable[%d]\n", pstParam->stLDCV2Attr.bEnable);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:FocalLenX", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32FocalLenX);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32FocalLenX[%d]\n", pstParam->stLDCV2Attr.stAttr.s32FocalLenX);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:FocalLenY", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32FocalLenY);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32FocalLenY[%d]\n", pstParam->stLDCV2Attr.stAttr.s32FocalLenY);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:CoorShiftX", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32CoorShiftX);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32CoorShiftX[%d]\n", pstParam->stLDCV2Attr.stAttr.s32CoorShiftX);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:CoorShiftY", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32CoorShiftY);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32CoorShiftY[%d]\n", pstParam->stLDCV2Attr.stAttr.s32CoorShiftY);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:SrcJunPt", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32SrcJunPt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32SrcJunPt[%d]\n", pstParam->stLDCV2Attr.stAttr.s32SrcJunPt);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:MaxDU", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stLDCV2Attr.stAttr.s32MaxDu);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stLDCV2Attr.stAttr.s32MaxDU[%d]\n", pstParam->stLDCV2Attr.stAttr.s32MaxDu);

    HI_S32 i,j;
    for(i = 0; i < SRC_LENS_COEF_SEG; i++)
    {
        for(j = 0; j < SRC_LENS_COEF_NUM; j++)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:SrcCaliRatio[%d][%d]",
                szIniNodePrefix, i, j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->stLDCV2Attr.stAttr.as32SrcCaliRatio[i][j]);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("    stLDCV2Attr.stAttr.as32SrcCaliRatio[%d][%d] = %d\n",
                i, j, pstParam->stLDCV2Attr.stAttr.as32SrcCaliRatio[i][j]);
        }
    }

    for(i = 0; i < DST_LENS_COEF_SEG; i++)
    {
        for(j = 0; j < DST_LENS_COEF_NUM; j++)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:DstCaliRatio[%d][%d]",
                szIniNodePrefix, i, j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->stLDCV2Attr.stAttr.as32DstCaliRatio[i][j]);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("    stLDCV2Attr.stAttr.as32DstCaliRatio[%d][%d] = %d\n",
                i, j, pstParam->stLDCV2Attr.stAttr.as32DstCaliRatio[i][j]);
        }
    }

    for(i = 0; i < (DST_LENS_COEF_SEG - 1); i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:DstJunPt[%d]",
            szIniNodePrefix, i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stLDCV2Attr.stAttr.as32DstJunPt[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    stLDCV2Attr.stAttr.as32DstJunPt[%d] = %d\n",
                i, pstParam->stLDCV2Attr.stAttr.as32DstJunPt[i]);
    }

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "DISConfig");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:PdtType", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.enPdtType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.enPdtType[%d]\n", pstParam->stDISConfig.enPdtType);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:BufNum", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.u32BufNum);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.u32BufNum[%u]\n", pstParam->stDISConfig.u32BufNum);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:CropRatio", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.u32CropRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.u32CropRatio[%u]\n", pstParam->stDISConfig.u32CropRatio);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroOutputRange", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.u32GyroOutputRange);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.u32GyroOutputRange[%u]\n", pstParam->stDISConfig.u32GyroOutputRange);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroDataBitWidth", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.u32GyroDataBitWidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.u32GyroDataBitWidth[%u]\n", pstParam->stDISConfig.u32GyroDataBitWidth);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bCameraSteady", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.bCameraSteady);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.bCameraSteady[%d]\n", pstParam->stDISConfig.bCameraSteady);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bScale", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDISConfig.bScale);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stDISConfig.bScale[%d]\n", pstParam->stDISConfig.bScale);

    return HI_SUCCESS;
}

HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAllDISLdcv2Attr(PDT_PARAM_MEDIA_COMM_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_S32  s32Idx;

    for (s32Idx = 0; s32Idx < PDT_PARAM_LDCV2_CFG_CNT; ++s32Idx)
    {
        MLOGD("LDCV2CFG[%d] Configure:\n", s32Idx);
        snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s%d", PDT_INIPARAM_LDCV2CFG, s32Idx);

        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDISLdcv2Attr(szIniModuleName, &pstParam->astLDCV2Cfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniModuleName);
        MLOGD("\n\n");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadMotionSensorInfo(const HI_CHAR *pszIniModule,
    HI_MAPI_MOTIONSENSOR_INFO_S *pstParam)
{
#ifdef CONFIG_MOTIONSENSOR
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 i, j;

    MLOGD(YELLOW"MotionSensorInfo Configure:\n"NONE);
    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "FSR");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Gyro", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32GyroFSR);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    u32GyroFSR[%u]\n", pstParam->u32GyroFSR);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Acc", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32AccFSR);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    u32AccFSR[%u]\n", pstParam->u32AccFSR);


    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "GyroDrift");
    for(i = 0; i < AXIS_NUM; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroDrift[%d]",
            szIniNodePrefix, i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->aGyroDrift[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    aGyroDrift[%d] = %d\n", i, pstParam->aGyroDrift[i]);
    }

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "Rotation");
    for(i = 0; i < MATRIX_NUM; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Rotation[%d]",
            szIniNodePrefix, i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->aRotationMatrix[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    aRotationMatrix[%d] = %d\n", i, pstParam->aRotationMatrix[i]);
    }

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "TempDrift");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stTempDrift.enMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    TempDrift Mode[%u]\n", pstParam->stTempDrift.enMode);

    if(IMU_TEMP_DRIFT_CURV == pstParam->stTempDrift.enMode)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "IMUMatrixTemp");
        for(i = 0; i < MATRIX_TEMP_NUM; i++)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:IMUMatrixTemp[%d]",
                szIniNodePrefix, i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->stTempDrift.aTempMatrix[i]);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("    IMUMatrixTemp[%d] = %d\n", i, pstParam->stTempDrift.aTempMatrix[i]);
        }
    }
    else if(IMU_TEMP_DRIFT_LUT == pstParam->stTempDrift.enMode)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "TempDriftLUT");
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:RangeMin", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stTempDrift.stTempLut.s32RangeMin);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    s32RangeMin[%d]\n", pstParam->stTempDrift.stTempLut.s32RangeMin);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:RangeMax", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stTempDrift.stTempLut.s32RangeMax);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    s32RangeMax[%d]\n", pstParam->stTempDrift.stTempLut.s32RangeMax);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Step", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stTempDrift.stTempLut.u32Step);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("    u32Step[%u]\n", pstParam->stTempDrift.stTempLut.u32Step);

        for(i = 0; i < TEMP_LUT_SAMPLES; i++)
        {
            for(j = 0; j < AXIS_NUM; j++)
            {
                snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:IMUTempLut[%d][%d]",
                    szIniNodePrefix, i, j);
                s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                    (HI_S32*)&pstParam->stTempDrift.stTempLut.as32IMULut[i][j]);
                PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
                MLOGD("    stTempDrift.stTempLut.as32IMULut[%d][%d] = %d\n",
                    i, j, pstParam->stTempDrift.stTempLut.as32IMULut[i][j]);
            }
        }

        for(i = 0; i < TEMP_LUT_SAMPLES; i++)
        {
            for(j = 0; j < MFUSION_LUT_STATUS_NUM; j++)
            {
                snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroLutStatus[%d][%d]",
                    szIniNodePrefix, i, j);
                s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                    (HI_S32*)&pstParam->stTempDrift.stTempLut.as32GyroLutStatus[i][j]);
                PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
                MLOGD("    stTempDrift.stTempLut.as32GyroLutStatus[%d][%d] = %d\n",
                    i, j, pstParam->stTempDrift.stTempLut.as32GyroLutStatus[i][j]);
            }
        }
    }
    else
    {
        MLOGE("    TempDrift enMode error\n");
        return HI_FAILURE;
    }

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "MFusionAttr");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:DeviceMask", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.u32DeviceMask);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.u32DeviceMask[%u]\n", pstParam->stMFusionAttr.u32DeviceMask);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:TemperatureMask", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.u32TemperatureMask);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.u32TemperatureMask[%u]\n", pstParam->stMFusionAttr.u32TemperatureMask);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "SteadyDetectAttr");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:SteadyTimeThr", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.u32SteadyTimeThr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.u32SteadyTimeThr[%u]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.u32SteadyTimeThr);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroOffset", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32GyroOffset[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroOffset);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:AccOffset", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32AccOffset[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccOffset);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroRMS", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroRMS);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32GyroRMS[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroRMS);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:AccRMS", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccRMS);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32AccRMS[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccRMS);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroOffsetFactor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroOffsetFactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32GyroOffsetFactor[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroOffsetFactor);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:AccOffsetFactor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccOffsetFactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32AccOffsetFactor[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccOffsetFactor);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GyroRMSFactor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroRMSFactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32GyroRMSFactor[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32GyroRMSFactor);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:AccRMSFactor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccRMSFactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    stMFusionAttr.stSteadyDetectAttr.s32AccRMSFactor[%d]\n",
        pstParam->stMFusionAttr.stSteadyDetectAttr.s32AccRMSFactor);
#endif
    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadMediaCommConf(PDT_PARAM_MEDIA_COMM_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_U32  s32Idx = 0;

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_MEDIA_COMM);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadCommCfg(szIniModuleName, pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Comm");

    /* Load Vcapture Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVcapDevAttr(szIniModuleName, s32Idx, &pstParam->stVcapCfg.astVcapDevAttr[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapDev");
    }

    /* Load Vprocessor Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVprocCfg(szIniModuleName, &pstParam->stVprocCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Vproc");

    /* Load VideoEnc Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(szIniModuleName, "main", &pstParam->stMainVideoVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MainVideoEnc");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(szIniModuleName, "sub", &pstParam->stSubVideoVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SubVideoEnc");

    /* Load SnapEnc Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(szIniModuleName, "single", &pstParam->stSingleSnapVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SingSnapAttr");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(szIniModuleName, "multiple", &pstParam->stMultiSnapVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MultiSnapAttr");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(szIniModuleName, "sub", &pstParam->stSubSnapVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SubSnapAttr");

    /* Load Venc HierarchicalQp Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencHierarchicalQp(szIniModuleName, &pstParam->stHierarchicalQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HierarchicalQp");

    /* Load Acapture Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_ACAP_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAcapCfg(szIniModuleName, s32Idx, &pstParam->astAcapCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Acap");
    }

    /* Load Aencoder Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_AENC_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAencCfg(szIniModuleName, s32Idx, &pstParam->astAencCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Aenc");
    }

    /* Load Ao Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_AO_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAoCfg(szIniModuleName, s32Idx, &pstParam->astAoCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Ao");
    }

    /* Load OSD Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadOSDCfg(szIniModuleName, &pstParam->stOsdCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "OSD");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadOSDVideoAttr(szIniModuleName, &pstParam->stOsdVideoAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "OSDVideoAttr");

    /* Load LDCV2 Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAllDISLdcv2Attr(pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LDCV2Cfg");

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_MOTIONSENSORINFO);
    /* Load MotionSensorInfo */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadMotionSensorInfo(szIniModuleName, &pstParam->stMotionSensorInfo);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MotionSensorInfo");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

