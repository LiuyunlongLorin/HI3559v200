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

#include "inode/inode.h"
#define HI_FSTOOL_SUCCESS                      0            /* Success                              */
#define HI_ERR_FSTOOL_ILLEGAL_PARAM            1            /* Illegal param                        */
#define HI_ERR_FSTOOL_OPEN_FAILURE             2            /* Open operation error                 */
#define HI_ERR_FSTOOL_READ_FAILURE             3            /* Read FATs informations error         */
#define HI_ERR_FSTOOL_WRITE_FAILURE            4            /* Update FATs informations error       */
#define HI_ERR_FSTOOL_NOT_ENOUGH_MEMORY        5            /* Not enough memory when fsck         */
#define HI_ERR_FSTOOL_FSTYPE_UNSUPPORT         6            /* File System type not support         */
#define HI_ERR_FSTOOL_FS_EXCEPTION             7            /* FS exception and cannot repair  */
#define HI_ERR_FSTOOL_FRAGMENT_SEVERELY        8            /* FATs Fragment severely               */
#define HI_ERR_FSTOOL_DEVICE_EXCEPTION         9            /* Device is exception so can not format a FAT32 file system on it */
#define HI_ERR_FSTOOL_SYSTEM_CALL_FAILURE      10           /* System call failure */

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

/**
 * @brief set enabled log level, logs with equal or higher level than enabled will be output. The default level is HI_FSTOOL_LOG_LEVEL_ERROR
 * @param[in]enLogLevel : enabled log level
 * on success, 0 is returned. On error, error code returned
*/
int HI_FSTOOL_SetLogLevel(HI_FSTOOL_LOG_LEVEL_E enLogLevel);

/**
 * @brief check the fsystem file.
 * @param[in]pszPath : the path name for check
 * @param[in]u32FragmentThr   : the u32FragmentThr in inverse proportion to the fragment  degree.
 * @param[in]enMode : see HI_FSTOOL_CHECK_MODE_E
 * on success, 0 is returned. On error, error code returned
*/
u_int HI_FSTOOL_Check(const char* pszPath, unsigned int u32FragmentThr, HI_FSTOOL_CHECK_MODE_E enMode);


#endif
