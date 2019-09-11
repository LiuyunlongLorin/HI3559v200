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
#include "hi_timedtask.h"


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
#define PDT_PARAM_COMPRESS  HI_TRUE

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

/**------------------- MediaMode type ------------------------- */

/** MediaMode type */
typedef enum tagPDT_PARAM_MEDIAMODE_TYPE_E
{
    PDT_PARAM_MEDIAMODE_TYPE_NORM_REC = 0,
    PDT_PARAM_MEDIAMODE_TYPE_PHOTO,
    PDT_PARAM_MEDIAMODE_TYPE_BUTT
} PDT_PARAM_MEDIAMODE_TYPE_E;

/**------------------- MediaMode Configure ------------------------- */

/**
  * MediaMode Parameter: 3 Level
  * Level 1: Media Common Level, share in global
  * Level 2: WorkMode Type Common Level, share in specified workmode type
  * Level 3: MediaMode Specified Level, only used in specified mediamode
  */

/**======== Media Common Level  ======== */

/** MediaCommon Level : Video Capture PipeChn Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_CHN_S
{
    HI_BOOL       bEnable;
    HI_HANDLE     PipeChnHdl;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    ROTATION_E    enRotate;
    HI_BOOL       bFlip;
    HI_BOOL       bMirror;
    HI_S32        s32Brightness;
    HI_S32        s32Saturation;
    VI_LDC_ATTR_S    stLDCAttr;
} PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_CHN_S;

/** MediaCommon Level : Video Capture Pipe Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_S
{
    HI_BOOL       bEnable;
    HI_HANDLE     VcapPipeHdl;
    HI_MAPI_PIPE_TYPE_E enPipeType;
    PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_CHN_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_S;

/** MediaCommon Level : Video Capture Device Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VCAP_DEV_S
{
    HI_BOOL       bEnable;
    HI_HANDLE     VcapDevHdl;
    WDR_MODE_E  enWdrMode;
    HI_U32       u32VideoMode;
    PDT_PARAM_MEDIAMODE_COMM_VCAP_PIPE_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} PDT_PARAM_MEDIAMODE_COMM_VCAP_DEV_S;


/** MediaCommon Level : VPSS Port Attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VPSS_PORT_ATTR_S
{
    HI_BOOL       bEnable;
    HI_HANDLE     VportHdl;
    ROTATION_E    enRotate;
    HI_BOOL       bFlip;
    HI_BOOL       bMirror;
    HI_BOOL       bSupportBufferShare;
    VIDEO_FORMAT_E enVideoFormat;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
} PDT_PARAM_MEDIAMODE_COMM_VPSS_PORT_ATTR_S;

/** MediaCommon Level : VPSS Attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VPSS_ATTR_S
{
    HI_BOOL       bEnable;
    HI_HANDLE     VpssHdl;           /**<video processor vpss grp handle */
    HI_HANDLE     VcapPipeHdl;       /**<binded video capture pipe handle */
    HI_HANDLE     PipeChnHdl;    /**<binded video capture pipe chn handle */
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    HI_BOOL bNrEn;
    VPSS_NR_ATTR_S  stNrAttr;
    PDT_PARAM_MEDIAMODE_COMM_VPSS_PORT_ATTR_S astPortAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} PDT_PARAM_MEDIAMODE_COMM_VPSS_ATTR_S;

/** MediaCommon Level : Video Processor Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VPROC_CFG_S
{
    HI_U32 u32VpssCnt;
    PDT_PARAM_MEDIAMODE_COMM_VPSS_ATTR_S astVpssAttr[HI_PDT_VCAPDEV_VPSS_MAX_CNT];
} PDT_PARAM_MEDIAMODE_COMM_VPROC_CFG_S;

/** MediaCommon Level : venc Attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S
{
    HI_BOOL       bEnable;      /**<video encoder enable or not */
    HI_HANDLE     VencHdl;      /**<video encoder handle */
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE     ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE     ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
    HI_MAPI_PAYLOAD_TYPE_E  enPayload;  /**<the type of payload*/
    HI_MAPI_VENC_RC_MODE_E enRcMode; /**<rate control mode*/
    HI_PDT_MEDIA_VENC_TYPE_E    enVencType; /** 0:main video, 1: sub video, 2: single snap, 3: sub snap*/
} PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S;

/** MediaCommon Level : venc Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S
{
    HI_U32 u32VencCnt;
    PDT_PARAM_MEDIAMODE_COMM_VENC_ATTR_S astVencAttr[HI_PDT_VCAPDEV_VENC_MAX_CNT];
} PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S;

typedef struct tagPDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S
{
    PDT_PARAM_MEDIAMODE_COMM_VCAP_DEV_S     stVcapDev;
    PDT_PARAM_MEDIAMODE_COMM_VPROC_CFG_S    stVprocCfg;
    PDT_PARAM_MEDIAMODE_COMM_VENC_CFG_S     stVencCfg;
    HI_PDT_MEDIA_DISP_WND_CROP_CFG_S        stCropCfg;
#ifdef CONFIG_MOTIONDETECT_ON
    HI_PDT_PARAM_VIDEODETECT_CFG_S          stVideoDetectCfg;
#endif

} PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S;


/** MediaCommon Level : Video Encoder H264 VBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S
{
    HI_U32        u32MaxQp;      /* RW; Range:(MinQp, 51];the max QP value */
    HI_U32        u32MinQp;      /* RW; Range:[0, 51]; the min QP value */
    HI_U32        u32MaxIQp;     /* RW; Range:(MinQp, 51]; max qp for i frame */
    HI_U32        u32MinIQp;     /* RW; Range:[0, 51]; min qp for i frame */
} PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S;

/** MediaCommon Level : Video Encoder H264 VBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S
{
    HI_U32        u32MaxQp;      /* RW; Range:(MinQp, 51]; the max P B qp */
    HI_U32        u32MinQp;      /* RW; Range:[0, 51]; the min P B qp */
    HI_U32        u32MaxIQp;     /* RW; Range:(MinIQp, 51]; the max I qp */
    HI_U32        u32MinIQp;     /* RW; Range:[0, 51]; the min I qp */
} PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S;

/** MediaCommon Level : Video Encoder H264 QVBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_QVBR_S
{
    HI_U32        u32MaxQp;      /* RW; Range:(MinQp, 51]; the max P B qp */
    HI_U32        u32MinQp;      /* RW; Range:[0, 51]; the min P B qp */
    HI_U32        u32MaxIQp;     /* RW; Range:(MinIQp, 51]; the max I qp */
    HI_U32        u32MinIQp;     /* RW; Range:[0, 51]; the min I qp */
    HI_S32 s32BitPercentUL;      /* RW; Range:[30, 180]; Indicate the ratio of bitrate  upper limit*/
    HI_S32 s32BitPercentLL;      /* RW; Range:[30, 180]; Indicate the ratio of bitrate lower limit,
                                        can not be larger than s32BitPercentUL*/
    HI_S32 s32PsnrFluctuateUL;   /* RW; Range:[18, 40];
                                        Reduce the target bitrate when the value of psnr approch the upper limit*/
    HI_S32 s32PsnrFluctuateLL;   /* RW; Range:[18, 40];
                                        Increase the target bitrate when the value of psnr approch the lower limit,
                                        can not be larger than s32PsnrFluctuateUL*/
} PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_QVBR_S;

typedef  PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S   PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_CBR_S;
typedef  PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S   PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_VBR_S;
typedef  PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_QVBR_S  PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_QVBR_S;

/** MediaCommon Level : Video Encoder MJPEG CBR Common Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S
{
    HI_U32        u32MaxQfactor; /* the max Qfactor value */
    HI_U32        u32MinQfactor; /* the min Qfactor value */
} PDT_PARAM_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S;

/** MediaCommon Level : Video Encoder(H264/H265/MJPEG) Attribute */
typedef struct tagPDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S
{
    HI_MAPI_VENC_SCENE_MODE_E enSceneMode;    /**<scene mode, eg. DV */
    HI_U32                    u32H264Profile; /**<0:baseline,1:main,2:high */
    HI_U32                    u32H265Profile; /**<0:main */
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S  stH264Cbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S  stH264Vbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_QVBR_S stH264QVbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_CBR_S  stH265Cbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_VBR_S  stH265Vbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_QVBR_S stH265QVbr;
    PDT_PARAM_MEDIA_COMM_VENC_ATTR_MJPEG_CBR_S stMjpegCbr;
} PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S;


/** video display window configure */
typedef struct tagPDT_PARAM_DISP_WND_CFG_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  WndHdl;
    HI_S32     s32PreViewCamID;     /**< preview camera id*/
    ROTATION_E enRotate;
    ASPECT_RATIO_S stAspectRatio;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
} PDT_PARAM_DISP_WND_CFG_S;

typedef struct tagPDT_PARAM_MEDIA_COMM_DISP_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VdispHdl;
    HI_U32    u32BgColor;          /* Background color of a device, in RGB format. */
    HI_U32    u32WindowCnt;              /* Maximum display window number*/
    HI_PDT_MEDIA_DISP_CSC_ATTR_S stCscAttr;
    HI_PDT_MEDIA_BIND_SRC_CFG_S astDispSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    PDT_PARAM_DISP_WND_CFG_S  astWndCfg[HI_PDT_MEDIA_DISP_WND_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_DISP_CFG_S;


/** Media Common Level Configure */
typedef struct tagPDT_PARAM_MEDIA_COMM_CFG_S
{
    PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S stMainVideoVencAttr;
    PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S stSubVideoVencAttr;
    HI_MAPI_VENC_ATTR_JPEG_S  stSingleSnapVencAttr;
    HI_MAPI_VENC_ATTR_JPEG_S  stSubSnapVencAttr;
    VENC_HIERARCHICAL_QP_S    stHierarchicalQp;
    HI_PDT_MEDIA_BIND_SRC_CFG_S astLiveSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_MEDIA_OSD_CFG_S stOsdCfg;
    PDT_PARAM_MEDIA_COMM_DISP_CFG_S       astDispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
    HI_PDT_MEDIA_ACAP_CFG_S               astAcapCfg[HI_PDT_MEDIA_ACAP_MAX_CNT];
    HI_PDT_MEDIA_AENC_CFG_S               astAencCfg[HI_PDT_MEDIA_AENC_MAX_CNT];
    HI_PDT_MEDIA_AO_CFG_S                 astAoCfg[HI_PDT_MEDIA_AO_MAX_CNT];
} PDT_PARAM_MEDIA_COMM_CFG_S;


/**=================== USB  ==================== */
/** UVC Attribute */
typedef struct tagPDT_PARAM_UVC_ATTR_S
{
    HI_PDT_PARAM_MEDIAMODE_CFG_S astMediaModeCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_UVC_CFG_S       stUvcCfg;
} PDT_PARAM_UVC_ATTR_S;

/**======== WorkMode Type Common Level  ======== */

/** MediaMode Corresponding WorkModeType Enum */
typedef enum tagPDT_PARAM_WORKMODE_TYPE_E
{
    PDT_PARAM_WORKMODE_TYPE_REC = 0,
    PDT_PARAM_WORKMODE_TYPE_BUTT
} PDT_PARAM_WORKMODE_TYPE_E;

/**======== MediaMode Specified Level  ======== */

/** MediaModeSpec Level :video capture pipe channel attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_VCAP_PIPE_CHN_ATTR_S
{
    SIZE_S     stDestResolution;            /**<dynamic attribute,channel Output resolution setting,
                                                can not be dynamically modified when using GDC */
    FRAME_RATE_CTRL_S     stFrameRate;      /**<dynamic attribute,channel Frame rate control*/
} PDT_PARAM_MEDIAMODE_VCAP_PIPE_CHN_ATTR_S;

/** MediaModeSpec Level : video capture pipe attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_VCAP_PIPE_ATTR_S
{
    FRAME_RATE_CTRL_S       stFrameRate; /**<framerate control, dynamic attribute */
    HI_BOOL                 bIspBypass;
    HI_MAPI_PIPE_ISP_ATTR_S stIspPubAttr;
    HI_U32                  u32SceneParamLinearIdx;
    HI_U32                  u32SceneParamWdrIdx;
    FRAME_INTERRUPT_ATTR_S  stFrameInterruptAttr;
    PDT_PARAM_MEDIAMODE_VCAP_PIPE_CHN_ATTR_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} PDT_PARAM_MEDIAMODE_VCAP_PIPE_ATTR_S;

/** MediaModeSpec Level : Video capture Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_VCAP_DEV_ATTR_S
{
    HI_MEDIA_SENSOR_ATTR_S   stSnsAttr;
    HI_MAPI_VCAP_DEV_ATTR_S stVcapDevAttr;
    PDT_PARAM_MEDIAMODE_VCAP_PIPE_ATTR_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} PDT_PARAM_MEDIAMODE_VCAP_DEV_ATTR_S;

/** MediaModeSpec Level : video processor vpss port attribute */
typedef struct tagPDT_PARAM_MEDIAMODE_VPSS_PORT_ATTR_S
{
    SIZE_S     stResolution;
    FRAME_RATE_CTRL_S  stFrameRate;
} PDT_PARAM_MEDIAMODE_VPSS_PORT_ATTR_S;

/** MediaModeSpec Level : video processor vpss attribute */
typedef struct hiPDT_PARAM_MEDIAMODE_VPSS_ATTR_S
{
    HI_U32 u32MaxW;
    HI_U32 u32MaxH;
    FRAME_RATE_CTRL_S  stFrameRate;
    PDT_PARAM_MEDIAMODE_VPSS_PORT_ATTR_S astVportAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} PDT_PARAM_MEDIAMODE_VPSS_ATTR_S;

/** MediaModeSpec Level : Video Encoder Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_VENC_ATTR_S
{
    HI_U32  u32Width;                        /**<width of a picture to be encoded, in pixel*/
    HI_U32  u32Height;                       /**<height of a picture to be encoded, in pixel*/
    HI_U32  u32BufSize;                      /**<stream buffer size*/

    HI_U32    u32Gop;             /**<I frame interval [1, 65536]*/
    HI_FR32   u32SrcFrameRate;       /**<frame rate [1/64, 240]*/
    HI_FR32   u32DstFrameRate;       /**<frame rate [1/64, 240]*/
    HI_U32    u32H264_BitRate;
    HI_U32    u32H265_BitRate;
    HI_U32    u32StatTime;     /**<the rate statistic time [1, 60], the unit is sencond(s) */
} PDT_PARAM_MEDIAMODE_VENC_ATTR_S;


typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S
{
    HI_U32 u32VpssCnt;
    PDT_PARAM_MEDIAMODE_VPSS_ATTR_S astVpssCfg[HI_PDT_VCAPDEV_VPSS_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S;

typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S
{
    HI_U32 u32VencCnt;
    PDT_PARAM_MEDIAMODE_VENC_ATTR_S astVencAttr[HI_PDT_VCAPDEV_VENC_MAX_CNT];
} PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S;

/** MediaMode Special Configure */
typedef struct tagPDT_PARAM_MEDIAMODE_SPEC_CFG_S
{
    HI_PDT_MEDIAMODE_E                   enMediaMode;
    HI_MEDIA_VI_VPSS_MODE_S astMode[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
    HI_PDT_MEDIA_VB_CFG_S                stVBCfg;
    PDT_PARAM_MEDIAMODE_VCAP_DEV_ATTR_S  stVcapAttr;
    PDT_PARAM_MEDIAMODE_SPEC_VPROC_CFG_S stVprocCfg;
    PDT_PARAM_MEDIAMODE_SPEC_VENC_CFG_S  stVencCfg;
} PDT_PARAM_MEDIAMODE_SPEC_CFG_S;

typedef struct tagPDT_PARAM_CAM_MEDIA_CFG_S
{
    HI_S32 s32CamID;
    HI_BOOL  bEnable;
    HI_BOOL  bOsdShow;
    PDT_PARAM_CAM_MEDIAMODE_COMM_CFG_S stMediaCommCfg[PDT_PARAM_MEDIAMODE_TYPE_BUTT];
    HI_U32 u32MediaModeCnt;
    PDT_PARAM_MEDIAMODE_SPEC_CFG_S astMediaModeSpecCfg[PDT_PARAM_MEDIAMODE_CNT];
} PDT_PARAM_CAM_MEDIA_CFG_S;


/**------------------- WorkMode Configure  ------------------------- */

/** WorkMode Common Configure */
typedef struct tagPDT_PARAM_WORKMODE_COMM_CFG_S
{
    HI_PDT_WORKMODE_E           enPoweronWorkMode;
} PDT_PARAM_WORKMODE_COMM_CFG_S;


/** record workmode media configure*/
typedef struct tagPDT_PARAM_WORKMODE_REC_MEDIA_CFG_S
{
    HI_PDT_PARAM_MEDIAMODE_CFG_S astMediaModeCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_PHOTO_SRC_S     stPhotoSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S;

/** record workmode photo media configure*/
typedef struct tagPDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S
{
    HI_PDT_PARAM_MEDIAMODE_CFG_S astMediaModeCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_PHOTO_SRC_S     stPhotoSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S;


/** Record comm Param */
typedef struct tagPDT_PARAM_REC_COMM_ATTR_S
{
    HI_RECMNG_MUXER_TYPE_E      enRecMuxerType;
    HI_RECMNG_THM_TYPE_E        enRecThmType;
    HI_U32                      u32RepairUnit;   /**<repair data unit in bytes, [10M,500M]
                                                     0: not support repair */
    HI_U32                      u32PreAllocUnit; /**<pre allocate size in bytes, [0,100M]
                                                     0 for not use pre allocate function*/
} PDT_PARAM_REC_COMM_ATTR_S;

/** Record Manage Param */
typedef struct tagPDT_PARAM_RECMNG_ATTR_S
{
    HI_BOOL  bEnable;                           /**record task is enable, HI_TRUE: enable,  HI_FALSE: DISABLE*/
    HI_REC_SPLIT_ATTR_S stSplitAttr;      /**<split attribute, include split type and parameter */
    HI_U32              u32BufferTime_ms; /**<record buffer size in mili second */
    HI_U32              u32PreRecTimeSec; /**<pre-record time in second */
    HI_U32 u32FileCnt;
    HI_RECMNG_FILE_DATA_SOURCE_S astRecFileDataSrc[HI_PDT_RECTASK_FILE_MAX_CNT];
    HI_RECMNG_THM_ATTR_S stThmAttr;
    HI_U32              au32VBufSize[HI_PDT_RECTASK_FILE_MAX_CNT]; /**<set the vbuf size for fwrite (0,5M] unit :byte*/
    HI_S32 s32PlayFps;  /**<play framerate fps: -1 use venc fps; >0 use assigned fps */
    HI_REC_TYPE_E               enRecType;
    HI_U32                      u32Interval_ms;
} PDT_PARAM_RECMNG_ATTR_S;

/** Normal Record Specified Attribute */
typedef struct tagPDT_PARAM_NORM_REC_ATTR_S
{
    PDT_PARAM_RECMNG_ATTR_S   stRecMngAttr;
} PDT_PARAM_NORM_REC_ATTR_S;

/** Usb comm Param */
typedef struct tagPDT_PARAM_USB_COMM_ATTR_S
{
    HI_USB_MODE_E              enUsbMode;
} PDT_PARAM_USB_COMM_ATTR_S;

/** WorkMode Configure */
typedef struct tagPDT_PARAM_WORKMODE_CFG_S
{
    PDT_PARAM_WORKMODE_COMM_CFG_S           stCommCfg;
    PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S      stRecMediaCfg;
    PDT_PARAM_REC_COMM_ATTR_S               stRecCommAttr;
    PDT_PARAM_NORM_REC_ATTR_S               astNormRecAttr[HI_PDT_RECTASK_MAX_CNT];
    PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S    stPhotoMediaCfg;
    HI_PDT_PARAM_PLAYBACK_CFG_S             stPlayBackCfg;
    PDT_PARAM_USB_COMM_ATTR_S               stUsbCommAttr;
    PDT_PARAM_UVC_ATTR_S                    stUvcCfg;
    HI_USB_STORAGE_CFG_S                    stUsbStorageCfg;
} PDT_PARAM_WORKMODE_CFG_S;

/**------------------- DevMng  ------------------------- */
/** wifi ap configure */
typedef struct hiPDT_WIFI_APMODE_CFG_S
{
    HI_CHAR szFactroyWiFiSSID[HI_HAL_WIFI_SSID_LEN]; /**<factroy wifi ssid,aszWiFiSSID*/
    HI_HAL_WIFI_APMODE_CFG_S stWiFiApCfg;
} HI_PDT_WIFI_APMODE_CFG_S;

/** Device Configure */
typedef struct tagPDT_PARAM_DEVMNG_CFG_S
{
    HI_PDT_DEV_INFO_S        stDevInfo;
    HI_STORAGEMNG_CFG_S      stStorageCfg;
    HI_KEYMNG_CFG_S          stkeyMngCfg;
    HI_GAUGEMNG_CFG_S        stGaugeMngCfg;
    HI_PDT_WIFI_INFO_S       stWiFiInfo;
    HI_PDT_WIFI_APMODE_CFG_S stWiFiApModeCfg;
    HI_TIMEDTASK_ATTR_S      stScreenDormantAttr;
    HI_PDT_GSENSOR_CFG_S     stGsensorCfg;
    HI_S32                   s32ScreenBrightness;
    HI_S32                   s32GsensorSensitivity;
    HI_S32                   s32ParkingLevel;
    HI_S32                   s32ParkingCollision; /**<-parking collision count----*/
    HI_BOOL                  bKeyToneEnable;
    HI_BOOL                  bBootSoundEnable;
    HI_S32                   s32SystemLanguage;
} PDT_PARAM_DEVMNG_CFG_S;


/**-------------------  ParamItem ValueSet  ------------------------- */

/** Configure Item Supported ValueSet */
typedef struct tagPDT_PARAM_CONFITEM_VALUESET_S
{
    HI_PDT_ITEM_VALUESET_S astVideoPayloadType[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S stScreenDormantValues;
    HI_PDT_ITEM_VALUESET_S stScreenBrightnessValues;
    HI_PDT_ITEM_VALUESET_S astMediaModeValues[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S astPhotoMediaModeValues[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S stVolume;
    HI_PDT_ITEM_VALUESET_S stSensitivityLevel;
    HI_PDT_ITEM_VALUESET_S stParkingLevel;
    HI_PDT_ITEM_VALUESET_S stUsbMode;
    HI_PDT_ITEM_VALUESET_S stSystemLanguage;
    HI_PDT_ITEM_VALUESET_S astWdrState[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S astVideoMode[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S astLdcState[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S astFlipState[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S astMirrorState[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_ITEM_VALUESET_S stRecSplitTime;
    HI_PDT_ITEM_VALUESET_S stLapseRecIntervalTime;
    HI_PDT_ITEM_VALUESET_S stRecordType;
#ifdef CONFIG_MOTIONDETECT_ON
    HI_PDT_ITEM_VALUESET_S stMdSensitivity;
#endif
} PDT_PARAM_CONFITEM_VALUESET_S;


/**------------------- Param Struct  ------------------------- */

/** product param head information */
typedef struct tagPDT_PARAM_HEAD_S
{
    HI_U32 u32MagicStart;
    HI_U32 u32SysLen;
} PDT_PARAM_HEAD_S;

/** Param Configure */
typedef struct tagPDT_PARAM_CFG_S
{
    PDT_PARAM_HEAD_S               stHead;         /**<param head */
    PDT_PARAM_MEDIA_COMM_CFG_S     stMediaCommCfg; /**<media common configure */
    PDT_PARAM_WORKMODE_CFG_S       stWorkModeCfg;  /**<workmode configure */
    PDT_PARAM_CAM_MEDIA_CFG_S      astCamMediaCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT]; /**<each cam media configure*/
    HI_PDT_FILEMNG_CFG_S           stFileMngCfg;
    PDT_PARAM_DEVMNG_CFG_S         stDevMngCfg;
    PDT_PARAM_CONFITEM_VALUESET_S  stItemValues;   /**<configure item valueset */
    HI_U32                         u32MagicEnd;    /**<param end */
} PDT_PARAM_CFG_S;

/**------------------- Param Context  ------------------------- */

/** Param Context */
typedef struct tagPDT_PARAM_CONTEXT_S
{
    HI_BOOL         bInit;     /**<module init status */
    pthread_mutex_t mutexLock; /**<param lock, protect pstCfg */
    PDT_PARAM_CFG_S* pstCfg;   /**<param address */
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

