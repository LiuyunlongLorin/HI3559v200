/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_acap.h
 * @brief   msg acap header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_SERVER_ACAP_H__
#define __MSG_SERVER_ACAP_H__

#include "hi_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     MSG */
/** @{ */ /** <!-- [MSG] */

typedef enum tagMSG_ACAP_CMD_E {
    MSG_CMD_ACAP_INIT,
    MSG_CMD_ACAP_DEINIT,
    MSG_CMD_ACAP_START,
    MSG_CMD_ACAP_STOP,
    MSG_CMD_ACAP_ENABLE_VQE,
    MSG_CMD_ACAP_DISABLE_VQE,
    MSG_CMD_ACAP_SET_VOL,
    MSG_CMD_ACAP_GET_VOL,
    MSG_CMD_ACAP_MUTE,
    MSG_CMD_ACAP_UNMUTE,
    MSG_CMD_ACAP_GET_FRAME,
    MSG_CMD_ACAP_RELEASE_FRAME,
    MSG_CMD_ACAP_BUTT
} MSG_ACAP_CMD_E;

typedef struct tagMSG_ACAP_FRAME_S {
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;
} MSG_ACAP_FRAME_S;

/** @} */ /** <!-- ==== MSG End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MSG_SERVER_ACAP_H__ */
