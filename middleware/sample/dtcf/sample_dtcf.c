#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#include "hi_dtcf.h"
#include "hi_type.h"

#define HI_EMR_DIR "EMR"
#define HI_EMR_S_DIR "EMR_s"
#define HI_MOVIES_DIR "Movie"
#define HI_MOVIES_S_DIR "Movie_s"
#define HI_EMR_B_DIR "EMR_b"
#define HI_EMR_B_S_DIR "EMR_b_s"
#define HI_MOVIES_B_DIR "Movie_b"
#define HI_MOVIES_B_S_DIR "Movie_b_s"
#define HI_PHOTO_DIR "Photo"
#define MOUNT_POINT  "/tmp"

static HI_S32 SAMPLE_mount_tmp_dir()
{
#ifndef __HIX86__
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef __HuaweiLite__
    s32Ret = mount("/dev/mmcblk0p0", MOUNT_POINT, "vfat", 0, 0);
#else
    s32Ret = mount("/dev/mmcblk0p1", MOUNT_POINT, "vfat", MS_NOEXEC, 0);
#endif
    if (HI_SUCCESS != s32Ret)
    {
        printf("mount tmp failed, errno:%d \n", errno);
    }
#endif
    return s32Ret;
}

static HI_VOID SAMPLE_umount_tmp_dir()
{
 #ifndef __HIX86__
    HI_S32 s32Ret = umount(MOUNT_POINT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("umount tmp failed, errno:%d \n", errno);
    }
 #endif
    return;
}

HI_VOID printHelp()
{
    printf("1-----Scan files\n");
    printf("2-----Top file\n");
    printf("3-----Prev file\n");
    printf("4-----Next file\n");
    printf("5-----Bottom file\n");
    printf("6-----get oldest files\n");
    printf("7-----create file\n");
    printf("8-----get relation file path\n");
    printf("9-----get emergency file path\n");
    printf("a-----get directory names \n");
    printf("e-----EXIT\n");
}

HI_DTCF_DIR_E azScanDir[DTCF_DIR_BUTT] = {
        DTCF_DIR_NORM_FRONT,
        DTCF_DIR_NORM_FRONT_SUB,
};

HI_CHAR azFilePath[HI_FILE_PATH_LEN_MAX] = {0};
HI_CHAR azDstFilePath[HI_FILE_PATH_LEN_MAX] = {0};
HI_CHAR azDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX] = {
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0}
};
struct timeval tv1, tv2;

static HI_VOID dtcf_open_file(HI_CHAR *pFile)
{
    FILE *fHandle = HI_NULL;
    HI_CHAR azBuffer[300] = {0};
    fHandle = fopen(pFile, "wb+");
    if(fHandle == NULL)
    {
        printf("fopen %s error.\n", pFile);
        return;
    }
    HI_S32 s32Ret = fwrite(azBuffer, 300, 1, fHandle);
    if (s32Ret < 0)
    {
        printf("fwrite error %d\n", s32Ret);
    }
    if (fflush(fHandle))
    {
        perror("fflush file\n");
    }

    if(fclose(fHandle))
    {
        perror("close file\n");
    }
    return;
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __HuaweiLite__
    /*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
    /*init cpp running context on liteos end*/
#endif
    HI_U32 u32FileAmount = 0;
    HI_U32 u32FileIndex = 0;
    HI_BOOL bRun = HI_TRUE;
    HI_U32 i;
    HI_DTCF_DIR_E enDir;
    SAMPLE_mount_tmp_dir();
    HI_S32 s32Ret = HI_DTCF_Init("/tmp/YICarCam", HI_NULL);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_umount_tmp_dir();
        return HI_FAILURE;
    }
    printHelp();
    while (HI_TRUE == bRun)
    {
        int cmd;
        cmd = getchar();

        switch (cmd)
        {
            case '1':
                gettimeofday(&tv1, NULL);
                s32Ret = HI_DTCF_Scan(azScanDir, 2, &u32FileAmount);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_Scan error s32Ret: %x\n", s32Ret);
                    break;
                }
                gettimeofday(&tv2, NULL);
                printf("Scan %s %s find %d files used time:%ld sec, %ld usec\n", HI_MOVIES_DIR, HI_MOVIES_S_DIR, u32FileAmount, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
                break;

            case '2':
                u32FileIndex = 0;
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetFileByIndex(u32FileIndex, azFilePath, HI_FILE_PATH_LEN_MAX, &enDir);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetFileByIndex error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("Top file %s \n", azFilePath);
                break;

            case '3':
                if ((u32FileAmount == 0) || (u32FileIndex == 0))
                {
                    printf("!!!warning : no prev file\n");
                    break;
                }
                u32FileIndex--;
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetFileByIndex(u32FileIndex, azFilePath, HI_FILE_PATH_LEN_MAX, &enDir);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetFileByIndex error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("Prev file %s \n", azFilePath);
                break;

            case '4':
                if ((u32FileAmount == 0) || (u32FileIndex >= u32FileAmount - 1))
                {
                    printf("!!!warning : no next file\n");
                    break;
                }
                u32FileIndex++;
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetFileByIndex(u32FileIndex, azFilePath, HI_FILE_PATH_LEN_MAX, &enDir);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetFileByIndex error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("Next file %s \n", azFilePath);
                break;

            case '5':
                (u32FileAmount == 0)?(u32FileIndex = 0) : (u32FileIndex = u32FileAmount - 1);
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetFileByIndex(u32FileIndex, azFilePath, HI_FILE_PATH_LEN_MAX, &enDir);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetFileByIndex error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("Bottom file %s \n", azFilePath);
                break;

            case '6':
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetOldestFilePath(DTCF_DIR_NORM_FRONT,azFilePath, HI_FILE_PATH_LEN_MAX);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetOldestFilePath error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("DTCF_DIR_EMR_FRONT OldestFile %s \n", azFilePath);
                break;

            case '7':
                memset(azFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_CreateFilePath(HI_DTCF_FILE_TYPE_MP4, DTCF_DIR_NORM_FRONT, azFilePath, HI_FILE_PATH_LEN_MAX);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_CreateFilePath error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("HI_DTCF_CreateFilePath %s \n", azFilePath);
                dtcf_open_file(azFilePath);

                s32Ret = HI_DTCF_AddFile(azFilePath, DTCF_DIR_NORM_FRONT);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_AddFile error s32Ret: %x\n", s32Ret);
                }
                u32FileAmount++;
                break;

            case '8':
                memset(azDstFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetRelatedFilePath(azFilePath,DTCF_DIR_EMR_FRONT,azDstFilePath, HI_FILE_PATH_LEN_MAX);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetRelatedFilePath error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("HI_DTCF_GetRelatedFilePath srcPath:%s \n", azFilePath);
                printf("HI_DTCF_GetRelatedFilePath DTCF_DIR_EMR_FRONT:%s \n", azDstFilePath);

                memset(azDstFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetRelatedFilePath(azFilePath,DTCF_DIR_EMR_FRONT_SUB,azDstFilePath, HI_FILE_PATH_LEN_MAX);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetRelatedFilePath error s32Ret: %x\n", s32Ret);
                }
                printf("HI_DTCF_GetRelatedFilePath DTCF_DIR_EMR_FRONT_SUB:%s \n", azDstFilePath);
                break;

            case '9':
                memset(azDstFilePath, 0x0, sizeof(HI_CHAR) * HI_FILE_PATH_LEN_MAX);
                s32Ret = HI_DTCF_GetEmrFilePath(azFilePath, azDstFilePath, HI_FILE_PATH_LEN_MAX);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetEmrFilePath error s32Ret: %x\n", s32Ret);
                    break;
                }
                printf("HI_DTCF_GetEmrFilePath success:[srcPath:%s,emergency:%s]\n", azFilePath, azDstFilePath);
                break;

            case 'a':
                s32Ret = HI_DTCF_GetDirNames(azDirNames,DTCF_DIR_BUTT);
                if(HI_SUCCESS != s32Ret)
                {
                    printf("HI_DTCF_GetDirNames error s32Ret: %x\n", s32Ret);
                    break;
                }
                for(i = 0; i < DTCF_DIR_BUTT;i++)
                {
                    printf(":%s \n", azDirNames[i]);
                }
                break;

            case 'e':
                bRun = HI_FALSE;
                break;

            default:
                printHelp();
                break;
        }
    }

    if(HI_SUCCESS != HI_DTCF_DeInit())
    {
        printf("HI_DTCF_DeInit error s32Ret: %x\n", s32Ret);
    }
    SAMPLE_umount_tmp_dir();
    return HI_SUCCESS;

}
