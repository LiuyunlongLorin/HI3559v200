/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    msg_wrapper.h
 * @brief   msg module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MSG_WRAPPER_H__
#define __MSG_WRAPPER_H__

#include "hi_comm_ipcmsg.h"

typedef struct tagMAPI_PRIV_DATA_S {
    HI_S32 as32PrivData[HI_IPCMSG_PRIVDATA_NUM];
} MAPI_PRIV_DATA_S;

HI_S32 MAPI_SendSync(HI_U32 u32Module, HI_U32 u32CMD, HI_VOID *pBody, HI_U32 u32BodyLen,
                     MAPI_PRIV_DATA_S *pstPrivData);

HI_S32 MAPI_SendSyncWithResp(HI_U32 u32Module, HI_U32 u32CMD, HI_VOID *pReqBody, HI_VOID *pRespBody,
                             HI_U32 u32ReqBodyLen, MAPI_PRIV_DATA_S *pstPrivData);

#endif /* __MAPI_CLIENT_H__ */
