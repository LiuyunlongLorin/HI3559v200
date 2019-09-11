/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_disp_inner.h
 * @brief   disp module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_mapi_errno.h"
#include "hi_comm_vo.h"
#include "hi_mapi_log.h"
#include "hi_mapi_disp_define.h"
#include "mapi_disp_adapt.h"

#ifndef __MAPI_DISP_H__
#define __MAPI_DISP_H__

#define DISP_DEFAULT_CANVASNUM     2
#define DISP_VIDEOLAYER_DISPRECT_X 0
#define DISP_VIDEOLAYER_DISPRECT_Y 0

#define DISP_VIDEOFRAME_LUMINANCE_MAX 1000
#define DISP_VIDEOFRAME_LUMINANCE_MIN 200

#define DISP_CHECK_HANDLE(Hdl)                                                                          \
    do {                                                                                                \
        if (Hdl >= HI_MAPI_DISP_MAX_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "disp dev(%d) is invalid, only support [0, %d]\n", Hdl,    \
                           HI_MAPI_DISP_MAX_NUM - 1);                                                   \
            return HI_MAPI_DISP_EINVALID_DEVID;                                                         \
        }                                                                                               \
    } while (0)

#define DISP_CHECK_VIDEO_CSC_HANDLE(Hdl)                                                                     \
    do {                                                                                                     \
        if (Hdl >= VO_MAX_LAYER_NUM) {                                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "video csc handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           VO_MAX_LAYER_NUM - 1);                                                            \
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);                                                          \
            return HI_MAPI_DISP_EINVALID_DEVID;                                                              \
        }                                                                                                    \
    } while (0)

#define DISP_CHECK_GRAPHIC_CSC_HANDLE(Hdl)                                                                     \
    do {                                                                                                       \
        if (Hdl >= VO_MAX_GRAPHICS_LAYER_NUM) {                                                                \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "graphic csc handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           VO_MAX_GRAPHICS_LAYER_NUM - 1);                                                     \
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);                                                  \
            return HI_MAPI_DISP_EINVALID_DEVID;                                                                \
        }                                                                                                      \
    } while (0)

#define DISP_CHECK_WIN_ID(HdW)                                                                          \
    do {                                                                                                \
        if (HdW >= HI_MAPI_DISP_WND_MAX_NUM) {                                                          \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "disp window(%d) is invalid, only support [0, %d]\n", HdW, \
                           HI_MAPI_DISP_WND_MAX_NUM - 1);                                               \
            return HI_MAPI_DISP_EINVALID_CHNID;                                                         \
        }                                                                                               \
    } while (0)

#define DISP_CHECK_OSD_HANDLE(Hdl)                                                                          \
    do {                                                                                                    \
        if (Hdl >= HI_MAPI_DISP_OSD_MAX_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp osd handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_DISP_OSD_MAX_NUM - 1);                                                   \
            return HI_MAPI_DISP_EILLEGAL_PARAM;                                                             \
        }                                                                                                   \
    } while (0)

#define DISP_CHECK_VCAPPIPE_HANDLE(Hdl)                                                                     \
    do {                                                                                                    \
        if (Hdl >= HI_MAPI_VCAP_MAX_PIPE_NUM) {                                                             \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "VcapPipe handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VCAP_MAX_PIPE_NUM - 1);                                                  \
            return HI_MAPI_DISP_EILLEGAL_PARAM;                                                             \
        }                                                                                                   \
    } while (0)

#define DISP_CHECK_VCAPCHN_HANDLE(Hdl)                                                                     \
    do {                                                                                                   \
        if (Hdl >= HI_MAPI_PIPE_MAX_CHN_NUM) {                                                             \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "PipeChn handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_PIPE_MAX_CHN_NUM - 1);                                                  \
            return HI_MAPI_DISP_EILLEGAL_PARAM;                                                            \
        }                                                                                                  \
    } while (0)

#define DISP_CHECK_VPSSDVE_HANDLE(Hdl)                                                                  \
    do {                                                                                                \
        if (Hdl >= HI_MAPI_VPSS_MAX_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Vpss handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPSS_MAX_NUM - 1);                                                   \
            return HI_MAPI_DISP_EILLEGAL_PARAM;                                                         \
        }                                                                                               \
    } while (0)

#define DISP_CHECK_VPSSVPORT_HANDLE(Hdl)                                                                 \
    do {                                                                                                 \
        if (Hdl >= HI_MAPI_VPORT_MAX_NUM) {                                                              \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "VPort handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_MAPI_VPORT_MAX_NUM - 1);                                                   \
            return HI_MAPI_DISP_EILLEGAL_PARAM;                                                          \
        }                                                                                                \
    } while (0)

#define DISP_CHECK_NULL_PTR(pstPtr)                              \
    do {                                                         \
        if (NULL == (pstPtr)) {                                  \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Null pointer!\n"); \
            return HI_MAPI_DISP_ENULL_PTR;                       \
        }                                                        \
    } while (0)

#define DISP_CHECK_INIT(state)                                              \
    do {                                                                    \
        if (!state) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "DISP has not been inited\n"); \
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);                         \
            return HI_MAPI_DISP_ENOTREADY;                                  \
        }                                                                   \
    } while (0)

#define DISP_CHECK_BIND(state)                                           \
    do {                                                                 \
        if (state) {                                                     \
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Window has been bound\n"); \
            MUTEX_UNLOCK(g_Disp_FuncLock[DispHdl]);                      \
            return HI_SUCCESS;                                           \
        }                                                                \
    } while (0)

typedef struct tagMAPI_DISP_OSD_ATTR_S {
    HI_BOOL bOsdStarted;
    HI_MAPI_OSD_ATTR_S stOsdAttr;
} MAPI_DISP_OSD_ATTR_S;

typedef struct tagMAPI_DISP_WND_ATTR_S {
    HI_BOOL bWindowStarted;
    HI_BOOL bWindowBinded;
    MAPI_DISP_OSD_ATTR_S g_astDispOsdAttr[HI_MAPI_DISP_OSD_MAX_NUM];
} MAPI_DISP_WND_ATTR_S;

typedef struct tagMAPI_DISP_CONTEXT_S {
    HI_BOOL bDispInited;
    HI_BOOL bDispStarted;
    HI_U32 u32VLWidth;
    HI_U32 u32VLHeight;
    HI_U32 u32VLFrameRate;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    MAPI_DISP_WND_ATTR_S astWndAttr[HI_MAPI_DISP_WND_MAX_NUM];
} MAPI_DISP_CONTEXT_S;

HI_S32 MAPI_DISP_Init(HI_VOID);
HI_S32 MAPI_DISP_Deinit(HI_VOID);

#endif


