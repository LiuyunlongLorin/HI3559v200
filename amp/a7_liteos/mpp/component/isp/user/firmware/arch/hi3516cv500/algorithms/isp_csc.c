/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_csc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2014/08/08
  Description   :
  History       :
  1.Date        : 2014/08/08
    Author      :
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

#define ISP_CSC_DFT_LUMA 50
#define ISP_CSC_DFT_CON  50
#define ISP_CSC_DFT_HUE  50
#define ISP_CSC_DFT_SAT  50

typedef struct hiHI_ISP_CSC_ATTR
{
    HI_S16 as16CSCIdc[3];
    HI_S16 as16CSCOdc[3];
    HI_S16 as16CSCCoef[9];
} ISP_CSC_ATTR;

typedef struct hiISP_CSC_S
{
    HI_BOOL bCscUsrEn;
    HI_BOOL bLimitedRangeEn;
    HI_BOOL bExtEn;
    HI_BOOL bCtmodeEn;
    COLOR_GAMUT_E enColorGamut;
    HI_U8   u8Hue;
    HI_U8   u8Contrast;
    HI_U8   u8Luma;
    HI_U8   u8Sat;
    ISP_CSC_ATTR stIspCscAttr;

    HI_BOOL bUpdate;
} ISP_CSC_S;

/* BT.709 -> RGB Coefficient Matrix */
static ISP_CSC_ATTR g_stCSCAttr_601 =
{
    /*csc Input DC component (IDC)*/
    {0, 0, 0},
    /*csc Output DC component (ODC)*/
    {0, 128, 128},
    /*csc Multiplier coefficient*/
    {299, 587, 114, -172, -339, 511, 511, -428, -83},
};

/* BT.709 -> RGB Coefficient Matrix */
static ISP_CSC_ATTR g_stCSCAttr_601_limited =
{
    /*csc Input DC component (IDC)*/
    {0, 0, 0},
    /*csc Output DC component (ODC)*/
    {16, 128, 128},
    /*csc Multiplier coefficient*/
    {258, 504, 98, -148, -291, 439, 439, -368, -71},
};

/* RGB -> YUV BT.709 Coefficient Matrix */
static ISP_CSC_ATTR g_stCSCAttr_709 =
{
    /*csc Input DC component (IDC)*/
    {0, 0, 0},
    /*csc Output DC component (ODC)*/
    {0, 128, 128},
    /*csc Multiplier coefficient*/
    {213, 715, 72, -117, -394, 511, 511, -464, -47},
};

/* BT.709 -> RGB Coefficient Matrix */
static ISP_CSC_ATTR g_stCSCAttr_709_limited =
{
    /*csc Input DC component (IDC)*/
    {0, 0, 0},
    /*csc Output DC component (ODC)*/
    {16, 128, 128},
    /*csc Multiplier coefficient*/
    {184, 614, 62, -101, -338, 439, 439, -399, -40},
};

static const int ISP_CSC_SIN_TABLE[61] =
{
    -500,  -485,  -469,  -454,  -438,  -422,  -407,  -391,  -374,  -358,
    -342,  -325,  -309,  -292,  -276,  -259,  -242,  -225,  -208,  -191,
    -174,  -156,  -139,  -122,  -104,   -87,   -70,   -52,   -35,   -17,
    0,    17,    35,    52,    70,    87,   104,   122,   139,   156,
    174,   191,   208,   225,   242,   259,   276,   292,   309,   325,
    342,   358,   374,   391,   407,   422,   438,   454,   469,   485,
    500
};

static const int ISP_CSC_COS_TABLE[61] =
{
    866,   875,   883,   891,   899,   906,   914,   921,   927,   934,
    940,   946,   951,   956,   961,   966,   970,   974,   978,   982,
    985,   988,   990,   993,   995,   996,   998,   999,   999,  1000,
    1000,  1000,   999,   999,   998,   996,   995,   993,   990,   988,
    985,   982,   978,   974,   970,   966,   961,   956,   951,   946,
    940,   934,   927,   921,   914,   906,   899,   891,   883,   875,
    866
};

ISP_CSC_S    g_astCscCtx[ISP_MAX_PIPE_NUM];
ISP_CSC_ATTR g_astCscAttrCtx[ISP_MAX_PIPE_NUM];

#define CSC_GET_CTX(dev, pstCtx)            pstCtx = &g_astCscCtx[dev]
#define CSC_ATTR_GET_CTX(dev, pstAttrCtx)   pstAttrCtx = &g_astCscAttrCtx[dev]

/*CSC Reg write interface*/
static HI_VOID CscDynaRegsConfig(ISP_CSC_ATTR *pstCscAttr, ISP_CSC_DYNA_CFG_S *pstDynaRegCfg)
{
    HI_U8 i;

    for (i = 0;i < 9; i++)
    {
        pstDynaRegCfg->as16CscCoef[i] = pstCscAttr->as16CSCCoef[i] * 1024 / 1000;
    }

    for (i = 0;i < 3; i++)
    {
        pstDynaRegCfg->as16CscInDC[i]  = pstCscAttr->as16CSCIdc[i] * 4;
        pstDynaRegCfg->as16CscOutDC[i] = pstCscAttr->as16CSCOdc[i] * 4;
    }

    pstDynaRegCfg->bResh = HI_TRUE;
}

static HI_S32 CscRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    //Csc Regs Initialize
    HI_U8 i;
    ISP_CSC_ATTR  stCscAttr;

    //Initilize Color Gamut Information
    memcpy(&stCscAttr, &g_stCSCAttr_709, sizeof(ISP_CSC_ATTR));

    //Config all four csc attributes.
    for ( i = 0 ; i < pstRegCfg->u8CfgNum; i++ )
    {
        //Coefficiets config
        CscDynaRegsConfig(&stCscAttr, &pstRegCfg->stAlgRegCfg[i].stCscCfg.stDynaRegCfg);

        pstRegCfg->stAlgRegCfg[i].stCscCfg.bEnable = HI_TRUE;
    }

    // Refresh flags are forced on
    pstRegCfg->unKey.bit1CscCfg = HI_TRUE;

    return HI_SUCCESS;
}

static HI_S32 CscExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_CSC_ATTR  stCscAttr;

    hi_ext_system_csc_gamuttype_write(ViPipe, COLOR_GAMUT_BT709);
    memcpy(&stCscAttr, &g_stCSCAttr_709, sizeof(ISP_CSC_ATTR));

    //Write csc related parameters
    hi_ext_system_csc_hue_write(ViPipe, ISP_CSC_DFT_HUE);
    hi_ext_system_csc_sat_write(ViPipe, ISP_CSC_DFT_SAT);
    hi_ext_system_csc_contrast_write(ViPipe, ISP_CSC_DFT_CON);
    hi_ext_system_csc_luma_write(ViPipe, ISP_CSC_DFT_LUMA);
    hi_ext_system_csc_limitrange_en_write(ViPipe, HI_FALSE);
    hi_ext_system_csc_ext_en_write(ViPipe, HI_TRUE);
    hi_ext_system_csc_ctmode_en_write(ViPipe, HI_TRUE);
    hi_ext_system_csc_attr_update_write(ViPipe, HI_TRUE);

    //Write DC components
    for ( i = 0 ; i < 3 ; i++ )
    {
        hi_ext_system_csc_dcin_write(ViPipe, i,  stCscAttr.as16CSCIdc[i]);
        hi_ext_system_csc_dcout_write(ViPipe, i, stCscAttr.as16CSCOdc[i]);
    }

    //Write Coefficients
    for ( i = 0 ; i < 9 ; i++ )
    {
        hi_ext_system_csc_coef_write(ViPipe, i, stCscAttr.as16CSCCoef[i]);
    }

    hi_ext_system_csc_enable_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}


static HI_S32 CscReadExtRegs(VI_PIPE ViPipe, ISP_CSC_S *pstCsc)
{
    HI_U8 i;
    HI_U8 Gamut;

    pstCsc->bUpdate = hi_ext_system_csc_attr_update_read(ViPipe);
    hi_ext_system_csc_attr_update_write(ViPipe, HI_FALSE);

    if ( pstCsc->bUpdate )
    {
        pstCsc->bLimitedRangeEn = hi_ext_system_csc_limitrange_en_read(ViPipe);
        pstCsc->bExtEn = hi_ext_system_csc_ext_en_read(ViPipe);
        pstCsc->bCtmodeEn = hi_ext_system_csc_ctmode_en_read(ViPipe);

        Gamut = hi_ext_system_csc_gamuttype_read(ViPipe);
        if ( COLOR_GAMUT_BT2020 == Gamut )
        {
            ISP_TRACE(HI_DBG_WARN, "Invalid Color Gamut type, Set back to previous gamut type!\n");
            //REMAIN UNUPDATED;
        }
        else
        {
            pstCsc->enColorGamut = Gamut;
        }

        pstCsc->u8Contrast = hi_ext_system_csc_contrast_read(ViPipe);
        pstCsc->u8Hue = hi_ext_system_csc_hue_read(ViPipe);
        pstCsc->u8Sat = hi_ext_system_csc_sat_read(ViPipe);
        pstCsc->u8Luma = hi_ext_system_csc_luma_read(ViPipe);

        for ( i = 0 ; i < 9 ; i++ )
        {
            pstCsc->stIspCscAttr.as16CSCCoef[i] = hi_ext_system_csc_coef_read(ViPipe, i);
        }

        // DC components read
        for ( i = 0 ; i < 3 ; i++ )
        {
            pstCsc->stIspCscAttr.as16CSCIdc[i] = hi_ext_system_csc_dcin_read(ViPipe, i);
            pstCsc->stIspCscAttr.as16CSCOdc[i] = hi_ext_system_csc_dcout_read(ViPipe, i);
        }
    }

    return HI_SUCCESS;
}

HI_S32 CscCalMatrix(VI_PIPE ViPipe, ISP_CSC_S *pstCsc, ISP_CSC_ATTR *pstCscCoef)
{
    HI_S32 s32Luma     = 0;
    HI_S32 s32Contrast = 0;
    HI_S32 s32Hue      = 0;
    HI_S32 s32Satu     = 0;
    HI_S32 i = 0;
    HI_S16 s16BlackIn = 0, s16BlackOut = 0;
    ISP_CSC_ATTR *pstCscCoefTmp = HI_NULL;

    if (pstCsc->bExtEn)
    {
        s32Luma = (HI_S32)pstCsc->u8Luma - 50;  /* -50 ~ 50*/
        s32Luma = s32Luma * 0x80 / 50;  /* -128 ~ 128 */
        s32Luma = (s32Luma == 128) ? 127 : s32Luma;
    }
    else
    {
        s32Luma = (HI_S32)pstCsc->u8Luma * 64 / 100 - 32; /*-32 ~ 32*/
        s32Luma = CLIP3(s32Luma, -128, 128); /*Limit the result of Luma*/
    }

    s32Contrast = ((HI_S32)pstCsc->u8Contrast - 50) * 2 + 100;
    s32Hue      = (HI_S32)pstCsc->u8Hue * 60 / 100;
    s32Hue      = CLIP3(s32Hue, 0, 60); /*Limit the result of Hue array select*/
    s32Satu     = ((HI_S32)pstCsc->u8Sat - 50) * 2 + 100;

    switch ( pstCsc->enColorGamut)
    {
        case COLOR_GAMUT_BT601:
            pstCscCoefTmp  = (pstCsc->bLimitedRangeEn) ? &g_stCSCAttr_601_limited : &g_stCSCAttr_601;
            break;
        case COLOR_GAMUT_BT709:
            pstCscCoefTmp  = (pstCsc->bLimitedRangeEn) ? &g_stCSCAttr_709_limited : &g_stCSCAttr_709;
            break;
        case COLOR_GAMUT_USER:
            pstCscCoefTmp  = &(pstCsc->stIspCscAttr);
            pstCscCoefTmp->as16CSCOdc[0] = (pstCsc->bLimitedRangeEn) ? 16 : 0;
            break;
        default:
            ISP_TRACE(HI_DBG_ERR, "Invalid Color Gamut type!\n");
            return HI_FAILURE;
    }

    if ((pstCsc->bCtmodeEn) && (COLOR_GAMUT_USER >= pstCsc->enColorGamut))
    {
        s16BlackIn  = -128;
        s16BlackOut = (pstCsc->bLimitedRangeEn) ? 110 : 127;    /* 110 = (128 * 219) / 256, Full range to limited range conversion */
    }
    else
    {
        s16BlackIn  = 0;
        s16BlackOut = 0;
    }

    pstCscCoef->as16CSCCoef[0] = (HI_S16)((s32Contrast * ((HI_S32)pstCscCoefTmp->as16CSCCoef[0])) / 100);
    pstCscCoef->as16CSCCoef[1] = (HI_S16)((s32Contrast * ((HI_S32)pstCscCoefTmp->as16CSCCoef[1])) / 100);
    pstCscCoef->as16CSCCoef[2] = (HI_S16)((s32Contrast * ((HI_S32)pstCscCoefTmp->as16CSCCoef[2])) / 100);
    pstCscCoef->as16CSCCoef[3] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[3] * ISP_CSC_COS_TABLE[s32Hue] + pstCscCoefTmp->as16CSCCoef[6] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);
    pstCscCoef->as16CSCCoef[4] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[4] * ISP_CSC_COS_TABLE[s32Hue] + pstCscCoefTmp->as16CSCCoef[7] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);
    pstCscCoef->as16CSCCoef[5] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[5] * ISP_CSC_COS_TABLE[s32Hue] + pstCscCoefTmp->as16CSCCoef[8] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);
    pstCscCoef->as16CSCCoef[6] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[6] * ISP_CSC_COS_TABLE[s32Hue] - pstCscCoefTmp->as16CSCCoef[3] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);
    pstCscCoef->as16CSCCoef[7] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[7] * ISP_CSC_COS_TABLE[s32Hue] - pstCscCoefTmp->as16CSCCoef[4] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);
    pstCscCoef->as16CSCCoef[8] = (HI_S16)((s32Contrast * s32Satu * ((HI_S32)(pstCscCoefTmp->as16CSCCoef[8] * ISP_CSC_COS_TABLE[s32Hue] - pstCscCoefTmp->as16CSCCoef[5] * ISP_CSC_SIN_TABLE[s32Hue]) / 1000)) / 10000);

    for (i = 0; i < 3; i++)
    {
        if ( COLOR_GAMUT_USER == pstCsc->enColorGamut )
        {
            pstCscCoefTmp->as16CSCIdc[i] = 0;
        }
        pstCscCoef->as16CSCIdc[i] = pstCscCoefTmp->as16CSCIdc[i] + s16BlackIn;
        pstCscCoef->as16CSCOdc[i] = pstCscCoefTmp->as16CSCOdc[i];
    }

    // Add Luma
    pstCscCoef->as16CSCOdc[0] += ((HI_S16)s32Luma + s16BlackOut);

    for ( i = 0 ; i < 3 ; i++ )
    {
        hi_ext_system_csc_dcin_write(ViPipe, i,  pstCscCoef->as16CSCIdc[i]);
        hi_ext_system_csc_dcout_write(ViPipe, i, pstCscCoef->as16CSCOdc[i]);
    }

    if ( COLOR_GAMUT_USER != pstCsc->enColorGamut )
    {
        for ( i = 0 ; i < 9 ; i++ )
        {
            hi_ext_system_csc_coef_write(ViPipe, i, pstCscCoef->as16CSCCoef[i]);
        }
    }

    return HI_SUCCESS;
}

static HI_BOOL __inline CheckCscOpen(ISP_CSC_S *pstCsc)
{
    return (HI_TRUE == pstCsc->bCscUsrEn);
}

HI_S32 CscProcWrite(VI_PIPE ViPipe, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    return HI_SUCCESS;
}

HI_S32 CscColorGamutinfoGet(VI_PIPE ViPipe,  ISP_COLORGAMMUT_INFO_S  *pstColorGamutInfo)
{
    ISP_CSC_S    *pstCsc = HI_NULL;

    CSC_GET_CTX(ViPipe, pstCsc);

    pstColorGamutInfo->enColorGamut = pstCsc->enColorGamut;

    return HI_SUCCESS;
}


HI_S32 ISP_CscInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    CscRegsInitialize(ViPipe, pstRegCfg);
    CscExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_CscRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                  HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_U16 i;

    ISP_CSC_ATTR *pstCscAttr = HI_NULL;
    ISP_CSC_S    *pstCsc     = HI_NULL;
    ISP_CTX_S    *pstIspCtx  = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    CSC_GET_CTX(ViPipe, pstCsc);
    CSC_ATTR_GET_CTX(ViPipe, pstCscAttr);

    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    pstCsc->bCscUsrEn = hi_ext_system_csc_enable_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stCscCfg.bEnable = pstCsc->bCscUsrEn;
    }

    pstRegCfg->unKey.bit1CscCfg = 1;

    /*check hardware setting*/
    if (!CheckCscOpen(pstCsc))
    {
        return HI_SUCCESS;
    }

    CscReadExtRegs(ViPipe, pstCsc);

    CscCalMatrix(ViPipe, pstCsc, pstCscAttr);/*Calculate actual csc coefs*/

    for ( i = 0 ; i < pstRegCfg->u8CfgNum; i++ )
    {
        CscDynaRegsConfig(pstCscAttr, &pstRegCfg->stAlgRegCfg[i].stCscCfg.stDynaRegCfg);/*Coefficiets config*/
    }

    pstRegCfg->unKey.bit1CscCfg = HI_TRUE;/*Refresh flags are forced on*/

    return HI_SUCCESS;
}

HI_S32 ISP_CscCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    switch ( u32Cmd )
    {
        case ISP_PROC_WRITE:
            CscProcWrite(ViPipe, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        case ISP_COLORGAMUTINFO_GET:
            CscColorGamutinfoGet(ViPipe, (ISP_COLORGAMMUT_INFO_S *)pValue);
            break;

        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CscExit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterCsc(VI_PIPE ViPipe)
{

    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Csc);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_CSC;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_CscInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_CscRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_CscCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_CscExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
