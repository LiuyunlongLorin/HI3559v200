/************************************************************************
* Copyright (C) 2017 - 2018, Hisilicon Tech. Co., Ltd.
* ALL RIGHTS RESERVED
* FileName: isp_flicker.c
* Description:
*
*************************************************************************/
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#if 1
#define HI_ISP_FLICKER_GROUP            (16)
#define HI_ISP_FLICKER_MINBANDNUM       (3)
#define HI_ISP_FLICKER_MINVALIDBANDPCNT (6)
#define HI_ISP_FLICKER_WAVEDIFF1        (3)
#define HI_ISP_FLICKER_WAVEDIFF2        (4)
#define HI_ISP_FLICKER_PERIOD           (3)
#define HI_ISP_FLICKER_GRCNT            (5)
#define HI_ISP_FLICKER_GBCNT            (5)
#define HI_ISP_FLICKER_OVERTH           (50)
#define HI_ISP_FLICKER_OVERCNTTH        (520560)

typedef struct hiHI_ISP_FLICKER_EXTERN_REG_S
{
    HI_U8   u8MinBandNum;
    HI_U8   u8MinValidBandPcnt;
    HI_U8   u8WaveDiff1;
    HI_U8   u8WaveDiff2;
    HI_U8   u8Period;
    HI_U8   u8GrCnt;
    HI_U8   u8GbCnt;
    HI_S32  s32GbAvgcnt;
    HI_S32  s32GrAvgcnt;
} ISP_FLICKER_EXTERN_REG_S;

typedef struct hiHI_ISP_FLICKER_POINT_ATTR
{
    /*Alloc Mem when Initial*/
    HI_S32 *ps32GrMean;
    HI_S32 *ps32GbMean;
    HI_S32 *ps32GrCros0;
    HI_S32 *ps32GbCros0;
    HI_S32 *ps32GrCnt;
    HI_S32 *ps32GbCnt;
} ISP_FLICKER_POINT_ATTR;

typedef struct hiHI_ISP_FLICKER_STAT_INFO
{
    HI_S32 s32CurFrameDiffSumGr; //s24.0, current Frame Gr Sum
    HI_S32 s32CurFrameDiffSumGb; //s24.0, current Frame Gb Sum

    HI_U32 u32CurFrameAbsSumGr;  //u23.0
    HI_U32 u32CurFrameAbsSumGb;  //u23.0
    HI_U32 u32CurFrameCountOver; //u26.0
    HI_S32 *ps32GrMean;          //s11.0
    HI_S32 *ps32GbMean;          //s11.0
} ISP_FLICKER_STAT_INFO;

typedef struct hiHI_ISP_FLICKER_ATTR
{
    HI_BOOL bEnable;
    HI_BOOL bMalloc;
    HI_U16  u16FckInx;
    HI_U16  u16Height;
    HI_U16  u16OverThr;
    HI_U32  u32OverCntThr;
    HI_U32  u32FrameNum;
    HI_S32  s32PreFrameAvgGr;  //s15.0, previous Frame Gr Avg
    HI_S32  s32PreFrameAvgGb;  //s15.0, previous Frame Gb Avg
    HI_S32  s32CurFrameAvgGr;  //s15.0, previous Frame Gr Avg
    HI_S32  s32CurFrameAvgGb;  //s15.0, previous Frame Gb Avg
    HI_U32  u32CurFrameVarGr;
    HI_U32  u32CurFrameVarGb;
    HI_U32  u32CurFrameCountOver; //u26.0
    HI_S32  s32CurFlicker;        //u1.0,  current frame flicker,1 is flicker,0 is no flicker
    HI_S32  s32RetFlickResult;    //u1.0,  the result,1 is flicker,0 is no flicker
    HI_S32  s32CurFreq;           //u1.0,  current frame flicker ,1 is 50Hz,0 is 60Hz
    HI_S32  s32PreFreq;
    HI_S32  s32RetFreqResult;
    HI_S32  s32FlickerHist;
    HI_S32  s32FreqHist;
    HI_S32 *pS32Mem;

    ISP_FLICKER_EXTERN_REG_S stFckExtReg;
    ISP_FLICKER_POINT_ATTR   stFckPoint;
    ISP_FLICKER_STAT_INFO    stFckStatInfo;
} ISP_FLICKER_ATTR;

ISP_FLICKER_ATTR *g_pastFlickerCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define FLICKER_GET_CTX(dev, pstCtx)   (pstCtx = g_pastFlickerCtx[dev])
#define FLICKER_SET_CTX(dev, pstCtx)   (g_pastFlickerCtx[dev] = pstCtx)
#define FLICKER_RESET_CTX(dev)         (g_pastFlickerCtx[dev] = HI_NULL)

HI_S32 FlickerCtxInit(VI_PIPE ViPipe)
{
    ISP_FLICKER_ATTR *pastFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pastFlickerCtx);

    if (HI_NULL == pastFlickerCtx)
    {
        pastFlickerCtx = (ISP_FLICKER_ATTR *)ISP_MALLOC(sizeof(ISP_FLICKER_ATTR));
        if (HI_NULL == pastFlickerCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] FlickerCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }

        memset(pastFlickerCtx, 0, sizeof(ISP_FLICKER_ATTR));
    }

    FLICKER_SET_CTX(ViPipe, pastFlickerCtx);

    return HI_SUCCESS;
}

HI_VOID FlickerCtxExit(VI_PIPE ViPipe)
{
    ISP_FLICKER_ATTR *pastFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pastFlickerCtx);
    ISP_FREE(pastFlickerCtx);
    FLICKER_RESET_CTX(ViPipe);
}

static HI_VOID FlickerFreeMem(VI_PIPE ViPipe)
{
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);
    ISP_CHECK_POINTER_VOID(pstFlickerCtx);

    if ((HI_TRUE == pstFlickerCtx->bMalloc) && (HI_NULL != pstFlickerCtx->pS32Mem))
    {
        ISP_FREE(pstFlickerCtx->pS32Mem);
        pstFlickerCtx->stFckStatInfo.ps32GrMean = HI_NULL;
        pstFlickerCtx->stFckStatInfo.ps32GbMean = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GrMean    = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GbMean    = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GrCros0   = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GbCros0   = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GrCnt     = HI_NULL;
        pstFlickerCtx->stFckPoint.ps32GbCnt     = HI_NULL;
        pstFlickerCtx->bMalloc                  = HI_FALSE;
    }
}

HI_VOID SetReadFlickerStatKey(VI_PIPE ViPipe, HI_BOOL bReadEn)
{
    HI_U32    u32IsrAccess;

    u32IsrAccess = hi_ext_system_statistics_ctrl_highbit_read(ViPipe);

    if (HI_TRUE == bReadEn)
    {
        u32IsrAccess |= (1 << FLICKER_STAT_KEY_BIT);
    }
    else
    {
        u32IsrAccess &= (~(1 << FLICKER_STAT_KEY_BIT));
    }

    hi_ext_system_statistics_ctrl_highbit_write(ViPipe, u32IsrAccess);
}

static HI_VOID FlickerExtRegsInitialize(VI_PIPE ViPipe)
{
    hi_ext_system_flicker_result_write(ViPipe, 0);
    hi_ext_system_freq_result_write(ViPipe, 0x2);
    hi_ext_system_flicker_min_band_num_write(ViPipe, HI_ISP_FLICKER_MINBANDNUM);
    hi_ext_system_flicker_min_valid_band_pcnt_write(ViPipe, HI_ISP_FLICKER_MINVALIDBANDPCNT);
    hi_ext_system_flicker_wave_diff1_write(ViPipe, HI_ISP_FLICKER_WAVEDIFF1);
    hi_ext_system_flicker_wave_diff2_write(ViPipe, HI_ISP_FLICKER_WAVEDIFF2);
    hi_ext_system_flicker_period_write(ViPipe, HI_ISP_FLICKER_PERIOD);
    hi_ext_system_flicker_gr_cnt_write(ViPipe, HI_ISP_FLICKER_GRCNT);
    hi_ext_system_flicker_gb_cnt_write(ViPipe, HI_ISP_FLICKER_GBCNT);
    hi_ext_system_flicker_over_cnt_thr_write(ViPipe, HI_ISP_FLICKER_OVERCNTTH);
    hi_ext_system_flicker_over_thr_write(ViPipe, HI_ISP_FLICKER_OVERTH);
}

static HI_BOOL GetFlickEnable(VI_PIPE ViPipe)
{
    ISP_CTX_S  *pstIspCtx;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (IS_WDR_MODE(pstIspCtx->u8SnsWDRMode) && \
        ((ISP_MODE_RUNNING_OFFLINE == pstIspCtx->stBlockAttr.enIspRunningMode)  || \
         (ISP_MODE_RUNNING_ONLINE  == pstIspCtx->stBlockAttr.enIspRunningMode)))
    {
        return HI_TRUE;
    }
    else
    {
        return HI_FALSE;
    }
}

static HI_VOID FlickerDynaRegInit(HI_U8 u8WDRMode, ISP_FLICKER_DYNA_CFG_S *pstDynaRegCfg)
{
    pstDynaRegCfg->s16GrAvgPre   = 0;
    pstDynaRegCfg->s16GbAvgPre   = 0;
    pstDynaRegCfg->u16OverThr    = HI_ISP_FLICKER_OVERTH;
    pstDynaRegCfg->bResh         = HI_TRUE;
}

static HI_VOID FlickerRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_BOOL bEnable;
    HI_U8   i;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    bEnable = GetFlickEnable(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        FlickerDynaRegInit(pstIspCtx->u8SnsWDRMode, &pstRegCfg->stAlgRegCfg[i].stFlickerRegCfg.stDynaRegCfg);
        pstRegCfg->stAlgRegCfg[i].stFlickerRegCfg.bEnable = bEnable;
    }

    pstRegCfg->unKey.bit1FlickerCfg = 1;
}

static HI_VOID FlickerReadHWStatInfo(VI_PIPE ViPipe, ISP_STAT_S *pStatInfo)
{
    HI_U32 i;
    HI_U32 u32Val, u32Tmp;
    HI_S32 s32Val, s32Tmp;
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);

    u32Val = pStatInfo->stFlickerStat.au32CurDiffGr[0];//Diff
    s32Val = (u32Val & 0x400000) ? (-((HI_S32)(((~u32Val) & 0x3FFFFF) + 1))) : (HI_S32)(u32Val & 0x3FFFFF);
    pstFlickerCtx->stFckStatInfo.s32CurFrameDiffSumGr = s32Val;

    u32Val = pStatInfo->stFlickerStat.au32CurDiffGb[0];//Diff
    s32Val = (u32Val & 0x400000) ? (-((HI_S32)(((~u32Val) & 0x3FFFFF) + 1))) : (HI_S32)(u32Val & 0x3FFFFF);
    pstFlickerCtx->stFckStatInfo.s32CurFrameDiffSumGb = s32Val;

    pstFlickerCtx->stFckStatInfo.u32CurFrameAbsSumGr = pStatInfo->stFlickerStat.au32CurAbsGr[0];//ABS
    pstFlickerCtx->stFckStatInfo.u32CurFrameAbsSumGb = pStatInfo->stFlickerStat.au32CurAbsGb[0];//ABS

    pstFlickerCtx->s32CurFrameAvgGr = (2 * pstFlickerCtx->stFckStatInfo.s32CurFrameDiffSumGr + (HI_S32)(pstFlickerCtx->u16Height >> 1)) / DIV_0_TO_1((HI_S32)pstFlickerCtx->u16Height);
    pstFlickerCtx->s32CurFrameAvgGb = (2 * pstFlickerCtx->stFckStatInfo.s32CurFrameDiffSumGb + (HI_S32)(pstFlickerCtx->u16Height >> 1)) / DIV_0_TO_1((HI_S32)pstFlickerCtx->u16Height);

    for (i = 0; i < pstFlickerCtx->u16FckInx ; i++)
    {
        u32Val = pStatInfo->stFlickerStat.au32GMean[0][i];
        u32Tmp = (u32Val & 0x3FF800) >> 11;
        s32Tmp = (u32Tmp & 0x400) ? (-((HI_S32)(((~u32Tmp) & 0x3FF) + 1))) : (HI_S32)(u32Tmp & 0x3FF);
        pstFlickerCtx->stFckStatInfo.ps32GrMean[i] = s32Tmp;

        u32Tmp = u32Val & 0X7FF;
        s32Tmp = (u32Tmp & 0x400) ? (-((HI_S32)(((~u32Tmp) & 0x3FF) + 1))) : (HI_S32)(u32Tmp & 0x3FF);
        pstFlickerCtx->stFckStatInfo.ps32GbMean[i] = s32Tmp;
    }

    pstFlickerCtx->u32CurFrameVarGr  = (2 * pstFlickerCtx->stFckStatInfo.u32CurFrameAbsSumGr) / DIV_0_TO_1(pstFlickerCtx->u16Height);
    pstFlickerCtx->u32CurFrameVarGb  = (2 * pstFlickerCtx->stFckStatInfo.u32CurFrameAbsSumGb) / DIV_0_TO_1(pstFlickerCtx->u16Height);
}

static HI_VOID FlickerDetectResult(VI_PIPE ViPipe)
{
    HI_S32  s32GbAvgcnt;
    HI_S32  s32GrAvgcnt;
    ISP_FLICKER_EXTERN_REG_S *pstExtReg     = HI_NULL;
    ISP_FLICKER_ATTR         *pstFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);

    pstExtReg = &pstFlickerCtx->stFckExtReg;

    s32GbAvgcnt = pstExtReg->s32GbAvgcnt;
    s32GrAvgcnt = pstExtReg->s32GrAvgcnt;

    pstFlickerCtx->s32PreFrameAvgGr = pstFlickerCtx->s32CurFrameAvgGr;
    pstFlickerCtx->s32PreFrameAvgGb = pstFlickerCtx->s32CurFrameAvgGb;
    /***********flicker result***********/
    if (pstFlickerCtx->u32FrameNum == 1)
    {
        if (pstFlickerCtx->s32CurFlicker == 1)
        {
            pstFlickerCtx->s32RetFlickResult = 1;
        }
        else
        {
            pstFlickerCtx->s32RetFlickResult = 0;
        }
        pstFlickerCtx->s32FlickerHist = 0;
    }
    else
    {
        if (pstFlickerCtx->s32FlickerHist == pstExtReg->u8Period)
        {
            pstFlickerCtx->s32RetFlickResult = !(pstFlickerCtx->s32RetFlickResult);
            pstFlickerCtx->s32FlickerHist = 0;
        }
        if (pstFlickerCtx->s32CurFlicker == pstFlickerCtx->s32RetFlickResult)
        {
            pstFlickerCtx->s32FlickerHist = 0;
        }
        else
        {
            pstFlickerCtx->s32FlickerHist++;
        }
    }
    /************50Hz/60Hz******************/
    if (pstFlickerCtx->s32CurFlicker == 1)
    {
        if ((s32GrAvgcnt <= pstExtReg->u8GrCnt) && (s32GbAvgcnt <= pstExtReg->u8GbCnt))
        {
            pstFlickerCtx->s32CurFreq = 1;//50Hz
        }
        else
        {
            pstFlickerCtx->s32CurFreq = 0;//60Hz
        }
    }
    else
    {
        pstFlickerCtx->s32CurFreq     = 2;//light off
    }

    if (pstFlickerCtx->u32FrameNum == 1)
    {
        if (pstFlickerCtx->s32CurFreq == 1)
        {
            pstFlickerCtx->s32RetFreqResult = 1;
        }
        else if (pstFlickerCtx->s32CurFreq == 0)
        {
            pstFlickerCtx->s32RetFreqResult = 0;
        }
        else
        {
            pstFlickerCtx->s32RetFreqResult = 2;
        }
        pstFlickerCtx->s32FreqHist = 0;
    }
    else
    {
        if (pstFlickerCtx->s32FreqHist == pstExtReg->u8Period)
        {
            pstFlickerCtx->s32RetFreqResult = pstFlickerCtx->s32PreFreq;
            pstFlickerCtx->s32FreqHist = 0;
        }
        if (pstFlickerCtx->s32CurFreq == pstFlickerCtx->s32RetFreqResult)
        {
            pstFlickerCtx->s32FreqHist = 0;
        }
        else
        {
            pstFlickerCtx->s32FreqHist++;
        }
    }

    pstFlickerCtx->s32PreFreq = pstFlickerCtx->s32CurFreq;
}

static HI_VOID FilckerGetIndex(HI_S32 *pS32Cros0, HI_S32 *s32indx, HI_S32 *pS32Cnt, HI_U16 u16Inx)
{
    HI_U32 i, j;
    HI_S32 s32Cnt = 1;
    HI_S32 s32Tmp1, s32Tmp2;

    *s32indx  = 0;
    j = 1;

    for (i = 0; i < u16Inx; )
    {
        s32Tmp1 = *(pS32Cros0 + i);

        for ( ; j < u16Inx; )
        {
            s32Tmp2 = *(pS32Cros0 + j);

            if (s32Tmp1 == s32Tmp2)
            {
                s32Cnt++;
                j++;
            }
            else
            {
                break;
            }
        }

        if (i == u16Inx - 1)
        {
            *(pS32Cnt + *s32indx) = s32Cnt;
            break;
        }
        else if (j == u16Inx)
        {
            *(pS32Cnt + *s32indx) = s32Cnt;
            i = u16Inx;
        }
        else
        {
            *(pS32Cnt + *s32indx) = s32Cnt;
            s32Cnt = 1;
            i   = j;
            j++;
            (*s32indx)++;
        }
    }
}

static HI_VOID FlickerDetect(VI_PIPE ViPipe)
{
    HI_U16 u16Inx;
    HI_U32 i;
    HI_S32 s32Grindx, s32Gbindx;
    HI_S32 *pS32GrCros0 = HI_NULL, *pS32GbCros0 = HI_NULL, *pS32GrCnt = HI_NULL, *pS32GbCnt = HI_NULL;
    HI_S32 s32GrDiff3cnt, s32GbDiff3cnt, s32GrAvgcnt, s32GbAvgcnt;
    HI_S32 s32GrAvg, s32GbAvg, s32GrPat, s32GbPat, s32GrflickerThd, s32GbflickerThd;

    ISP_FLICKER_EXTERN_REG_S *pstExtReg = HI_NULL;
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);

    pstExtReg = &pstFlickerCtx->stFckExtReg;

    u16Inx = pstFlickerCtx->u16FckInx;

    pS32GrCros0 = pstFlickerCtx->stFckPoint.ps32GrCros0;
    pS32GbCros0 = pstFlickerCtx->stFckPoint.ps32GbCros0;

    pS32GrCnt   = pstFlickerCtx->stFckPoint.ps32GrCnt;
    pS32GbCnt   = pstFlickerCtx->stFckPoint.ps32GbCnt;

    /* previous frame-based mean is the zero-value axis.                */
    /* if group mean value is above this zero-value, it is marked as 1; */
    /* otherwise, it is marked as 0.                                    */
    for (i = 0; i < u16Inx; i++)
    {
        *(pS32GrCros0 + i) = 0;
        *(pS32GbCros0 + i) = 0;

        if (pstFlickerCtx->stFckStatInfo.ps32GrMean[i] >= pstFlickerCtx->s32PreFrameAvgGr)
        {
            *(pS32GrCros0 + i) = 1;
        }

        if (pstFlickerCtx->stFckStatInfo.ps32GbMean[i] >= pstFlickerCtx->s32PreFrameAvgGb)
        {
            *(pS32GbCros0 + i) = 1;
        }
    }

    /* pGr_cnt, pGb_cnt: the width of each wave                              */
    /* pGr_avg, pGb_avg: the average width of wave                           */
    /* Gr_pat, Gb_pat: the total number of valid wave                        */
    /* Gr/Gbflicker_thd: min. number of valid wave to indicate flickering    */
    /* Comparing the wave's avg. width with each wave's width,               */
    /* if the difference is less than 2, this wave is valid flicker wave.    */
    /* We allow one time of difference being 3 (from the video we have had.) */
    s32Grindx  = 0;
    s32Gbindx  = 0;
    FilckerGetIndex(pS32GrCros0, &s32Grindx, pS32GrCnt, u16Inx);
    FilckerGetIndex(pS32GbCros0, &s32Gbindx, pS32GbCnt, u16Inx);

    /**********Gr avg************/
    //remove first and last wave
    s32GrDiff3cnt = 0;
    s32GbDiff3cnt = 0;
    s32GrAvgcnt   = 0;
    s32GbAvgcnt   = 0;
    if (s32Grindx  > pstExtReg->u8MinBandNum && s32Gbindx > pstExtReg->u8MinBandNum)
    {
        s32GrAvg = 0;
        for (i = 1; i < s32Grindx ; i++)
        {
            if ( *(pS32GrCnt + i) > 2 )
            {
                s32GrAvg += *(pS32GrCnt + i);
                s32GrAvgcnt++;
            }
        }
        if (s32GrAvgcnt)
        {
            s32GrAvg = ( s32GrAvg + (s32GrAvgcnt >> 1) ) / DIV_0_TO_1(s32GrAvgcnt);
        }
        else
        {
            s32GrAvg = -1;
        }
        /********* Gb avg *******************/
        s32GbAvg = 0;
        //for (i = 0; i < s32Gbindx + 1; i++)
        for (i = 1; i < s32Gbindx; i++)
        {
            if ( *(pS32GbCnt + i) > 2 )
            {
                s32GbAvg += *(pS32GbCnt + i);
                s32GbAvgcnt++;
            }
        }
        if (s32GbAvgcnt)
        {
            s32GbAvg = ( s32GbAvg + (s32GbAvgcnt >> 1) ) / DIV_0_TO_1(s32GbAvgcnt);
        }
        else
        {
            s32GbAvg = -1;
        }
        /**************end ****************/
        s32GrPat = 0;
        for (i = 1; i < s32Grindx  ; i++)
        {
            if ( abs(s32GrAvg - * (pS32GrCnt + i)) <= pstExtReg->u8WaveDiff1)
            {
                s32GrPat++;
            }
            else if ( abs(s32GrAvg - * (pS32GrCnt + i)) == pstExtReg->u8WaveDiff2)
            {
                s32GrDiff3cnt++;
            }
        }

        s32GbPat = 0;
        for (i = 1; i < s32Gbindx ; i++)
        {
            if (abs(s32GbAvg - * (pS32GbCnt + i)) <= pstExtReg->u8WaveDiff1)
            {
                s32GbPat++;
            }
            else if (abs(s32GbAvg - * (pS32GbCnt + i)) == pstExtReg->u8WaveDiff2)
            {
                s32GbDiff3cnt++;
            }
        }
        if (s32GrDiff3cnt == 1)
        {
            s32GrPat++;
        }

        if (s32GbDiff3cnt == 1)
        {
            s32GbPat++;
        }

        if (s32GrAvg == -1 || s32GbAvg == -1)
        {
            s32GrPat = -1;
            s32GbPat = -1;
        }
    }
    else
    {
        s32GrPat = -1;
        s32GbPat = -1;
    }
    s32GrflickerThd = ((s32Grindx * pstExtReg->u8MinValidBandPcnt) + 5) / 10;
    s32GbflickerThd = ((s32Gbindx * pstExtReg->u8MinValidBandPcnt) + 5) / 10;
    // result
    pstFlickerCtx->s32CurFlicker = 0;
    if ( s32GrPat >= s32GrflickerThd && s32GbPat >= s32GbflickerThd )
    {
        pstFlickerCtx->s32CurFlicker = 1;
    }

    pstExtReg->s32GbAvgcnt  =   s32GbAvgcnt;
    pstExtReg->s32GrAvgcnt  =   s32GrAvgcnt;

    FlickerDetectResult( ViPipe);
}

static HI_S32 FlickerReadExtregs(VI_PIPE ViPipe)
{
    ISP_FLICKER_EXTERN_REG_S *pstExtReg     = HI_NULL;
    ISP_FLICKER_ATTR         *pstFlickerCtx = HI_NULL;

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);

    pstExtReg = &pstFlickerCtx->stFckExtReg;

    pstExtReg->u8MinBandNum       = hi_ext_system_flicker_min_band_num_read(ViPipe);
    pstExtReg->u8MinValidBandPcnt = hi_ext_system_flicker_min_valid_band_pcnt_read(ViPipe);
    pstExtReg->u8WaveDiff1        = hi_ext_system_flicker_wave_diff1_read(ViPipe);
    pstExtReg->u8WaveDiff2        = hi_ext_system_flicker_wave_diff2_read(ViPipe);
    pstExtReg->u8Period           = hi_ext_system_flicker_period_read(ViPipe);
    pstExtReg->u8GrCnt            = hi_ext_system_flicker_gr_cnt_read(ViPipe);
    pstExtReg->u8GbCnt            = hi_ext_system_flicker_gr_cnt_read(ViPipe);
    pstFlickerCtx->u16OverThr     = hi_ext_system_flicker_over_thr_read(ViPipe);
    pstFlickerCtx->u32OverCntThr  = hi_ext_system_flicker_over_cnt_thr_read(ViPipe);
    return HI_SUCCESS;
}

static HI_VOID FlickerInitialize(VI_PIPE ViPipe)
{
    HI_U8  u8MemCnt;
    HI_U32 u32MemSize;
    HI_U16 u16Height;
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;
    ISP_CTX_S        *pstIspCtx     = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);

    u16Height = pstIspCtx->stBlockAttr.stFrameRect.u32Height;

    pstFlickerCtx->u32FrameNum = 0;
    pstFlickerCtx->u16Height   = u16Height;
    pstFlickerCtx->u16FckInx   = u16Height >> 4;

    pstFlickerCtx->s32PreFrameAvgGb = 0;
    pstFlickerCtx->s32PreFrameAvgGr = 0;
    pstFlickerCtx->bEnable          = GetFlickEnable(ViPipe);

    FlickerFreeMem(ViPipe);

    u32MemSize = (u16Height / HI_ISP_FLICKER_GROUP) * sizeof(HI_S32);
    u8MemCnt   = sizeof(ISP_FLICKER_POINT_ATTR) / sizeof(HI_S32);

    if (0 == u32MemSize || 0 == u8MemCnt)
    {
        return;
    }
    pstFlickerCtx->pS32Mem = (HI_S32 *)ISP_MALLOC(u32MemSize * u8MemCnt);

    if (HI_NULL == pstFlickerCtx->pS32Mem)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d]:Alloc Mem for Flicker Failure!\n",ViPipe);
        pstFlickerCtx->bEnable = HI_FALSE;
        pstFlickerCtx->bMalloc = HI_FALSE;
        return;
    }

    pstFlickerCtx->stFckPoint.ps32GrMean  = pstFlickerCtx->pS32Mem;
    pstFlickerCtx->stFckPoint.ps32GbMean  = (HI_S32 *)((HI_U8 *)pstFlickerCtx->stFckPoint.ps32GrMean  + u32MemSize);
    pstFlickerCtx->stFckPoint.ps32GrCros0 = (HI_S32 *)((HI_U8 *)pstFlickerCtx->stFckPoint.ps32GbMean  + u32MemSize);
    pstFlickerCtx->stFckPoint.ps32GbCros0 = (HI_S32 *)((HI_U8 *)pstFlickerCtx->stFckPoint.ps32GrCros0 + u32MemSize);
    pstFlickerCtx->stFckPoint.ps32GrCnt   = (HI_S32 *)((HI_U8 *)pstFlickerCtx->stFckPoint.ps32GbCros0 + u32MemSize);
    pstFlickerCtx->stFckPoint.ps32GbCnt   = (HI_S32 *)((HI_U8 *)pstFlickerCtx->stFckPoint.ps32GrCnt   + u32MemSize);

    memset(pstFlickerCtx->stFckPoint.ps32GrMean,  0, u32MemSize);
    memset(pstFlickerCtx->stFckPoint.ps32GbMean,  0, u32MemSize);
    memset(pstFlickerCtx->stFckPoint.ps32GrCros0, 0, u32MemSize);
    memset(pstFlickerCtx->stFckPoint.ps32GbCros0, 0, u32MemSize);
    memset(pstFlickerCtx->stFckPoint.ps32GrCnt,  -1, u32MemSize);
    memset(pstFlickerCtx->stFckPoint.ps32GbCnt,  -1, u32MemSize);

    pstFlickerCtx->stFckStatInfo.ps32GrMean = pstFlickerCtx->stFckPoint.ps32GrMean;
    pstFlickerCtx->stFckStatInfo.ps32GbMean = pstFlickerCtx->stFckPoint.ps32GbMean;

    pstFlickerCtx->bMalloc = HI_TRUE;
}

HI_S32 ISP_FlickerInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;
    ISP_REG_CFG_S    *pstRegCfg     = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = FlickerCtxInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);
    ISP_CHECK_POINTER(pstFlickerCtx);

    FlickerExtRegsInitialize(ViPipe);
    FlickerRegsInitialize(ViPipe, pstRegCfg);
    FlickerInitialize(ViPipe);

    SetReadFlickerStatKey(ViPipe, pstFlickerCtx->bEnable);

    return HI_SUCCESS;
}

HI_S32 ISP_FlickerRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo, HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    ISP_CTX_S        *pstIspCtx     = HI_NULL;
    ISP_FLICKER_ATTR *pstFlickerCtx = HI_NULL;
    ISP_REG_CFG_S    *pstRegCfg     = (ISP_REG_CFG_S *)pRegCfg;
    ISP_FLICKER_DYNA_CFG_S *pstDynaRegCfg = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    FLICKER_GET_CTX(ViPipe, pstFlickerCtx);
    ISP_CHECK_POINTER(pstFlickerCtx);

    pstFlickerCtx->u32FrameNum = pstIspCtx->u32FrameCnt;

    if (!pstFlickerCtx->bEnable)
    {
        return HI_SUCCESS;
    }

    if (HI_FALSE == pstIspCtx->stLinkage.bStatReady)
    {
        return HI_SUCCESS;
    }
    ISP_CHECK_POINTER(pstFlickerCtx->pS32Mem);
    FlickerReadExtregs(ViPipe);
    FlickerReadHWStatInfo(ViPipe, (ISP_STAT_S *)pStatInfo);

    if (pstFlickerCtx->u32FrameNum)
    {
        FlickerDetect(ViPipe);
    }

    pstIspCtx->stLinkage.bFlicker     = (HI_BOOL)pstFlickerCtx->s32RetFlickResult;
    pstIspCtx->stLinkage.u8Freqresult = (HI_U8)  pstFlickerCtx->s32RetFreqResult;

    hi_ext_system_flicker_result_write(ViPipe, pstFlickerCtx->s32RetFlickResult);
    hi_ext_system_freq_result_write(ViPipe, pstFlickerCtx->s32RetFreqResult);

    pstDynaRegCfg = &pstRegCfg->stAlgRegCfg[0].stFlickerRegCfg.stDynaRegCfg;

    pstDynaRegCfg->u16OverThr    = pstFlickerCtx->u16OverThr;
    pstDynaRegCfg->s16GrAvgPre   = pstFlickerCtx->s32PreFrameAvgGr;
    pstDynaRegCfg->s16GbAvgPre   = pstFlickerCtx->s32PreFrameAvgGb;

    pstRegCfg->unKey.bit1FlickerCfg = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_FlickerCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_FlickerInit(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        case ISP_CHANGE_IMAGE_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            ISP_FlickerInit(ViPipe, (HI_VOID *)&pRegCfg->stRegCfg);
            break;
        default :
            break;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_FlickerExit(VI_PIPE ViPipe)
{
    FlickerFreeMem(ViPipe);
    FlickerCtxExit(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterFlicker(VI_PIPE ViPipe)
{
    ISP_CTX_S      *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs   = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Flicker);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_FLICKER;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_FlickerInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_FlickerRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_FlickerCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_FlickerExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
