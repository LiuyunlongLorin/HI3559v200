/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_aenc_define.h
 * @brief   aenc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_AENC_DEFINE_H__
#define __HI_MAPI_AENC_DEFINE_H__

#include "hi_comm_aio.h"
#include "hi_comm_aenc.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AENC */
/** @{ */ /** <!-- [AENC] */

/* Error code
    invalid handle error for aenc */
#define HI_MAPI_AENC_EINVALIDHDL HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_CHNID)

/** null pointer error for aenc */
#define HI_MAPI_AENC_ENULLPTR    HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)

/** not inited error for aenc */
#define HI_MAPI_AENC_ENOTINITED  HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)

/** state error for aenc */
#define HI_MAPI_AENC_ESTATEERR   HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)

/** illegal parameter error for aenc */
#define HI_MAPI_AENC_EILLPARAM   HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)

/** resource full error for aenc */
#define HI_MAPI_AENC_ERESFULL    HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_EXIST)

/** resource not exist error for aenc */
#define HI_MAPI_AENC_EUNEXIST    HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)

/** memory error for aenc */
#define HI_MAPI_AENC_EMEMERR     HI_MAPI_DEF_ERR(HI_MAPI_MOD_AENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)

/* Structure */
/** aenc init attr */
typedef struct hiMAPI_AENC_ATTR_S {
    HI_MAPI_AUDIO_FORMAT_E enAencFormat; /**< audio encode format type */
    HI_U32 u32PtNumPerFrm;               /**< sampling point number per frame */
    HI_VOID *pValue;                     /**< pointer of Aenc attr */
    HI_U32 u32Len;                       /**< Aenc attr struct len */
} HI_MAPI_AENC_ATTR_S;

/** data proc callback function pointer */
typedef HI_S32 (*HI_MAPI_AENC_DATAPROC_CALLBACK_FN_PTR)(HI_HANDLE AencHdl, const AUDIO_STREAM_S *pAStreamData,
    HI_VOID *pPrivateData);

/** aenc callback struct */
typedef struct hiMAPI_AENC_CALLBACK_S {
    HI_MAPI_AENC_DATAPROC_CALLBACK_FN_PTR pfnDataCB;
    HI_VOID *pPrivateData; /** private data */
} HI_MAPI_AENC_CALLBACK_S;

/** @} */ /** <!-- ==== AENC End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_AENC_H__ */
