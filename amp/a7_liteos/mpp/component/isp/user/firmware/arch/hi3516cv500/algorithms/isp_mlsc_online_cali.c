/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_mlsc_online_cali.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2018/01/04
  Description   :
  History       :
  1.Date        : 2018/01/04
    Author      :
    Modification: Created file

******************************************************************************/
#include <math.h>
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_ext_config.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define LSC_GRID_ROWS 33
#define LSC_GRID_COLS 33
#define LSC_GRID_POINTS (LSC_GRID_ROWS*LSC_GRID_COLS)
#define LSC_MEASURE_Q 1024
#define WINDOWSIZEX 20
#define WINDOWSIZEY 20
#define RLSC_NODE_NUM 129
#define RLSC_MAX_PIXEL_VALUE ((1<<16) - 1)

#define LSC_MAX_PIXEL_VALUE ((1<<20) - 1)

#define    BPRG    0
#define    BPGR    1
#define    BPGB    2
#define    BPBG    3

typedef struct hi_LSC_Grid_Step_S
{
    HI_U32 au32GridWidthStep[LSC_GRID_COLS];//Former expression: gridWidthStep[];
    HI_U32 au32GridHeightStep[LSC_GRID_ROWS];//Former expression: gridHeightStep[];
} ISP_LSC_Grid_Step_S;

typedef struct LS_DATA
{
    HI_U32 au32R_data[LSC_GRID_POINTS];
    HI_U32 au32Gr_data[LSC_GRID_POINTS];
    HI_U32 au32Gb_data[LSC_GRID_POINTS];
    HI_U32 au32B_data[LSC_GRID_POINTS];
} LS_DATA;

static const HI_FLOAT g_afMaxGainArray[8] =
{
    (HI_FLOAT)1023 / (HI_FLOAT)512,     //1.9
    (HI_FLOAT)1023 / (HI_FLOAT)256,     //2.8
    (HI_FLOAT)1023 / (HI_FLOAT)128,     //3.7
    (HI_FLOAT)1023 / (HI_FLOAT)64,      //4.6
    (HI_FLOAT)1023 / (HI_FLOAT)1024 + 1, //0.10
    (HI_FLOAT)1023 / (HI_FLOAT)512 + 1, //1.9
    (HI_FLOAT)1023 / (HI_FLOAT)256 + 1, //2.8
    (HI_FLOAT)1023 / (HI_FLOAT)128 + 1, //3.7
};

static const HI_U8 bayerPattLUT[4][2][2] =
{
    {{BPRG, BPGR}, {BPGB, BPBG}}, // 0 RGGB
    {{BPGR, BPRG}, {BPBG, BPGB}}, // 1 GRBG
    {{BPGB, BPBG}, {BPRG, BPGR}}, // 2 GBRG
    {{BPBG, BPGB}, {BPGR, BPRG}}  // 3 BGGR
};

typedef struct _RLSC_CALIB_GAIN
{
    HI_U32 r_gain[RLSC_NODE_NUM];
    HI_U32 gr_gain[RLSC_NODE_NUM];
    HI_U32 gb_gain[RLSC_NODE_NUM];
    HI_U32 b_gain[RLSC_NODE_NUM];
} RLSC_CALIB_GAIN;

typedef struct _RLSC_CALIBRATION_CFG_S
{
    HI_U32 u32Height;
    HI_U32 u32Width;
    HI_U32 u32Stride;

    HI_U32 u32BeginPosX;
    HI_U32 u32BeginPosY;
    HI_U32 u32EndPosX;
    HI_U32 u32EndPosY;

    HI_U32 u32WindowSize;
    HI_U32 u32EliminatePct;

    HI_U32 u32BitDepth;
    HI_U32 BayerFormat;

    HI_BOOL bOffsetInEn;
    HI_U32 u32BLCOffsetR;
    HI_U32 u32BLCOffsetGr;
    HI_U32 u32BLCOffsetGb;
    HI_U32 u32BLCOffsetB;

    HI_U32 u32GainScale;
} RLSC_CALIBRATION_CFG_S;

/**
 * [getMaxData description]
 * @param pu32data   [Input] Input array
 * @param length     [Input] array length
 * Get the maxmum data from the input pu32data array
 */
static HI_U32 getMaxData(HI_U32 *pu32data, int length)
{
    int i;

    HI_U32 u32MaxData = 0;
    for (i = 0; i < length; i++)
    {
        if (pu32data[i] > u32MaxData)
        {
            u32MaxData = pu32data[i];
        }
    }
    return u32MaxData;
}

/**
 * [getMinData description]
 * @param pu32data   [Input] Input array
 * @param length     [Input] array length
 * Get the minmum data from the input pu32data array
 */
static HI_U32 getMinData(HI_U32 *pu32data, int length)
{
    int i;

    HI_U32 u32MinData = LSC_MAX_PIXEL_VALUE;
    for (i = 0; i < length; i++)
    {
        if (pu32data[i] < u32MinData)
        {
            u32MinData = pu32data[i];
        }
    }
    return u32MinData;
}

/**
 * [dataEnlarge description]
 * @param u32Data        [Input] Input Data
 * @param u32MaxData     [Input] Input Target value
 * @param u32MeshScale   [Input] Input scale value
 * Generate gain value at each grid point, uniformed by input u32MeshScale
 */
static HI_U32 dataEnlarge(HI_U32 u32Data, HI_U32 u32MaxData, HI_U32 u32MeshScale)
{

    HI_U32 ratio; //Using unsigned 32 bit to present the ratio;
    HI_U32 u32LscQValue;
    if (u32MeshScale < 4)
    {
        u32LscQValue = 1U << (9 - u32MeshScale);
        ratio = (HI_U32)(((HI_FLOAT)u32MaxData / (HI_FLOAT)DIV_0_TO_1(u32Data)) * u32LscQValue);
    }
    else
    {
        u32LscQValue = 1U << (14 - u32MeshScale);
        ratio = (HI_U32)(((HI_FLOAT)u32MaxData / (HI_FLOAT)DIV_0_TO_1(u32Data) - 1) * u32LscQValue);
    }

    return MIN2(ratio, 1023);

}

static HI_U32 rlsc_dataEnlarge(HI_U32 data, HI_U32 maxData, HI_U8 u8Scale)
{
    HI_U32 ratio; //Using unsigned 32 bit to present the ratio;
    HI_U32 u32RlscQValue;
    if (u8Scale < 7)
    {
        u32RlscQValue = 1U << (15 - u8Scale);
        ratio = (HI_U32)(((HI_FLOAT)maxData / (HI_FLOAT)DIV_0_TO_1(data)) * u32RlscQValue);
    }
    else
    {
        u32RlscQValue = 1U << (23 - u8Scale);
        ratio = (HI_U32)(((HI_FLOAT)maxData / (HI_FLOAT)DIV_0_TO_1(data) - 1) * u32RlscQValue);
    }
    return MIN2(ratio, 65535);
}

/**
 * [sort description]
 * @param pu32Array  [Input] Input array
 * @param u32Num     [Input] array length
 * Sort the input array from min to max
 */
static HI_VOID sort(HI_U32 *pu32Array, HI_U32 u32Num)
{
    HI_U32 i, j;
    HI_U32 temp;
    for (i = 0; i < u32Num ; i++)
    {
        for (j = 0; j < (u32Num - 1) - i; j++)
        {
            if (pu32Array[j] > pu32Array[j + 1])
            {
                temp = pu32Array[j];
                pu32Array[j] = pu32Array[j + 1];
                pu32Array[j + 1] = temp;
            }
        }
    }

}

/**
 * [Get_Lsc_Data_Channel description]
 * @param pBayerImg        [Input]  Input image data
 * @param pu32GridData     [Output] MeshLsc Grid data generated
 * @param stLSCCaliCfg     [Input]  Mesh LSC Calibration configure
 * @param pstLscGridStepXY [Input]  Input Grid width&height information
 * @param u16BlcOffset     [Input]  Input BLC value
 * @param u8Value          [Input]  Indicate channel number: 0-R, 1-Gr, 2-Gb, 3-B
 */
static HI_S32 Get_Lsc_Data_Channel(HI_U16 *pBayerImg, HI_U32 *pu32GridData, ISP_MLSC_CALIBRATION_CFG_S *stLSCCaliCfg, ISP_LSC_Grid_Step_S *pstLscGridStepXY, HI_U16 u16BlcOffset, HI_U8 u8Value)
{
    HI_S32 s32Height   = (HI_S32)stLSCCaliCfg->u16DstImgHeight / 2;
    HI_S32 s32Width    = (HI_S32)stLSCCaliCfg->u16DstImgWidth / 2;
    HI_U32 size_x      = WINDOWSIZEX;
    HI_U32 size_y      = WINDOWSIZEY;
    HI_U32 grid_x_size = LSC_GRID_COLS;
    HI_U32 grid_y_size = LSC_GRID_ROWS;
    HI_S32 s32TrueBlcOffset  ;//= (HI_S32)(stLSCCaliCfg->u32BLCOffsetR<<8);
    HI_U32 u32EliminatePctLow = 10;
    HI_U32 u32EliminatePctHigh = 90;
    HI_U32 fullWidth = s32Width << 1;
    HI_U32 u32Stride = fullWidth;
    HI_U32 i, j;

    HI_U32 location_y = 0; //Added
    HI_U32 location_x;

    HI_U32 num, count, numTemp;
    HI_S32 x_start, x_end, y_start, y_end;

    HI_S32 x, y;

    //test
    HI_U32 pu32Data[400] = {0};

    HI_U32 u32Sum;
    HI_U32 u32CoorIndex = 0;
    HI_U32 u32ChnIndex  = 0;
    HI_U32 h, w;

    if (HI_NULL == pBayerImg || HI_NULL == pu32GridData)
    {
        ISP_TRACE(HI_DBG_ERR, "%s: LINE: %d pBayerImg/pls_data is NULL point !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    u32ChnIndex = (stLSCCaliCfg->enBayer & 0x3) ^ u8Value;

    w = u32ChnIndex & 0x1;
    h = (u32ChnIndex >> 1) & 0x1;

    s32TrueBlcOffset  = (HI_S32)(((HI_U32)u16BlcOffset) << 8); //Input BLC will always be 12 bits

    for (i = 0; i < grid_y_size; i++)
    {
        location_y = location_y + pstLscGridStepXY->au32GridHeightStep[i];
        location_x = 0; //Added
        for (j = 0; j < grid_x_size; j++)
        {
            location_x = location_x + pstLscGridStepXY->au32GridWidthStep[j];
            num = 0;

            x_start = location_x - size_x / 2;
            x_end   = location_x + size_x / 2;
            y_start = location_y - size_y / 2;
            y_end   = location_y + size_y / 2;

            if (x_start < 0)
            {
                x_start = 0;
            }
            if (x_end >= s32Width)
            {
                x_end = s32Width - 1;
            }
            if (y_start < 0)
            {
                y_start = 0;
            }
            if (y_end >= s32Height)
            {
                y_end = s32Height - 1;
            }

            for (y = y_start; y < y_end; y++)
            {
                for (x = x_start; x < x_end; x++)
                {
                    u32CoorIndex = 2 * x + 2 * y * u32Stride;
                    pu32Data[num] = (HI_U32)MAX2(0, ((HI_S32)((HI_U32)(pBayerImg[u32CoorIndex + w + h * u32Stride]) << (20 - stLSCCaliCfg->enRawBit)) - s32TrueBlcOffset ));
                    num++;
                }
            }

            sort(pu32Data, num);

            numTemp = 0;
            u32Sum  = 0;

            for (count = num * u32EliminatePctLow / 100; count < num * u32EliminatePctHigh / 100; count++)
            {
                u32Sum  += pu32Data[count];

                numTemp++;
            }

            if (numTemp)
            {
                pu32GridData[ i * grid_x_size + j]    = u32Sum / numTemp;
            }
            else
            {
                ISP_TRACE(HI_DBG_ERR, "Something goes wrong in getLSData()!");
                return HI_FAILURE;
            }
        }
    }

    return HI_SUCCESS;
}

static HI_U32 Get_Point_MeanValue(HI_U16 *pBayerImg, RLSC_CALIBRATION_CFG_S *stRLSCCaliCfg, HI_S32 point_x, HI_S32 point_y, HI_S32 square_radius, HI_U32 window_size, HI_U8 u8Value, HI_S32 s32TrueBlcOffset)
{
    HI_S32 s32Height = (HI_S32)stRLSCCaliCfg->u32Height;
    HI_S32 s32Width = (HI_S32)stRLSCCaliCfg->u32Width;
    HI_S32 x, y;
    HI_S32 x_start, x_end, y_start, y_end;
    HI_S32 diff_x, diff_y;
    HI_S32 begin_x = (HI_S32)stRLSCCaliCfg->u32BeginPosX;
    HI_S32 begin_y = (HI_S32)stRLSCCaliCfg->u32BeginPosY;
    HI_S32 s32SquareRadius;
    HI_U32 mean = 0;
    HI_U32 u32Stride = stRLSCCaliCfg->u32Stride;

    HI_U32 u32CoorIndex = 0;
    HI_U32 u32ChnIndex = 0;
    HI_U32 h, w, j;
    HI_U32 num, count, numTemp, u32Sum;
    HI_U32 u32EliminatePctLow = MIN2(50, stRLSCCaliCfg->u32EliminatePct);
    HI_U32 u32EliminatePctHigh = 100 - u32EliminatePctLow;

    HI_U32 pu32Data[400] = { 0 };

    if (point_x < 0 || point_x >= s32Width || point_y < 0 || point_y >= s32Height)
    {
        return mean;
    }

    x_start = (point_x - window_size / 2);
    x_end = (point_x + window_size / 2);
    y_start = (point_y - window_size / 2);
    y_end = (point_y + window_size / 2);

    if (x_start < 0)
    {
        x_start = 0;
    }
    if (x_end >= s32Width)
    {
        x_end = s32Width - 1;
    }
    if (y_start < 0)
    {
        y_start = 0;
    }
    if (y_end >= s32Height)
    {
        y_end = s32Height - 1;
    }

    u32ChnIndex = (stRLSCCaliCfg->BayerFormat & 0x3) ^ u8Value;
    w = u32ChnIndex & 0x1;
    h = (u32ChnIndex >> 1) & 0x1;
    num = 0;
    for (y = y_start; y < y_end; y++)
    {
        for (x = x_start; x < x_end; x++)
        {
            diff_x = (x > begin_x) ? (x - begin_x) : (begin_x - x);
            diff_y = (y > begin_y) ? (y - begin_y) : (begin_y - y);
            s32SquareRadius = diff_x * diff_x + diff_y * diff_y;
            u32CoorIndex = x + y * u32Stride;
            j = bayerPattLUT[u8Value][y & 0x1][x & 0x1];

            if ((j == u8Value) && (s32SquareRadius <= square_radius))
            {
                pu32Data[num] = (HI_U32)CLIP3(((HI_S32)pBayerImg[u32CoorIndex + h * u32Stride + w] - s32TrueBlcOffset), 0, RLSC_MAX_PIXEL_VALUE);
                num++;
            }
        }
    }
    sort(pu32Data, num);

    numTemp = 0;
    u32Sum = 0;
    for (count = num * u32EliminatePctLow / 100; count < num * u32EliminatePctHigh / 100; count++)
    {
        u32Sum += pu32Data[count];
        numTemp++;
    }
    if (numTemp)
    {
        mean = u32Sum / numTemp;
    }
    else
    {
        printf("Something goes wrong in getLSData()!\n");
        return HI_FAILURE;
    }

    return mean;
}

static HI_S32 Get_Rlsc_Data_Channel(HI_U16 *pBayerImg, HI_U32 *pu32GainData, RLSC_CALIBRATION_CFG_S *stRLSCCaliCfg, HI_U32 u32BlcOffset, HI_U8 u8Value)
{
    if (HI_NULL == pBayerImg || HI_NULL == pu32GainData || HI_NULL == stRLSCCaliCfg)
    {
        printf("%s: LINE: %d pBayerImg/pu32GainData/pstRLSCCaliCfg is NULL pointer !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }
    HI_S32 s32Height = (HI_S32)stRLSCCaliCfg->u32Height;
    HI_S32 s32Width = (HI_S32)stRLSCCaliCfg->u32Width;
    HI_S32 s32TrueBlcOffset;
    HI_S32 diff_x, diff_y;
    HI_S32 s32SquareRadius;
    HI_U32 u32SquareSecSize;
    HI_FLOAT fSin, fCos;

    HI_U32 i;
    HI_U32 center_x, center_y, delta_x, delta_y;
    HI_S32 location_x, location_y;
    HI_S32 last_loc_x, last_loc_y;

    HI_S32 location_x2, location_y2;
    HI_S32 location_x3, location_y3;
    HI_S32 location_x4, location_y4;

    HI_U32 half_window_size = stRLSCCaliCfg->u32WindowSize / 2;
    HI_U32 last_window_size = half_window_size;
    HI_U32 two_nodes_distance;
    HI_U32 mean = 0, temp;
    HI_U32 count = 0;

    diff_x = s32Width / 2;
    diff_y = s32Height / 2;
    center_x = s32Width / 2;
    center_y = s32Height / 2;

    //s32TrueBlcOffset = (stRLSCCaliCfg->bOffsetInEn) ? ((HI_S32)(u32BlcOffset << (20 - stRLSCCaliCfg->u32BitDepth))) : (0);
    if (stRLSCCaliCfg->bOffsetInEn)
    {
        HI_S32 s32BitShift = stRLSCCaliCfg->u32BitDepth - 12;
        HI_U32 u32BitShift = ABS(s32BitShift);

        if (s32BitShift >= 0)
        {
            s32TrueBlcOffset = u32BlcOffset << u32BitShift;
        }
        else
        {
            s32TrueBlcOffset = u32BlcOffset >> u32BitShift;
        }
    }
    else
    {
        s32TrueBlcOffset = 0;
    }

    s32SquareRadius = diff_x * diff_x + diff_y * diff_y;
    if (s32SquareRadius < 131584) //sqrt(129*squareSecSize)-sqrt(128*squareSecSize)=2, squareSecSize = 1028, squareRadius = 1028*128 = 131584
    {
        printf("%s: LINE: %d Radius is too small !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    u32SquareSecSize = s32SquareRadius / (RLSC_NODE_NUM - 1);
    fSin = ((HI_FLOAT)diff_y) / (HI_FLOAT)sqrt((HI_DOUBLE)s32SquareRadius);
    fCos = ((HI_FLOAT)diff_x) / (HI_FLOAT)sqrt((HI_DOUBLE)s32SquareRadius);
    last_loc_x = center_x;
    last_loc_y = center_y;

    pu32GainData[0] = Get_Point_MeanValue(pBayerImg, stRLSCCaliCfg, center_x, center_y, s32SquareRadius, half_window_size * 2, u8Value, s32TrueBlcOffset);
    for (i = 1; i < RLSC_NODE_NUM; i++)
    {
        delta_x = (HI_U32)(fCos * sqrt((HI_FLOAT)(i * u32SquareSecSize)));
        delta_y = (HI_U32)(fSin * sqrt((HI_FLOAT)(i * u32SquareSecSize)));
        location_x = center_x + delta_x;
        location_y = center_y + delta_y;
        two_nodes_distance = (HI_U32)sqrt((HI_FLOAT)(location_x - last_loc_x) * (location_x - last_loc_x) + (HI_FLOAT)(location_y - last_loc_y) * (location_y - last_loc_y));

        if (last_window_size + half_window_size > two_nodes_distance && half_window_size > 3)
        {
            half_window_size--;
        }

        location_x2 = center_x - delta_x;
        location_y2 = center_y + delta_y;
        location_x3 = center_x - delta_x;
        location_y3 = center_y - delta_y;
        location_x4 = center_x + delta_x;
        location_y4 = center_y - delta_y;

        temp = Get_Point_MeanValue(pBayerImg, stRLSCCaliCfg, location_x, location_y, s32SquareRadius, half_window_size * 2, u8Value, s32TrueBlcOffset);
        if (temp)
        {
            mean += temp;
            count++;
        }
        temp = Get_Point_MeanValue(pBayerImg, stRLSCCaliCfg, location_x2, location_y2, s32SquareRadius, half_window_size * 2, u8Value, s32TrueBlcOffset);
        if (temp)
        {
            mean += temp;
            count++;
        }
        temp = Get_Point_MeanValue(pBayerImg, stRLSCCaliCfg, location_x3, location_y3, s32SquareRadius, half_window_size * 2, u8Value, s32TrueBlcOffset);
        if (temp)
        {
            mean += temp;
            count++;
        }
        temp = Get_Point_MeanValue(pBayerImg, stRLSCCaliCfg, location_x4, location_y4, s32SquareRadius, half_window_size * 2, u8Value, s32TrueBlcOffset);
        if (temp)
        {
            mean += temp;
            count++;
        }

        if (!count)
        {
            pu32GainData[i] = 0;
        }
        else
        {
            pu32GainData[i] = mean / count;
        }

        last_window_size = half_window_size;
        last_loc_x = location_x;
        last_loc_y = location_y;
        mean = 0;
        count = 0;
        temp = 0;

    }

    return HI_SUCCESS;
}

/**
 * [getLSData description]
 * @param BayerImg         [Input]  Input image data
 * @param pstLSCGridData   [Output] MeshLsc Grid data generated
 * @param pstLscGridStepXY [Input]  Input Grid width&height information
 * @param pstLSCCaliCfg    [Input]  MeshLsc Calibration results
 */
static HI_S32 getLSData(HI_U16 *BayerImg, LS_DATA *pstLSCGridData, ISP_LSC_Grid_Step_S *pstLscGridStepXY, ISP_MLSC_CALIBRATION_CFG_S *pstLSCCaliCfg)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = Get_Lsc_Data_Channel(BayerImg, pstLSCGridData->au32R_data, pstLSCCaliCfg, pstLscGridStepXY, pstLSCCaliCfg->u16BLCOffsetR, 0);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Lsc_Data_Channel(BayerImg, pstLSCGridData->au32Gr_data, pstLSCCaliCfg, pstLscGridStepXY, pstLSCCaliCfg->u16BLCOffsetGr, 1);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Lsc_Data_Channel(BayerImg, pstLSCGridData->au32Gb_data, pstLSCCaliCfg, pstLscGridStepXY, pstLSCCaliCfg->u16BLCOffsetGb, 2);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Lsc_Data_Channel(BayerImg, pstLSCGridData->au32B_data, pstLSCCaliCfg, pstLscGridStepXY, pstLSCCaliCfg->u16BLCOffsetB, 3);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 getRLSCData(HI_U16 *BayerImg, RLSC_CALIB_GAIN *rlsc_data, RLSC_CALIBRATION_CFG_S *stRLSCCaliCfg)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = Get_Rlsc_Data_Channel(BayerImg, rlsc_data->r_gain, stRLSCCaliCfg, stRLSCCaliCfg->u32BLCOffsetR, 0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Rlsc_Data_Channel(BayerImg, rlsc_data->gr_gain, stRLSCCaliCfg, stRLSCCaliCfg->u32BLCOffsetGr, 1);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Rlsc_Data_Channel(BayerImg, rlsc_data->gb_gain, stRLSCCaliCfg, stRLSCCaliCfg->u32BLCOffsetGb, 2);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    s32Ret = Get_Rlsc_Data_Channel(BayerImg, rlsc_data->b_gain, stRLSCCaliCfg, stRLSCCaliCfg->u32BLCOffsetB, 3);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: LINE: %d Get_Lsc_Data of R Channel failure !\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


/**
 * [lc_normalization description]
 * @param ls_data            [Input]  Input Grid Data
 * @param u32LSCGridPoints   [Input] MeshLsc Calibration results
 * @param u32MeshScale       [Input]  Input meshLsc calibration config
 * This function is used for testing if the input mesh scale is suitable for the current condition
 */
static HI_VOID MeshScaleTest(LS_DATA *ls_data, HI_U32 u32LSCGridPoints, HI_U32 u32MeshScale)
{
    HI_U32 b_max_data, gb_max_data, gr_max_data, r_max_data;
    HI_U32 b_min_data, gb_min_data, gr_min_data, r_min_data;
    HI_FLOAT b_max_gain, gb_max_gain, gr_max_gain, r_max_gain;
    HI_FLOAT max_gain;

    // for b channel
    b_max_data  = getMaxData(ls_data->au32B_data, u32LSCGridPoints);
    // for gb channel
    gb_max_data = getMaxData(ls_data->au32Gb_data, u32LSCGridPoints);
    // for gr channel
    gr_max_data = getMaxData(ls_data->au32Gr_data, u32LSCGridPoints);
    // for r channel
    r_max_data  = getMaxData(ls_data->au32R_data, u32LSCGridPoints);


    // for b channel
    b_min_data  = getMinData(ls_data->au32B_data, u32LSCGridPoints);
    // for gb channel
    gb_min_data = getMinData(ls_data->au32Gb_data, u32LSCGridPoints);
    // for gr channel
    gr_min_data = getMinData(ls_data->au32Gr_data, u32LSCGridPoints);
    // for r channel
    r_min_data  = getMinData(ls_data->au32R_data, u32LSCGridPoints);

    b_max_gain  = (HI_FLOAT)b_max_data  / (HI_FLOAT)b_min_data;
    gb_max_gain = (HI_FLOAT)gb_max_data / (HI_FLOAT)gb_min_data;
    gr_max_gain = (HI_FLOAT)gr_max_data / (HI_FLOAT)gr_min_data;
    r_max_gain  = (HI_FLOAT)r_max_data  / (HI_FLOAT)r_min_data;

    max_gain = MAX2(MAX3(b_max_gain, gb_max_gain, gr_max_gain), r_max_gain);


    if (max_gain > g_afMaxGainArray[u32MeshScale])
    {
        printf("WARNING:\n");
        printf("Max gain = %f\n", max_gain);

        if (u32MeshScale < 4)  //min gain's value is 0
        {
            if ((max_gain > g_afMaxGainArray[0]) && (max_gain <= g_afMaxGainArray[1]))  //(2,4]
            {
                printf("Please set Mesh scale to %d\n", 1);
            }
            else if ((max_gain > g_afMaxGainArray[1]) && (max_gain <= g_afMaxGainArray[2]))//(4,8]
            {
                printf("Please set Mesh scale to %d\n", 2);
            }
            else if ((max_gain > g_afMaxGainArray[2]) && (max_gain <= g_afMaxGainArray[3]))//(8,16]
            {
                printf("Please set Mesh scale to %d\n", 3);
            }
            else//  >16
            {
                printf("Max gain is too large to find accurate mesh scale,please set Mesh Scale to %d to reduce loss\n", 3);
            }
        }
        else   //max gain's value is 1
        {
            if ((max_gain > g_afMaxGainArray[4]) && (max_gain <= g_afMaxGainArray[5]))  //(2,3]
            {
                printf("Please set Mesh scale to %d\n", 5);
            }
            else if ((max_gain > g_afMaxGainArray[5]) && (max_gain <= g_afMaxGainArray[6]))//(3,5]
            {
                printf("Please set Mesh scale to %d\n", 6);
            }
            else if ((max_gain > g_afMaxGainArray[6]) && (max_gain <= g_afMaxGainArray[7]))//(5,9]
            {
                printf("Please set Mesh scale to %d\n", 7);
            }
            else if ((max_gain > g_afMaxGainArray[7]) && (max_gain <= g_afMaxGainArray[3])) //(9,16]
            {
                printf("Please set Mesh scale to %d\n", 3);
            }
            else  //>16
            {
                printf("Max gain is too large to find accurate mesh scale,please set Mesh Scale to %d to reduce loss\n", 3);
            }
        }
    }

    return;
}


/**
 * [lc_normalization description]
 * @param pstLSCGridData     [Input]  Input Grid Data
 * @param pstLSCCaliResult   [Output] MeshLsc Calibration results
 * @param pstLSCCaliCfg      [Input]  Input meshLsc calibration config
 */
static HI_VOID lc_normalization(LS_DATA *pstLSCGridData, ISP_MESH_SHADING_TABLE_S *pstLSCCaliResult, ISP_MLSC_CALIBRATION_CFG_S *pstLSCCaliCfg)
{

    HI_U32 b_max_data, gb_max_data, gr_max_data, r_max_data;
    HI_U32 u32LSCGridPoints;
    HI_U32 i;

    u32LSCGridPoints = LSC_GRID_POINTS;//33*33

    MeshScaleTest(pstLSCGridData, LSC_GRID_POINTS, pstLSCCaliCfg->u32MeshScale);

    //find the Max data of each channel
    // for b channel
    b_max_data  = getMaxData(pstLSCGridData->au32B_data, u32LSCGridPoints);
    // for gb channel
    gb_max_data = getMaxData(pstLSCGridData->au32Gb_data, u32LSCGridPoints);
    // for gr channel
    gr_max_data = getMaxData(pstLSCGridData->au32Gr_data, u32LSCGridPoints);
    // for r channel
    r_max_data  = getMaxData(pstLSCGridData->au32R_data, u32LSCGridPoints);
    //printf("\nb_max_data=%d,gb_max_data=%d,gr_max_data=%d,r_max_data=%d\n",b_max_data,gb_max_data,gr_max_data,r_max_data);
    for ( i = 0 ; i < u32LSCGridPoints ; i++ )
    {
        //normalization process
        pstLSCCaliResult->stLscGainLut.au16BGain[i]  = (HI_U16)(MIN2(dataEnlarge(pstLSCGridData->au32B_data[i], b_max_data, pstLSCCaliCfg->u32MeshScale), LSC_MAX_PIXEL_VALUE));
        pstLSCCaliResult->stLscGainLut.au16GbGain[i] = (HI_U16)(MIN2(dataEnlarge(pstLSCGridData->au32Gb_data[i], gb_max_data, pstLSCCaliCfg->u32MeshScale), LSC_MAX_PIXEL_VALUE));
        pstLSCCaliResult->stLscGainLut.au16GrGain[i] = (HI_U16)(MIN2(dataEnlarge(pstLSCGridData->au32Gr_data[i], gr_max_data, pstLSCCaliCfg->u32MeshScale), LSC_MAX_PIXEL_VALUE));
        pstLSCCaliResult->stLscGainLut.au16RGain[i]  = (HI_U16)(MIN2(dataEnlarge(pstLSCGridData->au32R_data[i], r_max_data, pstLSCCaliCfg->u32MeshScale), LSC_MAX_PIXEL_VALUE));

    }
    pstLSCCaliResult->u8MeshScale = pstLSCCaliCfg->u32MeshScale;

    return;
}

static HI_VOID rlsc_normalization(RLSC_CALIB_GAIN *rlsc_data, RLSC_CALIB_GAIN *rlsc_gain, RLSC_CALIBRATION_CFG_S *stRLSCCaliCfg)
{
    HI_U32 r_max_data, gr_max_data, gb_max_data, b_max_data;
    HI_U32 u32NodeNum;
    HI_U32 i;
    HI_U8  u8Scale = stRLSCCaliCfg->u32GainScale;

    u32NodeNum = RLSC_NODE_NUM;

    r_max_data = getMaxData(rlsc_data->r_gain, u32NodeNum);
    gr_max_data = getMaxData(rlsc_data->gr_gain, u32NodeNum);
    gb_max_data = getMaxData(rlsc_data->gb_gain, u32NodeNum);
    b_max_data = getMaxData(rlsc_data->b_gain, u32NodeNum);

    for (i = 0; i < u32NodeNum; i++)
    {
        rlsc_gain->r_gain[i] = rlsc_dataEnlarge(rlsc_data->r_gain[i], r_max_data, u8Scale);
        rlsc_gain->gr_gain[i] = rlsc_dataEnlarge(rlsc_data->gr_gain[i], gr_max_data, u8Scale);
        rlsc_gain->gb_gain[i] = rlsc_dataEnlarge(rlsc_data->gb_gain[i], gb_max_data, u8Scale);
        rlsc_gain->b_gain[i] = rlsc_dataEnlarge(rlsc_data->b_gain[i], b_max_data, u8Scale);
    }
    return;
}

/**
 * [GeometricGridSizeY description]
 * @param pstLSCCaliCfg      [Input]  Input meshLsc calibration config
 * @param pstLscGridStepXY   [Output]  Grid width&height info
 * @param pstLSCCaliResult   [Output] MeshLsc Calibration results
 */
static HI_VOID GeometricGridSizeX(ISP_MLSC_CALIBRATION_CFG_S *pstLSCCaliCfg, ISP_LSC_Grid_Step_S *pstLscGridStepXY, ISP_MESH_SHADING_TABLE_S *pstLSCCaliResult)
{
    //Parameters Definition
    HI_U32   u32TmpStepX[(LSC_GRID_COLS - 1) / 2]; //Former expression: tmpStepX
    HI_FLOAT f16Rx[(LSC_GRID_COLS - 1) / 2]; //Former expression: rx
    HI_FLOAT f16SumR; //Former expression: sum_r

    HI_U32 i, sum;
    HI_U32 u32HalfGridSizeX = (LSC_GRID_COLS - 1) / 2; //Former expression: u32HalfGridSizeX;
    HI_U32 u32Width, diff; //Former expression: u32Width, diff

    HI_FLOAT f16R1 = 1.0f;
    //Read Image info

    u32Width = pstLSCCaliCfg->u16DstImgWidth / 2;

    //Function start
    //Horizental direction
    f16Rx[0] = 1.0f;
    for ( i = 1 ; i < u32HalfGridSizeX; i++ )
    {
        f16Rx[i] = f16Rx[i - 1] * f16R1;
    }
    f16SumR = 0;
    for ( i = 0 ; i < u32HalfGridSizeX; i++ )
    {
        f16SumR = f16SumR + f16Rx[i];
    }
    for ( i = 0 ; i < u32HalfGridSizeX; i++ )
    {
        u32TmpStepX[i] = (int)(((u32Width / 2 * 1024 / DIV_0_TO_1(f16SumR)) * f16Rx[i] + (1024 >> 1)) / 1024);
    }

    sum = 0;
    for (i = 0; i < u32HalfGridSizeX; i++)
    {
        sum = sum + u32TmpStepX[i];
    }
    if (sum != (u32Width / 2))
    {
        if (sum > (u32Width / 2))
        {
            diff = sum - u32Width / 2;
            for (i = 1; i <= diff; i++)
            {
                u32TmpStepX[u32HalfGridSizeX - i] = u32TmpStepX[u32HalfGridSizeX - i] - 1;
            }
        }
        else
        {
            diff = u32Width / 2 - sum;
            for (i = 1; i <= diff; i++)
            {
                u32TmpStepX[i - 1] = u32TmpStepX[i - 1] + 1;
            }
        }
    }

    //Return the step length value to gridStepWidth and gridStepHeight
    pstLscGridStepXY->au32GridWidthStep[0] = 0;
    for ( i = 1 ; i <= u32HalfGridSizeX; i++ )
    {
        pstLscGridStepXY->au32GridWidthStep[i] = u32TmpStepX[i - 1];
        pstLscGridStepXY->au32GridWidthStep[LSC_GRID_COLS - i] = u32TmpStepX[i - 1];
        pstLSCCaliResult->au16XGridWidth[i - 1] = pstLscGridStepXY->au32GridWidthStep[i];
    }

    return;
}


/**
 * [GeometricGridSizeY description]
 * @param pstLSCCaliCfg      [Input]  Input meshLsc calibration config
 * @param pstLscGridStepXY   [Output]  Grid width&height info
 * @param pstLSCCaliResult   [Output] MeshLsc Calibration results
 */
static HI_VOID GeometricGridSizeY(ISP_MLSC_CALIBRATION_CFG_S *pstLSCCaliCfg, ISP_LSC_Grid_Step_S *pstLscGridStepXY, ISP_MESH_SHADING_TABLE_S *pstLSCCaliResult)
{
    //param definition
    HI_FLOAT f16Ry[(LSC_GRID_ROWS - 1) / 2]; //Former expression: ry
    HI_FLOAT f16SumR; //Former expression: sum_r
    HI_U32   u32TmpStepY[(LSC_GRID_ROWS - 1) / 2]; //Former expression: u32TmpStepY

    HI_U32 j, sum;
    HI_U32 u32HalfGridSizeY = (LSC_GRID_ROWS - 1) / 2; //Former expression: u32HalfGridSizeY
    HI_U32 u32Height, diff;//Former expression: u32Height, diff

    //Read Image info
    u32Height = pstLSCCaliCfg->u16DstImgHeight / 2;

    HI_FLOAT f16R1 = 1.0f;

    //Function start: Horizental direction
    f16Ry[0] = 1.0f;

    //Vertical direction
    for ( j = 1 ; j < u32HalfGridSizeY; j++ )
    {
        f16Ry[j] = f16Ry[j - 1] * f16R1;
    }
    f16SumR = 0;
    for ( j = 0 ; j < u32HalfGridSizeY; j++ )
    {
        f16SumR = f16SumR + f16Ry[j];
    }
    for ( j = 0 ; j < u32HalfGridSizeY; j++ )
    {
        u32TmpStepY[j] = (int)(((u32Height / 2 * 1024 / DIV_0_TO_1(f16SumR)) * f16Ry[j] + (1024 >> 1)) / 1024);
    }

    sum = 0;
    for ( j = 0 ; j < u32HalfGridSizeY; j++ )
    {
        sum = sum + u32TmpStepY[j];
    }
    if (sum != (u32Height / 2))
    {
        if (sum > (u32Height / 2))
        {
            diff = sum - u32Height / 2;
            for (j = 1; j <= diff; j++)
            {
                u32TmpStepY[u32HalfGridSizeY - j] = u32TmpStepY[u32HalfGridSizeY - j] - 1;
            }
        }
        else
        {
            diff = u32Height / 2 - sum;
            for (j = 1; j <= diff; j++)
            {
                u32TmpStepY[j - 1] = u32TmpStepY[j - 1] + 1;
            }
        }
    }

    //Return the step length value to gridStepWidth and gridStepHeight
    pstLscGridStepXY->au32GridHeightStep[0] = 0;
    for ( j = 1 ; j <= u32HalfGridSizeY; j++ )
    {
        pstLscGridStepXY->au32GridHeightStep[j] = u32TmpStepY[j - 1];
        pstLscGridStepXY->au32GridHeightStep[LSC_GRID_ROWS - j] = u32TmpStepY[j - 1];
        pstLSCCaliResult->au16YGridWidth[j - 1] = pstLscGridStepXY->au32GridHeightStep[j];
    }

    return;
}

/**
 * [LSC_GenerateGridInfo description]
 * @param pu16Data           [Input]  Input Raw Data that used for calibration
 * @param pstLSCCaliCfg      [Input]  Input meshLsc calibration config
 * @param pstLscGridStepXY   [Input]  Grid width&height info
 * @param pstLSCCaliResult   [Output] MeshLsc Calibration results
 */
static HI_S32 LSC_GenerateGridInfo(HI_U16 *pu16Data, ISP_MLSC_CALIBRATION_CFG_S *pstLSCCaliCfg, ISP_LSC_Grid_Step_S *pstLscGridStepXY, ISP_MESH_SHADING_TABLE_S *pstLSCCaliResult)
{
    //Pass all the input params to the function
    LS_DATA *pstLSData;
    HI_S32 s32Ret = HI_SUCCESS;

    //memory allocation
    pstLSData = (LS_DATA *)ISP_MALLOC(sizeof(LS_DATA));
    if ( HI_NULL == pstLSData )
    {
        ISP_TRACE(HI_DBG_ERR, "pstLSData allocation fail!\n");
        return HI_FAILURE;
    }

    memset(pstLSData, 0, sizeof(LS_DATA));

    //get Lens Correction Coefficients
    s32Ret = getLSData(pu16Data, pstLSData, pstLscGridStepXY, pstLSCCaliCfg);
    if (s32Ret != HI_SUCCESS)
    {
        ISP_TRACE(HI_DBG_ERR, "There are some errors in getLSData()!\n");
        ISP_FREE(pstLSData);
        return HI_FAILURE;
    }

    lc_normalization(pstLSData, pstLSCCaliResult, pstLSCCaliCfg);

    ISP_FREE(pstLSData);
    return s32Ret;

}

static HI_S32 RLSC_GenerateGainInfo(HI_U16 *pu16Data, RLSC_CALIBRATION_CFG_S *pstRLSCCaliCfg, ISP_MESH_SHADING_TABLE_S *pstMLSCTable)
{
    RLSC_CALIB_GAIN *pstRLSCData;
    RLSC_CALIB_GAIN *pstRLSCGain;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;

    pstRLSCData = (RLSC_CALIB_GAIN *)ISP_MALLOC(sizeof(RLSC_CALIB_GAIN));
    if ( HI_NULL == pstRLSCData )
    {
        ISP_TRACE(HI_DBG_ERR, "pstRLSCData allocation fail!\n");
        return HI_FAILURE;
    }
    pstRLSCGain = (RLSC_CALIB_GAIN *)ISP_MALLOC(sizeof(RLSC_CALIB_GAIN));
    if ( HI_NULL == pstRLSCGain )
    {
        ISP_TRACE(HI_DBG_ERR, "pstRLSCGain allocation fail!\n");
        ISP_FREE(pstRLSCData);
        return HI_FAILURE;
    }
    memset(pstRLSCData, 0, sizeof(RLSC_CALIB_GAIN));
    memset(pstRLSCGain, 0, sizeof(RLSC_CALIB_GAIN));

    s32Ret = getRLSCData(pu16Data, pstRLSCData, pstRLSCCaliCfg);
    if (s32Ret != HI_SUCCESS)
    {
        ISP_TRACE(HI_DBG_ERR, "get RLSC Data fails!\n");
        ISP_FREE(pstRLSCGain);
        ISP_FREE(pstRLSCData);
        return HI_FAILURE;
    }

    rlsc_normalization(pstRLSCData, pstRLSCGain, pstRLSCCaliCfg);

    for (i = 0; i < RLSC_NODE_NUM; i++)
    {
        pstMLSCTable->stBNRLscGainLut.au16RGain[i] = pstRLSCGain->r_gain[i];
        pstMLSCTable->stBNRLscGainLut.au16GrGain[i] = pstRLSCGain->gr_gain[i];
        pstMLSCTable->stBNRLscGainLut.au16GbGain[i] = pstRLSCGain->gb_gain[i];
        pstMLSCTable->stBNRLscGainLut.au16BGain[i] = pstRLSCGain->b_gain[i];
    }

    ISP_FREE(pstRLSCGain);
    ISP_FREE(pstRLSCData);

    return HI_SUCCESS;
}

/**
 * [ISP_MeshShadingCalibration description]
 * @param pu16SrcRaw     [Input]  Input Raw Data that used for calibration
 * @param pstMLSCCaliCfg [Input]  Input meshLsc calibration config
 * @param pstMLSCTable   [Output] MeshLsc Calibration results
 */
HI_S32 ISP_MeshShadingCalibration(HI_U16 *pu16SrcRaw,
                                  ISP_MLSC_CALIBRATION_CFG_S *pstMLSCCaliCfg,
                                  ISP_MESH_SHADING_TABLE_S *pstMLSCTable)
{
    HI_S32 s32Ret;
    HI_S32 i, j;
    HI_U16 *pu16CropRaw = NULL;
    RLSC_CALIBRATION_CFG_S stRLSCCaliCfg;

    ISP_LSC_Grid_Step_S stLscGridStepXY;

    stRLSCCaliCfg.u32Height = pstMLSCCaliCfg->u16DstImgHeight;
    stRLSCCaliCfg.u32Width = pstMLSCCaliCfg->u16DstImgWidth;
    stRLSCCaliCfg.u32Stride = pstMLSCCaliCfg->u16DstImgWidth;
    stRLSCCaliCfg.u32BeginPosX = pstMLSCCaliCfg->u16DstImgWidth / 2;
    stRLSCCaliCfg.u32BeginPosY = pstMLSCCaliCfg->u16DstImgHeight / 2;
    stRLSCCaliCfg.u32EndPosX = pstMLSCCaliCfg->u16DstImgWidth;
    stRLSCCaliCfg.u32EndPosY = pstMLSCCaliCfg->u16DstImgHeight;
    stRLSCCaliCfg.u32WindowSize = 20;
    stRLSCCaliCfg.u32EliminatePct = 10;
    stRLSCCaliCfg.u32GainScale = 3;
    stRLSCCaliCfg.u32BitDepth = pstMLSCCaliCfg->enRawBit;
    stRLSCCaliCfg.BayerFormat = pstMLSCCaliCfg->enBayer;
    stRLSCCaliCfg.bOffsetInEn = HI_TRUE;
    stRLSCCaliCfg.u32BLCOffsetR = pstMLSCCaliCfg->u16BLCOffsetR;
    stRLSCCaliCfg.u32BLCOffsetGr = pstMLSCCaliCfg->u16BLCOffsetGr;
    stRLSCCaliCfg.u32BLCOffsetGb = pstMLSCCaliCfg->u16BLCOffsetGb;
    stRLSCCaliCfg.u32BLCOffsetB = pstMLSCCaliCfg->u16BLCOffsetB;

    pu16CropRaw = (HI_U16*)ISP_MALLOC(sizeof(HI_U16) * pstMLSCCaliCfg->u16DstImgWidth * pstMLSCCaliCfg->u16DstImgHeight);
    if (NULL == pu16CropRaw)
    {
        ISP_TRACE(HI_DBG_ERR, "malloc memory failed!\n");
        return HI_FAILURE;
    }
    memset(pu16CropRaw, 0, sizeof(HI_U16) * pstMLSCCaliCfg->u16DstImgWidth * pstMLSCCaliCfg->u16DstImgHeight);

    for (j = 0; j < pstMLSCCaliCfg->u16DstImgHeight; j++)
    {
        for (i = 0; i < pstMLSCCaliCfg->u16DstImgWidth; i++)
        {
            *(pu16CropRaw + j * pstMLSCCaliCfg->u16DstImgWidth + i) = *(pu16SrcRaw + (j + pstMLSCCaliCfg->u16OffsetY)*pstMLSCCaliCfg->u16ImgWidth + i + pstMLSCCaliCfg->u16OffsetX);
        }
    }

    /*Get Grid info*/
    GeometricGridSizeX(pstMLSCCaliCfg, &stLscGridStepXY, pstMLSCTable);

    GeometricGridSizeY(pstMLSCCaliCfg, &stLscGridStepXY, pstMLSCTable);

    //malloc memory
    //ISP_TRACE(HI_DBG_ERR, "Shading Calibration is processing!!!...\n");
    s32Ret = LSC_GenerateGridInfo(pu16CropRaw, pstMLSCCaliCfg, &stLscGridStepXY, pstMLSCTable);
    if (s32Ret == HI_FAILURE)
    {
        if (NULL != pu16CropRaw)
        {
            free(pu16CropRaw);
        }
        ISP_TRACE(HI_DBG_ERR, "There are some errors in LSC_GenerateGridInfo()!\n");
        return HI_FAILURE;
    }

    s32Ret = RLSC_GenerateGainInfo(pu16CropRaw, &stRLSCCaliCfg, pstMLSCTable);
    if (s32Ret == HI_FAILURE)
    {
        if (NULL != pu16CropRaw)
        {
            free(pu16CropRaw);
        }
        ISP_TRACE(HI_DBG_ERR, "There are some errors in RLSC_GenerateGainInfo()!\n");
        return HI_FAILURE;
    }

    if (NULL != pu16CropRaw)
    {
        free(pu16CropRaw);
    }

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
