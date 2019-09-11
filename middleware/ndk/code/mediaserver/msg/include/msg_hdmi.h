/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_hdmi.h
 * @brief   msg hdmi header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __MSG_HDMI_H__
#define __MSG_HDMI_H__

typedef enum tagMSG_HDMI_CMD_E {
    MSG_CMD_HDMI_INIT = 0,
    MSG_CMD_HDMI_DEINIT,
    MSG_CMD_HDMI_GET_SINKCAPBILITY,
    MSG_CMD_HDMI_SET_ATTR,
    MSG_CMD_HDMI_GET_ATTR,
    MSG_CMD_HDMI_START,
    MSG_CMD_HDMI_STOP,
    MSG_CMD_HDMI_EVENT_HOTPLUG,
    MSG_CMD_HDMI_EVENT_NOPLUG,
} MSG_HDMI_CMD_E;

#endif /* __MSG_HDMI_H__ */
