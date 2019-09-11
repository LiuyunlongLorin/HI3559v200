/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mn34220_cmos.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/11/07
  Description   :
  History       :
  1.Date        : 2013/11/07
    Author      :
    Modification: Created file

******************************************************************************/
#if !defined(__MN34220_CMOS_H_)
#define __MN34220_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mn34220_cmos_ex.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MN34220_ID 34220

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S             *g_astMn34220[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define MN34220_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_astMn34220[dev])
#define MN34220_SENSOR_SET_CTX(dev, pstCtx)   (g_astMn34220[dev] = pstCtx)
#define MN34220_SENSOR_RESET_CTX(dev)         (g_astMn34220[dev] = HI_NULL)

ISP_SNS_COMMBUS_U g_aunMn34220BusInfo[ISP_MAX_PIPE_NUM] =
{
    [0] = { .s8I2cDev = 0},
    [1 ... ISP_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

static ISP_FSWDR_MODE_E genFSWDRMode[ISP_MAX_PIPE_NUM] =
{
    [0 ... ISP_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

static HI_U32           gu32MaxTimeGetCnt[ISP_MAX_PIPE_NUM] = {0};
/****************************************************************************
 * extern                                                                   *
 ****************************************************************************/
extern unsigned char mn34220_i2c_addr;
extern unsigned int  mn34220_addr_byte;
extern unsigned int  mn34220_data_byte;

extern void mn34220_init(VI_PIPE ViPipe);
extern void mn34220_exit(VI_PIPE ViPipe);
extern void mn34220_standby(VI_PIPE ViPipe);
extern void mn34220_restart(VI_PIPE ViPipe);
extern int mn34220_write_register(VI_PIPE ViPipe, int addr, int data);
extern int mn34220_read_register(VI_PIPE ViPipe, int addr);

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
#define MN34220_FULL_LINES_MAX  (0xFFFF)

#define MN34220_EXPOSURE_ADDR_H             (0x0202)
#define MN34220_EXPOSURE_ADDR_L             (0x0203)
#define MN34220_LONG_EXPOSURE_ADDR_H        (0x312A)
#define MN34220_LONG_EXPOSURE_ADDR_L        (0x312B)
#define MN34220_SHORT_EXPOSURE_ADDR_H       (0x312C)
#define MN34220_SHORT_EXPOSURE_ADDR_L       (0x312D)
#define MN34220_SHORT_SHORT_EXPOSURE_ADDR_H (0x312E)
#define MN34220_SHORT_SHORT_EXPOSURE_ADDR_L (0x312F)
#define MN34220_AGC_ADDR_H                  (0x0204)
#define MN34220_AGC_ADDR_L                  (0x0205)
#define MN34220_DGC_ADDR_H                  (0x3108)
#define MN34220_DGC_ADDR_L                  (0x3109)
#define MN34220_VMAX_ADDR_H                 (0x0340)
#define MN34220_VMAX_ADDR_L                 (0x0341)

#define MN34220_INCREASE_LINES              (1) /* make real fps less than stand fps because NVR require*/
#define MN34220_VMAX_1080P30_LINEAR         (1125 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_1080P60_LINEAR         (1125 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_1080P60TO30_WDR        (1250 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_1080P50TO25_WDR        (1500 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_1080P120TO30_WDR       (1125 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_1080P90TO30_WDR        (1500 + MN34220_INCREASE_LINES)
#define MN34220_VMAX_720P_WDR               (750  + MN34220_INCREASE_LINES)
#define MN34220_VMAX_720P120_LINEAR         (750  + MN34220_INCREASE_LINES)
#define MN34220_VMAX_VGA240_LINEAR          (562  + MN34220_INCREASE_LINES)

#define MN34220_1080P_60FPS_MODE            (1)
#define MN34220_1080P_30FPS_MODE            (2)
#define MN34220_720P_120FPS_MODE            (3)
#define MN34220_VGA_240FPS_MODE             (4)
#define MN34220_720P_30FPS_MODE             (5)  /* for FPGA */

#define MN34220_RES_IS_VGA(w, h)        ((w) <= 640  && (h) <= 480)
#define MN34220_RES_IS_720P(w, h)       ((w) <= 1280 && (h) <= 720)
#define MN34220_RES_IS_1080P(w, h)      ((w) <= 1920 && (h) <= 1080)

static HI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));

    pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 25 / 2;
    pstAeSnsDft->u32FullLinesStd  = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq   = 0;
    pstAeSnsDft->u32FullLinesMax  = MN34220_FULL_LINES_MAX;

    pstAeSnsDft->stIntTimeAccu.enAccuType  = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset   = 0.8045;

    pstAeSnsDft->stAgainAccu.enAccuType  = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.38;

    pstAeSnsDft->stDgainAccu.enAccuType  = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.38;

    pstAeSnsDft->u32ISPDgainShift     = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift;

    pstAeSnsDft->u32InitExposure = 921600;

    if ((pstAeSnsDft->f32Fps == 50) || (pstAeSnsDft->f32Fps == 60))
    {
        pstAeSnsDft->stIntTimeAccu.f32Offset = 0.6082;
    }
    else if ((pstAeSnsDft->f32Fps == 25) || (pstAeSnsDft->f32Fps == 30))
    {
        pstAeSnsDft->stIntTimeAccu.f32Offset = 0.8045;
    }
    else
    {}

    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->bAERouteExValid = HI_FALSE;
    pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
    pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;

    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x38;
            pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 32382;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 32382;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
            break;

        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE: /*linear mode for ISP frame switching WDR*/
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x18;
            pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 32382;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 32382;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->u16ManRatioEnable = HI_TRUE;
            pstAeSnsDft->au32Ratio[0]      = 0x200;

            break;

        case WDR_MODE_2To1_LINE:
            pstAeSnsDft->u16ManRatioEnable = HI_FALSE;
            pstAeSnsDft->au32Ratio[0] = 0xc9;
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x1C;
            pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 32382;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = 8134;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 32382;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = 1024;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            if ((pstAeSnsDft->f32Fps == 50) || (pstAeSnsDft->f32Fps == 60))
            {
                pstAeSnsDft->stIntTimeAccu.f32Offset = 0.2155;
            }
            else if ((pstAeSnsDft->f32Fps == 25) || (pstAeSnsDft->f32Fps == 30))
            {
                pstAeSnsDft->stIntTimeAccu.f32Offset = 0.2818;
            }
            else
            {}

            if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
            {
                pstAeSnsDft->u32MaxAgainTarget = 32382;
                pstAeSnsDft->u32MaxDgainTarget = 32382;
                pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift;
                pstAeSnsDft->u8AeCompensation = 0x38;
                pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;
            }
            break;

        case WDR_MODE_3To1_LINE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x1C;
            pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;  // Manual ratio the AE compensation must be smaller than linear

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 32382;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = 32382;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 32382;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = 32382;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->bAERouteExValid = HI_FALSE;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
            pstAeSnsDft->u16ManRatioEnable = HI_FALSE;
            pstAeSnsDft->au32Ratio[0] = 0x200;
            pstAeSnsDft->au32Ratio[1] = 0x200;
            pstAeSnsDft->au32Ratio[2] = 0x40;

            if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
            {
                pstAeSnsDft->u8AeCompensation = 0x38;
                pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
                pstAeSnsDft->u32MaxAgainTarget = 32382;
                pstAeSnsDft->u32MaxDgainTarget = 32382;
                pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift;;
            }

            break;

        case WDR_MODE_4To1_LINE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x40;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 32382;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = 32382;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 32382;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = 32382;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;

            if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
            {
                pstAeSnsDft->u8AeCompensation = 0x38;
                pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
                pstAeSnsDft->u32MaxAgainTarget = 32382;
                pstAeSnsDft->u32MaxDgainTarget = 32382;
                pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift;;
            }

            break;
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_fps_set_2To1_LINE(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
    {
        /* In WDR_MODE_2To1_LINE mode, 30fps means 60 frames(30 short and 30 long) combine to 30 frames per sencond */
        if ((f32Fps <= 30) && (f32Fps >= 1))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_1080P60TO30_WDR * 30 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.2818;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else
    {
#if 1
        /* for FPGA */
        if ((f32Fps <= 30) && (f32Fps >= 2))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_720P_WDR * 30 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
#endif
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_fps_set_3To1_LINE(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 1))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_1080P90TO30_WDR * 30 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else
    {
#if 1
        /* for FPGA */
        if ((f32Fps <= 20) && (f32Fps >= 2))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_720P_WDR * 20 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
#endif
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_fps_set_4To1_LINE(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 1))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_1080P120TO30_WDR * 30 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else
    {
#if 1
        /* for FPGA */
        if ((f32Fps <= 15) && (f32Fps >= 2))
        {
            pstSnsState->u32FLStd     = MN34220_VMAX_720P_WDR * 15 / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.1077;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }

#endif
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_fps_set_None_LINE(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 1))
        {
            /* In 1080P30fps mode, the VMAX(FullLines) is MN34220_VMAX_1080P30_LINEAR,
                 and there are (MN34220_VMAX_1080P30_LINEAR*30) lines in 1 second,
                 so in f32Fps mode, VMAX(FullLines) is (MN34220_VMAX_1080P30_LINEAR*30)/f32Fps */
            pstSnsState->u32FLStd     = (MN34220_VMAX_1080P30_LINEAR * 30) / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.8045;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else if (MN34220_1080P_60FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 60) && (f32Fps >= 1.5))
        {
            /* In 1080P60fps mode, the VMAX(FullLines) is MN34220_VMAX_1080P60_LINEAR,
                 and there are (MN34220_VMAX_1080P60_LINEAR*60) lines in 1 second,
                 so in f32Fps mode, VMAX(FullLines) is (VMA X_1080P60_LINEAR*60)/f32Fps */
            pstSnsState->u32FLStd     = (MN34220_VMAX_1080P60_LINEAR * 60) / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.6082;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else if (MN34220_720P_30FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {
            /* In 720P120fps mode, the VMAX(FullLines) is MN34220_VMAX_720P120_LINEAR,
                 and there are (MN34220_VMAX_720P120_LINEAR*120) lines in 1 second,
                 so in f32Fps mode, VMAX(FullLines) is (VMA X_720P120_LINEAR*120)/f32Fps */
            pstSnsState->u32FLStd     = (MN34220_VMAX_720P120_LINEAR * 30) / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0;  /* TBD */
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else if (MN34220_720P_120FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 120) && (f32Fps >= 1.5))
        {
            /* In 720P120fps mode, the VMAX(FullLines) is MN34220_VMAX_720P120_LINEAR,
                 and there are (MN34220_VMAX_720P120_LINEAR*120) lines in 1 second,
                 so in f32Fps mode, VMAX(FullLines) is (VMA X_720P120_LINEAR*120)/f32Fps */
            pstSnsState->u32FLStd     = (MN34220_VMAX_720P120_LINEAR * 120) / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.5982;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else if (MN34220_VGA_240FPS_MODE == pstSnsState->u8ImgMode)
    {
        if ((f32Fps <= 240) && (f32Fps >= 2.5))
        {
            /* In VGA240fps mode, the VMAX(FullLines) is MN34220_VMAX_VGA240_LINEAR,
                 and there are (MN34220_VMAX_VGA240_LINEAR*240) lines in 1 second,
                 so in f32Fps mode, VMAX(FullLines) is (VMA X_720P240_LINEAR*240)/f32Fps */
            pstSnsState->u32FLStd     = (MN34220_VMAX_VGA240_LINEAR * 240) / f32Fps;
            pstAeSnsDft->stIntTimeAccu.f32Offset = 0.4309;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "Not support! u8ImgMode:%d, f32Fps:%f\n", pstSnsState->u8ImgMode, f32Fps);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


/* the function of sensor set fps */
static HI_VOID cmos_fps_set(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_S32 s32Ret;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        s32Ret = cmos_fps_set_2To1_LINE(ViPipe, f32Fps, pstAeSnsDft);

        if (HI_SUCCESS != s32Ret)
        {
            return;
        }
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
    {
        s32Ret = cmos_fps_set_3To1_LINE(ViPipe, f32Fps, pstAeSnsDft);

        if (HI_SUCCESS != s32Ret)
        {
            return;
        }
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
    {
        s32Ret = cmos_fps_set_4To1_LINE(ViPipe, f32Fps, pstAeSnsDft);

        if (HI_SUCCESS != s32Ret)
        {
            return;
        }
    }
    else
    {
        s32Ret = cmos_fps_set_None_LINE(ViPipe, f32Fps, pstAeSnsDft);

        if (HI_SUCCESS != s32Ret)
        {
            return;
        }
    }

    pstSnsState->u32FLStd = (pstSnsState->u32FLStd > MN34220_FULL_LINES_MAX) ? MN34220_FULL_LINES_MAX : pstSnsState->u32FLStd;

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((pstSnsState->u32FLStd & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (pstSnsState->u32FLStd & 0xFF);
    }
    else if (IS_2to1_WDR_MODE(pstSnsState->enWDRMode))
    {
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((pstSnsState->u32FLStd & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (pstSnsState->u32FLStd & 0xFF);
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = ((pstSnsState->u32FLStd & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = (pstSnsState->u32FLStd & 0xFF);
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[13].u32Data = ((pstSnsState->u32FLStd & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = (pstSnsState->u32FLStd & 0xFF);
    }
    else
    {
    }

    pstAeSnsDft->f32Fps               = f32Fps;
    pstAeSnsDft->u32LinesPer500ms     = pstSnsState->u32FLStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime        = pstSnsState->u32FLStd - 2;
    pstAeSnsDft->u32FullLinesStd      = pstSnsState->u32FLStd;
    pstSnsState->au32FL[0]            = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines         = pstSnsState->au32FL[0];

    return;
}

static HI_VOID cmos_slow_framerate_set(VI_PIPE ViPipe, HI_U32 u32FullLines,
                                       AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32FullLines = (u32FullLines > MN34220_FULL_LINES_MAX) ? MN34220_FULL_LINES_MAX : u32FullLines;
    pstSnsState->au32FL[0] = u32FullLines;

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (pstSnsState->au32FL[0] & 0xFF);
    }
    else if ((WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
    {
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (pstSnsState->au32FL[0] & 0xFF);
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = (pstSnsState->au32FL[0] & 0xFF);
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[13].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = (pstSnsState->au32FL[0] & 0xFF);
    }
    else
    {
    }

    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - 2;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID cmos_inttime_update(VI_PIPE ViPipe, HI_U32 u32IntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    static HI_BOOL bFirst = HI_TRUE;
    static HI_U8 u8Count = 0;

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
    {
        if (bFirst) /* short exposure */
        {
            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_FALSE;
        }
        else    /* long exposure */
        {
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_TRUE;
        }
    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        if (bFirst) /* short exposure */
        {
            pstSnsState->au32WDRIntTime[0] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_FALSE;
        }
        else    /* long exposure */
        {
            pstSnsState->au32WDRIntTime[1] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_TRUE;
        }
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
    {
        if (0 == u8Count) /* short exposure */
        {
            pstSnsState->au32WDRIntTime[0] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (u32IntTime & 0xFF);
            u8Count++;
        }
        else if (1 == u8Count) /* long exposure */
        {
            pstSnsState->au32WDRIntTime[1] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32IntTime & 0xFF);
            u8Count++;
        }
        else    /* long long exposure */
        {
            pstSnsState->au32WDRIntTime[2] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32IntTime & 0xFF);
            u8Count = 0;
        }
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
    {
        if (0 == u8Count) /* short short exposure */
        {
            pstSnsState->au32WDRIntTime[0] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = (u32IntTime & 0xFF);
            u8Count++;
        }
        else if (1 == u8Count) /* short exposure */
        {
            pstSnsState->au32WDRIntTime[1] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (u32IntTime & 0xFF);
            u8Count++;
        }
        else if (2 == u8Count) /* long exposure */
        {
            pstSnsState->au32WDRIntTime[2] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32IntTime & 0xFF);
            u8Count++;
        }
        else    /* long long exposure */
        {
            pstSnsState->au32WDRIntTime[3] = u32IntTime;
            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32IntTime & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32IntTime & 0xFF);
            u8Count = 0;
        }
    }
    else
    {
        pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32IntTime & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32IntTime & 0xFF);
        bFirst = HI_TRUE;
    }

    return;
}

static HI_U32 ad_gain_table[81] =
{
    1024, 1070, 1116, 1166, 1217, 1271, 1327, 1386, 1446, 1511, 1577, 1647, 1719, 1796, 1874, 1958, 2043,
    2135, 2227, 2327, 2428, 2537, 2647, 2766, 2886, 3015, 3146, 3287, 3430, 3583, 3739, 3907, 4077, 4259,
    4444, 4643, 4845, 5062, 5282, 5518, 5758, 6016, 6278, 6558, 6844, 7150, 7461, 7795, 8134, 8498, 8867,
    9264, 9667, 10100, 10539, 11010, 11489, 12003, 12526, 13086, 13655, 14266, 14887, 15552, 16229, 16955,
    17693, 18484, 19289, 20151, 21028, 21968, 22925, 23950, 24992, 26110, 27246, 28464, 29703, 31031, 32382
};

static HI_VOID cmos_again_calc_table(VI_PIPE ViPipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);

    if (*pu32AgainLin >= ad_gain_table[80])
    {
        *pu32AgainLin = ad_gain_table[80];
        *pu32AgainDb = 80;
        return ;
    }

    for (i = 1; i < 81; i++)
    {
        if (*pu32AgainLin < ad_gain_table[i])
        {
            *pu32AgainLin = ad_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }

    return;
}

static HI_VOID cmos_dgain_calc_table(VI_PIPE ViPipe, HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32DgainLin);
    CMOS_CHECK_POINTER_VOID(pu32DgainDb);

    if (*pu32DgainLin >= ad_gain_table[80])
    {
        *pu32DgainLin = ad_gain_table[80];
        *pu32DgainDb = 80;
        return ;
    }

    for (i = 1; i < 81; i++)
    {
        if (*pu32DgainLin < ad_gain_table[i])
        {
            *pu32DgainLin = ad_gain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }

    return;
}

static HI_VOID cmos_gains_update(VI_PIPE ViPipe, HI_U32 u32Again, HI_U32 u32Dgain)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32Tmp_Again = 0x100 + u32Again * 4;
    HI_U32 u32Tmp_Dgain = 0x100 + u32Dgain * 4;

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if ((pstSnsState->astRegsInfo[0].astI2cData[3].u32Data == ((u32Tmp_Again & 0xFF00) >> 8)) &&
        (pstSnsState->astRegsInfo[0].astI2cData[4].u32Data == (u32Tmp_Again & 0xFF)))
    {
        pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = 0x30;
    }
    else
    {
        pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = 0x0;
    }

    pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = ((u32Tmp_Again & 0xFF00) >> 8);
    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = (u32Tmp_Again & 0xFF);

    pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = ((u32Tmp_Dgain & 0xFF00) >> 8);
    pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = (u32Tmp_Dgain & 0xFF);

    return;
}

static HI_S32 cmos_get_inttime_max_temp(HI_U32 *pu32IntTimeMaxTmp, VI_PIPE ViPipe, HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32  u32IntTimeMaxTmp1 = 0;

    CMOS_CHECK_POINTER(pu32IntTimeMaxTmp);
    CMOS_CHECK_POINTER(au32Ratio);
    CMOS_CHECK_POINTER(au32IntTimeMax);
    CMOS_CHECK_POINTER(au32IntTimeMin);
    CMOS_CHECK_POINTER(pu32LFMaxIntTime);

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
    {
        *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[0] - 2) << 6) / DIV_0_TO_1(au32Ratio[0]);
    }
    else if ((WDR_MODE_3To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_FRAME == pstSnsState->enWDRMode))
    {
        *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[0] - 2) << 12) / (DIV_0_TO_1(au32Ratio[0]) * DIV_0_TO_1(au32Ratio[1]));
    }
    else if ((WDR_MODE_4To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_FRAME == pstSnsState->enWDRMode))
    {
        *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[0] - 2) << 18) / (DIV_0_TO_1(au32Ratio[0]) * DIV_0_TO_1(au32Ratio[1]) * DIV_0_TO_1(au32Ratio[2]));
    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe]) /* long frame mode enable */
        {
            /* when change LongFrameMode, the first 2 frames must limit the MaxIntTime to avoid flicker */
            *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 10 - pstSnsState->au32WDRIntTime[0];
            if (gu32MaxTimeGetCnt[ViPipe] < 2)
            {
                *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 10 - 750;
            }

            u32IntTimeMaxTmp1 = pstSnsState->au32FL[0] - 10;
            *pu32IntTimeMaxTmp  = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
            au32IntTimeMax[0] = *pu32IntTimeMaxTmp;
            au32IntTimeMin[0] = 2;
            gu32MaxTimeGetCnt[ViPipe]++;
            return 2;
        }
        else
        {
            /* 2 + PreShort + 3 + Long <= 1PreVmax; */
            /* 2 + Short + 3 + Long <= 1V;
               Ratio = Long * 0x40 / Short */
            /* To avoid Long frame and PreShort frame exposure time out of sync, so use (gu32FullLinesStd - 30) */
            *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 30 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
            u32IntTimeMaxTmp1 = ((pstSnsState->au32FL[0] - 30) << 6) / (au32Ratio[0] + 0x40);
            *pu32IntTimeMaxTmp = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
        }
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
    {
        if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe]) /* long frame mode enable */
        {
            /* when change LongFrameMode, the first 2 frames must limit the MaxIntTime to avoid flicker */
            *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 30 - pstSnsState->au32WDRIntTime[0] - pstSnsState->au32WDRIntTime[1];

            if (gu32MaxTimeGetCnt[ViPipe] < 2)
            {
                *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 30 - 750;
            }

            u32IntTimeMaxTmp1 = pstSnsState->au32FL[0] - 30;
            *pu32IntTimeMaxTmp = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
            au32IntTimeMax[0] = *pu32IntTimeMaxTmp;
            au32IntTimeMin[0] = 2;
            gu32MaxTimeGetCnt[ViPipe]++;
            return 2;
        }
        else
        {
            /* 2 + PreVS + 2 + PreS + 3 + L <= 1PreVmax; */
            /* 2 + SS + 2 + S + 3 + L <= 1V;
               Ratio0 = S * 0x40 / SS
               Ratio1 = L * 0x40 / S */
            *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 30 - pstSnsState->au32WDRIntTime[0] - pstSnsState->au32WDRIntTime[1]) << 12) \
                                 / (DIV_0_TO_1(au32Ratio[0]) * DIV_0_TO_1(au32Ratio[1]));
            u32IntTimeMaxTmp1 = ((pstSnsState->au32FL[0] - 30) << 12) / (au32Ratio[0] * au32Ratio[1] + au32Ratio[0] * 0x40 + 0x40 * 0x40);
            *pu32IntTimeMaxTmp = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
        }
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
    {
        if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe]) /* long frame mode enable */
        {
            /* when change LongFrameMode, the first 2 frames must limit the MaxIntTime to avoid flicker */
            *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 30 - pstSnsState->au32WDRIntTime[0] - pstSnsState->au32WDRIntTime[1] - pstSnsState->au32WDRIntTime[2];
            if (gu32MaxTimeGetCnt[ViPipe] < 2)
            {
                *pu32IntTimeMaxTmp = pstSnsState->au32FL[1] - 30 - 560;
            }

            u32IntTimeMaxTmp1 = pstSnsState->au32FL[0] - 30;
            *pu32IntTimeMaxTmp = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
            au32IntTimeMax[0] = *pu32IntTimeMaxTmp;
            au32IntTimeMin[0] = 2;
            gu32MaxTimeGetCnt[ViPipe]++;
            return 2;
        }
        else
        {
            /* 2 + PreVS + 2 + PreS + 2 + PreL + 3 + LL <= 1PreVmax;  */
            /* 2 + SS + 2 + S + 2 + L + 3 + LL <= 1V;
               Ratio0 = S * 0x40 / SS
               Ratio1 = L * 0x40 / S
               Ratio2 = LL * 0x40 / L */
            *pu32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 30 - pstSnsState->au32WDRIntTime[0] - pstSnsState->au32WDRIntTime[1] - pstSnsState->au32WDRIntTime[2]) << 18)  \
                                 / (DIV_0_TO_1(au32Ratio[0]) * DIV_0_TO_1(au32Ratio[1]) * DIV_0_TO_1(au32Ratio[2]));
            u32IntTimeMaxTmp1 = ((pstSnsState->au32FL[0] - 30) << 18) / (au32Ratio[0] * au32Ratio[1] * au32Ratio[2] + \
                                au32Ratio[0] * au32Ratio[1] * 0x40 + au32Ratio[0] * 0x40 * 0x40 + 0x40 * 0x40 * 0x40);
            *pu32IntTimeMaxTmp = (*pu32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? *pu32IntTimeMaxTmp : u32IntTimeMaxTmp1;
        }
    }

    return 1;
}

static HI_VOID cmos_get_inttime_max(VI_PIPE ViPipe, HI_U16 u16ManRatioEnable, HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 i = 0;
    HI_S32 s32Ret;
    HI_U32 u32IntTimeMaxTmp = 0;
    HI_U32 u32RatioTmp    = 0x40;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMax);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMin);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    s32Ret = cmos_get_inttime_max_temp(&u32IntTimeMaxTmp, ViPipe, au32Ratio, au32IntTimeMax, au32IntTimeMin, pu32LFMaxIntTime);
    if (2 == s32Ret)
    {
        return;
    }

    if (u32IntTimeMaxTmp >= 2)
    {
        if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = (pstSnsState->au32FL[0] - 2);
            au32IntTimeMin[0] = 2;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
        }
        else if ((WDR_MODE_3To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_FRAME == pstSnsState->enWDRMode))
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
            au32IntTimeMax[2] = (pstSnsState->au32FL[0] - 2);
            au32IntTimeMin[0] = 2;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
            au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
        }
        else if ((WDR_MODE_4To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_FRAME == pstSnsState->enWDRMode))
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
            au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
            au32IntTimeMax[3] = (pstSnsState->au32FL[0] - 2);
            au32IntTimeMin[0] = 2;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
            au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
            au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
        }
        else if (IS_LINE_WDR_MODE(pstSnsState->enWDRMode))
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
            au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
            au32IntTimeMax[3] = au32IntTimeMax[2] * au32Ratio[2] >> 6;
            au32IntTimeMin[0] = 2;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
            au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
            au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
        }
    }
    else
    {
        if (1 == u16ManRatioEnable)
        {
            ISP_TRACE(HI_DBG_ERR, "Manaul ExpRatio is too large!\n");
            return;
        }
        else
        {
            u32IntTimeMaxTmp = 2;

            if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp * i >> 6) > (pstSnsState->au32FL[0] - 2))
                    {
                        //u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = (pstSnsState->au32FL[0] - 2);
            }
            else if ((WDR_MODE_3To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_FRAME == pstSnsState->enWDRMode))
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp * i * i >> 12) > (pstSnsState->au32FL[0] - 2))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
                au32IntTimeMax[2] = (pstSnsState->au32FL[0] - 2);
            }
            else if ((WDR_MODE_4To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_FRAME == pstSnsState->enWDRMode))
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if (((HI_U64)u32IntTimeMaxTmp * i * i * i >> 18) > (pstSnsState->au32FL[0] - 2))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
                au32IntTimeMax[2] = au32IntTimeMax[1] * u32RatioTmp >> 6;
                au32IntTimeMax[3] = (pstSnsState->au32FL[0] - 2);
            }
            else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp + (u32IntTimeMaxTmp * i >> 6)) > (pstSnsState->au32FL[0] - 30))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
            }
            else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp + (u32IntTimeMaxTmp * i >> 6) + (u32IntTimeMaxTmp * i * i >> 12)) > (pstSnsState->au32FL[0] - 30))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
                au32IntTimeMax[2] = au32IntTimeMax[1] * u32RatioTmp >> 6;
            }
            else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp + (u32IntTimeMaxTmp * i >> 6) + (u32IntTimeMaxTmp * i * i >> 12) + ((HI_U64)u32IntTimeMaxTmp * i * i * i >> 18)) \
                        > (pstSnsState->au32FL[0] - 30))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }

                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
                au32IntTimeMax[2] = au32IntTimeMax[1] * u32RatioTmp >> 6;
                au32IntTimeMax[3] = au32IntTimeMax[2] * u32RatioTmp >> 6;
            }

            au32IntTimeMin[0] = au32IntTimeMax[0];
            au32IntTimeMin[1] = au32IntTimeMax[1];
            au32IntTimeMin[2] = au32IntTimeMax[2];
            au32IntTimeMin[3] = au32IntTimeMax[3];
        }
    }

    return;
}

/* Only used in Line_WDR mode */
static HI_VOID cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
    CMOS_CHECK_POINTER_VOID(pstAeFSWDRAttr);

    genFSWDRMode[ViPipe] = pstAeFSWDRAttr->enFSWDRMode;
    gu32MaxTimeGetCnt[ViPipe] = 0;
}

static HI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
    pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
    pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

    return HI_SUCCESS;
}

static HI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAwbSnsDft);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x1F0;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1A0;

    pstAwbSnsDft->as32WbPara[0] = 41;
    pstAwbSnsDft->as32WbPara[1] = 127;
    pstAwbSnsDft->as32WbPara[2] = -87;
    pstAwbSnsDft->as32WbPara[3] = 174535;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -123490;


    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
            break;

        case WDR_MODE_2To1_LINE:
        case WDR_MODE_3To1_LINE:
        case WDR_MODE_4To1_LINE:
        case WDR_MODE_2To1_FRAME:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcmFsWdr, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

    return HI_SUCCESS;
}

static ISP_CMOS_DNG_COLORPARAM_S g_stDngColorParam =
{
    {378, 256, 430},
    {439, 256, 439}
};

static  HI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstDef);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
#ifdef CONFIG_HI_ISP_CA_SUPPORT
    pstDef->unKey.bit1Ca      = 1;
    pstDef->pstCa             = &g_stIspCA;
#endif
    pstDef->unKey.bit1Clut    = 1;
    pstDef->pstClut           = &g_stIspCLUT;

    pstDef->unKey.bit1Dpc     = 1;
    pstDef->pstDpc            = &g_stCmosDpc;

    pstDef->unKey.bit1Wdr     = 1;
    pstDef->pstWdr            = &g_stIspWDR;

#ifdef CONFIG_HI_ISP_HLC_SUPPORT
    pstDef->unKey.bit1Hlc      = 0;
    pstDef->pstHlc             = &g_stIspHlc;
#endif
    pstDef->unKey.bit1Lsc      = 0;
    pstDef->pstLsc             = &g_stCmosLsc;

#ifdef CONFIG_HI_ISP_EDGEMARK_SUPPORT
    pstDef->unKey.bit1EdgeMark = 0;
    pstDef->pstEdgeMark        = &g_stIspEdgeMark;
#endif
#ifdef CONFIG_HI_ISP_PREGAMMA_SUPPORT
    pstDef->unKey.bit1PreGamma = 0;
    pstDef->pstPreGamma        = &g_stPreGamma;
#endif

    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
            pstDef->unKey.bit1Demosaic       = 1;
            pstDef->pstDemosaic              = &g_stIspDemosaic;
            pstDef->unKey.bit1Sharpen        = 1;
            pstDef->pstSharpen               = &g_stIspYuvSharpen;
            pstDef->unKey.bit1Drc            = 1;
            pstDef->pstDrc                   = &g_stIspDRC;
            pstDef->unKey.bit1BayerNr        = 1;
            pstDef->pstBayerNr               = &g_stIspBayerNr;
            pstDef->unKey.bit1AntiFalseColor = 1;
            pstDef->pstAntiFalseColor        = &g_stIspAntiFalseColor;
            pstDef->unKey.bit1Ldci           = 1;
            pstDef->pstLdci                  = &g_stIspLdci;
            pstDef->unKey.bit1Gamma          = 1;
            pstDef->pstGamma                 = &g_stIspGamma;
            pstDef->unKey.bit1Detail         = 1;
            pstDef->pstDetail                = &g_stIspDetail;
#ifdef CONFIG_HI_ISP_CR_SUPPORT
            pstDef->unKey.bit1Ge             = 1;
            pstDef->pstGe                    = &g_stIspGe;
#endif
            pstDef->unKey.bit1Dehaze         = 1;
            pstDef->pstDehaze                = &g_stIspDehaze;
            memcpy(&pstDef->stNoiseCalibration,   &g_stIspNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;

        case WDR_MODE_BUILT_IN:
        case WDR_MODE_2To1_LINE:
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
        case WDR_MODE_3To1_LINE:
        case WDR_MODE_3To1_FRAME:
        case WDR_MODE_3To1_FRAME_FULL_RATE:
        case WDR_MODE_4To1_LINE:
        case WDR_MODE_4To1_FRAME:
        case WDR_MODE_4To1_FRAME_FULL_RATE:
            pstDef->unKey.bit1Demosaic       = 1;
            pstDef->pstDemosaic              = &g_stIspDemosaicWdr;
            pstDef->unKey.bit1Sharpen        = 1;
            pstDef->pstSharpen               = &g_stIspYuvSharpenWdr;
            pstDef->unKey.bit1Drc            = 1;
            pstDef->pstDrc                   = &g_stIspDRCWDR;
            pstDef->unKey.bit1Gamma          = 1;
            pstDef->pstGamma                 = &g_stIspGammaFSWDR;
#ifdef CONFIG_HI_ISP_PREGAMMA_SUPPORT
            pstDef->unKey.bit1PreGamma       = 0;
            pstDef->pstPreGamma              = &g_stPreGamma;
#endif
            pstDef->unKey.bit1BayerNr        = 1;
            pstDef->pstBayerNr               = &g_stIspBayerNrWdr2To1;
            pstDef->unKey.bit1Detail         = 1;
            pstDef->pstDetail                = &g_stIspDetailWdr2To1;
#ifdef CONFIG_HI_ISP_CR_SUPPORT
            pstDef->unKey.bit1Ge             = 1;
            pstDef->pstGe                    = &g_stIspWdrGe;
#endif
            pstDef->unKey.bit1AntiFalseColor = 1;
            pstDef->pstAntiFalseColor        = &g_stIspWdrAntiFalseColor;
            pstDef->unKey.bit1Ldci           = 1;
            pstDef->pstLdci                  = &g_stIspWdrLdci;
            pstDef->unKey.bit1Dehaze         = 1;
            pstDef->pstDehaze                = &g_stIspDehazeWDR;
            memcpy(&pstDef->stNoiseCalibration, &g_stIspNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;
    }

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x40;
        pstDef->stWdrSwitchAttr.au32ExpRatio[1] = 0x40;
        pstDef->stWdrSwitchAttr.au32ExpRatio[2] = 0x40;
    }
    else if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x200;
        pstDef->stWdrSwitchAttr.au32ExpRatio[1] = 0x40;
        pstDef->stWdrSwitchAttr.au32ExpRatio[2] = 0x40;
    }

    pstDef->stSensorMode.u32SensorID = MN34220_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;

    memcpy(&pstDef->stDngColorParam, &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

    pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 12;
    pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 4095;

    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Denominator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Numerator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Denominator = 1;
    pstDef->stSensorMode.stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Numerator = 1;
    pstDef->stSensorMode.stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimRows = 2;
    pstDef->stSensorMode.stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimCols = 2;
    pstDef->stSensorMode.stDngRawFormat.stBlcRepeatDim.u16BlcRepeatRows = 2;
    pstDef->stSensorMode.stDngRawFormat.stBlcRepeatDim.u16BlcRepeatCols = 2;
    pstDef->stSensorMode.stDngRawFormat.enCfaLayout = CFALAYOUT_TYPE_RECTANGULAR;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[0] = 0;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[1] = 1;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPlaneColor[2] = 2;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[0] = 0;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[1] = 1;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[2] = 1;
    pstDef->stSensorMode.stDngRawFormat.au8CfaPattern[3] = 2;
    pstDef->stSensorMode.bValidDngRawFormat = HI_TRUE;

    return HI_SUCCESS;
}

static HI_S32 cmos_get_isp_black_level(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstBlackLevel);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    for (i = 0; i < 4; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = 0x100;
    }

    return HI_SUCCESS;

}

static HI_VOID cmos_set_pixel_detect(VI_PIPE ViPipe, HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps, u32MaxIntTime_5Fps;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        return;
    }
    else
    {
        if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (MN34220_VMAX_1080P30_LINEAR * 30) / 5;
        }
        else if (MN34220_1080P_60FPS_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (MN34220_VMAX_1080P60_LINEAR * 60) / 5;
        }
        else if (MN34220_720P_120FPS_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (MN34220_VMAX_720P120_LINEAR * 120) / 5;
        }
        else if (MN34220_VGA_240FPS_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (MN34220_VMAX_VGA240_LINEAR * 240) / 5;
        }
        else
        {
            return;
        }
    }


    u32MaxIntTime_5Fps = u32FullLines_5Fps - 2;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        mn34220_write_register(ViPipe, MN34220_VMAX_ADDR_H, (u32FullLines_5Fps & 0xFF00) >> 8);  /* 5fps */
        mn34220_write_register(ViPipe, MN34220_VMAX_ADDR_L, u32FullLines_5Fps & 0xFF);           /* 5fps */
        mn34220_write_register(ViPipe, 0x0202, (u32MaxIntTime_5Fps & 0xFF00) >> 8);      /* max exposure lines */
        mn34220_write_register(ViPipe, 0x0203, u32MaxIntTime_5Fps & 0xFF);               /* max exposure lines */
        mn34220_write_register(ViPipe, 0x0204, 0x01);                                    /* min AG */
        mn34220_write_register(ViPipe, 0x0205, 0x00);                                    /* min AG */
        mn34220_write_register(ViPipe, 0x3108, 0x01);                                    /* min DG */
        mn34220_write_register(ViPipe, 0x3109, 0x00);                                    /* min DG */
    }
    else /* setup for ISP 'normal mode' */
    {
        mn34220_write_register(ViPipe, MN34220_VMAX_ADDR_H, (pstSnsState->u32FLStd & 0xFF00) >> 8);
        mn34220_write_register(ViPipe, MN34220_VMAX_ADDR_L, pstSnsState->u32FLStd & 0xFF);
        pstSnsState->bSyncInit = HI_FALSE;
    }

    return;
}

static HI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    switch (u8Mode& 0x3F)
    {
        case WDR_MODE_NONE:
            if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P30_LINEAR;
            }
            else if (MN34220_1080P_60FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P60_LINEAR;
            }
            else if (MN34220_720P_120FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_720P120_LINEAR;
            }
            else if (MN34220_VGA_240FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_VGA240_LINEAR;
            }

            pstSnsState->enWDRMode = WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        case WDR_MODE_2To1_LINE:
            pstSnsState->u32FLStd = MN34220_VMAX_1080P60TO30_WDR;
            pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
            printf("2to1 line WDR mode\n");
            break;

        case WDR_MODE_2To1_FRAME:
            if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P30_LINEAR;
            }
            else if (MN34220_1080P_60FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P60_LINEAR;
            }

            pstSnsState->enWDRMode = WDR_MODE_2To1_FRAME;
            printf("2to1 half-rate frame WDR mode\n");
            break;

        case WDR_MODE_2To1_FRAME_FULL_RATE:
            if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P30_LINEAR;
            }
            else if (MN34220_1080P_60FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P60_LINEAR;
            }

            pstSnsState->enWDRMode = WDR_MODE_2To1_FRAME_FULL_RATE;
            printf("2to1 full-rate frame WDR mode\n");
            break;

        case WDR_MODE_3To1_LINE:
            if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P90TO30_WDR;
            }
            else
            {
                /* for FPGA */
                pstSnsState->u32FLStd = MN34220_VMAX_720P_WDR;
            }

            pstSnsState->enWDRMode = WDR_MODE_3To1_LINE;
            printf("3to1 line WDR mode\n");
            break;

        case WDR_MODE_4To1_LINE:
            if (MN34220_1080P_30FPS_MODE == pstSnsState->u8ImgMode)
            {
                pstSnsState->u32FLStd = MN34220_VMAX_1080P120TO30_WDR;
            }
            else
            {
                /* for FPGA */
                pstSnsState->u32FLStd = MN34220_VMAX_720P_WDR;
            }

            pstSnsState->enWDRMode = WDR_MODE_4To1_LINE;
            printf("4to1 line WDR mode\n");
            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "NOT support this mode!\n");
            return HI_FAILURE;
    }

    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
    memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSnsRegsInfo);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((HI_FALSE == pstSnsState->bSyncInit) || (HI_FALSE == pstSnsRegsInfo->bConfig))
    {
        pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
        pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunMn34220BusInfo[ViPipe].s8I2cDev;
        pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;

        pstSnsState->astRegsInfo[0].u32RegNum = 9;

        if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 2;
        }
        else if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 2;
        }
        else if ((WDR_MODE_3To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_FRAME == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 4;
        }
        else if ((WDR_MODE_4To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_FRAME == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 6;
        }
        else
        {
        }

        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = mn34220_i2c_addr;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = mn34220_addr_byte;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = mn34220_data_byte;
        }

        /* Shutter (Shutter Long) */
        pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr = MN34220_EXPOSURE_ADDR_H;
        pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr = MN34220_EXPOSURE_ADDR_L;

        /* OB correction filter coefficient */
        pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr = 0x3280;

        /* AG */
        pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr = MN34220_AGC_ADDR_H;
        pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr = MN34220_AGC_ADDR_L;

        /* DG */
        pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = MN34220_DGC_ADDR_H;
        pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = MN34220_DGC_ADDR_L;

        /* VMAX */
        pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = MN34220_VMAX_ADDR_H;
        pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = MN34220_VMAX_ADDR_L;

        if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_L;

            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = MN34220_VMAX_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = MN34220_VMAX_ADDR_L;
        }
        else if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = MN34220_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = MN34220_EXPOSURE_ADDR_L;

            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = MN34220_VMAX_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = MN34220_VMAX_ADDR_L;
        }
        else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_L;
            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = MN34220_SHORT_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = MN34220_SHORT_EXPOSURE_ADDR_L;

            pstSnsState->astRegsInfo[0].astI2cData[11].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[11].u32RegAddr = MN34220_VMAX_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[12].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[12].u32RegAddr = MN34220_VMAX_ADDR_L;
        }
        else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = MN34220_LONG_EXPOSURE_ADDR_L;
            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = MN34220_SHORT_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = MN34220_SHORT_EXPOSURE_ADDR_L;
            pstSnsState->astRegsInfo[0].astI2cData[11].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[11].u32RegAddr = MN34220_SHORT_SHORT_EXPOSURE_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[12].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[12].u32RegAddr = MN34220_SHORT_SHORT_EXPOSURE_ADDR_L;

            pstSnsState->astRegsInfo[0].astI2cData[13].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[13].u32RegAddr = MN34220_VMAX_ADDR_H;
            pstSnsState->astRegsInfo[0].astI2cData[14].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[14].u32RegAddr = MN34220_VMAX_ADDR_L;
        }
        else
        {
        }

        pstSnsState->bSyncInit = HI_TRUE;
    }
    else
    {
        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            if (pstSnsState->astRegsInfo[0].astI2cData[i].u32Data == pstSnsState->astRegsInfo[1].astI2cData[i].u32Data)
            {
                pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_FALSE;
            }
            else
            {
                pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            }
        }

        if ((WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].astI2cData[0].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[1].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[7].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[8].bUpdate = HI_TRUE;
        }
    }

    pstSnsRegsInfo->bConfig = HI_FALSE;
    memcpy(pstSnsRegsInfo, &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    memcpy(&pstSnsState->astRegsInfo[1], &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));

    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode_VGA(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode, HI_U8 *pu8SensorImageMode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    CMOS_CHECK_POINTER(pu8SensorImageMode);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        if (pstSensorImageMode->f32Fps <= 240)
        {
            *pu8SensorImageMode = MN34220_VGA_240FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                  pstSensorImageMode->u16Width,
                  pstSensorImageMode->u16Height,
                  pstSensorImageMode->f32Fps,
                  pstSnsState->enWDRMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode_720P(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode, HI_U8 *pu8SensorImageMode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    CMOS_CHECK_POINTER(pu8SensorImageMode);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        if (pstSensorImageMode->f32Fps <= 120)
        {
            *pu8SensorImageMode = MN34220_720P_120FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }

        /* for FPGA */
        if (pstSensorImageMode->f32Fps <= 30)
        {
            *pu8SensorImageMode = MN34220_720P_30FPS_MODE;
        }

    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)   /* for FPGA */
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            *pu8SensorImageMode = MN34220_720P_30FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->u8ImgMode);
            return HI_FAILURE;
        }
    }
    else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)   /* for FPGA */
    {
        if (pstSensorImageMode->f32Fps <= 20)
        {
            *pu8SensorImageMode = MN34220_720P_30FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }
    else if (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode)   /* for FPGA */
    {
        if (pstSensorImageMode->f32Fps <= 15)
        {
            *pu8SensorImageMode = MN34220_720P_30FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                  pstSensorImageMode->u16Width,
                  pstSensorImageMode->u16Height,
                  pstSensorImageMode->f32Fps,
                  pstSnsState->enWDRMode);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode_1080P(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode, HI_U8 *pu8SensorImageMode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    CMOS_CHECK_POINTER(pu8SensorImageMode);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_4To1_LINE == pstSnsState->enWDRMode))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            *pu8SensorImageMode = MN34220_1080P_30FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }
    else
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            *pu8SensorImageMode = MN34220_1080P_30FPS_MODE;
        }
        else if (pstSensorImageMode->f32Fps <= 60)
        {
            *pu8SensorImageMode = MN34220_1080P_60FPS_MODE;
        }
        else
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8  u8SensorImageMode = 0;
    HI_S32 s32Ret;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    u8SensorImageMode = pstSnsState->u8ImgMode;
    pstSnsState->bSyncInit = HI_FALSE;

    if (MN34220_RES_IS_VGA(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
    {
        s32Ret = cmos_set_image_mode_VGA(ViPipe, pstSensorImageMode, &u8SensorImageMode);
        if (HI_FAILURE == s32Ret)
        {
            return HI_FAILURE;
        }
    }
    else if (MN34220_RES_IS_720P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
    {
        s32Ret = cmos_set_image_mode_720P(ViPipe, pstSensorImageMode, &u8SensorImageMode);
        if (HI_FAILURE == s32Ret)
        {
            return HI_FAILURE;
        }
    }
    else if (MN34220_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
    {
        s32Ret = cmos_set_image_mode_1080P(ViPipe, pstSensorImageMode, &u8SensorImageMode);
        if (HI_FAILURE == s32Ret)
        {
            return HI_FAILURE;
        }
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                  pstSensorImageMode->u16Width,
                  pstSensorImageMode->u16Height,
                  pstSensorImageMode->f32Fps,
                  pstSnsState->enWDRMode);
        return HI_FAILURE;
    }

    /* Sensor first init */
    if (HI_FALSE == pstSnsState->bInit)
    {
        pstSnsState->u8ImgMode = u8SensorImageMode;
        return HI_SUCCESS;
    }

    /* Switch SensorImageMode */
    if (u8SensorImageMode == pstSnsState->u8ImgMode)
    {
        /* Don't need to switch SensorImageMode */
        return ISP_DO_NOT_NEED_SWITCH_IMAGEMODE;
    }

    pstSnsState->u8ImgMode = u8SensorImageMode;
    memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    MN34220_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = MN34220_1080P_30FPS_MODE;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd = MN34220_VMAX_1080P30_LINEAR;
    pstSnsState->au32FL[0] = MN34220_VMAX_1080P30_LINEAR;
    pstSnsState->au32FL[1] = MN34220_VMAX_1080P30_LINEAR;

    memset(&pstSnsState->astRegsInfo[0], 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&pstSnsState->astRegsInfo[1], 0, sizeof(ISP_SNS_REGS_INFO_S));
}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = mn34220_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit = mn34220_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = cmos_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

    return HI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

static HI_S32 mn34220_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunMn34220BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    MN34220_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

    if (HI_NULL == pastSnsStateCtx)
    {
        pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
        if (HI_NULL == pastSnsStateCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

    MN34220_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    MN34220_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    MN34220_SENSOR_RESET_CTX(ViPipe);
}

static HI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
    HI_S32 s32Ret;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;
    ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

    CMOS_CHECK_POINTER(pstAeLib);
    CMOS_CHECK_POINTER(pstAwbLib);

    s32Ret = sensor_ctx_init(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    stSnsAttrInfo.eSensorId = MN34220_ID;

    s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret |= HI_MPI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor register callback function failed!\n");
        return s32Ret;
    }

    s32Ret  = cmos_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret |= HI_MPI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret  = cmos_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret |= HI_MPI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
    HI_S32 s32Ret;

    CMOS_CHECK_POINTER(pstAeLib);
    CMOS_CHECK_POINTER(pstAwbLib);

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(ViPipe, MN34220_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, MN34220_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, MN34220_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }

    sensor_ctx_exit(ViPipe);

    return HI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsMn34220Obj =
{
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = mn34220_standby,
    .pfnRestart             = mn34220_restart,
    .pfnMirrorFlip          = HI_NULL,
    .pfnWriteReg            = mn34220_write_register,
    .pfnReadReg             = mn34220_read_register,
    .pfnSetBusInfo          = mn34220_set_bus_info,
    .pfnSetInit             = HI_NULL
};


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MN34220_CMOS_H_ */
