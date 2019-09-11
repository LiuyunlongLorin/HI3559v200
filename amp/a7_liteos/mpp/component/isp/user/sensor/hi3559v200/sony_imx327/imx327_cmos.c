
/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : imx327_cmos.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2016/06/28
  Description   :
  History       :
  1.Date        : 2016/06/28
    Author      :
    Modification: Created file

******************************************************************************/
#if !defined(__IMX327_CMOS_H_)
#define __IMX327_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"

#include "imx327_cmos_ex.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define IMX327_ID 327
#define SENSOR_IMX327_WIDTH 1920
#define SENSOR_IMX327_HEIGHT 1080
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastImx327[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define IMX327_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastImx327[dev])
#define IMX327_SENSOR_SET_CTX(dev, pstCtx)   (g_pastImx327[dev] = pstCtx)
#define IMX327_SENSOR_RESET_CTX(dev)         (g_pastImx327[dev] = HI_NULL)

ISP_SNS_COMMBUS_U g_aunImx327BusInfo[ISP_MAX_PIPE_NUM] =
{
    [0] = { .s8I2cDev = 0},
    [1 ... ISP_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

static ISP_FSWDR_MODE_E genFSWDRMode[ISP_MAX_PIPE_NUM] =
{
    [0 ... ISP_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

static HI_U32 gu32MaxTimeGetCnt[ISP_MAX_PIPE_NUM] = {0};
static HI_U32 g_au32InitExposure[ISP_MAX_PIPE_NUM]  = {0};
static HI_U32 g_au32LinesPer500ms[ISP_MAX_PIPE_NUM] = {0};

static HI_U16 g_au16InitWBGain[ISP_MAX_PIPE_NUM][3] = {{0}};
static HI_U16 g_au16SampleRgain[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16SampleBgain[ISP_MAX_PIPE_NUM] = {0};

extern const unsigned int imx327_i2c_addr;
extern unsigned int imx327_addr_byte;
extern unsigned int imx327_data_byte;

typedef struct hiIMX327_STATE_S
{
    HI_U8       u8Hcg;
    HI_U32      u32BRL;
    HI_U32      u32RHS1_MAX;
    HI_U32      u32RHS2_MAX;
} IMX327_STATE_S;

IMX327_STATE_S g_astimx327State[ISP_MAX_PIPE_NUM] = {{0}};

extern void imx327_init(VI_PIPE ViPipe);
extern void imx327_exit(VI_PIPE ViPipe);
extern void imx327_standby(VI_PIPE ViPipe);
extern void imx327_restart(VI_PIPE ViPipe);
extern int  imx327_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx327_read_register(VI_PIPE ViPipe, int addr);

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
#define IMX327_FULL_LINES_MAX  (0x3FFFF)
#define IMX327_FULL_LINES_MAX_2TO1_WDR  (0x8AA)    // considering the YOUT_SIZE and bad frame

/*****Imx327 Register Address*****/
#define IMX327_SHS1_ADDR (0x3020)
#define IMX327_SHS2_ADDR (0x3024)
#define IMX327_GAIN_ADDR (0x3014)
#define IMX327_HCG_ADDR  (0x3009)
#define IMX327_VMAX_ADDR (0x3018)
#define IMX327_HMAX_ADDR (0x301c)
#define IMX327_RHS1_ADDR (0x3030)
#define IMX327_RHS2_ADDR (0x3034)
#define IMX327_Y_OUT_SIZE_ADDR (0x3418)

#define IMX327_INCREASE_LINES (1) /* make real fps less than stand fps because NVR require*/

#define IMX327_VMAX_1080P30_LINEAR  (1125+IMX327_INCREASE_LINES)
//#define IMX327_VMAX_1080P60TO30_WDR (1156+IMX327_INCREASE_LINES)  //12bit
#define IMX327_VMAX_1080P60TO30_WDR (1220+IMX327_INCREASE_LINES)   //10bit

//sensor fps mode
#define IMX327_SENSOR_1080P_30FPS_LINEAR_MODE         (1)
#define IMX327_SENSOR_1080P_30FPS_WDR_2to1_MODE       (2)
#define IMX327_SENSOR_1080P_30FPS_FRAME_WDR_2to1_MODE (3)

#define IMX327_RES_IS_1080P(w, h)      ((w) <= 1920 && (h) <= 1080)

#define IMX327_ERR_MODE_PRINT(pstSensorImageMode, pstSnsState)\
    do{\
        ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",\
                  pstSensorImageMode->u16Width,  \
                  pstSensorImageMode->u16Height, \
                  pstSensorImageMode->f32Fps,    \
                  pstSnsState->enWDRMode);\
    }while(0)

static HI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));

    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq = 50 * 256;
    pstAeSnsDft->u32FullLinesMax = IMX327_FULL_LINES_MAX;
    pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

    if (g_au32LinesPer500ms[ViPipe] == 0)
    {
        pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 30 / 2;
    }
    else
    {
        pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
    }

    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_0;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_32_0;

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

            pstAeSnsDft->u32MaxAgain = 62416;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 38485;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->u8AeCompensation = 0x38;
            pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
            pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 1;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 1;
            break;

        case WDR_MODE_2To1_FRAME:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x18;
            pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 1;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

            pstAeSnsDft->u32MaxAgain = 62416;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 38485;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->u16ManRatioEnable = HI_FALSE;
            pstAeSnsDft->au32Ratio[0] = 0x400;

            break;
        case WDR_MODE_2To1_LINE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;

            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

            pstAeSnsDft->u32MaxAgain = 62416;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 38485;
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
            pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;

            pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 52000;

            if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
            {
                pstAeSnsDft->u8AeCompensation = 64;
                pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
            }
            else
            {
                pstAeSnsDft->u8AeCompensation = 32;
                pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;
                pstAeSnsDft->u16ManRatioEnable = HI_TRUE;
                pstAeSnsDft->au32Ratio[0] = 0x400;
                pstAeSnsDft->au32Ratio[1] = 0x40;
                pstAeSnsDft->au32Ratio[2] = 0x40;
            }
            break;
    }

    return HI_SUCCESS;
}

/* the function of sensor set fps */
static HI_VOID cmos_fps_set(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32VMAX = IMX327_VMAX_1080P30_LINEAR;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    switch (pstSnsState->u8ImgMode)
    {
        case IMX327_SENSOR_1080P_30FPS_WDR_2to1_MODE:
            if ((f32Fps <= 30) && (f32Fps >= 16.51))
            {
                u32VMAX = IMX327_VMAX_1080P60TO30_WDR * 30 / DIV_0_TO_1_FLOAT(f32Fps);
            }
            else
            {
                ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
                return;
            }
            u32VMAX = (u32VMAX > IMX327_FULL_LINES_MAX_2TO1_WDR) ? IMX327_FULL_LINES_MAX_2TO1_WDR : u32VMAX;
            break;

        case IMX327_SENSOR_1080P_30FPS_LINEAR_MODE:
        case IMX327_SENSOR_1080P_30FPS_FRAME_WDR_2to1_MODE:
            if ((f32Fps <= 30) && (f32Fps >= 0.119))
            {
                u32VMAX = IMX327_VMAX_1080P30_LINEAR * 30 / DIV_0_TO_1_FLOAT(f32Fps);
            }
            else
            {
                ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
                return;
            }
            u32VMAX = (u32VMAX > IMX327_FULL_LINES_MAX) ? IMX327_FULL_LINES_MAX : u32VMAX;
            break;
        default:
            return;
    }

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (u32VMAX & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = ((u32VMAX & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32VMAX & 0xF0000) >> 16);
    }
    else
    {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32VMAX & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((u32VMAX & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = ((u32VMAX & 0xF0000) >> 16);
    }

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->u32FLStd = u32VMAX * 2;
        /*
            RHS1 limitation:
            2n + 5
            RHS1 <= FSC - BRL*2 -21
            (2 * VMAX_IMX290_1080P30_WDR - 2 * gu32BRL - 21) - (((2 * VMAX_IMX290_1080P30_WDR - 2 * 1109 - 21) - 5) %2)
        */

        g_astimx327State[ViPipe].u32RHS1_MAX = (u32VMAX - g_astimx327State[ViPipe].u32BRL) * 2 - 21;
    }
    else
    {
        pstSnsState->u32FLStd = u32VMAX;
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 2;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

    return;
}

static HI_VOID cmos_slow_framerate_set(VI_PIPE ViPipe, HI_U32 u32FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        u32FullLines = (u32FullLines > 2 * IMX327_FULL_LINES_MAX_2TO1_WDR) ? 2 * IMX327_FULL_LINES_MAX_2TO1_WDR : u32FullLines;
        pstSnsState->au32FL[0] = (u32FullLines >> 1) << 1;
        g_astimx327State[ViPipe].u32RHS1_MAX = pstSnsState->au32FL[0] - g_astimx327State[ViPipe].u32BRL * 2 - 21;
    }
    else if ( WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        u32FullLines = (u32FullLines > IMX327_FULL_LINES_MAX) ? IMX327_FULL_LINES_MAX : u32FullLines;
        pstSnsState->au32FL[0] = u32FullLines;
    }
    else
    {
        u32FullLines = (u32FullLines > IMX327_FULL_LINES_MAX) ? IMX327_FULL_LINES_MAX : u32FullLines;
        pstSnsState->au32FL[0] = u32FullLines;
    }

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (pstSnsState->au32FL[0] & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((pstSnsState->au32FL[0] & 0xF0000) >> 16);
    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = ((pstSnsState->au32FL[0] >> 1) & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = (((pstSnsState->au32FL[0] >> 1) & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (((pstSnsState->au32FL[0] >> 1) & 0xF0000) >> 16);
    }
    else if ( WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (pstSnsState->au32FL[0] & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = ((pstSnsState->au32FL[0] & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = (((pstSnsState->au32FL[0] & 0x30000)) >> 16);
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
    static HI_BOOL bFirst[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 1};
    HI_U32 u32Value = 0;

    static HI_U32 u32ShortIntTime[ISP_MAX_PIPE_NUM] = {0};
    static HI_U32 u32LongIntTime[ISP_MAX_PIPE_NUM] = {0};

    static HI_U32 u32RHS1[ISP_MAX_PIPE_NUM]  = {0};

    static HI_U32 u32SHS1[ISP_MAX_PIPE_NUM]  = {0};
    static HI_U32 u32SHS2[ISP_MAX_PIPE_NUM]  = {0};

    HI_U32 u32YOUTSIZE;

    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        //printf("bFirst = %d\n",bFirst[ViPipe]);
        if (bFirst[ViPipe]) /* short exposure */
        {
            pstSnsState->au32WDRIntTime[0] = u32IntTime;
            u32ShortIntTime[ViPipe] = u32IntTime;
            bFirst[ViPipe] = HI_FALSE;
        }
        else   /* long exposure */
        {
            pstSnsState->au32WDRIntTime[1] = u32IntTime;
            u32LongIntTime[ViPipe] = u32IntTime;

            u32SHS2[ViPipe] = pstSnsState->au32FL[1] - u32LongIntTime[ViPipe] - 1;

            //allocate the RHS1
            u32SHS1[ViPipe] = (u32ShortIntTime[ViPipe] % 2) + 2;
            u32RHS1[ViPipe] = u32ShortIntTime[ViPipe] + u32SHS1[ViPipe] + 1;

            if (IMX327_SENSOR_1080P_30FPS_WDR_2to1_MODE == pstSnsState->u8ImgMode)
            {
                u32YOUTSIZE = (1097 + (u32RHS1[ViPipe] - 1) / 2 + 7) * 2;
                u32YOUTSIZE = (u32YOUTSIZE >= 0x1FFF) ? 0x1FFF : u32YOUTSIZE;
            }
            else
            {
                u32YOUTSIZE = (1097 + (u32RHS1[ViPipe] - 1) / 2 + 7) * 2;
                u32YOUTSIZE = (u32YOUTSIZE >= 0x1FFF) ? 0x1FFF : u32YOUTSIZE;
            }

            /*printf("u32ShortIntTime = %d u32SHS1 = %d \n",u32ShortIntTime[ViPipe],u32SHS1);
            printf("ViPipe = %d RHS1 = %d u32YOUTSIZE = %d \n",ViPipe,u32RHS1[ViPipe], u32YOUTSIZE);*/

            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = (u32SHS1[ViPipe] & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = ((u32SHS1[ViPipe] & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = ((u32SHS1[ViPipe] & 0xF0000) >> 16);

            pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (u32SHS2[ViPipe] & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = ((u32SHS2[ViPipe] & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32SHS2[ViPipe] & 0xF0000) >> 16);

            pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = (u32RHS1[ViPipe] & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = ((u32RHS1[ViPipe] & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[13].u32Data = ((u32RHS1[ViPipe] & 0xF0000) >> 16);

            pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = (u32YOUTSIZE & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[15].u32Data = ((u32YOUTSIZE & 0x1F00) >> 8);

            bFirst[ViPipe] = HI_TRUE;
        }

    }
    else if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        if (bFirst[ViPipe]) /* short exposure */
        {
            u32Value = pstSnsState->au32FL[0] - u32IntTime - 1;
            pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = (u32Value & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = ((u32Value & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = ((u32Value & 0x30000) >> 16);
            bFirst[ViPipe] = HI_FALSE;
        }
        else    /* long exposure */
        {
            u32Value = pstSnsState->au32FL[0] - u32IntTime - 1;
            pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (u32Value & 0xFF);
            pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = ((u32Value & 0xFF00) >> 8);
            pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = ((u32Value & 0x30000) >> 16);
            bFirst[ViPipe] = HI_TRUE;
        }
    }
    else
    {
        u32Value = pstSnsState->au32FL[0] - u32IntTime - 1;

        pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = (u32Value & 0xFF);
        pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = ((u32Value & 0xFF00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = ((u32Value & 0x30000) >> 16);

        bFirst[ViPipe] = HI_TRUE;
    }

    return;

}

static HI_U32 gain_table[231] =
{
    1024, 1059, 1097, 1135, 1175, 1217, 1259, 1304, 1349, 1397, 1446, 1497, 1549, 1604, 1660, 1719, 1779, 1842, 1906,
    1973, 2043, 2114, 2189, 2266, 2345, 2428, 2513, 2602, 2693, 2788, 2886, 2987, 3092, 3201, 3313, 3430, 3550, 3675,
    3804, 3938, 4076, 4219, 4368, 4521, 4680, 4845, 5015, 5191, 5374, 5562, 5758, 5960, 6170, 6387, 6611, 6843, 7084,
    7333, 7591, 7857, 8134, 8419, 8715, 9022, 9339, 9667, 10007, 10358, 10722, 11099, 11489, 11893, 12311, 12743, 13191,
    13655, 14135, 14631, 15146, 15678, 16229, 16799, 17390, 18001, 18633, 19288, 19966, 20668, 21394, 22146, 22924,
    23730, 24564, 25427, 26320, 27245, 28203, 29194, 30220, 31282, 32381, 33519, 34697, 35917, 37179, 38485, 39838,
    41238, 42687, 44187, 45740, 47347, 49011, 50734, 52517, 54362, 56272, 58250, 60297, 62416, 64610, 66880, 69231,
    71663, 74182, 76789, 79487, 82281, 85172, 88165, 91264, 94471, 97791, 101227, 104785, 108467, 112279, 116225,
    120309, 124537, 128913, 133444, 138133, 142988, 148013, 153214, 158599, 164172, 169941, 175913, 182095, 188495,
    195119, 201976, 209073, 216421, 224026, 231899, 240049, 248485, 257217, 266256, 275613, 285298, 295324, 305703,
    320110, 327567, 339078, 350994, 363329, 376097, 389314, 402995, 417157, 431817, 446992, 462700, 478960, 495792,
    513215, 531251, 549920, 569246, 589250, 609958, 631393, 653581, 676550, 700325, 724936, 750412, 776783, 804081,
    832338, 861588, 891866, 923208, 955652, 989236, 1024000, 1059985, 1097235, 1135795, 1175709, 1217026, 1259795,
    1304067, 1349894, 1397333, 1446438, 1497269, 1549886, 1604353, 1660733, 1719095, 1779508, 1842043, 1906777,
    1913785, 2043148, 2114949, 2189273, 2266208, 2345848, 2428286, 2513621, 2601956, 2662563, 2788046, 2886024
};

static HI_VOID cmos_again_calc_table(VI_PIPE ViPipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);

    if (*pu32AgainLin >= gain_table[230])
    {
        *pu32AgainLin = gain_table[230];
        *pu32AgainDb = 230;
        return ;
    }

    for (i = 1; i < 231; i++)
    {
        if (*pu32AgainLin < gain_table[i])
        {
            *pu32AgainLin = gain_table[i - 1];
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

    if (*pu32DgainLin >= gain_table[140])
    {
        *pu32DgainLin = gain_table[140];
        *pu32DgainDb = 140;
        return ;
    }

    for (i = 1; i < 141; i++)
    {
        if (*pu32DgainLin < gain_table[i])
        {
            *pu32DgainLin = gain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }
    return;
}

/*static HI_U32 gain_table[262] =
{
    1024, 1059, 1097, 1135, 1175, 1217, 1259, 1304, 1349, 1397, 1446, 1497, 1549, 1604, 1660, 1719, 1779, 1842, 1906,
    1973, 2043, 2048, 2119, 2194, 2271, 2351, 2434, 2519, 2608, 2699, 2794, 2892, 2994, 3099, 3208, 3321, 3438, 3559,
    3684, 3813, 3947, 4086, 4229, 4378, 4532, 4691, 4856, 5027, 5203, 5386, 5576, 5772, 5974, 6184, 6402, 6627, 6860,
    7101, 7350, 7609, 7876, 8153, 8439, 8736, 9043, 9361, 9690, 10030, 10383, 10748, 11125, 11516, 11921, 12340, 12774,
    13222, 13687, 14168, 14666, 15182, 15715, 16267, 16839, 17431, 18043, 18677, 19334, 20013, 20717, 21445, 22198,
    22978, 23786, 24622, 25487, 26383, 27310, 28270, 29263, 30292, 31356, 32458, 33599, 34780, 36002, 37267, 38577,
    39932, 41336, 42788, 44292, 45849, 47460, 49128, 50854, 52641, 54491, 56406, 58388, 60440, 62564, 64763, 67039,
    69395, 71833, 74358, 76971, 79676, 82476, 85374, 88375, 91480, 94695, 98023, 101468, 105034, 108725, 112545,
    116501, 120595, 124833, 129220, 133761, 138461, 143327, 148364, 153578, 158975, 164562, 170345, 176331, 182528,
    188942, 195582, 202455, 209570, 216935, 224558, 232450, 240619, 249074, 257827, 266888, 276267, 285976, 296026,
    306429, 317197, 328344, 339883, 351827, 364191, 376990, 390238, 403952, 418147, 432842, 448053, 463799, 480098,
    496969, 514434, 532512, 551226, 570597, 590649, 611406, 632892, 655133, 678156, 701988, 726657, 752194, 778627,
    805990, 834314, 863634, 893984, 925400, 957921, 991585, 1026431, 1062502, 1099841, 1138491, 1178500, 1219916,
    1262786, 1307163, 1353100, 1400651, 1449872, 1500824, 1553566, 1608162, 1664676, 1723177, 1783733, 1846417,
    1911304, 1978472, 2048000, 2119971, 2194471, 2271590, 2351418, 2434052, 2519590, 2608134, 2699789, 2794666,
    2892876, 2994538, 3099773, 3208706, 3321467, 3438190, 3559016, 3684087, 3813554, 3947571, 4086297, 4229898,
    4378546, 4532417, 4691696, 4856573, 5027243, 5203912, 5386788, 5576092, 5772048, 5974890, 6184861, 6402210,
    6627198, 6860092, 7101170, 7350721, 7609041, 7876439, 8153234
};

static HI_VOID cmos_again_calc_table(VI_PIPE ViPipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);

    if (*pu32AgainLin >= gain_table[261])
    {
        *pu32AgainLin = gain_table[261];
        *pu32AgainDb = 261;
        return ;
    }

    for (i = 1; i < 262; i++)
    {
        if (*pu32AgainLin < gain_table[i])
        {
            *pu32AgainLin = gain_table[i - 1];
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

    if (*pu32DgainLin >= gain_table[140])
    {
        *pu32DgainLin = gain_table[140];
        *pu32DgainDb = 140;
        return ;
    }

    for (i = 1; i < 141; i++)
    {
        if (*pu32DgainLin < gain_table[i])
        {
            *pu32DgainLin = gain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }
    return;
}*/

static HI_VOID cmos_gains_update(VI_PIPE ViPipe, HI_U32 u32Again, HI_U32 u32Dgain)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32HCG = g_astimx327State[ViPipe].u8Hcg;
    HI_U32 u32Tmp;

    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (u32Again >= 27)
    {
        u32HCG = u32HCG | 0x10;  // bit[4] HCG  .Reg0x3009[7:0]
        u32Again = u32Again - 27;
    }

    u32Tmp = u32Again + u32Dgain;

    pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = (u32Tmp & 0xFF);
    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = (u32HCG & 0xFF);

    return;
}

static HI_VOID cmos_get_inttime_max(VI_PIPE ViPipe, HI_U16 u16ManRatioEnable, HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 i = 0;
    HI_U32 u32IntTimeMaxTmp0 = 0;
    HI_U32 u32IntTimeMaxTmp  = 0;
    HI_U32 u32RatioTmp = 0x40;
    HI_U32 u32ShortTimeMinLimit = 0;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMax);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMin);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    //u32ShortTimeMinLimit = (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) ? 2 : 2;
    u32ShortTimeMinLimit = 2;

    if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        u32IntTimeMaxTmp = ((pstSnsState->au32FL[0] - 2) << 6) / DIV_0_TO_1(au32Ratio[0]);
    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        /*  limitation for line base WDR

            SHS1 limitation:
            2 or more
            RHS1 - 2 or less

            SHS2 limitation:
            RHS1 + 2 or more
            FSC - 2 or less

            RHS1 Limitation
            2n + 5 (n = 0,1,2...)
            RHS1 <= FSC - BRL * 2 - 21

            short exposure time = RHS1 - (SHS1 + 1) <= RHS1 - 3
            long exposure time = FSC - (SHS2 + 1) <= FSC - (RHS1 + 3)
            ExposureShort + ExposureLong <= FSC - 6
            short exposure time <= (FSC - 6) / (ratio + 1)
        */
        if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
        {
            u32IntTimeMaxTmp0 = pstSnsState->au32FL[1] - 6 - pstSnsState->au32WDRIntTime[0];
            u32IntTimeMaxTmp = pstSnsState->au32FL[0] - 10;
            u32IntTimeMaxTmp = (u32IntTimeMaxTmp0 < u32IntTimeMaxTmp) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMin[0] = u32ShortTimeMinLimit;
            return;
        }
        else
        {
            u32IntTimeMaxTmp0 = ((pstSnsState->au32FL[1] - 6 - pstSnsState->au32WDRIntTime[0]) * 0x40)  / DIV_0_TO_1(au32Ratio[0]);
            u32IntTimeMaxTmp  = ((pstSnsState->au32FL[0] - 6) * 0x40)  / DIV_0_TO_1(au32Ratio[0] + 0x40);
            u32IntTimeMaxTmp  = (u32IntTimeMaxTmp0 < u32IntTimeMaxTmp) ? u32IntTimeMaxTmp0 : u32IntTimeMaxTmp;
            u32IntTimeMaxTmp  = (u32IntTimeMaxTmp > (g_astimx327State[ViPipe].u32RHS1_MAX - 3)) ? (g_astimx327State[ViPipe].u32RHS1_MAX - 3) : u32IntTimeMaxTmp;
            u32IntTimeMaxTmp  = (0 == u32IntTimeMaxTmp) ? 1 : u32IntTimeMaxTmp;
        }

    }
    else
    {
    }

    if (u32IntTimeMaxTmp >= u32ShortTimeMinLimit)
    {
        if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = (pstSnsState->au32FL[0] - 2);
            au32IntTimeMin[0] = 2;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
        }
        if (IS_LINE_WDR_MODE(pstSnsState->enWDRMode))
        {
            au32IntTimeMax[0] = u32IntTimeMaxTmp;
            au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
            au32IntTimeMax[2] = au32IntTimeMax[1] * au32Ratio[1] >> 6;
            au32IntTimeMax[3] = au32IntTimeMax[2] * au32Ratio[2] >> 6;
            au32IntTimeMin[0] = u32ShortTimeMinLimit;
            au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
            au32IntTimeMin[2] = au32IntTimeMin[1] * au32Ratio[1] >> 6;
            au32IntTimeMin[3] = au32IntTimeMin[2] * au32Ratio[2] >> 6;
        }
        else
        {
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
            u32IntTimeMaxTmp = u32ShortTimeMinLimit;

            if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
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
            else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
            {
                u32RatioTmp = 0xFFF;
                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
            }
            else if (WDR_MODE_3To1_LINE == pstSnsState->enWDRMode)
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp + (u32IntTimeMaxTmp * i >> 6) + (u32IntTimeMaxTmp * i * i >> 12)) > (pstSnsState->au32FL[0] - 12))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }
                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
                au32IntTimeMax[2] = au32IntTimeMax[1] * u32RatioTmp >> 6;
            }
            else
            {
            }
            au32IntTimeMin[0] = au32IntTimeMax[0];
            au32IntTimeMin[1] = au32IntTimeMax[1];
            au32IntTimeMin[2] = au32IntTimeMax[2];
            au32IntTimeMin[3] = au32IntTimeMax[3];
        }
    }

    return;
}

/* Only used in LINE_WDR mode */
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

//awb static param for Fuji Lens New IR_Cut
#define CALIBRATE_STATIC_WB_R_GAIN  450
#define CALIBRATE_STATIC_WB_GR_GAIN 256
#define CALIBRATE_STATIC_WB_GB_GAIN 256
#define CALIBRATE_STATIC_WB_B_GAIN  447

/* Calibration results for Auto WB Planck */
#define CALIBRATE_AWB_P1 -53
#define CALIBRATE_AWB_P2 309
#define CALIBRATE_AWB_Q1 0
#define CALIBRATE_AWB_A1 160279
#define CALIBRATE_AWB_B1 128
#define CALIBRATE_AWB_C1 -110119

/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN 0
#define GOLDEN_BGAIN 0
static HI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAwbSnsDft);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    pstAwbSnsDft->u16WbRefTemp = 4950;

#if 0
    pstAwbSnsDft->au16GainOffset[0] = 0x1AE;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1B5;

    pstAwbSnsDft->as32WbPara[0] = -185;
    pstAwbSnsDft->as32WbPara[1] = 639;
    pstAwbSnsDft->as32WbPara[2] = 198;
    pstAwbSnsDft->as32WbPara[3] = 204479;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -149150;
#endif

    pstAwbSnsDft->au16GainOffset[0] = CALIBRATE_STATIC_WB_R_GAIN;
    pstAwbSnsDft->au16GainOffset[1] = CALIBRATE_STATIC_WB_GR_GAIN;
    pstAwbSnsDft->au16GainOffset[2] = CALIBRATE_STATIC_WB_GB_GAIN;
    pstAwbSnsDft->au16GainOffset[3] = CALIBRATE_STATIC_WB_B_GAIN;

    pstAwbSnsDft->as32WbPara[0] = CALIBRATE_AWB_P1;
    pstAwbSnsDft->as32WbPara[1] = CALIBRATE_AWB_P2;
    pstAwbSnsDft->as32WbPara[2] = CALIBRATE_AWB_Q1;
    pstAwbSnsDft->as32WbPara[3] = CALIBRATE_AWB_A1;
    pstAwbSnsDft->as32WbPara[4] = CALIBRATE_AWB_B1;
    pstAwbSnsDft->as32WbPara[5] = CALIBRATE_AWB_C1;

    pstAwbSnsDft->u16GoldenRgain = GOLDEN_RGAIN;
    pstAwbSnsDft->u16GoldenBgain = GOLDEN_BGAIN;

    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
            break;

        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_LINE:
            memcpy(&pstAwbSnsDft->stCcm,    &g_stAwbCcmFsWdr, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));

            break;
    }

    pstAwbSnsDft->u16InitRgain = g_au16InitWBGain[ViPipe][0];
    pstAwbSnsDft->u16InitGgain = g_au16InitWBGain[ViPipe][1];
    pstAwbSnsDft->u16InitBgain = g_au16InitWBGain[ViPipe][2];
    pstAwbSnsDft->u16SampleRgain = g_au16SampleRgain[ViPipe];
    pstAwbSnsDft->u16SampleBgain = g_au16SampleBgain[ViPipe];

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

static HI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstDef);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
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
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_LINE:
            pstDef->unKey.bit1Dpc            = 1;
            pstDef->pstDpc                   = &g_stCmosWdrDpc;
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
        pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x400;
    }
    else if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
    {
        pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x40;
    }

    pstDef->stSensorMode.u32SensorID = IMX327_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;


    memcpy(&pstDef->stDngColorParam, &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

    switch (pstSnsState->u8ImgMode)
    {
        default:
        case IMX327_SENSOR_1080P_30FPS_LINEAR_MODE:
        case IMX327_SENSOR_1080P_30FPS_FRAME_WDR_2to1_MODE:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 12;
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 4095;
            break;

        case IMX327_SENSOR_1080P_30FPS_WDR_2to1_MODE:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 10;
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 1023;
            break;
    }

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
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    /* black level of linear mode */
    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        for (i = 0; i < 4; i++)
        {
            pstBlackLevel->au16BlackLevel[i] = 0xF0;    // 240
        }
    }

    /* black level of DOL mode */
    else
    {
        pstBlackLevel->au16BlackLevel[0] = 0xf0;
        pstBlackLevel->au16BlackLevel[1] = 0xf0;
        pstBlackLevel->au16BlackLevel[2] = 0xf0;
        pstBlackLevel->au16BlackLevel[3] = 0xf0;
    }

    return HI_SUCCESS;
}

static HI_VOID cmos_set_pixel_detect(VI_PIPE ViPipe, HI_BOOL bEnable)
{

    HI_U32 u32FullLines_5Fps, u32MaxIntTime_5Fps;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        return;
    }
    else
    {
        if ((IMX327_SENSOR_1080P_30FPS_LINEAR_MODE == pstSnsState->u8ImgMode) || \
            (IMX327_SENSOR_1080P_30FPS_FRAME_WDR_2to1_MODE == pstSnsState->u8ImgMode))
        {
            u32FullLines_5Fps = (IMX327_VMAX_1080P30_LINEAR * 30) / 5;
        }

        else
        {
            return;
        }
    }

    //u32FullLines_5Fps = (u32FullLines_5Fps > IMX290_FULL_LINES_MAX) ? IMX290_FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxIntTime_5Fps = 4;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        imx327_write_register (ViPipe, IMX327_GAIN_ADDR, 0x00);

        imx327_write_register (ViPipe, IMX327_VMAX_ADDR, u32FullLines_5Fps & 0xFF);
        imx327_write_register (ViPipe, IMX327_VMAX_ADDR + 1, (u32FullLines_5Fps & 0xFF00) >> 8);
        imx327_write_register (ViPipe, IMX327_VMAX_ADDR + 2, (u32FullLines_5Fps & 0xF0000) >> 16);

        imx327_write_register (ViPipe, IMX327_SHS1_ADDR, u32MaxIntTime_5Fps & 0xFF);
        imx327_write_register (ViPipe, IMX327_SHS1_ADDR + 1,  (u32MaxIntTime_5Fps & 0xFF00) >> 8);
        imx327_write_register (ViPipe, IMX327_SHS1_ADDR + 2, (u32MaxIntTime_5Fps & 0xF0000) >> 16);

    }
    else /* setup for ISP 'normal mode' */
    {
        pstSnsState->u32FLStd = (pstSnsState->u32FLStd > 0x1FFFF) ? 0x1FFFF : pstSnsState->u32FLStd;
        imx327_write_register (ViPipe, IMX327_VMAX_ADDR, pstSnsState->u32FLStd & 0xFF);
        imx327_write_register (ViPipe, IMX327_VMAX_ADDR + 1, (pstSnsState->u32FLStd & 0xFF00) >> 8);
        imx327_write_register (ViPipe, IMX327_VMAX_ADDR + 2, (pstSnsState->u32FLStd & 0xF0000) >> 16);
        pstSnsState->bSyncInit = HI_FALSE;
    }

    return;
}

static HI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit    = HI_FALSE;

    switch (u8Mode & 0x3F)
    {
        case WDR_MODE_NONE:
            pstSnsState->enWDRMode = WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        case WDR_MODE_2To1_FRAME:
            pstSnsState->enWDRMode = WDR_MODE_2To1_FRAME;
            printf("2to1 half-rate frame WDR mode\n");
            break;

        case WDR_MODE_2To1_LINE:
            pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
            printf("2to1 line WDR 1080p mode(60fps->30fps)\n");
            break;

        default:
            ISP_TRACE(HI_DBG_ERR, "NOT support this mode!\n");
            return HI_FAILURE;
    }

    memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSnsRegsInfo);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((HI_FALSE == pstSnsState->bSyncInit) || (HI_FALSE == pstSnsRegsInfo->bConfig))
    {
        pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
        pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunImx327BusInfo[ViPipe].s8I2cDev;
        pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        pstSnsState->astRegsInfo[0].u32RegNum = 8;

        if ( WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 3;
            pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        }

        else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].u32RegNum = 16;
            pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        }

        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = imx327_i2c_addr;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = imx327_addr_byte;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = imx327_data_byte;
        }

        //Linear Mode Regs
        pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr = IMX327_SHS1_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr = IMX327_SHS1_ADDR + 1;
        pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr = IMX327_SHS1_ADDR + 2;

        pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 0;       //make shutter and gain effective at the same time
        pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr = IMX327_GAIN_ADDR;  //gain
        pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr = IMX327_HCG_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX327_VMAX_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX327_VMAX_ADDR + 1;
        pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX327_VMAX_ADDR + 2;

        if ( WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX327_SHS1_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX327_SHS1_ADDR + 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX327_SHS1_ADDR + 2;

            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = IMX327_VMAX_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = IMX327_VMAX_ADDR + 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = IMX327_VMAX_ADDR + 2;

        }
        //DOL 2t1 Mode Regs
        else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 0;

            pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX327_SHS2_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX327_SHS2_ADDR + 1;
            pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX327_SHS2_ADDR + 2;

            pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = IMX327_VMAX_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = IMX327_VMAX_ADDR + 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = IMX327_VMAX_ADDR + 2;

            pstSnsState->astRegsInfo[0].astI2cData[11].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[11].u32RegAddr = IMX327_RHS1_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[12].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[12].u32RegAddr = IMX327_RHS1_ADDR + 1;
            pstSnsState->astRegsInfo[0].astI2cData[13].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[13].u32RegAddr = IMX327_RHS1_ADDR + 2;

            pstSnsState->astRegsInfo[0].astI2cData[14].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[14].u32RegAddr = IMX327_Y_OUT_SIZE_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[15].u8DelayFrmNum = 1;
            pstSnsState->astRegsInfo[0].astI2cData[15].u32RegAddr = IMX327_Y_OUT_SIZE_ADDR + 1;
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

        if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[0].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[1].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[2].bUpdate = HI_TRUE;

            pstSnsState->astRegsInfo[0].astI2cData[5].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[6].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[7].bUpdate = HI_TRUE;
        }
    }

    pstSnsRegsInfo->bConfig = HI_FALSE;
    memcpy(pstSnsRegsInfo, &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    memcpy(&pstSnsState->astRegsInfo[1], &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    //printf("regNum: %d\n", pstSnsState->astRegsInfo[0].u32RegNum);
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = 0;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    u8SensorImageMode = pstSnsState->u8ImgMode;
    pstSnsState->bSyncInit = HI_FALSE;

    if (pstSensorImageMode->f32Fps <= 30)
    {
        if (WDR_MODE_NONE == pstSnsState->enWDRMode)
        {
            if (IMX327_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
            {
                u8SensorImageMode              = IMX327_SENSOR_1080P_30FPS_LINEAR_MODE;
                pstSnsState->u32FLStd          = IMX327_VMAX_1080P30_LINEAR;
                g_astimx327State[ViPipe].u8Hcg = 0x2;
            }
            else
            {
                IMX327_ERR_MODE_PRINT(pstSensorImageMode, pstSnsState);
                return HI_FAILURE;
            }
        }
        else if (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode)
        {
            if (IMX327_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
            {
                u8SensorImageMode              = IMX327_SENSOR_1080P_30FPS_FRAME_WDR_2to1_MODE;
                pstSnsState->u32FLStd          = IMX327_VMAX_1080P30_LINEAR;
                g_astimx327State[ViPipe].u8Hcg = 0x2;
            }
            else
            {
                IMX327_ERR_MODE_PRINT(pstSensorImageMode, pstSnsState);
                return HI_FAILURE;
            }
        }
        else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            if (IMX327_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
            {
                u8SensorImageMode               = IMX327_SENSOR_1080P_30FPS_WDR_2to1_MODE;
                pstSnsState->u32FLStd           = IMX327_VMAX_1080P60TO30_WDR * 2;
                g_astimx327State[ViPipe].u32BRL = 1109;
                g_astimx327State[ViPipe].u8Hcg  = 0x1;
            }
            else
            {
                IMX327_ERR_MODE_PRINT(pstSensorImageMode, pstSnsState);
                return HI_FAILURE;
            }
        }
        else
        {
            IMX327_ERR_MODE_PRINT(pstSensorImageMode, pstSnsState);
            return HI_FAILURE;
        }
    }
    else
    {
    }

    if ((HI_TRUE == pstSnsState->bInit) && (u8SensorImageMode == pstSnsState->u8ImgMode))
    {
        /* Don't need to switch SensorImageMode */
        return ISP_DO_NOT_NEED_SWITCH_IMAGEMODE;
    }

    pstSnsState->u8ImgMode = u8SensorImageMode;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX327_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = IMX327_SENSOR_1080P_30FPS_LINEAR_MODE;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd  = IMX327_VMAX_1080P30_LINEAR;
    pstSnsState->au32FL[0] = IMX327_VMAX_1080P30_LINEAR;
    pstSnsState->au32FL[1] = IMX327_VMAX_1080P30_LINEAR;

    memset(&pstSnsState->astRegsInfo[0], 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&pstSnsState->astRegsInfo[1], 0, sizeof(ISP_SNS_REGS_INFO_S));
}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);

    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init         = imx327_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit         = imx327_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init  = sensor_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode      = cmos_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode        = cmos_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_isp_default     = cmos_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect    = cmos_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info    = cmos_get_sns_regs_info;

    return HI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

static HI_S32 imx327_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunImx327BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX327_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

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

    IMX327_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX327_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    IMX327_SENSOR_RESET_CTX(ViPipe);
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

    stSnsAttrInfo.eSensorId = IMX327_ID;

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

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(ViPipe, IMX327_ID);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, IMX327_ID);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, IMX327_ID);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }

    sensor_ctx_exit(ViPipe);

    return HI_SUCCESS;
}

static HI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
    CMOS_CHECK_POINTER(pstInitAttr);

    g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
    g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
    g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
    g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
    g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
    g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
    g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;

    return HI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsImx327Obj =
{
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = imx327_standby,
    .pfnRestart             = imx327_restart,
    .pfnMirrorFlip          = HI_NULL,
    .pfnWriteReg            = imx327_write_register,
    .pfnReadReg             = imx327_read_register,
    .pfnSetBusInfo          = imx327_set_bus_info,
    .pfnSetInit             = sensor_set_init
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX327_CMOS_H_ */
