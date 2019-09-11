/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_ao_adapt.c
 * @brief   NDK ao server arch functions(hi3559v200)
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include "mapi_ao_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_VOID HAL_MAPI_AO_InitContext(MAPI_AO_CONTEXT_S *pstAoContext)
{
    pstAoContext->enAoState = MAPI_AO_UNINITED;
    pstAoContext->stAoAttr.u32ChnCnt = 2;
    pstAoContext->stAoAttr.u32ClkSel = 0;
    pstAoContext->stAoAttr.u32EXFlag = 0;
    pstAoContext->stAoAttr.u32FrmNum = 4;
    pstAoContext->stAoAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
