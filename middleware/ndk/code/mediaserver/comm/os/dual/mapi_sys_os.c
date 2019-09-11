/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_sys_os.c
 * @brief   NDK sys server arch functions(dual os)
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include "msg_server.h"
#include "mapi_comm_inner.h"
#include "hi_mapi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_SYS_Init(HI_VOID)
{
    HI_S32 s32Ret;

    s32Ret = MAPI_Media_MSG_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "Media_MSG_Init fail,ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_SYS_Deinit(HI_VOID)
{
    HI_S32 s32Ret;

    s32Ret = MAPI_Media_MSG_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "Media_MSG_Deinit fail,ret:%x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
