/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_datatime.c
* @brief     datatime module source file
* @author    HiMobileCam middleware develop team
* @date      2017.02.13
*/
#include "securec.h"
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "hi_dtcf.h"
#include "hi_dtcf_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef MW_VERSION
#define MW_VERSION "0.0.0"
#endif
static volatile const HI_CHAR DTCF_VERSIONINFO[] = "HIBERRY DTCF MW Version: "MW_VERSION;
static volatile const HI_CHAR DTCF_BUILD_DATE[] = "HIBERRY DTCF Build Date:"__DATE__" Time:"__TIME__;

#define DTCF_USR_PROC_ENTRY_NAME "dtcf"

// default directory name
#define HI_EMR_DIR          "EMR"
#define HI_EMR_S_DIR        "EMR_s"
#define HI_MOVIES_DIR       "Movie"
#define HI_MOVIES_S_DIR     "Movie_s"
#define HI_PARK_DIR         "Park"
#define HI_PARK_S_DIR       "Park_s"
#define HI_EMR_B_DIR        "EMR_b"
#define HI_EMR_B_S_DIR      "EMR_b_s"
#define HI_MOVIES_B_DIR     "Movie_b"
#define HI_MOVIES_B_S_DIR   "Movie_b_s"
#define HI_PARK_B_DIR       "Park_b"
#define HI_PARK_B_S_DIR     "Park_b_s"
#define HI_PHOTO_DIR        "Photo"
#define HI_PHOTO_B_DIR      "Photo_b"

#define HI_DTCF_DIR_MODE    (0760)

#define HI_FILE_SUFFIX_LEN_MAX       (8)

#define HI_DTCF_TIME_STR_LEN         (20)
#define HI_FILE_NAME_LEN_MIN         (23) //e.g. YYYY_MM_DD_HHMMSS_XX.TS


#define HI_DTCF_SCAN_FILE_AMOUNT_MAX (10 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2) //10240

#define CHECK_DTCF_NULL_PTR(ptr)\
    do{\
        if(HI_NULL == ptr)\
        {\
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "DTCF NULL pointer\n" );\
            return HI_DTCF_ERR_NULL_PTR;\
        }\
    }while(0)

#define CHECK_DTCF_STATUS_CREATED(status)\
    do{\
        if(DTCF_STATUS_CREATED != status && DTCF_STATUS_SCANED != status )\
        {\
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "status is not DTCF_STATUS_CREATED|DTCF_STATUS_SCANED, current status:%d\n", s_enDtcfStatu);\
            return HI_DTCF_ERR_STATUS_ERROR;\
        }\
    }while(0)

#define CHECK_DTCF_STATUS_SCANED(status)\
    do{\
        if(DTCF_STATUS_SCANED != status)\
        {\
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "status is not DTCF_STATUS_SCANED, current status:%d\n", s_enDtcfStatu);\
            return HI_DTCF_ERR_STATUS_ERROR;\
        }\
    }while(0)

#define CHECK_DTCF_ENDIR_RANGE(enDir)\
    do{\
        if(enDir >= DTCF_DIR_BUTT)\
        {\
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "DTCF enDir:%d out of Range\n", enDir);\
            return HI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define CHECK_DTCF_FILETYPE_RANGE(fileType)\
    do{\
        if(fileType >= HI_DTCF_FILE_TYPE_BUTT)\
        {\
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "DTCF fileType out of Range\n" );\
            return HI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define CHECK_DTCF_FILE_PATH_LENGTH(length)\
    do{\
        if(length == 0)\
        {\
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "DTCF file path length is 0\n" );\
            return HI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define DTCF_LOG(level,fmt...)\
    do{\
        HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, level, "[%s][%d]:",__FUNCTION__, __LINE__);\
        HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, level, ##fmt);\
    }while(0)

#define DTCF_THREAD_MUTEX_LOCK(pLock) \
    do {\
        HI_S32 s32LockRet = pthread_mutex_lock(&pLock);\
        if (0 != s32LockRet)         \
        {                                               \
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "[%s][%d]DTCF mutex lock failed %d!\n", __FUNCTION__, __LINE__, s32LockRet);  \
        }                                               \
    } while (0)

#define DTCF_THREAD_MUTEX_UNLOCK(pLock) \
    do {\
        HI_S32 s32unLockRet = pthread_mutex_unlock(&pLock);\
        if (0 != s32unLockRet)         \
        {                                               \
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "[%s][%d]DTCF mutex unlock failed ret %d!\n", __FUNCTION__, __LINE__, s32unLockRet);  \
        }                                               \
    } while (0)

#define FREE_SCANDIR_LIST(list, cnt) \
    do {\
        HI_S32 i = 0;\
        while (i < cnt)\
        {  \
            free(list[i++]);\
        }   \
        free(list);\
    } while(0)

#define CHECK_DTCF_PATH_IS_DIR(path) \
    do {\
        struct stat s_buf = {0};\
        if (HI_SUCCESS != stat(path, &s_buf)) { \
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "[%s][%d] stat sys errno:%d  %s\n", __FUNCTION__, __LINE__, errno, strerror(errno));\
            return HI_DTCF_ERR_SYSTEM_ERROR;\
        }\
        if (!S_ISDIR(s_buf.st_mode)){\
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "[%s][%d] %s is not a directory\n", __FUNCTION__, __LINE__, path);\
            return HI_DTCF_PATH_IS_NOT_DIR_ERROR;\
        }\
    } while(0)

#define IS_DIGIT_CHAR(c, min, max)\
    do{\
        if(max < c || c < min)\
        {\
            return HI_FALSE;\
        }\
    }while(0)

#define IS_SAME_CHAR(c, tag)\
    do{\
        if(tag != c)\
        {\
            return HI_FALSE;\
        }\
    }while(0)

typedef enum hiDTCF_STATUS_E
{
    DTCF_STATUS_CREATED = 0,
    DTCF_STATUS_SCANED,
    DTCF_STATUS_DESTROY,
    DTCF_STATUS_BUTT
} HI_DTCF_STATUS_E;


typedef struct hiDTCF_TEMP_DIR_S
{
    HI_CHAR azTmpDir[HI_FILE_PATH_LEN_MAX];
    HI_U32 u32DirNameLen;
    HI_DTCF_DIR_E enDir;
    HI_DTCF_FILE_TYPE_E enType;
    HI_CHAR azTimeStr[HI_DTCF_TIME_STR_LEN + 1];
} HI_DTCF_TEMP_DIR_S;

typedef struct hiDTCF_DIRENT_S
{
    struct dirent *pdirent;
    HI_DTCF_DIR_E enDir;
} HI_DTCF_DIRENT_S;

typedef struct hiDTCF_SCAN_FILE_LIST_S
{
    HI_DTCF_DIRENT_S *pstDirentList;
    HI_U32 u32DirentlistCount;
    HI_U32 u32listCount;
    HI_DTCF_DIR_E enScanDirs[DTCF_DIR_BUTT];
    HI_U32 u32ScanDirCount;
} HI_DTCF_SCAN_FILE_LIST_S;

static HI_DTCF_SCAN_FILE_LIST_S s_stDtcfScanFileList = {HI_NULL, 0, 0, {0}, 0};

static HI_CHAR s_azRootDir[HI_FILE_PATH_LEN_MAX] = {0};
static HI_DTCF_STATUS_E s_enDtcfStatu = DTCF_STATUS_BUTT;
static pthread_mutex_t s_pthDtcfInterface_lock = PTHREAD_MUTEX_INITIALIZER;

static HI_CHAR g_azDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX] =
        {
            HI_EMR_DIR,
            HI_EMR_S_DIR,
            HI_MOVIES_DIR,
            HI_MOVIES_S_DIR,
            HI_PARK_DIR,
            HI_PARK_S_DIR,
            HI_EMR_B_DIR,
            HI_EMR_B_S_DIR,
            HI_MOVIES_B_DIR,
            HI_MOVIES_B_S_DIR,
            HI_PARK_B_DIR,
            HI_PARK_B_S_DIR,
            HI_PHOTO_DIR,
            HI_PHOTO_B_DIR
        };

static char g_szFileTypeSuffix[HI_DTCF_FILE_TYPE_BUTT][HI_FILE_SUFFIX_LEN_MAX] =
        {
            "MP4",
            "JPG",
            "TS"
        };

static char g_szFileNameSuffix[DTCF_DIR_BUTT][HI_FILE_SUFFIX_LEN_MAX] =
        {
            "",
            "_s",
            "",
            "_s",
            "",
            "_s",
            "_b",
            "_b_s",
            "_b",
            "_b_s",
            "_b",
            "_b_s",
            "",
            "_b"
        };

static HI_VOID dtcf_FreeScanNameList()
{
    HI_S32 s32Ret = EOK;
    while (s_stDtcfScanFileList.u32DirentlistCount--)
    {
        if(HI_NULL != s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent)
        {
            free(s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent);
            s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent = HI_NULL;
        }
    }
    free(s_stDtcfScanFileList.pstDirentList);
    s_stDtcfScanFileList.pstDirentList = HI_NULL;
    s_stDtcfScanFileList.u32DirentlistCount = 0;
    s_stDtcfScanFileList.u32ScanDirCount = 0;
    s_stDtcfScanFileList.u32listCount = 0;
    s32Ret = memset_s(&s_stDtcfScanFileList.enScanDirs, sizeof(HI_DTCF_DIR_E)*DTCF_DIR_BUTT, DTCF_DIR_BUTT,sizeof(HI_DTCF_DIR_E) * DTCF_DIR_BUTT);
    if(EOK!=s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "memset_s err: 0x%x\n", s32Ret);
    }
}

static HI_BOOL DTCF_FileNameRuleCheckFileTypeSuffix(const HI_CHAR *pFileTypeSuffix, const HI_U32 u32Len)
{
    HI_U32 suffexLen,i;
    for (i = 0; i < HI_DTCF_FILE_TYPE_BUTT; i++)
    {
        suffexLen = strlen(g_szFileTypeSuffix[i]);
        if (0 == strncmp(g_szFileTypeSuffix[i], pFileTypeSuffix, suffexLen) && u32Len == suffexLen)
        {
                return HI_TRUE;
        }
    }
    return HI_FALSE;
}

static HI_BOOL DTCF_FileNameRuleCheckFileNameSuffix(const HI_CHAR *pFileNameSuffix, const HI_U32 u32Len)
{
    if('s' == pFileNameSuffix[0] && '.' == pFileNameSuffix[1])
    {
        return DTCF_FileNameRuleCheckFileTypeSuffix(&pFileNameSuffix[2], u32Len - 2);
    }
    else if('b' == pFileNameSuffix[0])
    {
        if(pFileNameSuffix[1] == '.')
        {
            return DTCF_FileNameRuleCheckFileTypeSuffix(&pFileNameSuffix[2],u32Len - 2);
        }
        else if(pFileNameSuffix[1] == '_' && pFileNameSuffix[2] == 's' && pFileNameSuffix[3] == '.')
        {
            return DTCF_FileNameRuleCheckFileTypeSuffix(&pFileNameSuffix[4],u32Len - 4);
        }
    }
    return HI_FALSE;
}

/**
 * 1970_01_01_182605.MP4
 * 1970_01_01_182605_s.MP4
 * 1970_01_01_182605_b.MP4
 * 1970_01_01_182605_b_s.MP4
 */
static HI_BOOL DTCF_FileNameRuleCheck(const HI_CHAR *pazPath, const HI_U32 u32Len)
{
    if (u32Len < HI_FILE_NAME_LEN_MIN)
        return HI_FALSE;
    IS_DIGIT_CHAR(pazPath[0], '1', '9');
    IS_DIGIT_CHAR(pazPath[1], '0', '9');
    IS_DIGIT_CHAR(pazPath[2], '0', '9');
    IS_DIGIT_CHAR(pazPath[3], '0', '9');
    IS_SAME_CHAR(pazPath[4], '_');
    IS_DIGIT_CHAR(pazPath[5], '0', '9');
    IS_DIGIT_CHAR(pazPath[6], '0', '9');
    IS_SAME_CHAR(pazPath[7], '_');
    IS_DIGIT_CHAR(pazPath[8], '0', '9');
    IS_DIGIT_CHAR(pazPath[9], '0', '9');
    IS_SAME_CHAR(pazPath[10], '_');
    IS_DIGIT_CHAR(pazPath[11], '0', '9');
    IS_DIGIT_CHAR(pazPath[12], '0', '9');
    IS_DIGIT_CHAR(pazPath[13], '0', '9');
    IS_DIGIT_CHAR(pazPath[14], '0', '9');
    IS_DIGIT_CHAR(pazPath[15], '0', '9');
    IS_DIGIT_CHAR(pazPath[16], '0', '9');
    IS_SAME_CHAR(pazPath[17], '_');
    IS_DIGIT_CHAR(pazPath[18], '0', '9');
    IS_DIGIT_CHAR(pazPath[19], '0', '9');//check string such as 1970_01_01_182605_00

    if(pazPath[20] == '.') //check string such as 1970_01_01_182605_00.MP4
    {
        return DTCF_FileNameRuleCheckFileTypeSuffix(&pazPath[21],u32Len - 21);
    }
    else if(pazPath[20] == '_') //check string such as 1970_01_01_182605_00_*.MP4
    {
        return DTCF_FileNameRuleCheckFileNameSuffix(&pazPath[21],u32Len - 21);
    }
    else
    {
        return HI_FALSE;
    }

}

static HI_S32 DTCF_CheckRootDir(const HI_CHAR* pszRootDir)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_U32 u32Len = 0;
    HI_CHAR aszPath[HI_FILE_PATH_LEN_MAX] = {0};

    u32Len = strlen(pszRootDir);
    if((u32Len >= HI_FILE_PATH_LEN_MAX) || (u32Len == 0)) {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "RootDir:%s too long\n", pszRootDir);
        return HI_DTCF_UNSUPPORT_PATH;
    }

    s32Ret = snprintf_s(aszPath, HI_FILE_PATH_LEN_MAX,
        HI_FILE_PATH_LEN_MAX - 1, "%s", pszRootDir);
    if (s32Ret < 0) {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "RootDir:%s is invalid path\n", pszRootDir);
        return HI_DTCF_UNSUPPORT_PATH;
    }

    for (i = 0; i < u32Len; i++) {
        if (i == 0 && aszPath[i] != '/') {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "RootDir:%s is invalid path\n", pszRootDir);
            return HI_DTCF_UNSUPPORT_PATH;
        }

        if (aszPath[i] == '/') {
            continue;
        }

        if (!((aszPath[i] >= 'a' && aszPath[i] <= 'z')||
            (aszPath[i] >= 'A' && aszPath[i] <= 'Z')||
            (aszPath[i] >= '0' && aszPath[i] <= '9')||
            (aszPath[i] == '_') || (aszPath[i] == '.'))) {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "RootDir:%s is invalid path\n", pszRootDir);
            return HI_DTCF_UNSUPPORT_PATH;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 selector(const struct dirent *dir)
{
    CHECK_DTCF_NULL_PTR(dir);
    HI_U32 len = strlen(dir->d_name);
    if(HI_TRUE == DTCF_FileNameRuleCheck(dir->d_name, len))
        return 1;

    DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO, "dir->d_name:%s\n", dir->d_name);
    return 0;
}

static HI_S32 sortBydname ( const void *a , const void *b )
{
    HI_DTCF_DIRENT_S *aa = (HI_DTCF_DIRENT_S *)a;
    HI_DTCF_DIRENT_S *bb = (HI_DTCF_DIRENT_S *)b;
    return alphasort((void *)&bb->pdirent, (void *)&aa->pdirent);
}

static HI_S32 DTCF_checkFilePath(const HI_CHAR *pazSrcFilePath, HI_DTCF_TEMP_DIR_S *pstSrcTmpDir)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 index = 0;
    HI_U32 u32FilePathLen = 0;
    HI_CHAR aszFileName[HI_FILE_PATH_LEN_MAX] = {0};
    HI_DTCF_DIR_E i;
    HI_DTCF_FILE_TYPE_E j;

    u32FilePathLen = strlen(pazSrcFilePath);
    if (u32FilePathLen >= HI_FILE_PATH_LEN_MAX)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "pazSrcFilePath:%s too long\n", pazSrcFilePath);
        return HI_DTCF_UNSUPPORT_PATH;
    }

    if(strncmp(s_azRootDir, pazSrcFilePath, strlen(s_azRootDir)) != 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "s_azRootDir:%s pazSrcFilePath:%s\n", s_azRootDir, pazSrcFilePath);
        return HI_DTCF_UNSUPPORT_PATH;
    }
    index += strlen(s_azRootDir);
    pstSrcTmpDir->u32DirNameLen = u32FilePathLen;
    s32Ret = strncpy_s(pstSrcTmpDir->azTmpDir, sizeof(pstSrcTmpDir->azTmpDir), pazSrcFilePath, pstSrcTmpDir->u32DirNameLen);
    if(s32Ret != EOK)
    {
        return HI_DTCF_UNSUPPORT_PATH;
    }

    index += 1;
    while(index < HI_FILE_PATH_LEN_MAX && '/' == pazSrcFilePath[index])
    {
        index++;
    }
    HI_CHAR *cNext = strstr(pazSrcFilePath + index, "/");
    if(HI_NULL == cNext)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "pazSrcFilePath:%s index:%d, not find next '/'\n", pazSrcFilePath, index);
        return HI_DTCF_UNSUPPORT_PATH;
    }
    for(i = DTCF_DIR_EMR_FRONT; i < DTCF_DIR_BUTT; i ++)
    {
        if(0 == strncmp(g_azDirNames[i], pazSrcFilePath + index, (cNext - pazSrcFilePath - index)))
        {
            pstSrcTmpDir->enDir = (HI_DTCF_DIR_E)i;
            index += strlen(g_azDirNames[i]);
            break;
        }
    }
    if(DTCF_DIR_BUTT == i)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "pazSrcFilePath:%s index:%d, not support\n", pazSrcFilePath, index);
        return HI_DTCF_UNSUPPORT_PATH;
    }
    index += 1;
    while(index < HI_FILE_PATH_LEN_MAX && '/' == pazSrcFilePath[index])
    {
        index++;
    }

    s32Ret = strncpy_s(aszFileName, HI_FILE_PATH_LEN_MAX, pazSrcFilePath + index, HI_FILE_PATH_LEN_MAX - 1);
    if (s32Ret != EOK) {
        return HI_DTCF_UNSUPPORT_PATH;
    }

    if (!DTCF_FileNameRuleCheck(aszFileName, strlen(aszFileName))) {
        return HI_DTCF_UNSUPPORT_PATH;
    }

    s32Ret = strncpy_s(pstSrcTmpDir->azTimeStr, HI_DTCF_TIME_STR_LEN+1, pazSrcFilePath + index, HI_DTCF_TIME_STR_LEN);
    if(s32Ret != EOK)
    {
        return HI_DTCF_UNSUPPORT_PATH;
    }

    HI_CHAR *s = strstr(pazSrcFilePath + index, ".");
    if(HI_NULL == s)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "pazSrcFilePath:%s index:%d, not find '.'\n", pazSrcFilePath, index);
        return HI_DTCF_UNSUPPORT_PATH;
    }

    for(j = HI_DTCF_FILE_TYPE_MP4; j < HI_DTCF_FILE_TYPE_BUTT; j ++)
    {
        if(0 == strncmp(g_szFileTypeSuffix[j], s + 1, strlen(g_szFileTypeSuffix[j])))
        {
            pstSrcTmpDir->enType = (HI_DTCF_FILE_TYPE_E)j;
            break;
        }
    }

    if(HI_DTCF_FILE_TYPE_BUTT == j)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "pazSrcFilePath:%s :%s, not support\n", pazSrcFilePath, s);
        return HI_DTCF_UNSUPPORT_PATH;
    }
    return HI_SUCCESS;
}

static HI_S32 DTCF_mkdir(const char *dir, mode_t mode)
{
    MW_UNUSED(mode);

    HI_S32 s32Ret = mkdir(dir, HI_DTCF_DIR_MODE);
    if(s32Ret < 0)
    {
        if(errno == EEXIST)
        {
            CHECK_DTCF_PATH_IS_DIR(dir);
        }
        else
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "mkdir %s fail s32Ret:%d\n",dir, s32Ret);
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "[%d]:%s \n", errno, strerror(errno));
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }
    }
    return HI_SUCCESS;
}

static HI_S32 DTCF_mkdirs(const char *muldir, mode_t mode)
{
    HI_S32 i=0;
    HI_S32 len=0;
    HI_S32 s32Ret = HI_SUCCESS;
    char str[HI_FILE_PATH_LEN_MAX + 1] = { 0 };

    s32Ret = strncpy_s(str, HI_FILE_PATH_LEN_MAX + 1, muldir, HI_FILE_PATH_LEN_MAX);
    if(EOK!=s32Ret)
    {
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (0 == i && str[i] == '/')
        {
            i++;
        }
        if (str[i] == '/')
        {
            str[i] = '\0';
            s32Ret = mkdir(str, mode);
            if (s32Ret < 0)
            {
                if (errno == EEXIST)
                {
                    CHECK_DTCF_PATH_IS_DIR(str);
                }
                else
                {
                    DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "mkdir str:%s fail s32Ret:%d\n", str, s32Ret);
                    perror(str);
                    return s32Ret;
                }
            }

            str[i] = '/';
        }
    }

    if (len > 0)
    {
        s32Ret = mkdir(str, mode);
        if (s32Ret < 0)
        {
            if (errno == EEXIST)
            {
                CHECK_DTCF_PATH_IS_DIR(str);
            }
            else
            {
                DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "mkdir str:%s fail s32Ret:%d\n", str, s32Ret);
                perror(str);
                return s32Ret;
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 DTCF_ExpandFileList(HI_U32 u32size)
{
    HI_U32 u32ChangePoit = 10;
    if(s_stDtcfScanFileList.u32DirentlistCount + u32size > HI_DTCF_SCAN_FILE_AMOUNT_MAX)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"u32DirentlistCount:%d +  u32size:%u > HI_DTCF_SCAN_FILE_AMOUNT_MAX:%d\n", s_stDtcfScanFileList.u32DirentlistCount, u32size, HI_DTCF_SCAN_FILE_AMOUNT_MAX);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(s_stDtcfScanFileList.u32DirentlistCount + u32size >= s_stDtcfScanFileList.u32listCount)
    {
        while(u32ChangePoit < HI_DTCF_SCAN_FILE_AMOUNT_MAX)
        {
            if(u32ChangePoit < s_stDtcfScanFileList.u32DirentlistCount + u32size)
            {
                u32ChangePoit = u32ChangePoit * 2;
            }
            else
            {
                break;
            }
        }
        HI_DTCF_DIRENT_S *newDirEnt = NULL;
        newDirEnt = (HI_DTCF_DIRENT_S *) malloc ((u32ChangePoit) * sizeof (HI_DTCF_DIRENT_S));
        if(HI_NULL == newDirEnt)
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "malloc mem error, errno:%d\n", errno);
            return HI_DTCF_ERR_SYSTEM_ERROR;
        }
        if(EOK!=memset_s(newDirEnt, (u32ChangePoit)*sizeof(HI_DTCF_DIRENT_S), 0x0, (u32ChangePoit) * sizeof (HI_DTCF_DIRENT_S)))
        {
            free(newDirEnt);
            newDirEnt = NULL;
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }
        if(HI_NULL != s_stDtcfScanFileList.pstDirentList)
        {
            if( EOK != memcpy_s( newDirEnt, (u32ChangePoit)*sizeof(HI_DTCF_DIRENT_S), s_stDtcfScanFileList.pstDirentList,
                            (s_stDtcfScanFileList.u32DirentlistCount*sizeof(HI_DTCF_DIRENT_S)) ) )
            {
                free(newDirEnt);
                newDirEnt = NULL;
                return HI_DTCF_ERR_EINVAL_PAEAMETER;
            }
            free(s_stDtcfScanFileList.pstDirentList);
        }

        s_stDtcfScanFileList.pstDirentList = newDirEnt;
        s_stDtcfScanFileList.u32listCount = u32ChangePoit;
    }


    return HI_SUCCESS;
}

static HI_S32 DTCF_Addfile(const HI_CHAR *pazFileName, HI_DTCF_DIR_E enDir)
{
    HI_U32 i = 0;
    HI_U32 low, high, mid;
    HI_S32 s32Ret = DTCF_ExpandFileList(1);
    if (HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "DTCF_ExpandFileList 1 fail s32Ret:%d\n", s32Ret);
        return s32Ret;
    }

    struct dirent *p = (struct dirent *) malloc(sizeof(struct dirent));
    if (HI_NULL == p)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "malloc dirent fail\n");
        return HI_DTCF_ERR_NOMEM;
    }

    s32Ret = memset_s(p, sizeof(struct dirent), 0x0, sizeof(struct dirent));
    if(EOK!=s32Ret)
    {
        free(p);
        p = NULL;
        return HI_DTCF_ERR_SYSTEM_ERROR;
    }

    p->d_reclen = strlen(pazFileName);
    s32Ret = snprintf_s(p->d_name, sizeof(p->d_name), NAME_MAX, "%s", pazFileName);
    if (s32Ret >= NAME_MAX || s32Ret <= 0)
    {
        free(p);
        p = NULL;
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "snprintf_s pazFilePath fail, s32Ret:%d\n", s32Ret);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    HI_DTCF_DIRENT_S stDirentTmp = { HI_NULL, DTCF_DIR_BUTT };
    stDirentTmp.pdirent = p;
    stDirentTmp.enDir = enDir;

    if (s_stDtcfScanFileList.u32DirentlistCount == 0)
    {
        s_stDtcfScanFileList.pstDirentList[0].pdirent = stDirentTmp.pdirent;
        s_stDtcfScanFileList.pstDirentList[0].enDir = stDirentTmp.enDir;
        s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + 1;
        return HI_SUCCESS;
    }

    low = 0;
    high = s_stDtcfScanFileList.u32DirentlistCount - 1;
    while (low <= high && high < s_stDtcfScanFileList.u32DirentlistCount)
    {
        mid = (low + high) / 2;
        s32Ret = sortBydname((HI_VOID *) &s_stDtcfScanFileList.pstDirentList[mid],
                (HI_VOID *) &stDirentTmp);
        if (s32Ret > 0)
        {
            high = mid - 1;
        }
        else if (0 == s32Ret)
        {
            free(p);
            p = NULL;
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "%s is exit at %d \n", pazFileName, mid);
            return HI_DTCF_SAME_FILENAME_PATH;
        }
        else
        {
            low = mid + 1;
        }
    }

    for (i = s_stDtcfScanFileList.u32DirentlistCount; i > high + 1; i--)
    {
        s_stDtcfScanFileList.pstDirentList[i].pdirent =
                s_stDtcfScanFileList.pstDirentList[i - 1].pdirent;
        s_stDtcfScanFileList.pstDirentList[i].enDir =
                s_stDtcfScanFileList.pstDirentList[i - 1].enDir;
    }
    s_stDtcfScanFileList.pstDirentList[high + 1].pdirent = stDirentTmp.pdirent;
    s_stDtcfScanFileList.pstDirentList[high + 1].enDir = stDirentTmp.enDir;
    s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + 1;

    return HI_SUCCESS;
}

HI_S32 HI_DTCF_Init(const HI_CHAR *pazRootDir, const HI_CHAR azDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX])
{
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_DTCF_NULL_PTR(pazRootDir);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    s32Ret = DTCF_CheckRootDir(pazRootDir);
    if (s32Ret != HI_SUCCESS)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "DTCF init failed:0x%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    if((DTCF_STATUS_BUTT != s_enDtcfStatu && DTCF_STATUS_DESTROY != s_enDtcfStatu)
            || DTCF_STATUS_CREATED == s_enDtcfStatu)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "status error, current status:%d\n", s_enDtcfStatu);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_STATUS_ERROR;
    }

    s32Ret = DTCF_mkdirs(pazRootDir, HI_DTCF_DIR_MODE);
    if(s32Ret != HI_SUCCESS)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "DTCF_mkdirs azRootDir:%s fail s32Ret:%x\n", pazRootDir, s32Ret);
        perror(pazRootDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    s32Ret = snprintf_s(s_azRootDir, HI_FILE_PATH_LEN_MAX, HI_FILE_PATH_LEN_MAX-1, "%s", pazRootDir);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(HI_NULL != azDirNames)
    {
        HI_CHAR (*pazDirNamesTmp)[HI_DIR_LEN_MAX] = (HI_CHAR (*)[HI_DIR_LEN_MAX])azDirNames;
        s32Ret = memset_s(g_azDirNames, sizeof(HI_CHAR)*HI_DIR_LEN_MAX*DTCF_DIR_BUTT, 0x0, sizeof(HI_CHAR)*HI_DIR_LEN_MAX*DTCF_DIR_BUTT);
        if(EOK!=s32Ret)
        {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_SYSTEM_ERROR;
        }

        for(i = 0; i < DTCF_DIR_BUTT; i++)
        {
            s32Ret = snprintf_s(g_azDirNames[i], HI_DIR_LEN_MAX, HI_DIR_LEN_MAX-1, "%s", pazDirNamesTmp[i]);
            if(s32Ret < 0)
            {
                DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s g_azDirNames[%d] fail, s32Ret:%d\n", i, s32Ret);
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return HI_DTCF_ERR_EINVAL_PAEAMETER;
            }
            else if(s32Ret >= HI_DIR_LEN_MAX)
            {
                DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazDirNamesTmp[i]:%s is too long, s32Ret:%d\n", pazDirNamesTmp[i], s32Ret);
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return HI_DTCF_ERR_EINVAL_PAEAMETER;
            }
        }
    }

    for(i = 0; i < DTCF_DIR_BUTT; i++)
    {
        if(strlen(g_azDirNames[i]) == 0)
            continue;
        HI_CHAR azTmpDir[HI_FILE_PATH_LEN_MAX] = {0};
        s32Ret = snprintf_s(azTmpDir, HI_FILE_PATH_LEN_MAX, HI_FILE_PATH_LEN_MAX-1, "%s/%s", s_azRootDir, g_azDirNames[i]);
        if(s32Ret <= 0)
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s stTmpDir.azTmpDir fail, s32Ret:%d\n", s32Ret);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        s32Ret = DTCF_mkdir(azTmpDir, HI_DTCF_DIR_MODE);
        if(s32Ret != 0)
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "mkdir g_azDirNames[%d]:%s fail s32Ret:%d\n", i, g_azDirNames[i], s32Ret);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return s32Ret;
        }
    }

    s_enDtcfStatu = DTCF_STATUS_CREATED;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_DeInit(void)
{
    if((DTCF_STATUS_BUTT == s_enDtcfStatu || DTCF_STATUS_DESTROY == s_enDtcfStatu)
            && DTCF_STATUS_CREATED != s_enDtcfStatu)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "status error, current status:%d\n", s_enDtcfStatu);
        return HI_DTCF_ERR_STATUS_ERROR;
    }
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(HI_NULL != s_stDtcfScanFileList.pstDirentList)
    {
        dtcf_FreeScanNameList();
    }

    HI_CHAR azDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX] =
            {
                HI_EMR_DIR,
                HI_EMR_S_DIR,
                HI_MOVIES_DIR,
                HI_MOVIES_S_DIR,
                HI_PARK_DIR,
                HI_PARK_S_DIR,
                HI_EMR_B_DIR,
                HI_EMR_B_S_DIR,
                HI_MOVIES_B_DIR,
                HI_MOVIES_B_S_DIR,
                HI_PARK_B_DIR,
                HI_PARK_B_S_DIR,
                HI_PHOTO_DIR,
                HI_PHOTO_B_DIR
            };
    if( EOK!=memcpy_s(g_azDirNames,sizeof(HI_CHAR) * HI_DIR_LEN_MAX * DTCF_DIR_BUTT,azDirNames,sizeof(HI_CHAR) * HI_DIR_LEN_MAX * DTCF_DIR_BUTT) )
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    s_enDtcfStatu = DTCF_STATUS_DESTROY;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetDirNames(HI_CHAR (*pazDirNames)[HI_DIR_LEN_MAX], HI_U32 u32DirAmount)
{
    HI_S32 s32Ret = EOK;
    HI_U32 i;
    CHECK_DTCF_NULL_PTR(pazDirNames);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(0 == u32DirAmount || u32DirAmount > DTCF_DIR_BUTT)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "u32DirAmount :%d is error\n", u32DirAmount);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for(i = 0; i < u32DirAmount; i++)
    {
        s32Ret = strncpy_s(pazDirNames[i], HI_DIR_LEN_MAX, g_azDirNames[i], HI_DIR_LEN_MAX-1);
        if( EOK!=s32Ret )
        {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

/*
 */
HI_S32 HI_DTCF_Scan(HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32FileAmount)
{
    HI_U32 i;
    CHECK_DTCF_NULL_PTR(enDirs);
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(0 == u32DirCount || u32DirCount > DTCF_DIR_BUTT)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(HI_NULL != s_stDtcfScanFileList.pstDirentList)
    {
        dtcf_FreeScanNameList();
    }

    for(i = 0; i< u32DirCount; i++)
    {
        HI_S32 s32ScanNamelistTmpCount = 0;
        struct dirent **dirScanNameTmpList = HI_NULL;
        if(enDirs[i] >= DTCF_DIR_BUTT)
        {
            HI_DTCF_LOG_Printf(MODULE_NAME_DTCF, HI_DTCF_LOG_LEVEL_ERR, "DTCF enDir:%d out of Range\n", enDirs[i]);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        if(strlen(g_azDirNames[(HI_S32)enDirs[i]]) == 0)
        {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        HI_CHAR azTmpDir[HI_FILE_PATH_LEN_MAX] = {0};
        HI_S32 s32Ret = snprintf_s(azTmpDir, HI_FILE_PATH_LEN_MAX, HI_FILE_PATH_LEN_MAX-1, "%s/%s/", s_azRootDir, g_azDirNames[(HI_S32)enDirs[i]]);
        if(s32Ret <= 0)
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s stTmpDir.azTmpDir fail, s32Ret:%d\n", s32Ret);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }
//        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "azTmpDir:%s\n", azTmpDir);
        s32ScanNamelistTmpCount = scandir(azTmpDir, &dirScanNameTmpList, selector, HI_NULL);
        if(s32ScanNamelistTmpCount < 0)
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "scandir %s return:%d\n", azTmpDir, s32ScanNamelistTmpCount);
            if(HI_NULL != dirScanNameTmpList)
            {
                DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "scandir free dirScanNameTmpList\n");
                free(dirScanNameTmpList);
            }
            continue;
        }

        if(HI_NULL != dirScanNameTmpList)
        {
            s32Ret = DTCF_ExpandFileList(s32ScanNamelistTmpCount);
            if(HI_SUCCESS != s32Ret)
            {
                DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_ExpandFileList fail, s32Ret:%x\n", s32Ret);
                FREE_SCANDIR_LIST(dirScanNameTmpList, s32ScanNamelistTmpCount);
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return s32Ret;
            }
    //        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "s_stDtcfScanFileList.pstDirentList:%p\n", s_stDtcfScanFileList.pstDirentList);
            HI_S32 n = 0;
            for(n = 0; n < s32ScanNamelistTmpCount; n++)
            {
                s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount + n].pdirent =  dirScanNameTmpList[n];
                s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount + n].enDir = enDirs[i];
            }
            s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + s32ScanNamelistTmpCount;
            free(dirScanNameTmpList);
        }
        s_stDtcfScanFileList.enScanDirs[i] = enDirs[i];
    }
    s_stDtcfScanFileList.u32ScanDirCount = u32DirCount;
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    if(s_stDtcfScanFileList.u32DirentlistCount > 0 && HI_NULL != s_stDtcfScanFileList.pstDirentList)
    {
        qsort (s_stDtcfScanFileList.pstDirentList, s_stDtcfScanFileList.u32DirentlistCount, sizeof (HI_DTCF_DIRENT_S), sortBydname);
    }
    s_enDtcfStatu = DTCF_STATUS_SCANED;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetFileByIndex(HI_U32 u32Index, HI_CHAR *pazFileName, HI_U32 u32Length, HI_DTCF_DIR_E *penDir)
{
    CHECK_DTCF_NULL_PTR(pazFileName);
    CHECK_DTCF_NULL_PTR(penDir);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(u32Index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    HI_S32 s32Ret = snprintf_s(pazFileName, u32Length, u32Length-1, "%s/%s/%s", s_azRootDir, g_azDirNames[(HI_S32)s_stDtcfScanFileList.pstDirentList[u32Index].enDir], s_stDtcfScanFileList.pstDirentList[u32Index].pdirent->d_name);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFileName fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((HI_U32)s32Ret >= u32Length)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    *penDir = s_stDtcfScanFileList.pstDirentList[u32Index].enDir;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_DelFileByIndex(HI_U32 u32Index, HI_U32 *pu32FileAmount)
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(u32Index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(HI_NULL != s_stDtcfScanFileList.pstDirentList[u32Index].pdirent)
    {
        free(s_stDtcfScanFileList.pstDirentList[u32Index].pdirent);
        s_stDtcfScanFileList.pstDirentList[u32Index].pdirent = HI_NULL;
    }

    s32Ret = memmove_s(s_stDtcfScanFileList.pstDirentList + u32Index, (sizeof(HI_DTCF_DIRENT_S)*(s_stDtcfScanFileList.u32DirentlistCount-u32Index)),
        s_stDtcfScanFileList.pstDirentList + u32Index + 1, (sizeof(HI_DTCF_DIRENT_S) * (s_stDtcfScanFileList.u32DirentlistCount - u32Index - 1)));
    if(HI_SUCCESS!=s32Ret)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s_stDtcfScanFileList.u32DirentlistCount--;
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_AddFile(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E enDir)
{
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    HI_U32 i = 0;
    HI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};
    HI_CHAR azFileName[HI_FILE_PATH_LEN_MAX] = {0};
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    HI_S32 s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_checkFilePath error s32Ret:%x\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    if(enDir != stSrcTmpDir.enDir)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"pazSrcFilePath:%s enDir:%d is not match\n", pazSrcFilePath, enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for(i = 0; i < s_stDtcfScanFileList.u32ScanDirCount; i++)
    {
        if(enDir == s_stDtcfScanFileList.enScanDirs[i])
            break;
    }

    if( i == s_stDtcfScanFileList.u32ScanDirCount)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"enDir:%d is not scan\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s32Ret = snprintf_s(azFileName, HI_FILE_PATH_LEN_MAX, HI_FILE_PATH_LEN_MAX-1, "%s%s.%s", stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if(s32Ret >= HI_FILE_PATH_LEN_MAX)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", HI_FILE_PATH_LEN_MAX, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s32Ret = DTCF_Addfile(azFileName, enDir);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_Addfile fail, s32Ret:%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetOldestFileIndex(HI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32Index)
{
    CHECK_DTCF_NULL_PTR(enDirs);
    CHECK_DTCF_NULL_PTR(pu32Index);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    HI_S32 i = 0;
    HI_U32 j = 0;
    HI_S32 index = -1;

    if(0 == u32DirCount || u32DirCount > DTCF_DIR_BUTT)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"u32DirCount is illegal, s32Ret:%x\n", HI_DTCF_ERR_EINVAL_PAEAMETER);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for (j = 0; j < u32DirCount; j++)
    {
        if (DTCF_DIR_BUTT <= enDirs[j])
        {
            DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"enDirs[%d] is illegal, s32Ret:%x\n", j,HI_DTCF_ERR_EINVAL_PAEAMETER);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_ERR_EINVAL_PAEAMETER;
        }
    }

    DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO,"u32DirCount:%d u32DirentlistCount:%d\n", u32DirCount, s_stDtcfScanFileList.u32DirentlistCount);
    for(i = s_stDtcfScanFileList.u32DirentlistCount - 1; i >= 0; i--)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO,"i:%d s_stDtcfScanFileList.pstDirentList[i].enDir:%d \n", i, s_stDtcfScanFileList.pstDirentList[i].enDir);
        for(j = 0; j < u32DirCount; j++)
        {
            if(enDirs[j] == s_stDtcfScanFileList.pstDirentList[i].enDir)
            {
                index = i;
                break;
            }
        }

        if(index >= 0)
        {
            break;
        }
    }

    if(index < 0 || (HI_U32)index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"can't find file index:%d\n", index);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    *pu32Index = index;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetFileAmount(HI_U32 *pu32FileAmount)
{
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetOldestFilePath(HI_DTCF_DIR_E enDir, HI_CHAR *pazFilePath, HI_U32 u32Length)
{
    CHECK_DTCF_NULL_PTR(pazFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(strlen(g_azDirNames[enDir]) == 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "g_azDirNames[%d] undefined\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_UNDEFINE_DIR;
    }

    HI_DTCF_TEMP_DIR_S stTmpDir = {{0},0,0,0,{0}};
    HI_S32 s32Ret = snprintf_s(stTmpDir.azTmpDir, HI_FILE_PATH_LEN_MAX, HI_FILE_PATH_LEN_MAX-1, "%s/%s/", s_azRootDir, g_azDirNames[enDir]);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s stTmpDir.azTmpDir fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if(s32Ret >= HI_FILE_PATH_LEN_MAX)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    HI_S32 i = 0;
    HI_S32 index = -1;
    DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO,"enDir:%d u32DirentlistCount:%d\n", enDir, s_stDtcfScanFileList.u32DirentlistCount);
    for(i = s_stDtcfScanFileList.u32DirentlistCount - 1; i >= 0; i--)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO,"i:%d s_stDtcfScanFileList.pstDirentList[i].enDir:%d \n", i, s_stDtcfScanFileList.pstDirentList[i].enDir);
        if(enDir == s_stDtcfScanFileList.pstDirentList[i].enDir)
        {
            index = i;
            break;
        }
    }
    if(index < 0 || (HI_U32)index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"can't find enDir:%d file index:%d\n", enDir,index);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s32Ret = snprintf_s(pazFilePath, u32Length, u32Length-1, "%s%s", stTmpDir.azTmpDir, s_stDtcfScanFileList.pstDirentList[index].pdirent->d_name);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((HI_U32)s32Ret >= u32Length)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

static HI_S32 HI_DTCF_GetTime(HI_CHAR *pazTime, HI_U32 u32BufSize) //日期时间字符串
{
    CHECK_DTCF_NULL_PTR(pazTime);
    HI_S32 s32Ret = HI_SUCCESS;
    struct tm *t = HI_NULL;
    struct  timeval    tv;
    if(0 != gettimeofday(&tv,HI_NULL))
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"gettimeofday error, errno:%d\n", errno);
        perror("gettimeofday");
        return HI_DTCF_ERR_SYSTEM_ERROR;
    }

    if(tv.tv_usec > 500000)
    {
        tv.tv_sec++;
    }

    t = localtime(&tv.tv_sec);
    if(HI_NULL == t)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"localtime error, errno:%d\n", errno);
        perror("localtime");
        return HI_DTCF_ERR_SYSTEM_ERROR;
    }

    DTCF_LOG(HI_DTCF_LOG_LEVEL_INFO,"%4d_%02d%02d_%02d%02d%02d  %ld\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,tv.tv_usec);

    static struct tm s_lasttime;
    static HI_S32 s_s32Num=-1;

    if(t->tm_year==s_lasttime.tm_year&&
       t->tm_mon==s_lasttime.tm_mon&&
       t->tm_mday==s_lasttime.tm_mday&&
       t->tm_hour==s_lasttime.tm_hour&&
       t->tm_min==s_lasttime.tm_min&&
       t->tm_sec==s_lasttime.tm_sec)
    {
        s_s32Num++;
    }
    else
    {
        s_s32Num = 0;
    }

    // if this API was called 100 times or more IN THE SAME SECOND, will cause string length over HI_DTCF_TIME_STR_LEN
    if(s_s32Num>=100)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "s_s32Num=%d, file name over lenght error!\n", s_s32Num);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s32Ret = memcpy_s(&s_lasttime,sizeof(struct tm),t,sizeof(struct tm));
    if( EOK!=s32Ret )
    {
        return HI_DTCF_ERR_SYSTEM_ERROR;
    }

    s32Ret = snprintf_s(pazTime, u32BufSize, HI_DTCF_TIME_STR_LEN, "%4d_%02d_%02d_%02d%02d%02d_%02d",t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,s_s32Num);
    if(s32Ret<0)
    {
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    return HI_SUCCESS;
}

HI_S32 HI_DTCF_CreateFilePath(HI_DTCF_FILE_TYPE_E enFileType, HI_DTCF_DIR_E enDir, HI_CHAR *pazFilePath, HI_U32 u32Length)
{
    CHECK_DTCF_NULL_PTR(pazFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_FILETYPE_RANGE(enFileType);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(strlen(g_azDirNames[enDir]) == 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR, "g_azDirNames[%d] undefined\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_UNDEFINE_DIR;
    }

    HI_CHAR azTime[HI_DTCF_TIME_STR_LEN + 1] = {0};
    HI_S32 s32Ret = HI_DTCF_GetTime(azTime, HI_DTCF_TIME_STR_LEN+1);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"HI_DTCF_GetTime fail, s32Ret:0x%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    s32Ret = snprintf_s(pazFilePath, u32Length, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enDir], azTime, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[enFileType]);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((HI_U32)s32Ret >= u32Length)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetRelatedFilePath(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E enDir, HI_CHAR *pazDstFilePath, HI_U32 u32Length)
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(pazDstFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    HI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_checkFilePath error s32Ret:%x\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    if(enDir == stSrcTmpDir.enDir)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_SAME_DIR_PATH;
    }

    if(HI_DIR_LEN_MAX < strlen(g_azDirNames[enDir]) || strlen(g_azDirNames[enDir]) <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"enDir:%d is not define\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_UNDEFINE_DIR;
    }

    s32Ret = snprintf_s(pazDstFilePath, u32Length, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enDir], stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((HI_U32)s32Ret >= u32Length)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetEmrFilePath(const HI_CHAR *pazSrcFilePath, HI_CHAR *pazDstFilePath, HI_U32 u32Length)
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(pazDstFilePath);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    HI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_checkFilePath error s32Ret:%d\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    HI_DTCF_DIR_E enEmrDir;
    switch(stSrcTmpDir.enDir)
    {
        case DTCF_DIR_NORM_FRONT:
        {
            enEmrDir = DTCF_DIR_EMR_FRONT;
            break;
        }
        case DTCF_DIR_NORM_FRONT_SUB:
        {
            enEmrDir = DTCF_DIR_EMR_FRONT_SUB;
            break;
        }
        case DTCF_DIR_NORM_REAR:
        {
            enEmrDir = DTCF_DIR_EMR_REAR;
            break;
        }
        case DTCF_DIR_NORM_REAR_SUB:
        {
            enEmrDir = DTCF_DIR_EMR_REAR_SUB;
            break;
        }
        default:
        {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return HI_DTCF_UNSUPPORT_PATH;
        }
    }

    if((HI_DTCF_FILE_TYPE_MP4 != stSrcTmpDir.enType) && (HI_DTCF_FILE_TYPE_TS != stSrcTmpDir.enType))
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_UNSUPPORT_PATH;
    }
    s32Ret = snprintf_s(pazDstFilePath, u32Length, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enEmrDir], stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enEmrDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if(s32Ret <= 0)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((HI_U32)s32Ret >= u32Length)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"snprintf_s u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return HI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

HI_S32 HI_DTCF_GetFileDirType(const HI_CHAR *pazSrcFilePath, HI_DTCF_DIR_E *penDir)
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(penDir);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    HI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(HI_SUCCESS != s32Ret)
    {
        DTCF_LOG(HI_DTCF_LOG_LEVEL_ERR,"DTCF_checkFilePath error s32Ret:%d\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    *penDir = stSrcTmpDir.enDir;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
