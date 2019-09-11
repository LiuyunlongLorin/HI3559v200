/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_venc_arch.h
 * @brief   venc arch module header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __MAPI_VENC_ARCH_H__
#define __MAPI_VENC_ARCH_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_VENC_ConfigH265CarTrans(HI_HANDLE VencHdl, HI_MAPI_PAYLOAD_TYPE_E enType,
                                        HI_MAPI_VENC_SCENE_MODE_E enSceneMode);
HI_S32 HAL_MAPI_VENC_ConfigH264Attr(const HI_MAPI_VENC_RC_ATTR_S *pstRcAttr, VENC_CHN_ATTR_S *pstVencChnAttr);
HI_S32 HAL_MAPI_VENC_ConfigH265Attr(const HI_MAPI_VENC_RC_ATTR_S *pstRcAttr, VENC_CHN_ATTR_S *pstVencChnAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_VENC_ARCH_H__ */
