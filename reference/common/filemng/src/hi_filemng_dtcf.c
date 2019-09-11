/**
 * @file    hi_filemng_dtcf.c
 * @brief   DTCF file manager function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __HuaweiLite__
#include <fs/fs.h>
#endif
#include "hi_eventhub.h"
#include "hi_appcomm_util.h"
#include "hi_filemng_dtcf.h"
#include "filemng_comm.h"

#if defined(__UCLIBC__)&&!defined(__HuaweiLite__)
#include <sys/syscall.h>
#define FALLOC_FL_KEEP_SIZE 0x01
#define HIDWORD(a) ((HI_U32)(((HI_U64)(a)) >> 32))
#define LODWORD(a) ((HI_U32)(HI_U64)(a))
#define fallocate(fd, mode, offset, len) syscall(__NR_fallocate,fd,mode,LODWORD(offset),HIDWORD(offset),LODWORD(len),HIDWORD(len))
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_PREALLOC_FILE_MAX_NUM (100)
#define FILEMNG_PREALLOC_FILE_NAME_PREFIX ".PreAllocFile"
#define FILEMNG_PrintInfo() {MLOGI("FileObjCount %u(movie:%u+emr:%u+photo:%u) \n",s_u32FILEMNGAllObjCnt, s_u32FILEMNGRecObjCnt - s_u32FILEMNGEmrRecObjCnt, s_u32FILEMNGEmrRecObjCnt, s_u32FILEMNGPhotoObjCnt);}

typedef struct tagFILEMNG_PHOTO_QUEUE_S {
    HI_U32 u32Index;
    HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];
} FILEMNG_PHOTO_QUEUE_S;

typedef struct tagFILEMNG_PHOTO_CTX_S {
    HI_U32 u32Count;
    FILEMNG_PHOTO_QUEUE_S *List;
} FILEMNG_PHOTO_CTX_S;

static pthread_mutex_t s_FILEMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static HI_FILEMNG_COMM_CFG_S s_stFILEMNGCfg;
static HI_FILEMNG_DTCF_CFG_S s_stFILEMNGDTCF_Cfg;
static HI_BOOL s_bFILEMNGInit = HI_FALSE;
static FILEMNG_DISK_STATE_E s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE; /**<0-not available;1-available;2-scan completed */
static HI_U32 s_u32FILEMNGRecObjCnt   = 0;/**<record file object count */
static HI_U32 s_u32FILEMNGEmrRecObjCnt = 0;/**<emr record file object count */
static HI_U32 s_u32FILEMNGPhotoObjCnt = 0;/**<photo file object count */
static HI_U32 s_u32FILEMNGAllObjCnt   = 0;/**<all file object count */
static FILEMNG_PHOTO_CTX_S g_Photos[HI_FILEMNG_DTCF_MAX_PHOTO_DIR];
static HI_DTCF_DIR_E s_aenDirs[DTCF_DIR_BUTT];
static HI_U32 s_u32DirCount = 0;
static HI_U32 s_u32MaxDirCount = 0;
static HI_S32 s_s32WritingRecCnt = 0;

/**-------------------------internal function interface------------------------- */

static HI_S32 FILEMNG_Disable(HI_VOID)
{
    HI_S32 s32Ret = HI_DTCF_DeInit();
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_DeInit, s32Ret);
    }

    s_u32FILEMNGRecObjCnt = 0;
    s_u32FILEMNGPhotoObjCnt = 0;
    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetMovieSpace(HI_U32 *pu32Size_MB)
{
    HI_U64 u64Size_KB = 0;
    HI_U64 u64MovieSpace = 0;
    HI_CHAR szPath[HI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64MovieSpace += u64Size_KB;
    }

    *pu32Size_MB = u64MovieSpace >> 10;
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetEmrSpace(HI_U32 *pu32Size_MB)
{
    HI_U64 u64Size_KB = 0;
    HI_U64 u64EmrSpace = 0;
    HI_CHAR szPath[HI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT]);
        HI_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR]);
        HI_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB]);
        HI_du(szPath, &u64Size_KB);
        u64EmrSpace += u64Size_KB;
    }

    *pu32Size_MB = u64EmrSpace >> 10;
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetPhotoSpace(HI_U32 *pu32Size_MB)
{
    HI_U64 u64Size_KB = 0;
    HI_U64 u64PhotoSpace = 0;
    HI_CHAR szPath[HI_APPCOMM_MAX_PATH_LEN];

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT]);
        HI_du(szPath, &u64Size_KB);
        u64PhotoSpace += u64Size_KB;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], HI_DIR_LEN_MAX)) {
        snprintf(szPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s",
                 s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR]);
        HI_du(szPath, &u64Size_KB);
        u64PhotoSpace += u64Size_KB;
    }

    *pu32Size_MB = u64PhotoSpace >> 10;
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetEmrCnt(HI_U32 *pu32Count)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32DirCount = 0;
    HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];

    /**scan scope */
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_REAR;
    }

    if (u32DirCount > 0) {
        s32Ret = HI_DTCF_Scan(enDirs, u32DirCount, pu32Count);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_DTCF_Scan, s32Ret);
            return HI_FILEMNG_EINTER;
        }
    } else {
        *pu32Count = 0;
    }
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_CreatePreAllocFile(HI_CHAR *pszFileName, HI_U32 u32PreAllocUnit)
{
    HI_S32 s32Ret = 0;
    HI_S32 fd = -1;
    fd = open(pszFileName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (0 >= fd) {
        MLOGE("create prealloc file[%s] fail:%s\n", pszFileName, strerror(errno));
        return HI_FILEMNG_EINTER;
    }

    s32Ret = fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, u32PreAllocUnit);
    if (HI_SUCCESS != s32Ret) {
        close(fd);
        MLOGE("fallocate error:%s\n", strerror(errno));
        return HI_FILEMNG_EINTER;
    }

    ftruncate(fd, u32PreAllocUnit);
    close(fd);
    return FILEMNG_HideFile(pszFileName, HI_TRUE);
}

static HI_S32 FILEMNG_PreAllocFileCheck(HI_CHAR *pszFileName, HI_U32 u32PreAllocUnit)
{
    struct stat FileStat;
    if (0 != stat(pszFileName, &FileStat)) {
        MLOGE("stat %s error:%s\n", pszFileName, strerror(errno));
        return HI_FILEMNG_EINTER;
    }

    if (u32PreAllocUnit != FileStat.st_size) {
        MLOGW("PreAllocFile(%lu) is not expected(%u)\n", (HI_UL)FileStat.st_size, u32PreAllocUnit);
        return HI_FILEMNG_EINTER;
    }
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetPhotoCntByDir(HI_DTCF_DIR_E enDir, HI_U32 *pu32Count)
{
    HI_S32 s32Ret = 0;
    HI_S32 i, j;
    HI_U32 u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;
    HI_DTCF_DIR_E enDirs[1] = {enDir};
    s32Ret = HI_DTCF_Scan(enDirs, 1, pu32Count);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_Scan, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    HI_BOOL PreAllocUnitChecked = HI_FALSE;

    /**create prealloc photo files */
    if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
        if (*pu32Count > s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            MLOGW("The count of photos %u is over pre allocation %u\n", *pu32Count, s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]);
        }

        for (i = 0, j = *pu32Count - 1; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            if (i + *pu32Count < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
                /*New pre allocated files join the team at first.*/
                HI_U32 number = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - *pu32Count - i - 1;
                snprintf(g_Photos[u32DirIdx].List[i].szFileName, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s/%s_%02d",
                         s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[enDir],
                         FILEMNG_PREALLOC_FILE_NAME_PREFIX, number);
                HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];

                if (NULL == realpath(g_Photos[u32DirIdx].List[i].szFileName, szFileName)) {
                    /**PreAllocFile not exsit,create it */
                    s32Ret = FILEMNG_CreatePreAllocFile(g_Photos[u32DirIdx].List[i].szFileName,
                                                        s_stFILEMNGDTCF_Cfg.u32PreAllocUnit[u32DirIdx]);
                    if (HI_SUCCESS != s32Ret) {
                        HI_LOG_PrintFuncErr(FILEMNG_CreatePreAllocFile, s32Ret);
                        return HI_FILEMNG_EINTER;
                    }
                } else {
                    if (PreAllocUnitChecked == HI_FALSE) {
                        /**PreAllocFile exsit,check filesize is expected or not*/
                        s32Ret = FILEMNG_PreAllocFileCheck(g_Photos[u32DirIdx].List[i].szFileName,
                                                           s_stFILEMNGDTCF_Cfg.u32PreAllocUnit[u32DirIdx]);
                        if (HI_SUCCESS != s32Ret) {
                            HI_EVENT_S stEvent = {.EventID = HI_EVENT_FILEMNG_UNIDENTIFICATION};
                            HI_EVTHUB_Publish(&stEvent);
                            HI_LOG_PrintFuncErr(FILEMNG_PreAllocFileCheck, s32Ret);
                            return HI_FILEMNG_EINTER;
                        }

                        PreAllocUnitChecked = HI_TRUE; /*Check each directory only once to improve performance*/
                    }
                }
            } else {
                /*then existing files join the team*/
                HI_DTCF_DIR_E enDir;
                s32Ret = HI_DTCF_GetFileByIndex(j, g_Photos[u32DirIdx].List[i].szFileName, HI_APPCOMM_MAX_PATH_LEN, &enDir);
                if (HI_SUCCESS != s32Ret) {
                    HI_LOG_PrintFuncErr(HI_DTCF_GetFileByIndex, s32Ret);
                    return HI_FILEMNG_EINTER;
                }

                j--;
            }

            g_Photos[u32DirIdx].List[i].u32Index = i;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetPhotoCnt(HI_U32 *pu32Count)
{
    HI_S32 s32Ret = 0;
    *pu32Count = 0;

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
        s32Ret = FILEMNG_GetPhotoCntByDir(DTCF_DIR_PHOTO_FRONT, &g_Photos[0].u32Count);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(FILEMNG_GetPhotoCntByDir, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        *pu32Count += g_Photos[0].u32Count;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], HI_DIR_LEN_MAX)) {
        s32Ret = FILEMNG_GetPhotoCntByDir(DTCF_DIR_PHOTO_REAR, &g_Photos[1].u32Count);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(FILEMNG_GetPhotoCntByDir, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        *pu32Count += g_Photos[1].u32Count;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GetTotalCnt(HI_U32 *pu32Count)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32DirCount = 0;
    HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];

    /**scan scope */
    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_NORM_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PARK_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_EMR_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_NORM_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PARK_REAR;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PHOTO_FRONT;
    }

    if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], HI_DIR_LEN_MAX)) {
        enDirs[u32DirCount++] = DTCF_DIR_PHOTO_REAR;
    }

    if (u32DirCount > 0) {
        s32Ret = HI_DTCF_Scan(enDirs, u32DirCount, &s_u32FILEMNGAllObjCnt);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_DTCF_Scan, s32Ret);
            return HI_FILEMNG_EINTER;
        }
    } else {
        *pu32Count = 0;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_AddRecord(const HI_CHAR *pszFilePath, HI_DTCF_DIR_E enDir)
{
    HI_S32 s32Ret = 0;
    s32Ret = HI_DTCF_AddFile(pszFilePath, enDir);
    s_s32WritingRecCnt--;
    MLOGD("s_s32WritingRecCnt=%d\n", s_s32WritingRecCnt);

    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_AddFile, s32Ret);
        if (HI_DTCF_SAME_FILENAME_PATH == s32Ret) {
            return HI_FILEMNG_EEXIST;
        }
        return HI_FILEMNG_EINVAL;
    } else {
        if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir) {
            s_u32FILEMNGEmrRecObjCnt++;
        }

        s_u32FILEMNGRecObjCnt++;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_AddPhoto(const HI_CHAR *pszFilePath, HI_DTCF_DIR_E enDir)
{
    HI_S32 s32Ret = 0;
    s32Ret = HI_DTCF_AddFile(pszFilePath, enDir);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_AddFile, s32Ret);
        if (HI_DTCF_SAME_FILENAME_PATH == s32Ret) {
            return HI_FILEMNG_EEXIST;
        }
        return HI_FILEMNG_EINVAL;
    } else {
        g_Photos[enDir - DTCF_DIR_PHOTO_FRONT].u32Count++;
        s_u32FILEMNGPhotoObjCnt++;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_RemoveRecord(const HI_CHAR *pszFilePath, HI_DTCF_DIR_E enDir)
{
    HI_S32 s32Ret = 0;
    HI_CHAR  szSubFilePath[HI_APPCOMM_MAX_PATH_LEN];
    if (DTCF_DIR_EMR_FRONT == enDir || DTCF_DIR_EMR_REAR == enDir) {
        if (DTCF_DIR_EMR_FRONT == enDir &&
                0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT_SUB], HI_DIR_LEN_MAX)) {
            s32Ret = HI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_EMR_FRONT_SUB, szSubFilePath, HI_APPCOMM_MAX_PATH_LEN);
            if (HI_SUCCESS == s32Ret) {
                MLOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetRelatedFilePath, s32Ret);
                return HI_FILEMNG_EINTER;
            }
        }

        if (DTCF_DIR_EMR_REAR == enDir && 0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR_SUB], HI_DIR_LEN_MAX)) {
            s32Ret = HI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_EMR_REAR_SUB, szSubFilePath, HI_APPCOMM_MAX_PATH_LEN);
            if (HI_SUCCESS == s32Ret) {
                MLOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetRelatedFilePath, s32Ret);
                return HI_FILEMNG_EINTER;
            }
        }

        if (0 < s_u32FILEMNGEmrRecObjCnt) {
            s_u32FILEMNGEmrRecObjCnt--;
        } else {
            MLOGE("s_u32FILEMNGEmrRecObjCnt is not correct\n");
            return HI_FILEMNG_EINTER;
        }
    } else {
        if (DTCF_DIR_NORM_FRONT == enDir &&
                0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT_SUB], HI_DIR_LEN_MAX)) {
            s32Ret = HI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_NORM_FRONT_SUB, szSubFilePath, HI_APPCOMM_MAX_PATH_LEN);
            if (HI_SUCCESS == s32Ret) {
                MLOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetRelatedFilePath, s32Ret);
                return HI_FILEMNG_EINTER;
            }
        }
        if (DTCF_DIR_NORM_REAR == enDir &&
                0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR_SUB], HI_DIR_LEN_MAX)) {
            s32Ret = HI_DTCF_GetRelatedFilePath(pszFilePath, DTCF_DIR_NORM_REAR_SUB, szSubFilePath, HI_APPCOMM_MAX_PATH_LEN);
            if (HI_SUCCESS == s32Ret) {
                MLOGI("remove(%s)\n", szSubFilePath);
                remove(szSubFilePath);
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetRelatedFilePath, s32Ret);
                return HI_FILEMNG_EINTER;
            }
        }
    }
    MLOGI("remove(%s)\n", pszFilePath);
    remove(pszFilePath);
    if (0 < s_u32FILEMNGRecObjCnt) {
        s_u32FILEMNGRecObjCnt--;
        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
    } else {
        MLOGE("s_u32FILEMNGRecObjCnt is not correct\n");
        return HI_FILEMNG_EINTER;
    }
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_RemovePhoto(const HI_CHAR *pszFilePath, HI_DTCF_DIR_E enDir)
{
    HI_S32 i, j = 0;
    HI_U32 u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;

    if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
        for (i = 0; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            /*search file in team*/
            if (0 == strncmp(g_Photos[u32DirIdx].List[i].szFileName, pszFilePath, HI_APPCOMM_MAX_PATH_LEN)) {
                break;
            }
        }

        if (i == s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            /*file is not in the team,just remove it*/
            MLOGI("remove(%s)\n", pszFilePath);
            remove(pszFilePath);
        } else {
            /*file is in the team,rename it and set it's index to team header*/
            HI_U32 u32FreeIndex = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - MIN(g_Photos[u32DirIdx].u32Count,
                                  s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]);
            snprintf(g_Photos[u32DirIdx].List[i].szFileName, HI_APPCOMM_MAX_PATH_LEN, "%s%s/%s/%s_%02d", s_stFILEMNGCfg.szMntPath,
                     s_stFILEMNGDTCF_Cfg.szRootDir, s_stFILEMNGDTCF_Cfg.aszDirNames[enDir], FILEMNG_PREALLOC_FILE_NAME_PREFIX, u32FreeIndex);
            rename(pszFilePath, g_Photos[u32DirIdx].List[i].szFileName);
            MLOGI("rename %s to %s\n", pszFilePath, g_Photos[u32DirIdx].List[i].szFileName);
            FILEMNG_HideFile(g_Photos[u32DirIdx].List[i].szFileName, HI_TRUE);

            /*increase the indexs of other files*/
            for (j = 0; j < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; j++) {
                if (g_Photos[u32DirIdx].List[j].u32Index < g_Photos[u32DirIdx].List[i].u32Index) {
                    g_Photos[u32DirIdx].List[j].u32Index++;
                }
            }

            g_Photos[u32DirIdx].List[i].u32Index = 0;
        }
    } else {
        MLOGI("remove(%s)\n", pszFilePath);
        remove(pszFilePath);
    }

    if (0 < g_Photos[u32DirIdx].u32Count) {
        g_Photos[u32DirIdx].u32Count--;
        s_u32FILEMNGPhotoObjCnt--;
        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
    } else {
        MLOGE("g_Photos[%d].u32Count is not correct\n", u32DirIdx);
        return HI_FILEMNG_EINTER;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GenerateRecordName(HI_DTCF_FILE_TYPE_E enType, HI_DTCF_DIR_E enDir,
        HI_FILEMNG_OBJ_FILENAME_S *pstFileName)
{
    HI_S32 s32Ret = 0;
    memset(pstFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    HI_DTCF_DIR_E enSubDir = DTCF_DIR_BUTT;

    switch (enDir) {
        case DTCF_DIR_EMR_FRONT:
            enSubDir = DTCF_DIR_EMR_FRONT_SUB;
            break;
        case DTCF_DIR_NORM_FRONT:
            enSubDir = DTCF_DIR_NORM_FRONT_SUB;
            break;
        case DTCF_DIR_PARK_FRONT:
            enSubDir = DTCF_DIR_PARK_FRONT_SUB;
            break;
        case DTCF_DIR_EMR_REAR:
            enSubDir = DTCF_DIR_EMR_REAR_SUB;
            break;
        case DTCF_DIR_NORM_REAR:
            enSubDir = DTCF_DIR_NORM_REAR_SUB;
            break;
        case DTCF_DIR_PARK_REAR:
            enSubDir = DTCF_DIR_PARK_REAR_SUB;
            break;
        case DTCF_DIR_PHOTO_FRONT:
        case DTCF_DIR_PHOTO_REAR:
        case DTCF_DIR_BUTT:
            return HI_FILEMNG_EINVAL;
        default:
            return HI_FILEMNG_ENOTMAIN;
    }

    s32Ret = HI_DTCF_CreateFilePath(enType, enDir, pstFileName->szFileName[0], HI_APPCOMM_MAX_PATH_LEN);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_CreateFilePath, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    pstFileName->u8FileCnt++;

    if (strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[enSubDir], HI_DIR_LEN_MAX)) {
        s32Ret = HI_DTCF_GetRelatedFilePath(pstFileName->szFileName[0], enSubDir,
                                            pstFileName->szFileName[1], HI_APPCOMM_MAX_PATH_LEN);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_DTCF_CreateFilePath, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        pstFileName->u8FileCnt++;
    }

    if (HI_TRUE == s_stFILEMNGCfg.stRepairCfg.bEnable) {
        HI_S32 i = 0;
        for (i = 0; i < pstFileName->u8FileCnt; i++) {
            FILEMNG_REPAIRER_Backup(pstFileName->szFileName[i]);
        }
    }

    FILEMNG_SPACEMONITOR_CheckSpace();

    s_s32WritingRecCnt++;
    MLOGD("s_s32WritingRecCnt=%d\n", s_s32WritingRecCnt);
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_GeneratePhotoName(HI_DTCF_FILE_TYPE_E enType, HI_DTCF_DIR_E enDir, HI_BOOL bPreAlloc,
                                        HI_FILEMNG_OBJ_FILENAME_S *pstFileName)
{
    HI_S32 s32Ret = 0;
    HI_S32 i, j;
    memset(pstFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    s32Ret = HI_DTCF_CreateFilePath(enType, enDir, pstFileName->szFileName[0], HI_APPCOMM_MAX_PATH_LEN);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DTCF_CreateFilePath, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    pstFileName->u8FileCnt = 1;

    if (bPreAlloc) {
        HI_U32 u32DirIdx = enDir - DTCF_DIR_PHOTO_FRONT;
        for (i = 0; i < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]; i++) {
            if (0 == g_Photos[u32DirIdx].List[i].u32Index) {
                g_Photos[u32DirIdx].List[i].u32Index = s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] - 1;
                rename(g_Photos[u32DirIdx].List[i].szFileName, pstFileName->szFileName[0]);
                FILEMNG_HideFile(pstFileName->szFileName[0], HI_FALSE);
                HI_DTCF_DIR_E enDirType = DTCF_DIR_BUTT;
                s32Ret = HI_DTCF_GetFileDirType(g_Photos[u32DirIdx].List[i].szFileName, &enDirType);

                if (HI_SUCCESS == s32Ret && enDirType == enDir) {
                    for (j = 0; j < s_u32FILEMNGAllObjCnt; j++) {
                        HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];
                        s32Ret = HI_DTCF_GetFileByIndex(j, szFileName, HI_APPCOMM_MAX_PATH_LEN, &enDirType);
                        if (HI_SUCCESS != s32Ret) {
                            HI_LOG_PrintFuncErr(HI_DTCF_GetFileByIndex, s32Ret);
                            return HI_FILEMNG_EINTER;
                        }

                        if (0 == strncmp(g_Photos[u32DirIdx].List[i].szFileName, szFileName, HI_APPCOMM_MAX_PATH_LEN)) {
                            HI_U32 u32FileAmount;
                            HI_DTCF_DelFileByIndex(j, &u32FileAmount);
                            g_Photos[u32DirIdx].u32Count--;
                            s_u32FILEMNGPhotoObjCnt--;
                            s_u32FILEMNGAllObjCnt--;
                            break;
                        }
                    }
                }

                MLOGI("rename %s to %s\n", g_Photos[u32DirIdx].List[i].szFileName, pstFileName->szFileName[0]);
                memcpy(g_Photos[u32DirIdx].List[i].szFileName, pstFileName->szFileName[0], HI_APPCOMM_MAX_PATH_LEN);
            } else {
                g_Photos[u32DirIdx].List[i].u32Index--;
            }
        }
    } else {
        FILEMNG_SPACEMONITOR_CheckSpace();
    }
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_DTCF_GetRatioSpace(HI_U32 *pu32MovieSpace, HI_U32 *pu32EmrSpace)
{
    FILEMNG_GetMovieSpace(pu32MovieSpace);
    FILEMNG_GetEmrSpace(pu32EmrSpace);
    return HI_SUCCESS;
}

static HI_S32 FILEMNG_DTCF_Cover(HI_S32 s32FullFlag)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HI_U32 u32RemoveSize_MB = 0;
    HI_U32 u32FileAmount = 0;
    HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN];
    struct stat FileStat;
    HI_U32 u32FileObjCnt = 0;
    HI_S32 s32TimeOut_ms = 2000;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    while (s32TimeOut_ms > 0) {
        if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            MLOGI("wait disk scan completed...\n");
            HI_usleep(500 * 1000);
            s32TimeOut_ms -= 500;
            HI_MUTEX_LOCK(s_FILEMNGMutex);
        } else {
            break;
        }
    }

    if ((0 == s_s32WritingRecCnt) && (SPACEMONITOR_MASK_TOTALFULL != (SPACEMONITOR_MASK_TOTALFULL & s32FullFlag))) {
        MLOGW("record is stopped,need not to cover old files.\n");
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return -1;
    }

    s32Ret |= FILEMNG_GetEmrCnt(&s_u32FILEMNGEmrRecObjCnt);
    s32Ret |= FILEMNG_GetPhotoCnt(&s_u32FILEMNGPhotoObjCnt);
    s32Ret |= FILEMNG_GetTotalCnt(&s_u32FILEMNGAllObjCnt);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(FILEMNG_GetTotalCnt, s32Ret);
        goto end;
    }

    s_u32FILEMNGRecObjCnt = s_u32FILEMNGAllObjCnt - s_u32FILEMNGPhotoObjCnt;
    FILEMNG_PrintInfo();

    /**<0x000:space enough;0x001:total space full;0x010:movie space full;0x100:emr movie space full */
    if (SPACEMONITOR_MASK_ENOUGH == s32FullFlag) {
        HI_LOG_PrintH32(s32FullFlag);
        goto end;
    }

    if (0 == s_u32FILEMNGAllObjCnt) {
        HI_LOG_PrintS32(s_u32FILEMNGAllObjCnt);
        HI_EVENT_S stEvent;
        stEvent.EventID = HI_EVENT_FILEMNG_UNIDENTIFICATION;
        HI_EVTHUB_Publish(&stEvent);
        goto end;
    }

    if (SPACEMONITOR_MASK_EMRFULL == (SPACEMONITOR_MASK_EMRFULL & s32FullFlag)) {
        /*remove emr*/
        do {
            HI_DTCF_DIR_E aenDirs[] = {
                DTCF_DIR_EMR_FRONT,
                DTCF_DIR_EMR_REAR,
            };

            s32Ret = HI_DTCF_GetOldestFileIndex(aenDirs, ARRAY_SIZE(aenDirs), &u32Index);
            if (HI_SUCCESS == s32Ret) {
                HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
                HI_DTCF_GetFileByIndex(u32Index, szAbsPath, HI_APPCOMM_MAX_PATH_LEN, &enDir);
                stat(szAbsPath, &FileStat);
                HI_DTCF_DelFileByIndex(u32Index, &u32FileAmount);
                FILEMNG_RemoveRecord(szAbsPath, enDir);
                u32RemoveSize_MB += FileStat.st_size >> 20;
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetOldestFileIndex, s32Ret);
                break;
            }
        } while (u32RemoveSize_MB <= (s_stFILEMNGDTCF_Cfg.u32GuaranteedStage - s_stFILEMNGDTCF_Cfg.u32WarningStage));
    }

    if (SPACEMONITOR_MASK_TOTALFULL == (SPACEMONITOR_MASK_TOTALFULL & s32FullFlag)
            || SPACEMONITOR_MASK_MOVIEFULL == (SPACEMONITOR_MASK_MOVIEFULL & s32FullFlag)) {
        /*remove movies*/
        u32RemoveSize_MB = 0;
        do {
            HI_DTCF_DIR_E aenDirs[] = {
                DTCF_DIR_NORM_FRONT,
                DTCF_DIR_NORM_REAR,
                DTCF_DIR_PARK_FRONT,
                DTCF_DIR_PARK_REAR,
            };
            s32Ret = HI_DTCF_GetOldestFileIndex(aenDirs, ARRAY_SIZE(aenDirs), &u32Index);
            if (HI_SUCCESS == s32Ret) {
                HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
                HI_DTCF_GetFileByIndex(u32Index, szAbsPath, HI_APPCOMM_MAX_PATH_LEN, &enDir);
                stat(szAbsPath, &FileStat);
                HI_DTCF_DelFileByIndex(u32Index, &u32FileAmount);
                FILEMNG_RemoveRecord(szAbsPath, enDir);
                u32RemoveSize_MB += FileStat.st_size >> 20;
            } else {
                HI_LOG_PrintFuncErr(HI_DTCF_GetOldestFileIndex, s32Ret);
                break;
            }
        } while (u32RemoveSize_MB <= (s_stFILEMNGDTCF_Cfg.u32GuaranteedStage - s_stFILEMNGDTCF_Cfg.u32WarningStage));
    }

    FILEMNG_PrintInfo();
end:
    if (s_u32DirCount < s_u32MaxDirCount) {
        s32Ret = HI_DTCF_Scan(s_aenDirs, s_u32DirCount, &u32FileObjCnt);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_DTCF_Scan, s32Ret);
        }
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**-------------------------external function interface-------------------------*/
/**
 * @brief    register filemng event.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/29
 */
HI_S32 HI_FILEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = 0;
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SCAN_COMPLETED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SCAN_FAIL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SPACE_FULL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SPACE_ENOUGH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_BEGIN);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_END);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_FAILED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_UNIDENTIFICATION);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FILEMNG_EINTER);
    return HI_SUCCESS;
}

/**
 * @brief    init file manager.
 * @param[in] pstCfg:file manager comm configuration.
 * @param[in] pstDCF_Cfg:file manager DCF configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Init(const HI_FILEMNG_COMM_CFG_S *pstCfg, const HI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg)
{
    HI_U32 u32DirIdx = 0;
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDTCF_Cfg, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_EXPR(pstDTCF_Cfg->u8SharePercent < 100, HI_FILEMNG_EINVAL);

    for (u32DirIdx = 0; u32DirIdx < HI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < strnlen(pstDTCF_Cfg->aszDirNames[u32DirIdx + DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
            HI_APPCOMM_CHECK_EXPR(FILEMNG_PREALLOC_FILE_MAX_NUM >= pstDTCF_Cfg->u32PreAllocCnt[u32DirIdx], HI_FILEMNG_EINVAL);
            if (0 < pstDTCF_Cfg->u32PreAllocCnt[u32DirIdx]) {
                HI_APPCOMM_CHECK_EXPR(0 < pstDTCF_Cfg->u32PreAllocUnit[u32DirIdx], HI_FILEMNG_EINVAL);
            }
        }
    }

    if (0 != pstDTCF_Cfg->u32GuaranteedStage) {
        HI_APPCOMM_CHECK_EXPR(pstDTCF_Cfg->u32GuaranteedStage >= pstDTCF_Cfg->u32WarningStage, HI_FILEMNG_EINVAL);
    }

    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_TRUE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGD("%s\n", FILEMNG_Strerror(HI_FILEMNG_EINITIALIZED));
        return HI_FILEMNG_EINITIALIZED;
    }

    if (HI_TRUE == pstCfg->stRepairCfg.bEnable) {
        s32Ret = FILEMNG_REPAIRER_Init(&pstCfg->stRepairCfg);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
            return s32Ret;
        }
    }

    SPACEMONITOR_CFG_S stConfig;
    snprintf(stConfig.szMntPath, HI_APPCOMM_MAX_PATH_LEN, pstCfg->szMntPath);
    stConfig.u32WarningStage = pstDTCF_Cfg->u32WarningStage;
    stConfig.u32GuaranteedStage = pstDTCF_Cfg->u32GuaranteedStage;
    stConfig.u8SharePercent = pstDTCF_Cfg->u8SharePercent;
    stConfig.u32Interval = 0;
    stConfig.u32MaxCheckDelay = 10;
    stConfig.pfnCoverCB = FILEMNG_DTCF_Cover;
    stConfig.pfnGetRatioSpace = FILEMNG_DTCF_GetRatioSpace;
    s32Ret = FILEMNG_SPACEMONITOR_Create(&stConfig);
    if (HI_SUCCESS != s32Ret) {
        FILEMNG_REPAIRER_Deinit();
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(FILEMNG_SPACEMONITOR_Create, s32Ret);
        return s32Ret;
    }

    memcpy(&s_stFILEMNGCfg, pstCfg, sizeof(HI_FILEMNG_COMM_CFG_S));
    memcpy(&s_stFILEMNGDTCF_Cfg, pstDTCF_Cfg, sizeof(HI_FILEMNG_DTCF_CFG_S));
    s_u32FILEMNGRecObjCnt = 0;
    s_u32FILEMNGPhotoObjCnt = 0;
    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;

    for (u32DirIdx = 0; u32DirIdx < HI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] &&
                0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[u32DirIdx + DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
            g_Photos[u32DirIdx].List = (FILEMNG_PHOTO_QUEUE_S *)malloc(s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] * sizeof(
                                       FILEMNG_PHOTO_QUEUE_S));
            if (NULL == g_Photos[u32DirIdx].List) {
                FILEMNG_SPACEMONITOR_Destroy();
                FILEMNG_REPAIRER_Deinit();
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EINTER));
                return HI_FILEMNG_EINTER;
            }

            memset(g_Photos[u32DirIdx].List, 0, (s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx] * sizeof(FILEMNG_PHOTO_QUEUE_S)));
            g_Photos[u32DirIdx].u32Count = 0;
        }
    }

    s_bFILEMNGInit = HI_TRUE;
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    deinit file manager.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState) {
        FILEMNG_Disable();
    }

    s32Ret = FILEMNG_SPACEMONITOR_Destroy();
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(FILEMNG_SPACEMONITOR_Destroy, s32Ret);
    }

    if (HI_TRUE == s_stFILEMNGCfg.stRepairCfg.bEnable) {
        s32Ret = FILEMNG_REPAIRER_Deinit();
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(FILEMNG_REPAIRER_Deinit, s32Ret);
        }
    }

    HI_U32 u32DirIdx = 0;
    for (u32DirIdx = 0; u32DirIdx < HI_FILEMNG_DTCF_MAX_PHOTO_DIR; u32DirIdx++) {
        if (0 < s_stFILEMNGDTCF_Cfg.u32PreAllocCnt[u32DirIdx]) {
            HI_APPCOMM_SAFE_FREE(g_Photos[u32DirIdx].List);
        }
    }

    s_bFILEMNGInit = HI_FALSE;
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    call this function to set the disk state.
 * @param[in] bAvailable:disk state.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SetDiskState(HI_BOOL bAvailable)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if ((HI_TRUE == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState)
            || (HI_FALSE == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState)) {
        /**disk available status not change,do nothing. */
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    } else {
        if (bAvailable) {
            s32Ret = FILEMNG_CheckPath(s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir);
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                HI_LOG_PrintFuncErr(FILEMNG_CheckPath, s32Ret);
                return s32Ret;
            }

            HI_CHAR szRootPath[HI_APPCOMM_MAX_PATH_LEN];
            snprintf(szRootPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", s_stFILEMNGCfg.szMntPath, s_stFILEMNGDTCF_Cfg.szRootDir);
            s32Ret = HI_DTCF_Init(szRootPath, (const HI_CHAR(*)[HI_DIR_LEN_MAX])(s_stFILEMNGDTCF_Cfg.aszDirNames));
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                HI_LOG_PrintFuncErr(HI_DTCF_Init, s32Ret);
                return HI_FILEMNG_EINTER;
            }

            s32Ret |= FILEMNG_GetEmrCnt(&s_u32FILEMNGEmrRecObjCnt);
            s32Ret |= FILEMNG_GetPhotoCnt(&s_u32FILEMNGPhotoObjCnt);
            s32Ret |= FILEMNG_GetTotalCnt(&s_u32FILEMNGAllObjCnt);
            HI_EVENT_S stEvent;
            s_enFILEMNGDiskState = FILEMNG_DISK_STATE_SCAN_COMPLETED;
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                stEvent.EventID = HI_EVENT_FILEMNG_SCAN_FAIL;
                HI_EVTHUB_Publish(&stEvent);
                MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
                return HI_FILEMNG_EINTER;
            }

            /**scan scope */
            s_u32DirCount = 0;
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_FRONT], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_EMR_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_FRONT], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_NORM_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_FRONT], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PARK_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_EMR_REAR], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_EMR_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_NORM_REAR], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_NORM_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PARK_REAR], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PARK_REAR;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_FRONT], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PHOTO_FRONT;
            }
            if (0 < strnlen(s_stFILEMNGDTCF_Cfg.aszDirNames[DTCF_DIR_PHOTO_REAR], HI_DIR_LEN_MAX)) {
                s_aenDirs[s_u32DirCount++] = DTCF_DIR_PHOTO_REAR;
            }

            s_u32MaxDirCount = s_u32DirCount;
            s_u32FILEMNGRecObjCnt = s_u32FILEMNGAllObjCnt - s_u32FILEMNGPhotoObjCnt;
            FILEMNG_PrintInfo();
            stEvent.EventID = HI_EVENT_FILEMNG_SCAN_COMPLETED;
            HI_EVTHUB_Publish(&stEvent);
            s32Ret = FILEMNG_MARKER_Init(s_stFILEMNGCfg.szMntPath);
            if (HI_SUCCESS != s32Ret) {
                HI_LOG_PrintFuncErr(HI_LOG_PrintFuncErr, s32Ret);
            }
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return HI_SUCCESS;
        } else {
            FILEMNG_Disable();
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return HI_SUCCESS;
        }
    }
}

HI_S32 HI_FILEMNG_CheckDiskSpace(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }
    HI_U64 u64RealUsedSize = 0;
    HI_U32 u32MovieSpace_MB = 0;
    HI_U32 u32EmrSpace_MB = 0;
    HI_U32 u32PhotoSpace_MB = 0;
    FILEMNG_DTCF_GetRatioSpace(&u32MovieSpace_MB, &u32EmrSpace_MB);
    FILEMNG_GetPhotoSpace(&u32PhotoSpace_MB);
    u64RealUsedSize = u32MovieSpace_MB + u32EmrSpace_MB + u32PhotoSpace_MB;
    s32Ret = FILEMNG_SPACEMONITOR_JudgeStage(u64RealUsedSize);
    if (HI_SUCCESS != s32Ret) {
        MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**
 * @brief    add a file to the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_AddFile(const HI_CHAR *pszFilePath)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32Ret = HI_PathIsDirectory(pszFilePath);
    if (0 == s32Ret) {
        HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
        s32Ret = HI_DTCF_GetFileDirType(pszFilePath, &enDir);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DTCF_GetFileDirType, s32Ret);
            return HI_FILEMNG_EINVAL;
        }

        if (DTCF_DIR_EMR_FRONT_SUB == enDir || DTCF_DIR_NORM_FRONT_SUB == enDir
                || DTCF_DIR_EMR_REAR_SUB == enDir || DTCF_DIR_NORM_REAR_SUB == enDir) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return HI_FILEMNG_ENOTMAIN;
        } else if (DTCF_DIR_PHOTO_FRONT == enDir || DTCF_DIR_PHOTO_REAR == enDir) {
            s32Ret = FILEMNG_AddPhoto(pszFilePath, enDir);
        } else {
            s32Ret = FILEMNG_AddRecord(pszFilePath, enDir);
        }

        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
            return s32Ret;
        }

        s_u32FILEMNGAllObjCnt = s_u32FILEMNGPhotoObjCnt + s_u32FILEMNGRecObjCnt;
        FILEMNG_PrintInfo();
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
    return HI_FILEMNG_ELOST;
}

/**
 * @brief    remove a file from the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_RemoveFile(const HI_CHAR *pszFilePath)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32IsDir = 0;
    HI_S32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32IsDir = HI_PathIsDirectory(pszFilePath);
    HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];
    HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
    HI_U32 u32FileAmount;
    for (i = 0; i < s_u32FILEMNGAllObjCnt; i++, enDir = DTCF_DIR_BUTT) {
        s32Ret = HI_DTCF_GetFileByIndex(i, szFileName, HI_APPCOMM_MAX_PATH_LEN, &enDir);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DTCF_GetFileByIndex, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        if (0 == strncmp(pszFilePath, szFileName, HI_APPCOMM_MAX_PATH_LEN)) {
            HI_DTCF_DelFileByIndex(i, &u32FileAmount);
            break;
        }
    }

    if (DTCF_DIR_BUTT == enDir) {
        s32Ret = HI_DTCF_GetFileDirType(pszFilePath, &enDir);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DTCF_GetFileDirType, s32Ret);
            return HI_FILEMNG_EINVAL;
        }
    }

    if (DTCF_DIR_EMR_FRONT_SUB == enDir || DTCF_DIR_NORM_FRONT_SUB == enDir
            || DTCF_DIR_EMR_REAR_SUB == enDir || DTCF_DIR_NORM_REAR_SUB == enDir) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_FILEMNG_ENOTMAIN;
    } else if (DTCF_DIR_PHOTO_FRONT == enDir || DTCF_DIR_PHOTO_REAR == enDir) {
        s32Ret = FILEMNG_RemovePhoto(pszFilePath, enDir);
    } else {
        s32Ret = FILEMNG_RemoveRecord(pszFilePath, enDir);
    }

    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
        return s32Ret;
    }

    FILEMNG_PrintInfo();
    if (0 == s32IsDir) {
        FILEMNG_MARKER_CleanFlag(pszFilePath);
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    } else {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
        return HI_FILEMNG_ELOST;
    }
}

/**
 * @brief    mark a file with a symbol.
 * @param[in] pszFilePath:the file absolute path.
 * @param[in] u8Flag:the flag marked to the file.range:[0,255],0 unmark,1-255 special flag.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_MarkFile(const HI_CHAR *pszFilePath, HI_U8 u8Flag)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32Ret = HI_PathIsDirectory(pszFilePath);
    if (0 != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
        return HI_FILEMNG_ELOST;
    }

    if (0 == u8Flag) {
        s32Ret = FILEMNG_MARKER_CleanFlag(pszFilePath);
    } else {
        s32Ret = FILEMNG_MARKER_SetFlag(pszFilePath, u8Flag);
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**
 * @brief    create new file name to DTCF.
 * @param[in] enType:file type
 * @param[in] enDir:dir type,range:[0,DTCF_DIR_BUTT)
 * @param[in] bPreAlloc:is pre alloc file or not
 * @param[out] pstFileName:new file object name.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GenerateFileName(HI_DTCF_FILE_TYPE_E enType, HI_DTCF_DIR_E enDir, HI_BOOL bPreAlloc,
                                   HI_FILEMNG_OBJ_FILENAME_S *pstFileName)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstFileName, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_EXPR(DTCF_DIR_BUTT > enDir, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (HI_DTCF_FILE_TYPE_MP4 == enType || HI_DTCF_FILE_TYPE_TS == enType) {
        s32Ret = FILEMNG_GenerateRecordName(enType, enDir, pstFileName);
    } else if (HI_DTCF_FILE_TYPE_JPG == enType) {
        if (DTCF_DIR_PHOTO_FRONT != enDir && DTCF_DIR_PHOTO_REAR != enDir) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            MLOGE("file type is jpg,but dir is not photo\n");
            return HI_FILEMNG_EINVAL;
        }

        s32Ret = FILEMNG_GeneratePhotoName(enType, enDir, bPreAlloc, pstFileName);
    } else {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s:enType\n", FILEMNG_Strerror(HI_FILEMNG_EINVAL));
        return HI_FILEMNG_EINVAL;
    }

    if (HI_SUCCESS != s32Ret) {
        MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**
 * @brief    repair the damaged files.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE).
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Repair(HI_VOID)
{
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    FILEMNG_REPAIRER_Repair();
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    set the search scope
 * @param[in] aenDirs[DTCF_DIR_BUTT]:directory collection to search
 * @param[in] u32DirCount:directory collection count
 * @param[out] pu32FileObjCnt:file object count in the selected directorys
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SetSearchScope(HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32FileObjCnt)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pu32FileObjCnt, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_EXPR(u32DirCount <= DTCF_DIR_BUTT, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EBUSY));
        return HI_FILEMNG_EBUSY;
    }

    s32Ret = HI_DTCF_Scan(aenDirs, u32DirCount, pu32FileObjCnt);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DTCF_Scan, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    for (s_u32DirCount = 0; s_u32DirCount < u32DirCount; s_u32DirCount++) {
        s_aenDirs[s_u32DirCount] = aenDirs[s_u32DirCount];
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    get the specified file object count.
 * @param[in] enType:specified type,HI_FILEMNG_FILE_TYPE_BUTT means all type.
 * @param[out] pu32FileObjCnt:the specified file object count.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_EVENT_FILEMNG_SCAN_COMPLETED published
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileObjCnt(HI_FILEMNG_FILE_TYPE_E enType, HI_U32 *pu32FileObjCnt)
{
    HI_APPCOMM_CHECK_POINTER(pu32FileObjCnt, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    *pu32FileObjCnt = 0;
    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EBUSY));
        return HI_FILEMNG_EBUSY;
    } else if (FILEMNG_DISK_STATE_SCAN_COMPLETED == s_enFILEMNGDiskState) {
        switch (enType) {
            case HI_FILEMNG_FILE_TYPE_RECORD:
                *pu32FileObjCnt = s_u32FILEMNGRecObjCnt;
                break;
            case HI_FILEMNG_FILE_TYPE_PHOTO:
                *pu32FileObjCnt = s_u32FILEMNGPhotoObjCnt;
                break;
            default:
                *pu32FileObjCnt = s_u32FILEMNGAllObjCnt;
                break;
        }
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}


/**
 * @brief    get the file information by file index.
 * @param[in] u32FileIdx:the file absolute index,range[0,FileObjCnt).
 * @param[out] pazFileName:the file name.
 * @param[in] u32Length:the file path length.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileByIndex(HI_U32 u32FileIdx, HI_CHAR *pazFileName, HI_U32 u32Length)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pazFileName, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (u32FileIdx >= s_u32FILEMNGAllObjCnt) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EINVAL));
        return HI_FILEMNG_EINVAL;
    }

    if (FILEMNG_DISK_STATE_SCAN_COMPLETED != s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EBUSY));
        return HI_FILEMNG_EBUSY;
    }

    HI_DTCF_DIR_E enDir;
    s32Ret = HI_DTCF_GetFileByIndex(u32FileIdx, pazFileName, u32Length, &enDir);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DTCF_GetFileByIndex, s32Ret);
        return HI_FILEMNG_EINVAL;
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**
 * @brief    get the file information by file name.
 * @param[in] pszFilePath:the file absolute path.
 * @param[out] pstFileInfo:the file information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileInfoByName(const HI_CHAR *pszFilePath, HI_FILEMNG_FILE_INFO_S *pstFileInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstFileInfo, HI_FILEMNG_EINVAL);
    return FILEMNG_GetFileInfo(pszFilePath, pstFileInfo);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

