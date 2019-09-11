/**
 * @file      hi_product_media_define.h
 * @brief     product media struct definition
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */

#ifndef __HI_PRODUCT_MEDIA_DEFINE_H__
#define __HI_PRODUCT_MEDIA_DEFINE_H__

#include "hi_media_comm_define.h"


#ifdef __cplusplus
#if __cplusplus
 extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     MEDIA */
/** @{ */  /** <!-- [MEDIA] */
#ifdef ONE_SENSOR_CONNECT

#define HI_PDT_MEDIA_VCAP_DEV_MAX_CNT        (1)/**<video capture device(sensor) maximum count */
#define HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT    (2)/**<video capture pipe maximum count */
#define HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT    (1)/**<video capture pipe channel maximum count */
#define HI_PDT_MEDIA_VPSS_MAX_CNT            (1)/**<video processor vpss maximum count */
#define HI_PDT_MEDIA_VPSS_PORT_MAX_CNT       (3)/**<vpss port maximum count in each vproc vpss */
#define HI_PDT_MEDIA_VENC_MAX_CNT            (5)/**<video encoder maximum count */

#elif TWO_SENSOR_CONNECT

#define HI_PDT_MEDIA_VCAP_DEV_MAX_CNT        (2)/**<video capture device(sensor) maximum count */
#define HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT    (2)/**<video capture pipe maximum count */
#define HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT    (1)/**<video capture pipe channel maximum count */
#define HI_PDT_MEDIA_VPSS_MAX_CNT            (2)/**<video processor vpss maximum count */
#define HI_PDT_MEDIA_VPSS_PORT_MAX_CNT       (3)/**<vpss port maximum count in each vproc vpss */
#define HI_PDT_MEDIA_VENC_MAX_CNT            (8)/**<video encoder maximum count */

#else
#define HI_PDT_MEDIA_VCAP_DEV_MAX_CNT        (2)/**<video capture device(sensor) maximum count */
#define HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT    (2)/**<video capture pipe maximum count */
#define HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT    (1)/**<video capture pipe channel maximum count */
#define HI_PDT_MEDIA_VPSS_MAX_CNT            (2)/**<video processor vpss maximum count */
#define HI_PDT_MEDIA_VPSS_PORT_MAX_CNT       (3)/**<vpss port maximum count in each vproc vpss */
#define HI_PDT_MEDIA_VENC_MAX_CNT            (8)/**<video encoder maximum count */

#endif

#define HI_PDT_MEDIA_DISP_MAX_CNT            (1)/**<video display maximum count */
#define HI_PDT_MEDIA_DISP_WND_MAX_CNT        (HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)/**<video display window maximum count */

#define HI_PDT_MEDIA_ACAP_MAX_CNT            (1)/**<audio capture maximum count */
#define HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT       (2)/**<audio capture chnl maximum count */

#define HI_PDT_MEDIA_AENC_MAX_CNT            (1)/**<audio encoder maximum count */
#define HI_PDT_MEDIA_AO_MAX_CNT              (1)/**<audio output maximum count */

#define HI_PDT_MEDIA_VB_MAX_POOLS            (8) /**<vb maximum pools count */
#define UI_FILELIST_WINDOW_LEFT_LEN          (100)/**<irreguar screen */


/** media video module enum */
typedef enum hiPDT_MEDIA_VIDEOMOD_E
{
    HI_PDT_MEDIA_VIDEOMOD_VCAP = 0,
    HI_PDT_MEDIA_VIDEOMOD_VPSS,
    HI_PDT_MEDIA_VIDEOMOD_VENC,
    HI_PDT_MEDIA_VIDEOMOD_DISP,
    HI_PDT_MEDIA_VIDEOMOD_BUTT
} HI_PDT_MEDIA_VIDEOMOD_E;

typedef struct hiPDT_MEDIA_BIND_SRC_CFG_S
{
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE  ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE  ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
}HI_PDT_MEDIA_BIND_SRC_CFG_S;

/** media vi-vpss mode */
typedef struct hiPDT_MEDIA_VI_VPSS_MODE_S
{
    HI_MEDIA_VI_VPSS_MODE_S astMode[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT][HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} HI_PDT_MEDIA_VI_VPSS_MODE_S;

/** Media VB Configure */
typedef struct hiPDT_MEDIA_VB_CFG_S
{
    HI_U32 u32MaxPoolCnt; /**<maximum pools count, (0, HI_PDT_MEDIA_VB_MAX_POOLS] */
    struct tagMEDIA_CPOOL_S
    {
        HI_U32 u32BlkSize; /**<VB Block size */
        HI_U32 u32BlkCnt;  /**<VB Block count */
    } astCommPool[HI_PDT_MEDIA_VB_MAX_POOLS];
} HI_PDT_MEDIA_VB_CFG_S;

/** video capture pipe channel attribute */
typedef struct hPDT_MEDIA_VCAP_PIPE_CHN_ATTR_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  PipeChnHdl;
    SIZE_S     stDestResolution;            /**<dynamic attribute,channel Output resolution setting,
                                                can not be dynamically modified when using GDC */
    FRAME_RATE_CTRL_S     stFrameRate;      /**<dynamic attribute,channel Frame rate control*/
    HI_MEDIA_PIXEL_FMT_E  enPixelFormat;    /**<static attribute,[8bit,10bit,12bit,14bit,16bit]*/
    ROTATION_E enRotate;
    HI_BOOL    bFlip;
    HI_BOOL    bMirror;
    HI_S32 s32Brightness;                   /**<brightness, range[0,100] */
    HI_S32 s32Saturation;                   /**<saturation, range[0,100] */
    //HI_S32 s32Sharpen;
    VI_LDC_ATTR_S stLDCAttr;
} HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S;

/** video capture pipe attribute */
typedef struct hiPDT_MEDIA_VCAP_PIPE_ATTR_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VcapPipeHdl;
    HI_MAPI_PIPE_TYPE_E     enPipeType;  /**<pipe type, static attribute */
    FRAME_RATE_CTRL_S       stFrameRate; /**<framerate control, dynamic attribute */
    HI_BOOL                 bIspBypass;
    HI_MAPI_PIPE_ISP_ATTR_S stIspPubAttr;
    FRAME_INTERRUPT_ATTR_S  stFrameIntAttr;
    HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S astPipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} HI_PDT_MEDIA_VCAP_PIPE_ATTR_S;

/** video capture device attribute */
typedef struct hiPDT_MEDIA_VCAP_DEV_ATTR_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VcapDevHdl;
    WDR_MODE_E enWdrMode;
    HI_U32     u32VideoMode;
    SIZE_S     stResolution;
    HI_MEDIA_SENSOR_ATTR_S stSnsAttr;
    HI_MAPI_SNAP_EXIF_INFO_S stExifInfo;
    HI_PDT_MEDIA_VCAP_PIPE_ATTR_S astVcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} HI_PDT_MEDIA_VCAP_DEV_ATTR_S;

/** video capture configure */
typedef struct hiPDT_MEDIA_VCAP_CFG_S
{
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S  astVcapDevAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} HI_PDT_MEDIA_VCAP_CFG_S;

/** video processor vpss port attribute */
typedef struct hiPDT_MEDIA_VPSS_PORT_ATTR_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VportHdl;
    SIZE_S     stResolution;
    FRAME_RATE_CTRL_S  stFrameRate;
    ROTATION_E enRotate;
    HI_BOOL    bFlip;
    HI_BOOL    bMirror;
    VIDEO_FORMAT_E enVideoFormat;
    HI_MEDIA_PIXEL_FMT_E enPixelFormat;
    HI_BOOL    bSupportBufferShare;
} HI_PDT_MEDIA_VPSS_PORT_ATTR_S;

/** video processor vpss attribute */
typedef struct hiPDT_MEDIA_VPSS_ATTR_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VpssHdl;           /**<video processor vpss grp handle */
    HI_HANDLE VcapPipeHdl;       /**<binded video capture pipe handle */
    HI_HANDLE VcapPipeChnHdl;    /**<binded video capture pipe chn handle */
    HI_MEDIA_VPSS_ATTR_S stVpssAttr; /**<vproc vpss attribute */
    HI_PDT_MEDIA_VPSS_PORT_ATTR_S astVportAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} HI_PDT_MEDIA_VPSS_ATTR_S;

/** video processor configure */
typedef struct hiPDT_MEDIA_VPROC_CFG_S
{
    HI_PDT_MEDIA_VPSS_ATTR_S astVpssAttr[HI_PDT_MEDIA_VPSS_MAX_CNT]; /**<vpss attribute */
} HI_PDT_MEDIA_VPROC_CFG_S;

/** video Encoder Type */
typedef enum hiPDT_MEDIA_VENC_TYPE_E
{
    HI_PDT_MEDIA_VENC_TYPE_REC = 0, /**<rec video stream */
    HI_PDT_MEDIA_VENC_TYPE_LIVE,      /**<live video stream */
    HI_PDT_MEDIA_VENC_TYPE_SNAP,    /**<snap */
    HI_PDT_MEDIA_VENC_TYPE_THM,       /**<thumbnail */
    HI_PDT_MEDIA_VENC_TYPE_BUTT
} HI_PDT_MEDIA_VENC_TYPE_E;

/** video encoder configure */
typedef struct hiPDT_MEDIA_VENC_CFG_S
{
    HI_BOOL   bEnable;      /**<video encoder enable or not */
    HI_HANDLE VencHdl;      /**<video encoder handle */
    HI_PDT_MEDIA_VENC_TYPE_E    enVencType;/** 0:rec video, 1: live video, 2: snap, 3: thumbnail*/
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
    HI_MEDIA_VENC_ATTR_S stVencAttr;
} HI_PDT_MEDIA_VENC_CFG_S;

typedef struct hiPDT_MEDIA_OSD_CFG_S
{
    HI_S32 s32OsdCnt;
    SIZE_S  stBaseFontSize;
    SIZE_S  stBaseImageSize;
    HI_OSD_ATTR_S astOsdAttr[HI_OSD_MAX_CNT];
} HI_PDT_MEDIA_OSD_CFG_S;

/** video configure */
typedef struct hiPDT_MEDIA_VIDEO_CFG_S
{
    HI_PDT_MEDIA_VCAP_CFG_S  stVcapCfg;
    HI_PDT_MEDIA_VPROC_CFG_S stVprocCfg;
    HI_PDT_MEDIA_VENC_CFG_S  astVencCfg[HI_PDT_MEDIA_VENC_MAX_CNT];
    HI_PDT_MEDIA_OSD_CFG_S   stOsdCfg;
} HI_PDT_MEDIA_VIDEO_CFG_S;

/** video display window crop info */
typedef struct hiPDT_MEDIA_DISP_WND_CROP_CFG_S
{
    HI_BOOL     bEnable;        /**<video display window crop enable or not */
    HI_U32      u32MaxH;        /**<fixed */
    HI_U32      u32CurH;        /**<range:[0,u32MaxH-1] */
} HI_PDT_MEDIA_DISP_WND_CROP_CFG_S;


/** video display window configure */
typedef struct hiPDT_MEDIA_DISP_WND_CFG_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  WndHdl;
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE  ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE  ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
    ROTATION_E enRotate;
    ASPECT_RATIO_S stAspectRatio;
    HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
    HI_PDT_MEDIA_DISP_WND_CROP_CFG_S stCropCfg;
} HI_PDT_MEDIA_DISP_WND_CFG_S;

/** video display csc attribute */
typedef struct hiPDT_MEDIA_DISP_CSC_ATTR_S
{
    HI_BOOL bEnable;
    HI_MAPI_DISP_CSCATTREX_S stAttrEx;
} HI_PDT_MEDIA_DISP_CSC_ATTR_S;

/** video display configure */
typedef struct hiPDT_MEDIA_DISP_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VdispHdl;
    HI_MAPI_DISP_ATTR_S stDispAttr;
    SIZE_S stImageSize;
    HI_U32 u32BufferLen;
    HI_PDT_MEDIA_DISP_CSC_ATTR_S stCscAttr;
    HI_PDT_MEDIA_DISP_WND_CFG_S  astWndCfg[HI_PDT_MEDIA_DISP_WND_MAX_CNT];
} HI_PDT_MEDIA_DISP_CFG_S;

/** video out configure */
typedef struct hiPDT_MEDIA_VIDEOOUT_CFG_S
{
    HI_PDT_MEDIA_DISP_CFG_S astDispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
} HI_PDT_MEDIA_VIDEOOUT_CFG_S;

typedef struct hiPDT_MEDIA_ACAPCHNL_CFG_S
{
    HI_HANDLE AcapChnlHdl;
    HI_BOOL bEnable;
    HI_BOOL bEnableVqe;
} HI_PDT_MEDIA_ACAPCHNL_CFG_S;


/** audio capture configure */
typedef struct hiPDT_MEDIA_ACAP_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE AcapHdl;
    HI_PDT_MEDIA_ACAPCHNL_CFG_S AcapChnlCfg[HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT];
    HI_MAPI_ACAP_ATTR_S stAcapAttr;
    HI_S32    s32AudioGain; /* ?->percent volume */
} HI_PDT_MEDIA_ACAP_CFG_S;

/** audio Encoder configure */
typedef struct hiPDT_MEDIA_AENC_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE AencHdl;
    HI_HANDLE AcapHdl;     /**<binded audio capture handle */
    HI_HANDLE AcapChnlHdl;     /**<binded audio capture channel handle */
    HI_MAPI_AUDIO_FORMAT_E enFormat;
    HI_U32 u32PtNumPerFrm; /**<sampling point number per frame*/
    union tagPDT_MEDIA_AENC_ENCODER_U
    {
        HI_MAPI_AENC_ATTR_AAC_S stAACAttr;
    } unAttr;
} HI_PDT_MEDIA_AENC_CFG_S;

/** audio configure */
typedef struct hiPDT_MEDIA_AUDIO_CFG_S
{
    HI_PDT_MEDIA_ACAP_CFG_S astAcapCfg[HI_PDT_MEDIA_ACAP_MAX_CNT];
    HI_PDT_MEDIA_AENC_CFG_S astAencCfg[HI_PDT_MEDIA_AENC_MAX_CNT];
} HI_PDT_MEDIA_AUDIO_CFG_S;

/** audio output configure */
typedef struct hiPDT_MEDIA_AO_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE AoHdl;
    HI_MAPI_AO_ATTR_S stAoAttr;
    HI_S32    s32Volume;
} HI_PDT_MEDIA_AO_CFG_S;

/** audio out configure */
typedef struct hiPDT_MEDIA_AUDIOOUT_CFG_S
{
    HI_PDT_MEDIA_AO_CFG_S astAoCfg[HI_PDT_MEDIA_AO_MAX_CNT];
} HI_PDT_MEDIA_AUDIOOUT_CFG_S;

/** media configure */
typedef struct hiPDT_MEDIA_CFG_S
{
    HI_PDT_MEDIA_VI_VPSS_MODE_S stViVpssMode;
    HI_PDT_MEDIA_VB_CFG_S       stVBCfg;
    HI_PDT_MEDIA_VIDEO_CFG_S    stVideoCfg;
    HI_PDT_MEDIA_VIDEOOUT_CFG_S stVideoOutCfg;
    HI_PDT_MEDIA_AUDIO_CFG_S    stAudioCfg;
    HI_PDT_MEDIA_AUDIOOUT_CFG_S stAudioOutCfg;
} HI_PDT_MEDIA_CFG_S;

/** @}*/  /** <!-- ==== MEDIA End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_MEDIA_DEFINE_H__ */

