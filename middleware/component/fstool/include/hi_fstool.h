/**
* Copyright (C), 2016-2032, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_fstool.h
* @brief     fstool struct and interface
* @author    HiMobileCam Develop Team
* @date      2017.11.30
*/

#ifndef HI_FSTOOL_H
#define HI_FSTOOL_H

#include <sys/types.h>
#include <stdbool.h>

#define HI_FSTOOL_SUCCESS                      0            /* Success                              */
#define HI_ERR_FSTOOL_ILLEGAL_PARAM            0x201            /* Illegal param                        */
#define HI_ERR_FSTOOL_OPEN_FAILURE             0x202            /* Open operation error                 */
#define HI_ERR_FSTOOL_READ_FAILURE             0x203            /* Read FATs informations error         */
#define HI_ERR_FSTOOL_WRITE_FAILURE            0x204            /* Update FATs informations error       */
#define HI_ERR_FSTOOL_NOT_ENOUGH_MEMORY        0x205            /* Not enough memory when fsck         */
#define HI_ERR_FSTOOL_FSTYPE_UNSUPPORT         0x206            /* File System type not support         */
#define HI_ERR_FSTOOL_FS_EXCEPTION             0x207            /* FS exception and cannot repair  */
#define HI_ERR_FSTOOL_FRAGMENT_SEVERELY        0x208            /* FATs Fragment severely               */
#define HI_ERR_FSTOOL_DEVICE_EXCEPTION         0x209            /* Device is exception so can not format a FAT32 file system on it */
#define HI_ERR_FSTOOL_SYSTEM_CALL_FAILURE      0x210           /* System call failure */
#define HI_ERR_FSTOOL_NOT_64K_CLUSTER           0x211           /* Cluster size is not 64K */

typedef enum hiFSTOOL_CHECK_MODE_E
{
    HI_FSTOOL_CHECK_MODE_ALWAYS = 0x00,  /*Check FS exception always*/
    HI_FSTOOL_CHECK_MODE_DIRTY,          /*Check FS exception when FS is dirty*/
    HI_FSTOOL_CHECK_MODE_ALWAYS_MAIN_FAT,/*Check FS exception on the basis of main fat always*/
    HI_FSTOOL_CHECK_MODE_DIRTY_MAIN_FAT, /*Check FS exception on the basis of main fat when FS is dirty*/
    HI_FSTOOL_CHECK_MODE_BUTT            /***others**/
} HI_FSTOOL_CHECK_MODE_E;

typedef enum hiFSTOOL_FORMAT_MODE_E
{
    HI_FSTOOL_FORMAT_MODE_ADVANCED = 0x00,  /*Advanced format*/
    HI_FSTOOL_FORMAT_MODE_DISCARD,          /*Discard the partition and advanced format*/
    HI_FSTOOL_FORMAT_MODE_BUTT             /***others**/
} HI_FSTOOL_FORMAT_MODE_E;

typedef enum hiFSTOOL_LOG_LEVEL_E
{
    HI_FSTOOL_LOG_LEVEL_DEBUG = 0,
    HI_FSTOOL_LOG_LEVEL_INFO,
    HI_FSTOOL_LOG_LEVEL_WARN,
    HI_FSTOOL_LOG_LEVEL_ERROR,
    HI_FSTOOL_LOG_LEVEL_FATAL,
    HI_FSTOOL_LOG_LEVEL_BUTT
} HI_FSTOOL_LOG_LEVEL_E;

/** allocates u32BufSize bytes memory*/
typedef int (*HI_FSTOOL_BUF_ALLOC_FN)(void* pPhyAddr, size_t szBufSize, void** ppVmAddr);

/** frees the memory space pointed to by pVmAddr*/
typedef int (*HI_FSTOOL_BUF_FREE_FN)(void* pPhyAddr, void* pVmAddr);

typedef struct hiFSTOOL_BUF_OPS_S
{
    HI_FSTOOL_BUF_ALLOC_FN pfnBufAlloc;                        /**< allocates u32BufSize bytes memory*/
    HI_FSTOOL_BUF_FREE_FN  pfnBufFree;                         /**< frees the memory space*/
} HI_FSTOOL_BUF_OPS_S;

typedef struct hiFSTOOL_CHECK_CFG
{
    HI_FSTOOL_CHECK_MODE_E enMode;                        /**< file system check mode*/
    unsigned int u32FragmentThr;                         /**< inverse proportion to the fragment  degree*/
} HI_FSTOOL_CHECK_CFG_S;


typedef struct hiFSTOOL_FORMAT_CFG
{
    HI_FSTOOL_FORMAT_MODE_E enMode;                        /**< fscheck mode*/
    unsigned int u32ClusterSize;                         /**< cluster size,unit Byte*/
    bool enable4KAlignCheck;                           /**< 4K align check enable or disable flag*/
} HI_FSTOOL_FORMAT_CFG_S;


/**
 * @brief register buffer ops to module
 * @param[in]pstOPS : buffer ops
 * on success, 0 is returned. On error, error code returned
*/
int HI_FSTOOL_RegisterBufOps(const HI_FSTOOL_BUF_OPS_S* pstOPS);

/**
 * @brief unregister buffer ops
*/
void HI_FSTOOL_UnregisterBufOps(void);

/**
 * @brief set enabled log level, logs with equal or higher level than enabled will be output. The default level is HI_FSTOOL_LOG_LEVEL_ERROR
 * @param[in]enLogLevel : enabled log level
 * on success, 0 is returned. On error, error code returned
*/
int HI_FSTOOL_SetLogLevel(HI_FSTOOL_LOG_LEVEL_E enLogLevel);

/**
 * @brief check the fsystem file.
 * @param[in]pszPath : the path name for check
 * @param[in]pstCheckCfg   : the point of file system check config.
 * on success, 0 is returned. On error, error code returned
*/
int HI_FSTOOL_Check(const char* pszPath, HI_FSTOOL_CHECK_CFG_S *pstCheckCfg);

/**
 * @brief format the fsystem file.
 * @param[in]paszPath       : the path name of device
 * @param[in]pstFormatCfg     : the point of format config.
 * on success, 0 is returned. On error, error code returned
*/
int HI_FSTOOL_Format(const char* pszPath, HI_FSTOOL_FORMAT_CFG_S *pstFormatCfg);

#endif
