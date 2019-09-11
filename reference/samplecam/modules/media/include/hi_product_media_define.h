
/**
 * @file    hi_product_media_define.h
 * @brief   media module struct
 *
 * Copyright (c) 2017 Huawei Tech.Co., Ltd.
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2019/4/1
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

#define HI_PDT_MEDIA_VCAP_DEV_MAX_CNT        (2) /**<video capture device(sensor) maximum count */
#define HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT    (2) /**<video capture pipe maximum count */
#define HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT    (1) /**<video capture pipe channel maximum count */
#define HI_PDT_MEDIA_VPSS_MAX_CNT            (2) /**<video processor vpss maximum count */
#define HI_PDT_MEDIA_VPSS_PORT_MAX_CNT       (2) /**<vpss port maximum count in each vproc vpss */

#define HI_PDT_MEDIA_DISP_MAX_CNT            (1) /**<video display maximum count */
#define HI_PDT_MEDIA_DISP_WND_MAX_CNT        (HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)/**<video display window maximum count */

#define HI_PDT_MEDIA_ACAP_MAX_CNT            (1) /**<audio capture maximum count */
#define HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT       (2) /**<audio capture chnl maximum count */

#define HI_PDT_MEDIA_AENC_MAX_CNT            (1) /**<audio encoder maximum count */
#define HI_PDT_MEDIA_AO_MAX_CNT              (1) /**<audio output maximum count */

#define HI_PDT_MEDIA_VB_MAX_POOLS            (8) /**<vb maximum pools count */


/** media vi-vpss mode */
typedef struct tagMEDIA_ViVpssMode {
    HI_MEDIA_VI_VPSS_MODE_S mode[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT][HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} MEDIA_ViVpssMode;

/** Media VB Configure */
typedef struct tagMEDIA_VbCfg {
    HI_U32 maxPoolCnt; /**<maximum pools count, (0, HI_PDT_MEDIA_VB_MAX_POOLS] */
    struct tagMEDIA_CommPool {
        HI_U32 blkSize; /**<VB Block size */
        HI_U32 blkCnt;  /**<VB Block count */
    } commPool[HI_PDT_MEDIA_VB_MAX_POOLS];
} MEDIA_VbCfg;

/** media video module enum */
typedef enum tagMEDIA_VideoMod {
    MEDIA_VIDEOMOD_VCAP = 0,
    MEDIA_VIDEOMOD_VPSS,
    MEDIA_VIDEOMOD_VENC,
    MEDIA_VIDEOMOD_DISP,
    MEDIA_VIDEOMOD_BUTT
} MEDIA_VideoMod;

/** video capture pipe channel attribute */
typedef struct tagMEDIA_VcapPipeChnAttr {
    HI_BOOL              enable;
    HI_HANDLE            pipeChnHdl;
    SIZE_S               destSize;
    FRAME_RATE_CTRL_S    frameRate;      /**<dynamic attribute,channel Frame rate control*/
    COMPRESS_MODE_E      compressMode;
    HI_MEDIA_PIXEL_FMT_E pixelFormat;    /**<static attribute,[8bit,10bit,12bit,14bit,16bit]*/
} MEDIA_VcapPipeChnAttr;

/** video capture pipe attribute */
typedef struct tagMEDIA_VcapPipeAttr {
    HI_BOOL   enable;
    HI_HANDLE pipeHdl;
    HI_MAPI_PIPE_TYPE_E     piepType;  /**<pipe type, static attribute */
    FRAME_RATE_CTRL_S       frameRate; /**<framerate control, dynamic attribute */
    COMPRESS_MODE_E         compressMode;
    HI_BOOL                 ispBypass;
    HI_MAPI_PIPE_ISP_ATTR_S ispPubAttr;
    MEDIA_VcapPipeChnAttr   pipeChnAttr[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} MEDIA_VcapPipeAttr;

/** video capture device attribute */
typedef struct tagMEDIA_VcapDevAttr {
    HI_BOOL    enable;
    HI_HANDLE  devHdl;
    HI_MAPI_SENSOR_ATTR_S   snsAttr;
    HI_MAPI_VCAP_DEV_ATTR_S devAttr;
    MEDIA_VcapPipeAttr      vcapPipeAttr[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} MEDIA_VcapDevAttr;

/** video capture configure */
typedef struct tagMEDIA_VcapCfg {
    MEDIA_VcapDevAttr  devAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} MEDIA_VcapCfg;

/** video processor vpss port attribute */
typedef struct tagMEDIA_VpssPortAttr {
    HI_BOOL    enable;
    HI_HANDLE  portHdl;
    SIZE_S     resolution;
    HI_BOOL    supportBufferShare;
    FRAME_RATE_CTRL_S    frameRate;
    VIDEO_FORMAT_E       videoFormat;
    HI_MEDIA_PIXEL_FMT_E pixelFormat;
} MEDIA_VpssPortAttr;

/** video processor vpss attribute */
typedef struct tagMEDIA_VpssAttr {
    HI_BOOL   enable;
    HI_HANDLE vpssHdl;             /**<video processor vpss grp handle */
    HI_HANDLE vcapPipeHdl;         /**<binded video capture pipe handle */
    HI_HANDLE vcapPipeChnHdl;      /**<binded video capture pipe chn handle */
    HI_MEDIA_VPSS_ATTR_S vpssAttr; /**<vproc vpss attribute */
    MEDIA_VpssPortAttr   portAttr[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} MEDIA_VpssAttr;

/** video processor configure */
typedef struct tagMEDIA_VprocCfg {
    MEDIA_VpssAttr vpssAttr[HI_PDT_MEDIA_VPSS_MAX_CNT]; /**<vpss attribute */
} MEDIA_VprocCfg;

/** video configure */
typedef struct tagMEDIA_VideoCfg {
    MEDIA_VcapCfg  vcapCfg;
    MEDIA_VprocCfg vprocCfg;
} MEDIA_VideoCfg;

/** video display window configure */
typedef struct tagMEDIA_DispWndCfg {
    HI_BOOL    enable;
    HI_HANDLE  wndHdl;
    MEDIA_VideoMod bindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE  modHdl;        /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE  chnHdl;        /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
    ROTATION_E rotation;
    HI_MAPI_DISP_WINDOW_ATTR_S wndAttr;
} MEDIA_DispWndCfg;

/** video display csc attribute */
typedef struct tagMEDIA_DispCscAttr {
    HI_BOOL enable;
    HI_MAPI_DISP_CSCATTREX_S cscAttr;
} MEDIA_DispCscAttr;

/** video display configure */
typedef struct tagMEDIA_DispCfg {
    HI_BOOL   enable;
    HI_HANDLE dispHdl;
    HI_MAPI_DISP_ATTR_S dispAttr;
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S videoLayerAttr;
    MEDIA_DispCscAttr   cscAttr;
    MEDIA_DispWndCfg    wndCfg[HI_PDT_MEDIA_DISP_WND_MAX_CNT];
} MEDIA_DispCfg;

/** video out configure */
typedef struct tagMEDIA_VideoOutCfg {
    MEDIA_DispCfg dispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
} MEDIA_VideoOutCfg;

/** media configure */
typedef struct tagMEDIA_Cfg {
    MEDIA_ViVpssMode  viVpssMode;
    MEDIA_VbCfg       vbCfg;
    MEDIA_VideoCfg    videoCfg;
    MEDIA_VideoOutCfg videoOutCfg;
} MEDIA_Cfg;


/** @}*/  /** <!-- ==== MEDIA End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

