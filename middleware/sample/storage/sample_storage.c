#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif
#include "hi_storage.h"
#include "fstool_wrapper.h"

#define STORAGE_SAMPLE_NAME     "STORAGE_SAMPLE"
#define STORAGE_FSTYPE_VFAT     "vfat"

#define STORAGE_SAMPLE_CHECK_RETURN(s32Ret) \
    do {\
        if (HI_SUCCESS != s32Ret)\
        {\
            printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);\
            return s32Ret;\
        }\
    } while (0)

static HI_MW_PTR s_hStorage = HI_NULL;

static HI_VOID GetState(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    s32Ret = HI_STORAGE_GetState(hStorage, &enState);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----State:%d\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, enState);
}

static HI_VOID GetFSInfo(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_FS_INFO_S stFSInfo = {0};
    s32Ret = HI_STORAGE_GetFSInfo(hStorage, &stFSInfo);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----cluster(%llu), total(%llu), available(%llu), used(%llu)\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, stFSInfo.u64ClusterSize, stFSInfo.u64TotalSize, stFSInfo.u64AvailableSize, stFSInfo.u64UsedSize);
}

static HI_VOID GetDevInfo(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_DEV_INFO_S stDevInfo = {{0}};
    s32Ret = HI_STORAGE_GetDevInfo(hStorage, &stDevInfo);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----DevType (%s) CID(%s), mode(%s), clock(%s), speed(%d)\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, stDevInfo.aszDevType,stDevInfo.aszCID, stDevInfo.aszWorkMode, stDevInfo.aszWorkClock, stDevInfo.enTranSpeed);
}

static HI_VOID FSFormat(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64ClusterSize = KB_2_BYTE_FACTOR*64;
    s32Ret = HI_STORAGE_Format(hStorage, STORAGE_FSTYPE_VFAT, u64ClusterSize);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----format SUCCESS, Byte(%llu)\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, u64ClusterSize);
}

static HI_VOID Pause(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_STORAGE_Pause(hStorage);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----Pause SUCCESS\n", STORAGE_SAMPLE_NAME,__func__, __LINE__);
}

static HI_VOID Resume(HI_MW_PTR hStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_STORAGE_Resume(hStorage);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s]:[%s][%d]-----FAILURE:0x%x\n", STORAGE_SAMPLE_NAME,__func__, __LINE__, s32Ret);
        return;
    }
    printf("[%s]:[%s][%d]-----Resume SUCCESS\n", STORAGE_SAMPLE_NAME,__func__, __LINE__);
}

static HI_VOID OnStateChange(HI_MW_PTR hStorage, const HI_CHAR* pszPartitionPath, HI_STORAGE_STATE_E enState, HI_S32 s32ErrCode)
{
    (void)(hStorage);
    switch (enState)
    {
        case HI_STORAGE_STATE_DEV_UNPLUGGED :
            printf("!!!!!dev(%s) plugged out\n", pszPartitionPath);
            break;
        case HI_STORAGE_STATE_DEV_CONNECTING :
            printf("!!!!!dev(%s) connecting\n", pszPartitionPath);
            break;
        case HI_STORAGE_STATE_DEV_ERROR :
            printf("!!!!!dev(%s) dev error\n", pszPartitionPath);
            break;
        case HI_STORAGE_STATE_FS_CHECKING :
            printf("!!!!!dev(%s) fs checking\n", pszPartitionPath);
            break;
        case HI_STORAGE_STATE_FS_CHECK_FAILED :
            printf("!!!!!dev(%s) check failed and errno(0x%x)\n", pszPartitionPath, s32ErrCode);
            break;
        case HI_STORAGE_STATE_FS_EXCEPTION :
            printf("!!!!!dev(%s) fs exception and errno(0x%x)\n", pszPartitionPath, s32ErrCode);
            break;
        case HI_STORAGE_STATE_MOUNTED :
            printf("!!!!!dev(%s) fs mounted\n", pszPartitionPath);
            break;
        case HI_STORAGE_STATE_MOUNT_FAILED :
            printf("!!!!!dev(%s) mount failed and errno(0x%x)\n", pszPartitionPath,s32ErrCode);
            break;
        case HI_STORAGE_STATE_IDEL :
            printf("!!!!!dev(%s) is idel\n", pszPartitionPath);
            break;
        default :
            break;
    }
}

static HI_S32 Create(HI_MW_PTR* phStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_CFG_S stCfg = {0};
    HI_STORAGE_FS_TOOL_S stFSTool = {0};

    stCfg.u8DevPortNo = 0;
    stCfg.u8DevErrCountThr = 3;
#ifdef __HuaweiLite__
    snprintf(stCfg.aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, "%s", "/dev/mmcblk0p0");
#else
    snprintf(stCfg.aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, "%s", "/dev/mmcblk0p1");
#endif
    snprintf(stCfg.aszMountPath, HI_STORAGE_PATH_LEN_MAX, "%s", "/tmp");

    stFSTool.pfnCheck = FSTOOL_WRAPPER_Check;
    stFSTool.pfnFormat = FSTOOL_WRAPPER_Format;
    snprintf(stFSTool.aszFSType, HI_STORAGE_PATH_LEN_MAX, "%s", STORAGE_FSTYPE_VFAT);

    s32Ret = FSTOOL_WRAPPER_RegBufOps();
    STORAGE_SAMPLE_CHECK_RETURN(s32Ret);

    s32Ret = HI_STORAGE_RegisterFSTool(&stFSTool);
    STORAGE_SAMPLE_CHECK_RETURN(s32Ret);

    s32Ret = HI_STORAGE_Create(&stCfg, OnStateChange, phStorage);
    if (HI_SUCCESS != s32Ret)
    {
        (HI_VOID)HI_STORAGE_UnregisterFSTool("vfat");
    }
    STORAGE_SAMPLE_CHECK_RETURN(s32Ret);
    return HI_SUCCESS;
}

static HI_VOID Destroy(HI_MW_PTR* phStorage)
{
    (HI_VOID)HI_STORAGE_Destroy(phStorage);
    (HI_VOID)HI_STORAGE_UnregisterFSTool(STORAGE_FSTYPE_VFAT);
}

void ShowHelp()
{
    printf("STORAGE HELP:\n"
        "\n"
        "cmd description\n"
        "cmd[gs] :get storage state \n"
        "cmd[gfi] :get fs info \n"
        "cmd[gdi] :get dev info \n"
        "cmd[fmt] :format \n"
        "cmd[p] :pause storage \n"
        "cmd[r] :resume storage \n");

    printf("cmd[h]:show help info\n"
        "cmd[exit]:exit the sample\n"
        "\n");
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __HuaweiLite__
    extern char __init_array_start__, __init_array_end__;
    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__,NO_SCATTER);
#endif
    (void)(argc);
    (void)(argv);

    HI_S32 s32Ret = HI_SUCCESS;
    char cmd[516] = {0};
    s32Ret = Create(&s_hStorage);
    STORAGE_SAMPLE_CHECK_RETURN(s32Ret);

    printf("Input CMD: ");
    while (NULL != fgets(cmd, 516, stdin))
    {
        if (0 == strncmp(cmd, "h", 1))
        {
            ShowHelp();
            printf("Input CMD: ");
            continue;
        }
        else if (0 == strncmp(cmd, "exit", 4))
        {
            printf("will exit\n");
            break;
        }
        else if (0 == strncmp(cmd, "gs", 2))
        {
            GetState(s_hStorage);
        }
        else if (0 == strncmp(cmd, "gfi", 3))
        {
            GetFSInfo(s_hStorage);
        }
        else if (0 == strncmp(cmd, "gdi", 3))
        {
            GetDevInfo(s_hStorage);
        }
        else if (0 == strncmp(cmd, "fmt", 3))
        {
            FSFormat(s_hStorage);
        }
        else if (0 == strncmp(cmd, "p", 1))
        {
            Pause(s_hStorage);
        }
        else if (0 == strncmp(cmd, "r", 1))
        {
            Resume(s_hStorage);
        }

        printf("Input CMD: ");
    }
    Destroy(s_hStorage);
    s_hStorage = HI_NULL;
    return 0;
}
