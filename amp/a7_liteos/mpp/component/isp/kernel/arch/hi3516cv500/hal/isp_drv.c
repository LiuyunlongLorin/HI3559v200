

/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_drv.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/11/07
  Description   :
  History       :
  1.Date        : 2013/11/07
    Author      :
    Modification: Created file

******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "isp_drv_defines.h"
#include "isp_reg_define.h"
#include "isp_stt_define.h"
#include "hi_common.h"
#include "hi_osal.h"
#include "hi_math.h"
#include "mkp_isp.h"
#include "isp_drv.h"
#include "isp.h"
#include "mm_ext.h"
#include "sys_ext.h"

extern osal_spinlock_t g_stIspLock[ISP_MAX_PIPE_NUM];
extern osal_spinlock_t g_stIspSyncLock[ISP_MAX_PIPE_NUM];

extern HI_BOOL         g_IntBothalf;
extern HI_BOOL         g_UseBothalf;

void  *reg_vicap_base_va = HI_NULL;
void  *reg_vicap_ch_base_va[ISP_MAX_PIPE_NUM] = {HI_NULL};
void  *reg_ispfe_base_va[ISP_MAX_PIPE_NUM]    = {HI_NULL};
void  *reg_ispbe_base_va[ISP_MAX_BE_NUM]      = {HI_NULL};
void  *reg_viproc_base_va[ISP_MAX_BE_NUM]     = {HI_NULL};

#define ENABLE_CLOCK 1

HI_U32 g_DrcCurLumaLut[HI_ISP_DRC_SHP_LOG_CONFIG_NUM][HI_ISP_DRC_EXP_COMP_SAMPLE_NUM - 1] =
{
    {1,     1,      5,      31,     180,    1023,   32767},
    {1,     3,      8,      52,     277,    1446,   38966},
    {2,     5,      15,     87,     427,    2044,   46337},
    {4,     9,      27,     144,    656,    2888,   55101},
    {7,     16,     48,     240,    1008,   4080,   65521},
    {12,    29,     85,     399,    1547,   5761,   77906},
    {23,    53,     151,    660,    2372,   8128,   92622},
    {42,    97,     267,    1090,   3628,   11458,  110100},
    {76,    175,    468,    1792,   5537,   16130,  130840},
    {138,   313,    816,    2933,   8423,   22664,  155417},
    {258,   555,    1412,   4770,   12758,  31760,  184476},
    {441,   977,    2420,   7699,   19215,  44338,  218711},
    {776,   1698,   4100,   12304,  28720,  61568,  258816},
    {1344,  2907,   6847,   19416,  42491,  84851,  305376},
    {2283,  4884,   11224,  30137,  62006,  115708, 358680},
    {3783,  8004,   17962,  45770,  88821,  155470, 418391},
};

static HI_U16 Sqrt32(HI_U32 u32Arg)
{
    HI_U32 u32Mask = (HI_U32)1 << 15;
    HI_U16 u16Res = 0;
    HI_U32 i = 0;

    for (i = 0; i < 16; i++)
    {
        if ((u16Res + (u32Mask >> i)) * (u16Res + (u32Mask >> i)) <= u32Arg)
        {
            u16Res = u16Res + (u32Mask >> i);
        }
    }

    /* rounding */
    if (u16Res * u16Res + u16Res < u32Arg)
    {
        ++u16Res;
    }

    return u16Res;
}


/*---------------------------------------------- isp drv FHY regs define -----------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------------*/
HI_S32 ISP_DRV_SetInputSel(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 *pu32InputSel)
{
    U_ISP_BE_INPUT_MUX o_isp_be_input_mux;
    ISP_CHECK_POINTER(pstIspBeRegs);
    ISP_CHECK_POINTER(pu32InputSel);

    o_isp_be_input_mux.u32 = pstIspBeRegs->ISP_BE_INPUT_MUX.u32;
    o_isp_be_input_mux.bits.isp_input0_sel = pu32InputSel[0];
    o_isp_be_input_mux.bits.isp_input1_sel = pu32InputSel[1];
    o_isp_be_input_mux.bits.isp_input2_sel = pu32InputSel[2];
    o_isp_be_input_mux.bits.isp_input3_sel = pu32InputSel[3];
    o_isp_be_input_mux.bits.isp_input4_sel = pu32InputSel[4];
    pstIspBeRegs->ISP_BE_INPUT_MUX.u32 = o_isp_be_input_mux.u32;
    return 1;
}
static __inline HI_S32 ISP_DRV_SetIspDgain(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 u32IspDgain)
{
    U_ISP_DG_GAIN1 o_isp_dg_gain1;
    U_ISP_DG_GAIN2 o_isp_dg_gain2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_dg_gain1.u32 = pstIspBeRegs->ISP_DG_GAIN1.u32;
    o_isp_dg_gain1.bits.isp_dg_rgain  = u32IspDgain;
    o_isp_dg_gain1.bits.isp_dg_grgain = u32IspDgain;
    pstIspBeRegs->ISP_DG_GAIN1.u32 = o_isp_dg_gain1.u32;

    o_isp_dg_gain2.u32 = pstIspBeRegs->ISP_DG_GAIN2.u32;
    o_isp_dg_gain2.bits.isp_dg_bgain  = u32IspDgain;
    o_isp_dg_gain2.bits.isp_dg_gbgain = u32IspDgain;
    pstIspBeRegs->ISP_DG_GAIN2.u32 = o_isp_dg_gain2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetIsp4Dgain0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 u32Isp4Dgain0)
{
    U_ISP_4DG_0_GAIN1 o_isp_4dg_0_gain1;
    U_ISP_4DG_0_GAIN2 o_isp_4dg_0_gain2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_4dg_0_gain1.u32 = pstIspBeRegs->ISP_4DG_0_GAIN1.u32;
    o_isp_4dg_0_gain1.bits.isp_4dg0_rgain  = u32Isp4Dgain0;
    o_isp_4dg_0_gain1.bits.isp_4dg0_grgain = u32Isp4Dgain0;
    pstIspBeRegs->ISP_4DG_0_GAIN1.u32 = o_isp_4dg_0_gain1.u32;

    o_isp_4dg_0_gain2.u32 = pstIspBeRegs->ISP_4DG_0_GAIN2.u32;
    o_isp_4dg_0_gain2.bits.isp_4dg0_bgain  = u32Isp4Dgain0;
    o_isp_4dg_0_gain2.bits.isp_4dg0_gbgain = u32Isp4Dgain0;
    pstIspBeRegs->ISP_4DG_0_GAIN2.u32 = o_isp_4dg_0_gain2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetIsp4Dgain1(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 u32Isp4Dgain1)
{
    U_ISP_4DG_1_GAIN1 o_isp_4dg_1_gain1;
    U_ISP_4DG_1_GAIN2 o_isp_4dg_1_gain2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_4dg_1_gain1.u32 = pstIspBeRegs->ISP_4DG_1_GAIN1.u32;
    o_isp_4dg_1_gain1.bits.isp_4dg1_rgain  = u32Isp4Dgain1;
    o_isp_4dg_1_gain1.bits.isp_4dg1_grgain = u32Isp4Dgain1;
    pstIspBeRegs->ISP_4DG_1_GAIN1.u32 = o_isp_4dg_1_gain1.u32;

    o_isp_4dg_1_gain2.u32 = pstIspBeRegs->ISP_4DG_1_GAIN2.u32;
    o_isp_4dg_1_gain2.bits.isp_4dg1_bgain  = u32Isp4Dgain1;
    o_isp_4dg_1_gain2.bits.isp_4dg1_gbgain = u32Isp4Dgain1;
    pstIspBeRegs->ISP_4DG_1_GAIN2.u32 = o_isp_4dg_1_gain2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetIsp4Dgain2(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 u32Isp4Dgain2)
{
    U_ISP_4DG_2_GAIN1 o_isp_4dg_2_gain1;
    U_ISP_4DG_2_GAIN2 o_isp_4dg_2_gain2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_4dg_2_gain1.u32 = pstIspBeRegs->ISP_4DG_2_GAIN1.u32;
    o_isp_4dg_2_gain1.bits.isp_4dg2_rgain  = u32Isp4Dgain2;
    o_isp_4dg_2_gain1.bits.isp_4dg2_grgain = u32Isp4Dgain2;
    pstIspBeRegs->ISP_4DG_2_GAIN1.u32 = o_isp_4dg_2_gain1.u32;

    o_isp_4dg_2_gain2.u32 = pstIspBeRegs->ISP_4DG_2_GAIN2.u32;
    o_isp_4dg_2_gain2.bits.isp_4dg2_bgain  = u32Isp4Dgain2;
    o_isp_4dg_2_gain2.bits.isp_4dg2_gbgain = u32Isp4Dgain2;
    pstIspBeRegs->ISP_4DG_2_GAIN2.u32 = o_isp_4dg_2_gain2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetIsp4Dgain3(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 u32Isp4Dgain3)
{
    U_ISP_4DG_3_GAIN1 o_isp_4dg_3_gain1;
    U_ISP_4DG_3_GAIN2 o_isp_4dg_3_gain2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_4dg_3_gain1.u32 = pstIspBeRegs->ISP_4DG_3_GAIN1.u32;
    o_isp_4dg_3_gain1.bits.isp_4dg3_rgain  = u32Isp4Dgain3;
    o_isp_4dg_3_gain1.bits.isp_4dg3_grgain = u32Isp4Dgain3;
    pstIspBeRegs->ISP_4DG_3_GAIN1.u32 = o_isp_4dg_3_gain1.u32;

    o_isp_4dg_3_gain2.u32 = pstIspBeRegs->ISP_4DG_3_GAIN2.u32;
    o_isp_4dg_3_gain2.bits.isp_4dg3_bgain  = u32Isp4Dgain3;
    o_isp_4dg_3_gain2.bits.isp_4dg3_gbgain = u32Isp4Dgain3;
    pstIspBeRegs->ISP_4DG_3_GAIN2.u32 = o_isp_4dg_3_gain2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrExporratio0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_exporatio0)
{
    U_ISP_WDR_EXPORRATIO o_isp_wdr_exporatio0;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_exporatio0.u32 = pstIspBeRegs->ISP_WDR_EXPORRATIO.u32;
    o_isp_wdr_exporatio0.bits.isp_wdr_exporratio0 = uisp_wdr_exporatio0;
    pstIspBeRegs->ISP_WDR_EXPORRATIO.u32 = o_isp_wdr_exporatio0.u32;
    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrExpoValue0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_expovalue0)
{

    U_ISP_WDR_EXPOVALUE o_isp_wdr_expovalue0;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_expovalue0.u32 = pstIspBeRegs->ISP_WDR_EXPOVALUE.u32;
    o_isp_wdr_expovalue0.bits.isp_wdr_expovalue0 = uisp_wdr_expovalue0;
    pstIspBeRegs->ISP_WDR_EXPOVALUE.u32 = o_isp_wdr_expovalue0.u32;

    return 1;
}
static __inline HI_S32 ISP_DRV_SetWdrExpoValue1(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_expovalue1)
{
    U_ISP_WDR_EXPOVALUE o_isp_wdr_expovalue0;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_expovalue0.u32 = pstIspBeRegs->ISP_WDR_EXPOVALUE.u32;
    o_isp_wdr_expovalue0.bits.isp_wdr_expovalue1 = uisp_wdr_expovalue1;
    pstIspBeRegs->ISP_WDR_EXPOVALUE.u32 = o_isp_wdr_expovalue0.u32;

    return 1;
}


static __inline HI_S32 ISP_DRV_SetFlickExporatio0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_flick_exporatio)
{
    U_ISP_FLICK_EXPORATIO o_isp_flick_exporatio;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_flick_exporatio.u32 = pstIspBeRegs->ISP_FLICK_EXPORATIO.u32;
    o_isp_flick_exporatio.bits.isp_flick_exporatio = uisp_flick_exporatio;
    pstIspBeRegs->ISP_FLICK_EXPORATIO.u32 = o_isp_flick_exporatio.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrBlcComp0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_blc_comp0)
{
    U_ISP_WDR_BLC_COMP o_isp_wdr_blc_comp0;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_blc_comp0.u32 = pstIspBeRegs->ISP_WDR_BLC_COMP.u32;
    o_isp_wdr_blc_comp0.bits.isp_wdr_blc_comp0 = uisp_wdr_blc_comp0;
    pstIspBeRegs->ISP_WDR_BLC_COMP.u32 = o_isp_wdr_blc_comp0.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrMaxRatio(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_maxratio)
{
    U_ISP_WDR_MAXRATIO o_isp_wdr_maxratio;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_maxratio.u32 = pstIspBeRegs->ISP_WDR_MAXRATIO.u32;
    o_isp_wdr_maxratio.bits.isp_wdr_maxratio = uisp_wdr_maxratio;
    pstIspBeRegs->ISP_WDR_MAXRATIO.u32 = o_isp_wdr_maxratio.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrLongThr(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_long_thr)
{
    U_ISP_WDR_WGTIDX_THR o_isp_wdr_wgtidx_thr;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_wgtidx_thr.u32 = pstIspBeRegs->ISP_WDR_WGTIDX_THR.u32;
    o_isp_wdr_wgtidx_thr.bits.isp_wdr_long_thr  = uisp_wdr_long_thr;
    pstIspBeRegs->ISP_WDR_WGTIDX_THR.u32 = o_isp_wdr_wgtidx_thr.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrShortThr(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_short_thr)
{
    U_ISP_WDR_WGTIDX_THR o_isp_wdr_wgtidx_thr;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_wgtidx_thr.u32 = pstIspBeRegs->ISP_WDR_WGTIDX_THR.u32;
    o_isp_wdr_wgtidx_thr.bits.isp_wdr_short_thr  = uisp_wdr_short_thr;
    pstIspBeRegs->ISP_WDR_WGTIDX_THR.u32 = o_isp_wdr_wgtidx_thr.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetWdrMdtEn(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_mdt_en)
{
    U_ISP_WDR_CTRL o_isp_wdr_ctrl;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_ctrl.u32 = pstIspBeRegs->ISP_WDR_CTRL.u32;
    o_isp_wdr_ctrl.bits.isp_wdr_mdt_en = uisp_wdr_mdt_en;
    pstIspBeRegs->ISP_WDR_CTRL.u32     = o_isp_wdr_ctrl.u32;

    return 1;
}


static __inline HI_S32 ISP_DRV_SetWdrFusionMode(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_wdr_fusion_mode)
{
    U_ISP_WDR_CTRL o_isp_wdr_ctrl;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_wdr_ctrl.u32 = pstIspBeRegs->ISP_WDR_CTRL.u32;
    o_isp_wdr_ctrl.bits.isp_wdr_fusionmode = uisp_wdr_fusion_mode;
    pstIspBeRegs->ISP_WDR_CTRL.u32     = o_isp_wdr_ctrl.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetLdciStatEvratio(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_ldci_stat_evratio)
{
    U_ISP_LDCI_STAT_EVRATIO o_isp_ldci_stat_evratio;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_ldci_stat_evratio.u32 = pstIspBeRegs->ISP_LDCI_STAT_EVRATIO.u32;
    o_isp_ldci_stat_evratio.bits.isp_ldci_stat_evratio = uisp_ldci_stat_evratio;
    pstIspBeRegs->ISP_LDCI_STAT_EVRATIO.u32 = o_isp_ldci_stat_evratio.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma0(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_0)
{
    U_ISP_DRC_PREV_LUMA_0 o_isp_drc_prev_luma_0;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_0.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_0.u32;
    o_isp_drc_prev_luma_0.bits.isp_drc_prev_luma_0 = uisp_drc_prev_luma_0;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_0.u32 = o_isp_drc_prev_luma_0.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma1(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_1)
{
    U_ISP_DRC_PREV_LUMA_1 o_isp_drc_prev_luma_1;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_1.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_1.u32;
    o_isp_drc_prev_luma_1.bits.isp_drc_prev_luma_1 = uisp_drc_prev_luma_1;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_1.u32 = o_isp_drc_prev_luma_1.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma2(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_2)
{
    U_ISP_DRC_PREV_LUMA_2 o_isp_drc_prev_luma_2;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_2.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_2.u32;
    o_isp_drc_prev_luma_2.bits.isp_drc_prev_luma_2 = uisp_drc_prev_luma_2;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_2.u32 = o_isp_drc_prev_luma_2.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma3(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_3)
{
    U_ISP_DRC_PREV_LUMA_3 o_isp_drc_prev_luma_3;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_3.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_3.u32;
    o_isp_drc_prev_luma_3.bits.isp_drc_prev_luma_3 = uisp_drc_prev_luma_3;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_3.u32 = o_isp_drc_prev_luma_3.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma4(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_4)
{
    U_ISP_DRC_PREV_LUMA_4 o_isp_drc_prev_luma_4;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_4.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_4.u32;
    o_isp_drc_prev_luma_4.bits.isp_drc_prev_luma_4 = uisp_drc_prev_luma_4;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_4.u32 = o_isp_drc_prev_luma_4.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma5(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_5)
{
    U_ISP_DRC_PREV_LUMA_5 o_isp_drc_prev_luma_5;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_5.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_5.u32;
    o_isp_drc_prev_luma_5.bits.isp_drc_prev_luma_5 = uisp_drc_prev_luma_5;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_5.u32 = o_isp_drc_prev_luma_5.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma6(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_6)
{
    U_ISP_DRC_PREV_LUMA_6 o_isp_drc_prev_luma_6;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_6.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_6.u32;
    o_isp_drc_prev_luma_6.bits.isp_drc_prev_luma_6 = uisp_drc_prev_luma_6;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_6.u32 = o_isp_drc_prev_luma_6.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcPrevLuma7(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_prev_luma_7)
{
    U_ISP_DRC_PREV_LUMA_7 o_isp_drc_prev_luma_7;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_prev_luma_7.u32 = pstIspBeRegs->ISP_DRC_PREV_LUMA_7.u32;
    o_isp_drc_prev_luma_7.bits.isp_drc_prev_luma_7 = uisp_drc_prev_luma_7;
    pstIspBeRegs->ISP_DRC_PREV_LUMA_7.u32 = o_isp_drc_prev_luma_7.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcShpCfg(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_shp_log, HI_U32 uisp_drc_shp_exp)
{
    U_ISP_DRC_SHP_CFG o_isp_drc_shp_cfg;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_shp_cfg.u32 = pstIspBeRegs->ISP_DRC_SHP_CFG.u32;
    o_isp_drc_shp_cfg.bits.isp_drc_shp_log = uisp_drc_shp_log;
    o_isp_drc_shp_cfg.bits.isp_drc_shp_exp = uisp_drc_shp_exp;
    pstIspBeRegs->ISP_DRC_SHP_CFG.u32 = o_isp_drc_shp_cfg.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcDivDenomLog(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_div_denom_log)
{
    U_ISP_DRC_DIV_DENOM_LOG o_isp_drc_div_denom_log;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_div_denom_log.u32 = pstIspBeRegs->ISP_DRC_DIV_DENOM_LOG.u32;
    o_isp_drc_div_denom_log.bits.isp_drc_div_denom_log = uisp_drc_div_denom_log;
    pstIspBeRegs->ISP_DRC_DIV_DENOM_LOG.u32 = o_isp_drc_div_denom_log.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcDenomExp(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_denom_exp)
{
    U_ISP_DRC_DENOM_EXP o_isp_drc_denom_exp;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_denom_exp.u32 = pstIspBeRegs->ISP_DRC_DENOM_EXP.u32;
    o_isp_drc_denom_exp.bits.isp_drc_denom_exp = uisp_drc_denom_exp;
    pstIspBeRegs->ISP_DRC_DENOM_EXP.u32 = o_isp_drc_denom_exp.u32;

    return 1;
}
static __inline HI_S32 ISP_DRV_SetDrcLutMixCtrl(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_lut_mix_ctrl)
{
    U_ISP_DRC_LUT_MIX_CRTL o_isp_drc_lut_mix_crtl;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_lut_mix_crtl.u32 = pstIspBeRegs->ISP_DRC_LUT_MIX_CRTL.u32;
    o_isp_drc_lut_mix_crtl.bits.isp_drc_lut_mix_ctrl = uisp_drc_lut_mix_ctrl;
    pstIspBeRegs->ISP_DRC_LUT_MIX_CRTL.u32 = o_isp_drc_lut_mix_crtl.u32;

    return 1;
}

static __inline HI_S32 ISP_DRV_SetDrcStrength(S_ISPBE_REGS_TYPE *pstIspBeRegs, HI_U32 uisp_drc_strength)
{
    U_ISP_DRC_STRENGTH o_isp_drc_strength;
    ISP_CHECK_POINTER(pstIspBeRegs);

    o_isp_drc_strength.u32 = pstIspBeRegs->ISP_DRC_STRENGTH.u32;
    o_isp_drc_strength.bits.isp_drc_strength = uisp_drc_strength;
    pstIspBeRegs->ISP_DRC_STRENGTH.u32 = o_isp_drc_strength.u32;

    return 1;
}

///*----------------------------------------------------------------------------------------------------------------------------*/

HI_S32 ISP_DRV_BeRemap(void)
{
    HI_U32 IspBePhyPipe = 0;

    for (IspBePhyPipe = 0; IspBePhyPipe < ISP_MAX_BE_NUM; IspBePhyPipe++)
    {
        reg_ispbe_base_va[IspBePhyPipe] = (void *)osal_ioremap(ISP_BE_REG_BASE(IspBePhyPipe), (HI_U32)VI_ISP_BE_REG_SIZE);

        if (HI_NULL == reg_ispbe_base_va[IspBePhyPipe])
        {
            osal_printk( "Remap ISP BE[%d] failed!\n", IspBePhyPipe);
            return HI_FAILURE;
        }

        reg_viproc_base_va[IspBePhyPipe] = (void *)osal_ioremap(ISP_VIPROC_REG_BASE(IspBePhyPipe), (HI_U32)VIPROC_REG_SIZE);

        if (HI_NULL == reg_viproc_base_va[IspBePhyPipe])
        {
            osal_printk( "Remap isp viproc[%d] failed!\n", IspBePhyPipe);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID ISP_DRV_BeUnmap(void)
{
    HI_U32 IspBePhyPipe = 0;

    for (IspBePhyPipe = 0; IspBePhyPipe < ISP_MAX_BE_NUM; IspBePhyPipe++)
    {
        if (reg_ispbe_base_va[IspBePhyPipe] != NULL)
        {
            osal_iounmap(reg_ispbe_base_va[IspBePhyPipe]);
            reg_ispbe_base_va[IspBePhyPipe] = NULL;
        }

        if (reg_viproc_base_va[IspBePhyPipe] != NULL)
        {
            osal_iounmap(reg_viproc_base_va[IspBePhyPipe]);
            reg_viproc_base_va[IspBePhyPipe] = NULL;
        }
    }
}

HI_S32 ISP_DRV_VicapRemap(void)
{
    reg_vicap_base_va = (void *)osal_ioremap(CAP_REG_BASE, (HI_U32)CAP_REG_SIZE_ALIGN);

    if (HI_NULL == reg_vicap_base_va)
    {
        osal_printk( "Remap ISP PT failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID ISP_DRV_VicapUnmap(void)
{
    if (reg_vicap_base_va != NULL) {
        osal_iounmap(reg_vicap_base_va);
        reg_vicap_base_va = NULL;
    }
}

HI_S32 ISP_DRV_FeRemap(void)
{
    HI_U8   i;
    VI_PIPE ViPipe;

    for (ViPipe = 0; ViPipe < ISP_MAX_PHY_PIPE_NUM; ViPipe++) {
        reg_ispfe_base_va[ViPipe] = (void *)osal_ioremap(ISP_FE_REG_BASE(ViPipe), (HI_U32)VI_ISP_FE_REG_SIZE);

        if (reg_ispfe_base_va[ViPipe] == HI_NULL) {
            osal_printk( "Remap ISP[%d] FE failed!\n", ViPipe);
            return HI_FAILURE;
        }

        reg_vicap_ch_base_va[ViPipe] = (void *)osal_ioremap(ISP_VICAP_CH_REG_BASE(ViPipe), (HI_U32)VICAP_CH_REG_SIZE_ALIGN);

        if (reg_vicap_ch_base_va[ViPipe] == HI_NULL) {
            osal_printk( "Remap ISP[%d] FE STT failed!\n", ViPipe);
            return HI_FAILURE;
        }

        /* enable port int & isp int */
        IO_RW_PT_ADDRESS(VICAP_HD_MASK) |= VICAP_INT_MASK_PT(ViPipe);
        IO_RW_PT_ADDRESS(VICAP_HD_MASK) |= VICAP_INT_MASK_ISP(ViPipe);

        IO_RW_FE_ADDRESS(ViPipe, ISP_INT_FE_MASK) = (0x0);
    }

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++) {
        osal_memset(&g_astIspDrvCtx[ViPipe], 0, sizeof(ISP_DRV_CTX_S));
        osal_snprintf(g_astIspDrvCtx[ViPipe].acName, sizeof(g_astIspDrvCtx[ViPipe].acName), "ISP[%d]", ViPipe);

        g_astIspDrvCtx[ViPipe].stWorkMode.enIspRunningMode     = ISP_MODE_RUNNING_OFFLINE;
        g_astIspDrvCtx[ViPipe].stWorkMode.u8BlockNum           = 1;

        g_astIspDrvCtx[ViPipe].stStitchAttr.bStitchEnable      = HI_FALSE;
        g_astIspDrvCtx[ViPipe].stStitchAttr.u8StitchPipeNum    = 2;
        g_astIspDrvCtx[ViPipe].stStitchAttr.as8StitchBindId[0] = 0;
        g_astIspDrvCtx[ViPipe].stStitchAttr.as8StitchBindId[1] = 3;
        g_astIspDrvCtx[ViPipe].stStitchAttr.as8StitchBindId[2] = -1;
        g_astIspDrvCtx[ViPipe].stStitchAttr.as8StitchBindId[3] = -1;

        for (i = 0; i < ISP_STRIPING_MAX_NUM; i++) {
            g_astIspDrvCtx[ViPipe].astChnSelAttr[i].u32ChannelSel = 0;
        }

        g_astIspDrvCtx[ViPipe].enIspRunningState = ISP_BE_BUF_STATE_INIT;
        g_astIspDrvCtx[ViPipe].enIspExitState    = ISP_BE_BUF_EXIT;

        /*snap attr init*/
        g_astIspDrvCtx[ViPipe].stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
        g_astIspDrvCtx[ViPipe].stSnapAttr.s32PicturePipeId = -1;
        g_astIspDrvCtx[ViPipe].stSnapAttr.s32PreviewPipeId = -1;
        g_astIspDrvCtx[ViPipe].stSnapAttr.bLoadCCM         = HI_TRUE;
        g_astIspDrvCtx[ViPipe].stSnapAttr.stProParam.enOperationMode = OPERATION_MODE_AUTO;

        g_astIspDrvCtx[ViPipe].bRunOnceOk      = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bRunOnceFlag    = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bYUVRunOnceOk   = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bYUVRunOnceFlag = HI_FALSE;
        g_astIspDrvCtx[ViPipe].pstUseNode      = HI_NULL;

        for (i = 0; i < PRO_MAX_FRAME_NUM; i++) {
            g_astIspDrvCtx[ViPipe].stSnapAttr.stProParam.stAutoParam.au16ProExpStep[i]   = 256;
            g_astIspDrvCtx[ViPipe].stSnapAttr.stProParam.stManualParam.au32ManExpTime[i] = 10000;
            g_astIspDrvCtx[ViPipe].stSnapAttr.stProParam.stManualParam.au32ManSysgain[i] = 1024;
        }

        g_astIspDrvCtx[ViPipe].stSnapAttr.stProParam.u8ProFrameNum = 3;

        for (i = 0; i < ISP_BAYER_CHN_NUM; i++) {
            g_astIspDrvCtx[ViPipe].stIspBeSyncPara.au32IspDgain[i] = 0x100;
        }

        osal_wait_init(&g_astIspDrvCtx[ViPipe].stIspWait);
        osal_wait_init(&g_astIspDrvCtx[ViPipe].stIspWaitVdStart);
        osal_wait_init(&g_astIspDrvCtx[ViPipe].stIspWaitVdEnd);
        osal_wait_init(&g_astIspDrvCtx[ViPipe].stIspWaitVdBeEnd);
        osal_wait_init(&g_astIspDrvCtx[ViPipe].stIspExitWait);
        g_astIspDrvCtx[ViPipe].bEdge          = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bVdStart       = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bVdEnd         = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bVdBeEnd       = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bMemInit       = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bIspInit       = HI_FALSE;
        g_astIspDrvCtx[ViPipe].bIspRunFlag    = HI_FALSE;
        g_astIspDrvCtx[ViPipe].u32ProTrigFlag = 0;
        osal_sema_init(&g_astIspDrvCtx[ViPipe].stIspSem, 1);
        osal_sema_init(&g_astIspDrvCtx[ViPipe].stIspSemVd, 1);
        osal_sema_init(&g_astIspDrvCtx[ViPipe].stIspSemBeVd, 1);
        osal_sema_init(&g_astIspDrvCtx[ViPipe].stProcSem, 1);
        osal_sema_init(&g_astIspDrvCtx[ViPipe].stProInfoSem, 1);
        SyncTaskInit(ViPipe);
    }

    return HI_SUCCESS;
}

HI_VOID ISP_DRV_FeUnmap(void)
{
    VI_PIPE ViPipe;

    for (ViPipe = 0; ViPipe < ISP_MAX_PHY_PIPE_NUM; ViPipe++) {
        if (reg_ispfe_base_va[ViPipe] != NULL) {
            osal_iounmap(reg_ispfe_base_va[ViPipe]);
            reg_ispfe_base_va[ViPipe] = NULL;
        }

        if (reg_vicap_ch_base_va[ViPipe] != NULL) {
            osal_iounmap(reg_vicap_ch_base_va[ViPipe]);
            reg_vicap_ch_base_va[ViPipe] = NULL;
        }
    }

    for (ViPipe = 0; ViPipe < ISP_MAX_PIPE_NUM; ViPipe++) {
        osal_sema_destory(&g_astIspDrvCtx[ViPipe].stIspSem);
        osal_sema_destory(&g_astIspDrvCtx[ViPipe].stIspSemVd);
        osal_sema_destory(&g_astIspDrvCtx[ViPipe].stIspSemBeVd);
        osal_sema_destory(&g_astIspDrvCtx[ViPipe].stProcSem);
        osal_sema_destory(&g_astIspDrvCtx[ViPipe].stProInfoSem);

        osal_wait_destory(&g_astIspDrvCtx[ViPipe].stIspWait);
        osal_wait_destory(&g_astIspDrvCtx[ViPipe].stIspWaitVdStart);
        osal_wait_destory(&g_astIspDrvCtx[ViPipe].stIspWaitVdEnd);
        osal_wait_destory(&g_astIspDrvCtx[ViPipe].stIspWaitVdBeEnd);
        osal_wait_destory(&g_astIspDrvCtx[ViPipe].stIspExitWait);

        SyncTaskExit(ViPipe);
    }
}

HI_S32 ISP_DRV_GetViprocRegsAttr(VI_PIPE ViPipe, S_VIPROC_REGS_TYPE *apstViprocReg[], ISP_BE_REGS_ATTR_S *pstBlkAttr)
{
    HI_U8  k;
    HI_S8  s8BlockId = 0;
    HI_U8  u8BlkDev  = 0;
    HI_U8  u8BlkNum  = 1;
    ISP_RUNNING_MODE_E  enIspRunningMode;
    ISP_DRV_CTX_S       *pstDrvCtx      = HI_NULL;
    ISP_BE_WO_REG_CFG_S *pstIspBeRegCfg = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstBlkAttr);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    enIspRunningMode = pstDrvCtx->stWorkMode.enIspRunningMode;

    u8BlkNum = ISP_DRV_GetBlockNum(ViPipe);
    u8BlkNum = DIV_0_TO_1(u8BlkNum);
    s8BlockId = ISP_DRV_GetBlockId(ViPipe, enIspRunningMode);

    if (-1 == s8BlockId)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Online Mode Pipe Err!\n", ViPipe);
        return HI_FAILURE;
    }

    u8BlkDev = (HI_U8)s8BlockId;
    pstBlkAttr->u8BlockDev = u8BlkDev;
    pstBlkAttr->u8BlockNum = u8BlkNum;

    switch ( enIspRunningMode )
    {
        case ISP_MODE_RUNNING_ONLINE :
            ISP_DRV_VIPROCREG_CTX(u8BlkDev, apstViprocReg[u8BlkDev]);
            break;

        case ISP_MODE_RUNNING_SIDEBYSIDE :
            for (k = 0; k < ISP_MAX_BE_NUM; k++)
            {
                ISP_DRV_VIPROCREG_CTX(k, apstViprocReg[k]);
            }

            break;
        case ISP_MODE_RUNNING_OFFLINE:
            ISP_CHECK_BEBUF_INIT(ViPipe);
            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                ISP_TRACE(HI_DBG_WARN, "ISP[%d] UseNode is Null!\n", ViPipe);
                return HI_FAILURE;
            }
            pstIspBeRegCfg = (ISP_BE_WO_REG_CFG_S *)pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
            apstViprocReg[u8BlkDev] = &pstIspBeRegCfg->stBeRegCfg[u8BlkDev].stViProcReg;

            break;

        case ISP_MODE_RUNNING_STRIPING :
            ISP_CHECK_BEBUF_INIT(ViPipe);
            if (HI_NULL == pstDrvCtx->pstUseNode)
            {
                ISP_TRACE(HI_DBG_WARN, "ISP[%d] UseNode is Null!\n", ViPipe);
                return HI_FAILURE;
            }
            pstIspBeRegCfg = (ISP_BE_WO_REG_CFG_S *)pstDrvCtx->pstUseNode->stBeCfgBuf.pVirAddr;
            for (k = 0; k < ISP_STRIPING_MAX_NUM; k++)
            {
                apstViprocReg[k] = &pstIspBeRegCfg->stBeRegCfg[k].stViProcReg;
            }
            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Running Mode Err!\n", ViPipe);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetOnlineSttAddr(VI_PIPE ViPipe, HI_U64 u64PhyBaseAddr)
{
    HI_U8   k;
    HI_U8   u8BlkDev = 0;
    HI_U8   u8BlkNum = 1;
    HI_U32  s32Ret   = HI_SUCCESS;
    HI_U64  u64SttSize = 0;
    HI_U64  u64PhyAddr = 0;
    ISP_BE_REGS_ATTR_S  stIspBeRegsAttr = {0};
    S_VIPROC_REGS_TYPE *apstViprocReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_PIPE(ViPipe);

    s32Ret = ISP_DRV_GetViprocRegsAttr(ViPipe, apstViprocReg, &stIspBeRegsAttr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get ViprocRegs Attr Err!\n", ViPipe);
        return HI_FAILURE;
    }

    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    u64SttSize = sizeof(S_ISP_ONLINE_STT_REGS_TYPE);

    for (k = 0; k < u8BlkNum; k++)
    {
        u64PhyAddr = u64PhyBaseAddr + k * u64SttSize;

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_AE_HIST_ADDR_LOW.u32         = GetLowAddr (u64PhyAddr + ISP_AE_HIST_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_AE_AVER_R_GR_ADDR_LOW.u32    = GetLowAddr (u64PhyAddr + ISP_AE_AVER_R_GR_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_AE_AVER_GB_B_ADDR_LOW.u32    = GetLowAddr (u64PhyAddr + ISP_AE_AVER_GB_B_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_AWB_STAT_ADDR_LOW.u32        = GetLowAddr (u64PhyAddr + ISP_AWB_STAT_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_AF_STAT_ADDR_LOW.u32         = GetLowAddr (u64PhyAddr + ISP_AF_STAT_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_LA_AVER_ADDR_LOW.u32         = GetLowAddr (u64PhyAddr + ISP_LA_AVER_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_DEHAZE_MIN_STAT_ADDR_LOW.u32 = GetLowAddr (u64PhyAddr + ISP_DEHAZE_MIN_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_DEHAZE_MAX_STAT_ADDR_LOW.u32 = GetLowAddr (u64PhyAddr + ISP_DEHAZE_MAX_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_LDCI_LPF_ADDR_LOW.u32        = GetLowAddr (u64PhyAddr + ISP_LDCI_LPF_MAP_ADDR_OFFSET);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISP_FLICK_GMEAN_ADDR_LOW.u32     = GetLowAddr (u64PhyAddr + ISP_FLICK_GMEAN_ADDR_OFFSET);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SetFeSttAddr(VI_PIPE ViPipeBind, HI_U64 u64PhyAddr)
{
    S_ISP_VICAP_CH_REGS_TYPE *pstFeSttAddrReg = HI_NULL;

    //ISP_CHECK_PIPE(ViPipeBind);
    ISP_CHECK_VIR_PIPE(ViPipeBind);
    ISP_DRV_FESTTREG_CTX(ViPipeBind, pstFeSttAddrReg);

    pstFeSttAddrReg->CH_WCH_STT_AE_HIST_ADDR_L.u32      = GetLowAddr (u64PhyAddr + ISP_AE_HIST_ADDR_OFFSET);

    pstFeSttAddrReg->CH_WCH_STT_AE_AVER_R_GR_ADDR_L.u32 = GetLowAddr (u64PhyAddr + ISP_AE_AVER_R_GR_ADDR_OFFSET);

    pstFeSttAddrReg->CH_WCH_STT_AE_AVER_GB_B_ADDR_L.u32 = GetLowAddr (u64PhyAddr + ISP_AE_AVER_GB_B_ADDR_OFFSET);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigChnSel(S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8   k;
    HI_U32  au32ChnSwitch[5]  = {0};
    HI_U32  u32ChannelSel     = 0;

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
    }

    ISP_CHECK_POINTER(pstDrvCtx);

    u32ChannelSel = pstDrvCtx->astChnSelAttr[0].u32ChannelSel;

    if (HI_TRUE == pstDrvCtx->bYuvMode)
    {
        au32ChnSwitch[4] = 1;
    }
    else
    {
        au32ChnSwitch[4] = 0;
    }

    if (IS_FULL_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
    {
        switch ( u32ChannelSel & 0x3 )
        {
            case 0:
                au32ChnSwitch[0] = (pstDrvCtx->stSyncCfg.u8VCNumMax - pstDrvCtx->stSyncCfg.u8VCNum) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[1] = (au32ChnSwitch[0] + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[2] = (au32ChnSwitch[0] + 2) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[3] = (au32ChnSwitch[0] + 3) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                break;

            case 1:
                au32ChnSwitch[1] = (pstDrvCtx->stSyncCfg.u8VCNumMax - pstDrvCtx->stSyncCfg.u8VCNum) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[0] = (au32ChnSwitch[1] + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[2] = (au32ChnSwitch[1] + 2) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[3] = (au32ChnSwitch[1] + 3) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                break;

            case 2:
                au32ChnSwitch[2] = (pstDrvCtx->stSyncCfg.u8VCNumMax - pstDrvCtx->stSyncCfg.u8VCNum) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[1] = (au32ChnSwitch[2] + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[0] = (au32ChnSwitch[2] + 2) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[3] = (au32ChnSwitch[2] + 3) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                break;

            case 3:
                au32ChnSwitch[3] = (pstDrvCtx->stSyncCfg.u8VCNumMax - pstDrvCtx->stSyncCfg.u8VCNum) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[2] = (au32ChnSwitch[3] + 1) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[1] = (au32ChnSwitch[3] + 2) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                au32ChnSwitch[0] = (au32ChnSwitch[3] + 3) % DIV_0_TO_1(pstDrvCtx->stSyncCfg.u8VCNumMax + 1);
                break;
        }

        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_DRV_SetInputSel(apstBeReg[k + u8BlkDev], &au32ChnSwitch[0]);
        }

    }
    else if ((IS_LINE_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode)) ||
             (IS_HALF_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode)))
    {
        switch ( u32ChannelSel & 0x3 )
        {
            case 0:
                au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
                au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
                au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
                au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
                break;

            case 1:
                au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
                au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
                au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
                au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
                break;

            case 2:
                au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
                au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
                au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
                au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
                break;

            case 3:
                au32ChnSwitch[3] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[0];
                au32ChnSwitch[2] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[1];
                au32ChnSwitch[1] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[2];
                au32ChnSwitch[0] = pstDrvCtx->astChnSelAttr[0].au32WdrChnSel[3];
                break;
        }

        /* offline mode: isp BE buffer poll, so chn switch need each frame refres */
        if (IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) ||
            (IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)))
        {
            for (k = 0; k < u8BlkNum; k++)
            {
                ISP_DRV_SetInputSel(apstBeReg[k + u8BlkDev], &au32ChnSwitch[0]);
            }
        }
    }
    else
    {
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigWdr(VI_PIPE ViPipe, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, ISP_FSWDR_SYNC_CFG_S *pstWDRRegCfg, HI_U8 u8BlkNum, HI_U8 u8BlkDev, HI_U32 *u32Ratio)
{
    HI_U8   k;
    HI_U8   u8LFMode          = 0;
    HI_U32  au32ExpoValue[3]  = {0};
    HI_U32  au32BlcComp[3]    = {0};
    HI_S32  u32MaxRatio       = 0;
    HI_U16  u16LongThr        = 0xFFF;
    HI_U16  u16ShortThr       = 0xFFF;
    HI_BOOL bWDRMdtEn         = 0;
    HI_U8   u8CfgDlyMax;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstDrvCtx);
    ISP_CHECK_POINTER(pstWDRRegCfg);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
    }

    u8CfgDlyMax = ISP_DRV_GetBeSyncIndex(pstDrvCtx);
    u8CfgDlyMax = MIN2(u8CfgDlyMax, CFG2VLD_DLY_LIMIT - 1);

    u8LFMode = pstDrvCtx->stSyncCfg.u8LFMode[u8CfgDlyMax];

    u16LongThr  = pstWDRRegCfg->u16LongThr;
    u16ShortThr = pstWDRRegCfg->u16ShortThr;
    bWDRMdtEn   = pstWDRRegCfg->bWDRMdtEn;

    for (k = 0; k < u8BlkNum; k++)
    {
        if (IS_2to1_WDR_MODE(pstDrvCtx->stSyncCfg.u8WDRMode))
        {
            ISP_DRV_SetWdrExporratio0(apstBeReg[k + u8BlkDev], MIN2((ISP_BITMASK(10) * 64 / DIV_0_TO_1(u32Ratio[0])), 0x3FF));
            ISP_DRV_SetFlickExporatio0(apstBeReg[k + u8BlkDev], MIN2(u32Ratio[0], 0X3FFF));

            pstDrvCtx->stIspBeSyncPara.u32WdrExpRatio   = MIN2((ISP_BITMASK(10) * 64 / DIV_0_TO_1(u32Ratio[0])), 0x3FF);
            pstDrvCtx->stIspBeSyncPara.u32FlickExpRatio = MIN2(u32Ratio[0], 0X3FFF);

            au32ExpoValue[0] = MIN2(u32Ratio[0], ISP_BITMASK(14));
            au32ExpoValue[1] = MIN2(64, ISP_BITMASK(14));

            if ( 0 == pstWDRRegCfg->bFusionMode)
            {
                ISP_DRV_SetWdrExpoValue0(apstBeReg[k + u8BlkDev], au32ExpoValue[0]);
                ISP_DRV_SetWdrExpoValue1(apstBeReg[k + u8BlkDev], au32ExpoValue[1]);

                pstDrvCtx->stIspBeSyncPara.au32WdrExpVal[0] = au32ExpoValue[0];
                pstDrvCtx->stIspBeSyncPara.au32WdrExpVal[1] = au32ExpoValue[1];

                u32MaxRatio = ((1 << 22) - 1) / DIV_0_TO_1(au32ExpoValue[0]);
            }
            else
            {
                ISP_DRV_SetWdrExpoValue0(apstBeReg[k + u8BlkDev], MIN2((au32ExpoValue[0] + 64), ISP_BITMASK(14)));
                ISP_DRV_SetWdrExpoValue1(apstBeReg[k + u8BlkDev], au32ExpoValue[1]);

                pstDrvCtx->stIspBeSyncPara.au32WdrExpVal[0] = MIN2((au32ExpoValue[0] + 64), ISP_BITMASK(14));
                pstDrvCtx->stIspBeSyncPara.au32WdrExpVal[1] = au32ExpoValue[1];

                u32MaxRatio = ((1 << 22) - 1) / DIV_0_TO_1(au32ExpoValue[0] + 64);
            }

            au32BlcComp[0] = (au32ExpoValue[0] - au32ExpoValue[1]) * pstWDRRegCfg->u16Offset0;

            ISP_DRV_SetWdrBlcComp0(apstBeReg[k + u8BlkDev], au32BlcComp[0]);

            pstDrvCtx->stIspBeSyncPara.u32WdrBlcComp = au32BlcComp[0];

            ISP_DRV_SetWdrMaxRatio(apstBeReg[k + u8BlkDev], u32MaxRatio);

            pstDrvCtx->stIspBeSyncPara.u32WdrMaxRatio = u32MaxRatio;

            ISP_DRV_SetWdrFusionMode(apstBeReg[k + u8BlkDev], pstWDRRegCfg->bFusionMode);

            pstDrvCtx->stIspBeSyncPara.bFusionMode = pstWDRRegCfg->bFusionMode;
        }

        if ((0 != u8LFMode) && (au32ExpoValue[0] < 0x44))
        {
            u16LongThr  = 0xFFF;
            u16ShortThr = 0xFFF;
            bWDRMdtEn   = 0;
        }

        ISP_DRV_SetWdrLongThr(apstBeReg[k + u8BlkDev],  u16LongThr);
        ISP_DRV_SetWdrShortThr(apstBeReg[k + u8BlkDev], u16ShortThr);
        ISP_DRV_SetWdrMdtEn(apstBeReg[k + u8BlkDev],    bWDRMdtEn);
    }

    pstDrvCtx->stIspBeSyncPara.u16LongThr  = u16LongThr;
    pstDrvCtx->stIspBeSyncPara.u16ShortThr = u16ShortThr;
    pstDrvCtx->stIspBeSyncPara.bWDRMdtEn   = bWDRMdtEn;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigDe(VI_PIPE ViPipe, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_SYNC_CFG_BUF_NODE_S *pstCfgNode)
{
    HI_BOOL bNry0En, bDeEn;
    HI_U8   k;
    HI_U8   u8BlkDev, u8BlkNum;
    HI_S32  s32Ret;
    ISP_BE_REGS_ATTR_S  stIspBeRegsAttr = {0};
    S_VIPROC_REGS_TYPE *apstViprocReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstCfgNode);

    s32Ret = ISP_DRV_GetViprocRegsAttr(ViPipe, apstViprocReg, &stIspBeRegsAttr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get ViprocRegs Attr Err!\n", ViPipe);
        return HI_FAILURE;
    }

    u8BlkDev = stIspBeRegsAttr.u8BlockDev;
    u8BlkNum = stIspBeRegsAttr.u8BlockNum;

    bDeEn   = pstCfgNode->stDeRegCfg.bDeEnable;
    bNry0En = HI_FALSE;

    if (CKFN_VI_GetPipeNRXStatus())
    {
        s32Ret = CALL_VI_GetPipeNRXStatus(ViPipe, &bNry0En);

        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "Pipe:%d CALL_VI_GetPipeNRXStatus failed 0x%x!\n", ViPipe, s32Ret);

            for (k = 0; k < u8BlkNum; k++)
            {
                ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
                apstBeReg[k + u8BlkDev]->ISP_BNR_DETAIL.bits.isp_bnr_de_enable   = bDeEn;
                apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL1.bits.isp_de_en   = bDeEn;
                apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL1.bits.isp_nry0_en = HI_FALSE;
                apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL0.bits.isp_bnr_en  = pstCfgNode->stDeRegCfg.bBnrEnable;
            }

            return s32Ret;
        }

        if (HI_TRUE == bNry0En)
        {
            bDeEn = HI_FALSE;
        }
    }

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
        apstBeReg[k + u8BlkDev]->ISP_BNR_DETAIL.bits.isp_bnr_de_enable   = bDeEn;
        apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL1.bits.isp_de_en   = bDeEn;
        apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL1.bits.isp_nry0_en = bNry0En;
        apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL0.bits.isp_bnr_en  = pstCfgNode->stDeRegCfg.bBnrEnable;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigLdci(S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8   k;
    HI_U32  u32LdciComp = 0x1000;
    HI_U32  u32LdciCompIndex = 2;

    ISP_CHECK_POINTER(pstDrvCtx);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
    }

    u32LdciCompIndex = ISP_DRV_GetBeSyncIndex(pstDrvCtx);
    if (u32LdciCompIndex >= 1)
    {
        u32LdciCompIndex = u32LdciCompIndex - 1; // ldci compensate is earlier tham drc one frame
    }
    else
    {
        u32LdciCompIndex = 0;
    }
    u32LdciCompIndex = MIN2(u32LdciCompIndex, CFG2VLD_DLY_LIMIT - 1);


    u32LdciComp = pstDrvCtx->stSyncCfg.u32DRCComp[u32LdciCompIndex];
    u32LdciComp = Sqrt32(u32LdciComp << DRC_COMP_SHIFT);
    u32LdciComp = MIN2(u32LdciComp, 0xFFFF);
    if (SNAP_STATE_CFG == pstDrvCtx->stSnapInfoLoad.enSnapState)
    {
        u32LdciComp = 0x1000;
    }

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_DRV_SetLdciStatEvratio(apstBeReg[k + u8BlkDev], u32LdciComp);
    }

    pstDrvCtx->stIspBeSyncPara.u32LdciComp = u32LdciComp;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigDrc(VI_PIPE ViPipe, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, ISP_DRC_REG_CFG_2_S *pstDRCRegCfg, HI_U8 u8BlkNum, HI_U8 u8BlkDev, HI_U32 *u32Ratio)
{
    HI_U8  i, k;
    HI_U8  u8CfgDlyMax;
    HI_U32 u32DrcDivDenomLog;
    HI_U32 u32DrcDenomExp;
    HI_U32 u32DrcExpRatio = 0x1000;
    HI_U32 au32DrcPrevLuma[SYNC_DRC_PRELUMA_NUM] = {0};
    HI_BOOL bUpdateLogParam = HI_FALSE;

    HI_U32 au32DrcDivDenomLog[16] = {52429, 55188,  58254,  61681,  65536,  69905,  74898, 80659, \
                                     87379, 95319, 104843, 116472, 130980, 149557, 174114, 207870
                                    };

    HI_U32 au32DrcDenomExp[16] = {1310720, 1245184, 1179648, 1114113, 1048577, 983043, 917510, 851980, \
                                  786455,  720942,  655452,  590008,  524657, 459488, 394682, 330589
                                 };

    static HI_U8 u8DrcShpLog [ISP_MAX_PIPE_NUM][ISP_STRIPING_MAX_NUM] = \
    {[0 ... ISP_MAX_PIPE_NUM - 1] = {8, 8, 8}};
    static HI_U8 u8DrcShpExp [ISP_MAX_PIPE_NUM][ISP_STRIPING_MAX_NUM] = \
    {[0 ... ISP_MAX_PIPE_NUM - 1] = {8, 8, 8}};

    ISP_CHECK_POINTER(pstDrvCtx);
    ISP_CHECK_POINTER(pstDRCRegCfg);
    ISP_CHECK_POINTER(u32Ratio);
    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
    }

    /* config drc compensation */
    u8CfgDlyMax = ISP_DRV_GetBeSyncIndex(pstDrvCtx);
    u8CfgDlyMax = MIN2(u8CfgDlyMax, CFG2VLD_DLY_LIMIT - 1);

    u32DrcExpRatio = pstDrvCtx->stSyncCfg.u32DRCComp[u8CfgDlyMax];

    if (u32DrcExpRatio != 0x1000) // do division only when u32DrcExpRatio != 4096
    {
        u32DrcExpRatio = DIV_0_TO_1(u32DrcExpRatio);
        u32DrcExpRatio = osal_div_u64((1 << (DRC_COMP_SHIFT + DRC_COMP_SHIFT)), u32DrcExpRatio);
        u32DrcExpRatio = MIN2(u32DrcExpRatio, (15 << DRC_COMP_SHIFT)); // Maximum supported ratio is 15
    }

    if (SNAP_STATE_CFG == pstDrvCtx->stSnapInfoLoad.enSnapState)
    {
        u32DrcExpRatio = 0x1000;
    }

    if (u8DrcShpLog[ViPipe][u8BlkDev] != pstDRCRegCfg->u8ShpLog || \
        u8DrcShpExp[ViPipe][u8BlkDev] != pstDRCRegCfg->u8ShpExp)
    {
        u8DrcShpLog[ViPipe][u8BlkDev] = pstDRCRegCfg->u8ShpLog;
        u8DrcShpExp[ViPipe][u8BlkDev] = pstDRCRegCfg->u8ShpExp;
        bUpdateLogParam = HI_TRUE;
    }
    else
    {
        bUpdateLogParam = HI_FALSE;
    }

    // Compensate on PrevLuma when ShpLog/ShpExp is modified, but no compensation under offline repeat mode
    if (bUpdateLogParam && (!pstDRCRegCfg->bIsOfflineRepeatMode))
    {
        for (i = 0; i < SYNC_DRC_PRELUMA_NUM - 1; i++)
        {
            au32DrcPrevLuma[i] = (HI_U32)((HI_S32)g_DrcCurLumaLut[u8DrcShpLog[ViPipe][u8BlkDev]][i] + pstDRCRegCfg->as32PrevLumaDelta[i]);
        }
    }
    else
    {
        for (i = 0; i < SYNC_DRC_PRELUMA_NUM - 1; i++)
        {
            au32DrcPrevLuma[i] = g_DrcCurLumaLut[u8DrcShpLog[ViPipe][u8BlkDev]][i];
        }
    }
    au32DrcPrevLuma[SYNC_DRC_PRELUMA_NUM - 1] = (1 << 20);

    if ((u32DrcExpRatio != 0x1000) && (!pstDRCRegCfg->bIsOfflineRepeatMode))
    {
        for (i = 0; i < SYNC_DRC_PRELUMA_NUM; i++)
        {
            au32DrcPrevLuma[i] = (HI_U32)(((HI_U64)u32DrcExpRatio * au32DrcPrevLuma[i]) >> DRC_COMP_SHIFT);
        }
    }

    u32DrcDivDenomLog = au32DrcDivDenomLog[u8DrcShpLog[ViPipe][u8BlkDev]];
    u32DrcDenomExp    = au32DrcDenomExp[u8DrcShpExp[ViPipe][u8BlkDev]];

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_DRV_SetDrcShpCfg(apstBeReg[k + u8BlkDev], u8DrcShpLog[ViPipe][u8BlkDev], u8DrcShpExp[ViPipe][u8BlkDev]);
        ISP_DRV_SetDrcDivDenomLog(apstBeReg[k + u8BlkDev], u32DrcDivDenomLog);
        ISP_DRV_SetDrcDenomExp(apstBeReg[k + u8BlkDev], u32DrcDenomExp);

        ISP_DRV_SetDrcPrevLuma0(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[0]);
        ISP_DRV_SetDrcPrevLuma1(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[1]);
        ISP_DRV_SetDrcPrevLuma2(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[2]);
        ISP_DRV_SetDrcPrevLuma3(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[3]);
        ISP_DRV_SetDrcPrevLuma4(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[4]);
        ISP_DRV_SetDrcPrevLuma5(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[5]);
        ISP_DRV_SetDrcPrevLuma6(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[6]);
        ISP_DRV_SetDrcPrevLuma7(apstBeReg[k + u8BlkDev], au32DrcPrevLuma[7]);
    }

    for (i = 0; i < SYNC_DRC_PRELUMA_NUM; i++)
    {
        pstDrvCtx->stIspBeSyncPara.au32DrcPrevLuma[i] = au32DrcPrevLuma[i];
    }
    pstDrvCtx->stIspBeSyncPara.u8DrcShpLog       = u8DrcShpLog[ViPipe][u8BlkDev];
    pstDrvCtx->stIspBeSyncPara.u32DrcDivDenomLog = u32DrcDivDenomLog;
    pstDrvCtx->stIspBeSyncPara.u32DrcDenomExp    = u32DrcDenomExp;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfigDgain(S_ISPBE_REGS_TYPE *apstBeReg[], HI_U32  u32IspDgain, HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8 k;

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
        ISP_DRV_SetIspDgain(apstBeReg[k + u8BlkDev], u32IspDgain);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ConfigCCM( ISP_DRV_CTX_S *pstDrvCtx, HI_U16 *au16CCM)
{
#if 0
    HI_U8 i;

    ISP_CHECK_POINTER(pstDrvCtx);

    for (i = 0; i < CCM_MATRIX_SIZE; i++)
    {
        pstDrvCtx->stIspBeSyncPara.au16CCM[i] = au16CCM[i];
    }
#endif
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_RegConfig4Dgain(VI_PIPE ViPipe, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, HI_U32 *au32WDRGain, HI_U8 u8BlkNum, HI_U8 u8BlkDev, HI_U32 *au32WDRWBGain, HI_U32 *au32WBViGain)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 k;
    ISP_BE_REGS_ATTR_S  stIspBeRegsAttr = {0};
    S_VIPROC_REGS_TYPE *apstViprocReg[ISP_STRIPING_MAX_NUM] = {HI_NULL};

    s32Ret = ISP_DRV_GetViprocRegsAttr(ViPipe, apstViprocReg, &stIspBeRegsAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get ViprocRegs Attr Err!\n", ViPipe);
        return s32Ret;
    }

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstViprocReg[k + u8BlkDev]);

        apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL0.bits.isp_4dg_en = 0;
        if (IS_FS_WDR_MODE(pstDrvCtx->stWDRAttr.enWDRMode))
        {
            apstViprocReg[k + u8BlkDev]->VIPROC_ISPBE_CTRL0.bits.isp_4dg_en = 1;
        }

        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
        ISP_DRV_SetIsp4Dgain0(apstBeReg[k + u8BlkDev], au32WDRGain[0]);
        ISP_DRV_SetIsp4Dgain1(apstBeReg[k + u8BlkDev], au32WDRGain[1]);
    }

    pstDrvCtx->stIspBeSyncPara.au32WDRGain[0] = au32WDRGain[0];
    pstDrvCtx->stIspBeSyncPara.au32WDRGain[1] = au32WDRGain[1];

    return HI_SUCCESS;
}

/*read FE statistics information*/

HI_S32 ISP_DRV_FE_AE_Global_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_FE_STT_REGS_TYPE *pstFeSttReg, HI_U32 k)
{
    HI_S32 i;

    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(pstFeSttReg);

    for (i = 0; i < 1024; i++)
    {
        pstStat->stFEAeStat1.au32HistogramMemArray[k][i] = pstFeSttReg->ISP_AE_HIST[i].u32;//pstFeReg->ISP_AE1_HIST_RDATA.u32;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_AE_Local_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_FE_STT_REGS_TYPE *pstFeSttReg, HI_U32 k)
{
    HI_S32 i, j;
    HI_U32 u32AveMem = 0;

    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(pstFeSttReg);

    for (i = 0; i < AE_ZONE_ROW; i++)
    {
        for (j = 0; j < AE_ZONE_COLUMN; j++)
        {
            u32AveMem = pstFeSttReg->ISP_AE_AVER_R_GR[i * AE_ZONE_COLUMN + j].u32;//pstFeReg->ISP_AE1_AVER_R_GR_RDATA.u32;
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][1] = (HI_U16)((u32AveMem & 0xFFFF));

            u32AveMem = pstFeSttReg->ISP_AE_AVER_GB_B[i * AE_ZONE_COLUMN + j].u32;//pstFeReg->ISP_AE1_AVER_GB_B_RDATA.u32;
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][3] = (HI_U16)((u32AveMem & 0xFFFF));
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_APB_StatisticsRead(ISP_STAT_S *pstStat, S_ISPFE_REGS_TYPE *pstFeReg, VI_PIPE ViPipeBind, HI_U32 k, ISP_STAT_KEY_U unStatkey)
{
    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(pstFeReg);

    if (unStatkey.bit1FEAeGloStat)
    {
        pstStat->stFEAeStat1.u32PixelWeight[k] = pstFeReg->ISP_AE1_COUNT_STAT.u32;
        pstStat->stFEAeStat1.u32PixelCount[k]  = pstFeReg->ISP_AE1_TOTAL_STAT.u32;

        pstStat->stFEAeStat2.u16GlobalAvgR[k]  = pstFeReg->ISP_AE1_TOTAL_R_AVER.u32;
        pstStat->stFEAeStat2.u16GlobalAvgGr[k] = pstFeReg->ISP_AE1_TOTAL_GR_AVER.u32;
        pstStat->stFEAeStat2.u16GlobalAvgGb[k] = pstFeReg->ISP_AE1_TOTAL_GB_AVER.u32;
        pstStat->stFEAeStat2.u16GlobalAvgB[k]  = pstFeReg->ISP_AE1_TOTAL_B_AVER.u32;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_AE_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_FE_STT_REGS_TYPE *pstFeSttReg, HI_U32 k)
{
    HI_S32 i, j;
    HI_U32 u32AveMem = 0;

    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(pstFeSttReg);

    for (i = 0; i < 1024; i++)
    {
        pstStat->stFEAeStat1.au32HistogramMemArray[k][i] = pstFeSttReg->ISP_AE_HIST[i].u32;
    }

    for (i = 0; i < AE_ZONE_ROW; i++)
    {
        for (j = 0; j < AE_ZONE_COLUMN; j++)
        {
            u32AveMem = pstFeSttReg->ISP_AE_AVER_R_GR[i * AE_ZONE_COLUMN + j].u32;
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][1] = (HI_U16)((u32AveMem & 0xFFFF));

            u32AveMem = pstFeSttReg->ISP_AE_AVER_GB_B[i * AE_ZONE_COLUMN + j].u32;
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
            pstStat->stFEAeStat3.au16ZoneAvg[k][i][j][3] = (HI_U16)((u32AveMem & 0xFFFF));
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_AF_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISPFE_REGS_TYPE *pstFeReg, S_ISP_FE_STT_REGS_TYPE *pstFeSttReg, VI_PIPE ViPipeBind, HI_U32 k)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_SwitchSttAddr(VI_PIPE ViPipeBind, HI_U8 u8WdrIndex, ISP_DRV_CTX_S *pstDrvCtx)
{
    HI_U32  u32CurReadFlag;

    ISP_CHECK_PIPE(ViPipeBind);
    ISP_CHECK_POINTER(pstDrvCtx);

    if ((0 != pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].u32CurWriteFlag) &&
        (1 != pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].u32CurWriteFlag))
    {
        ISP_TRACE(HI_DBG_ERR, "Err FE u32CurWriteFlag != 0/1 !!!\n");
        return HI_FAILURE;
    }

    u32CurReadFlag = pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].u32CurWriteFlag;

    if (HI_TRUE == pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].bFirstFrame)
    {
        osal_memcpy(pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].astFeSttBuf[1].pVirAddr, \
                    pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].astFeSttBuf[0].pVirAddr, sizeof(S_ISP_FE_STT_REGS_TYPE));

        u32CurReadFlag = 1;
        pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].bFirstFrame = HI_FALSE;
    }

    pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].u32CurWriteFlag = 1 - u32CurReadFlag;

    ISP_DRV_SetFeSttAddr(ViPipeBind, pstDrvCtx->stFeSttAttr.stFeSttBuf[u8WdrIndex].astFeSttBuf[u32CurReadFlag].u64PhyAddr);

    return HI_SUCCESS;
}


HI_S32 ISP_DRV_FE_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISPFE_REGS_TYPE *pstFeReg, ISP_DRV_CTX_S *pstDrvCtx, VI_PIPE ViPipeBind, HI_U32 k, ISP_STAT_KEY_U unStatkey)
{
    HI_S32 s32Ret;
    HI_U32 u32CurReadFlag;
    S_ISP_FE_STT_REGS_TYPE *pstFeSttReg = HI_NULL;

    ISP_CHECK_PIPE(ViPipeBind);
    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(pstFeReg);

    s32Ret = ISP_DRV_FE_SwitchSttAddr(ViPipeBind, k, pstDrvCtx);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP_DRV_FE_SwitchSttAddr Failed !!!\n");
        return HI_FAILURE;
    }

    u32CurReadFlag = 1 - pstDrvCtx->stFeSttAttr.stFeSttBuf[k].u32CurWriteFlag;

    pstFeSttReg = (S_ISP_FE_STT_REGS_TYPE *)pstDrvCtx->stFeSttAttr.stFeSttBuf[k].astFeSttBuf[u32CurReadFlag].pVirAddr;

    if (unStatkey.bit1FEAeGloStat)
    {
        ISP_DRV_FE_AE_Global_StatisticsRead(pstStat, pstFeSttReg, k);
    }

    if (unStatkey.bit1FEAeLocStat)
    {
        ISP_DRV_FE_AE_Local_StatisticsRead(pstStat, pstFeSttReg, k);
    }

    if (unStatkey.bit1FEAfStat)
    {
        ISP_DRV_FE_AF_STT_StatisticsRead(pstStat, pstFeReg, pstFeSttReg, ViPipeBind, k);
    }

    return HI_SUCCESS;
}

HI_S32  ISP_DRV_FE_StitchStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    return HI_SUCCESS;
}

/*read BE statistics information from phy:online*/
HI_S32 ISP_DRV_BE_COMM_StatisticsRead(ISP_STAT_S *pstStat, S_ISPBE_REGS_TYPE *apstBeReg[], HI_U8 u8BlkDev)
{
    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(apstBeReg[u8BlkDev]);

    pstStat->stCommStat.au16WhiteBalanceGain[0] = apstBeReg[u8BlkDev]->ISP_WB_GAIN1.bits.isp_wb_rgain;
    pstStat->stCommStat.au16WhiteBalanceGain[1] = apstBeReg[u8BlkDev]->ISP_WB_GAIN1.bits.isp_wb_grgain;
    pstStat->stCommStat.au16WhiteBalanceGain[2] = apstBeReg[u8BlkDev]->ISP_WB_GAIN2.bits.isp_wb_gbgain;
    pstStat->stCommStat.au16WhiteBalanceGain[3] = apstBeReg[u8BlkDev]->ISP_WB_GAIN2.bits.isp_wb_bgain;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AE_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8  k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 i, j;
    HI_U32 u32AveMem;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);

        for (i = 0; i < 1024; i++)
        {
            if (k == 0)
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i]  = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_HIST[i].u32;
            }
            else
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] += apstBeOnlineStt[k + u8BlkDev]->ISP_AE_HIST[i].u32;
            }
        }

        for (i = 0; i < AE_ZONE_ROW; i++)
        {
            if (k < (AE_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_AVER_R_GR[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFFFF));

                u32AveMem = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_AVER_GB_B[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFFFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AE_STT_GlobalStatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U32 i, k;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);

        for (i = 0; i < 1024; i++)
        {
            if (k == 0)
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i]  = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_HIST[i].u32;
            }
            else
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] += apstBeOnlineStt[k + u8BlkDev]->ISP_AE_HIST[i].u32;
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AE_STT_LocalStatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U32 i, j, k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 u32AveMem;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);

        for (i = 0; i < AE_ZONE_ROW; i++)
        {
            if (k < (AE_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_AVER_R_GR[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFFFF));

                u32AveMem = apstBeOnlineStt[k + u8BlkDev]->ISP_AE_AVER_GB_B[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFFFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_MG_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8  k;
    HI_U8  u8BlockOffset     = 0;
    HI_U8  u8BlockZoneWidth  = 0;
    HI_U32 i, j;
    HI_U32 u32AveMem;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);

        for (i = 0; i < MG_ZONE_ROW; i++)
        {
            if (k < (MG_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (MG_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = MG_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeOnlineStt[k + u8BlkDev]->ISP_LA_AVER[i * u8BlockZoneWidth + j].u32;
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFF000000) >> 24);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFF0000) >> 16);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFF00) >> 8);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AWB_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISPBE_REGS_TYPE *apstBeReg[], S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_S32 i, j, k;
    HI_U8  u8BlockOffset = 0;
    //HI_U8  u8BlockZoneWidth = 0;
    HI_U32 u32Value;
    HI_U32 u32Zone;
    HI_U32 u32Col, u32Row;
    HI_U32 u32ZoneBin = 1;
    HI_U32 u32WholeCol = 0;

    ISP_CHECK_POINTER(pstStat);

    u8BlockOffset = 0;

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
        u32Zone = apstBeReg[k + u8BlkDev]->ISP_AWB_ZONE.u32;
        u32WholeCol  += MIN2((u32Zone & 0x3F), AWB_ZONE_ORIG_COLUMN);
    }

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);
        ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
        u32Zone = apstBeReg[k + u8BlkDev]->ISP_AWB_ZONE.u32;
        u32Col  = MIN2((u32Zone & 0x3F), AWB_ZONE_ORIG_COLUMN);
        u32Row  = MIN2(((u32Zone & 0x3F00) >> 8), AWB_ZONE_ORIG_ROW);

        //u32ZoneBin = apstBeReg[k + u8BlkDev]->ISP_AWB_ZONE_BIN.u32;

        for (i = 0; i < u32Row; i++)
        {
            for (j = 0; j < u32Col; j++)
            {
                HI_U16 u16WStartAddr = (i * u32WholeCol + j + u8BlockOffset) * u32ZoneBin;
                HI_U16 u16RStartAddr = (i * u32WholeCol + j) * u32ZoneBin * 2;
                HI_U16 m;

                for (m = 0; m < u32ZoneBin; m++)
                {
                    u32Value = apstBeOnlineStt[k + u8BlkDev]->ISP_AWB_STAT[u16RStartAddr + 2 * m + 0].u32;
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgR[u16WStartAddr + m] = (u32Value & 0xFFFF);
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgG[u16WStartAddr + m] = ((u32Value >> 16) & 0xFFFF);
                    u32Value = apstBeOnlineStt[k + u8BlkDev]->ISP_AWB_STAT[u16RStartAddr + 2 * m + 1].u32;
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgB[u16WStartAddr + m] = (u32Value & 0xFFFF);
                    pstStat->stAwbStat2.au16MeteringMemArrayCountAll[u16WStartAddr + m] = ((u32Value >> 16) & 0xFFFF);
                }
            }
        }

        u8BlockOffset += u32Col;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AF_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkDev, HI_U32 u32Zone)
{
#ifdef CONFIG_HI_ISP_AF_SUPPORT
    HI_U32 i, j;
    HI_U32 u32BEAfStatData = 0;
    HI_U32 u32Col = (u32Zone & 0x1F);
    HI_U32 u32Row = ((u32Zone & 0x1F00) >> 8);

    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(apstBeOnlineStt[u8BlkDev]);

    for (i = 0; i < u32Row; i++)
    {
        for (j = 0; j < u32Col; j++)
        {
            u32BEAfStatData = apstBeOnlineStt[u8BlkDev]->ISP_AF_STAT[(i * u32Col + j) * 4].u32;
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16v1    = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16h1    = (HI_U16)(0xFFFF & u32BEAfStatData);
            u32BEAfStatData = apstBeOnlineStt[u8BlkDev]->ISP_AF_STAT[(i * u32Col + j) * 4 + 1].u32;
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16v2    = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16h2    = (HI_U16)(0xFFFF & u32BEAfStatData);
            u32BEAfStatData = apstBeOnlineStt[u8BlkDev]->ISP_AF_STAT[(i * u32Col + j) * 4 + 2].u32;
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16HlCnt = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
            pstStat->stBEAfStat.stZoneMetrics[i][j].u16y     = (HI_U16)(0xFFFF & u32BEAfStatData);
        }
    }
#endif
    return HI_SUCCESS;
}
HI_S32 ISP_DRV_BE_DEHAZE_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{

    HI_U32 i, j, m;

    ISP_CHECK_POINTER(pstStat);

    j = DEFOG_ZONE_NUM / 2;

    for (i = 0; i < u8BlkNum; i++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[i + u8BlkDev]);

        for (m = 0; m < j; m++)
        {
            pstStat->stDehazeStat.au32DehazeMinDout[i][m] = apstBeOnlineStt[i + u8BlkDev]->ISP_DEHAZE_MINSTAT[m].u32;
        }


        for (m = 0; m < DEFOG_ZONE_NUM; m++)
        {
            pstStat->stDehazeStat.au32DehazeMaxStatDout[i][m] = apstBeOnlineStt[i + u8BlkDev]->ISP_DEHAZE_MAXSTAT[m].u32;
        }
    }

    return HI_SUCCESS;

}

HI_S32 ISP_DRV_BE_FLICKER_STT_StatisticsRead(ISP_STAT_S *pstStat, S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev)
{
    HI_U8  k;
    HI_U16 j;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeOnlineStt[k + u8BlkDev]);

        for (j = 0; j < ISP_FLICKER_G_MEAN_LENGTH; j++)
        {
            pstStat->stFlickerStat.au32GMean[k][j] = apstBeOnlineStt[k + u8BlkDev]->ISP_FLICK_GMEAN[j].u32;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_APB_StatisticsRead(ISP_STAT_S *pstStat, S_ISPBE_REGS_TYPE *apstBeReg[], HI_U8 u8BlkNum, HI_U8 u8BlkDev, ISP_STAT_KEY_U unStatkey)
{
    HI_U8 k = 0;
    /*AE*/
    HI_U32 u32PixelWeight    = 0;
    HI_U32 u32PixelWeightTmp = 0;
    HI_U64 u64GlobalAvgSumR  = 0;
    HI_U64 u64GlobalAvgSumGr = 0;
    HI_U64 u64GlobalAvgSumGb = 0;
    HI_U64 u64GlobalAvgSumB  = 0;
    /*AWB*/
    HI_U64 u64MeteringAwbAvgR = 0;
    HI_U64 u64MeteringAwbAvgG = 0;
    HI_U64 u64MeteringAwbAvgB = 0;
    HI_U32 u32MeteringAwbCountAll = 0;

    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(apstBeReg[u8BlkDev]);

    if (unStatkey.bit1CommStat)
    {
        pstStat->stCommStat.au16WhiteBalanceGain[0] = apstBeReg[u8BlkDev]->ISP_WB_GAIN1.bits.isp_wb_rgain;
        pstStat->stCommStat.au16WhiteBalanceGain[1] = apstBeReg[u8BlkDev]->ISP_WB_GAIN1.bits.isp_wb_grgain;
        pstStat->stCommStat.au16WhiteBalanceGain[2] = apstBeReg[u8BlkDev]->ISP_WB_GAIN2.bits.isp_wb_gbgain;
        pstStat->stCommStat.au16WhiteBalanceGain[3] = apstBeReg[u8BlkDev]->ISP_WB_GAIN2.bits.isp_wb_bgain;
    }

    /*Flicker*/
    if (unStatkey.bit1Flicker)
    {
        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
            pstStat->stFlickerStat.au32CurDiffGr[k] = apstBeReg[k + u8BlkDev]->ISP_FLICK_GR_DIFF_CUR.bits.isp_flick_gr_diff_cur;
            pstStat->stFlickerStat.au32CurDiffGb[k] = apstBeReg[k + u8BlkDev]->ISP_FLICK_GB_DIFF_CUR.bits.isp_flick_gb_diff_cur;
            pstStat->stFlickerStat.au32CurAbsGr[k]  = apstBeReg[k + u8BlkDev]->ISP_FLICK_GR_ABS_CUR.bits.isp_flick_gr_abs_cur;
            pstStat->stFlickerStat.au32CurAbsGb[k]  = apstBeReg[k + u8BlkDev]->ISP_FLICK_GB_ABS_CUR.bits.isp_flick_gb_abs_cur;
        }
    }

    /*AE*/
    if (unStatkey.bit1BEAeGloStat)
    {
        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
            if (k == 0)
            {
                u32PixelWeightTmp = apstBeReg[k + u8BlkDev]->ISP_AE_COUNT_STAT.bits.isp_ae_count_pixels;
                pstStat->stBEAeStat1.u32PixelWeight = u32PixelWeightTmp;
                pstStat->stBEAeStat1.u32PixelCount  = apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_STAT.bits.isp_ae_total_pixels;
            }
            else
            {
                u32PixelWeightTmp = apstBeReg[k + u8BlkDev]->ISP_AE_COUNT_STAT.bits.isp_ae_count_pixels;
                pstStat->stBEAeStat1.u32PixelWeight += u32PixelWeightTmp;
                pstStat->stBEAeStat1.u32PixelCount  += apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_STAT.bits.isp_ae_total_pixels;
            }

            u64GlobalAvgSumR  += ((HI_U64)apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_R_AVER.u32)  * ((HI_U64)u32PixelWeightTmp);
            u64GlobalAvgSumGr += ((HI_U64)apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_GR_AVER.u32) * ((HI_U64)u32PixelWeightTmp);
            u64GlobalAvgSumGb += ((HI_U64)apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_GB_AVER.u32) * ((HI_U64)u32PixelWeightTmp);
            u64GlobalAvgSumB  += ((HI_U64)apstBeReg[k + u8BlkDev]->ISP_AE_TOTAL_B_AVER.u32)  * ((HI_U64)u32PixelWeightTmp);
        }

        u32PixelWeight = DIV_0_TO_1(pstStat->stBEAeStat1.u32PixelWeight);

        pstStat->stBEAeStat2.u16GlobalAvgR  = osal_div_u64(u64GlobalAvgSumR,  u32PixelWeight);
        pstStat->stBEAeStat2.u16GlobalAvgGr = osal_div_u64(u64GlobalAvgSumGr, u32PixelWeight);
        pstStat->stBEAeStat2.u16GlobalAvgGb = osal_div_u64(u64GlobalAvgSumGb, u32PixelWeight);
        pstStat->stBEAeStat2.u16GlobalAvgB  = osal_div_u64(u64GlobalAvgSumB,  u32PixelWeight);
    }


    if (unStatkey.bit1AwbStat1)
    {
        for (k = 0; k < u8BlkNum; k++)
        {
            ISP_CHECK_POINTER(apstBeReg[k + u8BlkDev]);
            u64MeteringAwbAvgR += (HI_U64)apstBeReg[k + u8BlkDev]->ISP_AWB_AVG_R.bits.isp_awb_avg_r * apstBeReg[k + u8BlkDev]->ISP_AWB_CNT_ALL.bits.isp_awb_count_all;
            u64MeteringAwbAvgG += (HI_U64)apstBeReg[k + u8BlkDev]->ISP_AWB_AVG_G.bits.isp_awb_avg_g * apstBeReg[k + u8BlkDev]->ISP_AWB_CNT_ALL.bits.isp_awb_count_all;
            u64MeteringAwbAvgB += (HI_U64)apstBeReg[k + u8BlkDev]->ISP_AWB_AVG_B.bits.isp_awb_avg_b * apstBeReg[k + u8BlkDev]->ISP_AWB_CNT_ALL.bits.isp_awb_count_all;
            u32MeteringAwbCountAll += apstBeReg[k + u8BlkDev]->ISP_AWB_CNT_ALL.bits.isp_awb_count_all;
        }

        pstStat->stAwbStat1.u16MeteringAwbAvgR = (HI_U16)(osal_div_u64(u64MeteringAwbAvgR, DIV_0_TO_1(u32MeteringAwbCountAll)));
        pstStat->stAwbStat1.u16MeteringAwbAvgG = (HI_U16)(osal_div_u64(u64MeteringAwbAvgG, DIV_0_TO_1(u32MeteringAwbCountAll)));
        pstStat->stAwbStat1.u16MeteringAwbAvgB = (HI_U16)(osal_div_u64(u64MeteringAwbAvgB, DIV_0_TO_1(u32MeteringAwbCountAll)));
        pstStat->stAwbStat1.u16MeteringAwbCountAll = (HI_U16)(u32MeteringAwbCountAll / DIV_0_TO_1(u8BlkNum));
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_STT_StatisticsRead(VI_PIPE ViPipe, ISP_STAT_S *pstStat, S_ISPBE_REGS_TYPE *apstBeReg[], HI_U8 u8BlkNum, HI_U8 u8BlkDev, ISP_STAT_KEY_U unStatkey)
{
    HI_U8  k;
    HI_U32 u32CurReadFlag;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_BE_RW_ONLINE_STT_REG_S *pstBeOnlineSttReg = HI_NULL;
    S_ISP_ONLINE_STT_REGS_TYPE *apstBeOnlineStt[ISP_MAX_BE_NUM] = {HI_NULL};
    static HI_U8 au8Cnt[ISP_MAX_PIPE_NUM] = {0};

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStat);
    ISP_CHECK_POINTER(apstBeReg[u8BlkDev]);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if ((0 != pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag) &&
        (1 != pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag))
    {
        ISP_TRACE(HI_DBG_ERR, "Err u32CurWriteFlag != 0/1 !!!\n");
    }

    /*switch ping pong buffer*/
    u32CurReadFlag = pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag;
    if (0 == au8Cnt[ViPipe])
    {
        osal_memcpy(pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[1].pVirAddr, pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[0].pVirAddr, sizeof(ISP_BE_RW_ONLINE_STT_REG_S));
        u32CurReadFlag = 1;
        au8Cnt[ViPipe]++;
    }

    pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag = 1 - u32CurReadFlag;

    ISP_DRV_SetOnlineSttAddr(ViPipe, pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[u32CurReadFlag].u64PhyAddr);

    pstBeOnlineSttReg = (ISP_BE_RW_ONLINE_STT_REG_S *)pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[u32CurReadFlag].pVirAddr;

    for (k = 0; k < u8BlkNum; k++)
    {
        apstBeOnlineStt[k + u8BlkDev] = &pstBeOnlineSttReg->stBeOnlineSttReg[k + u8BlkDev];
    }

    /* BE AE statistics */
    if (unStatkey.bit1BEAeGloStat)
    {
        ISP_DRV_BE_AE_STT_GlobalStatisticsRead(pstStat, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    if (unStatkey.bit1BEAeLocStat)
    {
        ISP_DRV_BE_AE_STT_LocalStatisticsRead(pstStat, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    if (unStatkey.bit1MgStat)
    {
        ISP_DRV_BE_MG_STT_StatisticsRead(pstStat, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    /* BE AWB statistics */
    if (unStatkey.bit1AwbStat2)
    {
        ISP_DRV_BE_AWB_STT_StatisticsRead(pstStat, apstBeReg, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    /* BE AF statistics */
    if (unStatkey.bit1BEAfStat)
    {
        ISP_DRV_BE_AF_STT_StatisticsRead(pstStat, apstBeOnlineStt, u8BlkDev, apstBeReg[u8BlkDev]->ISP_AF_ZONE.u32);
    }

    /* BE dehaze statistics */
    if (unStatkey.bit1Dehaze)
    {
        ISP_DRV_BE_DEHAZE_STT_StatisticsRead(pstStat, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    if (unStatkey.bit1Flicker)
    {
        ISP_DRV_BE_FLICKER_STT_StatisticsRead(pstStat, apstBeOnlineStt, u8BlkNum, u8BlkDev);
    }

    return HI_SUCCESS;
}

/*read BE statistics information:offline*/

HI_S32 ISP_DRV_BE_AE_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U8  k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 i, j;
    HI_U32 u32AveMem;
    HI_U64 u64GlobalAvgSumR  = 0;
    HI_U64 u64GlobalAvgSumGr = 0;
    HI_U64 u64GlobalAvgSumGb = 0;
    HI_U64 u64GlobalAvgSumB  = 0;
    HI_U32 u32PixelWeightTmp = 0;
    HI_U32 u32PixelWeight    = 0;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        for (i = 0; i < 1024; i++)
        {
            if (k == 0)
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] = apstBeStt[k]->ISP_AE_HIST[i].u32;
            }
            else
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] += apstBeStt[k]->ISP_AE_HIST[i].u32;
            }
        }

        if (k == 0)
        {
            u32PixelWeightTmp = apstBeStt[k]->ISP_AE_COUNT_STAT_RSTT.bits.isp_ae_count_pixels_stt;
            pstStat->stBEAeStat1.u32PixelWeight = u32PixelWeightTmp;
            pstStat->stBEAeStat1.u32PixelCount  = apstBeStt[k]->ISP_AE_TOTAL_STAT_RSTT.bits.isp_ae_total_pixels_stt;
        }
        else
        {
            u32PixelWeightTmp = apstBeStt[k]->ISP_AE_COUNT_STAT_RSTT.bits.isp_ae_count_pixels_stt;
            pstStat->stBEAeStat1.u32PixelWeight += u32PixelWeightTmp;
            pstStat->stBEAeStat1.u32PixelCount  += apstBeStt[k]->ISP_AE_TOTAL_STAT_RSTT.bits.isp_ae_total_pixels_stt;
        }

        u64GlobalAvgSumR  += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_R_AVER_RSTT.u32)  * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumGr += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_GR_AVER_RSTT.u32) * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumGb += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_GB_AVER_RSTT.u32) * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumB  += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_B_AVER_RSTT.u32)  * ((HI_U64)u32PixelWeightTmp);

        for (i = 0; i < AE_ZONE_ROW; i++)
        {
            if (k < (AE_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeStt[k]->ISP_AE_AVER_R_GR[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFFFF));

                u32AveMem = apstBeStt[k]->ISP_AE_AVER_GB_B[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFFFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    u32PixelWeight = DIV_0_TO_1(pstStat->stBEAeStat1.u32PixelWeight);

    pstStat->stBEAeStat2.u16GlobalAvgR  = osal_div_u64(u64GlobalAvgSumR,  u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgGr = osal_div_u64(u64GlobalAvgSumGr, u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgGb = osal_div_u64(u64GlobalAvgSumGb, u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgB  = osal_div_u64(u64GlobalAvgSumB,  u32PixelWeight);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AE_Global_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U32 i, k;
    HI_U64 u64GlobalAvgSumR  = 0;
    HI_U64 u64GlobalAvgSumGr = 0;
    HI_U64 u64GlobalAvgSumGb = 0;
    HI_U64 u64GlobalAvgSumB  = 0;
    HI_U32 u32PixelWeightTmp = 0;
    HI_U32 u32PixelWeight    = 0;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        for (i = 0; i < 1024; i++)
        {
            if (k == 0)
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] = apstBeStt[k]->ISP_AE_HIST[i].u32;
            }
            else
            {
                pstStat->stBEAeStat1.au32HistogramMemArray[i] += apstBeStt[k]->ISP_AE_HIST[i].u32;
            }
        }

        if (k == 0)
        {
            u32PixelWeightTmp = apstBeStt[k]->ISP_AE_COUNT_STAT_RSTT.bits.isp_ae_count_pixels_stt;
            pstStat->stBEAeStat1.u32PixelWeight = u32PixelWeightTmp;
            pstStat->stBEAeStat1.u32PixelCount  = apstBeStt[k]->ISP_AE_TOTAL_STAT_RSTT.bits.isp_ae_total_pixels_stt;
        }
        else
        {
            u32PixelWeightTmp = apstBeStt[k]->ISP_AE_COUNT_STAT_RSTT.bits.isp_ae_count_pixels_stt;
            pstStat->stBEAeStat1.u32PixelWeight += u32PixelWeightTmp;
            pstStat->stBEAeStat1.u32PixelCount  += apstBeStt[k]->ISP_AE_TOTAL_STAT_RSTT.bits.isp_ae_total_pixels_stt;
        }

        u64GlobalAvgSumR  += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_R_AVER_RSTT.u32)  * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumGr += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_GR_AVER_RSTT.u32) * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumGb += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_GB_AVER_RSTT.u32) * ((HI_U64)u32PixelWeightTmp);
        u64GlobalAvgSumB  += ((HI_U64)apstBeStt[k]->ISP_AE_TOTAL_B_AVER_RSTT.u32)  * ((HI_U64)u32PixelWeightTmp);
    }

    u32PixelWeight = DIV_0_TO_1(pstStat->stBEAeStat1.u32PixelWeight);

    pstStat->stBEAeStat2.u16GlobalAvgR  = osal_div_u64(u64GlobalAvgSumR,  u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgGr = osal_div_u64(u64GlobalAvgSumGr, u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgGb = osal_div_u64(u64GlobalAvgSumGb, u32PixelWeight);
    pstStat->stBEAeStat2.u16GlobalAvgB  = osal_div_u64(u64GlobalAvgSumB,  u32PixelWeight);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AE_Local_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U32 i, j, k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 u32AveMem;;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        for (i = 0; i < AE_ZONE_ROW; i++)
        {
            if (k < (AE_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = AE_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeStt[k]->ISP_AE_AVER_R_GR[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFFFF));

                u32AveMem = apstBeStt[k]->ISP_AE_AVER_GB_B[i * u8BlockZoneWidth + j].u32;
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFFFF0000) >> 16);
                pstStat->stBEAeStat3.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFFFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    return HI_SUCCESS;
}


HI_S32 ISP_DRV_BE_MG_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U8  k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 i, j;
    HI_U32 u32AveMem;

    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        for (i = 0; i < MG_ZONE_ROW; i++)
        {
            if (k < (MG_ZONE_COLUMN % DIV_0_TO_1(u8BlkNum)))
            {
                u8BlockZoneWidth = (MG_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum)) + 1;
            }
            else
            {
                u8BlockZoneWidth = MG_ZONE_COLUMN / DIV_0_TO_1(u8BlkNum);
            }

            for (j = 0; j < u8BlockZoneWidth; j++)
            {
                u32AveMem = apstBeStt[k]->ISP_LA_AVER[i * u8BlockZoneWidth + j].u32;
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][0] = (HI_U16)((u32AveMem & 0xFF000000) >> 24);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][1] = (HI_U16)((u32AveMem & 0xFF0000) >> 16);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][2] = (HI_U16)((u32AveMem & 0xFF00) >> 8);
                pstStat->stMgStat.au16ZoneAvg[i][j + u8BlockOffset][3] = (HI_U16)((u32AveMem & 0xFF));
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AWB_OfflineStatisticsRead(VI_PIPE ViPipe, ISP_STAT_S *pstStat,  S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum, HI_U8 u8BlkDev, ISP_STAT_KEY_U unStatkey)
{
    HI_S32 i, j, k;
    HI_U8  u8BlockOffset    = 0;
    HI_U8  u8BlockZoneWidth = 0;
    HI_U32 u32Value;
    HI_U64 u64MeteringAwbAvgR = 0;
    HI_U64 u64MeteringAwbAvgG = 0;
    HI_U64 u64MeteringAwbAvgB = 0;
    HI_U32 u32MeteringAwbCountAll = 0;
    HI_U32 u32Col, u32Row;
    //HI_U32 u32ZoneBin;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstStat);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        u32Col  = pstDrvCtx->astKernelCfg[pstDrvCtx->u32RegCfgInfoFlag].astAlgKernelCfg[k].stAWBZoneCfg.u32Colum;
        u32Row  = pstDrvCtx->astKernelCfg[pstDrvCtx->u32RegCfgInfoFlag].astAlgKernelCfg[k].stAWBZoneCfg.u32Row;
        u32Col  = MIN2(u32Col, AWB_ZONE_ORIG_COLUMN);
        u32Row  = MIN2(u32Row, AWB_ZONE_ORIG_ROW);

        //u32ZoneBin = pstDrvCtx->astKernelCfg[pstDrvCtx->u32RegCfgInfoFlag].astAlgKernelCfg[k].stAWBZoneCfg.u32ZoneBin;

        if (unStatkey.bit1AwbStat1)
        {
            u64MeteringAwbAvgR += (HI_U64)apstBeStt[k]->ISP_AWB_AVG_R_RSTT.bits.isp_awb_avg_r_stt \
                                  * apstBeStt[k]->ISP_AWB_CNT_ALL_RSTT.bits.isp_awb_count_all_stt;
            u64MeteringAwbAvgG += (HI_U64)apstBeStt[k]->ISP_AWB_AVG_G_RSTT.bits.isp_awb_avg_g_stt \
                                  * apstBeStt[k]->ISP_AWB_CNT_ALL_RSTT.bits.isp_awb_count_all_stt;
            u64MeteringAwbAvgB += (HI_U64)apstBeStt[k]->ISP_AWB_AVG_B_RSTT.bits.isp_awb_avg_b_stt \
                                  * apstBeStt[k]->ISP_AWB_CNT_ALL_RSTT.bits.isp_awb_count_all_stt;
            u32MeteringAwbCountAll += apstBeStt[k]->ISP_AWB_CNT_ALL_RSTT.bits.isp_awb_count_all_stt;
        }

        if (unStatkey.bit1AwbStat2)
        {
            for (i = 0; i < u32Row; i++)
            {
                if (k < (u32Col % DIV_0_TO_1(u8BlkNum)))
                {
                    u8BlockZoneWidth = (u32Col / DIV_0_TO_1(u8BlkNum)) + 1;
                }
                else
                {
                    u8BlockZoneWidth = u32Col / DIV_0_TO_1(u8BlkNum);
                }

                for (j = 0; j < u8BlockZoneWidth; j++)
                {
                    u32Value = apstBeStt[k]->ISP_AWB_STAT[(i * u8BlockZoneWidth + j) * 2].u32;
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgR[i * u32Col + j + u8BlockOffset] = (u32Value & 0xFFFF);
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgG[i * u32Col + j + u8BlockOffset] = ((u32Value >> 16) & 0xFFFF);
                    u32Value = apstBeStt[k]->ISP_AWB_STAT[(i * u8BlockZoneWidth + j) * 2 + 1].u32;
                    pstStat->stAwbStat2.au16MeteringMemArrayAvgB[i * u32Col + j + u8BlockOffset] = (u32Value & 0xFFFF);
                    pstStat->stAwbStat2.au16MeteringMemArrayCountAll[i * u32Col + j + u8BlockOffset] = ((u32Value >> 16) & 0xFFFF);
                }
            }
        }

        u8BlockOffset += u8BlockZoneWidth;
    }

    pstStat->stAwbStat1.u16MeteringAwbAvgR = (HI_U16)(osal_div_u64(u64MeteringAwbAvgR, DIV_0_TO_1(u32MeteringAwbCountAll)));
    pstStat->stAwbStat1.u16MeteringAwbAvgG = (HI_U16)(osal_div_u64(u64MeteringAwbAvgG, DIV_0_TO_1(u32MeteringAwbCountAll)));
    pstStat->stAwbStat1.u16MeteringAwbAvgB = (HI_U16)(osal_div_u64(u64MeteringAwbAvgB, DIV_0_TO_1(u32MeteringAwbCountAll)));
    pstStat->stAwbStat1.u16MeteringAwbCountAll = (HI_U16)(u32MeteringAwbCountAll / DIV_0_TO_1(u8BlkNum));

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_AF_OfflineStatisticsRead(VI_PIPE ViPipe, ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
#ifdef CONFIG_HI_ISP_AF_SUPPORT
    HI_U8  k;
    HI_U8  u8ColIndex    = 0;
    HI_U8  u8BlockOffset = 0;
    HI_U32 i, j;
    HI_U32 u32Col;
    HI_U32 u32Row;
    HI_U32 u32BEAfStatData   = 0;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_POINTER(pstStat);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        u32Col     = pstDrvCtx->astKernelCfg[pstDrvCtx->u32RegCfgInfoFlag].astAlgKernelCfg[k].stBeAFZoneCfg.u32Colum;
        u32Row     = pstDrvCtx->astKernelCfg[pstDrvCtx->u32RegCfgInfoFlag].astAlgKernelCfg[k].stBeAFZoneCfg.u32Row;
        u32Col     = MIN2(u32Col, AF_ZONE_COLUMN);
        u32Row     = MIN2(u32Row, AF_ZONE_ROW);

        for (i = 0; i < u32Row; i++)
        {
            for (j = 0; j < u32Col; j++)
            {
                u8ColIndex = MIN2(j + u8BlockOffset, AF_ZONE_COLUMN);

                u32BEAfStatData = apstBeStt[k]->ISP_AF_STAT[(i * u32Col + j) * 4].u32;
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16v1 = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16h1 = (HI_U16)(0xFFFF & u32BEAfStatData);
                u32BEAfStatData = apstBeStt[k]->ISP_AF_STAT[(i * u32Col + j) * 4 + 1].u32;
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16v2 = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16h2 = (HI_U16)(0xFFFF & u32BEAfStatData);
                u32BEAfStatData = apstBeStt[k]->ISP_AF_STAT[(i * u32Col + j) * 4 + 2].u32;
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16HlCnt = (HI_U16)((0xFFFF0000 & u32BEAfStatData) >> 16);
                pstStat->stBEAfStat.stZoneMetrics[i][u8ColIndex].u16y     = (HI_U16)(0xFFFF & u32BEAfStatData);
            }
        }

        u8BlockOffset += u32Col;
    }
#endif
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_Dehaze_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U32 i, j, m;

    j = DEFOG_ZONE_NUM / 2;

    ISP_CHECK_POINTER(pstStat);

    for (i = 0; i < u8BlkNum; i++)
    {
        ISP_CHECK_POINTER(apstBeStt[i]);

        for (m = 0; m < j; m++)
        {
            pstStat->stDehazeStat.au32DehazeMinDout[i][m] = apstBeStt[i]->ISP_DEHAZE_MINSTAT[m].u32;
        }

        for (m = 0; m < DEFOG_ZONE_NUM; m++)
        {
            pstStat->stDehazeStat.au32DehazeMaxStatDout[i][m] = apstBeStt[i]->ISP_DEHAZE_MAXSTAT[m].u32;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_DPC_OfflineCalibInfoRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U8  k;
    HI_U16 j;
    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        pstStat->stDpStat.au16DefectPixelCount[k] = apstBeStt[k]->ISP_DPC_BPT_CALIB_NUMBER_RSTT.bits.isp_dpc_bpt_calib_number_rstt;

        for (j = 0; j < pstStat->stDpStat.au16DefectPixelCount[k]; j++)
        {
            pstStat->stDpStat.au32DefectPixelLut[k][j] = apstBeStt[k]->ISP_DPC_BPT_RSTT[j].u32;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_Fliker_OfflineStatisticsRead(ISP_STAT_S *pstStat, S_ISP_STT_REGS_TYPE *apstBeStt[], HI_U8 u8BlkNum)
{
    HI_U8  k;
    HI_U16 j;
    ISP_CHECK_POINTER(pstStat);

    for (k = 0; k < u8BlkNum; k++)
    {
        ISP_CHECK_POINTER(apstBeStt[k]);

        pstStat->stFlickerStat.au32CurDiffGr[k] = apstBeStt[k]->ISP_FLICK_GR_DIFF_CUR_RSTT.bits.isp_flick_gr_diff_cur_rstt;
        pstStat->stFlickerStat.au32CurDiffGb[k] = apstBeStt[k]->ISP_FLICK_GB_DIFF_CUR_RSTT.bits.isp_flick_gb_diff_cur_rstt;
        pstStat->stFlickerStat.au32CurAbsGr[k]  = apstBeStt[k]->ISP_FLICK_GR_ABS_CUR_RSTT.bits.isp_flick_gr_abs_cur_rstt;
        pstStat->stFlickerStat.au32CurAbsGb[k]  = apstBeStt[k]->ISP_FLICK_GB_ABS_CUR_RSTT.bits.isp_flick_gb_abs_cur_rstt;

        for (j = 0; j < ISP_FLICKER_G_MEAN_LENGTH; j++)
        {
            pstStat->stFlickerStat.au32GMean[k][j] = apstBeStt[k]->ISP_FLICK_GMEAN[j].u32;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineAEStitchGlobalStatisticsRead(ISP_STAT_S *pstStat, ISP_DRV_CTX_S *pstDrvCtx)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineAEStitchLocalStatisticsRead(ISP_STAT_S *pstStat, ISP_DRV_CTX_S *pstDrvCtx)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineAWBStitchStatisticsRead(ISP_STAT_S *pstStat, S_ISPBE_REGS_TYPE *apstBeReg[], ISP_DRV_CTX_S *pstDrvCtx, HI_U8 u8BlkDev, ISP_STAT_KEY_U unStatkey)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BE_StitchStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FE_StitchNonStatisticsRead(VI_PIPE ViPipe, ISP_STAT_INFO_S *pstStatInfo)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeLutSttBufInit(VI_PIPE ViPipe)
{
    HI_U32 i;
    HI_S32 s32Ret;
    HI_U8  *pu8VirAddr;
    HI_U64 u64PhyAddr, u64Size;
    unsigned long u32Flags;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    ISP_DRV_CTX_S *pstDrvCtx        = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stBeLut2SttAttr.bInit);

    osal_snprintf(acName, sizeof(acName), "BeLutStt[%d]", ViPipe);
    u64Size = sizeof(S_ISP_LUT_WSTT_TYPE);

    s32Ret = CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size * 2 * ISP_MAX_BE_NUM);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc ISP be Lut stt buf err\n");
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size * 2 * ISP_MAX_BE_NUM);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeLut2SttAttr.bInit = HI_TRUE;

    for (i = 0; i < ISP_MAX_BE_NUM; i++)
    {
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].u64PhyAddr = u64PhyAddr + 2 * i * u64Size;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].pVirAddr   = (HI_VOID *)(pu8VirAddr + 2 * i * u64Size);
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].u64Size    = u64Size;

        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].u64PhyAddr = u64PhyAddr + (2 * i + 1) * u64Size;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].pVirAddr   = (HI_VOID *)(pu8VirAddr + (2 * i + 1) * u64Size);
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].u64Size    = u64Size;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeLutSttBufExit(VI_PIPE ViPipe)
{
    HI_U32 i;
    HI_U64 u64PhyAddr;
    HI_VOID *pVirAddr;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stBeLut2SttAttr.bInit);

    u64PhyAddr = pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[0].astLutSttBuf[0].u64PhyAddr;
    pVirAddr   = pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[0].astLutSttBuf[0].pVirAddr;

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeLut2SttAttr.bInit = HI_FALSE;

    for (i = 0; i < ISP_MAX_BE_NUM; i++)
    {
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].u64PhyAddr = 0;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].u64Size    = 0;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[0].pVirAddr   = HI_NULL;

        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].u64PhyAddr = 0;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].u64Size    = 0;
        pstDrvCtx->stBeLut2SttAttr.astBeLutSttBuf[i].astLutSttBuf[1].pVirAddr   = HI_NULL;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOnlineSttBufInit(VI_PIPE ViPipe)
{
    HI_U32 i;
    HI_S32 s32Ret;
    HI_U8  *pu8VirAddr;
    HI_U64 u64PhyAddr, u64Size;
    unsigned long u32Flags;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    ISP_DRV_CTX_S *pstDrvCtx        = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stBeOnlineSttBuf.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].BeSttOn", ViPipe);
    u64Size = sizeof(ISP_BE_RW_ONLINE_STT_REG_S);

    s32Ret = CMPI_MmzMallocCached(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size * 2);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc ISP be online stt buf err\n");
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size * 2);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeOnlineSttBuf.bInit = HI_TRUE;

    for (i = 0; i < 2; i++)
    {
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64PhyAddr = u64PhyAddr + i * u64Size;
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].pVirAddr   = (HI_VOID *)(pu8VirAddr + i * u64Size);
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64Size    = u64Size;
    }

    pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag = 0;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    s32Ret = ISP_DRV_SetOnlineSttAddr(ViPipe, pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[0].u64PhyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Set ISP online stt addr Err!\n", ViPipe);
        //return HI_FAILURE;
        goto fail0;
    }

    return HI_SUCCESS;

fail0:
    pstDrvCtx->stBeOnlineSttBuf.bInit = HI_FALSE;

    for (i = 0; i < 2; i++)
    {
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64PhyAddr = 0;
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].pVirAddr   = HI_NULL;
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64Size    = 0;
    }

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pu8VirAddr, u64PhyAddr, u64Size * 2);
        CMPI_MmzFree(u64PhyAddr, (HI_VOID *)pu8VirAddr);
    }

    return HI_FAILURE;
}

HI_S32 ISP_DRV_BeOnlineSttBufExit(VI_PIPE ViPipe)
{
    HI_U32 i;
    HI_U64 u64PhyAddr;//, u64Size;
    HI_VOID *pVirAddr;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stBeOnlineSttBuf.bInit);

    u64PhyAddr = pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[0].u64PhyAddr;
    pVirAddr   = pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[0].pVirAddr;
    //u64Size    = pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[0].u64Size;

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeOnlineSttBuf.bInit = HI_FALSE;

    for (i = 0; i < 2; i++)
    {
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64PhyAddr = 0;
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].u64Size    = 0;
        pstDrvCtx->stBeOnlineSttBuf.astBeSttBuf[i].pVirAddr   = HI_NULL;
    }

    pstDrvCtx->stBeOnlineSttBuf.u32CurWriteFlag = 0;

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pVirAddr, u64PhyAddr, u64Size * 2);
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FeSttAddrInit(VI_PIPE ViPipe)
{
    HI_U8   u8ChnNumMax, k;
    VI_PIPE ViPipeBind;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    S_ISP_VICAP_CH_REGS_TYPE *pstFeSttAddrReg = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_VIR_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stFeSttAttr.bInit);

    u8ChnNumMax = pstDrvCtx->stWDRAttr.stDevBindPipe.u32Num;

    if (u8ChnNumMax > ISP_WDR_CHN_MAX)
    {
        ISP_TRACE(HI_DBG_ERR, "unsupported %dTo1 WDR\n", u8ChnNumMax);
        return HI_FAILURE;
    }

    for (k = 0; k < u8ChnNumMax; k++)
    {
        pstDrvCtx->stFeSttAttr.stFeSttBuf[k].bFirstFrame = HI_TRUE;

        ViPipeBind = pstDrvCtx->stWDRAttr.stDevBindPipe.PipeId[k];

        ISP_CHECK_PIPE(ViPipeBind);

        ISP_DRV_SetFeSttAddr(ViPipeBind, pstDrvCtx->stFeSttAttr.stFeSttBuf[k].astFeSttBuf[0].u64PhyAddr);

        ISP_DRV_FESTTREG_CTX(ViPipeBind, pstFeSttAddrReg);
        pstFeSttAddrReg->VI_CH_REG_NEWER.bits.ch_reg_newer = 1;
        pstFeSttAddrReg->CH_UPDATE.bits.ch_update          = 1;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FeStitchSttAddrInit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_NormalFeSttBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U32 i, j;
    HI_U8  *pu8VirAddr;
    HI_U64 u64PhyAddr, u64Size, u64SizeTemp;
    unsigned long u32Flags;
    HI_CHAR acName[MAX_MMZ_NAMELEN] = {0};
    ISP_DRV_CTX_S *pstDrvCtx        = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stFeSttAttr.bInit);

    if (HI_FALSE == pstDrvCtx->stWDRAttr.bMastPipe)
    {
        return HI_SUCCESS;
    }

    osal_snprintf(acName, sizeof(acName), "ISP[%d].FeStt", ViPipe);

    u64Size = sizeof(S_ISP_FE_STT_REGS_TYPE);

    u64SizeTemp = u64Size * 2 * ISP_WDR_CHN_MAX;
    s32Ret = CMPI_MmzMallocNocache(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64SizeTemp);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc ISP fe stt buf[%d] err\n", ViPipe);
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64SizeTemp);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stFeSttAttr.bInit = HI_TRUE;

    for (i = 0; i < ISP_WDR_CHN_MAX; i++)
    {
        for (j = 0; j < 2; j++)
        {
            pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].u64PhyAddr = u64PhyAddr + (2 * i + j) * u64Size;
            pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].pVirAddr   = (HI_VOID *)(pu8VirAddr + (2 * i + j) * u64Size);
            pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].u64Size    = u64Size;
        }

        pstDrvCtx->stFeSttAttr.stFeSttBuf[i].u32CurWriteFlag = 0;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FeSttBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_VIR_PIPE(ViPipe);

    s32Ret = ISP_DRV_NormalFeSttBufInit(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_WARN, "ISP_DRV_NormalFeSttBufInit failure!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_FeSttBufExit(VI_PIPE ViPipe)
{
    HI_U32  i, j;
    HI_U64  u64PhyAddr = 0;
    HI_VOID *pVirAddr;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_VIR_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stFeSttAttr.bInit);

    u64PhyAddr = pstDrvCtx->stFeSttAttr.stFeSttBuf[0].astFeSttBuf[0].u64PhyAddr;
    pVirAddr   = pstDrvCtx->stFeSttAttr.stFeSttBuf[0].astFeSttBuf[0].pVirAddr;

    if (HI_TRUE == pstDrvCtx->stWDRAttr.bMastPipe)
    {
        osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

        pstDrvCtx->stFeSttAttr.bInit = HI_FALSE;

        for (i = 0; i < ISP_WDR_CHN_MAX; i++)
        {
            for (j = 0; j < 2; j++)
            {
                pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].u64PhyAddr = 0;
                pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].u64Size    = 0;
                pstDrvCtx->stFeSttAttr.stFeSttBuf[i].astFeSttBuf[j].pVirAddr   = HI_NULL;
            }

            pstDrvCtx->stFeSttAttr.stFeSttBuf[i].u32CurWriteFlag = 0;
            pstDrvCtx->stFeSttAttr.stFeSttBuf[i].bFirstFrame     = HI_FALSE;
        }

        osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);
    }

    if (0 != u64PhyAddr)
    {
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineSttBufInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U32 i;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    HI_U64 u64PhyAddr, u64Size;
    HI_U8  *pu8VirAddr;
    unsigned long u32Flags;
    HI_CHAR acName[MAX_MMZ_NAMELEN]       = {0};

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_INIT(ViPipe, pstDrvCtx->stBeOffSttAttr.bInit);

    osal_snprintf(acName, sizeof(acName), "ISP[%d].BeStt", ViPipe);

    u64Size = sizeof(S_ISP_STT_REGS_TYPE);

    u64Size = (u64Size + 255) / 256 * 256; //256bytes align

    s32Ret = CMPI_MmzMallocCached(HI_NULL, acName, &u64PhyAddr, (HI_VOID **)&pu8VirAddr, u64Size * ISP_STRIPING_MAX_NUM);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "alloc ISP BeSttBuf err\n");
        return HI_ERR_ISP_NOMEM;
    }

    osal_memset(pu8VirAddr, 0, u64Size * ISP_STRIPING_MAX_NUM);

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeOffSttAttr.bInit = HI_TRUE;

    for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
    {
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].u64PhyAddr = u64PhyAddr + i * u64Size;
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].pVirAddr   = (HI_VOID *)(pu8VirAddr + i * u64Size);
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].u64Size    = u64Size;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeOfflineSttBufExit(VI_PIPE ViPipe)
{
    HI_U32 i;
    //HI_U64 u64Size;
    HI_U64 u64PhyAddr;
    HI_VOID *pVirAddr;
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    ISP_CHECK_BUF_EXIT(ViPipe, pstDrvCtx->stBeOffSttAttr.bInit);

    u64PhyAddr = pstDrvCtx->stBeOffSttAttr.astBeSttBuf[0].u64PhyAddr;
    pVirAddr   = pstDrvCtx->stBeOffSttAttr.astBeSttBuf[0].pVirAddr;
    //u64Size    = pstDrvCtx->stBeOffSttAttr.astBeSttBuf[0].u64Size * ISP_STRIPING_MAX_NUM;

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);

    pstDrvCtx->stBeOffSttAttr.bInit = HI_FALSE;

    for (i = 0; i < ISP_STRIPING_MAX_NUM; i++)
    {
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].u64PhyAddr = 0;
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].u64Size    = 0;
        pstDrvCtx->stBeOffSttAttr.astBeSttBuf[i].pVirAddr   = HI_NULL;
    }

    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    if (0 != u64PhyAddr)
    {
        //hil_mmb_invalid_cache_byaddr(pVirAddr, u64PhyAddr, u64Size);
        CMPI_MmzFree(u64PhyAddr, pVirAddr);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_BeSttBufInit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
        IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        if (HI_SUCCESS != ISP_DRV_BeOnlineSttBufInit(ViPipe))
        {
            return HI_FAILURE;
        }

        if (HI_SUCCESS != ISP_DRV_BeLutSttBufInit(ViPipe))
        {
            goto fail0;
        }
    }
    else if (IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode) || \
             IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        if (HI_SUCCESS != ISP_DRV_BeOfflineSttBufInit(ViPipe))
        {
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "enIspOnlineMode err 0x%x!\n", pstDrvCtx->stWorkMode.enIspRunningMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

fail0:
    ISP_DRV_BeOnlineSttBufExit(ViPipe);

    return HI_FAILURE;
}

HI_S32 ISP_DRV_BeSttBufExit(VI_PIPE ViPipe)
{
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);

    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (IS_ONLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)\
        || IS_SIDEBYSIDE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        if (HI_SUCCESS != ISP_DRV_BeOnlineSttBufExit(ViPipe))
        {
            ISP_TRACE(HI_DBG_ERR, "pipe[%d] ISP_DRV_BeOnlineSttBufExit failed!\n", ViPipe);
            return HI_FAILURE;
        }

        if (HI_SUCCESS != ISP_DRV_BeLutSttBufExit(ViPipe))
        {
            ISP_TRACE(HI_DBG_ERR, "pipe[%d] ISP_DRV_BeLutSttBufExit failed!\n", ViPipe);
            return HI_FAILURE;
        }
    }
    else if (IS_OFFLINE_MODE(pstDrvCtx->stWorkMode.enIspRunningMode)\
             || IS_STRIPING_MODE(pstDrvCtx->stWorkMode.enIspRunningMode))
    {
        if (HI_SUCCESS != ISP_DRV_BeOfflineSttBufExit(ViPipe))
        {
            ISP_TRACE(HI_DBG_ERR, "pipe[%d] ISP_DRV_BeOfflineSttBufExit failed!\n", ViPipe);
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "enIspOnlineMode err 0x%x!\n", pstDrvCtx->stWorkMode.enIspRunningMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SttBufInit(VI_PIPE ViPipe)
{
    ISP_CHECK_PIPE(ViPipe);

    if (HI_SUCCESS != ISP_DRV_BeSttBufInit(ViPipe))
    {
        return HI_FAILURE;
    }

    if (HI_SUCCESS != ISP_DRV_FeSttBufInit(ViPipe))
    {
        ISP_DRV_BeSttBufExit(ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_SttBufExit(VI_PIPE ViPipe)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_EXIT_STATE(ViPipe);

    if (HI_SUCCESS != ISP_DRV_BeSttBufExit(ViPipe))
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] ISP_DRV_BeSttBufExit failed!\n", ViPipe);
        return HI_FAILURE;
    }

    if (HI_SUCCESS != ISP_DRV_FeSttBufExit(ViPipe))
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] ISP_DRV_FeSttBufExit failed!\n", ViPipe);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetFusionAttr(VI_PIPE ViPipe, VI_FUSION_GROUP_S *pstFusionAttr)
{
#ifdef CONFIG_HI_VI_MONO_COLOR_FUSION
    HI_U8  i;
    HI_S32 s32Ret;
    VI_FUSION_GROUP_S stFusionAttr;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstFusionAttr);

    osal_memset(&stFusionAttr, 0 , sizeof(VI_FUSION_GROUP_S));

    if (!CKFN_VI_GetFusionGroupAttr())
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CKFN_VI_GetFusionGroupAttr is null\n", ViPipe);
        return HI_FAILURE;
    }

    s32Ret = CALL_VI_GetFusionGroupAttr(ViPipe, &stFusionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "pipe[%d] CALL_VI_GetFusionGroupAttr failed 0x%x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    ISP_CHECK_BOOL(stFusionAttr.bFusionEn);

    if (HI_TRUE == stFusionAttr.bFusionEn)
    {
        if ((stFusionAttr.u32PipeNum < 2) || (stFusionAttr.u32PipeNum > ISP_MAX_PIPE_NUM))
        {
            ISP_TRACE(HI_DBG_ERR, "Err Mono fusion u32PipeNum %d!\n", stFusionAttr.u32PipeNum);
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }

        for (i = 0; i < ISP_MAX_PIPE_NUM; i++)
        {
            ISP_CHECK_PIPE(stFusionAttr.PipeId[i]);
        }
    }

    osal_memcpy(pstFusionAttr, &stFusionAttr, sizeof(VI_FUSION_GROUP_S));
#else
    osal_memset(pstFusionAttr, 0, sizeof(VI_FUSION_GROUP_S));
#endif

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetP2EnInfo(VI_PIPE ViPipe, HI_BOOL *pbP2En)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pbP2En);

    *pbP2En = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StaKernelGet(VI_PIPE ViPipe, ISP_DRV_AF_STATISTICS_S *pstFocusStat)
{
    unsigned long u32Flags;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;
    ISP_STAT_S *pstStat = HI_NULL;
    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    if (HI_NULL == pstFocusStat)
    {
        ISP_TRACE(HI_DBG_ERR, "get statistic active buffer err, pstFocusStat is NULL!\n");
        return HI_FAILURE;
    }

    if (!pstDrvCtx->stStatisticsBuf.pstActStat)
    {
        ISP_TRACE(HI_DBG_INFO, "get statistic active buffer err, stat not ready!\n");
        return HI_FAILURE;
    }

    if (!pstDrvCtx->stStatisticsBuf.pstActStat->pVirtAddr)
    {
        ISP_TRACE(HI_DBG_ERR, "get statistic active buffer err, pVirtAddr is NULL!\n");
        return HI_FAILURE;
    }

    pstStat = (ISP_STAT_S *)pstDrvCtx->stStatisticsBuf.pstActStat->pVirtAddr;

    osal_spin_lock_irqsave(&g_stIspLock[ViPipe], &u32Flags);
    osal_memset(&(pstFocusStat->stFEAFStat), 0, sizeof(ISP_DRV_FE_FOCUS_STATISTICS_S));
    osal_memcpy(&(pstFocusStat->stBEAFStat), &pstStat->stBEAfStat, sizeof(ISP_DRV_BE_FOCUS_STATISTICS_S));
    osal_spin_unlock_irqrestore(&g_stIspLock[ViPipe], &u32Flags);

    return HI_SUCCESS;
}

HI_S32 ISP_DRV_StitchSyncCtrlInit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_GetStitchBeSyncPara(VI_PIPE ViPipe, ISP_BE_SYNC_PARA_S *pstBeSyncPara)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DRV_ResetFeCfg(VI_PIPE ViPipe)
{
#ifdef ENABLE_CLOCK
    HI_U8   k;
    HI_BOOL bReset = HI_TRUE;
    VI_PIPE ViPipeBind;
    MPP_CHN_S stMppChn;
    ISP_DRV_CTX_S *pstDrvCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    pstDrvCtx = ISP_DRV_GET_CTX(ViPipe);

    stMppChn.enModId = HI_ID_ISP;

    for (k = 0; k < pstDrvCtx->stWDRAttr.stDevBindPipe.u32Num; k++)
    {
        ViPipeBind = pstDrvCtx->stWDRAttr.stDevBindPipe.PipeId[k];

        ISP_CHECK_PIPE(ViPipeBind);
        stMppChn.s32DevId = ViPipeBind;
        CALL_SYS_DrvIoCtrl(&stMppChn, SYS_VI_ISP_CFG_RESET_SEL, &bReset);
    }
    bReset = HI_FALSE;
    for (k = 0; k < pstDrvCtx->stWDRAttr.stDevBindPipe.u32Num; k++)
    {
        ViPipeBind = pstDrvCtx->stWDRAttr.stDevBindPipe.PipeId[k];

        ISP_CHECK_PIPE(ViPipeBind);
        stMppChn.s32DevId = ViPipeBind;
        CALL_SYS_DrvIoCtrl(&stMppChn, SYS_VI_ISP_CFG_RESET_SEL, &bReset);
    }

#endif

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
