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
#include "hi_timedtask.h"
#include "hi_product_scene.h"
#include "hi_product_media_define.h"
#include "hi_photomng.h"
#include "hi_recordmng.h"
#if defined(CONFIG_FILEMNG_DTCF)
#include "hi_filemng_dtcf.h"
#else
#include "hi_filemng_dcf.h"
#endif
#include "hi_liteplayer.h"
#include "hi_usb.h"
#include "hi_keymng.h"
#include "hi_gaugemng.h"
#include "hi_storagemng.h"
#include "hi_hal_wifi.h"
#include "hi_upgrade_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     PARAM */
/** @{ */  /** <!-- [PARAM] */

/** record media stream maximum number */
#define HI_PDT_REC_VSTREAM_MAX_CNT (2)

/** item value description maximum length */
#define HI_PDT_ITEM_VALUE_DESCRIPTION_LEN  (12)

/** item value set maximum number */
#define HI_PDT_ITEM_VALUESET_MAX_NUM       (10)


/** WorkMode Enum */
typedef enum hiPDT_WORKMODE_E
{
    HI_PDT_WORKMODE_NORM_REC = 0,
    HI_PDT_WORKMODE_LOOP_REC,
    HI_PDT_WORKMODE_LPSE_REC,
    HI_PDT_WORKMODE_SLOW_REC,
    HI_PDT_WORKMODE_SING_PHOTO,
    HI_PDT_WORKMODE_DLAY_PHOTO,
    HI_PDT_WORKMODE_LPSE_PHOTO,
    HI_PDT_WORKMODE_BURST,
    HI_PDT_WORKMODE_RECSNAP,
    HI_PDT_WORKMODE_PLAYBACK,
    HI_PDT_WORKMODE_UVC,
    HI_PDT_WORKMODE_USB_STORAGE,
    HI_PDT_WORKMODE_SUSPEND,
    HI_PDT_WORKMODE_HDMI_PREVIEW,
    HI_PDT_WORKMODE_HDMI_PLAYBACK,
    HI_PDT_WORKMODE_UPGRADE,
    HI_PDT_WORKMODE_BUTT
} HI_PDT_WORKMODE_E;

/** MediaMode Enum
      Record/Record+Photo: Resolution_FrameRate_Mode
      Photo/MPhoto: Resolution_Mode */
typedef enum hiPDT_MEDIAMODE_E
{
    HI_PDT_MEDIAMODE_PLAYBACK = 0,
    HI_PDT_MEDIAMODE_HDMIPREVIEW,
    HI_PDT_MEDIAMODE_HDMIPLAYBACK,
    HI_PDT_MEDIAMODE_720P_30_REC,
    HI_PDT_MEDIAMODE_720P_60_REC,
    HI_PDT_MEDIAMODE_720P_120_REC,
    HI_PDT_MEDIAMODE_720P_240_REC,
    HI_PDT_MEDIAMODE_1080P_30_REC,
    HI_PDT_MEDIAMODE_1080P_60_REC,
    HI_PDT_MEDIAMODE_1080P_60_RECSNAP,
    HI_PDT_MEDIAMODE_1080P_90_REC,
    HI_PDT_MEDIAMODE_1080P_120_REC,
    HI_PDT_MEDIAMODE_1080P_240_REC,
    HI_PDT_MEDIAMODE_1440P_30_REC,
    HI_PDT_MEDIAMODE_1440P_30_RECSNAP,
    HI_PDT_MEDIAMODE_1440P_60_REC,
    HI_PDT_MEDIAMODE_1440P_60_RECSNAP,
    HI_PDT_MEDIAMODE_4K2K_25_REC,
    HI_PDT_MEDIAMODE_4K2K_30_REC,
    HI_PDT_MEDIAMODE_4K2K_25_RECSNAP,
    HI_PDT_MEDIAMODE_4K2K_30_RECSNAP,
    HI_PDT_MEDIAMODE_4K2K_60_REC,
    HI_PDT_MEDIAMODE_4K2K_60_RECSNAP,
    HI_PDT_MEDIAMODE_4K2K_120_REC,
    HI_PDT_MEDIAMODE_4K2K_120_RECSNAP,
    HI_PDT_MEDIAMODE_4K2K_PHOTO,
    HI_PDT_MEDIAMODE_4K2K_MPHOTO,
    HI_PDT_MEDIAMODE_12M_PHOTO,
    HI_PDT_MEDIAMODE_12M_MPHOTO,
    HI_PDT_MEDIAMODE_16M_PHOTO,
    HI_PDT_MEDIAMODE_16M_MPHOTO,
    HI_PDT_MEDIAMODE_BUTT,
} HI_PDT_MEDIAMODE_E;

/** Param Type Enum */
typedef enum hiPDT_PARAM_TYPE_E
{
    /** WorkMode Dependent Param */
    HI_PDT_PARAM_TYPE_MEDIAMODE = 0,       /**<Rec/Photo/MPhoto/RecSnap/HDMIPreview, HI_PDT_MEDIAMODE_E */
    HI_PDT_PARAM_TYPE_PHOTO_SCENE,         /**<Single/Delay HI_PDT_PHOTO_SCENE_E */
    HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT,    /**<Single/Delay HI_PHOTOMNG_OUTPUT_FORMAT_E */
    HI_PDT_PARAM_TYPE_DELAY_TIME,          /**<Delay HI_U32 */
    HI_PDT_PARAM_TYPE_LAPSE_INTERVAL,      /**<LapseRec/LapsePhoto/RecSnap HI_U32 */
    HI_PDT_PARAM_TYPE_BURST_TYPE,          /**<Burst HI_PDT_JPG_BURST_TYPE_E */
    HI_PDT_PARAM_TYPE_LOOP_TIME,           /**<LoopRec HI_U32 */
    HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE,    /**<Rec/RecSnap MainRecVenc HI_MAPI_PAYLOAD_TYPE_E */
    HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV,      /**<Rec/Photo/MPhoto/RecSnap HI_SCENE_EV_E */
    HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME,    /**<Photo HI_U32 */
    HI_PDT_PARAM_TYPE_PROTUNE_ISO,         /**<Rec/Photo/MPhoto/RecSnap HI_U32 */
    HI_PDT_PARAM_TYPE_PROTUNE_WB,          /**<Rec/Photo/MPhoto/RecSnap HI_U32 */
    HI_PDT_PARAM_TYPE_PROTUNE_METRY,       /**<Rec/Photo/MPhoto/RecSnap HI_SCENE_METRY_TYPE_E */
#ifdef CONFIG_MOTIONSENSOR
    HI_PDT_PARAM_TYPE_MOTIONSENSOR_INFO,   /**<Rec/Photo/MPhoto/RecSnap HI_SCENE_METRY_TYPE_E */
#endif

    HI_PDT_PARAM_TYPE_OSD,                 /**<All HI_BOOL */
    HI_PDT_PARAM_TYPE_AUDIO,               /**<All HI_BOOL */
    HI_PDT_PARAM_TYPE_FLIP,                /**<All HI_BOOL */
    HI_PDT_PARAM_TYPE_DIS,                 /**<All HI_BOOL */
    HI_PDT_PARAM_TYPE_LDC,                 /**<All HI_BOOL */

    /* Customed Workmode Dependent Param */

    /** WorkMode Independent Param */
    HI_PDT_PARAM_TYPE_POWERON_WORKMODE,    /**<HI_PDT_WORKMODE_E */
    HI_PDT_PARAM_TYPE_POWERON_ACTION,      /**<HI_PDT_POWERON_ACTION_E */
    HI_PDT_PARAM_TYPE_WIFI_AP,             /**<HI_HAL_WIFI_APMODE_CFG_S */
    HI_PDT_PARAM_TYPE_DEV_INFO,            /**<HI_UPGRADE_DEV_INFO_S */
    HI_PDT_PARAM_TYPE_VOLUME,              /**<HI_S32 */
    HI_PDT_PARAM_TYPE_KEYTONE,             /**<HI_S32 */
    HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS,   /**<HI_S32 */
    HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE, /**<HI_TIMEDTASK_ATTR_S */
    HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE,    /**<HI_TIMEDTASK_ATTR_S */
    HI_PDT_PARAM_TYPE_BOOTSOUND,           /**<Boot Sound*/
    HI_PDT_PARAM_TYPE_USB_MODE,
    HI_PDT_PARAM_TYPE_LANGUAGE,

    /* Customed Workmode Independent Param */

    HI_PDT_PARAM_TYPE_BUTT
} HI_PDT_PARAM_TYPE_E;


/**------------------- Media  ------------------------- */

/** Video Professional Tune Attribute */
typedef struct hiPDT_VIDEO_PROTUNE_ATTR_S
{
    HI_U32                    u32WB; /**<white balance, 0:auto, >0 manual, eg. 3000K */
    HI_U32                    u32ISO; /**<0:auto, >0 manual, eg. 100,200 */
    HI_PDT_SCENE_EV_E         enEV;
    HI_PDT_SCENE_METRY_TYPE_E enMetryType;
    HI_U16                    u16MetryAverageParamIdx;
    HI_U16                    u16MetryCenterParamIdx;
} HI_PDT_VIDEO_PROTUNE_ATTR_S;

/** Photo Professional Tune Attribute */
typedef struct hiPDT_PHOTO_PROTUNE_ATTR_S
{
    HI_U32                    u32WB; /**<white balance, 0:auto; >0 manual, eg. 3000K */
    HI_U32                    u32ISO; /**<0:auto; >0 manual, eg. 100,200 */
    HI_PDT_SCENE_EV_E         enEV;
    HI_U32                    u32ExpTime_us; /**<exposure time, 0:auto; >0 manual, eg. 500000us */
    HI_PDT_SCENE_METRY_TYPE_E enMetryType;
    HI_U16                    u16MetryAverageParamIdx;
    HI_U16                    u16MetryCenterParamIdx;
} HI_PDT_PHOTO_PROTUNE_ATTR_S;

/** Media VcapPipeChn Capability: LDC/DIS */
typedef struct hPDT_PARAM_MEDIA_VCAP_PIPE_CHN_CAPABILITY_S
{
    HI_BOOL bSupportLDC;
    HI_BOOL bSupportDIS;
} HI_PDT_PARAM_MEDIA_VCAP_PIPE_CHN_CAPABILITY_S;

/** Media VcapPipe Capability */
typedef struct hiPDT_PARAM_MEDIA_VCAP_PIPE_CAPABILITY_S
{
    HI_PDT_PARAM_MEDIA_VCAP_PIPE_CHN_CAPABILITY_S astPipeChn[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} HI_PDT_PARAM_MEDIA_VCAP_PIPE_CAPABILITY_S;

/** Media Vcap Device Capability */
typedef struct hiPDT_PARAM_MEDIA_VCAP_DEV_CAPABILITY_S
{
    HI_PDT_PARAM_MEDIA_VCAP_PIPE_CAPABILITY_S astVcapPipe[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} HI_PDT_PARAM_MEDIA_VCAP_DEV_CAPABILITY_S;

/** Media Capability: LDC/DIS */
typedef struct hiPDT_PARAM_MEDIA_CAPABILITY_S
{
    HI_PDT_PARAM_MEDIA_VCAP_DEV_CAPABILITY_S astVcapDev[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
} HI_PDT_PARAM_MEDIA_CAPABILITY_S;

/**------------------- Scene  ------------------------- */

/** Scene Type Enum */
typedef enum hiPDT_PARAM_SCENE_TYPE_E
{
    HI_PDT_PARAM_SCENE_TYPE_LINEAR = 0, /**<Linear Scene, Photo/Video */
    HI_PDT_PARAM_SCENE_TYPE_HDR,        /**<Photo HDR Only */
    HI_PDT_PARAM_SCENE_TYPE_MFNR,       /**<Photo MFNR Only */
    HI_PDT_PARAM_SCENE_TYPE_BUTT
} HI_PDT_PARAM_SCENE_TYPE_E;

/** Scene Configure */
typedef struct hiPDT_PARAM_SCENE_CFG_S
{
    HI_U8 au8SceneParamIdx[HI_PDT_SCENE_PIPE_MAX_NUM];
} HI_PDT_PARAM_SCENE_CFG_S;

/**------------------- FileMng  ------------------------- */

/** FileMng Configure */
typedef struct hiPDT_FILEMNG_CFG_S
{
    HI_FILEMNG_COMM_CFG_S stCommCfg;
#if defined(CONFIG_FILEMNG_DTCF)
    HI_FILEMNG_DTCF_CFG_S stDtcfCfg;
#else
    HI_FILEMNG_DCF_CFG_S  stDcfCfg;
#endif
} HI_PDT_FILEMNG_CFG_S;

/**------------------- Record ------------------------- */

/** Record Data Source */
typedef struct hiPDT_REC_SRC_S
{
    HI_HANDLE aVencHdl[HI_PDT_REC_VSTREAM_MAX_CNT]; /**<video encoder handle */
    HI_HANDLE AencHdl; /**<audio encoder handle */
    HI_HANDLE ThmHdl;  /**<thumbnail encoder handle */
} HI_PDT_REC_SRC_S;

/** Record Common Attribute */
typedef struct hiPDT_REC_COMM_ATTR_S
{
    HI_PDT_MEDIAMODE_E     enMediaMode;
    HI_REC_SPLIT_ATTR_S    stSplitAttr;      /**<split attribute, include split type and parameter */
    HI_U32                 u32BufferTimeMSec; /**<record buffer size in millisecond */
    HI_U32                 u32PreRecTimeSec; /**<pre-record time in second */
    HI_RECMNG_MUXER_TYPE_E enMuxerType;
    HI_RECMNG_THM_TYPE_E   enThmType;
    HI_U32                 u32RepairUnit;    /**<repair data unit in bytes, [10M,500M]
                                                 0: not support repair */
    HI_U32                 u32PreAllocUnit;  /**<pre allocate size in bytes, [0,100M]
                                                     0 for not use pre allocate function*/
    HI_U32                 au32VBufSize[HI_PDT_REC_VSTREAM_MAX_CNT]; /**<set the vbuf size for fwrite (0,5M] unit :byte*/
    HI_U8                  u8FileTypeIdx;
    HI_U8                  au8Reserved[3];
} HI_PDT_REC_COMM_ATTR_S;

/** Normal Record Attribute */
typedef struct hiPDT_NORM_REC_ATTR_S
{
    HI_PDT_REC_SRC_S            stDataSrc;
    HI_PDT_REC_COMM_ATTR_S      stCommAttr;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_NORM_REC_ATTR_S;

/** Loop Record Attribute */
typedef struct hiPDT_LOOP_REC_ATTR_S
{
    HI_PDT_REC_SRC_S            stDataSrc;
    HI_PDT_REC_COMM_ATTR_S      stCommAttr;
    HI_U32                      u32LoopTime_min;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_LOOP_REC_ATTR_S;

/** Slow Record Attribute */
typedef struct hiPDT_SLOW_REC_ATTR_S
{
    HI_PDT_REC_SRC_S            stDataSrc;
    HI_PDT_REC_COMM_ATTR_S      stCommAttr;
    HI_U32                      au32PlayFrmRate[HI_PDT_REC_VSTREAM_MAX_CNT];
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_SLOW_REC_ATTR_S;

/** Lapse Record Attribute */
typedef struct hiPDT_LAPSE_REC_ATTR_S
{
    HI_PDT_REC_SRC_S            stDataSrc;
    HI_PDT_REC_COMM_ATTR_S      stCommAttr;
    HI_U32                      u32Interval_ms;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_LAPSE_REC_ATTR_S;

/**------------------- RecSnap ------------------------- */

/** Record+Snap Data Source */
typedef struct hiPDT_RECSNAP_SRC_S
{
    HI_PDT_REC_SRC_S        stRecSrc;
    HI_PHOTOMNG_PHOTO_SRC_S stPhotoSrc;
} HI_PDT_RECSNAP_SRC_S;

/** Record+Photo Attribute */
typedef struct hiPDT_RECSNAP_ATTR_S
{
    HI_PDT_RECSNAP_SRC_S        stDataSrc;
    HI_PDT_REC_COMM_ATTR_S      stRecAttr;
    HI_U32                      u32SnapFileTypeIndex;
    HI_U32                      u32SnapInterval_ms;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_RECSNAP_ATTR_S;


/**------------------- Photo ------------------------- */

/** Photo Common Attribute */
typedef struct hiPDT_PHOTO_COMM_ATTR_S
{
    HI_PDT_MEDIAMODE_E enMediaMode;
    HI_U8              u8JpgFileTypeIdx;
    HI_U8              u8DngFileTypeIdx;
    HI_U8              u8JpgDngFileTypeIdx;
    HI_U8              u8Reserved;
} HI_PDT_PHOTO_COMM_ATTR_S;

/** Photo Scene */
typedef enum hiPDT_PHOTO_SCENE_E
{
    HI_PDT_PHOTO_SCENE_NORM = 0,
    HI_PDT_PHOTO_SCENE_HDR,
    HI_PDT_PHOTO_SCENE_LL,
    HI_PDT_PHOTO_SCENE_BUTT
} HI_PDT_PHOTO_SCENE_E;

/** Photo Process Alg */
typedef struct hiPDT_PHOTO_PROCALG_S
{
    //HI_PHOTOMNG_PROCALG_E enProcAlg;
    //HI_PHOTO_HDR_ATTR_S   stHdrAttr;
    //HI_PHOTO_LL_ATTR_S    stLLAttr;
} HI_PDT_PHOTO_PROCALG_S;

/** JPG Burst Type */
typedef enum hiPDT_JPG_BURST_TYPE_E
{
    HI_PDT_JPG_BURST_TYPE_3_1 = 0, /**<3 pictures in 1 second */
    HI_PDT_JPG_BURST_TYPE_5_1,
    HI_PDT_JPG_BURST_TYPE_10_1,
    HI_PDT_JPG_BURST_TYPE_15_1,
    HI_PDT_JPG_BURST_TYPE_30_1,
    HI_PDT_JPG_BURST_TYPE_20_2,
    HI_PDT_JPG_BURST_TYPE_30_2,
    HI_PDT_JPG_BURST_TYPE_30_3,
    HI_PDT_JPG_BURST_TYPE_BUTT
} HI_PDT_JPG_BURST_TYPE_E;

/** Single Photo Attribute */
typedef struct hiPDT_SINGLE_PHOTO_ATTR_S
{
    HI_PHOTOMNG_PHOTO_SRC_S     stDataSrc;
    HI_PDT_PHOTO_COMM_ATTR_S    stCommAttr;
    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutputFmt;
    HI_PDT_PHOTO_SCENE_E        enScene;
    HI_PDT_PHOTO_PROCALG_S      stProcAlg;
    HI_PDT_PHOTO_PROTUNE_ATTR_S stProTune;
} HI_PDT_SINGLE_PHOTO_ATTR_S;

/** Delay Photo Attribute */
typedef struct hiPDT_DELAY_PHOTO_ATTR_S
{
    HI_PHOTOMNG_PHOTO_SRC_S     stDataSrc;
    HI_PDT_PHOTO_COMM_ATTR_S    stCommAttr;
    HI_PHOTOMNG_OUTPUT_FORMAT_E enOutputFmt;
    HI_PDT_PHOTO_SCENE_E        enScene;
    HI_PDT_PHOTO_PROCALG_S      stProcAlg;
    HI_U32                      u32DelayTime_s;
    HI_PDT_PHOTO_PROTUNE_ATTR_S stProTune;
} HI_PDT_DELAY_PHOTO_ATTR_S;

/** Lapse Photo Attribute */
typedef struct hiPDT_LAPSE_PHOTO_ATTR_S
{
    HI_PHOTOMNG_PHOTO_SRC_S     stDataSrc;
    HI_PDT_PHOTO_COMM_ATTR_S    stCommAttr;
    HI_U32                      u32Interval_ms;
    HI_PDT_PHOTO_PROTUNE_ATTR_S stProTune;
} HI_PDT_LAPSE_PHOTO_ATTR_S;

/** Burst Photo Attribute */
typedef struct hiPDT_BURST_ATTR_S
{
    HI_PHOTOMNG_PHOTO_SRC_S     stDataSrc;
    HI_PDT_PHOTO_COMM_ATTR_S    stCommAttr;
    HI_PDT_JPG_BURST_TYPE_E     enJpgBurstType;
    HI_PDT_VIDEO_PROTUNE_ATTR_S stProTune;
} HI_PDT_BURST_ATTR_S;

/**------------------- USB ------------------------- */

/** UVC Attribute */
typedef struct hiPDT_PARAM_UVC_ATTR_S
{
    HI_PDT_MEDIAMODE_E enMediaMode;
    HI_UVC_CFG_S       stUvcCfg;
} HI_PDT_PARAM_UVC_ATTR_S;

/**------------------- Playback ------------------------- */

/** Playback Attribute */
typedef struct hiPDT_PARAM_PLAYBACK_ATTR_S
{
    HI_PDT_MEDIAMODE_E enMediaMode;
    HI_LITEPLAYER_PARAM_S    stPlayerParam;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
} HI_PDT_PARAM_PLAYBACK_ATTR_S;

/**------------------- HDMI ------------------------- */

/** HDMI Preview Attribute */
typedef struct hiPDT_PARAM_HDMI_PREVIEW_ATTR_S
{
    HI_PDT_MEDIAMODE_E enMediaMode;
} HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S;

/** HDMI Playback Attribute */
typedef struct hiPDT_PARAM_HDMI_PLAYBACK_ATTR_S
{
    HI_PDT_MEDIAMODE_E enMediaMode;
    HI_LITEPLAYER_PARAM_S    stPlayerParam;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
} HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S;

/**-------------------  WorkMode  ------------------------- */

/** PowerOn Action */
typedef enum hiPDT_POWERON_ACTION_E
{
    HI_PDT_POWERON_ACTION_IDLE = 0,
    HI_PDT_POWERON_ACTION_NORM_REC,
    HI_PDT_POWERON_ACTION_LOOP_REC,
    HI_PDT_POWERON_ACTION_LPSE_REC,
    HI_PDT_POWERON_ACTION_SLOW_REC,
    HI_PDT_POWERON_ACTION_RECSNAP,
    HI_PDT_POWERON_ACTION_LPSE_PHOTO,
    HI_PDT_POWERON_ACTION_SING_PHOTO,
    HI_PDT_POWERON_ACTION_BUTT
} HI_PDT_POWERON_ACTION_E;

/** WorkMode Attribute */
typedef union hiPDT_WORKMODE_ATTR_U
{
    HI_PDT_NORM_REC_ATTR_S     stNormRecAttr;
    HI_PDT_LOOP_REC_ATTR_S     stLoopRecAttr;
    HI_PDT_SLOW_REC_ATTR_S     stSlowRecAttr;
    HI_PDT_LAPSE_REC_ATTR_S    stLapseRecAttr;
    HI_PDT_SINGLE_PHOTO_ATTR_S stSinglePhotoAttr;
    HI_PDT_DELAY_PHOTO_ATTR_S  stDelayPhotoAttr;
    HI_PDT_LAPSE_PHOTO_ATTR_S  stLapsePhotoAttr;
    HI_PDT_BURST_ATTR_S        stBurstAttr;
    HI_PDT_RECSNAP_ATTR_S      stRecSnapAttr;
    HI_PDT_PARAM_UVC_ATTR_S    stUvcAttr;
    HI_USB_STORAGE_CFG_S       stUsbStorageCfg;
    HI_PDT_PARAM_PLAYBACK_ATTR_S      stPlaybackAttr;
    HI_PDT_PARAM_HDMI_PREVIEW_ATTR_S  stHDMIPreviewAttr;
    HI_PDT_PARAM_HDMI_PLAYBACK_ATTR_S stHDMIPlaybackAttr;
} HI_PDT_WORKMODE_ATTR_U;

/** WorkMode Configure */
typedef struct hiPDT_WORKMODE_CFG_S
{
    HI_PDT_WORKMODE_E      enWorkMode;
    HI_PDT_WORKMODE_ATTR_U unModeAttr;
} HI_PDT_WORKMODE_CFG_S;


/**-------------------  DevMng  ------------------------- */

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

