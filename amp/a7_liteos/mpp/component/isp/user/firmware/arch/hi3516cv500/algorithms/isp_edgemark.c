/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_edgemark.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2017/01/16
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

typedef struct hiISP_EDGEMARK_S
{
    HI_BOOL bEdgeMarkMpiUpdateEn;
    HI_BOOL bEnable;               /*RW; Range:[0, 1]; Format:1.0;Enable/Disable Edge Mark*/
    HI_U8   u8Threshold;           /* RW; Range: [0, 255];  Format:8.0;*/
    HI_U32  u32Color;              /* RW; Range: [0, 0xFFFFFF];  Format:32.0;*/
} ISP_EDGEMARK_S;

ISP_EDGEMARK_S g_astEdgeMarkCtx[ISP_MAX_PIPE_NUM] = {{0}};
#define EDGEMARK_GET_CTX(dev, pstCtx)   pstCtx = &g_astEdgeMarkCtx[dev]

static HI_VOID EdgeMarkExtRegsInitialize(VI_PIPE ViPipe)
{
    ISP_EDGEMARK_S *pstEdgeMark = HI_NULL;

    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);

    hi_ext_system_edgemark_mpi_update_en_write(ViPipe, HI_TRUE);
    hi_ext_system_manual_isp_edgemark_en_write(ViPipe, pstEdgeMark->bEnable);
    hi_ext_system_manual_isp_edgemark_color_write(ViPipe, pstEdgeMark->u32Color);
    hi_ext_system_manual_isp_edgemark_threshold_write(ViPipe, pstEdgeMark->u8Threshold);
}

static void EdgeMarkCheckReg(ISP_EDGEMARK_REG_CFG_S *pstEdgeMarkReg)
{
    pstEdgeMarkReg->bEnable       = MIN2( pstEdgeMarkReg->bEnable, 0x1);
    pstEdgeMarkReg->u8markEdgeSft = MIN2( pstEdgeMarkReg->u8markEdgeSft, 12);
    pstEdgeMarkReg->u8uMarkValue  = MIN2( pstEdgeMarkReg->u8uMarkValue, 255);
    pstEdgeMarkReg->u8vMarkValue  = MIN2( pstEdgeMarkReg->u8vMarkValue, 255);
}

//****Sharpen hardware Regs that will change with MPI and ISO****//
static HI_VOID EdgeMarkRegInit(VI_PIPE ViPipe, ISP_EDGEMARK_REG_CFG_S *pstEdgeMarkReg)
{
    ISP_EDGEMARK_S *pstEdgeMark = HI_NULL;

    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);

    pstEdgeMarkReg->bEnable        = pstEdgeMark->bEnable;
    pstEdgeMarkReg->u8markEdgeSft  = 0;
    pstEdgeMarkReg->u8MarkEdgeThd  = pstEdgeMark->u8Threshold;
    pstEdgeMarkReg->u8uMarkValue   = 120;
    pstEdgeMarkReg->u8vMarkValue   = 220;
    pstEdgeMarkReg->u32UpdateIndex = 1;
    EdgeMarkCheckReg(pstEdgeMarkReg);
}

static HI_VOID EdgeMarkRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pRegCfg)
{
    HI_U32 i;

    for (i = 0; i < pRegCfg->u8CfgNum; i++)
    {
        EdgeMarkRegInit(ViPipe, &pRegCfg->stAlgRegCfg[i].stEdgeMarkRegCfg);
    }

    pRegCfg->unKey.bit1EdgeMarkCfg = 1;
}

static HI_S32 EdgeMarkReadExtregs(VI_PIPE ViPipe)
{
    ISP_EDGEMARK_S *pstEdgeMark = HI_NULL;

    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);
    pstEdgeMark->bEdgeMarkMpiUpdateEn = hi_ext_system_edgemark_mpi_update_en_read(ViPipe);

    hi_ext_system_edgemark_mpi_update_en_write(ViPipe, HI_FALSE);

    if (pstEdgeMark->bEdgeMarkMpiUpdateEn)
    {
        pstEdgeMark->u32Color    = hi_ext_system_manual_isp_edgemark_color_read(ViPipe);
        pstEdgeMark->u8Threshold = hi_ext_system_manual_isp_edgemark_threshold_read(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_S32 EdgeMarkCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_EDGEMARK_S *pstEdgeMark)
{
    ISP_CHECK_BOOL(pstEdgeMark->bEnable);

    if (pstEdgeMark->u32Color > 0xFFFFFF)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32Color!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 EdgeMarkInitialize(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_EDGEMARK_S     *pstEdgeMark = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft   = HI_NULL;

    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    if (pstSnsDft->unKey.bit1EdgeMark)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstEdgeMark);

        s32Ret = EdgeMarkCheckCmosParam(ViPipe, pstSnsDft->pstEdgeMark);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstEdgeMark->bEnable     = pstSnsDft->pstEdgeMark->bEnable;
        pstEdgeMark->u8Threshold = pstSnsDft->pstEdgeMark->u8Threshold;
        pstEdgeMark->u32Color    = pstSnsDft->pstEdgeMark->u32Color;
    }
    else
    {
        pstEdgeMark->bEnable     = HI_FALSE;
        pstEdgeMark->u8Threshold = 100;
        pstEdgeMark->u32Color    = 0xFF0000;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_EdgeMarkInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret;

    s32Ret = EdgeMarkInitialize(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    EdgeMarkRegsInitialize(ViPipe, (ISP_REG_CFG_S *)pRegCfg);
    EdgeMarkExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

static void EdgeMarkRgb2Uv(HI_U8 u8R, HI_U8 u8G, HI_U8 u8B,  HI_U8 *u,  HI_U8 *v)
{
    HI_U8 i;
    /* RGB -> BT.709 (i.e. HD) */
    const HI_S16 cscMat[6] =
    {
        //R    G   B
        //183, 614, 62,       //Y
        -101, -338, 439, //U
        439, -399, -40      //V
    };//range[16,235]

    HI_S16 cscCoef[6];

    for (i = 0; i < 6; i++)
    {
        cscCoef[i] = cscMat[i] * 1024 / 1000;
    }

    *u = (HI_U8)(CLIP3(SignedRightShift((cscCoef[0] * u8R + cscCoef[1] * u8G + cscCoef[2] * u8B + (128 << 10)), 10), 0, 255));
    *v = (HI_U8)(CLIP3(SignedRightShift((cscCoef[3] * u8R + cscCoef[4] * u8G + cscCoef[5] * u8B + (128 << 10)), 10), 0, 255));

}

static void EdgeMarkMPI2Reg(ISP_EDGEMARK_REG_CFG_S *pstEdgeMarkRegCfg, ISP_EDGEMARK_S *pstEdgeMark)
{
    HI_U8   u8RValue, u8GValue, u8BValue;

    pstEdgeMarkRegCfg->bEnable       = pstEdgeMark->bEnable;
    pstEdgeMarkRegCfg->u8MarkEdgeThd = pstEdgeMark->u8Threshold;
    pstEdgeMarkRegCfg->u8markEdgeSft = 0;

    u8BValue = (HI_U8)((pstEdgeMark->u32Color) & 0xFF);
    u8GValue = (HI_U8)((pstEdgeMark->u32Color >> 8) & 0xFF);
    u8RValue = (HI_U8)((pstEdgeMark->u32Color >> 16) & 0xFF);

    EdgeMarkRgb2Uv(u8RValue, u8GValue, u8BValue, &(pstEdgeMarkRegCfg->u8uMarkValue), &(pstEdgeMarkRegCfg->u8vMarkValue));

    EdgeMarkCheckReg(pstEdgeMarkRegCfg);
}

static HI_BOOL __inline CheckEdgeMarkOpen(ISP_EDGEMARK_S *pstEdgeMark)
{
    return (HI_TRUE == pstEdgeMark->bEnable);
}

HI_S32 EdgeMarkProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    ISP_CTRL_PROC_WRITE_S stProcTmp;

    ISP_EDGEMARK_S *pstEdgeMark = HI_NULL;

    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);

    if ((HI_NULL == pstProc->pcProcBuff) || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----EDGEMARK INFO--------------------------------------------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16s"    "%16s"        "%16s\n",
                    "bEnable", "Threshold", "Color");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%16u"  "%16u"      "%16u\n",
                    (HI_U16)pstEdgeMark->bEnable,
                    (HI_U16)pstEdgeMark->u8Threshold,
                    (HI_U32)pstEdgeMark->u32Color
                   );

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

HI_S32 ISP_EdgeMarkRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8  i;
    ISP_CTX_S      *pstIspCtx   = HI_NULL;
    ISP_EDGEMARK_S *pstEdgeMark = HI_NULL;
    ISP_REG_CFG_S  *pstRegCfg   = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    EDGEMARK_GET_CTX(ViPipe, pstEdgeMark);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    /* calculate every two interrupts */
    if ((0 != pstIspCtx->u32FrameCnt % 2) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState))
    {
        return HI_SUCCESS;
    }

    pstEdgeMark->bEnable = hi_ext_system_manual_isp_edgemark_en_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stEdgeMarkRegCfg.bEnable = pstEdgeMark->bEnable;
    }

    pstRegCfg->unKey.bit1EdgeMarkCfg = 1;

    /*check hardware setting*/
    if (!CheckEdgeMarkOpen(pstEdgeMark))
    {
        return HI_SUCCESS;
    }

    EdgeMarkReadExtregs(ViPipe);

    if (pstEdgeMark->bEdgeMarkMpiUpdateEn)
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstRegCfg->stAlgRegCfg[i].stEdgeMarkRegCfg.u32UpdateIndex      += 1;
            EdgeMarkMPI2Reg(&(pstRegCfg->stAlgRegCfg[i].stEdgeMarkRegCfg), pstEdgeMark);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_EdgeMarkCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    switch (u32Cmd)
    {
        case ISP_PROC_WRITE:
            EdgeMarkProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_EdgeMarkExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stEdgeMarkRegCfg.bEnable = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1EdgeMarkCfg = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterEdgeMark(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1EdgeMark);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType =  ISP_ALG_EDGEAMRK;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_EdgeMarkInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_EdgeMarkRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_EdgeMarkCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_EdgeMarkExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
