/**
 * @file      hi_product_param_inner.h
 * @brief     parameter module inner struct and interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#ifndef __HI_PRODUCT_PARAM_INNER_H__
#define __HI_PRODUCT_PARAM_INNER_H__

#include "hi_product_param_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

/** redefine product module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "Param"

/** parameter compress or not */
#if (defined(HI3556AV100) || defined(HI3559V200) || defined(HI3556V200))
#define PDT_PARAM_COMPRESS  HI_TRUE
#else
#define PDT_PARAM_COMPRESS  HI_FALSE
#endif

/** parameter physical memory address */
#ifdef CFG_PARAM_PHY_ADDR
#define PDT_PARAM_PHY_ADDR	    CFG_PARAM_PHY_ADDR
#else
#define PDT_PARAM_PHY_ADDR	    (0x80000000)
#endif

/** param magic start macro */
#define PDT_PARAM_MAGIC_START   (0x70617261)
/** param magic end macro */
#define PDT_PARAM_MAGIC_END     (0x7061726d)

/**------------------- MediaMode Configure ------------------------- */

/**
  * MediaMode Parameter: 3 Level
  * Level 1: Media Common Level, share in global
  * Level 2: WorkMode Type Common Level, share in specified workmode type
  * Level 3: MediaMode Specified Level, only used in specified mediamode
  */

/**======== Media Common Level  ======== */

/** MediaCommon Level : Video Capture PipeChn Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_VCAP_PIPE_CHN_S
{
    HI_HANDLE     PipeChnHdl;
    ROTATION_E    enRotate;
    HI_BOOL       bFlip;
    HI_BOOL       bMirror;
    HI_S32        s32Brightness;
    HI_S32        s32Saturation;
    LDC_ATTR_S    stLDCAttr;
    DIS_MODE_E    enDISMotionType;
} PDT_PARAM_MEDIA_COMM_VCAP_PIPE_CHN_S;

/** MediaCommon Level : Video Capture Pipe Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_VCAP_PIPE_S
{
    HI_HANDLE     VcapPipeHdl;
    PDT_PARAM_MEDIA_COMM_VCAP_PIPE_CHN_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_VCAP_PIPE_S;

/** MediaCommon Level : Video Capture Device Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_VCAP_DEV_S
{
    HI_HANDLE     VcapDevHdl;
    PDT_PARAM_MEDIA_COMM_VCAP_PIPE_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_VCAP_DEV_S;

/** MediaCommon Level : Video Capture Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_VCAP_CFG_S
{
    PDT_PARAM_MEDIA_COMM_VCAP_DEV_S  astVcapDevAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_VCAP_CFG_S;

/** MediaCommon Level : VPSS Port Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VPSS_PORT_ATTR_S
{
    HI_HANDLE     VportHdl;
    ROTATION_E    enRotate;
    HI_BOOL       bFlip;
    HI_BOOL       bMirror;
} PDT_PARAM_MEDIA_COMM_VPSS_PORT_ATTR_S;

/** MediaCommon Level : VPSS Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VPSS_ATTR_S
{
    HI_HANDLE     VpssHdl;           /**<video processor vpss grp handle */
    HI_HANDLE     VcapPipeHdl;       /**<binded video capture pipe handle */
    HI_HANDLE     VcapPipeChnHdl;    /**<binded video capture pipe chn handle */
    PDT_PARAM_MEDIA_COMM_VPSS_PORT_ATTR_S astPortAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_VPSS_ATTR_S;

/** MediaCommon Level : Video Processor Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_VPROC_CFG_S
{
    PDT_PARAM_MEDIA_COMM_VPSS_ATTR_S astVpssAttr[HI_PDT_MEDIA_VPSS_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_VPROC_CFG_S;

/** MediaCommon Level : Video Encoder H264 VBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S
{
    HI_U32        u32MaxQp;      /* RW; Range:(MinQp, 51];the max QP value */
    HI_U32        u32MinQp;      /* RW; Range:[0, 51]; the min QP value */
    HI_U32        u32MaxIQp;     /* RW; Range:(MinQp, 51]; max qp for i frame */
    HI_U32        u32MinIQp;     /* RW; Range:[0, 51]; min qp for i frame */
} HI_PDT_MEDIA_COMM_VENC_ATTR_H264_CBR_S;

/** MediaCommon Level : Video Encoder H264 VBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S
{
    HI_U32        u32MaxQp;      /* RW; Range:(MinQp, 51]; the max P B qp */
    HI_U32        u32MinQp;      /* RW; Range:[0, 51]; the min P B qp */
    HI_U32        u32MaxIQp;     /* RW; Range:(MinIQp, 51]; the max I qp */
    HI_U32        u32MinIQp;     /* RW; Range:[0, 51]; the min I qp */
} HI_PDT_MEDIA_COMM_VENC_ATTR_H264_VBR_S;

typedef HI_PDT_MEDIA_COMM_VENC_ATTR_H264_CBR_S  HI_PDT_MEDIA_COMM_VENC_ATTR_H265_CBR_S;
typedef HI_PDT_MEDIA_COMM_VENC_ATTR_H264_VBR_S  HI_PDT_MEDIA_COMM_VENC_ATTR_H265_VBR_S;

/** MediaCommon Level : Video Encoder MJPEG CBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S
{
    HI_U32        u32MaxQfactor; /* the max Qfactor value */
    HI_U32        u32MinQfactor; /* the min Qfactor value */
} HI_PDT_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S;

/** MediaCommon Level : Video Encoder(H264/H265/MJPEG) Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S
{
    HI_MAPI_PAYLOAD_TYPE_E    enType;         /**<encoder type: H264/H265/MJPEG */
    HI_MAPI_VENC_SCENE_MODE_E enSceneMode;    /**<scene mode, eg. DV */
    HI_U32                    u32H264Profile; /**<0:baseline,1:main,2:high */
    HI_U32                    u32H265Profile; /**<0:main */
    HI_MAPI_VENC_RC_MODE_E    enRcMode;
    HI_PDT_MEDIA_COMM_VENC_ATTR_H264_CBR_S  stH264Cbr;
    HI_PDT_MEDIA_COMM_VENC_ATTR_H264_VBR_S  stH264Vbr;
    HI_PDT_MEDIA_COMM_VENC_ATTR_H265_CBR_S  stH265Cbr;
    HI_PDT_MEDIA_COMM_VENC_ATTR_H265_VBR_S  stH265Vbr;
    HI_PDT_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S stMjpegCbr;
} PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S;

/** MediaCommon Level : Snap Encoder(JPEG) Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S
{
    HI_MAPI_PAYLOAD_TYPE_E    enType;         /**<encoder type : JPEG */
    HI_BOOL                   bEnableDCF;
    HI_U32                    u32Qfactor;     /**<jpeg encode quality factor, range[1~99] */
    HI_BOOL                   bMpfEnable;     /**<support mpf or not */
    VENC_JPEG_ENCODE_MODE_E   enEncodeMode;
} PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S;

/** Video Encoder Type */
typedef enum tagPDT_PARAM_VENC_TYPE_E
{
    PDT_PARAM_VENC_TYPE_MAIN_VIDEO = 0, /**<main video stream, eg. main record */
    PDT_PARAM_VENC_TYPE_SUB_VIDEO,      /**<sub video stream, eg. sub record, live stream */
    PDT_PARAM_VENC_TYPE_SINGLE_SNAP,    /**<single snap, eg. single/delay videomode */
    PDT_PARAM_VENC_TYPE_MULTI_SNAP,     /**<multiple snap, eg. lapse/burst/continuous videomode */
    PDT_PARAM_VENC_TYPE_SUB_SNAP,       /**<sub snap, eg. record thumbnail */
    PDT_PARAM_VENC_TYPE_BUTT
} PDT_PARAM_VENC_TYPE_E;

/** sensor time sequence enum */
typedef enum tagPDT_PARAM_SENSOR_TIME_SEQUENCE_E
{
    PDT_SENSOR_TIME_SEQUENCE_4K30 = 0,
    PDT_SENSOR_TIME_SEQUENCE_4K60,
    PDT_SENSOR_TIME_SEQUENCE_1080P90,
    PDT_SENSOR_TIME_SEQUENCE_1080P120,
    PDT_SENSOR_TIME_SEQUENCE_1080P240,
    PDT_SENSOR_TIME_SEQUENCE_720P120,
    PDT_SENSOR_TIME_SEQUENCE_720P240,
    PDT_SENSOR_TIME_SEQUENCE_BUTT
} PDT_PARAM_SENSOR_TIME_SEQUENCE_E;

/** LDCV2 Config */
typedef struct tagPDT_PARAM_LDCV2_CFG_S
{
    PDT_PARAM_SENSOR_TIME_SEQUENCE_E  enSensorTimeSequece;
    VI_LDCV2_ATTR_S                   stLDCV2Attr;
    DIS_CONFIG_S                      stDISConfig;
} PDT_PARAM_LDCV2_CFG_S;

/** Media Common Level Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_CFG_S
{
    HI_BOOL       bLDCEnable; /**<total ldc enable or not */
    HI_BOOL       bDISEnable; /**<total dis enable or not */
    HI_BOOL       bFlip;      /**<total flip/mirror enable or not*/
    HI_BOOL       bAudio;     /**<total audio enable or not */

    PDT_PARAM_MEDIA_COMM_VCAP_CFG_S       stVcapCfg;
    PDT_PARAM_MEDIA_COMM_VPROC_CFG_S      stVprocCfg;

    PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S stMainVideoVencAttr;
    PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S stSubVideoVencAttr;
    PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S  stSingleSnapVencAttr;
    PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S  stMultiSnapVencAttr;
    PDT_PARAM_MEDIA_COMM_SNAP_ENC_ATTR_S  stSubSnapVencAttr;
    VENC_HIERARCHICAL_QP_S                stHierarchicalQp;

    HI_PDT_MEDIA_ACAP_CFG_S               astAcapCfg[HI_PDT_MEDIA_ACAP_MAX_CNT];
    HI_PDT_MEDIA_AENC_CFG_S               astAencCfg[HI_PDT_MEDIA_AENC_MAX_CNT];
    HI_PDT_MEDIA_AO_CFG_S                 astAoCfg[HI_PDT_MEDIA_AO_MAX_CNT];
    HI_PDT_MEDIA_OSD_CFG_S                stOsdCfg;
    HI_PDT_MEDIA_OSD_VIDEO_ATTR_S         stOsdVideoAttr;
    PDT_PARAM_LDCV2_CFG_S                 astLDCV2Cfg[PDT_PARAM_LDCV2_CFG_CNT];
    HI_MAPI_MOTIONSENSOR_INFO_S           stMotionSensorInfo;
} PDT_PARAM_MEDIA_COMM_CFG_S;


/**======== WorkMode Type Common Level  ======== */

/** MediaMode Corresponding WorkModeType Enum */
typedef enum tagPDT_PARAM_WORKMODE_TYPE_E
{
    PDT_PARAM_WORKMODE_TYPE_REC = 0,
    PDT_PARAM_WORKMODE_TYPE_PHOTO,
    PDT_PARAM_WORKMODE_TYPE_MPHOTO,
    PDT_PARAM_WORKMODE_TYPE_RECSNAP,
    PDT_PARAM_WORKMODE_TYPE_PLAYBACK,
    PDT_PARAM_WORKMODE_TYPE_HDMIPREVIEW,
    PDT_PARAM_WORKMODE_TYPE_HDMIPLAYBACK,
    PDT_PARAM_WORKMODE_TYPE_BUTT
} PDT_PARAM_WORKMODE_TYPE_E;

/** WorkModeTypeCommon Level : Video Capture PipeChn Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VCAP_PIPE_CHN_S
{
    HI_BOOL              bEnable;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
} PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_CHN_S;

/** WorkModeTypeCommon Level : Video Capture Pipe Scene Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VCAP_PIPE_SCENE_S
{
    HI_BOOL              bVpssBypass;
    HI_HANDLE            PipeChnHdl;
    HI_HANDLE            MainPipeHdl;
    HI_HANDLE            VpssHdl;
    HI_HANDLE            VPortHdl;
    HI_HANDLE            VencHdl;
} PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_SCENE_S;

/** WorkModeTypeCommon Level : Video Capture Pipe Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VCAP_PIPE_S
{
    HI_BOOL              bEnable;
    HI_MAPI_PIPE_TYPE_E  enPipeType;  /**<pipe type, static attribute */
    HI_BOOL              bIspBypass;
    PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_SCENE_S stSceneCfg;
    PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_CHN_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_S;

/** WorkModeTypeCommon Level : Video Capture Device Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VCAP_DEV_S
{
    HI_BOOL              bEnable;
    PDT_PARAM_WORKMODE_COMM_VCAP_PIPE_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} PDT_PARAM_WORKMODE_COMM_VCAP_DEV_S;

/** WorkModeTypeCommon Level : Video Capture Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VCAP_CFG_S
{
    PDT_PARAM_WORKMODE_COMM_VCAP_DEV_S  astVcapDevAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_PARAM_WORKMODE_COMM_VCAP_CFG_S;

/** WorkModeTypeCommon Level : VPSS Port Attribute */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VPSS_PORT_ATTR_S
{
    HI_BOOL              bEnable;
    VIDEO_FORMAT_E       enVideoFormat;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    ASPECT_RATIO_S       stAspectRatio;
} PDT_PARAM_WORKMODE_COMM_VPSS_PORT_ATTR_S;

/** WorkModeTypeCommon Level : VPSS Attribute */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VPSS_ATTR_S
{
    HI_BOOL              bEnable;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    HI_BOOL              bNrEn;
    VPSS_NR_ATTR_S       stNrAttr;
    PDT_PARAM_WORKMODE_COMM_VPSS_PORT_ATTR_S astPortAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} PDT_PARAM_WORKMODE_COMM_VPSS_ATTR_S;

/** WorkModeTypeCommon Level : Video Processor Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VPROC_CFG_S
{
    PDT_PARAM_WORKMODE_COMM_VPSS_ATTR_S astVpssAttr[HI_PDT_MEDIA_VPSS_MAX_CNT];
} PDT_PARAM_WORKMODE_COMM_VPROC_CFG_S;

/** WorkModeTypeCommon Level : Video Encoder Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_VENC_CFG_S
{
    HI_BOOL       bEnable;      /**<video encoder enable or not */
    HI_HANDLE     VencHdl;      /**<video encoder handle */
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE     ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE     ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
    PDT_PARAM_VENC_TYPE_E enVencType;
} PDT_PARAM_WORKMODE_COMM_VENC_CFG_S;

/** WorkModeTypeCommon Level : AO Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_AO_CFG_S
{
    HI_HANDLE AoHdl;
    HI_HANDLE AoChnHdl;
    AIO_I2STYPE_E enI2sType;
} PDT_PARAM_WORKMODE_COMM_AO_CFG_S;

/** WorkMode Type Common Level Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S
{
    PDT_PARAM_WORKMODE_COMM_VCAP_CFG_S  stVcapCfg;
    PDT_PARAM_WORKMODE_COMM_VPROC_CFG_S stVprocCfg;
    PDT_PARAM_WORKMODE_COMM_VENC_CFG_S  astVencCfg[HI_PDT_MEDIA_VENC_MAX_CNT];
    HI_PDT_MEDIA_DISP_CFG_S             astDispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
    PDT_PARAM_WORKMODE_COMM_AO_CFG_S    astAoCfg[HI_PDT_MEDIA_AO_MAX_CNT];
} PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S;

/** WorkMode Record Type(Norm/Loop/Slow/Lapse) Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_REC_MEDIA_CFG_S
{
    HI_PDT_REC_SRC_S                    stDataSrc;
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S;

/** WorkMode Photo Type(Sing/Delay/Lapse) Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S
{
    HI_PHOTOMNG_PHOTO_SRC_S             stDataSrc;
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S;

/** WorkMode Mutliple Photo(Burst) Type Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_MPHOTO_MEDIA_CFG_S
{
    HI_PHOTOMNG_PHOTO_SRC_S             stDataSrc;
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_MPHOTO_MEDIA_CFG_S;

/** WorkMode Record+Photo(RecSnap) Type Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_RECSNAP_MEDIA_CFG_S
{
    HI_PDT_RECSNAP_SRC_S                stDataSrc;
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_RECSNAP_MEDIA_CFG_S;

/** WorkMode Playback Type Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_PLAYBACK_MEDIA_CFG_S
{
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_PLAYBACK_MEDIA_CFG_S;

/** WorkMode HDMI Display Specified Configure */
typedef struct tagPDT_PARAM_HDMI_DISP_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VdispHdl;
    HI_PDT_MEDIA_DISP_CSC_ATTR_S stCscAttr;
} PDT_PARAM_HDMI_DISP_CFG_S;

/** WorkMode HDMI Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_HDMI_MEDIA_CFG_S
{
    PDT_PARAM_HDMI_DISP_CFG_S        astDispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
    PDT_PARAM_WORKMODE_COMM_AO_CFG_S astAoCfg[HI_PDT_MEDIA_AO_MAX_CNT];
} PDT_PARAM_WORKMODE_HDMI_MEDIA_CFG_S;

/** WorkMode HDMIPreview Type Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_HDMIPREVIEW_MEDIA_CFG_S
{
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_HDMIPREVIEW_MEDIA_CFG_S;

/** WorkMode HDMIPlayback Type Media Configure */
typedef struct tagPDT_PARAM_WORKMODE_HDMIPLAYBACK_MEDIA_CFG_S
{
    PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S stMediaCfg;
} PDT_PARAM_WORKMODE_HDMIPLAYBACK_MEDIA_CFG_S;


/**======== MediaMode Specified Level  ======== */

/** MediaModeSpec Level : Video Capture PipeChn Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VCAP_CHN_PIPE_S
{
    SIZE_S               stDestResolution;
    FRAME_RATE_CTRL_S    stFrameRate;
    HI_BOOL              bSupportLDC; /**<support ldc or not in current mediamode */
    HI_BOOL              bSupportDIS; /**<support dis or not in current mediamode */
    DIS_MOTION_LEVEL_E   enDISAccuracy;
    DIS_ATTR_S          stDisAttr;
    PDT_PARAM_SENSOR_TIME_SEQUENCE_E  enSensorTimeSequece;
} PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_CHN_S;

/** MediaModeSpec Level : Video Capture Pipe Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_S
{
    FRAME_RATE_CTRL_S       stFrameRate; /**<framerate control, dynamic attribute */
    VI_VPSS_MODE_E          enViVpssMode;
    HI_U8                   au8SceneParamIdx[HI_PDT_PARAM_SCENE_TYPE_BUTT];
    HI_U8                   u8Reserved;
    HI_MAPI_PIPE_ISP_ATTR_S stIspPubAttr;
    PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_CHN_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_S;

/** MediaModeSpec Level : Video Capture Device Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VCAP_DEV_S
{
    WDR_MODE_E enWdrMode;
    SIZE_S     stResolution;
    HI_MEDIA_SENSOR_ATTR_S  stSnsAttr;
    PDT_PARAM_MEDIAMODE_SPEC_VCAP_PIPE_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VCAP_DEV_S;

/** MediaModeSpec Level : Video Capture Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VCAP_CFG_S
{
    PDT_PARAM_MEDIAMODE_SPEC_VCAP_DEV_S  astVcapDevAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VCAP_CFG_S;


/** MediaModeSpec Level : VPSS Port Attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VPSS_PORT_ATTR_S
{
    SIZE_S             stResolution;
    FRAME_RATE_CTRL_S  stFrameRate;
} PDT_PARAM_MEDIAMODE_SPEC_VPSS_PORT_ATTR_S;

/** MediaModeSpec Level : VPSS Attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VPSS_ATTR_S
{
    HI_U32             u32MaxW;
    HI_U32             u32MaxH;
    FRAME_RATE_CTRL_S  stFrameRate;
    PDT_PARAM_MEDIAMODE_SPEC_VPSS_PORT_ATTR_S astPortAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VPSS_ATTR_S;

/** MediaModeSpec Level : Video Processor Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S
{
    PDT_PARAM_MEDIAMODE_SPEC_VPSS_ATTR_S astVpssAttr[HI_PDT_MEDIA_VPSS_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S;

/** MediaModeSpec Level : Video Encoder Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S
{
    SIZE_S      stResolution;
    HI_U32      u32BufSize;
    HI_U32      u32Gop;              /**<I frame interval [1, 65536]*/
    HI_U32      u32SrcFrameRate;     /**<frame rate [1/64, 240]*/
    HI_FR32     fr32DstFrameRate;    /**<frame rate [1/64, 240]*/
    HI_U32      u32H264Bitrate_Kbps; /**<cbr:average bitrate, vbr:maximum bitrate */
    HI_U32      u32H265Bitrate_Kbps; /**<cbr:average bitrate, vbr:maximum bitrate */
    HI_U32      u32StatTime;         /**<the rate statistic time [1, 60], the unit is sencond(s) */
    SIZE_S      stMpfSize;
} PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S;


/** MediaMode Special Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_CFG_S
{
    HI_PDT_MEDIAMODE_E                   enMediaMode;
    PDT_PARAM_WORKMODE_TYPE_E            enWorkModeType;
    HI_PDT_MEDIA_VB_CFG_S                stVBCfg;
    PDT_PARAM_MEDIAMODE_SPEC_VCAP_CFG_S  stVcapCfg;
    PDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S stVprocCfg;
    PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S  astVencCfg[HI_PDT_MEDIA_VENC_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_CFG_S;

/**------------------- WorkMode Configure  ------------------------- */

/** WorkMode Common Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_CFG_S
{
    HI_PDT_WORKMODE_E           enPoweronWorkMode;
    HI_PDT_POWERON_ACTION_E     enPoweronAction;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stVideoProTune;  /**<NormRec/LoopRec/SlowRec/LpseRec/RecSnap/Burst/Continuous */
    HI_PDT_PHOTO_PROTUNE_ATTR_S stPhotoProTune;  /**<Single/Delay/LapsePhoto */
    HI_RECMNG_MUXER_TYPE_E      enRecMuxerType;
    HI_RECMNG_THM_TYPE_E        enRecThmType;
    HI_U32                      u32RepairUnit;   /**<repair data unit in bytes, [10M,500M]
                                                     0: not support repair */
    HI_U32                      u32PreAllocUnit; /**<pre allocate size in bytes, [0,100M]
                                                     0 for not use pre allocate function*/
    HI_USB_MODE_E               enUsbMode;
} PDT_PARAM_WORKMODE_COMM_CFG_S;

/** Record Common Param */
typedef struct tagPDT_PARAM_REC_COMM_ATTR_S
{
    HI_PDT_MEDIAMODE_E  enMediaMode;
    HI_REC_SPLIT_ATTR_S stSplitAttr;       /**<split attribute, include split type and parameter */
    HI_U32              u32BufferTimeMSec; /**<record buffer size in millisecond */
    HI_U32              u32PreRecTimeSec;  /**<pre-record time in second */
    HI_U32              au32VBufSize[HI_PDT_REC_VSTREAM_MAX_CNT]; /**<set the vbuf size for fwrite (0,5M] unit :byte*/
    HI_U8               u8FileTypeIdx;
    HI_U8               au8Reserved[3];
} PDT_PARAM_REC_COMM_ATTR_S;

/** Normal Record Specified Attribute */
typedef struct tagPDT_PARAM_NORM_REC_ATTR_S
{
    PDT_PARAM_REC_COMM_ATTR_S stCommAttr;
} PDT_PARAM_NORM_REC_ATTR_S;

/** Loop Record Specified Attribute */
typedef struct tagPDT_PARAM_LOOP_REC_ATTR_S
{
    PDT_PARAM_REC_COMM_ATTR_S stCommAttr;
    HI_U32                    u32LoopTime_min;
} PDT_PARAM_LOOP_REC_ATTR_S;

/** Slow Record Specified Attribute */
typedef struct tagPDT_PARAM_SLOW_REC_ATTR_S
{
    PDT_PARAM_REC_COMM_ATTR_S stCommAttr;
    HI_U32                    au32PlayFrmRate[HI_PDT_REC_VSTREAM_MAX_CNT];
} PDT_PARAM_SLOW_REC_ATTR_S;

/** Lapse Record Specified Attribute */
typedef struct tagPDT_PARAM_LAPSE_REC_ATTR_S
{
    PDT_PARAM_REC_COMM_ATTR_S stCommAttr;
    HI_U32                    u32Interval_ms;
} PDT_PARAM_LAPSE_REC_ATTR_S;

/** Record+Photo Specified Configure */
typedef struct tagPDT_PARAM_RECSNAP_ATTR_S
{
    PDT_PARAM_REC_COMM_ATTR_S stCommAttr;
    HI_U32                    u32SnapFileTypeIndex;
    HI_U32                    u32SnapInterval_ms;
} PDT_PARAM_RECSNAP_ATTR_S;

/** Photo(Single/Delay) Common Process Alg Param */
typedef struct tagPDT_PARAM_PHOTO_PROCALG_S
{
    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutputFmt;
    HI_PDT_PHOTO_SCENE_E        enScene;
    HI_PDT_PHOTO_PROCALG_S      stProcAlg;
} PDT_PARAM_PHOTO_PROCALG_S;

/** Single Photo Specified Attribute */
typedef struct tagPDT_PARAM_SINGLE_PHOTO_ATTR_S
{
    HI_PDT_PHOTO_COMM_ATTR_S stCommAttr;
} PDT_PARAM_SINGLE_PHOTO_ATTR_S;

/** Delay Photo Specified Attribute */
typedef struct tagPDT_PARAM_DELAY_PHOTO_ATTR_S
{
    HI_PDT_PHOTO_COMM_ATTR_S stCommAttr;
    HI_U32                   u32DelayTime_s;
} PDT_PARAM_DELAY_PHOTO_ATTR_S;

/** Lapse Photo Specified Attribute */
typedef struct tagPDT_PARAM_LAPSE_PHOTO_ATTR_S
{
    HI_PDT_PHOTO_COMM_ATTR_S stCommAttr;
    HI_U32                   u32Interval_ms;
    HI_U32                   u32ExpTime_us; /**<lapse photo exposure time, 0:auto; >0 manual, eg. 500000us */
} PDT_PARAM_LAPSE_PHOTO_ATTR_S;

/** Burst Photo Specified Attribute */
typedef struct tagPDT_PARAM_BURST_ATTR_S
{
    HI_PDT_PHOTO_COMM_ATTR_S stCommAttr;
    HI_PDT_JPG_BURST_TYPE_E  aenJpgBurstType[PDT_PARAM_MPHOTO_MEDIAMODE_CNT];
} PDT_PARAM_BURST_ATTR_S;

/** WorkMode Configure */
typedef struct tagPDT_PARAM_WORKMODE_CFG_S
{
    PDT_PARAM_WORKMODE_COMM_CFG_S               stCommCfg;
    PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S          stRecMediaCfg;
    PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S        stPhotoMediaCfg;
    PDT_PARAM_WORKMODE_MPHOTO_MEDIA_CFG_S       stMPhotoMediaCfg;
    PDT_PARAM_WORKMODE_RECSNAP_MEDIA_CFG_S      stRecSnapMediaCfg;
    PDT_PARAM_WORKMODE_PLAYBACK_MEDIA_CFG_S     stPlayBackMediaCfg;
    PDT_PARAM_WORKMODE_HDMIPREVIEW_MEDIA_CFG_S  stHDMIPreviewMediaCfg;
    PDT_PARAM_WORKMODE_HDMIPLAYBACK_MEDIA_CFG_S stHDMIPlaybackMediaCfg;
    PDT_PARAM_NORM_REC_ATTR_S                   stNormRecAttr;
    PDT_PARAM_LOOP_REC_ATTR_S                   stLoopRecAttr;
    PDT_PARAM_SLOW_REC_ATTR_S                   stSlowRecAttr;
    PDT_PARAM_LAPSE_REC_ATTR_S                  stLpseRecAttr;
    PDT_PARAM_RECSNAP_ATTR_S                    stRecSnapAttr;
    PDT_PARAM_PHOTO_PROCALG_S                   stPhotoProcAlg;
    PDT_PARAM_SINGLE_PHOTO_ATTR_S               stSingPhotoAttr;
    PDT_PARAM_DELAY_PHOTO_ATTR_S                stDelayPhotoAttr;
    PDT_PARAM_LAPSE_PHOTO_ATTR_S                stLpsePhotoAttr;
    PDT_PARAM_BURST_ATTR_S                      stBurstAttr;
    HI_PDT_PARAM_UVC_ATTR_S                     stUvcAttr;
    HI_USB_STORAGE_CFG_S                        stUsbStorageCfg;
    HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S            stHDMIPreviewAttr;
    HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S           stHDMIPlaybackAttr;
    HI_PDT_PARAM_PLAYBACK_ATTR_S                stPlaybackAttr;
} PDT_PARAM_WORKMODE_CFG_S;

/**------------------- DevMng  ------------------------- */

/** Device Configure */
typedef struct tagPDT_PARAM_DEVMNG_CFG_S
{
    HI_UPGRADE_DEV_INFO_S    stDevInfo;
    HI_STORAGEMNG_CFG_S      stStorageCfg;
    HI_KEYMNG_CFG_S          stkeyMngCfg;
    HI_GAUGEMNG_CFG_S        stGaugeMngCfg;
    HI_HAL_WIFI_APMODE_CFG_S stWiFiApCfg;
    HI_TIMEDTASK_ATTR_S      stSystemDormantAttr;
    HI_TIMEDTASK_ATTR_S      stScreenDormantAttr;
    HI_S32                   s32ScreenBrightness;
    HI_S32                   s32SystemLanguage;
    HI_S32                   s32SysVolume;
    HI_BOOL                  bBootSoundEnable;
    HI_BOOL                  bKeyToneEnable;
} PDT_PARAM_DEVMNG_CFG_S;


/**-------------------  ParamItem ValueSet  ------------------------- */

/** Normal Record Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_NORM_REC_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
} PDT_PARAM_NORM_REC_VALUESET_S;

/** Loop Record Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_LOOP_REC_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S stLoopTimeValues;
} PDT_PARAM_LOOP_REC_VALUESET_S;

/** Slow Record Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_SLOW_REC_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
} PDT_PARAM_SLOW_REC_VALUESET_S;

/** Lapse Record Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_LAPSE_REC_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S stLapseIntervalValues;
} PDT_PARAM_LAPSE_REC_VALUESET_S;

/** Record+Snap Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_RECSNAP_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S stLapseIntervalValues;
} PDT_PARAM_RECSNAP_VALUESET_S;

/** Single+Delay Photo Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_SING_PHOTO_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S stSceneValues;
    HI_PDT_ITEM_VALUESET_S stDelayTimeValues;
    HI_PDT_ITEM_VALUESET_S stOutputFormatValues;
} PDT_PARAM_SING_PHOTO_VALUESET_S;

/** Lapse Photo Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_LAPSE_PHOTO_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S stLapseIntervalValues;
    HI_PDT_ITEM_VALUESET_S stExpTimeValues;
} PDT_PARAM_LAPSE_PHOTO_VALUESET_S;

/** Burst Photo Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_BURST_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stMediaModeValues;
    HI_PDT_ITEM_VALUESET_S astBurstTypeValues[PDT_PARAM_MPHOTO_MEDIAMODE_CNT];
} PDT_PARAM_BURST_VALUESET_S;

/** Video/Multiple Photo Protune Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_VIDEO_PROTUNE_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stWBValues;
    HI_PDT_ITEM_VALUESET_S stISOValues;
    HI_PDT_ITEM_VALUESET_S stMetryTypeValues;
    HI_PDT_ITEM_VALUESET_S stEVValues;
} PDT_PARAM_VIDEO_PROTUNE_VALUESET_S;

/** Photo Protune Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_PHOTO_PROTUNE_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stWBValues;
    HI_PDT_ITEM_VALUESET_S stISOValues;
    HI_PDT_ITEM_VALUESET_S stMetryTypeValues;
    HI_PDT_ITEM_VALUESET_S stEVValues;
    HI_PDT_ITEM_VALUESET_S stExpTimeValues;
} PDT_PARAM_PHOTO_PROTUNE_VALUESET_S;

/** DeviceManager Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_DEVMNG_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S stSysScreemDormantValues; /**<screen/system dormant values */
    HI_PDT_ITEM_VALUESET_S stSystemLanguage;
} PDT_PARAM_DEVMNG_VALUESET_S;

/** Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_CONFITEM_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S             stPowerOnActionValues;
    HI_PDT_ITEM_VALUESET_S             stVideoPayloadType;
    PDT_PARAM_NORM_REC_VALUESET_S      stNormRec;
    PDT_PARAM_LOOP_REC_VALUESET_S      stLoopRec;
    PDT_PARAM_SLOW_REC_VALUESET_S      stSlowRec;
    PDT_PARAM_LAPSE_REC_VALUESET_S     stLpseRec;
    PDT_PARAM_RECSNAP_VALUESET_S       stRecSnap;
    PDT_PARAM_SING_PHOTO_VALUESET_S    stSingPhoto;
    PDT_PARAM_LAPSE_PHOTO_VALUESET_S   stLpsePhoto;
    PDT_PARAM_BURST_VALUESET_S         stBurst;
    PDT_PARAM_VIDEO_PROTUNE_VALUESET_S stVideoProtune;
    PDT_PARAM_PHOTO_PROTUNE_VALUESET_S stPhotoProtune;
    PDT_PARAM_DEVMNG_VALUESET_S        stDevMng;
    HI_PDT_ITEM_VALUESET_S             stUsbMode;
} PDT_PARAM_CONFITEM_VALUESET_S;


/**------------------- Param Struct  ------------------------- */

/** product param head information */
typedef struct tagPDT_PARAM_HEAD_S
{
    HI_U32 u32MagicStart;
    HI_U32 u32SysLen;
} PDT_PARAM_HEAD_S;

/** Param Configure */
typedef struct tagPARAM_CFG_S
{
    PDT_PARAM_HEAD_S               stHead;         /**<param head */
    PDT_PARAM_MEDIA_COMM_CFG_S     stMediaCommCfg; /**<media common configure */
    PDT_PARAM_WORKMODE_CFG_S       stWorkModeCfg;  /**<workmode configure */
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S astMediaModeSpecCfg[PDT_PARAM_MEDIAMODE_CNT]; /**<mediamode specified configure */
    HI_PDT_FILEMNG_CFG_S           stFileMngCfg;
    PDT_PARAM_DEVMNG_CFG_S         stDevMngCfg;
    PDT_PARAM_CONFITEM_VALUESET_S  stItemValues;   /**<configure item valueset */
    HI_U32                         u32MagicEnd;    /**<param end */
} PDT_PARAM_CFG_S;

/**------------------- Param Context  ------------------------- */

/** Param Context */
typedef struct tagPARAM_CONTEXT_S
{
    HI_BOOL         bInit;     /**<module init status */
    pthread_mutex_t mutexLock; /**<param lock, protect pstCfg */
    PDT_PARAM_CFG_S *pstCfg;   /**<param address */
} PDT_PARAM_CONTEXT_S;


/**
 * @brief set save flag
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/22
 */
HI_VOID PDT_PARAM_SetSaveFlg(HI_VOID);

/**
 * @brief     get param context
 * @return    param context pointer : success, NULL : failure.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
PDT_PARAM_CONTEXT_S* PDT_PARAM_GetCtx(HI_VOID);


/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_INNER_H__ */

