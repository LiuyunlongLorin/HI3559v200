#ifndef __HI_DNG_H__
#define __HI_DNG_H__

#include <unistd.h>
#include <string.h>
#include "hi_mw_type.h"
#include "tiffio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DNG_MAX_FILE_NAME                    (128) /* file name path max */
#define DNG_MUXER_LENINFO_LEN                (4)
#define DNG_MUXER_REF_BW                     (6)
#define DNG_MUXER_CROP_LEN                   (2)
#define DNG_MUXER_SCALE_LEN                  (2)
#define DNG_MUXER_REPEAT_LEN                 (2)
#define DNG_MUXER_GPSVER_LEN                 (4)
#define DNG_MUXER_TUDE_LEN                   (3)
#define DNG_MUXER_XPCOMMENT_LEN              (128)
#define DNG_MUXER_XPKEYWORDS_LEN             (16)
#define DNG_MUXER_CFACOLORPLANE              (3)
#define DNG_MUXER_MATRIX_SIZE                (9)
#define DNG_MUXER_NP_SIZE                    (6)
#define DNG_MUXER_ISP_BAYER_CHN              (4)
#define DNG_MUXER_DCF_DESCRIPTION_LENGTH     (32)
#define DNG_MUXER_DATETIME_LENGTH            (20)
#define DNG_MUXER_PROFILEHUESATMAPDATA_LENGH (324) /* HueDivisions * SaturationDivisions * ValueDivisions * 3 default: 6*6*3*3 */
#define DNG_MUXER_PROFILEHUESATMAPDIM_LENGH  (3)
#define DNG_DATETIME_LEN                     (32)
#define TIFFTAG_PROFILEHUESATMAPDIMS         (50937)
#define TIFFTAG_PROFILEHUESATMAPDATA1        (50938)
#define TIFFTAG_PROFILEHUESATMAPDATA2        (50939)
#define TIFFTAG_PROFILEEMBEDPOLICY           (50941)
#define TIFFTAG_ORIGINALDEFAULTFINALSIZE     (51089)
#define TIFFTAG_ORIGINALBESTQUALITYSIZE      (51090)
#define TIFFTAG_ORIGINALDEFAULTCROPSIZE      (51091)
#define TIFFTAG_OPCODELIST1                  (51008)
#define TIFFTAG_DEFAULTUSERCROP              (51125)
#define TIFFTAG_FORWARDMATRIX1               (50964)
#define TIFFTAG_FORWARDMATRIX2               (50965)
#define TIFFTAG_NOISEPROFILE                 (51041)
#define TIFFTAG_XPCOMMENT                    (40092)
#define TIFFTAG_XPKEYWORDS                   (40094)

#define GPSTAG_VERSIONID    (0)
#define GPSTAG_LATITUDEREF  (1)
#define GPSTAG_LATITUDE     (2)
#define GPSTAG_LONGITUDEREF (3)
#define GPSTAG_LONGITUDE    (4)
#define GPSTAG_ALTITUDEREF  (5)
#define GPSTAG_ALTITUDE     (6)

typedef enum hiDNG_MUXER_IMAGE_TYPE_E {
    HI_DNG_MUXER_IMAGE_IFD0 = 1,  // it may have subifds when it's thm data;if no subifds,it's raw data
    HI_DNG_MUXER_IMAGE_SCREEN,    // sub ifd of ifd0,must have ifd0 already
    HI_DNG_MUXER_IMAGE_RAW,       // subifd of ifd0,must have ifd0 already
} HI_DNG_MUXER_IMAGE_TYPE_E;

typedef enum hiDNG_MUXER_CFALAYOUT_TYPE_E {
    DNG_MUXER_CFALAYOUT_TYPE_RECTANGULAR = 1,
    DNG_MUXER_CFALAYOUT_TYPE_A, /* a,b,c... not support */
    DNG_MUXER_CFALAYOUT_TYPE_B,
    DNG_MUXER_CFALAYOUT_TYPE_C,
    DNG_MUXER_CFALAYOUT_TYPE_D,
    DNG_MUXER_CFALAYOUT_TYPE_E,
    DNG_MUXER_CFALAYOUT_TYPE_F,
    DNG_MUXER_CFALAYOUT_TYPE_G,
    DNG_MUXER_CFALAYOUT_TYPE_H,
    DNG_MUXER_CFALAYOUT_TYPE_BUTT
} HI_DNG_MUXER_CFALAYOUT_TYPE_E;

typedef struct hiDNG_MUXER_SRATIONAL_S {
    HI_S32 s32Numerator;   /* represents the numerator of a fraction, */
    HI_S32 s32Denominator; /* the denominator. */
} HI_DNG_MUXER_SRATIONAL_S;

typedef struct hiDNG_MUXER_RATIONAL_S {
    HI_U32 u32Numerator;   /* represents the numerator of a fraction, */
    HI_U32 u32Denominator; /* the denominator. */
} HI_DNG_MUXER_RATIONAL_S;

typedef struct hiDNG_MUXER_EXIF_INFO_S {
    HI_CHAR aszDateTimeOriginal[DNG_MUXER_DATETIME_LENGTH];  // 0x9003 DateTimeOriginal
    HI_CHAR aszCaptureTime[DNG_MUXER_DATETIME_LENGTH];       // 0x9004 DateTimeDigitized
    HI_FLOAT fExposureTime;                                  /* Exposure time (reciprocal of shutter speed). */
    HI_U16 u16ExposureMode;                                  /* Indicates the exposure mode set when the image was shot.
                                                                              0 = Auto exposure,1 = Manual exposure, 2 = Auto bracket */
    HI_U16 u16ExposureProgram;                               /* Exposure program that the camera used when image was taken. '1' means manual control, '2'
                                                              program normal, '3' aperture priority, '4' shutter priority, '5' program creative (slow program),
                                                              '6' program action(high-speed program), '7' portrait mode, '8' landscape mode */
    HI_U16 u16LightSource;                                   /* Light source, actually this means white balance setting. '0' means unknown, '1' daylight, '2'
                                                                               fluorescent, '3' tungsten, '10' flash, '17' standard light A, '18' standard light B, '19' standard light
                                                                               C, '20' D55, '21' D65, '22' D75, '255' other */
    HI_FLOAT fFNumber;                                       /* The actual F-number (F-stop) of lens when the image was taken */
    HI_U16 u16ISOSpeedRatings;                               /* CCD sensitivity equivalent to Ag-Hr film speedrate */
    HI_FLOAT fExposureBiasValue;                             /* Exposure bias (compensation) value of taking picture */
    HI_U16 u16MeteringMode;                                  /* Exposure metering method. '0' means unknown, '1' average, '2' center weighted average, '3'
                                                                              spot, '4' multi-spot, '5' multi-segment, '6' partial, '255' other */
    HI_FLOAT fFocalLength;                                   /* Focal length of lens used to take image. Unit is millimeter */
    HI_U16 u16WhiteBalance;                                  /* Indicates the white balance mode set when the image was shot. */
    HI_U16 u16Saturation;                                    /* Indicates the direction of saturation processing applied by the camera when the image was shot.
                                                                              0 = Normal,1 = Low saturation,2 = High saturation */
    HI_U16 u16Sharpness;                                     /* Indicates the direction of sharpness processing applied by the camera when the image was shot.
                                                                              0 = Normal,1 = Soft,2 = Hard . */
    HI_FLOAT fMaxApertureValue;                              /* Maximum aperture value of lens. */
    HI_U8 u8SceneType;                                       /* Indicates the type of scene. Value '0x01' means that the image was directly photographed. */
    HI_U16 u16CustomRendered;                                /* Indicates the use of special processing on image data, such as rendering geared to output.
                                                                               0 = Normal process  1 = Custom process   */
    HI_U16 u16FocalLengthIn35mmFilm;                         /* Indicates the equivalent focal length assuming a 35mm film camera, in mm */
    HI_U16 u16SceneCaptureType;                              /* Indicates the type of scene that was shot. 0 = Standard,1 = Landscape,2 = Portrait,3 = Night scene. */
    HI_U16 u16GainControl;                                   /* Indicates the degree of overall image gain adjustment. 0 = None,1 = Low gain up,2 = High gain up,3 = Low gain down,4 = High gain down. */
    HI_U16 u16Contrast;                                      /* Indicates the direction of contrast processing applied by the camera when the image was shot. */
    HI_U16 u16Flash;                                         /* whether the picture is captured when a flash lamp is on */
    HI_FLOAT fDigitalZoomRatio;                              /* Indicates the digital zoom ratio when the image was shot.
                                                                                   If the numerator of the recorded value is 0, this indicates that digital zoom was not used. */
} HI_DNG_MUXER_EXIF_INFO_S;

typedef struct hiDNG_MUXER_IMAGE_GPS_INFO_S {
    HI_U8 au8GpsVersion[DNG_MUXER_GPSVER_LEN];
    HI_CHAR chGPSLatitudeRef;                    /* GPS LatitudeRef Indicates whether the latitude is north or south latitude,
                                    * 'N'/'S', default 'N' */
    HI_FLOAT afGPSLatitude[DNG_MUXER_TUDE_LEN];  /* GPS Latitude is expressed as degrees, minutes and seconds,a typical format like "dd/1,mm/1,ss/1",
                                    *  default 0/1, 0/1, 0/1 */
    HI_CHAR chGPSLongitudeRef;                   /* GPS LongitudeRef Indicates whether the longitude is east or west longitude,
                                    * 'E'/'W', default 'E' */
    HI_FLOAT afGPSLongitude[DNG_MUXER_TUDE_LEN]; /* GPS Longitude is expressed as degrees, minutes and seconds,a typical format like "dd/1,mm/1,ss/1",
                                    *  default 0/1, 0/1, 0/1 */
    HI_U8 u8GPSAltitudeRef;                      /* GPS AltitudeRef Indicates the reference altitude used, 0 - above sea level, 1 - below sea level
                                    * default 0 */
    HI_FLOAT fGPSAltitude;                       /* GPS AltitudeRef Indicates the altitude based on the reference u8GPSAltitudeRef, the reference unit is meters,
                                    * default 0/1 */
} HI_DNG_MUXER_GPS_INFO_S;

typedef struct hiDNG_MUXER_RAW_FORMAT_S {
    HI_FLOAT af32BlackLevel[DNG_MUXER_ISP_BAYER_CHN];
    HI_U8 au8CfaPattern[DNG_MUXER_ISP_BAYER_CHN];    /* 0:red 1:green 2: blue */
    HI_U8 au8CfaPlaneColor[DNG_MUXER_CFACOLORPLANE]; /* 0:red 1:green 2: blue */
    HI_DNG_MUXER_CFALAYOUT_TYPE_E enCfaLayout;
    HI_U16 au16BlcRepeatDim[DNG_MUXER_REPEAT_LEN];
    HI_U32 u32WhiteLevel;
    HI_FLOAT afDefaultScale[DNG_MUXER_SCALE_LEN];
    HI_U16 au16CfaRepeatPatternDim[DNG_MUXER_REPEAT_LEN];
} HI_DNG_MUXER_RAW_FORMAT_S;

typedef struct hiDNG_MUXER_IMAGE_STATIC_INFO_S {
    HI_FLOAT afColorMatrix1[DNG_MUXER_MATRIX_SIZE];
    HI_FLOAT afColorMatrix2[DNG_MUXER_MATRIX_SIZE];
    HI_FLOAT afCameraCalibration1[DNG_MUXER_MATRIX_SIZE];
    HI_FLOAT afCameraCalibration2[DNG_MUXER_MATRIX_SIZE];
    HI_U8 u8CalibrationIlluminant1; /* Light source, actually this means white balance setting. '0' means unknown, '1' daylight, '2'
                                                       fluorescent, '3' tungsten, '10' flash, '17' standard light A, '18' standard light B, '19' standard light
                                                       C, '20' D55, '21' D65, '22' D75, '255' other */
    HI_U8 u8CalibrationIlluminant2;
    HI_FLOAT afForwadMatrix1[DNG_MUXER_MATRIX_SIZE];
    HI_FLOAT afForwadMatrix2[DNG_MUXER_MATRIX_SIZE];

} HI_DNG_MUXER_IMAGE_STATIC_INFO_S;  // For raw

typedef struct hiDNG_MUXER_IMAGE_DYNAMIC_INFO_S {
    HI_FLOAT afAsShotNeutral[DNG_MUXER_CFACOLORPLANE];
    HI_DOUBLE adNoiseProfile[DNG_MUXER_NP_SIZE];
} HI_DNG_MUXER_IMAGE_DYNAMIC_INFO_S;  // for every image

typedef struct hiDNG_MUXER_IFD0_INFO_S {
    HI_U32 u32SubIFDs;  // count of subifd
    HI_CHAR aszUniqueCameraModel[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR aszCameraSerialNumber[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR aszMake[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR aszModel[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR aszSoftware[DNG_MUXER_DCF_DESCRIPTION_LENGTH];
    HI_CHAR aszDateTime[DNG_MUXER_DATETIME_LENGTH];
    HI_U8 u8Orientation;
    HI_U32 au32ProfileHueSatMapDim[DNG_MUXER_PROFILEHUESATMAPDIM_LENGH];
    HI_FLOAT *pfProfileHueSatMapData1;
    HI_U32 u32Map1Len;
    HI_FLOAT *pfProfileHueSatMapData2;
    HI_U32 u32Map2Len;
    HI_FLOAT afLensInfo[DNG_MUXER_LENINFO_LEN];
    HI_U8 *pu8XmpBuf;  // optional ApplicationNotes TIFFTAG_XMLPACKET
    HI_U32 u32XmpDataLen;
    HI_U8 *pu8XPComment;  // optional
    HI_U32 u32XPCommentLen;
    HI_U8 *pu8XPKeywords;  // optional
    HI_U32 u32XPKeywordsLen;
    HI_U8 *pu8DNGPrivData;  // optional
    HI_U32 u32DNGPrivDataLen;
    HI_BOOL bHasExifIFD;
    HI_BOOL bHasGpsIFD;
    HI_DNG_MUXER_EXIF_INFO_S stExifInfo;
    HI_DNG_MUXER_GPS_INFO_S stGpsInfo;
    HI_DNG_MUXER_IMAGE_DYNAMIC_INFO_S stDynamicInfo;
    HI_DNG_MUXER_IMAGE_STATIC_INFO_S stStaticInfo;
    HI_DNG_MUXER_RAW_FORMAT_S stRawFormat;  // if have no raw subifd,should muxer rawformat param in ifd0
} HI_DNG_MUXER_IFD0_INFO_S;

// screennail subifd
typedef struct hiDNG_MUXER_SCREENIFD_INFO_S {
    HI_FLOAT fReferenceBlackWhite[DNG_MUXER_REF_BW];
} HI_DNG_MUXER_SCREENIFD_INFO_S;

// raw subifd
typedef struct hiDNG_MUXER_RAWIFD_INFO_S {
    HI_FLOAT afDefaultCropOrigin[DNG_MUXER_CROP_LEN];
    HI_FLOAT afDefaultCropSize[DNG_MUXER_CROP_LEN];
    HI_VOID *pu8OpcodeList1;  // optional
    HI_U32 u32OpcodeList1DataLen;
    HI_DNG_MUXER_RAW_FORMAT_S stRawFormat;
} HI_DNG_MUXER_RAWIFD_INFO_S;

typedef struct hiDNG_MUXER_IMAGE_DATA_S {
    HI_U32 u32DataLen;
    HI_U8 *pu8DataBuf;
    HI_U32 u32Stride;  // used for row data,less or equal to u32width
} HI_DNG_MUXER_IMAGE_DATA_S;

typedef struct hiDNG_MUXER_IFD_INFO_S {
    HI_U16 u16Photometric;  // photometric interpretation
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U16 u16BitPerSample;
    HI_U16 u16SamplePerPixel;
    HI_U16 u16Compression;                  // such as jpeg
    HI_U16 u16PlanarConfig;                 // storage organization
    HI_U32 u32SubfileType;                  // 0 for raw file,newsubfiletype 254
    HI_DNG_MUXER_IMAGE_TYPE_E enImageType;  // thm or screamnail or raw
    union {
        HI_DNG_MUXER_IFD0_INFO_S stIFD0Info;           /* IFD0 info */
        HI_DNG_MUXER_RAWIFD_INFO_S stRawIFDInfo;       /* IFDO subifd: raw IFD info */
        HI_DNG_MUXER_SCREENIFD_INFO_S stScreenIFDInfo; /* IFDO subifd: screennail IFD info */
    };
    HI_DNG_MUXER_IMAGE_DATA_S stImageData;
} HI_DNG_MUXER_IFD_INFO_S;

typedef enum hiDNG_DEMUXER_IMAGE_TYPE_E {
    HI_DNG_DEMUXER_IMAGE_IFD0 = 1,  // usually thm or raw
    HI_DNG_DEMUXER_IMAGE_SUBSCREEN,
    HI_DNG_DEMUXER_IMAGE_SUBRAW,
} HI_DNG_DEMUXER_IMAGE_TYPE_E;

typedef struct hiDNG_DEMUXER_IMAGE_INFO_S {
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32DataLen;
} HI_DNG_DEMUXER_IMAGE_INFO_S;

typedef enum hiDNG_CONFIG_TYPE_E {
    HI_DNG_CONFIG_TYPE_MUXER = 1,  // usually thm or raw
    HI_DNG_CONFIG_TYPE_DEMUXER,
    HI_DNG_CONFIG_TYPE_BUTT,
} HI_DNG_CONFIG_TYPE_E;

typedef struct hiDNG_CONFIG_S {
    HI_DNG_CONFIG_TYPE_E enConfigType;
    HI_CHAR aszFileName[DNG_MAX_FILE_NAME]; /* file path and file name */
} HI_DNG_CONFIG_S;

/**
 * @brief create dng.
 * @param[out] ppDng HI_MW_PTR* : return dng  handle
 * @param[in] pstDngCfg HI_DNG_CONFIG_S * : dng config parameter
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_DNG_Create(HI_MW_PTR *ppDng, HI_DNG_CONFIG_S *pstDngCfg);

/**
 * @brief destroy dng .
 * @param[in] pDng HI_MW_PTR :  dng handle
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_DNG_Destroy(HI_MW_PTR pDng);

/**
 * @brief add dng IFD.
 * @param[in] pDng HI_MW_PTR : dng handle
 * @param[in] pstIFDInfo HI_DNG_MUXER_IFD_INFO_S : dng IFD info
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_DNG_AddIFD(HI_MW_PTR pDng, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo);

/**
 * @brief get dng IFD image info.
 * @param[in] pDng HI_MW_PTR : dng handle
 * @param[in] u32IfdIndex HI_U32 : ifd index
 * @param[out] pstImageInfo HI_DNG_DEMUXER_IMAGE_INFO_S* : image info
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_DNG_GetImageInfo(HI_MW_PTR pDng, HI_U32 u32IfdIndex, HI_DNG_DEMUXER_IMAGE_INFO_S *pstImageInfo);

/**
 * @brief get dng IFD image data,please call it after HI_DNG_GetImageInfo.
 * @param[in] pDng HI_MW_PTR : dng handle
 * @param[in] u32DataLen HI_U32 : image data length
 * @param[out] pu8Buffer HI_U8* : image data buffer
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_DNG_GetImageData(HI_MW_PTR pDng, HI_U8 *pu8Buffer, HI_U32 u32DataLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
