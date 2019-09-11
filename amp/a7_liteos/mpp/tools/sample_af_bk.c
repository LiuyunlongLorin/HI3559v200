#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hi_type.h"
#include "mpi_isp.h"

#define BLEND_SHIFT  6
#define ALPHA        64           // 1
#define BELTA        54           // 0.85

static int AFWeight[15][17] =
{
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};


int main(int argc, char *argv[])
{
    HI_U8 u8WdrChn;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32FrmCnt = 0;
    HI_U32 i, j, k;
    HI_U16 u16StatData;
    VI_PIPE ViPipe;
    ISP_AF_STATISTICS_S stIspStatics;
    ISP_STATISTICS_CFG_S stIspStaticsCfg;
    ISP_PUB_ATTR_S stPubattr;
    ISP_FOCUS_ZONE_S stZoneMetrics[WDR_CHN_MAX][AF_ZONE_ROW][AF_ZONE_COLUMN] = {0};

    ISP_FOCUS_STATISTICS_CFG_S  stFocusCfg =
    {
        {1, 17, 15, 3840, 2160, 1, 0, {0, 0, 0, 3840, 2160}, 0, {0x2, 0x4, 0}, {1, 0x9bff}, 0xf0},
        {1, {1, 1, 1}, 15, {188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 220, 8, 14}, {127, 12, 2047} },
        {0, {1, 1, 0}, 2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0 }, {15, 12, 2047} },
        {{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800} },
        {{ -12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047} },
        {4, {0, 0}, {1, 1}, 0}
    };

    char param1, param2 = '0';

    if (argc < 6)
    {
        printf("use like. ./sample_af isp_Pipe c    -> Fv curve\n");
        printf("......... ./sample_af isp_Pipe h 0 fe nc-> h0 blocks\n");
        printf("......... ./sample_af isp_Pipe h 1 fe nc-> h1 blocks\n");
        printf("......... ./sample_af isp_Pipe v 0 fe c -> v0 blocks\n");
        printf("......... ./sample_af isp_Pipe v 1 fe c -> v1 blocks\n");
        printf("......... ./sample_af isp_Pipe y 0 be c -> y  blocks\n");
        printf("......... ./sample_af isp_Pipe y 1 be nc-> hlcnt blocks\n");
        return -1;
    }

    if ((param1 = *argv[2]) != 'c' && !argv[3])
    {
        printf("args was too less!, should be %c + X\n", *argv[1]);
        return -1;
    }
    else if (argv[3])
    {
        param2 = *argv[3];
    }

    if ('0' != *argv[1] && '1' != *argv[1])
    {
        printf("ViPipe num Err!\n");
        return -1;
    }

    ViPipe = atoi(argv[1]);
    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubattr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetPubAttr error!(s32Ret = 0x%x)\n", s32Ret);
        return HI_FAILURE;
    }

    if (strcmp(argv[5], "c") == 0)
    {
        printf("configed\n");
        s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stIspStaticsCfg);
        stFocusCfg.stConfig.u16Vsize = stPubattr.stWndRect.u32Height;
        stFocusCfg.stConfig.u16Hsize = stPubattr.stWndRect.u32Width;
        printf("stFocusCfg.stConfig.u16Vsize=%d\n", stFocusCfg.stConfig.u16Vsize);
        printf("stFocusCfg.stConfig.u16Hsize=%d\n", stFocusCfg.stConfig.u16Hsize);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
            return HI_FAILURE;
        }

        memcpy(&stIspStaticsCfg.stFocusCfg, &stFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));
        s32Ret = HI_MPI_ISP_SetStatisticsConfig(ViPipe, & stIspStaticsCfg);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_SetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        printf("unconfiged\n");
        printf("stPubattr.stWndRect.u32Height=%d\n", stPubattr.stWndRect.u32Height);
        printf("stPubattr.stWndRect.u32Width=%d\n", stPubattr.stWndRect.u32Width);
    }

    while (1)
    {
        s32Ret = HI_MPI_ISP_GetVDTimeOut(ViPipe, ISP_VD_FE_START, 5000);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetVDTimeOut error!(s32Ret = 0x%x)\n", s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_ISP_GetFocusStatistics(ViPipe, &stIspStatics);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetStatistics error!(s32Ret = 0x%x)\n", s32Ret);
            return HI_FAILURE;
        }

        if (strcmp(argv[4], "fe") == 0)
        {
            memcpy(stZoneMetrics, &stIspStatics.stFEAFStat, sizeof(ISP_FOCUS_ZONE_S) * WDR_CHN_MAX * AF_ZONE_ROW * AF_ZONE_COLUMN);
        }
        else
        {
            memcpy(stZoneMetrics[0], &stIspStatics.stBEAFStat, sizeof(ISP_FOCUS_ZONE_S) * AF_ZONE_ROW * AF_ZONE_COLUMN);
        }

        if (param1 != 'c')
        {
            if ((++u32FrmCnt % 30))
            {
                continue;
            }

            u8WdrChn = 1;
            for (k = 0; k < u8WdrChn; k++)
            {
                printf("~~~k(wdr channel)=%d~~~\n", k);
                for (i = 0; i < 15; i++)
                {
                    for (j = 0; j < 17; j++)
                    {
                        if (param1 == 'h' && param2 == '0')
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16h1;
                        }
                        else if (param1 == 'h' && param2 == '1')
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16h2;
                        }
                        else if (param1 == 'v' && param2 == '0')
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16v1;
                        }
                        else if (param1 == 'v' && param2 == '1')
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16v2;
                        }
                        else if (param1 == 'y' && param2 == '0')
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16y;
                        }
                        else
                        {
                            u16StatData = stZoneMetrics[k][i][j].u16HlCnt;
                        }
                        printf("%6d", u16StatData);
                    }
                    printf("\n");
                }
            }
            printf("------------------------------------------------%c-%c----------------------------------------------\n\n\n\n\n", param1, param2);

            continue;
        }

        HI_U32 u32SumFv1 = 0;
        HI_U32 u32SumFv2 = 0;
        HI_U32 u32WgtSum = 0;
        HI_U32 u32Fv1_n, u32Fv2_n, u32Fv1, u32Fv2;

        if ((++u32FrmCnt % 2))
        {
            continue;
        }

        u8WdrChn = 1;
        for (k = 0; k < u8WdrChn; k++)
        {
            for ( i = 0 ; i < stFocusCfg.stConfig.u16Vwnd; i++ )
            {
                for ( j = 0 ; j < stFocusCfg.stConfig.u16Hwnd; j++ )
                {
                    HI_U32 u32H1 = stZoneMetrics[k][i][j].u16h1;
                    HI_U32 u32H2 = stZoneMetrics[k][i][j].u16h2;
                    HI_U32 u32V1 = stZoneMetrics[k][i][j].u16v1;
                    HI_U32 u32V2 = stZoneMetrics[k][i][j].u16v2;

                    u32Fv1_n = (u32H1 * ALPHA + u32V1 * ((1 << BLEND_SHIFT) - ALPHA)) >> BLEND_SHIFT;
                    u32Fv2_n = (u32H2 * BELTA + u32V2 * ((1 << BLEND_SHIFT) - BELTA)) >> BLEND_SHIFT;

                    u32SumFv1 += AFWeight[i][j] * u32Fv1_n;
                    u32SumFv2 += AFWeight[i][j] * u32Fv2_n;
                    u32WgtSum += AFWeight[i][j];
                }
            }
        }

        u32Fv1 = u32SumFv1 / u32WgtSum;
        u32Fv2 = u32SumFv2 / u32WgtSum;

        printf("%4d    %4d\n", u32Fv1, u32Fv2);
    }

    return HI_SUCCESS;
}
