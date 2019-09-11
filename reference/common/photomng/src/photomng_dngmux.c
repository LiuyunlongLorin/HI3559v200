/**
 * @file    photomng_dngmux.c
 * @brief   dng muxer adapt.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/2
 * @version   1.0

 */
#include "photomng_dngmux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "PHOTOMNG"

#define DIV(numerator,denominator)  ((0!=denominator)?((numerator*1.0)/(denominator)):0)
#define PHOTOMNG_U32_TO_FLOAT(u32Val) ((HI_FLOAT)((0!=((u32Val)&0xFFFF))?(((u32Val>>16)&0xFFFF)*1.0/((u32Val)&0xFFFF)):0))

static HI_S32 PHOTOMNG_DNG_SetGPSIFD(HI_DNG_MUXER_GPS_INFO_S* pstGPSInfo)
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

static HI_S32 PHOTOMNG_DNG_SetExifIFD(HI_DNG_MUXER_IFD_INFO_S* pstThmIFD, JPEG_DCF_S* pstJpegDcf)
{
    if (pstJpegDcf)
    {
        pstThmIFD->stIFD0Info.bHasExifIFD = HI_TRUE;
        snprintf(pstThmIFD->stIFD0Info.stExifInfo.aszCaptureTime, DNG_MUXER_DATETIME_LENGTH, "%s", pstJpegDcf->au8CaptureTime);
        snprintf(pstThmIFD->stIFD0Info.stExifInfo.aszDateTimeOriginal, DNG_MUXER_DATETIME_LENGTH, "%s", pstJpegDcf->au8CaptureTime);
        pstThmIFD->stIFD0Info.stExifInfo.u16Flash = pstJpegDcf->bFlash;
        pstThmIFD->stIFD0Info.stExifInfo.u16ISOSpeedRatings = pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32ISOSpeedRatings;
        pstThmIFD->stIFD0Info.stExifInfo.fDigitalZoomRatio = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->u32DigitalZoomRatio);
        pstThmIFD->stIFD0Info.stExifInfo.fExposureBiasValue = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32ExposureBiasValue);
        pstThmIFD->stIFD0Info.stExifInfo.fExposureTime = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32ExposureTime);
        pstThmIFD->stIFD0Info.stExifInfo.fFNumber = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32FNumber);
        pstThmIFD->stIFD0Info.stExifInfo.fFocalLength = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u32FocalLength);
        pstThmIFD->stIFD0Info.stExifInfo.fMaxApertureValue = PHOTOMNG_U32_TO_FLOAT(pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32MaxApertureValue);
        pstThmIFD->stIFD0Info.stExifInfo.u16Contrast = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8Contrast;
        pstThmIFD->stIFD0Info.stExifInfo.u16CustomRendered = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8CustomRendered;
        pstThmIFD->stIFD0Info.stExifInfo.u16ExposureMode = pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u8ExposureMode;
        pstThmIFD->stIFD0Info.stExifInfo.u16ExposureProgram = pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u8ExposureProgram;
        pstThmIFD->stIFD0Info.stExifInfo.u16FocalLengthIn35mmFilm = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8FocalLengthIn35mmFilm;
        pstThmIFD->stIFD0Info.stExifInfo.u16GainControl = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8GainControl;
        pstThmIFD->stIFD0Info.stExifInfo.u16LightSource = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8LightSource;
        pstThmIFD->stIFD0Info.stExifInfo.u16MeteringMode = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8MeteringMode;
        pstThmIFD->stIFD0Info.stExifInfo.u16Saturation = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8Saturation;
        pstThmIFD->stIFD0Info.stExifInfo.u16SceneCaptureType = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8SceneCaptureType;
        pstThmIFD->stIFD0Info.stExifInfo.u8SceneType = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8SceneType;
        pstThmIFD->stIFD0Info.stExifInfo.u16Sharpness = pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.u8Sharpness;
        pstThmIFD->stIFD0Info.stExifInfo.u16WhiteBalance = pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u8WhiteBalance;
    }

    return HI_SUCCESS;
}


HI_S32 PHOTOMNG_DNG_SetThmIFD(HI_DNG_MUXER_IFD_INFO_S*  pstThmIFD,
                              HI_U32 u32Width,
                              HI_U32 u32Height,
                              DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                              DNG_IMAGE_STATIC_INFO_S* pstStaticInfo,
                              JPEG_DCF_S* pstJpegDcf,
                              HI_PHOTOMNG_DNG_USERINFO_S* pstDNGUserInfo)
{
    //set exif info
    HI_U32 i = 0;
    PHOTOMNG_DNG_SetExifIFD(pstThmIFD, pstJpegDcf);
    snprintf(pstThmIFD->stIFD0Info.aszDateTime, DNG_MUXER_DATETIME_LENGTH, "%s", pstThmIFD->stIFD0Info.stExifInfo.aszCaptureTime);
    snprintf(pstThmIFD->stIFD0Info.aszMake, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Make);
    snprintf(pstThmIFD->stIFD0Info.aszModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Model);
    snprintf(pstThmIFD->stIFD0Info.aszSoftware, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Software);

    //HI_FLOAT afColorMatrix1[DNG_MUXER_MATRIX_SIZE] = {1.005913973, -0.3384189901, -0.2515580055, -0.2034379985, 1.081694007, 0.3984909949, -0.04316899881, 0.2171149995, 1.196020008};
    //HI_FLOAT afColorMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.982698977 , -0.2974979874 , -0.08117099809 , -0.3391309974 , 1.113445044 , 0.2588509914 , -0.05263899968 , 0.1755000051, 0.5555719731};
    //HI_FLOAT afCameraCalibration1[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};
    //HI_FLOAT afCameraCalibration2[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};

    //HI_FLOAT afForwadMatrix1[DNG_MUXER_MATRIX_SIZE] = {0.6308299897, 0.2701559957, 0.06322500066, 0.1345120068, 1.048423052, -0.1829790022, 0.003497999629, -0.4157229957, 1.237597943};
    //HI_FLOAT afForwadMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.6535180209 , 0.2953670022, 0.01532499912 , 0.1991869953, 1.059399009 , -0.2586300072 , -0.001023999881 , -0.3067879972, 1.133185983};

    for (i = 0; i < DNG_MUXER_MATRIX_SIZE; i++)
    {
        pstThmIFD->stIFD0Info.stStaticInfo.afCameraCalibration1[i] = DIV(pstStaticInfo->astCameraCalibration1[i].s32Numerator, pstStaticInfo->astCameraCalibration1[i].s32Denominator);
        pstThmIFD->stIFD0Info.stStaticInfo.afCameraCalibration2[i] = DIV(pstStaticInfo->astCameraCalibration2[i].s32Numerator, pstStaticInfo->astCameraCalibration2[i].s32Denominator);
        pstThmIFD->stIFD0Info.stStaticInfo.afColorMatrix1[i] = DIV(pstStaticInfo->astColorMatrix1[i].s32Numerator, pstStaticInfo->astColorMatrix1[i].s32Denominator);
        pstThmIFD->stIFD0Info.stStaticInfo.afColorMatrix2[i] = DIV(pstStaticInfo->astColorMatrix2[i].s32Numerator, pstStaticInfo->astColorMatrix2[i].s32Denominator);
        pstThmIFD->stIFD0Info.stStaticInfo.afForwadMatrix1[i] = DIV(pstStaticInfo->astForwadMatrix1[i].s32Numerator, pstStaticInfo->astForwadMatrix1[i].s32Denominator);
        pstThmIFD->stIFD0Info.stStaticInfo.afForwadMatrix2[i] = DIV(pstStaticInfo->astForwadMatrix2[i].s32Numerator, pstStaticInfo->astForwadMatrix2[i].s32Denominator);
    }



    pstThmIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant1 = pstStaticInfo->u8CalibrationIlluminant1;
    pstThmIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant2 = pstStaticInfo->u8CalibrationIlluminant2;

    //HI_FLOAT afAsShotNeutral_def[DNG_MUXER_CFACOLORPLANE] = {0.6355748176, 1, 0.4075707494};
    //HI_DOUBLE adNoiseProfile[DNG_MUXER_NP_SIZE] = {2e-005, 4.5e-007, 2e-005, 4.5e-007, 2e-005 , 4.5e-007};

    if (pstDynInfo)
    {
        for (i = 0; i < DNG_MUXER_NP_SIZE; i++)
        {
            pstThmIFD->stIFD0Info.stDynamicInfo.adNoiseProfile[i] = pstDynInfo->adNoiseProfile[i];
        }

        for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
        {
            pstThmIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] = DIV(pstDynInfo->astAsShotNeutral[i].u32Numerator, pstDynInfo->astAsShotNeutral[i].u32Denominator);
            //if(1==pstThmIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i])
            //{
            //    pstThmIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] = afAsShotNeutral_def[i];
            //}
        }
    }

    pstThmIFD->u32Height = u32Height;
    pstThmIFD->u32Width = u32Width;
    pstThmIFD->u16BitPerSample = 8;
    pstThmIFD->enImageType = HI_DNG_MUXER_IMAGE_IFD0;
    pstThmIFD->u16Compression = COMPRESSION_JPEG;
    pstThmIFD->u16Photometric = PHOTOMETRIC_YCBCR;
    pstThmIFD->u16SamplePerPixel = 3;
    pstThmIFD->u32SubfileType = 1;
    pstThmIFD->stIFD0Info.u32SubIFDs = 2;
    pstThmIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;//storage organization
    pstThmIFD->stIFD0Info.u8Orientation = ORIENTATION_TOPLEFT;
    //set by user
    if(pstDNGUserInfo)
    {
        snprintf(pstThmIFD->stIFD0Info.aszCameraSerialNumber, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstDNGUserInfo->szCameraSerialNumber);
        snprintf(pstThmIFD->stIFD0Info.aszUniqueCameraModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstDNGUserInfo->szUniqueCameraModel);
        pstThmIFD->stIFD0Info.pfProfileHueSatMapData1 = pstDNGUserInfo->pfProfileHueSatMapData1;
        pstThmIFD->stIFD0Info.u32Map1Len = pstDNGUserInfo->u32Map1Len;
        pstThmIFD->stIFD0Info.pfProfileHueSatMapData2 = pstDNGUserInfo->pfProfileHueSatMapData2;
        pstThmIFD->stIFD0Info.u32Map2Len = pstDNGUserInfo->u32Map2Len;
        memcpy(pstThmIFD->stIFD0Info.afLensInfo,pstDNGUserInfo->afLensInfo,DNG_MUXER_LENINFO_LEN*sizeof(HI_FLOAT));
        pstThmIFD->stIFD0Info.pu8DNGPrivData = pstDNGUserInfo->pu8DNGPrivData;
        pstThmIFD->stIFD0Info.u32DNGPrivDataLen = pstDNGUserInfo->u32DNGPrivDataLen;
    }

    pstThmIFD->stIFD0Info.bHasGpsIFD = HI_TRUE;
    PHOTOMNG_DNG_SetGPSIFD(&(pstThmIFD->stIFD0Info.stGpsInfo));

    return HI_SUCCESS;
}

HI_S32 PHOTOMNG_DNG_SetRawIFD(HI_DNG_MUXER_IFD_INFO_S* pstRawIFD,
                              DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                              DNG_IMAGE_STATIC_INFO_S* pstStaticInfo)
{
    HI_U32 i = 0;
    //set info for raw ifd

    for (i = 0; i < DNG_MUXER_ISP_BAYER_CHN; i++)
    {
        if (pstDynInfo)
        {
            pstRawIFD->stRawIFDInfo.stRawFormat.af32BlackLevel[i] = pstDynInfo->au32BlackLevel[i];
        }

        pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPattern[i] = pstStaticInfo->stDngRawFormat.au8CfaPattern[i];
    }

    for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
    {
        pstRawIFD->stRawIFDInfo.stRawFormat.au8CfaPlaneColor[i] = pstStaticInfo->stDngRawFormat.au8CfaPlaneColor[i];
    }

    pstRawIFD->stRawIFDInfo.stRawFormat.enCfaLayout = (HI_DNG_MUXER_CFALAYOUT_TYPE_E)pstStaticInfo->stDngRawFormat.enCfaLayout;
    pstRawIFD->stRawIFDInfo.stRawFormat.u32WhiteLevel = pstStaticInfo->stDngRawFormat.u32WhiteLevel;

    pstRawIFD->stRawIFDInfo.stRawFormat.au16CfaRepeatPatternDim[0] = pstStaticInfo->stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimRows;
    pstRawIFD->stRawIFDInfo.stRawFormat.au16CfaRepeatPatternDim[1] = pstStaticInfo->stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimCols;

    pstRawIFD->stRawIFDInfo.stRawFormat.au16BlcRepeatDim[0] = pstStaticInfo->stDngRawFormat.stBlcRepeatDim.u16BlcRepeatRows;
    pstRawIFD->stRawIFDInfo.stRawFormat.au16BlcRepeatDim[1] = pstStaticInfo->stDngRawFormat.stBlcRepeatDim.u16BlcRepeatCols;

    pstRawIFD->stRawIFDInfo.stRawFormat.afDefaultScale[0] = DIV(pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Numerator, pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Denominator);
    pstRawIFD->stRawIFDInfo.stRawFormat.afDefaultScale[1] = DIV(pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Numerator, pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Denominator);

    //we muxerd raw into dng use bitsample 16;the u16OriginalBitpersample is used for transform raw data into 16 bit
    pstRawIFD->u16BitPerSample = pstStaticInfo->stDngRawFormat.u8BitsPerSample;

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

HI_S32 PHOTOMNG_DNG_SetScreenIFD(HI_DNG_MUXER_IFD_INFO_S* pstScreenIFD,
                                 HI_U32 u32Width,
                                 HI_U32 u32Height)
{

    pstScreenIFD->u32Height = u32Height;
    pstScreenIFD->u32Width = u32Width;

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



HI_S32 PHOTOMNG_DNG_SetIFD0(HI_DNG_MUXER_IFD_INFO_S* pstIFD,
                            DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                            DNG_IMAGE_STATIC_INFO_S* pstStaticInfo,
                            JPEG_DCF_S* pstJpegDcf,
                            HI_PHOTOMNG_DNG_USERINFO_S* pstDNGUserInfo)
{
    //set exif info
    HI_U32 i = 0;
    PHOTOMNG_DNG_SetExifIFD(pstIFD,pstJpegDcf);
    snprintf(pstIFD->stIFD0Info.aszDateTime, DNG_MUXER_DATETIME_LENGTH, "%s", pstIFD->stIFD0Info.stExifInfo.aszCaptureTime);
    snprintf(pstIFD->stIFD0Info.aszMake, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Make);
    snprintf(pstIFD->stIFD0Info.aszModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Model);
    snprintf(pstIFD->stIFD0Info.aszSoftware, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstJpegDcf->stIspDCFInfo.stIspDCFConstInfo.au8Software);

    //HI_FLOAT afColorMatrix1[DNG_MUXER_MATRIX_SIZE] = {1.005913973, -0.3384189901, -0.2515580055, -0.2034379985, 1.081694007, 0.3984909949, -0.04316899881, 0.2171149995, 1.196020008};
    //HI_FLOAT afColorMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.982698977 , -0.2974979874 , -0.08117099809 , -0.3391309974 , 1.113445044 , 0.2588509914 , -0.05263899968 , 0.1755000051, 0.5555719731};
    //HI_FLOAT afCameraCalibration1[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};
    //HI_FLOAT afCameraCalibration2[DNG_MUXER_MATRIX_SIZE] = {1, 0, 0, 0 , 1 , 0 , 0, 0, 1};

    //HI_FLOAT afForwadMatrix1[DNG_MUXER_MATRIX_SIZE] = {0.6308299897, 0.2701559957, 0.06322500066, 0.1345120068, 1.048423052, -0.1829790022, 0.003497999629, -0.4157229957, 1.237597943};
    //HI_FLOAT afForwadMatrix2[DNG_MUXER_MATRIX_SIZE] = {0.6535180209 , 0.2953670022, 0.01532499912 , 0.1991869953, 1.059399009 , -0.2586300072 , -0.001023999881 , -0.3067879972, 1.133185983};

    for (i = 0; i < DNG_MUXER_MATRIX_SIZE; i++)
    {
        pstIFD->stIFD0Info.stStaticInfo.afCameraCalibration1[i] = DIV(pstStaticInfo->astCameraCalibration1[i].s32Numerator, pstStaticInfo->astCameraCalibration1[i].s32Denominator);
        pstIFD->stIFD0Info.stStaticInfo.afCameraCalibration2[i] = DIV(pstStaticInfo->astCameraCalibration2[i].s32Numerator, pstStaticInfo->astCameraCalibration2[i].s32Denominator);
        pstIFD->stIFD0Info.stStaticInfo.afColorMatrix1[i] = DIV(pstStaticInfo->astColorMatrix1[i].s32Numerator, pstStaticInfo->astColorMatrix1[i].s32Denominator);
        pstIFD->stIFD0Info.stStaticInfo.afColorMatrix2[i] = DIV(pstStaticInfo->astColorMatrix2[i].s32Numerator, pstStaticInfo->astColorMatrix2[i].s32Denominator);
        pstIFD->stIFD0Info.stStaticInfo.afForwadMatrix1[i] = DIV(pstStaticInfo->astForwadMatrix1[i].s32Numerator, pstStaticInfo->astForwadMatrix1[i].s32Denominator);
        pstIFD->stIFD0Info.stStaticInfo.afForwadMatrix2[i] = DIV(pstStaticInfo->astForwadMatrix2[i].s32Numerator, pstStaticInfo->astForwadMatrix2[i].s32Denominator);
    }



    pstIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant1 = pstStaticInfo->u8CalibrationIlluminant1;
    pstIFD->stIFD0Info.stStaticInfo.u8CalibrationIlluminant2 = pstStaticInfo->u8CalibrationIlluminant2;

    //HI_FLOAT afAsShotNeutral_def[DNG_MUXER_CFACOLORPLANE] = {0.6355748176, 1, 0.4075707494};
    //HI_DOUBLE adNoiseProfile[DNG_MUXER_NP_SIZE] = {2e-005, 4.5e-007, 2e-005, 4.5e-007, 2e-005 , 4.5e-007};

    if (pstDynInfo)
    {
        for (i = 0; i < DNG_MUXER_NP_SIZE; i++)
        {
            pstIFD->stIFD0Info.stDynamicInfo.adNoiseProfile[i] = pstDynInfo->adNoiseProfile[i];
        }

        for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
        {
            pstIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] = DIV(pstDynInfo->astAsShotNeutral[i].u32Numerator, pstDynInfo->astAsShotNeutral[i].u32Denominator);
            //if(1==pstIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i])
            //{
            //    pstIFD->stIFD0Info.stDynamicInfo.afAsShotNeutral[i] = afAsShotNeutral_def[i];
            //}
        }
    }

    pstIFD->u16BitPerSample = pstStaticInfo->stDngRawFormat.u8BitsPerSample;
    pstIFD->enImageType = HI_DNG_MUXER_IMAGE_IFD0;
    pstIFD->u16Compression = COMPRESSION_NONE;
    pstIFD->u16Photometric = PHOTOMETRIC_CFA;
    pstIFD->u16SamplePerPixel = 1;
    pstIFD->u32SubfileType = 0;

    pstIFD->stIFD0Info.u32SubIFDs = 0;
    pstIFD->u16PlanarConfig = PLANARCONFIG_CONTIG;//storage organization
    pstIFD->stIFD0Info.u8Orientation = ORIENTATION_TOPLEFT;
    //set by user
    if(pstDNGUserInfo)
    {
        snprintf(pstIFD->stIFD0Info.aszCameraSerialNumber, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstDNGUserInfo->szCameraSerialNumber);
        snprintf(pstIFD->stIFD0Info.aszUniqueCameraModel, DNG_MUXER_DCF_DESCRIPTION_LENGTH, "%s", pstDNGUserInfo->szUniqueCameraModel);
        pstIFD->stIFD0Info.pfProfileHueSatMapData1 = pstDNGUserInfo->pfProfileHueSatMapData1;
        pstIFD->stIFD0Info.u32Map1Len = pstDNGUserInfo->u32Map1Len;
        pstIFD->stIFD0Info.pfProfileHueSatMapData2 = pstDNGUserInfo->pfProfileHueSatMapData2;
        pstIFD->stIFD0Info.u32Map2Len = pstDNGUserInfo->u32Map2Len;
        memcpy(pstIFD->stIFD0Info.afLensInfo,pstDNGUserInfo->afLensInfo,DNG_MUXER_LENINFO_LEN*sizeof(HI_FLOAT));
        pstIFD->stIFD0Info.pu8DNGPrivData = pstDNGUserInfo->pu8DNGPrivData;
        pstIFD->stIFD0Info.u32DNGPrivDataLen = pstDNGUserInfo->u32DNGPrivDataLen;
    }

    pstIFD->stIFD0Info.bHasGpsIFD = HI_FALSE;
    //PHOTOMNG_DNG_SetGPSIFD(&(pstIFD->stIFD0Info.stGpsInfo));

    i = 0;

    for (i = 0; i < DNG_MUXER_ISP_BAYER_CHN; i++)
    {
        if (pstDynInfo)
        {
            pstIFD->stIFD0Info.stRawFormat.af32BlackLevel[i] = pstDynInfo->au32BlackLevel[i];
        }
        pstIFD->stIFD0Info.stRawFormat.au8CfaPattern[i] = pstStaticInfo->stDngRawFormat.au8CfaPattern[i];

    }

    for (i = 0; i < DNG_MUXER_CFACOLORPLANE; i++)
    {
        pstIFD->stIFD0Info.stRawFormat.au8CfaPlaneColor[i] = pstStaticInfo->stDngRawFormat.au8CfaPlaneColor[i];
    }

    pstIFD->stIFD0Info.stRawFormat.enCfaLayout = (HI_DNG_MUXER_CFALAYOUT_TYPE_E)pstStaticInfo->stDngRawFormat.enCfaLayout;
    pstIFD->stIFD0Info.stRawFormat.u32WhiteLevel = pstStaticInfo->stDngRawFormat.u32WhiteLevel;

    pstIFD->stIFD0Info.stRawFormat.au16CfaRepeatPatternDim[0] = pstStaticInfo->stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimRows;
    pstIFD->stIFD0Info.stRawFormat.au16CfaRepeatPatternDim[1] = pstStaticInfo->stDngRawFormat.stCfaRepeatPatternDim.u16RepeatPatternDimCols;

    pstIFD->stIFD0Info.stRawFormat.au16BlcRepeatDim[0] = pstStaticInfo->stDngRawFormat.stBlcRepeatDim.u16BlcRepeatRows;
    pstIFD->stIFD0Info.stRawFormat.au16BlcRepeatDim[1] = pstStaticInfo->stDngRawFormat.stBlcRepeatDim.u16BlcRepeatCols;

    pstIFD->stIFD0Info.stRawFormat.afDefaultScale[0] = DIV(pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Numerator, pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleH.u32Denominator);
    pstIFD->stIFD0Info.stRawFormat.afDefaultScale[1] = DIV(pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Numerator, pstStaticInfo->stDngRawFormat.stDefaultScale.stDefaultScaleV.u32Denominator);


    return HI_SUCCESS;
}

HI_S32 PHOTOMNG_DNG_Muxer(PHOTOMNG_DNG_DATA_BUFFER_S* pstDataBuf, HI_CHAR* pszFileName, HI_BOOL bOneRaw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pDNGMuxer = 0;

    //create muxer
    HI_DNG_CONFIG_S   stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_DNG_CONFIG_S));
    stMuxerCfg.enConfigType = HI_DNG_CONFIG_TYPE_MUXER;
    snprintf(stMuxerCfg.aszFileName, DNG_MAX_FILE_NAME, pszFileName);
    s32Ret = HI_DNG_Create(&pDNGMuxer, &stMuxerCfg);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_DNG_Create failed  %d \n", s32Ret);
        return s32Ret;
    }

    //add IFD with image
    if (HI_TRUE == bOneRaw)
    {
        s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stRawInfo.stRawIFD));

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_DNG_AddIFD THM IFD failed  %d \n", s32Ret);
            goto DESTROY_MUXER;
        }
    }
    else
    {
        s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stThmInfo.stThmIFD));

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_DNG_AddIFD THM IFD failed  %d \n", s32Ret);
            goto DESTROY_MUXER;
        }

        s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stRawInfo.stRawIFD));

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_DNG_AddIFD RAW IFD failed  %d \n", s32Ret);
            goto DESTROY_MUXER;
        }

        s32Ret = HI_DNG_AddIFD(pDNGMuxer, &(pstDataBuf->stScreenNailInfo.stScreenIFD));

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_DNG_AddIFD SCREEN IFD failed  %d \n", s32Ret);
            goto DESTROY_MUXER;
        }
    }

DESTROY_MUXER:
    //destroy muxer
    s32Ret = HI_DNG_Destroy(pDNGMuxer);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_DNG_Destroy failed  %d \n", s32Ret);
    }

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
