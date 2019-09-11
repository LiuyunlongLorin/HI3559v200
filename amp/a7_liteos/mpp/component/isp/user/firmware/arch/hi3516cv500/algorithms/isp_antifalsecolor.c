/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_antifalsecolor.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2015/07/21
  Description   :
  History       :
  1.Date        : 2015/07/21
    Author      :
    Modification: Created file

******************************************************************************/

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_proc.h"
#include "isp_math_utils.h"
#include <math.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static const  HI_U8 g_au8AntiFalseColorThreshold[ISP_AUTO_ISO_STRENGTH_NUM] = {8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 4, 3, 2, 1, 0};
static const  HI_U8 g_au8AntiFalseColorStrength[ISP_AUTO_ISO_STRENGTH_NUM]  = {8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 4, 3, 2, 1, 0};

typedef struct hiISP_ANTIFALSECOLOR_S
{
    HI_BOOL bEnable;                    /*RW;Range:[0x0,0x1];Format:1.0; AntiFalseColor Enable*/
    HI_U8   u8WdrMode;
    HI_U8   au8LutAntiFalseColorGrayRatio[ISP_AUTO_ISO_STRENGTH_NUM];             //u5.0,
    HI_U8   au8LutAntiFalseColorCmaxSel[ISP_AUTO_ISO_STRENGTH_NUM];               //u4.0,
    HI_U8   au8LutAntiFalseColorDetgSel[ISP_AUTO_ISO_STRENGTH_NUM];               //u4.0,
    HI_U16  au16AntiFalseColorHfThreshLow[ISP_AUTO_ISO_STRENGTH_NUM];             //10.0,
    HI_U16  au16AntiFalseColorHfThreshHig[ISP_AUTO_ISO_STRENGTH_NUM];             //10.0,

    ISP_OP_TYPE_E enOpType;
    ISP_ANTIFALSECOLOR_AUTO_ATTR_S stAuto;
    ISP_ANTIFALSECOLOR_MANUAL_ATTR_S stManual;
} ISP_ANTIFALSECOLOR_S;

ISP_ANTIFALSECOLOR_S *g_pastAntiFalseColorCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define ANTIFALSECOLOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastAntiFalseColorCtx[dev])
#define ANTIFALSECOLOR_SET_CTX(dev, pstCtx)   (g_pastAntiFalseColorCtx[dev] = pstCtx)
#define ANTIFALSECOLOR_RESET_CTX(dev)         (g_pastAntiFalseColorCtx[dev] = HI_NULL)

HI_S32 AntiFalseColorCtxInit(VI_PIPE ViPipe)
{
    ISP_ANTIFALSECOLOR_S *pastAntiFalseColorCtx = HI_NULL;

    ANTIFALSECOLOR_GET_CTX(ViPipe, pastAntiFalseColorCtx);

    if (HI_NULL == pastAntiFalseColorCtx)
    {
        pastAntiFalseColorCtx = (ISP_ANTIFALSECOLOR_S *)ISP_MALLOC(sizeof(ISP_ANTIFALSECOLOR_S));
        if (HI_NULL == pastAntiFalseColorCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] AntiFalseColorCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pastAntiFalseColorCtx, 0, sizeof(ISP_ANTIFALSECOLOR_S));

    ANTIFALSECOLOR_SET_CTX(ViPipe, pastAntiFalseColorCtx);

    return HI_SUCCESS;
}

HI_VOID AntiFalseColorCtxExit(VI_PIPE ViPipe)
{
    ISP_ANTIFALSECOLOR_S *pastAntiFalseColorCtx = HI_NULL;

    ANTIFALSECOLOR_GET_CTX(ViPipe, pastAntiFalseColorCtx);
    ISP_FREE(pastAntiFalseColorCtx);
    ANTIFALSECOLOR_RESET_CTX(ViPipe);
}

static HI_VOID  AntiFalseColorInitFwWdr(VI_PIPE ViPipe)
{
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;

    HI_U8   au8LutAntiFalseColorGrayRatio[ISP_AUTO_ISO_STRENGTH_NUM] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    HI_U8   au8LutAntiFalseColorCmaxSel[ISP_AUTO_ISO_STRENGTH_NUM]   = {4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    HI_U8   au8LutAntiFalseColorDetgSel[ISP_AUTO_ISO_STRENGTH_NUM]   = {4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    HI_U16  au16AntiFalseColorHfThreshLow[ISP_AUTO_ISO_STRENGTH_NUM] = {96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96};
    HI_U16  au16AntiFalseColorHfThreshHig[ISP_AUTO_ISO_STRENGTH_NUM] = {256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256};

    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);

    memcpy(pstAntiFalseColor->au8LutAntiFalseColorGrayRatio, au8LutAntiFalseColorGrayRatio, sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au8LutAntiFalseColorCmaxSel,   au8LutAntiFalseColorCmaxSel,   sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au8LutAntiFalseColorDetgSel,   au8LutAntiFalseColorDetgSel,   sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au16AntiFalseColorHfThreshLow, au16AntiFalseColorHfThreshLow, sizeof(HI_U16)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au16AntiFalseColorHfThreshHig, au16AntiFalseColorHfThreshHig, sizeof(HI_U16)*ISP_AUTO_ISO_STRENGTH_NUM);
}

static HI_VOID  AntiFalseColorInitFwLinear(VI_PIPE ViPipe)
{
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;

    HI_U8   au8LutAntiFalseColorGrayRatio[ISP_AUTO_ISO_STRENGTH_NUM] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    HI_U8   au8LutAntiFalseColorCmaxSel[ISP_AUTO_ISO_STRENGTH_NUM]   = {4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
    HI_U8   au8LutAntiFalseColorDetgSel[ISP_AUTO_ISO_STRENGTH_NUM]   = {4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    HI_U16  au16AntiFalseColorHfThreshLow[ISP_AUTO_ISO_STRENGTH_NUM] = {30, 30, 35, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48};
    HI_U16  au16AntiFalseColorHfThreshHig[ISP_AUTO_ISO_STRENGTH_NUM] = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128};

    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);

    memcpy(pstAntiFalseColor->au8LutAntiFalseColorGrayRatio, au8LutAntiFalseColorGrayRatio, sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au8LutAntiFalseColorCmaxSel,   au8LutAntiFalseColorCmaxSel,   sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au8LutAntiFalseColorDetgSel,   au8LutAntiFalseColorDetgSel,   sizeof(HI_U8)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au16AntiFalseColorHfThreshLow, au16AntiFalseColorHfThreshLow, sizeof(HI_U16)*ISP_AUTO_ISO_STRENGTH_NUM);
    memcpy(pstAntiFalseColor->au16AntiFalseColorHfThreshHig, au16AntiFalseColorHfThreshHig, sizeof(HI_U16)*ISP_AUTO_ISO_STRENGTH_NUM);
}

static HI_S32 AntiFalseColor_SetLongFrameMode(VI_PIPE ViPipe)
{
    ISP_CTX_S*          pstIspCtx   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
	
    if ( (ISP_FSWDR_LONG_FRAME_MODE  == pstIspCtx->stLinkage.enFSWDRMode) || (ISP_FSWDR_AUTO_LONG_FRAME_MODE == pstIspCtx->stLinkage.enFSWDRMode))
    {
        AntiFalseColorInitFwLinear(ViPipe);
    }
    else
    {
        AntiFalseColorInitFwWdr(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_S32 AntiFalseColorCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_ANTIFALSECOLOR_S  *pstAntiFalseColor)
{
    HI_U8   i;

    ISP_CHECK_BOOL(pstAntiFalseColor->bEnable);

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        if (pstAntiFalseColor->au8AntiFalseColorThreshold[i] > 0x20)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid au8AntiFalseColorThreshold[%d]!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pstAntiFalseColor->au8AntiFalseColorStrength[i] > 0x1f)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid au8AntiFalseColorStrength[%d]!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 AntiFalseColorExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U32 i;
    HI_S32 s32Ret;
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);

    pstAntiFalseColor->u8WdrMode = pstIspCtx->u8SnsWDRMode;

    if (0 != pstAntiFalseColor->u8WdrMode)
    {
        AntiFalseColorInitFwWdr(ViPipe);
    }
    else
    {
        AntiFalseColorInitFwLinear(ViPipe);
    }

    hi_ext_system_antifalsecolor_manual_mode_write(ViPipe, HI_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_MODE_DEFAULT);

    pstAntiFalseColor->stManual.u8AntiFalseColorThreshold    = HI_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_THRESHOLD_DEFAULT;
    pstAntiFalseColor->stManual.u8AntiFalseColorStrength     = HI_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_STRENGTH_DEFAULT;

    hi_ext_system_antifalsecolor_manual_threshold_write(ViPipe, pstAntiFalseColor->stManual.u8AntiFalseColorThreshold);
    hi_ext_system_antifalsecolor_manual_strenght_write(ViPipe, pstAntiFalseColor->stManual.u8AntiFalseColorStrength);

    if (pstSnsDft->unKey.bit1AntiFalseColor)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstAntiFalseColor);

        s32Ret = AntiFalseColorCheckCmosParam(ViPipe, pstSnsDft->pstAntiFalseColor);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        hi_ext_system_antifalsecolor_enable_write(ViPipe, pstSnsDft->pstAntiFalseColor->bEnable);

        memcpy(pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold, pstSnsDft->pstAntiFalseColor->au8AntiFalseColorThreshold, ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U8));
        memcpy(pstAntiFalseColor->stAuto.au8AntiFalseColorStrength,  pstSnsDft->pstAntiFalseColor->au8AntiFalseColorStrength,  ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U8));
    }
    else
    {
        hi_ext_system_antifalsecolor_enable_write(ViPipe, HI_EXT_SYSTEM_ANTIFALSECOLOR_ENABLE_DEFAULT);
        memcpy(pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold, g_au8AntiFalseColorThreshold, ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U8));
        memcpy(pstAntiFalseColor->stAuto.au8AntiFalseColorStrength,  g_au8AntiFalseColorStrength,  ISP_AUTO_ISO_STRENGTH_NUM * sizeof(HI_U8));
    }

    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
    {
        hi_ext_system_antifalsecolor_auto_threshold_write(ViPipe, i, pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold[i]);
        hi_ext_system_antifalsecolor_auto_strenght_write(ViPipe, i, pstAntiFalseColor->stAuto.au8AntiFalseColorStrength[i]);
    }

    return HI_SUCCESS;
}

static HI_VOID AntiFalseColorRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S  *pstRegCfg)
{
    HI_U32 i;
    ISP_ANTIFALSECOLOR_STATIC_CFG_S *pstStaticRegCfg = HI_NULL;
    ISP_ANTIFALSECOLOR_DYNA_CFG_S   *pstDynaRegCfg   = HI_NULL;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stAntiFalseColorRegCfg.bFcrEnable = HI_ISP_DEMOSAIC_FCR_EN_DEFAULT;
        pstStaticRegCfg = &pstRegCfg->stAlgRegCfg[i].stAntiFalseColorRegCfg.stStaticRegCfg;
        pstDynaRegCfg   = &pstRegCfg->stAlgRegCfg[i].stAntiFalseColorRegCfg.stDynaRegCfg;

        //static
        pstStaticRegCfg->bResh        = HI_TRUE;
        pstStaticRegCfg->u16FcrLimit1 = HI_ISP_DEMOSAIC_FCR_LIMIT1_DEFAULT;
        pstStaticRegCfg->u16FcrLimit2 = HI_ISP_DEMOSAIC_FCR_LIMIT2_DEFAULT;
        pstStaticRegCfg->u16FcrThr    = HI_ISP_DEMOSAIC_FCR_THR_DEFAULT;

        /*dynamic*/
        pstDynaRegCfg->bResh             = HI_TRUE;
        pstDynaRegCfg->u8FcrGain         = HI_ISP_DEMOSAIC_FCR_GAIN_DEFAULT;
        pstDynaRegCfg->u8FcrRatio        = HI_ISP_DEMOSAIC_FCR_RATIO_DEFAULT;
        pstDynaRegCfg->u8FcrGrayRatio    = HI_ISP_DEMOSAIC_FCR_GRAY_RATIO_DEFAULT;
        pstDynaRegCfg->u8FcrCmaxSel      = HI_ISP_DEMOSAIC_FCR_CMAX_SEL_DEFAULT;
        pstDynaRegCfg->u8FcrDetgSel      = HI_ISP_DEMOSAIC_FCR_DETG_SEL_DEFAULT;
        pstDynaRegCfg->u16FcrHfThreshLow = HI_ISP_DEMOSAIC_FCR_HF_THRESH_LOW_DEFAULT;
        pstDynaRegCfg->u16FcrHfThreshHig = HI_ISP_DEMOSAIC_FCR_HF_THRESH_HIGH_DEFAULT;
    }

    pstRegCfg->unKey.bit1FcrCfg = 1;
}

static HI_S32 AntiFalseColorReadExtregs(VI_PIPE ViPipe)
{
    HI_U32 i;
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;

    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);

    pstAntiFalseColor->enOpType   = hi_ext_system_antifalsecolor_manual_mode_read(ViPipe);

    if (OP_TYPE_AUTO == pstAntiFalseColor->enOpType)
    {
        for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)
        {
            pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold[i] = hi_ext_system_antifalsecolor_auto_threshold_read(ViPipe, i);
            pstAntiFalseColor->stAuto.au8AntiFalseColorStrength[i]  = hi_ext_system_antifalsecolor_auto_strenght_read(ViPipe, i);
        }
    }
    else if (OP_TYPE_MANUAL == pstAntiFalseColor->enOpType)
    {
        pstAntiFalseColor->stManual.u8AntiFalseColorThreshold       = hi_ext_system_antifalsecolor_manual_threshold_read(ViPipe);
        pstAntiFalseColor->stManual.u8AntiFalseColorStrength        = hi_ext_system_antifalsecolor_manual_strenght_read(ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AntiFalseColorInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_REG_CFG_S  *pstRegCfg  = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = AntiFalseColorCtxInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    AntiFalseColorRegsInitialize(ViPipe, pstRegCfg);
    s32Ret = AntiFalseColorExtRegsInitialize(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_BOOL __inline CheckAntiFalseColorOpen(ISP_ANTIFALSECOLOR_S *pstAntiFalseColor)
{
    return (HI_TRUE == pstAntiFalseColor->bEnable);
}

static HI_VOID AntiFalseColorCfg(ISP_ANTIFALSECOLOR_DYNA_CFG_S *pstAntiFalseColorDynaCfg, ISP_ANTIFALSECOLOR_S *pstAntiFalseColor, HI_U32 u32Iso, \
    HI_U32 u32ISO2, HI_U32 u32ISO1, HI_U8 u8IsoIndexUpper, HI_U8 u8IsoIndexLower)
{
    pstAntiFalseColorDynaCfg->u8FcrGrayRatio    = (HI_U8)LinearInter( u32Iso, u32ISO1, pstAntiFalseColor->au8LutAntiFalseColorGrayRatio[u8IsoIndexLower], \
                                                                              u32ISO2, pstAntiFalseColor->au8LutAntiFalseColorGrayRatio[u8IsoIndexUpper]);
    pstAntiFalseColorDynaCfg->u8FcrCmaxSel      = (HI_U8)LinearInter( u32Iso, u32ISO1, pstAntiFalseColor->au8LutAntiFalseColorCmaxSel[u8IsoIndexLower], \
                                                                              u32ISO2, pstAntiFalseColor->au8LutAntiFalseColorCmaxSel[u8IsoIndexUpper]);
    pstAntiFalseColorDynaCfg->u8FcrDetgSel      = (HI_U8)LinearInter( u32Iso, u32ISO1, pstAntiFalseColor->au8LutAntiFalseColorDetgSel[u8IsoIndexLower], \
                                                                              u32ISO2, pstAntiFalseColor->au8LutAntiFalseColorDetgSel[u8IsoIndexUpper]);
    pstAntiFalseColorDynaCfg->u16FcrHfThreshLow = (HI_U16)LinearInter(u32Iso, u32ISO1, pstAntiFalseColor->au16AntiFalseColorHfThreshLow[u8IsoIndexLower],\
                                                                              u32ISO2, pstAntiFalseColor->au16AntiFalseColorHfThreshLow[u8IsoIndexUpper]);
    pstAntiFalseColorDynaCfg->u16FcrHfThreshHig = (HI_U16)LinearInter(u32Iso, u32ISO1, pstAntiFalseColor->au16AntiFalseColorHfThreshHig[u8IsoIndexLower],\
                                                                              u32ISO2, pstAntiFalseColor->au16AntiFalseColorHfThreshHig[u8IsoIndexUpper]);
    pstAntiFalseColorDynaCfg->bResh = HI_TRUE;
}

static HI_S32 ISP_AntiFalseColor_Fw(HI_U32 u32Iso, VI_PIPE ViPipe, HI_U8 u8CurBlk, ISP_REG_CFG_S *pstReg)
{
    HI_U8  u8IsoIndexUpper, u8IsoIndexLower;
    HI_U32 u32ISO1 = 0;
    HI_U32 u32ISO2 = 0;
    ISP_ANTIFALSECOLOR_DYNA_CFG_S *pstAntiFalseColorDynaCfg = &pstReg->stAlgRegCfg[u8CurBlk].stAntiFalseColorRegCfg.stDynaRegCfg;
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;

    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);

    u8IsoIndexUpper = GetIsoIndex(u32Iso);
    u8IsoIndexLower = MAX2((HI_S8)u8IsoIndexUpper - 1, 0);
    u32ISO1 = g_au32IsoLut[u8IsoIndexLower];
    u32ISO2 = g_au32IsoLut[u8IsoIndexUpper];

    AntiFalseColorCfg(pstAntiFalseColorDynaCfg, pstAntiFalseColor, u32Iso, u32ISO2, u32ISO1, u8IsoIndexUpper, u8IsoIndexLower);

    if (OP_TYPE_AUTO == pstAntiFalseColor->enOpType)
    {
        pstAntiFalseColorDynaCfg->u8FcrRatio = (HI_U8)LinearInter(u32Iso, u32ISO1, pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold[u8IsoIndexLower], \
                                                                          u32ISO2, pstAntiFalseColor->stAuto.au8AntiFalseColorThreshold[u8IsoIndexUpper]);
        pstAntiFalseColorDynaCfg->u8FcrGain  = (HI_U8)LinearInter(u32Iso, u32ISO1, pstAntiFalseColor->stAuto.au8AntiFalseColorStrength[u8IsoIndexLower],\
                                                                          u32ISO2, pstAntiFalseColor->stAuto.au8AntiFalseColorStrength[u8IsoIndexUpper]);
    }
    else if (OP_TYPE_MANUAL == pstAntiFalseColor->enOpType)
    {
        pstAntiFalseColorDynaCfg->u8FcrRatio = pstAntiFalseColor->stManual.u8AntiFalseColorThreshold;
        pstAntiFalseColorDynaCfg->u8FcrGain  = pstAntiFalseColor->stManual.u8AntiFalseColorStrength;
    }

    pstAntiFalseColorDynaCfg->bResh = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_AntiFalseColorRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8 i;
    ISP_REG_CFG_S *pstReg = (ISP_REG_CFG_S *)pRegCfg;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ANTIFALSECOLOR_S *pstAntiFalseColor = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ANTIFALSECOLOR_GET_CTX(ViPipe, pstAntiFalseColor);
    ISP_CHECK_POINTER(pstAntiFalseColor);

    /* calculate every two interrupts */
    if ((0 != pstIspCtx->u32FrameCnt % 2) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState))
    {
        return HI_SUCCESS;
    }

    pstAntiFalseColor->bEnable = hi_ext_system_antifalsecolor_enable_read(ViPipe);

    for (i = 0; i < pstReg->u8CfgNum; i++)
    {
        pstReg->stAlgRegCfg[i].stAntiFalseColorRegCfg.bFcrEnable = pstAntiFalseColor->bEnable;
    }

    pstReg->unKey.bit1FcrCfg = 1;

    /*check hardware setting*/
    if (!CheckAntiFalseColorOpen(pstAntiFalseColor))
    {
        return HI_SUCCESS;
    }
	
    if (pstIspCtx->stLinkage.enFSWDRMode != pstIspCtx->stLinkage.enPreFSWDRMode )
    {
        AntiFalseColor_SetLongFrameMode(ViPipe);
    }
	
    AntiFalseColorReadExtregs(ViPipe);

    for (i = 0; i < pstReg->u8CfgNum; i++)
    {
        ISP_AntiFalseColor_Fw(pstIspCtx->stLinkage.u32Iso, ViPipe, i , pstReg);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AntiFalseColorCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);

            ISP_AntiFalseColorInit(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;

        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AntiFalseColorExit(VI_PIPE ViPipe)
{
    HI_U16 i;
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stAntiFalseColorRegCfg.bFcrEnable = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1FcrCfg = 1;

    AntiFalseColorCtxExit(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterFcr(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Fcr);

    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_ANTIFALSECOLOR;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_AntiFalseColorInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_AntiFalseColorRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_AntiFalseColorCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_AntiFalseColorExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
