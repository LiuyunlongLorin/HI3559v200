/******************************************************************************

  Copyright (C), 2016-2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_awb_calibration.c
  Version       : Initial Draft
  Author        : Hisilicon BVT PQ group
  Created       : 2016/12/15
  Description   :
  History       :
  1.Date        : 2016/12/15
    Author      : h00372898
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "isp_math_utils.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

//#define ROW_START 14
//#define ROW_END 18
//#define COLUMN_START 14
//#define COLUMN_END 18

HI_S32 ISP_GetLightboxGain(VI_PIPE ViPipe, ISP_AWB_Calibration_Gain_S *pstAWBCalibrationGain)
{
    HI_U32 u32Rsum = 0;
    HI_U32 u32Bsum = 0;
    HI_U32 u32Gsum = 0;
    HI_S32 i, j;
    HI_S32 s32ZoneCol;
    HI_S32 s32ZoneRow;
    HI_S32 s32ZoneColStart;
    HI_S32 s32ZoneColEnd;
    HI_S32 s32ZoneRowStart;
    HI_S32 s32ZoneRowEnd;
    HI_S32 s32Ret;
    ISP_WB_STATISTICS_S  *pstWBStat  = NULL;
    ISP_STATISTICS_CFG_S *pstStatCfg = NULL;

    pstWBStat = (ISP_WB_STATISTICS_S *)ISP_MALLOC(sizeof(ISP_WB_STATISTICS_S));
    if (HI_NULL == pstWBStat)
    {
        ISP_TRACE(HI_DBG_ERR, "pstWBStat malloc failure !\n");
        return HI_FAILURE;
    }
    pstStatCfg = (ISP_STATISTICS_CFG_S *)ISP_MALLOC(sizeof(ISP_STATISTICS_CFG_S));
    if (HI_NULL == pstStatCfg)
    {
        ISP_TRACE(HI_DBG_ERR, "pstWBStat malloc failure !\n");
        ISP_FREE(pstWBStat);

        return HI_FAILURE;
    }
    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, pstStatCfg);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_FREE(pstWBStat);
        ISP_FREE(pstStatCfg);

        return HI_FAILURE;
    }
    if (pstStatCfg->stWBCfg.u16ZoneCol * pstStatCfg->stWBCfg.u16ZoneRow < 16)
    {
        ISP_TRACE(HI_DBG_ERR, "Not support zone number less than 16 !\n");
        ISP_FREE(pstWBStat);
        ISP_FREE(pstStatCfg);

        return HI_FAILURE;
    }
    s32Ret = HI_MPI_ISP_GetWBStatistics(ViPipe, pstWBStat);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Get WB statics failed!\n");
        ISP_FREE(pstWBStat);
        ISP_FREE(pstStatCfg);

        return HI_FAILURE;
    }

    s32ZoneCol = pstStatCfg->stWBCfg.u16ZoneCol;
    s32ZoneRow = pstStatCfg->stWBCfg.u16ZoneRow;
    s32ZoneColStart = s32ZoneCol / 2 - 2;
    s32ZoneColEnd   = s32ZoneCol / 2 + 2;
    s32ZoneRowStart = s32ZoneRow / 2 - 2;
    s32ZoneRowEnd   = s32ZoneRow / 2 + 2;

    /*Get_statistics*/
    for (j = s32ZoneRowStart; j < s32ZoneRowEnd; j++)
    {
        for (i = j * s32ZoneCol + s32ZoneColStart; i < j * s32ZoneCol + s32ZoneColEnd; i++)
        {
            u32Rsum += pstWBStat->au16ZoneAvgR[i];
            u32Bsum += pstWBStat->au16ZoneAvgB[i];
            u32Gsum += pstWBStat->au16ZoneAvgG[i];
#if 0
            printf("CurR =%8d, CurB = %8d, CurG = %8d\n", pstStatisticsAttr->stWBStat.stBayerStatistics.au16ZoneAvgR[i], pstStatisticsAttr->stWBStat.stBayerStatistics.au16ZoneAvgB[i], pstStatisticsAttr->stWBStat.stBayerStatistics.au16ZoneAvgG[i]);
#endif
        }
    }

    pstAWBCalibrationGain->u16AvgRgain = (u32Gsum << 8) / DIV_0_TO_1(u32Rsum) ;/* G/R*256 */
    pstAWBCalibrationGain->u16AvgBgain = (u32Gsum << 8) / DIV_0_TO_1(u32Bsum) ;/*avarage for 16 zones*/
#if 0
    printf("u16AvgRgain =%8d, u16AvgBgain = %8d\n", pstAWBCalibrationGain->u16AvgRgain, pstAWBCalibrationGain->u16AvgBgain);
#endif

    ISP_FREE(pstWBStat);
    ISP_FREE(pstStatCfg);

    return HI_SUCCESS;
}


