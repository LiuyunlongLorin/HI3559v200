/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_comm_define.h
 * @brief   comm module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_COMM_DEFINE_H__
#define __HI_MAPI_COMM_DEFINE_H__

#include "hi_type.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_video.h"
#include "hi_comm_region.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/** \addtogroup     COMM */
/** @{ */ /** <!-- [COMM] */

#define MUTEX_INIT_LOCK(mutex)                  \
    do {                                        \
        (void)pthread_mutex_init(&mutex, NULL); \
    } while (0)

#define MUTEX_LOCK(mutex)                 \
    do {                                  \
        (void)pthread_mutex_lock(&mutex); \
    } while (0)

#define MUTEX_UNLOCK(mutex)                 \
    do {                                    \
        (void)pthread_mutex_unlock(&mutex); \
    } while (0)

#define MUTEX_DESTROY(mutex)                 \
    do {                                     \
        (void)pthread_mutex_destroy(&mutex); \
    } while (0)

#define MAPI_UNUSED(x) (void)(x)

/* VCAP attribute range */

#if defined (__HI3556AV100__) || defined (__HI3519AV100__)
#define HI_MAPI_VCAP_MAX_DEV_NUM          (5)
#define HI_MAPI_VCAP_MAX_PIPE_NUM         (6)
#define HI_MAPI_PIPE_MAX_CHN_NUM          (2)
#define HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM (6)

/* VPROC attribute range */
#define HI_MAPI_VPSS_MAX_NUM  (32)
#define HI_MAPI_VPORT_MAX_NUM (4)
#endif

#ifdef __HI3559AV100__
#define HI_MAPI_VCAP_MAX_DEV_NUM          (8)
#define HI_MAPI_VCAP_MAX_PIPE_NUM         (8)
#define HI_MAPI_PIPE_MAX_CHN_NUM          (1)
#define HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM (8)

/* VPROC attribute range */
#define HI_MAPI_VPSS_MAX_NUM  (32)
#define HI_MAPI_VPORT_MAX_NUM (4)
#endif

#ifdef __HI3559V200__
#define HI_MAPI_VCAP_MAX_DEV_NUM          (2)
#define HI_MAPI_VCAP_MAX_PIPE_NUM         (4)
#define HI_MAPI_PIPE_MAX_CHN_NUM          (1)
#define HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM (1) /* not valid */

/* VPROC attribute range */
#define HI_MAPI_VPSS_MAX_NUM  (16)
#define HI_MAPI_VPORT_MAX_NUM (3)
#endif

#ifdef __HI3518EV300__
#define HI_MAPI_VCAP_MAX_DEV_NUM          (1)
#define HI_MAPI_VCAP_MAX_PIPE_NUM         (2)
#define HI_MAPI_PIPE_MAX_CHN_NUM          (1)
#define HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM (1) /* not valid */

/* VPROC attribute range */
#define HI_MAPI_VPSS_MAX_NUM  (8)
#define HI_MAPI_VPORT_MAX_NUM (3)
#endif

/* the maximum number of professional snap */
#define HI_MAPI_PRO_MAX_FRAME_NUM (8)
/* the maximum number of normal snap */
#define HI_MAPI_NORMAL_MAX_FRAME_NUM (0xFFFFFFFF)
#define HI_MAPI_VCAP_OSD_MAX_NUM     (4)

/* VPROC attribute range */
#define HI_MAPI_STITCH_MAX_NUM      (32)
#define HI_MAPI_STITCH_PORT_MAX_NUM (2)
#define HI_MAPI_STITCH_PIPE_MAX_NUM (4)
#define HI_MAPI_VPROC_OSD_MAX_NUM   (4)

/* VENC attribute range */
#define HI_MAPI_VENC_MAX_CHN_NUM      (8)
#define HI_MAPI_VENC_CALLBACK_MAX_NUM (5)
#define HI_MAPI_VENC_OSD_MAX_NUM      (8)

/* DISP attribute range */
#define HI_MAPI_DISP_OSD_MAX_NUM (1)
/* HDMI attribute range */
#define HI_MAPI_HDMI_MAX_NUM (1)

/* OSD attribute range */
#define HI_MAPI_MIN_OSD_BMP_W (2)
#define HI_MAPI_MAX_OSD_BMP_W (4094)
#define HI_MAPI_MIN_OSD_BMP_H (2)
#define HI_MAPI_MAX_OSD_BMP_H (4094)

#define HI_MAPI_OSD_BMP_W_ALIGN (2)
#define HI_MAPI_OSD_BMP_H_ALIGN (2)

#define HI_MAPI_MIN_OSD_ALPHA (0)
#define HI_MAPI_MAX_OSD_ALPHA (255)

#define HI_MAPI_MIN_OSD_REGION_X (0)
#define HI_MAPI_MAX_OSD_REGION_X (8190)
#define HI_MAPI_MIN_OSD_REGION_Y (0)
#define HI_MAPI_MAX_OSD_REGION_Y (8190)

#define HI_MAPI_OSD_REGION_X_ALIGN (2)
#define HI_MAPI_OSD_REGION_Y_ALIGN (2)

#define HI_MAPI_RGN_BUF_MIN_NUM (1)
#define HI_MAPI_RGN_BUF_MAX_NUM (6)
/* audio attribute range */
#define HI_MAPI_AIO_MAX_POINT_PER_FRAME (2048)

#define HI_MAPI_ACAP_CHN_MAX_NUM (2)

#define HI_MAPI_AENC_CHN_MAX_NUM          (2)
#define HI_MAPI_AENC_CHN_CALLBACK_MAX_NUM (5)

#define HI_MAPI_AO_CHN_MAX_NUM                (3)
#define HI_MAPI_AO_MIN_GAIN                   (-121)
#define HI_MAPI_AO_MAX_GAIN                   (6)
#define HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM (3)

#define HI_MAPI_MODE_NAME_LEN (8)

#define HI_IPCMSG_PORT_HIMPP (1)

#define HI_INVALID_HANDLE (-1)
#define HI_INVALID_VALUE  (-1)

/* pthread  name lenght */

#define HI_MAPI_PTHREAD_NAME_LEN (16)

typedef enum hiMAPI_MOD_ID_E {
    HI_MAPI_MOD_SYS = 0,
    HI_MAPI_MOD_VCAP,
    HI_MAPI_MOD_VPROC,
    HI_MAPI_MOD_VENC,
    HI_MAPI_MOD_ACAP,
    HI_MAPI_MOD_AENC,
    HI_MAPI_MOD_AO,
    HI_MAPI_MOD_DISP,
    HI_MAPI_MOD_HDMI,
    HI_MAPI_MOD_VDEC,
    HI_MAPI_MOD_PM,
    HI_MAPI_MOD_BUTT,
} HI_MAPI_MOD_ID_E;

typedef enum hiMAPI_PAYLOAD_TYPE_E {
    HI_MAPI_PAYLOAD_TYPE_H264,
    HI_MAPI_PAYLOAD_TYPE_H265,
    HI_MAPI_PAYLOAD_TYPE_MJPEG,
    HI_MAPI_PAYLOAD_TYPE_JPEG,
    HI_MAPI_PAYLOAD_TYPE_AAC,
    HI_MAPI_PAYLOAD_TYPE_BUTT
} HI_MAPI_PAYLOAD_TYPE_E;

typedef enum hiMAPI_FRAME_DATA_TYPE_E {
    HI_FRAME_DATA_TYPE_RAW,
    HI_FRAME_DATA_TYPE_YUV,
    HI_FRAME_DATA_TYPE_BUTT
} HI_MAPI_FRAME_DATA_TYPE_E;

/* Frame supplemental information */
typedef struct hiMAPI_FRAME_DATA_S {
    HI_MAPI_FRAME_DATA_TYPE_E enFrameDataType;
    HI_U32 u32Width;
    HI_U32 u32Height;
    PIXEL_FORMAT_E enPixelFormat;
    COMPRESS_MODE_E enCompressMode;
    HI_U64 u64PhyAddr[3];
    HI_U64 u64VirAddr[3];
    HI_U32 u32Stride[3];
    HI_U64 u64pts;
    HI_U32 u32PoolId;
    VIDEO_SUPPLEMENT_S stVideoSupplement;
} HI_MAPI_FRAME_DATA_S;

typedef enum hiMAPI_AUDIO_SOUND_MODE_E {
    HI_MAPI_AUDIO_SOUND_MODE_LEFT = 0,
    HI_MAPI_AUDIO_SOUND_MODE_RIGHT = 1,
    HI_MAPI_AUDIO_SOUND_MODE_STEREO = 2,
    HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT = 3,
    HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT = 4,
    HI_MAPI_AUDIO_SOUND_MODE_BUTT
} HI_MAPI_AUDIO_SOUND_MODE_E;

typedef enum hiMAPI_AUDIO_FORMAT_E {
    HI_MAPI_AUDIO_FORMAT_AACLC = 0, /* AAC-LC format  */
    HI_MAPI_AUDIO_FORMAT_G711A,     /* G711A format  */
    HI_MAPI_AUDIO_FORMAT_G711U,     /* G711U format  */
    HI_MAPI_AUDIO_FORMAT_OPUS,      /* NOT SUPPORT NOW ! */
    HI_MAPI_AUDIO_FORMAT_BUTT
} HI_MAPI_AUDIO_FORMAT_E;

/** osd display attribute.dynamic attribute */
typedef struct hiHI_MAPI_OSD_DISPATTR_S {
    HI_BOOL bShow;
    HI_U32 u32Color;
    HI_U32 u32FgAlpha;
    HI_U32 u32BgAlpha;
    HI_S32 s32RegionX;
    HI_S32 s32RegionY;
    ATTACH_DEST_E enAttachDest;  // venc
} HI_MAPI_OSD_DISPATTR_S;

/** osd attribute */
typedef struct hiHI_MAPI_OSD_ATTR_S {
    HI_U32 u32CanvasNum;                     /**buff num in rgn area, default to 2. when venc proc B-frame need to plus 2**/
    BITMAP_S stBitmapAttr;                   /**<static attribute */
    HI_MAPI_OSD_DISPATTR_S stOsdDisplayAttr; /**<dynamic attribute */
} HI_MAPI_OSD_ATTR_S;

/** media config param */
typedef struct hiHI_MAPI_MEDIA_CONFIG_S {
    VI_VPSS_MODE_S stVIVPSSMode;
    VB_CONFIG_S stVbConfig;
} HI_MAPI_MEDIA_CONFIG_S;

/** the param of the venc mod */
typedef struct hiMAPI_VENC_PARAM_MOD_S {
    HI_U32 u32H264eLowPowerMode; /**< H264e low power consumption mode Range:[0,1]; */
    HI_U32 u32H265eLowPowerMode; /**< H265e low power consumption mode Range:[0,2]; */
} HI_MAPI_VENC_PARAM_MOD_S;

/** media init attribute */
typedef struct hiHI_MAPI_MEDIA_ATTR_S {
    HI_MAPI_MEDIA_CONFIG_S stMediaConfig;
    HI_MAPI_VENC_PARAM_MOD_S stVencModPara;
} HI_MAPI_MEDIA_ATTR_S;

/** datafifo node num config */
typedef struct hiHI_MAPI_DATAFIFO_CONFIG_S {
    HI_U32 u32VencDatafifeNodeNum;
} HI_MAPI_DATAFIFO_CONFIG_S;

/** @} */ /** <!-- ==== COMM End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_COMM_DEFINE_H__ */

