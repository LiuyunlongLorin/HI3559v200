/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : OS08A10_cmos.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2018/11/07
  Description   :
  History       :
  1.Date        : 2018/11/07
    Author      :
    Modification: Created file

******************************************************************************/
#if !defined(__OS08A10_CMOS_H_)
#define __OS08A10_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"

#include "os08a10_cmos_ex.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define OS08A10_ID 0x08a10

#ifndef CLIP3
#define CLIP3(x,min,max)    ((x)< (min) ? (min) : ((x)>(max)?(max):(x)))
#endif

/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/
ISP_SNS_STATE_S *g_pastOS08A10[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define OS08A10_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastOS08A10[dev])
#define OS08A10_SENSOR_SET_CTX(dev, pstCtx)   (g_pastOS08A10[dev] = pstCtx)
#define OS08A10_SENSOR_RESET_CTX(dev)         (g_pastOS08A10[dev] = HI_NULL)

static HI_U32 g_au32InitExposure[ISP_MAX_PIPE_NUM]  = {0};
static HI_U32 g_au32LinesPer500ms[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16InitWBGain[ISP_MAX_PIPE_NUM][3] = {{0}};
static HI_U16 g_au16SampleRgain[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16SampleBgain[ISP_MAX_PIPE_NUM] = {0};

ISP_SNS_COMMBUS_U g_aunOS08A10BusInfo[ISP_MAX_PIPE_NUM] =
{
    [0] = { .s8I2cDev = 0},
    [1 ... ISP_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

static ISP_FSWDR_MODE_E genFSWDRMode[ISP_MAX_PIPE_NUM] =
{
    [0 ... ISP_MAX_PIPE_NUM - 1] = ISP_FSWDR_NORMAL_MODE
};

typedef struct hiOS08A10_STATE_S
{
    HI_U32      u32BRL;
    HI_U32      u32RHS1_MAX;
    HI_U32      u32deltaRHS1;
} OS08A10_STATE_S;

OS08A10_STATE_S g_astOS08A10State[ISP_MAX_PIPE_NUM] = {{0}};

static HI_U32  gu32MaxTimeGetCnt[ISP_MAX_PIPE_NUM] = {0};

///static HI_U32 gu32STimeFps = 30;
///static HI_U32 gu32LGain = 0;
static HI_U32 u32PreAGain[ISP_MAX_PIPE_NUM] = {0x80, 0x80};
static HI_U32 u32CurAGain[ISP_MAX_PIPE_NUM] = {0x80, 0x80};


/****************************************************************************
 * extern                                                                   *
 ****************************************************************************/
extern unsigned char OS08A10_i2c_addr;
extern unsigned int OS08A10_addr_byte;
extern unsigned int OS08A10_data_byte;

extern void OS08A10_init(VI_PIPE ViPipe);
extern void OS08A10_exit(VI_PIPE ViPipe);
extern void OS08A10_standby(VI_PIPE ViPipe);
extern void OS08A10_restart(VI_PIPE ViPipe);
extern int OS08A10_write_register(VI_PIPE ViPipe, int addr, int data);
extern int OS08A10_read_register(VI_PIPE ViPipe, int addr);

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
#define OS08A10_FULL_LINES_MAX  (0xE245)
#define OS08A10_FULL_LINES_MAX_2TO1_WDR  (0xE245)    // considering the YOUT_SIZE and bad frame

/*****OS08A10 Register Address*****/
#define OS08A10_EXPO_H_ADDR  (0x3501)
#define OS08A10_EXPO_L_ADDR  (0x3502)
#define OS08A10_AGAIN_H_ADDR (0x3508)
#define OS08A10_AGAIN_L_ADDR (0x3509)
#define OS08A10_DGAIN_H_ADDR (0x350a)
#define OS08A10_DGAIN_L_ADDR (0x350b)
#define OS08A10_VMAX_H_ADDR  (0x380e)
#define OS08A10_VMAX_L_ADDR  (0x380f)
#define OS08A10_R3740_ADDR   (0x3740)
#define OS08A10_R3741_ADDR   (0x3741)
/*****OS08A10 2to1 WDR Register Address*****/
#define OS08A10_SHORT_EXPO_H_ADDR  (0x3511)
#define OS08A10_SHORT_EXPO_L_ADDR  (0x3512)
#define OS08A10_SHORT_AGAIN_H_ADDR (0x350c)
#define OS08A10_SHORT_AGAIN_L_ADDR (0x350d)
#define OS08A10_SHORT_DGAIN_H_ADDR (0x350e)
#define OS08A10_SHORT_DGAIN_L_ADDR (0x350f)

#define OS08A10_INCREASE_LINES (1) /* make real fps less than stand fps because NVR require*/

#define OS08A10_VMAX_4k2k30_LINEAR  (2316+OS08A10_INCREASE_LINES)
//#define OS08A10_VMAX_4k2k25_WDR     (2316+OS08A10_INCREASE_LINES)
#define OS08A10_VMAX_4k2k30_WDR     (2316+OS08A10_INCREASE_LINES)

#define OS08A10_VMAX_1080P30_LINEAR  (2316+OS08A10_INCREASE_LINES)

#define OS08A10_VMAX_1080P120_LINEAR     (1158+OS08A10_INCREASE_LINES)

//sensor fps mode

#define OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE      (1)
#define OS08A10_SENSOR_1080P30FPS_10BIT_LINEAR_MODE	   (2)
#define OS08A10_SENSOR_1080P120FPS_10BI_LINEAR_MODE	   (3)

#define OS08A10_RES_IS_8M_10BIT_LINEAR(w, h)    ((w == 3840) && (h == 2160))

#define OS08A10_RES_IS_5M_12BIT_LINEAR(w, h)    ((w == 2592) && (h == 1944))
#define OS08A10_RES_IS_4M_12BIT_LINEAR(w, h)    ((w == 2592) && (h == 1520))
#define OS08A10_RES_IS_2M_12BIT_LINEAR(w, h)    ((w == 1920) && (h == 1080))

//sensor gain
//#define OS08A10_AGAIN_MIN    (1024)
//#define OS08A10_AGAIN_MAX    (15000)     //the max again is 15.5x = 15872


///#define OS08A10_AD_GAIN_TBL_RANGE  241
///#define OS08A10_AGAIN_TBL_RANGE  100
///#define OS08A10_DGAIN_TBL_RANGE  140

///#define OS08A10_FRAME_TYPE_SHORT      1
///#define OS08A10_FRAME_TYPE_LONG       0

static HI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32Fll = 0;
    HI_U32 U32MaxFps = 0;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
    {
        u32Fll = OS08A10_VMAX_4k2k30_LINEAR;
        U32MaxFps = 30;
        //pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
        //pstSnsState->u32FLStd = u32Fll;
        //pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
    }
/*
    else if (OS08A10_SENSOR_8M_30FPS_LINEAR_MODE == pstSnsState->u8ImgMode)
    {
        u32Fll = OS08A10_VMAX_4k2k30_LINEAR;
        U32MaxFps = 30;
        //pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
        //pstSnsState->u32FLStd = u32Fll;
        //pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
    }
    else if (OS08A10_SENSOR_8M_30FPS_LINEAR_MODE == pstSnsState->u8ImgMode)
    {
        u32Fll = OS08A10_VMAX_4k2k30_LINEAR;
        U32MaxFps = 25 ;
        //pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
        //pstSnsState->u32FLStd = u32Fll;
        //pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
    }
    else
    {
        u32Fll = OS08A10_VMAX_4k2k30_LINEAR;
        U32MaxFps = 30;
    }
*/
    pstSnsState->u32FLStd = u32Fll;

    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = (pstSnsState->u32FLStd > 8) ? (pstSnsState->u32FLStd - 8) : (0);

    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq = 50*256;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->bAERouteExValid = HI_FALSE;
    pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
    pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;

    if (g_au32InitExposure[ViPipe] == 0)
    {
        pstAeSnsDft->u32InitExposure = 130000;
    }
    else
    {
        pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe];
    }

    if (g_au32LinesPer500ms[ViPipe] == 0)
    {
        pstAeSnsDft->u32LinesPer500ms = (u32Fll * U32MaxFps) >> 1;
    }
    else
    {
        pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
    }

    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
        {
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u32MaxAgain = 1984;//15.5x
            pstAeSnsDft->u32MinAgain = 128;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            pstAeSnsDft->u32MaxDgain = 2047;
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
            pstAeSnsDft->u8AeCompensation = 0x38;
            pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;

            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
            pstAeSnsDft->u32MinIntTime = 2;///20;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

            break;
        }

        case WDR_MODE_2To1_LINE:
        {
            /* FS WDR mode */
            pstAeSnsDft->au8HistThresh[0] = 0xc;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = 0x38;

            //pstAeSnsDft->u32MaxIntTime = g_apstSnsState[ViPipe]->u32FLStd - 8;
            pstAeSnsDft->u32MinIntTime = 27;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;;

            pstAeSnsDft->u32MaxIntTimeStep = 1000;
            pstAeSnsDft->u32LFMinExposure = 15000000;
            pstAeSnsDft->enAeExpMode = AE_EXP_LOWLIGHT_PRIOR;

            if (ISP_FSWDR_LONG_FRAME_MODE != genFSWDRMode[ViPipe])
            {
                pstAeSnsDft->u16ManRatioEnable = HI_FALSE;
                pstAeSnsDft->au32Ratio[0] = 0x400;
                pstAeSnsDft->au32Ratio[1] = 0x40;
                pstAeSnsDft->au32Ratio[2] = 0x40;
            }

            break;
        }
    }

    return HI_SUCCESS;
}

/* the function of sensor set fps */
static HI_VOID cmos_fps_set(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32MaxFps;
    HI_U32 u32Lines;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    switch (pstSnsState->u8ImgMode)
    {
        case OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE:
            if ((f32Fps <= 30.0) && (f32Fps >= 1.2))
            {
                u32MaxFps = 30;
                u32Lines = OS08A10_VMAX_4k2k30_LINEAR * u32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
                pstSnsState->u32FLStd = u32Lines;
                //pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
            }
            else
            {
                ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
                return;
            }

            break;
		case OS08A10_SENSOR_1080P120FPS_10BI_LINEAR_MODE:
           if ((f32Fps <= 120) && (f32Fps >= 30))
           {
               u32Lines = OS08A10_VMAX_1080P120_LINEAR * 120 / f32Fps;
           }
           else
           {
               printf("Not support Fps: %f\n", f32Fps);
               return;
           }
           u32Lines = (u32Lines > OS08A10_FULL_LINES_MAX) ? OS08A10_FULL_LINES_MAX : u32Lines;
           break;

        default:
            ISP_TRACE(HI_DBG_ERR, "Not support this Mode!!!\n");
            return;
            break;
    }

    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = ((u32Lines & 0xFF00) >> 8);
    pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (u32Lines & 0xFF);

    pstSnsState->u32FLStd = u32Lines;

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 0x48;
    }
    else
    {
        pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
    }
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];

    return;

}

static HI_VOID cmos_slow_framerate_set(VI_PIPE ViPipe,HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        u32FullLines = (u32FullLines > OS08A10_FULL_LINES_MAX_2TO1_WDR) ? OS08A10_FULL_LINES_MAX_2TO1_WDR : u32FullLines;
    }
    else
    {
        u32FullLines = (u32FullLines > OS08A10_FULL_LINES_MAX) ? OS08A10_FULL_LINES_MAX : u32FullLines;
    }

    pstSnsState->au32FL[0] = u32FullLines;

    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = (((pstSnsState->au32FL[0]) & 0xFF00) >> 8);
    pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = ((pstSnsState->au32FL[0]) & 0xFF);

    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - 0x48;
    }
    else
    {
        pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - 8;
    }

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID cmos_inttime_update(VI_PIPE ViPipe,HI_U32 u32IntTime)
{
    HI_U32 u32Value = 0;

    static HI_BOOL bFirst[ISP_MAX_PIPE_NUM] ={1, 1};

    static HI_U32 u32ShortIntTime[ISP_MAX_PIPE_NUM] = {0};
    static HI_U32 u32LongIntTime[ISP_MAX_PIPE_NUM] = {0};
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
      if (bFirst[ViPipe]) /* short exposure */
      {
           pstSnsState->au32WDRIntTime[0] = u32IntTime;
           u32ShortIntTime[ViPipe] = u32IntTime;

           pstSnsState->astRegsInfo[0].astI2cData[10].u32Data = ((u32ShortIntTime[ViPipe] & 0xFF00) >> 8);
           pstSnsState->astRegsInfo[0].astI2cData[11].u32Data = (u32ShortIntTime[ViPipe] & 0xFF);

           bFirst[ViPipe] = HI_FALSE;
       }
       else   /* long exposure */
       {
           pstSnsState->au32WDRIntTime[1] = u32IntTime;
           u32LongIntTime[ViPipe] = (u32IntTime + u32ShortIntTime[ViPipe] <= pstSnsState->au32FL[1] - 0x48) ? u32IntTime:pstSnsState->au32FL[1] - 0x48 - u32ShortIntTime[ViPipe];

           pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32LongIntTime[ViPipe] & 0xFF00) >> 8);
           pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32LongIntTime[ViPipe] & 0xFF);

           bFirst[ViPipe] = HI_TRUE;
        }
    }
    else
    {
         u32Value = u32IntTime;

         pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = ((u32Value & 0xFF00) >> 8);
         pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = (u32Value & 0xFF);
    }

  return;
}

static HI_VOID cmos_gains_update(VI_PIPE ViPipe,HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U32 u32AGainReg = 0;
    HI_U32 u32DGainReg = 0;
    HI_U32 u32AnalogTiming_1 = 0;
    HI_U32 u32AnalogTiming_2 = 0;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32PreAGain[ViPipe] = u32CurAGain[ViPipe];

    //Analog Gain
    //u32Again = (u32Again*128)/1024;

    u32Again = CLIP3(u32Again, 0x80, 0x7FF);

    u32AGainReg = u32Again;
    //printf("u32AGainReg %d \n",u32AGainReg);


    u32CurAGain[ViPipe] = u32Again;
    if (u32Again < 0x100)
    {
        u32AnalogTiming_1 = 0x07;
        u32AnalogTiming_2 = 0x03;
    }
    else if (u32Again < 0x200)
    {
        u32AnalogTiming_1 = 0x09;
        u32AnalogTiming_2 = 0x03;
    }
    else if (u32Again < 0x400)
    {
        u32AnalogTiming_1 = 0x10;
        u32AnalogTiming_2 = 0x03;
    }
    else
    {
        u32AnalogTiming_1 = 0x1b;
        u32AnalogTiming_2 = 0x03;
    }

    // Digital Gain
    //*
    if(u32Dgain < 0x80)
    {
        u32DGainReg = 0x400;
    }
    else if(u32Dgain < 0x800)
    {
        u32DGainReg = u32Dgain*0x8;
    }
    else
    {
        u32DGainReg = 0x3fff;
    }
    //*/

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        /*
        pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = ((u32AGainReg & 0xff00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = (u32AGainReg & 0xff);
        pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = ((u32DGainReg & 0xff00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = (u32DGainReg & 0xff);

        pstSnsState->astRegsInfo[0].astI2cData[12].u32Data = ((u32AGainReg & 0xff00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[13].u32Data = (u32AGainReg & 0xff);
        pstSnsState->astRegsInfo[0].astI2cData[14].u32Data = ((u32DGainReg & 0xff00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[15].u32Data = (u32DGainReg & 0xff);

        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = (u32AnalogTiming_1 & 0xff);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = (u32AnalogTiming_2 & 0xff);
        */
    }
    else
    {
        pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = ((u32AGainReg & 0x3f00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = (u32AGainReg & 0xff);
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = ((u32DGainReg & 0x3f00) >> 8);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = (u32DGainReg & 0xff);

        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = (u32AnalogTiming_1 & 0xff);
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = (u32AnalogTiming_2 & 0xff);
    }

    return;
}

static HI_VOID cmos_get_inttime_max(VI_PIPE ViPipe,HI_U16 u16ManRatioEnable, HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 u32IntTimeMaxTmp = 0;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMax);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMin);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if ((WDR_MODE_2To1_LINE == pstSnsState->enWDRMode))
    {
        printf("null pointer when get ae sensor ExpRatio/IntTimeMax/IntTimeMin value!\n");
        return;
    }

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        /* Short + Long < VMax - 0x48; */
        /* Ratio = Long * 0x40 / Short */
        u32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 0x48 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
        au32IntTimeMax[0] = (pstSnsState->au32FL[0] - 0x48) * 0x40 / (au32Ratio[0] + 0x40);

        au32IntTimeMax[0] = (u32IntTimeMaxTmp < au32IntTimeMax[0]) ? u32IntTimeMaxTmp : au32IntTimeMax[0];
        au32IntTimeMax[1] = au32IntTimeMax[0] * au32Ratio[0] >> 6;
        au32IntTimeMin[0] = 2;
        au32IntTimeMin[1] = au32IntTimeMin[0] * au32Ratio[0] >> 6;
    }

    return;

}


/* Only used in FSWDR mode */
static HI_VOID cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
    CMOS_CHECK_POINTER_VOID(pstAeFSWDRAttr);

    genFSWDRMode[ViPipe]      = pstAeFSWDRAttr->enFSWDRMode;
    gu32MaxTimeGetCnt[ViPipe] = 0;
}


static HI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= cmos_slow_framerate_set;
    pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
    //pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
    //pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
    pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

    return HI_SUCCESS;
}

/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN 0
#define GOLDEN_BGAIN 0

static HI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAwbSnsDft);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    pstAwbSnsDft->u16WbRefTemp = 4950;
    pstAwbSnsDft->au16GainOffset[0] = 531;
    pstAwbSnsDft->au16GainOffset[1] = 256;
    pstAwbSnsDft->au16GainOffset[2] = 256;
    pstAwbSnsDft->au16GainOffset[3] = 450;
    pstAwbSnsDft->as32WbPara[0] = -42;
    pstAwbSnsDft->as32WbPara[1] = 298;
    pstAwbSnsDft->as32WbPara[2] = 0;
    pstAwbSnsDft->as32WbPara[3] = 175532;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -125725;

    pstAwbSnsDft->u16GoldenRgain = GOLDEN_RGAIN;
    pstAwbSnsDft->u16GoldenBgain = GOLDEN_BGAIN;


    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
            break;

        case WDR_MODE_2To1_LINE:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcmFsWdr, sizeof(AWB_CCM_S));
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
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
#ifdef CONFIG_HI_ISP_CA_SUPPORT
    pstDef->unKey.bit1Ca       = 1;
    pstDef->pstCa              = &g_stIspCA;
#endif
    pstDef->unKey.bit1Clut     = 1;
    pstDef->pstClut            = &g_stIspCLUT;

    pstDef->unKey.bit1Dpc      = 1;
    pstDef->pstDpc             = &g_stCmosDpc;

    pstDef->unKey.bit1Wdr      = 1;
    pstDef->pstWdr             = &g_stIspWDR;
#ifdef CONFIG_HI_ISP_HLC_SUPPORT
    pstDef->unKey.bit1Hlc      = 0;
    pstDef->pstHlc             = &g_stIspHlc;
#endif
    pstDef->unKey.bit1Lsc      = 1;
    pstDef->pstLsc             = &g_stCmosLsc;
#ifdef CONFIG_HI_ISP_EDGEMARK_SUPPORT
    pstDef->unKey.bit1EdgeMark = 0;
    pstDef->pstEdgeMark        = &g_stIspEdgeMark;
#endif
#ifdef CONFIG_HI_ISP_CR_SUPPORT
    pstDef->unKey.bit1Ge       = 1;
    pstDef->pstGe              = &g_stIspGe;
#endif
    pstDef->unKey.bit1Detail   = 1;
    pstDef->pstDetail          = &g_stIspDetail;

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
            pstDef->unKey.bit1Gamma          = 1;
            pstDef->pstGamma                 = &g_stIspGamma;
            pstDef->unKey.bit1BayerNr        = 1;
            pstDef->pstBayerNr               = &g_stIspBayerNr;
            pstDef->unKey.bit1AntiFalseColor = 1;
            pstDef->pstAntiFalseColor        = &g_stIspAntiFalseColor;
            pstDef->unKey.bit1Ldci           = 1;
            pstDef->pstLdci                  = &g_stIspLdci;
            pstDef->unKey.bit1Dehaze         = 1;
            pstDef->pstDehaze                = &g_stIspDehaze;
            memcpy(&pstDef->stNoiseCalibration, &g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;

        case WDR_MODE_2To1_LINE:
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

            pstDef->stWdrSwitchAttr.au32ExpRatio[0] = 0x40;

            memcpy(&pstDef->stNoiseCalibration, &g_stIspNoiseCalibratio, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;
    }

    pstDef->stSensorMode.u32SensorID = OS08A10_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;

    memcpy(&pstDef->stDngColorParam, &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

    switch (pstSnsState->u8ImgMode)
    {
        default:
        case OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 12;
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 2592;
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
    //HI_S32  i;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstBlackLevel->au16BlackLevel[0] = 254;
        pstBlackLevel->au16BlackLevel[1] = 256;
        pstBlackLevel->au16BlackLevel[2] = 256;
        pstBlackLevel->au16BlackLevel[3] = 252;
    }
    else if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        pstBlackLevel->au16BlackLevel[0] = 254;
        pstBlackLevel->au16BlackLevel[1] = 256;
        pstBlackLevel->au16BlackLevel[2] = 256;
        pstBlackLevel->au16BlackLevel[3] = 252;
    }
    else
    {
        pstBlackLevel->au16BlackLevel[0] = 254;
        pstBlackLevel->au16BlackLevel[1] = 256;
        pstBlackLevel->au16BlackLevel[2] = 256;
        pstBlackLevel->au16BlackLevel[3] = 252;
    }


    return 0;

}

static HI_VOID cmos_set_pixel_detect(VI_PIPE ViPipe, HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = 0;
    HI_U32 u32MaxIntTime_5Fps = 0;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        return;
    }
    else
    {
        if (OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (OS08A10_VMAX_4k2k30_LINEAR * 30) / 5;
        }
		else if (OS08A10_SENSOR_1080P30FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            u32FullLines_5Fps = (OS08A10_VMAX_1080P30_LINEAR * 30) / 5;
        }

        else
        {
            return;
        }
    }

    //u32FullLines_5Fps = (u32FullLines_5Fps > OS08A10_FULL_LINES_MAX) ? OS08A10_FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxIntTime_5Fps = u32FullLines_5Fps - 8;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        OS08A10_write_register (ViPipe,OS08A10_AGAIN_H_ADDR,0x00);
        OS08A10_write_register (ViPipe,OS08A10_AGAIN_L_ADDR,0x80);

        OS08A10_write_register (ViPipe,OS08A10_VMAX_L_ADDR, u32FullLines_5Fps & 0xFF);
        OS08A10_write_register (ViPipe,OS08A10_VMAX_H_ADDR, (u32FullLines_5Fps & 0xFF00) >> 8);

        OS08A10_write_register (ViPipe,OS08A10_EXPO_L_ADDR, u32MaxIntTime_5Fps & 0xFF);
        OS08A10_write_register (ViPipe,OS08A10_EXPO_H_ADDR,  (u32MaxIntTime_5Fps & 0xFF00) >> 8);

    }
    else /* setup for ISP 'normal mode' */
    {
        pstSnsState->u32FLStd = (pstSnsState->u32FLStd > OS08A10_FULL_LINES_MAX) ? OS08A10_FULL_LINES_MAX : pstSnsState->u32FLStd;
        OS08A10_write_register (ViPipe,OS08A10_VMAX_L_ADDR, pstSnsState->u32FLStd & 0xFF);
        OS08A10_write_register (ViPipe,OS08A10_VMAX_H_ADDR, (pstSnsState->u32FLStd & 0xFF00) >> 8);
        pstSnsState->bSyncInit = HI_FALSE ;
    }

    return;
}


static HI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    switch(u8Mode & 0x3F)
    {
        case WDR_MODE_NONE:
            pstSnsState->enWDRMode = WDR_MODE_NONE;
            printf("linear mode\n");
            break;

        case WDR_MODE_2To1_LINE:
            pstSnsState->enWDRMode = WDR_MODE_2To1_LINE;
            //pstSnsState->u32FLStd = OS08A10_VMAX_4k2k30_WDR;
            //pstSnsState->u8ImgMode = OS08A10_VMAX_4k2k30_WDR;
            printf("2to1 line WDR mode\n");
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
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((HI_FALSE == pstSnsState->bSyncInit) || (HI_FALSE == pstSnsRegsInfo->bConfig))
    {
        pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
        pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunOS08A10BusInfo[ViPipe].s8I2cDev;
        pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        pstSnsState->astRegsInfo[0].u32RegNum = 10;

        if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 6;
        }

        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = OS08A10_i2c_addr;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = OS08A10_addr_byte;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = OS08A10_data_byte;
            //pstSnsState->astRegsInfo[0].astI2cData[i].u8IntPos = 1; /* config registers in EOF interrupt */
        }

        //Linear Mode Regs
        pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr = OS08A10_EXPO_H_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr = OS08A10_EXPO_L_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr = OS08A10_AGAIN_H_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr = OS08A10_AGAIN_L_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = OS08A10_DGAIN_H_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = OS08A10_DGAIN_L_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr = OS08A10_VMAX_H_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = OS08A10_VMAX_L_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = OS08A10_R3740_ADDR;
        pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = OS08A10_R3741_ADDR;

        if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
        {
            pstSnsState->astRegsInfo[0].astI2cData[10].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[10].u32RegAddr = OS08A10_SHORT_EXPO_H_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[11].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[11].u32RegAddr = OS08A10_SHORT_EXPO_L_ADDR;

            pstSnsState->astRegsInfo[0].astI2cData[12].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[12].u32RegAddr = OS08A10_SHORT_AGAIN_H_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[13].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[13].u32RegAddr = OS08A10_SHORT_AGAIN_L_ADDR;

            pstSnsState->astRegsInfo[0].astI2cData[14].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[14].u32RegAddr = OS08A10_SHORT_DGAIN_H_ADDR;
            pstSnsState->astRegsInfo[0].astI2cData[15].u8DelayFrmNum = 0;
            pstSnsState->astRegsInfo[0].astI2cData[15].u32RegAddr = OS08A10_SHORT_DGAIN_L_ADDR;
        }

        pstSnsState->bSyncInit = HI_TRUE;
    }
    else
    {
        for (i=0; i<pstSnsState->astRegsInfo[0].u32RegNum; i++)
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
    }

    pstSnsRegsInfo->bConfig = HI_FALSE;
    memcpy(pstSnsRegsInfo, &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    memcpy(&pstSnsState->astRegsInfo[1], &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));

    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U8 u8SensorImageMode = 0;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    //u8SensorImageMode = pstSnsState->u8ImgMode;
    pstSnsState->bSyncInit = HI_FALSE;

    printf("cmos_set_image_mode width: %d, height: %d, snsmode: %d, wdrmode:%d\n",
           pstSensorImageMode->u16Width,
           pstSensorImageMode->u16Height,
           pstSensorImageMode->u8SnsMode,
           pstSnsState->enWDRMode);

    if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
    {
        {
            ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
                      pstSensorImageMode->u16Width,
                      pstSensorImageMode->u16Height,
                      pstSensorImageMode->f32Fps,
                      pstSnsState->enWDRMode);
            return HI_FAILURE;
        }
    }
    else if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        if (OS08A10_RES_IS_8M_10BIT_LINEAR(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)
            || OS08A10_RES_IS_4M_12BIT_LINEAR(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
        {
            u8SensorImageMode      = OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE;
            pstSnsState->u32FLStd  = OS08A10_VMAX_4k2k30_LINEAR;
            //g_astOS08A10State[ViPipe].u32BRL = 1984 * 2;
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

    /* Sensor first init */
    if (HI_FALSE == pstSnsState->bInit)
    {
        pstSnsState->u8ImgMode = u8SensorImageMode;
        return HI_SUCCESS;
    }

    if ((HI_TRUE == pstSnsState->bInit) && (u8SensorImageMode == pstSnsState->u8ImgMode))
    {
        /* Don't need to switch SensorImageMode */
        return ISP_DO_NOT_NEED_SWITCH_IMAGEMODE;
    }
    pstSnsState->u8ImgMode = u8SensorImageMode;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    //memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = OS08A10_SENSOR_8M_30FPS_10BIT_LINEAR_MODE;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd = OS08A10_VMAX_4k2k30_LINEAR;
    pstSnsState->au32FL[0] = OS08A10_VMAX_4k2k30_LINEAR;
    pstSnsState->au32FL[1] = OS08A10_VMAX_4k2k30_LINEAR;

    memset(&pstSnsState->astRegsInfo[0], 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&pstSnsState->astRegsInfo[1], 0, sizeof(ISP_SNS_REGS_INFO_S));
}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);

    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = OS08A10_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit = OS08A10_exit;
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

static HI_S32 OS08A10_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunOS08A10BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

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

    OS08A10_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    OS08A10_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    OS08A10_SENSOR_RESET_CTX(ViPipe);
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

    stSnsAttrInfo.eSensorId = OS08A10_ID;

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

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(ViPipe, OS08A10_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, OS08A10_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, OS08A10_ID);

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

    g_au32InitExposure[ViPipe]  = pstInitAttr->u32Exposure;
    g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
    g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
    g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
    g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
    g_au16SampleRgain[ViPipe]   = pstInitAttr->u16SampleRgain;
    g_au16SampleBgain[ViPipe]   = pstInitAttr->u16SampleBgain;

    return HI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsOS08A10Obj =
{
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = OS08A10_standby,
    .pfnRestart             = OS08A10_restart,
    .pfnMirrorFlip          = HI_NULL,
    .pfnWriteReg            = OS08A10_write_register,
    .pfnReadReg             = OS08A10_read_register,
    .pfnSetBusInfo          = OS08A10_set_bus_info,
    .pfnSetInit             = sensor_set_init
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __OS08A10_CMOS_H_ */
