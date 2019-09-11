/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_sys_adapt.c
 * @brief   NDK sys server arch functions(hi3559v200)
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include "hi_mapi_sys.h"
#include "mapi_comm_inner.h"
#include "hi_mapi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 HAL_MAPI_Sys_EnableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "mpi dsp module is not support !\n");
    return HI_MAPI_SYS_ENOT_PERM;
}

HI_S32 HAL_MAPI_Sys_DisableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "mpi dsp module is not support !\n");
    return HI_MAPI_SYS_ENOT_PERM;
}

HI_VOID HAL_MAPI_VB_SetSupplementConfig(VB_SUPPLEMENT_CONFIG_S *pstSupplementConf)
{
    pstSupplementConf->u32SupplementConfig = VB_SUPPLEMENT_JPEG_MASK | VB_SUPPLEMENT_ISPINFO_MASK |
                                             VB_SUPPLEMENT_DNG_MASK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
