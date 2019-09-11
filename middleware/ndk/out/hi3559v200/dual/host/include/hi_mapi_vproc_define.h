/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_vproc_define.h
 * @brief   vproc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_VPROC_DEFINE_H__
#define __HI_VPROC_DEFINE_H__

#include "hi_comm_video.h"
#include "hi_comm_vpss.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"

#ifdef SUPPORT_STITCH
#include "hi_comm_avs.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HI_MAPI_VPROC_ENULL_PTR      HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
#define HI_MAPI_VPROC_ENOTREADY \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
#define HI_MAPI_VPROC_EINVALID_DEVID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_DEVID)
#define HI_MAPI_VPROC_EINVALID_CHNID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_CHNID)
#define HI_MAPI_VPROC_EEXIST         HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_EXIST)
#define HI_MAPI_VPROC_EUNEXIST       HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)
#define HI_MAPI_VPROC_ENOT_SUPPORT \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_SUPPORT)
#define HI_MAPI_VPROC_ENOT_PERM      HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
#define HI_MAPI_VPROC_ENOMEM         HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)
#define HI_MAPI_VPROC_ENOBUF         HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOBUF)
#define HI_MAPI_VPROC_EILLEGAL_PARAM \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
#define HI_MAPI_VPROC_EBUSY          HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUSY)
#define HI_MAPI_VPROC_EBUF_EMPTY     HI_MAPI_DEF_ERR(HI_MAPI_MOD_VPROC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUF_EMPTY)

typedef struct hiMAPI_VPSS_ATTR_S {
    HI_U32 u32MaxW;
    HI_U32 u32MaxH;
    FRAME_RATE_CTRL_S stFrameRate;
    PIXEL_FORMAT_E enPixelFormat;
    HI_BOOL bNrEn;
    VPSS_NR_ATTR_S stNrAttr;
} HI_MAPI_VPSS_ATTR_S;

typedef struct hiMAPI_VPORT_ATTR_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_BOOL bSupportBufferShare;
    FRAME_RATE_CTRL_S stFrameRate;
    VIDEO_FORMAT_E enVideoFormat;
    PIXEL_FORMAT_E enPixelFormat;
    COMPRESS_MODE_E enCompressMode;
    ASPECT_RATIO_S stAspectRatio;
} HI_MAPI_VPORT_ATTR_S;

/** vproc cmd list */
typedef enum hiMAPI_VPROC_CMD_E {
    HI_VPROC_CMD_PortMirror,
    HI_VPROC_CMD_PortFlip,
    HI_VPROC_CMD_PortRotate,
    HI_VPROC_CMD_PortBufWrap,
    HI_VPROC_CMD_BUTT
} HI_MAPI_VPROC_CMD_E;

/** VPROC_CMD_SetPortMirror,VPROC_CMD_GetPortMirror */
typedef struct hiMAPI_VPROC_MIRROR_ATTR_S {
    HI_BOOL bEnable;
} HI_MAPI_VPROC_MIRROR_ATTR_S;

/** VPROC_CMD_SetPortFlip,VPROC_CMD_GetPortFlip */
typedef struct hiMAPI_VPROC_FLIP_ATTR_S {
    HI_BOOL bEnable;
} HI_MAPI_VPROC_FLIP_ATTR_S;

typedef struct hiMAPI_DUMP_YUV_ATTR_S {
    HI_BOOL bEnable;
    HI_U32 u32Depth;
} HI_MAPI_DUMP_YUV_ATTR_S;

typedef enum hiMAPI_PHOTO_TYPE_E {
    VPROC_PHOTO_TYPE_HDR = 0x0,
    VPROC_PHOTO_TYPE_SFNR = 0x1,
    VPROC_PHOTO_TYPE_MFNR = 0x2,
    VPROC_PHOTO_TYPE_DE = 0x3,
    VPROC_PHOTO_TYPE_MFNR_DE = 0x4,
    VPROC_PHOTO_TYPE_BUTT
} HI_MAPI_PHOTO_TYPE_E;

typedef HI_S32 (*PFN_VPROC_BNRRawProc)(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S *pBNRRaw,
    HI_VOID *pPrivateData);

typedef struct hiDUMP_BNRRAW_CALLBACK_FUNC_S {
    PFN_VPROC_BNRRawProc pfunVProcBNRProc; /**< dumo bnr raw callback function */
    HI_VOID *pPrivateData;                 /**< Private data of the callback functions and parameters */
} HI_DUMP_BNRRAW_CALLBACK_FUNC_S;

typedef HI_S32 (*PFN_VPROC_YUVDataProc)(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S *pVPortYUV,
    HI_VOID *pPrivateData);

typedef struct hiDUMP_YUV_CALLBACK_FUNC_S {
    PFN_VPROC_YUVDataProc pfunVProcYUVProc; /**< dumo yuv callback function */
    HI_VOID *pPrivateData;                  /**< Private data of the callback functions and parameters */
} HI_DUMP_YUV_CALLBACK_FUNC_S;

typedef struct hiPHOTO_DUMP_CALLBACK_FUNC_S {
    HI_DUMP_YUV_CALLBACK_FUNC_S stYUVDataCB;
    HI_DUMP_BNRRAW_CALLBACK_FUNC_S stBNRRawCB;
} HI_PHOTO_DUMP_CALLBACK_FUNC_S;

#ifdef SUPPORT_STITCH
typedef struct hiVPROC_STITCH_SRC_S {
    HI_HANDLE VpssHdl;
    HI_HANDLE VPortHdl;
} HI_VPROC_STITCH_SRC_S;

typedef struct hiMAPI_STITCH_ATTR_S {
    AVS_MODE_E enMode;           /* Group work mode */
    HI_U32 u32PipeNum;           /* RW; Range: [1, 4]; Pipe number. */
    HI_BOOL bSyncPipe;           /* RW; Whether sync pipe image. */
    AVS_LUT_S stLUT;             /* Look up table. */
    AVS_GAIN_ATTR_S stGainAttr;  /* Gain attribute. */
    AVS_OUTPUT_ATTR_S stOutAttr; /* Output attribute. */
    FRAME_RATE_CTRL_S stFrameRate;
    HI_VPROC_STITCH_SRC_S astStitchSrc[HI_MAPI_STITCH_PIPE_MAX_NUM];
} HI_MAPI_STITCH_ATTR_S;

typedef struct hiMAPI_STITCH_PORT_ATTR_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
    COMPRESS_MODE_E enCompressMode;
} HI_MAPI_STITCH_PORT_ATTR_S;

#endif

/** @} */ /** <!-- ==== VPROC End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_VPROC_DEFINE_H__ */
