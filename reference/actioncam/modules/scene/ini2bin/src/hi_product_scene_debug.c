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
#include "hi_confaccess.h"
#include "hi_product_scene_iniparam.h"

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


HI_VOID PDT_PARAM_DebugStaticAWB(const HI_PDT_SCENE_STATIC_AWB_S* pstStaticAwb)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticAwb);
    printf(RED"StaticAwb Configure:\n"NONE);

    HI_S32 i = 0;
    printf("AutoStaticWb is: ");
    for (i = 0; i < 4; i++)
    {
        printf("%d ",pstStaticAwb->au16AutoStaticWB[i]);
    }
    printf("\n");

    printf("AutoCurvePara is: ");
    for (i = 0; i < 6; i++)
    {
        printf("%d ",pstStaticAwb->as32AutoCurvePara[i]);
    }
    printf("\n");

    printf("AutoSpeed is %d.\n",pstStaticAwb->u16AutoSpeed);
    printf("AutoLowColorTemp is %d.\n",pstStaticAwb->u16AutoLowColorTemp);
    printf("AutoHighColorTemp is %d.\n",pstStaticAwb->u16AutoHighColorTemp);
    printf("AutoRGStrength is %d.\n",pstStaticAwb->u8RGStrength);
    printf("AutoBGStrength is %d.\n",pstStaticAwb->u8BGStrength);
    printf("AutoCrMax is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticAwb->au16AutoCrMax[i]);
    }
    printf("\n");
    printf("AutoCrMin is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticAwb->au16AutoCrMin[i]);
    }
    printf("\n");
    printf("AutoCbMax is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticAwb->au16AutoCbMax[i]);
    }
    printf("\n");
    printf("AutoCbMin is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticAwb->au16AutoCbMin[i]);
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

HI_VOID PDT_PARAM_DebugStaticSaturation(const HI_PDT_SCENE_STATIC_SATURATION_S* pstStaticSaturation)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticSaturation);
    printf(RED"StaticStaticSaturation Configure:\n"NONE);
    HI_S32 i = 0;
    printf("AutoSat is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticSaturation->au8AutoSat[i]);
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

HI_VOID PDT_PARAM_DebugStaticCCM(const HI_PDT_SCENE_STATIC_CCM_S* pstStaticCCM)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticCCM);

    printf(RED"StaticCCM Configure:\n"NONE);
    HI_S32 i ,j = 0;
    printf("TotalNum is %d.\n",pstStaticCCM->u32TotalNum);
    printf("AutoColorTemp is: ");
    for (i = 0; i < pstStaticCCM->u32TotalNum; i++)
    {
        printf("%d ",pstStaticCCM->au16AutoColorTemp[i]);
    }
    printf("\n");

    printf("AutoCCMTable is: \n");
    for (i = 0; i < pstStaticCCM->u32TotalNum; i++)
    {
        for (j = 0; j < CCM_MATRIX_SIZE; j++)
        {
            printf("%d ",pstStaticCCM->au16AutoCCM[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticCSC(const HI_PDT_SCENE_STATIC_CSC_S* pstStaticCSC)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticCSC);
    printf(RED"StaticCSC Configure:\n"NONE);
    printf("Enable is %d.\n",pstStaticCSC->bEnable);
    printf("ColorGamut is %d.\n",pstStaticCSC->enColorGamut);
}

HI_VOID PDT_PARAM_DebugStaticNr(const HI_PDT_SCENE_STATIC_NR_S* pstStaticNr)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticNr);

    printf(RED"StaticNr Configure:\n"NONE);

    HI_S32 i, j = 0;
    printf("Enable is %d.\n",pstStaticNr->bEnable);

    printf("FineStr is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticNr->au8FineStr[i]);
    }
    printf("\n");

    printf("CoringRatio is: ");
    for (i = 0; i < HI_ISP_BAYERNR_LUT_LENGTH; i++)
    {
        printf("%d ",pstStaticNr->au16CoringRatio[i]);
    }
    printf("\n");
    printf("ChromaStr is: ");
    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            printf("%d ",pstStaticNr->au8ChromaStr[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("CoringWgt is: ");
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        printf("%d ",pstStaticNr->au16CoringWgt[i]);
    }
    printf("\n");
    printf("CoarseStr is: \n");
    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            printf("%d ",pstStaticNr->au16CoarseStr[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

HI_VOID PDT_PARAM_DebugStaticSharpen(const HI_PDT_SCENE_STATIC_SHARPEN_S* pstStaticSharpen)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticSharpen);

    printf(RED"StaticSharpen Configure:\n"NONE);

    HI_S32 i, j = 0;
    printf("Enable is %d.\n",pstStaticSharpen->bEnable);


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

HI_VOID PDT_PARAM_DebugStaticShading(const HI_PDT_SCENE_STATIC_SHADING_S* pstStaticShading)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstStaticShading);
    printf(RED"StaticShading Configure:\n"NONE);

    printf("Enable is %d.\n",pstStaticShading->bEnable);
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

HI_VOID PDT_PARAM_DebugDynamicShading(const HI_PDT_SCENE_DYNAMIC_SHADING_S* pstDynamicShading)
{
    PDT_SCENEDEBUG_CHECK_POINTER(pstDynamicShading);
    printf(RED"DynamicShading Configure:\n"NONE);

    HI_S32 i = 0;
    printf("ExpThreshCnt is %d.\n",pstDynamicShading->u32ExpThreshCnt);

    printf("ExpThreshLtoH is: ");
    for (i = 0; i < pstDynamicShading->u32ExpThreshCnt; i++)
    {
        printf("%lld ",pstDynamicShading->au64ExpThreshLtoH[i]);
    }
    printf("\n");

    printf("ManualStrength is: ");
    for (i = 0; i < pstDynamicShading->u32ExpThreshCnt; i++)
    {
        printf("%d ",pstDynamicShading->au16ManualStrength[i]);
    }
    printf("\n");
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

HI_VOID PDT_PARAM_DebugPipeParamALL(const HI_PDT_SCENE_PIPE_PARAM_S* pstScenePipeParam)
{
    PDT_PARAM_DebugStaticAE(&(pstScenePipeParam->stStaticAe));
    PDT_PARAM_DebugStaticAWB(&(pstScenePipeParam->stStaticAwb));
    PDT_PARAM_DebugStaticClut(&(pstScenePipeParam->stStaticClut));
    PDT_PARAM_DebugStaticCa(&(pstScenePipeParam->stStaticCa));
    PDT_PARAM_DebugStaticCrossTalk(&(pstScenePipeParam->stStaticCrossTalk));
    PDT_PARAM_DebugStaticAeRouteEX(&(pstScenePipeParam->stStaticAeRouteEx));
    PDT_PARAM_DebugStaticSharpen(&(pstScenePipeParam->stStaticSharpen));
    PDT_PARAM_DebugStaticSaturation(&(pstScenePipeParam->stStaticSaturation));
    PDT_PARAM_DebugStaticCCM(&(pstScenePipeParam->stStaticCcm));
    PDT_PARAM_DebugStaticStatistics(&(pstScenePipeParam->stStaticStatistics));
    extern HI_VOID PDT_PARAM_DebugStatic3DNR(const HI_PDT_SCENE_STATIC_3DNR_S* pstStatic3Dnr);
    PDT_PARAM_DebugStatic3DNR(&(pstScenePipeParam->stStatic3DNR));
    PDT_PARAM_DebugStaticCSC(&(pstScenePipeParam->stStaticCsc));
    PDT_PARAM_DebugStaticLDCI(&(pstScenePipeParam->stStaticLdci));
    PDT_PARAM_DebugStaticShading(&(pstScenePipeParam->stStaticShading));
    PDT_PARAM_DebugStaticNr(&(pstScenePipeParam->stStaticNr));
    PDT_PARAM_DebugDynamicGamma(&(pstScenePipeParam->stDynamicGamma));
    PDT_PARAM_DebugDynamicShading(&(pstScenePipeParam->stDynamicShading));
    PDT_PARAM_DebugDynamicAE(&(pstScenePipeParam->stDynamicAe));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
