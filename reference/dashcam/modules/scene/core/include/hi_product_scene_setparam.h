#ifndef _HI_PDT_SCENE_SETPARAM_H_
#define _HI_PDT_SCENE_SETPARAM_H_

#include "hi_common.h"
#include "hi_comm_dis.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vpss.h"
#include "hi_comm_venc.h"
#include "hi_product_scene_specparam.h"

#ifdef SUPPORT_PQ_HDRMODE
#include "mpi_hdr.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT          (16)
#define HI_PDT_SCENE_GAMMA_EXPOSURE_MAX_COUNT       (10)
#define HI_PDT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT      (10)
#define HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT     (10)
#define HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT    (10)
#define HI_PDT_SCENE_DRC_ISO_MAX_COUNT               (7)
#define HI_SCENE_DRC_RATIO_MAX_COUNT                 (5)
#define HI_PDT_SCENE_CLUT_ISO_MAX_COUNT              (5)
#define HI_PDT_SCENE_FSWDR_EXPOSURE_MAX_COUNT       (3)
#define HI_PDT_SCENE_BAYER_CHN_NUM                   (4)
#define HI_PDT_SCENE_AWB_CURVE_PARA_NUM              (6)
#define HI_PDT_SCENE_AWB_ZONE_ORIG_ROW               (32)
#define HI_PDT_SCENE_AWB_ZONE_ORIG_COLUMN            (32)

typedef struct hiPDT_SCENE_STATIC_AE_S
{
    HI_BOOL bAERouteExValid;
    HI_BOOL bAEGainSepCfg;
    HI_U8  u8PriorFrame;
    HI_U8  u8AERunInterval;
    HI_U32 u32MaxTime;
    HI_U32 u32AutoISPDGainMax;
    HI_U32 u32AutoISPDGainMin;
    HI_U32 u32AutoDGainMax;
    HI_U32 u32AutoDGainMin;
    HI_U32 u32AutoSysGainMax;
    HI_U8 u8AutoSpeed;
    HI_U8 u8AutoTolerance;
	HI_U16 u16WhiteDelayFrame;
	HI_U16 u16BlackDelayFrame;
    HI_U8 u8ChangeFpsEnable;
} HI_PDT_SCENE_STATIC_AE_S;

typedef struct hiPDT_SCENE_STATIC_DRC_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U8 u8CurveSelect;
    HI_U8 u8DRCOpType;;
    HI_U16 au16ToneMappingValue[HI_ISP_DRC_TM_NODE_NUM];
} HI_PDT_SCENE_STATIC_DRC_S;

typedef struct hiPDT_SCENE_STATIC_DEHAZE_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_BOOL bUserLutEnable;
    HI_U8   u8DehazeOpType;
    HI_U8   au8DehazeLut[256];
} HI_PDT_SCENE_STATIC_DEHAZE_S;

typedef struct hiSCENE_STATIC_AEROUTEEX_S
{
    HI_U32 u32TotalNum;
    HI_U32 au32IntTime[ISP_AE_ROUTE_EX_MAX_NODES];
    HI_U32 au32Again[ISP_AE_ROUTE_EX_MAX_NODES];
    HI_U32 au32Dgain[ISP_AE_ROUTE_EX_MAX_NODES];
    HI_U32 au32IspDgain[ISP_AE_ROUTE_EX_MAX_NODES];
} HI_PDT_SCENE_STATIC_AEROUTEEX_S;

typedef struct hiSCENE_STATIC_AEROUTE_S
{
    HI_U32 u32TotalNum;
    HI_U32 au32IntTime[ISP_AE_ROUTE_MAX_NODES];
    HI_U32 au32SysGain[ISP_AE_ROUTE_MAX_NODES];
} HI_PDT_SCENE_STATIC_AEROUTE_S;

typedef struct hiPDT_SCENE_STATIC_CROSSTALK_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
} HI_PDT_SCENE_STATIC_CROSSTALK_S;

typedef struct hiPDT_SCENE_STATIC_CA_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
} HI_PDT_SCENE_STATIC_CA_S;

typedef struct hiPDT_SCENE_STATIC_LDCI_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U8 u8LDCIOpType;
    HI_U8 u8GaussLPFSigma;
    HI_U8 au8AutoHePosWgt[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au8AutoHePosSigma[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au8AutoHePosMean[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au8AutoHeNegWgt[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au8AutoHeNegSigma[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au8AutoHeNegMean[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8 au16AutoBlcCtrl[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_LDCI_S;

typedef struct hiPDT_SCENE_STATIC_SHARPEN_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U8  au8LumaWgt[ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16AutoTextureStr[ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16AutoEdgeStr[ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8AutoOverShoot[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8AutoUnderShoot[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8ShootSupStr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8ShootSupAdj[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16AutoTextureFreq[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16AutoEdgeFreq[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8AutoDetailCtrl[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8DetailCtrlThr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8EdgeFiltStr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8RGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8BGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8SkinGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16MaxSharpGain[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_SHARPEN_S;

typedef struct hiPDT_SCENE_STATIC_DEMOSAIC_ATTR_S
{
    HI_BOOL bUsed;
    HI_U8  au8NonDirStr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8NonDirMFDetailEhcStr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16DetailSmoothStr[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_DEMOSAIC_S;

typedef struct hiPDT_SCENE_STATIC_AWB_S
{
    HI_U16 au16StaticWB[HI_PDT_SCENE_BAYER_CHN_NUM];
    HI_S32 as32CurvePara[HI_PDT_SCENE_AWB_CURVE_PARA_NUM];
    HI_U16 u16Speed;
    HI_U16 u16LowColorTemp;
    HI_U16 u16LowStart;
    HI_U16 u16LowStop;
    HI_U32 u32OutThresh;
    HI_U8  u8OutShiftLimit;
    HI_U16 u16HighStart;
    HI_U16 u16HighStop;
    HI_U8  u8Tolerance;
    HI_U8  u8ShiftLimit;
    HI_BOOL bFineTunEn;
    HI_BOOL bAWBZoneWtEn;
    HI_U8   au8ZoneWt[HI_PDT_SCENE_AWB_ZONE_ORIG_ROW*HI_PDT_SCENE_AWB_ZONE_ORIG_COLUMN];
} HI_PDT_SCENE_STATIC_AWB_S;

typedef struct hiPDT_SCENE_STATIC_CLUT_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U32  u32GainR;
    HI_U32  u32GainG;
    HI_U32  u32GainB;
} HI_PDT_SCENE_STATIC_CLUT_S;

typedef struct hiPDT_SCENE_STATIC_STATISTICSCFG_S
{
    HI_U8 au8AEWeight[AE_ZONE_ROW][AE_ZONE_COLUMN];
    HI_BOOL bAFEnable;
} HI_PDT_SCENE_STATIC_STATISTICSCFG_S;


typedef struct hiPDT_SCENE_STATIC_NR_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U16 au16CoringRatio[HI_ISP_BAYERNR_LUT_LENGTH];
    HI_U16 au16CoringWgt[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8FineStr[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_NR_S;

typedef struct hiPDT_SCENE_STATIC_CAC_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U8  au8DePurpleCrStr[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8DePurpleCbStr[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_CAC_S;

typedef struct hiPDT_SCENE_STATIC_SHADING_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
} HI_PDT_SCENE_STATIC_SHADING_S;

typedef struct hiPDT_SCENE_STATIC_WDREXPOSURE_S
{
    HI_BOOL bUsed;
    HI_U8 u8ExpRatioType;
    HI_U16 u16Tolerance;
    HI_U16 u16Speed;
    HI_U32 u32ExpRatioMax;
    HI_U32 u32ExpRatioMin;
    HI_U32 au32ExpRatio[EXP_RATIO_NUM];
} HI_PDT_SCENE_STATIC_WDREXPOSURE_S;

typedef struct hiPDT_SCENE_STATIC_FSWDR_S
{
    HI_BOOL bUsed;
    HI_U8 WDRMergeMode;
} HI_PDT_SCENE_STATIC_FSWDR_S;

typedef struct hiPDT_SCENE_STATIC_SATURATION_S
{
    HI_BOOL bUsed;
    HI_U8   au8Sat[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_SATURATION_S;

typedef struct hiPDT_SCENE_STATIC_CCM_S
{
    HI_BOOL bUsed;
    HI_U32 u32TotalNum;
    HI_U16 au16AutoColorTemp[CCM_MATRIX_NUM];
    HI_U16 au16AutoCCM[CCM_MATRIX_NUM][CCM_MATRIX_SIZE];
} HI_PDT_SCENE_STATIC_CCM_S;

typedef struct hiPDT_SCENE_STATIC_DE_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U16  au16GlobalGain[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16  au16GainLF[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16  au16GainHF[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_DE_S;

typedef struct hiPDT_SCENE_STATIC_DP_S
{
    HI_BOOL bUsed;
    HI_BOOL bEnable;
    HI_U16  au16Strength[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16  au16BlendRatio[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_STATIC_DP_S;

typedef struct hiPDT_SCENE_STATIC_HIERARCHICAL_QP_S
{
    HI_BOOL bUsed;
    HI_BOOL bHierarchicalQpEn;
    HI_S32  as32HierarchicalQpDelta[4];
    HI_S32  as32HierarchicalFrameNum[4];
}HI_PDT_SCENE_STATIC_QP_S;

typedef struct hiPDT_SCENE_DYNAMIC_GAMMA_S
{
    HI_BOOL bUsed;
    HI_U32 u32InterVal;
    HI_U32 u32TotalNum;
    HI_U64 au64ExpThreshLtoH[HI_PDT_SCENE_GAMMA_EXPOSURE_MAX_COUNT];
    HI_U16 au16Table[HI_PDT_SCENE_GAMMA_EXPOSURE_MAX_COUNT][GAMMA_NODE_NUM];
} HI_PDT_SCENE_DYNAMIC_GAMMA_S;

typedef struct hiPDT_SCENE_DYNAMIC_AE_S
{
    HI_BOOL bUsed;
    HI_U8  u8AEExposureCnt;
    HI_U64 au64ExpLtoHThresh[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
    HI_U8  au8AutoCompensation[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
    HI_U8  au8AutoMaxHistOffset[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
    HI_U8  au8AntiFlickerEn[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_AE_S;

typedef struct hiPDT_SCENE_DYNAMIC_WDREXPOSURE_S
{
    HI_BOOL bUsed;
    HI_U8  u8ExposureCnt;
    HI_U64 au64ExpLtoHThresh[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
    HI_U32 au32ExpRatioMax[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
    HI_U32 au32ExpRatioMin[HI_PDT_SCENE_AE_EXPOSURE_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_WDREXPOSURE_S;

typedef struct hiPDT_SCENE_DYNAMIC_DEHAZE_S
{
    HI_BOOL bUsed;
    HI_U32 u32ExpThreshCnt;
    HI_U64 au64ExpThreshLtoH[HI_PDT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];
    HI_U8  au8ManualStrength[HI_PDT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_DEHAZE_S;

typedef struct hiPDT_SCENE_DYNAMIC_NR_S
{
    HI_BOOL bUsed;
    HI_U16 u16ISOCount;
    HI_U32 au32ISO[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8FrameStrShort[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U8  au8FrameStrLong[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_DYNAMIC_NR_S;

typedef struct hiPDT_SCENE_DYNAMIC_SHADING_S
{
    HI_BOOL bUsed;
    HI_U16 u16ExpCount;
    HI_U32 au32ExpThreshLtoH[HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
    HI_U16 au16MeshStr[HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
    HI_U16 au16BlendRatio[HI_PDT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_SHADING_S;

typedef struct hiPDT_SCENE_DYNAMIC_FSWDR_S
{
    HI_BOOL bUsed;
    HI_U32 u32ExposureCnt;
    HI_U32 u32ISO;
    HI_U8  au8WDRMergeMode[HI_PDT_SCENE_FSWDR_EXPOSURE_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_FSWDR_S;

typedef struct hiPDT_SCENE_DYNAMIC_CLUT_S
{
    HI_BOOL bUsed;
    HI_U32 u32ISOCount;
    HI_U32 au32ISOLevel[HI_PDT_SCENE_CLUT_ISO_MAX_COUNT];
    HI_U32 au32GainR[HI_PDT_SCENE_CLUT_ISO_MAX_COUNT];
    HI_U32 au32GainG[HI_PDT_SCENE_CLUT_ISO_MAX_COUNT];
    HI_U32 au32GainB[HI_PDT_SCENE_CLUT_ISO_MAX_COUNT];
} HI_PDT_SCENE_DYNAMIC_CLUT_S;

typedef struct hiPDT_SCENE_DYNAMIC_DRC_S
{
    HI_BOOL bUsed;
    HI_U32 u32ISOCount;
    HI_U32 au32ISOLevel[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U32 u32Interval;
    HI_BOOL bEnable;
    HI_U8  au8LocalMixingBrightMax[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8LocalMixingBrightMin[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8LocalMixingDarkMax[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8LocalMixingDarkMin[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8BrightGainLmt[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8BrightGainLmtStep[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8DarkGainLmtY[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8DarkGainLmtC[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_S8  as8DetailAdjustFactor[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8SpatialFltCoef[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8RangeFltCoef[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8GradRevMax[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8GradRevThr[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8Asymmetry[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8SecondPole[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8Compress[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U8  au8Stretch[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U16 u16Strength[HI_PDT_SCENE_DRC_ISO_MAX_COUNT];
    HI_U32 u32RatioCount;
    HI_U32 au32RatioLevel[HI_SCENE_DRC_RATIO_MAX_COUNT];
    HI_U16 au16TMValue[HI_SCENE_DRC_RATIO_MAX_COUNT][HI_ISP_DRC_TM_NODE_NUM];
} HI_PDT_SCENE_DYNAMIC_DRC_S;

typedef struct hiPDT_SCENE_DYNAMIC_CAC_S
{
    HI_BOOL bUsed;
    HI_U16  au32ISOCount;
    HI_U32  au32ISOLevel[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16  u16PurpleDetRange[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16  u16VarThr[ISP_AUTO_ISO_STRENGTH_NUM];
} HI_PDT_SCENE_DYNAMIC_CAC_S;

typedef struct hiPDT_SCENE_DYNAMIC_QVBRVENC_S
{
    HI_BOOL bUsed;
    HI_U16 u16ExpCount;
    HI_U64 au64ExpThreshLtoH[HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT];
    HI_U32 au32MaxQp[HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT];
    HI_U32 au32MinQp[HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT];
    HI_U32 au32MinIQp[HI_PDT_SCENE_QVBRVENC_EXPOSURE_MAX_COUNT];
}HI_PDT_SCENE_DYNAMIC_QVBRVENC_S;

typedef struct hiPDT_SCENE_DYNAMIC_BLACKLEVEL_S
{
    HI_BOOL bUsed;
    HI_U32 au32ISOCount;
    HI_U32 au32ISOLevel[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16BlackLevelRR[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16BlackLevelGR[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16BlackLevelGB[ISP_AUTO_ISO_STRENGTH_NUM];
    HI_U16 au16BlackLevelBB[ISP_AUTO_ISO_STRENGTH_NUM];
}HI_PDT_SCENE_DYNAMIC_BLACKLEVEL_S;

typedef struct hiPDT_SCENE_PIPE_PARAM_S
{
    HI_PDT_SCENE_STATIC_AE_S            stStaticAe;
    HI_PDT_SCENE_STATIC_AEROUTEEX_S     stStaticAeRouteEx;
    HI_PDT_SCENE_STATIC_AEROUTE_S       stStaticAeRoute;
    HI_PDT_SCENE_STATIC_AEROUTE_S       stStaticSFAeRoute;
    HI_PDT_SCENE_STATIC_AWB_S           stStaticAWB;
    HI_PDT_SCENE_STATIC_SATURATION_S    stStaticSaturation;
    HI_PDT_SCENE_STATIC_CCM_S           stStaticCCM;
    HI_PDT_SCENE_STATIC_LDCI_S          stStaticLdci;
    HI_PDT_SCENE_STATIC_STATISTICSCFG_S stStaticStatistics;
    HI_PDT_SCENE_STATIC_3DNR_S          stStatic3DNR;
    HI_PDT_SCENE_STATIC_NR_S            stStaticNr;
    HI_PDT_SCENE_STATIC_CAC_S           stStaticCAC;
    HI_PDT_SCENE_STATIC_CLUT_S          stStaticClut;
    HI_PDT_SCENE_STATIC_CA_S            stStaticCa;
    HI_PDT_SCENE_STATIC_CROSSTALK_S     stStaticCrossTalk;
    HI_PDT_SCENE_STATIC_SHADING_S       stStaticShading;
    HI_PDT_SCENE_STATIC_DRC_S           stStaticDrc;
    HI_PDT_SCENE_STATIC_WDREXPOSURE_S   stStaticWdrExposure;
    HI_PDT_SCENE_STATIC_FSWDR_S         stStaticFsWdr;
    HI_PDT_SCENE_STATIC_SHARPEN_S       stStaticSharpen;
    HI_PDT_SCENE_STATIC_DEMOSAIC_S      stStaticDemosaic;
    HI_PDT_SCENE_STATIC_DEHAZE_S        stStaticDehaze;
    HI_PDT_SCENE_STATIC_DE_S            stStaticDE;
    HI_PDT_SCENE_STATIC_DP_S            stStaticDPC;
    HI_PDT_SCENE_STATIC_QP_S            stStaticQP;
    HI_PDT_SCENE_DYNAMIC_AE_S           stDynamicAe;
    HI_PDT_SCENE_DYNAMIC_WDREXPOSURE_S  stDynamicWdrExposure;
    HI_PDT_SCENE_DYNAMIC_DEHAZE_S       stDynamicDehaze;
    HI_PDT_SCENE_DYNAMIC_GAMMA_S        stDynamicGamma;
    HI_PDT_SCENE_DYNAMIC_FSWDR_S        stDynamicFsWdr;
    HI_PDT_SCENE_DYNAMIC_CLUT_S         stDynamicClut;
    HI_PDT_SCENE_DYNAMIC_DRC_S          stDynamicDrc;
    HI_PDT_SCENE_DYNAMIC_NR_S           stDynamicNR;
    HI_PDT_SCENE_DYNAMIC_SHADING_S      stDynamicShading;
    HI_PDT_SCENE_DYNAMIC_QVBRVENC_S     stDynamicQVBRVENC;
    HI_PDT_SCENE_DYNAMIC_CAC_S          stDynamicCAC;
    HI_PDT_SCENE_DYNAMIC_BLACKLEVEL_S   stDynamicBlackLevel;
} HI_PDT_SCENE_PIPE_PARAM_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif