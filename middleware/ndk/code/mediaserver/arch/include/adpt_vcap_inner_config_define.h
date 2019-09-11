/**
* Copyright (C), 2017-2018, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file    adpt_vcap_inner_config_define.h
* @brief   NDK mapi functions declaration
* @author  HiMobileCam NDK develop team
* @date    2018/2/7
*/
#ifndef __ADPT_VCAP_INNER_CONFIG_DEFINE__
#define __ADPT_VCAP_INNER_CONFIG_DEFINE__

#include "hi_type.h"
#include "hi_common.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_mapi_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagVCAP_INNER_CONFIG_ATTR_S {
    VI_DEV_ATTR_S astDevAttr[HI_MAPI_VCAP_MAX_DEV_NUM];
    VI_PIPE_ATTR_S astPipeAttr[HI_MAPI_VCAP_MAX_PIPE_NUM];
    VI_CHN_ATTR_S astChnAttr[HI_MAPI_PIPE_MAX_CHN_NUM];
} VCAP_INNER_CONFIG_ATTR_S;

HI_VOID MAPI_VCAP_LoadParam(VCAP_INNER_CONFIG_ATTR_S *pstVcap, const HI_S32 s32SensorType[]);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __ADPT_VCAP_INNER_CONFIG_DEFINE__ */
