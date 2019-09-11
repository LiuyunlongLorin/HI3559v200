/******************************************************************************

  Copyright (C), 2001-2013, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : os04b_cmos.c
  Version       : Initial Draft
  Author        : Hisilicon BVT ISP group
  Created       : 2017/08/22
  Description   : Sony OS04B sensor driver
  History       :
  1.Date        : 2017/08/22
  Author        : 
  Modification  : Created file

******************************************************************************/

#if !defined(__OS04B_2L_CMOS_H_)
#define __OS04B_2L_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"

#include "os04b_2l_cmos_ex.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define OS04B_2L_ID 04
#define HIGH_8BITS(x) (((x) & 0xff00) >> 8)
#define LOW_8BITS(x)  ((x) & 0x00ff)

#ifndef MAX
#define MAX(a, b) (((a) < (b)) ?  (b) : (a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) > (b)) ?  (b) : (a))
#endif

ISP_SNS_STATE_S *g_pastOs04b_2l[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define OS04B_2L_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastOs04b_2l[dev])
#define OS04B_2L_SENSOR_SET_CTX(dev, pstCtx)   (g_pastOs04b_2l[dev] = pstCtx)
#define OS04B_2L_SENSOR_RESET_CTX(dev)         (g_pastOs04b_2l[dev] = HI_NULL)

ISP_SNS_COMMBUS_U g_aunOs04b_2lBusInfo[ISP_MAX_PIPE_NUM] = 
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

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
extern const unsigned int os04b_2l_i2c_addr;
extern unsigned int os04b_2l_addr_byte;
extern unsigned int os04b_2l_data_byte;

extern void os04b_2l_init(VI_PIPE ViPipe);
extern void os04b_2l_exit(VI_PIPE ViPipe);
extern void os04b_2l_standby(VI_PIPE ViPipe);
extern void os04b_2l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern void os04b_2l_restart(VI_PIPE ViPipe);
extern int os04b_2l_write_register(VI_PIPE ViPipe, int addr, int data);
extern int os04b_2l_read_register(VI_PIPE ViPipe, int addr);

//define 
#define OS04B_2L_FULL_LINES_MAX  (0xFFF2)

//Register define
#define PAGE_SELECT_ADDR	(0xfd)
#define EXP_ADDR_H		    (0x03)
#define EXP_ADDR_L		    (0x04)
#define AGAIN_ADDR		    (0x24)
#define DGAIN_ADDR		    (0x39)
#define VBLANK_H           (0x05)
#define VBLANK_L           (0x06)
#define TRIGGER_ADDR		(0x01)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define OS04B_2L_VMAX_4M30_LINEAR  (1488+INCREASE_LINES)

#define Os04b_2l_4M_2560x1440_10bit_linear30 (0) /* for FPGA */

static HI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_U32 u32Fll = 0;
    HI_FLOAT f32maxFps = 0;

    CMOS_CHECK_POINTER(pstAeSnsDft);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);
	
    memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));
    
    if(Os04b_2l_4M_2560x1440_10bit_linear30 == pstSnsState->u8ImgMode)
    {
        u32Fll = OS04B_2L_VMAX_4M30_LINEAR;
        f32maxFps = 30;
    }
    
    pstSnsState->u32FLStd = u32Fll;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    
    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    
    pstAeSnsDft->u32MaxIntTime = (pstSnsState->u32FLStd >= 8) ? (pstSnsState->u32FLStd - 8) : (0);
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0625;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.125;

    pstAeSnsDft->u32MaxAgain = 15872;
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->u32MaxDgain = 255;
    pstAeSnsDft->u32MinDgain = 8;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 6 << pstAeSnsDft->u32ISPDgainShift;

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
        pstAeSnsDft->u32LinesPer500ms = ((HI_U64)(u32Fll * f32maxFps)) >> 1;
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
            pstAeSnsDft->u8AeCompensation = 0x38;
            pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            break;

            

    }

    return HI_SUCCESS;
}

/* the function of sensor set fps */
static HI_VOID cmos_fps_set(VI_PIPE ViPipe, HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    HI_FLOAT f32maxFps;
    HI_U32 u32Lines;
    HI_U32 u32VBlank;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    switch (pstSnsState->u8ImgMode)
    {
        case Os04b_2l_4M_2560x1440_10bit_linear30:
            if ((f32Fps <= 30) && (f32Fps >= 0.68))
            {
                f32maxFps = 30;//fix to 30fps and dont support fps change
                u32Lines = (OS04B_2L_VMAX_4M30_LINEAR) * f32maxFps / f32Fps;
                u32VBlank = u32Lines - OS04B_2L_VMAX_4M30_LINEAR;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }
            break;

        default:
            printf("Not support this Mode\n");
            return;
            break;
    }

    pstSnsState->u32FLStd = u32Lines;
    pstSnsState->u32FLStd = (pstSnsState->u32FLStd > OS04B_2L_FULL_LINES_MAX) ? OS04B_2L_FULL_LINES_MAX : pstSnsState->u32FLStd;

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;

    pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
    pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
    pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = LOW_8BITS(u32VBlank);
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = HIGH_8BITS(u32VBlank);
    }
    
    return;
}

static HI_VOID cmos_slow_framerate_set(VI_PIPE ViPipe, HI_U32 u32FullLines,
                                       AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32VBlank;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(pstAeSnsDft);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->au32FL[0] = (u32FullLines > OS04B_2L_FULL_LINES_MAX) ? OS04B_2L_FULL_LINES_MAX : u32FullLines;
    pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
    pstAeSnsDft->u32MaxIntTime = pstSnsState->au32FL[0] - 8;
    u32VBlank = pstSnsState->au32FL[0] - OS04B_2L_VMAX_4M30_LINEAR;

    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        pstSnsState->astRegsInfo[0].astI2cData[6].u32Data = LOW_8BITS(u32VBlank);
        pstSnsState->astRegsInfo[0].astI2cData[5].u32Data = HIGH_8BITS(u32VBlank);
    }
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID cmos_inttime_update(VI_PIPE ViPipe, HI_U32 u32IntTime)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    if(u32IntTime <= 2)
	{
		u32IntTime = 2;
	}

    pstSnsState->astRegsInfo[0].astI2cData[1].u32Data = HIGH_8BITS(u32IntTime);
    pstSnsState->astRegsInfo[0].astI2cData[2].u32Data =  LOW_8BITS(u32IntTime);

    return;
}

#define AGAIN_NODE_NUM 64
static HI_U32 au32Again_table[AGAIN_NODE_NUM] =
{
    1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856, 1920, 1984,
    2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968,
    4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936,
    8192, 8704, 9216, 9728, 10240, 10752, 11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848,
    15360, 15872
};

static HI_VOID cmos_again_calc_table(VI_PIPE ViPipe, HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    CMOS_CHECK_POINTER_VOID(pu32AgainLin);
    CMOS_CHECK_POINTER_VOID(pu32AgainDb);

    if (*pu32AgainLin >= au32Again_table[AGAIN_NODE_NUM - 1])
    {
        *pu32AgainLin = au32Again_table[AGAIN_NODE_NUM - 1];
        *pu32AgainDb = AGAIN_NODE_NUM - 1;
        return ;
    }

    for (i = 1; i < AGAIN_NODE_NUM; i++)
    {
        if (*pu32AgainLin < au32Again_table[i])
        {
            *pu32AgainLin = au32Again_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
    return;
}

static HI_VOID cmos_gains_update(VI_PIPE ViPipe, HI_U32 u32Again, HI_U32 u32Dgain)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    HI_U32 temp_gain;

    temp_gain = (HI_U32)(au32Again_table[u32Again]/1024.0*16.0);

    pstSnsState->astRegsInfo[0].astI2cData[3].u32Data = temp_gain;
    pstSnsState->astRegsInfo[0].astI2cData[4].u32Data = u32Dgain;
    
    return;
}

/* Only used in WDR_MODE_2To1_LINE and WDR_MODE_2To1_FRAME mode */
static HI_VOID cmos_get_inttime_max(VI_PIPE ViPipe, HI_U16 u16ManRatioEnable, HI_U32 *au32Ratio, HI_U32 *au32IntTimeMax, HI_U32 *au32IntTimeMin, HI_U32 *pu32LFMaxIntTime)
{
    HI_U32 i = 0;
    HI_U32 u32IntTimeMaxTmp = 0;
    HI_U32 u32IntTimeMaxTmp1 = 0;
    HI_U32 u32RatioTmp = 0x40;
    HI_U32 u32ShortTimeMinLimit = 0;

    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER_VOID(au32Ratio);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMax);
    CMOS_CHECK_POINTER_VOID(au32IntTimeMin);
    CMOS_CHECK_POINTER_VOID(pu32LFMaxIntTime);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    u32ShortTimeMinLimit =  2;


    if ((WDR_MODE_2To1_LINE == pstSnsState->enWDRMode))
    {
        if (ISP_FSWDR_LONG_FRAME_MODE == genFSWDRMode[ViPipe]) /* long frame mode enable */
        {
            if (gu32MaxTimeGetCnt[ViPipe] > 15)
            {
                u32IntTimeMaxTmp = pstSnsState->au32FL[0] - 50 - pstSnsState->au32WDRIntTime[0];
                u32IntTimeMaxTmp1 = pstSnsState->au32FL[0] - 50;
            }
            else
            {
                u32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
                u32IntTimeMaxTmp1 = (pstSnsState->au32FL[0] - 50) * 0x40 / (au32Ratio[0] + 0x40);
            }

            u32IntTimeMaxTmp = (u32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? u32IntTimeMaxTmp : u32IntTimeMaxTmp1;
            gu32MaxTimeGetCnt[ViPipe]++;
        }
        else if (ISP_FSWDR_AUTO_LONG_FRAME_MODE == genFSWDRMode[ViPipe])
        {
            if (pstSnsState->au32WDRIntTime[0] == u32ShortTimeMinLimit && au32Ratio[0] == 0x40)
            {
                if (gu32MaxTimeGetCnt[ViPipe] > 15)
                {
                    u32IntTimeMaxTmp = pstSnsState->au32FL[0] - 50 - pstSnsState->au32WDRIntTime[0];
                    u32IntTimeMaxTmp1 = pstSnsState->au32FL[0] - 50;
                }
                else
                {
                    u32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
                    u32IntTimeMaxTmp1 = (pstSnsState->au32FL[0] - 50) * 0x40 / (au32Ratio[0] + 0x40);
                }

                u32IntTimeMaxTmp = (u32IntTimeMaxTmp < u32IntTimeMaxTmp1) ? u32IntTimeMaxTmp : u32IntTimeMaxTmp1;
                *pu32LFMaxIntTime = u32IntTimeMaxTmp;
                gu32MaxTimeGetCnt[ViPipe]++;
            }
            else
            {
                gu32MaxTimeGetCnt[ViPipe] = 0;
                u32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
                u32IntTimeMaxTmp1 = (pstSnsState->au32FL[0] - 50) * 0x40 / (au32Ratio[0] + 0x40);
                u32IntTimeMaxTmp = (u32IntTimeMaxTmp <  u32IntTimeMaxTmp1) ? u32IntTimeMaxTmp :  u32IntTimeMaxTmp1;
                *pu32LFMaxIntTime = u32IntTimeMaxTmp1;
            }
        }
        else
        {
            u32IntTimeMaxTmp = ((pstSnsState->au32FL[1] - 50 - pstSnsState->au32WDRIntTime[0]) << 6) / DIV_0_TO_1(au32Ratio[0]);
            u32IntTimeMaxTmp1 = (pstSnsState->au32FL[0] - 50) * 0x40 / (au32Ratio[0] + 0x40);
            u32IntTimeMaxTmp = (u32IntTimeMaxTmp <  u32IntTimeMaxTmp1) ? u32IntTimeMaxTmp :  u32IntTimeMaxTmp1;
        }
    }

    if (u32IntTimeMaxTmp >= u32ShortTimeMinLimit)
    {
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
            printf("Manaul ExpRatio is too large!\n");
            return;
        }
        else
        {
            u32IntTimeMaxTmp = u32ShortTimeMinLimit;

            if (WDR_MODE_2To1_LINE == pstSnsState->enWDRMode)
            {
                for (i = 0x40; i <= 0xFFF; i++)
                {
                    if ((u32IntTimeMaxTmp + (u32IntTimeMaxTmp * i >> 6)) > (pstSnsState->au32FL[0] - 50))
                    {
                        u32RatioTmp = i - 1;
                        break;
                    }
                }
                au32IntTimeMax[0] = u32IntTimeMaxTmp;
                au32IntTimeMax[1] = au32IntTimeMax[0] * u32RatioTmp >> 6;
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

/* Only used in FSWDR mode */
static HI_VOID cmos_ae_fswdr_attr_set(VI_PIPE ViPipe, AE_FSWDR_ATTR_S *pstAeFSWDRAttr)
{
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
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;
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
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x1A0;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1E6;
    
    pstAwbSnsDft->as32WbPara[0] = 33;
    pstAwbSnsDft->as32WbPara[1] = 223;
    pstAwbSnsDft->as32WbPara[2] = 0;
    pstAwbSnsDft->as32WbPara[3] = 130289;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -77176;

    pstAwbSnsDft->u16GoldenRgain = GOLDEN_RGAIN;
    pstAwbSnsDft->u16GoldenBgain = GOLDEN_BGAIN;

    switch (pstSnsState->enWDRMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
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
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
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
            pstDef->unKey.bit1Gamma          = 1;
            pstDef->pstGamma                 = &g_stIspGamma;
            pstDef->unKey.bit1Detail         = 1;
            pstDef->pstDetail                = &g_stIspDetail;
#ifdef CONFIG_HI_ISP_CR_SUPPORT
            pstDef->unKey.bit1Ge             = 1;
            pstDef->pstGe                    = &g_stIspGe;
#endif
            pstDef->unKey.bit1AntiFalseColor = 1;
            pstDef->pstAntiFalseColor        = &g_stIspAntiFalseColor;
            pstDef->unKey.bit1Ldci           = 1;
            pstDef->pstLdci                  = &g_stIspLdci;
            pstDef->unKey.bit1Dehaze         = 1;
            pstDef->pstDehaze                = &g_stIspDehaze;
            memcpy(&pstDef->stNoiseCalibration,    &g_stIspNoiseCalibration, sizeof(ISP_CMOS_NOISE_CALIBRATION_S));
            break;
    }
	
	pstDef->stSensorMode.u32SensorID = OS04B_2L_ID;
    pstDef->stSensorMode.u8SensorMode = pstSnsState->u8ImgMode;

    memcpy(&pstDef->stDngColorParam, &g_stDngColorParam, sizeof(ISP_CMOS_DNG_COLORPARAM_S));

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
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
    if (WDR_MODE_NONE == pstSnsState->enWDRMode)
    {
        for (i=0; i<4; i++)
        {
             pstBlackLevel->au16BlackLevel[i] = 256;  /*12bit,0x40*/
        }
    }
    
    return HI_SUCCESS;
}

static HI_VOID cmos_set_pixel_detect(VI_PIPE ViPipe, HI_BOOL bEnable)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        os04b_2l_write_register(ViPipe,0xfd,0x01);//the first page 5fps
        os04b_2l_write_register(ViPipe,0x03,0x0E);//exportion VTS*3 -4
        os04b_2l_write_register(ViPipe,0x04,0x3F);
        os04b_2l_write_register(ViPipe,0x01,0x01);//trigger     
    }
    else /* setup for ISP 'normal mode' */
    {    
        os04b_2l_write_register(ViPipe,0xfd,0x01);//the first page
        os04b_2l_write_register(ViPipe,0x03,0x04);//exportion
        os04b_2l_write_register(ViPipe,0x04,0x48);
        os04b_2l_write_register(ViPipe,0x01,0x01);
        
        pstSnsState->bSyncInit = HI_FALSE ;
    }
    return;
}

static HI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, HI_U8 u8Mode)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    pstSnsState->bSyncInit = HI_FALSE;

    switch (u8Mode)
    {
        case WDR_MODE_NONE:
            pstSnsState->enWDRMode = WDR_MODE_NONE;
            
            pstSnsState->u8ImgMode = Os04b_2l_4M_2560x1440_10bit_linear30;
			pstSnsState->u32FLStd = OS04B_2L_VMAX_4M30_LINEAR;
            printf("linear mode\n");
        break;
        
        default:
            ISP_TRACE(HI_DBG_ERR, "NOT support this mode!\n");
            return HI_FAILURE;
            break;
    }
    pstSnsState->au32FL[0]  = pstSnsState->u32FLStd;
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0] ;
    memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSnsRegsInfo);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    if ((HI_FALSE == pstSnsState->bSyncInit) || (HI_FALSE == pstSnsRegsInfo->bConfig))
    {
        pstSnsState->astRegsInfo[0].enSnsType = ISP_SNS_I2C_TYPE;
        pstSnsState->astRegsInfo[0].unComBus.s8I2cDev = g_aunOs04b_2lBusInfo[ViPipe].s8I2cDev;
        pstSnsState->astRegsInfo[0].u8Cfg2ValidDelayMax = 2;
        pstSnsState->astRegsInfo[0].u32RegNum = 8;
        
        if ((WDR_MODE_2To1_LINE == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME == pstSnsState->enWDRMode) || (WDR_MODE_2To1_FRAME_FULL_RATE == pstSnsState->enWDRMode))
        {
            pstSnsState->astRegsInfo[0].u32RegNum += 8;
        }

        for (i = 0; i < pstSnsState->astRegsInfo[0].u32RegNum; i++)
        {
            pstSnsState->astRegsInfo[0].astI2cData[i].bUpdate = HI_TRUE;
            pstSnsState->astRegsInfo[0].astI2cData[i].u8DevAddr = os04b_2l_i2c_addr;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32AddrByteNum = os04b_2l_addr_byte;
            pstSnsState->astRegsInfo[0].astI2cData[i].u32DataByteNum = os04b_2l_data_byte;
        }
        pstSnsState->astRegsInfo[0].astI2cData[0].u8DelayFrmNum = 0;         //page
        pstSnsState->astRegsInfo[0].astI2cData[0].u32RegAddr    = PAGE_SELECT_ADDR;       
        
        pstSnsState->astRegsInfo[0].astI2cData[1].u8DelayFrmNum = 0;         //exporsion
        pstSnsState->astRegsInfo[0].astI2cData[1].u32RegAddr    = EXP_ADDR_H;
        pstSnsState->astRegsInfo[0].astI2cData[2].u8DelayFrmNum = 0;          //exporsion
        pstSnsState->astRegsInfo[0].astI2cData[2].u32RegAddr    = EXP_ADDR_L;

        pstSnsState->astRegsInfo[0].astI2cData[3].u8DelayFrmNum = 0;          //analog gain
        pstSnsState->astRegsInfo[0].astI2cData[3].u32RegAddr    = AGAIN_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[4].u8DelayFrmNum = 0;          //Digital gain
        pstSnsState->astRegsInfo[0].astI2cData[4].u32RegAddr    = DGAIN_ADDR;

        pstSnsState->astRegsInfo[0].astI2cData[5].u8DelayFrmNum = 0;          //trigger
        pstSnsState->astRegsInfo[0].astI2cData[5].u32RegAddr    = VBLANK_H;
        pstSnsState->astRegsInfo[0].astI2cData[6].u8DelayFrmNum = 0;          //trigger
        pstSnsState->astRegsInfo[0].astI2cData[6].u32RegAddr    = VBLANK_L;

        pstSnsState->astRegsInfo[0].astI2cData[7].u8DelayFrmNum = 0;          //trigger
        pstSnsState->astRegsInfo[0].astI2cData[7].u32RegAddr    = TRIGGER_ADDR;   
        
        pstSnsState->astRegsInfo[0].astI2cData[0].u32Data       = 0x01;       //init the page ,VTS enable, the trigger data
        pstSnsState->astRegsInfo[0].astI2cData[7].u32Data       = 0x01; 

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
        pstSnsState->astRegsInfo[0].astI2cData[0].bUpdate = HI_TRUE; //always refresh the data
        pstSnsState->astRegsInfo[0].astI2cData[7].bUpdate = HI_TRUE;
    }
    
    pstSnsRegsInfo->bConfig = HI_FALSE;
    memcpy(pstSnsRegsInfo, &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    memcpy(&pstSnsState->astRegsInfo[1], &pstSnsState->astRegsInfo[0], sizeof(ISP_SNS_REGS_INFO_S));
    pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

    return HI_SUCCESS;
}

static HI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode;
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    CMOS_CHECK_POINTER(pstSensorImageMode);
    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER(pstSnsState);

    u8SensorImageMode = pstSnsState->u8ImgMode;
    pstSnsState->bSyncInit = HI_FALSE;

    if (HI_NULL == pstSensorImageMode )
    {
        ISP_TRACE(HI_DBG_ERR, "null pointer when set image mode\n");
        return HI_FAILURE;
    }

    if((pstSensorImageMode->u16Width <= 2560)&&(pstSensorImageMode->u16Height <= 1440))
    {
        if(WDR_MODE_NONE == pstSnsState->enWDRMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = Os04b_2l_4M_2560x1440_10bit_linear30;
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
        return HI_FAILURE;
    }
    pstSnsState->u8ImgMode = u8SensorImageMode;
    memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

    return HI_SUCCESS;
}

static HI_VOID sensor_global_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pstSnsState = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pstSnsState);
    CMOS_CHECK_POINTER_VOID(pstSnsState);

    pstSnsState->bInit = HI_FALSE;
    pstSnsState->bSyncInit = HI_FALSE;
    pstSnsState->u8ImgMode = Os04b_2l_4M_2560x1440_10bit_linear30;
    pstSnsState->enWDRMode = WDR_MODE_NONE;
    pstSnsState->u32FLStd = OS04B_2L_VMAX_4M30_LINEAR; 
    pstSnsState->au32FL[0] = OS04B_2L_VMAX_4M30_LINEAR;
    pstSnsState->au32FL[1] = OS04B_2L_VMAX_4M30_LINEAR;
    
    memset(&pstSnsState->astRegsInfo[0], 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&pstSnsState->astRegsInfo[1], 0, sizeof(ISP_SNS_REGS_INFO_S));

}

static HI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    CMOS_CHECK_POINTER(pstSensorExpFunc);

    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = os04b_2l_init;
    pstSensorExpFunc->pfn_cmos_sensor_exit = os04b_2l_exit;
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
static HI_S32 os04b_2l_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
    g_aunOs04b_2lBusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;
    
    return HI_SUCCESS;
}

static HI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

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

    OS04B_2L_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

    return HI_SUCCESS;
}

static HI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
    ISP_SNS_STATE_S *pastSnsStateCtx = HI_NULL;

    OS04B_2L_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
    SENSOR_FREE(pastSnsStateCtx);
    OS04B_2L_SENSOR_RESET_CTX(ViPipe);
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

    stSnsAttrInfo.eSensorId = OS04B_2L_ID;

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

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(ViPipe, OS04B_2L_ID);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, OS04B_2L_ID);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, OS04B_2L_ID);
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

ISP_SNS_OBJ_S stSnsOs04b10_2lObj =
{
    .pfnRegisterCallback    = sensor_register_callback,
    .pfnUnRegisterCallback  = sensor_unregister_callback,
    .pfnStandby             = os04b_2l_standby,
    .pfnRestart             = os04b_2l_restart,
    .pfnMirrorFlip          = os04b_2l_mirror_flip,
    .pfnWriteReg            = os04b_2l_write_register,
    .pfnReadReg             = os04b_2l_read_register,
    .pfnSetBusInfo          = os04b_2l_set_bus_info,
    .pfnSetInit             = sensor_set_init
};


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
