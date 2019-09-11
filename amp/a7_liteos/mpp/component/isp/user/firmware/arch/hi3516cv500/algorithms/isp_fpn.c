/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_fpn.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/07/24
  Description   :
  History       :
  1.Date        : 2013/07/24
    Author      :
    Modification: Created file

******************************************************************************/
#include "isp_config.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_ext_config.h"

#include "hi_comm_vi.h"
#include "mpi_vi.h"
#include "vi_ext.h"
#include "isp_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/****************************************************************************
 * MACRO DEFINITION                                                         *
 ****************************************************************************/
#define FPN_OVERFLOWTHR                     0x7C0
#define ISP_FPN_MAX_O                       0xFFF
#define FPN_OVERFLOWTHR_OFF                 0x3FFF
#define FPN_CHN_NUM                         2
#define ISP_FPN_MODE_CORRECTION             0x0
#define ISP_FPN_MODE_CALIBRATE              0x1

#define ISP_FPN_CLIP(min,max,x)             ( (x)<= (min) ? (min) : ((x)>(max)?(max):(x)) )

typedef enum hiISP_SENSOR_BIT_WIDTH_E
{
    ISP_SENSOR_8BIT  = 8,
    ISP_SENSOR_10BIT = 10,
    ISP_SENSOR_12BIT = 12,
    ISP_SENSOR_14BIT = 14,
    ISP_SENSOR_16BIT = 16,
    ISP_SENSOR_32BIT = 32,
    ISP_SENSOR_BUTT
} ISP_SENSOR_BIT_WIDTH_E;

HI_S32  ISP_SetCalibrateAttr(VI_PIPE ViPipe, ISP_FPN_CALIBRATE_ATTR_S *pstCalibrate)
{
    VI_FPN_ATTR_S stFpnAttr;
    HI_S32 s32Ret;
    HI_U8 u8WDRMode;

    u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);

    if (IS_WDR_MODE(u8WDRMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Do not support FPN Calibration in WDR mode!\n");
        return HI_FAILURE;
    }

    stFpnAttr.enFpnWorkMode = FPN_MODE_CALIBRATE;
    memcpy(&stFpnAttr.stCalibrateAttr, pstCalibrate, sizeof(ISP_FPN_CALIBRATE_ATTR_S));

    hi_ext_system_fpn_cablibrate_enable_write(ViPipe, 1);

    s32Ret = MPI_VI_SetFPNAttr(ViPipe, &stFpnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    memcpy(pstCalibrate, &stFpnAttr.stCalibrateAttr, sizeof(ISP_FPN_CALIBRATE_ATTR_S));
    pstCalibrate->stFpnCaliFrame.u32Iso = hi_ext_system_fpn_sensor_iso_read(ViPipe);
    hi_ext_system_fpn_cablibrate_enable_write(ViPipe, 0);

    return HI_SUCCESS;
}

HI_U32 ISP_Fpn_GetStrength(HI_U32 u32Iso, HI_U32 u32CalibrateIso)
{
    HI_U32 u32Strength;

    u32Strength = 256 * u32Iso / DIV_0_TO_1(u32CalibrateIso);

    return u32Strength;
}

HI_S32 ISP_SetCorrectionAttr(VI_PIPE ViPipe, const ISP_FPN_ATTR_S *pstCorrection)
{
    HI_S32 s32Ret;
    HI_U8 u8WDRMode;
    VI_FPN_ATTR_S stFpnAttr;

    u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);

    if (IS_WDR_MODE(u8WDRMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Do not support FPN Correction in WDR mode!\n");
        return HI_FAILURE;
    }

    stFpnAttr.enFpnWorkMode = FPN_MODE_CORRECTION;
    memcpy(&stFpnAttr.stCorrectionAttr, pstCorrection, sizeof(ISP_FPN_ATTR_S));

    s32Ret = MPI_VI_SetFPNAttr(ViPipe, &stFpnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    hi_ext_system_manual_fpn_opmode_write(ViPipe, pstCorrection->enOpType);
    hi_ext_system_manual_fpn_Enable_write(ViPipe, pstCorrection->bEnable);
    hi_ext_system_manual_fpn_iso_write(ViPipe, pstCorrection->stFpnFrmInfo.u32Iso);
    hi_ext_system_manual_fpn_Gain_write(ViPipe, pstCorrection->stManual.u32Strength);

    return s32Ret;
}

HI_S32 ISP_GetCorrectionAttr(VI_PIPE ViPipe, ISP_FPN_ATTR_S *pstCorrection)
{
    HI_U8 u8BlkDev = 0;
    HI_U8 index    = 0;
    HI_S32 s32Ret  = HI_SUCCESS;
    VI_FPN_ATTR_S  stTempViFpnAttr;

    s32Ret = MPI_VI_GetFPNAttr(ViPipe, &stTempViFpnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    memcpy(pstCorrection, &stTempViFpnAttr.stCorrectionAttr, sizeof(ISP_FPN_ATTR_S));
    pstCorrection->stAuto.u32Strength = (isp_fpn_corr_gainoffset_read(ViPipe, u8BlkDev, index) >> 16);

    return HI_SUCCESS;
}

static HI_VOID FPNExtRegsDefault(VI_PIPE ViPipe)
{
    hi_ext_system_fpn_sensor_iso_write(ViPipe, HI_EXT_SYSTEM_FPN_SENSOR_ISO_DEFAULT);
    hi_ext_system_manual_fpn_iso_write(ViPipe, HI_EXT_SYSTEM_FPN_MANU_ISO_DEFAULT);

    hi_ext_system_manual_fpn_CorrCfg_write(ViPipe, HI_EXT_SYSTEM_FPN_MANU_CORRCFG_DEFAULT);
    hi_ext_system_manual_fpn_Gain_write(ViPipe, HI_EXT_SYSTEM_FPN_STRENGTH_DEFAULT);
    hi_ext_system_manual_fpn_opmode_write(ViPipe, HI_EXT_SYSTEM_FPN_OPMODE_DEFAULT);
    hi_ext_system_manual_fpn_update_write(ViPipe, HI_EXT_SYSTEM_FPN_MANU_UPDATE_DEFAULT);

    hi_ext_system_manual_fpn_Type_write(ViPipe, 0);
    hi_ext_system_manual_fpn_Offset_write(ViPipe, 0);
    hi_ext_system_manual_fpn_Enable_write(ViPipe, 0);
    hi_ext_system_manual_fpn_Pixelformat_write(ViPipe, 0);
    hi_ext_system_fpn_cablibrate_enable_write(ViPipe, 0);

    return;
}

static HI_VOID FPNRegsDefault(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    pstRegCfg->unKey.bit1FpnCfg = 1;

    return;
}

static HI_VOID FPNExtRegsInitialize(VI_PIPE ViPipe)
{
    return;
}

static HI_VOID FPNRegsInitialize(VI_PIPE ViPipe)
{
    return;
}

static HI_S32 FPNReadExtregs(VI_PIPE ViPipe)
{
    return 0;
}

HI_S32 FPNUpdateExtRegs(VI_PIPE ViPipe)
{
    HI_U8 i = 0;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pRegCfg = HI_NULL;
    HI_U8 u8FpnOpMode;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    hi_ext_system_fpn_sensor_iso_write(ViPipe, pstIspCtx->stLinkage.u32SensorIso);
    u8FpnOpMode = hi_ext_system_manual_fpn_opmode_read(ViPipe);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        if (hi_ext_system_fpn_cablibrate_enable_read(ViPipe))
        {
            pRegCfg->stRegCfg.stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg.u32IspFpnCalibCorr = ISP_FPN_MODE_CALIBRATE;
        }
        else
        {
            pRegCfg->stRegCfg.stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg.u32IspFpnCalibCorr = ISP_FPN_MODE_CORRECTION;
        }

        if (IS_WDR_MODE(pstIspCtx->u8PreSnsWDRMode) && (OP_TYPE_AUTO == u8FpnOpMode))
        {
            pRegCfg->stRegCfg.stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg.u32IspFpnOverflowThr = FPN_OVERFLOWTHR;
        }
        else
        {
            pRegCfg->stRegCfg.stAlgRegCfg[i].stFpnRegCfg.stDynaRegCfg.u32IspFpnOverflowThr = FPN_OVERFLOWTHR_OFF;
        }
    }

    return HI_SUCCESS;
}

HI_VOID IspSetStrength(VI_PIPE ViPipe)
{
    HI_U32       u32Iso, u32Gain, i;
    HI_U32       u32CalibrateIso;
    HI_U8        u8FpnOpMode, u8FpnEn;
    //ISP_CTX_S    *pstIspCtx = HI_NULL;
    ISP_REGCFG_S *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
    //ISP_GET_CTX(ViPipe, pstIspCtx);

    u8FpnEn = isp_ext_system_manual_fpn_Enable_read(ViPipe);
    if (!u8FpnEn)
    {
        return;
    }

    u8FpnOpMode = hi_ext_system_manual_fpn_opmode_read(ViPipe);
    if (OP_TYPE_MANUAL == u8FpnOpMode)
    {
        for (i = 0; i < FPN_CHN_NUM; i++)
        {
            pRegCfg->stRegCfg.stAlgRegCfg[0].stFpnRegCfg.stDynaRegCfg.u32IspFpnStrength[i] = isp_ext_system_manual_fpn_Gain_read(ViPipe);
        }
        pRegCfg->stRegCfg.unKey.bit1FpnCfg = 1;
        return;
    }

    u32Iso = hi_ext_system_fpn_sensor_iso_read(ViPipe);
    u32CalibrateIso = hi_ext_system_manual_fpn_iso_read(ViPipe);
    u32Gain = ISP_Fpn_GetStrength(u32Iso, u32CalibrateIso);
    u32Gain = ISP_FPN_CLIP(0, 0x3FF, u32Gain);

    for (i = 0; i < FPN_CHN_NUM; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[0].stFpnRegCfg.stDynaRegCfg.u32IspFpnStrength[i] = u32Gain;
    }

    pRegCfg->stRegCfg.unKey.bit1FpnCfg = 1;
}

HI_S32 ISP_FPNInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    FPNRegsDefault(ViPipe, (ISP_REG_CFG_S *)pRegCfg);
    FPNExtRegsDefault(ViPipe);
    FPNReadExtregs(ViPipe);
    FPNRegsInitialize(ViPipe);
    FPNExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_FPNRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                  HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    FPNUpdateExtRegs(ViPipe);
    IspSetStrength(ViPipe);

    return HI_SUCCESS;
}

HI_S32 FpnProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    HI_U8 u8BlkDev = 0; /* BE Num, for SBS mode, BE0 & BE1 is the same FPN config, so just 0 is ok */
    ISP_CTRL_PROC_WRITE_S stProcTmp;
    HI_U32 u32Offset = 0;
    HI_U32 u32Strength = 0;

    if ((HI_NULL == pstProc->pcProcBuff)
        || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----FPN CORRECT INFO------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "%8s" "%7s"     "%9s"       "%8s",
                    "En", "OpType", "Strength", "Offset\n");

    u32Offset = isp_fpn_corr_gainoffset_read(ViPipe, u8BlkDev, 0) & 0xfff;
    u32Strength = (isp_fpn_corr_gainoffset_read(ViPipe, u8BlkDev, 0) >> 16) & 0xffff;

    if (isp_ext_system_manual_fpn_Enable_read(ViPipe))
    {
        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%8d" "%4d"  "%9d"  "%8d",
                        isp_ext_system_manual_fpn_Enable_read(ViPipe),
                        hi_ext_system_manual_fpn_opmode_read(ViPipe),
                        u32Strength,
                        u32Offset);
    }
    else
    {
        ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                        "%8d" "%4s"  "%9s"  "%8s\n",
                        isp_fpn_en_read(ViPipe, u8BlkDev),
                        "--",
                        "--",
                        "--");
    }

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

HI_S32 ISP_FPNCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    switch (u32Cmd)
    {
        case ISP_PROC_WRITE:
            FpnProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;

        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_FPNExit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterFPN(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Fpn);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_FPN;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_FPNInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_FPNRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_FPNCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_FPNExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
