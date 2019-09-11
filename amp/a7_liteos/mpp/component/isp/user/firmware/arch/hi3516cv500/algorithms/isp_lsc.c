/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_lsc.c
  Version       : Initial Draft
  Author        :
  Created       : 2016/08/17
  Last Modified :
  Description   : Lens Shading Correction Algorithms
  Function List :
  History       :
  1.Date        : 2016/08/17
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include "isp_alg.h"
#include "isp_ext_config.h"
#include "isp_config.h"
#include "isp_sensor.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define LSC_GAIN_BITWIDTH 10
#define LSC_GAIN_MAX 1023
#define BNR_LSC_DEFAULT_GAIN 4096

static const HI_U16  g_au16MeshGainDef[8] = {512, 256, 128, 64, 0, 0, 0, 0};

typedef struct hiISP_LSC
{
    HI_BOOL bLscEnable;
    HI_BOOL bLscCoefUpdata;
    HI_BOOL bLutUpdate;

    HI_U8  u8MeshScale;
    HI_U16 u16MeshStrength;
    HI_U16 u16MeshWeight;
    HI_U16 au16FirstPointPosX[ISP_STRIPING_MAX_NUM];
    HI_U16 au16CurWidth[ISP_STRIPING_MAX_NUM];
    HI_U16 u16BlkColStart;
    HI_U16 u16BlkColEnd;

    HI_U16 au16DeltaX[HI_ISP_LSC_GRID_COL - 1];
    HI_U16 au16DeltaY[(HI_ISP_LSC_GRID_ROW - 1) / 2];
    HI_U16 au16InvX[HI_ISP_LSC_GRID_COL - 1];
    HI_U16 au16InvY[(HI_ISP_LSC_GRID_ROW - 1) / 2];
} ISP_LSC_S;

ISP_LSC_S *g_pastLscCtx[ISP_MAX_PIPE_NUM] = {HI_NULL};

#define LSC_GET_CTX(dev, pstCtx)   (pstCtx = g_pastLscCtx[dev])
#define LSC_SET_CTX(dev, pstCtx)   (g_pastLscCtx[dev] = pstCtx)
#define LSC_RESET_CTX(dev)         (g_pastLscCtx[dev] = HI_NULL)

static HI_S32 LscCtxInit(VI_PIPE ViPipe)
{
    ISP_LSC_S *pastLscCtx = HI_NULL;

    LSC_GET_CTX(ViPipe, pastLscCtx);

    if (HI_NULL == pastLscCtx)
    {
        pastLscCtx = (ISP_LSC_S *)ISP_MALLOC(sizeof(ISP_LSC_S));
        if (HI_NULL == pastLscCtx)
        {
            ISP_TRACE(HI_DBG_ERR, "Isp[%d] LscCtx malloc memory failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }
    }

    memset(pastLscCtx, 0, sizeof(ISP_LSC_S));

    LSC_SET_CTX(ViPipe, pastLscCtx);

    return HI_SUCCESS;
}

static HI_VOID LscCtxExit(VI_PIPE ViPipe)
{
    ISP_LSC_S *pastLscCtx = HI_NULL;

    LSC_GET_CTX(ViPipe, pastLscCtx);
    ISP_FREE(pastLscCtx);
    LSC_RESET_CTX(ViPipe);
}

static HI_VOID geometricInvSize(ISP_LSC_S *pstLsc)
{
    HI_S32 i;

    for ( i = 0 ; i < (HI_ISP_LSC_GRID_COL - 1); i++ )
    {
        if ( 0 != pstLsc->au16DeltaX[i])
        {
            pstLsc->au16InvX[i] = (4096 * 1024 / pstLsc->au16DeltaX[i] + 512) >> 10;
        }
        else
        {
            pstLsc->au16InvX[i] = 0;
        }
    }

    for ( i = 0 ; i < ((HI_ISP_LSC_GRID_ROW - 1) / 2); i++ )
    {
        if ( 0 != pstLsc->au16DeltaY[i])
        {
            pstLsc->au16InvY[i] = (4096 * 1024 / pstLsc->au16DeltaY[i] + 512) >> 10;
        }
        else
        {
            pstLsc->au16InvY[i] = 0;
        }
    }

    return;
}

static HI_VOID LscGetLutIndex(HI_U8 u8CurBlk, ISP_LSC_S *pstLsc, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U16 u16Width = pstLsc->au16CurWidth[u8CurBlk] >> 1;
    HI_U16 u16FirstPointPosX = pstLsc->au16FirstPointPosX[u8CurBlk] >> 1;
    HI_U16 u16LastPointPosX;
    HI_U16 u16WidthSumOfBlk;
    HI_U16 u16Dis2Right, u16Dis2Left;
    HI_U16 u16BlkColStart;
    HI_U16 u16BlkColEnd;

    u16BlkColStart      = 0;
    u16BlkColEnd        = 0;
    u16WidthSumOfBlk    = pstLsc->au16DeltaX[0];

    while ((u16FirstPointPosX >= u16WidthSumOfBlk) && (u16FirstPointPosX != 0))
    {
        u16BlkColStart++;
        u16WidthSumOfBlk += pstLsc->au16DeltaX[u16BlkColStart];
    }

    u16Dis2Right = u16WidthSumOfBlk - u16FirstPointPosX;
    u16Dis2Left  = pstLsc->au16DeltaX[u16BlkColStart] - u16Dis2Right;

    pstUsrRegCfg->u16WidthOffset = u16Dis2Left;

    u16LastPointPosX = u16FirstPointPosX + u16Width;
    u16BlkColEnd     = u16BlkColStart;
    while (u16LastPointPosX > u16WidthSumOfBlk)
    {
        u16BlkColEnd++;
        u16WidthSumOfBlk += pstLsc->au16DeltaX[u16BlkColEnd];
    }
    u16BlkColEnd += 1;

    pstLsc->u16BlkColStart  = u16BlkColStart;
    pstLsc->u16BlkColEnd    = u16BlkColEnd;

    return;
}

static HI_S32 LscGetGainLutInitialize(VI_PIPE ViPipe, ISP_LSC_S *pstLsc, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U16 i, j;
    HI_U16 u16BlkColEnd;
    HI_U16 u16BlkColStart;
    HI_U16 u16IndexOffset;
    HI_U16 u16SrcIndex;
    HI_U32 u32DefGain;
    ISP_CMOS_DEFAULT_S   *pstSnsDft  = HI_NULL;
    const ISP_CMOS_LSC_S *pstCmosLsc = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    u16BlkColEnd   = pstLsc->u16BlkColEnd;
    u16BlkColStart = pstLsc->u16BlkColStart;

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);

        pstCmosLsc = pstSnsDft->pstLsc;

        for (j = 0; j < HI_ISP_LSC_GRID_ROW; j++)
        {
            for (i = 0; i <= (u16BlkColEnd - u16BlkColStart); i++)
            {
                u16IndexOffset = j * HI_ISP_LSC_GRID_COL;
                u16SrcIndex    = u16IndexOffset  + u16BlkColStart + i;

                pstUsrRegCfg->au32RGain[u16IndexOffset + i]  = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16R_Gain[u16SrcIndex]  << LSC_GAIN_BITWIDTH) + \
                                                                pstCmosLsc->astLscCalibTable[0].au16R_Gain[u16SrcIndex];
                pstUsrRegCfg->au32GrGain[u16IndexOffset + i] = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16Gr_Gain[u16SrcIndex] << LSC_GAIN_BITWIDTH) + \
                                                                pstCmosLsc->astLscCalibTable[0].au16Gr_Gain[u16SrcIndex];
                pstUsrRegCfg->au32GbGain[u16IndexOffset + i] = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16Gb_Gain[u16SrcIndex] << LSC_GAIN_BITWIDTH) + \
                                                                pstCmosLsc->astLscCalibTable[0].au16Gb_Gain[u16SrcIndex];
                pstUsrRegCfg->au32BGain[u16IndexOffset + i]  = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16B_Gain[u16SrcIndex]  << LSC_GAIN_BITWIDTH) + \
                                                                pstCmosLsc->astLscCalibTable[0].au16B_Gain[u16SrcIndex];
            }
        }
    }
    else
    {
        u32DefGain = ((HI_U32)g_au16MeshGainDef[pstLsc->u8MeshScale] << LSC_GAIN_BITWIDTH) + g_au16MeshGainDef[pstLsc->u8MeshScale];

        for (j = 0; j < HI_ISP_LSC_GRID_ROW; j++)
        {
            for (i = 0; i <= (u16BlkColEnd - u16BlkColStart); i++)
            {
                u16IndexOffset = j * HI_ISP_LSC_GRID_COL;

                pstUsrRegCfg->au32RGain[u16IndexOffset + i]   = u32DefGain;
                pstUsrRegCfg->au32GrGain[u16IndexOffset + i]  = u32DefGain;
                pstUsrRegCfg->au32GbGain[u16IndexOffset + i]  = u32DefGain;
                pstUsrRegCfg->au32BGain[u16IndexOffset + i]   = u32DefGain;
            }
        }
    }

    for (i = 0; i < (u16BlkColEnd - u16BlkColStart); i++)
    {
        pstUsrRegCfg->au16DeltaX[i] = pstLsc->au16DeltaX[u16BlkColStart + i];
        pstUsrRegCfg->au16InvX[i]   = pstLsc->au16InvX[u16BlkColStart   + i];
    }

    for (i = 0; i < (HI_ISP_LSC_GRID_ROW - 1) / 2; i++)
    {
        pstUsrRegCfg->au16DeltaY[i] = pstLsc->au16DeltaY[i];
        pstUsrRegCfg->au16InvY[i]   = pstLsc->au16InvY[i];
    }

    return HI_SUCCESS;
}

static HI_VOID LscGetGainLut(VI_PIPE ViPipe, ISP_LSC_S *pstLsc, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U16 i, j;
    HI_U16 u16BlkColEnd;
    HI_U16 u16BlkColStart;
    HI_U16 u16IndexOffset;
    HI_U16 u16SrcIndex;
    HI_U16 r_gain0, r_gain1, gr_gain0, gr_gain1, gb_gain0, gb_gain1, b_gain0, b_gain1;

    u16BlkColEnd   = pstLsc->u16BlkColEnd;
    u16BlkColStart = pstLsc->u16BlkColStart;

    for (j = 0; j < HI_ISP_LSC_GRID_ROW; j++)
    {
        for (i = 0; i <= (u16BlkColEnd - u16BlkColStart); i++)
        {
            u16IndexOffset = j * HI_ISP_LSC_GRID_COL;
            u16SrcIndex    = u16IndexOffset  + u16BlkColStart + i;

            r_gain0  = hi_ext_system_isp_mesh_shading_r_gain0_read(ViPipe, u16SrcIndex);
            r_gain1  = hi_ext_system_isp_mesh_shading_r_gain1_read(ViPipe, u16SrcIndex);

            gr_gain0 = hi_ext_system_isp_mesh_shading_gr_gain0_read(ViPipe, u16SrcIndex);
            gr_gain1 = hi_ext_system_isp_mesh_shading_gr_gain1_read(ViPipe, u16SrcIndex);

            gb_gain0 = hi_ext_system_isp_mesh_shading_gb_gain0_read(ViPipe, u16SrcIndex);
            gb_gain1 = hi_ext_system_isp_mesh_shading_gb_gain1_read(ViPipe, u16SrcIndex);

            b_gain0  = hi_ext_system_isp_mesh_shading_b_gain0_read(ViPipe, u16SrcIndex);
            b_gain1  = hi_ext_system_isp_mesh_shading_b_gain1_read(ViPipe, u16SrcIndex);

            pstUsrRegCfg->au32RGain[u16IndexOffset + i]  = ((HI_U32)r_gain1  << LSC_GAIN_BITWIDTH) + r_gain0;
            pstUsrRegCfg->au32GrGain[u16IndexOffset + i] = ((HI_U32)gr_gain1 << LSC_GAIN_BITWIDTH) + gr_gain0;
            pstUsrRegCfg->au32GbGain[u16IndexOffset + i] = ((HI_U32)gb_gain1 << LSC_GAIN_BITWIDTH) + gb_gain0;
            pstUsrRegCfg->au32BGain[u16IndexOffset + i]  = ((HI_U32)b_gain1  << LSC_GAIN_BITWIDTH) + b_gain0;
        }
    }

    for (i = 0; i < (u16BlkColEnd - u16BlkColStart); i++)
    {
        pstUsrRegCfg->au16DeltaX[i] = pstLsc->au16DeltaX[u16BlkColStart + i];
        pstUsrRegCfg->au16InvX[i]   = pstLsc->au16InvX[u16BlkColStart   + i];
    }

    for (i = 0; i < (HI_ISP_LSC_GRID_ROW - 1) / 2; i++)
    {
        pstUsrRegCfg->au16DeltaY[i] = pstLsc->au16DeltaY[i];
        pstUsrRegCfg->au16InvY[i]   = pstLsc->au16InvY[i];
    }
}

static HI_VOID LscStaticRegsInitialize(HI_U8 i, VI_PIPE ViPipe, ISP_LSC_STATIC_CFG_S *pstLscStaticRegCfg)
{
    pstLscStaticRegCfg->u8WinNumH   = HI_ISP_LSC_GRID_COL - 1;
    pstLscStaticRegCfg->u8WinNumV   = HI_ISP_LSC_GRID_ROW - 1;
    pstLscStaticRegCfg->bStaticResh = HI_TRUE;
}

static HI_VOID LscUsrRegsMultiInitialize(HI_U8 u8CurBlk, VI_PIPE ViPipe, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    ISP_LSC_S  *pstLsc    = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);

    pstUsrRegCfg->u16MeshStr  = pstLsc->u16MeshStrength;
    pstUsrRegCfg->u16Weight   = pstLsc->u16MeshWeight;
    pstUsrRegCfg->u8MeshScale = pstLsc->u8MeshScale;

    LscGetLutIndex(u8CurBlk, pstLsc, pstUsrRegCfg);

    LscGetGainLutInitialize(ViPipe, pstLsc, pstUsrRegCfg);

    pstUsrRegCfg->u32UpdateIndex = 1;

    pstUsrRegCfg->bLutUpdate     = HI_TRUE;
    pstUsrRegCfg->bCoefUpdate    = HI_TRUE;
    pstUsrRegCfg->u8BufId        = 0;
}

static HI_VOID LscUsrRegsSingleInitialize(VI_PIPE ViPipe, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U16     i;
    HI_U32     u32DefGain;
    ISP_LSC_S  *pstLsc    = HI_NULL;

    ISP_CMOS_DEFAULT_S    *pstSnsDft  = HI_NULL;
    const ISP_CMOS_LSC_S  *pstCmosLsc = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    LSC_GET_CTX(ViPipe, pstLsc);

    pstUsrRegCfg->u16MeshStr     = pstLsc->u16MeshStrength;
    pstUsrRegCfg->u16Weight      = pstLsc->u16MeshWeight;
    pstUsrRegCfg->u8MeshScale    = pstLsc->u8MeshScale;
    pstUsrRegCfg->u16WidthOffset = 0;

    memcpy(pstUsrRegCfg->au16DeltaX, pstLsc->au16DeltaX, sizeof(HI_U16) * (HI_ISP_LSC_GRID_COL - 1));
    memcpy(pstUsrRegCfg->au16InvX,   pstLsc->au16InvX,   sizeof(HI_U16) * (HI_ISP_LSC_GRID_COL - 1));

    memcpy(pstUsrRegCfg->au16DeltaY, pstLsc->au16DeltaY, sizeof(HI_U16) * (HI_ISP_LSC_GRID_ROW - 1) / 2);
    memcpy(pstUsrRegCfg->au16InvY,   pstLsc->au16InvY,   sizeof(HI_U16) * (HI_ISP_LSC_GRID_ROW - 1) / 2);

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER_VOID(pstSnsDft->pstLsc);

        pstCmosLsc = pstSnsDft->pstLsc;

        for (i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
        {
            pstUsrRegCfg->au32RGain[i]  = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16R_Gain[i]  << LSC_GAIN_BITWIDTH) + pstCmosLsc->astLscCalibTable[0].au16R_Gain[i];
            pstUsrRegCfg->au32GrGain[i] = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16Gr_Gain[i] << LSC_GAIN_BITWIDTH) + pstCmosLsc->astLscCalibTable[0].au16Gr_Gain[i];
            pstUsrRegCfg->au32GbGain[i] = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16Gb_Gain[i] << LSC_GAIN_BITWIDTH) + pstCmosLsc->astLscCalibTable[0].au16Gb_Gain[i];
            pstUsrRegCfg->au32BGain[i]  = ((HI_U32)pstCmosLsc->astLscCalibTable[1].au16B_Gain[i]  << LSC_GAIN_BITWIDTH) + pstCmosLsc->astLscCalibTable[0].au16B_Gain[i];
        }
    }
    else
    {
        u32DefGain = ((HI_U32)g_au16MeshGainDef[pstLsc->u8MeshScale] << LSC_GAIN_BITWIDTH) + g_au16MeshGainDef[pstLsc->u8MeshScale];

        for (i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
        {
            pstUsrRegCfg->au32RGain[i]  = u32DefGain;
            pstUsrRegCfg->au32GrGain[i] = u32DefGain;
            pstUsrRegCfg->au32GbGain[i] = u32DefGain;
            pstUsrRegCfg->au32BGain[i]  = u32DefGain;
        }
    }

    pstUsrRegCfg->u32UpdateIndex = 1;

    pstUsrRegCfg->bLutUpdate           = HI_TRUE;
    pstUsrRegCfg->bCoefUpdate          = HI_TRUE;
    pstUsrRegCfg->u8BufId              = 0;
    pstUsrRegCfg->bSwitchLut2SttRegNew = HI_FALSE;
    pstUsrRegCfg->u8SwitchRegNewCnt    = 0;
}

static HI_VOID LscRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_S32 i;
    ISP_LSC_S *pstLsc    = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);

    if (1 == pstRegCfg->u8CfgNum)
    {
        LscStaticRegsInitialize(0, ViPipe, &pstRegCfg->stAlgRegCfg[0].stLscRegCfg.stStaticRegCfg);
        LscUsrRegsSingleInitialize(ViPipe, &pstRegCfg->stAlgRegCfg[0].stLscRegCfg.stUsrRegCfg);

        pstRegCfg->stAlgRegCfg[0].stLscRegCfg.bLscEn             = pstLsc->bLscEnable;
        pstRegCfg->stAlgRegCfg[0].stLscRegCfg.bLut2SttEn         = HI_TRUE;
        pstRegCfg->stAlgRegCfg[0].stBeLutUpdateCfg.bLscLutUpdate = HI_TRUE;
    }
    else
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            LscStaticRegsInitialize(i, ViPipe, &pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stStaticRegCfg);
            LscUsrRegsMultiInitialize(i, ViPipe, &pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg);

            pstRegCfg->stAlgRegCfg[i].stLscRegCfg.bLscEn             = pstLsc->bLscEnable;
            pstRegCfg->stAlgRegCfg[i].stLscRegCfg.bLut2SttEn         = HI_TRUE;
            pstRegCfg->stAlgRegCfg[i].stBeLutUpdateCfg.bLscLutUpdate = HI_TRUE;
        }
    }

    pstRegCfg->unKey.bit1LscCfg    = 1;
}

static HI_VOID BNRLscStaticRegsInitialize(VI_PIPE ViPipe, ISP_RLSC_STATIC_CFG_S *pstStaticRegCfg)
{
    pstStaticRegCfg->u16NodeNum  = HI_ISP_RLSC_POINTS;
    pstStaticRegCfg->bStaticResh = HI_TRUE;
    return;
}

static HI_S32 BNRLscUsrRegsInitialize(HI_U8 u8CurBlk, VI_PIPE ViPipe, ISP_RLSC_USR_CFG_S *pstUsrRegCfg)
{
    ISP_CTX_S  *pstIspCtx;
    ISP_RECT_S  stBlockRect;
    ISP_CMOS_DEFAULT_S    *pstSnsDft  = HI_NULL;
    const ISP_CMOS_LSC_S  *pstCmosLsc = HI_NULL;

    HI_U8  i;
    HI_U8  u8IndexR, u8IndexGr, u8IndexGb, u8IndexB;
    HI_U32 u32Width, u32Height;
    HI_U32 u32OffCenter;
    const HI_U32 temp = (1U << 31);

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    ISP_GetBlockRect(&stBlockRect, &pstIspCtx->stBlockAttr, u8CurBlk);

    u32Width  = pstIspCtx->stBlockAttr.stFrameRect.u32Width;
    u32Height = pstIspCtx->stBlockAttr.stFrameRect.u32Height;

    pstUsrRegCfg->u16WidthOffset = ABS(stBlockRect.s32X);//TODO: Fill in the exact bias value.!!!important in SBS mode
    pstUsrRegCfg->u8GainScale = HI_ISP_RLSC_DEFAULT_SCALE;

    pstUsrRegCfg->u16CenterRX  = u32Width  / 2;
    pstUsrRegCfg->u16CenterRY  = u32Height / 2;
    pstUsrRegCfg->u16CenterGrX = u32Width  / 2;
    pstUsrRegCfg->u16CenterGrY = u32Height / 2;
    pstUsrRegCfg->u16CenterGbX = u32Width  / 2;
    pstUsrRegCfg->u16CenterGbY = u32Height / 2;
    pstUsrRegCfg->u16CenterBX  = u32Width  / 2;
    pstUsrRegCfg->u16CenterBY  = u32Height / 2;

    u32OffCenter = temp / DIV_0_TO_1(u32Width * u32Width / 4 + u32Height * u32Height / 4);

    pstUsrRegCfg->u16OffCenterR  = u32OffCenter;
    pstUsrRegCfg->u16OffCenterGr = u32OffCenter;
    pstUsrRegCfg->u16OffCenterGb = u32OffCenter;
    pstUsrRegCfg->u16OffCenterB  = u32OffCenter;

    u8IndexR  = pstIspCtx->enBayer ^ 0x0;
    u8IndexGr = pstIspCtx->enBayer ^ 0x1;
    u8IndexGb = pstIspCtx->enBayer ^ 0x2;
    u8IndexB  = pstIspCtx->enBayer ^ 0x3;

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);
        pstCmosLsc = pstSnsDft->pstLsc;

        pstUsrRegCfg->u16GainStr = pstCmosLsc->u16MeshStrength;
        memcpy(pstUsrRegCfg->au16GainLut[u8IndexR],  pstCmosLsc->stBNRLscCalibTable.au16R_Gain,  HI_ISP_RLSC_POINTS * sizeof(HI_U16));
        memcpy(pstUsrRegCfg->au16GainLut[u8IndexGr], pstCmosLsc->stBNRLscCalibTable.au16Gr_Gain, HI_ISP_RLSC_POINTS * sizeof(HI_U16));
        memcpy(pstUsrRegCfg->au16GainLut[u8IndexGb], pstCmosLsc->stBNRLscCalibTable.au16Gb_Gain, HI_ISP_RLSC_POINTS * sizeof(HI_U16));
        memcpy(pstUsrRegCfg->au16GainLut[u8IndexB],  pstCmosLsc->stBNRLscCalibTable.au16B_Gain,  HI_ISP_RLSC_POINTS * sizeof(HI_U16));
    }
    else
    {
        pstUsrRegCfg->u16GainStr  = HI_ISP_RLSC_DEFAULT_RADIAL_STR;
        for ( i = 0 ; i < HI_ISP_RLSC_POINTS ; i++ )
        {
            pstUsrRegCfg->au16GainLut[u8IndexR][i]  = BNR_LSC_DEFAULT_GAIN;
            pstUsrRegCfg->au16GainLut[u8IndexGr][i] = BNR_LSC_DEFAULT_GAIN;
            pstUsrRegCfg->au16GainLut[u8IndexGb][i] = BNR_LSC_DEFAULT_GAIN;
            pstUsrRegCfg->au16GainLut[u8IndexB][i]  = BNR_LSC_DEFAULT_GAIN;
        }
    }

    pstUsrRegCfg->au16GainLut[u8IndexR][HI_ISP_RLSC_POINTS]  = pstUsrRegCfg->au16GainLut[u8IndexR][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexGr][HI_ISP_RLSC_POINTS] = pstUsrRegCfg->au16GainLut[u8IndexGr][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexGb][HI_ISP_RLSC_POINTS] = pstUsrRegCfg->au16GainLut[u8IndexGb][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexB][HI_ISP_RLSC_POINTS]  = pstUsrRegCfg->au16GainLut[u8IndexB][HI_ISP_RLSC_POINTS - 1];

    pstUsrRegCfg->u32UpdateIndex        = 1;
    pstUsrRegCfg->bLutUpdate            = HI_TRUE;
    pstUsrRegCfg->bCoefUpdate           = HI_TRUE;
    pstUsrRegCfg->bUsrResh              = HI_TRUE;
    pstUsrRegCfg->u8BufId               = 0;
    pstUsrRegCfg->u8SwitchRegNewCnt     = 0;
    pstUsrRegCfg->bSwitchLut2SttRegNew  = HI_FALSE;

    return HI_SUCCESS;
}

static HI_VOID BNRLscRegsInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_S32 i;
    ISP_LSC_S *pstLsc    = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        BNRLscStaticRegsInitialize(ViPipe, &pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stStaticRegCfg);
        BNRLscUsrRegsInitialize(i, ViPipe, &pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg);

        pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.bRLscEn = pstLsc->bLscEnable;
    }

    pstRegCfg->unKey.bit1RLscCfg = 1;
}
static HI_S32 BNRLscExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U16 i;
    ISP_CMOS_DEFAULT_S   *pstSnsDft  = HI_NULL;
    const ISP_CMOS_LSC_S *pstCmosLsc = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);
        pstCmosLsc = pstSnsDft->pstLsc;
        for ( i = 0 ; i < HI_ISP_RLSC_POINTS ; i++ )
        {
            hi_ext_system_isp_bnr_shading_r_gain_write(ViPipe,  i, pstCmosLsc->stBNRLscCalibTable.au16R_Gain[i]);
            hi_ext_system_isp_bnr_shading_gr_gain_write(ViPipe, i, pstCmosLsc->stBNRLscCalibTable.au16Gr_Gain[i]);
            hi_ext_system_isp_bnr_shading_gb_gain_write(ViPipe, i, pstCmosLsc->stBNRLscCalibTable.au16Gb_Gain[i]);
            hi_ext_system_isp_bnr_shading_b_gain_write(ViPipe,  i, pstCmosLsc->stBNRLscCalibTable.au16B_Gain[i]);
        }
    }
    else
    {
        for ( i = 0 ; i < HI_ISP_RLSC_POINTS ; i++ )
        {
            hi_ext_system_isp_bnr_shading_r_gain_write(ViPipe, i,  BNR_LSC_DEFAULT_GAIN);
            hi_ext_system_isp_bnr_shading_gr_gain_write(ViPipe, i, BNR_LSC_DEFAULT_GAIN);
            hi_ext_system_isp_bnr_shading_gb_gain_write(ViPipe, i, BNR_LSC_DEFAULT_GAIN);
            hi_ext_system_isp_bnr_shading_b_gain_write(ViPipe, i,  BNR_LSC_DEFAULT_GAIN);
        }
    }
    return HI_SUCCESS;
}

static HI_S32 LscExtRegsInitialize(VI_PIPE ViPipe)
{
    HI_U16 i;
    ISP_LSC_S            *pstLsc     = HI_NULL;
    ISP_CMOS_DEFAULT_S   *pstSnsDft  = HI_NULL;
    const ISP_CMOS_LSC_S *pstCmosLsc = HI_NULL;

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    LSC_GET_CTX(ViPipe, pstLsc);

    hi_ext_system_isp_mesh_shading_enable_write(ViPipe, pstLsc->bLscEnable);
    hi_ext_system_isp_mesh_shading_attr_updata_write(ViPipe, HI_FALSE);
    hi_ext_system_isp_mesh_shading_lut_attr_updata_write(ViPipe, HI_FALSE);
    hi_ext_system_isp_mesh_shading_mesh_strength_write(ViPipe, pstLsc->u16MeshStrength);
    hi_ext_system_isp_mesh_shading_blendratio_write(ViPipe, pstLsc->u16MeshWeight);
    hi_ext_system_isp_mesh_shading_mesh_scale_write(ViPipe, pstLsc->u8MeshScale);

    for (i = 0; i < (HI_ISP_LSC_GRID_COL - 1) / 2; i++)
    {
        hi_ext_system_isp_mesh_shading_xgrid_write(ViPipe, i, pstLsc->au16DeltaX[i]);
    }

    for (i = 0; i < (HI_ISP_LSC_GRID_ROW - 1) / 2; i++)
    {
        hi_ext_system_isp_mesh_shading_ygrid_write(ViPipe, i, pstLsc->au16DeltaY[i]);
    }

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);

        pstCmosLsc = pstSnsDft->pstLsc;

        for (i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
        {
            hi_ext_system_isp_mesh_shading_r_gain0_write(ViPipe,  i, pstCmosLsc->astLscCalibTable[0].au16R_Gain[i] );
            hi_ext_system_isp_mesh_shading_gr_gain0_write(ViPipe, i, pstCmosLsc->astLscCalibTable[0].au16Gr_Gain[i]);
            hi_ext_system_isp_mesh_shading_gb_gain0_write(ViPipe, i, pstCmosLsc->astLscCalibTable[0].au16Gb_Gain[i]);
            hi_ext_system_isp_mesh_shading_b_gain0_write(ViPipe,  i, pstCmosLsc->astLscCalibTable[0].au16B_Gain[i] );

            hi_ext_system_isp_mesh_shading_r_gain1_write(ViPipe,  i, pstCmosLsc->astLscCalibTable[1].au16R_Gain[i] );
            hi_ext_system_isp_mesh_shading_gr_gain1_write(ViPipe, i, pstCmosLsc->astLscCalibTable[1].au16Gr_Gain[i]);
            hi_ext_system_isp_mesh_shading_gb_gain1_write(ViPipe, i, pstCmosLsc->astLscCalibTable[1].au16Gb_Gain[i]);
            hi_ext_system_isp_mesh_shading_b_gain1_write(ViPipe,  i, pstCmosLsc->astLscCalibTable[1].au16B_Gain[i] );
        }
    }
    else
    {
        for (i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
        {
            hi_ext_system_isp_mesh_shading_r_gain0_write(ViPipe,  i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_gr_gain0_write(ViPipe, i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_gb_gain0_write(ViPipe, i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_b_gain0_write(ViPipe,  i, g_au16MeshGainDef[pstLsc->u8MeshScale]);

            hi_ext_system_isp_mesh_shading_r_gain1_write(ViPipe,  i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_gr_gain1_write(ViPipe, i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_gb_gain1_write(ViPipe, i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
            hi_ext_system_isp_mesh_shading_b_gain1_write(ViPipe,  i, g_au16MeshGainDef[pstLsc->u8MeshScale]);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 LscReadExtRegs(VI_PIPE ViPipe)
{
    HI_U16  i;
    ISP_LSC_S *pstLsc    = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);

    pstLsc->bLscCoefUpdata  = hi_ext_system_isp_mesh_shading_attr_updata_read(ViPipe);
    hi_ext_system_isp_mesh_shading_attr_updata_write(ViPipe, HI_FALSE);

    if (pstLsc->bLscCoefUpdata)
    {
        pstLsc->u16MeshStrength = hi_ext_system_isp_mesh_shading_mesh_strength_read(ViPipe);
        pstLsc->u16MeshWeight   = hi_ext_system_isp_mesh_shading_blendratio_read(ViPipe);
    }

    pstLsc->bLutUpdate      = hi_ext_system_isp_mesh_shading_lut_attr_updata_read(ViPipe);
    hi_ext_system_isp_mesh_shading_lut_attr_updata_write(ViPipe, HI_FALSE);

    if (pstLsc->bLutUpdate)
    {
        pstLsc->u8MeshScale = hi_ext_system_isp_mesh_shading_mesh_scale_read(ViPipe);

        for (i = 0; i < (HI_ISP_LSC_GRID_COL - 1) / 2; i++)
        {
            pstLsc->au16DeltaX[i] = hi_ext_system_isp_mesh_shading_xgrid_read(ViPipe, i);
        }

        for (i = (HI_ISP_LSC_GRID_COL - 1) / 2; i < HI_ISP_LSC_GRID_COL - 1; i++)
        {
            pstLsc->au16DeltaX[i] = pstLsc->au16DeltaX[HI_ISP_LSC_GRID_COL - 2 - i];
        }

        for ( i = 0; i < (HI_ISP_LSC_GRID_ROW - 1) / 2; i++)
        {
            pstLsc->au16DeltaY[i] = hi_ext_system_isp_mesh_shading_ygrid_read(ViPipe, i);
        }

        geometricInvSize(pstLsc);
    }

    return HI_SUCCESS;
}

static HI_VOID geometricGridSizeLsc(HI_U16 *pu16Delta, HI_U16 *pu16Inv, HI_U16 u16Length, HI_U16 u16GridSize)
{
    HI_U16 i, sum;
    HI_U16 u16HalfGridSize;
    HI_U16 diff;
    HI_U16 *pu16TmpStep = HI_NULL;
    HI_U16 u16SumR;

    u16HalfGridSize = (u16GridSize - 1) >> 1;

    if (0 == u16HalfGridSize)
    {
        return;
    }

    pu16TmpStep = (HI_U16 *)ISP_MALLOC(sizeof(HI_U16) * u16HalfGridSize);

    if (HI_NULL  == pu16TmpStep)
    {
        return ;
    }

    memset(pu16TmpStep, 0, sizeof(HI_U16)*u16HalfGridSize);

    u16SumR = (HI_U16)((((u16Length >> 1) * 1024 / DIV_0_TO_1(u16HalfGridSize)) + 512) >> 10);

    for ( i = 0 ; i < u16HalfGridSize; i++)
    {
        pu16TmpStep[i] = u16SumR;
    }

    sum = 0;
    for (i = 0; i < u16HalfGridSize; i++)
    {
        sum = sum + pu16TmpStep[i];
    }

    if (sum != (u16Length >> 1))
    {
        if (sum > (u16Length >> 1))
        {
            diff = sum - (u16Length >> 1);
            for (i = 1; i <= diff; i++)
            {
                pu16TmpStep[u16HalfGridSize - i] = pu16TmpStep[u16HalfGridSize - i] - 1;
            }
        }
        else
        {
            diff = (u16Length >> 1) - sum;
            for (i = 1; i <= diff; i++)
            {
                pu16TmpStep[i - 1] = pu16TmpStep[i - 1] + 1;
            }
        }
    }

    for ( i = 0 ; i < u16HalfGridSize; i++ )
    {
        pu16Delta[i] = pu16TmpStep[i];
        pu16Inv[i]   = (pu16Delta[i] == 0) ? (0) : ((4096 * 1024 / DIV_0_TO_1(pu16Delta[i]) + 512) >> 10);
    }

    ISP_FREE(pu16TmpStep);
}

static HI_VOID LscImageSize(VI_PIPE ViPipe, HI_U8 u8BlkNum, ISP_LSC_S  *pstLsc)
{
    HI_U16     i;
    ISP_RECT_S stBlockRect;
    ISP_CTX_S  *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    geometricGridSizeLsc(pstLsc->au16DeltaX, pstLsc->au16InvX, pstIspCtx->stBlockAttr.stFrameRect.u32Width  / 2, HI_ISP_LSC_GRID_COL);
    geometricGridSizeLsc(pstLsc->au16DeltaY, pstLsc->au16InvY, pstIspCtx->stBlockAttr.stFrameRect.u32Height / 2, HI_ISP_LSC_GRID_ROW);

    for (i = 0; i < (HI_ISP_LSC_GRID_COL - 1) / 2; i++)
    {
        pstLsc->au16DeltaX[HI_ISP_LSC_GRID_COL - 2 - i] = pstLsc->au16DeltaX[i];
        pstLsc->au16InvX[HI_ISP_LSC_GRID_COL - 2 - i]   = pstLsc->au16InvX[i];
    }

    for (i = 0; i < u8BlkNum; i++)
    {
        ISP_GetBlockRect(&stBlockRect, &pstIspCtx->stBlockAttr, i);

        pstLsc->au16CurWidth[i]       = stBlockRect.u32Width;
        pstLsc->au16FirstPointPosX[i] = stBlockRect.s32X;
    }
}

static HI_VOID BNRLscImageSize(VI_PIPE ViPipe, HI_U8 u8BlkNum, ISP_RLSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U32 u32Width, u32Height;
    HI_U32 u32OffCenter;
    const HI_U32 temp = (1U << 31);

    ISP_CTX_S  *pstIspCtx = HI_NULL;
    ISP_RECT_S  stBlockRect;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_GetBlockRect(&stBlockRect, &pstIspCtx->stBlockAttr, u8BlkNum);

    u32Width  = pstIspCtx->stBlockAttr.stFrameRect.u32Width;
    u32Height = pstIspCtx->stBlockAttr.stFrameRect.u32Height;

    pstUsrRegCfg->u16WidthOffset = ABS(stBlockRect.s32X);

    pstUsrRegCfg->u16CenterRX  = u32Width  / 2;
    pstUsrRegCfg->u16CenterRY  = u32Height / 2;
    pstUsrRegCfg->u16CenterGrX = u32Width  / 2;
    pstUsrRegCfg->u16CenterGrY = u32Height / 2;
    pstUsrRegCfg->u16CenterGbX = u32Width  / 2;
    pstUsrRegCfg->u16CenterGbY = u32Height / 2;
    pstUsrRegCfg->u16CenterBX  = u32Width  / 2;
    pstUsrRegCfg->u16CenterBY  = u32Height / 2;

    u32OffCenter = temp / DIV_0_TO_1(u32Width * u32Width / 4 + u32Height * u32Height / 4);

    pstUsrRegCfg->u16OffCenterR  = u32OffCenter;
    pstUsrRegCfg->u16OffCenterGr = u32OffCenter;
    pstUsrRegCfg->u16OffCenterGb = u32OffCenter;
    pstUsrRegCfg->u16OffCenterB  = u32OffCenter;

    pstUsrRegCfg->u32UpdateIndex += 1;
    pstUsrRegCfg->bLutUpdate     = HI_TRUE;
    pstUsrRegCfg->bUsrResh       = HI_TRUE;
}

static HI_S32 LscCheckCmosParam(VI_PIPE ViPipe, const ISP_CMOS_LSC_S *pstCmosLsc)
{
    HI_U16 i;

    if (pstCmosLsc->u8MeshScale > 7)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u8MeshScale!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    for ( i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
    {
        if ((pstCmosLsc->astLscCalibTable[0].au16R_Gain[i]  > LSC_GAIN_MAX) || (pstCmosLsc->astLscCalibTable[0].au16Gr_Gain[i] > LSC_GAIN_MAX) || \
            (pstCmosLsc->astLscCalibTable[0].au16Gb_Gain[i] > LSC_GAIN_MAX) || (pstCmosLsc->astLscCalibTable[0].au16B_Gain[i]  > LSC_GAIN_MAX) || \
            (pstCmosLsc->astLscCalibTable[1].au16R_Gain[i]  > LSC_GAIN_MAX) || (pstCmosLsc->astLscCalibTable[1].au16Gr_Gain[i] > LSC_GAIN_MAX) || \
            (pstCmosLsc->astLscCalibTable[1].au16Gb_Gain[i] > LSC_GAIN_MAX) || (pstCmosLsc->astLscCalibTable[1].au16B_Gain[i]  > LSC_GAIN_MAX))
        {
            ISP_TRACE(HI_DBG_ERR, "Invalid Gain!\n");
            return HI_ERR_ISP_ILLEGAL_PARAM;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 LscInitialize(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_S32 s32Ret;
    ISP_LSC_S          *pstLsc    = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    pstLsc->u16MeshWeight   = HI_ISP_LSC_DEFAULT_WEIGHT;

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);

        s32Ret = LscCheckCmosParam(ViPipe, pstSnsDft->pstLsc);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstLsc->u16MeshStrength = pstSnsDft->pstLsc->u16MeshStrength;
        pstLsc->u8MeshScale     = pstSnsDft->pstLsc->u8MeshScale;
    }
    else
    {
        pstLsc->u16MeshStrength = HI_ISP_LSC_DEFAULT_MESH_STRENGTH;
        pstLsc->u8MeshScale     = HI_ISP_LSC_DEFAULT_MESH_SCALE;
    }

    LscImageSize(ViPipe, pstRegCfg->u8CfgNum, pstLsc);

    pstLsc->bLscEnable = HI_TRUE;

    return HI_SUCCESS;
}

static HI_VOID Lsc_Usr_Fw(VI_PIPE ViPipe, HI_U8 u8CurBlk, ISP_LSC_S *pstLsc, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    LscGetLutIndex(u8CurBlk, pstLsc, pstUsrRegCfg);

    LscGetGainLut(ViPipe, pstLsc, pstUsrRegCfg);

    pstUsrRegCfg->u8MeshScale     = pstLsc->u8MeshScale;
    pstUsrRegCfg->u32UpdateIndex += 1;
    pstUsrRegCfg->bLutUpdate      = HI_TRUE;
}

static HI_VOID  Lsc_Usr_Fw_Normal(VI_PIPE ViPipe, ISP_LSC_S *pstLsc, ISP_LSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U16 i;
    HI_U16 r_gain0, r_gain1, gr_gain0, gr_gain1, gb_gain0, gb_gain1, b_gain0, b_gain1;

    pstUsrRegCfg->u16WidthOffset  = 0;
    pstUsrRegCfg->u8MeshScale     = pstLsc->u8MeshScale;
    pstUsrRegCfg->u32UpdateIndex += 1;
    pstUsrRegCfg->bLutUpdate      = HI_TRUE;

    memcpy(pstUsrRegCfg->au16DeltaX, pstLsc->au16DeltaX, sizeof(HI_U16) * (HI_ISP_LSC_GRID_COL - 1));
    memcpy(pstUsrRegCfg->au16InvX,   pstLsc->au16InvX,   sizeof(HI_U16) * (HI_ISP_LSC_GRID_COL - 1));

    memcpy(pstUsrRegCfg->au16DeltaY, pstLsc->au16DeltaY, sizeof(HI_U16) * (HI_ISP_LSC_GRID_ROW - 1) / 2);
    memcpy(pstUsrRegCfg->au16InvY,   pstLsc->au16InvY,   sizeof(HI_U16) * (HI_ISP_LSC_GRID_ROW - 1) / 2);

    for ( i = 0; i < HI_ISP_LSC_GRID_POINTS; i++)
    {
        r_gain0  = hi_ext_system_isp_mesh_shading_r_gain0_read(ViPipe, i);
        r_gain1  = hi_ext_system_isp_mesh_shading_r_gain1_read(ViPipe, i);

        gr_gain0 = hi_ext_system_isp_mesh_shading_gr_gain0_read(ViPipe, i);
        gr_gain1 = hi_ext_system_isp_mesh_shading_gr_gain1_read(ViPipe, i);

        gb_gain0 = hi_ext_system_isp_mesh_shading_gb_gain0_read(ViPipe, i);
        gb_gain1 = hi_ext_system_isp_mesh_shading_gb_gain1_read(ViPipe, i);

        b_gain0  = hi_ext_system_isp_mesh_shading_b_gain0_read(ViPipe, i);
        b_gain1  = hi_ext_system_isp_mesh_shading_b_gain1_read(ViPipe, i);

        pstUsrRegCfg->au32RGain[i]  = ((HI_U32)r_gain1  << LSC_GAIN_BITWIDTH) + r_gain0;
        pstUsrRegCfg->au32GrGain[i] = ((HI_U32)gr_gain1 << LSC_GAIN_BITWIDTH) + gr_gain0;
        pstUsrRegCfg->au32GbGain[i] = ((HI_U32)gb_gain1 << LSC_GAIN_BITWIDTH) + gb_gain0;
        pstUsrRegCfg->au32BGain[i]  = ((HI_U32)b_gain1  << LSC_GAIN_BITWIDTH) + b_gain0;
    }

    return;
}

static HI_VOID Lsc_Usr_Fw_Bnr_Lsc(VI_PIPE ViPipe, ISP_RLSC_USR_CFG_S *pstUsrRegCfg)
{
    HI_U32 i;
    HI_U8  u8IndexR, u8IndexGr, u8IndexGb, u8IndexB;
    ISP_CTX_S  *pstIspCtx;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8IndexR  = pstIspCtx->enBayer ^ 0x0;
    u8IndexGr = pstIspCtx->enBayer ^ 0x1;
    u8IndexGb = pstIspCtx->enBayer ^ 0x2;
    u8IndexB  = pstIspCtx->enBayer ^ 0x3;

    for ( i = 0 ; i < HI_ISP_RLSC_POINTS ; i++ )
    {
        pstUsrRegCfg->au16GainLut[u8IndexR][i]  = hi_ext_system_isp_bnr_shading_r_gain_read(ViPipe, i);
        pstUsrRegCfg->au16GainLut[u8IndexGr][i] = hi_ext_system_isp_bnr_shading_gr_gain_read(ViPipe, i);
        pstUsrRegCfg->au16GainLut[u8IndexGb][i] = hi_ext_system_isp_bnr_shading_gb_gain_read(ViPipe, i);
        pstUsrRegCfg->au16GainLut[u8IndexB][i]  = hi_ext_system_isp_bnr_shading_b_gain_read(ViPipe, i);
    }

    pstUsrRegCfg->au16GainLut[u8IndexR][HI_ISP_RLSC_POINTS]  = pstUsrRegCfg->au16GainLut[u8IndexR][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexGr][HI_ISP_RLSC_POINTS] = pstUsrRegCfg->au16GainLut[u8IndexGr][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexGb][HI_ISP_RLSC_POINTS] = pstUsrRegCfg->au16GainLut[u8IndexGb][HI_ISP_RLSC_POINTS - 1];
    pstUsrRegCfg->au16GainLut[u8IndexB][HI_ISP_RLSC_POINTS]  = pstUsrRegCfg->au16GainLut[u8IndexB][HI_ISP_RLSC_POINTS - 1];

    pstUsrRegCfg->u32UpdateIndex += 1;
    pstUsrRegCfg->bLutUpdate      = HI_TRUE;
    pstUsrRegCfg->bUsrResh        = HI_TRUE;
}

static HI_BOOL __inline CheckLscOpen(ISP_LSC_S *pstLsc)
{
    return (HI_TRUE == pstLsc->bLscEnable);
}

static HI_S32 LscImageResWrite(VI_PIPE ViPipe, ISP_REG_CFG_S *pstRegCfg)
{
    HI_U8  i;
    HI_S32 s32Ret;
    ISP_LSC_S          *pstLsc    = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;

    LSC_GET_CTX(ViPipe, pstLsc);
    ISP_CHECK_POINTER(pstLsc);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    if (pstSnsDft->unKey.bit1Lsc)
    {
        ISP_CHECK_POINTER(pstSnsDft->pstLsc);

        s32Ret = LscCheckCmosParam(ViPipe, pstSnsDft->pstLsc);
        if (HI_SUCCESS != s32Ret)
        {
            return s32Ret;
        }

        pstLsc->u16MeshStrength = pstSnsDft->pstLsc->u16MeshStrength;
        pstLsc->u8MeshScale     = pstSnsDft->pstLsc->u8MeshScale;
    }
    else
    {
        pstLsc->u16MeshStrength = HI_ISP_LSC_DEFAULT_MESH_STRENGTH;
        pstLsc->u8MeshScale     = HI_ISP_LSC_DEFAULT_MESH_SCALE;
    }

    LscImageSize(ViPipe, pstRegCfg->u8CfgNum, pstLsc);

    LscExtRegsInitialize(ViPipe);

    if (1 == pstRegCfg->u8CfgNum)
    {
        Lsc_Usr_Fw_Normal(ViPipe, pstLsc, &pstRegCfg->stAlgRegCfg[0].stLscRegCfg.stUsrRegCfg);
    }
    else
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            Lsc_Usr_Fw(ViPipe, i, pstLsc, &pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg);
        }
    }

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        BNRLscImageSize(ViPipe, i, &pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg);
    }

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew  = HI_TRUE;
        pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt     = 0;
        pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.bSwitchLut2SttRegNew = HI_TRUE;
        pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.u8SwitchRegNewCnt    = 0;
        pstRegCfg->stAlgRegCfg[i].stBeLutUpdateCfg.bLscLutUpdate                = HI_TRUE;
    }

    pstRegCfg->unKey.bit1LscCfg  = 1;
    pstRegCfg->unKey.bit1RLscCfg = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_LscInit(VI_PIPE ViPipe, HI_VOID *pRegCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_REG_CFG_S *pstRegCfg = (ISP_REG_CFG_S *)pRegCfg;

    s32Ret = LscCtxInit(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = LscInitialize(ViPipe, pstRegCfg);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    LscRegsInitialize(ViPipe, pstRegCfg);
    LscExtRegsInitialize(ViPipe);

    BNRLscRegsInitialize(ViPipe, pstRegCfg);
    BNRLscExtRegsInitialize(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_LscRun(VI_PIPE ViPipe, const HI_VOID *pStatInfo,
                  HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    HI_S32 i;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_LSC_S *pstLsc    = HI_NULL;
    ISP_REG_CFG_S *pstRegCfg  = (ISP_REG_CFG_S *)pRegCfg;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    LSC_GET_CTX(ViPipe, pstLsc);
    ISP_CHECK_POINTER(pstLsc);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }

    pstLsc->bLscEnable = hi_ext_system_isp_mesh_shading_enable_read(ViPipe);

    for (i = 0; i < pstRegCfg->u8CfgNum; i++)
    {
        pstRegCfg->stAlgRegCfg[i].stLscRegCfg.bLscEn = pstLsc->bLscEnable;
    }

    pstRegCfg->unKey.bit1LscCfg  = 1;
    pstRegCfg->unKey.bit1RLscCfg = 1;
    /*check hardware setting*/
    if (!CheckLscOpen(pstLsc))
    {
        return HI_SUCCESS;
    }

    LscReadExtRegs(ViPipe);

    if (pstLsc->bLscCoefUpdata)
    {
        for (i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.bCoefUpdate   = HI_TRUE;
            pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u16MeshStr    = pstLsc->u16MeshStrength;
            pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg.u16Weight     = pstLsc->u16MeshWeight;
            pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.bCoefUpdate  = HI_TRUE;
            pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg.u16GainStr   = pstLsc->u16MeshStrength;
        }
    }

    if (pstLsc->bLutUpdate)
    {
        if (1 == pstRegCfg->u8CfgNum)
        {
            Lsc_Usr_Fw_Normal(ViPipe, pstLsc, &pstRegCfg->stAlgRegCfg[0].stLscRegCfg.stUsrRegCfg);
        }
        else
        {
            for ( i = 0; i < pstRegCfg->u8CfgNum; i++)
            {
                Lsc_Usr_Fw(ViPipe, i, pstLsc, &pstRegCfg->stAlgRegCfg[i].stLscRegCfg.stUsrRegCfg);
            }
        }

        for ( i = 0; i < pstRegCfg->u8CfgNum; i++)
        {
            Lsc_Usr_Fw_Bnr_Lsc(ViPipe, &pstRegCfg->stAlgRegCfg[i].stRLscRegCfg.stUsrRegCfg);
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_LscCtrl(VI_PIPE ViPipe, HI_U32 u32Cmd, HI_VOID *pValue)
{
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;
    ISP_CTX_S     *pstIspCtx = HI_NULL;

    switch (u32Cmd)
    {
        case ISP_CHANGE_IMAGE_MODE_SET:
            ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
            ISP_CHECK_POINTER(pRegCfg);
            LscImageResWrite(ViPipe, &pRegCfg->stRegCfg);
            break;
        case ISP_WDR_MODE_SET:
            ISP_GET_CTX(ViPipe, pstIspCtx);

            if (pstIspCtx->stBlockAttr.u8BlockNum != pstIspCtx->stBlockAttr.u8PreBlockNum)
            {
                ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);
                ISP_CHECK_POINTER(pRegCfg);
                LscImageResWrite(ViPipe, &pRegCfg->stRegCfg);
            }
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_LscExit(VI_PIPE ViPipe)
{
    HI_U8 i;
    ISP_REGCFG_S  *pRegCfg   = HI_NULL;

    ISP_REGCFG_GET_CTX(ViPipe, pRegCfg);

    for (i = 0; i < pRegCfg->stRegCfg.u8CfgNum; i++)
    {
        pRegCfg->stRegCfg.stAlgRegCfg[i].stLscRegCfg.bLscEn = HI_FALSE;
        pRegCfg->stRegCfg.stAlgRegCfg[i].stRLscRegCfg.bRLscEn = HI_FALSE;
    }

    pRegCfg->stRegCfg.unKey.bit1LscCfg = 1;
    pRegCfg->stRegCfg.unKey.bit1RLscCfg = 1;

    LscCtxExit(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterLsc(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_ALG_CHECK(pstIspCtx->unAlgKey.bit1Lsc);
    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_LSC;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_LscInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_LscRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_LscCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_LscExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
