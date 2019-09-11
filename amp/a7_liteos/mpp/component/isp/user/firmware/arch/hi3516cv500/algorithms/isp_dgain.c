/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_dgain.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2016/12/21
  Description   :
  History       :
  1.Date        : 2016/12/21
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

typedef struct hiISP_DGain
{
    HI_BOOL bEnable;
} ISP_DG_S;

ISP_DG_S g_astDgCtx[ISP_MAX_PIPE_NUM] = {{0}};
#define DG_GET_CTX(dev, pstCtx)   pstCtx = &g_astDgCtx[dev]

static HI_VOID BeDgStaticInit(VI_PIPE ViPipe, ISP_DG_STATIC_CFG_S *pstStaticRegCfg)
{
    pstStaticRegCfg->bResh = HI_TRUE;
}

static HI_VOID BeDgDynaInit(VI_PIPE ViPipe, ISP_DG_DYNA_CFG_S *pstDynaRegCfg)
{
    pstDynaRegCfg->u32ClipValue = 0xFFFFF;
    pstDynaRegCfg->u16GainR     = 0x100;
    pstDynaRegCfg->u16GainGR    = 0x100;
    pstDynaRegCfg->u16GainGB    = 0x100;
    pstDynaRegCfg->u16GainB     = 0x100;
    pstDynaRegCfg->bResh        = HI_TRUE;
}

static HI_VOID WDRDgStaticInit(VI_PIPE ViPipe, HI_U8 i, ISP_4DG_STATIC_CFG_S *pstStaticRegCfg)
{
    pstStaticRegCfg->u16GainR0  = 0x100;
    pstStaticRegCfg->u16GainGR0 = 0x100;
    pstStaticRegCfg->u16GainGB0 = 0x100;
    pstStaticRegCfg->u16GainB0  = 0x100;
    pstStaticRegCfg->u16GainR1  = 0x100;
    pstStaticRegCfg->u16GainGR1 = 0x100;
    pstStaticRegCfg->u16GainGB1 = 0x100;
    pstStaticRegCfg->u16GainB1  = 0x100;
    pstStaticRegCfg->u16GainR2  = 0x100;
    pstStaticRegCfg->u16GainGR2 = 0x100;
    pstStaticRegCfg->u16GainGB2 = 0x100;
    pstStaticRegCfg->u16GainB2  = 0x100;
    pstStaticRegCfg->u16GainR3  = 0x100;
    pstStaticRegCfg->u16GainGR3 = 0x100;
    pstStaticRegCfg->u16GainGB3 = 0x100;
    pstStaticRegCfg->u16GainB3  = 0x100;
    pstStaticRegCfg->bResh      = HI_TRUE;
}

static HI_VOID WDRDgDynaInit(VI_PIPE ViPipe, ISP_4DG_DYNA_CFG_S *pstDynaRegCfg)
{
    pstDynaRegCfg->u32ClipValue0  = 0xFFFFF;
    pstDynaRegCfg->u32ClipValue1  = 0xFFFFF;
    pstDynaRegCfg->u32ClipValue2  = 0xFFFFF;
    pstDynaRegCfg->u32ClipValue3  = 0xFFFFF;
    pstDynaRegCfg->bResh          = HI_TRUE;
}

static HI_VOID FeDgDynaInit(VI_PIPE ViPipe, ISP_FE_DG_DYNA_CFG_S *pstDynaRegCfg)
{
    HI_S32 i = 0;

    for (i = 0; i < 4; i++)
    {
        pstDynaRegCfg->au16GainR[i]  = 0x100;
        pstDynaRegCfg->au16GainGR[i] = 0x100;
        pstDynaRegCfg->au16GainGB[i] = 0x100;
        pstDynaRegCfg->au16GainB[i]  = 0x100;
    }
    pstDynaRegCfg->u32ClipValue  = 0xFFFFF;
    pstDynaRegCfg->bResh         = HI_TRUE;
}

static HI_VOID WDRDgEnInit(VI_PIPE ViPipe, HI_U8 i, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 u8WDRMode = 0;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8WDRMode = pstIspCtx->u8SnsWDRMode;

    if (IS_LINEAR_MODE(u8WDRMode))
    {
        pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.bEnable = HI_FALSE;
    }
    else if (IS_2to1_WDR_MODE(u8WDRMode) || IS_3to1_WDR_MODE(u8WDRMode) || IS_4to1_WDR_MODE(u8WDRMode) )
    {
        pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.bEnable = HI_TRUE;
    }
    else if (IS_BUILT_IN_WDR_MODE(u8WDRMode))
    {
        pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.bEnable = HI_FALSE;
    }
}

static HI_VOID DgRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 i;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        BeDgStaticInit(ViPipe, &pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stStaticRegCfg);
        BeDgDynaInit(ViPipe, &pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg);

        WDRDgStaticInit(ViPipe, i, &pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.stStaticRegCfg);
        WDRDgDynaInit(ViPipe, &pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.stDynaRegCfg);

        WDRDgEnInit(ViPipe, i, pstRegCfg);

        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.bDgEn = HI_TRUE;
        pstRegCfg->unKey.bit1DgCfg                 = 1;
        pstRegCfg->unKey.bit1WDRDgCfg              = 1;
    }

    {
        FeDgDynaInit(ViPipe, &pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg);
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.bDgEn = HI_TRUE;
        pstRegCfg->unKey.bit1FeDgCfg                 = 1;
    }
}

static HI_VOID DgExtRegsInitialize(VI_PIPE ViPipe)
{
    hi_ext_system_isp_dgain_enable_write(ViPipe, HI_TRUE);
}

static HI_VOID DgInitialize(VI_PIPE ViPipe)
{
    ISP_DG_S *pstDg     = HI_NULL;

    DG_GET_CTX(ViPipe, pstDg);

    pstDg->bEnable = HI_TRUE;
}

static HI_VOID ISP_DgWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_U8  i, j;
    HI_U16 u16Gain;
    ISP_REG_CFG_S           *pstRegCfg        = (ISP_REG_CFG_S *)pRegCfg;
    ISP_CMOS_BLACK_LEVEL_S  *pstSnsBlackLevel = HI_NULL;

    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);

    u16Gain = CLIP3(4095 * 256 / DIV_0_TO_1(4095 - pstSnsBlackLevel->au16BlackLevel[1]) + 1, 0x100, 0x200);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++) {
        WDRDgEnInit(ViPipe, i, pstRegCfg);
        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stStaticRegCfg.bResh   = HI_TRUE;
        pstRegCfg->stAlgRegCfg[i].st4DgRegCfg.stStaticRegCfg.bResh  = HI_TRUE;

        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg.u16GainR  = u16Gain;
        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg.u16GainGR = u16Gain;
        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg.u16GainGB = u16Gain;
        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.stDynaRegCfg.u16GainB  = u16Gain;
    }

    pstRegCfg->unKey.bit1WDRDgCfg = 1;
    pstRegCfg->unKey.bit1FeDgCfg  = 1;
    pstRegCfg->unKey.bit1DgCfg    = 1;

    for (j = 0; j < 4; j++) {
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainR[j]  = u16Gain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainGR[j] = u16Gain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainGB[j] = u16Gain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainB[j]  = u16Gain;
    }
}

HI_S32 ISP_DgInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    DgRegsInitialize(ViPipe, pstRegCfg);
    DgExtRegsInitialize(ViPipe);
    DgInitialize(ViPipe);

    return HI_SUCCESS;
}

static HI_BOOL __inline CheckDgOpen(ISP_DG_S *pstDg)
{
    return (HI_TRUE == pstDg->bEnable);
}

HI_S32 ISP_DgRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                 HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_S32 i;
    HI_U32 u32IspDgain;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_DG_S  *pstDg     = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;
    HI_U32 u32WDRGain;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    DG_GET_CTX(ViPipe, pstDg);

    if (pstIspCtx->stLinkage.bDefectPixel && (0 == hi_ext_system_dpc_static_defect_type_read(ViPipe)))
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstRegCfg->stAlgRegCfg[i].stDgRegCfg.bDgEn = HI_FALSE;
        }

        pstRegCfg->unKey.bit1DgCfg   = 1;

        return HI_SUCCESS;
    }

    pstDg->bEnable = hi_ext_system_isp_dgain_enable_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stDgRegCfg.bDgEn = pstDg->bEnable;
    }

    pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.bDgEn = pstDg->bEnable;

    pstRegCfg->unKey.bit1FeDgCfg = 1;
    pstRegCfg->unKey.bit1DgCfg   = 1;

    /*check hardware setting*/
    if (!CheckDgOpen(pstDg))
    {
        return HI_SUCCESS;
    }

    u32IspDgain = pstIspCtx->stLinkage.u32IspDgain;

    for(i = 0; i < 4; i++)
    {
        u32WDRGain = ((HI_U64)u32IspDgain * pstIspCtx->stLinkage.au32WDRGain[i]) >> 8;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainR[i]  = u32WDRGain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainGR[i] = u32WDRGain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainGB[i] = u32WDRGain;
        pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.au16GainB[i]  = u32WDRGain;
    }

    pstRegCfg->stAlgRegCfg[0].stFeDgRegCfg.stDynaRegCfg.bResh     = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 ISP_DgCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_DgWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_DgExit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterDg(VI_PIPE ViPipe)
{
    ISP_CTX_S      *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Dg);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_DG;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_DgInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_DgRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_DgCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_DgExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
