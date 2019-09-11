/************************************************************************
* Copyright (C) 2017 - 2018, Hisilicon Tech. Co., Ltd.
* ALL RIGHTS RESERVED
* FileName: isp_pregamma.c
* Description:
*
*************************************************************************/
#include <math.h>
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_ext_config.h"
#include "hi_math.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static const HI_U32 au32PreGAMMA[PREGAMMA_NODE_NUM] =
{
    0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
    4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680,
    7936, 8192, 8448, 8704, 8960, 9216, 9472, 9728, 9984, 10240, 10496, 10752, 11008, 11264,
    11520, 11776, 12032, 12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080, 14336, 14592,
    14848, 15104, 15360, 15616, 15872, 16128, 16384, 16896, 17408, 17920, 18432, 18944, 19456,
    19968, 20480, 20992, 21504, 22016, 22528, 23040, 23552, 24064, 24576, 25088, 25600, 26112,
    26624, 27136, 27648, 28160, 28672, 29184, 29696, 30208, 30720, 31232, 31744, 32256, 32768,
    33792, 34816, 35840, 36864, 37888, 38912, 39936, 40960, 41984, 43008, 44032, 45056, 46080,
    47104, 48128, 49152, 50176, 51200, 52224, 53248, 54272, 55296, 56320, 57344, 58368, 59392,
    60416, 61440, 62464, 63488, 64512, 65536, 67584, 69632, 71680, 73728, 75776, 77824, 79872,
    81920, 83968, 86016, 88064, 90112, 92160, 94208, 96256, 98304, 100352, 102400, 104448, 106496,
    108544, 110592, 112640, 114688, 116736, 118784, 120832, 122880, 124928, 126976, 129024, 131072,
    135168, 139264, 143360, 147456, 151552, 155648, 159744, 163840, 167936, 172032, 176128, 180224,
    184320, 188416, 192512, 196608, 200704, 204800, 208896, 212992, 217088, 221184, 225280, 229376,
    233472, 237568, 241664, 245760, 249856, 253952, 258048, 262144, 270336, 278528, 286720, 294912,
    303104, 311296, 319488, 327680, 335872, 344064, 352256, 360448, 368640, 376832, 385024, 393216,
    401408, 409600, 417792, 425984, 434176, 442368, 450560, 458752, 466944, 475136, 483328, 491520,
    499712, 507904, 516096, 524288, 540672, 557056, 573440, 589824, 606208, 622592, 638976, 655360,
    671744, 688128, 704512, 720896, 737280, 753664, 770048, 786432, 802816, 819200, 835584, 851968,
    868352, 884736, 901120, 917504, 933888, 950272, 966656, 983040, 999424, 1015808, 1032192, 1048575
};

static const HI_U8 g_au8PregammaSegIdxBase[PREGAMMA_SEG_NUM] = {0, 0, 32, 64, 96, 128, 160, 192};
static const HI_U8 g_au8PregammaSegMaxVal[PREGAMMA_SEG_NUM]  = {0, 2,  4,  8, 16,  32,  64, 128};

typedef struct hiISP_PREGAMMA_S
{
    HI_BOOL bEnable;
    HI_BOOL bLutUpdate;
} ISP_PREGAMMA_S;

ISP_PREGAMMA_S g_astPreGammaCtx[ISP_MAX_PIPE_NUM] = {{0}};
#define PREGAMMA_GET_CTX(dev, pstCtx)   pstCtx = &g_astPreGammaCtx[dev]

static HI_S32 PreGammaCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_PREGAMMA_S *pstPreGamma)
{
    HI_U16 i;

    ISP_CHECK_BOOL(pstPreGamma->bEnable);

    for (i = 0; i < PREGAMMA_NODE_NUM; i++)
    {
        if (pstPreGamma->au32PreGamma[i] > HI_ISP_PREGAMMA_LUT_MAX)
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid au32PreGamma[%d]!\n", i);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PreGammaRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8  u8BlockNum;
    HI_U16 i;
    HI_S32 s32Ret;
    const HI_U32           *pau32PreGAMMA  = HI_NULL;
    ISP_PREGAMMA_REG_CFG_S *pstPreGammaCfg = HI_NULL;
    ISP_PREGAMMA_S         *pstPreGammaCtx = HI_NULL;
    ISP_CTX_S              *pstIspCtx      = HI_NULL;
    ISP_CMOS_DEFAULT_S     *pstSnsDft      = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    PREGAMMA_GET_CTX(ViPipe, pstPreGammaCtx);

    pstPreGammaCtx->bLutUpdate = HI_FALSE;

    u8BlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

    /*Read from CMOS*/
    if (pstSnsDft->unKey.bit1PreGamma)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstPreGamma);

        s32Ret = PreGammaCheckCmosParam(ViPipe, pstSnsDft->pstPreGamma);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pau32PreGAMMA           = pstSnsDft->pstPreGamma->au32PreGamma;
        pstPreGammaCtx->bEnable = pstSnsDft->pstPreGamma->bEnable;
    }
    else
    {
        pau32PreGAMMA           = au32PreGAMMA;
        pstPreGammaCtx->bEnable =  HI_FALSE;
    }

    for (i = 0 ; i < PREGAMMA_NODE_NUM ; i++)
    {
        hi_ext_system_pregamma_lut_write(ViPipe, i, pau32PreGAMMA[i]);
    }

    for (i = 0 ; i < u8BlockNum ; i++)
    {
        pstPreGammaCfg = &pstRegCfg->stAlgRegCfg[i].stPreGammaCfg;

        /*Static*/
        pstPreGammaCfg->stStaticRegCfg.u8BitDepthIn  = 20;
        pstPreGammaCfg->stStaticRegCfg.u8BitDepthOut = 20;
        pstPreGammaCfg->stStaticRegCfg.bStaticResh   = HI_TRUE;
        memcpy(pstPreGammaCfg->stStaticRegCfg.au8SegIdxBase, g_au8PregammaSegIdxBase, PREGAMMA_SEG_NUM * sizeof(HI_U8));
        memcpy(pstPreGammaCfg->stStaticRegCfg.au8SegMaxVal,  g_au8PregammaSegMaxVal,  PREGAMMA_SEG_NUM * sizeof(HI_U8));

        /*Dynamic*/
        //Enable Gamma
        pstPreGammaCfg->bPreGammaEn                       = pstPreGammaCtx->bEnable;
        pstPreGammaCfg->stDynaRegCfg.bPreGammaLutUpdateEn = HI_TRUE;
        pstPreGammaCfg->stDynaRegCfg.u32UpdateIndex       = 1;
        pstPreGammaCfg->stDynaRegCfg.u8BufId              = 0;
        memcpy(pstPreGammaCfg->stDynaRegCfg.u32PreGammaLUT, pau32PreGAMMA, PREGAMMA_NODE_NUM * sizeof(HI_U32));
    }

    pstRegCfg->unKey.bit1PreGammaCfg = 1;

    return HI_SUCCESS;
}

static HI_VOID PreGammaExtRegsInitialize(VI_PIPE ViPipe)
{
    ISP_PREGAMMA_S         *pstPreGammaCtx = HI_NULL;

    PREGAMMA_GET_CTX(ViPipe, pstPreGammaCtx);

    hi_ext_system_pregamma_en_write(ViPipe, pstPreGammaCtx->bEnable);
    hi_ext_system_pregamma_lut_update_write(ViPipe, HI_FALSE);
}

HI_S32 ISP_PreGammaInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = PreGammaRegsInitialize(ViPipe, pstRegCfg);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    PreGammaExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

static HI_VOID ISP_PreGammaWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_U8  i;
    HI_U32 au32UpdateIdx[ISP_STRIPING_MAX_NUM] = {0};
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        au32UpdateIdx[i] = pstRegCfg->stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg.u32UpdateIndex;
    }

    ISP_PreGammaInit(ViPipe, pRegCfg);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg.u32UpdateIndex = au32UpdateIdx[i] + 1;
        pstRegCfg->stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg.bSwitchMode    = HI_TRUE;
    }
}

static HI_S32 PreGammaReadExtRegs(VI_PIPE ViPipe)
{
    ISP_PREGAMMA_S *pstPreGammaCtx = HI_NULL;

    PREGAMMA_GET_CTX(ViPipe, pstPreGammaCtx);

    pstPreGammaCtx->bLutUpdate = hi_ext_system_pregamma_lut_update_read(ViPipe);
    hi_ext_system_pregamma_lut_update_write(ViPipe, HI_FALSE);

    return HI_SUCCESS;
}

HI_S32 PreGammaProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    ISP_CTRL_PROC_WRITE_S stProcTmp;
    ISP_PREGAMMA_S *pstPreGamma = HI_NULL;

    PREGAMMA_GET_CTX(ViPipe, pstPreGamma);

    if ((HI_NULL == pstProc->pcProcBuff)
        || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
                    "-----PreGamma INFO--------------------------------------------------------------\n");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen, "%16s\n", "Enable");

    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen, "%16u\n", pstPreGamma->bEnable);

    pstProc->u32WriteLen += 1;

    return HI_SUCCESS;
}

static HI_BOOL __inline CheckPreGammaOpen(ISP_PREGAMMA_S *pstPreGamma)
{
    return (HI_TRUE == pstPreGamma->bEnable);
}

HI_S32 ISP_PreGammaRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                       HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U16 i, j;
    ISP_PREGAMMA_S *pstPreGammaCtx = HI_NULL;
    ISP_REG_CFG_S  *pstReg         = (ISP_REG_CFG_S *)pRegCfg;
    ISP_PREGAMMA_DYNA_CFG_S *pstDynaRegCfg = HI_NULL;

    PREGAMMA_GET_CTX(ViPipe, pstPreGammaCtx);

    pstPreGammaCtx->bEnable = hi_ext_system_pregamma_en_read(ViPipe);

    for (i = 0; i < pstReg->u8CfgNum; i++)
    {
        pstReg->stAlgRegCfg[i].stPreGammaCfg.bPreGammaEn = pstPreGammaCtx->bEnable;
    }

    pstReg->unKey.bit1PreGammaCfg = 1;

    /*check hardware setting*/
    if (!CheckPreGammaOpen(pstPreGammaCtx))
    {
        return HI_SUCCESS;
    }

    PreGammaReadExtRegs(ViPipe);

    if (pstPreGammaCtx->bLutUpdate)
    {
        for (i = 0; i < pstReg->u8CfgNum; i++)
        {
            pstDynaRegCfg = &pstReg->stAlgRegCfg[i].stPreGammaCfg.stDynaRegCfg;
            for (j = 0 ; j < PREGAMMA_NODE_NUM ; j++)
            {
                pstDynaRegCfg->u32PreGammaLUT[j] = hi_ext_system_pregamma_lut_read(ViPipe, j);
            }

            pstDynaRegCfg->bPreGammaLutUpdateEn = HI_TRUE;
            pstDynaRegCfg->u32UpdateIndex      += 1;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_PreGammaCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_PreGammaWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_PROC_WRITE:
            PreGammaProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_PreGammaExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stPreGammaCfg.bPreGammaEn = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1PreGammaCfg = 1;
    return HI_SUCCESS;
}


HI_S32 ISP_AlgRegisterPreGamma(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1PreGamma);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_PREGAMMA;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_PreGammaInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_PreGammaRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_PreGammaCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_PreGammaExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
