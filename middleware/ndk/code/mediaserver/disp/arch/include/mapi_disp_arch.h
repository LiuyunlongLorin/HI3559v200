/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_disp_arch.c
 * @brief   disp arch header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __MAPI_DISP_ARCH_H__
#define __MAPI_DISP_ARCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_SetWindow_AspectRatioAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                          const HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *pstAspectRatioAttr);
HI_S32 HAL_MAPI_GetWindow_AspectRatioAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                          HI_MAPI_DISP_WINDOW_ASPECTRATIO_S *pstAspectRatioAttr);

HI_S32 HAL_MAPI_DISP_SetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                      const HI_MAPI_OSD_ATTR_S *pstOsdAttr,
                                      MAPI_DISP_OSD_ATTR_S *pstOsdContext);
HI_S32 HAL_MAPI_DISP_GetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOsdAttr,
                                      MAPI_DISP_OSD_ATTR_S *pstOsdContext);
HI_S32 HAL_MAPI_DISP_StartWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                    MAPI_DISP_OSD_ATTR_S *pstOsdContext);
HI_S32 HAL_MAPI_DISP_StopWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                   MAPI_DISP_OSD_ATTR_S *pstOsdContext);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_DISP_ARCH_H__ */
