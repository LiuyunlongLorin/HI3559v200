/******************************************************************************

*  Copyright (C), 2017 - 2018, Hisilicon Tech. Co., Ltd.
*  ALL RIGHTS RESERVED
*  File Name     : isp_clut.c
*  Description   :

******************************************************************************/

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_proc.h"
#include "mpi_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static const  HI_U16 g_au16GainR = 128;
static const  HI_U16 g_au16GainG = 128;
static const  HI_U16 g_au16GainB = 128;

typedef struct hi_ISP_CLUT_CTX_S
{
    HI_BOOL         bClutLutUpdateEn;
    HI_BOOL         bClutCtrlUpdateEn;
    HI_U32          *pu32VirAddr;
    ISP_CLUT_ATTR_S stClutCtrl;
} ISP_CLUT_CTX_S;

ISP_CLUT_CTX_S g_astClutCtx[ISP_MAX_PIPE_NUM] = {{0}};

#define CLUT_GET_CTX(dev, pstCtx)   pstCtx = &g_astClutCtx[dev]

static HI_VOID ClutExtRegsInitialize(VI_PIPE ViPipe)
{
    ISP_CLUT_CTX_S *pstClutCtx = HI_NULL;
    CLUT_GET_CTX(ViPipe, pstClutCtx);

    hi_ext_system_clut_en_write(ViPipe, pstClutCtx->stClutCtrl.bEnable);
    hi_ext_system_clut_gainR_write(ViPipe, (HI_U16)pstClutCtx->stClutCtrl.u32GainR);
    hi_ext_system_clut_gainG_write(ViPipe, (HI_U16)pstClutCtx->stClutCtrl.u32GainG);
    hi_ext_system_clut_gainB_write(ViPipe, (HI_U16)pstClutCtx->stClutCtrl.u32GainB);
    hi_ext_system_clut_ctrl_update_en_write(ViPipe, HI_FALSE);
    hi_ext_system_clut_lut_update_en_write(ViPipe, HI_FALSE);
}

static HI_VOID ClutUsrCoefRegsInitialize(VI_PIPE ViPipe, ISP_CLUT_USR_COEF_CFG_S *pstUsrCoefRegCfg)
{
    ISP_CLUT_CTX_S  *pstClutCtx  = HI_NULL;
    CLUT_GET_CTX(ViPipe, pstClutCtx);

    memcpy(pstUsrCoefRegCfg->au32LuStt, pstClutCtx->pu32VirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    pstUsrCoefRegCfg->bResh          = HI_TRUE;
    pstUsrCoefRegCfg->u8BufId        = 0;
    pstUsrCoefRegCfg->u32UpdateIndex = 1;
}

static HI_VOID ClutUsrCtrlRegsInitialize(VI_PIPE ViPipe, ISP_CLUT_USR_CTRL_CFG_S *pstUsrCtrlRegCfg)
{
    pstUsrCtrlRegCfg->bDemoMode        = HI_FALSE;
    pstUsrCtrlRegCfg->u32GainR         = 128;
    pstUsrCtrlRegCfg->u32GainB         = 128;
    pstUsrCtrlRegCfg->u32GainG         = 128;
    pstUsrCtrlRegCfg->bDemoEnable      = HI_FALSE;
    pstUsrCtrlRegCfg->bResh            = HI_TRUE;
}
static HI_VOID ClutUsrRegsInitialize(VI_PIPE ViPipe, ISP_CLUT_USR_CFG_S *pstUsrRegCfg)
{
    ClutUsrCoefRegsInitialize(ViPipe, &pstUsrRegCfg->stClutUsrCoefCfg);
    ClutUsrCtrlRegsInitialize(ViPipe, &pstUsrRegCfg->stClutUsrCtrlCfg);
}

HI_S32 ClutRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 i;
    ISP_CLUT_CTX_S *pstClutCtx = HI_NULL;
    CLUT_GET_CTX(ViPipe, pstClutCtx);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        ClutUsrRegsInitialize(ViPipe, &pstRegCfg->stAlgRegCfg[i].stClutCfg.stUsrRegCfg);
        pstRegCfg->stAlgRegCfg[i].stClutCfg.bEnable = pstClutCtx->stClutCtrl.bEnable;
    }

    pstRegCfg->unKey.bit1ClutCfg   = 1;

    return HI_SUCCESS;
}

static HI_S32 ClutReadExtregs(VI_PIPE ViPipe)
{
    ISP_CLUT_CTX_S *pstCLUTCtx = HI_NULL;

    CLUT_GET_CTX(ViPipe, pstCLUTCtx);

    pstCLUTCtx->bClutCtrlUpdateEn = hi_ext_system_clut_ctrl_update_en_read(ViPipe);
    hi_ext_system_clut_ctrl_update_en_write(ViPipe, HI_FALSE);

    if (pstCLUTCtx->bClutCtrlUpdateEn)
    {
        pstCLUTCtx->stClutCtrl.bEnable    = hi_ext_system_clut_en_read(ViPipe);
        pstCLUTCtx->stClutCtrl.u32GainR   = hi_ext_system_clut_gainR_read(ViPipe);
        pstCLUTCtx->stClutCtrl.u32GainG   = hi_ext_system_clut_gainG_read(ViPipe);
        pstCLUTCtx->stClutCtrl.u32GainB   = hi_ext_system_clut_gainB_read(ViPipe);
    }

    pstCLUTCtx->bClutLutUpdateEn = hi_ext_system_clut_lut_update_en_read(ViPipe);
    hi_ext_system_clut_lut_update_en_write(ViPipe, HI_FALSE);

    return HI_SUCCESS;
}

static HI_S32 ClutCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_CLUT_S *pstCmosClut)
{
    ISP_CHECK_BOOL(pstCmosClut->bEnable);

    if (pstCmosClut->u32GainR > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainR!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstCmosClut->u32GainG > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainG!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstCmosClut->u32GainB > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainB!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 ClutInitialize(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_MMZ_BUF_EX_S   stClutBuf;
    ISP_CLUT_CTX_S     *pstClutCtx = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft  = HI_NULL;

    CLUT_GET_CTX(ViPipe, pstClutCtx);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    pstClutCtx->pu32VirAddr = HI_NULL;

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_CLUT_BUF_GET, &stClutBuf.u64PhyAddr))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Clut Buffer Err\n");
        return HI_FAILURE;
    }

    stClutBuf.pVirAddr = HI_MPI_SYS_Mmap(stClutBuf.u64PhyAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    if ( !stClutBuf.pVirAddr )
    {
        return HI_FAILURE;
    }

    pstClutCtx->pu32VirAddr = (HI_U32 *)stClutBuf.pVirAddr;

    if (pstSnsDft->unKey.bit1Clut)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstClut);

        s32Ret = ClutCheckCmosParam(ViPipe, pstSnsDft->pstClut);

        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstClutCtx->stClutCtrl.bEnable  = pstSnsDft->pstClut->bEnable;
        pstClutCtx->stClutCtrl.u32GainR = pstSnsDft->pstClut->u32GainR;
        pstClutCtx->stClutCtrl.u32GainG = pstSnsDft->pstClut->u32GainG;
        pstClutCtx->stClutCtrl.u32GainB = pstSnsDft->pstClut->u32GainB;

        memcpy(pstClutCtx->pu32VirAddr, pstSnsDft->pstClut->stClutLut.au32lut, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));
    }
    else
    {
        pstClutCtx->stClutCtrl.bEnable  = HI_FALSE;
        pstClutCtx->stClutCtrl.u32GainR = (HI_U32)g_au16GainR;
        pstClutCtx->stClutCtrl.u32GainG = (HI_U32)g_au16GainG;
        pstClutCtx->stClutCtrl.u32GainB = (HI_U32)g_au16GainB;

        memset(pstClutCtx->pu32VirAddr, 0, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));
    }

    return HI_SUCCESS;
}

HI_VOID Isp_Clut_Usr_Coef_Fw(ISP_CLUT_CTX_S *pstCLUTCtx, ISP_CLUT_USR_COEF_CFG_S *pstClutUsrCoefCfg)
{
    memcpy(pstClutUsrCoefCfg->au32LuStt, pstCLUTCtx->pu32VirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    pstClutUsrCoefCfg->bResh           = HI_TRUE;
    pstClutUsrCoefCfg->u32UpdateIndex += 1;
}

HI_VOID Isp_Clut_Usr_Ctrl_Fw(ISP_CLUT_ATTR_S *pstClutCtrl, ISP_CLUT_USR_CTRL_CFG_S *pstClutUsrCtrlCfg)
{
    pstClutUsrCtrlCfg->u32GainR       = pstClutCtrl->u32GainR;
    pstClutUsrCtrlCfg->u32GainG       = pstClutCtrl->u32GainG;
    pstClutUsrCtrlCfg->u32GainB       = pstClutCtrl->u32GainB;
    pstClutUsrCtrlCfg->bResh          = HI_TRUE;
}

static HI_BOOL __inline CheckClutOpen(ISP_CLUT_CTX_S *pstClutCtx)
{
    return (HI_TRUE == pstClutCtx->stClutCtrl.bEnable);
}

static HI_S32 ISP_ClutInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = ClutInitialize(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    ClutRegsInitialize(ViPipe, pstRegCfg);
    ClutExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

static HI_S32 ISP_ClutRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                          HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U8 i;
    ISP_CTX_S      *pstIspCtx  = HI_NULL;
    ISP_CLUT_CTX_S *pstCLUTCtx = HI_NULL;
    ISP_REG_CFG_S  *pstReg     = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    CLUT_GET_CTX(ViPipe, pstCLUTCtx);

    /* calculate every two interrupts */
    if ((0 != pstIspCtx->u32FrameCnt % 2) && (HI_TRUE != pstIspCtx->stLinkage.bSnapState))
    {
        return HI_SUCCESS;
    }

    pstCLUTCtx->stClutCtrl.bEnable = hi_ext_system_clut_en_read(ViPipe);

    for (i = 0; i < pstReg->u8CfgNum; i++)
    {
        pstReg->stAlgRegCfg[i].stClutCfg.bEnable = pstCLUTCtx->stClutCtrl.bEnable;
    }

    pstReg->unKey.bit1ClutCfg = 1;

    /*check hardware setting*/
    if (!CheckClutOpen(pstCLUTCtx))
    {
        return HI_SUCCESS;
    }

    ClutReadExtregs(ViPipe);

    if (pstCLUTCtx->bClutCtrlUpdateEn)
    {
        for (i = 0; i < pstReg->u8CfgNum; i++)
        {
            Isp_Clut_Usr_Ctrl_Fw(&pstCLUTCtx->stClutCtrl, &pstReg->stAlgRegCfg[i].stClutCfg.stUsrRegCfg.stClutUsrCtrlCfg);
        }
    }

    if (pstCLUTCtx->bClutLutUpdateEn)
    {
        for (i = 0; i < pstReg->u8CfgNum; i++)
        {
            if (!pstCLUTCtx->pu32VirAddr)
            {
                return HI_FAILURE;
            }

            Isp_Clut_Usr_Coef_Fw(pstCLUTCtx, &pstReg->stAlgRegCfg[i].stClutCfg.stUsrRegCfg.stClutUsrCoefCfg);
        }
    }

    return HI_SUCCESS;
}
static HI_S32 ISP_ClutCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    return HI_SUCCESS;
}

static HI_S32 ISP_ClutExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S   *pRegCfg    = HI_NULL;
    ISP_CLUT_CTX_S *pstClutCtx = HI_NULL;

    CLUT_GET_CTX(ViPipe, pstClutCtx);

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stClutCfg.bEnable = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1ClutCfg = 1;

    if ( HI_NULL != pstClutCtx->pu32VirAddr )
    {
        HI_MPI_SYS_Munmap((HI_VOID *)pstClutCtx->pu32VirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterClut(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Clut);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_CLUT;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_ClutInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_ClutRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_ClutCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_ClutExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
