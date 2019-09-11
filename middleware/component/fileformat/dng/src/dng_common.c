#include "securec.h"
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include "hi_mw_type.h"
#include "hi_list.h"
#include "tiffio.h"
#include "hi_dng_err.h"
#include "hi_dng.h"
#include "dng_common.h"
#include "dng_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DNG_MAX_WIDTH (30000)

static HI_S32 DNG_MUXER_ConvertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, HI_U16 u16BitPerSample,
                                        HI_U16 *pu16OutData)
{
    HI_S32 i = 0;
    HI_S32 u32Tmp = 0;
    HI_S32 s32OutCnt = 0;
    ;
    HI_U32 u32Val = 0;
    HI_U64 u64Val = 0;
    HI_U8 *pu8Tmp = NULL;

    switch (u16BitPerSample) {
        case 10: {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++) {
                /* byte4 byte3 byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 5 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32);

                pu16OutData[s32OutCnt++] = u64Val & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 10) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 20) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 30) & 0x3ff;
            }
        }
        break;

        case 12: {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++) {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;
                u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
            }
        }
        break;

        case 14: {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++) {
                pu8Tmp = pu8Data + 7 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32) +
                         ((HI_U64)pu8Tmp[5] << 40) + ((HI_U64)pu8Tmp[6] << 48);

                pu16OutData[s32OutCnt++] = u64Val & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 14) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 28) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 42) & 0x3fff;
            }
        }
        break;

        default:
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsuport raw u16BitPerSample: %d\n", u16BitPerSample);
            return -1;
            break;
    }

    return s32OutCnt;
}

static HI_S32 DNG_MUXER_WriteRawData(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    HI_U16 *pu16Data = NULL;
    HI_U32 u32Row = 0;
    HI_U8 *rowdata = NULL;
    HI_U32 u32RowLen = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    HI_U8 *pu8TmpBuf = NULL;

    pu8TmpBuf = pstIFDInfo->stImageData.pu8DataBuf;
    u32Width = pstIFDInfo->u32Width;
    u32Height = pstIFDInfo->u32Height;

    if (8 == pstIFDInfo->u16BitPerSample) {
        u32RowLen = u32Width;
    } else {
        u32RowLen = u32Width * 2;
    }

    rowdata = (HI_U8 *)malloc(u32RowLen);
    if (NULL == rowdata) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData malloc rowdata buf fail \n");
        return HI_FAILURE;
    }
    memset_s(rowdata, u32RowLen, 0x00, u32RowLen);

    if ((8 != pstIFDInfo->u16BitPerSample) && (16 != pstIFDInfo->u16BitPerSample) && (pstIFDInfo->u32Width > 0)) {
        pu16Data = (HI_U16 *)malloc(u32Width * 2);
        if (NULL == pu16Data) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData pu16Data Malloc error\n");
            s32Ret = HI_FAILURE;
            goto REL_ROW;
        }
        memset_s(pu16Data, u32Width * 2, 0x00, u32Width * 2);
    }

    for (u32Row = 0; u32Row < u32Height; u32Row++) {
        if (8 == pstIFDInfo->u16BitPerSample) {
            memcpy_s(rowdata, u32RowLen, pu8TmpBuf, u32RowLen);
            if (-1 == TIFFWriteScanline(tif, rowdata, u32Row, 0)) {
                DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData TIFFWriteScanline error\n");
                s32Ret = HI_FAILURE;
                goto REL_DATA;
            }
        } else if (16 == pstIFDInfo->u16BitPerSample) {
            memcpy_s(rowdata, u32RowLen, pu8TmpBuf, u32RowLen);
            if (-1 == TIFFWriteScanline(tif, rowdata, u32Row, 0)) {
                DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData TIFFWriteScanline  error\n");
                s32Ret = HI_FAILURE;
                goto REL_DATA;
            }
        } else {
            if (pu16Data) {
                if (-1 == DNG_MUXER_ConvertBitPixel(pu8TmpBuf, u32Width, pstIFDInfo->u16BitPerSample, pu16Data)) {
                    DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData DNG_MUXER_ConvertBitPixel fail \n");
                    s32Ret = HI_FAILURE;
                    goto REL_DATA;
                }
                memcpy_s(rowdata, u32RowLen, pu16Data, u32RowLen);

                if (-1 == TIFFWriteScanline(tif, rowdata, u32Row, 0)) {
                    DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_WriteRawData TIFFWriteScanline fail \n");
                    s32Ret = HI_FAILURE;
                    goto REL_DATA;
                }
            }
        }

        pu8TmpBuf += pstIFDInfo->stImageData.u32Stride;
    }

REL_DATA:
    if (pu16Data) {
        free(pu16Data);
        pu16Data = NULL;
    }

REL_ROW:
    if (rowdata) {
        free(rowdata);
        rowdata = NULL;
    }

    return s32Ret;
}

static HI_S32 DNG_MUXER_SetNoramlTag(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    TIFFSetField(tif, TIFFTAG_SUBFILETYPE, pstIFDInfo->u32SubfileType);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, pstIFDInfo->u16SamplePerPixel);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, pstIFDInfo->u16Compression);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, pstIFDInfo->u32Width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, pstIFDInfo->u32Height);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, pstIFDInfo->u16Photometric);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, pstIFDInfo->u16PlanarConfig);

    if (COMPRESSION_JPEG == pstIFDInfo->u16Compression) {
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, pstIFDInfo->u16BitPerSample);
        TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
    } else if (COMPRESSION_NONE == pstIFDInfo->u16Compression) {  // raw
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
    }

    return HI_SUCCESS;
}

static HI_S32 DNG_MUXER_SetExifIFD(TIFF *tif, HI_DNG_MUXER_EXIF_INFO_S *pstExifInfo, HI_U64 *pu64ExifOffset)
{
    HI_U64 u64ExifOffset = 0;

    if (0 != TIFFCreateEXIFDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetExifIFD fail TIFFCreateEXIFDirectory error\n");
        return HI_FAILURE;
    }

    TIFFSetField(tif, EXIFTAG_ISOSPEEDRATINGS, 1, &pstExifInfo->u16ISOSpeedRatings);
    TIFFSetField(tif, EXIFTAG_EXPOSUREBIASVALUE, pstExifInfo->fExposureBiasValue);
    TIFFSetField(tif, EXIFTAG_EXPOSURETIME, pstExifInfo->fExposureTime);
    TIFFSetField(tif, EXIFTAG_FNUMBER, pstExifInfo->fFNumber);
    TIFFSetField(tif, EXIFTAG_FOCALLENGTH, pstExifInfo->fFocalLength);
    TIFFSetField(tif, EXIFTAG_EXPOSUREMODE, pstExifInfo->u16ExposureMode);
    TIFFSetField(tif, EXIFTAG_EXPOSUREPROGRAM, pstExifInfo->u16ExposureProgram);
    TIFFSetField(tif, EXIFTAG_LIGHTSOURCE, pstExifInfo->u16LightSource);
    TIFFSetField(tif, EXIFTAG_METERINGMODE, pstExifInfo->u16MeteringMode);
    TIFFSetField(tif, EXIFTAG_SATURATION, pstExifInfo->u16Saturation);
    TIFFSetField(tif, EXIFTAG_SHARPNESS, pstExifInfo->u16Sharpness);
    TIFFSetField(tif, EXIFTAG_WHITEBALANCE, pstExifInfo->u16WhiteBalance);

    TIFFSetField(tif, EXIFTAG_DATETIMEORIGINAL, pstExifInfo->aszDateTimeOriginal);
    TIFFSetField(tif, EXIFTAG_DATETIMEDIGITIZED, pstExifInfo->aszCaptureTime);
    TIFFSetField(tif, EXIFTAG_FLASH, pstExifInfo->u16Flash);
    TIFFSetField(tif, EXIFTAG_DIGITALZOOMRATIO, pstExifInfo->fDigitalZoomRatio);
    TIFFSetField(tif, EXIFTAG_MAXAPERTUREVALUE, pstExifInfo->fMaxApertureValue);
    TIFFSetField(tif, EXIFTAG_CONTRAST, pstExifInfo->u16Contrast);
    TIFFSetField(tif, EXIFTAG_CUSTOMRENDERED, pstExifInfo->u16CustomRendered);
    TIFFSetField(tif, EXIFTAG_FOCALLENGTHIN35MMFILM, pstExifInfo->u16FocalLengthIn35mmFilm);
    TIFFSetField(tif, EXIFTAG_GAINCONTROL, pstExifInfo->u16GainControl);

    TIFFSetField(tif, EXIFTAG_SCENECAPTURETYPE, pstExifInfo->u16SceneCaptureType);
    TIFFSetField(tif, EXIFTAG_SCENETYPE, pstExifInfo->u8SceneType);

    if (1 != TIFFWriteCustomDirectory(tif, (uint64 *)&u64ExifOffset)) {  //
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetExifIFD fail TIFFWriteCustomDirectory error\n");
        return HI_FAILURE;
    }

    *pu64ExifOffset = u64ExifOffset;
    return HI_SUCCESS;
}

static HI_S32 DNG_MUXER_SetGpsIFD(TIFF *tif, HI_DNG_MUXER_GPS_INFO_S *pstGpsInfo, HI_U64 *pu64GpsOffset)
{
    HI_U64 u64GpsOffset = 0;

    if (0 != TIFFCreateDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetGpsIFD TIFFCreateDirectory failed.\n");
        return HI_FAILURE;
    }
    TIFFSetField(tif, GPSTAG_VERSIONID, pstGpsInfo->au8GpsVersion);
    TIFFSetField(tif, GPSTAG_ALTITUDEREF, pstGpsInfo->u8GPSAltitudeRef);
    TIFFSetField(tif, GPSTAG_ALTITUDE, pstGpsInfo->fGPSAltitude);
    TIFFSetField(tif, GPSTAG_LATITUDE, pstGpsInfo->afGPSLatitude);
    TIFFSetField(tif, GPSTAG_LONGITUDE, pstGpsInfo->afGPSLongitude);
    TIFFSetField(tif, GPSTAG_LATITUDEREF, &pstGpsInfo->chGPSLatitudeRef);
    TIFFSetField(tif, GPSTAG_LONGITUDEREF, &pstGpsInfo->chGPSLongitudeRef);

    if (1 != TIFFWriteCustomDirectory(tif, (uint64 *)&u64GpsOffset)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetExifIFD fail TIFFWriteCustomDirectory error\n");
        return HI_FAILURE;
    }

    *pu64GpsOffset = u64GpsOffset;

    return HI_SUCCESS;
}

HI_S32 DNG_MUXER_CheckIfdInfo(DNG_FORMAT_S *pstDng, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    if (NULL == pstIFDInfo->stImageData.pu8DataBuf) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->stImageData.pu8DataBuf null \n");
        return HI_ERR_DNG_NULL_PTR;
    }

    if (0 == pstIFDInfo->stImageData.u32DataLen) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->stImageData.u32DataLen zero \n");
        return HI_ERR_DNG_INVALIDARG;
    }

    if (0 == pstIFDInfo->u32Height || 0 == pstIFDInfo->u32Width) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport IFD  width or height \n");
        return HI_ERR_DNG_INVALIDARG;
    }

    if ((pstIFDInfo->u32Width > DNG_MAX_WIDTH) || (DNG_MAX_WIDTH < pstIFDInfo->u32Height)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport IFD  width larger than max \n");
        return HI_ERR_DNG_INVALIDARG;
    }

    if (pstIFDInfo->enImageType < HI_DNG_MUXER_IMAGE_IFD0 || pstIFDInfo->enImageType > HI_DNG_MUXER_IMAGE_RAW) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport enImageType: %d \n", pstIFDInfo->enImageType);
        return HI_ERR_DNG_INVALIDARG;
    }

    if (HI_FALSE == pstDng->bIFD0Flag && HI_DNG_MUXER_IMAGE_IFD0 != pstIFDInfo->enImageType) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "not add ifd0 yet, do not add other ifd:%d !  \n",
                       pstIFDInfo->enImageType);
        return HI_ERR_DNG_ADD_IFD;
    }

    if (HI_TRUE == pstDng->bIFD0Flag) {
        if (HI_DNG_MUXER_IMAGE_IFD0 == pstIFDInfo->enImageType) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "already add ifd0 yet, do not add ifd0 again ! \n");
            return HI_ERR_DNG_ADD_IFD;
        }
        if (0 == pstDng->u32SubIfdNum) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "already add all subifd yet, do not add subifd again ! \n");
            return HI_ERR_DNG_ADD_IFD;
        }
    }

    if (PHOTOMETRIC_CFA != pstIFDInfo->u16Photometric && PHOTOMETRIC_YCBCR != pstIFDInfo->u16Photometric) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u16Photometric: %d \n",
                       pstIFDInfo->u16Photometric);
        return HI_ERR_DNG_INVALIDARG;
    }

    if (COMPRESSION_NONE != pstIFDInfo->u16Compression && COMPRESSION_JPEG != pstIFDInfo->u16Compression) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u16Compressions: %d \n",
                       pstIFDInfo->u16Compression);
        return HI_ERR_DNG_INVALIDARG;
    }

    if (HI_DNG_MUXER_IMAGE_RAW == pstIFDInfo->enImageType) {
        if (pstIFDInfo->u32Height * pstIFDInfo->stImageData.u32Stride > pstIFDInfo->stImageData.u32DataLen) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER data info error height %d  datalen %d stride:%d \n",
                           pstIFDInfo->u32Height, pstIFDInfo->stImageData.u32DataLen, pstIFDInfo->stImageData.u32Stride);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (pstIFDInfo->stRawIFDInfo.stRawFormat.enCfaLayout < DNG_MUXER_CFALAYOUT_TYPE_RECTANGULAR ||
            pstIFDInfo->stRawIFDInfo.stRawFormat.enCfaLayout > DNG_MUXER_CFALAYOUT_TYPE_H) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR,
                           "unsupport pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout: %d for raw\n",
                           pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (0 != pstIFDInfo->u32SubfileType % 2) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u32SubfileType: %d for raw\n",
                           pstIFDInfo->u32SubfileType);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (COMPRESSION_NONE != pstIFDInfo->u16Compression) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u16Compressions: %d for raw\n",
                           pstIFDInfo->u16Compression);
            return HI_ERR_DNG_INVALIDARG;
        }
    }
    if (HI_DNG_MUXER_IMAGE_SCREEN == pstIFDInfo->enImageType) {
        if (0 == pstIFDInfo->u32SubfileType % 2) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u32SubfileType: %d for screen image\n",
                           pstIFDInfo->u32SubfileType);
            return HI_ERR_DNG_INVALIDARG;
        }
        if (COMPRESSION_NONE == pstIFDInfo->u16Compression) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u16Compressions: %d for screen image\n",
                           pstIFDInfo->u16Compression);
            return HI_ERR_DNG_INVALIDARG;
        }
    }

    if (HI_DNG_MUXER_IMAGE_IFD0 == pstIFDInfo->enImageType &&
        0 != pstIFDInfo->stIFD0Info.u32SubIFDs) {  // ifd0 is thm,include sub raw

        if (pstIFDInfo->stIFD0Info.u32SubIFDs > DNG_MUXER_SUBIFD_MAX) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport subifd num: %d for ifd0 \n",
                           pstIFDInfo->stIFD0Info.u32SubIFDs);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (0 == pstIFDInfo->u32SubfileType % 2) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u32SubfileType: %d for ifd0 \n",
                           pstIFDInfo->u32SubfileType);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (COMPRESSION_NONE == pstIFDInfo->u16Compression) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR,
                           "unsupport pstIFDInfo->u16Compressions: %d for thm ifd0 image\n", pstIFDInfo->u16Compression);
            return HI_ERR_DNG_INVALIDARG;
        }
    }

    if (HI_DNG_MUXER_IMAGE_IFD0 == pstIFDInfo->enImageType && 0 == pstIFDInfo->stIFD0Info.u32SubIFDs) {  // one raw
        if (pstIFDInfo->u32Height * pstIFDInfo->stImageData.u32Stride > pstIFDInfo->stImageData.u32DataLen) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER data info error height %d  datalen %d stride:%d \n",
                           pstIFDInfo->u32Height, pstIFDInfo->stImageData.u32DataLen, pstIFDInfo->stImageData.u32Stride);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout < DNG_MUXER_CFALAYOUT_TYPE_RECTANGULAR ||
            pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout > DNG_MUXER_CFALAYOUT_TYPE_H) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR,
                           "unsupport pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout: %d for raw\n",
                           pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (0 != pstIFDInfo->u32SubfileType % 2) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u32SubfileType: %d for raw\n",
                           pstIFDInfo->u32SubfileType);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (COMPRESSION_NONE != pstIFDInfo->u16Compression) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport pstIFDInfo->u16Compressions: %d for raw\n",
                           pstIFDInfo->u16Compression);
            return HI_ERR_DNG_INVALIDARG;
        }
    }

    return HI_SUCCESS;
}

HI_S32 DNG_MUXER_SetIFD0(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    HI_U64 u64ExifOffset = 0;
    HI_U64 u64GpsOffset = 0;
    HI_U64 *pu64Subifdoffset = NULL;
    HI_S32 s32Ret = HI_SUCCESS;
    if (pstIFDInfo->stIFD0Info.bHasExifIFD) {
        if (HI_SUCCESS != DNG_MUXER_SetExifIFD(tif, &pstIFDInfo->stIFD0Info.stExifInfo, &u64ExifOffset)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 fail DNG_MUXER_SetExifIFD error\n");
            return HI_ERR_DNG_ADD_IFD;
        }
    }

    if (pstIFDInfo->stIFD0Info.bHasGpsIFD) {
        if (HI_SUCCESS != DNG_MUXER_SetGpsIFD(tif, &pstIFDInfo->stIFD0Info.stGpsInfo, &u64GpsOffset)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 fail DNG_MUXER_SetGpsIFD error\n");
            return HI_ERR_DNG_ADD_IFD;
        }
    }

    TIFFCreateDirectory(tif);

    DNG_MUXER_SetNoramlTag(tif, pstIFDInfo);

    if (0 != pstIFDInfo->stIFD0Info.u32SubIFDs) {
        pu64Subifdoffset = (HI_U64 *)malloc(pstIFDInfo->stIFD0Info.u32SubIFDs * sizeof(HI_U64));
        if (NULL == pu64Subifdoffset) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 malloc subifdoffset error \n");
            return HI_ERR_DNG_MALLOC;
        }
        memset_s(pu64Subifdoffset, pstIFDInfo->stIFD0Info.u32SubIFDs * sizeof(HI_U64), 0x00,
                 pstIFDInfo->stIFD0Info.u32SubIFDs * sizeof(HI_U64));
        TIFFSetField(tif, TIFFTAG_SUBIFD, pstIFDInfo->stIFD0Info.u32SubIFDs, pu64Subifdoffset);
    }

    TIFFSetField(tif, TIFFTAG_MAKE, pstIFDInfo->stIFD0Info.aszMake);
    TIFFSetField(tif, TIFFTAG_MODEL, pstIFDInfo->stIFD0Info.aszModel);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, pstIFDInfo->stIFD0Info.aszSoftware);
    TIFFSetField(tif, TIFFTAG_DATETIME, pstIFDInfo->stIFD0Info.aszDateTime);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, pstIFDInfo->stIFD0Info.u8Orientation);  //

    if (pstIFDInfo->stIFD0Info.u32XmpDataLen != 0 && pstIFDInfo->stIFD0Info.pu8XmpBuf != NULL) {
        TIFFSetField(tif, TIFFTAG_XMLPACKET, pstIFDInfo->stIFD0Info.u32XmpDataLen, pstIFDInfo->stIFD0Info.pu8XmpBuf);
    }

    if (0 != pstIFDInfo->stIFD0Info.u32DNGPrivDataLen && NULL != pstIFDInfo->stIFD0Info.pu8DNGPrivData) {
        TIFFSetField(tif, TIFFTAG_DNGPRIVATEDATA, pstIFDInfo->stIFD0Info.u32DNGPrivDataLen,
                     pstIFDInfo->stIFD0Info.pu8DNGPrivData);
    }

    if (0 != pstIFDInfo->stIFD0Info.u32XPCommentLen && NULL != pstIFDInfo->stIFD0Info.pu8XPComment) {
        TIFFSetField(tif, TIFFTAG_XPCOMMENT, pstIFDInfo->stIFD0Info.u32XPCommentLen, pstIFDInfo->stIFD0Info.pu8XPComment);
    }
    if (0 != pstIFDInfo->stIFD0Info.u32XPKeywordsLen && NULL != pstIFDInfo->stIFD0Info.pu8XPKeywords) {
        TIFFSetField(tif, TIFFTAG_XPKEYWORDS, pstIFDInfo->stIFD0Info.u32XPKeywordsLen,
                     pstIFDInfo->stIFD0Info.pu8XPKeywords);
    }

    static const HI_U32 au32ProfileHueSatMapDim[] = { 6, 6, 3 };
    TIFFSetField(tif, TIFFTAG_PROFILEHUESATMAPDIMS, au32ProfileHueSatMapDim);
    if (0 != pstIFDInfo->stIFD0Info.u32Map1Len && NULL != pstIFDInfo->stIFD0Info.pfProfileHueSatMapData1) {
        TIFFSetField(tif, TIFFTAG_PROFILEHUESATMAPDATA1, pstIFDInfo->stIFD0Info.u32Map1Len,
                     pstIFDInfo->stIFD0Info.pfProfileHueSatMapData1);
    }
    if (0 != pstIFDInfo->stIFD0Info.u32Map2Len && NULL != pstIFDInfo->stIFD0Info.pfProfileHueSatMapData2) {
        TIFFSetField(tif, TIFFTAG_PROFILEHUESATMAPDATA2, pstIFDInfo->stIFD0Info.u32Map2Len,
                     pstIFDInfo->stIFD0Info.pfProfileHueSatMapData2);
    }
    TIFFSetField(tif, TIFFTAG_PROFILEEMBEDPOLICY, 0);
    TIFFSetField(tif, TIFFTAG_LENSINFO, pstIFDInfo->stIFD0Info.afLensInfo);

    TIFFSetField(tif, TIFFTAG_ASSHOTNEUTRAL, DNG_MUXER_CFACOLORPLANE,
                 pstIFDInfo->stIFD0Info.stDynamicInfo.afAsShotNeutral);
    TIFFSetField(tif, TIFFTAG_COLORMATRIX1, DNG_MUXER_MATRIX_SIZE, pstIFDInfo->stIFD0Info.stStaticInfo.afColorMatrix1);
    TIFFSetField(tif, TIFFTAG_COLORMATRIX2, DNG_MUXER_MATRIX_SIZE, pstIFDInfo->stIFD0Info.stStaticInfo.afColorMatrix2);
    TIFFSetField(tif, TIFFTAG_CAMERACALIBRATION1, DNG_MUXER_MATRIX_SIZE,
                 pstIFDInfo->stIFD0Info.stStaticInfo.afCameraCalibration1);
    TIFFSetField(tif, TIFFTAG_CAMERACALIBRATION2, DNG_MUXER_MATRIX_SIZE,
                 pstIFDInfo->stIFD0Info.stStaticInfo.afCameraCalibration2);

    TIFFSetField(tif, TIFFTAG_CALIBRATIONILLUMINANT1, pstIFDInfo->stIFD0Info.stStaticInfo.u8CalibrationIlluminant1);
    TIFFSetField(tif, TIFFTAG_CALIBRATIONILLUMINANT2, pstIFDInfo->stIFD0Info.stStaticInfo.u8CalibrationIlluminant2);
    TIFFSetField(tif, TIFFTAG_FORWARDMATRIX1, pstIFDInfo->stIFD0Info.stStaticInfo.afForwadMatrix1);
    TIFFSetField(tif, TIFFTAG_FORWARDMATRIX2, pstIFDInfo->stIFD0Info.stStaticInfo.afForwadMatrix2);
    TIFFSetField(tif, TIFFTAG_NOISEPROFILE, DNG_MUXER_NP_SIZE, pstIFDInfo->stIFD0Info.stDynamicInfo.adNoiseProfile);

    // set inside
    TIFFSetField(tif, TIFFTAG_DNGVERSION, "\01\01\00\00");          //
    TIFFSetField(tif, TIFFTAG_DNGBACKWARDVERSION, "\01\00\00\00");  //
    TIFFSetField(tif, TIFFTAG_UNIQUECAMERAMODEL, pstIFDInfo->stIFD0Info.aszUniqueCameraModel);
    TIFFSetField(tif, TIFFTAG_CAMERASERIALNUMBER, pstIFDInfo->stIFD0Info.aszCameraSerialNumber);

    static const float AnalogBalance[DNG_MUXER_CFACOLORPLANE] = { 1.0, 1.0, 1.0 };
    TIFFSetField(tif, TIFFTAG_ANALOGBALANCE, DNG_MUXER_CFACOLORPLANE, AnalogBalance);

    static const float BaselineExposure = 0.0;
    TIFFSetField(tif, TIFFTAG_BASELINEEXPOSURE, BaselineExposure);
    static const float BaselineNoise = 1.0;
    TIFFSetField(tif, TIFFTAG_BASELINENOISE, BaselineNoise);

    static const float BaselineSharpness = 1.0;
    TIFFSetField(tif, TIFFTAG_BASELINESHARPNESS, BaselineSharpness);

    static const float LinearResponseLimit = 1.0;
    TIFFSetField(tif, TIFFTAG_LINEARRESPONSELIMIT, LinearResponseLimit);

    static const float ShadowScale = 1.0;
    TIFFSetField(tif, TIFFTAG_SHADOWSCALE, ShadowScale);

    static const HI_U32 au32DefaultFinalSize[2] = { 0, 0 };
    TIFFSetField(tif, TIFFTAG_ORIGINALDEFAULTFINALSIZE, au32DefaultFinalSize);

    static const HI_U32 au32DefaultBestQualitySize[2] = { 0, 0 };
    TIFFSetField(tif, TIFFTAG_ORIGINALBESTQUALITYSIZE, au32DefaultBestQualitySize);

    static const HI_FLOAT au32DefaultCropSize[2] = { 0.0, 0.0 };
    TIFFSetField(tif, TIFFTAG_ORIGINALDEFAULTCROPSIZE, au32DefaultCropSize);

    if (pstIFDInfo->stIFD0Info.bHasExifIFD) {
        TIFFSetField(tif, TIFFTAG_EXIFIFD, u64ExifOffset);
    }
    if (pstIFDInfo->stIFD0Info.bHasGpsIFD) {
        TIFFSetField(tif, TIFFTAG_GPSIFD, u64GpsOffset);
    }

    if (0 == pstIFDInfo->stIFD0Info.u32SubIFDs) {  // means there is no subifd, store raw data in ifd0
        TIFFSetField(tif, TIFFTAG_BLACKLEVEL, 4, pstIFDInfo->stIFD0Info.stRawFormat.af32BlackLevel);

        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tif, TIFFTAG_WHITELEVEL, 1, &pstIFDInfo->stIFD0Info.stRawFormat.u32WhiteLevel);
        TIFFSetField(tif, TIFFTAG_BLACKLEVELREPEATDIM, pstIFDInfo->stIFD0Info.stRawFormat.au16BlcRepeatDim);

        TIFFSetField(tif, TIFFTAG_CFALAYOUT, pstIFDInfo->stIFD0Info.stRawFormat.enCfaLayout);
        TIFFSetField(tif, TIFFTAG_CFAPLANECOLOR, DNG_MUXER_CFACOLORPLANE,
                     pstIFDInfo->stIFD0Info.stRawFormat.au8CfaPlaneColor);
        TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, pstIFDInfo->stIFD0Info.stRawFormat.au16CfaRepeatPatternDim);
        TIFFSetField(tif, TIFFTAG_CFAPATTERN, pstIFDInfo->stIFD0Info.stRawFormat.au8CfaPattern);

        TIFFSetField(tif, TIFFTAG_DEFAULTSCALE, pstIFDInfo->stIFD0Info.stRawFormat.afDefaultScale);
        TIFFSetField(tif, TIFFTAG_BAYERGREENSPLIT, 0);
        TIFFSetField(tif, TIFFTAG_ANTIALIASSTRENGTH, 1.0);
        TIFFSetField(tif, TIFFTAG_BESTQUALITYSCALE, 1.0);

        static const HI_FLOAT DefaultUserCrop[4] = { 0, 0, 1.0, 1.0 };
        TIFFSetField(tif, TIFFTAG_DEFAULTUSERCROP, DefaultUserCrop);

        HI_U32 au32ActiveArea[4] = { 0, 0, 0, 0 };
        au32ActiveArea[2] = pstIFDInfo->u32Height;
        au32ActiveArea[3] = pstIFDInfo->u32Width;
        TIFFSetField(tif, TIFFTAG_ACTIVEAREA, au32ActiveArea);
    }

    if (1 != TIFFCheckpointDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 TIFFCheckpointDirectory fail\n");
        s32Ret = HI_ERR_DNG_ADD_IFD;
        goto ERROR;
    }

    // write data
    if (COMPRESSION_JPEG == pstIFDInfo->u16Compression) {
        if (-1 == TIFFWriteRawStrip(tif, 0, pstIFDInfo->stImageData.pu8DataBuf, pstIFDInfo->stImageData.u32DataLen)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 TIFFWriteRawStrip fail\n");
            s32Ret = HI_ERR_DNG_WRITE;
            goto ERROR;
        }
    } else if (COMPRESSION_NONE == pstIFDInfo->u16Compression) {
        if (HI_SUCCESS != DNG_MUXER_WriteRawData(tif, pstIFDInfo)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetIFD0 DNG_MUXER_WriteRawData fail\n");
            s32Ret = HI_ERR_DNG_WRITE;
            goto ERROR;
        }
    }

ERROR:
    if (0 != pstIFDInfo->stIFD0Info.u32SubIFDs) {
        if (NULL != pu64Subifdoffset) {
            free(pu64Subifdoffset);
            pu64Subifdoffset = NULL;
        }
    }

    return s32Ret;
}

HI_S32 DNG_MUXER_SetRawIFD(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    DNG_MUXER_SetNoramlTag(tif, pstIFDInfo);

    TIFFSetField(tif, TIFFTAG_DEFAULTCROPORIGIN, pstIFDInfo->stRawIFDInfo.afDefaultCropOrigin);
    TIFFSetField(tif, TIFFTAG_DEFAULTCROPSIZE, pstIFDInfo->stRawIFDInfo.afDefaultCropSize);
    TIFFSetField(tif, TIFFTAG_BLACKLEVEL, DNG_MUXER_ISP_BAYER_CHN,
                 pstIFDInfo->stRawIFDInfo.stRawFormat.af32BlackLevel);

    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(tif, TIFFTAG_WHITELEVEL, 1, &pstIFDInfo->stRawIFDInfo.stRawFormat.u32WhiteLevel);
    TIFFSetField(tif, TIFFTAG_BLACKLEVELREPEATDIM, pstIFDInfo->stRawIFDInfo.stRawFormat.au16BlcRepeatDim);

    TIFFSetField(tif, TIFFTAG_CFALAYOUT, pstIFDInfo->stRawIFDInfo.stRawFormat.enCfaLayout);
    TIFFSetField(tif, TIFFTAG_CFAPLANECOLOR, DNG_MUXER_CFACOLORPLANE,
                 pstIFDInfo->stRawIFDInfo.stRawFormat.au8CfaPlaneColor);
    TIFFSetField(tif, TIFFTAG_CFAREPEATPATTERNDIM, pstIFDInfo->stRawIFDInfo.stRawFormat.au16CfaRepeatPatternDim);
    TIFFSetField(tif, TIFFTAG_CFAPATTERN, pstIFDInfo->stRawIFDInfo.stRawFormat.au8CfaPattern);

    TIFFSetField(tif, TIFFTAG_DEFAULTSCALE, pstIFDInfo->stRawIFDInfo.stRawFormat.afDefaultScale);
    TIFFSetField(tif, TIFFTAG_BAYERGREENSPLIT, 0);
    TIFFSetField(tif, TIFFTAG_ANTIALIASSTRENGTH, 1.0);
    TIFFSetField(tif, TIFFTAG_BESTQUALITYSCALE, 1.0);

    static const HI_FLOAT DefaultUserCrop[4] = { 0, 0, 1.0, 1.0 };
    TIFFSetField(tif, TIFFTAG_DEFAULTUSERCROP, DefaultUserCrop);

    HI_U32 au32ActiveArea[4] = { 0, 0, 0, 0 };
    au32ActiveArea[2] = pstIFDInfo->u32Height;
    au32ActiveArea[3] = pstIFDInfo->u32Width;
    TIFFSetField(tif, TIFFTAG_ACTIVEAREA, au32ActiveArea);

    if (pstIFDInfo->stRawIFDInfo.u32OpcodeList1DataLen != 0 && pstIFDInfo->stRawIFDInfo.pu8OpcodeList1 != NULL) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetRawIFD OpcodeList1 len %d \n",
                       pstIFDInfo->stRawIFDInfo.u32OpcodeList1DataLen);
        TIFFSetField(tif, TIFFTAG_OPCODELIST1, pstIFDInfo->stRawIFDInfo.u32OpcodeList1DataLen,
                     pstIFDInfo->stRawIFDInfo.pu8OpcodeList1);
    }

    if (1 != TIFFCheckpointDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetRawIFD TIFFCheckpointDirectory fail\n");
        return HI_ERR_DNG_ADD_IFD;
    }

    if (HI_SUCCESS != DNG_MUXER_WriteRawData(tif, pstIFDInfo)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetRawIFD DNG_MUXER_WriteRawData fail\n");
        return HI_ERR_DNG_WRITE;
    }

    return HI_SUCCESS;
}

HI_S32 DNG_MUXER_SetScreenNailIFD(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    DNG_MUXER_SetNoramlTag(tif, pstIFDInfo);

    TIFFSetField(tif, TIFFTAG_REFERENCEBLACKWHITE, pstIFDInfo->stScreenIFDInfo.fReferenceBlackWhite);
    TIFFSetField(tif, TIFFTAG_YCBCRPOSITIONING, 1);

    static const HI_FLOAT afYCbCrCoefficients[DNG_MUXER_CFACOLORPLANE] = { 0.299, 0.587, 0.114 };
    TIFFSetField(tif, TIFFTAG_YCBCRCOEFFICIENTS, afYCbCrCoefficients);

    HI_U16 u16YCbCrSubSamplingH = 2;
    HI_U16 u16YCbCrSubSamplingV = 2;
    TIFFSetField(tif, TIFFTAG_YCBCRSUBSAMPLING, u16YCbCrSubSamplingH, u16YCbCrSubSamplingV);

    if (1 != TIFFCheckpointDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetScreenNailIFD TIFFCheckpointDirectory fail\n");
        return HI_ERR_DNG_ADD_IFD;
    }

    // write data
    if (-1 == TIFFWriteRawStrip(tif, 0, pstIFDInfo->stImageData.pu8DataBuf, pstIFDInfo->stImageData.u32DataLen)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "DNG_MUXER_SetScreenNailIFD TIFFWriteRawStrip fail\n");
        return HI_ERR_DNG_WRITE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
