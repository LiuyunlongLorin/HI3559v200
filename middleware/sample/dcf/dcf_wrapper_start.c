#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <errno.h>
#include <mntent.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/vfs.h>
#include <unistd.h>
#include "hi_dcf_wrapper.h"
#include "dcf_wrapper_app.h"

HI_DCF_WRAPPER_INDEX g_stDCFIdx;
#ifndef __HuaweiLite__
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p1";
#else
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p0";
#endif
static const HI_CHAR* MOUNT_POINT = "/tmp";

extern HI_DCF_WRAPPER_NAMERULE_CFG_S g_stNameRuleCfg_DV;
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

static HI_VOID OnScanEvent(const HI_DCF_WRAPPER_INDEX iDCF, const HI_DCF_WRAPPER_SCAN_STATE_E enState, HI_S32 s32ScanErrCode)
{
    if(HI_DCF_WRAPPER_SCAN_STATE_LAST_DIR_DONE == enState)
    {
        printf("HI_DCF_WRAPPER_SCAN_STATE_LAST_DIR_DONE\n");
    }
    else if(HI_DCF_WRAPPER_SCAN_STATE_FIRST_DIR_DONE == enState)
    {
        printf("HI_DCF_WRAPPER_SCAN_STATE_FIRST_DIR_DONE\n");
    }
    else if(HI_DCF_WRAPPER_SCAN_STATE_ALL_DIR_DONE == enState)
    {
        printf("HI_DCF_WRAPPER_SCAN_STATE_ALL_DIR_DONE\n");
    }
    else if(HI_DCF_WRAPPER_SCAN_STATE_FAILURE == enState)
    {
        printf("HI_DCF_WRAPPER_SCAN_STATE_FAILURE: %x\n", s32ScanErrCode);
    }
    return;
}

HI_S32 FileOptInit()
{
    HI_S32 s32Ret = HI_SUCCESS;
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

    s32Ret = HI_DCF_WRAPPER_Init();
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    s32Ret = HI_DCF_WRAPPER_Create(OnScanEvent, &g_stNameRuleCfg_DV, &g_stDCFIdx);
    DCF_SAMPLE_CHECK_RETURN(s32Ret);

    printf("[%s]:[%s][%d][SUCCESS]\n", DCF_SAMPLE_NAME,__func__, __LINE__);
    return HI_SUCCESS;
}

HI_S32 FileOptDeInit()
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (-1 == umount(MOUNT_POINT))
    {
        printf("umount fail\n");
    }

    s32Ret = HI_DCF_WRAPPER_Destroy(g_stDCFIdx);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_DCF_WRAPPER_Destroy fail %d\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_DCF_WRAPPER_DeInit();
    DCF_SAMPLE_CHECK_RETURN(s32Ret);
    printf("[%s]:[%s][%d][SUCCESS]\n", DCF_SAMPLE_NAME,__func__, __LINE__);
    return HI_SUCCESS;
}
