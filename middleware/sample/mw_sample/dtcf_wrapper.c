#include <stdio.h>
#include <sys/mount.h>
#include <errno.h>
#include <mntent.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/vfs.h>
#include <unistd.h>
#include "dtcf_wrapper.h"
#include "hi_dtcf.h"

static const HI_CHAR* MOUNT_POINT = "/tmp";
#ifndef __HuaweiLite__
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p1";
#else
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p0";
#endif
extern HI_U32 g_u32StreamCnt;
HI_S32 CheckMountPoint()
{
#ifndef __HuaweiLite__
    FILE* pMntTable = setmntent("/proc/mounts", "r");
    struct mntent stMntEnt, *pstMntEnt;
    HI_CHAR aszBuf[1024] = {0};
    if (NULL == pMntTable)
    {
        printf("setmntent /proc/mounts fail\n");
        return HI_FAILURE;
    }
    while (NULL != (pstMntEnt = getmntent_r(pMntTable, &stMntEnt, aszBuf, 1024)))
    {
        //printf("mount_point: %s dev: %s\n", pstMntEnt->mnt_dir, pstMntEnt->mnt_fsname);
        if (0 == strncmp(SDCARD_DEV, pstMntEnt->mnt_fsname, strlen(SDCARD_DEV)))
        {
            if (0 == strncmp(MOUNT_POINT, pstMntEnt->mnt_dir, strlen(MOUNT_POINT)))
            {
                printf("alread mount %s\n", MOUNT_POINT);
                return HI_SUCCESS;
            }
            else
            {
                printf("%s mount to %s\nremount...\n", SDCARD_DEV, pstMntEnt->mnt_dir);
                if (-1 == umount(pstMntEnt->mnt_dir))
                {
                    printf("umount fail\n");
                }
                if (0 == mount(SDCARD_DEV, MOUNT_POINT, "vfat", MS_NOEXEC, 0))
                {
                    return HI_SUCCESS;
                }
            }
        }
    }
    return HI_FAILURE;
#else
    mount(SDCARD_DEV, MOUNT_POINT, "vfat", 0, 0);
#endif
return HI_SUCCESS;
}

static HI_BOOL s_bCapacityCheckStop = HI_FALSE;
static pthread_t s_CapacityThread;

static pthread_mutex_t s_pthDtcfOpt_lock = PTHREAD_MUTEX_INITIALIZER;
static HI_VOID FileOptLock()
{
    HI_S32 s32LockRet = pthread_mutex_lock(&s_pthDtcfOpt_lock);
    if (0 != s32LockRet)
    {
        printf("[%s][%d]DTCF mutex lock failed %d!\n", __FUNCTION__, __LINE__, s32LockRet);
    }
}

static HI_VOID FileOptUnLock()
{
    HI_S32 s32unLockRet = pthread_mutex_unlock(&s_pthDtcfOpt_lock);
    if (0 != s32unLockRet)
    {
        printf("[%s][%d]DTCF mutex unlock failed ret %d!\n", __FUNCTION__, __LINE__, s32unLockRet);
    }
}

void* DiskCapacityCheckThread(void* arg)
{
    prctl(PR_SET_NAME, "DiskCapacityCheck", 0, 0, 0);
    while (HI_FALSE == s_bCapacityCheckStop)
    {
        struct statfs stSDInfo;
        if (-1 == statfs(MOUNT_POINT, &stSDInfo))
        {
            printf("statfs64 fail %s   errno:%d\n",strerror(errno), errno);
            continue;
        }
        if (stSDInfo.f_bavail * stSDInfo.f_bsize < 500 * 1024 * 1024)
        {
            HI_CHAR aszOldestFile[1024] = {0};
            HI_CHAR aszOldestSubFile[1024] = {0};
            HI_U32 u32Index;
            HI_U32 u32FileAmount;
            HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
            FileOptLock();
            HI_DTCF_DIR_E azOldestDir[DTCF_DIR_BUTT] = {
                     DTCF_DIR_NORM_FRONT
            };
            if (HI_SUCCESS != HI_DTCF_GetOldestFileIndex(azOldestDir, 1, &u32Index))
            {
                if (HI_SUCCESS != HI_DTCF_GetOldestFileIndex(azOldestDir, 1, &u32Index))
                {
                    printf("no avail file to delte\n");
                }
                else
                {
                    if (HI_SUCCESS != HI_DTCF_GetFileByIndex(u32Index, aszOldestFile, 1024, &enDir))
                    {
                        printf("HI_DTCF_GetFileByIndex fail\n");
                    }
                    printf("delete %s\n", aszOldestFile);
                    remove(aszOldestFile);
                    if (HI_SUCCESS != HI_DTCF_DelFileByIndex(u32Index, &u32FileAmount))
                    {
                        printf("HI_DTCF_DelFileByIndex fail\n");
                    }
                    if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_EMR_FRONT_SUB, aszOldestSubFile, 1024))
                    {
                        printf("delete %s\n", aszOldestSubFile);
                        remove(aszOldestSubFile);
                    }
                    if (4 == g_u32StreamCnt)
                    {
                        if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_EMR_REAR, aszOldestSubFile, 1024))
                        {
                            printf("delete %s\n", aszOldestSubFile);
                            remove(aszOldestSubFile);
                        }
                        if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_EMR_REAR_SUB, aszOldestSubFile, 1024))
                        {
                            printf("delete %s\n", aszOldestSubFile);
                            remove(aszOldestSubFile);
                        }
                    }
                }
            }
            else
            {
                if (HI_SUCCESS != HI_DTCF_GetFileByIndex(u32Index, aszOldestFile, 1024, &enDir))
                {
                    printf("HI_DTCF_GetFileByIndex fail\n");
                }
                printf("delete %s\n", aszOldestFile);
                remove(aszOldestFile);
                if (HI_SUCCESS != HI_DTCF_DelFileByIndex(u32Index, &u32FileAmount))
                {
                    printf("HI_DTCF_DelFileByIndex fail\n");
                }
                if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_NORM_FRONT_SUB, aszOldestSubFile, 1024))
                {
                    printf("delete %s\n", aszOldestSubFile);
                    remove(aszOldestSubFile);
                }
                if (4 == g_u32StreamCnt)
                {
                    if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_NORM_REAR, aszOldestSubFile, 1024))
                    {
                        printf("delete %s\n", aszOldestSubFile);
                        remove(aszOldestSubFile);
                    }
                    if (HI_SUCCESS == HI_DTCF_GetRelatedFilePath(aszOldestFile, DTCF_DIR_NORM_REAR_SUB, aszOldestSubFile, 1024))
                    {
                        printf("delete %s\n", aszOldestSubFile);
                        remove(aszOldestSubFile);
                    }
                }
            }
            FileOptUnLock();
        }
        usleep(500 * 1000);
    }
    return NULL;
}

HI_S32 FileOptInit()
{
    HI_U32 u32FileAmount;
    const HI_CHAR apszDirs[DTCF_DIR_BUTT][HI_DIR_LEN_MAX] = {"emr", "emr_s", "norm", "norm_s", "", "", "emr_b", "emr_b_s", "norm_b", "norm_b_s", "", "", ""};
    if (HI_SUCCESS != CheckMountPoint())
    {
        #ifndef __HuaweiLite__
        if (-1 == mount(SDCARD_DEV, MOUNT_POINT, "vfat", MS_NOEXEC, 0))
        {
            printf("mount fail %d\n", errno);
            return HI_FAILURE;
        }
        #endif
    }
    s_bCapacityCheckStop = HI_FALSE;
    pthread_create(&s_CapacityThread, NULL, DiskCapacityCheckThread, NULL);
    HI_S32 s32Ret = HI_DTCF_Init(MOUNT_POINT, apszDirs);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_DTCF_Init fail %d\n", s32Ret);
        return HI_FAILURE;
    }
    HI_DTCF_DIR_E azScanDir[DTCF_DIR_BUTT] =
    {
        DTCF_DIR_EMR_FRONT,
        DTCF_DIR_NORM_FRONT,
    };
    return HI_DTCF_Scan(azScanDir, 2, &u32FileAmount);
}

HI_S32 FileOptDeInit()
{
    s_bCapacityCheckStop = HI_TRUE;
    pthread_join(s_CapacityThread, NULL);
    if (-1 == umount(MOUNT_POINT))
    {
        printf("umount fail\n");
    }
    return HI_DTCF_DeInit();
}

HI_S32 AddNorm(const HI_CHAR* pszFileName)
{
    HI_DTCF_DIR_E enDir = DTCF_DIR_BUTT;
    if (HI_SUCCESS != HI_DTCF_GetFileDirType(pszFileName, &enDir))
    {
        printf("HI_DTCF_GetFileDirType %s fail\n", pszFileName);
    }
    if (DTCF_DIR_NORM_FRONT == enDir)
    {
        FileOptLock();
        if (HI_SUCCESS != HI_DTCF_AddFile(pszFileName, DTCF_DIR_NORM_FRONT))
        {
            printf("HI_DTCF_AddFile %s fail\n", pszFileName);
        }
        FileOptUnLock();
    }
    return HI_SUCCESS;
}

HI_S32 MoveEmr(const HI_CHAR* pszFileName)
{
    HI_CHAR aszEmrFilePath[512];
    if (HI_SUCCESS != HI_DTCF_GetEmrFilePath(pszFileName, aszEmrFilePath, 512))
    {
        printf("call HI_DTCF_GetEmrFilePath fail\n");
        return HI_FAILURE;
    }
    if (-1 == rename(pszFileName, aszEmrFilePath))
    {
        printf("call rename %s fail %s\n", aszEmrFilePath, strerror(errno));
        return HI_FAILURE;
    }
    FileOptLock();
    if (HI_SUCCESS != HI_DTCF_AddFile(aszEmrFilePath, DTCF_DIR_EMR_FRONT))
    {
        printf("HI_DTCF_AddFile %s fail\n", aszEmrFilePath);
        FileOptUnLock();
        return HI_FAILURE;
    }
    FileOptUnLock();
    return HI_SUCCESS;
}

HI_S32 RequestFileNames(HI_MW_PTR pRecord,HI_U32 u32FileCnt, HI_CHAR (*paszFileNames)[HI_REC_FILE_NAME_LEN])
{
    HI_S32 s32Ret = HI_DTCF_CreateFilePath(HI_DTCF_FILE_TYPE_MP4, DTCF_DIR_NORM_FRONT, paszFileNames[0], HI_REC_FILE_NAME_LEN);
    MW_CHECK_RETURN(s32Ret);
    s32Ret = HI_DTCF_GetRelatedFilePath(paszFileNames[0], DTCF_DIR_NORM_FRONT_SUB, paszFileNames[1], HI_REC_FILE_NAME_LEN);
    MW_CHECK_RETURN(s32Ret);
    s32Ret = HI_DTCF_GetRelatedFilePath(paszFileNames[0], DTCF_DIR_NORM_REAR, paszFileNames[2], HI_REC_FILE_NAME_LEN);
    MW_CHECK_RETURN(s32Ret);
    s32Ret = HI_DTCF_GetRelatedFilePath(paszFileNames[0], DTCF_DIR_NORM_REAR_SUB, paszFileNames[3], HI_REC_FILE_NAME_LEN);
    return s32Ret;
}

HI_S32 RequestDualFileNames(HI_MW_PTR pRecord,HI_U32 u32FileCnt, HI_CHAR (*paszFileNames)[HI_REC_FILE_NAME_LEN])
{
    HI_S32 s32Ret = HI_DTCF_CreateFilePath(HI_DTCF_FILE_TYPE_MP4, DTCF_DIR_NORM_FRONT, paszFileNames[0], HI_REC_FILE_NAME_LEN);
    MW_CHECK_RETURN(s32Ret);
    s32Ret = HI_DTCF_GetRelatedFilePath(paszFileNames[0], DTCF_DIR_NORM_REAR, paszFileNames[1], HI_REC_FILE_NAME_LEN);
    return HI_SUCCESS;
}
