#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <tiffio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "hi_mw_type.h"
#include "hi_dng_err.h"
#include "hi_dng.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define MAX_SAMPLE_CMD_LEN     30
#define MAX_SNAP_CNT    3
#define MAX_FRAME_NUM   5   //max depth of raw
#define SNAP_SENSOR_ID  0
#define SNAP_IMAGE_CAPTURE_ID  0
#define SNAP_VIDEO_CAPTURE_ID  1
#define SNAP_PROC_ID  0
#define SNAP_PORT_ID  1
#define SNAP_VENC_ID  0
#define SNAP_VENC_ID1  1

#define MAX_FRM_WIDTH   5000
#define LINEAR_RAW_NUM 1 //video mode, need LINEAR_RAW_NUM continous raw frames in one raw file
#define DEBUGINFO_FRMAE_NUM 5 //depth of DebugInfo

typedef struct hiDNG_THM_BUF_S
{
    HI_BOOL bDataArrived;
    HI_DNG_MUXER_IFD_INFO_S stThmIFD;
} HI_DNG_THM_BUF_S;

typedef struct hiDNG_SCREEMNAIL_BUF_S
{
    HI_BOOL bDataArrived;
    HI_DNG_MUXER_IFD_INFO_S stScreenIFD;
} HI_DNG_SCREEMNAIL_BUF_S;

typedef struct hiDNG_RAW_BUF_S
{
    HI_BOOL bDataArrived;
    HI_DNG_MUXER_IFD_INFO_S stRawIFD;
} HI_DNG_RAW_BUF_S;

typedef struct hiDNG_DATA_BUFFER_S
{
    HI_DNG_THM_BUF_S stThmInfo;//IFD 0 ,usual for thm, can be raw too
    HI_DNG_SCREEMNAIL_BUF_S stScreenNailInfo;
    HI_DNG_RAW_BUF_S stRawInfo;
} HI_DNG_DATA_BUFFER_S;

typedef struct hiDNG_DATA_CTX_S
{
    HI_DNG_DATA_BUFFER_S s_astDngDataBuf[MAX_FRAME_NUM];
} HI_DNG_DATA_CTX_S;
static HI_DNG_DATA_CTX_S s_stDngCtx ;

static HI_VOID print_help_info(HI_VOID)
{
    printf("List all testtool command\n");
    printf("h             list all command we provide\n");
    printf("q             quit sample test\n");
    printf("1             dng single photo sample with thm and screennail \n");
    printf("2             dng multi photo sample \n");
    printf("3             dng with no thm \n");

}




#define FREE_ARRAY(i,p,count)  do{for(i=0;i<count;i++){if(NULL != p[i]){free(p[i]);p[i]=NULL;}}}while(0)


HI_S32  SAMPLE_SetGPSIFD(HI_DNG_MUXER_GPS_INFO_S* pstGPSInfo)
{
    pstGPSInfo->au8GpsVersion[0] = 2;
    pstGPSInfo->au8GpsVersion[1] = 2;
    pstGPSInfo->au8GpsVersion[2] = 0;
    pstGPSInfo->au8GpsVersion[3] = 0;

    pstGPSInfo->afGPSLatitude[0] = 0;
    pstGPSInfo->afGPSLatitude[1] = 0;
    pstGPSInfo->afGPSLatitude[2] = 0;
    pstGPSInfo->afGPSLongitude[0] = 0;
    pstGPSInfo->afGPSLongitude[1] = 0;
    pstGPSInfo->afGPSLongitude[2] = 0;

    pstGPSInfo->chGPSLatitudeRef = 'N';
    pstGPSInfo->chGPSLongitudeRef = 'E';
    pstGPSInfo->fGPSAltitude = 0;
    pstGPSInfo->u8GPSAltitudeRef = 0;
    return HI_SUCCESS;
}

HI_S32  SAMPLE_SetExifIFD(HI_DNG_MUXER_IFD_INFO_S* pstThmIFD)
{
    pstThmIFD->stIFD0Info.bHasExifIFD = HI_TRUE;
    snprintf(pstThmIFD->stIFD0Info.stExifInfo.aszCaptureTime, DNG_MUXER_DATETIME_LENGTH, "%s",  "2017:11:12 12:12:12");
    snprintf(pstThmIFD->stIFD0Info.stExifInfo.aszDateTimeOriginal, DNG_MUXER_DATETIME_LENGTH, "%s",  "2017:11:12 12:12:12");
    pstThmIFD->stIFD0Info.stExifInfo.u16Flash = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16ISOSpeedRatings = 2970;
    pstThmIFD->stIFD0Info.stExifInfo.fDigitalZoomRatio = 0;
    pstThmIFD->stIFD0Info.stExifInfo.fExposureBiasValue = 0;
    pstThmIFD->stIFD0Info.stExifInfo.fExposureTime =  1.0f / 30;
    pstThmIFD->stIFD0Info.stExifInfo.fFNumber = 32.0;
    pstThmIFD->stIFD0Info.stExifInfo.fFocalLength = 0;
    pstThmIFD->stIFD0Info.stExifInfo.fMaxApertureValue = 65536;
    pstThmIFD->stIFD0Info.stExifInfo.u16Contrast = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16CustomRendered = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16ExposureMode = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16ExposureProgram = 2;
    pstThmIFD->stIFD0Info.stExifInfo.u16FocalLengthIn35mmFilm = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16GainControl = 1;
    pstThmIFD->stIFD0Info.stExifInfo.u16LightSource = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16MeteringMode = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16Saturation = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16SceneCaptureType = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u8SceneType = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16Sharpness = 0;
    pstThmIFD->stIFD0Info.stExifInfo.u16WhiteBalance = 0;

    return HI_SUCCESS;
}

HI_S32  SAMPLE_SetThmIFD(HI_DNG_MUXER_IFD_INFO_S* pstThmIFD)
{
    //set exif info
    HI_U32 i = 0;
    SAMPLE_SetExifIFD(pstThmIFD);
    snprintf(pstThmIFD->stIFD0Info.aszDateTime, DNG_MUXER_DATETIME_LENGTH, "%s", "2017:11:12 12:12:12");
    snprintf(pstThmIFD->stIFD0Info.aszMake, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "make");
    snprintf(pstThmIFD->stIFD0Info.aszModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "model");
    snprintf(pstThmIFD->stIFD0Info.aszSoftware, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "software");

    HI_FLOAT afColorMatrix1[DNG_MUXER_MATRIX_SIZE] = {1.005913973, -0.3384189901, -0.2515580055, -0.2034379985, 1.081694007, 0.3984909949, -0.04316899881, 0.2171149995, 1.196020008};
    HI_FLOAT afColorMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.982698977 , -0.2974979874 , -0.08117099809 , -0.3391309974 , 1.113445044 , 0.2588509914 , -0.05263899968 , 0.1755000051, 0.5555719731};
    HI_FLOAT afCameraCalibration1[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};
    HI_FLOAT afCameraCalibration2[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};

    HI_FLOAT afForwadMatrix1[DNG_MUXER_MATRIX_SIZE] = {0.6308299897, 0.2701559957, 0.06322500066, 0.1345120068, 1.048423052, -0.1829790022, 0.003497999629, -0.4157229957, 1.237597943};
    HI_FLOAT afForwadMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.6535180209 , 0.2953670022, 0.01532499912 , 0.1991869953, 1.059399009 , -0.2586300072 , -0.001023999881 , -0.3067879972, 1.133185983};

    for (i = 0; i < DNG_MUXER_MATRIX_SIZE; i++)
    {
        pstThmIFD->stIFD0Info.stStaticInfo.afCameraCalibration1[i] = afCameraCalibration1[i];
        pstThmIFD->stIFD0Info.stStaticInfo.afCameraCalibration2[i] = afCameraCalibration2[i];
        pstThmIFD->stIFD0Info.stStaticInfo.afColorMatrix1[i] = afColorMatrix1[i];
        pstThmIFD->stIFD0Info.stStaticInfo.afColorMatrix2[i] = afColorMatrix2[i];
        pstThmIFD->stIFD0Info.stStaticInfo.afForwadMatrix1[i] = afForwadMatrix1[i];
        pstThmIFD->stIFD0Info.stStaticInfo.afForwadMatrix2[i] = afForwadMatrix2[i];
    }



    pstThmIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant1 = 17;
    pstThmIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant2 = 23;

    HI_FLOAT afAsShotNeutral[DNG_MUXER_CFACOLORPLANE] = {0.6355748176, 1, 0.4075707494};
    HI_DOUBLE adNoiseProfile[DNG_MUXER_NP_SIZE] = {2e-005, 4.5e-007, 2e-005, 4.5e-007, 2e-005 , 4.5e-007};


    for (i = 0; i < DNG_MUXER_NP_SIZE; i++)
    {
        pstThmIFD->stIFD0Info.stDynamicInfo.adNoiseProfile[i] = adNoiseProfile[i];
    }

    for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
    {
        pstThmIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] =  afAsShotNeutral[i];
    }

    pstThmIFD->u32Height = 120;
    pstThmIFD->u32Width = 160;
    pstThmIFD->u16BitPerSample = 8;
    pstThmIFD->enImageType = HI_DNG_MUXER_IMAGE_IFD0;
    pstThmIFD->u16Compression = COMPRESSION_JPEG;
    pstThmIFD->u16Photometric = PHOTOMETRIC_YCBCR;
    pstThmIFD->u16SamplePerPixel = 3;
    pstThmIFD->u32SubfileType = 1;
    pstThmIFD->stIFD0Info.u32SubIFDs = 2;
    pstThmIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;//storage organization
    pstThmIFD->stIFD0Info.u8Orientation = ORIENTATION_TOPLEFT;
    snprintf(pstThmIFD->stIFD0Info.aszCameraSerialNumber, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "dd0001");
    snprintf(pstThmIFD->stIFD0Info.aszUniqueCameraModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "hisicam");
    //set by user
    //pstThmIFD->stIFD0Info.afProfileHueSatMapData1
    //pstThmIFD->stIFD0Info.afProfileHueSatMapData2
    //pstThmIFD->stIFD0Info.afLensInfo
    //pstThmIFD->stIFD0Info.pu8DNGPrivData
    //pstThmIFD->stIFD0Info.u32DNGPrivDataLen

    pstThmIFD->stIFD0Info.bHasGpsIFD = HI_TRUE;
    SAMPLE_SetGPSIFD(&(pstThmIFD->stIFD0Info.stGpsInfo));

    return HI_SUCCESS;
}

HI_S32  SAMPLE_SetRawIFD(HI_DNG_MUXER_IFD_INFO_S* pstRawIFD)
{
    HI_U32 i = 0;
    //set info for raw ifd
    pstRawIFD->u16BitPerSample = 16;
    for (i = 0; i < DNG_MUXER_ISP_BAYER_CHN; i++)
    {
        pstRawIFD->stRawIFDInfo.stRawFormat.af32BlackLevel[i] = 200;

    }
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPattern[0] = 0;
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPattern[1] = 1;
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPattern[2] = 1;
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPattern[3] = 2;

    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPlaneColor[0] = 0;
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPlaneColor[1] = 1;
    pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPlaneColor[2] = 2;

    pstRawIFD->stRawIFDInfo.stRawFormat.enCfaLayout = DNG_MUXER_CFALAYOUT_TYPE_RECTANGULAR;
    pstRawIFD->stRawIFDInfo.stRawFormat.u32WhiteLevel = 4095;

    pstRawIFD->stRawIFDInfo.stRawFormat.au16CfaRepeatPatternDim[0] = 2;
    pstRawIFD->stRawIFDInfo.stRawFormat.au16CfaRepeatPatternDim[1] = 2;

    pstRawIFD->stRawIFDInfo.stRawFormat.au16BlcRepeatDim[0] = 2;
    pstRawIFD->stRawIFDInfo.stRawFormat.au16BlcRepeatDim[1] = 2;

    pstRawIFD->stRawIFDInfo.stRawFormat.afDefaultScale[0] = 1;
    pstRawIFD->stRawIFDInfo.stRawFormat.afDefaultScale[1] = 1;

    //we muxerd raw into dng use bitsample 16;the u16OriginalBitpersample is used for transform raw data into 16 bit
    pstRawIFD->u16BitPerSample = 16;

    pstRawIFD->enImageType = HI_DNG_MUXER_IMAGE_RAW;
    pstRawIFD->u16Compression = COMPRESSION_NONE;
    pstRawIFD->u16Photometric = PHOTOMETRIC_CFA;
    pstRawIFD->u16SamplePerPixel = 1;
    pstRawIFD->u32SubfileType = 0;
    pstRawIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;
    pstRawIFD->stRawIFDInfo.afDefaultCropOrigin[0] = 8;
    pstRawIFD->stRawIFDInfo.afDefaultCropOrigin[1] = 8;
    pstRawIFD->stRawIFDInfo.afDefaultCropSize[0] =  pstRawIFD->u32Width - pstRawIFD->stRawIFDInfo.afDefaultCropOrigin[0] * 2 ;
    pstRawIFD->stRawIFDInfo.afDefaultCropSize[1] = pstRawIFD->u32Height - pstRawIFD->stRawIFDInfo.afDefaultCropOrigin[1] * 2 ;
    //set by user
    //pstRawIFD->stRawIFDInfo.pu8OpcodeList1
    //pstRawIFD->stRawIFDInfo.u32OpcodeList1DataLen


    return HI_SUCCESS;
}

HI_S32  SAMPLE_SetScreenIFD(HI_DNG_MUXER_IFD_INFO_S* pstScreenIFD)
{

    pstScreenIFD->u32Height = 720;
    pstScreenIFD->u32Width = 960;

    pstScreenIFD->u16BitPerSample = 8;
    pstScreenIFD->enImageType = HI_DNG_MUXER_IMAGE_SCREEN;
    pstScreenIFD->u16Compression = COMPRESSION_JPEG;
    pstScreenIFD->u16Photometric = PHOTOMETRIC_YCBCR;
    pstScreenIFD->u16SamplePerPixel = 3;
    pstScreenIFD->u32SubfileType = 1;
    pstScreenIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[0] = 0;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[1] = 255;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[2] = 128;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[3] = 255;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[4] = 128;
    pstScreenIFD->stScreenIFDInfo.fReferenceBlackWhite[5] = 255;

    return HI_SUCCESS;
}



HI_S32  SAMPLE_SetIFD0(HI_DNG_MUXER_IFD_INFO_S* pstIFD)
{
    //set exif info
    HI_U32 i = 0;
    //SAMPLE_SetExifIFD(pstIFD);
    snprintf(pstIFD->stIFD0Info.aszDateTime, DNG_MUXER_DATETIME_LENGTH, "%s", "2017:11:12 12:12:12");
    snprintf(pstIFD->stIFD0Info.aszMake, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "make");
    snprintf(pstIFD->stIFD0Info.aszModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "model");
    snprintf(pstIFD->stIFD0Info.aszSoftware, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "software");

    HI_FLOAT afColorMatrix1[DNG_MUXER_MATRIX_SIZE] = {1.005913973, -0.3384189901, -0.2515580055, -0.2034379985, 1.081694007, 0.3984909949, -0.04316899881, 0.2171149995, 1.196020008};
    HI_FLOAT afColorMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.982698977 , -0.2974979874 , -0.08117099809 , -0.3391309974 , 1.113445044 , 0.2588509914 , -0.05263899968 , 0.1755000051, 0.5555719731};
    HI_FLOAT afCameraCalibration1[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};
    HI_FLOAT afCameraCalibration2[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};

    HI_FLOAT afForwadMatrix1[DNG_MUXER_MATRIX_SIZE] = {0.6308299897, 0.2701559957, 0.06322500066, 0.1345120068, 1.048423052, -0.1829790022, 0.003497999629, -0.4157229957, 1.237597943};
    HI_FLOAT afForwadMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.6535180209 , 0.2953670022, 0.01532499912 , 0.1991869953, 1.059399009 , -0.2586300072 , -0.001023999881 , -0.3067879972, 1.133185983};

    for (i = 0; i < DNG_MUXER_MATRIX_SIZE; i++)
    {
        pstIFD->stIFD0Info.stStaticInfo.afCameraCalibration1[i] = afCameraCalibration1[i];
        pstIFD->stIFD0Info.stStaticInfo.afCameraCalibration2[i] = afCameraCalibration2[i];
        pstIFD->stIFD0Info.stStaticInfo.afColorMatrix1[i] = afColorMatrix1[i];
        pstIFD->stIFD0Info.stStaticInfo.afColorMatrix2[i] = afColorMatrix2[i];
        pstIFD->stIFD0Info.stStaticInfo.afForwadMatrix1[i] = afForwadMatrix1[i];
        pstIFD->stIFD0Info.stStaticInfo.afForwadMatrix2[i] = afForwadMatrix2[i];
    }



    pstIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant1 = 17;
    pstIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant2 = 23;

    HI_FLOAT afAsShotNeutral[DNG_MUXER_CFACOLORPLANE] = {0.6355748176, 1, 0.4075707494};
    HI_DOUBLE adNoiseProfile[DNG_MUXER_NP_SIZE] = {2e-005, 4.5e-007, 2e-005, 4.5e-007, 2e-005 , 4.5e-007};


    for (i = 0; i < DNG_MUXER_NP_SIZE; i++)
    {
        pstIFD->stIFD0Info.stDynamicInfo.adNoiseProfile[i] = adNoiseProfile[i];
    }

    for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
    {
        pstIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] =  afAsShotNeutral[i];
    }
    pstIFD->u16BitPerSample = 16;
    pstIFD->enImageType = HI_DNG_MUXER_IMAGE_IFD0;
    pstIFD->u16Compression = COMPRESSION_NONE;
    pstIFD->u16Photometric = PHOTOMETRIC_CFA;
    pstIFD->u16SamplePerPixel = 1;
    pstIFD->u32SubfileType = 0;

    pstIFD->stIFD0Info.u32SubIFDs = 0;
    pstIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;//storage organization
    pstIFD->stIFD0Info.u8Orientation = ORIENTATION_TOPLEFT;
    snprintf(pstIFD->stIFD0Info.aszCameraSerialNumber, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "dd0001");
    snprintf(pstIFD->stIFD0Info.aszUniqueCameraModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", "hisicam");
    //set by user


    pstIFD->stIFD0Info.bHasGpsIFD = HI_FALSE;
    //SAMPLE_SetGPSIFD(&(pstIFD->stIFDInfo.stGpsInfo));

    i = 0;

    for (i = 0; i < DNG_MUXER_ISP_BAYER_CHN; i++)
    {
        pstIFD->stIFD0Info.stRawFormat.af32BlackLevel[i] = 200;

    }
    pstIFD->stIFD0Info.stRawFormat.au8CfaPattern[0] = 0;
    pstIFD->stIFD0Info.stRawFormat.au8CfaPattern[1] = 1;
    pstIFD->stIFD0Info.stRawFormat.au8CfaPattern[2] = 1;
    pstIFD->stIFD0Info.stRawFormat.au8CfaPattern[3] = 2;

    pstIFD->stIFD0Info.stRawFormat.au8CfaPlaneColor[0] = 0;
    pstIFD->stIFD0Info.stRawFormat.au8CfaPlaneColor[1] = 1;
    pstIFD->stIFD0Info.stRawFormat.au8CfaPlaneColor[2] = 2;

    pstIFD->stIFD0Info.stRawFormat.enCfaLayout = DNG_MUXER_CFALAYOUT_TYPE_RECTANGULAR;
    pstIFD->stIFD0Info.stRawFormat.u32WhiteLevel = 4095;

    pstIFD->stIFD0Info.stRawFormat.au16CfaRepeatPatternDim[0] = 2;
    pstIFD->stIFD0Info.stRawFormat.au16CfaRepeatPatternDim[1] = 2;

    pstIFD->stIFD0Info.stRawFormat.au16BlcRepeatDim[0] = 2;
    pstIFD->stIFD0Info.stRawFormat.au16BlcRepeatDim[1] = 2;

    pstIFD->stIFD0Info.stRawFormat.afDefaultScale[0] = 1;
    pstIFD->stIFD0Info.stRawFormat.afDefaultScale[1] = 1;


    return HI_SUCCESS;
}

HI_S32 SAMPLE_DNG_DataProcOneRaw(HI_DNG_MUXER_IFD_INFO_S* pstIFD)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32DataLen = 0;
    HI_U32 width = 3840;
    HI_U32 height = 2160;
    HI_U32 row = 0;
    //get raw info

    pstIFD->u32Height = height;
    pstIFD->u32Width = width;

    u32DataLen = width * 2 * height;
    pstIFD->stImageData.u32Stride = width * 2;
    pstIFD->stImageData.u32DataLen = u32DataLen;
    pstIFD->stImageData.pu8DataBuf = (HI_U8*)malloc(u32DataLen);
    if (NULL == pstIFD->stImageData.pu8DataBuf)
    {

        printf("SAMPLE_DNG_DataProcRaw--------pstRawIFD->stImageData.pu8DataBuf malloc fail\n");
        return HI_FAILURE;
    }
    HI_U8* pu8Tmp = pstIFD->stImageData.pu8DataBuf;

    FILE* rawFile = fopen("test.raw", "r");
    if (NULL == rawFile)
    {
        printf("open test.raw fail\n");
        return HI_FAILURE;
    }
    char* rowdata = (char*)malloc(width * 2);
    for (row = 0; row < height; row++)
    {
        fread(rowdata, width * 2, 1, rawFile);
        memcpy(pu8Tmp, rowdata, width * 2);
        pu8Tmp += width * 2;
    }
    free(rowdata);
    fclose(rawFile);

    SAMPLE_SetIFD0(pstIFD);

    return s32Ret;
}

HI_S32 SAMPLE_DNG_DataProcThumbNail(HI_DNG_MUXER_IFD_INFO_S* pstThmIFD)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32DataLen = 0;
    HI_U32 jpeglen = 0;
    FILE* pfd = NULL;
    HI_U8 aszBuf[64 * 1024] = {0};


    pfd = fopen("thm.jpg", "r");
    if (NULL == pfd)
    {
        printf("open thm.jpg fail\n");
        return HI_FAILURE;
    }

    fseeko(pfd, 0, SEEK_SET);
    jpeglen = fread(aszBuf, 1, 64 * 1024, pfd);
    printf("SAMPLE_DNG_DataProcThumbNail jpeglen: %d\n", jpeglen);
    fclose(pfd);

    u32DataLen = jpeglen;
    pstThmIFD->u32Width = 160;
    pstThmIFD->u32Height = 120;
    pstThmIFD->stImageData.u32DataLen = u32DataLen;
    pstThmIFD->stImageData.pu8DataBuf = (HI_U8*)malloc(u32DataLen);
    if (NULL == pstThmIFD->stImageData.pu8DataBuf)
    {
        printf("SAMPLE_DNG_VENC_VIDEO_DataProcThumbNail malloc fail \n");

        return HI_FAILURE;
    }
    memcpy(pstThmIFD->stImageData.pu8DataBuf, aszBuf, u32DataLen);

    SAMPLE_SetThmIFD(pstThmIFD);



    return s32Ret;
}


HI_S32 SAMPLE_DNG_DataProcScreenNail(HI_DNG_MUXER_IFD_INFO_S* pstScreenIFD)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32DataLen = 0;
    HI_U8 aszBuf[500 * 1024] = {0};
    HI_U32 u32ScreenLen = 0;
    FILE* pfd = NULL;

    pfd = fopen("screennail.jpg", "r");
    if (NULL == pfd)
    {
        printf("open screennail.jpg fail\n");
        return HI_FAILURE;
    }

    fseeko(pfd, 0, SEEK_SET);
    u32ScreenLen = fread(aszBuf, 1, 500 * 1024, pfd);
    printf("SAMPLE_DNG_DataProcScreenNail datalen: %d\n", u32ScreenLen);
    fclose(pfd);

    u32DataLen = u32ScreenLen;
    pstScreenIFD->u32Width = 960;
    pstScreenIFD->u32Height = 720;
    pstScreenIFD->stImageData.u32DataLen = u32DataLen;
    pstScreenIFD->stImageData.pu8DataBuf = (HI_U8*)malloc(u32DataLen);
    if (NULL == pstScreenIFD->stImageData.pu8DataBuf)
    {
        printf("SAMPLE_DNG_VENC_VIDEO_DataProcScreenNail malloc fail \n");

        return HI_FAILURE;
    }
    memcpy(pstScreenIFD->stImageData.pu8DataBuf, aszBuf, u32DataLen);
    SAMPLE_SetScreenIFD(pstScreenIFD);



    return s32Ret;
}

HI_S32 SAMPLE_DNG_DataProcRaw(HI_DNG_MUXER_IFD_INFO_S* pstRawIFD)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32DataLen = 0;
    HI_U32 width = 3840;
    HI_U32 height = 2160;
    HI_U32 row = 0;
    //get raw info

    pstRawIFD->u32Height = height;
    pstRawIFD->u32Width = width;

    u32DataLen = width * 2 * height;
    pstRawIFD->stImageData.u32Stride = width * 2;
    pstRawIFD->stImageData.u32DataLen = u32DataLen;
    pstRawIFD->stImageData.pu8DataBuf = (HI_U8*)malloc(u32DataLen);
    if (NULL == pstRawIFD->stImageData.pu8DataBuf)
    {

        printf("SAMPLE_DNG_DataProcRaw--------pstRawIFD->stImageData.pu8DataBuf malloc fail\n");
        return HI_FAILURE;
    }
    HI_U8* pu8Tmp = pstRawIFD->stImageData.pu8DataBuf;

    FILE* rawFile = fopen("test.raw", "r");
    if (NULL == rawFile)
    {
        printf("open test.raw fail\n");
        return HI_FAILURE;
    }
    char* rowdata = (char*)malloc(width * 2);
    for (row = 0; row < height; row++)
    {
        fread(rowdata, width * 2, 1, rawFile);
        memcpy(pu8Tmp, rowdata, width * 2);
        pu8Tmp += width * 2;
    }
    free(rowdata);
    fclose(rawFile);


    SAMPLE_SetRawIFD(pstRawIFD);

    printf("--------raw image  bitpersample  %d --------\n", pstRawIFD->u16BitPerSample);



    return s32Ret;
}





HI_S32 SAMPLE_DNG_Init()
{
    //init ctx

    memset(&s_stDngCtx, 0x00, sizeof(HI_DNG_DATA_CTX_S));


    return HI_SUCCESS;

}

HI_S32 SAMPLE_DNG_DelInit()
{
    //release bufff
    HI_U32 i = 0;


    for (i = 0; i < MAX_FRAME_NUM; i++)
    {

        if (s_stDngCtx.s_astDngDataBuf[i].stThmInfo.bDataArrived && s_stDngCtx.s_astDngDataBuf[i].stThmInfo.stThmIFD.stImageData.pu8DataBuf)
        {
            free(s_stDngCtx.s_astDngDataBuf[i].stThmInfo.stThmIFD.stImageData.pu8DataBuf);
            s_stDngCtx.s_astDngDataBuf[i].stThmInfo.stThmIFD.stImageData.pu8DataBuf = NULL;
        }
        if (s_stDngCtx.s_astDngDataBuf[i].stScreenNailInfo.bDataArrived)
        {
            free(s_stDngCtx.s_astDngDataBuf[i].stScreenNailInfo.stScreenIFD.stImageData.pu8DataBuf);
            s_stDngCtx.s_astDngDataBuf[i].stScreenNailInfo.stScreenIFD.stImageData.pu8DataBuf = NULL;
        }
        if (s_stDngCtx.s_astDngDataBuf[i].stRawInfo.stRawIFD.stImageData.pu8DataBuf)
        {
            free(s_stDngCtx.s_astDngDataBuf[i].stRawInfo.stRawIFD.stImageData.pu8DataBuf);
            s_stDngCtx.s_astDngDataBuf[i].stRawInfo.stRawIFD.stImageData.pu8DataBuf = NULL;
        }


    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_DNG_Muxer(HI_DNG_DATA_BUFFER_S* pstDataBuf, HI_U32 u32PhotoSeq)
{
    printf("u32PhotoSeq %d \n", u32PhotoSeq);
    struct timeval begin = {0, 0};
    struct timeval end = {0, 0};
    gettimeofday(&begin, NULL);
    HI_U64 u64DiffTime = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pDNGMuxer = 0;

    s32Ret = SAMPLE_DNG_DataProcThumbNail(&(pstDataBuf->stThmInfo.stThmIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("SAMPLE_DNG_DataProcThumbNail failed  %d \n", s32Ret);
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_DNG_DataProcRaw(&(pstDataBuf->stRawInfo.stRawIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("SAMPLE_DNG_DataProcRaw failed  %d \n", s32Ret);
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_DNG_DataProcScreenNail(&(pstDataBuf->stScreenNailInfo.stScreenIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("SAMPLE_DNG_DataProcScreenNail failed  %d \n", s32Ret);
        return HI_FAILURE;
    }


    //create muxer
    HI_DNG_CONFIG_S   stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_DNG_CONFIG_S));
    stMuxerCfg.enConfigType = HI_DNG_CONFIG_TYPE_MUXER;

    snprintf(stMuxerCfg.aszFileName, DNG_MAX_FILE_NAME, "%d%s", u32PhotoSeq, "sample.dng");
    s32Ret = HI_DNG_Create(&pDNGMuxer, &stMuxerCfg);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_Create failed  %d \n", s32Ret);
        return HI_FAILURE;
    }

    //add IFD with image

    HI_U8 au8XP[8] = {0};
    pstDataBuf->stThmInfo.stThmIFD.stIFD0Info.pu8XPComment = au8XP;
    pstDataBuf->stThmInfo.stThmIFD.stIFD0Info.u32XPCommentLen = 8;

    s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stThmInfo.stThmIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_AddIFD THM IFD failed  %d \n", s32Ret);
        goto DESTROY_MUXER;
    }


    s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stRawInfo.stRawIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_AddIFD RAW IFD failed  %d \n", s32Ret);
        goto DESTROY_MUXER;
    }


    s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stScreenNailInfo.stScreenIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_AddIFD SCREEN IFD failed  %d \n", s32Ret);
        goto DESTROY_MUXER;
    }

DESTROY_MUXER:

    //destroy muxer
    if (HI_SUCCESS != HI_DNG_Destroy(pDNGMuxer))
    {
        printf("HI_DNG_AddIFD failed  %d \n", s32Ret);
    }
    gettimeofday(&end, NULL);
    u64DiffTime = (end.tv_sec * 1000000 + end.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / 1000;
    printf("muxer one  picture cost time  %lld ms\n", u64DiffTime);

    return s32Ret;
}


HI_S32 SAMPLE_DNG_MuxerOneRaw(HI_DNG_DATA_BUFFER_S* pstDataBuf, HI_U32 u32PhotoSeq)
{
    printf("u32PhotoSeq %d \n", u32PhotoSeq);
    struct timeval begin = {0, 0};
    struct timeval end = {0, 0};
    gettimeofday(&begin, NULL);
    HI_U64 u64DiffTime = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pDNGMuxer = 0;

    s32Ret = SAMPLE_DNG_DataProcOneRaw(&(pstDataBuf->stThmInfo.stThmIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("SAMPLE_DNG_DataProcOneRaw failed  %d \n", s32Ret);
        return HI_FAILURE;
    }

    //create muxer
    HI_DNG_CONFIG_S   stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_DNG_CONFIG_S));
    stMuxerCfg.enConfigType = HI_DNG_CONFIG_TYPE_MUXER;

    snprintf(stMuxerCfg.aszFileName, DNG_MAX_FILE_NAME, "%d%s", u32PhotoSeq, "sample.dng");
    s32Ret = HI_DNG_Create(&pDNGMuxer, &stMuxerCfg);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_Create failed  %d \n", s32Ret);
        return HI_FAILURE;
    }

    /*add IFD with image*/

    s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stThmInfo.stThmIFD));
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_DNG_AddIFD THM IFD failed  %d \n", s32Ret);
        goto DESTROY_MUXER;
    }



DESTROY_MUXER:

    //destroy muxer
    if (HI_SUCCESS != HI_DNG_Destroy(pDNGMuxer))
    {
        printf("HI_DNG_AddIFD failed  %d \n", s32Ret);
    }
    gettimeofday(&end, NULL);
    u64DiffTime = (end.tv_sec * 1000000 + end.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / 1000;
    printf("muxer one  picture cost time  %lld ms\n", u64DiffTime);

    return s32Ret;
}


HI_S32 SAMPLE_DNG_TakePicture(HI_U32 u32DngPhotoCount)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    struct timeval begin = {0, 0};
    struct timeval end = {0, 0};
    HI_U64 u64DiffTime = 0;
    gettimeofday(&begin, NULL);

    if (u32DngPhotoCount > MAX_FRAME_NUM)
    {
        printf("================u32DngPhotoCount:%d reach max: %d error\n", u32DngPhotoCount, MAX_FRAME_NUM);
        return HI_FAILURE;

    }

    s32Ret = SAMPLE_DNG_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("================SAMPLE_DNG_Init fail\n");
        return HI_FAILURE;
    }

    //get ccm every time

    for (i = 0; i < u32DngPhotoCount; i++)
    {

        printf("-------%d -----------------start SAMPLE_DNG_Muxer\n", i);


        s32Ret = SAMPLE_DNG_Muxer(&(s_stDngCtx.s_astDngDataBuf[i]), i);
        if (HI_SUCCESS != s32Ret )
        {
            printf("SAMPLE_DNG_Muxer %d error \n", i);
        }


    }


    SAMPLE_DNG_DelInit();

    gettimeofday(&end, NULL);
    u64DiffTime = (end.tv_sec * 1000000 + end.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / 1000;
    printf("take picture  total  cost time %lld ms\n", u64DiffTime);

    return s32Ret;

}

HI_S32 SAMPLE_DNG_NoThm()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    struct timeval begin = {0, 0};
    struct timeval end = {0, 0};
    HI_U64 u64DiffTime = 0;
    gettimeofday(&begin, NULL);



    s32Ret = SAMPLE_DNG_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("================SAMPLE_DNG_Init fail\n");
        return HI_FAILURE;
    }

    //get ccm every time


    printf("-------%d -----------------start SAMPLE_DNG_Muxer\n", i);


    s32Ret = SAMPLE_DNG_MuxerOneRaw(&(s_stDngCtx.s_astDngDataBuf[0]), 0);
    if (HI_SUCCESS != s32Ret )
    {
        printf("SAMPLE_DNG_Muxer %d error \n", i);
    }


    SAMPLE_DNG_DelInit();

    gettimeofday(&end, NULL);
    u64DiffTime = (end.tv_sec * 1000000 + end.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec) / 1000;
    printf("take picture  total  cost time %lld ms\n", u64DiffTime);

    return s32Ret;

}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main()
#endif
{
    HI_CHAR   InputCmd[32];
    HI_S32 s32InputNumber = 0, i = 0;


    printf("please input 'h' to get help or 'q' to quit!\n");

    while (1)
    {
        memset(InputCmd, 0, 30);

        printf("<input cmd:>");
        if (NULL == fgets((char*)InputCmd, 30, stdin))
        {
            print_help_info();
            continue;
        }

        /*filter backspace key*/
        for (i = 0; InputCmd[i] != '\0' && InputCmd[i] != 0xa; i++);

        if (InputCmd[i] == 0xa) { InputCmd[i] = '\0'; }

        if ( (0 == strcmp("q", (char*)InputCmd))
             || (0 == strcmp("Q", (char*)InputCmd))
             || (0 == strcmp("quit", (char*)InputCmd))
             || (0 == strcmp("QUIT", (char*)InputCmd)) )
        {
            printf("quit the program!\n");
            break;
        }

        if ( (0 == strcmp("help", (char*)InputCmd)) ||
             (0 == strcmp("h", (char*)InputCmd) ))
        {
            print_help_info();
            continue;
        }

        s32InputNumber = atoi(InputCmd);

        printf("[%s.%d]\n", __FUNCTION__, __LINE__);

        switch (s32InputNumber)
        {

            case 1:/* single snap dng  */
                SAMPLE_DNG_TakePicture(1);
                break;
            case 2:/* multi snap dng  */
                SAMPLE_DNG_TakePicture(5);
                break;
            case 3:/*no thm dng*/
                SAMPLE_DNG_NoThm();

                break;

            default:
            {
                printf("input cmd: %s is error \n", InputCmd);
                print_help_info();
                break;
            }
        }
    }




    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
