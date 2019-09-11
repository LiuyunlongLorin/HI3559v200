/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_black_offset.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017/04/24
  Description   :
  History       :
  1.Date        : 2017/04/24
    Modification: Created file

******************************************************************************/

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define LUT_FACTOR (8)

typedef struct hiISP_BLACKLEVEL_S
{
    HI_BOOL bPreDefectPixel;
    HI_U8   u8BlackLevelChange;
    HI_U8   u8WDRModeState;
    HI_U16  au16BlackLevel[ISP_BAYER_CHN_NUM];
    HI_U16  au16RmDiffBlackLevel[ISP_BAYER_CHN_NUM];
    HI_U16  au16ActualBlackLevel[ISP_BAYER_CHN_NUM];
    ISP_OP_TYPE_E enOpType;
} ISP_BLACKLEVEL_S;

ISP_BLACKLEVEL_S g_astBlackLevelCtx[ISP_MAX_PIPE_NUM] = {{0}};
#define BLACKLEVEL_GET_CTX(dev, pstBlackLevelCtx)   pstBlackLevelCtx = &g_astBlackLevelCtx[dev]

static HI_VOID BlcInitialize(VI_PIPE ViPipe)
{
    HI_U8  i;
    HI_U8  u8WDRMode;
    ISP_BLACKLEVEL_S        *pstBlackLevel    = HI_NULL;
    ISP_CMOS_BLACK_LEVEL_S  *pstSnsBlackLevel = HI_NULL;
    ISP_CTX_S               *pstIspCtx        = HI_NULL;

    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);
    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8WDRMode = pstIspCtx->u8SnsWDRMode;

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = pstSnsBlackLevel->au16BlackLevel[i];
    }

    if (IS_LINEAR_MODE(u8WDRMode) || IS_BUILT_IN_WDR_MODE(u8WDRMode))
    {
        pstBlackLevel->u8WDRModeState = HI_FALSE;
    }
    else
    {
        pstBlackLevel->u8WDRModeState = HI_TRUE;
    }

    pstBlackLevel->bPreDefectPixel    = HI_FALSE;
    pstBlackLevel->enOpType           = OP_TYPE_AUTO;
}

static HI_VOID BlcExtRegsInitialize(VI_PIPE ViPipe)
{
    ISP_BLACKLEVEL_S        *pstBlackLevel    = HI_NULL;

    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);

    hi_ext_system_black_level_manual_mode_write(ViPipe, HI_EXT_SYSTEM_BLACK_LEVEL_MANUAL_MODE_DEFAULT);
    hi_ext_system_black_level_change_write(ViPipe, HI_EXT_SYSTEM_BLACK_LEVEL_CHANGE_DEFAULT);
    hi_ext_system_black_level_00_write(ViPipe, pstBlackLevel->au16BlackLevel[0]);
    hi_ext_system_black_level_01_write(ViPipe, pstBlackLevel->au16BlackLevel[1]);
    hi_ext_system_black_level_10_write(ViPipe, pstBlackLevel->au16BlackLevel[2]);
    hi_ext_system_black_level_11_write(ViPipe, pstBlackLevel->au16BlackLevel[3]);

    hi_ext_system_black_level_query_00_write(ViPipe, pstBlackLevel->au16ActualBlackLevel[0]);
    hi_ext_system_black_level_query_01_write(ViPipe, pstBlackLevel->au16ActualBlackLevel[1]);
    hi_ext_system_black_level_query_10_write(ViPipe, pstBlackLevel->au16ActualBlackLevel[2]);
    hi_ext_system_black_level_query_11_write(ViPipe, pstBlackLevel->au16ActualBlackLevel[3]);
}

static HI_VOID BalanceBlackLevel(ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8  i;
    HI_U16 u16OffsetMin;

    /* calulate Min blacklevel of RGrGbG channel */
    u16OffsetMin = pstBlackLevel->au16BlackLevel[0];
    for (i = 1; i < ISP_BAYER_CHN_NUM; i++)
    {
        if (u16OffsetMin > pstBlackLevel->au16BlackLevel[i])
        {
            u16OffsetMin = pstBlackLevel->au16BlackLevel[i];
        }
    }

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstBlackLevel->au16RmDiffBlackLevel[i] = u16OffsetMin;
    }
}

static HI_S32 LinearInterpol(HI_U16 xm, HI_U16 x0, HI_S32 y0, HI_U16 x1, HI_S32 y1, HI_U8 u8Shift)
{
    HI_S32 ym;

    if ( xm <= (x0 << u8Shift))
    {
        return y0;
    }
    if ( xm >= (x1 << u8Shift))
    {
        return y1;
    }

    ym = (y1 - y0) * (xm - (x0 << u8Shift)) / DIV_0_TO_1(x1 - x0) + y0;

    return ym;
}

static HI_VOID GetBuiltInExpanderBlc(const ISP_CMOS_EXPANDER_S *pstSnsExpander, HI_U16 *pu16SensorBlc, HI_U16 *pu16ExpanderBlc)
{
    HI_U8  i, j;
    HI_U8  u8Shift    = 12 - LUT_FACTOR;
    HI_U8  u8IndexUp  = ISP_EXPANDER_POINT_NUM - 1;
    HI_U8  u8IndexLow = 0;
    HI_U16 au16X[ISP_EXPANDER_POINT_NUM + 1] = {0};
    HI_U32 au32Y[ISP_EXPANDER_POINT_NUM + 1] = {0};

    for (i = 1; i < ISP_EXPANDER_POINT_NUM + 1; i++)
    {
        au16X[i] = pstSnsExpander->astExpanderPoint[i - 1].u16X;
        au32Y[i] = pstSnsExpander->astExpanderPoint[i - 1].u32Y;
    }

    for (j = 0; j < ISP_BAYER_CHN_NUM; j++)
    {
        u8IndexUp  = ISP_EXPANDER_POINT_NUM - 1;

        for (i = 0; i < ISP_EXPANDER_POINT_NUM; i++)
        {
            if ((pu16SensorBlc[j] >> u8Shift) < au16X[i])
            {
                u8IndexUp = i;
                break;
            }
        }

        u8IndexLow = (HI_U8)MAX2((HI_S8)u8IndexUp - 1, 0);

        pu16ExpanderBlc[j] = ((HI_U32)LinearInterpol(pu16SensorBlc[j], au16X[u8IndexLow], au32Y[u8IndexLow], au16X[u8IndexUp], au32Y[u8IndexUp], u8Shift)) >> (6 + u8Shift);
    }
}

static HI_VOID BE_BlcDynaRegs_Linear(VI_PIPE ViPipe, ISP_BE_BLC_CFG_S  *pstBeBlcCfg, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8 i, j;

    for (j = 0; j < ISP_BAYER_CHN_NUM; j++)
    {
        for (i = 0; i < ISP_WDR_CHN_MAX; i++)
        {
            pstBeBlcCfg->st4DgBlc[i].stUsrRegCfg.au16Blc[j]     = pstBlackLevel->au16RmDiffBlackLevel[j] << 2; /*4DG*/
            pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.au16Blc[j]     = 0;                                           /*WDR*/
            pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.u16OutBlc      = 0;                                           /*WDR*/
            pstBeBlcCfg->stFlickerBlc[i].stUsrRegCfg.au16Blc[j] = 0;                                           /*flicker*/
        }

        pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[j] = 0;                                          /*expander*/
        pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[j]      = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*lsc*/
        pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[j]       = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*Dg*/
        pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[j]       = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*AE*/
        pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[j]       = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*MG*/
        pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[j]       = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*WB*/
    }
    /*bnr*/
    pstBeBlcCfg->stBnrBlc.stUsrRegCfg.au16Blc[0]  = pstBlackLevel->au16RmDiffBlackLevel[0];//12bits

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstBlackLevel->au16ActualBlackLevel[i] = pstBlackLevel->au16BlackLevel[i];
    }
}

static HI_VOID BE_BlcDynaRegs_Wdr(VI_PIPE ViPipe, ISP_BE_BLC_CFG_S  *pstBeBlcCfg, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8  i, j;
    HI_U8  u8WDRModeState = pstBlackLevel->u8WDRModeState;
    HI_U16 WDROutBlc = 0;

    if (HI_FALSE == u8WDRModeState) //reg value same as linear mode
    {
        BE_BlcDynaRegs_Linear(ViPipe, pstBeBlcCfg, pstBlackLevel);
    }
    else if (HI_TRUE == u8WDRModeState)
    {
        WDROutBlc = ((pstBlackLevel->au16RmDiffBlackLevel[0] >> 4) << 6);/*WDR outblc*/
        for (j = 0; j < ISP_BAYER_CHN_NUM; j++)
        {
            for (i = 0; i < ISP_WDR_CHN_MAX; i++)
            {
                pstBeBlcCfg->st4DgBlc[i].stUsrRegCfg.au16Blc[j]      = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*4DG*/
                pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.au16Blc[j]      = pstBlackLevel->au16RmDiffBlackLevel[j];     /*WDR*/
                pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.u16OutBlc       = WDROutBlc;                                  /*WDR*/
                pstBeBlcCfg->stFlickerBlc[i].stUsrRegCfg.au16Blc[j]  = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*flicker*/
            }

            pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[j] = 0;              /*expander*/
            pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[j]      = WDROutBlc >> 6; /*lsc*/
            pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[j]       = WDROutBlc >> 6; /*Dg*/
            pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[j]       = WDROutBlc >> 6; /*AE*/
            pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[j]       = WDROutBlc >> 6; /*MG*/
            pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[j]       = WDROutBlc >> 6; /*WB*/
        }

        /*bnr*/
        pstBeBlcCfg->stBnrBlc.stUsrRegCfg.au16Blc[0]  = WDROutBlc >> 8;//12bits

        for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
        {
            pstBlackLevel->au16ActualBlackLevel[i] = pstBlackLevel->au16BlackLevel[i];
        }
    }
}

static HI_S32 ExpanderCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_EXPANDER_S *pstExpander)
{
    HI_U8 i;

    for (i = 0; i < ISP_EXPANDER_POINT_NUM; i++)
    {
        if (pstExpander->astExpanderPoint[i].u16X > 0x101)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid astExpanderPoint[%d].u16X:%d!\n", i, pstExpander->astExpanderPoint[i].u16X);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstExpander->astExpanderPoint[i].u32Y > 0x100000)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid astExpanderPoint[%d].u16Y:%d!\n", i, pstExpander->astExpanderPoint[i].u32Y);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 BE_BlcDynaRegs_BuiltIn(VI_PIPE ViPipe, ISP_BE_BLC_CFG_S  *pstBeBlcCfg, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8  i, j;
    HI_U16 au16BlackLevel[ISP_BAYER_CHN_NUM] = {0};
    HI_U16 au16ActualBlackLevel[ISP_BAYER_CHN_NUM] = {0};
    HI_S32 s32Ret;
    ISP_CMOS_DEFAULT_S *pstSnsDft     = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    if (pstSnsDft->unKey.bit1Expander)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstExpander);
        s32Ret = ExpanderCheckCmosParam(ViPipe, pstSnsDft->pstExpander);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        GetBuiltInExpanderBlc(pstSnsDft->pstExpander, pstBlackLevel->au16RmDiffBlackLevel, au16BlackLevel);
        GetBuiltInExpanderBlc(pstSnsDft->pstExpander, pstBlackLevel->au16BlackLevel, au16ActualBlackLevel);
    }
    else
    {
        for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
        {
            au16BlackLevel[i]       = pstBlackLevel->au16RmDiffBlackLevel[i] << 2;//14bits
            au16ActualBlackLevel[i] = pstBlackLevel->au16BlackLevel[i] << 2;      //14bits
        }
    }

    for (j = 0; j < ISP_BAYER_CHN_NUM; j++)
    {
        for (i = 0; i < ISP_WDR_CHN_MAX; i++)
        {
            pstBeBlcCfg->st4DgBlc[i].stUsrRegCfg.au16Blc[j]     = 0;/*4DG*/
            pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.au16Blc[j]     = 0;/*WDR*/
            pstBeBlcCfg->stWdrBlc[i].stUsrRegCfg.u16OutBlc      = 0;/*WDR*/
            pstBeBlcCfg->stFlickerBlc[i].stUsrRegCfg.au16Blc[j] = 0;/*flicker*/
        }

        pstBeBlcCfg->stExpanderBlc.stUsrRegCfg.au16Blc[j] = 0;                 /*expander*/
        pstBeBlcCfg->stLscBlc.stUsrRegCfg.au16Blc[j]      = au16BlackLevel[j]; /*lsc*/
        pstBeBlcCfg->stDgBlc.stUsrRegCfg.au16Blc[j]       = au16BlackLevel[j]; /*Dg*/
        pstBeBlcCfg->stAeBlc.stUsrRegCfg.au16Blc[j]       = au16BlackLevel[j]; /*AE*/
        pstBeBlcCfg->stMgBlc.stUsrRegCfg.au16Blc[j]       = au16BlackLevel[j]; /*MG*/
        pstBeBlcCfg->stWbBlc.stUsrRegCfg.au16Blc[j]       = au16BlackLevel[j]; /*WB*/
    }

    /*bnr*/
    pstBeBlcCfg->stBnrBlc.stUsrRegCfg.au16Blc[0] = au16BlackLevel[0] >> 2;//12bits

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstBlackLevel->au16ActualBlackLevel[i] = au16ActualBlackLevel[i];/*Notice: Actual Blc is 14bits*/
    }

    return HI_SUCCESS;
}

static HI_VOID BE_BlcDynaRegs(VI_PIPE ViPipe, HI_U8  u8WdrMode, ISP_BE_BLC_CFG_S *pstBeBlcCfg, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    pstBlackLevel->u8WDRModeState = hi_ext_system_wdr_en_read(ViPipe);

    if (IS_LINEAR_MODE(u8WdrMode))
    {
        BE_BlcDynaRegs_Linear(ViPipe, pstBeBlcCfg, pstBlackLevel);
    }
    else if (IS_2to1_WDR_MODE(u8WdrMode) || IS_3to1_WDR_MODE(u8WdrMode) || IS_4to1_WDR_MODE(u8WdrMode))
    {
        BE_BlcDynaRegs_Wdr(ViPipe, pstBeBlcCfg, pstBlackLevel);
    }
    else if (IS_BUILT_IN_WDR_MODE(u8WdrMode))
    {
        BE_BlcDynaRegs_BuiltIn(ViPipe, pstBeBlcCfg, pstBlackLevel);
    }

    pstBeBlcCfg->stDeBlc.stUsrRegCfg.au16Blc[0]     = pstBlackLevel->au16RmDiffBlackLevel[0];

    pstBeBlcCfg->bReshDyna = HI_TRUE;
}

static HI_VOID BE_BlcStaticRegs(HI_U8  u8WdrMode, ISP_BE_BLC_CFG_S  *pstBeBlcCfg)
{
    HI_U8 i;

    /*4DG*/
    for (i = 0; i < ISP_WDR_CHN_MAX; i++)
    {
        pstBeBlcCfg->st4DgBlc[i].stStaticRegCfg.bBlcIn  = HI_TRUE;
        pstBeBlcCfg->st4DgBlc[i].stStaticRegCfg.bBlcOut = HI_TRUE;
    }

    /*WDR*/
    pstBeBlcCfg->stWdrBlc[0].stStaticRegCfg.bBlcOut = HI_TRUE;
    /*lsc*/
    pstBeBlcCfg->stLscBlc.stStaticRegCfg.bBlcIn     = HI_TRUE;
    pstBeBlcCfg->stLscBlc.stStaticRegCfg.bBlcOut    = HI_TRUE;
    /*Dg*/
    pstBeBlcCfg->stDgBlc.stStaticRegCfg.bBlcIn      = HI_TRUE;
    pstBeBlcCfg->stDgBlc.stStaticRegCfg.bBlcOut     = HI_FALSE;
    /*AE*/
    pstBeBlcCfg->stAeBlc.stStaticRegCfg.bBlcIn      = HI_FALSE;
    /*MG*/
    pstBeBlcCfg->stMgBlc.stStaticRegCfg.bBlcIn      = HI_FALSE;
    /*WB*/
    pstBeBlcCfg->stWbBlc.stStaticRegCfg.bBlcIn      = HI_FALSE;
    pstBeBlcCfg->stWbBlc.stStaticRegCfg.bBlcOut     = HI_FALSE;

    pstBeBlcCfg->bReshStatic = HI_TRUE;
}

static HI_VOID FE_BlcDynaRegs(ISP_FE_BLC_CFG_S  *pstFeBlcCfg, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8 j;

    for (j = 0; j < ISP_BAYER_CHN_NUM; j++)
    {
        pstFeBlcCfg->stFeBlc.stUsrRegCfg.au16Blc[j]   = (pstBlackLevel->au16BlackLevel[j] - pstBlackLevel->au16RmDiffBlackLevel[j]) << 2;
        pstFeBlcCfg->stFeDgBlc.stUsrRegCfg.au16Blc[j] = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*Fe Dg*/
        pstFeBlcCfg->stFeWbBlc.stUsrRegCfg.au16Blc[j] = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*Fe WB*/
        pstFeBlcCfg->stFeAeBlc.stUsrRegCfg.au16Blc[j] = pstBlackLevel->au16RmDiffBlackLevel[j] << 2;/*Fe AE*/
    }

    pstFeBlcCfg->bReshDyna = HI_TRUE;
}

static HI_VOID FE_BlcStaticRegs(ISP_FE_BLC_CFG_S  *pstFeBlcCfg)
{
    /*Fe Dg*/
    pstFeBlcCfg->stFeDgBlc.stStaticRegCfg.bBlcIn  = HI_TRUE;
    pstFeBlcCfg->stFeDgBlc.stStaticRegCfg.bBlcOut = HI_TRUE;
    /*Fe WB*/
    pstFeBlcCfg->stFeWbBlc.stStaticRegCfg.bBlcIn  = HI_TRUE;
    pstFeBlcCfg->stFeWbBlc.stStaticRegCfg.bBlcOut = HI_TRUE;
    /*Fe AE*/
    pstFeBlcCfg->stFeAeBlc.stStaticRegCfg.bBlcIn  = HI_FALSE;
    /*Fe BLC*/
    pstFeBlcCfg->stFeBlc.stStaticRegCfg.bBlcIn    = HI_TRUE;

    pstFeBlcCfg->bReshStatic = HI_TRUE;
}

static HI_VOID BlcRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8  i, u8WdrMode;
    ISP_BLACKLEVEL_S *pstBlackLevel = HI_NULL;
    ISP_CTX_S        *pstIspCtx     = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);

    u8WdrMode = pstIspCtx->u8SnsWDRMode;

    BalanceBlackLevel(pstBlackLevel);

    /*BE*/
    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        BE_BlcDynaRegs(ViPipe, u8WdrMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg, pstBlackLevel);
        BE_BlcStaticRegs(u8WdrMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg);
        pstRegCfg->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0 = pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWdrBlc[0].stUsrRegCfg.au16Blc[0];
    }
    pstRegCfg->unKey.bit1BeBlcCfg = 1;

    /*FE*/
    FE_BlcDynaRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg, pstBlackLevel);
    FE_BlcStaticRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg);
    pstRegCfg->unKey.bit1FeBlcCfg = 1;
}

static HI_S32 BlcReadExtregs(VI_PIPE ViPipe)
{
    ISP_BLACKLEVEL_S *pstBlackLevel = HI_NULL;
    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);

    pstBlackLevel->u8BlackLevelChange = hi_ext_system_black_level_change_read(ViPipe);
    hi_ext_system_black_level_change_write(ViPipe, HI_FALSE);

    if (pstBlackLevel->u8BlackLevelChange)
    {
        pstBlackLevel->enOpType           = hi_ext_system_black_level_manual_mode_read(ViPipe);
        pstBlackLevel->au16BlackLevel[0]  = hi_ext_system_black_level_00_read(ViPipe);
        pstBlackLevel->au16BlackLevel[1]  = hi_ext_system_black_level_01_read(ViPipe);
        pstBlackLevel->au16BlackLevel[2]  = hi_ext_system_black_level_10_read(ViPipe);
        pstBlackLevel->au16BlackLevel[3]  = hi_ext_system_black_level_11_read(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_VOID BlcWriteActualValue(VI_PIPE ViPipe, HI_U16 *pu16BlackLevel)
{
    hi_ext_system_black_level_query_00_write(ViPipe, pu16BlackLevel[0]);
    hi_ext_system_black_level_query_01_write(ViPipe, pu16BlackLevel[1]);
    hi_ext_system_black_level_query_10_write(ViPipe, pu16BlackLevel[2]);
    hi_ext_system_black_level_query_11_write(ViPipe, pu16BlackLevel[3]);
}

static HI_S32 ISP_BlcProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    ISP_CTRL_PROC_WRITE_S stProcTmp;
    ISP_BLACKLEVEL_S      *pstBlackLevel = HI_NULL;

    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);

    if ((HI_NULL == pstProc->pcProcBuff) || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----Black Level Actual INFO--------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16s"     "%16s"     "%16s"     "%16s\n",
                    "BlcR", "BlcGr", "BlcGb", "BlcB");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16u"     "%16u"     "%16u"     "%16u\n",
                    pstBlackLevel->au16ActualBlackLevel[0],
                    pstBlackLevel->au16ActualBlackLevel[1],
                    pstBlackLevel->au16ActualBlackLevel[2],
                    pstBlackLevel->au16ActualBlackLevel[3]
                   );

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

HI_BOOL CheckWDRState(VI_PIPE ViPipe, ISP_CTX_S *pstIspCtx, ISP_BLACKLEVEL_S *pstBlackLevel)
{
    HI_U8   bWDREn;
    HI_BOOL bWDRStateChange;

    bWDREn = hi_ext_system_wdr_en_read(ViPipe);

    if (IS_2to1_WDR_MODE(pstIspCtx->u8SnsWDRMode) || IS_3to1_WDR_MODE(pstIspCtx->u8SnsWDRMode) || IS_4to1_WDR_MODE(pstIspCtx->u8SnsWDRMode))
    {
        bWDRStateChange = (pstBlackLevel->u8WDRModeState == bWDREn) ? HI_FALSE : HI_TRUE;
    }
    else
    {
        bWDRStateChange = HI_FALSE;
    }

    pstBlackLevel->u8WDRModeState = bWDREn;

    return bWDRStateChange;
}

static HI_S32 ISP_BlcWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_U8 i;
    ISP_CTX_S               *pstIspCtx     = HI_NULL;
    ISP_BLACKLEVEL_S        *pstBlackLevel = HI_NULL;
    ISP_CMOS_BLACK_LEVEL_S  *pstSnsBlackLevel = HI_NULL;
    ISP_REG_CFG_S           *pstRegCfg     = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);
    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);

    for (i = 0; i < ISP_BAYER_CHN_NUM; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = pstSnsBlackLevel->au16BlackLevel[i];
    }

    BalanceBlackLevel(pstBlackLevel);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        BE_BlcDynaRegs(ViPipe, pstIspCtx->u8SnsWDRMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg, pstBlackLevel);
        pstRegCfg->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0 = pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWdrBlc[0].stUsrRegCfg.au16Blc[0];
    }

    FE_BlcDynaRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg, pstBlackLevel);

    BlcExtRegsInitialize(ViPipe);

    pstRegCfg->unKey.bit1BeBlcCfg = 1;
    pstRegCfg->unKey.bit1FeBlcCfg = 1;
    pstRegCfg->stAlgRegCfg[0].stFeBlcCfg.bReshDyna   = HI_TRUE;
    pstRegCfg->stAlgRegCfg[0].stFeBlcCfg.bReshStatic = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_BlcInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    BlcInitialize(ViPipe);
    BlcRegsInitialize(ViPipe, pstRegCfg);
    BlcExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_BlcRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8   i;
    HI_BOOL bWDRStateChange;
    VI_PIPE MainPipe = 0;
    ISP_CTX_S              *pstIspCtx        = HI_NULL;
    ISP_CMOS_BLACK_LEVEL_S *pstSnsBlackLevel = HI_NULL;
    ISP_BLACKLEVEL_S       *pstBlackLevel    = HI_NULL;
    ISP_REG_CFG_S          *pstRegCfg        = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    BLACKLEVEL_GET_CTX(ViPipe, pstBlackLevel);

    bWDRStateChange = CheckWDRState(ViPipe, pstIspCtx, pstBlackLevel);

    if (0 == hi_ext_system_dpc_static_defect_type_read(ViPipe)) //hot pixel
    {
        if (pstIspCtx->stLinkage.bDefectPixel)
        {
            if (HI_FALSE == pstBlackLevel->bPreDefectPixel)
            {
                for (i = 0; i < pstRegCfg->u8CfgNum; i++)
                {
                    pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWbBlc.stStaticRegCfg.bBlcIn = HI_TRUE;
                    pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.bReshStatic                   = HI_TRUE;
                }
            }
        }
        else if (pstBlackLevel->bPreDefectPixel)
        {
            for (i = 0; i < pstRegCfg->u8CfgNum; i++)
            {
                pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWbBlc.stStaticRegCfg.bBlcIn = HI_FALSE;
                pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.bReshStatic                   = HI_TRUE;
            }
        }

        pstBlackLevel->bPreDefectPixel = pstIspCtx->stLinkage.bDefectPixel;
    }

    BlcReadExtregs(ViPipe);

    pstRegCfg->unKey.bit1FeBlcCfg = 1;
    pstRegCfg->unKey.bit1BeBlcCfg = 1;

    /* TODO: mannual mode update */
    if (OP_TYPE_MANUAL == pstBlackLevel->enOpType)
    {
        BalanceBlackLevel(pstBlackLevel);
        FE_BlcDynaRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg, pstBlackLevel);

        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            BE_BlcDynaRegs(ViPipe, pstIspCtx->u8SnsWDRMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg, pstBlackLevel);
            pstRegCfg->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0 = pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWdrBlc[0].stUsrRegCfg.au16Blc[0];
        }

        BlcWriteActualValue(ViPipe, pstBlackLevel->au16ActualBlackLevel);

        return HI_SUCCESS;
    }

    /* some sensors's blacklevel is changed with iso. */
    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);

    /* TODO: sensors's blacklevel is changed by cmos. */
    if (HI_TRUE == pstSnsBlackLevel->bUpdate)
    {
        if (ISP_SNAP_NONE != pstIspCtx->stLinkage.enSnapPipeMode)
        {
            if (ViPipe == pstIspCtx->stLinkage.s32PicturePipeId)
            {
                MainPipe = pstIspCtx->stLinkage.s32PreviewPipeId;
                ISP_CHECK_PIPE(MainPipe);
                ISP_SensorGetBlc(MainPipe, &pstSnsBlackLevel);
            }
            else
            {
                ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);
            }

            memcpy(&pstBlackLevel->au16BlackLevel[0], &pstSnsBlackLevel->au16BlackLevel[0], ISP_BAYER_CHN_NUM * sizeof(HI_U16));
        }

        ISP_SensorUpdateBlc(ViPipe);
        memcpy(&pstBlackLevel->au16BlackLevel[0], &pstSnsBlackLevel->au16BlackLevel[0], ISP_BAYER_CHN_NUM * sizeof(HI_U16));

        BalanceBlackLevel(pstBlackLevel);

        FE_BlcDynaRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg, pstBlackLevel);

        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            BE_BlcDynaRegs(ViPipe, pstIspCtx->u8SnsWDRMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg, pstBlackLevel);
            pstRegCfg->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0 = pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWdrBlc[0].stUsrRegCfg.au16Blc[0];
        }

        BlcWriteActualValue(ViPipe, pstBlackLevel->au16ActualBlackLevel);

        return HI_SUCCESS;
    }

    /*sensors's blacklevel is changed by mpi. */
    if (((HI_TRUE == pstBlackLevel->u8BlackLevelChange) && \
         (OP_TYPE_AUTO == pstBlackLevel->enOpType)) || \
        (HI_TRUE == bWDRStateChange))
    {
        ISP_SensorUpdateBlc(ViPipe);
        memcpy(&pstBlackLevel->au16BlackLevel[0], &pstSnsBlackLevel->au16BlackLevel[0], ISP_BAYER_CHN_NUM * sizeof(HI_U16));

        BalanceBlackLevel(pstBlackLevel);

        FE_BlcDynaRegs(&pstRegCfg->stAlgRegCfg[0].stFeBlcCfg, pstBlackLevel);

        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            BE_BlcDynaRegs(ViPipe, pstIspCtx->u8SnsWDRMode, &pstRegCfg->stAlgRegCfg[i].stBeBlcCfg, pstBlackLevel);
            pstRegCfg->stAlgRegCfg[i].stWdrRegCfg.stSyncRegCfg.u16Offset0 = pstRegCfg->stAlgRegCfg[i].stBeBlcCfg.stWdrBlc[0].stUsrRegCfg.au16Blc[0];
        }

        BlcWriteActualValue(ViPipe, pstBlackLevel->au16ActualBlackLevel);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_BlcCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_BlcWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_PROC_WRITE :
            ISP_BlcProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_BlcExit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterBlc(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Blc);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_BLC;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_BlcInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_BlcRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_BlcCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_BlcExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
