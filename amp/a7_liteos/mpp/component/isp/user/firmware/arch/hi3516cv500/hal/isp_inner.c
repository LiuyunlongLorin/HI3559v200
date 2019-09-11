/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_inner.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2018/05/09
  Description   :
  History       :
  1.Date        : 2018/05/09
    Author      :
    Modification: Created file

******************************************************************************/

#include "mpi_sys.h"
#include "hi_comm_vi.h"
#include "hi_comm_isp.h"
#include "hi_comm_3a.h"
#include "hi_ae_comm.h"
#include "hi_awb_comm.h"
#include "isp_inner.h"
#include "isp_main.h"
#include "isp_vreg.h"
#include "isp_ext_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
HI_VOID ISP_CalcGridInfo(HI_U16 u16Wdith, HI_U16 u16StartPox, HI_U16 u16BlockNum, HI_U16 *au16GridInfo)
{
    HI_U16 i;
    HI_U16 integer;
    HI_U16 remainder;

    integer   = u16Wdith / DIV_0_TO_1(u16BlockNum);
    remainder = u16Wdith % DIV_0_TO_1(u16BlockNum);
    au16GridInfo[0] = u16StartPox;
    for (i = 1; i < u16BlockNum; i++)
    {
        if (remainder > 0)
        {
            au16GridInfo[i] = au16GridInfo[i - 1] + integer + 1;
            remainder = remainder - 1;
        }
        else
        {
            au16GridInfo[i] = au16GridInfo[i - 1] + integer ;
        }
    }

    return;
}
HI_U32 ISP_GetStripingActiveImgStart(HI_U8 u8BlockIndex, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U32 u32OverLap ;
    HI_U32 u32BlockStart;

    u32OverLap = pstIspWorkMode->u32OverLap;
    if (u8BlockIndex == 0)
    {
        u32BlockStart = pstIspWorkMode->astBlockRect[u8BlockIndex].s32X ;
    }
    else
    {
        u32BlockStart = pstIspWorkMode->astBlockRect[u8BlockIndex].s32X + u32OverLap;
    }

    return u32BlockStart;
}

HI_U32 ISP_GetStripingActiveImgWidth(HI_U8 u8BlockIndex, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U32 u32BlockWidth;
    HI_U32 u32OverLap;
    HI_U8   u8BlockNum;

    u32OverLap    = pstIspWorkMode->u32OverLap;
    u32BlockWidth = pstIspWorkMode->astBlockRect[u8BlockIndex].u32Width;
    u8BlockNum    = pstIspWorkMode->u8BlockNum;

    if ((u8BlockIndex == 0 ) || (u8BlockIndex == (u8BlockNum - 1))) //first block and last block
    {
        u32BlockWidth = u32BlockWidth - u32OverLap;
    }
    else
    {
        u32BlockWidth = u32BlockWidth - u32OverLap * 2;
    }
    return u32BlockWidth;
}

HI_U32 ISP_GetStripingGridXInfo(HI_U16 *au16GridPos, HI_U16 u16GridNum, ISP_WORKING_MODE_S *pstIspWorkMode)
{
    HI_U8  i;
    HI_U16 u16Start;
    HI_U16 u16Width;
    HI_U16 u16DivNum;
    HI_U16 index = 0;

    for (i = 0; i < pstIspWorkMode->u8BlockNum; i++)
    {
        u16Start = ISP_GetStripingActiveImgStart(i, pstIspWorkMode);
        u16Width = ISP_GetStripingActiveImgWidth(i, pstIspWorkMode);

        if ( i < u16GridNum % DIV_0_TO_1(pstIspWorkMode->u8BlockNum))
        {
            u16DivNum = u16GridNum / DIV_0_TO_1(pstIspWorkMode->u8BlockNum) + 1;
        }
        else
        {
            u16DivNum = u16GridNum / DIV_0_TO_1(pstIspWorkMode->u8BlockNum);
        }

        ISP_CalcGridInfo(u16Width, u16Start, u16DivNum, &(au16GridPos[index]));
        index = index + u16DivNum;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_GetAEGridInfo(VI_PIPE ViPipe, ISP_AE_GRID_INFO_S *pstFEGridInfo, ISP_AE_GRID_INFO_S *pstBEGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16ImgTotalWidth, u16ImgTotalHeight;
    HI_U16  u16ImgStartX, u16ImgStartY;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX = 0;
    HI_U16  u16BeStartY = 0;
    ISP_WORKING_MODE_S stIspWorkMode;

    memset(pstFEGridInfo, 0, sizeof(ISP_AE_GRID_INFO_S));
    memset(pstBEGridInfo, 0, sizeof(ISP_AE_GRID_INFO_S));

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_system_ae_crop_en_read(ViPipe);

    if (HI_TRUE == bCropEn)
    {
        u16ImgStartX      = hi_ext_system_ae_crop_x_read(ViPipe);
        u16ImgStartY      = hi_ext_system_ae_crop_y_read(ViPipe);
        u16ImgTotalWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
        u16ImgTotalHeight = hi_ext_system_ae_crop_height_read(ViPipe);
    }
    else
    {
        u16ImgStartX      = 0;
        u16ImgStartY      = 0;
        u16ImgTotalWidth  = hi_ext_sync_total_width_read(ViPipe);
        u16ImgTotalHeight = hi_ext_sync_total_height_read(ViPipe);
    }

    ISP_CalcGridInfo(u16ImgTotalWidth,  u16ImgStartX, AE_ZONE_COLUMN, pstFEGridInfo->au16GridXPos);
    ISP_CalcGridInfo(u16ImgTotalHeight, u16ImgStartY, AE_ZONE_ROW, pstFEGridInfo->au16GridYPos);

    pstFEGridInfo->au16GridXPos[AE_ZONE_COLUMN] = u16ImgStartX + u16ImgTotalWidth - 1 ;
    pstFEGridInfo->au16GridYPos[AE_ZONE_ROW]    = u16ImgStartY + u16ImgTotalHeight - 1;
    pstFEGridInfo->u8Status = 1;

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstBEGridInfo->au16GridXPos, AE_ZONE_COLUMN, &stIspWorkMode);
        u16BeStartY   = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight   = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AE_ZONE_ROW, pstBEGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_ae_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_ae_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_ae_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, AE_ZONE_COLUMN, pstBEGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, AE_ZONE_ROW, pstBEGridInfo->au16GridYPos);
    }

    pstBEGridInfo->au16GridXPos[AE_ZONE_COLUMN] = u16BeStartX + u16BeWidth  - 1; //last position
    pstBEGridInfo->au16GridYPos[AE_ZONE_ROW]    = u16BeStartY + u16BeHeight - 1; //last position

    pstBEGridInfo->u8Status = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetMGGridInfo(VI_PIPE ViPipe, ISP_MG_GRID_INFO_S *pstGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX = 0;
    HI_U16  u16BeStartY = 0;
    ISP_WORKING_MODE_S stIspWorkMode;

    memset(pstGridInfo, 0, sizeof(ISP_MG_GRID_INFO_S));

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_system_ae_crop_en_read(ViPipe);

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstGridInfo->au16GridXPos, MG_ZONE_COLUMN, &stIspWorkMode);
        u16BeStartY   = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight   = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, MG_ZONE_ROW, pstGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_ae_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_ae_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_ae_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_ae_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, MG_ZONE_COLUMN, pstGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, MG_ZONE_ROW,    pstGridInfo->au16GridYPos);

    }

    pstGridInfo->au16GridXPos[MG_ZONE_COLUMN] = u16BeStartX + u16BeWidth  - 1; //last position
    pstGridInfo->au16GridYPos[MG_ZONE_ROW]    = u16BeStartY + u16BeHeight - 1; //last position
    pstGridInfo->u8Status                     = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetAFGridInfo(VI_PIPE ViPipe, ISP_FOCUS_GRID_INFO_S *pstFEGridInfo, ISP_FOCUS_GRID_INFO_S *pstBEGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX = 0;
    HI_U16  u16BeStartY = 0;
    HI_U16  u16AfXGridNum, u16AfYGridNum;
    ISP_WORKING_MODE_S stIspWorkMode;

    memset(pstFEGridInfo, 0, sizeof(ISP_FOCUS_GRID_INFO_S));
    memset(pstBEGridInfo, 0, sizeof(ISP_FOCUS_GRID_INFO_S));

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    bCropEn = hi_ext_af_crop_enable_read(ViPipe);
    u16AfYGridNum = hi_ext_af_window_vnum_read(ViPipe);
    u16AfXGridNum = hi_ext_af_window_hnum_read(ViPipe);

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstBEGridInfo->au16GridXPos, u16AfXGridNum, &stIspWorkMode);
        u16BeStartY = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16AfYGridNum, pstBEGridInfo->au16GridYPos);
        u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_af_crop_pos_x_read(ViPipe);
            u16BeStartY = hi_ext_af_crop_pos_y_read(ViPipe);
            u16BeWidth  = hi_ext_af_crop_hsize_read(ViPipe);
            u16BeHeight = hi_ext_af_crop_vsize_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth,  u16BeStartX, u16AfXGridNum, pstBEGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16AfYGridNum,    pstBEGridInfo->au16GridYPos);

    }

    pstBEGridInfo->au16GridXPos[u16AfXGridNum] = u16BeStartX + u16BeWidth  - 1; //last position
    pstBEGridInfo->au16GridYPos[u16AfYGridNum] = u16BeStartY + u16BeHeight - 1; //last position
    pstBEGridInfo->u8Status = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetWBGridInfo(VI_PIPE ViPipe, ISP_AWB_GRID_INFO_S *pstGridInfo)
{
    HI_BOOL bCropEn = HI_FALSE;
    HI_U16  u16BeWidth, u16BeHeight;
    HI_U16  u16BeStartX = 0;
    HI_U16  u16BeStartY = 0;
    HI_U16  u16awbXGridNum, u16awbYGridNum;
    ISP_WORKING_MODE_S stIspWorkMode;

    memset(pstGridInfo, 0,sizeof(ISP_AWB_GRID_INFO_S));

    if (HI_SUCCESS !=  ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
        return HI_FAILURE;
    }

    u16awbYGridNum = hi_ext_system_awb_vnum_read(ViPipe);
    u16awbXGridNum = hi_ext_system_awb_hnum_read(ViPipe);
    bCropEn        = hi_ext_system_awb_crop_en_read(ViPipe);

    if ((IS_STRIPING_MODE(stIspWorkMode.enIspRunningMode)) ||
        (IS_SIDEBYSIDE_MODE(stIspWorkMode.enIspRunningMode)))
    {
        ISP_GetStripingGridXInfo(pstGridInfo->au16GridXPos, u16awbXGridNum, &stIspWorkMode);
        u16BeStartY = stIspWorkMode.astBlockRect[0].s32Y;
        u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16awbYGridNum, pstGridInfo->au16GridYPos);
        u16BeWidth    = stIspWorkMode.stFrameRect.u32Width;
    }
    else
    {
        if (HI_TRUE == bCropEn)
        {
            u16BeStartX = hi_ext_system_awb_crop_x_read(ViPipe);
            u16BeStartY = hi_ext_system_awb_crop_y_read(ViPipe);
            u16BeWidth  = hi_ext_system_awb_crop_width_read(ViPipe);
            u16BeHeight = hi_ext_system_awb_crop_height_read(ViPipe);
        }
        else
        {
            u16BeStartX = 0;
            u16BeStartY = 0;
            u16BeWidth  = stIspWorkMode.stFrameRect.u32Width;
            u16BeHeight = stIspWorkMode.stFrameRect.u32Height;
        }

        ISP_CalcGridInfo(u16BeWidth, u16BeStartX, u16awbXGridNum, pstGridInfo->au16GridXPos);
        ISP_CalcGridInfo(u16BeHeight, u16BeStartY, u16awbYGridNum, pstGridInfo->au16GridYPos);
    }

    pstGridInfo->au16GridXPos[u16awbXGridNum] = u16BeStartX + u16BeWidth  - 1; //last position
    pstGridInfo->au16GridYPos[u16awbYGridNum] = u16BeStartY + u16BeHeight - 1; //last position
    pstGridInfo->u8Status                     = 1;

    return HI_SUCCESS;
}

HI_S32 ISP_GetAEStitchStatistics(VI_PIPE ViPipe, ISP_AE_STITCH_STATISTICS_S *pstAeStitchStat)
{
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetWBStitchStatistics(VI_PIPE ViPipe, ISP_WB_STITCH_STATISTICS_S *pstStitchWBStat)
{
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetFeFocusStatistics(VI_PIPE ViPipe, ISP_FE_FOCUS_STATISTICS_S *pstFEAFStat, ISP_STAT_S *pstIspActStat, HI_U8 u8WdrChn)
{
    memset(pstFEAFStat, 0, sizeof(ISP_FE_FOCUS_STATISTICS_S));

    return HI_SUCCESS;
}

HI_S32 ISP_SetRadialShadingAttr(VI_PIPE ViPipe, const ISP_RADIAL_SHADING_ATTR_S *pstRaShadingAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRadialShadingAttr(VI_PIPE ViPipe, ISP_RADIAL_SHADING_ATTR_S *pstRaShadingAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetRadialShadingLUT(VI_PIPE ViPipe, const ISP_RADIAL_SHADING_LUT_ATTR_S *pstRaShadingLutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRadialShadingLUT(VI_PIPE ViPipe, ISP_RADIAL_SHADING_LUT_ATTR_S *pstRaShadingLutAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetPipeDifferAttr(VI_PIPE ViPipe, const ISP_PIPE_DIFF_ATTR_S *pstPipeDiffer)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetPipeDifferAttr(VI_PIPE ViPipe, ISP_PIPE_DIFF_ATTR_S *pstPipeDiffer)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetRcAttr(VI_PIPE ViPipe, const ISP_RC_ATTR_S *pstRcAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRcAttr(VI_PIPE ViPipe, ISP_RC_ATTR_S *pstRcAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetRGBIRAttr(VI_PIPE ViPipe, const ISP_RGBIR_ATTR_S *pstRGBIRAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetRGBIRAttr(VI_PIPE ViPipe, ISP_RGBIR_ATTR_S *pstRGBIRAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetPreLogLUTAttr(VI_PIPE ViPipe, const ISP_PRELOGLUT_ATTR_S *pstPreLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetPreLogLUTAttr(VI_PIPE ViPipe, ISP_PRELOGLUT_ATTR_S *pstPreLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetLogLUTAttr(VI_PIPE ViPipe, const ISP_LOGLUT_ATTR_S *pstLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_GetLogLUTAttr(VI_PIPE ViPipe, ISP_LOGLUT_ATTR_S *pstLogLUTAttr)
{
    ISP_TRACE(HI_DBG_ERR, "Not support this interface!\n");
    return HI_ERR_ISP_NOT_SUPPORT;
}

HI_S32 ISP_SetClutCoeff(VI_PIPE ViPipe, const ISP_CLUT_LUT_S *pstClutLUT)
{
    HI_U32         *pu32VirAddr = HI_NULL;
    ISP_MMZ_BUF_EX_S stClutBuf;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstClutLUT);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_CLUT_BUF_GET, &stClutBuf.u64PhyAddr))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Clut Buffer Err\n");
        return HI_ERR_ISP_NOMEM;
    }

    stClutBuf.pVirAddr = HI_MPI_SYS_Mmap(stClutBuf.u64PhyAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    if (HI_NULL == stClutBuf.pVirAddr)
    {
        return HI_ERR_ISP_NULL_PTR;
    }

    pu32VirAddr = (HI_U32 *)stClutBuf.pVirAddr;

    memcpy(pu32VirAddr, pstClutLUT->au32lut, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    hi_ext_system_clut_lut_update_en_write(ViPipe, HI_TRUE);

    HI_MPI_SYS_Munmap(stClutBuf.pVirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    return HI_SUCCESS;
}

HI_S32 ISP_GetClutCoeff(VI_PIPE ViPipe, ISP_CLUT_LUT_S *pstClutLUT)
{
    HI_U32         *pu32VirAddr = HI_NULL;
    ISP_MMZ_BUF_EX_S stClutBuf;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstClutLUT);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    if (HI_SUCCESS != ioctl(g_as32IspFd[ViPipe], ISP_CLUT_BUF_GET, &stClutBuf.u64PhyAddr))
    {
        ISP_TRACE(HI_DBG_ERR, "Get Clut Buffer Err\n");
        return HI_ERR_ISP_NOMEM;
    }

    stClutBuf.pVirAddr = HI_MPI_SYS_Mmap(stClutBuf.u64PhyAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    if ( HI_NULL == stClutBuf.pVirAddr )
    {
        return HI_ERR_ISP_NULL_PTR;
    }

    pu32VirAddr = (HI_U32 *)stClutBuf.pVirAddr;

    memcpy(pstClutLUT->au32lut, pu32VirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    HI_MPI_SYS_Munmap(stClutBuf.pVirAddr, HI_ISP_CLUT_LUT_LENGTH * sizeof(HI_U32));

    return HI_SUCCESS;
}

HI_S32 ISP_SetClutAttr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstClutAttr)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstClutAttr);
    ISP_CHECK_BOOL(pstClutAttr->bEnable);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    if (pstClutAttr->u32GainR > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainR!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstClutAttr->u32GainG > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainG!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }
    if (pstClutAttr->u32GainB > 4096)
    {
        ISP_TRACE(HI_DBG_ERR, "Invalid u32GainB!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    hi_ext_system_clut_en_write(ViPipe, pstClutAttr->bEnable);
    hi_ext_system_clut_gainR_write(ViPipe, pstClutAttr->u32GainR);
    hi_ext_system_clut_gainG_write(ViPipe, pstClutAttr->u32GainG);
    hi_ext_system_clut_gainB_write(ViPipe, pstClutAttr->u32GainB);
    hi_ext_system_clut_ctrl_update_en_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}
HI_S32 ISP_GetClutAttr(VI_PIPE ViPipe, ISP_CLUT_ATTR_S *pstClutAttr)
{
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstClutAttr);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    pstClutAttr->bEnable  = hi_ext_system_clut_en_read(ViPipe);
    pstClutAttr->u32GainR = hi_ext_system_clut_gainR_read(ViPipe);
    pstClutAttr->u32GainG = hi_ext_system_clut_gainG_read(ViPipe);
    pstClutAttr->u32GainB = hi_ext_system_clut_gainB_read(ViPipe);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
