/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_ao.h
 * @brief   msg ao header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __MSG_AO_H__
#define __MSG_AO_H__

#include "hi_mapi_ao_define.h"
#include "hi_mapi_ao.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AO */
/** @{ */ /** <!-- [AO] */

typedef enum tagMSG_AO_CMD_E {
    MSG_CMD_AO_INIT,
    MSG_CMD_AO_DEINIT,

    MSG_CMD_AO_START,
    MSG_CMD_AO_STOP,

    MSG_CMD_AO_SET_VOLUME,
    MSG_CMD_AO_GET_VOLUME,

    MSG_CMD_AO_MUTE,
    MSG_CMD_AO_UNMUTE,

    MSG_CMD_AO_SEND_FRAME,
    MSG_CMD_AO_SEND_SYS_FRAME

} MSG_AO_CMD_E;

typedef struct tagMSG_AO_FRAME_S {
    AUDIO_FRAME_S stAFrm;
    HI_U32 u32Timeout;
} MSG_AO_FRAME_S;

/** @} */ /** <!-- ==== AO ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
