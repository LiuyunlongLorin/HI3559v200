/**
 * @file      hi_product_iniparam_devmng.c
 * @brief     load device manage parameter
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


static HI_S32 PDT_INIPARAM_DEVMNG_LoadDevInfo(const HI_CHAR *pszIniModule,
                HI_UPGRADE_DEV_INFO_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    /* Load DevInfo Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "devinfo:software_version");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%s", pszString);
        free(pszString);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "devinfo:model");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szModel, HI_APPCOMM_COMM_STR_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("software version[%s], model[%s]\n", pstParam->szSoftVersion, pstParam->szModel);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_DEVMNG_LoadWiFiAPCfg(const HI_CHAR *pszIniModule,
                HI_HAL_WIFI_APMODE_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_S32  s32Value;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "wifi.ap:ssid_hide");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bHideSSID);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("hide ssid[%d]\n", pstParam->bHideSSID);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "wifi.ap:ssid");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stCfg.szWiFiSSID, HI_HAL_WIFI_SSID_LEN, "%s", pszString);
        free(pszString);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "wifi.ap:channel");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (s32Value > 0)
    {
        pstParam->s32Channel = s32Value;
    }
    MLOGD("ssid[%s], chn[%d]\n", pstParam->stCfg.szWiFiSSID, pstParam->s32Channel);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_DEVMNG_LoadStorageCfg(const HI_CHAR *pszIniModule,
                HI_STORAGEMNG_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "storagemng:mount_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szMntPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "storagemng:dev_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szDevPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("mount path[%s], device path[%s]\n", pstParam->szMntPath, pstParam->szDevPath);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_DEVMNG_LoadKeyMngCfg(const HI_CHAR *pszIniModule,
                HI_KEYMNG_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_U32  u32Idx = 0;

    /* Key Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.key:key_cnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stKeyCfg.u32KeyCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    if (pstParam->stKeyCfg.u32KeyCnt > HI_KEYMNG_KEY_IDX_BUTT)
    {
        MLOGE("Invalid key count[%u]\n", pstParam->stKeyCfg.u32KeyCnt);
        return HI_FAILURE;
    }
    MLOGD("Key Count[%u]\n", pstParam->stKeyCfg.u32KeyCnt);

    for (u32Idx = 0; u32Idx < pstParam->stKeyCfg.u32KeyCnt; ++u32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.key:key_type%u", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stKeyCfg.astKeyAttr[u32Idx].enType);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.key:key_id%u", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stKeyCfg.astKeyAttr[u32Idx].s32Id);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Key[%u], type[%d, 0:click, 1:hold] Id[%d]\n", u32Idx,
            pstParam->stKeyCfg.astKeyAttr[u32Idx].enType, pstParam->stKeyCfg.astKeyAttr[u32Idx].s32Id);

        if (HI_KEYMNG_KEY_TYPE_CLICK == pstParam->stKeyCfg.astKeyAttr[u32Idx].enType)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.key:longkey_enable%u", u32Idx);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.bLongClickEnable);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.key:longkey_time%u", u32Idx);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.u32LongClickTime_msec);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("      Click Attr: LongKeyEnable[%d], LongClickTime[%ums]\n",
                pstParam->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.bLongClickEnable,
                pstParam->stKeyCfg.astKeyAttr[u32Idx].unAttr.stClickKeyAttr.u32LongClickTime_msec);
        }
    }

    /* Group Key Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.grpkey:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stGrpKeyCfg.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Group Key Enable[%d]\n", pstParam->stGrpKeyCfg.bEnable);

    for (u32Idx = 0; u32Idx < HI_KEYMNG_KEY_NUM_EACH_GRP; ++u32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keymng.grpkey:key_idx%u", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->stGrpKeyCfg.au32GrpKeyIdx[u32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Group Key[%u] Index[%u]\n", u32Idx, pstParam->stGrpKeyCfg.au32GrpKeyIdx[u32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_DEVMNG_LoadGaugeMngCfg(const HI_CHAR *pszIniModule,
                HI_GAUGEMNG_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "gaugemng:low_level");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32LowLevel);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "gaugemng:ultralow_level");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32UltraLowLevel);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("LowLevel[%d], UltraLowLevel[%d]\n", pstParam->s32LowLevel, pstParam->s32UltraLowLevel);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_DEVMNG_LoadTimedTskAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_TIMEDTASK_ATTR_S *pstAttr)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstAttr->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstAttr->u32Time_sec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("prefix[%s]: enable[%d], time[%us]\n", pszNodePrefix,  pstAttr->bEnable, pstAttr->u32Time_sec);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadDevMngConf(PDT_PARAM_DEVMNG_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_DEVMNG);

    /* Load DevInfo Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadDevInfo(szIniModuleName, &pstParam->stDevInfo);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "devinfo");

    /* Load Storage Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadStorageCfg(szIniModuleName, &pstParam->stStorageCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "storage");

    /* Load KeyMng Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadKeyMngCfg(szIniModuleName, &pstParam->stkeyMngCfg);
    if (HI_SUCCESS != s32Ret)
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "keymng");

    /* Load GaugeMng Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadGaugeMngCfg(szIniModuleName, &pstParam->stGaugeMngCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "gaugemng");

    /* Load WiFi Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadWiFiAPCfg(szIniModuleName, &pstParam->stWiFiApCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "wifi ap");

    /* Load System Dormant Task Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadTimedTskAttr(szIniModuleName, "system.dormant", &pstParam->stSystemDormantAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "system.dormant");

    /* Load Screen Dormant Task Configure */
    s32Ret = PDT_INIPARAM_DEVMNG_LoadTimedTskAttr(szIniModuleName, "screen.dormant", &pstParam->stScreenDormantAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "screen.dormant");

    /* Load Screen Brightness Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "screen:brigtness");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, &pstParam->s32ScreenBrightness);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("screen brigntness[%d]\n", pstParam->s32ScreenBrightness);

    /* Load System Volume Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:system_volume");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, &pstParam->s32SysVolume);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("system volume[%d]\n", pstParam->s32SysVolume);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:system_language");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, &pstParam->s32SystemLanguage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("system language[%d]\n", pstParam->s32SystemLanguage);

    /* Load BootSound Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "bootsound:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, (HI_S32 *)&pstParam->bBootSoundEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("bootsound enable[%d]\n", pstParam->bBootSoundEnable);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "keytone:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, szIniModuleName, szIniNodeName, 0, (HI_S32 *)&pstParam->bKeyToneEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("keytone enable[%d]\n", pstParam->bKeyToneEnable);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

