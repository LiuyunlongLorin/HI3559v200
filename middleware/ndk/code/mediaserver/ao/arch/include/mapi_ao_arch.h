/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_ao_arch.h
 * @brief   NDK ao arch head file.
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#ifndef __MAPI_AO_ARCH_H__
#define __MAPI_AO_ARCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_AO_SetExternAttr(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S *pstAoAttr, AIO_ATTR_S *pstAioAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_AO_ARCH_H__ */
