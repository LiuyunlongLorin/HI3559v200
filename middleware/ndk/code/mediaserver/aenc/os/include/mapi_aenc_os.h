/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_aenc_os.c
 * @brief   server aenc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_AENC_OS_H__
#define __MAPI_AENC_OS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_VOID MAPI_AENC_DatafifoCfg_Init(HI_VOID);
HI_VOID MAPI_AENC_DatafifoCfg_Deinit(HI_VOID);
HI_BOOL MAPI_AENC_IsRegisterCallback(HI_HANDLE AencHdl, MAPI_AENC_CHN_S *pstAencChn);
HI_S32 MAPI_AENC_GetAStream(HI_HANDLE AencHdl, MAPI_AENC_CHN_S *pstAencChn);
HI_VOID MAPI_AENC_ClearDatafifo(HI_HANDLE AencHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_AENC_OS_H__ */

