#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/mount.h>
#include <errno.h>
#include <limits.h>

#include "hi_mw_type.h"
#include "stg_log.h"
#include "hi_defs.h"
#include "hi_storage.h"
#include "stg_common.h"
#include "stg_proc.h"
#include "stg_fstool.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef __HuaweiLite__
#define MS_NOEXEC 8
#endif

#define STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT    (3)
#define STG_FSTOOL_FSTYPE_ID_OFFSET_VFAT     (82)
#define STG_FSTOOL_FSTYPE_ID_LENGTH          (8)
#define STG_FSTOOL_FSTYPE_ID_EXFAT  "EXFAT   "
#define STG_FSTOOL_FSTYPE_ID_VFAT   "FAT32   "
#define STG_FSTOOL_FSTYPE_EXFAT     "exfat"
#define STG_FSTOOL_FSTYPE_VFAT      "vfat"
#define STG_FSTOOL_TEST_PARTITION_FILE_NAME "test.file"
#define STG_FSTOOL_TEST_PARTITION_DATA_LEN  (4096)
#define STG_FSTOOL_FAT_BOOT_SEC_LEN   (512)

#define STG_FSTOOL_PARTITION_DETECT_CNT      (20)
#define STG_FSTOOL_PARTITION_DETECT_INTERVAL (10000)//unit:us

typedef struct tagSTG_FSTOOL_S
{
    STG_FS_STATE_E enState;
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX];
    HI_CHAR aszMountPath[HI_STORAGE_PATH_LEN_MAX];
} STG_FSTOOL_S;

typedef struct STG_FSTOOL_NODE_S
{
    struct STG_FSTOOL_NODE_S* pstNext;
    HI_STORAGE_FS_TOOL_S stTool;
} STG_FSTOOL_NODE_S;

static STG_FSTOOL_NODE_S* pstFSTooList = HI_NULL;

static HI_S32 STG_FSTOOL_ReadPartition(const HI_CHAR* pszPartition, off_t offset, size_t nmemb, HI_CHAR* pszFSType)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* pFile = HI_NULL;
    HI_CHAR* pRealPath = HI_NULL;
    HI_CHAR aszRealPath[PATH_MAX + 1] = {0};

    pRealPath = realpath(pszPartition, aszRealPath);
    if (HI_NULL == pRealPath)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  real path(%s) failed , and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, pszPartition, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    pFile = fopen(aszRealPath, "r");
    if (HI_NULL == pFile)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open file(%s) failed , and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, aszRealPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    if (0 != fseeko(pFile, offset, SEEK_SET) || nmemb != fread(pszFSType, 1, nmemb, pFile))
    {
        s32Ret = HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read file(%s) failed , and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, aszRealPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
    }
    else
    {
        pszFSType[nmemb] = '\0';
    }

    if (0 != fclose(pFile))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read file(%s) failed , and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, aszRealPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
    }

    return s32Ret;
}

static HI_S32 STG_FSTOOL_IsExFatType(HI_CHAR* pszBootSector)
{
    if (0 == strncmp(pszBootSector + STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT, STG_FSTOOL_FSTYPE_ID_EXFAT, STG_FSTOOL_FSTYPE_ID_LENGTH))
    {
        return HI_SUCCESS;
    }
    else
    {
        return HI_FAILURE;
    }
}

static HI_S32 STG_FSTOOL_IsFat32Type(HI_CHAR* pszBootSector)
{
    //if 1 condition of 2 is agree,then think it is fat32 system.
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;
    HI_U32 fat32FileTypeOffeset = 82;
    //check BS_FilSysType equals "FAT32   ",if equals then return;
    HI_CHAR aszBSFilSysType[8] = {0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20};
    for (s32Index = 0; s32Index < 8; s32Index++)
    {
        if (aszBSFilSysType[s32Index] != pszBootSector[s32Index + fat32FileTypeOffeset])
        {
            break;
        }
    }
    if(s32Index == 8)
    {
        return HI_SUCCESS;
    }

    //accord to fat32 check bpb_bkbootsec and bpb reserved,if equal also judge fat32 system;
    HI_U32 fat32BPBBkBootSec = 50;
    HI_CHAR aszFat32Identify[14] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (s32Index = 0; s32Index < 14; s32Index++)
    {
        if (aszFat32Identify[s32Index] != pszBootSector[s32Index + fat32BPBBkBootSec])
        {
            s32Ret = HI_FAILURE;
            break;
        }
    }
    return s32Ret;
}

static HI_S32 STG_FSTOOL_GetFSType(const HI_CHAR* pszPartition, HI_CHAR* pszFSType)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszBootSector[STG_FSTOOL_FAT_BOOT_SEC_LEN + 1] = {0};
    s32Ret = STG_FSTOOL_ReadPartition(pszPartition, 0, STG_FSTOOL_FAT_BOOT_SEC_LEN, aszBootSector);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get fs type of partition(%s) failed , and ret:%x!\n", __FUNCTION__, __LINE__, pszPartition, s32Ret);
        return s32Ret;
    }

    s32Ret = STG_FSTOOL_IsFat32Type(aszBootSector);
    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = snprintf_s(pszFSType, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", STG_FSTOOL_FSTYPE_VFAT);
        if ( s32Ret < 0 )
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  snprintf failed,and ret:%x!\n", __FUNCTION__, __LINE__, s32Ret);
            return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
        }
        return HI_SUCCESS;
    }

    s32Ret = STG_FSTOOL_IsExFatType(aszBootSector);
    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = snprintf_s(pszFSType, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", STG_FSTOOL_FSTYPE_EXFAT);
        if ( s32Ret < 0 )
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  snprintf failed,and ret:%x!\n", __FUNCTION__, __LINE__, s32Ret);
            return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
        }
        return HI_SUCCESS;
    }

    //get type from other  file system support should add here,and fill pszFSType for mount and format ops.
    STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  unknow fs type of partition(%s), and ret:%x!\n", __FUNCTION__, __LINE__, pszPartition, HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT);
    return HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT;
}

static HI_S32 STG_FSTOOL_GetNode(const HI_CHAR* pszFSType, STG_FSTOOL_NODE_S** ppstNode)
{
    STG_FSTOOL_NODE_S* pstTmpNode = pstFSTooList;

    while (pstTmpNode)
    {
        if (0 == strncmp(pstTmpNode->stTool.aszFSType, pszFSType, HI_STORAGE_PATH_LEN_MAX))
        {
            break;
        }

        pstTmpNode = pstTmpNode->pstNext;
    }

    if (HI_NULL == pstTmpNode)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  no tool of fs(%s) , and ret:%x!\n", __FUNCTION__, __LINE__, pszFSType, HI_ERR_STORAGE_ILLEGAL_PARAM);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    *ppstNode = pstTmpNode;
    return HI_SUCCESS;
}

static HI_S32 STG_FSTOOL_DetectPartition(HI_CHAR* pszPartition)
{
    HI_CHAR* pRealPath = HI_NULL;
    HI_CHAR aszRealPath[PATH_MAX + 1] = {0};
    HI_U32 u32DetectCnt = 0;

    while (u32DetectCnt < STG_FSTOOL_PARTITION_DETECT_CNT)
    {
        pRealPath = realpath(pszPartition, aszRealPath);
        if (HI_NULL == pRealPath)
        {
            u32DetectCnt++;
            hi_mw_usleep(STG_FSTOOL_PARTITION_DETECT_INTERVAL);
            continue;
        }
        else
        {
            break;
        }
    }

    if (HI_NULL == pRealPath)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  partition(%s) is not found in time, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, pszPartition, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION, errno);
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    return HI_SUCCESS;
}

static HI_S32 STG_FSTOOL_TestPartition(HI_MW_PTR pFSTool, HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 fd = 0;
    HI_CHAR aszTestFile[HI_STORAGE_PATH_LEN_MAX+16] = {0};
    HI_CHAR aszTestData[STG_FSTOOL_TEST_PARTITION_DATA_LEN] = {0};
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;

    (HI_VOID)STG_FSTOOL_Umount(pFSTool, HI_FALSE);

    s32Ret = STG_FSTOOL_Mount(pFSTool, pStorage);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  device(%s) is exception, system errno(%d), and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, errno, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    s32Ret = snprintf_s(aszTestFile, HI_STORAGE_PATH_LEN_MAX+16, HI_STORAGE_PATH_LEN_MAX+16-1, "%s/%s", pstFSTool->aszMountPath, STG_FSTOOL_TEST_PARTITION_FILE_NAME);
    if ( s32Ret < 0 )
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "aszTestFile snprintf_s failed!\n");
        return HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE;
    }
    fd = open(aszTestFile, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRWXU);
    if (0 > fd)
    {
        (HI_VOID)STG_FSTOOL_Umount(pFSTool, HI_FALSE);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open test file(%s) fail, system errno(%d), and ret:%x!\n", __FUNCTION__, __LINE__, aszTestFile, errno, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    s32Ret = memset_s(aszTestData, STG_FSTOOL_TEST_PARTITION_DATA_LEN, 0x00, STG_FSTOOL_TEST_PARTITION_DATA_LEN);
    if ( EOK != s32Ret )
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "aszTestData memset_s failed! s32Ret = 0x%x\n", s32Ret);
        s32Ret = HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE;
    }

    if (0 > write(fd, aszTestData, STG_FSTOOL_TEST_PARTITION_DATA_LEN))
    {
        if (errno != ENOSPC)
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  write test file(%s) fail, system errno(%d), and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, errno, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
            s32Ret = HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
        }
        else
        {
            //space is full not return erron
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_WARN, "%s  %d  write test file(%s) fail, system errno(%d) for space is full!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, errno);
        }
    }

    if (0 != close(fd))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  close file failure, system errno(%d), and ret:%x!\n", __FUNCTION__, __LINE__, errno, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
        s32Ret = HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    if (0 != remove(aszTestFile))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  device(%s) is exception, system errno(%d), and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, errno, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
        s32Ret = HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    (HI_VOID)STG_FSTOOL_Umount(pFSTool, HI_FALSE);
    return s32Ret;
}

HI_S32 STG_FSTOOL_Register(const HI_STORAGE_FS_TOOL_S* pstFSTool)
{
    HI_S32 s32Ret = EOK;
    STG_FSTOOL_NODE_S* pstTmpNode = pstFSTooList;
    STG_FSTOOL_NODE_S* pstCurNode = HI_NULL;

    while (pstTmpNode)
    {
        if (0 == strncmp(pstTmpNode->stTool.aszFSType, pstFSTool->aszFSType, HI_STORAGE_PATH_LEN_MAX))
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  tool of fs(%s) has registered , and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszFSType, HI_ERR_STORAGE_ILLEGAL_PARAM);
            return HI_ERR_STORAGE_ILLEGAL_PARAM;
        }

        pstTmpNode = pstTmpNode->pstNext;
    }

    pstCurNode = (STG_FSTOOL_NODE_S*)malloc(sizeof(STG_FSTOOL_NODE_S));
    if (HI_NULL == pstCurNode)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  malloc failed , and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstCurNode, sizeof(STG_FSTOOL_NODE_S), 0x00, sizeof(STG_FSTOOL_NODE_S));
    if ( EOK != s32Ret )
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    pstCurNode->stTool.pfnCheck = pstFSTool->pfnCheck;
    pstCurNode->stTool.pfnFormat = pstFSTool->pfnFormat;
    s32Ret = snprintf_s(pstCurNode->stTool.aszFSType, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pstFSTool->aszFSType);
    if ( s32Ret < 0 )
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    pstCurNode->pstNext = pstFSTooList;
    pstFSTooList = pstCurNode;
    return HI_SUCCESS;
}

HI_S32 STG_FSTOOL_Unregister(const HI_CHAR* pszFSType)
{
    STG_FSTOOL_NODE_S* pstTmpNode = pstFSTooList;
    STG_FSTOOL_NODE_S* pstPrevNode = HI_NULL;

    while (pstTmpNode)
    {
        if (0 == strncmp(pstTmpNode->stTool.aszFSType, pszFSType, HI_STORAGE_PATH_LEN_MAX))
        {
            break;
        }

        pstPrevNode = pstTmpNode;
        pstTmpNode = pstTmpNode->pstNext;
    }

    if (HI_NULL == pstTmpNode)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  no tool of fs(%s) , and ret:%x!\n", __FUNCTION__, __LINE__, pszFSType, HI_ERR_STORAGE_ILLEGAL_PARAM);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    if (HI_NULL == pstPrevNode)
    {
        pstFSTooList = pstFSTooList->pstNext;
    }
    else
    {
        pstPrevNode->pstNext = pstTmpNode->pstNext;
    }

    free(pstTmpNode);
    pstTmpNode = HI_NULL;
    return HI_SUCCESS;
}

HI_S32 STG_FSTOOL_Create(const STG_FSTOOL_CFG_S* pstCfg, HI_MW_PTR* ppFSTool)
{
    HI_S32 s32Ret = EOK;
    STG_FSTOOL_S* pstTool = (STG_FSTOOL_S*)malloc(sizeof(STG_FSTOOL_S));
    if (HI_NULL == pstTool)
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstTool, sizeof(STG_FSTOOL_S), 0x00, sizeof(STG_FSTOOL_S));
    if ( EOK != s32Ret )
    {
        free(pstTool);
        pstTool = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    pstTool->enState = STG_FS_STATE_IDEL;
    s32Ret = snprintf_s(pstTool->aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pstCfg->aszPartitionPath);
    if ( s32Ret < 0 )
    {
        free(pstTool);
        pstTool = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    s32Ret = snprintf_s(pstTool->aszMountPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pstCfg->aszMountPath);
    if ( s32Ret < 0 )
    {
        free(pstTool);
        pstTool = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    *ppFSTool = pstTool;
    return HI_SUCCESS;
}

HI_VOID STG_FSTOOL_Destroy(HI_MW_PTR pFSTool)
{
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    free(pstFSTool);
}

HI_VOID STG_FSTOOL_SetState(HI_MW_PTR pFSTool, STG_FS_STATE_E enState)
{
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    pstFSTool->enState = enState;
    return;
}

HI_VOID STG_FSTOOL_GetState(HI_MW_PTR pFSTool, STG_FS_STATE_E* penState)
{
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    *penState = pstFSTool->enState;
    return;
}

HI_S32 STG_FSTOOL_GetPartitionPath(HI_MW_PTR pFSTool, HI_CHAR* pszPartitionPath)
{
    HI_S32 s32Ret = 0;
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    s32Ret = snprintf_s(pszPartitionPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pstFSTool->aszPartitionPath);
    if ( s32Ret < 0 )
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 STG_FSTOOL_GetInfo(HI_MW_PTR pFSTool, HI_STORAGE_FS_INFO_S* pstInfo)
{
    struct statfs stFSbuf = {0};
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;

    if (HI_SUCCESS != statfs(pstFSTool->aszMountPath, &stFSbuf))
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    pstInfo->u64ClusterSize = (HI_U64)stFSbuf.f_bsize;
    pstInfo->u64TotalSize = (HI_U64)(stFSbuf.f_bsize * stFSbuf.f_blocks);
    pstInfo->u64AvailableSize = (HI_U64)(stFSbuf.f_bsize * stFSbuf.f_bavail);
    pstInfo->u64UsedSize = (HI_U64)(pstInfo->u64TotalSize - pstInfo->u64AvailableSize);
    return HI_SUCCESS;
}

HI_STORAGE_FSTOOL_ERR_E STG_FSTOOL_Check(HI_MW_PTR pFSTool, HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_FSTOOL_ERR_E enRet = HI_STORAGE_FSTOOL_SUCCESS;
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    STG_FSTOOL_NODE_S* pstToolNode = HI_NULL;
    HI_CHAR aszFSType[HI_STORAGE_PATH_LEN_MAX] = {0};

    s32Ret = STG_FSTOOL_DetectPartition(pstFSTool->aszPartitionPath);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  do fsck(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION);
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    s32Ret = STG_FSTOOL_GetFSType(pstFSTool->aszPartitionPath, aszFSType);
    if (HI_SUCCESS != s32Ret)
    {
        if (HI_ERR_STORAGE_SYSTEM_CALL_FAILURE == s32Ret)
        {
            enRet = HI_STORAGE_FSTOOL_ERR_READ_FAIL;
        }
        else
        {
            enRet = HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT;
        }
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  do fsck(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, enRet);
        return enRet;
    }

    if (HI_SUCCESS != STG_FSTOOL_GetNode(aszFSType, &pstToolNode))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  do fsck(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT);
        return HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT;
    }

    s32Ret = STG_FSTOOL_TestPartition(pFSTool, pStorage);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  test partition(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, s32Ret);
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }

    return pstToolNode->stTool.pfnCheck(pstFSTool->aszPartitionPath);
}

HI_S32 STG_FSTOOL_Format(HI_MW_PTR pFSTool, const HI_CHAR* pszFSType, HI_U64 u64ClusterSize)
{
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    STG_FSTOOL_NODE_S* pstToolNode = HI_NULL;

    if (HI_SUCCESS != STG_FSTOOL_GetNode(pszFSType, &pstToolNode))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  not support format(%s), and ret:%x!\n", __FUNCTION__, __LINE__, pszFSType, HI_ERR_STORAGE_ILLEGAL_PARAM);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    return pstToolNode->stTool.pfnFormat(pstFSTool->aszPartitionPath, u64ClusterSize);
}

/*pStorage is used to refresh proc*/
HI_S32 STG_FSTOOL_Mount(HI_MW_PTR pFSTool, HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    STG_FSTOOL_NODE_S* pstToolNode = HI_NULL;
    HI_CHAR aszFSType[HI_STORAGE_PATH_LEN_MAX] = {0};

    s32Ret = STG_FSTOOL_GetFSType(pstFSTool->aszPartitionPath, aszFSType);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  do mount(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    if (HI_SUCCESS != STG_FSTOOL_GetNode(aszFSType, &pstToolNode))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  do mount(%s) failed, and ret:%x!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT);
        return HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT;
    }

    STG_PROC_SetStartTime(pStorage, STG_PROC_DURATION_TYPE_MOUNT);
    if ( HI_SUCCESS != mount(pstFSTool->aszPartitionPath, pstFSTool->aszMountPath, aszFSType, MS_NOEXEC | MS_NOATIME | MS_NODIRATIME, 0))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  mount %s on %s failure, and ret:%x, errno(%d)!\n", __FUNCTION__, __LINE__, pstFSTool->aszPartitionPath, pstFSTool->aszMountPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    STG_PROC_RefDuration(pStorage, STG_PROC_DURATION_TYPE_MOUNT);

    return HI_SUCCESS;
}

HI_S32 STG_FSTOOL_Umount(HI_MW_PTR pFSTool, HI_BOOL bEnableLog)
{
    STG_FSTOOL_S* pstFSTool = (STG_FSTOOL_S*)pFSTool;
    if ( HI_SUCCESS != umount(pstFSTool->aszMountPath))
    {
        if (bEnableLog)
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  umount %s failure, and ret:%x, errno(%d)!\n", __FUNCTION__, __LINE__, pstFSTool->aszMountPath, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        }
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
