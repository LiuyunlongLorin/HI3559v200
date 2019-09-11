/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_sys_adapt.h
 * @brief   NDK sys specification Type Definition.(hi3559v200)
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#ifndef __MAPI_SYS_ADAPT_H__
#define __MAPI_SYS_ADAPT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum hiSVP_DSP_ID_E {
    SVP_DSP_ID_0 = 0x0,
    SVP_DSP_ID_BUTT
} SVP_DSP_ID_E;

typedef enum hiSVP_DSP_MEM_TYPE_E {
    SVP_DSP_MEM_TYPE_SYS_DDR_DSP_0 = 0x0,
    SVP_DSP_MEM_TYPE_IRAM_DSP_0 = 0x1,
    SVP_DSP_MEM_TYPE_DRAM_0_DSP_0 = 0x2,
    SVP_DSP_MEM_TYPE_DRAM_1_DSP_0 = 0x3,
    SVP_DSP_MEM_TYPE_BUTT
} SVP_DSP_MEM_TYPE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MAPI_SYS_ADAPT_H__ */
