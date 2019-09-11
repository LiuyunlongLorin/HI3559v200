#include "securec.h"
#include <sys/time.h>
#include <limits.h>
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
#include "dng_log.h"
#include "dng_common.h"

#define N(a) (sizeof(a) / sizeof(a[0]))

#define DNG_CHECK_NULL_ERROR(condition) \
    do {                                \
        if (condition == NULL) {        \
            return HI_ERR_DNG_NULL_PTR; \
        }                               \
    } while (0)
#define DNG_LOCK(mutex)                       \
    do {                                      \
        (HI_VOID)pthread_mutex_lock(&mutex); \
    } while (0)

#define DNG_UNLOCK(mutex)                       \
    do {                                        \
        (HI_VOID)pthread_mutex_unlock(&mutex); \
    } while (0)

static const TIFFFieldInfo xtiffFieldInfo[] = {
    { TIFFTAG_PROFILEHUESATMAPDIMS,  3,  3,  TIFF_LONG,  FIELD_CUSTOM, 0, 0, (char *)("ProfileHueSatMapDims") },
    { TIFFTAG_PROFILEHUESATMAPDATA1, -1, -1, TIFF_FLOAT, FIELD_CUSTOM, 0, 1, (char *)("ProfileHueSatMapData1") },
    { TIFFTAG_PROFILEHUESATMAPDATA2, -1, -1, TIFF_FLOAT, FIELD_CUSTOM, 0, 1, (char *)("ProfileHueSatMapData2") },
    { TIFFTAG_PROFILEEMBEDPOLICY,    1,  1,  TIFF_LONG,  FIELD_CUSTOM, 0, 0, (char *)("ProfileEmbedPolicy") },

    { TIFFTAG_ORIGINALDEFAULTFINALSIZE, 2,  2,  TIFF_LONG,      FIELD_CUSTOM, 0, 0, (char *)("OriginalDefaultFinalSize") },
    { TIFFTAG_ORIGINALBESTQUALITYSIZE,  2,  2,  TIFF_LONG,      FIELD_CUSTOM, 0, 0, (char *)("OriginalBestQualitySize") },
    { TIFFTAG_ORIGINALDEFAULTCROPSIZE,  2,  2,  TIFF_RATIONAL,  FIELD_CUSTOM, 0, 0, (char *)("OriginalDefaultCropSize") },
    { TIFFTAG_FORWARDMATRIX1,           9,  9,  TIFF_SRATIONAL, FIELD_CUSTOM, 0, 0, (char *)("ForwardMatrix1") },
    { TIFFTAG_FORWARDMATRIX2,           9,  9,  TIFF_SRATIONAL, FIELD_CUSTOM, 0, 0, (char *)("ForwardMatrix2") },
    { TIFFTAG_OPCODELIST1,              -1, -1, TIFF_UNDEFINED, FIELD_CUSTOM, 0, 1, (char *)("OpcodeList1") },
    { TIFFTAG_DEFAULTUSERCROP,          4,  4,  TIFF_RATIONAL,  FIELD_CUSTOM, 0, 0, (char *)("DefaultUserCrop") },
    { TIFFTAG_NOISEPROFILE,             -1, -1, TIFF_DOUBLE,    FIELD_CUSTOM, 0, 1, (char *)("NoiseProfile") },
    { TIFFTAG_XPCOMMENT,                -1, -1, TIFF_BYTE,      FIELD_CUSTOM, 0, 1, (char *)("XPComment") },
    { TIFFTAG_XPKEYWORDS,               -1, -1, TIFF_BYTE,      FIELD_CUSTOM, 0, 1, (char *)("XPKeywords") },

    { GPSTAG_VERSIONID,    4, 4, TIFF_BYTE,     FIELD_CUSTOM, 0, 0, (char *)("GPSVersionId") },
    { GPSTAG_LATITUDEREF,  2, 2, TIFF_ASCII,    FIELD_CUSTOM, 0, 0, (char *)("GPSLatitudeRef") },
    { GPSTAG_LATITUDE,     3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, (char *)("GPSLatitude") },
    { GPSTAG_LONGITUDEREF, 2, 2, TIFF_ASCII,    FIELD_CUSTOM, 0, 0, (char *)("GPSLongitudeRef") },
    { GPSTAG_LONGITUDE,    3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, (char *)("GPSLongitude") },
    { GPSTAG_ALTITUDEREF,  1, 1, TIFF_BYTE,     FIELD_CUSTOM, 0, 0, (char *)("GPSAltitudeRef") },
    { GPSTAG_ALTITUDE,     1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, (char *)("GPSAltitude") },

};  // The const_cast<char*> casts are necessary because the
// string literals are inherently const,
// but the definition of TIFFFieldInfo
// requires a non-const string pointer.
// The Intel and Microsoft compilers
// tolerate this, but gcc doesn't.

static HI_DNG_CTX_S s_stDngCTX = { HI_FALSE, 0, PTHREAD_MUTEX_INITIALIZER,
                                   HI_LIST_INIT_HEAD_DEFINE(s_stDngCTX.pDngList)
                                 };

static TIFFExtendProc parent_extender = NULL;  // In case we want a chain of extensions

static void registerCustomTIFFTags(TIFF *tif)
{
    /* Install the extended Tag field info */
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = TIFFMergeFieldInfo(tif, xtiffFieldInfo, N(xtiffFieldInfo));
    if (s32Ret < 0) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "TIFFMergeFieldInfo failed   \n");
    }
    if (parent_extender) {
       (*parent_extender)(tif);
    }
    return;
}

HI_VOID SAMPLE_AugmentTiffWithCustomTags()
{
    static HI_BOOL first_time = HI_TRUE;

    if (!first_time) {
        return;
    }
    first_time = HI_FALSE;
    parent_extender = TIFFSetTagExtender(registerCustomTIFFTags);

    return;
}

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HI_S32 DNG_CheckCfg(HI_DNG_CONFIG_S *pstDngCfg)
{
    HI_U32 i = 0;
    HI_BOOL bFound = HI_FALSE;
    for (i = 0; i < DNG_MAX_FILE_NAME; i++) {
        if ('\0' == pstDngCfg->aszFileName[i]) {
            bFound = HI_TRUE;
            break;
        } else if (';' == pstDngCfg->aszFileName[i]) {
            return HI_ERR_DNG_INVALIDARG;
        }
    }

    if ((HI_TRUE != bFound) || (strlen(pstDngCfg->aszFileName) >= DNG_MAX_FILE_NAME)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport DNG filename \n");
        return HI_ERR_DNG_INVALIDARG;
    }

    if (pstDngCfg->enConfigType < HI_DNG_CONFIG_TYPE_MUXER || pstDngCfg->enConfigType >= HI_DNG_CONFIG_TYPE_BUTT) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "unsupport DNG config type :%d \n", pstDngCfg->enConfigType);
        return HI_ERR_DNG_INVALIDARG;
    }

    return HI_SUCCESS;
}
static HI_BOOL DNG_FindDng(HI_MW_PTR pHandle)
{
    List_Head_S *pstTmpNode = NULL;
    List_Head_S *pstPosNode = NULL;
    DNG_FORMAT_S *pstDngNode = NULL;

    DNG_LOCK(s_stDngCTX.mDngListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &s_stDngCTX.pDngList)
    {
        pstDngNode = HI_LIST_ENTRY(pstPosNode, DNG_FORMAT_S, pstDngListPtr);
        if (pHandle == (HI_MW_PTR)pstDngNode) {
            DNG_UNLOCK(s_stDngCTX.mDngListLock);
            return HI_TRUE;
        }
    }
    DNG_UNLOCK(s_stDngCTX.mDngListLock);

    return HI_FALSE;
}

HI_S32 HI_DNG_Create(HI_MW_PTR *ppDng, HI_DNG_CONFIG_S *pstDngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    DNG_FORMAT_S *pstDng = NULL;
    TIFF *tif = NULL;
    HI_U16 count16 = 0;
    HI_U64 *pu64Subifdoffset = NULL;
    HI_CHAR chrFileName[PATH_MAX + 1] = "";

    DNG_CHECK_NULL_ERROR(ppDng);
    DNG_CHECK_NULL_ERROR(pstDngCfg);

    s32Ret = DNG_CheckCfg(pstDngCfg);
    if (HI_SUCCESS != s32Ret) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_Create fail cfg param illegal\n");
        return HI_ERR_DNG_INVALIDARG;
    }

    memset_s(chrFileName, sizeof(chrFileName), 0, sizeof(chrFileName));

    if (0 == s_stDngCTX.s32DngNum) {
        SAMPLE_AugmentTiffWithCustomTags();
    }
    if (DNG_MAX_COUNT <= s_stDngCTX.s32DngNum) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_Create fail, is full \n");
        return HI_ERR_DNG_REACH_MAX;
    }

    pstDng = (DNG_FORMAT_S *)malloc(sizeof(DNG_FORMAT_S));
    if (NULL == pstDng) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng muxer malloc error \n");
        return HI_ERR_DNG_MALLOC;
    }
    memset_s(pstDng, sizeof(DNG_FORMAT_S), 0x00, sizeof(DNG_FORMAT_S));
    pstDng->enConfigType = pstDngCfg->enConfigType;
    pthread_mutex_init(&pstDng->mDngLock, NULL);

    if (HI_DNG_CONFIG_TYPE_MUXER == pstDngCfg->enConfigType) {
        snprintf_s(chrFileName, PATH_MAX + 1, DNG_MAX_FILE_NAME, "%s", pstDngCfg->aszFileName);
        tif = TIFFOpen(chrFileName, "w");
        if (NULL == tif) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng muxer open file error \n");
            s32Ret = HI_ERR_DNG_CREATE_MUXER;
            goto ERROR;
        }
    } else if (HI_DNG_CONFIG_TYPE_DEMUXER == pstDngCfg->enConfigType) {
        if (NULL == realpath(pstDngCfg->aszFileName, chrFileName)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_WARN, "HI_DNG_Create cfg file is not exist!\n");
            s32Ret = HI_ERR_DNG_INVALIDARG;
            goto ERROR;
        }
        tif = TIFFOpen(chrFileName, "rb");
        if (NULL == tif) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer open file error \n");
            s32Ret = HI_ERR_DNG_CREATE_DEMUXER;
            goto ERROR;
        }
        TIFFGetField(tif, TIFFTAG_SUBIFD, &count16, &pu64Subifdoffset);
        pstDng->u32SubIfdNum = count16;
    }

    pstDng->pstTiffHandle = tif;

    DNG_LOCK(s_stDngCTX.mDngListLock);

    HI_List_Add(&(pstDng->pstDngListPtr), &(s_stDngCTX.pDngList));
    s_stDngCTX.s32DngNum += 1;

    DNG_UNLOCK(s_stDngCTX.mDngListLock);

    *ppDng = (HI_MW_PTR)pstDng;

    return HI_SUCCESS;

ERROR:
    if (pstDng) {
        free(pstDng);
        pstDng = NULL;
    }
    return s32Ret;
}

HI_S32 HI_DNG_Destroy(HI_MW_PTR pHandle)
{
    TIFF *tif = NULL;
    DNG_FORMAT_S *pstDng = NULL;
    DNG_CHECK_NULL_ERROR(pHandle);
    List_Head_S *pstPosNode = NULL;
    List_Head_S *pstTmpNode = NULL;
    DNG_FORMAT_S *pstDngNode = NULL;

    if (!DNG_FindDng(pHandle)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng muxer handle not find \n");
        return HI_ERR_DNG_HANDLE_INVALID;
    }

    pstDng = (DNG_FORMAT_S *)pHandle;

    DNG_LOCK(s_stDngCTX.mDngListLock);
    HI_List_For_Each_Safe(pstPosNode, pstTmpNode, &s_stDngCTX.pDngList)
    {
        pstDngNode = HI_LIST_ENTRY(pstPosNode, DNG_FORMAT_S, pstDngListPtr);
        /* delete the stream */
        if (pHandle == (HI_MW_PTR)pstDngNode) {
            HI_List_Del(&(pstDngNode->pstDngListPtr));
            s_stDngCTX.s32DngNum -= 1;
        }
    }

    DNG_UNLOCK(s_stDngCTX.mDngListLock);

    tif = (TIFF *)pstDng->pstTiffHandle;
    TIFFClose(tif);
    pthread_mutex_destroy(&pstDng->mDngLock);

    if (pstDng) {
        free(pstDng);
        pstDng = NULL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_DNG_AddIFD(HI_MW_PTR pDng, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    TIFF *tif = NULL;
    DNG_FORMAT_S *pstDng = NULL;

    DNG_CHECK_NULL_ERROR(pDng);
    DNG_CHECK_NULL_ERROR(pstIFDInfo);

    if (!DNG_FindDng(pDng)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng handle invalid \n");
        return HI_ERR_DNG_HANDLE_INVALID;
    }
    pstDng = (DNG_FORMAT_S *)pDng;
    if (HI_DNG_CONFIG_TYPE_MUXER != pstDng->enConfigType) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng  add ifd not support config type \n");
        return HI_ERR_DNG_ADD_IFD;
    }
    s32Ret = DNG_MUXER_CheckIfdInfo(pstDng, pstIFDInfo);
    if (HI_SUCCESS != s32Ret) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng add ifd info parm illegal \n");
        return s32Ret;
    }

    DNG_LOCK(pstDng->mDngLock);

    tif = (TIFF *)pstDng->pstTiffHandle;
    if (HI_DNG_MUXER_IMAGE_IFD0 == pstIFDInfo->enImageType) {
        s32Ret = DNG_MUXER_SetIFD0(tif, pstIFDInfo);
        if (HI_SUCCESS != s32Ret) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_AddIFD fail set IFD error image type %d \n",
                           pstIFDInfo->enImageType);
            DNG_UNLOCK(pstDng->mDngLock);
            return s32Ret;
        }

        pstDng->bIFD0Flag = HI_TRUE;
        pstDng->u32SubIfdNum = pstIFDInfo->stIFD0Info.u32SubIFDs;
    } else if ((HI_DNG_MUXER_IMAGE_RAW == pstIFDInfo->enImageType) && (pstDng->u32SubIfdNum > 0)) {
        s32Ret = DNG_MUXER_SetRawIFD(tif, pstIFDInfo);
        if (HI_SUCCESS != s32Ret) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_AddIFD fail set IFD error image type %d \n",
                           pstIFDInfo->enImageType);
            DNG_UNLOCK(pstDng->mDngLock);
            return s32Ret;
        }
        pstDng->u32SubIfdNum -= 1;
    } else if ((HI_DNG_MUXER_IMAGE_SCREEN == pstIFDInfo->enImageType) && (pstDng->u32SubIfdNum > 0)) {
        s32Ret = DNG_MUXER_SetScreenNailIFD(tif, pstIFDInfo);
        if (HI_SUCCESS != s32Ret) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_AddIFD fail set IFD error image type %d \n",
                           pstIFDInfo->enImageType);
            DNG_UNLOCK(pstDng->mDngLock);
            return s32Ret;
        }
        pstDng->u32SubIfdNum -= 1;
    }

    if (1 != TIFFWriteDirectory(tif)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_AddIFD fail TIFFWriteDirectory error \n");
        DNG_UNLOCK(pstDng->mDngLock);
        return HI_FAILURE;
    }

    DNG_UNLOCK(pstDng->mDngLock);

    return HI_SUCCESS;
}

HI_S32 HI_DNG_GetImageInfo(HI_MW_PTR pDng, HI_U32 u32IfdIndex, HI_DNG_DEMUXER_IMAGE_INFO_S *pstImageInfo)
{
    DNG_FORMAT_S *pstDng = NULL;

    TIFF *tif = NULL;
    HI_S32 s32Size = 0;
    HI_U16 count16 = 0;
    HI_U64 *pu64Subifdoffset = NULL;

    DNG_CHECK_NULL_ERROR(pDng);
    DNG_CHECK_NULL_ERROR(pstImageInfo);

    if (!DNG_FindDng(pDng)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer handle invalid \n");
        return HI_ERR_DNG_HANDLE_INVALID;
    }

    pstDng = (DNG_FORMAT_S *)pDng;
    if (HI_DNG_CONFIG_TYPE_DEMUXER != pstDng->enConfigType) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng getImageInfo not support config type \n");
        return HI_ERR_DNG_GET_INFO;
    }

    tif = (TIFF *)pstDng->pstTiffHandle;

    if (u32IfdIndex > pstDng->u32SubIfdNum) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer do not have such ifd index:%d  ifd sum num :%d \n",
                       u32IfdIndex, (pstDng->u32SubIfdNum + 1));
        return HI_ERR_DNG_INVALIDARG;
    }

    DNG_LOCK(pstDng->mDngLock);

    if (0 == u32IfdIndex) {
        if (0 == TIFFSetDirectory(tif, 0)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer TIFFSetDirectory for ifd0 failed \n");
            DNG_UNLOCK(pstDng->mDngLock);
            return HI_ERR_DNG_GET_INFO;
        }
        s32Size = TIFFRawStripSize(tif, 0);

    } else {
        if (0 == TIFFGetField(tif, TIFFTAG_SUBIFD, &count16, &pu64Subifdoffset)) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer get image info TIFFTAG_SUBIFD error \n");
            DNG_UNLOCK(pstDng->mDngLock);
            return HI_ERR_DNG_GET_INFO;
        }
        if (u32IfdIndex > TIFFTAG_SUBIFD) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR,
                           "dng demuxer do not have such ifd index:%d , ifd sum num :%d \n", u32IfdIndex, (count16 + 1));
            DNG_UNLOCK(pstDng->mDngLock);
            return HI_ERR_DNG_INVALIDARG;
        }

        if (0 == TIFFSetSubDirectory(tif, pu64Subifdoffset[u32IfdIndex - 1])) {
            DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer TIFFSetSubDirectory for subifd failed \n");
            DNG_UNLOCK(pstDng->mDngLock);
            return HI_ERR_DNG_GET_INFO;
        }
        s32Size = TIFFRawStripSize(tif, 0);
    }

    if (s32Size <= 0) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer get ifd size error \n");
        DNG_UNLOCK(pstDng->mDngLock);
        return HI_ERR_DNG_GET_INFO;
    }

    pstImageInfo->u32DataLen = s32Size;

    if (0 == TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &pstImageInfo->u32Width)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer get TIFFTAG_IMAGEWIDTH error \n");
        DNG_UNLOCK(pstDng->mDngLock);
        return HI_ERR_DNG_GET_INFO;
    }

    if (0 == TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &pstImageInfo->u32Height)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng demuxer get TIFFTAG_IMAGELENGTH  error \n");
        DNG_UNLOCK(pstDng->mDngLock);
        return HI_ERR_DNG_GET_INFO;
    }

    pstDng->bGetInfoFlag = HI_TRUE;
    pstDng->u32CurIndex = u32IfdIndex;

    DNG_UNLOCK(pstDng->mDngLock);

    return HI_SUCCESS;
}

// please call after get the image info,HI_DNG_GetImageInfo
HI_S32 HI_DNG_GetImageData(HI_MW_PTR pHandle, HI_U8 *pu8Buffer, HI_U32 u32DataLen)
{
    DNG_FORMAT_S *pstDng = NULL;
    TIFF *tif = NULL;

    DNG_CHECK_NULL_ERROR(pHandle);
    DNG_CHECK_NULL_ERROR(pu8Buffer);
    if (!DNG_FindDng(pHandle)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng handle invalid \n");
        return HI_ERR_DNG_HANDLE_INVALID;
    }

    pstDng = (DNG_FORMAT_S *)pHandle;
    if (HI_DNG_CONFIG_TYPE_DEMUXER != pstDng->enConfigType) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng getImageData not support config type \n");
        return HI_ERR_DNG_GET_IMAGE;
    }

    if (HI_FALSE == pstDng->bGetInfoFlag) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR,
                       "please get image info first \n");
        return HI_ERR_DNG_GET_IMAGE;
    }

    tif = (TIFF *)pstDng->pstTiffHandle;
    if (NULL == pstDng->pstTiffHandle) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "dng tiff handle is null \n");
        return HI_ERR_DNG_GET_IMAGE;
    }
    DNG_LOCK(pstDng->mDngLock);

    if (-1 == TIFFReadRawStrip(tif, 0, pu8Buffer, u32DataLen)) {
        DNG_LOG_Printf(MODULE_NAME_DNG, DNG_LOG_LEVEL_ERR, "HI_DNG_GetImageData TIFFReadRawStrip failed  \n");
        DNG_UNLOCK(pstDng->mDngLock);
        return HI_ERR_DNG_GET_IMAGE;
    }

    DNG_UNLOCK(pstDng->mDngLock);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
