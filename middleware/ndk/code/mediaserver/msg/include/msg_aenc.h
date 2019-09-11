/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_aenc.h
 * @brief   msg aenc header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __MSG_SERVER_AENC_H__
#define __MSG_SERVER_AENC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AENC */
/** @{ */ /** <!-- [AENC] */

typedef enum tagMSG_AENC_CMD_E {
    MSG_CMD_AENC_INIT = 0,
    MSG_CMD_AENC_DEINIT,
    MSG_CMD_AENC_START,
    MSG_CMD_AENC_STOP,
    MSG_CMD_AENC_STOP_FINISH,
    MSG_CMD_AENC_BIND_ACAP,
    MSG_CMD_AENC_UNBIND_ACAP,
    MSG_CMD_AENC_REGISTER_AENCODER,
    MSG_CMD_AENC_UNREGISTER_AENCODER,
    MSG_CMD_AENC_SEND_FRAME

} MSG_AENC_CMD_E;

/** @} */ /** <!-- ==== AENC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MSG_SERVER_AENC_H__ */
