/**
 * @file      hi_product_param_debug.c
 * @brief     param debug interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/15
 * @version   1.0

 */

#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_scene_setparam.h"

/** Param Debug Pointer Check */
#define PDT_SCENEDEBUG_CHECK_POINTER(p) \
    do { if (!(p)) { \
            MLOGE("null pointer\n");\
            return; \
        } }while(0)


HI_VOID PDT_PARAM_DebugStaticAE(const HI_PDT_SCENE_STATIC_AE_S* pstStaticAe)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticAe);
    printf(RED"StaticAE Configure:\n"NONE);

    printf("AERunInterval is %d.\n",pstStaticAe->u8AERunInterval);
    printf("MaxTime is %d.\n",pstStaticAe->u32MaxTime);
    printf("AERouteExValid is %d.\n",pstStaticAe->bAERouteExValid);
    printf("AutoSysGainMax is %d.\n",pstStaticAe->u32AutoSysGainMax);
    printf("AutoSpeed is %d.\n",pstStaticAe->u8AutoSpeed);
    printf("AutoISPDGainMax is %d.\n",pstStaticAe->u32AutoISPDGainMax);
    printf("AutoISPDGainMin is %d.\n",pstStaticAe->u32AutoISPDGainMin);
    printf("AutoDGainMax is %d.\n",pstStaticAe->u32AutoDGainMax);
    printf("AutoDGainMin is %d.\n",pstStaticAe->u32AutoDGainMin);
    printf("AutoTolerance is %d.\n",pstStaticAe->u8AutoTolerance);
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticAERoute(const HI_PDT_SCENE_STATIC_AEROUTE_S* pstStaticAeRoute)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticAeRoute);
    printf(RED"StaticAERoute Configure:\n"NONE);

    HI_S32 i = 0;
    printf("TotalNum is %d.\n",pstStaticAeRoute->u32TotalNum);
    printf("RouteEXIntTime is: ");
    for (i = 0; i < pstStaticAeRoute->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRoute->au32IntTime[i]);
    }
    printf("\n");
    printf("RouteEXAGain is: ");
    for (i = 0; i < pstStaticAeRoute->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRoute->au32SysGain[i]);
    }

    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticAWB(const HI_PDT_SCENE_STATIC_AWB_S* pstStaticAWB)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticAWB);
    printf(RED"StaticAWB Configure:\n"NONE);

    HI_S32 i = 0;
    printf("u16Speed is %d.\n",pstStaticAWB->u16Speed);
    printf("u16LowColorTemp is %d.\n",pstStaticAWB->u16LowColorTemp);
    printf("u8Tolerance is %d.\n",pstStaticAWB->u8Tolerance);
    printf("u8ShiftLimit is %d.\n",pstStaticAWB->u8ShiftLimit);
    printf("u16HighStart is %d.\n",pstStaticAWB->u16HighStart);
    printf("u16HighStop is %d.\n",pstStaticAWB->u16HighStop);
    printf("u16LowStart is %d.\n",pstStaticAWB->u16LowStart);
    printf("u16LowStop is %d.\n",pstStaticAWB->u16LowStop);
    printf("u32OutThresh is %d.\n",pstStaticAWB->u32OutThresh);


    printf("au16StaticWB is: ");
    for (i = 0; i < 4; i++)
    {
        printf("%d ",pstStaticAWB->au16StaticWB[i]);
    }
    printf("\n");
    printf("as32CurvePara is: ");
    for (i = 0; i < 6; i++)
    {
        printf("%d ",pstStaticAWB->as32CurvePara[i]);
    }

    printf("\n");
}




HI_VOID PDT_PARAM_DebugStaticAeRouteEX(const HI_PDT_SCENE_STATIC_AEROUTEEX_S* pstStaticAeRouteEx)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticAeRouteEx);

    printf(RED"StaticAeRouteEx Configure:\n"NONE);

    HI_S32 i = 0;
    printf("TotalNum is %d.\n",pstStaticAeRouteEx->u32TotalNum);

    printf("RouteEXIntTime is: ");
    for (i = 0; i < pstStaticAeRouteEx->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRouteEx->au32IntTime[i]);
    }
    printf("\n");
    printf("RouteEXAGain is: ");
    for (i = 0; i < pstStaticAeRouteEx->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRouteEx->au32Again[i]);
    }
    printf("\n");
    printf("RouteEXDGain is: ");
    for (i = 0; i < pstStaticAeRouteEx->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRouteEx->au32Dgain[i]);
    }
    printf("\n");
    printf("RouteEXISPDGain is: ");
    for (i = 0; i < pstStaticAeRouteEx->u32TotalNum; i++)
    {
        printf("%d ",pstStaticAeRouteEx->au32IspDgain[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticLDCI(const HI_PDT_SCENE_STATIC_LDCI_S* pstStaticLdci)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticLdci);
    printf(RED"StaticStaticLDCI Configure:\n"NONE);


    HI_S32 i = 0;
    printf("Enable is %d.\n",pstStaticLdci->bEnable);
    printf("LDCIOpType is %d.\n",pstStaticLdci->u8LDCIOpType);
    printf("LDCIGaussLPFSigma is %d.\n",pstStaticLdci->u8GaussLPFSigma);
    printf("AutoHePosWgt is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHePosWgt[i]);
    }
    printf("\n");
    printf("AutoHePosWgt is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHePosSigma[i]);
    }
    printf("\n");
    printf("AutoHePosMean is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHePosMean[i]);
    }
    printf("\n");
    printf("AutoHeNegWgt is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHeNegWgt[i]);
    }
    printf("\n");
    printf("AutoHeNegSigma is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHeNegSigma[i]);
    }
    printf("\n");
    printf("AutoHeNegMean is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au8AutoHeNegMean[i]);
    }
    printf("\n");
    printf("AutoBlcCtrl is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticLdci->au16AutoBlcCtrl[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticStatistics(const HI_PDT_SCENE_STATIC_STATISTICSCFG_S* pstStaticStatistics)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticStatistics);
    HI_S32 i, j = 0;
    printf(RED"StaticStatistics Configure:\n"NONE);

    printf("AFEnable is %d.\n",pstStaticStatistics->bAFEnable);
    printf("ExpWeight is: \n");
    for (i = 0; i < AE_ZONE_ROW; i++)
    {
        for (j = 0; j < AE_ZONE_COLUMN; j++)
        {
            printf("%3d ",pstStaticStatistics->au8AEWeight[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticNr(const HI_PDT_SCENE_STATIC_NR_S* pstStaticNr)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticNr);

    printf(RED"StaticNr Configure:\n"NONE);

    printf("Enable is %d.\n",pstStaticNr->bEnable);
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticClut(const HI_PDT_SCENE_STATIC_CLUT_S* pstStaticClut)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticClut);
    printf(RED"StaticClut Configure:\n"NONE);

    printf("Enable is %d.\n",pstStaticClut->bEnable);
}

HI_VOID PDT_PARAM_DebugStaticCa(const HI_PDT_SCENE_STATIC_CA_S* pstStaticCa)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticCa);
    printf(RED"StaticCa Configure:\n"NONE);

    printf("Enable is %d.\n",pstStaticCa->bEnable);
}

HI_VOID PDT_PARAM_DebugStaticCrossTalk(const HI_PDT_SCENE_STATIC_CROSSTALK_S* pstStaticCrossTalk)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticCrossTalk);
    printf(RED"StaticCrossTalk Configure:\n"NONE);

    printf("Enable is %d.\n",pstStaticCrossTalk->bEnable);
}

HI_VOID PDT_PARAM_DebugStaticSharpen(const HI_PDT_SCENE_STATIC_SHARPEN_S* pstStaticSharpen)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticSharpen);

    printf(RED"StaticSharpen Configure:\n"NONE);

    HI_S32 i, j = 0;
    printf("Enable is %d.\n",pstStaticSharpen->bEnable);

    printf("au8LumaWgt is: \n");
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            printf("%d ",pstStaticSharpen->au8LumaWgt[i][j]);
        }
        printf("\n");
    }
    printf("\n");


    printf("AutoTextureStr is: \n");
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            printf("%d ",pstStaticSharpen->au16AutoTextureStr[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("AutoEdgeStr is: \n");
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            printf("%d ",pstStaticSharpen->au16AutoEdgeStr[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("AutoOverShoot is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8AutoOverShoot[i]);
    }
    printf("\n");
    printf("AutoUnderShoot is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8AutoUnderShoot[i]);
    }
    printf("\n");
    printf("AutoShootSupStr is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8ShootSupStr[i]);
    }
    printf("\n");

    printf("AutoTextureFreq is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au16AutoTextureFreq[i]);
    }
    printf("\n");

    printf("AutoEdgeFreq is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au16AutoEdgeFreq[i]);
    }
    printf("\n");

    printf("AutoDetailCtrl is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8AutoDetailCtrl[i]);
    }
    printf("\n");

    printf("AutoEdgeFiltStr is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8EdgeFiltStr[i]);
    }
    printf("\n");

    printf("AutoRGain is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8RGain[i]);
    }
    printf("\n");

    printf("AutoBGain is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8BGain[i]);
    }
    printf("\n");

    printf("AutoSkinGain is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSharpen->au8SkinGain[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticDrc(const HI_PDT_SCENE_STATIC_DRC_S* pstStaticDrc)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticDrc);
    HI_S32 i = 0;
    printf(RED"StaticDrc Configure:\n"NONE);

    printf("bEnable is %d.\n",pstStaticDrc->bEnable);
    printf("u8CurveSelect is %d.\n",pstStaticDrc->u8CurveSelect);
    printf("u8DRCOpType is %d.\n",pstStaticDrc->u8DRCOpType);
    printf("au16ToneMappingValue is: ");
    for (i = 0; i < HI_ISP_DRC_TM_NODE_NUM; i++)
    {
        printf("%d ",pstStaticDrc->au16ToneMappingValue[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticWDRExposure(const HI_PDT_SCENE_STATIC_WDREXPOSURE_S* pstStaticWdrExposure)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticWdrExposure);
    HI_S32 i = 0;
    printf(RED"pstStaticWdrExposure Configure:\n"NONE);

    printf("u8ExpRatioType is %d.\n",pstStaticWdrExposure->u8ExpRatioType);
    printf("u16Tolerance is %d.\n",pstStaticWdrExposure->u16Tolerance);
    printf("u16Speed is %d.\n",pstStaticWdrExposure->u16Speed);
    printf("u32ExpRatioMax is %d.\n",pstStaticWdrExposure->u32ExpRatioMax);
    printf("u32ExpRatioMin is %d.\n",pstStaticWdrExposure->u32ExpRatioMin);
    printf("au16ToneMappingValue is: ");
    for (i = 0; i < EXP_RATIO_NUM; i++)
    {
        printf("%d ",pstStaticWdrExposure->au32ExpRatio[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticFsWdr (const HI_PDT_SCENE_STATIC_FSWDR_S* pstStaticFsWdr)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticFsWdr);
    printf(RED"pstStaticFsWdr Configure:\n"NONE);

    printf("WDRMergeMode is %d.\n",pstStaticFsWdr->WDRMergeMode);
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticDehaze(const HI_PDT_SCENE_STATIC_DEHAZE_S* pstStaticDehaze)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticDehaze);
    HI_S32 i = 0;
    printf(RED"pstStaticDehaze Configure:\n"NONE);

    printf("bEnable is %d.\n",pstStaticDehaze->bEnable);
    printf("u8DehazeOpType is %d.\n",pstStaticDehaze->u8DehazeOpType);
    printf("bUserLutEnable is %d.\n",pstStaticDehaze->bUserLutEnable);
    printf("au8DehazeLut is: ");
    for (i = 0; i < 256; i++)
    {
        printf("%d ",pstStaticDehaze->au8DehazeLut[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugDynamicFswdr(const HI_PDT_SCENE_DYNAMIC_FSWDR_S* pstDynamicFsWdr)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicFsWdr);
    HI_S32 i = 0;
    printf(RED"pstDynamicFsWdr Configure:\n"NONE);

    printf("u32ExposureCnt is %d.\n",pstDynamicFsWdr->u32ExposureCnt);
    printf("u32ISO is: ");
    printf("%d ",pstDynamicFsWdr->u32ISO);
    printf("\n");
    printf("au8WDRMergeMode is: ");
    for (i = 0; i < pstDynamicFsWdr->u32ExposureCnt; i++)
    {
        printf("%d ",pstDynamicFsWdr->au8WDRMergeMode[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugDynamicDehaze(const HI_PDT_SCENE_DYNAMIC_DEHAZE_S* pstDynamicDehaze)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicDehaze);
    HI_S32 i = 0;
    printf(RED"pstDynamicDehaze Configure:\n"NONE);

    printf("u32ExpThreshCnt is %d.\n",pstDynamicDehaze->u32ExpThreshCnt);
    printf("au64ExpThreshLtoH is: ");
    for (i = 0; i < pstDynamicDehaze->u32ExpThreshCnt; i++)
    {
        printf("%lld ",pstDynamicDehaze->au64ExpThreshLtoH[i]);
    }
    printf("\n");
    printf("au8ManualStrength is: ");
    for (i = 0; i < pstDynamicDehaze->u32ExpThreshCnt; i++)
    {
        printf("%d ",pstDynamicDehaze->au8ManualStrength[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugDynamicGamma(const HI_PDT_SCENE_DYNAMIC_GAMMA_S* pstDynamicGamma)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicGamma);

    printf(RED"DynamicGamma Configure:\n"NONE);
    HI_S32 i, j, k = 0;
    printf("TotalNum is %d.\n",pstDynamicGamma->u32TotalNum);

    printf("Interval is %d.\n",pstDynamicGamma->u32InterVal);

    printf("gammaExpThreshLtoH is: ");
    for (i = 0; i < pstDynamicGamma->u32TotalNum; i++)
    {
        printf("%lld ",pstDynamicGamma->au64ExpThreshLtoH[i]);
    }
    printf("\n");

    for (i = 0; i < pstDynamicGamma->u32TotalNum; i++)
    {
        printf("Table_%d is: \n", i);
        for (j = 0; j < 32; j++)
        {
            for (k = 0; k < 32; k++)
            {
                printf("%d ",pstDynamicGamma->au16Table[i][32 * j + k]);
            }
            printf("\n");
        }
        printf(" %d\n", pstDynamicGamma->au16Table[i][GAMMA_NODE_NUM - 1]);
    }
}

HI_VOID PDT_PARAM_DebugDynamicAE(const HI_PDT_SCENE_DYNAMIC_AE_S* pstDynamicAe)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicAe);
    HI_S32 i = 0;
    printf(RED"DynamicAE Configure:\n"NONE);

    printf("aeExpCount is %d.\n",pstDynamicAe->u8AEExposureCnt);

    printf("aeExpLtoHThresh is: ");
    for (i = 0; i < pstDynamicAe->u8AEExposureCnt; i++)
    {
        printf("%lld ",pstDynamicAe->au64ExpLtoHThresh[i]);
    }
    printf("\n");

    printf("AutoCompesation is: ");
    for (i = 0; i < pstDynamicAe->u8AEExposureCnt; i++)
    {
        printf("%d ",pstDynamicAe->au8AutoCompensation[i]);
    }
    printf("\n");

    printf("AutoHistOffset is: ");
    for (i = 0; i < pstDynamicAe->u8AEExposureCnt; i++)
    {
        printf("%d ",pstDynamicAe->au8AutoMaxHistOffset[i]);
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugDynamicDrc(const HI_PDT_SCENE_DYNAMIC_DRC_S* pstDynamicDrc)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicDrc);
    HI_S32 i = 0;
    printf(RED"pstDynamicDrc Configure:\n"NONE);

    printf("bEnable is %d.\n",pstDynamicDrc->bEnable);
    printf("u32Interval is %d.\n",pstDynamicDrc->u32Interval);
    printf("u32ISOCount is %d.\n",pstDynamicDrc->u32ISOCount);
    printf("au32ISOLevel is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au32ISOLevel[i]);
    }
    printf("\n");
    printf("au8LocalMixingBrightMax is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8LocalMixingBrightMax[i]);
    }
    printf("\n");
    printf("au8LocalMixingBrightMin is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8LocalMixingBrightMin[i]);
    }
    printf("\n");
    printf("au8LocalMixingDarkMax is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8LocalMixingDarkMax[i]);
    }
    printf("\n");
    printf("au8LocalMixingDarkMin is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8LocalMixingDarkMin[i]);
    }
    printf("\n");
    printf("au8DarkGainLmtY is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8DarkGainLmtY[i]);
    }
    printf("\n");
    printf("au8DarkGainLmtC is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8DarkGainLmtC[i]);
    }
    printf("\n");
    printf("u16Strength is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->u16Strength[i]);
    }
    printf("\n");
    printf("as8DetailAdjustFactor is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->as8DetailAdjustFactor[i]);
    }
    printf("\n");
    printf("au8SpatialFltCoef is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8SpatialFltCoef[i]);
    }
    printf("\n");
    printf("au8RangeFltCoef is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8RangeFltCoef[i]);
    }
    printf("\n");
    printf("au8GradRevMax is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8GradRevMax[i]);
    }
    printf("\n");
    printf("au8GradRevThr is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8GradRevThr[i]);
    }
    printf("\n");
    printf("au8Asymmetry is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8Asymmetry[i]);
    }
    printf("\n");
    printf("au8SecondPole is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8SecondPole[i]);
    }
    printf("\n");
    printf("au8Compress is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8Compress[i]);
    }
    printf("\n");
    printf("au8Stretch is: ");
    for (i = 0; i < pstDynamicDrc->u32ISOCount; i++)
    {
        printf("%d ",pstDynamicDrc->au8Stretch[i]);
    }
    printf("\n");
}


HI_VOID PDT_PARAM_DebugPipeParamALL(const HI_PDT_SCENE_PIPE_PARAM_S* pstScenePipeParam)
{
    PDT_PARAM_DebugStaticAE(&(pstScenePipeParam->stStaticAe));
    PDT_PARAM_DebugStaticClut(&(pstScenePipeParam->stStaticClut));
    PDT_PARAM_DebugStaticCa(&(pstScenePipeParam->stStaticCa));
    PDT_PARAM_DebugStaticCrossTalk(&(pstScenePipeParam->stStaticCrossTalk));
    PDT_PARAM_DebugStaticAeRouteEX(&(pstScenePipeParam->stStaticAeRouteEx));
    PDT_PARAM_DebugStaticStatistics(&(pstScenePipeParam->stStaticStatistics));
    extern HI_VOID PDT_PARAM_DebugStatic3DNR(const HI_PDT_SCENE_STATIC_3DNR_S* pstStatic3Dnr);
    PDT_PARAM_DebugStatic3DNR(&(pstScenePipeParam->stStatic3DNR));
    PDT_PARAM_DebugStaticLDCI(&(pstScenePipeParam->stStaticLdci));
    PDT_PARAM_DebugStaticNr(&(pstScenePipeParam->stStaticNr));
    PDT_PARAM_DebugStaticSharpen(&(pstScenePipeParam->stStaticSharpen));
    PDT_PARAM_DebugStaticAERoute(&(pstScenePipeParam->stStaticAeRoute));
    PDT_PARAM_DebugStaticAWB(&(pstScenePipeParam->stStaticAWB));
    PDT_PARAM_DebugStaticDrc(&(pstScenePipeParam->stStaticDrc));
    PDT_PARAM_DebugStaticWDRExposure(&(pstScenePipeParam->stStaticWdrExposure));
    PDT_PARAM_DebugStaticFsWdr(&(pstScenePipeParam->stStaticFsWdr));
    PDT_PARAM_DebugStaticDehaze(&(pstScenePipeParam->stStaticDehaze));
    PDT_PARAM_DebugDynamicGamma(&(pstScenePipeParam->stDynamicGamma));
    PDT_PARAM_DebugDynamicAE(&(pstScenePipeParam->stDynamicAe));
    PDT_PARAM_DebugDynamicFswdr(&(pstScenePipeParam->stDynamicFsWdr));
    PDT_PARAM_DebugDynamicDehaze(&(pstScenePipeParam->stDynamicDehaze));
    PDT_PARAM_DebugDynamicDrc(&(pstScenePipeParam->stDynamicDrc));
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
