/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_venc.h
 * @brief   msg venc header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_SERVER_VENC_H__
#define __MSG_SERVER_VENC_H__

typedef enum tagMSG_VENC_CMD_E {
    MSG_CMD_VENC_INIT = 0,
    MSG_CMD_VENC_SET_DATAFIFO_STATUS,
    MSG_CMD_VENC_DEINIT,
    MSG_CMD_VENC_START,
    MSG_CMD_VENC_STOP_READY,
    MSG_CMD_VENC_STOP,
    MSG_CMD_VENC_SET_ATTR,
    MSG_CMD_VENC_GET_ATTR,
    MSG_CMD_VENC_BIND_VCAP,
    MSG_CMD_VENC_UNBIND_VCAP,
    MSG_CMD_VENC_BIND_VPROC,
    MSG_CMD_VENC_UNBIND_VPROC,
    MSG_CMD_VENC_REQ_IDR,
    MSG_CMD_VENC_SET_EXTEND_ATTR,
    MSG_CMD_VENC_GET_EXTEND_ATTR,
    MSG_CMD_VENC_SET_OSD_ATTR,
    MSG_CMD_VENC_GET_OSD_ATTR,
    MSG_CMD_VENC_START_OSD,
    MSG_CMD_VENC_STOP_OSD,

} MSG_VENC_CMD_E;

#endif /* __MSG_SERVER_VENC_H__ */


