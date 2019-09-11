/**
 * @file    hi_filemng_comm.h
 * @brief   define file manager comm struct
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

*/
#ifndef __HI_FILEMNG_COMM_H__
#define __HI_FILEMNG_COMM_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     FILEMNG */
/** @{ */  /** <!-- [FILEMNG] */

/** macro define */
#define HI_FILEMNG_MAX_DATETIME_LEN     (20)/**<max date time string length */
#define HI_FILEMNG_MAX_FILECNT_IN_OBJ   (6)/**<max file count in one file object */


/** error code define */
#define HI_FILEMNG_EINVAL                   HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EINVAL)/**<Invalid argument */
#define HI_FILEMNG_ENOTINIT                 HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ENOINIT)/**<Not inited */
#define HI_FILEMNG_ELOST                    HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ELOST)/**<No such file */
#define HI_FILEMNG_ENOTREADY                HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ENOTREADY)/**<No such device or address */
#define HI_FILEMNG_EACCES                   HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EACCES)/**<Permission denied */
#define HI_FILEMNG_EEXIST                   HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EEXIST)/**<File exists */
#define HI_FILEMNG_EFULL                    HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EFULL)/**<No space left on device */
#define HI_FILEMNG_EBUSY                    HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EBUSY)/**<Operation now in progress */
#define HI_FILEMNG_ENORES                   HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ENORES)/**<Too many files,not enough filename */
#define HI_FILEMNG_EINTER                   HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EINTER)/**<Internal error */
#define HI_FILEMNG_EINITIALIZED             HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_EINITIALIZED)/**<Already Initialized */
#define HI_FILEMNG_ENOTMAIN                 HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ERRNO_CUSTOM_BOTTOM+1)/**<Not Basic File */
#define HI_FILEMNG_EUNIDENTIFICATION        HI_APPCOMM_ERR_ID(HI_APP_MOD_FILEMNG, HI_ERRNO_CUSTOM_BOTTOM+2)/**<Too many Unrecognized files */

/** struct define */
/** file repair configuration */
typedef struct hiFILEMNG_REPAIR_CFG_S {
    HI_BOOL bEnable; /**<file repair enable or not */
    HI_BOOL bParseExtraMdat; /**<parse extra Mdat or not */
    HI_CHAR szRootPath[HI_APPCOMM_MAX_PATH_LEN]; /**<file repair root path. eg."/app/private/" */
    HI_CHAR szTopDirName[HI_APPCOMM_MAX_PATH_LEN]; /**<file repair top directory name in root path. eg."DCIM" */
} HI_FILEMNG_REPAIR_CFG_S;

/** file type */
typedef enum hiFILEMNG_FILE_TYPE_E {
    HI_FILEMNG_FILE_TYPE_RECORD = 0,/**<record file. eg. *.MP4,*.LRV,*.MOV,etc */
    HI_FILEMNG_FILE_TYPE_PHOTO,/**<photo file. eg. *.JPG,*.DNG,etc */
    HI_FILEMNG_FILE_TYPE_BUTT
} HI_FILEMNG_FILE_TYPE_E;

/** file information */
typedef struct hiFILEMNG_FILE_INFO_S {
    HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN];/**<file name ,eg. "/app/sd/HiCAM/Photo/2017_05_27_11281500.JPG" */
    HI_CHAR szCreateTime[HI_FILEMNG_MAX_DATETIME_LEN];   /**<file create time ,eg."2017/05/27 11:28:15" */
    HI_U64  u64FileSize_byte;/**<file size in bytes. eg. 120,100,100 */
    HI_U32  u32Duration_sec;/**<record file duration in seconds. eg. 300 */
    HI_U8   u8Flag;/**<mark flag. 0 means no mark */
} HI_FILEMNG_FILE_INFO_S;

/** file name list in file object */
typedef struct hiFILEMNG_OBJ_FILENAME_S {
    HI_U8   u8FileCnt;/**<file count in the file object */
    HI_CHAR szFileName[HI_FILEMNG_MAX_FILECNT_IN_OBJ][HI_APPCOMM_MAX_PATH_LEN];/**<file name list int the file object */
} HI_FILEMNG_OBJ_FILENAME_S;

/** file object information*/
typedef struct hiFILEMNG_FILEOBJ_S {
    HI_FILEMNG_FILE_INFO_S stBasicFile;/**<basic file information*/
    HI_FILEMNG_OBJ_FILENAME_S stFileNames;/**<file name list in the file object*/
} HI_FILEMNG_FILEOBJ_S;

/** file manager configuration */
typedef struct hiFILEMNG_COMM_CFG_S {
    HI_CHAR szMntPath[HI_APPCOMM_MAX_PATH_LEN];/**<disk mount path. eg."/app/sd/"*/
    HI_FILEMNG_REPAIR_CFG_S stRepairCfg;/**<file repair configuration */
} HI_FILEMNG_COMM_CFG_S;

/** event ID define */
typedef enum hiEVENT_FILEMNG_E {
    HI_EVENT_FILEMNG_SCAN_COMPLETED = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 0),
    HI_EVENT_FILEMNG_SCAN_FAIL      = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 1),
    HI_EVENT_FILEMNG_SPACE_FULL     = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 2),
    HI_EVENT_FILEMNG_SPACE_ENOUGH   = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 3),
    HI_EVENT_FILEMNG_REPAIR_BEGIN   = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 4),
    HI_EVENT_FILEMNG_REPAIR_END     = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 5),
    HI_EVENT_FILEMNG_REPAIR_FAILED  = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 6),
    HI_EVENT_FILEMNG_UNIDENTIFICATION = HI_APPCOMM_EVENT_ID(HI_APP_MOD_FILEMNG, 7),
    HI_EVENT_FILEMNG_BUTT
} HI_EVENT_FILEMNG_E;

/** @}*/  /** <!-- ==== FILEMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_FILEMNG_COMM_H__*/

