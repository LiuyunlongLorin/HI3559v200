/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_sys.h
 * @brief   sys module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_SYS_H__
#define __HI_MAPI_SYS_H__

#include "hi_mapi_errno.h"
#include "hi_mapi_comm_define.h"
#include "mapi_sys_adapt.h"

#define HI_MAPI_SYS_EILLEGAL_PARAM HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
#define HI_MAPI_SYS_ENULL_PTR      HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
#define HI_MAPI_SYS_ENOT_PERM      HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
#define HI_MAPI_SYS_ENOTREADY      HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
#define HI_MAPI_SYS_EBUSY          HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUSY)
#define HI_MAPI_SYS_ENOMEM         HI_MAPI_DEF_ERR(HI_MAPI_MOD_SYS, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_MAPI_PATH_LEN 64

typedef struct hiMAPI_Sys_DspAttr_S {
#ifdef SUPPORT_PHOTO_POST_PROCESS
    HI_CHAR aszBinPath[SVP_DSP_MEM_TYPE_BUTT][HI_MAPI_PATH_LEN];
    SVP_DSP_ID_E enDsp;
#endif
} HI_MAPI_Sys_DspAttr_S;

/** \addtogroup     COMM */
/** @{ */ /** <!-- [COMM] */

/**
 * @brief Sys initialization,including message initialization for communicating between HuaweiLite and Linux.
 *        It must be called by both HuaweiLite's app and Linux's app.It must be called the very first.
 * @return 0  Sys initialize success.
 * @return non 0  Sys initialize fail.
 */
HI_S32 HI_MAPI_Sys_Init(HI_VOID);

/**
 * @brief Sys de-initialize.
 * @return 0  Sys de-initialize success.
 * @return non 0  Sys de-initialize fail.
 */
HI_S32 HI_MAPI_Sys_Deinit(HI_VOID);

/**
 * @brief Sys Get-VPSSVENCWrapBufferLine.
 * @return 0  Sys Get-VPSSVENCWrapBufferLine success.
 * @return non 0  Sys Get-VPSSVENCWrapBufferLine fail.
 */
HI_S32 HI_MAPI_Sys_GetVprocVencWrapBufferLine(VPSS_VENC_WRAP_PARAM_S *pWrapParam, HI_U32 *pu32BufLine);

/**
 * @brief dsp enable.
 * @return 0  dsp enable success.
 * @return non 0  dsp enable fail.
 */
HI_S32 HI_MAPI_Sys_EnableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr);

/**
 * @brief dsp disable.
 * @return 0  dsp disable success.
 * @return non 0  dsp disable fail.
 */
HI_S32 HI_MAPI_Sys_DisableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr);

/**
 * @brief Media initialization.
 *        It must be called before other media MAPI,such as HI_MAPI_VCap_XXX,HI_MAPI_VProc_XXX,HI_MAPI_VEnc_XXX,HI_MAPI_Vo_XXX.etc.
 * @param[in] pstMediaAttr HI_MAPI_MEDIA_ATTR_S*:pointer of Media initialization attribute.
 * @return 0  Media initialize success.
 * @return non 0  Media initialize fail.
 */
HI_S32 HI_MAPI_Media_Init(const HI_MAPI_MEDIA_ATTR_S *pstMediaAttr);

/**
 * @brief Media de-initialize.
 * @return 0  Media de-initialize success.
 * @return non 0  Media de-initialize fail.
 */
HI_S32 HI_MAPI_Media_Deinit(HI_VOID);

/**
 * @brief set venc datafifo node num.
 * @param[in] pstDatafifoCfg HI_MAPI_DATAFIFO_CONFIG_S *:pointer of datafifo node num attribute..
 * @return 0  set venc datafifo node num successful.
 * @return non 0  set venc datafifo node num fail.
 */
HI_S32 HI_MAPI_Sys_SetDatafifoNodeNum(HI_MAPI_DATAFIFO_CONFIG_S *pstDatafifoCfg);

/**
 * @brief Allocate buffer from MMZ.
 * @param[out] pu64PhyAddr HI_U64*:The physical address of buffer.
 * @param[out] ppVirtAddr HI_VOID **:The virtual address of buffer.
 * @param[in] u32Len HI_U32:The length of buffer.
 * @param[in] pstrName HI_CHAR*:The name of buffer.
 * @return 0  Allocate buffer successful.
 * @return non 0  Allocate buffer fail.
 */
HI_S32 HI_MAPI_AllocBuffer(HI_U64 *pu64PhyAddr, HI_VOID **ppVirtAddr, HI_U32 u32Len, const HI_CHAR *pstrName);

/**
 * @brief Free buffer from MMZ.
 * @param[in] pu64PhyAddr HI_U64*:The physical address of buffer.
 * @param[in] pVirtAddr HI_VOID *:The virtual address of buffer.
 * @param[in] u32Len HI_U32:The length of buffer.
 * @return 0  Free buffer successful.
 * @return non 0  Free buffer fail.
 */
HI_S32 HI_MAPI_FreeBuffer(HI_U64 u64PhyAddr, HI_VOID *pVirtAddr, HI_U32 u32Len);

/** @} */ /** <!-- ==== COMM End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_SYS_H__ */
