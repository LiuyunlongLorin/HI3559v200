/**
* Copyright (C), 2016-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_product_iniparam_devmng.c
* @brief     load device manage parameter interface implementation
* @author    HiMobileCam Reference Develop Team
* @date      2017.12.05
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

HI_S32 HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(const HI_CHAR *pszEnumStr,
                HI_PDT_MEDIAMODE_E *penMediaMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_720P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_720P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_25))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_25;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1296P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1296P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_25))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_25;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1520P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1520P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1600P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1600P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1944P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1944P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_2160P_30))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_2160P_30;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_60))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_60;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1080P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1080P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_720P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_720P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1296P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1296P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1440P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1440P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1520P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1520P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1600P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1600P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_1944P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_1944P;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PHOTO_2160P))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PHOTO_2160P;
    }
    else
    {
        MLOGE("Invalid MediaMode[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("MediaMode[%s] Enum[%d]\n", pszEnumStr, *penMediaMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnLDCCfg(const HI_CHAR *pszIniModule,HI_S32 s32PipeIdx,
                HI_S32 s32ChnIdx,VI_LDC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vcap.pipe.%d.chn.%d.ldc",s32PipeIdx,s32ChnIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:aspect",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.bAspect);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:xratio",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32XRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);


    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:yratio",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32YRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:xyratio",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32XYRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);


    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:centerxoffset",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32CenterXOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:centeryoffset",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32CenterYOffset);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:distortionratio",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stAttr.s32DistortionRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("pipe[%d] chn[%d] ldc attr,enable[%d],aspect[%d],xratio[%d],yratio[%d],xyratio[%d],centerxoffset[%d],centeryoffset[%d], distortionratio[%d] \n",
        s32PipeIdx,s32ChnIdx,pstParam->bEnable,pstParam->stAttr.bAspect,pstParam->stAttr.s32XRatio,pstParam->stAttr.s32YRatio,pstParam->stAttr.s32XYRatio,pstParam->stAttr.s32CenterXOffset,
        pstParam->stAttr.s32CenterYOffset,pstParam->stAttr.s32DistortionRatio);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnCommCfg(const HI_CHAR *pszIniModule,HI_S32 s32PipeIdx,
                HI_S32 s32ChnIdx,PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_CHN_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vcap.pipe.%d.chn.%d",s32PipeIdx,s32ChnIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pipechnhdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->PipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pixelformat",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enPixelFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:rotate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:mirror",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bMirror);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:flip",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bFlip);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:brightness",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->s32Brightness);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:saturation",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->s32Saturation);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("pipe[%d]  chn[%d] comm attr, enable[%d], pipechnhdl[%d], pixelformat[%d, 0-yuv420,1-yuv422] \n",
        s32PipeIdx,s32ChnIdx,pstParam->bEnable,pstParam->PipeChnHdl,pstParam->enPixelFormat);
    MLOGD("rotate[%d],mirror[%d],flip[%d],brightness[%d],saturation[%d] \n", pstParam->enRotate,pstParam->bMirror,
        pstParam->bFlip,pstParam->s32Brightness,pstParam->s32Saturation);

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnLDCCfg(pszIniModule,s32PipeIdx,s32ChnIdx,&pstParam->stLDCAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LDCAttr");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeCommCfg(const HI_CHAR *pszIniModule,HI_S32 s32PipeIdx,
                PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vcap.pipe.%d",s32PipeIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:vcappipehdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

     snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pipetype",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enPipeType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("pipe[%d] comm attr, enable[%d], vcappipehdl[%d],  pipetype[%d] \n"
        ,s32PipeIdx,pstParam->bEnable,pstParam->VcapPipeHdl,pstParam->enPipeType);
    for(i = 0 ;i < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnCommCfg(pszIniModule,s32PipeIdx,i, &pstParam->astPipeChnAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeChnComm");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_COMM_VCAP_DEV_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:vcapdevhdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VcapDevHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:wdr_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enWdrMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:video_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 1,(HI_S32*)&pstParam->u32VideoMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vcap dev comm attr  enable[%d],vcapdevhdl[%d] wdr_mode[%d]\n",pstParam->bEnable,pstParam->VcapDevHdl,pstParam->enWdrMode);

    for(i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeCommCfg(pszIniModule,i,&pstParam->astVcapPipeAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeComm");
    }

    return HI_SUCCESS;

}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVportCommCfg(const HI_CHAR *pszIniModule,HI_S32 s32VpssIdx,
                HI_S32 s32VportIdx,PDT_PARAM_MEDIAMODE_COMM_VPSS_PORT_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vpss.%d.vport.%d",s32VpssIdx,s32VportIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:vporthdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:rotate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:mirror",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bMirror);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:flip",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bFlip);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:video_format",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enVideoFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:buffershare",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bSupportBufferShare);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pixelformat",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enPixelFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss[%d]vport[%d] attr,enable[%d],vporthdl[%d],rotate[%d],mirror[%d],flip[%d], buffershare[%d]\n",
        s32VpssIdx,s32VportIdx,pstParam->bEnable,pstParam->VportHdl,pstParam->enRotate,pstParam->bMirror,pstParam->bFlip,pstParam->bSupportBufferShare);
    MLOGD("video_format[%d,0-LINEAR,1-TILE,2-TILE64], pixelformat[%d,0-yuv420,1-yuv422] \n",pstParam->enVideoFormat,pstParam->enPixelFormat);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVpssCommCfg(const HI_CHAR *pszIniModule,HI_S32 s32VpssIdx,
                PDT_PARAM_MEDIAMODE_COMM_VPSS_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vpss.%d",s32VpssIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:vpsshdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VpssHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:vcappipehdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pipechnhdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->PipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:pixelformat",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enPixelFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:nrenable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bNrEn);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:nrcompressmode",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stNrAttr.enCompressMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:nrmotionmode",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stNrAttr.enNrMotionMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss[%d] attr, enable[%d],vpsshdl[%d],vcappipehdl[%d],pipechnhdl[%d],pixelformat[%d] \n",
        s32VpssIdx,pstParam->bEnable,pstParam->VpssHdl,pstParam->VcapPipeHdl,pstParam->PipeChnHdl,pstParam->enPixelFormat);
    MLOGD("nr attr, nrenable[%d],nrcompressmode[%d],nrmotionmode[%d] \n",pstParam->bNrEn,pstParam->stNrAttr.enCompressMode,
        pstParam->stNrAttr.enNrMotionMode);

    for(i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVportCommCfg(pszIniModule,s32VpssIdx,i,&pstParam->astPortAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vport config");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVprocCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_COMM_VPROC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vpss.common:vpsscnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32VpssCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss comm  vpsscnt[%u] \n",pstParam->u32VpssCnt);

    for(i = 0;i < pstParam->u32VpssCnt && HI_PDT_VCAPDEV_VPSS_MAX_CNT; i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVpssCommCfg(pszIniModule,i,&pstParam->astVpssAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vpss attr");
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVencCommAttr(const HI_CHAR *pszIniModule,HI_S32 s32VencIdx,
                PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"venc.%d",s32VencIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:enable",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:venchdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:modhdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->ModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:chnhdl",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->ChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN,"%s:bindedmod", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBindedMod);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:payload",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enPayload);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:rcmode",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enRcMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:type",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enVencType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("venc[%d] attr, enable[%d],venchdl[%d],bindedmod[%d],modhdl[%d],chnhdl[%d] \n",s32VencIdx,
        pstParam->bEnable,pstParam->VencHdl,pstParam->enBindedMod,pstParam->ModHdl,pstParam->ChnHdl);
    MLOGD("payload[%d,0-h264,1-h265,2-mjpeg,3-jpeg,4-aac],rcmode[%d,0-cbr,1-vbr],type[%d,0-main video,1-sub video,2-single snap,3-sub snap] \n",
       pstParam->enPayload,pstParam->enRcMode,pstParam->enVencType);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVencCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","venc.common:venccnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32VencCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("venc comm  venccnt[%u] \n",pstParam->u32VencCnt);

    for(i = 0;i < pstParam->u32VencCnt && i < HI_PDT_VCAPDEV_VENC_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVencCommAttr(pszIniModule,i,&pstParam->astVencAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "venc comm");
    }
    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadSvpCfg(const HI_CHAR *pszIniModule,HI_PDT_PARAM_VIDEODETECT_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 s32Value = 0;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.com:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.com:algcnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->algCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("videodetect comm: bEnable[%d],algCnt[%d]\n", pstParam->bEnable, pstParam->algCnt);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.yuvsrc:yuv_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->yuvSrc.yuvMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.yuvsrc:mod_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->yuvSrc.modHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.yuvsrc:chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->yuvSrc.chnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.yuvsrc:dump_depth");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->yuvSrc.dumpDepth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("videodetect yuvsrc: yuvmode[%d], modhdl[%d], chnhdl[%d], dumpdepth[%d]\n",pstParam->yuvSrc.yuvMode,
        pstParam->yuvSrc.modHdl, pstParam->yuvSrc.chnHdl, pstParam->yuvSrc.dumpDepth);

    for (i = 0; i < pstParam->algCnt; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d:alg_type", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].algType);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:alg_mode", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.enAlgMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:sad_mode", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.enSadMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:sad_out_ctrl", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.enSadOutCtrl);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:sad_thr", i);

        s32Value = 0;
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->algAttr[i].mdAttr.u16SadThr = (HI_U16)s32Value;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:width", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.u32Width);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr:height", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.u32Height);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("videodetect.algattr.%d.mdattr, alg_mode[%d], sad_mode[%d], sad_out_ctrl[%d], sad_Thr[%d], width[%d], height[%d]\n", i,
          pstParam->algAttr[i].mdAttr.enAlgMode,
          pstParam->algAttr[i].mdAttr.enSadMode,
          pstParam->algAttr[i].mdAttr.enSadOutCtrl ,
          pstParam->algAttr[i].mdAttr.u16SadThr,
          pstParam->algAttr[i].mdAttr.u32Width,
          pstParam->algAttr[i].mdAttr.u32Height);


        s32Value = 0;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.staddctrl:x", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->algAttr[i].mdAttr.stAddCtrl.u0q16X = (HI_U0Q16)s32Value;
        s32Value = 0;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.staddctrl:y", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->algAttr[i].mdAttr.stAddCtrl.u0q16Y = (HI_U0Q16)s32Value;
        MLOGD("videodetect.algattr.%d.mdattr.staddctrl: X[%d], Y[%d] \n", i, pstParam->algAttr[i].mdAttr.stAddCtrl.u0q16X,
            pstParam->algAttr[i].mdAttr.stAddCtrl.u0q16Y);


        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.stcclctrl:mode", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].mdAttr.stCclCtrl.enMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        s32Value = 0;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.stcclctrl:init_area_thr", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->algAttr[i].mdAttr.stCclCtrl.u16InitAreaThr = (HI_U16)s32Value;
        s32Value = 0;
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.stcclctrl:step", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        pstParam->algAttr[i].mdAttr.stCclCtrl.u16Step = (HI_U16)s32Value;
        MLOGD("videodetect.algattr.%d.mdattr.stcclctrl:mode[%d], init_area_thr[%d], step[%d] \n",i,
            pstParam->algAttr[i].mdAttr.stCclCtrl.enMode,
            pstParam->algAttr[i].mdAttr.stCclCtrl.u16InitAreaThr,
            pstParam->algAttr[i].mdAttr.stCclCtrl.u16Step);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.algProcmdattr:md_sensitivity", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].algProcMdAttr.mdSensitivity);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("videodetect.algattr.%d.sensitivity:md_sensitivity[%d]\n",i, pstParam->algAttr[i].algProcMdAttr.mdSensitivity);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "videodetect.algattr.%d.mdattr.algProcmdattr:md_stable_cnt", i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->algAttr[i].algProcMdAttr.mdStableCnt);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("videodetect.algattr.%d.sensitivity:mdStableCnt[%d]\n",i, pstParam->algAttr[i].algProcMdAttr.mdStableCnt);



    }

    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadDispCropCfg(const HI_CHAR *pszIniModule,HI_PDT_MEDIA_DISP_WND_CROP_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "disp.window.crop:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "disp.window.crop:max_h");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxH);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "disp.window.crop:cur_h");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32CurH);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("   window crop enable[%d] u32MaxH[%d], u32CurH[%d]\n",
        pstParam->bEnable, pstParam->u32MaxH, pstParam->u32CurH);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapCommCfg(pszIniModule,&pstParam->stVcapDev);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vcap comm");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVprocCommCfg(pszIniModule ,&pstParam->stVprocCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vproc comm");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVencCommCfg(pszIniModule,&pstParam->stVencCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "venc comm");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadDispCropCfg(pszIniModule, &pstParam->stCropCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "crop config");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadSvpCfg(pszIniModule, &pstParam->stVideoDetectCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "videodetect config");
#endif

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadSpecVBCfg(const HI_CHAR *pszIniModule,
                HI_PDT_MEDIA_VB_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 u32Idx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vb:max_poolcnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxPoolCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (HI_PDT_MEDIA_VB_MAX_POOLS < pstParam->u32MaxPoolCnt)
    {
        MLOGE("Invalid VB MaxPoolCnt[%u beyond %u]\n", pstParam->u32MaxPoolCnt, HI_PDT_MEDIA_VB_MAX_POOLS);
        return HI_FAILURE;
    }
    MLOGD("VB MaxPoolCnt[%u]\n", pstParam->u32MaxPoolCnt);

    for (u32Idx = 0; u32Idx < pstParam->u32MaxPoolCnt; ++u32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vb.pool.%u:blk_size", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astCommPool[u32Idx].u32BlkSize);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vb.pool.%u:blk_count", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astCommPool[u32Idx].u32BlkCnt);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Pool[%u] BlkSize[%uBytes] Count[%u]\n", u32Idx,
            pstParam->astCommPool[u32Idx].u32BlkSize, pstParam->astCommPool[u32Idx].u32BlkCnt);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnSpecCfg(const HI_CHAR *pszIniModule,HI_S32 s32PipeIdx,
                HI_S32 s32ChnIdx,PDT_PARAM_MEDIAMODE_VCAP_PIPE_CHN_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vcap.pipe.%d.chl.%d",s32PipeIdx,s32ChnIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_width",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stDestResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_height",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stDestResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("pipe[%d] chn[%d] dest resolution , dst_width[%d],dst_height[%d] \n",
        s32PipeIdx,s32ChnIdx,pstParam->stDestResolution.u32Width,pstParam->stDestResolution.u32Height);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:src_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("pipe[%d] chn[%d] src_framerate[%d],dst_framerate[%d] \n",
        s32PipeIdx,s32ChnIdx,pstParam->stFrameRate.s32SrcFrameRate,pstParam->stFrameRate.s32DstFrameRate);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeSpecCfg(const HI_CHAR *pszIniModule,HI_S32 s32PipeIdx,
                PDT_PARAM_MEDIAMODE_VCAP_PIPE_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vcap.pipe.%d",s32PipeIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:src_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:isp_Bypass",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->bIspBypass);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vcap pipe[%d] , src_framerate[%d], dst_framerate[%d], isp_Bypass[%d] \n",s32PipeIdx,
        pstParam->stFrameRate.s32SrcFrameRate,pstParam->stFrameRate.s32DstFrameRate,pstParam->bIspBypass);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:isp_framerate",szIniNodePrefix);
    HI_DOUBLE dFrameRate=0;
    s32Ret = HI_CONFACCESS_GetDouble(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_DOUBLE*)&dFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->stIspPubAttr.f32FrameRate = (HI_FLOAT)dFrameRate;

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:isp_width",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stIspPubAttr.stSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:isp_height",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stIspPubAttr.stSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("isp pub attr, isp_framerate[%f], isp_width[%d], isp_height[%d] \n",pstParam->stIspPubAttr.f32FrameRate,
        pstParam->stIspPubAttr.stSize.u32Width,pstParam->stIspPubAttr.stSize.u32Height);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:frameinterrupt_type",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameInterruptAttr.enIntType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:earlyline",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameInterruptAttr.u32EarlyLine);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:scene_param_linear_idx",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32SceneParamLinearIdx);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:scene_param_wdr_idx",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32SceneParamWdrIdx);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vcap pipe[%d] , u32SceneParamLinearIdx[%u],u32SceneParamWdrIdx[%u] \n",
        s32PipeIdx,pstParam->u32SceneParamLinearIdx,pstParam->u32SceneParamWdrIdx);

    for(i = 0 ;i < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeChnSpecCfg(pszIniModule,s32PipeIdx,i, &pstParam->astPipeChnAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeChnSpec");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVcapSpecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_VCAP_DEV_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:sensor_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stSnsAttr.u32SnsMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:sensor_width");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stSnsAttr.stResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:sensor_height");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stSnsAttr.stResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vcapdev  sensor attr, sensor_mode[%d],sensor_width[%d],sensor_height[%d] \n",
        pstParam->stSnsAttr.u32SnsMode,
        pstParam->stSnsAttr.stResolution.u32Width,pstParam->stSnsAttr.stResolution.u32Height);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:bas_width");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stVcapDevAttr.stBasSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vcap.dev:bas_height");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stVcapDevAttr.stBasSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vcap dev attr,bas_width[%u],bas_height[%u] \n",pstParam->stVcapDevAttr.stBasSize.u32Width,
        pstParam->stVcapDevAttr.stBasSize.u32Height);

    for(i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapPipeSpecCfg(pszIniModule,i,&pstParam->astVcapPipeAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeSpec");
    }

    return HI_SUCCESS;

}


static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVportSpecCfg(const HI_CHAR *pszIniModule,HI_S32 s32VpssIdx,
                HI_S32 s32VportIdx,PDT_PARAM_MEDIAMODE_VPSS_PORT_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vpss.%d.vport.%d",s32VpssIdx,s32VportIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:res_width",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:res_height",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:src_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss[%d]vport[%d] attr,res_width[%d],res_height[%d],src_framerate[%d],dst_framerate[%d] \n",
        s32VpssIdx,s32VportIdx,pstParam->stResolution.u32Width,pstParam->stResolution.u32Height,pstParam->stFrameRate.s32SrcFrameRate,pstParam->stFrameRate.s32SrcFrameRate);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVpssSpecCfg(const HI_CHAR *pszIniModule,HI_S32 s32VpssIdx,
                PDT_PARAM_MEDIAMODE_VPSS_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"vpss.%d",s32VpssIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:max_width",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32MaxW);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:max_height",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32MaxH);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:src_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss[%d] spec attr, max_width[%u],max_height[%u],src_framerate[%d],dst_framerate[%d] \n",s32VpssIdx,
        pstParam->u32MaxW,pstParam->u32MaxH,pstParam->stFrameRate.s32SrcFrameRate,pstParam->stFrameRate.s32DstFrameRate);

    for(i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVportSpecCfg(pszIniModule,s32VpssIdx,i,&pstParam->astVportAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vport spec config");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVprocSpecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","vpss.common:vpsscnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32VpssCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vpss spec  vpsscnt[%u] \n",pstParam->u32VpssCnt);

    for(i = 0;i < pstParam->u32VpssCnt && i < HI_PDT_VCAPDEV_VPSS_MAX_CNT; i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVpssSpecCfg(pszIniModule,i,&pstParam->astVpssCfg[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vpss spec attr");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVencSpecAttr(const HI_CHAR *pszIniModule,HI_S32 s32VencIdx,
                PDT_PARAM_MEDIAMODE_VENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix,PDT_INIPARAM_NODE_NAME_LEN,"venc.%d",s32VencIdx);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:res_width",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:res_height",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:bufsize",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32BufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:gop",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32Gop);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:src_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:dst_framerate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:h265bitrate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32H265_BitRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:h264bitrate",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32H264_BitRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s:stat_time",szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32StatTime);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("venc[%d] spec attr, res_width[%u],res_height[%u],bufsize[%u]\n",s32VencIdx,
        pstParam->u32Width,pstParam->u32Height,pstParam->u32BufSize);
    MLOGD("gop[%u],src_framerate[%u],dst_framerate[%u],h265bitrate[%u],h264bitrate[%u] u32StatTime[%uS]\n",
       pstParam->u32Gop,pstParam->u32SrcFrameRate,pstParam->u32DstFrameRate,
       pstParam->u32H265_BitRate,pstParam->u32H264_BitRate,pstParam->u32StatTime);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadVencSpecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","venc.common:venccnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32VencCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("venc comm  venccnt[%u] \n",pstParam->u32VencCnt);

    for(i = 0;i < pstParam->u32VencCnt && i < HI_PDT_VCAPDEV_VENC_MAX_CNT;i++)
    {
        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVencSpecAttr(pszIniModule,i,&pstParam->astVencAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "venc spec");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_CAM_MEDIA_LoadSpecCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = HI_NULL;

    snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"%s","common:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL,&pszString);
    if(NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString,&pstParam->enMediaMode);
        HI_APPCOMM_SAFE_FREE(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    for(i = 0;i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;i++)
    {
        snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"common.vcap.pipe.%d:vcappipe_hdl",i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->astMode[i].VcapPipeHdl);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName,PDT_INIPARAM_NODE_NAME_LEN,"common.vcap.pipe.%d:vivpssmode",i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->astMode[i].enMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("vcap.pipe[%d] vivpssmode, vcappipe_hdl[%d], vivpssmode[%d] \n",i,pstParam->astMode[i].VcapPipeHdl,pstParam->astMode[i].enMode);
    }

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadSpecVBCfg(pszIniModule,&pstParam->stVBCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VB config");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVcapSpecCfg(pszIniModule,&pstParam->stVcapAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vcap attr");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVprocSpecCfg(pszIniModule,&pstParam->stVprocCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "vproc config");

    s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadVencSpecCfg(pszIniModule,&pstParam->stVencCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "venc config");

    return HI_SUCCESS;
}


HI_S32 HI_PDT_INIPARAM_LoadCamMediaCfg(PDT_PARAM_CAM_MEDIA_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  i = 0,j = 0;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        MLOGD("Cam[%d] Media comm Configure:\n", i);
        snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "cam%d_comm", i);

        pstParam[i].s32CamID = i;

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "camera:enable");
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, (HI_S32*)&pstParam[i].bEnable);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "camera:osdshow");
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, (HI_S32*)&pstParam[i].bOsdShow);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "mediamode:count");
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, (HI_S32*)&pstParam[i].u32MediaModeCnt);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("Cam[%d] Rec Media comm Configure:\n", i);
        snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "cam%d_comm_record", i);

        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadCommCfg(szIniModuleName, &pstParam[i].stMediaCommCfg[PDT_PARAM_MEDIAMODE_TYPE_NORM_REC]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniModuleName);

        MLOGD("Cam[%d] Photo Media comm Configure:\n", i);
        snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "cam%d_comm_photo", i);

        s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadCommCfg(szIniModuleName, &pstParam[i].stMediaCommCfg[PDT_PARAM_MEDIAMODE_TYPE_PHOTO]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniModuleName);

        for(j = 0;j < pstParam[i].u32MediaModeCnt && j < PDT_PARAM_MEDIAMODE_CNT;j++)
        {
            MLOGD("Cam[%d] osdshow[%d] MediaMode[%d] Configure:\n", i,pstParam[i].bOsdShow,j);
            snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "cam%d_mediamode%d", i,j);
            s32Ret = PDT_INIPARAM_CAM_MEDIA_LoadSpecCfg(szIniModuleName, &pstParam[i].astMediaModeSpecCfg[j]);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniModuleName);
        }
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

