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

    /* Load MountPath/RootDir Configure */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "storagemng:mount_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, PDT_INIPARAM_DEVMNG, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szMntPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        HI_APPCOMM_SAFE_FREE(pszString);
    }

    MLOGD("mount path[%s] \n", pstParam->szMntPath);

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
        HI_APPCOMM_SAFE_FREE(pszString);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common.repair:repair_topdirname");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stRepairCfg.szTopDirName, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        HI_APPCOMM_SAFE_FREE(pszString);
    }
    MLOGD("repair enable[%d], parse extra mdat[%d], repair root dir[%s], topdirname[%s]\n",
        pstParam->stRepairCfg.bEnable, pstParam->stRepairCfg.bParseExtraMdat,
        pstParam->stRepairCfg.szRootPath, pstParam->stRepairCfg.szTopDirName);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_FILEMNG_LoadDTCFCfg(const HI_CHAR *pszIniModule,
                HI_FILEMNG_DTCF_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = HI_NULL;

    for(i = 0;i < HI_FILEMNG_DTCF_MAX_PHOTO_DIR; i++)
    {
        /*load dtcf  pre_alloc_cnt*/
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:pre_alloc_cnt");
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocCnt[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        /*load dtcf  pre_alloc_unit*/
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:pre_alloc_unit");
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocUnit[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("dtcf, pre_alloc_cnt[%d],pre_alloc_unit[%d]\n",
        pstParam->u32PreAllocCnt[i],pstParam->u32PreAllocUnit[i]);
    }
    /*load dtcf  share_percent*/
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:share_percent");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u8SharePercent);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    /*load dtcf  warning_stage*/
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf:warning_stage");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WarningStage);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    /*load dtcf  guaranteed_stage*/
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

    MLOGD("dtcf, share_percent[%d],warning_stage[%d],guaranteed_stage[%d] RootDir[%s]\n",
        pstParam->u8SharePercent,pstParam->u32WarningStage,pstParam->u32GuaranteedStage,pstParam->szRootDir);

    /*load dtcf  dir_name*/
    for(i = 0 ;i < DTCF_DIR_BUTT; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "dtcf.dir.name:dir_name%d",i);
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
        MLOGD("dtcf, DirName[%d]:%s \n",i,pstParam->aszDirNames[i]);
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadFileMngConf(HI_PDT_FILEMNG_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_FILEMNG);

    /* Load DevInfo Configure */
    s32Ret = PDT_INIPARAM_FILEMNG_LoadCommCfg(szIniModuleName, &pstParam->stCommCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "filemng comm");

    /* Load Storage Configure */
    s32Ret = PDT_INIPARAM_FILEMNG_LoadDTCFCfg(szIniModuleName, &pstParam->stDtcfCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "dtcf");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

