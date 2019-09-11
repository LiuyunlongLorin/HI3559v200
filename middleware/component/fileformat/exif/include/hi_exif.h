/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file       hi_exif.h
* @brief     hiEXIF module header file
* @author    HiMobileCam middleware develop team
* @date      2018.03.06
*/

#ifndef __HI_EXIF_H__
#define __HI_EXIF_H__

#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"

/** \addtogroup     EXIF */
/** @{ */ /** <!-- [EXIF] */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum hiAPP_EXIF_ERR_CODE_E {
    APP_EXIF_ERR_NO_AVAILABLE_HANDLE = 0x40,  /**<no available tag editor handle */
    APP_EXIF_ERR_NO_INVALID_HANDLE = 0x41,    /**< invalid tag editor handle */
    APP_EXIF_ERR_STATUS = 0x45,               /**< err status */
    APP_EXIF_ERR_INNER_FAILED = 0x46,         /**< inner call failed */
    APP_EXIF_ERR_MALLOC_FAILED = 0x47,        /**< malloc failed */
    APP_EXIF_ERR_RES_OCCUPY = 0x48,           /**< res used failed */
    APP_EXIF_ERR_UNSUPPORT_DATA = 0x49,       /**< unsupport this data  */
    APP_EXIF_INSUFFICIENT_FIXED_SPACE = 0x50, /**< insufficient prealloc space  */
    APP_EXIF_ERR_FILE_OPERATE = 0x51,         /**< call system file operate failed  */
    APP_EXIF_ERR_READ_DATA = 0x52,            /**< read  data failed  */
    APP_EXIF_ERR_OPEN_FILE = 0x53,            /**< open file failed  */
    APP_EXIF_ERR_UNSUPPORT_FILE = 0x54,       /**< unsupport this FILE  */
    APP_EXIF_ERR_RES_NOT_RELEASE = 0x55,      /**< instance  not closed */
    APP_EXIF_ERR_SAFE_FUNCTION = 0x56,        /**< safe function  exec failed */
    APP_EXIF_BUTT = 0xFF
} HI_APP_EXIF_ERR_CODE_E;

/* general error code */
#define HI_ERR_EXIF_NULL_PTR                 HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_ERR_NULL_PTR)
#define HI_ERR_EXIF_ILLEGAL_PARAM            HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_ERR_ILLEGAL_PARAM)
#define HI_ERR_EXIF_HANDLE_INVALID           HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_NO_INVALID_HANDLE)
#define HI_ERR_EXIF_NO_AVAILABLE_HANDLE      HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_NO_AVAILABLE_HANDLE)
#define HI_ERR_EXIF_STATUS                   HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_STATUS)
#define HI_ERR_EXIF_INNER_FAILED             HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_INNER_FAILED)
#define HI_ERR_EXIF_MALLOC_FAILED            HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_MALLOC_FAILED)
#define HI_ERR_EXIF_RES_OCCUPY               HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_RES_OCCUPY)
#define HI_ERR_EXIF_UNSUPPORT_DATA           HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_UNSUPPORT_DATA)
#define HI_ERR_EXIF_INSUFFICIENT_FIXED_SPACE HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_INSUFFICIENT_FIXED_SPACE)
#define HI_ERR_EXIF_FILE_OPERATE             HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_READ_DATA)
#define HI_ERR_EXIF_READ_DATA                HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_READ_DATA)
#define HI_ERR_EXIF_OPEN_FILE                HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_OPEN_FILE)
#define HI_ERR_EXIF_UNSUPPORT_FILE           HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_UNSUPPORT_FILE)
#define HI_ERR_EXIF_RES_NOT_RELEASE          HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_RES_NOT_RELEASE)
#define HI_ERR_EXIF_SAFE_FUNCTION            HI_APP_DEF_ERR(HI_APPID_TAG_EDITOR, APP_ERR_LEVEL_ERROR, APP_EXIF_ERR_SAFE_FUNCTION)

#define HI_EXIF_SUBIMG_MAX_CNT   (6)
#define HI_EXIF_DATA_SEG_MAX_CNT (8)

typedef enum hiEXIF_DATA_TYPE_E {
    HI_EXIF_DATA_TYPE_BYTE = 1,        // !< 8-bit unsigned integer
    HI_EXIF_DATA_TYPE_ASCII = 2,       // !< 8-bit bytes w/ last byte null
    HI_EXIF_DATA_TYPE_SHORT = 3,       // !< 16-bit unsigned integer
    HI_EXIF_DATA_TYPE_LONG = 4,        // !< 32-bit unsigned integer
    HI_EXIF_DATA_TYPE_RATIONAL = 5,    // !< 64-bit unsigned fraction
    HI_EXIF_DATA_TYPE_SBYTE = 6,       // !< 8-bit signed integer
    HI_EXIF_DATA_TYPE_UNDEFINED = 7,   // !< 8-bit untyped data
    HI_EXIF_DATA_TYPE_SSHORT = 8,      // !< 16-bit signed integer */
    HI_EXIF_DATA_TYPE_SLONG = 9,       // !< 32-bit signed integer */
    HI_EXIF_DATA_TYPE_SRATIONAL = 10,  // !< 64-bit signed fraction */
    HI_EXIF_DATA_TYPE_FLOAT = 11,      // !< 32-bit IEEE floating point */
    HI_EXIF_DATA_TYPE_DOUBLE = 12,     // !< 64-bit IEEE floating point */
    HI_EXIF_DATA_TYPE_BUTT             // MAX +1 Valid Data Type Value */
} HI_EXIF_DATA_TYPE_E;

typedef enum hiEXIF_TAG_LOCATION_E {
    HI_EXIF_APP1_IFD0 = 0,
    HI_EXIF_APP1_EXIFIFD,
    HI_EXIF_APP1_EXIFINTEROP_IFD,
    HI_EXIF_APP1_IFD0_GPSINFO_IFD,
    HI_EXIF_APP1_IFD1,
    HI_EXIF_LOCATION_BUTT
} HI_EXIF_TAG_LOCATION_E;

typedef struct hiEXIF_SUBIMG_S {
    HI_U32 u32Width;    /**< The height of the media file's resolution */
    HI_U32 u32Height;   /**< The width of the media file's resolution */
    HI_U32 u32DataLen;  /**< The data len of sub img */
    HI_BOOL bThumbnail; /* true:this is the thumbnail */
} HI_EXIF_SUBIMG_S;

typedef struct hiEXIF_INFO_S {
    HI_S64 s64FileSize;
    HI_EXIF_SUBIMG_S stSubImg[HI_EXIF_SUBIMG_MAX_CNT];
} HI_EXIF_INFO_S;

typedef struct hiEXIF_DATA_S {
    HI_U32 u32PicIndex;
    HI_U8 *pu8Data;
    HI_U32 u32DataLen;
    HI_U32 u32HasReadDataLen; /* output param */
} HI_EXIF_DATA_S;

typedef struct hiEXIF_TAG_ENTRY_S {
    HI_U16 u16TagNum;
    HI_EXIF_DATA_TYPE_E enDataType;
    HI_U32 u32Cnt;
    const void *pValue; /**< int array, char array and so on */
} HI_EXIF_TAG_ENTRY_S;

typedef struct hiEXIF_CONFIG_DEMUXER_S {
    const char *pszFileName; /**< the input image file name , the length of pszFileName shoud be in (0,PATH_MAX] */
} HI_EXIF_CONFIG_DEMUXER_S;

typedef struct hiEXIF_CONFIG_EDITOR_S {
    const char *pszFileName;     /**< the output file name  for img data , the length of pszFileName shoud be in (0,PATH_MAX] */
    HI_BOOL bIsOverWrite;        /**< HI_FALSE:if  pszFileName exist then return error; HI_TRUE: if pszFileName exist then overwrite it */
    HI_U32 u32FixedFileSize;     /**< 0: do not limit file size,  other value: the output file size can not be larger than  u32FixedFileSize */
    HI_U32 u32MaxExifHeaderSize; /**< 0: default 200k,  other value: the output exif header size can not be larger than  u32MaxExifHeaderSize */
} HI_EXIF_CONFIG_EDITOR_S;

typedef enum hiEXIF_CONFIG_TYPE_E {
    HI_EXIF_CONFIG_DEMUXER = 0, /**< exif module for demuxer */
    HI_EXIF_CONFIG_EDITOR,      /**< exif module for editor */
    HI_EXIF_CONFIG_BUTT
} HI_EXIF_CONFIG_TYPE_E;

typedef struct hiEXIF_ATTR_S {
    HI_EXIF_CONFIG_TYPE_E enExifConfigType;
    union {
        HI_EXIF_CONFIG_EDITOR_S stEditorConfig;
        HI_EXIF_CONFIG_DEMUXER_S stDemuxerConfig;
    };
} HI_EXIF_ATTR_S;

typedef struct hiEXIF_BUFFER_SOURCE_S {
    HI_VOID *pImgBuf[HI_EXIF_DATA_SEG_MAX_CNT];
    HI_U32 u32BufSize[HI_EXIF_DATA_SEG_MAX_CNT];
} HI_EXIF_BUFFER_SOURCE_S;

/**
*   @brief init exif module
*   @param[in] N/A
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_Init(HI_VOID);

/**
*   @brief open exif instance
*   @param[out] ppExif: the address of the pointer of exif
*   @param[in] pstExifAttr : the attribute of the exif instance
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_Open(HI_MW_PTR *ppExif, HI_EXIF_ATTR_S *pstExifAttr);

/**
*   @brief get picture infomation
*   @param[in] pEXIF : pointer of exif instance
*   @param[out] pstExifInfo : the address of exif  image info
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_GetInfo(HI_MW_PTR pEXIF, HI_EXIF_INFO_S *pstExifInfo);

/**
*   @brief read picture data
*   @param[in] pEXIF : pointer of exif instance
*   @param[out] pstExifData : the address of exif data info
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_ReadPicData(HI_MW_PTR pEXIF, HI_EXIF_DATA_S *pstExifData);

/**
*   @brief set the picture memory data
*   @param[in] pEXIF : pointer of exif instance
*   @param[in] pstBufSource :  image data source info include data address and data size.
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_SetBufferSource(HI_MW_PTR pEXIF, HI_EXIF_BUFFER_SOURCE_S *pstBufSource);

/**
*   @brief set or update tag of the exif picture
*   @param[in] pEXIF : pointer of exif instance
*   @param[in] pstTagEntry : pointer of tag entry  info
*   @param[in] enExifTagLocation : the location where the tag insert
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_SetTag(HI_MW_PTR pEXIF, HI_EXIF_TAG_ENTRY_S *pstTagEntry, HI_EXIF_TAG_LOCATION_E enExifTagLocation);

/**
*   @brief close exif instance
*   @param[in] pEXIF : pointer of exif instance
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_Close(HI_MW_PTR pEXIF);

/**
*   @brief deinit exif module
*   @param[in] N/A
*   @retval  0 success,others failed
*/
HI_S32 HI_EXIF_DeInit(HI_VOID);

/** @} */ /** <!-- ==== EXIF  End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_EXIF_H__ */
