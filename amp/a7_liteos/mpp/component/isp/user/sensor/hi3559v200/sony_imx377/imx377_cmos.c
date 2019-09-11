
/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : imx377_cmos.c

  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/11/07
  Description   :
  History       :
  1.Date        : 2013/11/07
    Author      :
    Modification: Created file

******************************************************************************/
#if !defined(__IMX377_CMOS_H_)
#define __IMX377_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"

#include "imx377_cmos_ex.h"
#include "imx377_cmos_priv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define IMX377_ID 377

#define HIGHER_4BITS(x) (((x) & 0xf0000) >> 16)
//#define HIGH_8BITS(x) (((x) & 0xff00) >> 8)
//#define LOW_8BITS(x)  ((x)& 0x00ff)

ISP_SNS_STATE_S *g_pastImx377[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define IMX377_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastImx377[dev])
#define IMX377_SENSOR_SET_CTX(dev, pstCtx)   (g_pastImx377[dev] = pstCtx)
#define IMX377_SENSOR_RESET_CTX(dev)         (g_pastImx377[dev] = HI_NULL)

static HI_U32 g_au32InitExposure[ISP_MAX_PIPE_NUM]  = {0};
static HI_U32 g_au32LinesPer500ms[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16InitWBGain[ISP_MAX_PIPE_NUM][3] = {{0}};
static HI_U16 g_au16SampleRgain[ISP_MAX_PIPE_NUM] = {0};
static HI_U16 g_au16SampleBgain[ISP_MAX_PIPE_NUM] = {0};

ISP_SNS_COMMBUS_U g_aunImx377BusInfo[ISP_MAX_PIPE_NUM] =
{
    [0] = { .s8I2cDev = 0},
    [1 ... ISP_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

static HI_U32  gu32AGain[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 1024};
static HI_U32  gu32DGain[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = 1024};
/****************************************************************************
 * extern                                                                   *
 ****************************************************************************/
extern unsigned char imx377_i2c_addr;
extern unsigned int  imx377_addr_byte;
extern unsigned int  imx377_data_byte;

const IMX377_VIDEO_MODE_TBL_S g_astImx377ModeTbl[IMX377_MODE_BUTT] =
{
    {0x8E3, 0.11, 4, 8, 30,  1, "4K2K_10BIT_30FPS"  },               /*  720Mbps*1/2.5" mode*mode1*/
    {0x8E3, 0.22, 4, 8, 60,  2, "4K2K_10BIT_60FPS"  },               /*1440Mbps*1/2.5" mode*mode1*/
    {0x8E3, 0.44, 4, 7, 120, 0, "2K1K_12BIT_120FPS_WeightBinning"  },/*1440Mbps*1/2.5" mode*mode2*/
    {0x8E3, 0.44, 4, 7, 120, 1, "2K1K_12BIT_120FPS_NormalBinning"  },/*1440Mbps*1/2.5" mode*mode2*/
    {0x483, 0.45, 4, 8, 240, 1, "720P_10BIT_240FPS_Narrow"  },       /*1440Mbps*1/2.3" mode*mode1*/
    {0x38E, 0.44, 6, 4, 300, 0, "720P_12BIT_300FPS_Wide"},           /*1440Mbps*1/2.5" mode*mode4*/
    {0xC30, 0.26, 4, 8, 35,  0, "4K3K_12BIT_30FPS" },                /*1440Mbps*1/2.3" mode*mode0*/
    {0xDF7, 0.26, 4, 8, 30,  0, "4K2K_12BIT_30FPS" },                /*1440Mbps*1/2.5" mode*mode0*/
    {0xC4E, 0.15, 4, 8, 20,  1, "4K3K_10BIT_20FPS" }                 /*  720Mbps*1/2.3" mode*mode1*/
};


extern void imx377_init(VI_PIPE ViPipe);
extern void imx377_exit(VI_PIPE ViPipe);
extern void imx377_standby(VI_PIPE ViPipe);
extern void imx377_restart(VI_PIPE ViPipe);
extern int  imx377_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  imx377_read_register(VI_PIPE ViPipe, int addr);

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
#define IMX377_FULL_LINES_MAX  (0xFFFFF)
#define IMX377_SHR_MAX 0xFFFF
#define IMX377_SVR_MAX 0xFFFF
#define FL_ALIGNUP(x,g) (MIN((((x)+(g)-1)/(g)), IMX377_SVR_MAX)*(g))

#define IMX377_ERR_MODE_PRINT(pstSensorImageMode)\
    do{\
        ISP_TRACE(HI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, SnsMode:%d\n",\
                  pstSensorImageMode->u16Width,  \
                  pstSensorImageMode->u16Height, \
                  pstSensorImageMode->f32Fps,    \
                  pstSensorImageMode->u8SnsMode);\
    }while(0)

static HI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32Fll = 0;
    HI_U32 U32MaxFps = 30;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    u32Fll = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    U32MaxFps = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32MaxFps;
    pstAeSnsDft->stIntTimeAccu.f32Offset = g_astImx377ModeTbl[pstSnsState->u8ImgMode].fOffset;
    pstSnsState->u32FLStd = u32Fll;
    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;

    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32HmaxTimes = (1000000*1000) / (u32Fll * U32MaxFps);

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.3;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.3;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;

    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->bAERouteExValid = HI_FALSE;
    pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
    pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;

    if (g_au32InitExposure[ViPipe] == 0)
    {
        pstAeSnsDft->u32InitExposure = 1000000;
    }
    else
    {
        pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe];;
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

            pstAeSnsDft->u8AeCompensation = 0x2D;
            pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;

            pstAeSnsDft->u32MinIntTime = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineMin;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

            pstAeSnsDft->u32MaxAgain = 21845;
            pstAeSnsDft->u32MinAgain = 1500;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

            pstAeSnsDft->u32MaxDgain = 8192;  /* if Dgain enable,please set ispdgain bigger than 1*/
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

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
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32MaxFps = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32MaxFps;
    u32Lines = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax * u32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
    pstSnsState->u32FLStd = u32Lines;
    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;

    /* SHR 16bit, So limit full_lines as 0xFFFF */
    if (f32Fps > u32MaxFps || u32Lines > IMX377_SHR_MAX)
    {
        ISP_TRACE(HI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
        return;
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;

    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32HmaxTimes = (1000000 * 1000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

    pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = LOW_8BITS(pstSnsState->au32FL[0]);
    pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = HIGH_8BITS(pstSnsState->au32FL[0]);
    pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = HIGHER_4BITS(pstSnsState->au32FL[0]);

    return;
}

static HI_VOID cmos_slow_framerate_set(VI_PIPE ViPipe, HI_U32 u32FullLines,
                                       AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32Vmax;
    HI_U32 u32OneFrameLines;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (u32FullLines <= IMX377_FULL_LINES_MAX)
    {
        pstSnsState->au32FL[0] = u32FullLines;
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = LOW_8BITS(pstSnsState->au32FL[0]);
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = HIGH_8BITS(pstSnsState->au32FL[0]);
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = HIGHER_4BITS(pstSnsState->au32FL[0]);

    }
    else
    {
        u32Vmax = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;
        u32OneFrameLines = u32Vmax*g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32MaxFps;
        if (u32FullLines > (u32OneFrameLines*30)) //Maximum support 30 second exposure
        {
            u32FullLines = (u32OneFrameLines*30);
        }
        pstSnsState->au32FL[0] = FL_ALIGNUP(u32FullLines, u32Vmax);
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = LOW_8BITS(u32Vmax);
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = HIGH_8BITS(u32Vmax);
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data = HIGHER_4BITS(u32Vmax);

    }

    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID cmos_inttime_update(VI_PIPE ViPipe, HI_U32 u32IntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32Shr = 0;

    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32Shr = pstSnsState->au32FL[0] - u32IntTime;
    u32Shr = MIN(u32Shr, IMX377_SHR_MAX);
    u32Shr = MAX(u32Shr, g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit);

    pstSnsState->astRegsInfo[0].astI2cData[0].u32Data = LOW_8BITS(u32Shr);
    pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = HIGH_8BITS(u32Shr);

    if (pstSnsState->au32FL[0] <= IMX377_FULL_LINES_MAX)
    {
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = 0;
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = 0;
    }
    else
    {
        HI_U16 u16SvrReg;
        u16SvrReg = pstSnsState->au32FL[0] / DIV_0_TO_1(g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax) - 1;
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = LOW_8BITS(u16SvrReg);
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = HIGH_8BITS(u16SvrReg);
    }

    return;
}

static HI_VOID cmos_again_calc_table(VI_PIPE ViPipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);

    *pu32AgainDb = 2048 - ((1 << 21) / DIV_0_TO_1(*pu32AgainLin));
    *pu32AgainDb = MIN((*pu32AgainDb), 1952);
    *pu32AgainLin = (1 << 21) / DIV_0_TO_1(2048 - (*pu32AgainDb));

    gu32AGain[ViPipe] = *pu32AgainLin;

    return;
}

static HI_U32 dgain_table[4] =
{
    1024, 2048, 4096, 8192
};

static HI_VOID cmos_dgain_calc_table(VI_PIPE ViPipe, HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32DgainLin);
    CMOS_CHECK_POINTER_VOID(pu32DgainDb);

    if (*pu32DgainLin >= dgain_table[3])
    {
        gu32DGain[ViPipe] = dgain_table[3];
        *pu32DgainLin = dgain_table[3];
        *pu32DgainDb = 3;
        return ;
    }

    for (i = 1; i < 4; i++)
    {
        if (*pu32DgainLin < dgain_table[i])
        {
            gu32DGain[ViPipe] = dgain_table[i - 1];
            *pu32DgainLin = dgain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }

    return;
}

static HI_VOID cmos_gains_update(VI_PIPE ViPipe, HI_U32 u32Again, HI_U32 u32Dgain)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->astRegsInfo[0].astI2cData[2].u32Data = (u32Again & 0xFF);
    pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = ((u32Again >> 8) & 0x00FF);
    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = (u32Dgain & 0x03);

    return;
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

    return HI_SUCCESS;
}


/* AWB default parameter and function */
#define CALIBRATE_STATIC_WB_R_GAIN 0x1c5
#define CALIBRATE_STATIC_WB_GR_GAIN 0x100
#define CALIBRATE_STATIC_WB_GB_GAIN 0x100
#define CALIBRATE_STATIC_WB_B_GAIN 0x1d1

/* Calibration results for Auto WB Planck */
#define CALIBRATE_AWB_P1 -0x0012
#define CALIBRATE_AWB_P2 0x010b
#define CALIBRATE_AWB_Q1 -0x0007
#define CALIBRATE_AWB_A1 0x2711F
#define CALIBRATE_AWB_B1 0x0080
#define CALIBRATE_AWB_C1 -0x1A5C1

/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN 0
#define GOLDEN_BGAIN 0
static HI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstAwbSnsDft);
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4900;

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
            ;
    }
    pstAwbSnsDft->u16InitRgain = g_au16InitWBGain[ViPipe][0];
    pstAwbSnsDft->u16InitGgain = g_au16InitWBGain[ViPipe][1];
    pstAwbSnsDft->u16InitBgain = g_au16InitWBGain[ViPipe][2];
    pstAwbSnsDft->u16SampleRgain = g_au16SampleRgain[ViPipe];
    pstAwbSnsDft->u16SampleBgain = g_au16SampleBgain[ViPipe];

    return HI_SUCCESS;
}

static HI_S32 cmos_get_awb_spec_default(VI_PIPE ViPipe, AWB_SPEC_SENSOR_DEFAULT_S *pstAwbSpecSnsDft)
{
    CMOS_CHECK_POINTER(pstAwbSpecSnsDft);

    memset(pstAwbSpecSnsDft, 0, sizeof(AWB_SPEC_SENSOR_DEFAULT_S));

    memcpy(&pstAwbSpecSnsDft->stSpecAwbAttrs, &g_SpecAWBFactTbl, sizeof(ISP_SPECAWB_ATTR_S));
    memcpy(&pstAwbSpecSnsDft->stCaaControl, &g_SpecKCAWBCaaTblControl, sizeof(ISP_SPECAWB_CAA_CONTROl_S));

    return HI_SUCCESS;
}

static HI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S* pstExpFuncs)
{
    CMOS_CHECK_POINTER(pstExpFuncs);

    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;
    pstExpFuncs->pfn_cmos_get_awb_spec_default = cmos_get_awb_spec_default;

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
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
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
    pstDef->unKey.bit1Lsc      = 0;
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
            memcpy(&pstDef->stNoiseCalibration, &g_stIspNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;
    }

    pstDef->stSensorMode.u32SensorID = IMX377_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;

    memcpy(&pstDef->stDngColorParam, &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

    switch (pstSnsState->u8ImgMode)
    {
        default:
        case IMX377_1080P_120FPS_12BIT_LINEAR_MODE_WEIGHT:
        case IMX377_1080P_120FPS_12BIT_LINEAR_MODE_NORMAL:
        case IMX377_12M_30FPS_12BIT_LINEAR_MODE:
        case IMX377_8M_30FPS_12BIT_LINEAR_MODE:
        case IMX377_720x1364P_300FPS_12BIT_LINEAR_MODE:
            pstDef->stSensorMode.stDngRawFormat.u8BitsPerSample = 12;
            pstDef->stSensorMode.stDngRawFormat.u32WhiteLevel = 4095;
            break;

        case IMX377_8M_30FPS_10BIT_LINEAR_MODE:
        case IMX377_8M_60FPS_10BIT_LINEAR_MODE:
        case IMX377_720P_240FPS_10BIT_LINEAR_MODE:
        case IMX377_12M_20FPS_10BIT_LINEAR_MODE:
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
    CMOS_CHECK_POINTER(pstBlackLevel);

    /* It need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    if ((gu32AGain[ViPipe] >= IMX377_AGAIN_MAX) && (IMX377_DGAIN_8X == gu32DGain[ViPipe]))
    {
        pstBlackLevel->au16BlackLevel[0] = 0xD3;
        pstBlackLevel->au16BlackLevel[1] = 0xD2;
        pstBlackLevel->au16BlackLevel[2] = 0xD1;
        pstBlackLevel->au16BlackLevel[3] = 0xD1;
    }
    else if ((gu32AGain[ViPipe] >= IMX377_AGAIN_MAX) && (IMX377_DGAIN_4X == gu32DGain[ViPipe]))
    {
        pstBlackLevel->au16BlackLevel[0] = 0xCD;
        pstBlackLevel->au16BlackLevel[1] = 0xCC;
        pstBlackLevel->au16BlackLevel[2] = 0xCC;
        pstBlackLevel->au16BlackLevel[3] = 0xCC;
    }
    else if ((gu32AGain[ViPipe] >= IMX377_AGAIN_MAX) && (IMX377_DGAIN_2X == gu32DGain[ViPipe]))
    {
        pstBlackLevel->au16BlackLevel[0] = 0xCB;
        pstBlackLevel->au16BlackLevel[1] = 0xCA;
        pstBlackLevel->au16BlackLevel[2] = 0xCA;
        pstBlackLevel->au16BlackLevel[3] = 0xCA;
    }
    else if ((gu32AGain[ViPipe] >= IMX377_AGAIN_MAX) && (IMX377_DGAIN_1X == gu32DGain[ViPipe]))
    {
        pstBlackLevel->au16BlackLevel[0] = 0xC9;
        pstBlackLevel->au16BlackLevel[1] = 0xC9;
        pstBlackLevel->au16BlackLevel[2] = 0xC9;
        pstBlackLevel->au16BlackLevel[3] = 0xC9;
    }
    else
    {
        pstBlackLevel->au16BlackLevel[0] = 0xC8;
        pstBlackLevel->au16BlackLevel[1] = 0xC8;
        pstBlackLevel->au16BlackLevel[2] = 0xC8;
        pstBlackLevel->au16BlackLevel[3] = 0xC8;
    }

    return HI_SUCCESS;

}
static HI_VOID cmos_set_pixel_detect(VI_PIPE ViPipe, HI_BOOL bEnable)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* Sensor must be programmed for slow frame rate (5 fps and below) */
        /* change frame rate to 5 fps by setting 1 frame length  */
        if (IMX377_8M_30FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x52);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0x35);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_8M_60FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0xA4);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0x6A);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_1080P_120FPS_12BIT_LINEAR_MODE_WEIGHT == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x48);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0xD5);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_1080P_120FPS_12BIT_LINEAR_MODE_NORMAL == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x48);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0xD5);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_720P_240FPS_10BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x90);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0xD8);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_12M_30FPS_12BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x50);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0x55);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_720x1364P_300FPS_12BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0x48);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0xD5);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        else if (IMX377_8M_30FPS_12BIT_LINEAR_MODE == pstSnsState->u8ImgMode)
        {
            imx377_write_register(ViPipe, IMX377_VMAX , 0xCA);
            imx377_write_register(ViPipe, IMX377_VMAX  + 1, 0x53);
            imx377_write_register(ViPipe, IMX377_VMAX  + 2, 0x00);
        }
        imx377_write_register(ViPipe, IMX377_SHR_L, g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32ExpLineLimit);

        /* Analog and Digital gains both must be programmed for their minimum values */

        imx377_write_register(ViPipe, IMX377_SHR_H, 0x00);
        imx377_write_register(ViPipe, IMX377_DGAIN, 0x00);
        imx377_write_register(ViPipe, IMX377_PGC_L, 0x00);
        imx377_write_register(ViPipe, IMX377_PGC_H, 0x00);
    }
    else /* setup for ISP 'normal mode' */
    {
        imx377_write_register(ViPipe, IMX377_VMAX, (pstSnsState->u32FLStd & 0xff));
        imx377_write_register(ViPipe, IMX377_VMAX  + 1, (pstSnsState->u32FLStd & 0xff00) >> 8);
        imx377_write_register(ViPipe, IMX377_VMAX  + 2,  (pstSnsState->u32FLStd & 0xf0000) >> 16);
        pstSnsState->bSyncInit  = HI_FALSE;
    }

    return;
}

static HI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    switch (u8Mode& 0x3F)
    {
        case WDR_MODE_NONE:
            pstSnsState->enWDRMode = WDR_MODE_NONE;
            printf("linear mode\n");
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
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((HI_FALSE == pstSnsState->bSyncInit) || (HI_FALSE == pstSnsRegsInfo->bConfig))
    {
        pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
        pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunImx377BusInfo[ViPipe].s8I2cDev;
        pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        pstSnsState->astRegsInfo[0].u32RegNum = 10;

        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = imx377_i2c_addr;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = imx377_addr_byte;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = imx377_data_byte;
        }

        //shutter related
        pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr = IMX377_SHR_L;
        pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr = IMX377_SHR_H;

        // gain related
        pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr = IMX377_PGC_L;
        pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr = IMX377_PGC_H;
        pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 1;

        // Dgain
        pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 1;
        pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr = IMX377_DGAIN;

        //Vmax
        pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr = IMX377_VMAX;
        pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr = IMX377_VMAX + 1;
        pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr = IMX377_VMAX + 2;

        //svr registers
        pstSnsState->astRegsInfo[0].astI2cData[8].u32RegAddr = IMX377_SVR_L;
        pstSnsState->astRegsInfo[0].astI2cData[8].u32Data = 0;
        pstSnsState->astRegsInfo[0].astI2cData[8].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[9].u32RegAddr = IMX377_SVR_H;
        pstSnsState->astRegsInfo[0].astI2cData[9].u8DelayFrmNum = 0;
        pstSnsState->astRegsInfo[0].astI2cData[9].u32Data = 0;

        pstSnsState->astRegsInfo[0].astI2cData[2].u8IntPos = 0;
        pstSnsState->astRegsInfo[0].astI2cData[3].u8IntPos = 0;

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
    }

    memcpy(pstSnsRegsInfo, &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    memcpy(&pstSnsState->astRegsInfo[1], &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));

    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U8 u8SnsMode;
    HI_U32 u32W, u32H;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    u32H      = pstSensorImageMode->u16Height;
    u32W      = pstSensorImageMode->u16Width;
    u8SnsMode = pstSensorImageMode->u8SnsMode;

    if (IMX377_RES_IS_1M(u32W, u32H))
    {
        if (0 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_720x1364P_300FPS_12BIT_LINEAR_MODE;
        }
        else if (1 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_720P_240FPS_10BIT_LINEAR_MODE;
        }
        else
        {
            IMX377_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        }
    }
    else if (IMX377_RES_IS_2M(u32W, u32H))
    {
        if (0 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_1080P_120FPS_12BIT_LINEAR_MODE_WEIGHT;
        }
        else if (1 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_1080P_120FPS_12BIT_LINEAR_MODE_NORMAL;
        }
        else
        {
            IMX377_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        }
    }
    else if (IMX377_RES_IS_8M(u32W, u32H))
    {
        if (0 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_8M_30FPS_12BIT_LINEAR_MODE;
        }
        else if (1 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_8M_30FPS_10BIT_LINEAR_MODE;
        }
        else if (2 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_8M_60FPS_10BIT_LINEAR_MODE;
        }
        else
        {
            IMX377_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        }
    }
    else if (IMX377_RES_IS_12M(u32W, u32H))
    {
        if (0 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_12M_30FPS_12BIT_LINEAR_MODE;
        }
        else if (1 == u8SnsMode)
        {
            u8SensorImageMode = IMX377_12M_20FPS_10BIT_LINEAR_MODE;
        }
        else
        {
            IMX377_ERR_MODE_PRINT(pstSensorImageMode);
            return HI_FAILURE;
        }
    }
    else
    {
        IMX377_ERR_MODE_PRINT(pstSensorImageMode);
        return HI_FAILURE;
    }

    /* Switch SensorImageMode */
    if ((HI_TRUE == pstSnsState->bInit) && (u8SensorImageMode == pstSnsState->u8ImgMode))
    {
        /* Don't need to switch SensorImageMode */
        return ISP_DO_NOT_NEED_SWITCH_IMAGEMODE;
    }

    pstSnsState->u8ImgMode = u8SensorImageMode;
    pstSnsState->u32FLStd  = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = IMX377_8M_30FPS_10BIT_LINEAR_MODE;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[0] = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;
    pstSnsState->au32FL[1] = g_astImx377ModeTbl[pstSnsState->u8ImgMode].u32VMax;

    memset(&pstSnsState->astRegsInfo[0], 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&pstSnsState->astRegsInfo[1], 0, sizeof(ISP_SNS_REGS_INFO_S));
}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);

    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = imx377_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit = imx377_exit;
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

static HI_S32 imx377_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunImx377BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

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

    IMX377_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    IMX377_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    IMX377_SENSOR_RESET_CTX(ViPipe);
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

    stSnsAttrInfo.eSensorId = IMX377_ID;

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

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(ViPipe, IMX377_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, IMX377_ID);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, IMX377_ID);

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

ISP_SNS_OBJ_S stSnsImx377Obj =
{
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = imx377_standby,
    .pfnRestart             = imx377_restart,
    .pfnMirrorFlip          = HI_NULL,
    .pfnWriteReg            = imx377_write_register,
    .pfnReadReg             = imx377_read_register,
    .pfnSetBusInfo          = imx377_set_bus_info,
    .pfnSetInit             = sensor_set_init
};


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX377_CMOS_H_ */
