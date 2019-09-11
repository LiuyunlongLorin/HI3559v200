/**
* Copyright (C),  2017-2018,  Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file        mapi_hdmi_client.c
* @brief      hdmi client module
* @author   HiMobileCam NDK Develop Team
* @date      2017/12/15
*/
#include <stdio.h>
#include "hi_type.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_hdmi_define.h"
#include "hi_ipcmsg.h"
#include "msg_hdmi.h"
#include "msg_define.h"
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "hi_mapi_log.h"
#include "mapi_hdmi_inner.h"
#include "msg_wrapper.h"
#include "hi_mapi_sys.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_HDMI_CALLBACK_FUNC_S g_stHdmiHandle = { HI_NULL, HI_NULL };

HI_VOID MAPI_GET_HDMI_CALLBACK(HI_HDMI_CALLBACK_FUNC_S **ppstHdmiCallback)
{
    *ppstHdmiCallback = &g_stHdmiHandle;
    return;
}

HI_S32 HI_MAPI_HDMI_Init(HI_HDMI_ID_E enHdmi, const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_BOOL bSysInit;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstHdmiCallBackFunc);

    bSysInit = MAPI_GetSysInitStatus();
    if (bSysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "sys not init yet, so HDMI can't be inited\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_INIT, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_INIT fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    memcpy(&g_stHdmiHandle, pstHdmiCallBackFunc, sizeof(HI_HDMI_CALLBACK_FUNC_S));
    return s32Ret;
}

HI_S32 HI_MAPI_HDMI_Deinit(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    HDMI_CHECK_HANDLE(enHdmi);

    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_DEINIT, HI_NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_DEINIT fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    memset(&g_stHdmiHandle, 0, sizeof(HI_HDMI_CALLBACK_FUNC_S));
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_GetSinkCapability(HI_HDMI_ID_E enHdmi,
    HI_MAPI_HDMI_SINKCAPABILITY_S *pstsinkcapabiility)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstsinkcapabiility);
    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_GET_SINKCAPBILITY, pstsinkcapabiility,
                           sizeof(HI_MAPI_HDMI_SINKCAPABILITY_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_GET_SINKCAPBILITY fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_GetAttr(HI_HDMI_ID_E enHdmi, HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstHdmiAttr);
    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_GET_ATTR, pstHdmiAttr, sizeof(HI_MAPI_HDMI_ATTR_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_GET_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_E enHdmi, const HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_HDMI_ATTR_S stHdmiAttr;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstHdmiAttr);

    memcpy(&stHdmiAttr, pstHdmiAttr, sizeof(HI_MAPI_HDMI_ATTR_S));
    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_SET_ATTR, &stHdmiAttr, sizeof(HI_MAPI_HDMI_ATTR_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_SET_ATTR fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_Start(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    HDMI_CHECK_HANDLE(enHdmi);
    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_START, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_START fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_Stop(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    HDMI_CHECK_HANDLE(enHdmi);
    u32ModFd = MODFD(HI_MAPI_MOD_HDMI, (HI_U32)enHdmi, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_HDMI_STOP, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "MSG_CMD_HDMI_STOP fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


