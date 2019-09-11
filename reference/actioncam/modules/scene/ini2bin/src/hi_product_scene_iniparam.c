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
HI_S32 PDT_SCENE_LoadStaticAE(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AE_S* pstStaticAe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = HI_NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AERunInterval");/*AERunInterval*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AERouteExValid");/*AERouteExValid*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSysGainMax");/*AutoSysGainMax*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSpeed");/*AutoSpeed*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoISPDGainMax");/*AutoISPDGainMax*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoISPDGainMin");/*AutoISPDGainMin*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoDGainMax");/*AutoDGainMax*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoDGainMin");/*AutoDGainMin*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:AutoTolerance");/*AutoTolerance*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ae:ChangeFpsEnable");/**/
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

HI_S32 PDT_SCENE_LoadStaticAWB(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_AWB_S* pstStaticAwb)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoStaticWb");/*AutoStaticWb*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 4; u32IdxM++)
        {
            pstStaticAwb->au16AutoStaticWB[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCurvePara");/*AutoCurvePara*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < 6; u32IdxM++)
        {
            pstStaticAwb->as32AutoCurvePara[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoSpeed");/*AutoSpeed*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AutoSpeed", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AutoSpeed failed\n");
            return HI_FAILURE;
        }
        pstStaticAwb->u16AutoSpeed = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoLowColorTemp");/*AutoLowColorTemp*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AutoLowColorTemp", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AutoLowColorTemp failed\n");
            return HI_FAILURE;
        }
        pstStaticAwb->u16AutoLowColorTemp = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoHighColorTemp");/*AutoHighColorTemp*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AutoHighColorTemp", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AutoHighColorTemp failed\n");
            return HI_FAILURE;
        }
        pstStaticAwb->u16AutoHighColorTemp = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoRGStrength");/*AutoRGStrength*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AutoRGStrength", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AutoRGStrength failed\n");
            return HI_FAILURE;
        }
        pstStaticAwb->u8RGStrength = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoBGStrength");/*AutoBGStrength*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AutoBGStrength", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AutoBGStrength failed\n");
            return HI_FAILURE;
        }
        pstStaticAwb->u8BGStrength = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCrMax");/*AutoCrMax*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticAwb->au16AutoCrMax[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCrMin");/*AutoCrMin*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticAwb->au16AutoCrMin[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCbMax");/*AutoCbMax*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticAwb->au16AutoCbMax[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCbMin");/*AutoCbMin*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticAwb->au16AutoCbMin[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_awb:AlgType");/*AlgType*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_awb:AlgType", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_awb:AlgType failed\n");
            return HI_FAILURE;
        }

        pstStaticAwb->enAlgType = (ISP_AWB_ALG_E)s32Value;
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

HI_S32 PDT_SCENE_LoadStaticSaturation(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_SATURATION_S* pstStaticSaturation)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_saturation:AutoSat");/*AutoSat*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSaturation->au8AutoSat[u32IdxM] = s_as64LineNum[u32IdxM];
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ldci:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ldci:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ldci:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticLdci->bUsed = (HI_BOOL)s32Value;
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

HI_S32 PDT_SCENE_LoadStaticDemosaic(const HI_CHAR* pszIniModule, HI_PDT_SCENE_STATIC_DEMOSAIC_S* pstStaticDemosaic)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR* pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_Demosaic:Used", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_Demosaic:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDemosaic->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_Demosaic:Enable", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_Demosaic:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDemosaic->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:au8NonDirStr");/*au8NonDirStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8NonDirStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:au8NonDirMFDetailEhcStr");/*au8NonDirMFDetailEhcStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8NonDirMFDetailEhcStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:au8NonDirHFDetailEhcStr");/*au8NonDirHFDetailEhcStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8NonDirHFDetailEhcStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:au8DetailSmoothRange");/*FineStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au8DetailSmoothRange[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_Demosaic:au16DetailSmoothStr");/*CoringRatio*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDemosaic->au16DetailSmoothStr[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDPC(const HI_CHAR* pszIniModule, HI_PDT_SCENE_STATIC_DPC_S* pstStaticDPC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR* pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

	snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DPC:Used", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DPC:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDPC->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DPC:Enable", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DPC:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDPC->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:bSupTwinkleEn");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DPC:bSupTwinkleEn", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DPC:bSupTwinkleEn failed\n");
            return HI_FAILURE;
        }

        pstStaticDPC->bSupTwinkleEn = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:s8SoftThr");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DPC:s8SoftThr", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DPC:s8SoftThr failed\n");
            return HI_FAILURE;
        }

        pstStaticDPC->s8SoftThr = (HI_S8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:u8SoftSlope");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DPC:u8SoftSlope", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DPC:u8SoftSlope failed\n");
            return HI_FAILURE;
        }

        pstStaticDPC->u8SoftSlope = (HI_U8)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:au16Strength");/*au16Strength*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDPC->au16Strength[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DPC:au16BlendRatio");/*au16BlendRatio*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDPC->au16BlendRatio[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticDE(const HI_CHAR* pszIniModule, HI_PDT_SCENE_STATIC_DE_S* pstStaticDE)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR* pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

	snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DE:Used", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DE:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDE->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DE:Enable", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DE:Enable failed\n");
            return HI_FAILURE;
        }

        pstStaticDE->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:Optype");/*Optype*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_DE:Optype", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_DE:Optype failed\n");
            return HI_FAILURE;
        }

        pstStaticDE->u16Optype = (HI_U16)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:au16LumaGainLut");/*au16LumaGainLut*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < HI_ISP_DE_LUMA_GAIN_LUT_N; u32IdxM++)
        {
            pstStaticDE->au16LumaGainLut[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:au16GlobalGain");/*au16GlobalGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GlobalGain[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:au16GainLF");/*au16GainLF*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GainLF[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_DE:au16GainHF");/*au16GainHF*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);

        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticDE->au16GainHF[u32IdxM] = s_as64LineNum[u32IdxM];
        }

        free(pszString);
        pszString = HI_NULL;
    }

    return HI_SUCCESS;
}

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

HI_S32 PDT_SCENE_LoadStaticCCM(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CCM_S* pstStaticCCM)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:TotalNum");/*TotalNum*/
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
        pstStaticCCM->u32TotalNum = (HI_U32)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:AutoColorTemp");/*AutoColorTemp*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:AutoCCMTable_0");/*AutoCCMTable_0*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < pstStaticCCM->u32TotalNum; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ccm:AutoCCMTable_%d", u32IdxM);
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

HI_S32 PDT_SCENE_LoadStaticCSC(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CSC_S* pstStaticCSC)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_csc:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_csc:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_csc:Used failed\n");
            return HI_FAILURE;
        }
        pstStaticCSC->bUsed= s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_csc:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_csc:Enable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_csc:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticCSC->bEnable = s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_csc:ColorGamut");/*ColorGamut*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_csc:ColorGamut", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_csc:ColorGamut failed\n");
            return HI_FAILURE;
        }
        pstStaticCSC->enColorGamut = s32Value;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticNr(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_NR_S* pstStaticNr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_nr:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_nr:bUsed failed\n");
            return HI_FAILURE;
        }
        pstStaticNr->bUsed = (HI_BOOL)s32Value;
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:NrLscEnable");/*NrLscEnable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_nr:NrLscEnable", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_nr:NrLscEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticNr->bNrLscEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:FineStr");/*FineStr*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoringRatio");/*CoringRatio*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:ChromaStr_0");/*ChromaStr_0*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < ISP_BAYER_CHN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:ChromaStr_%d", u32IdxM);
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
                    pstStaticNr->au8ChromaStr[u32IdxM][u32IdxN] = (HI_U32)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoringWgt");/*CoringWgt*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoarseStr_0");/*CoarseStr_0*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        for (u32IdxM = 0; u32IdxM < ISP_BAYER_CHN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_nr:CoarseStr_%d", u32IdxM);
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
                    pstStaticNr->au16CoarseStr[u32IdxM][u32IdxN] = (HI_U16)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_sharpen:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticSharpen->bUsed = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:Enable");/*Enable*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureStr_0");/*AutoTextureStr*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeStr_0");/*AutoEdgeStr*/
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
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoLumaWgt_0");/*AutoLumaWgt*/
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
                    pstStaticSharpen->au8AutoLumaWgt[u32IdxM][u32IdxN] = (HI_U8)s_as64LineNum[u32IdxN];
                }
                free(pszString);
                pszString = HI_NULL;
            }
        }
    }
    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoOverShoot");/*AutoOverShoot*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoUnderShoot");/*AutoUnderShoot*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoShootSupStr");/*AutoShootSupStr*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoShootSupAdj");/*AutoShootSupAdj*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureFreq");/*AutoTextureFreq*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeFreq");/*AutoEdgeFreq*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoDetailCtrl");/*AutoDetailCtrl*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoDetailCtrlThr");/*AutoDetailCtrlThr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < ISP_AUTO_ISO_STRENGTH_NUM; u32IdxM++)
        {
            pstStaticSharpen->au8AutoDetailCtrlThr[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeFiltStr");/*AutoEdgeFiltStr*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoRGain");/*AutoRGain*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoBGain");/*AutoBGain*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoSkinGain");/*AutoSkinGain*/
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

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticPostSharpen(const HI_CHAR* pszIniModule, HI_PDT_SCENE_STATIC_POSTSHARPEN_S* pstStaticSharpen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR* pszString = NULL;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_postsharpen:Used", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_postsharpen:bEnable failed\n");
            return HI_FAILURE;
        }

        pstStaticSharpen->bUsed = (HI_BOOL)s32Value;
    }


    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_postsharpen:Enable", 0, &s32Value);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_postsharpen:bEnable failed\n");
            return HI_FAILURE;
        }

        pstStaticSharpen->bEnable = (HI_BOOL)s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoTextureStr_0");/*AutoTextureStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;

        for (u32IdxM = 0; u32IdxM < ISP_SHARPEN_GAIN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoTextureStr_%d", u32IdxM);
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoEdgeStr_0");/*AutoEdgeStr*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;

        for (u32IdxM = 0; u32IdxM < ISP_SHARPEN_GAIN_NUM; u32IdxM++)
        {
            snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoEdgeStr_%d", u32IdxM);
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoOverShoot");/*AutoOverShoot*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoUnderShoot");/*AutoUnderShoot*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoShootSupStr");/*AutoShootSupStr*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoTextureFreq");/*AutoTextureFreq*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoEdgeFreq");/*AutoEdgeFreq*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_postsharpen:AutoDetailCtrl");/*AutoDetailCtrl*/
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

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadStaticShading(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_SHADING_S* pstStaticShading)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_shading:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_shading:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:Enable failed\n");
            return HI_FAILURE;
        }
        pstStaticShading->bUsed = (HI_BOOL)s32Value;
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


HI_S32 PDT_SCENE_LoadStaticClut(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CLUT_S* pstStaticClut)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_clut:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_clut:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_shading:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticClut->bUsed = (HI_BOOL)s32Value;
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

HI_S32 PDT_SCENE_LoadStaticCa(const HI_CHAR *pszIniModule, HI_PDT_SCENE_STATIC_CA_S* pstStaticCa)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_ca:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_ca:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_ca:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticCa->bUsed = (HI_BOOL)s32Value;
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "static_crosstalk:Used");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "static_crosstalk:Used", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load static_crosstalk:bEnable failed\n");
            return HI_FAILURE;
        }
        pstStaticCrossTalk->bUsed = (HI_BOOL)s32Value;
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

HI_S32 PDT_SCENE_LoadDynamicGamma(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_GAMMA_S* pstDynamicGamma)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_U32 u32IdxN = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};


    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:TotalNum");/*TotalNum*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Interval");/*Interval*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshLtoH");/*ExpThreshLtoH*/
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

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_0");/*Table_0*/
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

HI_S32 PDT_SCENE_LoadDynamicShading(const HI_CHAR *pszIniModule, HI_PDT_SCENE_DYNAMIC_SHADING_S* pstDynamicShading)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32IdxM = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};


    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpThreshCnt");/*ExpThreshCnt*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, "dynamic_shading:ExpThreshCnt", 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load dynamic_shading:ExpThreshCnt failed\n");
            return HI_FAILURE;
        }
        pstDynamicShading->u32ExpThreshCnt = s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u32ExpThreshCnt; u32IdxM++)
        {
            pstDynamicShading->au64ExpThreshLtoH[u32IdxM] = s_as64LineNum[u32IdxM];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ManualStrength");/*ManualStrength*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u32ExpThreshCnt; u32IdxM++)
        {
            pstDynamicShading->au16ManualStrength[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
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
        for (u32IdxM = 0; u32IdxM < pstDynamicShading->u32ExpThreshCnt; u32IdxM++)
        {
            pstDynamicShading->au16BlendRatio[u32IdxM] = (HI_U16)s_as64LineNum[u32IdxM];
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

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_LoadSceneParam(const HI_CHAR *pszIniModule, HI_PDT_SCENE_PIPE_PARAM_S* pstSceneParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_SCENE_LoadStaticAE(pszIniModule, &pstSceneParam->stStaticAe);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAE failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticAWB(pszIniModule, &pstSceneParam->stStaticAwb);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAWB failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticClut(pszIniModule, &pstSceneParam->stStaticClut);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticClut failed\n");
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

    s32Ret = PDT_SCENE_LoadStaticAeRouteEX(pszIniModule, &pstSceneParam->stStaticAeRouteEx);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticAeRouteEX failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticSharpen(pszIniModule, &pstSceneParam->stStaticSharpen);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticSharpen failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticPostSharpen(pszIniModule, &pstSceneParam->stStaticPostSharpen);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticPostSharpen failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticSaturation(pszIniModule, &pstSceneParam->stStaticSaturation);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticSaturation failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticCCM(pszIniModule, &pstSceneParam->stStaticCcm);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticCCM failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticStatistics(pszIniModule, &pstSceneParam->stStaticStatistics);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticStatistics failed\n");
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

    s32Ret = PDT_SCENE_LoadStaticCSC(pszIniModule, &pstSceneParam->stStaticCsc);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticCSC failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticLDCI(pszIniModule, &pstSceneParam->stStaticLdci);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticLDCI failed\n");
        return HI_FAILURE;
    }
    s32Ret = PDT_SCENE_LoadStaticNr(pszIniModule, &pstSceneParam->stStaticNr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticNr failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDemosaic(pszIniModule, &pstSceneParam->stStaticDemosaic);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDemosaic failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDPC(pszIniModule, &pstSceneParam->stStaticDPC);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDPC failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticDE(pszIniModule, &pstSceneParam->stStaticDE);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticDE failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadStaticShading(pszIniModule, &pstSceneParam->stStaticShading);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadStaticShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicGamma(pszIniModule, &pstSceneParam->stDynamicGamma);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicGamma failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicShading(pszIniModule, &pstSceneParam->stDynamicShading);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_SCENE_LoadDynamicAE(pszIniModule, &pstSceneParam->stDynamicAe);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_SCENE_LoadDynamicAE failed\n");
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

HI_S32 PDT_SCENE_LoadSnapParam(const HI_CHAR *pszIniModule, HI_PDT_SCENE_SNAP_AEROUTE_PARAM_S* pstSnapAeRouteParam, PDT_SCENE_SPECIALPARAM_S* pstSpecialParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = HI_NULL;
    HI_S32 s32Value = 0;
    HI_U32 u32ModeIdx = 0;
    HI_CHAR aszIniNodeName[PDT_SCENE_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "normalroute:NodeCount");/*NodeCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load normalroute:NodeCount failed\n");
            return HI_FAILURE;
        }
        pstSnapAeRouteParam->stNormalRoute.u32Count = s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "normalroute:SysGain");/*SysGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stNormalRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stNormalRoute.astLongExpAERoute[u32ModeIdx].u32SysGain = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "normalroute:IntTime");/*IntTime*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stNormalRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stNormalRoute.astLongExpAERoute[u32ModeIdx].u32ExpTime = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "postprocessroute:NodeCount");/*NodeCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load postprocessroute:NodeCount failed\n");
            return HI_FAILURE;
        }
        pstSnapAeRouteParam->stPostProcessRoute.u32Count = s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "postprocessroute:SysGain");/*SysGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stPostProcessRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stPostProcessRoute.astLongExpAERoute[u32ModeIdx].u32SysGain = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "postprocessroute:IntTime");/*IntTime*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stPostProcessRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stPostProcessRoute.astLongExpAERoute[u32ModeIdx].u32ExpTime = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "nightroute:NodeCount");/*NodeCount*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load nightroute:NodeCount failed\n");
            return HI_FAILURE;
        }
        pstSnapAeRouteParam->stNightRoute.u32Count = s32Value;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "nightroute:SysGain");/*SysGain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stNightRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stNightRoute.astLongExpAERoute[u32ModeIdx].u32SysGain = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "nightroute:IntTime");/*IntTime*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        PDT_SCENE_GetNumbersInOneLine(pszString);
        for (u32ModeIdx = 0; u32ModeIdx < pstSnapAeRouteParam->stNightRoute.u32Count; u32ModeIdx++)
        {
            pstSnapAeRouteParam->stNightRoute.astLongExpAERoute[u32ModeIdx].u32ExpTime = s_as64LineNum[u32ModeIdx];
        }
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, PDT_SCENE_INIPARAM_NODE_NAME_LEN, "specialparam:MinimumSysgain");/*MinimumSysgain*/
    s32Ret = HI_CONFACCESS_GetString(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        free(pszString);
        pszString = HI_NULL;
        s32Ret = HI_CONFACCESS_GetInt(PDT_SCENE_INIPARAM, pszIniModule, aszIniNodeName, 0, &s32Value);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("load specialparam:MinimumSysgain failed\n");
            return HI_FAILURE;
        }
        pstSpecialParam->u32MinimumSysgain = s32Value;
    }
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_LoadSceneSnapConf(HI_PDT_SCENE_SNAP_AEROUTE_PARAM_S* pstSnapAeRouteParam, PDT_SCENE_SPECIALPARAM_S* pstSpecialParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszIniModuleName[PDT_SCENE_INIPARAM_MODULE_NAME_LEN] = {0,};

    snprintf(aszIniModuleName, PDT_SCENE_INIPARAM_MODULE_NAME_LEN, "%s", PDT_SCENE_INI_SCENESNAP);

    s32Ret = PDT_SCENE_LoadSnapParam(aszIniModuleName, pstSnapAeRouteParam, pstSpecialParam);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("load PDT_SCENE_LoadSnapParam config failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

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
    s32Ret = PDT_SCENE_LoadSceneSnapConf(&pstSceneParam->stSnapAeRouteParam, &pstSceneParam->stSpecialParam);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SceneSnap");

    s32Ret = PDT_SCENE_LoadSceneExparamConf(&pstSceneParam->stExParam);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SceneExParam");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

