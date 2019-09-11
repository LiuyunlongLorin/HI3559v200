/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_hdmi_inner.h
 * @brief   server vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "hi_mapi_errno.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_log.h"

#ifndef __MAPI_HDMI_H__
#define __MAPI_HDMI_H__

#define HDMI_CHECK_HANDLE(Hdl) \
    do { \
        if ((Hdl) >= HI_HDMI_ID_BUTT) { \
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Hdmi handle(%d) is invalid, only support [0, %d]\n", Hdl, \
                           HI_HDMI_ID_BUTT - 1); \
            return HI_MAPI_HDMI_EINVALID_DEVID; \
        } \
    } while (0)

#define HDMI_CHECK_NULL_PTR(pstPtr)                                                                            \
    do {                                                                                                       \
        if (NULL == (pstPtr)) {                                                                                \
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Null pointer! FUNCTION %s, LINE %d!\n", __FUNCTION__, __LINE__); \
            return HI_MAPI_HDMI_ENULL_PTR;                                                                     \
        }                                                                                                      \
    } while (0)

#define HDMI_CHECK_INIT(state)                                              \
    do {                                                                    \
        if (!state) {                                                       \
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n"); \
            return HI_MAPI_HDMI_ENOTREADY;                                  \
        }                                                                   \
    } while (0)

typedef struct tagMAPI_HDMI_CONTEXT_S {
    HI_BOOL bHdmiInited;
    HI_BOOL bHdmiStarted;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCBFunc;
} MAPI_HDMI_CONTEXT_S;

HI_VOID MAPI_GET_HDMI_CALLBACK(HI_HDMI_CALLBACK_FUNC_S **ppstHdmiCallback);

#endif


