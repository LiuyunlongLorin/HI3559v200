/**
 * @file      hi_product_scene_ini2bin.c
 * @brief     ini2bin implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */
#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_scene_setparam.h"
#include "hi_confaccess.h"
#include "hi_product_scene_iniparam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

static HI_S64 s_as64LineNum[5000];

static HI_S32  PDT_SCENE_GetNumbersInOneLine(HI_CHAR* pszInputLine)
{
    HI_CHAR*    pszVRBegin     = pszInputLine;
    HI_CHAR*    pszVREnd       = pszVRBegin;
    HI_U32      u32PartCount = 0;
    HI_CHAR     szPart[20] = {0};
    HI_U32      u32WholeCount = 0;
    HI_S32      s32Length = strlen(pszInputLine);
    HI_U64      u64Hex;

    HI_S32      i = 0;
    HI_BOOL     bHex = HI_FALSE;
    memset(s_as64LineNum, 0 , sizeof(HI_S64) * 5000);
    while ((pszVREnd != NULL))
    {
        if ((u32WholeCount > s32Length) || (u32WholeCount == s32Length))
        {
            break;
        }

        while ((*pszVREnd != '|') && (*pszVREnd != '\0') && (*pszVREnd != ','))
        {
            if (*pszVREnd == 'x')
            {
                bHex= HI_TRUE;
            }
            pszVREnd++;
            u32PartCount++;
            u32WholeCount++;
        }

        memcpy(szPart, pszVRBegin, u32PartCount);

        if (bHex == HI_TRUE)
        {
            HI_CHAR* pszstr;
            u64Hex = (HI_U64)strtoll(szPart + 2, &pszstr, 16);
            s_as64LineNum[i] = u64Hex;
        }
        else
        {
            s_as64LineNum[i] = atoll(szPart);
        }

        memset(szPart, 0, 20);
        u32PartCount = 0;
        pszVREnd++;
        pszVRBegin = pszVREnd;
        u32WholeCount++;
        i++;
    }

    return i;
}
#ifdef CONFIG_SCENEAUTO_SUPPORT
HI_S32 PDT_SCENE_LoadStaticStatistics(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_STATISTICSCFG_S* pstStaticStatistics)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_statistics:ExpWeight_0");/*AEWeight*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < AE_ZONE_ROW; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_statistics:ExpWeight_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                printf("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < AE_ZONE_COLUMN; u32IdxN++)
                {
                    pstStaticStatistics->au8AEWeight[u32IdxM][u32IdxN] = (HI_U8)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_statistics:AFEnable");/*AFEnable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_statistics:AFEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_statistics:AFEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticStatistics->bAFEnable = (HI_BOOL)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticAE(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AE_S* pstStaticAe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = HI_NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****AERunInterval****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AERunInterval");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AERunInterval", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AERunInterval failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u8AERunInterval= (HI_U8)s32Value;
    }

    /****AEGainSepCfg****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AEGainSepCfg");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AEGainSepCfg", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AEGainSepCfg failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->bAEGainSepCfg= (HI_BOOL)s32Value;
    }

    /****PriorFrame****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:PriorFrame");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:PriorFrame", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:PriorFrame failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u8PriorFrame= (HI_U8)s32Value;
    }

    /****AERouteExValid****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AERouteExValid");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AERouteExValid", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoExpTimeMax failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->bAERouteExValid = (HI_BOOL)s32Value;
    }

    /****MaxTime****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:MaxTime");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:MaxTime", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:MaxTime failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32MaxTime= (HI_U32)s32Value;
    }

    /****AutoSysGainMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSysGainMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoSysGainMax", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoSysGainMax failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32AutoSysGainMax = (HI_U32)s32Value;
    }

    /****AutoSpeed****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSpeed");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoSpeed", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoSpeed failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u8AutoSpeed = (HI_U8)s32Value;
    }

    /****AutoISPDGainMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoISPDGainMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoISPDGainMax", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoISPDGainMax failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32AutoISPDGainMax = (HI_U32)s32Value;
    }

    /****AutoISPDGainMin****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoISPDGainMin");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoISPDGainMin", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoISPDGainMin failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32AutoISPDGainMin = (HI_U32)s32Value;
    }

    /****AutoDGainMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoDGainMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoDGainMax", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoDGainMax failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32AutoDGainMax = (HI_U32)s32Value;
    }

    /****AutoDGainMin****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoDGainMin");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoDGainMin", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoDGainMin failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u32AutoDGainMin = (HI_U32)s32Value;
    }

    /****AutoTolerance****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoTolerance");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoTolerance", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoTolerance failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u8AutoTolerance = (HI_U32)s32Value;
    }

    /****AutoBlackDelayFrame****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoBlackDelayFrame");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoBlackDelayFrame", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoTolerance failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u16BlackDelayFrame = (HI_U32)s32Value;
    }

    /****AutoTolerance****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoWhiteDelayFrame");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:AutoWhiteDelayFrame", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:AutoTolerance failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u16WhiteDelayFrame = (HI_U32)s32Value;
    }

    /****ChangeFpsEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:ChangeFpsEnable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ae:ChangeFpsEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ae:ChangeFpsEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticAe->u8ChangeFpsEnable = (HI_U32)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticAeRoute(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AEROUTE_S* pstStaticAeRoute)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****TotalNum****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aeroute:TotalNum");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_aeroute:TotalNum", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_aeroute:TotalNum failed\n");
            return HI_FAILURE;
        }
        pstStaticAeRoute->u32TotalNum = (HI_U32)s32Value;
    }

    /****RouteIntTime****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aeroute:RouteIntTime");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRoute->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRoute->au32IntTime[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****RouteSysGain****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aeroute:RouteSysGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRoute->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRoute->au32SysGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticSFAeRoute(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AEROUTE_S* pstStaticSFAeRoute)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****TotalNum****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sf_aeroute:TotalNum");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_sf_aeroute:TotalNum", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_sf_aeroute:TotalNum failed\n");
            return HI_FAILURE;
        }
        pstStaticSFAeRoute->u32TotalNum = (HI_U32)s32Value;
    }

    /****RouteIntTime****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sf_aeroute:RouteIntTime");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticSFAeRoute->u32TotalNum; u32IdxM++)
        {
            pstStaticSFAeRoute->au32IntTime[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****RouteSysGain****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sf_aeroute:RouteSysGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticSFAeRoute->u32TotalNum; u32IdxM++)
        {
            pstStaticSFAeRoute->au32SysGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticAeRouteEX(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AEROUTEEX_S* pstStaticAeRouteEx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aerouteex:TotalNum");/*Tolerance*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_aerouteex:TotalNum", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_aerouteex:TotalNum failed\n");
            return HI_FAILURE;
        }
        pstStaticAeRouteEx->u32TotalNum = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXIntTime");/*RouteEXIntTime*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRouteEx->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRouteEx->au32IntTime[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXAGain");/*RouteEXAGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRouteEx->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRouteEx->au32Again[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXDGain");/*RouteEXDGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRouteEx->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRouteEx->au32Dgain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXISPDGain");/*RouteEXISPDGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticAeRouteEx->u32TotalNum; u32IdxM++)
        {
            pstStaticAeRouteEx->au32IspDgain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticWdrExposure(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_WDREXPOSURE_S *pstStaticWdrExposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioType");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:ExpRatioType", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:ExpRatioType failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->u8ExpRatioType = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpWDRTolerance");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:ExpWDRTolerance", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:ExpRatioMax failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->u16Tolerance = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpWDRSpeed");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:ExpWDRSpeed", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:ExpRatioMax failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->u16Speed = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioMax");/*ExpRatioMax*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:ExpRatioMax", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:ExpRatioMax failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->u32ExpRatioMax = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioMin");/*ExpRatioMin*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_wdrexposure:ExpRatioMin", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_wdrexposure:ExpRatioMin failed\n");
            return HI_FAILURE;
        }
        pstStaticWdrExposure->u32ExpRatioMin = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatio");/*ExpRatio*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < EXP_RATIO_NUM; u32IdxM++)
        {
            pstStaticWdrExposure->au32ExpRatio[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticFsWdr(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_FSWDR_S *pstStaticFsWdr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_fswdr:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_fswdr:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_fswdr:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticFsWdr->bUsed= (HI_BOOL)s32Value;
    }

    /****WDRMergeMode****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_fswdr:WDRMergeMode");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_fswdr:WDRMergeMode", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_fswdr:WDRMergeMode failed\n");
            return HI_FAILURE;
        }
        pstStaticFsWdr->WDRMergeMode = (HI_U8)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticAWB(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AWB_S *pstStaticAWB)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****StaticWB****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:StaticWB");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 4; u32IdxM++)
        {
            pstStaticAWB->au16StaticWB[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****CurvePara****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:CurvePara");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 6; u32IdxM++)
        {
            pstStaticAWB->as32CurvePara[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Speed****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:Speed");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:Speed", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:Speed failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16Speed = s32Value;
    }

    /****LowColorTemp****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:LowColorTemp");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:LowColorTemp", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:LowColorTemp failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16LowColorTemp = s32Value;
    }

    /****LowStop****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:LowStop");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:LowStop", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:LowStop failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16LowStop = s32Value;
    }

    /****LowStart****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:LowStart");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:LowStart", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:LowStart failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16LowStart = s32Value;
    }

    /****OutThresh****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:OutThresh");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:OutThresh", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:OutThresh failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u32OutThresh = s32Value;
    }

    /****OutShiftLimit****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:OutShiftLimit");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:OutShiftLimit", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:OutShiftLimit failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u8OutShiftLimit = s32Value;
    }

    /****HighStart****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:HighStart");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:HighStart", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:HighStart failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16HighStart = s32Value;
    }

    /****HighStop****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:HighStop");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:HighStop", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:HighStop failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u16HighStop = s32Value;
    }

    /****Tolerance****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:Tolerance");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:Tolerance", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:Tolerance failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u8Tolerance = (HI_U8)s32Value;
    }

    /****ShiftLimit****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:ShiftLimit");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:ShiftLimit", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:ShiftLimit failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->u8ShiftLimit = s32Value;
    }

    /****FineTunEn****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:FineTunEn");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:FineTunEn", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:FineTunEn failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->bFineTunEn = s32Value;
    }

    /****AWBZoneWtEn****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AWBZoneWtEn");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AWBZoneWtEn", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AWBZoneWtEn failed\n");
            return HI_FAILURE;
        }
        pstStaticAWB->bAWBZoneWtEn = s32Value;
    }

    /****ZoneWt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:ZoneWt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < (HI_PDT_SCENE_AWB_ZONE_ORIG_ROW*HI_PDT_SCENE_AWB_ZONE_ORIG_COLUMN); u32IdxM++)
        {
            pstStaticAWB->au8ZoneWt[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticSaturation(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_SATURATION_S *pstStaticSaturation)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_saturation:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_saturation:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_saturation:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticSaturation->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_saturation:Saturation");/*Saturation*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSaturation->au8Sat[u32IdxM] = (HI_U8)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticCCM(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CCM_S *pstStaticCCM)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0, u32IdxN = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ccm:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ccm:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticCCM->bUsed= (HI_BOOL)s32Value;
    }

    /****TotalNum****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:TotalNum");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ccm:TotalNum", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ccm:TotalNum failed\n");
            return HI_FAILURE;
        }
        pstStaticCCM->u32TotalNum = s32Value;
    }

    /****ColorTemp****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:ColorTemp");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstStaticCCM->u32TotalNum; u32IdxM++)
        {
            pstStaticCCM->au16AutoColorTemp[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****CCM****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:CCM_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < pstStaticCCM->u32TotalNum; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:CCM_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < CCM_MATRIX_SIZE; u32IdxN++)
                {
                    pstStaticCCM->au16AutoCCM[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticClut(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CLUT_S* pstStaticClut)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_clut:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticClut->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticClut->bEnable= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:GainR");/*GainR*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:GainR", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:GainR failed\n");
            return HI_FAILURE;
        }

        pstStaticClut->u32GainR = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:GainG");/*GainG*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:GainG", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:GainG failed\n");
            return HI_FAILURE;
        }

        pstStaticClut->u32GainG = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:GainB");/*GainB*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:GainB", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:GainB failed\n");
            return HI_FAILURE;
        }

        pstStaticClut->u32GainB = (HI_U32)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDrc(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_DRC_S *pstStaticDrc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_drc:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_drc:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_drc:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticDrc->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_drc:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_drc:bEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_drc:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticDrc->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_drc:CurveSelect");/*CurveSelect*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_drc:CurveSelect", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_drc:CurveSelect failed\n");
            return HI_FAILURE;
        }
        pstStaticDrc->u8CurveSelect = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_drc:DRCOpType");/*DRCOpType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_drc:DRCOpType", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_drc:DRCOpType failed\n");
            return HI_FAILURE;
        }
        pstStaticDrc->u8DRCOpType = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_drc:DRCToneMappingValue");/*DRCToneMappingValue*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < HI_ISP_DRC_TM_NODE_NUM; u32IdxM++)
        {
            pstStaticDrc->au16ToneMappingValue[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDehaze(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_DEHAZE_S *pstStaticDehaze)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dehaze:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dehaze:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dehaze:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticDehaze->bUsed= (HI_BOOL)s32Value;
    }

    /****bEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dehaze:bEnable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dehaze:bEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dehaze:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticDehaze->bEnable = (HI_BOOL)s32Value;
    }

    /****bDehazeUserLutEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dehaze:bDehazeUserLutEnable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dehaze:bDehazeUserLutEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dehaze:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticDehaze->bUserLutEnable = (HI_BOOL)s32Value;
    }

    /****DehazeOpType****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dehaze:DehazeOpType");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dehaze:DehazeOpType", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dehaze:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticDehaze->u8DehazeOpType = (HI_BOOL)s32Value;
    }

    /****DehazeLut****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dehaze:DehazeLut");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 256; u32IdxM++)
        {
            pstStaticDehaze->au8DehazeLut[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticLDCI(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_LDCI_S* pstStaticLdci)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ldci:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ldci:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticLdci->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ldci:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ldci:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticLdci->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:LDCIOpType");/*LDCIOpType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ldci:LDCIOpType", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ldci:LDCIOpType failed\n");
            return HI_FAILURE;
        }
        pstStaticLdci->u8LDCIOpType = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:LDCIGaussLPFSigma");/*LDCIGaussLPFSigma*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ldci:LDCIGaussLPFSigma", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ldci:LDCIGaussLPFSigma failed\n");
            return HI_FAILURE;
        }
        pstStaticLdci->u8GaussLPFSigma = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHePosWgt");/*AutoHePosWgt*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHePosWgt[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHePosSigma");/*AutoHePosSigma*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHePosSigma[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHePosMean");/*AutoHePosMean*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHePosMean[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHeNegWgt");/*AutoHeNegWgt*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHeNegWgt[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHeNegSigma");/*AutoHeNegSigma*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHeNegSigma[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoHeNegMean");/*AutoHeNegMean*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au8AutoHeNegMean[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:AutoBlcCtrl");/*AutoBlcCtrl*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticLdci->au16AutoBlcCtrl[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDemosaic(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_DEMOSAIC_S *pstStaticDemosaic)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_Demosaic:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_Demosaic:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticDemosaic->bUsed= (HI_BOOL)s32Value;
    }

    /****DirStr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:DirStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8NonDirStr[u32IdxM] = (HI_U8)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****MFDetailEhcStr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:MFDetailEhcStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8NonDirMFDetailEhcStr[u32IdxM] = (HI_U8)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DetailSmoothStr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:DetailSmoothStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au16DetailSmoothStr[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticNr(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_NR_S* pstStaticNr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_nr:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_nr:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticNr->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_nr:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_nr:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticNr->bEnable= (HI_BOOL)s32Value;
    }

    /****CoringRatio****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoringRatio");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < HI_ISP_BAYERNR_LUT_LENGTH; u32IdxM++)
        {
            pstStaticNr->au16CoringRatio[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****CoringWgt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoringWgt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticNr->au16CoringWgt[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****FineStr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:FineStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticNr->au8FineStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticCAC(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CAC_S* pstStaticCAC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_localcac:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_localcac:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_localcac:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticCAC->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_localcac:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_localcac:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_localcac:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticCAC->bEnable= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_localcac:DePurpleCrStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticCAC->au8DePurpleCrStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_localcac:DePurpleCbStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticCAC->au8DePurpleCbStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDPC(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_DP_S *pstStaticDPC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dpc:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dpc:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dpc:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticDPC->bUsed= (HI_BOOL)s32Value;
    }

    /****bEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dpc:Enable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_dpc:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_dpc:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticDPC->bEnable = (HI_BOOL)s32Value;
    }

    /*Strength*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dpc:Strength");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDPC->au16Strength[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*BlendRatio*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_dpc:BlendRatio");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDPC->au16BlendRatio[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticShading(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_SHADING_S* pstStaticShading)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_shading:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_shading:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticShading->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_shading:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_shading:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticShading->bEnable= (HI_BOOL)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDE(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_DE_S *pstStaticDE)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_de:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_de:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_de:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticDE->bUsed= (HI_BOOL)s32Value;
    }

    /****bEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_de:Enable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_de:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_de:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticDE->bEnable = (HI_BOOL)s32Value;
    }

    /*GlobalGain*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_de:GlobalGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GlobalGain[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*GainLF*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_de:GainLF");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GainLF[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*GainHF*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_de:GainHF");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GainHF[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticSharpen(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_SHARPEN_S* pstStaticSharpen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_sharpen:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_sharpen:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticSharpen->bUsed= (HI_BOOL)s32Value;
    }

    /*Enable*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:Enable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_sharpen:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticSharpen->bEnable= (HI_BOOL)s32Value;
    }

    /*AutoLumaWgt*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoLumaWgt_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < ISP_SHARPEN_GAIN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoLumaWgt_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxN++)
                {
                    pstStaticSharpen->au8LumaWgt[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    /*AutoTextureStr*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureStr_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < ISP_SHARPEN_GAIN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureStr_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxN++)
                {
                    pstStaticSharpen->au16AutoTextureStr[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    /*AutoEdgeStr*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeStr_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < ISP_SHARPEN_GAIN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeStr_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxN++)
                {
                    pstStaticSharpen->au16AutoEdgeStr[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    /*AutoOverShoot*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoOverShoot");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8AutoOverShoot[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoUnderShoot*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoUnderShoot");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8AutoUnderShoot[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoShootSupStr*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoShootSupStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8ShootSupStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****AutoShootSupAdj****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoShootSupAdj");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8ShootSupAdj[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****AutoTextureFreq****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureFreq");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au16AutoTextureFreq[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoEdgeFreq*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeFreq");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au16AutoEdgeFreq[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoDetailCtrl*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoDetailCtrl");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8AutoDetailCtrl[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoDetailCtrlThr*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoDetailCtrlThr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8DetailCtrlThr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoEdgeFiltStr*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeFiltStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8EdgeFiltStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoRGain*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoRGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8RGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoBGain*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoBGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8BGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoSkinGain*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoSkinGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8SkinGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*AutoMaxSharpGain*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoMaxSharpGain");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au16MaxSharpGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticCa(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CA_S* pstStaticCa)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ca:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ca:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ca:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticCa->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ca:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ca:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ca:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticCa->bEnable= (HI_BOOL)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticCrossTalk(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CROSSTALK_S* pstStaticCrossTalk)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_crosstalk:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_crosstalk:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_crosstalk:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticCrossTalk->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_crosstalk:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_crosstalk:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_crosstalk:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticCrossTalk->bEnable= (HI_BOOL)s32Value;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticQP(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_QP_S* pstStaticQP)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****Used****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_HIERARCHICAL_QP:Used");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_HIERARCHICAL_QP:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_HIERARCHICAL_QP:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticQP->bUsed = (HI_BOOL)s32Value;
    }

    /****HierarchicalQpEn****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_HIERARCHICAL_QP:HierarchicalQpEn");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_HIERARCHICAL_QP:HierarchicalQpEn", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_HIERARCHICAL_QP:HierarchicalQpEn failed\n");
            return HI_FAILURE;
        }
        pstStaticQP->bHierarchicalQpEn= (HI_BOOL)s32Value;
    }

    /****HierarchicalQpDelta****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_HIERARCHICAL_QP:HierarchicalQpDelta");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 4; u32IdxM++)
        {
            pstStaticQP->as32HierarchicalQpDelta[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****HierarchicalFrameNum****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_HIERARCHICAL_QP:HierarchicalFrameNum");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 4; u32IdxM++)
        {
            pstStaticQP->as32HierarchicalFrameNum[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicAE(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_AE_S* pstDynamicAe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_ae:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_ae:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicAe->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:aeExpCount");/*aeExpCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_ae:aeExpCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_ae:aeExpCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicAe->u8AEExposureCnt = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:aeExpLtoHThresh");/*aeExpLtoHThresh*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicAe->u8AEExposureCnt; u32IdxM++)
        {
            pstDynamicAe->au64ExpLtoHThresh[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:AutoCompesation");/*AutoCompensation*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicAe->u8AEExposureCnt; u32IdxM++)
        {
            pstDynamicAe->au8AutoCompensation[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:AutoHistOffset");/*AutoHistOffset*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicAe->u8AEExposureCnt; u32IdxM++)
        {
            pstDynamicAe->au8AutoMaxHistOffset[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_ae:AntiflickerEn");/*AntiflickerEn*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicAe->u8AEExposureCnt; u32IdxM++)
        {
            pstDynamicAe->au8AntiFlickerEn[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicWdrExposure(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_WDREXPOSURE_S* pstDynamicWdrExposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_wdrexposure:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_wdrexposure:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_wdrexposure:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicWdrExposure->bUsed= (HI_BOOL)s32Value;
    }

    /*ExpCount*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_wdrexposure:ExpCount");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_wdrexposure:ExpCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_wdrexposure:ExpCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicWdrExposure->u8ExposureCnt = (HI_U8)s32Value;
    }

    /*ExpLtoHThresh*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_wdrexposure:ExpLtoHThresh");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicWdrExposure->u8ExposureCnt; u32IdxM++)
        {
            pstDynamicWdrExposure->au64ExpLtoHThresh[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*ExpRatioMax*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_wdrexposure:ExpRatioMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicWdrExposure->u8ExposureCnt; u32IdxM++)
        {
            pstDynamicWdrExposure->au32ExpRatioMax[u32IdxM] = (HI_U32)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /*ExpRatioMin*/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_wdrexposure:ExpRatioMin");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicWdrExposure->u8ExposureCnt; u32IdxM++)
        {
            pstDynamicWdrExposure->au32ExpRatioMin[u32IdxM] = (HI_U32)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicFsWdr(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_FSWDR_S *pstDynamicFsWdr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_fswdr:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_fswdr:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicFsWdr->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:ExpCount");/*ExpCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_fswdr:ExpCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_fswdr:ExpCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicFsWdr->u32ExposureCnt = (HI_U32)s32Value;
    }

    /****ISO****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:ISO");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_fswdr:ISO", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_fswdr:ISO failed\n");
            return HI_FAILURE;
        }
        pstDynamicFsWdr->u32ISO = (HI_U32)s32Value;
    }

    /****WDRMergeMode****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:WDRMergeMode");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicFsWdr->u32ExposureCnt; u32IdxM++)
        {
            pstDynamicFsWdr->au8WDRMergeMode[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicClut(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_CLUT_S* pstDynamicClut)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_clut:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_clut:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicClut->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:Used");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_clut:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_clut:Used failed\n");
            return HI_FAILURE;
        }
        pstDynamicClut->bUsed= (HI_BOOL)s32Value;
    }

    /****IsoCnt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:IsoCnt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_clut:IsoCnt", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_clut:IsoCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicClut->u32ISOCount = (HI_U32)s32Value;
    }

    /****IsoLevel****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:IsoLevel");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicClut->u32ISOCount; u32IdxM++)
        {
            pstDynamicClut->au32ISOLevel[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****GainR****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:GainR");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicClut->u32ISOCount; u32IdxM++)
        {
            pstDynamicClut->au32GainR[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****GainG****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:GainG");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicClut->u32ISOCount; u32IdxM++)
        {
            pstDynamicClut->au32GainG[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****GainB****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_clut:GainB");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicClut->u32ISOCount; u32IdxM++)
        {
            pstDynamicClut->au32GainB[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicNR(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_NR_S* pstDynamicNR)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_nr:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_nr:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_nr:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicNR->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_nr:ISOcount");/*IsoCnt*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_nr:ISOcount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_nr:ISOcount failed\n");
            return HI_FAILURE;
        }
        pstDynamicNR->u16ISOCount = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_nr:ISO");/*ISO*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicNR->u16ISOCount; u32IdxM++)
        {
            pstDynamicNR->au32ISO[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_nr:FrameStrShort");/*FrameStrShort*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicNR->u16ISOCount; u32IdxM++)
        {
            pstDynamicNR->au8FrameStrShort[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_nr:FrameStrLong");/*FrameStrLong*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicNR->u16ISOCount; u32IdxM++)
        {
            pstDynamicNR->au8FrameStrLong[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicShading(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_SHADING_S* pstDynamicShading)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_shading:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_shading:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicShading->bUsed= (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpCount");/*ExpCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_shading:ExpCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_shading:ExpCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicShading->u16ExpCount = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u16ExpCount; u32IdxM++)
        {
            pstDynamicShading->au32ExpThreshLtoH[u32IdxM] = (HI_U32)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:Strength");/*Strength*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u16ExpCount; u32IdxM++)
        {
            pstDynamicShading->au16MeshStr[u32IdxM] = (HI_U32)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:BlendRatio");/*BlendRatio*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u16ExpCount; u32IdxM++)
        {
            pstDynamicShading->au16BlendRatio[u32IdxM] = (HI_U32)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicDehaze(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_DEHAZE_S* pstDynamicDehaze)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_dehaze:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_dehaze:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicDehaze->bUsed= (HI_BOOL)s32Value;
    }

    /****ExpThreshCnt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ExpThreshCnt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_dehaze:ExpThreshCnt", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_dehaze:RationCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicDehaze->u32ExpThreshCnt = (HI_U32)s32Value;
    }

    /****ExpThreshLtoH****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ExpThreshLtoH");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDehaze->u32ExpThreshCnt; u32IdxM++)
        {
            pstDynamicDehaze->au64ExpThreshLtoH[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DehazeStr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ManualDehazeStr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDehaze->u32ExpThreshCnt; u32IdxM++)
        {
            pstDynamicDehaze->au8ManualStrength[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicDrc(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_DRC_S* pstDynamicDrc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0, u32IdxN = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_drc:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_drc:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicDrc->bUsed= (HI_BOOL)s32Value;
    }

    /****IsoCnt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:IsoCnt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_drc:IsoCnt", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_drc:IsoCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicDrc->u32ISOCount = (HI_U32)s32Value;
    }

    /****IsoLevel****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:IsoLevel");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au32ISOLevel[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Interval****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Interval");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_drc:Interval", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_drc:Interval failed\n");
            return HI_FAILURE;
        }
        pstDynamicDrc->u32Interval = (HI_U32)s32Value;
    }

    /****bEnable****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:bEnable");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_drc:bEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_drc:bEnable failed\n");
            return HI_FAILURE;
        }
        pstDynamicDrc->bEnable = (HI_BOOL)s32Value;
    }

    /****LocalMixingBrightMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:LocalMixingBrightMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8LocalMixingBrightMax[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****LocalMixingBrightMin****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:LocalMixingBrightMin");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8LocalMixingBrightMin[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****LocalMixingDarkMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:LocalMixingDarkMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8LocalMixingDarkMax[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****LocalMixingDarkMin****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:LocalMixingDarkMin");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8LocalMixingDarkMin[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****BrightGainLmt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:BrightGainLmt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8BrightGainLmt[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****BrightGainLmtStep****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:BrightGainLmtStep");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8BrightGainLmtStep[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DarkGainLmtY****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:DarkGainLmtY");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8DarkGainLmtY[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DarkGainLmtC****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:DarkGainLmtC");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8DarkGainLmtC[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DetailAdjustFactor****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:DetailAdjustFactor");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->as8DetailAdjustFactor[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****SpatialFltCoef****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:SpatialFltCoef");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8SpatialFltCoef[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****RangeFltCoef****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:RangeFltCoef");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8RangeFltCoef[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****GradRevMax****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:GradRevMax");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8GradRevMax[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****GradRevThr****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:GradRevThr");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8GradRevThr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Asymmetry****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Asymmetry");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8Asymmetry[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****SecondPole****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:SecondPole");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8SecondPole[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Compress****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Compress");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8Compress[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Stretch****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Stretch");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->au8Stretch[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Strength****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:Strength");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32ISOCount; u32IdxM++)
        {
            pstDynamicDrc->u16Strength[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****RatioCount****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:RatioCount");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_drc:RatioCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_drc:RatioCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicDrc->u32RatioCount = (HI_U32)s32Value;
    }

    /****RatioLevel****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:RatioLevel");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32RatioCount; u32IdxM++)
        {
            pstDynamicDrc->au32RatioLevel[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****DRCTMValue****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:DRCTMValue_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < pstDynamicDrc->u32RatioCount; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_drc:DRCTMValue_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < HI_ISP_DRC_TM_NODE_NUM; u32IdxN++)
                {
                    pstDynamicDrc->au16TMValue[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicGamma(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_GAMMA_S* pstDynamicGamma)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_gamma:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_gamma:Enable failed\n");
            return HI_FAILURE;
        }
        pstDynamicGamma->bUsed= (HI_BOOL)s32Value;
    }

    /****TotalNum****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:TotalNum");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_gamma:TotalNum", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_gamma:TotalNum failed\n");
            return HI_FAILURE;
        }
        pstDynamicGamma->u32TotalNum = (HI_U32)s32Value;
    }

    /****Interval****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Interval");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_gamma:Interval", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_gamma:Interval failed\n");
            return HI_FAILURE;
        }
        pstDynamicGamma->u32InterVal = (HI_U32)s32Value;
    }

    /****ExpThreshLtoH****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshLtoH");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicGamma->u32TotalNum; u32IdxM++)
        {
            pstDynamicGamma->au64ExpThreshLtoH[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****Table_0****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_0");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < pstDynamicGamma->u32TotalNum; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_%d", u32IdxM);
            s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("load %s failed\n", aszIniNodeName);
                return HI_FAILURE;
            }
            if (NULL != pszString)
            {
                PDT_SCENE_GetNumbersInOneLine(pszString);
                for (u32IdxN = 0; u32IdxN < GAMMA_NODE_NUM; u32IdxN++)
                {
                    pstDynamicGamma->au16Table[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicCAC(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_CAC_S* pstDynamicCAC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_cac:Used");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_cac:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_cac:Used failed\n");
            return HI_FAILURE;
        }
        pstDynamicCAC->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_cac:au32ISOCount");/*au32ISOCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_cac:au32ISOCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_cac:au32ISOCount failed\n");
            return HI_FAILURE;
        }
        pstDynamicCAC->au32ISOCount = s32Value;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_cac:au32ISOLevel");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicCAC->au32ISOCount; u32IdxM++)
        {
            pstDynamicCAC->au32ISOLevel[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_cac:PurpleDetRange");/*PurpleDetRange*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicCAC->au32ISOCount; u32IdxM++)
        {
            pstDynamicCAC->u16PurpleDetRange[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_cac:VarThr");/*VarThr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicCAC->au32ISOCount; u32IdxM++)
        {
            pstDynamicCAC->u16VarThr[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_LoadDynamicBlackLevel(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_BLACKLEVEL_S* pstDynamicBlackLevel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:Used");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_blacklevel:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_blacklevel:Used failed\n");
            return HI_FAILURE;
        }
        pstDynamicBlackLevel->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au32ISOCount");/*IsoCnt*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_blacklevel:au32ISOCount", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_blacklevel:IsoCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicBlackLevel->au32ISOCount = (HI_U32)s32Value;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au32ISOLevel");/*IsoLevel*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicBlackLevel->au32ISOCount; u32IdxM++)
        {
            pstDynamicBlackLevel->au32ISOLevel[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au16BlackLevelRR");/*au16BlackLevelRR*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicBlackLevel->au32ISOCount; u32IdxM++)
        {
            pstDynamicBlackLevel->au16BlackLevelRR[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au16BlackLevelGR");/*au16BlackLevelGR*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicBlackLevel->au32ISOCount; u32IdxM++)
        {
            pstDynamicBlackLevel->au16BlackLevelGR[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au16BlackLevelGB");/*au16BlackLevelGB*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicBlackLevel->au32ISOCount; u32IdxM++)
        {
            pstDynamicBlackLevel->au16BlackLevelGB[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_blacklevel:au16BlackLevelBB");/*au16BlackLevelBB*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicBlackLevel->au32ISOCount; u32IdxM++)
        {
            pstDynamicBlackLevel->au16BlackLevelBB[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadDynamicQVBRVENC(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_QVBRVENC_S* pstDynamicQVBRVENC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    /****Used****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:Used");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_QvbrVenc:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_QvbrVenc:Used failed\n");
            return HI_FAILURE;
        }
        pstDynamicQVBRVENC->bUsed = (HI_BOOL)s32Value;
    }

    /****ExpThreshCnt****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:ExpThreshCnt");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_QvbrVenc:ExpThreshCnt", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_QvbrVenc:ExpThreshCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicQVBRVENC->u16ExpCount= (HI_U32)s32Value;
    }

    /****ExpThreshLtoH****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:ExpThreshLtoH");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicQVBRVENC->u16ExpCount; u32IdxM++)
        {
            pstDynamicQVBRVENC->au64ExpThreshLtoH[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****MaxQp****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:MaxQp");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicQVBRVENC->u16ExpCount; u32IdxM++)
        {
            pstDynamicQVBRVENC->au32MaxQp[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****MinQp****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:MinQp");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicQVBRVENC->u16ExpCount; u32IdxM++)
        {
            pstDynamicQVBRVENC->au32MinQp[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    /****MinIQp****/
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_QvbrVenc:MinIQp");
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicQVBRVENC->u16ExpCount; u32IdxM++)
        {
            pstDynamicQVBRVENC->au32MinIQp[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadSceneParam(const HI_CHAR *pszIniModule, HI_PDT_SCENE_PIPE_PARAM_S* pstSceneParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_SCENE_LoadStaticStatistics(pszIniModule, &pstSceneParam->stStaticStatistics);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticStatistics failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticAE(pszIniModule, &pstSceneParam->stStaticAe);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAE failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticAeRoute(pszIniModule, &pstSceneParam->stStaticAeRoute);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAeRoute failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticSFAeRoute(pszIniModule, &pstSceneParam->stStaticSFAeRoute);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAeRoute failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticAeRouteEX(pszIniModule, &pstSceneParam->stStaticAeRouteEx);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAeRouteEX failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticWdrExposure(pszIniModule, &pstSceneParam->stStaticWdrExposure);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticWdrExposure failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticFsWdr(pszIniModule, &pstSceneParam->stStaticFsWdr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticFsWdr failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticAWB(pszIniModule, &pstSceneParam->stStaticAWB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAWB failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticSaturation(pszIniModule, &pstSceneParam->stStaticSaturation);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticSaturation failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticCCM(pszIniModule, &pstSceneParam->stStaticCCM);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticCCM failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticClut(pszIniModule, &pstSceneParam->stStaticClut);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticClut failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDrc(pszIniModule, &pstSceneParam->stStaticDrc);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDrc failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDehaze(pszIniModule, &pstSceneParam->stStaticDehaze);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDehaze failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticLDCI(pszIniModule, &pstSceneParam->stStaticLdci);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticLDCI failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDemosaic(pszIniModule, &pstSceneParam->stStaticDemosaic);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDemosaic failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticNr(pszIniModule, &pstSceneParam->stStaticNr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticNr failed\n");
        return HI_FAILURE;
    }
    s32Ret = PDT_SCENE_LoadStaticCAC(pszIniModule, &pstSceneParam->stStaticCAC);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticNr failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDPC(pszIniModule, &pstSceneParam->stStaticDPC);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDPC failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticShading(pszIniModule, &pstSceneParam->stStaticShading);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDE(pszIniModule, &pstSceneParam->stStaticDE);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDE failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticSharpen(pszIniModule, &pstSceneParam->stStaticSharpen);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticSharpen failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticCa(pszIniModule, &pstSceneParam->stStaticCa);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticCA failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticCrossTalk(pszIniModule, &pstSceneParam->stStaticCrossTalk);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticCrossTalk failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticQP(pszIniModule, &pstSceneParam->stStaticQP);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticQP failed\n");
        return HI_FAILURE;
    }

    extern HI_S32 PDT_SCENE_LoadStatic3DNR(const HI_CHAR *pszIniModule,    \
        HI_PDT_SCENE_STATIC_3DNR_S* pstStatic3Dnr);
    s32Ret = PDT_SCENE_LoadStatic3DNR(pszIniModule, &pstSceneParam->stStatic3DNR);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticThreeDNR failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicAE(pszIniModule, &pstSceneParam->stDynamicAe);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicAE failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicWdrExposure(pszIniModule, &pstSceneParam->stDynamicWdrExposure);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicWdrExposure failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicFsWdr(pszIniModule, &pstSceneParam->stDynamicFsWdr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicFsWdr failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicClut(pszIniModule, &pstSceneParam->stDynamicClut);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicClut failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicNR(pszIniModule, &pstSceneParam->stDynamicNR);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicNR failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicShading(pszIniModule, &pstSceneParam->stDynamicShading);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicCAC(pszIniModule, &pstSceneParam->stDynamicCAC);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicBlackLevel(pszIniModule, &pstSceneParam->stDynamicBlackLevel);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicBlackLevel failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicDehaze(pszIniModule, &pstSceneParam->stDynamicDehaze);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicDehaze failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicDrc(pszIniModule, &pstSceneParam->stDynamicDrc);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicDrc failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicGamma(pszIniModule, &pstSceneParam->stDynamicGamma);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicGamma failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicQVBRVENC(pszIniModule, &pstSceneParam->stDynamicQVBRVENC);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicQVBRVENC failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadSceneConf(HI_PDT_SCENE_PIPE_PARAM_S* pstScenePipeParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ModeIdx = 0;
    HI_CHAR aszIniModuleName[PDT_SCENE_INIPARAM_MODULE_NAME_LEN] = {0,};

    for (u32ModeIdx = 0; u32ModeIdx < PDT_SCENE_PIPETYPE_CNT; u32ModeIdx++)
    {
        snprintf(aszIniModuleName, PDT_SCENE_INIPARAM_MODULE_NAME_LEN, "%s%d", PDT_SCENE_INI_SCENEMODE, u32ModeIdx);

        s32Ret = PDT_SCENE_LoadSceneParam(aszIniModuleName, pstScenePipeParam + u32ModeIdx);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load scene[%d] config failed\n", u32ModeIdx);
            return HI_FAILURE;
        }
#if 0
        extern HI_VOID PDT_PARAM_DebugPipeParamALL(const HI_PDT_SCENE_PIPE_PARAM_S* pstScenePipeParam);
        PDT_PARAM_DebugPipeParamALL(pstScenePipeParam + u32ModeIdx);
#endif
    }

    return HI_SUCCESS;
}
#endif

HI_S32 PDT_SCENE_LoadExparam(const HI_CHAR *pszIniModule, PDT_SCENE_EXPARAM_S* pstSceneExparam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = HI_NULL;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_U32 u32IdxK = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "averagemetry:ExpWeight_0_0");/*AEWeight*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxK = 0; u32IdxK < PDT_SCENE_MAX_METRYAEWEIGHT_CNT; u32IdxK++)
        {
            for (u32IdxM = 0; u32IdxM < AE_ZONE_ROW; u32IdxM++)
            {
                snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "averagemetry:ExpWeight_%d_%d", u32IdxK, u32IdxM);
                s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("load %s failed\n", aszIniNodeName);
                    return HI_FAILURE;
                }
                if (NULL != pszString)
                {
                    PDT_SCENE_GetNumbersInOneLine(pszString);
                    for (u32IdxN = 0; u32IdxN < AE_ZONE_COLUMN; u32IdxN++)
                    {
                        pstSceneExparam->astAvergeMetryAeWeight[u32IdxK].au8AEWeight[u32IdxM][u32IdxN] = (HI_U8)s_as64LineNum[u32IdxN];
                    }
                    free(pszString);
                    pszString = HI_NULL;
                }
            }
        }
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "centermetry:ExpWeight_0_0");/*AEWeight*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxK = 0; u32IdxK < PDT_SCENE_MAX_METRYAEWEIGHT_CNT; u32IdxK++)
        {
            for (u32IdxM = 0; u32IdxM < AE_ZONE_ROW; u32IdxM++)
            {
                snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "centermetry:ExpWeight_%d_%d", u32IdxK, u32IdxM);
                s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("load %s failed\n", aszIniNodeName);
                    return HI_FAILURE;
                }
                if (NULL != pszString)
                {
                    PDT_SCENE_GetNumbersInOneLine(pszString);
                    for (u32IdxN = 0; u32IdxN < AE_ZONE_COLUMN; u32IdxN++)
                    {
                        pstSceneExparam->astCenterMetryAeWeight[u32IdxK].au8AEWeight[u32IdxM][u32IdxN] = (HI_U8)s_as64LineNum[u32IdxN];
                    }
                    free(pszString);
                    pszString = HI_NULL;
                }
            }
        }
    }
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_LoadSceneExparamConf(PDT_SCENE_EXPARAM_S* pstSceneExparam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszIniModuleName[PDT_SCENE_INIPARAM_MODULE_NAME_LEN] = {0,};

    snprintf(aszIniModuleName, PDT_SCENE_INIPARAM_MODULE_NAME_LEN, "%s", PDT_SCENE_INI_SCENEEXPARAM);

    s32Ret = PDT_SCENE_LoadExparam(aszIniModuleName, pstSceneExparam);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("load PDT_SCENE_LoadExparam config failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_LoadSceneParam(PDT_SCENE_PARAM_S* pstSceneParam)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneParam, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
#ifdef CONFIG_SCENEAUTO_SUPPORT
    s32Ret = PDT_SCENE_LoadSceneConf(pstSceneParam->astPipeParam);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SceneConf");
#endif
    s32Ret = PDT_SCENE_LoadSceneExparamConf(&pstSceneParam->stExParam);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SceneExParam");


    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

