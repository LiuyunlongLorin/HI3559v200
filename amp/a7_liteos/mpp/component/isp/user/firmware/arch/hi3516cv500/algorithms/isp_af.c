/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_af.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/05
  Description   :
  History       :
  1.Date        : 2013/01/05
    Author      :
    Modification: Created file

******************************************************************************/

#include <stdio.h>
#include "isp_alg.h"
#include "hi_comm_isp.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include <math.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define  AF_MIN_WIDTH  256
#define  AF_MIN_HEIGHT 120

ISP_FOCUS_STATISTICS_CFG_S  g_stFocusCfg[ISP_MAX_PIPE_NUM] =
{
    [0 ... (ISP_MAX_PIPE_NUM - 1)] =
    {
        {1, 17, 15, 1920, 1080, 1, 0, {0, 0, 0, 1920, 1080}, 0, {0x2, 0x4, 0}, {1, 0x9bff}, 0xf0},
        {1, {1, 1, 1}, 15, {188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 220, 8, 14}, {127, 12, 2047} },
        {0, {1, 1, 0}, 2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0 }, {15, 12, 2047} },
        {{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800} },
        {{ -12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047} },
        {4, {0, 0}, {1, 1}, 0}
    }
};

HI_S32 ISP_AfExtRegsDefault(VI_PIPE ViPipe)
{
    HI_U8 u8I, u8Shift0;
    HI_S16 s16G0, s16G1, s16G2;
    ISP_AF_H_PARAM_S *pstIIR = HI_NULL;
    HI_U32 u32Pls, u32Plg;
    float f32Pl, f32Temp;
    HI_U8 u8AfEnable = 0;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    /* AF STATISTICS CONIFG START */
    //hi_ext_af_enable_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.bEnable);
    u8AfEnable = (HI_TRUE == g_stFocusCfg[ViPipe].stConfig.bEnable) ? 0x3 : 0x0;
    hi_ext_system_af_enable_write(ViPipe, u8AfEnable);
    hi_ext_af_iir0_enable0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[0]);
    hi_ext_af_iir0_enable1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[1]);
    hi_ext_af_iir0_enable2_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[2]);
    hi_ext_af_iir1_enable0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[0]);
    hi_ext_af_iir1_enable1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[1]);
    hi_ext_af_iir1_enable2_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[2]);
    hi_ext_af_peakmode_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.enPeakMode);
    hi_ext_af_squmode_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.enSquMode);
    hi_ext_af_window_hnum_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.u16Hwnd);
    hi_ext_af_window_vnum_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.u16Vwnd);
    hi_ext_af_iir_gain0_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[0]);
    hi_ext_af_iir_gain0_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[0]);
    hi_ext_af_iir_gain1_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[1]);
    hi_ext_af_iir_gain1_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[1]);
    hi_ext_af_iir_gain2_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[2]);
    hi_ext_af_iir_gain2_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[2]);
    hi_ext_af_iir_gain3_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[3]);
    hi_ext_af_iir_gain3_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[3]);
    hi_ext_af_iir_gain4_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[4]);
    hi_ext_af_iir_gain4_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[4]);
    hi_ext_af_iir_gain5_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[5]);
    hi_ext_af_iir_gain5_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[5]);
    hi_ext_af_iir_gain6_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[6]);
    hi_ext_af_iir_gain6_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[6]);
    hi_ext_af_iir0_shift_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[0]);
    hi_ext_af_iir1_shift_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[1]);
    hi_ext_af_iir2_shift_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[2]);
    hi_ext_af_iir3_shift_group0_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[3]);
    hi_ext_af_iir0_shift_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[0]);
    hi_ext_af_iir1_shift_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[1]);
    hi_ext_af_iir2_shift_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[2]);
    hi_ext_af_iir3_shift_group1_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[3]);
    hi_ext_af_fir_h_gain0_group0_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[0]);
    hi_ext_af_fir_h_gain0_group1_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[0]);
    hi_ext_af_fir_h_gain1_group0_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[1]);
    hi_ext_af_fir_h_gain1_group1_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[1]);
    hi_ext_af_fir_h_gain2_group0_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[2]);
    hi_ext_af_fir_h_gain2_group1_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[2]);
    hi_ext_af_fir_h_gain3_group0_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[3]);
    hi_ext_af_fir_h_gain3_group1_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[3]);
    hi_ext_af_fir_h_gain4_group0_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[4]);
    hi_ext_af_fir_h_gain4_group1_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[4]);

    /* ds */
    hi_ext_af_iir0_ds_enable_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.bNarrowBand);
    hi_ext_af_iir1_ds_enable_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.bNarrowBand);

    /* PLG and PLS */
    for ( u8I = 0 ; u8I < 2 ; u8I++ )
    {
        pstIIR = u8I ? &(g_stFocusCfg[ViPipe].stHParam_IIR1) : &(g_stFocusCfg[ViPipe].stHParam_IIR0);

        u8Shift0 =  (HI_U8)pstIIR->au16IIRShift[0];
        s16G0 = pstIIR->as16IIRGain[0];
        s16G1 = pstIIR->as16IIRGain[1];
        s16G2 = pstIIR->as16IIRGain[2];

        f32Pl = (512.f / DIV_0_TO_1(512 - 2 * s16G1 - s16G2) * s16G0) /  (1 << u8Shift0);
        f32Temp = f32Pl;
        f32Temp = MIN2(7 - floor(log(f32Temp) / log(2)), 7);

        u32Pls = (HI_U32)f32Temp;
        u32Plg = (HI_U32)((f32Pl * (1 << u32Pls)) + 0.5);

        if (0 == u8I)
        {
            hi_ext_af_iir_pls_group0_write(ViPipe, u32Pls);
            hi_ext_af_iir_plg_group0_write(ViPipe, u32Plg);
        }
        else
        {
            hi_ext_af_iir_pls_group1_write(ViPipe, u32Pls);
            hi_ext_af_iir_plg_group1_write(ViPipe, u32Plg);
        }
    }

    /* AF crop */
    hi_ext_af_crop_enable_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stCrop.bEnable);
    hi_ext_af_crop_pos_x_write(ViPipe, 0);
    hi_ext_af_crop_pos_y_write(ViPipe, 0);
    hi_ext_af_crop_hsize_write(ViPipe, pstIspCtx->stBlockAttr.stFrameRect.u32Width / 8 * 8);
    hi_ext_af_crop_vsize_write(ViPipe, pstIspCtx->stBlockAttr.stFrameRect.u32Height / 2 * 2);

    /* AF raw cfg */
    hi_ext_af_pos_sel_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.enStatisticsPos);

    hi_ext_af_rawmode_write(ViPipe, ~(((g_stFocusCfg[ViPipe].stConfig.enStatisticsPos) >> 0x1) & 0x1));
    hi_ext_af_gain_limit_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stRawCfg.GammaGainLimit);
    hi_ext_af_gamma_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stRawCfg.GammaValue);
    hi_ext_af_bayermode_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stRawCfg.enPattern);

    /* AF pre median filter */
    hi_ext_af_mean_enable_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stPreFltCfg.bEn);
    hi_ext_af_mean_thres_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.stPreFltCfg.u16strength);

    /* level depend gain */
    hi_ext_af_iir0_ldg_enable_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.bLdEn);
    hi_ext_af_iir_thre0_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16ThLow    );
    hi_ext_af_iir_thre0_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16ThHigh   );
    hi_ext_af_iir_slope0_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16SlpLow   );
    hi_ext_af_iir_slope0_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16SlpHigh  );
    hi_ext_af_iir_gain0_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16GainLow  );
    hi_ext_af_iir_gain0_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16GainHigh );

    hi_ext_af_iir1_ldg_enable_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.bLdEn);
    hi_ext_af_iir_thre1_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16ThLow    );
    hi_ext_af_iir_thre1_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16ThHigh   );
    hi_ext_af_iir_slope1_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16SlpLow   );
    hi_ext_af_iir_slope1_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16SlpHigh  );
    hi_ext_af_iir_gain1_low_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16GainLow  );
    hi_ext_af_iir_gain1_high_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16GainHigh );

    hi_ext_af_fir0_ldg_enable_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.bLdEn);
    hi_ext_af_fir_thre0_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16ThLow    );
    hi_ext_af_fir_thre0_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16ThHigh   );
    hi_ext_af_fir_slope0_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16SlpLow   );
    hi_ext_af_fir_slope0_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16SlpHigh  );
    hi_ext_af_fir_gain0_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16GainLow  );
    hi_ext_af_fir_gain0_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16GainHigh );

    hi_ext_af_fir1_ldg_enable_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.bLdEn);
    hi_ext_af_fir_thre1_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16ThLow    );
    hi_ext_af_fir_thre1_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16ThHigh   );
    hi_ext_af_fir_slope1_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16SlpLow   );
    hi_ext_af_fir_slope1_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16SlpHigh  );
    hi_ext_af_fir_gain1_low_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16GainLow  );
    hi_ext_af_fir_gain1_high_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16GainHigh );

    /* AF coring */
    hi_ext_af_iir_thre0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Th   );
    hi_ext_af_iir_slope0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Slp  );
    hi_ext_af_iir_peak0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Lmt  );

    hi_ext_af_iir_thre1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Th   );
    hi_ext_af_iir_slope1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Slp  );
    hi_ext_af_iir_peak1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Lmt  );

    hi_ext_af_fir_thre0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Th   );
    hi_ext_af_fir_slope0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Slp  );
    hi_ext_af_fir_peak0_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Lmt  );

    hi_ext_af_fir_thre1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Th   );
    hi_ext_af_fir_slope1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Slp  );
    hi_ext_af_fir_peak1_coring_write(ViPipe, g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Lmt  );

    /* high luma counter */
    hi_ext_af_hiligh_thre_write(ViPipe, (HI_U8)g_stFocusCfg[ViPipe].stConfig.u16HighLumaTh);

    /* AF output shift */
    hi_ext_af_acc_shift0_h_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.au16AccShiftH[0]);
    hi_ext_af_acc_shift1_h_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.au16AccShiftH[1]);
    hi_ext_af_acc_shift0_v_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.au16AccShiftV[0]);
    hi_ext_af_acc_shift1_v_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.au16AccShiftV[1]);
    hi_ext_af_acc_shift_y_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.u16AccShiftY);
    hi_ext_af_shift_count_y_write(ViPipe, g_stFocusCfg[ViPipe].stFVParam.u16HlCntShift);
    //hi_vi_af_cnt_shift_highluma_write(ViPipe, pstStatCfg->stFocusCfg.stFVParam.u16HlCntShift);

    g_stFocusCfg[ViPipe].stConfig.u16Hsize = pstIspCtx->stBlockAttr.stFrameRect.u32Width;//hi_ext_sync_total_width_read(ViPipe);
    g_stFocusCfg[ViPipe].stConfig.u16Vsize = pstIspCtx->stBlockAttr.stFrameRect.u32Height;//hi_ext_sync_total_height_read(ViPipe);

    hi_ext_af_input_hsize_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.u16Hsize);
    hi_ext_af_input_vsize_write(ViPipe, g_stFocusCfg[ViPipe].stConfig.u16Vsize);

    hi_ext_af_set_flag_write(ViPipe, HI_EXT_AF_SET_FLAG_DISABLE);

    return HI_SUCCESS;
}

static HI_VOID ISP_AfBlkCfg(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg, ISP_CTX_S *pstIspCtx)
{
    HI_U8  i;
    HI_U8  u8BlockNum   = pstRegCfg->u8CfgNum;
    HI_U8  u8WindowHnum = hi_ext_af_window_hnum_read(ViPipe);
    HI_U8  u8WindowVnum = hi_ext_af_window_vnum_read(ViPipe);
    HI_U16 u16Overlap;
    ISP_RECT_S        stBlockRect;
    ISP_AF_REG_CFG_S  *pstAFdefaultCfg = HI_NULL;

    u16Overlap = pstIspCtx->stBlockAttr.u32OverLap;

    for (i = 0; i < u8BlockNum; i++)
    {
        pstAFdefaultCfg   = &pstRegCfg->stAlgRegCfg[i].stBEAfRegCfg;

        if (i < u8WindowHnum % DIV_0_TO_1(u8BlockNum))
        {
            pstAFdefaultCfg->u16WindowHnum = u8WindowHnum / DIV_0_TO_1(u8BlockNum) + 1;
        }
        else
        {
            pstAFdefaultCfg->u16WindowHnum = u8WindowHnum / DIV_0_TO_1(u8BlockNum);
        }

        pstAFdefaultCfg->u16WindowVnum     = u8WindowVnum;

        pstRegCfg->stKernelRegCfg.astAlgKernelCfg[i].stBeAFZoneCfg.u32Colum     =  pstAFdefaultCfg->u16WindowHnum;
        pstRegCfg->stKernelRegCfg.astAlgKernelCfg[i].stBeAFZoneCfg.u32Row       =  pstAFdefaultCfg->u16WindowVnum;

        if (1 == u8BlockNum)
        {
            pstAFdefaultCfg->bCropEnable  = hi_ext_af_crop_enable_read(ViPipe);
            pstAFdefaultCfg->u16CropPosY  = hi_ext_af_crop_pos_y_read(ViPipe);
            pstAFdefaultCfg->u16CropPosX  = hi_ext_af_crop_pos_x_read(ViPipe);
            pstAFdefaultCfg->u16CropHsize = hi_ext_af_crop_hsize_read(ViPipe);
            pstAFdefaultCfg->u16CropVsize = hi_ext_af_crop_vsize_read(ViPipe);
        }
        else
        {
            ISP_GetBlockRect(&stBlockRect, &pstIspCtx->stBlockAttr, i);

            pstAFdefaultCfg->bCropEnable        = HI_TRUE;
            pstAFdefaultCfg->u16CropPosY        = stBlockRect.s32Y;
            pstAFdefaultCfg->u16CropVsize       = stBlockRect.u32Height;

            if (i == 0)
            {
                if (u8BlockNum > 1)
                {
                    pstAFdefaultCfg->u16CropPosX   = 0;
                    pstAFdefaultCfg->u16CropHsize  = stBlockRect.u32Width - u16Overlap;
                }
                else
                {
                    pstAFdefaultCfg->u16CropPosX  = 0;
                    pstAFdefaultCfg->u16CropHsize = stBlockRect.u32Width;
                }
            }
            else if (i == (u8BlockNum - 1))
            {
                pstAFdefaultCfg->u16CropPosX   = u16Overlap;
                pstAFdefaultCfg->u16CropHsize  = stBlockRect.u32Width - u16Overlap;
            }
            else
            {
                pstAFdefaultCfg->u16CropPosX   = u16Overlap;
                pstAFdefaultCfg->u16CropHsize  = stBlockRect.u32Width - (u16Overlap << 1);
            }
        }
    }
}

HI_S32 ISP_SetAfBeDefault(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 u8BlockNum = 0;
    HI_U8 i;
    HI_U8 u8I, u8Shift0;
    HI_S16 s16G0, s16G1, s16G2;
    ISP_AF_H_PARAM_S *pstIIR = HI_NULL;
    HI_U32 u32Pls, u32Plg;
    float f32Pl, f32Temp;

    ISP_BAYER_FORMAT_E u32Cfa;
    ISP_AF_REG_CFG_S    *pstAFdefaultCfg = HI_NULL;
    ISP_CTX_S           *pstIspCtx = HI_NULL;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8BlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

    for (i = 0; i < u8BlockNum; i++)
    {
        pstAFdefaultCfg   = &pstRegCfg->stAlgRegCfg[i].stBEAfRegCfg;

        u32Cfa = hi_ext_system_rggb_cfg_read(ViPipe);

        pstAFdefaultCfg->u8BayerMode        = (HI_U8)u32Cfa;
        pstAFdefaultCfg->bLpfEnable         = HI_ISP_BE_AF_LPF_EN_DEFAULT;
        pstAFdefaultCfg->bFir0LpfEnable     = HI_ISP_BE_AF_FIR0_LPF_EN_DEFAULT;
        pstAFdefaultCfg->bFir1LpfEnable     = HI_ISP_BE_AF_FIR1_LPF_EN_DEFAULT;
        pstAFdefaultCfg->u8IirPlgGroup0     = HI_ISP_BE_AF_IIR0_PLG_DEFAULT;
        pstAFdefaultCfg->u8IirPlsGroup0     = HI_ISP_BE_AF_IIR0_PLS_DEFAULT;
        pstAFdefaultCfg->u8IirPlgGroup1     = HI_ISP_BE_AF_IIR1_PLG_DEFAULT;
        pstAFdefaultCfg->u8IirPlsGroup1     = HI_ISP_BE_AF_IIR1_PLS_DEFAULT;
        pstAFdefaultCfg->u8IirDilate0       = pstAFdefaultCfg->bIir0DsEnable * 3 + 3;
        pstAFdefaultCfg->u8IirDilate1       = pstAFdefaultCfg->bIir1DsEnable * 3 + 3;

        /* PLG and PLS */
        for ( u8I = 0 ; u8I < 2 ; u8I++ )
        {
            pstIIR = u8I ? &(g_stFocusCfg[ViPipe].stHParam_IIR1) : &(g_stFocusCfg[ViPipe].stHParam_IIR0);

            u8Shift0 = (HI_U8)pstIIR->au16IIRShift[0];
            s16G0 = pstIIR->as16IIRGain[0];
            s16G1 = pstIIR->as16IIRGain[1];
            s16G2 = pstIIR->as16IIRGain[2];

            f32Pl = (512.f / DIV_0_TO_1(512 - 2 * s16G1 - s16G2) * s16G0) / (1 << u8Shift0);
            f32Temp = f32Pl;
            f32Temp = MIN2(7 - floor(log(f32Temp) / log(2)), 7);

            u32Pls = (HI_U32)f32Temp;
            u32Plg = (HI_U32)((f32Pl * (1 << u32Pls)) + 0.5);

            if (0 == u8I)
            {
                pstAFdefaultCfg->u8IirPlsGroup0 = u32Pls;
                pstAFdefaultCfg->u8IirPlgGroup0 = u32Plg;
            }
            else
            {
                pstAFdefaultCfg->u8IirPlsGroup1 = u32Pls;
                pstAFdefaultCfg->u8IirPlgGroup1 = u32Plg;
            }
        }

        /* AF BE STATISTICS CONIFG START */
        if ((pstIspCtx->stBlockAttr.stFrameRect.u32Height < AF_MIN_HEIGHT ) ||
            (pstIspCtx->stBlockAttr.stFrameRect.u32Width < AF_MIN_WIDTH))
        {
            pstAFdefaultCfg->bAfEnable      = HI_FALSE;
        }
        else
        {
            pstAFdefaultCfg->bAfEnable      = g_stFocusCfg[ViPipe].stConfig.bEnable;
        }

        pstAFdefaultCfg->bIir0DsEnable      = g_stFocusCfg[ViPipe].stHParam_IIR0.bNarrowBand;
        pstAFdefaultCfg->bIir1DsEnable      = g_stFocusCfg[ViPipe].stHParam_IIR1.bNarrowBand;
        pstAFdefaultCfg->bIir0Enable0       = g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[0];
        pstAFdefaultCfg->bIir0Enable1       = g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[1];
        pstAFdefaultCfg->bIir0Enable2       = g_stFocusCfg[ViPipe].stHParam_IIR0.abIIREn[2];
        pstAFdefaultCfg->bIir1Enable0       = g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[0];
        pstAFdefaultCfg->bIir1Enable1       = g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[1];
        pstAFdefaultCfg->bIir1Enable2       = g_stFocusCfg[ViPipe].stHParam_IIR1.abIIREn[2];
        pstAFdefaultCfg->enPeakMode         = g_stFocusCfg[ViPipe].stConfig.enPeakMode;
        pstAFdefaultCfg->enSquMode          = g_stFocusCfg[ViPipe].stConfig.enSquMode;

        pstAFdefaultCfg->s16IirGain0Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[0];
        pstAFdefaultCfg->s16IirGain0Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[0];
        pstAFdefaultCfg->s16IirGain1Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[1];
        pstAFdefaultCfg->s16IirGain1Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[1];
        pstAFdefaultCfg->s16IirGain2Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[2];
        pstAFdefaultCfg->s16IirGain2Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[2];
        pstAFdefaultCfg->s16IirGain3Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[3];
        pstAFdefaultCfg->s16IirGain3Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[3];
        pstAFdefaultCfg->s16IirGain4Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[4];
        pstAFdefaultCfg->s16IirGain4Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[4];
        pstAFdefaultCfg->s16IirGain5Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[5];
        pstAFdefaultCfg->s16IirGain5Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[5];
        pstAFdefaultCfg->s16IirGain6Group0  = g_stFocusCfg[ViPipe].stHParam_IIR0.as16IIRGain[6];
        pstAFdefaultCfg->s16IirGain6Group1  = g_stFocusCfg[ViPipe].stHParam_IIR1.as16IIRGain[6];

        pstAFdefaultCfg->u16Iir0ShiftGroup0 = g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[0];
        pstAFdefaultCfg->u16Iir1ShiftGroup0 = g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[1];
        pstAFdefaultCfg->u16Iir2ShiftGroup0 = g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[2];
        pstAFdefaultCfg->u16Iir3ShiftGroup0 = g_stFocusCfg[ViPipe].stHParam_IIR0.au16IIRShift[3];
        pstAFdefaultCfg->u16Iir0ShiftGroup1 = g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[0];
        pstAFdefaultCfg->u16Iir1ShiftGroup1 = g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[1];
        pstAFdefaultCfg->u16Iir2ShiftGroup1 = g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[2];
        pstAFdefaultCfg->u16Iir3ShiftGroup1 = g_stFocusCfg[ViPipe].stHParam_IIR1.au16IIRShift[3];

        pstAFdefaultCfg->s16FirHGain0Group0 = g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[0];
        pstAFdefaultCfg->s16FirHGain0Group1 = g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[0];
        pstAFdefaultCfg->s16FirHGain1Group0 = g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[1];
        pstAFdefaultCfg->s16FirHGain1Group1 = g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[1];
        pstAFdefaultCfg->s16FirHGain2Group0 = g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[2];
        pstAFdefaultCfg->s16FirHGain2Group1 = g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[2];
        pstAFdefaultCfg->s16FirHGain3Group0 = g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[3];
        pstAFdefaultCfg->s16FirHGain3Group1 = g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[3];
        pstAFdefaultCfg->s16FirHGain4Group0 = g_stFocusCfg[ViPipe].stVParam_FIR0.as16FIRH[4];
        pstAFdefaultCfg->s16FirHGain4Group1 = g_stFocusCfg[ViPipe].stVParam_FIR1.as16FIRH[4];

        /* AF BE raw cfg */
        pstAFdefaultCfg->u8AfPosSel         = g_stFocusCfg[ViPipe].stConfig.enStatisticsPos;
        pstAFdefaultCfg->bRawMode           = ~(((g_stFocusCfg[ViPipe].stConfig.enStatisticsPos) >> 0x1) & 0x1);
        if(pstIspCtx->bISPYuvMode == HI_TRUE)
        {
            pstAFdefaultCfg->u8AfPosSel = 2;
            pstAFdefaultCfg->bRawMode   = 0;
        }
        pstAFdefaultCfg->u8GainLimit        = g_stFocusCfg[ViPipe].stConfig.stRawCfg.GammaGainLimit;
        pstAFdefaultCfg->u8Gamma            = g_stFocusCfg[ViPipe].stConfig.stRawCfg.GammaValue;
        pstAFdefaultCfg->u8BayerMode        = g_stFocusCfg[ViPipe].stConfig.stRawCfg.enPattern;
        pstAFdefaultCfg->bOffsetEnable      = 0x0;
        pstAFdefaultCfg->u16OffsetGr        = 0x0;
        pstAFdefaultCfg->u16OffsetGb        = 0x0;

        /* AF BE pre median filter */
        pstAFdefaultCfg->bMeanEnable        = g_stFocusCfg[ViPipe].stConfig.stPreFltCfg.bEn;
        pstAFdefaultCfg->u16MeanThres       = g_stFocusCfg[ViPipe].stConfig.stPreFltCfg.u16strength;

        /* level depend gain */
        pstAFdefaultCfg->bIir0LdgEnable     = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.bLdEn;
        pstAFdefaultCfg->u16IirThre0Low     = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16ThLow;
        pstAFdefaultCfg->u16IirThre0High    = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16ThHigh;
        pstAFdefaultCfg->u16IirSlope0Low    = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16SlpLow;
        pstAFdefaultCfg->u16IirSlope0High   = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16SlpHigh;
        pstAFdefaultCfg->u16IirGain0Low     = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16GainLow;
        pstAFdefaultCfg->u16IirGain0High    = g_stFocusCfg[ViPipe].stHParam_IIR0.stLd.u16GainHigh;

        pstAFdefaultCfg->bIir1LdgEnable     = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.bLdEn;
        pstAFdefaultCfg->u16IirThre1Low     = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16ThLow;
        pstAFdefaultCfg->u16IirThre1High    = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16ThHigh;
        pstAFdefaultCfg->u16IirSlope1Low    = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16SlpLow;
        pstAFdefaultCfg->u16IirSlope1High   = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16SlpHigh;
        pstAFdefaultCfg->u16IirGain1Low     = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16GainLow;
        pstAFdefaultCfg->u16IirGain1High    = g_stFocusCfg[ViPipe].stHParam_IIR1.stLd.u16GainHigh;

        pstAFdefaultCfg->bFir0LdgEnable     = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.bLdEn;
        pstAFdefaultCfg->u16FirThre0Low     = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16ThLow;
        pstAFdefaultCfg->u16FirThre0High    = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16ThHigh;
        pstAFdefaultCfg->u16FirSlope0Low    = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16SlpLow;
        pstAFdefaultCfg->u16FirSlope0High   = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16SlpHigh;
        pstAFdefaultCfg->u16FirGain0Low     = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16GainLow;
        pstAFdefaultCfg->u16FirGain0High    = g_stFocusCfg[ViPipe].stVParam_FIR0.stLd.u16GainHigh;

        pstAFdefaultCfg->bFir1LdgEnable     = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.bLdEn;
        pstAFdefaultCfg->u16FirThre1Low     = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16ThLow;
        pstAFdefaultCfg->u16FirThre1High    = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16ThHigh;
        pstAFdefaultCfg->u16FirSlope1Low    = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16SlpLow;
        pstAFdefaultCfg->u16FirSlope1High   = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16SlpHigh;
        pstAFdefaultCfg->u16FirGain1Low     = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16GainLow;
        pstAFdefaultCfg->u16FirGain1High    = g_stFocusCfg[ViPipe].stVParam_FIR1.stLd.u16GainHigh;

        /* AF BE coring */
        pstAFdefaultCfg->u16IirThre0Coring  = g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Th;
        pstAFdefaultCfg->u16IirSlope0Coring = g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Slp;
        pstAFdefaultCfg->u16IirPeak0Coring  = g_stFocusCfg[ViPipe].stHParam_IIR0.stCoring.u16Lmt;

        pstAFdefaultCfg->u16IirThre1Coring  = g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Th;
        pstAFdefaultCfg->u16IirSlope1Coring = g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Slp;
        pstAFdefaultCfg->u16IirPeak1Coring  = g_stFocusCfg[ViPipe].stHParam_IIR1.stCoring.u16Lmt;

        pstAFdefaultCfg->u16FirThre0Coring  = g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Th;
        pstAFdefaultCfg->u16FirSlope0Coring = g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Slp;
        pstAFdefaultCfg->u16FirPeak0Coring  = g_stFocusCfg[ViPipe].stVParam_FIR0.stCoring.u16Lmt;

        pstAFdefaultCfg->u16FirThre1Coring  = g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Th;
        pstAFdefaultCfg->u16IirSlope1Coring = g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Slp;
        pstAFdefaultCfg->u16FirPeak1Coring  = g_stFocusCfg[ViPipe].stVParam_FIR1.stCoring.u16Lmt;

        /* high luma counter */
        pstAFdefaultCfg->u8HilighThre       = (HI_U8)g_stFocusCfg[ViPipe].stConfig.u16HighLumaTh;

        /* AF output shift */
        pstAFdefaultCfg->u16AccShift0H      = g_stFocusCfg[ViPipe].stFVParam.au16AccShiftH[0];
        pstAFdefaultCfg->u16AccShift1H      = g_stFocusCfg[ViPipe].stFVParam.au16AccShiftH[1];
        pstAFdefaultCfg->u16AccShift0V      = g_stFocusCfg[ViPipe].stFVParam.au16AccShiftV[0];
        pstAFdefaultCfg->u16AccShift1V      = g_stFocusCfg[ViPipe].stFVParam.au16AccShiftV[1];
        pstAFdefaultCfg->u16AccShiftY       = g_stFocusCfg[ViPipe].stFVParam.u16AccShiftY;
        pstAFdefaultCfg->u16ShiftCountY     = g_stFocusCfg[ViPipe].stFVParam.u16HlCntShift;

        pstAFdefaultCfg->u32UpdateIndex     = 1;
        pstRegCfg->unKey.bit1AfBeCfg        = 1;
        pstRegCfg->stKernelRegCfg.unKey.bit1BeAfCfg = 1;
    }

    ISP_AfBlkCfg(ViPipe, pstRegCfg, pstIspCtx);

    return HI_SUCCESS;
}

HI_S32 ISP_AfBeCfgRegs(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 u8BlockNum = 0;
    HI_U8 u8WindowHnum;
    HI_U8 i;
    ISP_AF_REG_CFG_S    *pstAFdefaultCfg = HI_NULL;
    ISP_CTX_S           *pstIspCtx = HI_NULL;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8BlockNum = pstIspCtx->stBlockAttr.u8BlockNum;

    for (i = 0; i < u8BlockNum; i++)
    {
        pstAFdefaultCfg   = &pstRegCfg->stAlgRegCfg[i].stBEAfRegCfg;

        pstAFdefaultCfg->u8BayerMode        = hi_ext_af_bayermode_read(ViPipe);
        pstAFdefaultCfg->bIir0DsEnable      = hi_ext_af_iir0_ds_enable_read(ViPipe);
        pstAFdefaultCfg->bIir1DsEnable      = hi_ext_af_iir1_ds_enable_read(ViPipe);
        pstAFdefaultCfg->u8IirDilate0       = pstAFdefaultCfg->bIir0DsEnable * 3 + 3;
        pstAFdefaultCfg->u8IirDilate1       = pstAFdefaultCfg->bIir1DsEnable * 3 + 3;
        pstAFdefaultCfg->u8IirPlgGroup0     = hi_ext_af_iir_plg_group0_read(ViPipe);
        pstAFdefaultCfg->u8IirPlsGroup0     = hi_ext_af_iir_pls_group0_read(ViPipe);
        pstAFdefaultCfg->u8IirPlgGroup1     = hi_ext_af_iir_plg_group1_read(ViPipe);
        pstAFdefaultCfg->u8IirPlsGroup1     = hi_ext_af_iir_pls_group1_read(ViPipe);

        /* AF BE STATISTICS CONIFG START */
        //pstAFdefaultCfg->bAfEnable      = hi_ext_af_enable_read(ViPipe);
        pstAFdefaultCfg->bAfEnable          = ((hi_ext_system_af_enable_read(ViPipe) >> 1) & 0x1);
        pstAFdefaultCfg->bIir0Enable0       = hi_ext_af_iir0_enable0_read(ViPipe);
        pstAFdefaultCfg->bIir0Enable1       = hi_ext_af_iir0_enable1_read(ViPipe);
        pstAFdefaultCfg->bIir0Enable2       = hi_ext_af_iir0_enable2_read(ViPipe);
        pstAFdefaultCfg->bIir1Enable0       = hi_ext_af_iir1_enable0_read(ViPipe);
        pstAFdefaultCfg->bIir1Enable1       = hi_ext_af_iir1_enable1_read(ViPipe);
        pstAFdefaultCfg->bIir1Enable2       = hi_ext_af_iir1_enable2_read(ViPipe);
        pstAFdefaultCfg->enPeakMode         = hi_ext_af_peakmode_read(ViPipe);
        pstAFdefaultCfg->enSquMode          = hi_ext_af_squmode_read(ViPipe);
        pstAFdefaultCfg->s16IirGain0Group0  = hi_ext_af_iir_gain0_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain0Group1  = hi_ext_af_iir_gain0_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain1Group0  = hi_ext_af_iir_gain1_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain1Group1  = hi_ext_af_iir_gain1_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain2Group0  = hi_ext_af_iir_gain2_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain2Group1  = hi_ext_af_iir_gain2_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain3Group0  = hi_ext_af_iir_gain3_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain3Group1  = hi_ext_af_iir_gain3_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain4Group0  = hi_ext_af_iir_gain4_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain4Group1  = hi_ext_af_iir_gain4_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain5Group0  = hi_ext_af_iir_gain5_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain5Group1  = hi_ext_af_iir_gain5_group1_read(ViPipe);
        pstAFdefaultCfg->s16IirGain6Group0  = hi_ext_af_iir_gain6_group0_read(ViPipe);
        pstAFdefaultCfg->s16IirGain6Group1  = hi_ext_af_iir_gain6_group1_read(ViPipe);

        pstAFdefaultCfg->u16Iir0ShiftGroup0 = hi_ext_af_iir0_shift_group0_read(ViPipe);
        pstAFdefaultCfg->u16Iir1ShiftGroup0 = hi_ext_af_iir1_shift_group0_read(ViPipe);
        pstAFdefaultCfg->u16Iir2ShiftGroup0 = hi_ext_af_iir2_shift_group0_read(ViPipe);
        pstAFdefaultCfg->u16Iir3ShiftGroup0 = hi_ext_af_iir3_shift_group0_read(ViPipe);
        pstAFdefaultCfg->u16Iir0ShiftGroup1 = hi_ext_af_iir0_shift_group1_read(ViPipe);
        pstAFdefaultCfg->u16Iir1ShiftGroup1 = hi_ext_af_iir1_shift_group1_read(ViPipe);
        pstAFdefaultCfg->u16Iir2ShiftGroup1 = hi_ext_af_iir2_shift_group1_read(ViPipe);
        pstAFdefaultCfg->u16Iir3ShiftGroup1 = hi_ext_af_iir3_shift_group1_read(ViPipe);

        pstAFdefaultCfg->s16FirHGain0Group0 = hi_ext_af_fir_h_gain0_group0_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain0Group1 = hi_ext_af_fir_h_gain0_group1_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain1Group0 = hi_ext_af_fir_h_gain1_group0_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain1Group1 = hi_ext_af_fir_h_gain1_group1_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain2Group0 = hi_ext_af_fir_h_gain2_group0_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain2Group1 = hi_ext_af_fir_h_gain2_group1_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain3Group0 = hi_ext_af_fir_h_gain3_group0_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain3Group1 = hi_ext_af_fir_h_gain3_group1_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain4Group0 = hi_ext_af_fir_h_gain4_group0_read(ViPipe);
        pstAFdefaultCfg->s16FirHGain4Group1 = hi_ext_af_fir_h_gain4_group1_read(ViPipe);

        u8WindowHnum      = hi_ext_af_window_hnum_read(ViPipe);

        if (i < u8WindowHnum % DIV_0_TO_1(u8BlockNum))
        {
            pstAFdefaultCfg->u16WindowHnum = u8WindowHnum / DIV_0_TO_1(u8BlockNum) + 1;
        }
        else
        {
            pstAFdefaultCfg->u16WindowHnum = u8WindowHnum / DIV_0_TO_1(u8BlockNum);
        }

        pstAFdefaultCfg->u16WindowVnum     = hi_ext_af_window_vnum_read(ViPipe);
        pstRegCfg->stKernelRegCfg.astAlgKernelCfg[i].stBeAFZoneCfg.u32Colum     =  pstAFdefaultCfg->u16WindowHnum;
        pstRegCfg->stKernelRegCfg.astAlgKernelCfg[i].stBeAFZoneCfg.u32Row       =  pstAFdefaultCfg->u16WindowVnum;

        /* AF BE crop */
        if (1 == u8BlockNum)
        {
            pstAFdefaultCfg->bCropEnable        = hi_ext_af_crop_enable_read(ViPipe);
            pstAFdefaultCfg->u16CropPosY        = hi_ext_af_crop_pos_y_read(ViPipe);
            pstAFdefaultCfg->u16CropPosX        = hi_ext_af_crop_pos_x_read(ViPipe);
            pstAFdefaultCfg->u16CropHsize       = hi_ext_af_crop_hsize_read(ViPipe);
            pstAFdefaultCfg->u16CropVsize       = hi_ext_af_crop_vsize_read(ViPipe);
        }

        /* AF BE raw cfg */
        pstAFdefaultCfg->u8AfPosSel         = hi_ext_af_pos_sel_read(ViPipe);
        pstAFdefaultCfg->bRawMode           = hi_ext_af_rawmode_read(ViPipe);
        if(pstIspCtx->bISPYuvMode == HI_TRUE)
        {
            pstAFdefaultCfg->u8AfPosSel = 2;
            pstAFdefaultCfg->bRawMode   = 0;
        }
        pstAFdefaultCfg->u8GainLimit        = hi_ext_af_gain_limit_read(ViPipe);
        pstAFdefaultCfg->u8Gamma            = hi_ext_af_gamma_read(ViPipe);
        pstAFdefaultCfg->u8BayerMode        = hi_ext_af_bayermode_read(ViPipe);
        pstAFdefaultCfg->bOffsetEnable      = 0x0;
        pstAFdefaultCfg->u16OffsetGr        = 0x0;
        pstAFdefaultCfg->u16OffsetGb        = 0x0;

        /* AF BE pre median filter */
        pstAFdefaultCfg->bMeanEnable        = hi_ext_af_mean_enable_read(ViPipe);
        pstAFdefaultCfg->u16MeanThres       = hi_ext_af_mean_thres_read(ViPipe);

        /* level depend gain */
        pstAFdefaultCfg->bIir0LdgEnable     = hi_ext_af_iir0_ldg_enable_read(ViPipe);
        pstAFdefaultCfg->u16IirThre0Low     = hi_ext_af_iir_thre0_low_read(ViPipe);
        pstAFdefaultCfg->u16IirThre0High    = hi_ext_af_iir_thre0_high_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope0Low    = hi_ext_af_iir_slope0_low_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope0High   = hi_ext_af_iir_slope0_high_read(ViPipe);
        pstAFdefaultCfg->u16IirGain0Low     = hi_ext_af_iir_gain0_low_read(ViPipe);
        pstAFdefaultCfg->u16IirGain0High    = hi_ext_af_iir_gain0_high_read(ViPipe);

        pstAFdefaultCfg->bIir1LdgEnable     = hi_ext_af_iir1_ldg_enable_read(ViPipe);
        pstAFdefaultCfg->u16IirThre1Low     = hi_ext_af_iir_thre1_low_read(ViPipe);
        pstAFdefaultCfg->u16IirThre1High    = hi_ext_af_iir_thre1_high_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope1Low    = hi_ext_af_iir_slope1_low_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope1High   = hi_ext_af_iir_slope1_high_read(ViPipe);
        pstAFdefaultCfg->u16IirGain1Low     = hi_ext_af_iir_gain1_low_read(ViPipe);
        pstAFdefaultCfg->u16IirGain1High    = hi_ext_af_iir_gain1_high_read(ViPipe);

        pstAFdefaultCfg->bFir0LdgEnable     = hi_ext_af_fir0_ldg_enable_read(ViPipe);
        pstAFdefaultCfg->u16FirThre0Low     = hi_ext_af_fir_thre0_low_read(ViPipe);
        pstAFdefaultCfg->u16FirThre0High    = hi_ext_af_fir_thre0_high_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope0Low    = hi_ext_af_fir_slope0_low_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope0High   = hi_ext_af_fir_slope0_high_read(ViPipe);
        pstAFdefaultCfg->u16FirGain0Low     = hi_ext_af_fir_gain0_low_read(ViPipe);
        pstAFdefaultCfg->u16FirGain0High    = hi_ext_af_fir_gain0_high_read(ViPipe);

        pstAFdefaultCfg->bFir1LdgEnable     = hi_ext_af_fir1_ldg_enable_read(ViPipe);
        pstAFdefaultCfg->u16FirThre1Low     = hi_ext_af_fir_thre1_low_read(ViPipe);
        pstAFdefaultCfg->u16FirThre1High    = hi_ext_af_fir_thre1_high_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope1Low    = hi_ext_af_fir_slope1_low_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope1High   = hi_ext_af_fir_slope1_high_read(ViPipe);
        pstAFdefaultCfg->u16FirGain1Low     = hi_ext_af_fir_gain1_low_read(ViPipe);
        pstAFdefaultCfg->u16FirGain1High    = hi_ext_af_fir_gain1_high_read(ViPipe);

        /* AF BE coring */
        pstAFdefaultCfg->u16IirThre0Coring  = hi_ext_af_iir_thre0_coring_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope0Coring = hi_ext_af_iir_slope0_coring_read(ViPipe);
        pstAFdefaultCfg->u16IirPeak0Coring  = hi_ext_af_iir_peak0_coring_read(ViPipe);

        pstAFdefaultCfg->u16IirThre1Coring  = hi_ext_af_iir_thre1_coring_read(ViPipe);
        pstAFdefaultCfg->u16IirSlope1Coring = hi_ext_af_iir_slope1_coring_read(ViPipe);
        pstAFdefaultCfg->u16IirPeak1Coring  = hi_ext_af_iir_peak1_coring_read(ViPipe);

        pstAFdefaultCfg->u16FirThre0Coring  = hi_ext_af_fir_thre0_coring_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope0Coring = hi_ext_af_fir_slope0_coring_read(ViPipe);
        pstAFdefaultCfg->u16FirPeak0Coring  = hi_ext_af_fir_peak0_coring_read(ViPipe);

        pstAFdefaultCfg->u16FirThre1Coring  = hi_ext_af_fir_thre1_coring_read(ViPipe);
        pstAFdefaultCfg->u16FirSlope1Coring = hi_ext_af_fir_slope1_coring_read(ViPipe);
        pstAFdefaultCfg->u16FirPeak1Coring  = hi_ext_af_fir_peak1_coring_read(ViPipe);

        /* high luma counter */
        pstAFdefaultCfg->u8HilighThre       = hi_ext_af_hiligh_thre_read(ViPipe);

        /* AF output shift */
        pstAFdefaultCfg->u16AccShift0H      = hi_ext_af_acc_shift0_h_read(ViPipe);
        pstAFdefaultCfg->u16AccShift1H      = hi_ext_af_acc_shift1_h_read(ViPipe);
        pstAFdefaultCfg->u16AccShift0V      = hi_ext_af_acc_shift0_v_read(ViPipe);
        pstAFdefaultCfg->u16AccShift1V      = hi_ext_af_acc_shift1_v_read(ViPipe);
        pstAFdefaultCfg->u16AccShiftY       = hi_ext_af_acc_shift_y_read(ViPipe);
        pstAFdefaultCfg->u16ShiftCountY     = hi_ext_af_shift_count_y_read(ViPipe);

        pstAFdefaultCfg->u32UpdateIndex    += 1;
        pstRegCfg->unKey.bit1AfBeCfg        = 1;

        pstRegCfg->stKernelRegCfg.unKey.bit1BeAfCfg = 1;
    }

    return HI_SUCCESS;
}

static HI_VOID ISP_AfImageResWrite(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8 i;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    hi_ext_af_crop_pos_x_write(ViPipe, 0);
    hi_ext_af_crop_pos_y_write(ViPipe, 0);
    hi_ext_af_crop_hsize_write(ViPipe, pstIspCtx->stBlockAttr.stFrameRect.u32Width / 8 * 8);
    hi_ext_af_crop_vsize_write(ViPipe, pstIspCtx->stBlockAttr.stFrameRect.u32Height / 2 * 2);

    ISP_AfBlkCfg(ViPipe, pstRegCfg, pstIspCtx);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stBEAfRegCfg.u32UpdateIndex++;
    }

    pstRegCfg->unKey.bit1AfFeCfg = 1;
}

static HI_VOID ISP_AfWdrModeSet(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    ISP_CTX_S     *pstIspCtx = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stBlockAttr.u8BlockNum != pstIspCtx->stBlockAttr.u8PreBlockNum)
    {
        ISP_AfImageResWrite(ViPipe, pstRegCfg);
    }

    pstRegCfg->unKey.bit1AfFeCfg = 1;
}

HI_S32 ISP_AfInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    /* some registers about AF module shound be initial */
    ISP_AfExtRegsDefault(ViPipe);
    ISP_SetAfBeDefault(ViPipe, (ISP_REG_CFG_S *)pRegCfg);

    return HI_SUCCESS;
}

HI_S32 ISP_AfRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                 HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    ISP_CTX_S          *pstIspCtx   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    if (HI_TRUE == pstIspCtx->stLinkage.bSnapState)
    {
        return HI_SUCCESS;
    }

    if (HI_EXT_AF_SET_FLAG_ENABLE == hi_ext_af_set_flag_read(ViPipe))
    {
        hi_ext_af_set_flag_write(ViPipe, HI_EXT_AF_SET_FLAG_DISABLE);
        ISP_AfBeCfgRegs(ViPipe, (ISP_REG_CFG_S *)pRegCfg);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AfCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg    = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_AfWdrModeSet(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_CHANGE_IMAGE_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_AfImageResWrite(ViPipe, &pRegCfg->stRegCfg);
            break;
        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_AfExit(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterAf(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Af);

    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_AF;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_AfInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_AfRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_AfCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_AfExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
