/**
 * @file    filemng_common.c
 * @brief   file manager common function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifndef __HuaweiLite__
#include <linux/msdos_fs.h>
#endif
#include "hi_mp4_format.h"
#include "hi_appcomm_util.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_EXTENSION_LENGTH (4)

extern HI_S32 FILEMNG_MARKER_GetFlag(const HI_CHAR *pszFileName, HI_U8 *pu8Flag);

static HI_S32 FILEMNG_GetFileSuffix(const HI_CHAR *pszFileName, HI_CHAR *pszSuffix)
{
    const HI_CHAR *nameSuffix = strrchr(pszFileName, '.');
    if (NULL == nameSuffix) {
        MLOGE("input filename don't have . suffix \n");
        return HI_FILEMNG_EINVAL;
    }

    if (strnlen(nameSuffix, FILEMNG_EXTENSION_LENGTH + 1) > FILEMNG_EXTENSION_LENGTH ||
            strnlen(nameSuffix, FILEMNG_EXTENSION_LENGTH)  <  2) {
        MLOGE("input filename .suffix:%s  length too long, max len:%d \n", nameSuffix, FILEMNG_EXTENSION_LENGTH);
        return HI_FILEMNG_EINVAL;
    }

    snprintf(pszSuffix, FILEMNG_EXTENSION_LENGTH, nameSuffix + 1);
    return HI_SUCCESS;
}

HI_CHAR *FILEMNG_Strerror(HI_S32 s32ErrorCode)
{
    HI_CHAR *pszStrerror = NULL;

    switch (s32ErrorCode) {
        case HI_FILEMNG_EINVAL:
            pszStrerror = (HI_CHAR *)"Invalid argument";
            break;
        case HI_FILEMNG_ENOTINIT:
            pszStrerror = (HI_CHAR *)"Not inited";
            break;
        case HI_FILEMNG_EINITIALIZED:
            pszStrerror = (HI_CHAR *)"Already Initialized";
            break;
        case HI_FILEMNG_ELOST:
            pszStrerror = (HI_CHAR *)"No such file";
            break;
        case HI_FILEMNG_ENOTREADY:
            pszStrerror = (HI_CHAR *)"No such device or address";
            break;
        case HI_FILEMNG_EACCES:
            pszStrerror = (HI_CHAR *)"Permission denied";
            break;
        case HI_FILEMNG_EEXIST:
            pszStrerror = (HI_CHAR *)"File exists";
            break;
        case HI_FILEMNG_EFULL:
            pszStrerror = (HI_CHAR *)"No space left on device";
            break;
        case HI_FILEMNG_EBUSY:
            pszStrerror = (HI_CHAR *)"Operation now in progress";
            break;
        case HI_FILEMNG_ENORES:
            pszStrerror = (HI_CHAR *)"Too many files,not enough filename";
            break;
        case HI_FILEMNG_EINTER:
            pszStrerror = (HI_CHAR *)"Internal error";
            break;
        case HI_FILEMNG_ENOTMAIN:
            pszStrerror = (HI_CHAR *)"Not Basic File";
            break;
        case HI_SUCCESS:
            pszStrerror = (HI_CHAR *)"Success";
            break;
        default:
            pszStrerror = (HI_CHAR *)"Unknown error";
            break;
    }

    return  pszStrerror;
}

HI_S32 FILEMNG_CheckPath(const HI_CHAR *pszMntPath, const HI_CHAR *pszRootDir)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szRootPath[HI_APPCOMM_MAX_PATH_LEN];
    s32Ret = HI_PathIsDirectory(pszMntPath);

    if (1 != s32Ret) {
        return HI_FILEMNG_ENOTREADY;
    }

    snprintf(szRootPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", pszMntPath, pszRootDir);
    s32Ret = HI_PathIsDirectory(szRootPath);
    if (1 == s32Ret) {
        return HI_SUCCESS;
    } else if (-1 == s32Ret) {
        s32Ret = HI_mkdir(szRootPath, 0777);
        if (HI_SUCCESS != s32Ret) {
            return HI_FILEMNG_EACCES;
        }
    } else {
        return HI_FILEMNG_EEXIST;
    }

    return HI_SUCCESS;
}

HI_BOOL FILEMNG_IsMP4(const HI_CHAR *pszFilePath)
{
    HI_CHAR szSuffix[FILEMNG_EXTENSION_LENGTH] = {0};
    if (HI_SUCCESS == FILEMNG_GetFileSuffix(pszFilePath, szSuffix)) {
        if ((0 == strcasecmp(szSuffix, "MP4")) || (0 == strcasecmp(szSuffix, "LRV"))) {
            return HI_TRUE;
        }
    }
    return HI_FALSE;
}

HI_S32 FILEMNG_GetFileInfo(const HI_CHAR *pszFilePath, HI_FILEMNG_FILE_INFO_S *pstFileInfo)
{
    struct stat FileStat;
    memcpy(pstFileInfo->szAbsPath, pszFilePath, HI_APPCOMM_MAX_PATH_LEN);

    if (0 == stat(pszFilePath, &FileStat)) {
        pstFileInfo->u64FileSize_byte = FileStat.st_size;
        struct tm time;
        localtime_r(&FileStat.st_mtime, &time);
        snprintf(pstFileInfo->szCreateTime, HI_FILEMNG_MAX_DATETIME_LEN, "%04d/%02d/%02d %02d:%02d:%02d",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                 time.tm_hour, time.tm_min, time.tm_sec);
        pstFileInfo->u32Duration_sec = 0;

        if (FILEMNG_IsMP4(pszFilePath)) {
            HI_MW_PTR Handle;
            HI_MP4_CONFIG_S stMP4Cfg;
            snprintf(stMP4Cfg.aszFileName, HI_MP4_MAX_FILE_NAME, pszFilePath);
            stMP4Cfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
            stMP4Cfg.stDemuxerConfig.u32VBufSize = 1 << 20;

            if (HI_SUCCESS == HI_MP4_Create(&Handle, &stMP4Cfg)) {
                HI_U64 u64Duration = 0;
                HI_MP4_Destroy(Handle, &u64Duration);
                pstFileInfo->u32Duration_sec = u64Duration;
            }
        }

        FILEMNG_MARKER_GetFlag(pszFilePath, &pstFileInfo->u8Flag);
    } else {
        pstFileInfo->u64FileSize_byte = 0;
        MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
        return HI_FILEMNG_ELOST;
    }

    return HI_SUCCESS;
}

HI_S32 FILEMNG_HideFile(const HI_CHAR *pFilePath, HI_BOOL bHide)
{
    HI_S32 s32Ret = HI_SUCCESS;
#ifndef __HuaweiLite__
    HI_S32 fd = open(pFilePath, O_RDWR);
    if (fd == -1) {
        MLOGE("open %s error:%s\n", pFilePath, strerror(errno));
        return HI_FAILURE;
    } else {
        HI_U32 attr;
        s32Ret = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr);
        if (s32Ret == -1) {
            MLOGE("ioctl FAT_IOCTL_GET_ATTRIBUTES error:%s\n", strerror(errno));
        } else {
            if (bHide == HI_TRUE) {
                attr |= ATTR_HIDDEN;
            } else {
                attr &= ~ATTR_HIDDEN;
            }
            s32Ret = ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attr);
            if (s32Ret == -1) {
                MLOGE("ioctl FAT_IOCTL_SET_ATTRIBUTES error:%s\n", strerror(errno));
            }
        }
        close(fd);
    }
#endif
    return s32Ret;
}

HI_S32 FILEMNG_HideDir(const HI_CHAR *pDirPath, HI_BOOL bHide)
{
    HI_S32 s32Ret = HI_SUCCESS;
#ifndef __HuaweiLite__
    DIR *dp = opendir(pDirPath);
    if (dp == NULL) {
        MLOGE("open %s error:%s\n", pDirPath, strerror(errno));
        return HI_FAILURE;
    } else {
        HI_U32 attr;
        HI_S32 fd = dirfd(dp);
        s32Ret = ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attr);
        if (s32Ret == -1) {
            MLOGE("ioctl FAT_IOCTL_GET_ATTRIBUTES error:%s\n", strerror(errno));
        } else {
            if (bHide == HI_TRUE) {
                attr |= ATTR_HIDDEN;
            } else {
                attr &= ~ATTR_HIDDEN;
            }
            s32Ret = ioctl(fd, FAT_IOCTL_SET_ATTRIBUTES, &attr);
            if (s32Ret == -1) {
                MLOGE("ioctl FAT_IOCTL_SET_ATTRIBUTES error:%s\n", strerror(errno));
            }
        }
        closedir(dp);
    }
#endif
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

