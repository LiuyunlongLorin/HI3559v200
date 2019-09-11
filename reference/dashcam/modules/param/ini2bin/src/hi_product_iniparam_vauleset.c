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
            HI_APPCOMM_SAFE_FREE(pszString);
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
            HI_APPCOMM_SAFE_FREE(pszString);
        }

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:value%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, (HI_PDT_MEDIAMODE_E*)&pstItemValues->astValues[s32Idx].s32Value);
            HI_APPCOMM_SAFE_FREE(pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        }
        MLOGD("      [%d] Description[%s] Value[%d]\n", s32Idx,
            pstItemValues->astValues[s32Idx].szDesc, pstItemValues->astValues[s32Idx].s32Value);
    }
    MLOGD("\n\n");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_VALUESET_LoadPhotoMediaModeValues(const HI_CHAR *pszIniModule,
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
            HI_APPCOMM_SAFE_FREE(pszString);
        }

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:value%d", pszNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, (HI_PDT_MEDIAMODE_E*)&pstItemValues->astValues[s32Idx].s32Value);
            HI_APPCOMM_SAFE_FREE(pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        }
        MLOGD("      [%d] Description[%s] Value[%d]\n", s32Idx,
            pstItemValues->astValues[s32Idx].szDesc, pstItemValues->astValues[s32Idx].s32Value);
    }
    MLOGD("\n\n");

    return HI_SUCCESS;
}


HI_S32 HI_PDT_INIPARAM_LoadValueSetConf(PDT_PARAM_CONFITEM_VALUESET_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_VALUESET);

    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        /* Load cam enc Payload Type */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.enc.payloadtype",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astVideoPayloadType[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam mediamode */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.mediamode.record",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadMediaModeValues(szIniModuleName, szIniNodePrefix, &pstParam->astMediaModeValues[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load photo cam mediamode */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.mediamode.photo",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadPhotoMediaModeValues(szIniModuleName, szIniNodePrefix, &pstParam->astPhotoMediaModeValues[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam wdr status */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.wdr.status",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astWdrState[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam video mode */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.video.mode",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astVideoMode[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam ldc status */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.ldc.status",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astLdcState[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam flip status */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.flip.status",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astFlipState[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);

        /* Load cam mirror status */
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "cam.%d.mirror.status",i);
        s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, szIniNodePrefix, &pstParam->astMirrorState[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodePrefix);
    }

    /*load screen dormant values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "screen.dormant", &pstParam->stScreenDormantValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "screen.dormant");

    /*load screen brightness values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "screen.brightness", &pstParam->stScreenBrightnessValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "screen.brightness");

    /*load volume values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "volume", &pstParam->stVolume);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "volume");

    /*load drive sensitivity values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "drive_sensitivity", &pstParam->stSensitivityLevel);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "drive_sensitivity");

    /*load parking sensitivity values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "parking_sensitivity", &pstParam->stParkingLevel);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "parking_sensitivity");

    /*load usb mode values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "usb.mode", &pstParam->stUsbMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "usb.mode");

    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "system.language", &pstParam->stSystemLanguage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "system.language");

    /*load split time values*/
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName,"splite_time",&pstParam->stRecSplitTime);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret,"splite_time");

    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "record.lapse.interval", &pstParam->stLapseRecIntervalTime);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "record.lapse.interval");

    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "record.type", &pstParam->stRecordType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "record.type");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_INIPARAM_VALUESET_LoadItemValues(szIniModuleName, "md.sensitivity", &pstParam->stMdSensitivity);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "record.type");
#endif

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

