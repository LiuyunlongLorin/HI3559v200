/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_hlc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2017/12/16
    Author      :
    Modification: Created file

******************************************************************************/
#include "isp_config.h"
#include "hi_isp_debug.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_HLC_S
{
    HI_BOOL bHlcMpiUpdateEn;
    HI_BOOL bEnable;                        /*RW; Range:[0, 1]; Format:1.0;Enable/Disable Edge Mark*/
    HI_U8   u8LumaThr;                      /* RW; range: [0, 255];  Format:8.0;*/
    HI_U8   u8LumaTarget;                   /* RW; range: [0, 255];  Format:8.0;*/
} ISP_HLC_S;

ISP_HLC_S g_astHlcCtx[ISP_MAX_PIPE_NUM] = {{0}};
#define HLC_GET_CTX(dev, pstCtx)   pstCtx = &g_astHlcCtx[dev]

static HI_VOID HlcExtRegsInitialize(VI_PIPE ViPipe)
{
    ISP_HLC_S *pstHlc = HI_NULL;

    HLC_GET_CTX(ViPipe, pstHlc);

    hi_ext_system_hlc_mpi_update_en_write(ViPipe, HI_FALSE);
    hi_ext_system_manual_isp_hlc_en_write(ViPipe, pstHlc->bEnable);
    hi_ext_system_manual_isp_hlc_lumathr_write(ViPipe, pstHlc->u8LumaThr);
    hi_ext_system_manual_isp_hlc_lumatarget_write(ViPipe, pstHlc->u8LumaTarget);
}

static void HlcCheckReg(ISP_HLC_REG_CFG_S *pstHlcReg)
{
    pstHlcReg->bEnable = MIN2( pstHlcReg->bEnable, 0x1 );
}

static HI_VOID HlcRegInit(ISP_HLC_REG_CFG_S *pstHlcReg, ISP_HLC_S *pstHlc)
{
    pstHlcReg->u8yMaxValue     = pstHlc->u8LumaThr;
    pstHlcReg->u8yMaxLoadValue = pstHlc->u8LumaTarget;
    pstHlcReg->u8yMinValue     = 0;
    pstHlcReg->u8yMinLoadValue = 0;
    pstHlcReg->u8cMaxValue     = 255;
    pstHlcReg->u8cMaxLoadValue = 255;
    pstHlcReg->u8cMinValue     = 0;
    pstHlcReg->u8cMinLoadValue = 0;
    HlcCheckReg(pstHlcReg);
}

static HI_VOID HlcRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pRegCfg)
{
    HI_U32 i;

    ISP_HLC_S *pstHlc = HI_NULL;

    HLC_GET_CTX(ViPipe, pstHlc);

    for (i = 0; i < pRegCfg->u8CfgNum; i++)
    {
        pRegCfg->stAlgRegCfg[i].stHlcRegCfg.bEnable = pstHlc->bEnable;
        HlcRegInit(&pRegCfg->stAlgRegCfg[i].stHlcRegCfg, pstHlc);
    }

    pRegCfg->unKey.bit1HlcCfg = 1;
}

static HI_S32 HlcReadExtregs(VI_PIPE ViPipe)
{
    ISP_HLC_S *pstHlc = HI_NULL;

    HLC_GET_CTX(ViPipe, pstHlc);
    pstHlc->bHlcMpiUpdateEn = hi_ext_system_hlc_mpi_update_en_read(ViPipe);
    hi_ext_system_hlc_mpi_update_en_write(ViPipe, HI_FALSE);

    if (pstHlc->bHlcMpiUpdateEn)
    {
        pstHlc->u8LumaThr    = hi_ext_system_manual_isp_hlc_lumathr_read(ViPipe);
        pstHlc->u8LumaTarget = hi_ext_system_manual_isp_hlc_lumatarget_read(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_S32 HlcCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_HLC_S *pstCmosHlc)
{
    ISP_CHECK_BOOL(pstCmosHlc->bEnable);

    return HI_SUCCESS;
}

static HI_S32 HlcInitialize(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_HLC_S          *pstHlc    = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;

    HLC_GET_CTX(ViPipe, pstHlc);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    if (pstSnsDft->unKey.bit1Hlc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstHlc);

        s32Ret = HlcCheckCmosParam(ViPipe, pstSnsDft->pstHlc);

        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstHlc->bEnable      = pstSnsDft->pstHlc->bEnable;
        pstHlc->u8LumaThr    = pstSnsDft->pstHlc->u8LumaThr;
        pstHlc->u8LumaTarget = pstSnsDft->pstHlc->u8LumaTarget;
    }
    else
    {
        pstHlc->bEnable      = HI_FALSE;
        pstHlc->u8LumaThr    = 248;
        pstHlc->u8LumaTarget = 10;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_HlcInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HlcInitialize(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    HlcRegsInitialize(ViPipe, (ISP_REG_CFG_S *)pRegCfg);
    HlcExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

static void HlcMPI2Reg(ISP_HLC_REG_CFG_S *pstHlcRegCfg, ISP_HLC_S *pstHlc)
{
    pstHlcRegCfg->u8yMaxValue     = pstHlc->u8LumaThr;
    pstHlcRegCfg->u8yMaxLoadValue = pstHlc->u8LumaTarget;

    HlcCheckReg(pstHlcRegCfg);
}

static HI_BOOL __inline CheckHlcOpen(ISP_HLC_S *pstHlc)
{
    return (HI_TRUE == pstHlc->bEnable);
}

HI_S32 HlcProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    ISP_CTRL_PROC_WRITE_S stProcTmp;

    ISP_HLC_S *pstHlc = HI_NULL;

    HLC_GET_CTX(ViPipe, pstHlc);

    if ((HI_NULL == pstProc->pcProcBuff) || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }


    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----HLC INFO--------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16s"    "%16s"        "%16s\n",
                    "Enable", "LumaThr", "LumaTarget");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16u"  "%16u"      "%16u\n",
                    (HI_U16)pstHlc->bEnable,
                    (HI_U16)pstHlc->u8LumaThr,
                    (HI_U16)pstHlc->u8LumaTarget
                   );

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

HI_S32 ISP_HlcRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8  i;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_HLC_S *pstHlc = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    HLC_GET_CTX(ViPipe, pstHlc);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    /* calculate every two interrupts */
    if ((0 != pstIspCtx->u32FrameCnt % 2) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState))
    {
        return HI_SUCCESS;
    }

    pstHlc->bEnable = hi_ext_system_manual_isp_hlc_en_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stHlcRegCfg.bEnable = pstHlc->bEnable;
    }

    pstRegCfg->unKey.bit1HlcCfg = 1;

    /*check hardware setting*/
    if (!CheckHlcOpen(pstHlc))
    {
        return HI_SUCCESS;
    }

    HlcReadExtregs(ViPipe);

    if (pstHlc->bHlcMpiUpdateEn)
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            HlcMPI2Reg(&(pstRegCfg->stAlgRegCfg[i].stHlcRegCfg), pstHlc);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_HlcCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    switch (u32Cmd)
    {
        case ISP_PROC_WRITE:
            HlcProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;

        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_HlcExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stHlcRegCfg.bEnable = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1HlcCfg = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterHlc(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Hlc);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType =  ISP_ALG_EDGEAMRK;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_HlcInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_HlcRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_HlcCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_HlcExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
