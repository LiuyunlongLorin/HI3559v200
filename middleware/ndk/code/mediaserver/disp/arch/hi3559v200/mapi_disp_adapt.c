/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_disp_adapt.c
 * @brief   disp adapt function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "mapi_comm_inner.h"
#include "mapi_disp_inner.h"
#include "hi_mapi_disp_define.h"
#include "mapi_disp_adapt.h"
#include "mapi_disp_arch.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_SetWindow_AspectRatioAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                          const HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *pstAspectRatioAttr)
{
    HI_S32 s32Ret;
    VO_LAYER_PARAM_S stLayerParam;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstAspectRatioAttr);
    if (pstAspectRatioAttr->stAspectRatio.enMode == ASPECT_RATIO_AUTO) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI3559V200 not support ASPECT_RATIO_AUTO mode!!\n");
        return HI_MAPI_DISP_ENOT_SUPPORT;
    }

    stLayerParam.stAspectRatio.enMode = pstAspectRatioAttr->stAspectRatio.enMode;
    stLayerParam.stAspectRatio.u32BgColor = pstAspectRatioAttr->stAspectRatio.u32BgColor;
    stLayerParam.stAspectRatio.stVideoRect.s32X = pstAspectRatioAttr->stAspectRatio.stVideoRect.s32X;
    stLayerParam.stAspectRatio.stVideoRect.s32Y = pstAspectRatioAttr->stAspectRatio.stVideoRect.s32Y;
    stLayerParam.stAspectRatio.stVideoRect.u32Width = pstAspectRatioAttr->stAspectRatio.stVideoRect.u32Width;
    stLayerParam.stAspectRatio.stVideoRect.u32Height = pstAspectRatioAttr->stAspectRatio.stVideoRect.u32Height;

    s32Ret = HI_MPI_VO_SetVideoLayerParam(DispHdl, &stLayerParam);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_SetVideoLayerParam fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_GetWindow_AspectRatioAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                          HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *pstAspectRatioAttr)
{
    HI_S32 s32Ret;
    VO_LAYER_PARAM_S stLayerParam;
    memset(&stLayerParam, 0x0, sizeof(VO_LAYER_PARAM_S));

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstAspectRatioAttr);

    s32Ret = HI_MPI_VO_GetVideoLayerParam(DispHdl, &stLayerParam);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "HI_MPI_VO_GetVideoLayerAttr fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    pstAspectRatioAttr->stAspectRatio.enMode = stLayerParam.stAspectRatio.enMode;
    pstAspectRatioAttr->stAspectRatio.u32BgColor = stLayerParam.stAspectRatio.u32BgColor;
    pstAspectRatioAttr->stAspectRatio.stVideoRect.s32X = stLayerParam.stAspectRatio.stVideoRect.s32X;
    pstAspectRatioAttr->stAspectRatio.stVideoRect.s32Y = stLayerParam.stAspectRatio.stVideoRect.s32Y;
    pstAspectRatioAttr->stAspectRatio.stVideoRect.u32Width = stLayerParam.stAspectRatio.stVideoRect.u32Width;
    pstAspectRatioAttr->stAspectRatio.stVideoRect.u32Height = stLayerParam.stAspectRatio.stVideoRect.u32Height;

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_DISP_SetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                      const HI_MAPI_OSD_ATTR_S *pstOsdAttr,
                                      MAPI_DISP_OSD_ATTR_S *pstOsdContext)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "mpi region module is not support !\n");
    return HI_MAPI_DISP_ENOT_SUPPORT;
}

HI_S32 HAL_MAPI_DISP_GetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOsdAttr,
                                      MAPI_DISP_OSD_ATTR_S *pstOsdContext)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "mpi region module is not support !\n");
    return HI_MAPI_DISP_ENOT_SUPPORT;
}

HI_S32 HAL_MAPI_DISP_StartWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                    MAPI_DISP_OSD_ATTR_S *pstOsdContext)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "mpi region module is not support !\n");
    return HI_MAPI_DISP_ENOT_SUPPORT;
}

HI_S32 HAL_MAPI_DISP_StopWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                   MAPI_DISP_OSD_ATTR_S *pstOsdContext)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "mpi region module is not support !\n");
    return HI_MAPI_DISP_ENOT_SUPPORT;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
