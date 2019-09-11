/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_disp.h
 * @brief   disp module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_DISP_H__
#define __HI_MAPI_DISP_H__

#include "hi_mapi_disp_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     Disp */
/** @{ */ /** <!-- [Disp] */

/**
 * @brief Init DISP
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] pstDispAttr HI_MAPI_DISP_ATTR_S: DISP attribute
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_Init(HI_HANDLE DispHdl, const HI_MAPI_DISP_ATTR_S *pstDispAttr);

/**
 * @brief Deinit DISP
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_Deinit(HI_HANDLE DispHdl);

/**
 * @brief Start DISP
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] pstVideoLayerAttr HI_MAPI_DISP_VIDEOLAYER_ATTR_S: the attribute of VideoLayer
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
// HI_S32 HI_MAPI_DISP_Start(HI_HANDLE DispHdl);
HI_S32 HI_MAPI_DISP_Start(HI_HANDLE DispHdl, HI_MAPI_DISP_VIDEOLAYER_ATTR_S *pstVideoLayerAttr);

/**
 * @brief Stop DISP
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_Stop(HI_HANDLE DispHdl);

/**
 * @brief Set DISP extend attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] enCMD HI_MAPI_DISP_CMD_E: command of set DISP extend attribute
 * @param[in] pAttr HI_VOID: the extend attribute of DISP instance
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_SetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Get DISP extend attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] enCMD HI_MAPI_DISP_CMD_E: command of get DISP extend attribute
 * @param[out] pAttr HI_VOID: the extend attribute of DISP instance
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_GetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Set Window attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] pstWndAttr HI_MAPI_DISP_WINDOW_ATTR_S: the attribute of WINDOW instance
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_SetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                  const HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr);

/**
 * @brief Get Window attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[out] pstWndAttr HI_MAPI_DISP_WINDOW_ATTR_S: the attribute of WINDOW instance
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_GetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr);

/**
 * @brief Set Window extend attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] enCMD HI_MAPI_DISP_WINDOW_CMD_E: command of get Window extend attribute
 * @param[in] pAttr HI_VOID: the extend attribute of Window instance
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_SetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
                                    HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Get Window extend attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] enCMD HI_MAPI_DISP_WINDOW_CMD_E: command of set Window extend attribute
 * @param[out] pAttr HI_VOID: the extend attribute of Window instance
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_GetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
                                    HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Start Window
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_StartWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl);

/**
 * @brief Stop Window
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_StopWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl);

/**
 * @brief  clear Window
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_ClearWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl);

/**
 * @brief Set Window OSD attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] OsdHdl  HI_HANDLE: handle of OSD
 * @param[in] pstOsdAttr HI_MAPI_OSD_ATTR_S: the attribute of WINDOW OSD instance
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_SetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                     const HI_MAPI_OSD_ATTR_S *pstOsdAttr);

/**
 * @brief Get Window OSD attribute
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] OsdHdl  HI_HANDLE: handle of OSD
 * @param[out] pstOsdAttr HI_MAPI_OSD_ATTR_S: the attribute of WINDOW OSD instance
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_GetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
                                     HI_MAPI_OSD_ATTR_S *pstOsdAttr);

/**
 * @brief Start Window OSD
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] OsdHdl  HI_HANDLE: handle of OSD
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_StartWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl);
/**
 * @brief Stop Window OSD
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] OsdHdl  HI_HANDLE: handle of OSD
 * @return Non 0 indicate failed
 * @return  0 indicate success
 */
HI_S32 HI_MAPI_DISP_StopWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl);
/**
 * @brief   Send Video Frame to Window
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] pstFramedata HI_MAPI_FRAME_DATA_S: Framedata info
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_SendFrame(HI_HANDLE DispHdl, HI_HANDLE WndHdl, const HI_MAPI_FRAME_DATA_S *pstFramedata);

/**
 * @brief  DISP bind VProc
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] GrpHdl HI_HANDLE: handle of VPROC
 * @param[in] VPortHdl  HI_HANDLE: handle of VPort
 * @param[in] bStitch HI_BOOL：VPROC stitch
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_Bind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                               HI_BOOL bStitch);

/**
 * @brief  DISP unbind VProc
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] GrpHdl HI_HANDLE: handle of VPROC
 * @param[in] VPortHdl  HI_HANDLE: handle of VPort
 * @param[in] bStitch HI_BOOL：VPROC stitch
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_UnBind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                 HI_BOOL bStitch);

/**
 * @brief  DISP bind VCap
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] VcapPipeHdl HI_HANDLE: handle of VCap
 * @param[in] PipeChnHdl  HI_HANDLE: handle of VcapPipe
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_Bind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl);

/**
 * @brief  DISP unbind VCap
 * @param[in] DispHdl HI_HANDLE: handle of DISP
 * @param[in] WndHdl  HI_HANDLE: handle of WINDOW
 * @param[in] VcapPipeHdl HI_HANDLE: handle of VCap
 * @param[in] PipeChnHdl  HI_HANDLE: handle of VcapPipe
 * @return 0 indicate get success
 * @return Non 0 indicate get failed
 */
HI_S32 HI_MAPI_DISP_UnBind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl);

/*
HI_S32 HI_MAPI_DISP_OpenMipiTx(HI_MAPI_DISP_MIPITX_ATTR* pstMipiTxAttr);

HI_S32 HI_MAPI_DISP_CloseMipiTx(HI_VOID);
*/

/** @} */ /** <!-- ==== Disp End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_DISP_H__ */

