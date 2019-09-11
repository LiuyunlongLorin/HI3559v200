/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_datatime.h
* @brief     datatime module header file
* @author    HiMobileCam middleware develop team
* @date      2017.02.09
*/

#ifndef HI_DTCF_H_
#define HI_DTCF_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "hi_mw_type.h"

/** \addtogroup     DTCF */
/** @{ */  /** <!-- [DTCF] */

#define MODULE_NAME_DTCF  "DTCF"

#define HI_FILE_PATH_LEN_MAX         (256)
#define HI_DIR_LEN_MAX               (64)

/** DataTime Error number base */
#define HI_DTCF_ERRNO_BASE 0x2A00
/** Parameter is invalid */
#define HI_DTCF_ERR_EINVAL_PAEAMETER    (HI_DTCF_ERRNO_BASE + 1)
/** Null pointer*/
#define HI_DTCF_ERR_NULL_PTR            (HI_DTCF_ERRNO_BASE + 2)
/** failure caused by malloc memory */
#define HI_DTCF_ERR_NOMEM               (HI_DTCF_ERRNO_BASE + 3)
/** not support file path */
#define HI_DTCF_UNSUPPORT_PATH          (HI_DTCF_ERRNO_BASE + 4)
/** the same directory */
#define HI_DTCF_SAME_DIR_PATH           (HI_DTCF_ERRNO_BASE + 5)
/** Status error, operation not allowed*/
#define HI_DTCF_ERR_STATUS_ERROR        (HI_DTCF_ERRNO_BASE + 6)
/** Empty directory*/
#define HI_DTCF_ERR_EMPTEY_DIR          (HI_DTCF_ERRNO_BASE + 7)
/** directory not define*/
#define HI_DTCF_ERR_UNDEFINE_DIR        (HI_DTCF_ERRNO_BASE + 8)
/** System error*/
#define HI_DTCF_ERR_SYSTEM_ERROR        (HI_DTCF_ERRNO_BASE + 9)
/** the same filename */
#define HI_DTCF_SAME_FILENAME_PATH      (HI_DTCF_ERRNO_BASE + 10)
/** the path is not a directory */
#define HI_DTCF_PATH_IS_NOT_DIR_ERROR   (HI_DTCF_ERRNO_BASE + 11)

/**
 * File Directory Type
 */
typedef enum hiDTCF_DIR_E
{
    DTCF_DIR_EMR_FRONT = 0,
    DTCF_DIR_EMR_FRONT_SUB,
    DTCF_DIR_NORM_FRONT,
    DTCF_DIR_NORM_FRONT_SUB,
    DTCF_DIR_PARK_FRONT,
    DTCF_DIR_PARK_FRONT_SUB,
    DTCF_DIR_EMR_REAR,
    DTCF_DIR_EMR_REAR_SUB,
    DTCF_DIR_NORM_REAR,
    DTCF_DIR_NORM_REAR_SUB,
    DTCF_DIR_PARK_REAR,
    DTCF_DIR_PARK_REAR_SUB,
    DTCF_DIR_PHOTO_FRONT,
    DTCF_DIR_PHOTO_REAR,
    DTCF_DIR_BUTT
} HI_DTCF_DIR_E;

/**
 * File Type.
 */
typedef enum hiDTCF_FILE_TYPE_E
{
    HI_DTCF_FILE_TYPE_MP4,
    HI_DTCF_FILE_TYPE_JPG,
    HI_DTCF_FILE_TYPE_TS,
    HI_DTCF_FILE_TYPE_BUTT
} HI_DTCF_FILE_TYPE_E;

/**
  *    @brief init DTCF
  *
  *    init DTCF
  *    @param[in]  pazRootDir:  root of DTCF
  *    @param[in]  azDirNames: level two directory names
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_DTCF_Init(const HI_CHAR *pazRootDir, const HI_CHAR azDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX]);

/**
  *    @brief get level two directory names
  *
  *    create DTCF
  *    @param[out] pazDirNames: level two directory names
  *    @param[in]  u32DirAmount: level two directory amount
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetDirNames(HI_CHAR (*pazDirNames)[HI_DIR_LEN_MAX], HI_U32 u32DirAmount);

/**
  *    @brief DeInit DTCF
  *
  *    DeInit DTCF
  *    @return     0 success, non-zero error code
  */
HI_S32 HI_DTCF_DeInit(void);

/**
  *    @brief scan files
  *
  *    scan files in some directories.
  *    @param[in]    enDirs:  directories to scan.
  *    @param[in]    u32DirCount: count of directory.
  *    @param[out]   pu32FileAmount:files amount.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_Scan(HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32FileAmount);

/**
  *    @brief get file by index.
  *    @param[in]    u32Index:  index of file to get.
  *    @param[out]   pazFileName: file name.
  *    @param[in]    u32Length:  file length .
  *    @param[out]   penDir: file directory.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetFileByIndex(HI_U32 u32Index, HI_CHAR *pazFileName, HI_U32 u32Length, HI_DTCF_DIR_E *penDir);

/**
  *    @brief delete file by index.
  *    @param[in]    u32Index:  index of file to delete.
  *    @param[out]   pu32FileAmount: file amount.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_DelFileByIndex(HI_U32 u32Index, HI_U32 *pu32FileAmount);

/**
  *    @brief add file.
  *    @param[in]    pazSrcFilePath:  file path to add.
  *    @param[in]    enDir:  file dir type to add.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_AddFile(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E enDir);

/**
  *    @brief get oldest file index.
  *    @param[in]    enDirs[DTCF_DIR_BUTT]:  the oldest file enDir.
  *    @param[in]    u32DirCount:  the oldest file enDir count.
  *    @param[out]    pu32Index:  the oldest file index.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetOldestFileIndex(HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32Index);

/**
  *    @brief get file amount.
  *    @param[out]   pu32FileAmount: file amount.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetFileAmount(HI_U32 *pu32FileAmount);

/**
  *    @brief get oldest file in directory.
  *    @param[in]    enDirs:  directory type.
  *    @param[out]   pazFilePath: get oldest file full path.
  *    @param[in]   u32Length: get oldest file full path length.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetOldestFilePath(HI_DTCF_DIR_E enDir, HI_CHAR *pazFilePath, HI_U32 u32Length);

/**
  *    @brief create file full path
  *    @param[in]    enFileType:  type of file.
  *    @param[in]    enDir: type of directory.
  *    @param[out]   pazFilePath: file full path.
  *    @param[in]   u32Length: file full path length.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_CreateFilePath(HI_DTCF_FILE_TYPE_E enFileType, HI_DTCF_DIR_E enDir, HI_CHAR *pazFilePath, HI_U32 u32Length);

/**
  *    @brief get related file full path.
  *    @param[in]    pazSrcFilePath:  source file full path.
  *    @param[in]    enDir: related file directory.
  *    @param[out]   pazDstFilePath : related file full path.
  *    @param[in]   u32Length: file full path length.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetRelatedFilePath(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E enDir, HI_CHAR *pazDstFilePath, HI_U32 u32Length);

/**
  *    @brief get emergency file full path.
  *    @param[in]    pazSrcFilePath:  source file full path.
  *    @param[out]   pazDstFilePath : Emr file full path.
  *    @param[in]   u32Length: file full path length.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetEmrFilePath(const HI_CHAR *pazSrcFilePath, HI_CHAR *pazDstFilePath, HI_U32 u32Length);

/**
  *    @brief get file directory type.
  *    @param[in]    pazSrcFilePath:  source file full path.
  *    @param[out]   penDir : file directory type.
  *    @return       0 success, non-zero error code
  */
HI_S32 HI_DTCF_GetFileDirType(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E *penDir);

/** @}*/  /** <!-- ==== DTCF End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_DTCF_H_ */
