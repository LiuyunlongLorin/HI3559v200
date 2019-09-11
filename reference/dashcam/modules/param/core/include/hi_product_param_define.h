/**
 * @file      hi_product_param_define.h
 * @brief     parameter module struct definition
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#ifndef __HI_PRODUCT_PARAM_DEFINE_H__
#define __HI_PRODUCT_PARAM_DEFINE_H__

#include "hi_appcomm.h"
#include "hi_product_media_define.h"
#include "hi_photomng.h"
#include "hi_recordmng.h"
#include "hi_filemng_dtcf.h"
#include "hi_usb.h"
#include "hi_keymng.h"
#include "hi_gaugemng.h"
#include "hi_storagemng.h"
#include "hi_hal_wifi.h"
#include "hi_hal_gsensor.h"
#include "hi_product_scene_define.h"
#include "hi_liteplayer.h"

#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

/** record media stream maximum number */
#define HI_PDT_RECTASK_FILE_MAX_CNT (1)

/** item value description maximum length */
#define HI_PDT_ITEM_VALUE_DESCRIPTION_LEN  (12)

/** item value set maximum number */
#define HI_PDT_ITEM_VALUESET_MAX_NUM       (10)

/** max vpss number in one vcapdev */
#define HI_PDT_VCAPDEV_VPSS_MAX_CNT     (1)

/** max venc number in one vcapdev */
#define HI_PDT_VCAPDEV_VENC_MAX_CNT     (5)

#define HI_PDT_RECTASK_MAX_CNT          (HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
#define HI_PDT_RAWCAPTASK_MAX_CNT     HI_PDT_RECTASK_MAX_CNT

/** WorkMode Enum */
typedef enum hiPDT_WORKMODE_E
{
    HI_PDT_WORKMODE_NORM_REC = 0,
    HI_PDT_WORKMODE_PHOTO,
    HI_PDT_WORKMODE_PLAYBACK,
    HI_PDT_WORKMODE_UVC,
    HI_PDT_WORKMODE_USB_STORAGE,
    HI_PDT_WORKMODE_PARKING_REC,
    HI_PDT_WORKMODE_UPGRADE,
    HI_PDT_WORKMODE_BUTT
} HI_PDT_WORKMODE_E;

/** MediaMode Enum
      Record/Record+Photo: Resolution_FrameRate_Mode
      Photo/MPhoto: Resolution_Mode */
typedef enum hiPDT_MEDIAMODE_E
{
    HI_PDT_MEDIAMODE_720P_30 = 0,
    HI_PDT_MEDIAMODE_1080P_30,
    HI_PDT_MEDIAMODE_1080P_25,
    HI_PDT_MEDIAMODE_1080P_60,
    HI_PDT_MEDIAMODE_1296P_30,
    HI_PDT_MEDIAMODE_1440P_30,
    HI_PDT_MEDIAMODE_1440P_25,
    HI_PDT_MEDIAMODE_1600P_30,
    HI_PDT_MEDIAMODE_1520P_30,
    HI_PDT_MEDIAMODE_1944P_30,          /**resolution: 2688*1944 */
    HI_PDT_MEDIAMODE_2160P_30,
    HI_PDT_MEDIAMODE_PHOTO_720P,
    HI_PDT_MEDIAMODE_PHOTO_1080P,
    HI_PDT_MEDIAMODE_PHOTO_1296P,
    HI_PDT_MEDIAMODE_PHOTO_1440P,
    HI_PDT_MEDIAMODE_PHOTO_1600P,
    HI_PDT_MEDIAMODE_PHOTO_1520P,
    HI_PDT_MEDIAMODE_PHOTO_1944P,
    HI_PDT_MEDIAMODE_PHOTO_2160P,
    HI_PDT_MEDIAMODE_BUTT
} HI_PDT_MEDIAMODE_E;


/** Param Type Enum */
typedef enum hiPDT_PARAM_TYPE_E
{
    /** Workmode Dependent Param */
    /** related with cam param*/
    HI_PDT_PARAM_TYPE_CAM_STATUS = 0,      /**<CAM  Status*/
    HI_PDT_PARAM_TYPE_MEDIAMODE,           /**<specify cam HI_PDT_MEDIAMODE_E */
    HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE,    /**<specify cam HI_MPP_PAYLOAD_TYPE_E */
    HI_PDT_PARAM_TYPE_APK_VENCID,          /**<specify cam vencid which phone play video */
    HI_PDT_PARAM_TYPE_FLIP,                /**<All HI_PDT_MEDIA_SWITCH_S */
    HI_PDT_PARAM_TYPE_MIRROR,              /**<All HI_PDT_MEDIA_SWITCH_S */
    HI_PDT_PARAM_TYPE_LDC,                 /**<All HI_PDT_MEDIA_SWITCH_S */
    HI_PDT_PARAM_TYPE_REC,                 /**<ON: record  enable, OFF: do not record  video*/
    HI_PDT_PARAM_TYPE_OSD,                 /**<All HI_BOOL */
    HI_PDT_PARAM_TYPE_CROP,                /**<crop*/
    HI_PDT_PARAM_TYPE_WDR,                 /**<wdr*/
    HI_PDT_PARAM_TYPE_VIDEOMODE,           /**<video mode*/

    HI_PDT_PARAM_TYPE_SPLITTIME,           /**<record file splite time*/
    HI_PDT_PARAM_RECORD_TYPE,               /**<record type*/
    HI_PDT_PARAM_TYPE_LAPSE_INTERVAL,        /**<lapse interval*/
    HI_PDT_PARAM_MD_SENSITIVITY,             /**<md sensitivity*/

    /**not related with cam*/
    HI_PDT_PARAM_TYPE_AUDIO,               /**<All HI_PDT_MEDIA_SWITCH_S */
    HI_PDT_PARAM_TYPE_PREVIEW_CAMID,       /**<CAM ID PARAM*/

    /* Customed Workmode Dependent Param */

    /** WorkMode Independent Param */
    HI_PDT_PARAM_TYPE_POWERON_WORKMODE,       /**<WORKMODE:normal_rec or photo*/
    HI_PDT_PARAM_TYPE_WIFI,                   /**<ON: wifi  on, OFF: wifi off*/
    HI_PDT_PARAM_TYPE_WIFI_AP,                /**<HI_HAL_WIFI_APMODE_CFG_S */
    HI_PDT_PARAM_TYPE_DEV_INFO,               /**<HI_PDT_DEV_INFO_S */
    HI_PDT_PARAM_TYPE_VOLUME,                 /**<HI_S32 */
    HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS,      /**<HI_S32 */
    HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,    /**<HI_PDT_TIMEDTASK_ATTR_S */
    HI_PDT_PARAM_TYPE_KEYTONE,                /**<key tone*/
    HI_PDT_PARAM_TYPE_BOOTSOUND,              /**<boot sound*/
    HI_PDT_PARAM_TYPE_GSENSOR_SENSITIVITY,    /**<G-SENSOR SENSITIVITY*/
    HI_PDT_PARAM_TYPE_GSENSOR_PARKING,        /**<G-SENSOR PARKING SENSITIVITY*/
    HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT,  /**<G-SENSOR PARKING Collision count*/
    HI_PDT_PARAM_TYPE_USB_MODE,
    HI_PDT_PARAM_TYPE_LANGUAGE,
    HI_PDT_PARAM_TYPE_PREVIEWPIP,
    /* Customed Workmode Independent Param */
    HI_PDT_PARAM_TYPE_BUTT
} HI_PDT_PARAM_TYPE_E;

/*disp configure*/
typedef struct hiPDT_PARAM_DISP_CFG_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VdispHdl;
    HI_U32                   u32BgColor;          /* RW; Background color of a device, in RGB format. */
    HI_U32    u32WindowCnt;              /* Maximum display window number*/
    HI_PDT_MEDIA_DISP_CSC_ATTR_S stCscAttr;
    HI_PDT_MEDIA_DISP_WND_CFG_S  astWndCfg[HI_PDT_MEDIA_DISP_WND_MAX_CNT];
} HI_PDT_PARAM_DISP_CFG_S;

/*video out configure*/
typedef struct hiPDT_PARAM_VIDEOOUT_CFG_S
{
    HI_PDT_PARAM_DISP_CFG_S astDispCfg[HI_PDT_MEDIA_DISP_MAX_CNT];
} HI_PDT_PARAM_VIDEOOUT_CFG_S;


/** media comm Configure */
typedef struct hiPDT_PARAM_MEDIA_COMM_CFG_S
{
    HI_PDT_PARAM_VIDEOOUT_CFG_S stVideoOutCfg;
    HI_PDT_MEDIA_AUDIO_CFG_S    stAudioCfg;
    HI_PDT_MEDIA_AUDIOOUT_CFG_S stAudioOutCfg;
}HI_PDT_PARAM_MEDIA_COMM_CFG_S;


/** cam media Configure */
typedef struct hiPDT_PARAM_CAM_MEDIA_CFG_S
{
    HI_BOOL  bOsdShow;
    HI_MEDIA_VI_VPSS_MODE_S astMode[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
    HI_PDT_MEDIA_VB_CFG_S                stVBCfg;
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S  stVcapDevAttr;
    HI_U32 u32VpssCnt;
    HI_PDT_MEDIA_VPSS_ATTR_S astVpssCfg[HI_PDT_VCAPDEV_VPSS_MAX_CNT];
    HI_U32 u32VencCnt;
    HI_PDT_MEDIA_VENC_CFG_S  astVencCfg[HI_PDT_VCAPDEV_VENC_MAX_CNT];
    HI_PDT_MEDIA_DISP_WND_CROP_CFG_S stCropCfg;
} HI_PDT_PARAM_CAM_MEDIA_CFG_S;

/**------------------- Scene Vcap Configure----------------------- */
typedef struct hiPDT_PARAM_SCENE_CAM_VCAP_ATTR_S
{
    HI_BOOL bIspBypass;
    HI_HANDLE MainPipeHdl;          /**< MainIsp Pipe hdl, Isp in this  pipe can Control sensor */
    HI_HANDLE VcapPipeHdl;          /**< VcapPipe hdl */
    HI_HANDLE PipeChnHdl;           /**< VcapPipe CHn hdl */
    HI_HANDLE VpssHdl;              /**< Vpss hdl */
    HI_HANDLE VportHdl;             /**< Vport hdl */
    HI_HANDLE VencHdl;              /**< Venc hdl */
    HI_U8 u8PipeParamIndex;         /**<The index means this pipe param we set , is from the param array we put to the module by bin. the typeindex is the arrayIndex */
    HI_PDT_SCENE_PIPE_TYPE_E enPipeType;/**<pipe type means the pipe used to snap or video, 0 is snap 1 is video. 3DNR and DIS need*/
    HI_PDT_SCENE_PIPE_MODE_E enPipeMode;
}HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S;


/**------------------- Media  ------------------------- */

typedef struct hiPDT_PARAM_MEDIAMODE_CFG_S
{
    HI_S32 s32CamID;                                  /**<cam id, 0<=s32CamID<HI_PDT_MEDIA_VCAP_DEV_MAX_CNT:  valid, other value: invalid*/
    HI_PDT_MEDIAMODE_E                   enMediaMode;   /**<current s32CamID specified cam use mediamode*/
}HI_PDT_PARAM_MEDIAMODE_CFG_S;

typedef struct hiPDT_PARAM_CAM_MEDIAMODE_CFG_S
{
    HI_PDT_PARAM_MEDIAMODE_CFG_S astMediaModeCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
}HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S;

typedef struct hiPDT_PARAM_CAM_CONTEXT_S
{
    HI_BOOL bEnable;
    HI_PDT_PARAM_MEDIAMODE_CFG_S stMediaModeCfg;
}HI_PDT_PARAM_CAM_CONTEXT_S;


/**------------------- FileMng  ------------------------- */

/** FileMng Configure */
typedef struct hiPDT_FILEMNG_CFG_S
{
    HI_FILEMNG_COMM_CFG_S stCommCfg;
    HI_FILEMNG_DTCF_CFG_S  stDtcfCfg;
} HI_PDT_FILEMNG_CFG_S;

/**------------------- Record ------------------------- */

/** Photo Data Source */
typedef struct hiPDT_PARAM_PHOTO_SRC_S
{
    HI_BOOL   bEnable;
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE VcapPipeChnHdl;
    HI_HANDLE VpssHdl; /**<vpss or stitch */
    HI_HANDLE VportHdl;
    HI_HANDLE VencHdl;
} HI_PDT_PARAM_PHOTO_SRC_S;

/** Record Common Attribute */
typedef struct hiPDT_PARAM_REC_COMM_ATTR_S
{
    HI_BOOL  bEnable;                           /**record task is enable, HI_TRUE: enable,  HI_FALSE: DISABLE*/
    HI_REC_SPLIT_ATTR_S stSplitAttr;            /**<split attribute, include split type and parameter */
    HI_U32              u32FileCnt;
    HI_RECMNG_FILE_ATTR_S  astFileAttr[HI_PDT_RECTASK_FILE_MAX_CNT];
    HI_RECMNG_THM_TYPE_E        enRecThmType;
    HI_RECMNG_THM_ATTR_S        stThmAttr;
    HI_U32                      u32BufferTime_ms; /**<record buffer size in mili second */
    HI_U32                      u32PreRecTimeSec; /**<pre-record time in second */
    HI_REC_TYPE_E               enRecType;
    HI_U32                      u32Interval_ms;
} HI_PDT_PARAM_RECMNG_ATTR_S;

/** Normal Record Attribute */
typedef struct hiPDT_NORM_REC_ATTR_S
{
    HI_PDT_PARAM_PHOTO_SRC_S    stPhotoSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_RECMNG_ATTR_S  astRecMngAttr[HI_PDT_RECTASK_MAX_CNT];
} HI_PDT_NORM_REC_ATTR_S;


/**------------------- Video Detect ------------------------- */

/** Video Detect Attribute */
#ifdef CONFIG_MOTIONDETECT_ON
typedef struct hiPDT_PARAM_ALG_ATTR_S
{
    HI_VIDEODETECT_ALG_TYPE_E algType;
    HI_VIDEODETECT_ALGPROC_MD_S algProcMdAttr;
    MD_ATTR_S mdAttr;
} HI_PDT_PARAM_ALG_ATTR_S;

typedef struct hiPDT_PARAM_VIDEODETECT_CFG_S
{
    HI_BOOL bEnable;
    HI_U32 algCnt;
    HI_VIDEODETECT_YUV_SRC_S yuvSrc;
    HI_PDT_PARAM_ALG_ATTR_S algAttr[HI_VIDEODETECT_ALG_TYPE_BUTT];
}HI_PDT_PARAM_VIDEODETECT_CFG_S;
#endif


/**------------------- Playback ------------------------- */

/** Playback Attribute */
typedef struct hiPDT_PARAM_PLAYBACK_CFG_S
{
    HI_PDT_PARAM_MEDIAMODE_CFG_S astMediaModeCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_LITEPLAYER_PARAM_S stPlayerParam;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
} HI_PDT_PARAM_PLAYBACK_CFG_S;

/**------------------- USB ------------------------- */

typedef struct hiPDT_PARAM_UVC_CFG_S
{
    HI_UVC_CFG_S stUvcCfg;
}HI_PDT_PARAM_UVC_CFG_S;


/**------------------- Photo ------------------------- */

typedef struct hiPDT_PARA_PHOTO_ATTR_S
{
    HI_PDT_PARAM_PHOTO_SRC_S stPhotoSrc[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} HI_PDT_PARAM_PHOTO_CFG_S;


/**-------------------  WorkMode  ------------------------- */

/** WorkMode Attribute */
typedef union hiPDT_WORKMODE_CFG_U
{
    HI_PDT_NORM_REC_ATTR_S       stNormRecAttr;
    HI_PDT_PARAM_PHOTO_CFG_S     stPhotoCfg;
    HI_PDT_PARAM_PLAYBACK_CFG_S  stPlayBackCfg;
    HI_PDT_PARAM_UVC_CFG_S       stUvcCfg;
    HI_USB_STORAGE_CFG_S         stUsbStorageCfg;
} HI_PDT_WORKMODE_CFG_U;

/** WorkMode Configure */
typedef struct hiPDT_WORKMODE_CFG_S
{
    HI_PDT_WORKMODE_E     enWorkMode;
    HI_PDT_WORKMODE_CFG_U unModeCfg;
} HI_PDT_WORKMODE_CFG_S;


/**-------------------  DevMng  ------------------------- */

/** Device Information */
typedef struct hiPDT_DEV_INFO_S
{
    HI_CHAR szSysVersion[HI_APPCOMM_COMM_STR_LEN]; /**<linux kernel version */
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN]; /**<software version */
    HI_CHAR szModel[HI_APPCOMM_COMM_STR_LEN]; /**<product model */
} HI_PDT_DEV_INFO_S;

/** gsensor config */
typedef struct hiPDT_GSENSOR_CFG_S
{
    HI_HAL_GSENSOR_VALUE_S stThreshold;
    HI_HAL_GSENSOR_ATTR_S stAttr;
}HI_PDT_GSENSOR_CFG_S;

/** wifi enable and wifi mode */
typedef struct hiPDT_WIFI_INFO_S
{
    HI_BOOL bEnable;
    HI_HAL_WIFI_MODE_E enMode;
}HI_PDT_WIFI_INFO_S;


/**-------------------  ParamItem ValueSet  ------------------------- */

/**  item value description structure */
typedef struct hiPDT_ITEM_VALUE_S
{
    HI_CHAR szDesc[HI_PDT_ITEM_VALUE_DESCRIPTION_LEN]; /**<item value description in string */
    HI_S32  s32Value; /**<item value */
} HI_PDT_ITEM_VALUE_S;

/** effective item value set */
typedef struct hiPDT_ITEM_VALUESET_S
{
    HI_S32              s32Cnt; /**<value count in set */
    HI_PDT_ITEM_VALUE_S astValues[HI_PDT_ITEM_VALUESET_MAX_NUM]; /**<effective item value set */
} HI_PDT_ITEM_VALUESET_S;


/** @}*/  /** <!-- ==== PARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_PARAM_DEFINE_H__ */

