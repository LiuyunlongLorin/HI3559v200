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
#include "mpi_venc.h"
#include "hi_comm_vpss.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PDT_SCENE_MAX(a, b) (((a) < (b)) ? (b) : (a))
#define PDT_SCENE_DIV_0TO1(a)  ((0 == (a)) ? 1 : (a))


#define OPENPRINT 0
#define HI_PDT_SCENE_REF_EXP_RATIO_FRM   (16)
#define HI_PDT_SCENE_REF_EXP_RATIO_LEVEL (9)

static HI_BOOL s_bISPPause = HI_FALSE;
static HI_PDT_SCENE_PIPE_PARAM_S *s_pstScenePipeParam = HI_NULL;


#define PDT_CHECK_SCENE_PAUSE()\
    do{\
        if (HI_TRUE == s_bISPPause)\
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
    if (u32Count == 0) {
        MLOGE("error input.\n");
        return u32Level;
    }
    for (u32Level = 0; u32Level < u32Count; u32Level++) {
        if (u64Value <= pu64Thresh[u32Level]) {
            break;
        }
    }
    if (u32Level == u32Count) {
        u32Level = u32Count - 1;
    }
    return u32Level;
}

static  HI_U32 PDT_SCENE_GetLevelLtoH_U32(HI_U32 u32Value, HI_U32 u32Count, HI_U32 *pu32Thresh)
{
    HI_U32 u32Level = 0;
    if (u32Count == 0) {
        MLOGE("error input.\n");
        return u32Level;
    }
    for (u32Level = 0; u32Level < u32Count; u32Level++) {
        if (u32Value <= pu32Thresh[u32Level]) {
            break;
        }
    }
    if (u32Level == u32Count) {
        u32Level = u32Count - 1;
    }
    return u32Level;
}


static HI_U32 PDT_SCENE_Interpulate(HI_U32 u32Mid, HI_U32 u32Left, HI_U32 u32LValue, HI_U32 u32Right, HI_U32 u32RValue)
{
    HI_U32 u32Value = 0;
    HI_U32 k = 0;
    if (u32Mid <= u32Left) {
        u32Value = u32LValue;
        return u32Value;
    }
    if (u32Mid >= u32Right) {
        u32Value = u32RValue;
        return u32Value;
    }
    k = (u32Right - u32Left);
    u32Value = (((u32Right - u32Mid) * u32LValue + (u32Mid - u32Left) * u32RValue + (k >> 1)) / k);
    return u32Value;
}

static HI_U32 PDT_SCENE_TimeFilter(HI_U32 u32Para0, HI_U32 u32Para1, HI_U32 u32TimeCnt, HI_U32 u32Index)
{
    HI_U64 u64Temp = 0;
    HI_U32 u32Value = 0;
    if (u32Para0 > u32Para1) {
        u64Temp = (HI_U64)(u32Para0 - u32Para1) << 8;
        u64Temp = (u64Temp * (u32Index + 1)) / PDT_SCENE_DIV_0TO1(u32TimeCnt) >> 8;
        u32Value = u32Para0 - (HI_U32)u64Temp;
    } else {
        u64Temp = (HI_U64)(u32Para1 - u32Para0) << 8;
        u64Temp = (u64Temp * (u32Index + 1)) / PDT_SCENE_DIV_0TO1(u32TimeCnt) >> 8;
        u32Value = u32Para0 + (HI_U32)u64Temp;
    }
    return u32Value;
}

#endif

HI_S32 PDT_SCENE_SetStaticStatisticsCfg(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    ISP_STATISTICS_CFG_S stStatisticsCfg;
    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stStatisticsCfg);
    PDT_CHECK_SCENE_RET(s32Ret);
    if (HI_FALSE == bMetryFixed) {
        for (i = 0; i < AE_ZONE_ROW; i++) {
            for (j = 0; j < AE_ZONE_COLUMN; j++) {
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
#if (!defined(HI3518EV300))
    stExposureAttr.enPriorFrame = s_pstScenePipeParam[u8Index].stStaticAe.u8PriorFrame;
    stExposureAttr.bAEGainSepCfg = s_pstScenePipeParam[u8Index].stStaticAe.bAEGainSepCfg;
#endif
    stExposureAttr.stAuto.stExpTimeRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32MaxTime;
    stExposureAttr.stAuto.stISPDGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoISPDGainMax;
    stExposureAttr.stAuto.stISPDGainRange.u32Min = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoISPDGainMin;
    stExposureAttr.stAuto.stDGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoDGainMax;
    stExposureAttr.stAuto.stDGainRange.u32Min = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoDGainMin;
    stExposureAttr.stAuto.stSysGainRange.u32Max = s_pstScenePipeParam[u8Index].stStaticAe.u32AutoSysGainMax;
    stExposureAttr.stAuto.u8Speed = s_pstScenePipeParam[u8Index].stStaticAe.u8AutoSpeed;
    stExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = s_pstScenePipeParam[u8Index].stStaticAe.u16WhiteDelayFrame;
    stExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = s_pstScenePipeParam[u8Index].stStaticAe.u16BlackDelayFrame;
    stExposureAttr.stAuto.u8Tolerance = s_pstScenePipeParam[u8Index].stStaticAe.u8AutoTolerance;
    s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticAERoute(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (s_pstScenePipeParam[u8Index].stStaticAe.bAERouteExValid) {
        return HI_SUCCESS;
    }
    ISP_AE_ROUTE_S stAeRoute;
    s32Ret = HI_MPI_ISP_GetAERouteAttr(ViPipe, &stAeRoute);
    PDT_CHECK_SCENE_RET(s32Ret);
    stAeRoute.u32TotalNum = s_pstScenePipeParam[u8Index].stStaticAeRoute.u32TotalNum;
    for (i = 0; i < stAeRoute.u32TotalNum; i++) {
        stAeRoute.astRouteNode[i].u32IntTime = s_pstScenePipeParam[u8Index].stStaticAeRoute.au32IntTime[i];
        stAeRoute.astRouteNode[i].u32SysGain = s_pstScenePipeParam[u8Index].stStaticAeRoute.au32SysGain[i];
    }
    s32Ret = HI_MPI_ISP_SetAERouteAttr(ViPipe, &stAeRoute);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticSFAERoute(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
#if (!defined(HI3518EV300))
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticAe.bAEGainSepCfg) {
        return HI_SUCCESS;
    }
    ISP_AE_ROUTE_S stAeRoute;
    s32Ret = HI_MPI_ISP_GetAERouteSFAttr(ViPipe, &stAeRoute);
    PDT_CHECK_SCENE_RET(s32Ret);
    stAeRoute.u32TotalNum = s_pstScenePipeParam[u8Index].stStaticSFAeRoute.u32TotalNum;
    for (i = 0; i < stAeRoute.u32TotalNum; i++) {
        stAeRoute.astRouteNode[i].u32IntTime = s_pstScenePipeParam[u8Index].stStaticSFAeRoute.au32IntTime[i];
        stAeRoute.astRouteNode[i].u32SysGain = s_pstScenePipeParam[u8Index].stStaticSFAeRoute.au32SysGain[i];
    }
    s32Ret = HI_MPI_ISP_SetAERouteSFAttr(ViPipe, &stAeRoute);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
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
    for (i = 0; i < stAeRouteEx.u32TotalNum; i++) {
        stAeRouteEx.astRouteExNode[i].u32IntTime = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32IntTime[i];
        stAeRouteEx.astRouteExNode[i].u32Again = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32Again[i];
        stAeRouteEx.astRouteExNode[i].u32Dgain = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32Dgain[i];
        stAeRouteEx.astRouteExNode[i].u32IspDgain = s_pstScenePipeParam[u8Index].stStaticAeRouteEx.au32IspDgain[i];
    }
    s32Ret = HI_MPI_ISP_SetAERouteAttrEx(ViPipe, &stAeRouteEx);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticWDRExposure(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticWdrExposure.bUsed) {
        return HI_SUCCESS;
    }
    ISP_WDR_EXPOSURE_ATTR_S stWdrExposureAttr;
    s32Ret = HI_MPI_ISP_GetWDRExposureAttr(ViPipe, &stWdrExposureAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stWdrExposureAttr.enExpRatioType = (ISP_OP_TYPE_E)s_pstScenePipeParam[u8Index].stStaticWdrExposure.u8ExpRatioType;
    stWdrExposureAttr.u16Tolerance   = s_pstScenePipeParam[u8Index].stStaticWdrExposure.u16Tolerance;
    stWdrExposureAttr.u16Speed       = s_pstScenePipeParam[u8Index].stStaticWdrExposure.u16Speed;
    stWdrExposureAttr.u32ExpRatioMax = s_pstScenePipeParam[u8Index].stStaticWdrExposure.u32ExpRatioMax;
    stWdrExposureAttr.u32ExpRatioMin = s_pstScenePipeParam[u8Index].stStaticWdrExposure.u32ExpRatioMin;
    for (i = 0; i < EXP_RATIO_NUM; i++) {
        stWdrExposureAttr.au32ExpRatio[i] = s_pstScenePipeParam[u8Index].stStaticWdrExposure.au32ExpRatio[i];
    }
    s32Ret = HI_MPI_ISP_SetWDRExposureAttr(ViPipe, &stWdrExposureAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticFSWDR(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticFsWdr.bUsed) {
        return HI_SUCCESS;
    }
    ISP_WDR_FS_ATTR_S stFSWDRAttr;
    s32Ret = HI_MPI_ISP_GetFSWDRAttr(ViPipe, &stFSWDRAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stFSWDRAttr.enWDRMergeMode = s_pstScenePipeParam[u8Index].stStaticFsWdr.WDRMergeMode;
    stFSWDRAttr.stWDRCombine.bMotionComp = 0;
    s32Ret = HI_MPI_ISP_SetFSWDRAttr(ViPipe, &stFSWDRAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticAWB(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    PDT_CHECK_SCENE_Index(u8Index);
    ISP_WB_ATTR_S stWbAttr;
    ISP_AWB_ATTR_EX_S stWbExAttr;
    s32Ret = HI_MPI_ISP_GetWBAttr(ViPipe, &stWbAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    for (i = 0; i < 4; i++) {
        stWbAttr.stAuto.au16StaticWB[i] = s_pstScenePipeParam[u8Index].stStaticAWB.au16StaticWB[i];
    }
    for (i = 0; i < 6; i++) {
        stWbAttr.stAuto.as32CurvePara[i] = s_pstScenePipeParam[u8Index].stStaticAWB.as32CurvePara[i];
    }
    stWbAttr.stAuto.u16Speed = s_pstScenePipeParam[u8Index].stStaticAWB.u16Speed;
    stWbAttr.stAuto.u16LowColorTemp = s_pstScenePipeParam[u8Index].stStaticAWB.u16LowColorTemp;
    stWbAttr.stAuto.u8ShiftLimit = s_pstScenePipeParam[u8Index].stStaticAWB.u8ShiftLimit;
    stWbAttr.stAuto.bAWBZoneWtEn = s_pstScenePipeParam[u8Index].stStaticAWB.bAWBZoneWtEn;
    for (i = 0; i < (HI_PDT_SCENE_AWB_ZONE_ORIG_ROW * HI_PDT_SCENE_AWB_ZONE_ORIG_COLUMN); i++) {
        stWbAttr.stAuto.au8ZoneWt[i] = s_pstScenePipeParam[u8Index].stStaticAWB.au8ZoneWt[i];
    }
    s32Ret = HI_MPI_ISP_SetWBAttr(ViPipe, &stWbAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    s32Ret = HI_MPI_ISP_GetAWBAttrEx(ViPipe, &stWbExAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stWbExAttr.u8Tolerance = s_pstScenePipeParam[u8Index].stStaticAWB.u8Tolerance;
    stWbExAttr.stInOrOut.u16HighStart = s_pstScenePipeParam[u8Index].stStaticAWB.u16HighStart;
    stWbExAttr.stInOrOut.u16HighStop = s_pstScenePipeParam[u8Index].stStaticAWB.u16HighStop;
    stWbExAttr.stInOrOut.u16LowStart = s_pstScenePipeParam[u8Index].stStaticAWB.u16LowStart;
    stWbExAttr.stInOrOut.u16LowStop = s_pstScenePipeParam[u8Index].stStaticAWB.u16LowStop;
    stWbExAttr.stInOrOut.u32OutThresh = s_pstScenePipeParam[u8Index].stStaticAWB.u32OutThresh;
    stWbExAttr.stInOrOut.u8OutShiftLimit = s_pstScenePipeParam[u8Index].stStaticAWB.u8OutShiftLimit;
    stWbExAttr.bFineTunEn = s_pstScenePipeParam[u8Index].stStaticAWB.bFineTunEn;
    s32Ret = HI_MPI_ISP_SetAWBAttrEx(ViPipe, &stWbExAttr);
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
    if (!s_pstScenePipeParam[u8Index].stStaticCCM.bUsed) {
        return HI_SUCCESS;
    }
    ISP_COLORMATRIX_ATTR_S stColormatrixAttr;
    s32Ret = HI_MPI_ISP_GetCCMAttr(ViPipe, &stColormatrixAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stColormatrixAttr.stAuto.u16CCMTabNum = (HI_U16)s_pstScenePipeParam[u8Index].stStaticCCM.u32TotalNum;
    for (i = 0; i < s_pstScenePipeParam[u8Index].stStaticCCM.u32TotalNum; i++) {
        stColormatrixAttr.stAuto.astCCMTab[i].u16ColorTemp = s_pstScenePipeParam[u8Index].stStaticCCM.au16AutoColorTemp[i];
        for (j = 0; j < CCM_MATRIX_SIZE; j++) {
            stColormatrixAttr.stAuto.astCCMTab[i].au16CCM[j] = s_pstScenePipeParam[u8Index].stStaticCCM.au16AutoCCM[i][j];
        }
    }
    s32Ret = HI_MPI_ISP_SetCCMAttr(ViPipe, &stColormatrixAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticSaturation(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticSaturation.bUsed) {
        return HI_SUCCESS;
    }
    HI_U32 i = 0;
    ISP_SATURATION_ATTR_S stSaturationAttr;
    s32Ret = HI_MPI_ISP_GetSaturationAttr(ViPipe, &stSaturationAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stSaturationAttr.stAuto.au8Sat[i] = s_pstScenePipeParam[u8Index].stStaticSaturation.au8Sat[i];
    }
    HI_MPI_ISP_SetSaturationAttr(ViPipe, &stSaturationAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticClut(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticClut.bUsed) {
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

HI_S32 PDT_SCENE_SetStaticDRC(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticDrc.bUsed) {
        return HI_SUCCESS;
    }
    ISP_DRC_ATTR_S stDrcAttr;
    s32Ret = HI_MPI_ISP_GetDRCAttr(ViPipe, &stDrcAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stDrcAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDrc.bEnable;
    stDrcAttr.enCurveSelect = (ISP_DRC_CURVE_SELECT_E)s_pstScenePipeParam[u8Index].stStaticDrc.u8CurveSelect;
    stDrcAttr.enOpType = (ISP_OP_TYPE_E)s_pstScenePipeParam[u8Index].stStaticDrc.u8DRCOpType;
    for (i = 0; i < HI_ISP_DRC_TM_NODE_NUM; i++) {
        stDrcAttr.au16ToneMappingValue[i] = s_pstScenePipeParam[u8Index].stStaticDrc.au16ToneMappingValue[i];
    }
    s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stDrcAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDEHAZE(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticDehaze.bUsed) {
        return HI_SUCCESS;
    }
    ISP_DEHAZE_ATTR_S stDehazeAttr;
    s32Ret = HI_MPI_ISP_GetDehazeAttr(ViPipe, &stDehazeAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stDehazeAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDehaze.bEnable;
    stDehazeAttr.enOpType = (ISP_OP_TYPE_E)s_pstScenePipeParam[u8Index].stStaticDehaze.u8DehazeOpType;
    stDehazeAttr.bUserLutEnable = s_pstScenePipeParam[u8Index].stStaticDehaze.bUserLutEnable;
    for (i = 0; i < 256; i++) {
        stDehazeAttr.au8DehazeLut[i] = s_pstScenePipeParam[u8Index].stStaticDehaze.au8DehazeLut[i];
    }
    s32Ret = HI_MPI_ISP_SetDehazeAttr(ViPipe, &stDehazeAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticLDCI(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticLdci.bUsed) {
        return HI_SUCCESS;
    }
    ISP_LDCI_ATTR_S stLDCIAttr;
    HI_S32 i = 0;
    s32Ret = HI_MPI_ISP_GetLDCIAttr(ViPipe, &stLDCIAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stLDCIAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticLdci.bEnable;
    stLDCIAttr.enOpType = (ISP_OP_TYPE_E)s_pstScenePipeParam[u8Index].stStaticLdci.u8LDCIOpType;
    stLDCIAttr.u8GaussLPFSigma = s_pstScenePipeParam[u8Index].stStaticLdci.u8GaussLPFSigma;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stLDCIAttr.stAuto.au16BlcCtrl[i] = s_pstScenePipeParam[u8Index].stStaticLdci.au16AutoBlcCtrl[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Sigma = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosSigma[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Wgt = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosWgt[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHePosWgt.u8Mean = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHePosMean[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Sigma = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegSigma[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Wgt = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegWgt[i];
        stLDCIAttr.stAuto.astHeWgt[i].stHeNegWgt.u8Mean = s_pstScenePipeParam[u8Index].stStaticLdci.au8AutoHeNegMean[i];
    }
    s32Ret = HI_MPI_ISP_SetLDCIAttr(ViPipe, &stLDCIAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDemosaic(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticDemosaic.bUsed) {
        return HI_SUCCESS;
    }
    ISP_DEMOSAIC_ATTR_S stDemosaicAttr;
    s32Ret = HI_MPI_ISP_GetDemosaicAttr(ViPipe, &stDemosaicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stDemosaicAttr.stAuto.au8NonDirStr[i] = s_pstScenePipeParam[u8Index].stStaticDemosaic.au8NonDirStr[i];
        stDemosaicAttr.stAuto.au8NonDirMFDetailEhcStr[i] =
        s_pstScenePipeParam[u8Index].stStaticDemosaic.au8NonDirMFDetailEhcStr[i];
        stDemosaicAttr.stAuto.au16DetailSmoothStr[i]     = s_pstScenePipeParam[u8Index].stStaticDemosaic.au16DetailSmoothStr[i];
    }
    s32Ret = HI_MPI_ISP_SetDemosaicAttr(ViPipe, &stDemosaicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticNr(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    ISP_NR_ATTR_S stNrAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticNr.bUsed) {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_ISP_GetNRAttr(ViPipe, &stNrAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stNrAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticNr.bEnable;
    for (i = 0; i < HI_ISP_BAYERNR_LUT_LENGTH; i++) {
        stNrAttr.au16CoringRatio[i] = s_pstScenePipeParam[u8Index].stStaticNr.au16CoringRatio[i];
    }
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stNrAttr.stAuto.au16CoringWgt[i] = s_pstScenePipeParam[u8Index].stStaticNr.au16CoringWgt[i];
        stNrAttr.stAuto.au8FineStr[i] = s_pstScenePipeParam[u8Index].stStaticNr.au8FineStr[i];
    }
    s32Ret = HI_MPI_ISP_SetNRAttr(ViPipe, &stNrAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}


HI_S32 PDT_SCENE_SetStaticCAC(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    ISP_LOCAL_CAC_ATTR_S stCACAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticCAC.bUsed) {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_ISP_GetLocalCacAttr(ViPipe, &stCACAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stCACAttr.enOpType = OP_TYPE_AUTO;
    stCACAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticCAC.bEnable;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stCACAttr.stAuto.au8DePurpleCrStr[i] = s_pstScenePipeParam[u8Index].stStaticCAC.au8DePurpleCrStr[i];
        stCACAttr.stAuto.au8DePurpleCbStr[i] = s_pstScenePipeParam[u8Index].stStaticCAC.au8DePurpleCbStr[i];
    }
    s32Ret = HI_MPI_ISP_SetLocalCacAttr(ViPipe, &stCACAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}



HI_S32 PDT_SCENE_SetStaticDPC(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticDPC.bUsed) {
        return HI_SUCCESS;
    }
    ISP_DP_DYNAMIC_ATTR_S stDPDynamicAttr;
    s32Ret = HI_MPI_ISP_GetDPDynamicAttr(ViPipe, &stDPDynamicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stDPDynamicAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDPC.bEnable;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stDPDynamicAttr.stAuto.au16Strength[i]   = s_pstScenePipeParam[u8Index].stStaticDPC.au16Strength[i];
        stDPDynamicAttr.stAuto.au16BlendRatio[i] = s_pstScenePipeParam[u8Index].stStaticDPC.au16BlendRatio[i];
    }
    s32Ret = HI_MPI_ISP_SetDPDynamicAttr(ViPipe, &stDPDynamicAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticShading(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticShading.bUsed) {
        return HI_SUCCESS;
    }
    ISP_SHADING_ATTR_S stShadingAttr;
    s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe, &stShadingAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stShadingAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticShading.bEnable;
    s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe, &stShadingAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticDE(VI_PIPE ViPipe, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stStaticDE.bUsed) {
        return HI_SUCCESS;
    }
    ISP_DE_ATTR_S stDEAttr;
    s32Ret = HI_MPI_ISP_GetDEAttr(ViPipe, &stDEAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stDEAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticDE.bEnable;
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stDEAttr.stAuto.au16GlobalGain[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GlobalGain[i];
        stDEAttr.stAuto.au16GainLF[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GainLF[i];
        stDEAttr.stAuto.au16GainHF[i] = s_pstScenePipeParam[u8Index].stStaticDE.au16GainHF[i];
    }
    s32Ret = HI_MPI_ISP_SetDEAttr(ViPipe, &stDEAttr);
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
    if (!s_pstScenePipeParam[u8Index].stStaticSharpen.bUsed) {
        return HI_SUCCESS;
    }
    ISP_SHARPEN_ATTR_S stSharpenAttr;
    s32Ret = HI_MPI_ISP_GetIspSharpenAttr(ViPipe, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    stSharpenAttr.bEnable = s_pstScenePipeParam[u8Index].stStaticSharpen.bEnable;
    for (i = 0; i < ISP_SHARPEN_GAIN_NUM; i++) {
        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++) {
            stSharpenAttr.stAuto.au8LumaWgt[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8LumaWgt[i][j];
            stSharpenAttr.stAuto.au16TextureStr[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureStr[i][j];
            stSharpenAttr.stAuto.au16EdgeStr[i][j] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoEdgeStr[i][j];
        }
    }
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++) {
        stSharpenAttr.stAuto.au8OverShoot[i]     = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoOverShoot[i];
        stSharpenAttr.stAuto.au8UnderShoot[i]    = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoUnderShoot[i];
        stSharpenAttr.stAuto.au8ShootSupStr[i]   = s_pstScenePipeParam[u8Index].stStaticSharpen.au8ShootSupStr[i];
        stSharpenAttr.stAuto.au8ShootSupAdj[i]   = s_pstScenePipeParam[u8Index].stStaticSharpen.au8ShootSupAdj[i];
        stSharpenAttr.stAuto.au16TextureFreq[i]  = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureFreq[i];
        stSharpenAttr.stAuto.au16EdgeFreq[i]     = s_pstScenePipeParam[u8Index].stStaticSharpen.au16AutoEdgeFreq[i];
        stSharpenAttr.stAuto.au8DetailCtrl[i]    = s_pstScenePipeParam[u8Index].stStaticSharpen.au8AutoDetailCtrl[i];
        stSharpenAttr.stAuto.au8DetailCtrlThr[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au8DetailCtrlThr[i];
        stSharpenAttr.stAuto.au8EdgeFiltStr[i]   = s_pstScenePipeParam[u8Index].stStaticSharpen.au8EdgeFiltStr[i];
        stSharpenAttr.stAuto.au8RGain[i]         = s_pstScenePipeParam[u8Index].stStaticSharpen.au8RGain[i];
        stSharpenAttr.stAuto.au8BGain[i]         = s_pstScenePipeParam[u8Index].stStaticSharpen.au8BGain[i];
        stSharpenAttr.stAuto.au8SkinGain[i]      = s_pstScenePipeParam[u8Index].stStaticSharpen.au8SkinGain[i];
        stSharpenAttr.stAuto.au16MaxSharpGain[i] = s_pstScenePipeParam[u8Index].stStaticSharpen.au16MaxSharpGain[i];
    }
    s32Ret = HI_MPI_ISP_SetIspSharpenAttr(ViPipe, &stSharpenAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetStaticQP(VENC_CHN VeChn, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    if (!s_pstScenePipeParam[u8Index].stStaticQP.bUsed) {
        return HI_SUCCESS;
    }
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    VENC_HIERARCHICAL_QP_S stHierarchicalQP;
    s32Ret = HI_MPI_VENC_GetHierarchicalQp(VeChn, &stHierarchicalQP);
    PDT_CHECK_SCENE_RET(s32Ret);
    stHierarchicalQP.bHierarchicalQpEn = s_pstScenePipeParam[u8Index].stStaticQP.bHierarchicalQpEn;
    for (i = 0; i < 4; i++) {
        stHierarchicalQP.s32HierarchicalQpDelta[i] = s_pstScenePipeParam[u8Index].stStaticQP.as32HierarchicalQpDelta[i];
        stHierarchicalQP.s32HierarchicalFrameNum[i] = s_pstScenePipeParam[u8Index].stStaticQP.as32HierarchicalFrameNum[i];
    }
    s32Ret = HI_MPI_VENC_SetHierarchicalQp(VeChn, &stHierarchicalQP);
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
    if (!s_pstScenePipeParam[u8Index].stStaticCrossTalk.bUsed) {
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
    if (!s_pstScenePipeParam[u8Index].stStaticCa.bUsed) {
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

HI_S32 PDT_SCENE_SetDynamicAE(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32ExpLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicAe.bUsed) {
        return HI_SUCCESS;
    }
    if (u64Exposure != u64LastExposure) {
        ISP_EXPOSURE_ATTR_S stExposureAttr;
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetExposureAttr(ViPipe, &stExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt > HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt,
                                             s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh);
        if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt) {
            return HI_FAILURE;
        }
        if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt - 1) {
            stExposureAttr.stAuto.u8Compensation = s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel];
            stExposureAttr.stAuto.u8MaxHistOffset = s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel];
            stExposureAttr.stAuto.stAntiflicker.bEnable = s_pstScenePipeParam[u8Index].stDynamicAe.au8AntiFlickerEn[u32ExpLevel];
        } else {
            stExposureAttr.stAuto.u8Compensation = PDT_SCENE_Interpulate(u64Exposure,
                                                   s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],
                                                   s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel]);
            stExposureAttr.stAuto.u8MaxHistOffset = PDT_SCENE_Interpulate(u64Exposure,
                                                    s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel - 1],
                                                    s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel - 1],
                                                    s_pstScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],
                                                    s_pstScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel]);
            if ((HI_TRUE == s_pstScenePipeParam[u8Index].stDynamicAe.au8AntiFlickerEn[u32ExpLevel - 1]) &&
                    (HI_TRUE == s_pstScenePipeParam[u8Index].stDynamicAe.au8AntiFlickerEn[u32ExpLevel])) {
                stExposureAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
            } else {
                stExposureAttr.stAuto.stAntiflicker.bEnable = HI_FALSE;
            }
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#if 1
    if (s_pstScenePipeParam[u8Index].stStaticAe.u8ChangeFpsEnable) {
        static HI_U8 u8Stat = HI_TRUE;
        static HI_U8 u8fpschg_flag = HI_FALSE;
        static HI_U32 u8Stat_count = 0;
        if (HI_TRUE == u8Stat) {
            if (10 == u8Stat_count) {
                ISP_EXP_INFO_S stIspExpInfo;
                ISP_PUB_ATTR_S stPubAttr;
                s32Ret = HI_MPI_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);
                s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubAttr);
                if ((stIspExpInfo.u32ISO >= 850) && (((HI_U32)stPubAttr.f32FrameRate) > 100) && (HI_FALSE == u8fpschg_flag)) {
                    stPubAttr.f32FrameRate = 60.0;
                    u8fpschg_flag = HI_TRUE;
                    HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);
                    MLOGI(" ### f32FrameRate change from 120->60!###\n");
#if defined(AMP_LINUX_HUAWEILITE)
                    s32Ret = HI_MSG_SendASync(MSG_PDT_SCENE_FRAMERATE_FROM120TO60, HI_NULL, 0, HI_NULL);
                    if (HI_SUCCESS != s32Ret) {
                        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_FRAMERATE_FROM120TO60 Failed, %x\n\n, "NONE, s32Ret);
                        return s32Ret;
                    }
#endif
                } else if ((stIspExpInfo.u32ISO <= 550) && (HI_TRUE == u8fpschg_flag)) {
                    stPubAttr.f32FrameRate = 120.0;
                    u8fpschg_flag = HI_FALSE;
                    HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);
                    MLOGI(" ### f32FrameRate change from 60->120!###\n");
#if defined(AMP_LINUX_HUAWEILITE)
                    s32Ret = HI_MSG_SendASync(MSG_PDT_SCENE_FRAMERATE_FROM60TO120, HI_NULL, 0, HI_NULL);
                    if (HI_SUCCESS != s32Ret) {
                        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_FRAMERATE_FROM60TO120 Failed, %x\n\n, "NONE, s32Ret);
                        return s32Ret;
                    }
#endif
                }
                u8Stat_count = 0;
            }
            u8Stat_count++;
        }
    }
#endif
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicWdrExposure(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation, HI_U64 u64Exposure,
                                       HI_U64 u64LastExposure)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32ExpLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicWdrExposure.bUsed) {
        return HI_SUCCESS;
    }
    if (u64Exposure != u64LastExposure) {
        ISP_WDR_EXPOSURE_ATTR_S stWdrExposureAttr;
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetWDRExposureAttr(ViPipe, &stWdrExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicWdrExposure.u8ExposureCnt < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicWdrExposure.u8ExposureCnt > HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicWdrExposure.u8ExposureCnt,
                                             s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au64ExpLtoHThresh);
        if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicWdrExposure.u8ExposureCnt) {
            return HI_FAILURE;
        }
        if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicWdrExposure.u8ExposureCnt - 1) {
            stWdrExposureAttr.u32ExpRatioMax = s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMax[u32ExpLevel];
            stWdrExposureAttr.u32ExpRatioMin = s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMin[u32ExpLevel];
        } else {
            stWdrExposureAttr.u32ExpRatioMax = PDT_SCENE_Interpulate(u64Exposure,
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au64ExpLtoHThresh[u32ExpLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMax[u32ExpLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au64ExpLtoHThresh[u32ExpLevel],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMax[u32ExpLevel]);
            stWdrExposureAttr.u32ExpRatioMin = PDT_SCENE_Interpulate(u64Exposure,
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au64ExpLtoHThresh[u32ExpLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMin[u32ExpLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au64ExpLtoHThresh[u32ExpLevel],
                                               s_pstScenePipeParam[u8Index].stDynamicWdrExposure.au32ExpRatioMin[u32ExpLevel]);
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetWDRExposureAttr(ViPipe, &stWdrExposureAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicFsWdr(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicFsWdr.bUsed) {
        return HI_SUCCESS;
    }
    ISP_WDR_FS_ATTR_S stFSWDRAttr;
    if (u32ISO != u32LastISO) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetFSWDRAttr(ViPipe, &stFSWDRAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (u32ISO < s_pstScenePipeParam[u8Index].stDynamicFsWdr.u32ISO) {
            stFSWDRAttr.enWDRMergeMode = s_pstScenePipeParam[u8Index].stDynamicFsWdr.au8WDRMergeMode[0];
        } else if (s_pstScenePipeParam[u8Index].stDynamicFsWdr.u32ISO <= u32ISO) {
            stFSWDRAttr.enWDRMergeMode = s_pstScenePipeParam[u8Index].stDynamicFsWdr.au8WDRMergeMode[1];
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetFSWDRAttr(ViPipe, &stFSWDRAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicClut(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32IsoLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_CLUT_ATTR_S stClutAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicClut.bUsed) {
        return HI_SUCCESS;
    }
    if (u32ISO != u32LastISO) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetClutAttr(ViPipe, &stClutAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicClut.u32ISOCount < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicClut.u32ISOCount > HI_PDT_SCENE_CLUT_ISO_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32IsoLevel = PDT_SCENE_GetLevelLtoH_U32(u32ISO, s_pstScenePipeParam[u8Index].stDynamicClut.u32ISOCount,
                      s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel);
        if (u32IsoLevel < 0 || u32IsoLevel >= s_pstScenePipeParam[u8Index].stDynamicClut.u32ISOCount) {
            return HI_FAILURE;
        }
        if (u32IsoLevel == 0 || u32IsoLevel == s_pstScenePipeParam[u8Index].stDynamicClut.u32ISOCount - 1) {
            stClutAttr.u32GainR = s_pstScenePipeParam[u8Index].stDynamicClut.au32GainR[u32IsoLevel];
            stClutAttr.u32GainG = s_pstScenePipeParam[u8Index].stDynamicClut.au32GainG[u32IsoLevel];
            stClutAttr.u32GainB = s_pstScenePipeParam[u8Index].stDynamicClut.au32GainB[u32IsoLevel];
        } else {
            stClutAttr.u32GainR = PDT_SCENE_Interpulate(u32ISO,
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainR[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainR[u32IsoLevel]);
            stClutAttr.u32GainG = PDT_SCENE_Interpulate(u32ISO,
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainG[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainG[u32IsoLevel]);
            stClutAttr.u32GainB = PDT_SCENE_Interpulate(u32ISO,
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainB[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32ISOLevel[u32IsoLevel],
                                  s_pstScenePipeParam[u8Index].stDynamicClut.au32GainB[u32IsoLevel]);
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetClutAttr(ViPipe, &stClutAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicNR(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ISOLevel = 0;
    ISP_NR_ATTR_S stNrAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicNR.bUsed) {
        return HI_SUCCESS;
    }
    if (u32ISO != u32LastISO) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetNRAttr(ViPipe, &stNrAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicNR.u16ISOCount < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicNR.u16ISOCount > ISP_AUTO_ISO_STRENGTH_NUM) {
            return HI_FAILURE;
        }
        u32ISOLevel = PDT_SCENE_GetLevelLtoH_U32(u32ISO, s_pstScenePipeParam[u8Index].stDynamicNR.u16ISOCount,
                      s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO);
        if (u32ISOLevel < 0 || u32ISOLevel >= s_pstScenePipeParam[u8Index].stDynamicNR.u16ISOCount) {
            return HI_FAILURE;
        }
        if (0 == u32ISOLevel || (u32ISOLevel == (s_pstScenePipeParam[u8Index].stDynamicNR.u16ISOCount - 1))) {
            stNrAttr.stWdr.au8FusionFrameStr[0] = s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel];
            stNrAttr.stWdr.au8FusionFrameStr[1] = s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel];
            stNrAttr.stWdr.au8WDRFrameStr[0]    = s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel];
            stNrAttr.stWdr.au8WDRFrameStr[1]    = s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel];
        } else {
            stNrAttr.stWdr.au8FusionFrameStr[0] = PDT_SCENE_Interpulate(u32ISO,
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel - 1],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel - 1],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel]);
            stNrAttr.stWdr.au8FusionFrameStr[1] = PDT_SCENE_Interpulate(u32ISO,
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel - 1],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel - 1],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel],
                                                  s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel]);
            stNrAttr.stWdr.au8WDRFrameStr[0] = PDT_SCENE_Interpulate(u32ISO,
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrShort[u32ISOLevel]);
            stNrAttr.stWdr.au8WDRFrameStr[1] = PDT_SCENE_Interpulate(u32ISO,
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au32ISO[u32ISOLevel],
                                               s_pstScenePipeParam[u8Index].stDynamicNR.au8FrameStrLong[u32ISOLevel]);
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetNRAttr(ViPipe, &stNrAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicShading(VI_PIPE ViPipe, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ExpLevel = 0;
    ISP_SHADING_ATTR_S stShadingAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicShading.bUsed) {
        return HI_SUCCESS;
    }
    if (u64Exposure != u64LastExposure) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe, &stShadingAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicShading.u16ExpCount < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicShading.u16ExpCount > HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32ExpLevel = PDT_SCENE_GetLevelLtoH_U32(u64Exposure,
                      s_pstScenePipeParam[u8Index].stDynamicShading.u16ExpCount,
                      s_pstScenePipeParam[u8Index].stDynamicShading.au32ExpThreshLtoH);
        if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicShading.u16ExpCount) {
            return HI_FAILURE;
        }
        if (0 == u32ExpLevel || (u32ExpLevel == (s_pstScenePipeParam[u8Index].stDynamicShading.u16ExpCount - 1))) {
            stShadingAttr.u16MeshStr    = s_pstScenePipeParam[u8Index].stDynamicShading.au16MeshStr[u32ExpLevel];
            stShadingAttr.u16BlendRatio = s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel];
        } else {
            stShadingAttr.u16MeshStr    = PDT_SCENE_Interpulate(u64Exposure,
                                          s_pstScenePipeParam[u8Index].stDynamicShading.au32ExpThreshLtoH[u32ExpLevel - 1],
                                          s_pstScenePipeParam[u8Index].stDynamicShading.au16MeshStr[u32ExpLevel - 1],
                                          s_pstScenePipeParam[u8Index].stDynamicShading.au32ExpThreshLtoH[u32ExpLevel],
                                          s_pstScenePipeParam[u8Index].stDynamicShading.au16MeshStr[u32ExpLevel]);
            stShadingAttr.u16BlendRatio    = PDT_SCENE_Interpulate(u64Exposure,
                                             s_pstScenePipeParam[u8Index].stDynamicShading.au32ExpThreshLtoH[u32ExpLevel - 1],
                                             s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel - 1],
                                             s_pstScenePipeParam[u8Index].stDynamicShading.au32ExpThreshLtoH[u32ExpLevel],
                                             s_pstScenePipeParam[u8Index].stDynamicShading.au16BlendRatio[u32ExpLevel]);
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe, &stShadingAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}
HI_S32 PDT_SCENE_SetDynamicCAC(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32IsoLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicCAC.bUsed) {
        return HI_SUCCESS;
    }
    ISP_LOCAL_CAC_ATTR_S stCACAttr;
    if (u32ISO != u32LastISO) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetLocalCacAttr(ViPipe, &stCACAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        u32IsoLevel = PDT_SCENE_GetLevelLtoH_U32(u32ISO, s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOCount,
                      s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOLevel);
        if (u32IsoLevel == 0 || u32IsoLevel == s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOCount - 1) {
            stCACAttr.u16PurpleDetRange = s_pstScenePipeParam[u8Index].stDynamicCAC.u16PurpleDetRange[u32IsoLevel];
            stCACAttr.u16VarThr         = s_pstScenePipeParam[u8Index].stDynamicCAC.u16VarThr[u32IsoLevel];
        } else {
            stCACAttr.u16PurpleDetRange = PDT_SCENE_Interpulate(u32ISO,
                                          s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOLevel[u32IsoLevel - 1],
                                          s_pstScenePipeParam[u8Index].stDynamicCAC.u16PurpleDetRange[u32IsoLevel - 1],
                                          s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOLevel[u32IsoLevel],
                                          s_pstScenePipeParam[u8Index].stDynamicCAC.u16PurpleDetRange[u32IsoLevel]);
            stCACAttr.u16VarThr = PDT_SCENE_Interpulate(u32ISO,
                                  s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOLevel[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicCAC.u16VarThr[u32IsoLevel - 1],
                                  s_pstScenePipeParam[u8Index].stDynamicCAC.au32ISOLevel[u32IsoLevel],
                                  s_pstScenePipeParam[u8Index].stDynamicCAC.u16VarThr[u32IsoLevel]);
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetLocalCacAttr(ViPipe, &stCACAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicBlackLevel(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32IsoLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicBlackLevel.bUsed) {
        return HI_SUCCESS;
    }
    ISP_BLACK_LEVEL_S stBlackLevelAttr;
    if (u32ISO != u32LastISO) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetBlackLevelAttr(ViPipe, &stBlackLevelAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        u32IsoLevel = PDT_SCENE_GetLevelLtoH_U32(u32ISO, s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOCount,
                      s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel);
        if (u32IsoLevel == 0 || u32IsoLevel == s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOCount - 1) {
            stBlackLevelAttr.au16BlackLevel[0] = s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelRR[u32IsoLevel];
            stBlackLevelAttr.au16BlackLevel[1] = s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGR[u32IsoLevel];
            stBlackLevelAttr.au16BlackLevel[2] = s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGB[u32IsoLevel];
            stBlackLevelAttr.au16BlackLevel[3] = s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelBB[u32IsoLevel];
        } else {
            stBlackLevelAttr.au16BlackLevel[0] = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelRR[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelRR[u32IsoLevel]);
            stBlackLevelAttr.au16BlackLevel[1] = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGR[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGR[u32IsoLevel]);
            stBlackLevelAttr.au16BlackLevel[2] = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGB[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelGB[u32IsoLevel]);
            stBlackLevelAttr.au16BlackLevel[3] = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelBB[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicBlackLevel.au16BlackLevelBB[u32IsoLevel]);
        }
        stBlackLevelAttr.enOpType = OP_TYPE_MANUAL;
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetBlackLevelAttr(ViPipe, &stBlackLevelAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}
HI_S32 PDT_SCENE_SetDynamicQVBRVENC(VENC_CHN VeChn, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ExpLevel = 0;
    VENC_RC_PARAM_S stRcParam;
    VENC_CHN_ATTR_S stVencChnAttr;
    PDT_CHECK_SCENE_Index(u8Index);
    HI_BOOL bH265 = HI_FALSE;
    if (!s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.bUsed) {
        return HI_SUCCESS;
    }
    PDT_CHECK_SCENE_PAUSE();
    s32Ret = HI_MPI_VENC_GetChnAttr(VeChn, &stVencChnAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    if (VENC_RC_MODE_H265QVBR == stVencChnAttr.stRcAttr.enRcMode) {
        bH265 = HI_TRUE;
    } else if (VENC_RC_MODE_H264QVBR == stVencChnAttr.stRcAttr.enRcMode) {
        bH265 = HI_FALSE;
    } else {
        return HI_SUCCESS;
    }
    if (u64Exposure != u64LastExposure) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_VENC_GetRcParam(VeChn, &stRcParam);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.u16ExpCount < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.u16ExpCount > HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure,
                                             s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.u16ExpCount,
                                             s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH);
        if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.u16ExpCount) {
            return HI_FAILURE;
        }
        if (0 == u32ExpLevel || (u32ExpLevel == (s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.u16ExpCount - 1))) {
            if (bH265) {
                stRcParam.stParamH265QVbr.u32MaxQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel];
                stRcParam.stParamH265QVbr.u32MinQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel];
                stRcParam.stParamH265QVbr.u32MinIQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel];
            } else {
                stRcParam.stParamH264QVbr.u32MaxQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel];
                stRcParam.stParamH264QVbr.u32MinQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel];
                stRcParam.stParamH264QVbr.u32MinIQp = s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel];
            }
        } else {
            if (bH265) {
                stRcParam.stParamH265QVbr.u32MaxQp = PDT_SCENE_Interpulate(u64Exposure,
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel]);
                stRcParam.stParamH265QVbr.u32MinQp = PDT_SCENE_Interpulate(u64Exposure,
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel]);
                stRcParam.stParamH265QVbr.u32MinIQp = PDT_SCENE_Interpulate(u64Exposure,
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel - 1],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel]);
            } else {
                stRcParam.stParamH264QVbr.u32MaxQp = PDT_SCENE_Interpulate(u64Exposure,
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MaxQp[u32ExpLevel]);
                stRcParam.stParamH264QVbr.u32MinQp = PDT_SCENE_Interpulate(u64Exposure,
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel - 1],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                     s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinQp[u32ExpLevel]);
                stRcParam.stParamH264QVbr.u32MinIQp = PDT_SCENE_Interpulate(u64Exposure,
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel - 1],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au64ExpThreshLtoH[u32ExpLevel],
                                                      s_pstScenePipeParam[u8Index].stDynamicQVBRVENC.au32MinIQp[u32ExpLevel]);
            }
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_VENC_SetRcParam(VeChn, &stRcParam);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicDEHAZE(VI_PIPE ViPipe, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEHAZE_ATTR_S stDehazeAttr;
    HI_U32 u32ExpLevel = 0;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicDehaze.bUsed) {
        return HI_SUCCESS;
    }
    if (u64Exposure != u64LastExposure) {
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_GetDehazeAttr(ViPipe, &stDehazeAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt > HI_PDT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt,
                                             s_pstScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH);
        if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt) {
            return HI_FAILURE;
        }
        if (0 == stDehazeAttr.enOpType) {
            if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt - 1) {
                stDehazeAttr.stAuto.u8strength = s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel];
            } else {
                stDehazeAttr.stAuto.u8strength = PDT_SCENE_Interpulate(u64Exposure,
                                                 s_pstScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel]);
            }
        } else {
            if (u32ExpLevel == 0 || u32ExpLevel == s_pstScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt - 1) {
                stDehazeAttr.stManual.u8strength = s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel];
            } else {
                stDehazeAttr.stManual.u8strength = PDT_SCENE_Interpulate(u64Exposure,
                                                   s_pstScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel],
                                                   s_pstScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel]);
            }
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetDehazeAttr(ViPipe, &stDehazeAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicDRC(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 u32IsoLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicDrc.bUsed) {
        return HI_SUCCESS;
    }
    ISP_PUB_ATTR_S stPubAttr;
    ISP_DRC_ATTR_S stIspDrcAttr;
    HI_U8 u8LocalMixingBrightMax = 0;
    HI_U8 u8LocalMixingBrightMin = 0;
    HI_U8 u8LocalMixingDarkMax = 0;
    HI_U8 u8LocalMixingDarkMin = 0;
    HI_U8 u8BrightGainLmt = 0;
    HI_U8 u8BrightGainLmtStep = 0;
    HI_U8 u8DarkGainLmtY = 0;
    HI_U8 u8DarkGainLmtC = 0;
    HI_S8 s8DetailAdjustFactor = 0;
    HI_U8 u8SpatialFltCoef = 0;
    HI_U8 u8RangeFltCoef = 0;
    HI_U8 u8GradRevMax = 0;
    HI_U8 u8GradRevThr = 0;
    HI_U8 u8Asymmetry = 0;
    HI_U8 u8SecondPole = 0;
    HI_U8 u8Compress = 0;
    HI_U8 u8Stretch = 0;
    HI_U16 u16Strength = 0;
    PDT_CHECK_SCENE_PAUSE();
    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubAttr);
    PDT_CHECK_SCENE_PAUSE();
    PDT_CHECK_SCENE_PAUSE();
    s32Ret = HI_MPI_ISP_GetDRCAttr(ViPipe, &stIspDrcAttr);
    PDT_CHECK_SCENE_RET(s32Ret);
    if (s_pstScenePipeParam[u8Index].stDynamicDrc.u32ISOCount < 1 ||
            s_pstScenePipeParam[u8Index].stDynamicDrc.u32ISOCount > HI_PDT_SCENE_DRC_ISO_MAX_COUNT) {
        return HI_FAILURE;
    }
    u32IsoLevel = PDT_SCENE_GetLevelLtoH_U32(u32ISO, s_pstScenePipeParam[u8Index].stDynamicDrc.u32ISOCount,
                  s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel);
    if (u32IsoLevel < 0 || u32IsoLevel >= s_pstScenePipeParam[u8Index].stDynamicDrc.u32ISOCount) {
        return HI_FAILURE;
    }
    if (u32ISO != u32LastISO) {
        if (u32IsoLevel == 0 || u32IsoLevel == s_pstScenePipeParam[u8Index].stDynamicDrc.u32ISOCount - 1) {
            u8LocalMixingBrightMax = s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMax[u32IsoLevel];
            u8LocalMixingBrightMin = s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMin[u32IsoLevel];
            u8LocalMixingDarkMax   = s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMax[u32IsoLevel];
            u8LocalMixingDarkMin   = s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMin[u32IsoLevel];
            u8BrightGainLmt        = s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmt[u32IsoLevel];
            u8BrightGainLmtStep    = s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmtStep[u32IsoLevel];
            u8DarkGainLmtY         = s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtY[u32IsoLevel];
            u8DarkGainLmtC         = s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtC[u32IsoLevel];
            s8DetailAdjustFactor   = s_pstScenePipeParam[u8Index].stDynamicDrc.as8DetailAdjustFactor[u32IsoLevel];
            u8SpatialFltCoef       = s_pstScenePipeParam[u8Index].stDynamicDrc.au8SpatialFltCoef[u32IsoLevel];
            u8RangeFltCoef         = s_pstScenePipeParam[u8Index].stDynamicDrc.au8RangeFltCoef[u32IsoLevel];
            u8GradRevMax           = s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevMax[u32IsoLevel];
            u8GradRevThr           = s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevThr[u32IsoLevel];
            u8Asymmetry            = s_pstScenePipeParam[u8Index].stDynamicDrc.au8Asymmetry[u32IsoLevel];
            u8SecondPole           = s_pstScenePipeParam[u8Index].stDynamicDrc.au8SecondPole[u32IsoLevel];
            u8Compress             = s_pstScenePipeParam[u8Index].stDynamicDrc.au8Compress[u32IsoLevel];
            u8Stretch              = s_pstScenePipeParam[u8Index].stDynamicDrc.au8Stretch[u32IsoLevel];
            u16Strength            = s_pstScenePipeParam[u8Index].stDynamicDrc.u16Strength[u32IsoLevel];
        } else {
            u8LocalMixingBrightMax = PDT_SCENE_Interpulate(u32ISO,
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMax[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMax[u32IsoLevel]);
            u8LocalMixingBrightMin = PDT_SCENE_Interpulate(u32ISO,
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMin[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingBrightMin[u32IsoLevel]);
            u8LocalMixingDarkMax = PDT_SCENE_Interpulate(u32ISO,
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMax[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMax[u32IsoLevel]);
            u8LocalMixingDarkMin = PDT_SCENE_Interpulate(u32ISO,
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMin[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8LocalMixingDarkMin[u32IsoLevel]);
            u8BrightGainLmt        = PDT_SCENE_Interpulate(u32ISO,
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmt[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmt[u32IsoLevel]);
            u8BrightGainLmtStep    = PDT_SCENE_Interpulate(u32ISO,
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmtStep[u32IsoLevel - 1],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                     s_pstScenePipeParam[u8Index].stDynamicDrc.au8BrightGainLmtStep[u32IsoLevel]);
            u8DarkGainLmtY = PDT_SCENE_Interpulate(u32ISO,
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtY[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtY[u32IsoLevel]);
            u8DarkGainLmtC = PDT_SCENE_Interpulate(u32ISO,
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtC[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8DarkGainLmtC[u32IsoLevel]);
            s8DetailAdjustFactor = PDT_SCENE_Interpulate(u32ISO,
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.as8DetailAdjustFactor[u32IsoLevel - 1],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                   s_pstScenePipeParam[u8Index].stDynamicDrc.as8DetailAdjustFactor[u32IsoLevel]);
            u8SpatialFltCoef = PDT_SCENE_Interpulate(u32ISO,
                               s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                               s_pstScenePipeParam[u8Index].stDynamicDrc.au8SpatialFltCoef[u32IsoLevel - 1],
                               s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                               s_pstScenePipeParam[u8Index].stDynamicDrc.au8SpatialFltCoef[u32IsoLevel]);
            u8RangeFltCoef = PDT_SCENE_Interpulate(u32ISO,
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8RangeFltCoef[u32IsoLevel - 1],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                   s_pstScenePipeParam[u8Index].stDynamicDrc.au8RangeFltCoef[u32IsoLevel]);
            u8GradRevMax = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevMax[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevMax[u32IsoLevel]);
            u8GradRevThr = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevThr[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8GradRevThr[u32IsoLevel]);
            u8Asymmetry = PDT_SCENE_Interpulate(u32ISO,
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au8Asymmetry[u32IsoLevel - 1],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au8Asymmetry[u32IsoLevel]);
            u8SecondPole = PDT_SCENE_Interpulate(u32ISO,
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8SecondPole[u32IsoLevel - 1],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                 s_pstScenePipeParam[u8Index].stDynamicDrc.au8SecondPole[u32IsoLevel]);
            u8Compress = PDT_SCENE_Interpulate(u32ISO,
                                               s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicDrc.au8Compress[u32IsoLevel - 1],
                                               s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                               s_pstScenePipeParam[u8Index].stDynamicDrc.au8Compress[u32IsoLevel]);
            u8Stretch = PDT_SCENE_Interpulate(u32ISO,
                                              s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                              s_pstScenePipeParam[u8Index].stDynamicDrc.au8Stretch[u32IsoLevel - 1],
                                              s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                              s_pstScenePipeParam[u8Index].stDynamicDrc.au8Stretch[u32IsoLevel]);
            u16Strength = PDT_SCENE_Interpulate(u32ISO,
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel - 1],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.u16Strength[u32IsoLevel - 1],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.au32ISOLevel[u32IsoLevel],
                                                s_pstScenePipeParam[u8Index].stDynamicDrc.u16Strength[u32IsoLevel]);
        }
        stIspDrcAttr.u8DetailDarkStep = 0;
        stIspDrcAttr.bEnable = s_pstScenePipeParam[u8Index].stDynamicDrc.bEnable;
        stIspDrcAttr.u8LocalMixingBrightMax = u8LocalMixingBrightMax;
        stIspDrcAttr.u8LocalMixingBrightMin = u8LocalMixingBrightMin;
        stIspDrcAttr.u8LocalMixingDarkMax = u8LocalMixingDarkMax;
        stIspDrcAttr.u8LocalMixingDarkMin = u8LocalMixingDarkMin;
        stIspDrcAttr.u8BrightGainLmt = u8BrightGainLmt;
        stIspDrcAttr.u8BrightGainLmtStep = u8BrightGainLmtStep;
        stIspDrcAttr.u8DarkGainLmtY = u8DarkGainLmtY;
        stIspDrcAttr.u8DarkGainLmtC = u8DarkGainLmtC;
        stIspDrcAttr.s8DetailAdjustFactor = s8DetailAdjustFactor;
        stIspDrcAttr.u8SpatialFltCoef = u8SpatialFltCoef;
        stIspDrcAttr.u8RangeFltCoef = u8RangeFltCoef;
        stIspDrcAttr.u8GradRevMax = u8GradRevMax;
        stIspDrcAttr.u8GradRevThr = u8GradRevThr;
        stIspDrcAttr.stAsymmetryCurve.u8Asymmetry = u8Asymmetry;
        stIspDrcAttr.stAsymmetryCurve.u8SecondPole = u8SecondPole;
        stIspDrcAttr.stAsymmetryCurve.u8Compress = u8Compress;
        stIspDrcAttr.stAsymmetryCurve.u8Stretch = u8Stretch;
        if (0 == stIspDrcAttr.enOpType) {
            stIspDrcAttr.stAuto.u16Strength = u16Strength;
        } else if (1 == stIspDrcAttr.enOpType) {
            stIspDrcAttr.stManual.u16Strength = u16Strength;
        }
        PDT_CHECK_SCENE_PAUSE();
        s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stIspDrcAttr);
        PDT_CHECK_SCENE_RET(s32Ret);
    } else {
    }
    if (stIspDrcAttr.enCurveSelect == 2) {
        HI_U32 i = 0, j = 0;
        HI_U32 u32RatioLevel = 0;
        HI_U32 u32Ratio = 0;
        static HI_U32 u32LastRatio = 0;
        HI_U32 u32Sign = 0;
        HI_U16 au16ToneMappingValue[200] = {0};
        ISP_INNER_STATE_INFO_S stInnerStateInfo;
        s32Ret = HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
        PDT_CHECK_SCENE_RET(s32Ret);
        if (s_pstScenePipeParam[u8Index].stDynamicDrc.u32RatioCount < 1 ||
                s_pstScenePipeParam[u8Index].stDynamicDrc.u32RatioCount > HI_SCENE_DRC_RATIO_MAX_COUNT) {
            return HI_FAILURE;
        }
        u32Ratio = stInnerStateInfo.u32WDRExpRatioActual[0];
        u32RatioLevel = PDT_SCENE_GetLevelLtoH_U32(u32Ratio, s_pstScenePipeParam[u8Index].stDynamicDrc.u32RatioCount,
                        s_pstScenePipeParam[u8Index].stDynamicDrc.au32RatioLevel);
        if (u32RatioLevel < 0 || u32RatioLevel >= s_pstScenePipeParam[u8Index].stDynamicDrc.u32RatioCount) {
            return HI_FAILURE;
        }
        if (0 == u32RatioLevel) {
            for (i = 0; i < 200; i++) {
                au16ToneMappingValue[i] = s_pstScenePipeParam[u8Index].stDynamicDrc.au16TMValue[u32RatioLevel][i];
            }
        } else {
            for (i = 0; i < 200; i++) {
                au16ToneMappingValue[i] = PDT_SCENE_Interpulate(u32Ratio,
                                          s_pstScenePipeParam[u8Index].stDynamicDrc.au32RatioLevel[u32RatioLevel - 1],
                                          s_pstScenePipeParam[u8Index].stDynamicDrc.au16TMValue[u32RatioLevel - 1][i],
                                          s_pstScenePipeParam[u8Index].stDynamicDrc.au32RatioLevel[u32RatioLevel],
                                          s_pstScenePipeParam[u8Index].stDynamicDrc.au16TMValue[u32RatioLevel][i]);
            }
        }
        if (u32LastRatio < u32Ratio) {
            u32Sign = u32Ratio - u32LastRatio;
        } else {
            u32Sign = u32LastRatio - u32Ratio;
        }
        if (u32Sign < 32) {
            for (i = 0; i < 200; i++) {
                stIspDrcAttr.au16ToneMappingValue[i]  = au16ToneMappingValue[i];
            }
            PDT_CHECK_SCENE_PAUSE();
            s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stIspDrcAttr);
            PDT_CHECK_SCENE_RET(s32Ret);
        } else {
            for (j = 0; j < 5; j++) {
                for (i = 0; i < 200; i++) {
                    stIspDrcAttr.au16ToneMappingValue[i] = PDT_SCENE_TimeFilter(stIspDrcAttr.au16ToneMappingValue[i],
                                                           au16ToneMappingValue[i], 5, j);
                }
                PDT_CHECK_SCENE_PAUSE();
                s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stIspDrcAttr);
                PDT_CHECK_SCENE_RET(s32Ret);
                HI_usleep(30000);
            }
        }
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetDynamicGamma(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)
{
#ifdef CONFIG_SCENEAUTO_SUPPORT
    HI_U32 i, j = 0;
    HI_U32 u32ExpLevel = 0;
    static HI_U32 u32LastExpLevel = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_CHECK_SCENE_Index(u8Index);
    if (!s_pstScenePipeParam[u8Index].stDynamicGamma.bUsed) {
        return HI_SUCCESS;
    }
    ISP_GAMMA_ATTR_S stIspGammaAttr;
    if (s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum < 1 ||
            s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum > HI_PDT_SCENE_GAMMA_EXPOSURE_MAX_COUNT) {
        return HI_FAILURE;
    }
    u32ExpLevel = PDT_SCENE_GetLevelLtoH(u64Exposure, s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum,
                                         s_pstScenePipeParam[u8Index].stDynamicGamma.au64ExpThreshLtoH);
    if (u32ExpLevel < 0 || u32ExpLevel >= s_pstScenePipeParam[u8Index].stDynamicGamma.u32TotalNum) {
        return HI_FAILURE;
    }
    if (u32ExpLevel != u32LastExpLevel || u64LastExposure == 0) {
        for (i = 0; i < s_pstScenePipeParam[u8Index].stDynamicGamma.u32InterVal; i++) {
            PDT_CHECK_SCENE_PAUSE();
            s32Ret = HI_MPI_ISP_GetGammaAttr(ViPipe, &stIspGammaAttr);
            PDT_CHECK_SCENE_RET(s32Ret);
            for (j = 0; j < GAMMA_NODE_NUM; j++) {
                stIspGammaAttr.u16Table[j] = PDT_SCENE_TimeFilter(
                                             s_pstScenePipeParam[u8Index].stDynamicGamma.au16Table[u32LastExpLevel][j],
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

HI_S32 PDT_SCENE_SetPipeParam(HI_PDT_SCENE_PIPE_PARAM_S *pstScenePipeParam, HI_U32 u32Num)
{
    if (HI_NULL == pstScenePipeParam) {
        MLOGE("null pointer.\n");
        return HI_FAILURE;
    }
    if (u32Num > PDT_SCENE_PIPETYPE_CNT) {
        MLOGE("error input.\n");
        return HI_FAILURE;
    }
    s_pstScenePipeParam = pstScenePipeParam;
    return HI_SUCCESS;
}

HI_S32 PDT_SCENE_SetPause(HI_BOOL bPause)
{
    s_bISPPause = bPause;
    return HI_SUCCESS;
}

HI_BOOL PDT_SCENE_GetPauseState(HI_VOID)
{
    return s_bISPPause;
}

HI_PDT_SCENE_PIPE_PARAM_S *PDT_SCENE_GetParam(HI_VOID)
{
    return s_pstScenePipeParam;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
