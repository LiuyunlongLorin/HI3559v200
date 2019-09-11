/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_disp_client.c
 * @brief   NDK disp client functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */
#include <stdio.h>
#include "hi_mapi_comm_define.h"
#include "hi_mapi_disp_define.h"
#include "hi_ipcmsg.h"
#include "msg_disp.h"
#include "msg_define.h"
#include "hi_type.h"
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_log.h"
#include "mapi_disp_inner.h"
#include "msg_wrapper.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HI_MAPI_DISP_Init(HI_HANDLE DispHdl, const HI_MAPI_DISP_ATTR_S *pstDispAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    HI_BOOL bSysInit;

    bSysInit = MAPI_GetSysInitStatus();

    if (bSysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "sys not init yet, so DISP can't be inited\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pstDispAttr);

    memcpy(&stDispAttr, pstDispAttr, sizeof(HI_MAPI_DISP_ATTR_S));

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_INIT, &stDispAttr, sizeof(HI_MAPI_DISP_ATTR_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_INIT fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Deinit(HI_HANDLE DispHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_DEINIT, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_DEINIT fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_DISP_Start(HI_HANDLE DispHdl, HI_MAPI_DISP_VIDEOLAYER_ATTR_S *pstVideoLayerAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stVideoLayerAttr;

    DISP_CHECK_HANDLE(DispHdl);
    memcpy(&stVideoLayerAttr, pstVideoLayerAttr, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S));

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_START, &stVideoLayerAttr, sizeof(HI_MAPI_DISP_VIDEOLAYER_ATTR_S),
                           HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_START fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Stop(HI_HANDLE DispHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_STOP, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_STOP fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    if (u32Len == 0) {
        return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    switch (enCMD) {
        case HI_MAPI_DISP_CMD_VIDEO_CSC:
        case HI_MAPI_DISP_CMD_GRAPHIC_CSC: {
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    stMsgPriData.as32PrivData[0] = enCMD;
    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_SET_ATTREX, pAttr, u32Len, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_SET_ATTREX fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetAttrEx(HI_HANDLE DispHdl, HI_MAPI_DISP_CMD_E enCMD, HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    if (u32Len == 0) {
        return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    switch (enCMD) {
        case HI_MAPI_DISP_CMD_VIDEO_CSC:
        case HI_MAPI_DISP_CMD_GRAPHIC_CSC: {
            if (u32Len != sizeof(HI_MAPI_DISP_CSCATTREX_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_CSCATTREX_S\n",
                               u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    stMsgPriData.as32PrivData[0] = enCMD;
    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_GET_ATTREX, pAttr, u32Len, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_GET_ATTREX fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
                                  const HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstWndAttr);

    memcpy(&stWndAttr, pstWndAttr, sizeof(HI_MAPI_DISP_WINDOW_ATTR_S));

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_SET_WINDOW_ATTR, &stWndAttr, sizeof(HI_MAPI_DISP_WINDOW_ATTR_S),
                           HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_SET_WINDOW_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetWindowAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl,
    HI_MAPI_DISP_WINDOW_ATTR_S *pstWndAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pstWndAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_GET_WINDOW_ATTR, pstWndAttr, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_GET_WINDOW_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
                                    HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    if (u32Len == 0) {
        return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    switch (enCMD) {
        case HI_MAPI_DISP_WINDOW_CMD_ROTATE: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ROTATE_S\n", u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        case HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ASPECTRATIO_S\n", u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd %d.\n", enCMD);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = enCMD;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_SET_WINDOW_ATTREX, pAttr, u32Len, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_SET_WINDOW_ATTREX fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetWindowAttrEx(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_MAPI_DISP_WINDOW_CMD_E enCMD,
    HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_NULL_PTR(pAttr);

    if (u32Len == 0) {
        return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    switch (enCMD) {
        case HI_MAPI_DISP_WINDOW_CMD_ROTATE: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ROTATE_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ROTATE_S\n", u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        case HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO: {
            if (u32Len != sizeof(HI_MAPI_DISP_WINDOW_ASPECTRATIO_S)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_DISP,
                    "Disp_SETCSC u32Len:%x is not the size of HI_MAPI_DISP_WINDOW_ASPECTRATIO_S\n", u32Len);
                return HI_MAPI_DISP_EILLEGAL_PARAM;
            }
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "Do not support cmd: %d.\n", enCMD);
            return HI_MAPI_DISP_EILLEGAL_PARAM;
    }

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = enCMD;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_GET_WINDOW_ATTREX, pAttr, u32Len, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_GET_WINDOW_ATTREX fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StartWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_START_WINDOW, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_START_WINDOW fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StopWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_STOP_WINDOW, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_STOP_WINDOW fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_ClearWindow(HI_HANDLE DispHdl, HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_CLEAR_WINDOW, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_CLEAR_WINDOW fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
    const HI_MAPI_OSD_ATTR_S *pstOsdAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;
    HI_MAPI_OSD_ATTR_S stOsdAttr;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_OSD_HANDLE(OsdHdl);
    DISP_CHECK_NULL_PTR(pstOsdAttr);

    memcpy(&stOsdAttr, pstOsdAttr, sizeof(HI_MAPI_OSD_ATTR_S));

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = OsdHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_SET_WINDOW_OSD_ATTR, &stOsdAttr, sizeof(HI_MAPI_OSD_ATTR_S),
                           &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_SET_WINDOW_OSD_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_GetWindowOSDAttr(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl,
    HI_MAPI_OSD_ATTR_S *pstOsdAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_OSD_HANDLE(OsdHdl);
    DISP_CHECK_NULL_PTR(pstOsdAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = OsdHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_GET_WINDOW_OSD_ATTR, pstOsdAttr, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_GET_WINDOW_OSD_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StartWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_OSD_HANDLE(OsdHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = OsdHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_START_WINDOW_OSD, HI_NULL, 0, &stMsgPriData);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_START_WINDOW_OSD fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_StopWindowOSD(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE OsdHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_OSD_HANDLE(OsdHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = OsdHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_STOP_WINDOW_OSD, HI_NULL, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_STOP_WINDOW_OSD fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_SendFrame(HI_HANDLE DispHdl, HI_HANDLE WndHdl, const HI_MAPI_FRAME_DATA_S *pstFramedata)
{
    MAPI_UNUSED(DispHdl);
    MAPI_UNUSED(WndHdl);
    MAPI_UNUSED(pstFramedata);

    MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "client don't support sendframe!!\n");
    return HI_MAPI_DISP_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_DISP_Bind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
    HI_BOOL bStitch)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VPSSDVE_HANDLE(GrpHdl);
    DISP_CHECK_VPSSVPORT_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = GrpHdl;
    stMsgPriData.as32PrivData[1] = VPortHdl;
    stMsgPriData.as32PrivData[2] = bStitch;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_BIND_VPROC, HI_NULL, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_BIND_VPROC fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_UnBind_VProc(HI_HANDLE GrpHdl, HI_HANDLE VPortHdl, HI_HANDLE DispHdl, HI_HANDLE WndHdl,
    HI_BOOL bStitch)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VPSSDVE_HANDLE(GrpHdl);
    DISP_CHECK_VPSSVPORT_HANDLE(VPortHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);
    stMsgPriData.as32PrivData[0] = GrpHdl;
    stMsgPriData.as32PrivData[1] = VPortHdl;
    stMsgPriData.as32PrivData[2] = bStitch;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_UNBIND_VPROC, HI_NULL, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_UNBIND_VPROC fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_Bind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl,
    HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VCAPPIPE_HANDLE(VcapPipeHdl);
    DISP_CHECK_VCAPCHN_HANDLE(PipeChnHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);

    stMsgPriData.as32PrivData[0] = VcapPipeHdl;
    stMsgPriData.as32PrivData[1] = PipeChnHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_BIND_VCAP, HI_NULL, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_BIND_VCAP fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_DISP_UnBind_VCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE DispHdl,
    HI_HANDLE WndHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stMsgPriData;

    DISP_CHECK_HANDLE(DispHdl);
    DISP_CHECK_WIN_ID(WndHdl);
    DISP_CHECK_VCAPPIPE_HANDLE(VcapPipeHdl);
    DISP_CHECK_VCAPCHN_HANDLE(PipeChnHdl);

    u32ModFd = MODFD(HI_MAPI_MOD_DISP, DispHdl, WndHdl);

    stMsgPriData.as32PrivData[0] = VcapPipeHdl;
    stMsgPriData.as32PrivData[1] = PipeChnHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_DISP_UNBIND_VCAP, HI_NULL, 0, &stMsgPriData);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_DISP, "MSG_CMD_DISP_UNBIND_VCAP fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


