/**
 * @file      hi_product_iniparam_filemng.c
 * @brief     load filemng parameter
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


static HI_S32 PDT_INIPARAM_FILEMNG_LoadCommCfg(const HI_CHAR *pszIniModule,
                HI_FILEMNG_COMM_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    /* Load MountPath Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "storagemng:mount_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, PDT_INIPARAM_DEVMNG, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szMntPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("mount path[%s]\n", pstParam->szMntPath);

    /* Load FileRepair Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.repair:enable");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stRepairCfg.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.repair:parse_extra_mdat");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stRepairCfg.bParseExtraMdat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.repair:repair_rootdir");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stRepairCfg.szRootPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.repair:repair_topdirname");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stRepairCfg.szTopDirName, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("repair enable[%d], parse extra mdat[%d], repair root dir[%s], topdirname[%s]\n",
        pstParam->stRepairCfg.bEnable, pstParam->stRepairCfg.bParseExtraMdat,
        pstParam->stRepairCfg.szRootPath, pstParam->stRepairCfg.szTopDirName);

    return HI_SUCCESS;
}

#if defined(CONFIG_FILEMNG_DTCF)
static HI_S32 PDT_INIPARAM_FILEMNG_LoadDTCFCfg(const HI_CHAR *pszIniModule,
                HI_FILEMNG_DTCF_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = HI_NULL;
    HI_S32  i = 0;

    /* Load PreAlloc Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:pre_alloc_cnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:pre_alloc_unit");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocUnit);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("PreAllocCnt[%u] PreAllocUnit[%u]\n", pstParam->u32PreAllocCnt, pstParam->u32PreAllocUnit);

    /* Load Stage/RootDir */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:share_percent");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u8SharePercent);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:warning_stage");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WarningStage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:guaranteed_stage");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32GuaranteedStage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:rootdir");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szRootDir, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        HI_APPCOMM_SAFE_FREE(pszString);
    }
    MLOGD("SharePercent[%u] WarningStage[%u] GuaranteedStage[%u] RootDir[%s]\n",
        pstParam->u8SharePercent, pstParam->u32WarningStage, pstParam->u32GuaranteedStage, pstParam->szRootDir);

    /* Load DirName */
    for(i = 0 ; i < DTCF_DIR_BUTT; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf.dir.name:dir_name%d", i);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            if(0 == strncmp(pszString,"\"\"",sizeof("\"\"")))
            {
                memset(pstParam->aszDirNames[i],0x00,HI_DIR_LEN_MAX);
            }
            else
            {
                snprintf(pstParam->aszDirNames[i], HI_DIR_LEN_MAX, "%s", pszString);
            }
            HI_APPCOMM_SAFE_FREE(pszString);
        }
        MLOGD("DirName%02d[%s] \n", i, pstParam->aszDirNames[i]);
    }

    return HI_SUCCESS;
}

#else

static HI_S32 PDT_INIPARAM_FILEMNG_LoadDCFCfg(const HI_CHAR *pszIniModule,
                HI_FILEMNG_DCF_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;
    HI_U32  u32Idx = 0;
    HI_U32  u32FileIdx = 0;
    HI_S32  s32Value;

    /* Load Warning State Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf:warning_stage");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WarningStage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    /* Load Dir FreeChar Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf:dir_freechar");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szDirFreeChar, HI_DCF_WRAPPER_DIR_FREE_CHAR_LENGTH, "%s", pszString);
        free(pszString);
    }
    MLOGD("waring stage[%uMB], Dir freechar[%s]\n", pstParam->u32WarningStage, pstParam->szDirFreeChar);

    /* Load ObjType Count Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf:objtype_cnt");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (pstParam->u8ObjTypeCnt > HI_DCF_WRAPPER_MAX_OBJ_TYPE_CNT)
    {
        MLOGE("Invalid ObjType count[%u]\n", pstParam->u8ObjTypeCnt);
        return HI_FAILURE;
    }
    pstParam->u8ObjTypeCnt = s32Value;
    MLOGD("ObjType Count[%u]\n", pstParam->u8ObjTypeCnt);

    for (u32Idx = 0; u32Idx < pstParam->u8ObjTypeCnt; ++u32Idx)
    {
        /* Load ObjType Configure */
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf.objtype.%u:type", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astObjCfgTbl[u32Idx].enType);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf.objtype.%u:file_freechar", u32Idx);
        s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (NULL != pszString)
        {
            snprintf(pstParam->astObjCfgTbl[u32Idx].szFileFreeChar, HI_DCF_WRAPPER_FILE_FREE_CHAR_LENGTH, "%s", pszString);
            free(pszString);
        }
        MLOGD("ObjType[%u] Type[%d, 0:Record, 1:Photo] FileFreeChar[%s]\n", u32Idx,
            pstParam->astObjCfgTbl[u32Idx].enType, pstParam->astObjCfgTbl[u32Idx].szFileFreeChar);

        /* Load ObjType File Configure */
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf.objtype.%u:file_cnt", u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        if (pstParam->astObjCfgTbl[u32Idx].u8FileCnt > HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ)
        {
            MLOGE("Invalid File count[%u] in objtype\n", pstParam->astObjCfgTbl[u32Idx].u8FileCnt);
            return HI_FAILURE;
        }
        pstParam->astObjCfgTbl[u32Idx].u8FileCnt = s32Value;
        MLOGD("      FileCnt[%u]\n", pstParam->astObjCfgTbl[u32Idx].u8FileCnt);
        for (u32FileIdx = 0; u32FileIdx < pstParam->astObjCfgTbl[u32Idx].u8FileCnt; ++u32FileIdx)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf.objtype.%u:file_suffix%u", u32Idx, u32FileIdx);
            s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            if (NULL != pszString)
            {
                snprintf(pstParam->astObjCfgTbl[u32Idx].aszExtension[u32FileIdx], HI_DCF_WRAPPER_FILE_EXTENSION_LENGTH, "%s", pszString);
                free(pszString);
            }

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dcf.objtype.%u:needrepair%u", u32Idx, u32FileIdx);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->astObjCfgTbl[u32Idx].abNeedRepair[u32FileIdx]);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            MLOGD("      File[%u] Suffix[%s] needrepair[%d]\n", u32FileIdx, pstParam->astObjCfgTbl[u32Idx].aszExtension[u32FileIdx],pstParam->astObjCfgTbl[u32Idx].abNeedRepair[u32FileIdx]);
        }
    }

    return HI_SUCCESS;
}
#endif

HI_S32 HI_PDT_INIPARAM_LoadFileMngConf(HI_PDT_FILEMNG_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_FILEMNG);

    /* Load FileMng Comm Configure */
    s32Ret = PDT_INIPARAM_FILEMNG_LoadCommCfg(szIniModuleName, &pstParam->stCommCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "filemng comm");

    /* Load Dtc/Dtcf Configure */
#if defined(CONFIG_FILEMNG_DTCF)
    s32Ret = PDT_INIPARAM_FILEMNG_LoadDTCFCfg(szIniModuleName, &pstParam->stDtcfCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "dtcf");
#else
    s32Ret = PDT_INIPARAM_FILEMNG_LoadDCFCfg(szIniModuleName, &pstParam->stDcfCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "dcf");
#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

