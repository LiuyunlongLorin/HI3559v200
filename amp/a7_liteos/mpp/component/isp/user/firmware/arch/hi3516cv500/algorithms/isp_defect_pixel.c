/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_defect_pixel.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2013/01/16
    Author      :
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include "isp_sensor.h"
#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define ISP_DPCC_MODE              (35)
#define ISP_DPCC_HOT_MODE          (7)
#define ISP_DPCC_DEAD_MODE         (71)
#define ISP_DPCC_HIGHLIGHT_MODE    (160)
#define ISP_HOT_DEV_THRESH         (20)
#define ISP_DEAD_DEV_THRESH        (15)

#define ISP_DPCC_PAR_MAX_COUNT     (9)//(6)
#define ISP_DPC_SLOPE_GRADE        (5)
#define ISP_DPC_SOFT_SLOPE_GRADE   (5)

static const HI_U16 g_au16DpcStrength[ISP_AUTO_ISO_STRENGTH_NUM]   = {0, 0, 0, 152, 200, 200, 220, 220, 220, 220, 152, 152, 152, 152, 152, 152};
static const HI_U16 g_au16DpcBlendRatio[ISP_AUTO_ISO_STRENGTH_NUM] = {0, 0, 0,  0,  0,  0,  0,  0,  0,  0, 50, 50, 50, 50, 50, 50};
static const HI_U16 g_au16DpcLineThr1[27]       =   {0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0707, 0x0707, 0x0707, 0x0606, 0x0606, 0x0505, 0x0505, 0x0404, 0x0404, 0x0303, 0x0303, 0x0202, 0x0202, 0x0202, 0x0202};
static const HI_U16 g_au16DpcLineMadFac1[27]    =   {0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303};
static const HI_U16 g_au16DpcPgFac1[27]         =   {0x0404, 0x0404, 0x0404, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303, 0x0303};
static const HI_U16 g_au16DpcRndThr1[27]        =   {0x0a0a, 0x0a0a, 0x0a0a, 0x0a0a, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0808, 0x0707, 0x0606, 0x0505, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404, 0x0404};
static const HI_U16 g_au16DpcRgFac1[27]         =   {0x1f1f, 0x1e1e, 0x1d1d, 0x1d1d, 0x1d1d, 0x1b1b, 0x1919, 0x1717, 0x1515, 0x1313, 0x1111, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x0d0d, 0x0c0c, 0x0a0a, 0x0a0a, 0x0a0a, 0x0808, 0x0808, 0x0808, 0x0606, 0x0404, 0x0202};
static const HI_U16 g_au16DpcRoLimits1[27]      =   {0x0dfa, 0x0dfa, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0efe, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff};
static const HI_U16 g_au16DpcRndOffs1[27]       =   {0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff};
static const HI_U8  g_au8SlopeGrade[5]          =   {0, 76, 99, 100, 127};

static const HI_U16 g_au16SoftLineThr[5]     = {0x5454, 0x1818, 0x1212, 0x0a0a, 0x0a0a};
static const HI_U16 g_au16SoftLineMadFac[5]  = {0x1810, 0x1810, 0x1810, 0x1010, 0x0a0a};
static const HI_U8  g_au8SoftSlopeGrade[5]   = {0, 76, 100, 115, 120};

typedef struct hiISP_DEFECT_PIXEL_S
{
    /* Public */
    HI_BOOL bEnable;                // enable dpc module
    HI_BOOL bStatEn;
    HI_U16  u16DpccMode;
    HI_U32  u32DpccBadThresh;
    //static calib
    HI_BOOL bStaCalibrationEn;      // enable static calibration
    HI_U8   u8PixelDetectType;       //0: hot pixel detect; 1: dead pixel detect;
    HI_U8   u8FrameCnt;
    HI_U8   u8StaticDPThresh;
    HI_U8   u8TrialCount;
    HI_U8   u8TrialCntLimit;
    HI_U8   u8CalibStarted;
    HI_U8   u8CalibFinished;
    HI_U8   u8HotDevThresh;
    HI_U8   u8DeadDevThresh;
    HI_U16  u16DpCountMax;
    HI_U16  u16DpCountMin;
    HI_U16  u16BptCalibNum;
    HI_U16  au16BlkBpCalibNum[ISP_STRIPING_MAX_NUM];
    HI_U32  *pau32BptCalibTable[ISP_STRIPING_MAX_NUM];//for calibration,max size: ISP_STRIPING_MAX_NUM * ISP_DPC_MAX_BPT_NUM_NORMAL

    //static cor
    HI_BOOL bStaticEnable;
    HI_BOOL bStaicShow;
    HI_BOOL bStaticAttrUpdate;
    HI_U16  u16BptCorNum;
    HI_U16  au16Offset[ISP_STRIPING_MAX_NUM + 1];
    HI_U16  au16OffsetForSplit[ISP_STRIPING_MAX_NUM + 1];
    //dynamic cor
    HI_BOOL bDynamicEnable;
    HI_BOOL bDynamicManual;
    HI_BOOL bSupTwinkleEn;
    HI_BOOL bDynaAttrUpdateEn;
    HI_U16  u16BlendRatio;
    HI_U16  u16Strength;
    HI_S8   s8SupTwinkleThr;
    HI_U8   u8SupTwinkleSlope;

    ISP_CMOS_DPC_S stCmosDpc;
} ISP_DEFECT_PIXEL_S;

typedef struct
{
    HI_U8  u8DpccSetUse;
    HI_U16 u16DpccMethodsSet1;
    HI_U16 u16DpccMethodsSet2;
    HI_U16 u16DpccMethodsSet3;
    HI_U32 u32DpccBadThresh;
} ISP_DPC_CFG_S;

static const ISP_DPC_CFG_S g_stDpcDefCfg[ISP_DPCC_PAR_MAX_COUNT] =
{
    {0x01 , 0x1F1F, 0x0707, 0x1F1F, 0xff800080}, //0~75
    {0x03 , 0x1F1F, 0x0707, 0x1F1F, 0xff800080}, //ori set 1  (76)
    {0x03 , 0x1F1F, 0x0707, 0x1F1F, 0xff800080}, //ori set 2 (99)
    {0x07 , 0x1F1F, 0x0707, 0x1F1F, 0xff800080}, // set 23(RB set3, G set2) (100)
    {0x07 , 0x1F1F, 0x0707, 0x1F1F, 0xff800080}, //101 ~127
};

typedef struct
{
    HI_U8  au8DpccLineThr[2][3];
    HI_U8  au8DpccLineMadFac[2][3];
    HI_U8  au8DpccPgFac[2][3];
    HI_U8  au8DpccRndThr[2][3];
    HI_U8  au8DpccRgFac[2][3];
    HI_U8  au8DpccRo[2][3];
    HI_U8  au8DpccRndOffs[2][3];
} ISP_DPCC_DERIVED_PARAM_S;

static const ISP_DPCC_DERIVED_PARAM_S g_stDpcDerParam[5] =
{
    {{{0x54, 0x21, 0x20}, {0x54, 0x21, 0x20}}, {{0x1B, 0x18, 0x04}, {0x1B, 0x10, 0x04}}, {{0x08, 0x0B, 0x0A}, {0x08, 0x0B, 0x0A}}, {{0x0A, 0x08, 0x08}, {0x0A, 0x08, 0x06}}, {{0x26, 0x08, 0x04}, {0x26, 0x08, 0x04}}, {{0x1, 0x2, 0x2}, {0x1, 0x2, 0x1}}, {{0x2, 0x2, 0x2}, {0x2, 0x2, 0x2}}}, //0
    {{{0x08, 0x21, 0x20}, {0x08, 0x21, 0x20}}, {{0x1B, 0x18, 0x04}, {0x1B, 0x10, 0x04}}, {{0x08, 0x0B, 0x0A}, {0x08, 0x0B, 0x0A}}, {{0x0A, 0x08, 0x08}, {0x0A, 0x08, 0x06}}, {{0x26, 0x08, 0x04}, {0x26, 0x08, 0x04}}, {{0x1, 0x2, 0x2}, {0x1, 0x2, 0x1}}, {{0x2, 0x2, 0x2}, {0x2, 0x2, 0x2}}}, //76
    {{{0x08, 0x10, 0x20}, {0x08, 0x10, 0x20}}, {{0x04, 0x18, 0x04}, {0x04, 0x10, 0x04}}, {{0x08, 0x08, 0x0A}, {0x08, 0x06, 0x0A}}, {{0x0A, 0x08, 0x08}, {0x0A, 0x08, 0x06}}, {{0x20, 0x08, 0x04}, {0x20, 0x08, 0x04}}, {{0x2, 0x3, 0x2}, {0x2, 0x3, 0x1}}, {{0x3, 0x3, 0x3}, {0x3, 0x3, 0x3}}}, //99
    {{{0x08, 0x10, 0x20}, {0x08, 0x10, 0x20}}, {{0x04, 0x18, 0x04}, {0x04, 0x10, 0x04}}, {{0x08, 0x08, 0x0A}, {0x08, 0x06, 0x0A}}, {{0x0A, 0x08, 0x08}, {0x0A, 0x08, 0x06}}, {{0x20, 0x08, 0x04}, {0x20, 0x08, 0x04}}, {{0x2, 0x3, 0x2}, {0x2, 0x3, 0x1}}, {{0x3, 0x3, 0x3}, {0x3, 0x3, 0x3}}}, //100
    {{{0x01, 0x10, 0x20}, {0x01, 0x10, 0x20}}, {{0x03, 0x18, 0x04}, {0x03, 0x10, 0x04}}, {{0x03, 0x08, 0x0A}, {0x03, 0x06, 0x0A}}, {{0x04, 0x08, 0x08}, {0x04, 0x08, 0x06}}, {{0x08, 0x08, 0x04}, {0x08, 0x08, 0x04}}, {{0x2, 0x3, 0x2}, {0x2, 0x3, 0x1}}, {{0x3, 0x3, 0x3}, {0x3, 0x3, 0x3}}}, //127
};

ISP_DEFECT_PIXEL_S *g_pastDpCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define DP_GET_CTX(dev, pstCtx)   (pstCtx = g_pastDpCtx[dev])
#define DP_SET_CTX(dev, pstCtx)   (g_pastDpCtx[dev] = pstCtx)
#define DP_RESET_CTX(dev)         (g_pastDpCtx[dev] = HI_NULL)

static HI_S32 DpCtxInit(VI_PIPE ViPipe)
{
    ISP_DEFECT_PIXEL_S *pastDpCtx = HI_NULL;

    DP_GET_CTX(ViPipe, pastDpCtx);

    if (HI_NULL == pastDpCtx)
    {
        pastDpCtx = (ISP_DEFECT_PIXEL_S *)ISP_MALLOC(sizeof(ISP_DEFECT_PIXEL_S));
        if (HI_NULL == pastDpCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] DpCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pastDpCtx, 0, sizeof(ISP_DEFECT_PIXEL_S));

    DP_SET_CTX(ViPipe, pastDpCtx);

    return HI_SUCCESS;
}

static HI_VOID DpCtxExit(VI_PIPE ViPipe)
{
    ISP_DEFECT_PIXEL_S *pastDpCtx = HI_NULL;

    DP_GET_CTX(ViPipe, pastDpCtx);
    ISP_FREE(pastDpCtx);
    DP_RESET_CTX(ViPipe);
}

static HI_U8 DpGetChnNum(HI_U8 u8WDRMode)
{
    if (IS_LINEAR_MODE(u8WDRMode))
    {
        return 1;
    }
    else if (IS_BUILT_IN_WDR_MODE(u8WDRMode))
    {
        return 1;
    }
    else if (IS_2to1_WDR_MODE(u8WDRMode))
    {
        return 2;
    }
    else
    {
        /* unknow u8Mode */
        return 1;
    }
}

static HI_VOID Dpc_SafeFreeCalibLut(ISP_DEFECT_PIXEL_S *pstDp, HI_U8 u8Cnt)
{
    HI_U8 i;

    for (i = 0; i < u8Cnt; i++)
    {
        ISP_FREE(pstDp->pau32BptCalibTable[i]);
    }

    pstDp->u8CalibFinished = 0;
}
static HI_VOID ISP_DpEnableCfg(VI_PIPE ViPipe, HI_U8 u8CfgNum, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8     i, j, u8ChnNum;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8ChnNum = DpGetChnNum(pstIspCtx->u8SnsWDRMode);

    for (i = 0; i < u8CfgNum; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pstRegCfg->stAlgRegCfg[i].stDpRegCfg.abDpcEn[j] = (j < u8ChnNum) ? (HI_TRUE) : (HI_FALSE);
        }
    }
}

static HI_VOID DpStaticRegsInitialize(ISP_DPC_STATIC_CFG_S *pstStaticRegCfg)
{
    pstStaticRegCfg->u32DpccBptCtrl    = HI_ISP_DPC_DEFAULT_BPT_CTRL;
    pstStaticRegCfg->u8DpccOutputMode  = HI_ISP_DPC_DEFAULT_OUTPUT_MODE;
    pstStaticRegCfg->bStaticResh       = HI_TRUE;
}

static HI_VOID DpUsrRegsInitialize(ISP_DPC_USR_CFG_S *pstUsrRegCfg)
{
    pstUsrRegCfg->stUsrDynaCorRegCfg.bDpccHardThrEn         = HI_ISP_DPC_DEFAULT_HARD_THR_ENABLE;
    pstUsrRegCfg->stUsrDynaCorRegCfg.s8DpccSupTwinkleThrMax = HI_ISP_DPC_DEFAULT_SOFT_THR_MAX;
    pstUsrRegCfg->stUsrDynaCorRegCfg.s8DpccSupTwinkleThrMin = HI_ISP_DPC_DEFAULT_SOFT_THR_MIN;
    pstUsrRegCfg->stUsrDynaCorRegCfg.u16DpccRakeRatio       = HI_ISP_DPC_DEFAULT_SOFT_RAKE_RATIO;
    pstUsrRegCfg->stUsrDynaCorRegCfg.bResh                  = HI_TRUE;

    pstUsrRegCfg->stUsrStaCorRegCfg.u16DpccBptNumber        = HI_ISP_DPC_DEFAULT_BPT_NUMBER;
    pstUsrRegCfg->stUsrStaCorRegCfg.u32UpdateIndex          = 1;
    pstUsrRegCfg->stUsrStaCorRegCfg.u8BufId                 = 0;
    pstUsrRegCfg->stUsrStaCorRegCfg.bResh                   = HI_FALSE;
}

static HI_VOID DpDynaRegsInitialize(ISP_DPC_DYNA_CFG_S *pstDynaRegCfg)
{
    pstDynaRegCfg->bResh                 = HI_TRUE;
    pstDynaRegCfg->bDpcStatEn            = 0;
    pstDynaRegCfg->u32DpccAlpha          = HI_ISP_DPC_DEFAULT_ALPHA;
    pstDynaRegCfg->u16DpccMode           = HI_ISP_DPC_DEFAULT_MODE;
    pstDynaRegCfg->u8DpccSetUse         = HI_ISP_DPC_DEFAULT_SET_USE;
    pstDynaRegCfg->u16DpccMethodsSet1    = HI_ISP_DPC_DEFAULT_METHODS_SET_1;
    pstDynaRegCfg->u16DpccMethodsSet2    = HI_ISP_DPC_DEFAULT_METHODS_SET_2;
    pstDynaRegCfg->u16DpccMethodsSet3    = HI_ISP_DPC_DEFAULT_METHODS_SET_3;
    pstDynaRegCfg->au16DpccLineThr[0]    = HI_ISP_DPC_DEFAULT_LINE_THRESH_1;
    pstDynaRegCfg->au16DpccLineMadFac[0] = HI_ISP_DPC_DEFAULT_LINE_MAD_FAC_1;
    pstDynaRegCfg->au16DpccPgFac[0]      = HI_ISP_DPC_DEFAULT_PG_FAC_1;
    pstDynaRegCfg->au16DpccRndThr[0]     = HI_ISP_DPC_DEFAULT_RND_THRESH_1;
    pstDynaRegCfg->au16DpccRgFac[0]      = HI_ISP_DPC_DEFAULT_RG_FAC_1;
    pstDynaRegCfg->au16DpccLineThr[1]    = HI_ISP_DPC_DEFAULT_LINE_THRESH_2;
    pstDynaRegCfg->au16DpccLineMadFac[1] = HI_ISP_DPC_DEFAULT_LINE_MAD_FAC_2;
    pstDynaRegCfg->au16DpccPgFac[1]      = HI_ISP_DPC_DEFAULT_PG_FAC_2;
    pstDynaRegCfg->au16DpccRndThr[1]     = HI_ISP_DPC_DEFAULT_RND_THRESH_2;
    pstDynaRegCfg->au16DpccRgFac[1]      = HI_ISP_DPC_DEFAULT_RG_FAC_2;
    pstDynaRegCfg->au16DpccLineThr[2]    = HI_ISP_DPC_DEFAULT_LINE_THRESH_3;
    pstDynaRegCfg->au16DpccLineMadFac[2] = HI_ISP_DPC_DEFAULT_LINE_MAD_FAC_3;
    pstDynaRegCfg->au16DpccPgFac[2]      = HI_ISP_DPC_DEFAULT_PG_FAC_3;
    pstDynaRegCfg->au16DpccRndThr[2]     = HI_ISP_DPC_DEFAULT_RND_THRESH_3;
    pstDynaRegCfg->au16DpccRgFac[2]      = HI_ISP_DPC_DEFAULT_RG_FAC_3;
    pstDynaRegCfg->u16DpccRoLimits       = HI_ISP_DPC_DEFAULT_RO_LIMITS;
    pstDynaRegCfg->u16DpccRndOffs        = HI_ISP_DPC_DEFAULT_RND_OFFS;
    pstDynaRegCfg->u32DpccBadThresh      = HI_ISP_DPC_DEFAULT_BPT_THRESH;

    pstDynaRegCfg->au8DpccLineStdThr[0]  = HI_ISP_DPC_DEFAULT_LINE_STD_THR_1;
    pstDynaRegCfg->au8DpccLineStdThr[1]  = HI_ISP_DPC_DEFAULT_LINE_STD_THR_2;
    pstDynaRegCfg->au8DpccLineStdThr[2]  = HI_ISP_DPC_DEFAULT_LINE_STD_THR_3;
    pstDynaRegCfg->au8DpccLineStdThr[3]  = HI_ISP_DPC_DEFAULT_LINE_STD_THR_4;
    pstDynaRegCfg->au8DpccLineStdThr[4]  = HI_ISP_DPC_DEFAULT_LINE_STD_THR_5;


    pstDynaRegCfg->au8DpccLineDiffThr[0] = HI_ISP_DPC_DEFAULT_LINE_DIFF_THR_1;
    pstDynaRegCfg->au8DpccLineDiffThr[1] = HI_ISP_DPC_DEFAULT_LINE_DIFF_THR_2;
    pstDynaRegCfg->au8DpccLineDiffThr[2] = HI_ISP_DPC_DEFAULT_LINE_DIFF_THR_3;
    pstDynaRegCfg->au8DpccLineDiffThr[3] = HI_ISP_DPC_DEFAULT_LINE_DIFF_THR_4;
    pstDynaRegCfg->au8DpccLineDiffThr[4] = HI_ISP_DPC_DEFAULT_LINE_DIFF_THR_5;

    pstDynaRegCfg->au8DpccLineAverFac[0] = HI_ISP_DPC_DEFAULT_LINE_AVER_FAC_1;
    pstDynaRegCfg->au8DpccLineAverFac[1] = HI_ISP_DPC_DEFAULT_LINE_AVER_FAC_2;
    pstDynaRegCfg->au8DpccLineAverFac[2] = HI_ISP_DPC_DEFAULT_LINE_AVER_FAC_3;
    pstDynaRegCfg->au8DpccLineAverFac[3] = HI_ISP_DPC_DEFAULT_LINE_AVER_FAC_4;
    pstDynaRegCfg->au8DpccLineAverFac[4] = HI_ISP_DPC_DEFAULT_LINE_AVER_FAC_5;

    pstDynaRegCfg->u8DpccLineKerdiffFac  = HI_ISP_DPC_DEFAULT_LINE_KERDIFF_FAC;
    pstDynaRegCfg->u8DpccBlendMode       = HI_ISP_DPC_DEFAULT_BLEND_MODE;
    pstDynaRegCfg->u8DpccBitDepthSel     = HI_ISP_DPC_DEFAULT_BIT_DEPTH_SEL;
}

static HI_VOID DpRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8     i;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        DpStaticRegsInitialize(&pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stStaticRegCfg);
        DpDynaRegsInitialize(&pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg);
        DpUsrRegsInitialize(&pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stUsrRegCfg);

        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.bLut2SttEn = HI_TRUE;
    }

    ISP_DpEnableCfg(ViPipe, pstRegCfg->u8CfgNum, pstRegCfg);

    pstRegCfg->unKey.bit1DpCfg = 1;
}

static HI_VOID DpExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_DEFECT_PIXEL_S *pstDp     = HI_NULL;

    DP_GET_CTX(ViPipe,  pstDp);

    //dynamic attr
    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        hi_ext_system_dpc_dynamic_strength_table_write(ViPipe, i, pstDp->stCmosDpc.au16Strength[i]);
        hi_ext_system_dpc_dynamic_blend_ratio_table_write(ViPipe, i, pstDp->stCmosDpc.au16BlendRatio[i]);
    }

    hi_ext_system_dpc_dynamic_cor_enable_write(ViPipe, pstDp->bEnable);
    hi_ext_system_dpc_manual_mode_write(ViPipe, HI_EXT_SYSTEM_DPC_MANU_MODE_DEFAULT);
    hi_ext_system_dpc_dynamic_manual_enable_write(ViPipe, HI_EXT_SYSTEM_DPC_DYNAMIC_MANUAL_ENABLE_DEFAULT);
    hi_ext_system_dpc_dynamic_strength_write(ViPipe, HI_EXT_SYSTEM_DPC_DYNAMIC_STRENGTH_DEFAULT);
    hi_ext_system_dpc_dynamic_blend_ratio_write(ViPipe, HI_EXT_SYSTEM_DPC_DYNAMIC_BLEND_RATIO_DEFAULT);
    hi_ext_system_dpc_suppress_twinkle_enable_write(ViPipe, HI_EXT_SYSTEM_DPC_SUPPRESS_TWINKLE_ENABLE_DEFAULT);
    hi_ext_system_dpc_suppress_twinkle_thr_write(ViPipe, HI_EXT_SYSTEM_DPC_SUPPRESS_TWINKLE_THR_DEFAULT);
    hi_ext_system_dpc_suppress_twinkle_slope_write(ViPipe, HI_EXT_SYSTEM_DPC_SUPPRESS_TWINKLE_SLOPE_DEFAULT);
    hi_ext_system_dpc_dynamic_attr_update_write(ViPipe, HI_TRUE);

    //static calib
    hi_ext_system_dpc_static_calib_enable_write(ViPipe, HI_EXT_SYSTEM_DPC_STATIC_CALIB_ENABLE_DEFAULT);
    hi_ext_system_dpc_count_max_write(ViPipe, HI_EXT_SYSTEM_DPC_COUNT_MAX_DEFAULT);
    hi_ext_system_dpc_count_min_write(ViPipe, HI_EXT_SYSTEM_DPC_COUNT_MIN_DEFAULT);
    hi_ext_system_dpc_start_thresh_write(ViPipe, HI_EXT_SYSTEM_DPC_START_THRESH_DEFAULT);
    hi_ext_system_dpc_trigger_status_write(ViPipe, HI_EXT_SYSTEM_DPC_TRIGGER_STATUS_DEFAULT);
    hi_ext_system_dpc_trigger_time_write(ViPipe, HI_EXT_SYSTEM_DPC_TRIGGER_TIME_DEFAULT);
    hi_ext_system_dpc_static_defect_type_write(ViPipe, HI_EXT_SYSTEM_DPC_STATIC_DEFECT_TYPE_DEFAULT);
    hi_ext_system_dpc_finish_thresh_write(ViPipe, HI_EXT_SYSTEM_DPC_START_THRESH_DEFAULT);
    hi_ext_system_dpc_bpt_calib_number_write(ViPipe, HI_EXT_SYSTEM_DPC_BPT_CALIB_NUMBER_DEFAULT);
    //static attr
    hi_ext_system_dpc_bpt_cor_number_write(ViPipe, HI_EXT_SYSTEM_DPC_BPT_COR_NUMBER_DEFAULT );
    hi_ext_system_dpc_static_cor_enable_write(ViPipe, pstDp->bEnable);
    hi_ext_system_dpc_static_dp_show_write(ViPipe, HI_EXT_SYSTEM_DPC_STATIC_DP_SHOW_DEFAULT);
    hi_ext_system_dpc_static_attr_update_write(ViPipe, HI_TRUE);

    //debug
    hi_ext_system_dpc_alpha0_rb_write(ViPipe, HI_EXT_SYSTEM_DPC_DYNAMIC_ALPHA0_RB_DEFAULT);
    hi_ext_system_dpc_alpha0_g_write(ViPipe, HI_EXT_SYSTEM_DPC_DYNAMIC_ALPHA0_G_DEFAULT);
}

static HI_VOID DpcImageSize(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp)
{
    HI_U8      i, u8BlockNum;
    ISP_RECT_S stBlockRect;
    ISP_CTX_S  *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8BlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

    for (i = 0; i < u8BlockNum; i++)
    {
        ISP_GetBlockRect(&stBlockRect, &pstIspCtx->stBlockAttr, i);

        pstDp->au16Offset[i] = stBlockRect.s32X;
        pstDp->au16OffsetForSplit[i] = (i == 0) ? 0 : (pstDp->au16Offset[i] + pstIspCtx->stBlockAttr.u32OverLap);
    }

    pstDp->au16OffsetForSplit[u8BlockNum] = pstIspCtx->stBlockAttr.stFrameRect.u32Width;
    pstDp->au16Offset[u8BlockNum]         = pstIspCtx->stBlockAttr.stFrameRect.u32Width;
}

static HI_S32 DpcCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_DPC_S *pstCmosDpc)
{
    HI_U8 i;

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        if (pstCmosDpc->au16Strength[i] > 255)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid au16Strength[%d]!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstCmosDpc->au16BlendRatio[i] > 0x80)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid au16BlendRatio[%d]!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_VOID DpcDefCalibParam(ISP_DEFECT_PIXEL_S  *pstDp)
{
    pstDp->u8TrialCount     = 0;
    pstDp->u8CalibStarted   = 0;
    pstDp->u8CalibFinished  = 0;
    pstDp->bStatEn          = 0;
    pstDp->u8HotDevThresh   = ISP_HOT_DEV_THRESH;
    pstDp->u8DeadDevThresh  = ISP_DEAD_DEV_THRESH;
    pstDp->u8FrameCnt       = 0;
    pstDp->u32DpccBadThresh = 0xff800080;
}

static HI_S32 DpInitialize(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_DEFECT_PIXEL_S  *pstDp      = HI_NULL;
    ISP_CMOS_DEFAULT_S  *pstSnsDft  = HI_NULL;

    DP_GET_CTX(ViPipe, pstDp);

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    pstDp->bEnable = HI_TRUE;

    if (pstSnsDft->unKey.bit1Dpc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstDpc);

        s32Ret = DpcCheckCmosParam(ViPipe, pstSnsDft->pstDpc);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        memcpy(&pstDp->stCmosDpc, pstSnsDft->pstDpc, sizeof(ISP_CMOS_DPC_S));
    }
    else
    {
        memcpy(pstDp->stCmosDpc.au16Strength,   g_au16DpcStrength,   ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U16));
        memcpy(pstDp->stCmosDpc.au16BlendRatio, g_au16DpcBlendRatio, ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U16));
    }

    DpcDefCalibParam(pstDp);

    DpcImageSize(ViPipe, pstDp);

    return HI_SUCCESS;
}

static HI_VOID DpEnter(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp)
{
    ISP_SensorSetPixelDetect(ViPipe, HI_TRUE);
    pstDp->u8StaticDPThresh = hi_ext_system_dpc_start_thresh_read(ViPipe);
    pstDp->u8CalibStarted   = 1;
}

static HI_VOID DpExit(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp, ISP_REG_CFG_S *pstRegCfg)
{
    DpcDefCalibParam(pstDp);
    ISP_SensorSetPixelDetect(ViPipe, HI_FALSE);
    pstDp->u8CalibStarted  = 0;
    pstDp->u8CalibFinished = 1;
}

static HI_VOID DpReadStaticCalibExtregs(VI_PIPE ViPipe)
{
    ISP_DEFECT_PIXEL_S *pstDp = HI_NULL;

    DP_GET_CTX(ViPipe, pstDp);

    pstDp->u8PixelDetectType = hi_ext_system_dpc_static_defect_type_read(ViPipe);
    pstDp->u8TrialCntLimit   = (HI_U8)(hi_ext_system_dpc_trigger_time_read(ViPipe) >> 3);
    pstDp->u16DpCountMax     = hi_ext_system_dpc_count_max_read(ViPipe);
    pstDp->u16DpCountMin     = hi_ext_system_dpc_count_min_read(ViPipe);
}

static HI_S32 Dpc_Read_Calib_Num(VI_PIPE ViPipe, HI_U8 u8BlkNum, ISP_DEFECT_PIXEL_S *pstDp, ISP_STAT_S *pStatInfo)
{
    HI_U8  i;
    HI_U16 u16BptCnt = 0, j;
    HI_U16 u16CntTemp = 0;
    HI_U16 au16BpCalibNum[ISP_STRIPING_MAX_NUM] = {0};
    HI_U32 u32BptValue;
    ISP_CTX_S *pstIspCtx  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        pstDp->u16BptCalibNum = isp_dpc_bpt_calib_number_read(ViPipe, 0);
    }
    else if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        pstDp->u16BptCalibNum = MIN2(pStatInfo->stDpStat.au16DefectPixelCount[0], STATIC_DP_COUNT_NORMAL);

        ISP_CHECK_POINTER(pstDp->pau32BptCalibTable[0]);

        for (j = 0; j < pstDp->u16BptCalibNum; j++)
        {
            pstDp->pau32BptCalibTable[0][j] = pStatInfo->stDpStat.au32DefectPixelLut[0][j];
        }
    }
    else if (IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        for (i = 0; i < u8BlkNum; i++)
        {
            ISP_CHECK_POINTER(pstDp->pau32BptCalibTable[i]);
            u16CntTemp = MIN2(pStatInfo->stDpStat.au16DefectPixelCount[i], STATIC_DP_COUNT_NORMAL);

            for (j = 0; j < u16CntTemp; j++)
            {
                u32BptValue = pStatInfo->stDpStat.au32DefectPixelLut[i][j] + pstDp->au16Offset[i];

                if ((u32BptValue & 0x1FFF) < pstDp->au16Offset[i + 1])
                {
                    pstDp->pau32BptCalibTable[i][au16BpCalibNum[i]++] = u32BptValue;
                    u16BptCnt++;
                }
            }

            pstDp->au16BlkBpCalibNum[i] = au16BpCalibNum[i];
        }

        pstDp->u16BptCalibNum = u16BptCnt;
    }

    return HI_SUCCESS;
}

static HI_VOID Dpc_Calib_TimeOut(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp)
{
    printf("BAD PIXEL CALIBRATION TIME OUT  0x%x\n", pstDp->u8TrialCntLimit);
    pstDp->bStaCalibrationEn = HI_FALSE;
    hi_ext_system_dpc_static_calib_enable_write(ViPipe, HI_FALSE);
    hi_ext_system_dpc_finish_thresh_write(ViPipe, pstDp->u8StaticDPThresh);
    hi_ext_system_dpc_trigger_status_write(ViPipe, 0x2);
}

static HI_VOID Dpc_Calib_Max(HI_U16 u16BadPixelsCount, ISP_DEFECT_PIXEL_S *pstDp)
{
    printf("BAD_PIXEL_COUNT_UPPER_LIMIT 0x%x, 0x%x\n", pstDp->u8StaticDPThresh, u16BadPixelsCount );
    pstDp->u8FrameCnt = 2;
    pstDp->u8TrialCount++;
}

static HI_VOID Dpc_Calib_Min(HI_U16 u16BadPixelsCount, ISP_DEFECT_PIXEL_S *pstDp)
{
    printf("BAD_PIXEL_COUNT_LOWER_LIMIT 0x%x, 0x%x\n", pstDp->u8StaticDPThresh, u16BadPixelsCount);
    pstDp->u8FrameCnt = 2;
    pstDp->u8TrialCount++;
}

static HI_S32 SortingDpCalibLut(HI_U32 *pu32Lut0, HI_U32 *pu32Lut1, HI_U16 u16Cnt0, HI_U16 u16Cnt1)
{
    HI_U16 i = 0;
    HI_U16 j = 0;
    HI_U16 u16CntSum = 0;
    HI_U32 *pu32TempLut = HI_NULL;

    pu32TempLut = (HI_U32 *)ISP_MALLOC((u16Cnt0 + u16Cnt1) * sizeof(HI_U32));

    if (HI_NULL == pu32TempLut)
    {
        return HI_FAILURE;
    }

    while ((i <  u16Cnt0) && (j < u16Cnt1))
    {
        if (pu32Lut0[i] > (pu32Lut1[j]))
        {
            pu32TempLut[u16CntSum++] = pu32Lut1[j++];
        }
        else if (pu32Lut0[i] < (pu32Lut1[j]))
        {
            pu32TempLut[u16CntSum++] = pu32Lut0[i++];
        }
        else
        {
            pu32TempLut[u16CntSum++] = pu32Lut0[i];
            i++;
            j++;
        }
    }

    if (i >=  u16Cnt0)
    {
        while (j < u16Cnt1)
        {
            pu32TempLut[u16CntSum++] = pu32Lut1[j++];
        }
    }

    if (j >=  u16Cnt1)
    {
        while (i < u16Cnt0)
        {
            pu32TempLut[u16CntSum++] = pu32Lut0[i++];
        }
    }

    memcpy(pu32Lut0, pu32TempLut, u16CntSum * sizeof(HI_U32));

    ISP_FREE(pu32TempLut);

    return u16CntSum;
}

static HI_S32 MergingDpCalibLut(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S  *pstDp, HI_U8 u8BlkNum)
{
    HI_U8  k;
    HI_U16 i = 0;
    HI_U16 u16BpNum = 0;
    HI_S32 s32CntTemp = 0;
    HI_U32 *pu32BpTable = HI_NULL;

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(pstDp->pau32BptCalibTable[k]);
    }

    pu32BpTable = (HI_U32 *)ISP_MALLOC(pstDp->u16BptCalibNum * sizeof(HI_U32));

    if (HI_NULL == pu32BpTable)
    {
        return HI_FAILURE;
    }

    memcpy(pu32BpTable, pstDp->pau32BptCalibTable[0], pstDp->au16BlkBpCalibNum[0] * sizeof(HI_U32));
    u16BpNum = pstDp->au16BlkBpCalibNum[0];

    for (k = 1; k < u8BlkNum; k++)
    {
        s32CntTemp = SortingDpCalibLut(pu32BpTable, pstDp->pau32BptCalibTable[k], u16BpNum, pstDp->au16BlkBpCalibNum[k]);

        if (s32CntTemp < 0)
        {
            ISP_FREE(pu32BpTable);
            return HI_FAILURE;
        }

        u16BpNum = s32CntTemp;
    }

    for ( i = 0; i < pstDp->u16BptCalibNum; i++)
    {
        hi_ext_system_dpc_calib_bpt_write(ViPipe, i, pu32BpTable[i]);
    }

    ISP_FREE(pu32BpTable);

    return HI_SUCCESS;
}

static HI_VOID Dpc_Calib_Success(VI_PIPE ViPipe, HI_U8 u8BlkNum, ISP_DEFECT_PIXEL_S *pstDp, ISP_STAT_S *pStatInfo)
{
    HI_U16 j;

    ISP_CTX_S *pstIspCtx  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    printf("trial: 0x%x, findshed: 0x%x\n", pstDp->u8TrialCount, pstDp->u16BptCalibNum);

    hi_ext_system_dpc_bpt_calib_number_write(ViPipe, pstDp->u16BptCalibNum);

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        isp_dpc_bpt_raddr_write(ViPipe, 0, 0);

        for ( j = 0; j < pstDp->u16BptCalibNum; j++)
        {
            hi_ext_system_dpc_calib_bpt_write(ViPipe, j, isp_dpc_bpt_rdata_read(ViPipe, 0));
        }
    }
    else if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        ISP_CHECK_POINTER_VOID(pstDp->pau32BptCalibTable[0]);
        for ( j = 0; j < pstDp->u16BptCalibNum; j++)
        {
            hi_ext_system_dpc_calib_bpt_write(ViPipe, j, pstDp->pau32BptCalibTable[0][j]);
        }
    }
    else
    {
        MergingDpCalibLut(ViPipe, pstDp, u8BlkNum);
    }

    pstDp->u16DpccMode = (ISP_DPCC_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);
    pstDp->bStatEn     = 0;
    pstDp->bStaCalibrationEn = HI_FALSE;
    hi_ext_system_dpc_static_calib_enable_write(ViPipe, HI_FALSE);
    hi_ext_system_dpc_finish_thresh_write(ViPipe, pstDp->u8StaticDPThresh);
    hi_ext_system_dpc_trigger_status_write(ViPipe, 0x1);
}

static HI_VOID SetReadDpStatisKey(VI_PIPE ViPipe, HI_BOOL bIsStart)
{
    HI_U32    u32IsrAccess;
    ISP_CTX_S *pstIspCtx   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
        IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
    {
        u32IsrAccess = hi_ext_system_statistics_ctrl_highbit_read(ViPipe);

        if (HI_TRUE == bIsStart)
        {
            u32IsrAccess |= (1 << DP_STAT_KEY_BIT);
        }
        else
        {
            u32IsrAccess &= (~(1 << DP_STAT_KEY_BIT));
        }

        hi_ext_system_statistics_ctrl_highbit_write(ViPipe, u32IsrAccess);
    }
}

static HI_VOID Dpc_Hot_Calib(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S  *pstDp, HI_U8 u8BlkNum, ISP_REG_CFG_S *pstRegCfg, ISP_STAT_S *pStatInfo)
{
    if (pstDp->u8FrameCnt < 9)
    {
        if (pstDp->u8FrameCnt == 0)
        {
            hi_ext_system_dpc_trigger_status_write(ViPipe, ISP_STATE_INIT);
            DpEnter(ViPipe, pstDp);
        }

        pstDp->u8FrameCnt++;

        if (pstDp->u8FrameCnt == 4)
        {
            pstDp->u32DpccBadThresh = (pstDp->u8StaticDPThresh << 24) + (((50 + 0x80 * pstDp->u8HotDevThresh) / 100) << 16) + 0x00000080;
            pstDp->u16DpccMode      = (ISP_DPCC_HOT_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);
            pstDp->bStatEn          = 1;

            SetReadDpStatisKey(ViPipe, HI_TRUE);
        }

        /*calibrate Frame 5*/
        if (pstDp->u8FrameCnt == 6)
        {
            pstDp->u16DpccMode = (ISP_DPCC_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);
            pstDp->bStatEn     = 0;
            Dpc_Read_Calib_Num(ViPipe, u8BlkNum, pstDp, pStatInfo);
        }

        if (pstDp->u8FrameCnt == 7)
        {
            //Dpc_Read_Calib_Num(ViPipe, u8BlkNum, pstDp, pStatInfo);
            SetReadDpStatisKey(ViPipe, HI_FALSE);

            if (pstDp->u8TrialCount >= pstDp->u8TrialCntLimit)/*TIMEOUT*/
            {
                Dpc_Calib_TimeOut(ViPipe, pstDp);
                DpExit(ViPipe, pstDp, pstRegCfg);
            }
            else if (pstDp->u16BptCalibNum > pstDp->u16DpCountMax)
            {
                Dpc_Calib_Max(pstDp->u16BptCalibNum, pstDp);
                if (pstDp->u8StaticDPThresh == 255)
                {
                    pstDp->u8TrialCount = pstDp->u8TrialCntLimit;
                }
                else
                {
                    pstDp->u8StaticDPThresh++;
                }
            }
            else if (pstDp->u16BptCalibNum < pstDp->u16DpCountMin)
            {
                Dpc_Calib_Min(pstDp->u16BptCalibNum, pstDp);
                if (pstDp->u8StaticDPThresh == 0)
                {
                    pstDp->u8TrialCount = pstDp->u8TrialCntLimit;
                }
                else
                {
                    pstDp->u8StaticDPThresh--;
                }
            }
            else    /*SUCCESS*/
            {
                Dpc_Calib_Success(ViPipe, u8BlkNum, pstDp, pStatInfo);
                DpExit(ViPipe, pstDp, pstRegCfg);
            }
        }
    }
}

static HI_VOID Dpc_Dark_Calib(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S  *pstDp, HI_U8 u8BlkNum, ISP_REG_CFG_S *pstRegCfg, ISP_STAT_S *pStatInfo)
{
    if (pstDp->u8FrameCnt < 9)
    {
        if (pstDp->u8FrameCnt == 0)
        {
            hi_ext_system_dpc_trigger_status_write(ViPipe, ISP_STATE_INIT);
            pstDp->u8CalibStarted = 1;
            pstDp->u8StaticDPThresh = hi_ext_system_dpc_start_thresh_read(ViPipe);
        }

        pstDp->u8FrameCnt++;

        if (pstDp->u8FrameCnt == 4)
        {
            pstDp->u32DpccBadThresh = 0xFF800000 + (pstDp->u8StaticDPThresh << 8) + ((0x80 * pstDp->u8DeadDevThresh) / 100);
            pstDp->u16DpccMode      = (ISP_DPCC_DEAD_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);
            pstDp->bStatEn          = 1;

            SetReadDpStatisKey(ViPipe, HI_TRUE);
        }

        if (pstDp->u8FrameCnt == 6)
        {
            pstDp->u16DpccMode = (ISP_DPCC_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);
            pstDp->bStatEn     = 0;
            Dpc_Read_Calib_Num(ViPipe, u8BlkNum, pstDp, pStatInfo);
        }

        if (pstDp->u8FrameCnt == 7)
        {
            //Dpc_Read_Calib_Num(ViPipe, u8BlkNum, pstDp, pStatInfo);
            SetReadDpStatisKey(ViPipe, HI_FALSE);

            if (pstDp->u8TrialCount >= pstDp->u8TrialCntLimit)
            {
                Dpc_Calib_TimeOut(ViPipe, pstDp);
                DpcDefCalibParam(pstDp);

                pstDp->u8CalibStarted  = 0;
                pstDp->u8CalibFinished = 1;
            }
            else if (pstDp->u16BptCalibNum > pstDp->u16DpCountMax)
            {
                Dpc_Calib_Max(pstDp->u16BptCalibNum, pstDp);

                if (pstDp->u8StaticDPThresh == 0)
                {
                    pstDp->u8TrialCount = pstDp->u8TrialCntLimit;
                }
                else
                {
                    pstDp->u8StaticDPThresh--;
                }
            }
            else if (pstDp->u16BptCalibNum < pstDp->u16DpCountMin)
            {
                Dpc_Calib_Min(pstDp->u16BptCalibNum, pstDp);
                if (pstDp->u8StaticDPThresh == 255 )
                {
                    pstDp->u8TrialCount = pstDp->u8TrialCntLimit;
                }
                else
                {
                    pstDp->u8StaticDPThresh++;
                }
            }
            else
            {
                Dpc_Calib_Success(ViPipe, u8BlkNum, pstDp, pStatInfo);
                DpcDefCalibParam(pstDp);

                pstDp->u8CalibStarted  = 0;
                pstDp->u8CalibFinished = 1;
            }
        }
    }
}

static HI_VOID ISP_Dpc_StaticCalibration(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S  *pstDp, HI_U8 u8BlkNum, ISP_REG_CFG_S *pstRegCfg, ISP_STAT_S *pStatInfo)
{
    HI_U8 i;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if ((1 != pstDp->u8PixelDetectType) && (0 != pstDp->u8PixelDetectType))
    {
        ISP_TRACE(HI_DBG_ERR, "invalid static defect pixel detect type!\n");
        return;
    }

    if ((0 == pstDp->u8CalibStarted))
    {
        if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
            IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
        {
            for (i = 0; i < u8BlkNum; i++)
            {
                if (HI_NULL == pstDp->pau32BptCalibTable[i])
                {
                    pstDp->pau32BptCalibTable[i] = (HI_U32 *)ISP_MALLOC(STATIC_DP_COUNT_NORMAL * sizeof(HI_U32));

                    if (HI_NULL == pstDp->pau32BptCalibTable[i])
                    {
                        ISP_TRACE(HI_DBG_ERR, "malloc dpc calibration table buffer failed\n");
                        Dpc_SafeFreeCalibLut(pstDp, u8BlkNum);
                        return;
                    }
                }

                memset(pstDp->pau32BptCalibTable[i], 0, STATIC_DP_COUNT_NORMAL * sizeof(HI_U32));
            }
        }
    }

    pstIspCtx->stLinkage.bDefectPixel = HI_TRUE;

    if (0 == pstDp->u8PixelDetectType)
    {
        Dpc_Hot_Calib(ViPipe, pstDp, u8BlkNum, pstRegCfg, pStatInfo);
    }
    else
    {
        Dpc_Dark_Calib(ViPipe, pstDp, u8BlkNum, pstRegCfg, pStatInfo);
    }

    if (1 == pstDp->u8CalibFinished)
    {
        if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
            IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
        {
            Dpc_SafeFreeCalibLut(pstDp, u8BlkNum);
        }
    }
}

static HI_VOID ISP_Dpc_Calib_Mode(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp, ISP_REG_CFG_S *pstRegCfg, ISP_STAT_S *pStatInfo)
{
    HI_U8 i;

    DpReadStaticCalibExtregs(ViPipe);
    ISP_Dpc_StaticCalibration(ViPipe, pstDp, pstRegCfg->u8CfgNum, pstRegCfg, pStatInfo);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.u32DpccBadThresh  = pstDp->u32DpccBadThresh;
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.u16DpccMode       = pstDp->u16DpccMode;
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.bDpcStatEn        = pstDp->bStatEn;
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.bResh             = HI_TRUE;
    }
}

static HI_VOID DpReadExtregs(VI_PIPE ViPipe)
{
    HI_U16 i;
    ISP_DEFECT_PIXEL_S *pstDp = HI_NULL;

    DP_GET_CTX(ViPipe, pstDp);

    pstDp->bStaticAttrUpdate    = hi_ext_system_dpc_static_attr_update_read(ViPipe);
    hi_ext_system_dpc_static_attr_update_write(ViPipe, HI_FALSE);

    if (pstDp->bStaticAttrUpdate)
    {
        pstDp->u16BptCorNum         = hi_ext_system_dpc_bpt_cor_number_read(ViPipe);
        pstDp->bStaicShow           = hi_ext_system_dpc_static_dp_show_read(ViPipe);
    }

    pstDp->bDynaAttrUpdateEn = hi_ext_system_dpc_dynamic_attr_update_read(ViPipe);
    hi_ext_system_dpc_dynamic_attr_update_write(ViPipe, HI_FALSE);

    if (pstDp->bDynaAttrUpdateEn)
    {
        pstDp->bDynamicManual       = hi_ext_system_dpc_dynamic_manual_enable_read(ViPipe);

        for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
        {
            pstDp->stCmosDpc.au16Strength[i]    = hi_ext_system_dpc_dynamic_strength_table_read(ViPipe, i);
            pstDp->stCmosDpc.au16BlendRatio[i]  = hi_ext_system_dpc_dynamic_blend_ratio_table_read(ViPipe, i);
        }

        pstDp->u16BlendRatio        = hi_ext_system_dpc_dynamic_blend_ratio_read(ViPipe);
        pstDp->u16Strength          = hi_ext_system_dpc_dynamic_strength_read(ViPipe);
        pstDp->bSupTwinkleEn        = hi_ext_system_dpc_suppress_twinkle_enable_read(ViPipe);
        pstDp->s8SupTwinkleThr      = hi_ext_system_dpc_suppress_twinkle_thr_read(ViPipe);
        pstDp->u8SupTwinkleSlope    = hi_ext_system_dpc_suppress_twinkle_slope_read(ViPipe);
    }
}

static HI_VOID  SplitDpCorLut(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S  *pstDp, ISP_REG_CFG_S *pstRegCfg, HI_U8 u8BlkNum)
{
    HI_S8  j;
    HI_U16 au16BptNum[ISP_STRIPING_MAX_NUM] = {0};
    HI_U16 i;
    HI_U16 u16XValue;
    HI_U32 u32BptValue;

    for (j = 0; j < u8BlkNum; j++)
    {
        memset(pstRegCfg->stAlgRegCfg[j].stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.au32DpccBpTable, 0, STATIC_DP_COUNT_NORMAL * sizeof(HI_U32));
    }

    for (j = (HI_S8)u8BlkNum - 1; j >= 0; j--)
    {
        for (i = 0; i < pstDp->u16BptCorNum; i++)
        {
            u32BptValue = hi_ext_system_dpc_cor_bpt_read(ViPipe, i);

            u16XValue = u32BptValue & 0x1FFF;

            if ((u16XValue >= (pstDp->au16OffsetForSplit[j])) && (u16XValue < pstDp->au16OffsetForSplit[j + 1]))
            {
                pstRegCfg->stAlgRegCfg[j].stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.au32DpccBpTable[au16BptNum[j]] = u32BptValue - pstDp->au16Offset[j];
                au16BptNum[j]++;

                if (au16BptNum[j] >= STATIC_DP_COUNT_NORMAL)
                {
                    break;
                }
            }
        }
    }

    for (j = 0; j < (HI_S8)u8BlkNum; j++)
    {
        pstRegCfg->stAlgRegCfg[j].stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.u16DpccBptNumber = au16BptNum[j];
        pstRegCfg->stAlgRegCfg[j].stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.bResh            = HI_TRUE;
        pstRegCfg->stAlgRegCfg[j].stDpRegCfg.stUsrRegCfg.stUsrStaCorRegCfg.u32UpdateIndex  += 1;
    }

    return;
}

static HI_S32 CalcRakeRatio(HI_S32 x0, HI_S32 y0, HI_S32 x1, HI_S32 y1, HI_S32 shift)
{
    if (x0 == x1)
    {
        return 0;
    }
    else
    {
        return ((y1 - y0) << shift ) / DIV_0_TO_1(x1 - x0);
    }
}

static HI_VOID ISP_Dpc_Usr_Cfg(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 i;
    ISP_DPC_USR_DYNA_COR_CFG_S *pstUsrDynaCorRegCfg = HI_NULL;

    if (pstDp->bStaticAttrUpdate)
    {
        if (pstDp->bStaicShow || pstDp->bStaticEnable)
        {
            SplitDpCorLut(ViPipe, pstDp, pstRegCfg, pstRegCfg->u8CfgNum);
        }
    }

    if (pstDp->bDynaAttrUpdateEn)
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstUsrDynaCorRegCfg = &pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stUsrRegCfg.stUsrDynaCorRegCfg;

            pstUsrDynaCorRegCfg->bDpccHardThrEn         = pstDp->bSupTwinkleEn ? (HI_FALSE) : (HI_TRUE);
            pstUsrDynaCorRegCfg->s8DpccSupTwinkleThrMax = CLIP3(pstDp->s8SupTwinkleThr, -128, 127);
            pstUsrDynaCorRegCfg->s8DpccSupTwinkleThrMin = CLIP3(pstUsrDynaCorRegCfg->s8DpccSupTwinkleThrMax - pstDp->u8SupTwinkleSlope, -128, 127);
            pstUsrDynaCorRegCfg->u16DpccRakeRatio       = CalcRakeRatio(pstUsrDynaCorRegCfg->s8DpccSupTwinkleThrMin, 0, pstUsrDynaCorRegCfg->s8DpccSupTwinkleThrMax, 128, 2);
            pstUsrDynaCorRegCfg->bResh                  = HI_TRUE;
        }
    }
}

static HI_VOID DpcResSwitch(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    ISP_DEFECT_PIXEL_S  *pstDp     = HI_NULL;

    DP_GET_CTX(ViPipe, pstDp);
    ISP_CHECK_POINTER_VOID(pstDp);

    DpcImageSize(ViPipe, pstDp);

    pstDp->bStaticAttrUpdate = HI_TRUE;
    ISP_Dpc_Usr_Cfg(ViPipe, pstDp, pstRegCfg);
}

static HI_VOID ISP_Dpc_Show_Mode(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg, ISP_CTX_S *pstIspCtx)
{
    HI_U8 i;

    pstIspCtx->stLinkage.bDefectPixel = HI_FALSE;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.u16DpccMode = ISP_DPCC_HIGHLIGHT_MODE;
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.bResh       = HI_TRUE;
    }
}

static HI_VOID soft_inter(ISP_DPC_DYNA_CFG_S *pstDpcHwCfg, HI_U8 u8DpccStat)
{
    HI_U8  u8StatIdxUp, u8StatIdxLow;
    HI_U8  u8StatUpper, u8StatLower;
    HI_U8  u8DpccLineThrRb1, u8DpccLineThrG1, u8DpccLineMadFacRb1, u8DpccLineMadFacG1;
    HI_U8  i;

    u8StatIdxUp = ISP_DPC_SOFT_SLOPE_GRADE - 1;
    for (i = 0; i < ISP_DPC_SOFT_SLOPE_GRADE; i++)
    {
        if (u8DpccStat < g_au8SoftSlopeGrade[i])
        {
            u8StatIdxUp = i;
            break;
        }
    }
    u8StatIdxLow = MAX2((HI_S8)u8StatIdxUp - 1, 0);

    u8StatUpper = g_au8SoftSlopeGrade[u8StatIdxUp];
    u8StatLower = g_au8SoftSlopeGrade[u8StatIdxLow];

    u8DpccLineThrRb1    = (HI_U8)LinearInter(u8DpccStat, \
                          u8StatLower, (g_au16SoftLineThr[u8StatIdxLow] & 0xFF00) >> 8, \
                          u8StatUpper, (g_au16SoftLineThr[u8StatIdxUp]  & 0xFF00) >> 8);
    u8DpccLineThrG1     = (HI_U8)LinearInter(u8DpccStat, \
                          u8StatLower, g_au16SoftLineThr[u8StatIdxLow] & 0xFF, \
                          u8StatUpper, g_au16SoftLineThr[u8StatIdxUp]  & 0xFF);

    u8DpccLineMadFacRb1 = (HI_U8)LinearInter(u8DpccStat, \
                          u8StatLower, (g_au16SoftLineMadFac[u8StatIdxLow] & 0xFF00) >> 8, \
                          u8StatUpper, (g_au16SoftLineMadFac[u8StatIdxUp]  & 0xFF00) >> 8);
    u8DpccLineMadFacG1  = (HI_U8)LinearInter(u8DpccStat, \
                          u8StatLower, g_au16SoftLineMadFac[u8StatIdxLow] & 0xFF, \
                          u8StatUpper, g_au16SoftLineMadFac[u8StatIdxUp]  & 0xFF);
    pstDpcHwCfg->au16DpccLineThr[0]    = (((HI_U16)u8DpccLineThrRb1) << 8) + u8DpccLineThrG1;
    pstDpcHwCfg->au16DpccLineMadFac[0] = (((HI_U16)(u8DpccLineMadFacRb1 & 0x3F)) << 8) + (u8DpccLineMadFacG1 & 0x3F);
}

static HI_VOID set_dpcc_parameters_inter(ISP_DPC_DYNA_CFG_S *pstIspDpccHwCfg, HI_U8 u8DpccStat)
{
    HI_U8 i, j;
    HI_U8 u8StatUpper, u8StatLower;
    HI_U8 u8StatIdxUp, u8StatIdxLow;
    ISP_DPCC_DERIVED_PARAM_S stDpcDerParam;

    u8StatIdxUp = ISP_DPC_SLOPE_GRADE - 1;
    for (i = 0; i < ISP_DPC_SLOPE_GRADE; i++)
    {
        if (u8DpccStat < g_au8SlopeGrade[i])
        {
            u8StatIdxUp = i;
            break;
        }
    }
    u8StatIdxLow = MAX2((HI_S8)u8StatIdxUp - 1, 0);

    u8StatUpper = g_au8SlopeGrade[u8StatIdxUp];
    u8StatLower = g_au8SlopeGrade[u8StatIdxLow];

    pstIspDpccHwCfg->u8DpccSetUse       = g_stDpcDefCfg[u8StatIdxLow].u8DpccSetUse;
    pstIspDpccHwCfg->u16DpccMethodsSet1 = g_stDpcDefCfg[u8StatIdxLow].u16DpccMethodsSet1;
    pstIspDpccHwCfg->u16DpccMethodsSet2 = g_stDpcDefCfg[u8StatIdxLow].u16DpccMethodsSet2;
    pstIspDpccHwCfg->u16DpccMethodsSet3 = g_stDpcDefCfg[u8StatIdxLow].u16DpccMethodsSet3;
    pstIspDpccHwCfg->u32DpccBadThresh   = g_stDpcDefCfg[u8StatIdxLow].u32DpccBadThresh;

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 3; j++)
        {
            stDpcDerParam.au8DpccLineThr[i][j]    = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccLineThr[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccLineThr[i][j]);
            stDpcDerParam.au8DpccLineMadFac[i][j] = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccLineMadFac[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccLineMadFac[i][j]);
            stDpcDerParam.au8DpccPgFac[i][j]      = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccPgFac[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccPgFac[i][j]);
            stDpcDerParam.au8DpccRgFac[i][j]      = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccRgFac[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccRgFac[i][j]);
            stDpcDerParam.au8DpccRndThr[i][j]     = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccRndThr[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccRndThr[i][j]);
            stDpcDerParam.au8DpccRndOffs[i][j]    = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccRndOffs[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccRndOffs[i][j]);
            stDpcDerParam.au8DpccRo[i][j]         = (HI_U8)LinearInter(u8DpccStat, \
                                                    u8StatLower, g_stDpcDerParam[u8StatIdxLow].au8DpccRo[i][j], \
                                                    u8StatUpper, g_stDpcDerParam[u8StatIdxUp].au8DpccRo[i][j]);
        }
    }

    for (j = 0; j < 3; j++)
    {
        pstIspDpccHwCfg->au16DpccLineThr[j]    = ((HI_U16)(stDpcDerParam.au8DpccLineThr[0][j]          ) << 8) + (stDpcDerParam.au8DpccLineThr[1][j]          );
        pstIspDpccHwCfg->au16DpccLineMadFac[j] = ((HI_U16)(stDpcDerParam.au8DpccLineMadFac[0][j] & 0x3F) << 8) + (stDpcDerParam.au8DpccLineMadFac[1][j] & 0x3F);
        pstIspDpccHwCfg->au16DpccPgFac[j]      = ((HI_U16)(stDpcDerParam.au8DpccPgFac[0][j]      & 0x3F) << 8) + (stDpcDerParam.au8DpccPgFac[1][j]      & 0x3F);
        pstIspDpccHwCfg->au16DpccRndThr[j]     = ((HI_U16)(stDpcDerParam.au8DpccRndThr[0][j]           ) << 8) + (stDpcDerParam.au8DpccRndThr[1][j]           );
        pstIspDpccHwCfg->au16DpccRgFac[j]      = ((HI_U16)(stDpcDerParam.au8DpccRgFac[0][j]      & 0x3F) << 8) + (stDpcDerParam.au8DpccRgFac[1][j]     & 0x3F);
    }

    pstIspDpccHwCfg->u16DpccRoLimits    = ((HI_U16)(stDpcDerParam.au8DpccRo[0][2] & 0x3) << 10) + ((HI_U16)(stDpcDerParam.au8DpccRo[1][2] & 0x3) << 8) + ((stDpcDerParam.au8DpccRo[0][1] & 0x3) << 6) + \
                                          ((stDpcDerParam.au8DpccRo[1][1] & 0x3) << 4) + ((stDpcDerParam.au8DpccRo[0][0] & 0x3) << 2) + (stDpcDerParam.au8DpccRo[1][0] & 0x3);

    pstIspDpccHwCfg->u16DpccRndOffs     = ((HI_U16)(stDpcDerParam.au8DpccRndOffs[0][2] & 0x3) << 10) + ((HI_U16)(stDpcDerParam.au8DpccRndOffs[1][2] & 0x3) << 8) + ((stDpcDerParam.au8DpccRndOffs[0][1] & 0x3) << 6) + \
                                          ((stDpcDerParam.au8DpccRndOffs[1][1] & 0x3) << 4) + ((stDpcDerParam.au8DpccRndOffs[0][0] & 0x3) << 2) + (stDpcDerParam.au8DpccRndOffs[1][0] & 0x3);
}

static HI_S32 ISP_Dynamic_set(HI_U32 u32Iso, ISP_DPC_DYNA_CFG_S *pstDpcHwCfg, ISP_DEFECT_PIXEL_S *pstDpcFwCfg)
{
    HI_U8  u8Alpha0RB = 0;
    HI_U8  u8Alpha0G  = 0;  /*the blend ratio of 5 & 9 */
    HI_U8  u8Alpha1RB = 0;
    HI_U8  u8Alpha1G  = 0;  /*the blend ratio of input data and filtered result*/
    HI_U8  u8DpccStat;
    HI_U8  u8IsoIndexUpper, u8IsoIndexLower;
    HI_U16 u16BlendRatio = 0x0;
    HI_U16 u16Strength   = 0;
    HI_U16 u16DpccMode   = pstDpcFwCfg->u16DpccMode;
    ISP_CMOS_DPC_S *pstDpc = &pstDpcFwCfg->stCmosDpc;

    if (pstDpcFwCfg->bDynamicManual)
    {
        u16Strength = pstDpcFwCfg->u16Strength;
        u16BlendRatio = pstDpcFwCfg->u16BlendRatio;
    }
    else
    {
        u8IsoIndexUpper = GetIsoIndex(u32Iso);
        u8IsoIndexLower = MAX2((HI_S8)u8IsoIndexUpper - 1, 0);

        u16Strength     = (HI_U16)LinearInter(u32Iso, \
                                              g_au32IsoLut[u8IsoIndexLower], (HI_S32)pstDpc->au16Strength[u8IsoIndexLower], \
                                              g_au32IsoLut[u8IsoIndexUpper], (HI_S32)pstDpc->au16Strength[u8IsoIndexUpper]);
        u16BlendRatio   = (HI_U16)LinearInter(u32Iso, \
                                              g_au32IsoLut[u8IsoIndexLower], (HI_S32)pstDpc->au16BlendRatio[u8IsoIndexLower], \
                                              g_au32IsoLut[u8IsoIndexUpper], (HI_S32)pstDpc->au16BlendRatio[u8IsoIndexUpper]);
    }

    u8DpccStat = u16Strength >> 1;
    set_dpcc_parameters_inter(pstDpcHwCfg, u8DpccStat);
    if (u8DpccStat == 0)
    {
        u16DpccMode &= 0xFFFC;
    }
    else if (u8DpccStat > 100)
    {
        pstDpcHwCfg->u8DpccSetUse          = 0x7;
        pstDpcHwCfg->u16DpccMethodsSet1    = 0x1f1f;
        pstDpcHwCfg->au16DpccLineThr[0]    = g_au16DpcLineThr1[u8DpccStat - 101];
        pstDpcHwCfg->au16DpccLineMadFac[0] = g_au16DpcLineMadFac1[u8DpccStat - 101];
        pstDpcHwCfg->au16DpccPgFac[0]      = g_au16DpcPgFac1[u8DpccStat - 101];
        pstDpcHwCfg->au16DpccRndThr[0]     = g_au16DpcRndThr1[u8DpccStat - 101];
        pstDpcHwCfg->au16DpccRgFac[0]      = g_au16DpcRgFac1[u8DpccStat - 101];
        pstDpcHwCfg->u16DpccRoLimits       = g_au16DpcRoLimits1[u8DpccStat - 101];
        pstDpcHwCfg->u16DpccRndOffs        = g_au16DpcRndOffs1[u8DpccStat - 101];
    }

    if (pstDpcFwCfg->bSupTwinkleEn)
    {
        if ((u8DpccStat == 0) || !((pstDpcFwCfg->u16DpccMode & 0x2) >> 1))
        {
            pstDpcFwCfg->bSupTwinkleEn = 0;
        }
        else
        {
            soft_inter(pstDpcHwCfg, u8DpccStat);
        }
    }

    if (!((u16DpccMode & 0x2) >> 1))
    {
        u16BlendRatio = 0;
    }
    u8Alpha0RB = (u16BlendRatio > 0x80) ? (u16BlendRatio - 0x80) : 0x0;
    u8Alpha1RB = (u16BlendRatio > 0x80) ? 0x80 : u16BlendRatio;
    pstDpcHwCfg->u32DpccAlpha  = (u8Alpha0RB << 24) + (u8Alpha0G << 16) + (u8Alpha1RB << 8) + u8Alpha1G;
    pstDpcHwCfg->u16DpccMode   = u16DpccMode;

    return HI_SUCCESS;
}

static HI_S32 ISP_Dpc_Normal_Mode(VI_PIPE ViPipe, ISP_DEFECT_PIXEL_S *pstDp, ISP_REG_CFG_S *pstRegCfg, ISP_CTX_S *pstIspCtx)
{
    HI_U8 i;

    pstIspCtx->stLinkage.bDefectPixel = HI_FALSE;

    pstDp->u16DpccMode = (ISP_DPCC_MODE & 0x3dd) + (pstDp->bStaticEnable << 5) + (pstDp->bDynamicEnable << 1);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        ISP_Dynamic_set(pstIspCtx->stLinkage.u32Iso, &pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg, pstDp);
        pstRegCfg->stAlgRegCfg[i].stDpRegCfg.stDynaRegCfg.bResh = HI_TRUE;
    }

    return HI_SUCCESS;
}

static HI_VOID ISP_DpWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_REG_CFG_S *pstRegCfg  = (ISP_REG_CFG_S *)pRegCfg;
    ISP_CTX_S     *pstIspCtx  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    ISP_DpEnableCfg(ViPipe, pstRegCfg->u8CfgNum, pstRegCfg);

    if (pstIspCtx->stBlockAttr.u8BlockNum != pstIspCtx->stBlockAttr.u8PreBlockNum)
    {
        DpcResSwitch(ViPipe, pstRegCfg);
    }

    pstRegCfg->unKey.bit1DpCfg = 1;
}

HI_S32 ISP_DpInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = DpCtxInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = DpInitialize(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    DpRegsInitialize(ViPipe, pstRegCfg);
    DpExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_DpRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                 HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    ISP_CTX_S           *pstIspCtx  = HI_NULL;
    ISP_DEFECT_PIXEL_S  *pstDp      = HI_NULL;
    ISP_REG_CFG_S       *pstRegCfg  = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    DP_GET_CTX(ViPipe, pstDp);
    ISP_CHECK_POINTER(pstDp);

    if (HI_FALSE == pstIspCtx->stLinkage.bStatReady)
    {
        return HI_SUCCESS;
    }

    pstRegCfg->unKey.bit1DpCfg = 1;

    pstDp->bStaCalibrationEn = hi_ext_system_dpc_static_calib_enable_read(ViPipe);
    pstDp->bDynamicEnable    = hi_ext_system_dpc_dynamic_cor_enable_read(ViPipe);
    pstDp->bStaticEnable     = hi_ext_system_dpc_static_cor_enable_read(ViPipe);

    if ((HI_FALSE == pstDp->bStaCalibrationEn) && (1 == pstDp->u8CalibStarted))/*quit calibration*/
    {
        DpExit(ViPipe, pstDp, pstRegCfg);
        if (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) || \
            IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))
        {
            Dpc_SafeFreeCalibLut(pstDp, pstRegCfg->u8CfgNum);
        }
    }

    if (HI_TRUE == pstDp->bStaCalibrationEn)/*calibration mode */
    {
        ISP_Dpc_Calib_Mode(ViPipe, pstDp, pstRegCfg, (ISP_STAT_S *)pStatInfo);

        return HI_SUCCESS;
    }

    DpReadExtregs(ViPipe);

    if (HI_TRUE == pstDp->bStaicShow) /*highlight static defect pixels mode*/
    {
        ISP_Dpc_Show_Mode(ViPipe, pstRegCfg, pstIspCtx);
    }
    else/*normal detection and correction mode*/
    {
        ISP_Dpc_Normal_Mode(ViPipe, pstDp, pstRegCfg, pstIspCtx);
    }

    ISP_Dpc_Usr_Cfg(ViPipe, pstDp, pstRegCfg);

    return HI_SUCCESS;
}

HI_S32 ISP_DpCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S        *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_DpWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_CHANGE_IMAGE_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            DpcResSwitch(ViPipe, &pRegCfg->stRegCfg);
            break;
        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DpExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;
    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stDpRegCfg.abDpcEn[0] = HI_FALSE;
        pRegCfg->stRegCfg.stAlgRegCfg[i].stDpRegCfg.abDpcEn[1] = HI_FALSE;
        pRegCfg->stRegCfg.stAlgRegCfg[i].stDpRegCfg.abDpcEn[2] = HI_FALSE;
        pRegCfg->stRegCfg.stAlgRegCfg[i].stDpRegCfg.abDpcEn[3] = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1DpCfg = 1;

    DpCtxExit(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterDp(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Dp);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_DP;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_DpInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_DpRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_DpCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_DpExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
