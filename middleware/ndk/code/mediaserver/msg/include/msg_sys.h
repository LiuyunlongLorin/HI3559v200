/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_sys.c
 * @brief   msg sys header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_SERVER_SYS_H__
#define __MSG_SERVER_SYS_H__

typedef enum hiMSG_MEDIA_SYS_CMD_E {
    MSG_CMD_MEDIA_SYS_INIT = 0,
    MSG_CMD_MEDIA_SYS_DEINIT,
    MSG_CMD_MEDIA_SYS_ALLOC_BUFFER,
    MSG_CMD_MEDIA_SYS_FREE_BUFFER,
    MSG_CMD_MEDIA_SYS_ENABLE_DSP,
    MSG_CMD_MEDIA_SYS_DISABLE_DSP,
} MSG_MEDIA_SYS_CMD_E;

typedef struct hiMSG_ALLOCBUFFER_S {
    HI_U64 u64PhyAddr;
    HI_U32 u32Len;
    HI_CHAR aszstrName[128];
} MSG_ALLOCBUFFER_S;

typedef struct hiMSG_FREEBUFFER_S {
    HI_U64 u64PhyAddr;
    HI_VOID *pVirtAddr;
    HI_U32 u32Len;
} MSG_FREEBUFFER_S;
#endif /* __MSG_SERVER_SYS_H__ */
