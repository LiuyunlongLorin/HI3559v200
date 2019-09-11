/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_disp_define.h
 * @brief   disp module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_DISP_DEFINE_H__
#define __HI_MAPI_DISP_DEFINE_H__

#include "hi_mapi_comm_define.h"
#include "hi_comm_vo.h"
#include "hi_mapi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     DISP */
/** @{ */ /** <!-- [DISP] */
#define HI_MAPI_DISP_ENULL_PTR      HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
#define HI_MAPI_DISP_ENOTREADY      HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
#define HI_MAPI_DISP_EINVALID_DEVID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_DEVID)
#define HI_MAPI_DISP_EINVALID_CHNID \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_CHNID)
#define HI_MAPI_DISP_ENOT_SUPPORT   HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_SUPPORT)
#define HI_MAPI_DISP_ENOT_PERM      HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
#define HI_MAPI_DISP_ENOMEM         HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)
#define HI_MAPI_DISP_EILLEGAL_PARAM \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
#define HI_MAPI_DISP_EBUSY          HI_MAPI_DEF_ERR(HI_MAPI_MOD_DISP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUSY)

typedef struct hiMAPI_DISP_USERINFO_ATTR_S {
    VO_USER_INTFSYNC_INFO_S stUserInfo;
    HI_U32 u32DevFrameRate;
} HI_MAPI_DISP_USERINFO_ATTR_S;

typedef struct hiMAPI_DISP_ATTR_S {
    VO_PUB_ATTR_S stPubAttr;
    HI_MAPI_DISP_USERINFO_ATTR_S stUserInfoAttr;
} HI_MAPI_DISP_ATTR_S;

typedef struct hiMAPI_DISP_VIDEOLAYER_ATTR_S {
    SIZE_S stImageSize;
    HI_U32 u32VLFrameRate;
    HI_U32 u32BufLen;
} HI_MAPI_DISP_VIDEOLAYER_ATTR_S;

typedef struct hiMAPI_DISP_WINDOW_ATTR_S {
    RECT_S stRect;
    HI_U32 u32Priority;
} HI_MAPI_DISP_WINDOW_ATTR_S;

typedef struct hiMAPI_DISP_CSCATTREX_S {
    VO_CSC_S stVoCSC;
} HI_MAPI_DISP_CSCATTREX_S;

typedef struct hiMAPI_DISP_WINDOW_ROTATE_S {
    ROTATION_E enDispRorate;
} HI_MAPI_DISP_WINDOW_ROTATE_S;

typedef struct hiMAPI_DISP_WINDOW_ASPECTRATIO_S {
    ASPECT_RATIO_S stAspectRatio;
} HI_MAPI_DISP_WINDOW_ASPECTRATIO_S;

typedef enum hiMAPI_DISP_CMD_E {
    HI_MAPI_DISP_CMD_VIDEO_CSC,
    HI_MAPI_DISP_CMD_GRAPHIC_CSC,
    HI_MAPI_DISP_CMD_BUTT
} HI_MAPI_DISP_CMD_E;

typedef enum hiMAPI_DISP_WINDOW_CMD_E {
    HI_MAPI_DISP_WINDOW_CMD_ROTATE,
    HI_MAPI_DISP_WINDOW_CMD_ASPECTRATIO,
    HI_MAPI_DISP_WINDOW_CMD_BUTT
} HI_MAPI_DISP_WINDOW_CMD_E;

/** @} */ /** <!-- ==== DISP End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_DISP_DEFINE_H__ */
