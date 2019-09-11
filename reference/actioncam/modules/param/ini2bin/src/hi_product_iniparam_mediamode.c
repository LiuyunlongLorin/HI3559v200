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
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_PLAYBACK))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_PLAYBACK;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_HDMIPREVIEW))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_HDMIPREVIEW;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_HDMIPLAYBACK))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_HDMIPLAYBACK;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_720P_120_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_720P_120_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_720P_240_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_720P_240_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_30_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_30_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_60_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_60_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_90_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_90_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_120_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_120_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1080P_240_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1080P_240_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_30_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_30_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_30_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_30_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_60_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_60_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_1440P_60_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_1440P_60_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_25_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_25_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_30_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_30_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_25_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_25_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_30_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_30_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_60_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_60_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_60_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_60_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_120_REC))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_120_REC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_120_RECSNAP))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_120_RECSNAP;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_PHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_PHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_4K2K_MPHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_4K2K_MPHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_12M_PHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_12M_PHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_12M_MPHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_12M_MPHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_16M_PHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_16M_PHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_16M_MPHOTO))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_16M_MPHOTO;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, HI_PDT_MEDIAMODE_BUTT))
    {
        *penMediaMode = HI_PDT_MEDIAMODE_BUTT;
    }
    else
    {
        MLOGE("Invalid MediaMode[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("MediaMode[%s] Enum[%d]\n", pszEnumStr, *penMediaMode);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_MEDIAMODE_GetWorkModeType(HI_PDT_MEDIAMODE_E enMediaMode,
                PDT_PARAM_WORKMODE_TYPE_E *penWorkModeType)
{
    switch (enMediaMode)
    {
        case HI_PDT_MEDIAMODE_PLAYBACK:
            *penWorkModeType = PDT_PARAM_WORKMODE_TYPE_PLAYBACK;
            break;
        case HI_PDT_MEDIAMODE_HDMIPLAYBACK:
            *penWorkModeType = PDT_PARAM_WORKMODE_TYPE_HDMIPLAYBACK;
            break;
        case HI_PDT_MEDIAMODE_HDMIPREVIEW:
            *penWorkModeType = PDT_PARAM_WORKMODE_TYPE_HDMIPREVIEW;
            break;
        case HI_PDT_MEDIAMODE_720P_120_REC:
        case HI_PDT_MEDIAMODE_720P_240_REC:
        case HI_PDT_MEDIAMODE_1080P_30_REC:
        case HI_PDT_MEDIAMODE_1080P_60_REC:
        case HI_PDT_MEDIAMODE_1080P_90_REC:
        case HI_PDT_MEDIAMODE_1080P_120_REC:
        case HI_PDT_MEDIAMODE_1080P_240_REC:
        case HI_PDT_MEDIAMODE_1440P_30_REC:
        case HI_PDT_MEDIAMODE_1440P_60_REC:
        case HI_PDT_MEDIAMODE_4K2K_25_REC:
        case HI_PDT_MEDIAMODE_4K2K_30_REC:
        case HI_PDT_MEDIAMODE_4K2K_60_REC:
        case HI_PDT_MEDIAMODE_4K2K_120_REC:
            *penWorkModeType =  PDT_PARAM_WORKMODE_TYPE_REC;
            break;
        case HI_PDT_MEDIAMODE_1080P_60_RECSNAP:
        case HI_PDT_MEDIAMODE_1440P_30_RECSNAP:
        case HI_PDT_MEDIAMODE_1440P_60_RECSNAP:
        case HI_PDT_MEDIAMODE_4K2K_25_RECSNAP:
        case HI_PDT_MEDIAMODE_4K2K_30_RECSNAP:
        case HI_PDT_MEDIAMODE_4K2K_60_RECSNAP:
        case HI_PDT_MEDIAMODE_4K2K_120_RECSNAP:
            *penWorkModeType =  PDT_PARAM_WORKMODE_TYPE_RECSNAP;
            break;
        case HI_PDT_MEDIAMODE_4K2K_PHOTO:
        case HI_PDT_MEDIAMODE_12M_PHOTO:
        case HI_PDT_MEDIAMODE_16M_PHOTO:
            *penWorkModeType =  PDT_PARAM_WORKMODE_TYPE_PHOTO;
            break;
        case HI_PDT_MEDIAMODE_4K2K_MPHOTO:
        case HI_PDT_MEDIAMODE_12M_MPHOTO:
        case HI_PDT_MEDIAMODE_16M_MPHOTO:
            *penWorkModeType =  PDT_PARAM_WORKMODE_TYPE_MPHOTO;
            break;
       default:
            MLOGE("Unsupport MediaMode[%d]\n", enMediaMode);
            return HI_FAILURE;

    }
    MLOGD("MediaMode[%d] WorkModeType[%d]\n", enMediaMode, *penWorkModeType);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_GetViVpssModeEnum(const HI_CHAR *pszEnumStr,
                VI_VPSS_MODE_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_OFFLINE_VPSS_OFFLINE))
    {
        *penMode = VI_OFFLINE_VPSS_OFFLINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_OFFLINE_VPSS_ONLINE))
    {
        *penMode = VI_OFFLINE_VPSS_ONLINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_ONLINE_VPSS_OFFLINE))
    {
        *penMode = VI_ONLINE_VPSS_OFFLINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_ONLINE_VPSS_ONLINE))
    {
        *penMode = VI_ONLINE_VPSS_ONLINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_PARALLEL_VPSS_OFFLINE))
    {
        *penMode = VI_PARALLEL_VPSS_OFFLINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VI_PARALLEL_VPSS_PARALLEL))
    {
        *penMode = VI_PARALLEL_VPSS_PARALLEL;
    }
    else
    {
        MLOGE("Invalid VIVPSSMode[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("VIVPSSMode[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_GetWDRModeEnum(const HI_CHAR *pszEnumStr,
                WDR_MODE_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_NONE))
    {
        *penMode = WDR_MODE_NONE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_BUILT_IN))
    {
        *penMode = WDR_MODE_BUILT_IN;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_2To1_LINE))
    {
        *penMode = WDR_MODE_2To1_LINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_2To1_FRAME))
    {
        *penMode = WDR_MODE_2To1_FRAME;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_2To1_FRAME_FULL_RATE))
    {
        *penMode = WDR_MODE_2To1_FRAME_FULL_RATE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_3To1_LINE))
    {
        *penMode = WDR_MODE_3To1_LINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_3To1_FRAME))
    {
        *penMode = WDR_MODE_3To1_FRAME;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_3To1_FRAME_FULL_RATE))
    {
        *penMode = WDR_MODE_3To1_FRAME_FULL_RATE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_4To1_LINE))
    {
        *penMode = WDR_MODE_4To1_LINE;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_4To1_FRAME))
    {
        *penMode = WDR_MODE_4To1_FRAME;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, WDR_MODE_4To1_FRAME_FULL_RATE))
    {
        *penMode = WDR_MODE_4To1_FRAME_FULL_RATE;
    }
    else
    {
        MLOGE("Invalid WDRMode[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("WDRMode[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_GetDISAccuracyEnum(const HI_CHAR *pszEnumStr,
                DIS_MOTION_LEVEL_E *penLevel)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MOTION_LEVEL_HIGH))
    {
        *penLevel = DIS_MOTION_LEVEL_HIGH;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MOTION_LEVEL_NORMAL))
    {
        *penLevel = DIS_MOTION_LEVEL_NORMAL;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, DIS_MOTION_LEVEL_LOW))
    {
        *penLevel = DIS_MOTION_LEVEL_LOW;
    }
    else
    {
        MLOGE("Invalid DISMotionLevel[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("DISMotionLevel[%s] Enum[%d]\n", pszEnumStr, *penLevel);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVBCfg(const HI_CHAR *pszIniModule,
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

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadSensorAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VcapDevIdx, HI_MEDIA_SENSOR_ATTR_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.sensor:res_width", s32VcapDevIdx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.sensor:res_height", s32VcapDevIdx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.sensor:sensor_mode", s32VcapDevIdx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32SnsMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Sensor[%d] Resolution[%ux%u] Mode[%u]\n", s32VcapDevIdx,
        pstParam->stResolution.u32Width, pstParam->stResolution.u32Height, pstParam->u32SnsMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.sensor:wdr_mode", s32VcapDevIdx);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_GetWDRModeEnum(pszString, &pstParam->enWdrMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVcapPipeIspAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VcapIdx, HI_S32 s32VcapPipeIdx, HI_MAPI_PIPE_ISP_ATTR_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_DOUBLE dValue = 0;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.isp",
        s32VcapIdx, s32VcapPipeIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_height", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetDouble(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &dValue);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->f32FrameRate = (HI_FLOAT)dValue;
    MLOGD("Vcap[%d] Pipe[%d] Resolution[%ux%u] FrameRate[%ffps]\n", s32VcapIdx, s32VcapPipeIdx,
        pstParam->stSize.u32Width, pstParam->stSize.u32Height, pstParam->f32FrameRate);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVcapPipeChnAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VcapIdx, HI_S32 s32VcapPipeIdx, HI_S32 s32VcapPipeChnIdx,
                PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_CHN_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.chn.%d",
        s32VcapIdx, s32VcapPipeIdx, s32VcapPipeChnIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_res_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDestResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_res_height", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stDestResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:src_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vcap[%d] Pipe[%d] Chn[%d] Resolution[%ux%u] SrcFrmRate[%dfps] DstFrmRate[%ufps]\n",
        s32VcapIdx, s32VcapPipeIdx, s32VcapPipeChnIdx,
        pstParam->stDestResolution.u32Width, pstParam->stDestResolution.u32Height,
        pstParam->stFrameRate.s32SrcFrameRate, pstParam->stFrameRate.s32DstFrameRate);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ldc_support", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->bSupportLDC);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dis_support", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->bSupportDIS);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    %s LDC in current mediamode\n", pstParam->bSupportLDC ? "Support" : "Not support");
    MLOGD("    %s DIS in current mediamode\n", pstParam->bSupportDIS ? "Support" : "Not support");

    if(pstParam->bSupportDIS)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d.chn.%d.dis",
            s32VcapIdx, s32VcapPipeIdx, s32VcapPipeChnIdx);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:accuracy", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            s32Ret = PDT_INIPARAM_MEDIAMODE_GetDISAccuracyEnum(pszString, &pstParam->enDISAccuracy);
            free(pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        }

        MLOGI(YELLOW"        DIS Attr\n"NONE);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.bEnable);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    %s DIS in current mediamode\n", pstParam->stDisAttr.bEnable ? "Enable" : "Disable");

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:GdcBypass", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.bGdcBypass);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    %s DIS GdcBypass\n", pstParam->stDisAttr.bGdcBypass ? "Enable" : "Disable");

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:MovingSubjectLevel", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.u32MovingSubjectLevel);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    DIS u32MovingSubjectLevel[%u]\n", pstParam->stDisAttr.u32MovingSubjectLevel);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:RollingShutterCoef", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.s32RollingShutterCoef);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    DIS s32RollingShutterCoef[%d]\n", pstParam->stDisAttr.s32RollingShutterCoef);

	    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:Timelag", szIniNodePrefix);
	    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
	        (HI_S32*)&pstParam->stDisAttr.s32Timelag);
	    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
	    MLOGI("    DIS s32Timelag[%d]\n", pstParam->stDisAttr.s32Timelag);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ViewAngle", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.u32ViewAngle);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    DIS u32ViewAngle[%u]\n", pstParam->stDisAttr.u32ViewAngle);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:HorizontalLimit", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.u32HorizontalLimit);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    DIS u32HorizontalLimit[%u]\n", pstParam->stDisAttr.u32HorizontalLimit);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:VerticalLimit", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.u32VerticalLimit);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    DIS u32VerticalLimit[%u]\n", pstParam->stDisAttr.u32VerticalLimit);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:StillCrop", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stDisAttr.bStillCrop);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGI("    %s DIS bStillCrop\n", pstParam->stDisAttr.bStillCrop ? "Enable" : "Disable");

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:SensorTimeSequence", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetSensorTimeSequeceEnum(pszString, &pstParam->enSensorTimeSequece);
            free(pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        }
        MLOGI("    SensorTimeSequece[%d]\n", pstParam->enSensorTimeSequece);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVcapPipeAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32VcapIdx, HI_S32 s32VcapPipeIdx,
                PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32VcapPipeChnIdx;
    HI_S32  s32Value;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.pipe.%d",
        s32VcapIdx, s32VcapPipeIdx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:src_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vcap[%d] Pipe[%d] SrcFrmRate[%d] DstFrmRate[%d]\n",
        s32VcapIdx, s32VcapPipeIdx,
        pstParam->stFrameRate.s32SrcFrameRate, pstParam->stFrameRate.s32DstFrameRate);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.scene:linear_param_idx", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_LINEAR] = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.scene:hdr_param_idx", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_HDR] = s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.scene:mfnr_param_idx", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_MFNR] = s32Value;
    MLOGD("  Scene LinearParamIdx[%u] HdrParamIdx[%u] MfnrParamIdx[%u]\n",
        pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_LINEAR],
        pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_HDR],
        pstParam->au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_MFNR]);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vi_vpss_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_GetViVpssModeEnum(pszString, &pstParam->enViVpssMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    PDT_INIPARAM_MEDIAMODE_LoadVcapPipeIspAttr(pszIniModule, s32VcapIdx, s32VcapPipeIdx, &pstParam->stIspPubAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeIspAttr");

    for (s32VcapPipeChnIdx = 0; s32VcapPipeChnIdx < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++s32VcapPipeChnIdx)
    {
        PDT_INIPARAM_MEDIAMODE_LoadVcapPipeChnAttr(pszIniModule, s32VcapIdx, s32VcapPipeIdx,
            s32VcapPipeChnIdx, &pstParam->astPipeChnAttr[s32VcapPipeChnIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeChnAttr");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVcapDevAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_MEDIAMODE_SPEC_VCAP_DEV_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32VcapPipeIdx;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    PDT_INIPARAM_MEDIAMODE_LoadSensorAttr(pszIniModule, s32Idx, &pstParam->stSnsAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SensorAttr");

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.device:res_width", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.device:res_height", s32Idx);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("VcapDev[%d] Resolution[%ux%u]\n", s32Idx,
        pstParam->stResolution.u32Width, pstParam->stResolution.u32Height);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "vcap.%d.device:wdr_mode", s32Idx);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_GetWDRModeEnum(pszString, &pstParam->enWdrMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    for (s32VcapPipeIdx = 0; s32VcapPipeIdx < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++s32VcapPipeIdx)
    {
        PDT_INIPARAM_MEDIAMODE_LoadVcapPipeAttr(pszIniModule, s32Idx, s32VcapPipeIdx,
            &pstParam->astVcapPipeAttr[s32VcapPipeIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapPipeAttr");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVpssAttr(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_MEDIAMODE_SPEC_VPSS_ATTR_S* pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 s32VportIdx;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:max_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxW);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:max_height", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxH);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:src_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stFrameRate.s32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Vpss[%d] Resolution[%ux%u] SrcFrmRate[%dfps] DstFrmRate[%dfps]\n", s32Idx,
        pstParam->u32MaxW, pstParam->u32MaxH, pstParam->stFrameRate.s32SrcFrameRate,
        pstParam->stFrameRate.s32DstFrameRate);

    /* Load Vport Configure */
    for (s32VportIdx = 0; s32VportIdx < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++s32VportIdx)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "vpss.%d.vport.%d", s32Idx, s32VportIdx);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_width", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astPortAttr[s32VportIdx].stResolution.u32Width);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_height", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astPortAttr[s32VportIdx].stResolution.u32Height);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:src_framerate", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->astPortAttr[s32VportIdx].stFrameRate.s32SrcFrameRate);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_framerate", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->astPortAttr[s32VportIdx].stFrameRate.s32DstFrameRate);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("      Vprot[%d] Resolution[%ux%u] SrcFrmRate[%dfps] DstFrmRate[%dfps]\n", s32VportIdx,
            pstParam->astPortAttr[s32VportIdx].stResolution.u32Width,
            pstParam->astPortAttr[s32VportIdx].stResolution.u32Height,
            pstParam->astPortAttr[s32VportIdx].stFrameRate.s32SrcFrameRate,
            pstParam->astPortAttr[s32VportIdx].stFrameRate.s32DstFrameRate);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadVencCfg(const HI_CHAR *pszIniModule,
                HI_S32 s32Idx, PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:res_height", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stResolution.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:src_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32SrcFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:dst_framerate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->fr32DstFrameRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:gop", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32Gop);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Venc[%d] Resolution[%ux%u] SrcFrmRate[%ufps] DstFrmRate[%ufps] Gop[%u] \n", s32Idx,
        pstParam->stResolution.u32Width, pstParam->stResolution.u32Height,
        pstParam->u32SrcFrameRate, pstParam->fr32DstFrameRate, pstParam->u32Gop);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bufsize", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32BufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:h265bitrate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32H265Bitrate_Kbps);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:h264bitrate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32H264Bitrate_Kbps);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:stat_time", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32StatTime);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    BufSize[%uBytes] H265Bitrate[%u] H264Bitrate[%u] statTime[%uSec]\n",
        pstParam->u32BufSize, pstParam->u32H265Bitrate_Kbps,
        pstParam->u32H264Bitrate_Kbps, pstParam->u32StatTime);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mpf_size_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMpfSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mpf_size_height", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stMpfSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    MPF Size[%ux%u]\n", pstParam->stMpfSize.u32Width, pstParam->stMpfSize.u32Height);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIAMODE_LoadCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Idx;

    /* Load MediaMode Index */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:mediamode");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        free(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    /* WorkModeType */
    s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetWorkModeType(pstParam->enMediaMode, &pstParam->enWorkModeType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "WorkModeType");

    /* Load VB Configure */
    s32Ret = PDT_INIPARAM_MEDIAMODE_LoadVBCfg(pszIniModule, &pstParam->stVBCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VB");

    /* Load Vcapture Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_LoadVcapDevAttr(pszIniModule, s32Idx, &pstParam->stVcapCfg.astVcapDevAttr[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VcapDev");
    }

    /* Load Vprocessor-VPSS Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VPSS_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_LoadVpssAttr(pszIniModule, s32Idx, &pstParam->stVprocCfg.astVpssAttr[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VprocVpss");
    }

    /* Load Vencoder Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIAMODE_LoadVencCfg(pszIniModule, s32Idx, &pstParam->astVencCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Venc");
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadMediaModeConf(PDT_PARAM_MEDIAMODE_SPEC_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_S32  s32Idx;

    for (s32Idx = 0; s32Idx < PDT_PARAM_MEDIAMODE_CNT; ++s32Idx)
    {
        MLOGD("MediaMode[%d] Specified Configure:\n", s32Idx);
        snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s%d", PDT_INIPARAM_MEDIAMODE, s32Idx);

        s32Ret = PDT_INIPARAM_MEDIAMODE_LoadCfg(szIniModuleName, &pstParam[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniModuleName);
        MLOGD("\n\n");
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

