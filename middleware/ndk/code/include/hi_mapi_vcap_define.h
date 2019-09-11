/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_vcap_define.h
 * @brief   vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_VCAP_DEFINE_H__
#define __HI_MAPI_VCAP_DEFINE_H__

#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_log.h"
#include "mpi_isp.h"
#include "mpi_vi.h"
#include "mpi_ae.h"
#include "mpi_snap.h"
#include "hi_sns_ctrl.h"
#ifdef SUPPORT_GYRO
#include "hi_comm_motionfusion.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     VCAP */
/** @{ */ /** <!-- [VCAP] */


/* Sharpen intensity curve number of segments */
#define HI_MAPI_ISP_SHARPEN_GAIN_NUM (32)

/* Max length of exif description */
#define HI_MAPI_EXIF_DRSCRIPTION_LENGTH (32)

/* Max and min wb value */
#define HI_MAPI_VCAP_MAX_COLORTEMP (15000)
#define HI_MAPI_VCAP_MIN_COLORTEMP (1500)

/* Max and min sysgain value */
#define HI_MAPI_VCAP_SYSGAIN_MIN (0x400)
#define HI_MAPI_VCAP_SYSGAIN_MAX (0xFFFFFFFF)

/**The max number of groups that can be dump at one time */
#define HI_VCAP_RAWGROUP_MAX_NUM (8)
/**the max number of RAW frames per group */
#define HI_VCAP_RAW_EACHGROUP_MAX_NUM (4)
/**the min number of RAW frames per group */
#define HI_VCAP_RAW_EACHGROUP_MIN_NUM (1)
/* LDC */
#define HI_MAPI_VCAP_MAX_LDC_CENTERX_OFFSET   (511)
#define HI_MAPI_VCAP_MIN_LDC_CENTERX_OFFSET   (-511)
#define HI_MAPI_VCAP_MAX_LDC_CENTERY_OFFSET   (511)
#define HI_MAPI_VCAP_MIN_LDC_CENTERY_OFFSET   (-511)
#define HI_MAPI_VCAP_MAX_LDC_RATIO_X          (100)
#define HI_MAPI_VCAP_MIN_LDC_RATIO_X          (0)
#define HI_MAPI_VCAP_MAX_LDC_RATIO_Y          (100)
#define HI_MAPI_VCAP_MIN_LDC_RATIO_Y          (0)
#define HI_MAPI_VCAP_MAX_LDC_RATIO_XY         (100)
#define HI_MAPI_VCAP_MIN_LDC_RATIO_XY         (0)
#define HI_MAPI_VCAP_MAX_LDC_RATIO_DISTORTION (500)
#define HI_MAPI_VCAP_MIN_LDC_RATIO_DISTORTION (-300)
#define HI_MAPI_VCAP_MIN_SATURATION           (0)
#define HI_MAPI_VCAP_MAX_SATURATION           (100)
#define HI_MAPI_VCAP_MIN_LUMA                 (0)
#define HI_MAPI_VCAP_MAX_LUMA                 (100)
#define HI_MAPI_VCAP_AE_ZONE_ROW              (15)
#define HI_MAPI_VCAP_AE_ZONE_COLUMN           (17)

typedef enum hiMAPI_SENSOR_SCENE_E {
    HI_MAPI_SENSOR_SCENE_NORMAL,
    HI_MAPI_SENSOR_SCENE_STANDBY,
    HI_MAPI_SENSOR_SCENE_BUT
} HI_MAPI_SENSOR_SCENE_E;

/* VCAP DUMPRaw info */
typedef struct hiMAPI_DUMP_ATTR_S {
    PIXEL_FORMAT_E enPixFmt;
    VI_DUMP_ATTR_S stDumpAttr;
    BNR_DUMP_ATTR_S stDumpBNRAttr;
} HI_MAPI_DUMP_ATTR_S;

/* VCAP PIPE type  */
typedef enum hiMAPI_PIPE_TYPE_E {
    HI_MAPI_PIPE_TYPE_VIDEO,
    HI_MAPI_PIPE_TYPE_SNAP,
    HI_MAPI_PIPE_TYPE_BUTT
} HI_MAPI_PIPE_TYPE_E;

/* Aperture value */

typedef struct hiMAPI_APERTURE_INFO_S {
    ISP_IRIS_F_NO_E
    enMaxIrisFNOTarget; /**<RW, Range:[F32.0, F1.0], Max F number of Piris's aperture, it's related to the specific iris */
    ISP_IRIS_F_NO_E
    enMinIrisFNOTarget; /**<RW, Range:[F32.0, F1.0], Min F number of Piris's aperture, it's related to the specific iris */
    HI_BOOL bFNOExValid;                /**<RW, use equivalent gain to present FNO or not */
    HI_U32 u32MaxIrisFNOTarget;         /**<RW, Range:[1, 1024], Max equivalent gain of F number of Piris's aperture, only used
                                                                       when bFNOExValid is true, it's related to the specific iris */
    HI_U32 u32MinIrisFNOTarget;         /**<RW, Range:[1, 1024], Min equivalent gain of F number of Piris's aperture, only used
                                                                       when bFNOExValid is true, it's related to the specific iris */
} HI_MAPI_APERTURE_INFO_S;

typedef struct hiMAPI_SNAP_EXIF_INFO_S {
    HI_U8 au8ImageDescription[HI_MAPI_EXIF_DRSCRIPTION_LENGTH]; /**<Describes image */
    HI_U8 au8Make[HI_MAPI_EXIF_DRSCRIPTION_LENGTH];             /**<Shows manufacturer of digital cameras */
    HI_U8 au8Model[HI_MAPI_EXIF_DRSCRIPTION_LENGTH];            /**<Shows model number of digital cameras */
    HI_U8 au8Software[HI_MAPI_EXIF_DRSCRIPTION_LENGTH];         /**<Shows firmware (internal software of digital cameras)
                                                                                                            version number */
    HI_U32 u32MeteringMode;
    HI_U32 u32LightSource;
    HI_U32 u32FocalLength;
    HI_U8 u8FocalLengthIn35mmFilm;
    GPS_INFO_S stGPSInfo;
    HI_MAPI_APERTURE_INFO_S stApertureInfo; /**<set or get Aperture info */
} HI_MAPI_SNAP_EXIF_INFO_S;

typedef struct hiMAPI_MOTIONSENSOR_INFO_S {
#ifdef SUPPORT_GYRO
    HI_U32 u32GyroFSR;
    HI_U32 u32AccFSR;
    IMU_DRIFT aGyroDrift;
    IMU_MATRIX aRotationMatrix;
    MFUSION_ATTR_S stMFusionAttr;
    MFUSION_TEMP_DRIFT_S stTempDrift;
#endif
} HI_MAPI_MOTIONSENSOR_INFO_S;

/* VCAP DIS info */
typedef struct hiMAPI_VCAP_DIS_ATTR_S {
    DIS_ATTR_S stDisAttr;
    VI_LDCV2_ATTR_S stLDCV2Attr;
    DIS_CONFIG_S stDISConfig;
    HI_MAPI_MOTIONSENSOR_INFO_S stMotionSensorInfo;
} HI_MAPI_VCAP_DIS_ATTR_S;

typedef struct hiMAPI_VCAP_DEV_ATTR_S {
    SIZE_S stBasSize;     /**<CAP dev0 expected scale resolution, 1/2, 1/3, only when PIPE0 binding Dev0 BAS to take effect,ISP's
                                                width and height needs to be set to the BAS output width and height */
    WDR_MODE_E enWdrMode; /**<Only supports 2to1 WDR, WDR mode does not support BAS */
    HI_U32 u32CacheLine;  /**<the cached line number of data in offline pipe situation */
} HI_MAPI_VCAP_DEV_ATTR_S;

typedef struct hiMAPI_PIPE_CHN_ATTR_S {
    SIZE_S stDestSize;              /**<dynamic attribute,channel Output resolution setting, can not be dynamically modified
                                                                when using GDC */
    FRAME_RATE_CTRL_S stFrameRate;  /**<dynamic attribute,channel Frame rate control */
    COMPRESS_MODE_E enCompressMode; /**<dynamic attribute,channel Compression format */
    PIXEL_FORMAT_E enPixelFormat;   /**<static attribute,[8bit,10bit,12bit,14bit,16bit] */
} HI_MAPI_PIPE_CHN_ATTR_S;

typedef struct hiMAPI_PIPE_ISP_ATTR_S {
    HI_FLOAT f32FrameRate;
    SIZE_S stSize;
    ISP_BAYER_FORMAT_E enBayer;
    ISP_SNS_MIRRORFLIP_TYPE_E enSnsMirrorFlip;
} HI_MAPI_PIPE_ISP_ATTR_S;

typedef struct hiMAPI_VCAP_PIPE_ATTR_S {
    HI_MAPI_PIPE_TYPE_E enPipeType;         /**<static attribute,PIPE attribute */
    FRAME_RATE_CTRL_S stFrameRate;          /**<dynamic attribute, PIPE attribute */
    COMPRESS_MODE_E enCompressMode;         /**<dynamic attribute,PIPE attribute */
    HI_BOOL bIspBypass;                     /**<ISP bypass enable */
    VI_PIPE_BYPASS_MODE_E enPipeBypassMode; /**<Pipe Bypass Mode */
#ifndef __HI3559AV100__
    /* the attr is not support in current version */
    FRAME_INTERRUPT_ATTR_S stFrameIntAttr; /**<Pipe frame int attr */
#endif
    HI_MAPI_PIPE_ISP_ATTR_S stIspPubAttr;                             /**<ISP pub attribute */
    HI_MAPI_PIPE_CHN_ATTR_S astPipeChnAttr[HI_MAPI_PIPE_MAX_CHN_NUM]; /**<attributes corresponding to PIPE channel */
} HI_MAPI_VCAP_PIPE_ATTR_S;

typedef struct hiMAPI_VCAP_ATTR_S {
    HI_MAPI_VCAP_DEV_ATTR_S stVcapDevAttr; /**<VCAP DEV attribute */

    HI_U32 u32PipeBindNum;                                               /**<The number of pipe bound to Vcap dev */
    HI_HANDLE aPipeIdBind[HI_MAPI_VCAP_MAX_PIPE_NUM];                    /**<The ID of the pipe bound to Vcap dev */
    HI_MAPI_VCAP_PIPE_ATTR_S
    astVcapPipeAttr[HI_MAPI_VCAP_MAX_PIPE_NUM]; /**<The parameter set for the pipe bound to Vcap dev */
} HI_MAPI_VCAP_ATTR_S;

typedef struct hiMAPI_SENSOR_ATTR_S {
    HI_U8 u8SnsMode;
    WDR_MODE_E enWdrMode;
    SIZE_S stSize;
} HI_MAPI_SENSOR_ATTR_S;

/* redefine a struct for the need of getSensorAllModes.
the difference of HI_MAPI_SENSOR_ATTR_S and HI_MAPI_SENSOR_MODE_S is the addition of s32SnsMaxFrameRate */
typedef struct hiMAPI_SENSOR_MODE_S {
    HI_S32 s32Width;  /* it is related to mipi width */
    HI_S32 s32Height; /* it is related to mipi height */
    WDR_MODE_E enWdrMode;
    HI_U8 u8SnsMode;
    HI_FLOAT f32SnsMaxFrameRate;
} HI_MAPI_SENSOR_MODE_S;

/* NORMAL snap info */
typedef struct hiMAPI_SNAP_NORMAL_ATTR_S {
    HI_U32 u32FrameCnt;        /**<Snap frame number [0,0xFFFFFFFF] */
    HI_U32 u32RepeatSendTimes; /**<Set PIPE to send first frame repeatedly when offline(Set 2 when defog, set 1 for the others) */
} HI_MAPI_SNAP_NORMAL_ATTR_S;

typedef enum hiMAPI_VCAP_SNAP_CMD_E {
    HI_MAPI_VCAP_SNAP_CMD_BNR = 0,
    HI_MAPI_VCAP_SNAP_CMD_Sharpen,
    HI_MAPI_VCAP_SNAP_CMD_BUTT
} HI_MAPI_VCAP_SNAP_CMD_E;

typedef struct hiMAPI_VCAP_SNAP_ATTR_S {
    SNAP_TYPE_E enSnapType;
    HI_BOOL bLoadCCM; /**<HI_TRUE:use CCM of SnapIspInfo, HI_FALSE: Algorithm calculate */
    union {
        HI_MAPI_SNAP_NORMAL_ATTR_S stNormalAttr;
        SNAP_PRO_ATTR_S stProAttr;
    };
} HI_MAPI_VCAP_SNAP_ATTR_S;

/* Expand attributes */

typedef enum hiMAPI_VCAP_CMD_E {
    HI_MAPI_VCAP_CMD_ISP_ExposureAttr,  /**<ISP Expand attributes:Exposure time */
    HI_MAPI_VCAP_CMD_ISP_MeteringMode,  /**<ISP Expand attributes:Metering mode  */
    HI_MAPI_VCAP_CMD_ISP_WbAttr,        /**<ISP Expand attributes:White balance mode */
    HI_MAPI_VCAP_CMD_ISP_Sharpen,       /**<ISP Expand attributes:Sharpen */
    HI_MAPI_VCAP_CMD_ISP_Luma,          /**<ISP Expand attributes:Luma   */
    HI_MAPI_VCAP_CMD_ISP_Saturation,    /**<ISP Expand attributes:saturation */
    HI_MAPI_VCAP_CMD_ISP_Cac,           /**<ISP Expand attributes:Purple edge correction */
    HI_MAPI_VCAP_CMD_ISP_DngColorParam, /**<ISP Expand attributes:Get DNG color information    */
    HI_MAPI_VCAP_CMD_ISP_PipeDiffAttr,  /**<ISP Expand attributes:Multiple Splicing Differences ISP Pipe Discrepancy Attributes */
    HI_MAPI_VCAP_CMD_ISP_WDR,           /**<ISP Expand attributes:WDR Attributes,You can set the synthesis method and
                                                                      its parameters,Related to ghosting problems, indoor frequency flicker problem,
                                                                      Fusion mode can improve Flicker problem */

    HI_MAPI_VCAP_CMD_ISP_GetDngImageStaticInfo, /**<ISP Expand attributes:Get DNG static information */
    HI_MAPI_VCAP_CMD_ISP_GetExposureInfo,       /**<ISP Expand attributes:Query AE exposure information  */
    HI_MAPI_VCAP_CMD_ISP_GetAwbInfo,            /**<ISP Expand attributes:Query AWB information */
    HI_MAPI_VCAP_CMD_ISP_GetPubAttr,            /**<ISP Expand attributes:get ISP pub Attributes */

    HI_MAPI_VCAP_CMD_ISP_Set3AInit,   /**<ISP Expand attributes:set 3A init Attributes */
    HI_MAPI_VCAP_CMD_ISP_SetAEDebug,  /**<ISP Expand attributes:set AE debug */
    HI_MAPI_VCAP_CMD_ISP_SetAWBDebug, /**<ISP Expand attributes:set AWB debug */

    HI_MAPI_VCAP_CMD_LDC,        /**<CHN Expand attributes:LDC */
    HI_MAPI_VCAP_CMD_Rotate,     /**<CHN Expand attributes:Rotate  */
    HI_MAPI_VCAP_CMD_MirrorFlip, /**<CHN Expand attributes:mirror/flip */

    HI_MAPI_VCAP_CMD_PIPE_GetCmpParam, /**<PIPE Expand attributes:ComPressParam */
    HI_MAPI_VCAP_CMD_ENTER_STANDBY,    /**<PIPE Expand attributes:Sensor enter standby mode */
    HI_MAPI_VCAP_CMD_EXIT_STANDBY,     /**<PIPE Expand attributes:Sensor exit standby mode */
    HI_MAPI_VCAP_CMD_RESTART_SENSOR,   /**<PIPE Expand attributes:Sensor restart */
    HI_MAPI_VCAP_CMD_LDCV2,            /**<CHN Expand attributes:LDC */
    HI_MAPI_VCAP_CMD_BUTT
} HI_MAPI_VCAP_CMD_E;

typedef enum hiMAPI_VCAP_OP_TYPE_E {
    HI_MAPI_VCAP_OP_TYPE_AUTO = 0,
    HI_MAPI_VCAP_OP_TYPE_MANUAL = 1,
    HI_MAPI_VCAP_OP_TYPE_DISABLE = 2,
    HI_MAPI_VCAP_OP_TYPE_BUTT
} HI_MAPI_VCAP_OP_TYPE_E;

/* Expand attributes: Exposure (exposure time, metering mode, sensitivity, exposure compensation) */
typedef struct hiMAPI_VCAP_EXPOSURE_MANUAL_MODE_S {
    HI_MAPI_VCAP_OP_TYPE_E enExpTimeOpType;
    HI_MAPI_VCAP_OP_TYPE_E enAGainOpType;
    HI_MAPI_VCAP_OP_TYPE_E enDGainOpType;
    HI_MAPI_VCAP_OP_TYPE_E enISPDGainOpType;
    HI_U32 u32ExposureTime;
    HI_U32 u32AGain;
    HI_U32 u32DGain;
    HI_U32 u32ISPDGain;
} HI_MAPI_VCAP_EXPOSURE_MANUAL_MODE_S;

typedef struct hiMAPI_VCAP_AE_RANGE_S {
    HI_U32 u32Min;
    HI_U32 u32Max;
} HI_MAPI_VCAP_AE_RANGE_S;

typedef struct hiMAPI_VCAP_EXPOSURE_AUTO_MODE_S {
    HI_U32 u32EVBias;                       /**<Exposure compensation< [0,0xFFFF] */
    HI_MAPI_VCAP_AE_RANGE_S stSysGainRange; /**<ISO */
} HI_MAPI_VCAP_EXPOSURE_AUTO_MODE_S;

typedef struct hiMAPI_VCAP_EXPOSURE_ATTR_S {
    HI_MAPI_VCAP_OP_TYPE_E enOpType;
    HI_MAPI_VCAP_EXPOSURE_MANUAL_MODE_S stMExposureMode;
    HI_MAPI_VCAP_EXPOSURE_AUTO_MODE_S stAExposureMode;
} HI_MAPI_VCAP_EXPOSURE_ATTR_S;

/* Expand attributes: white balance mode */
typedef struct hiMAPI_VCAP_WB_MANUAL_MODE_S {
    HI_U32 u32ColorTemp; /**< Unit:Kelvin,1500~15000 */
} HI_MAPI_VCAP_WB_MANUAL_MODE_S;

typedef struct hiMAPI_VCAP_WB_ATTR_S {
    HI_MAPI_VCAP_OP_TYPE_E enOpType;
    HI_MAPI_VCAP_WB_MANUAL_MODE_S stWBMode;
} HI_MAPI_VCAP_WB_ATTR_S;

/** The current exposure info.HI_MAPI_VCAP_CMD_ISP_GetExposureInfo */
typedef struct hiMAPI_VCAP_EXPOSURE_INFO_S {
    HI_U32 u32ExpTime;
    HI_U32 u32AGain;
    HI_U32 u32DGain;
    HI_U32 u32ISPDGain;
    HI_U32 u32Exposure;
    HI_U32 u32LinesPer500ms;
    HI_U32 u32PirisFNO;
    HI_U32 u32LongExpTime;
    HI_U32 u32ShortExpTime;
    HI_U32 u32MedianExpTime;
    HI_BOOL bExposureIsMAX;
    HI_S16 s16HistError;
    HI_U8 u8AveLum;
    HI_U32 u32Fps;
    HI_U32 u32ISO;
    HI_U32 u32RefExpRatio;
    HI_U32 u32FirstStableTime;
} HI_MAPI_VCAP_EXPOSURE_INFO_S;

/* Expand attributes: sharpness */
typedef struct hiMAPI_VCAP_SHARPEN_MANUAL_ATTR_S {
    HI_U16 au16EdgeStr[HI_MAPI_ISP_SHARPEN_GAIN_NUM];
} HI_MAPI_VCAP_SHARPEN_MANUAL_ATTR_S;

typedef struct hiMAPI_VCAP_SHARPEN_S {
    HI_MAPI_VCAP_OP_TYPE_E enOpType;
    HI_MAPI_VCAP_SHARPEN_MANUAL_ATTR_S stSharpenManualAttr;
} HI_MAPI_VCAP_SHARPEN_S;

/* Expand attributes: brightness */
typedef struct hiMAPI_VCAP_LUMA_S {
    HI_U8 u8Luma; /**<Luma  cmd:HI_MAPI_VCAP_CMD_ISP_Luma */
} HI_MAPI_VCAP_LUMA_S;

/* Expand attributes: saturation */
typedef struct hiMAPI_VCAP_SATURATION_S {
    HI_U8 u8Saturation; /**< saturation  cmd:HI_MAPI_VCAP_CMD_ISP_Saturation */
} HI_MAPI_VCAP_SATURATION_S;
/**/
typedef struct hiMAPI_VCAP_METERINGMODE_S {
    HI_U8 au8Weight[HI_MAPI_VCAP_AE_ZONE_ROW][HI_MAPI_VCAP_AE_ZONE_COLUMN]; /* RW; Range:[0x0, 0xF]; Format:4.0;
                                                                                                                            AE weighting table */
} HI_MAPI_VCAP_METERINGMODE_S;

/* Expand attributes:mirror and flip */
typedef struct hiMAPI_VCAP_MIRRORFLIP_ATTR_S {
    HI_BOOL bMirror;
    HI_BOOL bFlip;
} HI_MAPI_VCAP_MIRRORFLIP_ATTR_S;

typedef struct hiMAPI_VCAP_RAW_DATA_S {
    HI_VOID *pPrivateData;
    HI_S32(*pfn_VCAP_RawDataProc)(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pVCapRawData, HI_S32 s32DataNum,
           HI_VOID *pPrivateData);
} HI_MAPI_VCAP_RAW_DATA_S;

/** Null pointer error for vcap */
#define HI_MAPI_VCAP_ENULL_PTR HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
/** illegal parameter error for vcap */
#define HI_MAPI_VCAP_EILLEGAL_PARA HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
/** invalid fd error for vcap */
#define HI_MAPI_VCAP_EINVALID_FD HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_FD)
/** not support error for vcap */
#define HI_MAPI_VCAP_ENOTSUPPORT HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_SUPPORT)
/** not inited error for vcap */
#define HI_MAPI_VCAP_ENOT_INITED HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_INITED)
/** dev\pipe\chn not config attr */
#define HI_MAPI_VCAP_ENOT_CONFIG HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_CONFIG)
/** operate fail */
#define HI_MAPI_VCAP_EOPERATE_FAIL HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_OPERATE_FAIL)
/** no memery for vcap */
#define HI_MAPI_VCAP_ENOMEM HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)
/** not permit */
#define HI_MAPI_VCAP_ENOT_PERM HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
/** not exist */
#define HI_MAPI_VCAP_EUNEXIST HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)
/** illegal handle */
#define HI_MAPI_VCAP_EILLEGAL_HANDLE \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_HANDLE)

#define HI_MAPI_VCAP_ETIME_OUT HI_MAPI_DEF_ERR(HI_MAPI_MOD_VCAP, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_TIME_OUT)

/** @} */ /** <!-- ==== VCAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_VCAP_DEFINE_H__ */
