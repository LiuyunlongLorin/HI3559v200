/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_define.h
 * @brief   msg define header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_DEFINE_H__
#define __MSG_DEFINE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/******************************************************************************
|----------------------------------------------------------------|
| |   MOD_ID    |   DEV_ID    |   CHN_ID    |      reserve         |
|----------------------------------------------------------------|
|<--8bits----><--8bits --><--8bits --><-----8bits---->|
******************************************************************************/

#define MODFD(MOD, DevID, ChnID) \
    ((HI_U32)((MOD & 0xFF) << 24 | ((DevID & 0xFF) << 16) | ((ChnID & 0xFF) << 8)))

#define GET_MOD_ID(ModFd) \
    (((ModFd) >> 24) & 0xFF)

#define GET_DEV_ID(ModFd) \
    (((ModFd) >> 16) & 0xFF)

#define GET_CHN_ID(ModFd) \
    (((ModFd) >> 8) & 0xFF)

HI_S32 MAPI_Media_MSG_GetSiId(HI_VOID);
#define MediaMsgId MAPI_Media_MSG_GetSiId()

#define HI_IPCMSG_SEND_SYNC_TIMEOUT 50000  // ms

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_VPROC_H__ */

