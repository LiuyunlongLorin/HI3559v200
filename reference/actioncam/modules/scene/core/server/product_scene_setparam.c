#include "hi_product_scene_setparam.h"
#include "product_scene_setparam_inner.h"

#include "hi_product_scene.h"
#if defined(AMP_LINUX_HUAWEILITE)
#include "hi_product_scene_msg_define.h"
#include "hi_appcomm_msg.h"
#endif

#include <unistd.h>
#include <string.h>
#ifdef CONFIG_SCENEAUTO_SUPPORT
#include "mpi_awb.h"
#include "mpi_ae.h"
#include "mpi_isp.h"
#include "mpi_vi.h"
#include "mpi_vpss.h"
#include "hi_comm_vpss.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PDT_SCENE_MAX(a, b) (((a) < (b)) ? (b) : (a))
#define PDT_SCENE_DIV_0TO1(a)  ((0 == (a)) ? 1 : (a))
HI_BOOL g_bISPPause;

static HI_PDT_SCENE_PIPE_PARAM_S* s_pstScenePipeParam = HI_NULL;

#define OPENPRINT 0

#define PDT_CHECK_SCENE_PAUSE()\
    do{\
        if (HI_TRUE == g_bISPPause)\
        {\
            return HI_SUCCESS;\
        }\
    }while(0);\

#define PDT_CHECK_SCENE_RET(s32Ret)\
    do{\
        if (HI_SUCCESS != s32Ret)\
        {\
            MLOGE("Failed at %s: LINE: %d with %#x!.\n", __FUNCTION__, __LINE__, s32Ret);\
        }\
    }while(0);\

#define PDT_CHECK_SCENE_NULLPTR(ptr)\
    do{\
        if (NULL == ptr)\
        {\
            MLOGE("NullPtr at %s: LINE: %d!.\n", __FUNCTION__, __LINE__);\
            return HI_FAILURE;\
        }\
    }while(0);\

#define PDT_FREE_SCENE_PTR(ptr)\
    do{\
        if (NULL != ptr)\
        {\
            free(ptr);\
            ptr = NULL;\
        }\
    }while(0);\

#define PDT_CHECK_SCENE_Index(index)\
            do{\
                if (index >= PDT_SCENE_PIPETYPE_CNT || index < 0)\
                {\
                    MLOGE("index at %s: LINE: %d! error.\n", __FUNCTION__, __LINE__);\
                    return HI_FAILURE;\
                }\
            }while(0);\


#ifdef CONFIG_SCENEAUTO_SUPPORT
static  HI_U32 PDT_SCENE_GetLevelLtoH(HI_U64 u64Value, HI_U32 u32Count, HI_U64 *pu64Thresh)
{
    HI_U32 u32Level = 0;

    if (u32Count == 0)
    {
         MLOGE("error input.\n");
         return u32Level;
    }

    for (u32Level = 0; u32Level < u32Count; u32Level++)
    {
        if(u64Value <= pu64Thresh[u32Level])
        {
            break;
        }
    }

    if (u32Level == u32Count)
    {
        u32Level = u32Count - 1;
    }

    return u32Level;
}

static HI_U32 PDT_SCENE_Interpulate(HI_U32 u32Mid,HI_U32 u32Left, HI_U32 u32LValue, HI_U32 u32Right, HI_U32 u32RValue)
{
    HI_U32 u32Value = 0;
    HI_U32 k = 0;

    if (u32Mid <= u32Left)
    {
        u32Value = u32LValue;
        return u32Value;
    }

    if (u32Mid >= u32Right)
    {
        u32Value = u32RValue;
        return u32Value;
    }

    k = (u32Right - u32Left);
    u32Value = (((u32Right - u32Mid) * u32LValue + (u32Mid - u32Left) * u32RValue + (k >> 1))/ k);
    return u32Value;
}

static HI_U32 PDT_SCENE_TimeFilter(HI_U32 u32Para0,HI_U32 u32Para1, HI_U32 u32TimeCnt, HI_U32 u32Index)
{
    HI_U64 u64Temp = 0;
    HI_U32 u32Value = 0;

    if (u32Para0 > u32Para1)
    {
        u64Temp = (HI_U64)(u32Para0 - u32Para1) << 8;
        u64Temp = (u64Temp * (u32Index + 1)) /PDT_SCENE_DIV_0TO1(u32TimeCnt) >> 8;
        u32Value = u32Para0 - (HI_U32)u64Temp;
    }

    else
    {
        u64Temp = (HI_U64)(u32Para1 - u32Para0) << 8;
        u64Temp = (u64Temp * (u32Index + 1)) /PDT_SCENE_DIV_0TO1(u32TimeCnt) >> 8;
        u32Value = u32Para0 + (HI_U32)u64Temp;
    }

    return u32Value;
}
#endif

HI_S32 PDT_SCENE_SetStaticAE(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_EXPOSURE_ATTR_S stExposureAttr;

    s32Ret = HI_MPI_ISP_GetExposureAttr(ViPipe, &stExposureAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stExposureAttr.bAERouteExValid = s_pstScenePipeParam[u8Index].stStaticAe.bAERouteExValid;
    stExposureAttr.u8AERunInterval = s_pstScenePipeParam[u8Index].stStaticAe.u8AERunInterval;
    stExposureAttr.stAuto.stISPDGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoISPDGainMax;
    stExposureAttr.stAuto.stISPDGainRange.u32Min = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoISPDGainMin;
    stExposureAttr.stAuto.stDGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoDGainMax;
    stExposureAttr.stAuto.stDGainRange.u32Min = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoDGainMin;
    stExposureAttr.stAuto.stSysGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoSysGainMax;
    stExposureAttr.stAuto.u8Speed = s_pstScenePipeParam[u8Index].stStaticAe.u8AutoSpeed;
    stExposureAttr.stAuto.u8Tolerance = s_pstScenePipeParam[u8Index].stStaticAe.u8AutoTolerance;

    s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticAWB(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_WB_ATTR_S stWbAttr;

    s32Ret = HI_MPI_ISP_GetWBAttr(ViPipe, &stWbAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    for (i = 0; i < 4; i++)
    {
        stWbAttr.stAuto.au16StaticWB[i] = s_pstScenePipeParam[u8Index].stStaticAwb.au16AutoStaticWB[i];
    }
    for (i = 0; i < 6; i++)
    {
        stWbAttr.stAuto.as32CurvePara[i] = s_pstScenePipeParam[u8Index].stStaticAwb.as32AutoCurvePara[i];
    }
    stWbAttr.stAuto.u16Speed = s_pstScenePipeParam[u8Index].stStaticAwb.u16AutoSpeed;
    stWbAttr.stAuto.u16HighColorTemp = s_pstScenePipeParam[u8Index].stStaticAwb.u16AutoHighColorTemp;
    stWbAttr.stAuto.u16LowColorTemp = s_pstScenePipeParam[u8Index].stStaticAwb.u16AutoLowColorTemp;
    stWbAttr.stAuto.u8RGStrength = s_pstScenePipeParam[u8Index].stStaticAwb.u8RGStrength;
    stWbAttr.stAuto.u8BGStrength = s_pstScenePipeParam[u8Index].stStaticAwb.u8BGStrength;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stWbAttr.stAuto.stCbCrTrack.au16CrMax[i] = s_pstScenePipeParam[u8Index].stStaticAwb.au16AutoCrMax[i];
        stWbAttr.stAuto.stCbCrTrack.au16CrMin[i] = s_pstScenePipeParam[u8Index].stStaticAwb.au16AutoCrMin[i];
        stWbAttr.stAuto.stCbCrTrack.au16CbMax[i] = s_pstScenePipeParam[u8Index].stStaticAwb.au16AutoCbMax[i];
        stWbAttr.stAuto.stCbCrTrack.au16CbMin[i] = s_pstScenePipeParam[u8Index].stStaticAwb.au16AutoCbMin[i];
    }

    stWbAttr.enAlgType = s_pstScenePipeParam[u8Index].stStaticAwb.enAlgType;

    s32Ret = HI_MPI_ISP_SetWBAttr(ViPipe, &stWbAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticAERouteEX(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_AE_ROUTE_EX_S stAeRouteEx;

    s32Ret = HI_MPI_ISP_GetAERouteAttrEx(ViPipe, &stAeRouteEx);
    PDT_CHECK_SCENE_RET(s32Ret);

    stAeRouteEx.u32TotalNum = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.u32TotalNum;
    for (i = 0; i < stAeRouteEx.u32TotalNum; i++)
    {
        stAeRouteEx.astRouteExNode[i].u32IntTime = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32IntTime[i];
        stAeRouteEx.astRouteExNode[i].u32Again= s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32Again[i];
        stAeRouteEx.astRouteExNode[i].u32Dgain= s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32Dgain[i];
        stAeRouteEx.astRouteExNode[i].u32IspDgain= s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32IspDgain[i];
    }

    s32Ret = HI_MPI_ISP_SetAERouteAttrEx(ViPipe, &stAeRouteEx);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticCrossTalk(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_CR_ATTR_S stCRAttr;
    if (!s_pstScenePipeParam[u8Index].stStaticCrossTalk.bUsed)
    {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_ISP_GetCrosstalkAttr(ViPipe, &stCRAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stCRAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticCrossTalk.bEnable;

    s32Ret = HI_MPI_ISP_SetCrosstalkAttr(ViPipe, &stCRAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticCA(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_CA_ATTR_S stCAAttr;
    if (!s_pstScenePipeParam[u8Index].stStaticCa.bUsed)
    {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_ISP_GetCAAttr(ViPipe, &stCAAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stCAAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticCa.bEnable;

    s32Ret = HI_MPI_ISP_SetCAAttr(ViPipe, &stCAAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticLDCI(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_LDCI_ATTR_S stLDCIAttr;
    HI_S32 i = 0;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticLdci.bUsed)
    {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_ISP_GetLDCIAttr(ViPipe,&stLDCIAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stLDCIAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticLdci.bEnable;
    stLDCIAttr.enOpType = (ISP_OP_TYPE_E)s_pstScenePipeParam[u8Index].stStaticLdci.u8LDCIOpType;
    stLDCIAttr.u8GaussLPFSigma = s_pstScenePipeParam[u8Index].stStaticLdci.u8GaussLPFSigma;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stLDCIAttr.stAuto.au16BlcCtrl[i] = s_pstScenePipeParam[u8Index].stStaticLdci.au16AutoBlcCtrl[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Sigma = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosSigma[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Wgt = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosWgt[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Mean = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosMean[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Sigma = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegSigma[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Wgt = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegWgt[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Mean = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegMean[i];
    }

    s32Ret = HI_MPI_ISP_SetLDCIAttr(ViPipe,&stLDCIAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticClut(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticClut.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_CLUT_ATTR_S stClutAttr;

    s32Ret = HI_MPI_ISP_GetClutAttr(ViPipe, &stClutAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stClutAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticClut.bEnable;
    stClutAttr.u32GainR = s_pstScenePipeParam[u8Index].stStaticClut.u32GainR;
    stClutAttr.u32GainG = s_pstScenePipeParam[u8Index].stStaticClut.u32GainG;
    stClutAttr.u32GainB = s_pstScenePipeParam[u8Index].stStaticClut.u32GainB;

    s32Ret = HI_MPI_ISP_SetClutAttr(ViPipe, &stClutAttr);

    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticStatisticsCfg(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_STATISTICS_CFG_S stStatisticsCfg;

    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stStatisticsCfg);
    PDT_CHECK_SCENE_RET(s32Ret);

    if (HI_FALSE == bMetryFixed)
    {
        for (i = 0; i < AE_ZONE_ROW; i++)
        {
            for (j = 0; j < AE_ZONE_COLUMN; j++)
            {
                stStatisticsCfg.stAECfg.au8Weight[i][j] = s_pstScenePipeParam[u8Index].stStaticStatistics.au8AEWeight[i][j];
            }
        }
    }
    stStatisticsCfg.stFocusCfg.stConfig.bEnable = s_pstScenePipeParam[u8Index].stStaticStatistics.bAFEnable;

    s32Ret = HI_MPI_ISP_SetStatisticsConfig(ViPipe, &stStatisticsCfg);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticSaturation(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_SATURATION_ATTR_S stSaturationAttr;

    s32Ret = HI_MPI_ISP_GetSaturationAttr(ViPipe, &stSaturationAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stSaturationAttr.stAuto.au8Sat[i] = s_pstScenePipeParam[u8Index].stStaticSaturation.au8AutoSat[i];
    }

    s32Ret = HI_MPI_ISP_SetSaturationAttr(ViPipe, &stSaturationAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticCCM(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 j = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_COLORMATRIX_ATTR_S stColormatrixAttr;

    s32Ret = HI_MPI_ISP_GetCCMAttr(ViPipe, &stColormatrixAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stColormatrixAttr.stAuto.u16CCMTabNum = s_pstScenePipeParam[u8Index].stStaticCcm.u32TotalNum;
    for (i = 0; i < s_pstScenePipeParam[u8Index].stStaticCcm.u32TotalNum; i++)
    {
        stColormatrixAttr.stAuto.astCCMTab[i].u16ColorTemp = s_pstScenePipeParam[u8Index].stStaticCcm.au16AutoColorTemp[i];
        for (j = 0; j < CCM_MATRIX_SIZE; j++)
        {
            stColormatrixAttr.stAuto.astCCMTab[i].au16CCM[j] = s_pstScenePipeParam[u8Index].stStaticCcm.au16AutoCCM[i][j];
        }
    }

    s32Ret = HI_MPI_ISP_SetCCMAttr(ViPipe, &stColormatrixAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticCSC(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticCsc.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_CSC_ATTR_S stCscAttr;

    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stCscAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stCscAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticCsc.bEnable;
    stCscAttr.enColorGamut = s_pstScenePipeParam[u8Index].stStaticCsc.enColorGamut;

    s32Ret = HI_MPI_ISP_SetCSCAttr(ViPipe, &stCscAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticSharpen(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticSharpen.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_SHARPEN_ATTR_S stSharpenAttr;

    s32Ret = HI_MPI_ISP_GetIspSharpenAttr(ViPipe, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stSharpenAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticSharpen.bEnable;
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            stSharpenAttr.stAuto.au16TextureStr[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureStr[i][j];
            stSharpenAttr.stAuto.au16EdgeStr[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoEdgeStr[i][j];
            stSharpenAttr.stAuto.au8LumaWgt[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoLumaWgt[i][j];
        }
    }

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stSharpenAttr.stAuto.au8OverShoot[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoOverShoot[i];
        stSharpenAttr.stAuto.au8UnderShoot[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoUnderShoot[i];
        stSharpenAttr.stAuto.au8ShootSupStr[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8ShootSupStr[i];
        stSharpenAttr.stAuto.au8ShootSupAdj[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8ShootSupAdj[i];
        stSharpenAttr.stAuto.au16TextureFreq[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureFreq[i];
        stSharpenAttr.stAuto.au16EdgeFreq[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoEdgeFreq[i];
        stSharpenAttr.stAuto.au8DetailCtrl[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoDetailCtrl[i];
        stSharpenAttr.stAuto.au8DetailCtrlThr[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoDetailCtrlThr[i];
        stSharpenAttr.stAuto.au8EdgeFiltStr[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8EdgeFiltStr[i];
        stSharpenAttr.stAuto.au8RGain[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8RGain[i];
        stSharpenAttr.stAuto.au8BGain[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8BGain[i];
        stSharpenAttr.stAuto.au8SkinGain[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8SkinGain[i];
    }

    s32Ret = HI_MPI_ISP_SetIspSharpenAttr(ViPipe, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticPostSharpen(VPSS_GRP VpssGrp, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticPostSharpen.bUsed)
    {
        return HI_SUCCESS;
    }
    VPSS_GRP_SHARPEN_ATTR_S stSharpenAttr;

    s32Ret = HI_MPI_VPSS_GetGrpSharpen(VpssGrp, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stSharpenAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticPostSharpen.bEnable;

    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            stSharpenAttr.stSharpenAutoAttr.au16TextureStr[i][j] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au16AutoTextureStr[i][j];
            stSharpenAttr.stSharpenAutoAttr.au16EdgeStr[i][j] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au16AutoEdgeStr[i][j];
        }
    }

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stSharpenAttr.stSharpenAutoAttr.au8OverShoot[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au8AutoOverShoot[i];
        stSharpenAttr.stSharpenAutoAttr.au8UnderShoot[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au8AutoUnderShoot[i];
        stSharpenAttr.stSharpenAutoAttr.au8ShootSupStr[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au8ShootSupStr[i];
        stSharpenAttr.stSharpenAutoAttr.au16TextureFreq[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au16AutoTextureFreq[i];
        stSharpenAttr.stSharpenAutoAttr.au16EdgeFreq[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au16AutoEdgeFreq[i];
        stSharpenAttr.stSharpenAutoAttr.au8DetailCtrl[i] = s_pstScenePipeParam[u8Index].stStaticPostSharpen.au8AutoDetailCtrl[i];
    }

    s32Ret = HI_MPI_VPSS_SetGrpSharpen(VpssGrp, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticNr(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j = 0;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticNr.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_NR_ATTR_S stNrAttr;

    s32Ret = HI_MPI_ISP_GetNRAttr(ViPipe, &stNrAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stNrAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticNr.bEnable;
    stNrAttr.bNrLscEnable = s_pstScenePipeParam[u8Index].stStaticNr.bNrLscEnable;
    stNrAttr.enOpType = OP_TYPE_AUTO;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
         stNrAttr.stAuto.au8FineStr[i] = s_pstScenePipeParam[u8Index].stStaticNr.au8FineStr[i];
         stNrAttr.stAuto.au16CoringWgt[i] = s_pstScenePipeParam[u8Index].stStaticNr.au16CoringWgt[i];
    }
    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)
        {
            stNrAttr.stAuto.au8ChromaStr[i][j] = s_pstScenePipeParam[u8Index].stStaticNr.au8ChromaStr[i][j];
            stNrAttr.stAuto.au16CoarseStr[i][j] = s_pstScenePipeParam[u8Index].stStaticNr.au16CoarseStr[i][j];
        }
    }

    for (i = 0; i < HI_ISP_BAYERNR_LUT_LENGTH; i++)
    {
         stNrAttr.au16CoringRatio[i] = s_pstScenePipeParam[u8Index].stStaticNr.au16CoringRatio[i];
    }

    s32Ret = HI_MPI_ISP_SetNRAttr(ViPipe, &stNrAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDemosaic(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticDemosaic.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_DEMOSAIC_ATTR_S stDemosaicAttr;

    s32Ret = HI_MPI_ISP_GetDemosaicAttr(ViPipe, &stDemosaicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stDemosaicAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDemosaic.bEnable;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stDemosaicAttr.stAuto.au8NonDirStr[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au8NonDirStr[i];
        stDemosaicAttr.stAuto.au8NonDirMFDetailEhcStr[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au8NonDirMFDetailEhcStr[i];
        stDemosaicAttr.stAuto.au8NonDirHFDetailEhcStr[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au8NonDirHFDetailEhcStr[i];
        stDemosaicAttr.stAuto.au8DetailSmoothRange[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au8DetailSmoothRange[i];
        stDemosaicAttr.stAuto.au16DetailSmoothStr[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au16DetailSmoothStr[i];
    }

    s32Ret = HI_MPI_ISP_SetDemosaicAttr(ViPipe, &stDemosaicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDPC(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    PDT_CHECK_SCENE_Index(u8Index);

    if (!s_pstScenePipeParam[u8Index].stStaticDPC.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_DP_DYNAMIC_ATTR_S stDPCAttr;

    s32Ret = HI_MPI_ISP_GetDPDynamicAttr(ViPipe, &stDPCAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stDPCAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDPC.bEnable;
    stDPCAttr.bSupTwinkleEn = s_pstScenePipeParam[u8Index].stStaticDPC.bSupTwinkleEn;
    stDPCAttr.s8SoftThr = s_pstScenePipeParam[u8Index].stStaticDPC.s8SoftThr;
    stDPCAttr.u8SoftSlope = s_pstScenePipeParam[u8Index].stStaticDPC.u8SoftSlope;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stDPCAttr.stAuto.au16Strength[i] = s_pstScenePipeParam[u8Index].stStaticDPC.au16Strength[i];
        stDPCAttr.stAuto.au16BlendRatio[i] = s_pstScenePipeParam[u8Index].stStaticDPC.au16BlendRatio[i];
    }

    s32Ret = HI_MPI_ISP_SetDPDynamicAttr(ViPipe, &stDPCAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDE(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    if (!s_pstScenePipeParam[u8Index].stStaticDE.bUsed)
    {
        return HI_SUCCESS;
    }
    ISP_DE_ATTR_S stDEAttr;

    s32Ret = HI_MPI_ISP_GetDEAttr(ViPipe, &stDEAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stDEAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDE.bEnable;
    stDEAttr.enOpType = s_pstScenePipeParam[u8Index].stStaticDE.u16Optype;

    for (i = 0; i < HI_ISP_DE_LUMA_GAIN_LUT_N; i++)
    {
        stDEAttr.au16LumaGainLut[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16LumaGainLut[i];
    }

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        stDEAttr.stAuto.au16GlobalGain[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GlobalGain[i];
        stDEAttr.stAuto.au16GainLF[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GainLF[i];
        stDEAttr.stAuto.au16GainHF[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GainHF[i];
    }

    s32Ret = HI_MPI_ISP_SetDEAttr(ViPipe, &stDEAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_SetStaticShading(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_SHADING_ATTR_S stShadingAttr;

    PDT_CHECK_SCENE_Index(u8Index);

    s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe, &stShadingAttr);
    PDT_CHECK_SCENE_RET(s32Ret);

    stShadingAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticShading.bEnable;

    s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe, &stShadingAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicGamma(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 i, j = 0;
    HI_U32 u32ExpLevel = 0;
    static HI_U32 u32LastExpLevel;
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_GAMMA_ATTR_S stIspGammaAttr;

    PDT_CHECK_SCENE_Index(u8Index);

    if (s_pstScenePipeParam[u8Index].stDynamicGamma.u32InterVal < 1 ||
        s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum < 1 ||
        s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum > HI_PDT_SCENE_GAMMA_EXPOSURE_MAX_COUNT)
    {
        return HI_FAILURE;
    }

    if (u64Exposure != u64LastExposure)
    {
        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum,
                                           s_pstScenePipeParam[u8Index].stDynamicGamma.au64ExpThreshLtoH);
        if(u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum)
        {
            return HI_FAILURE;
        }
        for (i = 0; i < s_pstScenePipeParam[u8Index].stDynamicGamma.u32InterVal; i++)
        {
            PDT_CHECK_SCENE_PAUSE();
            s32Ret = HI_MPI_ISP_GetGammaAttr(ViPipe, &stIspGammaAttr);
            PDT_CHECK_SCENE_RET(s32Ret);

            for (j = 0; j < GAMMA_NODE_NUM; j++)
            {
                    stIspGammaAttr.u16Table[j] = PDT_SCENE_TimeFilter(s_pstScenePipeParam[u8Index].stDynamicGamma.au16Table[u32LastExpLevel][j],
                                                             s_pstScenePipeParam[u8Index].stDynamicGamma.au16Table[u32ExpLevel][j],
                                                             s_pstScenePipeParam[u8Index].stDynamicGamma.u32InterVal, i);
            }
            stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
            PDT_CHECK_SCENE_PAUSE();
            s32Ret = HI_MPI_ISP_SetGammaAttr(ViPipe, &stIspGammaAttr);
            PDT_CHECK_SCENE_RET(s32Ret);
            HI_usleep(30000);
        }
        u32LastExpLevel = u32ExpLevel;

    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicAE(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32ExpLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_CHECK_SCENE_Index(u8Index);

    if (s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt < 1)
    {
        return HI_FAILURE;
    }

    if( s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt < 1 ||
        s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt > HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT)
    {
        return HI_FAILURE;
    }

    if (u64Exposure != u64LastExposure)
    {
        ISP_EXPOSURE_ATTR_S stExposureAttr;
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetExposureAttr(ViPipe, &stExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);

        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt,
                                                    s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh);
        if(u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt)
        {
            return HI_FAILURE;
        }
        if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt - 1)
        {
            stExposureAttr.stAuto.u8Compensation = s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel];
        }
        else
        {
            stExposureAttr.stAuto.u8Compensation = PDT_SCENE_Interpulate(u64Exposure,
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel - 1],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel - 1],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel]);

        }
        if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt - 1)
        {
            stExposureAttr.stAuto.u8MaxHistOffset = s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel];
        }
        else
        {
            stExposureAttr.stAuto.u8MaxHistOffset = PDT_SCENE_Interpulate(u64Exposure,
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel - 1],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel - 1],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],
                                                               s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel]);

        }

        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }

    if (s_pstScenePipeParam[u8Index].stStaticAe.u8ChangeFpsEnable)
    {
        static HI_U8 u8Stat = HI_TRUE;
        static HI_U8 u8fpschg_flag = HI_FALSE;
        static HI_U32 u8Stat_count = 0;


        if (HI_TRUE == u8Stat)
        {
            if (10 == u8Stat_count)
            {
                ISP_EXP_INFO_S stIspExpInfo;
                ISP_PUB_ATTR_S stPubAttr;

                s32Ret = HI_MPI_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);

                s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubAttr);

                if ((stIspExpInfo.u32ISO >= 850) && (((HI_U32)stPubAttr.f32FrameRate) > 100) && (HI_FALSE == u8fpschg_flag))
                {
                    stPubAttr.f32FrameRate = 60.0;
                    u8fpschg_flag = HI_TRUE;
                    HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);

                    MLOGI(" ### f32FrameRate change from 120->60!###\n");
                    s32Ret = HI_MSG_SendASync(MSG_PDT_SCENE_FRAMERATE_FROM120TO60, HI_NULL, 0, NULL);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_FRAMERATE_FROM120TO60 Failed, %x\n\n, "NONE, s32Ret);
                        return s32Ret;
                    }
                }
                else if ((stIspExpInfo.u32ISO <= 550) && (HI_TRUE == u8fpschg_flag))
                {
                    stPubAttr.f32FrameRate = 120.0;
                    u8fpschg_flag = HI_FALSE;
                    HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);

                    MLOGI(" ### f32FrameRate change from 60->120!###\n");
                    s32Ret = HI_MSG_SendASync(MSG_PDT_SCENE_FRAMERATE_FROM60TO120, HI_NULL, 0, NULL);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_FRAMERATE_FROM60TO120 Failed, %x\n\n, "NONE, s32Ret);
                        return s32Ret;
                    }
                }

                u8Stat_count = 0;
            }
            u8Stat_count++;
        }
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicShading(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32ExpLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64ExpCnt = 0;

    PDT_CHECK_SCENE_Index(u8Index);

    ISP_SHADING_ATTR_S stShadingAttr;

    if( s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt < 1 ||
        s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt > HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT)
    {
        return HI_FAILURE;
    }

    if (u64Exposure != u64LastExposure)
    {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe,&stShadingAttr);
        PDT_CHECK_SCENE_RET(s32Ret);

        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt,
                                          s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH);
        if(u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt)
        {
            return HI_FAILURE;
        }

        u64ExpCnt = s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt;

        //if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt - 1)
        if ((u64Exposure >= s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u64ExpCnt-1]) || (u64Exposure <= s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[0]))
        {
                stShadingAttr.u16MeshStr = s_pstScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel];
                stShadingAttr.u16BlendRatio = s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel];
        }
        else
        {
               stShadingAttr.u16MeshStr = PDT_SCENE_Interpulate(u64Exposure,
                         s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel - 1],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel - 1],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel]);
               stShadingAttr.u16BlendRatio = PDT_SCENE_Interpulate(u64Exposure,
                         s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel - 1],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel - 1],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel],
                         s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel]);
        }
            PDT_CHECK_SCENE_PAUSE();
            s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe,&stShadingAttr);
            PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetPipeParam(HI_PDT_SCENE_PIPE_PARAM_S* pstScenePipeParam, HI_U32 u32Num)
{
    if (HI_NULL == pstScenePipeParam)
    {
        MLOGE("null pointer.\n");
        return HI_FAILURE;
    }

    if (u32Num > PDT_SCENE_PIPETYPE_CNT)
    {
        MLOGE("error input.\n");
        return HI_FAILURE;
    }

    s_pstScenePipeParam = pstScenePipeParam;

    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetPause(HI_BOOL bPause)
{
    g_bISPPause = bPause;
    return HI_SUCCESS;
}

HI_BOOL PDT_SCENE_GetPauseState(HI_VOID)
{
    return g_bISPPause;

}

HI_PDT_SCENE_PIPE_PARAM_S* PDT_SCENE_GetParam(HI_VOID)
{
    return s_pstScenePipeParam;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
