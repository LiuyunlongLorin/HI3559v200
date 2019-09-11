/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_xmp.h
* @brief     xmp module header file
* @author    HiMobileCam middleware develop team
* @date      2017.06.24
*/

#ifndef __HI_XMP_H__
#define __HI_XMP_H__

#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HI_XMP_MAX_NAMESPACE_PATH (128)
#define HI_XMP_MAX_KEY_PATH       (64)
#define HI_XMP_MAX_HANDLE_NUM     (10)
#define HI_XMP_MAX_NAMESPACE_NUM  (10)
#define HI_XMP_MODULE_NAME        "XMP"

typedef enum hiAPP_XMP_ERR_CODE_E {
    /* general error code */
    APP_XMP_ERR_EINVAL_PAEAMETER = 0x61,
    APP_XMP_ERR_NULL_PTR = 0x62,
    APP_XMP_ERR_NOMEM = 0x63,
    APP_XMP_ERR_GET_PAEAMETER = 0x64,
    APP_XMP_ERR_INIT = 0x65,
    APP_XMP_ERR_INSTANTE = 0x66,
    APP_XMP_ERR_DEINIT = 0x67,
    APP_XMP_ERR_TYPE = 0x68,
    APP_XMP_BUTT = 0xFF
} APP_XMP_ERR_CODE_E;

/* general error code */
#define HI_ERR_XMP_EINVAL_PAEAMETER HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_EINVAL_PAEAMETER)
#define HI_ERR_XMP_NULL_PTR         HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_NULL_PTR)
#define HI_ERR_XMP_NOMEM            HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_NOMEM)
#define HI_ERR_XMP_GET_PAEAMETER    HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_GET_PAEAMETER)
#define HI_ERR_XMP_INIT             HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_INIT)
#define HI_ERR_XMP_INSTANTE         HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_INSTANTE)
#define HI_ERR_XMP_DEINIT           HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_DEINIT)
#define HI_ERR_XMP_TYPE             HI_APP_DEF_ERR(HI_APPID_XMP, APP_ERR_LEVEL_ERROR, APP_XMP_ERR_TYPE)

typedef struct hiXMP_NS_S {
    HI_CHAR aszPrefix[HI_XMP_MAX_KEY_PATH];
    HI_CHAR aszURI[HI_XMP_MAX_NAMESPACE_PATH];
} HI_XMP_NS_S;

typedef struct hiXMP_WRITER_ATTS_S {
    HI_S32 s32NSCnt;
    HI_XMP_NS_S *pstNameSpaces[HI_XMP_MAX_NAMESPACE_NUM];
} HI_XMP_WRITER_ATTS_S;

typedef struct hiXMP_READER_ATTRS_S {
    HI_S32 s32InputBufLen;
    HI_CHAR *pszInputBuf;
} HI_XMP_READER_ATTRS_S;

typedef enum hiXMP_ATTR_TYPE_E {
    HI_XMP_ATTR_READER = 0,
    HI_XMP_ATTR_WRITER,
    HI_XMP_ATTR_BUTT
} HI_XMP_ATTR_TYPE_E;

typedef struct hiXMP_ATTR_S {
    HI_XMP_ATTR_TYPE_E enXMPAttrType;
    union {
        HI_XMP_READER_ATTRS_S stReaderAttr;
        HI_XMP_WRITER_ATTS_S stWriterAttr;
    };
} HI_XMP_ATTR_S;

/**
 * @brief key value callback
 * @param[in] pszkey HI_CHAR* :  key
 * @param[in] pszValue HI_CHAR*  : value
 * @return   0 success
 * @return  err num  failure
 */
typedef HI_S32 (*HI_XMP_HANDLE_KEYVALUE_FN)(const HI_CHAR *pszkey, const HI_CHAR *pszValue);

/**
 * @brief The XMP module is initialized
 * @param   void
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_Init();

/**
 * @brief XMP module to deinit
 * @param   void
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_DeInit();

/**
 * @brief Create xmp instance object
 * @param[in] pstAttr HI_XMP_ATTR_S :  xmp attr
 * @param[out] pHandle HI_XMP_HANDLE* : return xmp handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_Create(HI_XMP_ATTR_S *pstAttr, HI_MW_PTR *ppXMP);

/**
 * @brief Destroy XMP instance object
 * @param[in] pHandle HI_XMP_HANDLE : xmp handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_Destroy(HI_MW_PTR pXMP);

/**
 * @brief Gets the XMP data attribute value after parsing
 * @param[in] hXMPhandle HI_XMP_HANDLE :xmp handle
 * @param[in] pszNameSpace HI_CHAR* : NameSpace
 * @param[in] pszkey HI_CHAR* : key
 * @param[out] pszValue HI_CHAR* : return Value
 * @param[in/out] ps32ValueBuflen HI_S32* : Value malloc Buflen
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_GetProperty(HI_MW_PTR pXMP, const HI_CHAR *pszNameSpace, const HI_CHAR *pszkey, HI_CHAR *pszValue,
                          HI_S32 *ps32ValueBuflen);

/**
 * @brief  Add XMP properties and attribute values
 * @param[in] hXMPhandle HI_XMP_HANDLE :xmp handle
 * @param[in] pszNameSpace HI_CHAR* : NameSpace
 * @param[in] pszkey HI_CHAR* : key
 * @param[out] pszValue HI_CHAR* :  Value
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_AddProperty(HI_MW_PTR pXMP, const HI_CHAR *pszNameSpace, const HI_CHAR *pszkey,
                          const HI_CHAR *pszValue);

/**
 * @brief Get the string after the XMP data is generated
 * @param[in] hXMPhandle HI_XMP_HANDLE :xmp handle
 * @param[out] pszBuf HI_CHAR* : return string value
 * @param[in/out] ps32Buflen HI_S32* : malloc buf/buf len
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_ToString(HI_MW_PTR pXMP, HI_CHAR *pszBuf, HI_S32 *ps32Buflen);

/**
 * @brief  Gets the parsed list of XMP data attributes and attribute values
 * @param[in] hXMPhandle HI_XMP_HANDLE :xmp handle
 * @param[in] pszNameSpace HI_CHAR* : NameSpace
 * @param[in] FnKeyValue HI_XMP_HANDLE_KEYVALUE_FN* : func  key value callback
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_XMP_ForeachKeyValue(HI_MW_PTR pXMP, const HI_CHAR *pszNameSpace, HI_XMP_HANDLE_KEYVALUE_FN FnKeyValue);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_XMP_H__ */
