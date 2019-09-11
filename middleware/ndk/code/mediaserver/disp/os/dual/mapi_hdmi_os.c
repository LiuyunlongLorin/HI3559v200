/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_hdmi_os.c
 * @brief   hdmi os function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_mapi_hdmi_define.h"
#include "hi_ipcmsg.h"
#include "msg_define.h"
#include "hi_mapi_log.h"
#include "msg_hdmi.h"
#include "mapi_comm_inner.h"
#include "hi_comm_hdmi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

void MAPI_HDMI_MessageResp(HI_IPCMSG_MESSAGE_S *pResp)
{
    MAPI_INFO_TRACE(HI_MAPI_MOD_HDMI, "MAPI_HDMI_EventUpload receive async resp: %s, return:%x\n",
                    (char *)pResp->pBody, pResp->s32RetVal);
    return;
}

static HI_VOID MAPI_HDMI_EventUpload(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ModFd;
    HI_IPCMSG_MESSAGE_S *pReq = HI_NULL;
    MAPI_UNUSED(pPrivateData);

    switch (event) {
        case HI_HDMI_EVENT_HOTPLUG:

            MAPI_INFO_TRACE(HI_MAPI_MOD_HDMI, " ### hdmi in!###\n");

            u32ModFd = MODFD(HI_MAPI_MOD_HDMI, 0, 0);
            pReq = HI_IPCMSG_CreateMessage(u32ModFd, MSG_CMD_HDMI_EVENT_HOTPLUG, HI_NULL, 0);
            if (pReq == HI_NULL) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateMessage return NULL.\n");
                return;
            }

            s32Ret = HI_IPCMSG_SendAsync(MAPI_Media_MSG_GetSiId(), pReq, MAPI_HDMI_MessageResp);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_SendASync fail s32Ret:%x\n", s32Ret);
                HI_IPCMSG_DestroyMessage(pReq);
                return;
            }
            HI_IPCMSG_DestroyMessage(pReq);
            break;

        case HI_HDMI_EVENT_NO_PLUG:

            MAPI_INFO_TRACE(HI_MAPI_MOD_HDMI, " ### hdmi out!###\n");

            u32ModFd = MODFD(HI_MAPI_MOD_HDMI, 0, 0);
            pReq = HI_IPCMSG_CreateMessage(u32ModFd, MSG_CMD_HDMI_EVENT_NOPLUG, HI_NULL, 0);
            if (pReq == HI_NULL) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_CreateMessage return NULL.\n");
                return;
            }

            s32Ret = HI_IPCMSG_SendAsync(MAPI_Media_MSG_GetSiId(), pReq, MAPI_HDMI_MessageResp);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_IPCMSG_SendASync fail s32Ret:%x\n", s32Ret);
                HI_IPCMSG_DestroyMessage(pReq);
                return;
            }
            HI_IPCMSG_DestroyMessage(pReq);

            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "un-known hdmi event:%d\n", event);
            break;
    }

    return;
}

HI_S32 HAL_MAPI_HDMI_Config_CallbackFunc(const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc,
                                         HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackconfig)
{
    MAPI_UNUSED(pstHdmiCallBackFunc);
    pstHdmiCallBackconfig->pfnHdmiEventCallback = MAPI_HDMI_EventUpload;
    pstHdmiCallBackconfig->pPrivateData = HI_NULL;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

