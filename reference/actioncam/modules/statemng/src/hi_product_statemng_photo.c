/**
 * @file      hi_product_statemng_photo.c
 * @brief     Realize the interface about photo states.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0
 */
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern PDT_STATEMNG_CONTEXT g_stSTATEMNGContext;

static HI_S32 PDT_STATEMNG_GetJPEGUserInfo(HI_PHOTOMNG_JPEG_USERINFO_S* pstJPEGUserInfo)
{
    static HI_CHAR* pszUniqueCameraModel = "HiMobileCam ActionCam";
    pstJPEGUserInfo->u32UniqueCameraModelLen = strlen(pszUniqueCameraModel)+1;
    pstJPEGUserInfo->pUniqueCameraModel = pszUniqueCameraModel;

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetPhotoScene(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_PHOTO_SCENE_E enSettingScene = *(HI_PDT_PHOTO_SCENE_E *)pvPayload;
    MLOGD(YELLOW"set Scene(%d), workmode(%d)\n"NONE, enSettingScene, enSettingWorkMode);

    /** filter workmode */
    if(!(HI_PDT_WORKMODE_SING_PHOTO == enSettingWorkMode ||
         HI_PDT_WORKMODE_DLAY_PHOTO == enSettingWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FALSE;
    }

    HI_PDT_PHOTO_SCENE_E enScene = HI_PDT_PHOTO_SCENE_BUTT;
    HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PHOTO_SCENE, (HI_VOID *)&(enScene));

    /** filter out the same parameter */
    if(enSettingScene == enScene)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/enSettingMediaMode pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingScene);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PHOTO_SCENE, (HI_VOID *)&(enSettingScene));
    PDT_STATEMNG_CHECK_RET(s32Ret,"save Scene param");
    MLOGD(YELLOW"save Scene(%d) param succeed\n"NONE, enSettingScene);

    g_stSTATEMNGContext.enScene = enSettingScene;

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enSettingWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get WorkModeCfg");

    /** generate PhotoMng attr */
    s32Ret = PDT_STATEMNG_GeneratePhotoAttr(enSettingWorkMode,
        &stEnterWorkModeCfg, &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate photo attr");

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode,
        g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
        &stEnterMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stEnterMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutputFormat =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt;
    if(HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == enOutputFormat)
    {
        MLOGD(YELLOW"JPEG, disable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_FALSE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG == enOutputFormat)
    {
        MLOGD(YELLOW"DNG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG == enOutputFormat)
    {
        MLOGD(YELLOW"DNG_JPEG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx;
    }

#ifdef CFG_POST_PROCESS
    if(HI_PDT_PHOTO_SCENE_HDR == enSettingScene)
    {
        g_stSTATEMNGContext.stPhotoProTune.u32WB = 0;
        g_stSTATEMNGContext.stPhotoProTune.u32ISO = 0;
        g_stSTATEMNGContext.stPhotoProTune.enEV = HI_PDT_SCENE_EV_0;
        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us = 0;
        g_stSTATEMNGContext.stPhotoProTune.enMetryType = HI_PDT_SCENE_METRY_TYPE_AVERAGE;
    }
#endif

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg),
        &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    return s32Ret;
}

/** Reset all(meida) */
HI_S32 PDT_STATEMNG_PhotoStatesResetAll(HI_PDT_MEDIA_CFG_S *pstMediaCfg,
    HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting MediaCfg from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(g_stSTATEMNGContext.enCurrentWorkMode,
        g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
        &stSettingMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stSettingMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutputFormat =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt;
    if(HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == enOutputFormat)
    {
        MLOGD(YELLOW"JPEG, disable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_FALSE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG == enOutputFormat)
    {
        MLOGD(YELLOW"DNG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG == enOutputFormat)
    {
        MLOGD(YELLOW"DNG_JPEG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx;
    }

#ifdef CFG_POST_PROCESS
    if ((HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        HI_PDT_PHOTO_SCENE_E enScene = HI_PDT_PHOTO_SCENE_BUTT;
        HI_PDT_PARAM_GetWorkModeParam(g_stSTATEMNGContext.enCurrentWorkMode,
            HI_PDT_PARAM_TYPE_PHOTO_SCENE, (HI_VOID *)&(enScene));

        if(HI_PDT_PHOTO_SCENE_HDR == enScene)
        {
            g_stSTATEMNGContext.stPhotoProTune.u32WB = 0;
            g_stSTATEMNGContext.stPhotoProTune.u32ISO = 0;
            g_stSTATEMNGContext.stPhotoProTune.enEV = HI_PDT_SCENE_EV_0;
            g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us = 0;
            g_stSTATEMNGContext.stPhotoProTune.enMetryType = HI_PDT_SCENE_METRY_TYPE_AVERAGE;
        }
    }
#endif

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg),
        &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetMediaMode(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_MEDIAMODE_E enSettingMediaMode = *(HI_PDT_MEDIAMODE_E *)pvPayload;
    MLOGD(YELLOW"set MediaMode(%d), workmode(%d)\n"NONE, enSettingMediaMode, enSettingWorkMode);

    /** filter out the same parameter */
    if(enSettingMediaMode == g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/enSettingMediaMode pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingMediaMode);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode = enSettingMediaMode;

    if(HI_PDT_WORKMODE_BURST == enSettingWorkMode)
    {
        HI_PDT_JPG_BURST_TYPE_E enCurrentBurstType =  HI_PDT_JPG_BURST_TYPE_BUTT;
        MLOGD(YELLOW"enEnterMediaMode(%d)\n"NONE, g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode);
        s32Ret = HI_PDT_PARAM_GetBurstType(g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode, &enCurrentBurstType);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get burst type");
        MLOGD(YELLOW"dst BurstType(%d), WorkMode(%d)\n"NONE, enCurrentBurstType, enSettingWorkMode);

        switch(enCurrentBurstType)
        {
            case HI_PDT_JPG_BURST_TYPE_3_1:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 3;
                break;
            case HI_PDT_JPG_BURST_TYPE_5_1:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 5;
                break;
            case HI_PDT_JPG_BURST_TYPE_10_1:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 10;
                break;
            case HI_PDT_JPG_BURST_TYPE_15_1:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 15;
                break;
            case HI_PDT_JPG_BURST_TYPE_20_2:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 20;
                break;
            case HI_PDT_JPG_BURST_TYPE_30_1:
            case HI_PDT_JPG_BURST_TYPE_30_2:
            case HI_PDT_JPG_BURST_TYPE_30_3:
                pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 30;
                break;
            default:
            {
                MLOGE(RED"param err\n"NONE);
                return HI_FAILURE;
            }
        }
    }

    /** reset media all */
    s32Ret = PDT_STATEMNG_PhotoStatesResetAll(pstMediaCfg, pstPhotoAttr);
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media all");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save MediaMode param");
    MLOGD(YELLOW"save MediaMode(%d) param succeed\n"NONE, enSettingMediaMode);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetOutputFormat(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PHOTOMNG_OUTPUT_FORMAT_E enSettingOutputFormat = *(HI_PHOTOMNG_OUTPUT_FORMAT_E *)pvPayload;
    MLOGD(YELLOW"set OutputFormat(%d), workmode(%d)\n"NONE, enSettingOutputFormat, enSettingWorkMode);

    /** filter workmode */
    if(!(HI_PDT_WORKMODE_SING_PHOTO == enSettingWorkMode ||
         HI_PDT_WORKMODE_DLAY_PHOTO == enSettingWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FALSE;
    }

    /** filter out the same parameter */
    if(enSettingOutputFormat == pstPhotoAttr->enOutPutFmt)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/enSettingOutputFormat pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingOutputFormat);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    if(HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == enSettingOutputFormat)
    {
        MLOGD(YELLOW"JPEG, disable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_FALSE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG == enSettingOutputFormat)
    {
        MLOGD(YELLOW"DNG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG == enSettingOutputFormat)
    {
        MLOGD(YELLOW"DNG_JPEG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx);
        pstMediaCfg->stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable = HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx;
    }

    /** change setting paramter in pstPhotoAttr */
    pstPhotoAttr->enOutPutFmt = enSettingOutputFormat;

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(pstMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT, (HI_VOID *)&enSettingOutputFormat);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save OutputFormat param");
    MLOGD(YELLOW"save OutputFormat(%d) param succeed\n"NONE, enSettingOutputFormat);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetDelayTime(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_S32 s32SettingDelayTime = *(HI_S32 *)pvPayload;
    MLOGD(YELLOW"set DelayTime(%d), WorkMode(%d)\n"NONE, s32SettingDelayTime, enSettingWorkMode);

    /** filter workmode */
    if(!(HI_PDT_WORKMODE_DLAY_PHOTO == enSettingWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FALSE;
    }

    /** filter out the same parameter */
    if(s32SettingDelayTime == pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.s32Time_sec)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/s32SettingDelayTime pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)s32SettingDelayTime);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** change setting paramter in pstPhotoAttr */
    pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.s32Time_sec = s32SettingDelayTime;

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_DELAY_TIME, (HI_VOID *)&s32SettingDelayTime);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save DelayTime param");
    MLOGD(YELLOW"save DelayTime(%d) param succeed\n"NONE, s32SettingDelayTime);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetLapseInterval(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_S32 s32SettingLapseInterval = *(HI_S32 *)pvPayload;
    MLOGD(YELLOW"set LapseInterval(%d), WorkMode(%d)\n"NONE, s32SettingLapseInterval, enSettingWorkMode);

    /** filter workmode */
    if(!(HI_PDT_WORKMODE_LPSE_PHOTO == enSettingWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FALSE;
    }

    /** filter out the same parameter */
    if(s32SettingLapseInterval == pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/s32SettingDelayTime pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)s32SettingLapseInterval);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** change setting paramter in pstPhotoAttr */
    pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms = s32SettingLapseInterval;

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, (HI_VOID *)&s32SettingLapseInterval);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save LapseInterval param");
    MLOGD(YELLOW"save LapseInterval(%d) param succeed\n"NONE, s32SettingLapseInterval);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetBurstType(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrameRate = 0;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_JPG_BURST_TYPE_E enSettingBurstType = *(HI_PDT_JPG_BURST_TYPE_E *)pvPayload;
    MLOGD(YELLOW"set BurstType(%d), WorkMode(%d)\n"NONE, enSettingBurstType, enSettingWorkMode);

    /** filter workmode */
    if(!(HI_PDT_WORKMODE_BURST == enSettingWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FAILURE;
    }

    /** exchange parameter*/
    HI_S32 s32SettingBurstPhotoCount = 0;
    switch(enSettingBurstType)
    {
        case HI_PDT_JPG_BURST_TYPE_3_1:
            s32SettingBurstPhotoCount = 3;
            u32FrameRate = 3;
            break;
        case HI_PDT_JPG_BURST_TYPE_5_1:
            s32SettingBurstPhotoCount = 5;
            u32FrameRate = 5;
            break;
        case HI_PDT_JPG_BURST_TYPE_10_1:
            s32SettingBurstPhotoCount = 10;
            u32FrameRate = 10;
            break;
        case HI_PDT_JPG_BURST_TYPE_15_1:
            s32SettingBurstPhotoCount = 15;
            u32FrameRate = 15;
            break;
        case HI_PDT_JPG_BURST_TYPE_20_2:
            s32SettingBurstPhotoCount = 20;
            u32FrameRate = 10;
            break;
        case HI_PDT_JPG_BURST_TYPE_30_1:
            s32SettingBurstPhotoCount = 30;
            u32FrameRate = 30;
            break;
        case HI_PDT_JPG_BURST_TYPE_30_2:
            s32SettingBurstPhotoCount = 30;
            u32FrameRate = 15;
            break;
        case HI_PDT_JPG_BURST_TYPE_30_3:
            s32SettingBurstPhotoCount = 30;
            u32FrameRate = 10;
            break;
        default:
        {
            MLOGE(RED"param err\n"NONE);
            return HI_FAILURE;
        }
    }

    HI_PDT_JPG_BURST_TYPE_E enCurrentBurstType = HI_PDT_JPG_BURST_TYPE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
     HI_PDT_PARAM_TYPE_BURST_TYPE, (HI_VOID *)&enCurrentBurstType);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get BurstType");
    MLOGD(YELLOW"curr BurstType(%d), WorkMode(%d)\n"NONE, enCurrentBurstType, enSettingWorkMode);

    /** filter out the same parameter */
    if(enCurrentBurstType == enSettingBurstType)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/s32SettingDelayTime pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingBurstType);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** change setting paramter in pstPhotoAttr */
    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = s32SettingBurstPhotoCount;

    /** find VpssHdl and VPortHdl */
    HI_HANDLE VencHdl = PDT_STATEMNG_PHOTO_VENC_HDL;
    HI_HANDLE ModHdl = 0;
    HI_HANDLE ChnHdl = 0;
    HI_S32 s32Idx;
    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        pstVencCfg = &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[s32Idx]);
        if((HI_TRUE == pstVencCfg->bEnable) &&
           (pstVencCfg->VencHdl == VencHdl))
        {
            MLOGD("find VencHdl(%d)\n", VencHdl);
            ModHdl =  pstVencCfg->ModHdl;
            ChnHdl = pstVencCfg->ChnHdl;
            break;
        }
    }

    /** change vprot frame rate */
    if(HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
    {
        HI_S32 i = 0, j = 0;
        HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
        HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVportAttr = NULL;
        for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
        {
            pstVpssAttr =
                &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i]);
            if((HI_TRUE == pstVpssAttr->bEnable) &&
               (pstVpssAttr->VpssHdl == ModHdl))
            {
                MLOGD("find VpssHdl(%d)\n", pstVpssAttr->VpssHdl);
                for(j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
                {
                    pstVportAttr =
                        &(pstVpssAttr->astVportAttr[j]);
                    if((HI_TRUE == pstVportAttr->bEnable) &&
                       (pstVportAttr->VportHdl == ChnHdl))
                    {
                        MLOGD("orig vport dst frame rate(%d)\n", pstVportAttr->stFrameRate.s32DstFrameRate);
                        pstVportAttr->stFrameRate.s32DstFrameRate = u32FrameRate;
                        MLOGD("update vport dst frame rate(%d)\n", pstVportAttr->stFrameRate.s32DstFrameRate);
                        break;
                    }
                }
            }
        }
    }
    else if(HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)/** change vcap pipe chanel frame rate */
    {
        HI_PDT_MEDIA_VCAP_CFG_S*  pstVcapCfg =
            &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg);

        HI_PDT_MEDIA_VCAP_DEV_ATTR_S*  pstVcapDevAttr = NULL;
        HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = NULL;
        HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
        HI_S32 i = 0, j = 0, k = 0;
        for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
        {
            pstVcapDevAttr = &(pstVcapCfg->astVcapDevAttr[i]);

            if(HI_TRUE == pstVcapDevAttr->bEnable)
            {
                for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
                {
                    pstVcapPipeAttr = &(pstVcapDevAttr->astVcapPipeAttr[j]);

                    if((HI_TRUE == pstVcapPipeAttr->bEnable) &&
                       (pstVcapPipeAttr->VcapPipeHdl == ModHdl))
                    {
                        for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
                        {
                            pstPipeChnAttr = &(pstVcapPipeAttr->astPipeChnAttr[k]);
                            if((HI_TRUE == pstPipeChnAttr->bEnable) &&
                               (pstPipeChnAttr->PipeChnHdl == ChnHdl))
                            {
                                MLOGD("orig vcap pipe chanel frame rate(%d)\n",
                                    pstPipeChnAttr->stFrameRate.s32DstFrameRate);
                                pstPipeChnAttr->stFrameRate.s32DstFrameRate = u32FrameRate;
                                MLOGD("update vcap pipe chanel frame rate(%d)\n",
                                    pstPipeChnAttr->stFrameRate.s32DstFrameRate);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg),
        &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
     HI_PDT_PARAM_TYPE_BURST_TYPE, (HI_VOID *)&enSettingBurstType);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save BurstType param");
    MLOGD(YELLOW"save BurstType(%d) param succeed\n"NONE, enSettingBurstType);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_PhotoStatesSettingMsgProc(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    switch(pstMsg->arg1)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_STATEMNG_PhotoStatesSetMediaMode(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
            s32Ret = PDT_STATEMNG_PhotoStatesSetPhotoScene(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
            s32Ret = PDT_STATEMNG_PhotoStatesSetOutputFormat(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
            s32Ret = PDT_STATEMNG_PhotoStatesSetDelayTime(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_STATEMNG_PhotoStatesSetLapseInterval(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
            s32Ret = PDT_STATEMNG_PhotoStatesSetBurstType(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_STATEMNG_SetEV(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            s32Ret = PDT_STATEMNG_SetExpTime(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_STATEMNG_SetISO(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_STATEMNG_SetWB(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_STATEMNG_SetMetry(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_STATEMNG_SetFlip(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_STATEMNG_SetOSD(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_DIS:
            s32Ret = PDT_STATEMNG_SetDIS(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_STATEMNG_SetLDC(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
        case HI_PDT_PARAM_TYPE_AUDIO:
        default:
            MLOGE(RED"not support type(%d)\n"NONE, pstMsg->arg1);
            return HI_FAILURE;
    }

    return s32Ret;
}

/** enter Photo states */
HI_S32 PDT_STATEMNG_PhotoStatesEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGI(YELLOW"Lorin add -> enter state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    if((HI_PDT_WORKMODE_SING_PHOTO != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_DLAY_PHOTO != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_LPSE_PHOTO != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_BURST != enEnterWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FAILURE;
    }
    else
    {
        g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
        g_stSTATEMNGContext.bRunning = HI_FALSE;
    }

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get WorkModeCfg");

    /** get enter mediamode */
    HI_PDT_PHOTO_COMM_ATTR_S *pstCommAttr = NULL;
    switch(enEnterWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
            pstCommAttr = &(stEnterWorkModeCfg.unModeAttr.stSinglePhotoAttr.stCommAttr);
            g_stSTATEMNGContext.enScene = stEnterWorkModeCfg.unModeAttr.stSinglePhotoAttr.enScene;
            memcpy(&(g_stSTATEMNGContext.stProcAlg),
                &(stEnterWorkModeCfg.unModeAttr.stSinglePhotoAttr.stProcAlg),
                sizeof(HI_PDT_PHOTO_PROCALG_S));
            break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            pstCommAttr = &(stEnterWorkModeCfg.unModeAttr.stDelayPhotoAttr.stCommAttr);
            g_stSTATEMNGContext.enScene = stEnterWorkModeCfg.unModeAttr.stDelayPhotoAttr.enScene;
            memcpy(&(g_stSTATEMNGContext.stProcAlg),
                &(stEnterWorkModeCfg.unModeAttr.stDelayPhotoAttr.stProcAlg),
                sizeof(HI_PDT_PHOTO_PROCALG_S));
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            pstCommAttr = &(stEnterWorkModeCfg.unModeAttr.stLapsePhotoAttr.stCommAttr);
            break;
        case HI_PDT_WORKMODE_BURST:
            pstCommAttr = &(stEnterWorkModeCfg.unModeAttr.stBurstAttr.stCommAttr);
            break;
        default:
        {
            MLOGE(RED"workmode err\n"NONE);
            return HI_FAILURE;
        }
    }
    g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode = pstCommAttr->enMediaMode;
    g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx = pstCommAttr->u8JpgFileTypeIdx;
    g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx = pstCommAttr->u8DngFileTypeIdx;
    g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx = pstCommAttr->u8JpgDngFileTypeIdx;
    MLOGD(YELLOW"enEnterMediaMode(%d) u8JpgFileTypeIdx(%d) "
        "u8DngFileTypeIdx(%d) u8JpgDngFileTypeIdx(%d)\n"NONE,
        g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
        g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx,
        g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx,
        g_stSTATEMNGContext.stPhotoMngCtx.u8JpgDngFileTypeIdx);

    /** generate PhotoMng attr */
    s32Ret = PDT_STATEMNG_GeneratePhotoAttr(enEnterWorkMode,
        &stEnterWorkModeCfg, &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate photo attr");

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enEnterWorkMode, g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
        &stEnterMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stEnterMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    MLOGD(YELLOW"OutPutFmt(%d)\n"NONE,
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt);

    if(HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)
    {
        MLOGD(YELLOW"JPEG, disable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8JpgFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable =
            HI_FALSE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx = pstCommAttr->u8JpgFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)
    {
        MLOGD(YELLOW"DNG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable =
            HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx = pstCommAttr->u8DngFileTypeIdx;
    }
    else if(HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)
    {
        MLOGD(YELLOW"DNG_JPEG, enable thm venc, u8FileTypeIdx(%d)\n"NONE,
            g_stSTATEMNGContext.stPhotoMngCtx.u8DngFileTypeIdx);
        g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[PDT_STATEMNG_THM_VENC_HANDLE].bEnable =
            HI_TRUE;
        g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx = pstCommAttr->u8JpgDngFileTypeIdx;
    }

#ifdef CFG_POST_PROCESS
    if ((HI_PDT_WORKMODE_SING_PHOTO == enEnterWorkMode) ||
        (HI_PDT_WORKMODE_DLAY_PHOTO == enEnterWorkMode))
    {
        HI_PDT_PHOTO_SCENE_E enScene = HI_PDT_PHOTO_SCENE_BUTT;
        HI_PDT_PARAM_GetWorkModeParam(enEnterWorkMode, HI_PDT_PARAM_TYPE_PHOTO_SCENE, (HI_VOID *)&(enScene));

        if(HI_PDT_PHOTO_SCENE_HDR == enScene)
        {
            g_stSTATEMNGContext.stPhotoProTune.u32WB = 0;
            g_stSTATEMNGContext.stPhotoProTune.u32ISO = 0;
            g_stSTATEMNGContext.stPhotoProTune.enEV = HI_PDT_SCENE_EV_0;
            g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us = 0;
            g_stSTATEMNGContext.stPhotoProTune.enMetryType = HI_PDT_SCENE_METRY_TYPE_AVERAGE;
        }
    }
#endif

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** set system configure */
    PDT_STATEMNG_SetSysConfig();

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    MLOGI("Lorin add -> Going to  Publish Switchmode Enent\n");
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, enEnterWorkMode, HI_TRUE, 0L, NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish switch workmode failed(0x%08X)\n"NONE, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_FAILURE;
    }
    else
    {
        MLOGD(YELLOW"publish switch to workmode(%s) msg(0x%08X) succeed\n"NONE,
            pstStateAttr->stState.name, HI_EVENT_STATEMNG_SWITCH_WORKMODE);
        g_stSTATEMNGContext.bInProgress = HI_FALSE;
    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    /** save poweron workmode */
    s32Ret = PDT_STATEMNG_SavePoweronWorkMode(g_stSTATEMNGContext.enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save poweron workmode param");

    if(g_stSTATEMNGContext.bSuspend)
    {
#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(on)");

        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(on)");
#endif
        g_stSTATEMNGContext.bSuspend = HI_FALSE;
    }

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** exit Photo states */
HI_S32 PDT_STATEMNG_PhotoStatesExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");
    enExitWorkMode = HI_PDT_WORKMODE_SING_PHOTO;
    if((HI_PDT_WORKMODE_SING_PHOTO != enExitWorkMode) &&
       (HI_PDT_WORKMODE_DLAY_PHOTO != enExitWorkMode) &&
       (HI_PDT_WORKMODE_LPSE_PHOTO != enExitWorkMode) &&
       (HI_PDT_WORKMODE_BURST != enExitWorkMode))
    {
        MLOGE(RED"workmode err\n"NONE);
        return HI_FAILURE;
    }

    /** exit workmode directly when it is running */
    if(g_stSTATEMNGContext.bRunning)
    {
        s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"stop PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
            return HI_FAILURE;
        }
        else
        {
            /** file data sync */
            sync();

            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.bRunning = HI_FALSE;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        }
    }

#ifdef CONFIG_RAWCAP_ON
    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
       (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
    {
        /** destroy raw capture task */
        HI_PDT_WORKMODE_E enCurrWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
        if ((HI_PDT_WORKMODE_SING_PHOTO == enCurrWorkMode) &&
            (HI_TRUE == g_stSTATEMNGContext.bRawCapTaskCreate))
        {
            s32Ret = HI_RAWCAP_DestroyTask(g_stSTATEMNGContext.RawCapTaskHdl);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE(RED"destroy Rawcap task failed(0x%08X)\n"NONE, s32Ret);
            }
            else
            {
                g_stSTATEMNGContext.bRawCapTaskCreate = HI_FALSE;
            }
        }
    }
#endif

    return s32Ret;
}

/** Photo states message process */
HI_S32 PDT_STATEMNG_PhotoStatesMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(g_stSTATEMNGContext.Mutex, pvArg, pStateID,
        pstMsg, g_stSTATEMNGContext.bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("curr state(%s)\n", pstStateAttr->stState.name);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        {
            MLOGD(YELLOW"process start msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp = HI_TRUE;

                /** publish start success result message */
                MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                g_stSTATEMNGContext.bInProgress = HI_FALSE;
                g_stSTATEMNGContext.bRunning = HI_TRUE;
                MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

                /** set PhotoMng attr */
                s32Ret = HI_PHOTOMNG_SetPhotoAttr(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl,
                    &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
                if(HI_SUCCESS != s32Ret)
                {
                    /** stop photomng task */
                    s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);

                    /**publish stop message */
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                                0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(RED"publish stop event(0x%08X) failed(0x%08X)\n"NONE,
                            HI_EVENT_STATEMNG_STOP, s32Ret);
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_FAILURE;
                    }
                    else
                    {
                        MLOGD(YELLOW"publish stop event(0x%08X) succeed\n"NONE, HI_EVENT_STATEMNG_STOP);
                        g_stSTATEMNGContext.bRunning = HI_FALSE;
                    }
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }

 #ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                       (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        /** create raw capture task */
                        HI_PDT_WORKMODE_E enCurrWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
                        if (HI_PDT_WORKMODE_SING_PHOTO == enCurrWorkMode)
                        {
                            g_stSTATEMNGContext.stRawCapCfg.u32RawDepth = PDT_STATEMNG_RAWCAP_MAX_DEPTH;
                            PDT_STATEMNG_GetRawCapTaskSrc(enCurrWorkMode,
                                &(g_stSTATEMNGContext.stRawCapCfg.stDataSource));
                            g_stSTATEMNGContext.stRawCapCfg.stGetFilenameCb.pfnGetFileNameCb =
                                PDT_STATEMNG_GetRawCapFileNames;
                            g_stSTATEMNGContext.stRawCapCfg.bRawFileSeparated = HI_TRUE;
                            g_stSTATEMNGContext.stRawCapCfg.stDataSource.bDumpYUV = HI_TRUE;

                            s32Ret = HI_RAWCAP_CreateTask(&(g_stSTATEMNGContext.stRawCapCfg),
                                &(g_stSTATEMNGContext.RawCapTaskHdl));
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE(RED"create Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                            }
                            else
                            {
                                MLOGI(YELLOW"create Rawcap task succeed\n"NONE);
                                g_stSTATEMNGContext.bRawCapTaskCreate = HI_TRUE;
                            }
                        }
                    }
#endif

                /** start PhotoMng task */
                s32Ret = HI_PHOTOMNG_TaskStart(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGD(YELLOW"start PhotoMng task failed(0x%08X)"NONE, s32Ret);
                    /** stop photomng task */
                    s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);

                    /**publish stop message */
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                                0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(RED"publish stop event(0x%08X) failed(0x%08X)\n"NONE,
                            HI_EVENT_STATEMNG_STOP, s32Ret);
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_FAILURE;
                    }
                    else
                    {
                        MLOGD(YELLOW"publish stop event(0x%08X) succeed\n"NONE, HI_EVENT_STATEMNG_STOP);
                        g_stSTATEMNGContext.bRunning = HI_FALSE;
                    }
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;

                }
                else
                {
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                       (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        /** trigger raw capture task */
                        HI_PDT_WORKMODE_E enCurrWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
                        if (HI_PDT_WORKMODE_SING_PHOTO == enCurrWorkMode)
                        {
                            s32Ret = HI_RAWCAP_Trigger(g_stSTATEMNGContext.RawCapTaskHdl,
                                g_stSTATEMNGContext.u32RawCapFrameCnt, &(g_stSTATEMNGContext.stTriggerMode));
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE(RED"trigger Rawcap failed(0x%08X)\n"NONE, s32Ret);
                            }
                            else
                            {
                                MLOGD(YELLOW"trigger Rawcap succeed\n"NONE);
                            }

                            s32Ret = HI_RAWCAP_DestroyTask(g_stSTATEMNGContext.RawCapTaskHdl);
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE(RED"destroy Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                            }
                            else
                            {
                                MLOGI(YELLOW"destroy Rawcap task succeed\n"NONE);
                                g_stSTATEMNGContext.bRawCapTaskCreate = HI_FALSE;
                            }
                        }
                    }
#endif
                    MLOGD(YELLOW"start PhotoMng task succeed\n"NONE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                    MLOGD(YELLOW"already started\n"NONE);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_STATEMNG_STOP:
        {
            MLOGD(YELLOW"process stop msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    if(HI_PHOTOMNG_EBUSY == s32Ret)
                    {
                        MLOGD(YELLOW"PhotoMng task busy(0x%08X)\n"NONE, s32Ret);
                        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                            g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                        return HI_PROCESS_MSG_RESULTE_OK;
                    }
                    else
                    {
                        MLOGD(YELLOW"stop PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
                        return HI_FAILURE;
                    }
                }
                else
                {
                    /** file data sync */
                    s32Ret = HI_async();
                    PDT_STATEMNG_CHECK_RET(s32Ret,"async");

                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;
                    g_stSTATEMNGContext.bInProgress = HI_FALSE;
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"no need stop\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            MLOGD(YELLOW"process PhotoMng task end msg(0x%08X)\n"NONE, pstMsg->what);

            /** check IsRunning */
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
#ifdef CFG_POST_PROCESS
                if((HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
                   (HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode))
                {
                    HI_HANDLE VcapPipeHdl =
                        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.stPhotoSRC.VcapPipeHdl;
                    HI_PDT_SCENE_LockAWB(VcapPipeHdl, HI_FALSE);
                    HI_PDT_SCENE_LockAE(VcapPipeHdl, HI_FALSE);
                }
#endif
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    if(HI_PHOTOMNG_EBUSY == s32Ret)
                    {
                        MLOGD(YELLOW"PhotoMng task busy(0x%08X)\n"NONE, s32Ret);
                        return HI_PROCESS_MSG_RESULTE_OK;
                    }
                    else
                    {
                        MLOGD(YELLOW"stop PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
                        return HI_FAILURE;
                    }
                }
                else
                {
                    /** file data async */
                    s32Ret = HI_async();
                    PDT_STATEMNG_CHECK_RET(s32Ret,"async");

                    /**publish stop message */
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                                0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(RED"publish stop event(0x%08X) failed(0x%08X)\n"NONE,
                            HI_EVENT_STATEMNG_STOP, s32Ret);
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_FAILURE;
                    }
                    else
                    {
                        MLOGD(YELLOW"publish stop event(0x%08X) succeed\n"NONE, HI_EVENT_STATEMNG_STOP);
                        g_stSTATEMNGContext.bRunning = HI_FALSE;

                    }
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"no need stop\n"NONE);
                if(g_stSTATEMNGContext.enExitMode != HI_EXIT_MODE_BUTT)
                {
                    MLOGD(YELLOW"upload HI_EVENT_PHOTOMNG_TASK_END for poweroff or reset\n"NONE);
                    return HI_PROCESS_MSG_UNHANDLER;
                }
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_STATEMNG_SETTING:
        {
            MLOGD(YELLOW"process setting msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                /** process all the setting type*/
                s32Ret = PDT_STATEMNG_PhotoStatesSettingMsgProc(pstMsg,
                    &(g_stSTATEMNGContext.stMediaCfg),
                    &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"can not process(%d)\n"NONE, s32Ret);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
                else
                {
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        /** need pre-process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
        {
            if(pstMsg->what == HI_EVENT_STORAGEMNG_DEV_UNPLUGED)
            {
                MLOGD(YELLOW"process unplug msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_POWEROFF)
            {
                MLOGD(YELLOW"process poweroff msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_FACTORY_RESET)
            {
                MLOGD(YELLOW"process factory reset msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_SWITCH_WORKMODE && pstMsg->arg2 != HI_PDT_WORKMODE_UPGRADE)
            {
                MLOGD(YELLOW"upload msg to Base state\n"NONE);
                return HI_PROCESS_MSG_UNHANDLER;
                break;
            }

            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"stop PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;

                    if(pstMsg->what == HI_EVENT_STATEMNG_POWEROFF)
                    {
                        MLOGI(YELLOW"set enExitMode(HI_EXIT_MODE_POWEROFF)\n"NONE);
                        g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_POWEROFF;
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                    else if(pstMsg->what == HI_EVENT_STATEMNG_FACTORY_RESET)
                    {
                        MLOGI(YELLOW"set enExitMode(HI_EXIT_MODE_RESET)\n"NONE);
                        g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_RESET;
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                    else
                    {
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                }
            }
            else
            {
                MLOGD(YELLOW"upload msg to Base state\n"NONE);
                return HI_PROCESS_MSG_UNHANDLER;
            }
            break;
        }

        /** PhotoMng error */
        case HI_EVENT_PHOTOMNG_ERROR:
        /** FileMng space full */
        case HI_EVENT_FILEMNG_SPACE_FULL:
            {
            MLOGD(YELLOW"process msg(0x%08X)\n"NONE, pstMsg->what);
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"stop PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;

                    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                                0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(RED"publish stop event(0x%08X) failed(0x%08X)\n"NONE,
                            HI_EVENT_STATEMNG_STOP, s32Ret);
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_FAILURE;
                    }

                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            break;
        }
#ifdef CONFIG_RAWCAP_ON
        case HI_EVENT_STATEMNG_DEBUG_SWITCH:
        {
            MLOGD(YELLOW"process debug switch msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            if (HI_PDT_WORKMODE_SING_PHOTO == enCurrentWorkMode)
            {
                /** check IsRunning */
                if(HI_FALSE == g_stSTATEMNGContext.bRunning)
                {
                    HI_VOID* pvPayload = pstMsg->aszPayload;
                    HI_BOOL bDebugOn = *(HI_BOOL *)pvPayload;
                    MLOGD(YELLOW"set bDebugOn(%d), workmode(%d)\n"NONE, bDebugOn, enCurrentWorkMode);

                    if(bDebugOn == g_stSTATEMNGContext.bRawCapOn)
                    {
                        MLOGD(YELLOW"same value\n"NONE);
                        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                                g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                        return HI_PROCESS_MSG_RESULTE_OK;
                    }

                    if(HI_PHOTOMNG_OUTPUT_FORMAT_JPEG != g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)
                    {
                        MLOGD(YELLOW"just update context bRawCapOn(not JPEG)\n"NONE);
                        g_stSTATEMNGContext.bRawCapOn = bDebugOn;
                        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                            g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                        return HI_PROCESS_MSG_RESULTE_OK;
                    }

                    if ((HI_FALSE == bDebugOn) &&
                        (HI_TRUE == g_stSTATEMNGContext.bRawCapTaskCreate))
                    {
                        s32Ret = HI_RAWCAP_DestroyTask(g_stSTATEMNGContext.RawCapTaskHdl);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED"destroy RawCap task failed(0x%08X)\n"NONE, s32Ret);
                            PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                                g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                            return HI_PROCESS_MSG_RESULTE_OK;
                        }
                        else
                        {
                            g_stSTATEMNGContext.bRawCapTaskCreate = HI_FALSE;
                        }
                    }

                    g_stSTATEMNGContext.bRawCapOn = bDebugOn;

                    /** reset media all */
                    s32Ret = PDT_STATEMNG_PhotoStatesResetAll(&(g_stSTATEMNGContext.stMediaCfg),
                        &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
                    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media all");

                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
                else
                {
                    MLOGD(YELLOW"task busy\n"NONE);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"no need process\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
#endif
        default:
        {
            MLOGD(YELLOW"upload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    g_stSTATEMNGContext.bInProgress = HI_FALSE;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Photo states, add single/delay/burst/lapse Photo states to HFSM, init PhotoMng module */
HI_S32 PDT_STATEMNG_PhotoStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stSinglePhoto =
        {{HI_PDT_WORKMODE_SING_PHOTO,
          PDT_STATEMNG_STATE_PHOTO_SINGLE,
          PDT_STATEMNG_PhotoStatesEnter,
          PDT_STATEMNG_PhotoStatesExit,
          PDT_STATEMNG_PhotoStatesMsgProc,
          NULL}};
    stSinglePhoto.stState.argv = &stSinglePhoto;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stSinglePhoto.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add SinglePhoto");

    static PDT_STATEMNG_STATE_ATTR_S stDelayPhoto =
        {{HI_PDT_WORKMODE_DLAY_PHOTO,
          PDT_STATEMNG_STATE_PHOTO_DELAY,
          PDT_STATEMNG_PhotoStatesEnter,
          PDT_STATEMNG_PhotoStatesExit,
          PDT_STATEMNG_PhotoStatesMsgProc,
          NULL}};
    stDelayPhoto.stState.argv = &stDelayPhoto;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stDelayPhoto.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add DelayPhoto");

    static PDT_STATEMNG_STATE_ATTR_S stBurstPhoto =
        {{HI_PDT_WORKMODE_BURST,
          PDT_STATEMNG_STATE_MUTIPHOTO_BURST,
          PDT_STATEMNG_PhotoStatesEnter,
          PDT_STATEMNG_PhotoStatesExit,
          PDT_STATEMNG_PhotoStatesMsgProc,
          NULL}};
    stBurstPhoto.stState.argv = &stBurstPhoto;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stBurstPhoto.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add BurstPhoto");

    static PDT_STATEMNG_STATE_ATTR_S stLapsePhoto =
        {{HI_PDT_WORKMODE_LPSE_PHOTO,
          PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE,
          PDT_STATEMNG_PhotoStatesEnter,
          PDT_STATEMNG_PhotoStatesExit,
          PDT_STATEMNG_PhotoStatesMsgProc,
          NULL}};
    stLapsePhoto.stState.argv = &stLapsePhoto;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stLapsePhoto.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add LapsePhoto");

    HI_PHOTOMNG_VENC_OPERATE_S stVencOps = {.pfnStart = NULL, .pfnStop = NULL};
    stVencOps.pfnStart = HI_PDT_MEDIA_VencStart;
    stVencOps.pfnStop = HI_PDT_MEDIA_VencStop;

    /** init APP Common PhotoMng module */
    s32Ret = HI_PHOTOMNG_Init(&stVencOps);
    PDT_STATEMNG_CHECK_RET(s32Ret,"init PhotoMng");

    /** generate PhotoMng task config */
    PDT_STATEMNG_GeneratePhotoMngTaskCfg(&(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoMngTaskConfig));

    /** create PhotoMng tast */
    s32Ret = HI_PHOTOMNG_TaskCreate(&(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoMngTaskConfig),
        &(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl));
    PDT_STATEMNG_CHECK_RET(s32Ret,"create PhotoMng task");

    s32Ret = HI_PHOTOMNG_RegGetJPEGUserInfoCB(PDT_STATEMNG_GetJPEGUserInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret,"RegGetJPEGUserInfoCB");

    return s32Ret;
}

/** deinit PhotoMng module */
HI_S32 PDT_STATEMNG_PhotoStatesDeinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** destroy PhotoMng tast */
    s32Ret = HI_PHOTOMNG_TaskDestroy(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"PhotoMng destroy task");

    /** deinit APP Common PhotoMng module */
    s32Ret = HI_PHOTOMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"PhotoMng Deinit");

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
