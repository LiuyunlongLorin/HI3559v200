/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_mcds.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2014/01/16
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

#define MCDS_EN             (1)
#define MCDS_FILTER_MODE    (1)     //1: filter mode; 0: discard mode

static HI_VOID McdsStaticRegInit(VI_PIPE ViPipe, ISP_MCDS_STATIC_REG_CFG_S *pstStaticRegCfg, ISP_CTX_S *pstIspCtx)
{
    static const HI_S16 Coeff_Filter_8tap_H[2][8] = {{ -16, 0, 145, 254, 145, 0, -16, 0}, {0, 0, 0, 256, 256, 0, 0, 0}};
    static const HI_S16 Coeff_Discard_8pixel_H[8] = {0, 0, 0, 512, 0, 0, 0, 0};

    static const HI_S8 Coeff_Filer_4tap_V[2][4] = {{4, 4, 6, 6}, {3, 3, 3, 3}};
    static const HI_S8 Coeff_Discard_4tap_V[4]  = {5, 6, 6, 6};

    if (MCDS_FILTER_MODE) //Filter Mode
    {
        memcpy(pstStaticRegCfg->as16HCoef, Coeff_Filter_8tap_H[0], 8 * sizeof(HI_S16));//SDR mode
        memcpy(pstStaticRegCfg->as8VCoef,  Coeff_Filer_4tap_V[0],  2 * sizeof(HI_S8));
    }
    else        //discard Mode
    {
        memcpy(pstStaticRegCfg->as16HCoef, Coeff_Discard_8pixel_H, 8 * sizeof(HI_S16));
        memcpy(pstStaticRegCfg->as8VCoef, Coeff_Discard_4tap_V, 2 * sizeof(HI_S8));
    }

    pstStaticRegCfg->bHcdsEn        = 1;
    pstStaticRegCfg->u16CoringLimit = 0;
    pstStaticRegCfg->u8MidfBldr     = 8;
    pstStaticRegCfg->bStaticResh    = HI_TRUE;
}

static HI_VOID McdsDynaRegInit(ISP_MCDS_DYNA_REG_CFG_S *pstDynaRegCfg, ISP_CTX_S *pstIspCtx)
{
    pstDynaRegCfg->bMidfEn   = 1;
    pstDynaRegCfg->bDynaResh = 1;
    if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pstIspCtx->stHdrAttr.enFormat)
    {
        pstDynaRegCfg->bVcdsEn    = HI_FALSE;
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstIspCtx->stHdrAttr.enFormat)
    {
        pstDynaRegCfg->bVcdsEn    = HI_TRUE;      //422: 0; 420: 1
    }
    else //400 is the same with 420
    {
        pstDynaRegCfg->bVcdsEn    = HI_TRUE;
    }
}

static HI_VOID McdsRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pRegCfg)
{
    HI_U32 i;

    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    for (i = 0; i < pRegCfg->u8CfgNum; i++)
    {
        pRegCfg->stAlgRegCfg[i].stMcdsRegCfg.bMCDSen = HI_TRUE;
        McdsStaticRegInit(ViPipe, &(pRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stStaticRegCfg), pstIspCtx);
        McdsDynaRegInit(&(pRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg), pstIspCtx);
    }

    pRegCfg->unKey.bit1McdsCfg = 1;

    return;
}

HI_S32 McdsProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    return HI_SUCCESS;
}

HI_S32 ISP_McdsInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    McdsRegsInitialize(ViPipe, pstRegCfg);

    return HI_SUCCESS;
}

HI_S32 ISP_McdsRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_BOOL bEnEdgeMarkRead;
    HI_U8   i;
    ISP_CTX_S     *pstIspCtx = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    /* calculate every two interrupts */
    if ((0 != pstIspCtx->u32FrameCnt % 2) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState))
    {
        return HI_SUCCESS;
    }

    bEnEdgeMarkRead = hi_ext_system_manual_isp_edgemark_en_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.bMCDSen = HI_TRUE;

        pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bDynaResh = 1;

        if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pstIspCtx->stHdrAttr.enFormat)
        {
            pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bVcdsEn = HI_FALSE;
        }
        else if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstIspCtx->stHdrAttr.enFormat)
        {
            pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bVcdsEn    = HI_TRUE;      //422: 0; 420: 1
        }
        else //400 is the same with 420
        {
           pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bVcdsEn    = HI_TRUE;
        }


        if (bEnEdgeMarkRead) //To close Median filter when edgemark is open
        {
            pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bMidfEn = 0;
        }
        else
        {
            pstRegCfg->stAlgRegCfg[i].stMcdsRegCfg.stDynaRegCfg.bMidfEn = 1;
        }
    }

    pstRegCfg->unKey.bit1McdsCfg = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_McdsCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_PROC_WRITE:
            McdsProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        case  ISP_CHANGE_IMAGE_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_McdsInit(ViPipe, &pRegCfg->stRegCfg);
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_McdsExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S  *pRegCfg = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stMcdsRegCfg.bMCDSen = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1McdsCfg = 1;
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterMcds(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Mcds);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_MCDS;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_McdsInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_McdsRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_McdsCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_McdsExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
