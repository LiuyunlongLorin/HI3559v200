/**
 * @file      hi_product_iniparam_vauleset.c
 * @brief     load configure item valueset
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
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


static HI_S32 PDT_INIPARAM_VALUESET_LoadItemValues(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_PDT_ITEM_VALUESET_S*pstItemValues)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Idx = 0;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:num", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstItemValues->s32Cnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Item[%s] Support ValueCnt[%d]\n", pszNodePrefix, pstItemValues->s32Cnt);

    for (s32Idx = 0; s32Idx < pstItemValues->s32Cnt; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:description%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            snprintf(pstItemValues->astValues[s32Idx].szDesc, HI_PDT_ITEM_VALUE_DESCRIPTION_LEN, "%s", pszString);
            free(pszString);
        }

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:value%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstItemValues->astValues[s32Idx].s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("      [%d] Description[%s] Value[%d]\n", s32Idx,
            pstItemValues->astValues[s32Idx].szDesc, pstItemValues->astValues[s32Idx].s32Value);
    }
    MLOGD("\n\n");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadMediaModeValues(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_PDT_ITEM_VALUESET_S*pstItemValues)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Idx = 0;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:num", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstItemValues->s32Cnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Item[%s] Support ValueCnt[%d]\n", pszNodePrefix, pstItemValues->s32Cnt);

    for (s32Idx = 0; s32Idx < pstItemValues->s32Cnt; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:description%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            snprintf(pstItemValues->astValues[s32Idx].szDesc, HI_PDT_ITEM_VALUE_DESCRIPTION_LEN, "%s", pszString);
            free(pszString);
        }

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:value%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, (HI_PDT_MEDIAMODE_E*)&pstItemValues->astValues[s32Idx].s32Value);
            free(pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        }
        MLOGD("      [%d] Description[%s] Value[%d]\n", s32Idx,
            pstItemValues->astValues[s32Idx].szDesc, pstItemValues->astValues[s32Idx].s32Value);
    }
    MLOGD("\n\n");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadNormRecValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_NORM_REC_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "normrec.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadLoopRecValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_LOOP_REC_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "looprec.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "looprec.time");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stLoopTimeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadSlowRecValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_SLOW_REC_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "slowrec.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadLapseRecValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_LAPSE_REC_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "lapserec.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "lapserec.interval");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stLapseIntervalValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadRecSnapValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_RECSNAP_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "recsnap.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "recsnap.interval");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stLapseIntervalValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadSinglePhotoValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_SING_PHOTO_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "singlephoto.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "singlephoto.scene");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stSceneValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "singlephoto.delay");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stDelayTimeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "singlephoto.outputformat");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stOutputFormatValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadLapsePhotoValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_LAPSE_PHOTO_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "lapsephoto.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "lapsephoto.interval");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stLapseIntervalValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "lapsephoto.exptime");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stExpTimeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadBurstValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_BURST_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  i = 0;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "burst.mediamode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(pszIniModule, szIniNodePrefix, &pstParam->stMediaModeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    for (i = 0; i < PDT_PARAM_MPHOTO_MEDIAMODE_CNT; ++i)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "burst.type%d", i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->astBurstTypeValues[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadVideoProtuneValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_VIDEO_PROTUNE_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.video.wb");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stWBValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.video.iso");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stISOValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.video.metry");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stMetryTypeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.video.ev");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stEVValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadPhotoProtuneValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_PHOTO_PROTUNE_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.snap.wb");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stWBValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.snap.iso");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stISOValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.snap.metry");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stMetryTypeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.snap.ev");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stEVValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "protune.snap.exptime");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stExpTimeValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadDevMngValues(const HI_CHAR *pszIniModule,
                PDT_PARAM_DEVMNG_VALUESET_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "system.screen.dormant");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stSysScreemDormantValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "system.language");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(pszIniModule, szIniNodePrefix, &pstParam->stSystemLanguage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadValueSetConf(PDT_PARAM_CONFITEM_VALUESET_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_VALUESET);

    /* Load Normol Record ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadNormRecValues(szIniModuleName, &pstParam->stNormRec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "NormRec");

    /* Load Loop Record ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadLoopRecValues(szIniModuleName, &pstParam->stLoopRec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LoopRec");

    /* Load Slow Record ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadSlowRecValues(szIniModuleName, &pstParam->stSlowRec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SlowRec");

    /* Load Lapse Record ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadLapseRecValues(szIniModuleName, &pstParam->stLpseRec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LapseRec");

    /* Load RecSnap ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadRecSnapValues(szIniModuleName, &pstParam->stRecSnap);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecSnap");

    /* Load SinglePhoto ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadSinglePhotoValues(szIniModuleName, &pstParam->stSingPhoto);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SinglePhoto");

    /* Load Lapse Photo ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadLapsePhotoValues(szIniModuleName, &pstParam->stLpsePhoto);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "LapsePhoto");

    /* Load Burst ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadBurstValues(szIniModuleName, &pstParam->stBurst);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Burst");

    /* Load Video Protune ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadVideoProtuneValues(szIniModuleName, &pstParam->stVideoProtune);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VideoProtune");

    /* Load Photo Protune ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadPhotoProtuneValues(szIniModuleName, &pstParam->stPhotoProtune);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoProtune");

    /* Load DevMng ValueSet */
    s32Ret = PDT_INIPARAM_VALUESET_LoadDevMngValues(szIniModuleName, &pstParam->stDevMng);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DevMng");

    /* Load Poweron Action */
    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "poweron.action");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->stPowerOnActionValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "usb.mode");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->stUsbMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    /* Load Video Payload Type */
    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "video.enc.payloadtype");
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->stVideoPayloadType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

