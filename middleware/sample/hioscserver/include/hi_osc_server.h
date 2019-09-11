/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_osc_server.h
* @brief     osc module header file
* @author    HiMobileCam middleware develop team
* @date      2017.10.01
*/

#ifndef __HI_OSC_SERVER_H__
#define __HI_OSC_SERVER_H__

#include "hi_mw_type.h"
#include "hi_defs.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
/** \addtogroup     OSCSVR */
/** @{ */  /** <!-- [OSCSVR] */
#define MODULE_NAME_OSCSERVER                       "OSCSERVER"

/*strcuct max length*/
#define HI_OSCSVR_MANUFACTYRER_MAX_LEN               (32)
#define HI_OSCSVR_MODEL_MAX_LEN                      (32)
#define HI_OSCSVR_SERIAL_NUMBER_MAX_LEN              (32)
#define HI_OSCSVR_FIRMWARE_VERSION_MAX_LEN           (16)
#define HI_OSCSVR_SUPPORT_URL_VERSION_MAX_LEN        (64)
#define HI_OSCSVR_STORAGE_URL_MAX_LEN                (64)
/*switch wifi*/
#define HI_OSCSVR_PRESHARED_KEY_MAX_LEN              (256)
#define HI_OSCSVR_WIFISSID_MAX_LEN                   (32)
#define HI_OSCSVR_WIFIPWD_MAX_LEN                    (16)
/*upload file*/
#define HI_OSCSVR_UPLOADURL_MAX_LEN                  (24)
#define HI_OSCSVR_ACCESS_TOKEN_MAX_LEN               (32)
/*Option len*/
#define HI_OSC_OPTION_CAPTURE_MODE_LEN               (12)
#define HI_OSC_OPTION_CAPTURE_MODE_SUPPORT           (3)
#define HI_OSC_OPTION_EXPOSE_SUPPORT_NUM             (5)
#define HI_OSC_OPTION_IOS_SUPPORT_NUM                (5)
#define HI_OSC_OPTION_SHUTTER_SUPPORT_NUM            (5)
#define HI_OSC_OPTION_APERTURE_SUPPORT_NUM           (8)
#define HI_OSC_OPTION_WHITE_BALANCE                  (20)
#define HI_OSC_OPTION_WHITE_BALANCE_NUM              (7)
#define HI_OSC_OPTION_EXPOSE_COMPENSATE_NUM          (6)
#define HI_OSC_OPTION_FORMAT_TYPE_NAME               (6)
#define HI_OSC_OPTION_FORMAT_SUP_NUM                 (6)
#define HI_OSC_OPTION_EXPOSE_DELAY_NUM               (6)
#define HI_OSC_OPTION_SLEEP_DELAY_NUM                (4)
#define HI_OSC_OPTION_OFF_DELAY_NUM                  (4)
#define HI_OSC_OPTION_DATE_TIME_ZONE                 (28)
#define HI_OSC_OPTION_HDR                            (6)
#define HI_OSC_OPTION_HDR_SUPPORT                    (2)
#define HI_OSC_OPTION_SHOTS_SUPPORT                  (4)
#define HI_OSC_OPTION_INCREMENT_SUPPORT              (4)
#define HI_OSC_OPTION_IMAGE_STABILIZATION            (4)
#define HI_OSC_OPTION_IMAGE_STABILIZATION_SUPPORT    (2)
#define HI_OSC_OPTION_WIFI_PASSWORD                  (14)
#define HI_OSC_OPTION_PREVIEW_FORMAT_SUPPORT         (7)
#define HI_OSC_OPTION_PHOTO_STITCHING                (9)
#define HI_OSC_OPTION_PHOTO_STITCHING_SUPPORT        (2)
#define HI_OSC_OPTION_VIDEO_STITCHING                (9)
#define HI_OSC_OPTION_VIDEO_STITCHING_SUPPORT        (2)
#define HI_OSC_OPTION_VIDEO_GPS                      (11)
#define HI_OSC_OPTION_VIDEO_GPS_SUPPORT              (2)
#define HI_OSC_OPTION_DELAY_PROCESS_SUPPORT          (2)
/*fileurl*/
#define HI_OSC_FILEURL_MAX_LEN                       (64)
#define HI_OSC_FILEURL_MAX_NUM                       (100)
/*list file*/
#define HI_OSC_LISTFILE_FILETYPE_LEN                 (6)
#define HI_OSC_LISTFILE_MAX_NUM                      (100)
/*list image*/
#define HI_OSC_CONTINUDATION_TOKEN_MAX_LEN           (18)
/*image property*/
#define HI_OSC_LISTFILE_FILE_NAME                    (32)
#define HI_OSC_LISTFILE_FILE_URL                     (128)
#define HI_OSC_LISTFILE_DATE_TIME                    (48)
#define HI_OSC_LISTFILE_THUMB_ENCODE                 (128)
#define HI_OSC_LISTFILE_PREVIEW_URL                  (128)
/*meta data exif*/
#define HI_OSC_METADATA_EXIF_VERSION                  (6)
#define HI_OSC_METADATA_EXIF_IMAGE_DESCRIPTION        (24)
#define HI_OSC_METADATA_EXIF_DATE_TIME                (24)
#define HI_OSC_METADATA_EXIF_GPS_PROCESS              (10)
#define HI_OSC_METADATA_EXIF_GPSLATITUDE_REF          (8)
#define HI_OSC_METADATA_EXIF_GPSLONGITUDE_REF         (16)
#define HI_OSC_METADATA_EXIF_MAKE                     (6)
#define HI_OSC_METADATA_EXIF_MODEL                    (16)
#define HI_OSC_METADATA_EXIF_SOFTWARE                 (8)
#define HI_OSC_METADATA_EXIF_COPYRIGHT                (16)
#define HI_OSC_METADATA_EXIF_MAKERNOTE                (16)
#define HI_OSC_METADATA_EXIF_IMAGE_ID                 (12)
/*meta data exif*/
#define HI_OSC_METADATA_XMP_PROJECT_TYPE              (12)
/*fingerprint len*/
#define HI_OSC_FINGER_PRINT_MAX_LEN                   (12)

/*options name Pack max len*/
#define HI_OSC_OPTIONS_NAME_MAX_LEN                   (28)

/*command enum*/
typedef enum hi_OSC_API_COMMAND_E
{
    /*api*/
    HI_OSC_COMMAND_INFO = 0,
    HI_OSC_COMMAND_STATE,
    HI_OSC_COMMAND_CHECKFORUPDATE,
    HI_OSC_COMMAND_COMM_STATUS,
    /*command*/
    HI_OSC_COMMAND_TAKE_PICTURE,
    HI_OSC_COMMAND_GET_PRELIVE_VIEW,
    HI_OSC_COMMAND_PROCESS_PICTURE,
    HI_OSC_COMMAND_START_CAPTURE,
    HI_OSC_COMMAND_STOP_CAPTURE,
    HI_OSC_COMMAND_LIST_FILE,
    HI_OSC_COMMAND_LIST_IMAGE,
    HI_OSC_COMMAND_DELETE,
    HI_OSC_COMMAND_GET_IMAGE,
    HI_OSC_COMMAND_GET_METADATA,
    HI_OSC_COMMAND_SET_OPTIONS,
    HI_OSC_COMMAND_GET_OPTIONS,
    HI_OSC_COMMAND_RESET,
    HI_OSC_COMMAND_START_SESSION,
    HI_OSC_COMMAND_UPDATE_SESSION,
    HI_OSC_COMMAND_CLOSE_SESSION,
    HI_OSC_COMMAND_SWITCH_WIFI,
    HI_OSC_COMMAND_UPLOAD_FILE,
    HI_OSC_COMMAND_PRIVATE,
    HI_OSC_COMMAND_INVAILD
} HI_OSC_API_COMMAND_E;

typedef struct hi_OSC_CONFIG_S
{
    HI_S32 s32ListFiles;           /*the max num of Listfile*/
    HI_S32 s32TimeOut;          /*Session timeout*/
    HI_BOOL bExclusiveUseFlag;  /*whether only for one use*/
} HI_OSC_CONFIG_S;

/*image info*/
typedef struct hi_OSC_FILE_PROPERTY_S_S
{
    /*list image*/
    HI_CHAR aszFileName[HI_OSC_LISTFILE_FILE_NAME];                /*the name of image*/
    HI_CHAR aszFileURL[HI_OSC_LISTFILE_FILE_URL];                  /*the url of the image*/
    HI_S32 s32FileSize;                                           /*the size of the image in bytes*/
    HI_CHAR aszDateTimeZone[HI_OSC_LISTFILE_DATE_TIME];            /*time for image, use 24 hour format*/
    HI_DOUBLE dLat;                                               /*Latitude of the image capture location, optional*/
    HI_DOUBLE dLng;                                               /*Longitude of the image capture location, optional*/
    HI_S32 s32Width;                                              /*width of the image*/
    HI_S32 s32Height;                                             /*height of the image*/
    HI_CHAR aszThumbnail[HI_OSC_LISTFILE_THUMB_ENCODE];            /*Base64 encoded string for thumbnail image (when maxThumbSize != NULL)*/
    /*added in list file*/
    HI_BOOL bIsProcessed;
    HI_CHAR aszPreviewUrl[HI_OSC_LISTFILE_PREVIEW_URL];
} HI_OSC_FILE_PROPERTY_S;

/*list file : File list MUST be sorted by capture time, newest*/
typedef struct hiOSCSERVER_LIST_FILE_S
{
    /*input*/
    HI_CHAR aszFileType[HI_OSC_LISTFILE_FILETYPE_LEN];
    HI_S32  s32StartPosition;              /*optional; if omitted, default to 0*/
    HI_S32  s32EntryCount;                 /*the number of image needed to be returned*/
    HI_S32  s32MaxThumbSize;               /*the size of thumb image*/
    HI_BOOL bIncludeThumb;                 /*(optional) default to true*/
    /*result*/
    HI_BOOL bInvaild;
    HI_S32  s32TotalEntries;
    HI_OSC_FILE_PROPERTY_S aszEntries[HI_OSC_LISTFILE_MAX_NUM];
} HI_OSCSERVER_LIST_FILE_S;

/*list file : File list MUST be sorted by capture time, newest*/
typedef struct hiOSCSERVER_LIST_IMAGE_S
{
    /*input*/
    HI_BOOL bIncludeThummb;                /*(optional) default to true*/
    HI_S32  s32EntryCounts;                 /*the number of image needed to be returned*/
    HI_S32  s32MaxThumbSize;               /*the size of thumb image*/
    HI_CHAR aszInContinuationToken[HI_OSC_CONTINUDATION_TOKEN_MAX_LEN];          /*(optional) continuation token*/
    /*result*/
    HI_BOOL bInvaild;
    HI_S32  s32TotalEntries;
    HI_CHAR aszOutContinuationToken[HI_OSC_CONTINUDATION_TOKEN_MAX_LEN];
    HI_OSC_FILE_PROPERTY_S aszEntries[HI_OSC_LISTFILE_MAX_NUM];
} HI_OSCSERVER_LIST_IMAGE_S;

/*API level supported by server*/
typedef enum hi_OSC_PATH_TYPE_E
{
    HI_OSC_ABSOLUTE_PATH = 0,
    HI_OSC_RELATIVE_PATH,
    HI_OSC_INIT_PATH
} HI_OSC_PATH_TYPE_E;

/*Option data*/
typedef struct hiOSC_OPTION_FILEFORMAT_S
{
    HI_CHAR aszType[HI_OSC_OPTION_FORMAT_TYPE_NAME];
    HI_S32  s32Width;
    HI_S32  s32Height;
    HI_U32  u32FrameRate;
} HI_OSC_OPTION_FILEFORMAT_S;

/*exposureBracket*/
typedef struct hiOSC_OPTION_EXPOSURE_BRACKET_S
{
    HI_S32     s32Shots;
    HI_DOUBLE  dIncrement;
    HI_BOOL    bAutoMode;   /*when hdrSupport == false*/
    HI_S32     aszShotsSupport[HI_OSC_OPTION_SHOTS_SUPPORT];
    HI_BOOL    bAutoModeSupport;
    HI_S32     aszIncrementSupport[HI_OSC_OPTION_INCREMENT_SUPPORT];
} HI_OSC_OPTION_EXPOSURE_BRACKET_S;

/*gps info*/
typedef struct hiOSC_OPTION_GPSINFO_S
{
    HI_DOUBLE  dLat;
    HI_DOUBLE  dLng;
} HI_OSC_OPTION_GPSINFO_S;

/*preview format*/
typedef struct hiOSC_OPTION_PREVIEW_FORMAT_S
{
    HI_U32  u32Width;
    HI_U32  u32Height;
    HI_U32  u32FrameRate;
} HI_OSC_OPTION_PREVIEW_FORMAT_S;

/*Capture Interval Support*/
typedef struct hiOSC_OPTION_CAPTURE_INTERVAL_S
{
    HI_U32  u32MinInterval;
    HI_U32  u32MaxInterval;
} HI_OSC_OPTION_CAPTURE_INTERVAL_S;

/*Capture Interval Support*/
typedef struct hiOSC_OPTION_CAPTURE_NUMBER_S
{
    HI_U32  u32MinNumber;
    HI_U32  u32MaxNumber;
} HI_OSC_OPTION_CAPTURE_NUMBER_S;

/*image options*/
typedef struct hiOSC_IMAGE_OPTIONS_S
{
    HI_S32  s32ExposureProgram;
    HI_S32  aszExposureProgramSupport[HI_OSC_OPTION_EXPOSE_SUPPORT_NUM];
    HI_S32  s32ISO;
    HI_S32  aszISOSupport[HI_OSC_OPTION_IOS_SUPPORT_NUM];
    HI_DOUBLE  dShutterSpeed;
    HI_DOUBLE  aszShutterSpeedSupport[HI_OSC_OPTION_SHUTTER_SUPPORT_NUM];
    HI_DOUBLE  dAperture;
    HI_DOUBLE  aszApertureSupport[HI_OSC_OPTION_APERTURE_SUPPORT_NUM];
    HI_CHAR aszWhiteBalance[HI_OSC_OPTION_WHITE_BALANCE];
    HI_CHAR aszWhiteBalanceSupport[HI_OSC_OPTION_WHITE_BALANCE_NUM][HI_OSC_OPTION_WHITE_BALANCE];
    HI_DOUBLE  dExposureCompensation;
    HI_DOUBLE  aszExposureCompensationSupport[HI_OSC_OPTION_EXPOSE_COMPENSATE_NUM];
    HI_S32  s32ExposureDelay;
    HI_S32  aszExposureDelaySupport[HI_OSC_OPTION_EXPOSE_DELAY_NUM];
    HI_BOOL bLevel1hdr;
    HI_BOOL bLevel1hdrSupport;
    HI_CHAR aszLevel2Hdr[HI_OSC_OPTION_HDR];
    HI_CHAR aszLevel2HdrSupport[HI_OSC_OPTION_HDR_SUPPORT][HI_OSC_OPTION_HDR];
    HI_CHAR aszImageStabilization[HI_OSC_OPTION_IMAGE_STABILIZATION];
    HI_CHAR aszImageStabilizationSupport[HI_OSC_OPTION_IMAGE_STABILIZATION_SUPPORT][HI_OSC_OPTION_IMAGE_STABILIZATION];
    HI_CHAR aszPhotoStitching[HI_OSC_OPTION_PHOTO_STITCHING];
    HI_CHAR aszPhotoStitchingSupport[HI_OSC_OPTION_PHOTO_STITCHING_SUPPORT][HI_OSC_OPTION_PHOTO_STITCHING];
    HI_OSC_OPTION_EXPOSURE_BRACKET_S stExposureBracket;
    HI_OSC_OPTION_EXPOSURE_BRACKET_S stExposureBracketSupport;
    HI_S32  s32CaptureInterval;
    HI_OSC_OPTION_CAPTURE_INTERVAL_S stCaptureIntervalSupport;
    HI_U32  u32CaptureNumber;
    HI_OSC_OPTION_CAPTURE_NUMBER_S stCaptureNumberSupport;
} HI_OSC_IMAGE_OPTIONS_S;

/*image options table*/
typedef struct hiOSC_IMAGE_OPTIONS_TABLE_S
{
    HI_BOOL  bExposureProgram;
    HI_BOOL  bExposureProgramSupport;
    HI_BOOL  bISO;
    HI_BOOL  bISOSupport;
    HI_BOOL  bShutterSpeed;
    HI_BOOL  bShutterSpeedSupport;
    HI_BOOL  bAperture;
    HI_BOOL  bApertureSupport;
    HI_BOOL  bWhiteBalance;
    HI_BOOL  bWhiteBalanceSupport;
    HI_BOOL  bExposureCompensation;
    HI_BOOL  bExposureCompensationSupport;
    HI_BOOL  bExposureDelay;
    HI_BOOL  bExposureDelaySupport;
    HI_BOOL  bLevel1hdr;
    HI_BOOL  bLevel1hdrSupport;
    HI_BOOL  bLevel2Hdr;
    HI_BOOL  bLevel2HdrSupport;
    HI_BOOL  bImageStabilization;
    HI_BOOL  bImageStabilizationSupport;
    HI_BOOL  bPhotoStitching;
    HI_BOOL  bPhotoStitchingSupport;
    HI_BOOL  bExposureBracket;
    HI_BOOL  bExposureBracketSupport;
    HI_BOOL bCaptureInterval;
    HI_BOOL bCaptureIntervalSupport;
    HI_BOOL bCaptureNumber;
    HI_BOOL bCaptureNumberSupport;
} HI_OSC_IMAGE_OPTIONS_TABLE_S;

/*video options*/
typedef struct hiOSC_VIDEO_OPTIONS_S
{
    HI_S32  s32RemainingVideoSeconds;
    HI_CHAR aszVideoStitching[HI_OSC_OPTION_VIDEO_STITCHING];
    HI_CHAR aszVideoStitchingSupport[HI_OSC_OPTION_VIDEO_STITCHING_SUPPORT][HI_OSC_OPTION_VIDEO_STITCHING];
    HI_CHAR aszVideoGPS[HI_OSC_OPTION_VIDEO_GPS];
    HI_CHAR aszVideoGPSSupport[HI_OSC_OPTION_VIDEO_GPS_SUPPORT][HI_OSC_OPTION_VIDEO_GPS];
} HI_OSC_VIDEO_OPTIONS_S;

/*video options table*/
typedef struct hiOSC_VIDEO_OPTIONS_TABLE_S
{
    HI_BOOL bRemainingVideoSeconds;
    HI_BOOL bVideoStitching;
    HI_BOOL bVideoStitchingSupport;
    HI_BOOL bVideoGPS;
    HI_BOOL bVideoGPSSupport;
} HI_OSC_VIDEO_OPTIONS_TABLE_S;

/*format options*/
typedef struct hiOSC_FORMAT_OPTIONS_S
{
    HI_OSC_OPTION_PREVIEW_FORMAT_S stPreviewFormat;
    HI_OSC_OPTION_PREVIEW_FORMAT_S aszPreviewFormatSupport[HI_OSC_OPTION_PREVIEW_FORMAT_SUPPORT];
    HI_OSC_OPTION_FILEFORMAT_S stFileFormat;
    HI_OSC_OPTION_FILEFORMAT_S aszFileFormatSupport[HI_OSC_OPTION_FORMAT_SUP_NUM];
} HI_OSC_FORMAT_OPTIONS_S;

/*format options table*/
typedef struct hiOSC_FORMAT_OPTIONS_TABLE_S
{
    HI_BOOL bPreviewFormat;
    HI_BOOL bPreviewFormatSupport;
    HI_BOOL bFileFormat;
    HI_BOOL bFileFormatSupport;
} HI_OSC_FORMAT_OPTIONS_TABLE_S;

/*info options*/
typedef struct hiOSC_INFO_OPTIONS_S
{
    HI_S32  s32SleepDelay;
    HI_S32  aszSleepDelaySupport[HI_OSC_OPTION_SLEEP_DELAY_NUM];
    HI_S32  s32OFFDelay;
    HI_S32  aszOFFDelaySupport[HI_OSC_OPTION_OFF_DELAY_NUM];
    HI_S32  s32TotalSpace;
    HI_S32  s32RemainingSpace;
    HI_S32  s32RemainingPictures;
    HI_CHAR aszDateTimeZone[HI_OSC_OPTION_DATE_TIME_ZONE];
    HI_BOOL bGyro;
    HI_BOOL bGyroSupport;
    HI_BOOL bGps;
    HI_BOOL bGpsSupport;
    HI_BOOL bDelayProcessing;
    HI_S32  s32PollingDelay;
    HI_S32  s32ClientVersion;
    HI_BOOL aszDelayProcessingSupport[HI_OSC_OPTION_DELAY_PROCESS_SUPPORT];
    HI_CHAR aszWifiPassword[HI_OSC_OPTION_WIFI_PASSWORD];
    HI_OSC_OPTION_GPSINFO_S stGPSInfo;
} HI_OSC_INFO_OPTIONS_S;

/*info options table*/
typedef struct hiOSC_INFO_OPTIONS_TABLE_S
{
    HI_BOOL  bSleepDelay;
    HI_BOOL  bSleepDelaySupport;
    HI_BOOL  bOFFDelay;
    HI_BOOL  bOFFDelaySupport;
    HI_BOOL  bTotalSpace;
    HI_BOOL  bRemainingSpace;
    HI_BOOL  bRemainingPictures;
    HI_BOOL  bDateTimeZone;
    HI_BOOL  bGyro;
    HI_BOOL  bGyroSupport;
    HI_BOOL  bGps;
    HI_BOOL  bGpsSupport;
    HI_BOOL  bDelayProcessing;
    HI_BOOL  bPollingDelay;
    HI_BOOL  bClientVersion;
    HI_BOOL  bDelayProcessingSupport;
    HI_BOOL  bWifiPassword;
    HI_BOOL  bGPSInfo;
} HI_OSC_INFO_OPTIONS_TABLE_S;

/*option union*/
typedef union hiOSCSERVER_OPTION_MODE_U
{
    HI_OSC_IMAGE_OPTIONS_S stImage;
    HI_OSC_VIDEO_OPTIONS_S stVideo;
} HI_OSC_OPTIONS_MODE_U;

/*option union*/
typedef enum hiOSCSERVER_OPTION_MODE_E
{
    HI_OSC_OPTIONS_MODE_INIT = 0X00,
    HI_OSC_OPTIONS_MODE_IMAGE,
    HI_OSC_OPTIONS_MODE_INTERVAL,
} HI_OSC_OPTIONS_MODE_E;

/*options*/
typedef struct hiOSCSERVER_OPTION_S
{
    HI_CHAR aszCaptureMode[HI_OSC_OPTION_CAPTURE_MODE_LEN];
    HI_CHAR aszCaptureModeSupport[HI_OSC_OPTION_CAPTURE_MODE_SUPPORT][HI_OSC_OPTION_CAPTURE_MODE_LEN];
    HI_OSC_OPTIONS_MODE_U unModeStuct;
    HI_OSC_OPTIONS_MODE_E enOptionMode;
    HI_BOOL bForMat;
    HI_OSC_FORMAT_OPTIONS_S stForMat;
    HI_BOOL bInfo;
    HI_OSC_INFO_OPTIONS_S stInfo;
} HI_OSCSERVER_OPTIONS_S;

/*options name table*/
typedef struct hiOSC_OPTIONS_TABLE_S
{
    HI_BOOL bCaptureMode;
    HI_BOOL bCaptureModeSupport;
    HI_OSC_IMAGE_OPTIONS_TABLE_S stImageTable;
    HI_OSC_VIDEO_OPTIONS_TABLE_S stVideoTable;
    HI_OSC_FORMAT_OPTIONS_TABLE_S stFormatTable;
    HI_OSC_INFO_OPTIONS_TABLE_S stInfoTable;
} HI_OSC_OPTIONS_TABLE_S;

/*Option data*/
typedef struct hiOSCSERVER_IMAGE_EXIF_S
{
    /*EXIF data*/
    HI_CHAR aszExifVersion[HI_OSC_METADATA_EXIF_VERSION];
    HI_CHAR aszImageDescription[HI_OSC_METADATA_EXIF_IMAGE_DESCRIPTION];
    HI_CHAR aszDateTime[HI_OSC_METADATA_EXIF_DATE_TIME];
    HI_S32  s32ImageWidth;
    HI_S32  s32ImageLength;
    HI_S32  s32ColorSpace;
    HI_S32  s32Compression;
    HI_S32  s32Orientation;
    HI_S32  s32Flash;
    HI_S32  s32FocalLength;
    HI_S32  s32WhiteBalance;
    HI_S32  s32ExposureTime;
    HI_S32  s32FNumber;
    HI_S32  s32ExposureProgram;
    HI_S32  s32ISOSpeedRatings;
    HI_S32  s32ShutterSpeedValue;
    HI_S32  s32ApertureValue;
    HI_S32  s32BrightnessValue;
    HI_S32  s32ExposureBiasValue;
    HI_CHAR aszGPSProcessingMethod[HI_OSC_METADATA_EXIF_GPS_PROCESS];
    HI_CHAR aszGPSLatitudeRef[HI_OSC_METADATA_EXIF_GPSLATITUDE_REF];
    HI_DOUBLE  dGPSLatitude;
    HI_CHAR aszGPSLongitudeRef[HI_OSC_METADATA_EXIF_GPSLONGITUDE_REF];
    HI_DOUBLE  dGPSLongitude;
    HI_CHAR aszMake[HI_OSC_METADATA_EXIF_MAKE];
    HI_CHAR aszModel[HI_OSC_METADATA_EXIF_MODEL];
    HI_CHAR aszSoftware[HI_OSC_METADATA_EXIF_SOFTWARE];
    HI_CHAR aszCopyright[HI_OSC_METADATA_EXIF_COPYRIGHT];
    HI_CHAR aszMakerNote[HI_OSC_METADATA_EXIF_MAKERNOTE];
    HI_CHAR aszImageUniqueID[HI_OSC_METADATA_EXIF_IMAGE_ID];
} HI_OSCSERVER_IMAGE_EXIF_S;

/*XMP data*/
typedef struct hiOSCSERVER_IMAGE_XMP_S
{
    HI_CHAR aszProjectionType[HI_OSC_METADATA_XMP_PROJECT_TYPE];
    HI_BOOL bUsePanoramaViewer;
    HI_DOUBLE dPoseHeadingDegrees;
    HI_S32 s32CroppedAreaImageWidthPixels;
    HI_S32 s32CroppedAreaImageHeightPixels;
    HI_S32 s32FullPanoWidthPixels;
    HI_S32 s32FullPanoHeightPixels;
    HI_S32 s32CroppedAreaLeftPixels;
    HI_S32 s32CroppedAreaTopPixels;
    /*XMP data*/
} HI_OSCSERVER_IMAGE_XMP_S;

/*Meta data*/
typedef struct hiOSCSERVER_METADATA_S
{
    HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_LEN];
    HI_BOOL bUrlInvaild;                                      /*the flag tell the url is invaild*/
    HI_OSCSERVER_IMAGE_XMP_S stXMP;
    HI_OSCSERVER_IMAGE_EXIF_S stExif;
} HI_OSCSERVER_METADATA_S;

/*upload file*/
typedef struct hiOSCSERVER_UPLOAD_FILE_S
{
    HI_BOOL bInvaild;
    HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_LEN];                 /*the url of file needed to be uploaded*/
    HI_CHAR aszUploadUrl[HI_OSCSVR_UPLOADURL_MAX_LEN];          /*the url of where needed to be uploaded*/
    HI_CHAR aszAccessToken[HI_OSCSVR_ACCESS_TOKEN_MAX_LEN];
} HI_OSCSERVER_UPLOAD_FILE_S;

/*switchwifi*/
typedef struct hiOSCSERVER_SWITCH_WIFI_S
{
    HI_BOOL bInvaild;
    HI_CHAR aszpreSharedKey[HI_OSCSVR_PRESHARED_KEY_MAX_LEN];
    HI_CHAR aszWifiSSID[HI_OSCSVR_WIFISSID_MAX_LEN];
    HI_CHAR aszWifiPWD[HI_OSCSVR_WIFIPWD_MAX_LEN];
} HI_OSCSERVER_SWITCH_WIFI_S;


/*API: /osc/info*/
typedef struct hiOSCSERVER_INFO_S
{
    HI_CHAR aszManufacturer[HI_OSCSVR_MANUFACTYRER_MAX_LEN];           /*the manufacturer of camera*/
    HI_CHAR aszModel[HI_OSCSVR_MODEL_MAX_LEN];                         /*the model of camera*/
    HI_CHAR aszSerialNumber[HI_OSCSVR_SERIAL_NUMBER_MAX_LEN];          /*the serial number of camera*/
    HI_CHAR aszFirmwareVersion[HI_OSCSVR_FIRMWARE_VERSION_MAX_LEN];    /*current firmware version*/
    HI_CHAR aszSupportURL[HI_OSCSVR_SUPPORT_URL_VERSION_MAX_LEN];      /*the url of web supported by camera*/
    HI_BOOL bGps;                                                     /*if the camera supports the gps, the value is true, or false*/
    HI_BOOL bGyro;                                                    /*if the camera supports the gyroscope, the value is true, or false*/
    HI_S32  s32Uptime;                                                /*the time after camera starting in seconds*/
    HI_S32  s32Httpport;                                              /*a JSON object contains the port of camera*/
    HI_S32  s32HttpUpdatesport;
    HI_S32  s32Httpsport;
    HI_S32  s32HttpsUpdatesport;
    HI_MW_PTR hVendorSpecific;                                        /*optinonal info specialzied by customer, none to -1*/
} HI_OSCSERVER_INFO_S;

/*API: /osc/state*/
typedef struct hiOSCSERVER_STATE_S
{
    HI_DOUBLE dBatteryLevel;                                          /*the level of camera battery*/
    HI_BOOL   bStorageChanged;                                        /*if new storage is inserted,is is true, or flase, is discarded in level2 */
    HI_CHAR   aszStorageUri[HI_OSCSVR_STORAGE_URL_MAX_LEN];            /*the only ID to tell different storage, to tell whether the storage is changed*/
    HI_CHAR   aszFingerPrint[HI_OSC_FINGER_PRINT_MAX_LEN];             /*the value of ingerprint(get by osc module)*/
    HI_MW_PTR hVendorSpecific;                                        /*optinonal info specialzied by customer, none to -1*/
} HI_OSCSERVER_STATE_S;

/*the level of oscserver api*/
typedef enum hi_OSC_SERVER_LEVEL_E
{
    HI_OSC_SERVER_LEVEL_1 = 1,
    HI_OSC_SERVER_LEVEL_2,
    HI_OSC_SERVER_LEVEL_BUT
} HI_OSC_SERVER_LEVEL_E;

/*the level of oscserver api*/
typedef enum hi_OSC_CAPTURE_MODE_E
{
    HI_OSC_CAPTURE_MODE_INTERVAL = 1,
    HI_OSC_CAPTURE_MODE_VIDEO,
    HI_OSC_CAPTURE_MODE_INIT
} HI_OSC_CAPTURE_MODE_E;

typedef enum hi_OSC_COMM_STATUS_E
{
    HIOSC_COMM_STATUS_DONE = 1,
    HIOSC_COMM_STATUS_INPROGRESS,
    HIOSC_COMM_STATUS_ERROR,
    HIOSC_COMM_STATUS_INIT
} HI_OSC_COMM_STATUS_E;

typedef enum hi_OSC_DEV_STATE_E
{
    HIOSC_DEV_STATE_BATTERY_LEVEL = 1,
    HIOSC_DEV_STATE_STORAGE_CHANGE,
    HIOSC_DEV_STATE_STORAGE_URL,
} HI_OSC_DEV_STATE_E;

/**
 * @brief create server instance.
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @param[in] pstOSCConfig HI_OSC_CONFIG_S : osc config info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_Create(HI_MW_PTR* ppOSC, HI_OSC_CONFIG_S* pstOSCConfig);

/**
* @brief start http server
* @param[in]handle : handle of OSCSVR
* @param[in]s32SocketFd : callback function
* @param[in]pClientReq : user private data
* @param[in]u32ReqLen : length of request
* @return 0 success
* @return -1 failure
*/
HI_S32 HI_OSCSVR_HTTP_DistribLink (HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);

/**
* @brief register  envent callback
* @param[in]hRecHdl : handle of OSCSVR
* @param[in]pfnEventCallback : callback function
* @param[in]pPrivateData : user private data
* @return 0 success
* @return -1 failure
*/
HI_S32 HI_OSCSVR_RegisterEventCallback(HI_MW_PTR pOSC, HI_OSC_API_COMMAND_E enEvent, HI_VOID* pfnEventCallback, HI_CHAR* pszCommandName);

/**
 * @brief destroy server instance.
 * @param[in] handle HI_HANDLE : oscserver handle
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_Destroy(HI_MW_PTR* ppOSC);

/**
 * @brief Exit server process.
 * @param[in] handle HI_HANDLE : oscserver handle
 * @return   0 success
 * @return err num failure
 */
HI_S32 HI_OSCSVR_ProcessExit(HI_MW_PTR pOSC);

/**
 * @brief add command status.
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_Update_CommandStatus(HI_MW_PTR pOSC, HI_CHAR* pszCommandId, HI_OSC_COMM_STATUS_E enComStatus, HI_CHAR aszResults[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN]);

/**
 * @brief send frame data.
 * @param[in,out] pHandle HI_HANDLE : return oscserver handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_Send_FrameData(HI_MW_PTR pOSC, HI_BOOL bFirstFrame, HI_S32* ps32SocketFd, HI_CHAR* pcFrameData, HI_S32 s32FrameLen);

/**
 * @brief change state data.
 * @param[in] handle HI_HANDLE : return oscserver handle
 * @param[in] enState HI_OSC_DEV_STATE_E : the enum of state
 * @param[in] pUpdates HI_VOID : the value to update, battery is double, change is bool, url is char*
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_OSCSVR_UpdateState(HI_MW_PTR pOSC, HI_OSC_DEV_STATE_E enState, HI_VOID* pUpdates);

/**
 *  get info event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_INFO_FN_PTR)(HI_OSCSERVER_INFO_S* pstOscInfo);

/**
 *  get state event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_STATE_FN_PTR)(HI_OSCSERVER_STATE_S* pstOscState);

/**
 *  take picture event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_TAKEPICTURE_FN_PTR)(HI_MW_PTR pOSC, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled);

/**
 *  get livepreview event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_GETLIVEPREVIEW_FN_PTR)(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_BOOL* pbDisabled);

/**
 *  process picture event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_PROCESSPICTURE_FN_PTR)(HI_MW_PTR pOSC, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled, HI_BOOL* pbUrlInvaild);

/**
 *  start capture event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_STARTCAPTURE_FN_PTR)(HI_MW_PTR pOSC, HI_OSC_PATH_TYPE_E enPathType, HI_CHAR* pszCommandId, HI_BOOL* pbComDisabled, HI_OSC_CAPTURE_MODE_E* penCaptureMode);

/**
 *  stop capture event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_STOPCAPTURE_FN_PTR)(HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszCaptureUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_BOOL* pbDisabled);

/**
 *  list file event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_LISTFILE_FN_PTR)(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_LIST_FILE_S* pstProperty);

/**
 *  list image event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_LISTIMAGE_FN_PTR)(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_LIST_IMAGE_S* pstListImage);

/**
 *  get metadata event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_GETMETADATA_FN_PTR)(HI_OSC_PATH_TYPE_E enPathType, HI_OSCSERVER_METADATA_S* pstMetaData);

/**
 *  delete file event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_DELETE_FN_PTR)(HI_OSC_PATH_TYPE_E enPathType, HI_CHAR aszFileUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN], HI_BOOL* bInvaild, HI_CHAR aszResultUrl[HI_OSC_FILEURL_MAX_NUM][HI_OSC_FILEURL_MAX_LEN]);

/**
 *  set option event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_SETOPTION_FN_PTR)(HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstSetOptions, HI_OSC_OPTIONS_TABLE_S* pstSetOptionsTable);

/**
 *  get option event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_GETOPTION_FN_PTR)(HI_OSC_SERVER_LEVEL_E enLevel, HI_OSCSERVER_OPTIONS_S* pstGetOptions, HI_OSC_OPTIONS_TABLE_S* pstGetOptionsTable);

/**
 *  reset event callback: reset the options to default
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_RESET_FN_PTR)(HI_VOID);

/**
 *  switch file event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_SWITCHWIFI_FN_PTR)(HI_OSCSERVER_SWITCH_WIFI_S* pstSwitchWifi);

/**
 *  upload file event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_UPLOADFILE_FN_PTR)(HI_OSCSERVER_UPLOAD_FILE_S* pstUploadFile);

/**
 *  private event callback
 *  user need to realize these event callback funcs that they are interested at.
 */
typedef HI_S32 (*HI_OSC_PRIVATECOM_FN_PTR)(HI_CHAR* pszInJsonPack, HI_CHAR* pszComName, HI_S32* ps32SocketFd);
/** @}*/  /** <!-- ==== OSCSVR End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSC_SERVER_H__*/
