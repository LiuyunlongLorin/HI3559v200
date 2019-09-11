//******************************************************************************
// Copyright     :  Copyright (C) 2017, Hisilicon Technologies Co., Ltd.
// File name     :  isp_lut_config.h
// Author        :
// Version       :  1.0
// Date          :  2017-02-23
// Description   :  Define all registers/tables
// History       :  2017-02-23 Create file
//******************************************************************************
#ifndef __ISP_LUT_CONFIG_H__
#define __ISP_LUT_CONFIG_H__

#include "hi_debug.h"
#include "hi_isp_debug.h"
#include "isp_vreg.h"
#include "isp_main.h"
#include "isp_regcfg.h"
#include "isp_lut_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


//******************************************************************************
//  Function    : iSetISP_AE_WEIGHTisp_ae_weight2
//  Description : Set the value of the member ISP_AE_WEIGHT.isp_ae_weight2
//  Input       : HI_U32 *upisp_ae_weight2: 4 bits
//******************************************************************************
static __inline HI_VOID isp_ae_weight_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U32 *upisp_ae_weight)
{
    HI_S32 i;

    for (i = 0; i < 64; i++)
    {
        pstBeReg->stIspBeLut.ISP_AE_WEIGHT[i].u32 = upisp_ae_weight[i];
    }
}

//******************************************************************************
//  Function    : iSetISP_DEMOSAIC_DEPURPLUTisp_demosaic_depurp_lut
//  Description : Set the value of the member ISP_DEMOSAIC_DEPURPLUT.isp_demosaic_depurp_lut
//  Input       : HI_U32 *upisp_demosaic_depurp_lut: 4 bits
//******************************************************************************
static __inline HI_VOID isp_demosaic_depurp_lut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_demosaic_depurp_lut)
{
    HI_S32 i;

    for (i = 0; i < 16; i++)
    {
        pstBeReg->stIspBeLut.ISP_DEMOSAIC_DEPURPLUT[i].u32 = upisp_demosaic_depurp_lut[i] & 0xF;
    }
}

//******************************************************************************
//  Function    : iSetISP_NDDM_GF_LUTisp_nddm_gflut
//  Description : Set the value of the member ISP_NDDM_GF_LUT.isp_nddm_gflut
//  Input       : HI_U32 *upisp_nddm_gflut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_nddm_gflut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_nddm_gflut)
{
    HI_S32 i;

    for (i = 0; i < 17; i++)
    {
        pstBeReg->stIspBeLut.ISP_NDDM_GF_LUT[i].u32 = upisp_nddm_gflut[i] & 0xFFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_LMT_EVENisp_bnr_lmt_even
//  Description : Set the value of the member ISP_BNR_LMT_EVEN.isp_bnr_lmt_even
//  Input       : HI_U32 *upisp_bnr_lmt_even: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_lmt_even_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_lmt_even)
{
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_LMT_EVEN[i].u32 = upisp_bnr_lmt_even[2 * i];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_LMT_ODDisp_bnr_lmt_odd
//  Description : Set the value of the member ISP_BNR_LMT_ODD.isp_bnr_lmt_odd
//  Input       : HI_U32 *upisp_bnr_lmt_odd: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_lmt_odd_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_lmt_odd)
{
    HI_S32 i;

    for (i = 0; i < 64; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_LMT_ODD[i].u32 = upisp_bnr_lmt_odd[2 * i + 1];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_COR_EVENisp_bnr_cor_even
//  Description : Set the value of the member ISP_BNR_COR_EVEN.isp_bnr_cor_even
//  Input       : HI_U32 *upisp_bnr_cor_even: 14 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_cor_even_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_bnr_cor_even)
{
    HI_S32 i;

    for (i = 0; i < 17; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_COR_EVEN[i].u32 = upisp_bnr_cor_even[2 * i] & 0x3FFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_COR_ODDisp_bnr_cor_odd
//  Description : Set the value of the member ISP_BNR_COR_ODD.isp_bnr_cor_odd
//  Input       : HI_U32 *upisp_bnr_cor_odd: 14 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_cor_odd_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_bnr_cor_odd)
{
    HI_S32 i;

    for (i = 0; i < 16; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_COR_ODD[i].u32 = upisp_bnr_cor_odd[2 * i + 1] & 0x3FFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_LMT_S_EVENisp_bnr_lmt_s_even
//  Description : Set the value of the member ISP_BNR_LMT_S_EVEN.isp_bnr_lmt_s_even
//  Input       : HI_U32 *upisp_bnr_lmt_s_even: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_lmt_s_even_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_lmt_s_even)
{
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_LMT_S_EVEN[i].u32 = upisp_bnr_lmt_s_even[2 * i];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_LMT_S_ODDisp_bnr_lmt_s_odd
//  Description : Set the value of the member ISP_BNR_LMT_S_ODD.isp_bnr_lmt_s_odd
//  Input       : HI_U32 *upisp_bnr_lmt_s_odd: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_lmt_s_odd_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_lmt_s_odd)
{
    HI_S32 i;

    for (i = 0; i < 64; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_LMT_S_ODD[i].u32 = upisp_bnr_lmt_s_odd[2 * i + 1];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_GCOR_EVENisp_bnr_gcor_even
//  Description : Set the value of the member ISP_BNR_GCOR_EVEN.isp_bnr_gcor_even
//  Input       : HI_U32 *upisp_bnr_gcor_even: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_gcor_even_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_gcor_even)
{
    HI_S32 i;

    for (i = 0; i < 17; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_GCOR_EVEN[i].u32 = upisp_bnr_gcor_even[2 * i];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_GCOR_ODDisp_bnr_gcor_odd
//  Description : Set the value of the member ISP_BNR_GCOR_ODD.isp_bnr_gcor_odd
//  Input       : HI_U32 *upisp_bnr_gcor_odd: 8 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_gcor_odd_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U8 *upisp_bnr_gcor_odd)
{
    HI_S32 i;

    for (i = 0; i < 16; i++)
    {
        pstBeReg->stIspBeLut.ISP_BNR_GCOR_ODD[i].u32 = upisp_bnr_gcor_odd[2 * i + 1];
    }
}

//******************************************************************************
//  Function    : iSetISP_DE_LUMA_GAINisp_de_luma_gain
//  Description : Set the value of the member ISP_DE_LUMA_GAIN.isp_de_luma_gain
//  Input       : HI_U32 *upisp_de_luma_gain: 9 bits
//******************************************************************************
static __inline HI_VOID isp_de_luma_gain_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_de_luma_gain)
{
    HI_S32 i;

    for (i = 0; i < 17; i++)
    {
        pstBeReg->stIspBeLut.ISP_DE_LUMA_GAIN[i].u32 = upisp_de_luma_gain[i] & 0x1FF;
    }
}

//******************************************************************************
//  Function    : iSetISP_EXPANDER_LUTisp_expander_lut
//  Description : Set the value of the member ISP_EXPANDER_LUT.isp_expander_lut
//  Input       : HI_U32 *upisp_expander_lut: 21 bits
//******************************************************************************
static __inline HI_VOID isp_expander_lut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U32 *upisp_expander_lut)
{
    HI_S32 i;

    for (i = 0; i < 257; i++)
    {
        pstBeReg->stIspBeLut.ISP_EXPANDER_LUT[i].u32 = upisp_expander_lut[i] & 0x1FFFFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYP_WLUTisp_ldci_poply1_wlut
//  Description : Set the value of the member ISP_LDCI_POLYP_WLUT.isp_ldci_poply1_wlut
//  Input       : HI_U32 *upisp_ldci_poply1_wlut: 10 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_poply1_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_poply1_wlut)
{
    U_ISP_LDCI_POLYP_WLUT o_isp_ldci_polyp_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyp_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32;
        o_isp_ldci_polyp_wlut.bits.isp_ldci_poply1_wlut = upisp_ldci_poply1_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32 = o_isp_ldci_polyp_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYP_WLUTisp_ldci_poply2_wlut
//  Description : Set the value of the member ISP_LDCI_POLYP_WLUT.isp_ldci_poply2_wlut
//  Input       : HI_U32 *upisp_ldci_poply2_wlut: 10 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_poply2_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_poply2_wlut)
{
    U_ISP_LDCI_POLYP_WLUT o_isp_ldci_polyp_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyp_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32;
        o_isp_ldci_polyp_wlut.bits.isp_ldci_poply2_wlut = upisp_ldci_poply2_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32 = o_isp_ldci_polyp_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYP_WLUTisp_ldci_poply3_wlut
//  Description : Set the value of the member ISP_LDCI_POLYP_WLUT.isp_ldci_poply3_wlut
//  Input       : HI_U32 *upisp_ldci_poply3_wlut: 10 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_poply3_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_poply3_wlut)
{
    U_ISP_LDCI_POLYP_WLUT o_isp_ldci_polyp_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyp_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32;
        o_isp_ldci_polyp_wlut.bits.isp_ldci_poply3_wlut = upisp_ldci_poply3_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYP_WLUT[i].u32 = o_isp_ldci_polyp_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYQ01_WLUTisp_ldci_plyq0_wlut
//  Description : Set the value of the member ISP_LDCI_POLYQ01_WLUT.isp_ldci_plyq0_wlut
//  Input       : HI_U32 *upisp_ldci_plyq0_wlut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_plyq0_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_plyq0_wlut)
{
    U_ISP_LDCI_POLYQ01_WLUT o_isp_ldci_polyq01_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyq01_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYQ01_WLUT[i].u32;
        o_isp_ldci_polyq01_wlut.bits.isp_ldci_plyq0_wlut = upisp_ldci_plyq0_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYQ01_WLUT[i].u32 = o_isp_ldci_polyq01_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYQ01_WLUTisp_ldci_plyq1_wlut
//  Description : Set the value of the member ISP_LDCI_POLYQ01_WLUT.isp_ldci_plyq1_wlut
//  Input       : HI_U32 *upisp_ldci_plyq1_wlut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_plyq1_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_plyq1_wlut)
{
    U_ISP_LDCI_POLYQ01_WLUT o_isp_ldci_polyq01_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyq01_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYQ01_WLUT[i].u32;
        o_isp_ldci_polyq01_wlut.bits.isp_ldci_plyq1_wlut = upisp_ldci_plyq1_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYQ01_WLUT[i].u32 = o_isp_ldci_polyq01_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYQ23_WLUTisp_ldci_plyq2_wlut
//  Description : Set the value of the member ISP_LDCI_POLYQ23_WLUT.isp_ldci_plyq2_wlut
//  Input       : HI_U32 *upisp_ldci_plyq2_wlut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_plyq2_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_plyq2_wlut)
{
    U_ISP_LDCI_POLYQ23_WLUT o_isp_ldci_polyq23_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyq23_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYQ23_WLUT[i].u32;
        o_isp_ldci_polyq23_wlut.bits.isp_ldci_plyq2_wlut = upisp_ldci_plyq2_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYQ23_WLUT[i].u32 = o_isp_ldci_polyq23_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_LDCI_POLYQ23_WLUTisp_ldci_plyq3_wlut
//  Description : Set the value of the member ISP_LDCI_POLYQ23_WLUT.isp_ldci_plyq3_wlut
//  Input       : HI_U32 *upisp_ldci_plyq3_wlut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_ldci_plyq3_wlut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_S16 *upisp_ldci_plyq3_wlut)
{
    U_ISP_LDCI_POLYQ23_WLUT o_isp_ldci_polyq23_wlut;
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        o_isp_ldci_polyq23_wlut.u32 = pstBeReg->stIspBeLut.ISP_LDCI_POLYQ23_WLUT[i].u32;
        o_isp_ldci_polyq23_wlut.bits.isp_ldci_plyq3_wlut = upisp_ldci_plyq3_wlut[i];
        pstBeReg->stIspBeLut.ISP_LDCI_POLYQ23_WLUT[i].u32 = o_isp_ldci_polyq23_wlut.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_DRC_TMLUT0isp_drc_tmlut0_diff
//  Description : Set the value of the member ISP_DRC_TMLUT0.isp_drc_tmlut0_diff
//  Input       : HI_U32 *upisp_drc_tmlut0_diff: 14 bits
//******************************************************************************
static __inline HI_VOID isp_drc_tmlut0_diff_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_drc_tmlut0_diff)
{
    U_ISP_DRC_TMLUT0 o_isp_drc_tmlut0;
    HI_S32 i;

    for (i = 0; i < 200; i++)
    {
        o_isp_drc_tmlut0.u32 = pstBeReg->stIspBeLut.ISP_DRC_TMLUT0[i].u32;
        o_isp_drc_tmlut0.bits.isp_drc_tmlut0_diff = upisp_drc_tmlut0_diff[i];
        pstBeReg->stIspBeLut.ISP_DRC_TMLUT0[i].u32 = o_isp_drc_tmlut0.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_DRC_TMLUT0isp_drc_tmlut0_value
//  Description : Set the value of the member ISP_DRC_TMLUT0.isp_drc_tmlut0_value
//  Input       : HI_U32 *upisp_drc_tmlut0_value: 16 bits
//******************************************************************************
static __inline HI_VOID isp_drc_tmlut0_value_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_drc_tmlut0_value)
{
    U_ISP_DRC_TMLUT0 o_isp_drc_tmlut0;
    HI_S32 i;

    for (i = 0; i < 200; i++)
    {
        o_isp_drc_tmlut0.u32 = pstBeReg->stIspBeLut.ISP_DRC_TMLUT0[i].u32;
        o_isp_drc_tmlut0.bits.isp_drc_tmlut0_value = upisp_drc_tmlut0_value[i];
        pstBeReg->stIspBeLut.ISP_DRC_TMLUT0[i].u32 = o_isp_drc_tmlut0.u32;
    }
}

//******************************************************************************
//  Function    : iSetISP_DRC_CCLUTisp_drc_cclut
//  Description : Set the value of the member ISP_DRC_CCLUT.isp_drc_cclut
//  Input       : HI_U32 *upisp_drc_cclut: 12 bits
//******************************************************************************
static __inline HI_VOID isp_drc_cclut_write(S_ISPBE_REGS_TYPE *pstBeReg, HI_U16 *upisp_drc_cclut)
{
    HI_S32 i;

    for (i = 0; i < 33; i++)
    {
        pstBeReg->stIspBeLut.ISP_DRC_CCLUT[i].u32 = upisp_drc_cclut[i] & 0xFFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_CLUT_LUT_WSTTisp_clut_lut_wstt
//  Description : Set the value of the member ISP_CLUT_LUT_WSTT.isp_clut_lut_wstt
//  Input       : HI_U32 *upisp_clut_lut_wstt: 30 bits
//  Return      : HI_S32 : 0-Error, 1-Success
//******************************************************************************
static __inline HI_VOID isp_clut_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U32 *upisp_clut_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 5508; i++)
    {
        pstBeLutSttReg->ISP_CLUT_LUT_WSTT[i].u32 = upisp_clut_lut_wstt[i];
    }
}

//******************************************************************************
//  Function    : iSetISP_DEHAZE_LUT_WSTTisp_dehaze_lut_wstt
//  Description : Set the value of the member ISP_DEHAZE_LUT_WSTT.isp_dehaze_lut_wstt
//  Input       : HI_U32 *upisp_dehaze_lut_wstt: 10 bits
//******************************************************************************
static __inline HI_VOID isp_dehaze_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U16 u16BlkNum,
        HI_U32 *upisp_dehaze_prestat_wstt, HI_U8 *upisp_dehaze_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < u16BlkNum; i++)
    {
        pstBeLutSttReg->ISP_DEHAZE_LUT_WSTT[2 * i].bits.isp_dehaze_lut_wstt     = upisp_dehaze_prestat_wstt[i] & 0x3ff;
        pstBeLutSttReg->ISP_DEHAZE_LUT_WSTT[2 * i + 1].bits.isp_dehaze_lut_wstt = ( upisp_dehaze_prestat_wstt[i] >> 16) & 0x3ff;
    }

    for (i = 2 * u16BlkNum; i < 1024; i++)
    {
        pstBeLutSttReg->ISP_DEHAZE_LUT_WSTT[i].bits.isp_dehaze_lut_wstt = 0;
    }

    for (i = 1024; i < 1280; i++)
    {
        pstBeLutSttReg->ISP_DEHAZE_LUT_WSTT[i].bits.isp_dehaze_lut_wstt = upisp_dehaze_lut_wstt[i - 1024];
    }
}

//******************************************************************************
//  Function    : iSetISP_BNR_LUT_WSTTisp_bnr_lut_wstt
//  Description : Set the value of the member ISP_BNR_LUT_WSTT.isp_bnr_lut_wstt
//  Input       : HI_U32 *upisp_bnr_lut_wstt: 32 bits
//******************************************************************************
static __inline HI_VOID isp_bnr_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, \
        HI_U16 *upisp_bnr_lut_wstt0, HI_U16 *upisp_bnr_lut_wstt1, HI_U16 *upisp_bnr_lut_wstt2, HI_U16 *upisp_bnr_lut_wstt3)
{
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        pstBeLutSttReg->ISP_BNR_LUT_WSTT[4 * i].bits.isp_bnr_lut_wstt     = (((HI_U32)upisp_bnr_lut_wstt1[2 * i] & 0xffff) << 16) + upisp_bnr_lut_wstt0[2 * i];
        pstBeLutSttReg->ISP_BNR_LUT_WSTT[4 * i + 1].bits.isp_bnr_lut_wstt = (((HI_U32)upisp_bnr_lut_wstt1[2 * i + 1] & 0xffff) << 16) + upisp_bnr_lut_wstt0[2 * i + 1];
        pstBeLutSttReg->ISP_BNR_LUT_WSTT[4 * i + 2].bits.isp_bnr_lut_wstt = (((HI_U32)upisp_bnr_lut_wstt3[2 * i] & 0xffff) << 16) + upisp_bnr_lut_wstt2[2 * i];
        pstBeLutSttReg->ISP_BNR_LUT_WSTT[4 * i + 3].bits.isp_bnr_lut_wstt = (((HI_U32)upisp_bnr_lut_wstt3[2 * i + 1] & 0xffff) << 16) + upisp_bnr_lut_wstt2[2 * i + 1];
    }
}


//******************************************************************************
//  Function    : iSetISP_GAMMA_LUT_WSTTisp_gamma_lut_wstt
//  Description : Set the value of the member ISP_GAMMA_LUT_WSTT.isp_gamma_lut_wstt
//  Input       : HI_U32 *upisp_gamma_lut_wstt: 12 bits
//******************************************************************************
static __inline HI_VOID isp_gamma_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U16 *upisp_gamma_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 257; i++)
    {
        pstBeLutSttReg->ISP_GAMMA_LUT_WSTT[i].u32 = upisp_gamma_lut_wstt[i] & 0xFFF;
    }
}

//******************************************************************************
//  Function    : iSetISP_SHARPEN_LUT_WSTTisp_sharpen_lut_wstt
//  Description : Set the value of the member ISP_SHARPEN_LUT_WSTT.isp_sharpen_lut_wstt
//  Input       : HI_U32 *upisp_sharpen_lut_wstt: 30 bits
//******************************************************************************
static __inline HI_VOID isp_sharpen_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, \
        HI_U16 *upisp_sharpen_mfgaind_wstt, HI_U16 *upisp_sharpen_mfgainud_wstt, \
        HI_U16 *upisp_sharpen_hfgaind_wstt, HI_U16 *upisp_sharpen_hfgainud_wstt)
{
    HI_S32 i;

    for (i = 0; i < 64; i++)
    {
        pstBeLutSttReg->ISP_SHARPEN_LUT_WSTT[i * 4].bits.isp_sharpen_lut_wstt     = upisp_sharpen_mfgaind_wstt[i];
        pstBeLutSttReg->ISP_SHARPEN_LUT_WSTT[i * 4 + 1].bits.isp_sharpen_lut_wstt = upisp_sharpen_mfgainud_wstt[i];
        pstBeLutSttReg->ISP_SHARPEN_LUT_WSTT[i * 4 + 2].bits.isp_sharpen_lut_wstt = upisp_sharpen_hfgaind_wstt[i];
        pstBeLutSttReg->ISP_SHARPEN_LUT_WSTT[i * 4 + 3].bits.isp_sharpen_lut_wstt = upisp_sharpen_hfgainud_wstt[i];
    }
}

static __inline HI_VOID isp_dpc_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U32 *upisp_dpc_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 2048; i++)
    {
        pstBeLutSttReg->ISP_DPC_LUT_WSTT[i].u32 = upisp_dpc_lut_wstt[i];
    }
}

//******************************************************************************
//  Function    : iSetISP_LSC_LUT_WSTTisp_lsc_lut_wstt
//  Description : Set the value of the member ISP_LSC_LUT_WSTT.isp_lsc_lut_wstt
//  Input       : HI_U32 *upisp_lsc_lut_wstt: 20 bits
//******************************************************************************
static __inline HI_VOID isp_lsc_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U32 *upisp_lsc_rgain_wstt, \
        HI_U32 *upisp_lsc_grgain_wstt, HI_U32 *upisp_lsc_gbgain_wstt, HI_U32 *upisp_lsc_bgain_wstt)
{
    HI_S32 i;

    for (i = 0; i < 1089; i++)
    {
        pstBeLutSttReg->ISP_LSC_LUT_WSTT[4 * i + 0].u32 = upisp_lsc_rgain_wstt[i];
        pstBeLutSttReg->ISP_LSC_LUT_WSTT[4 * i + 1].u32 = upisp_lsc_grgain_wstt[i];
        pstBeLutSttReg->ISP_LSC_LUT_WSTT[4 * i + 2].u32 = upisp_lsc_bgain_wstt[i];
        pstBeLutSttReg->ISP_LSC_LUT_WSTT[4 * i + 3].u32 = upisp_lsc_gbgain_wstt[i];
    }
}

//******************************************************************************
//  Function    : iSetISP_CA_LUT_WSTTisp_ca_lut_wstt
//  Description : Set the value of the member ISP_CA_LUT_WSTT.isp_ca_lut_wstt
//  Input       : HI_U32 *upisp_ca_lut_wstt: 11 bits
//******************************************************************************
static __inline HI_VOID isp_ca_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U16 *upisp_ca_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 128; i++)
    {
        pstBeLutSttReg->ISP_CA_LUT_WSTT[i].u32 = upisp_ca_lut_wstt[i] & 0x7FF;
    }
}

static __inline HI_VOID isp_ldci_drc_cgain_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, \
        HI_S16 *upisp_ldci_calcdrc_lut_wstt, HI_S16 *upisp_ldci_statdrc_lut_wstt, HI_U32 *upisp_ldci_cgain_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 65; i++)
    {
        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i].u32     = (upisp_ldci_cgain_lut_wstt[i] << 20) + (upisp_ldci_statdrc_lut_wstt[i] << 10) + \
                upisp_ldci_calcdrc_lut_wstt[i];

        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i + 3].u32 = 0;
    }
}

static __inline HI_VOID isp_ldci_he_delut_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, \
        HI_U32 *upisp_ldci_delut_lut_wstt, HI_U32 *upisp_ldci_hepos_lut_wstt, HI_U32 *upisp_ldci_heneg_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 33; i++)
    {
        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i + 1].u32 = (upisp_ldci_heneg_lut_wstt[i] << 18) + (upisp_ldci_hepos_lut_wstt[i] << 9) + \
                upisp_ldci_delut_lut_wstt[i];
    }

    for (i = 33; i < 65; i++)
    {
        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i + 1].u32 = 0;
    }
}

static __inline HI_VOID isp_ldci_usm_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, \
        HI_U32 *upisp_ldci_usmpos_lut_wstt, HI_U32 *upisp_ldci_usmneg_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 33; i++)
    {
        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i + 2].u32 = (upisp_ldci_usmneg_lut_wstt[i] << 9) + upisp_ldci_usmpos_lut_wstt[i];
    }

    for (i = 33; i < 65; i++)
    {
        pstBeLutSttReg->ISP_LDCI_LUT_WSTT[4 * i + 2].u32 = 0;
    }
}

//******************************************************************************
//  Function    : iSetISP_PREGAMMA_LUT_WSTTisp_pregamma_lut_wstt
//  Description : Set the value of the member ISP_PREGAMMA_LUT_WSTT.isp_pregamma_lut_wstt
//  Input       : HI_U32 *upisp_pregamma_lut_wstt: 20 bits
//******************************************************************************
static __inline HI_VOID isp_pregamma_lut_wstt_write(S_ISP_LUT_WSTT_TYPE *pstBeLutSttReg, HI_U32 *upisp_pregamma_lut_wstt)
{
    HI_S32 i;

    for (i = 0; i < 257; i++)
    {
        pstBeLutSttReg->ISP_PREGAMMA_LUT_WSTT[i].u32 = upisp_pregamma_lut_wstt[i] & 0x1FFFFF;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
