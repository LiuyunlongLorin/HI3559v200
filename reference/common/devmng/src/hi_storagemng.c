/**
 * @file      hi_storagemng.c
 * @brief     product parameter module interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "hi_storagemng.h"
#include "hi_eventhub.h"
#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/** maximum storagemng instance count */
#define STORAGEMNG_INSTANCE_MAX_CNT   (1)

/** error count threshlod */
#define STORAGEMNG_THRESHOLD_ERRCOUNT (1)

/** sector size, Bytes */
#define STORAGEMNG_SECTOR_SIZE        (512)

/** fragement degree,the more vlaue mean fragement degree less*/
#define STORAGEMNG_FRAGMENT_THR   (200)

/** which mci used for storage*/
#define STORAGEMNG_DEVPORT_NUM   (0)

/** cluster size,unit is byte*/
#define STORAGEMNG_CLUSTERSIZE (64*1024)

/** fs type */
typedef enum tagSTORAGEMNG_FSTYPE_E
{
    STORAGEMNG_FSTYPE_VFAT = 0,
    STORAGEMNG_FSTYPE_BUTT
} STORAGEMNG_FSTYPE_E;

/** StorageMng Context */
typedef struct tagSTORAGEMNG_CONTEXT_S
{
    HI_BOOL bUsed;
    HI_MW_PTR pStorageObj;
    HI_STORAGEMNG_CFG_S stCfg;
    HI_STORAGEMNG_CALLBACK_S stCallback;
    STORAGEMNG_FSTYPE_E enFsType;
    HI_FSTOOL_FORMAT_MODE_E enFormatMode;
} STORAGEMNG_CONTEXT_S;
static STORAGEMNG_CONTEXT_S s_stSTORAGEMNGCtx[STORAGEMNG_INSTANCE_MAX_CNT];


static HI_S32 STORAGEMNG_GetCtxByMntPath(const HI_CHAR* pszMntPath, STORAGEMNG_CONTEXT_S** ppstCtx)
{
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < STORAGEMNG_INSTANCE_MAX_CNT; ++s32Idx)
    {
        if (s_stSTORAGEMNGCtx[s32Idx].bUsed
            && (0 == strncmp(s_stSTORAGEMNGCtx[s32Idx].stCfg.szMntPath, pszMntPath, HI_APPCOMM_MAX_PATH_LEN)))
        {
            *ppstCtx = &s_stSTORAGEMNGCtx[s32Idx];
        }
    }
    if(*ppstCtx == NULL)
    {
        MLOGE("get context fail by MntPath[%s]\n",pszMntPath);
        return HI_STORAGEMNG_ENOTINIT;
    }
    return HI_SUCCESS;
}

static HI_S32 STORAGEMNG_GetCtxByDevPath(const HI_CHAR* pszDevPath, STORAGEMNG_CONTEXT_S** ppstCtx)
{
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < STORAGEMNG_INSTANCE_MAX_CNT; ++s32Idx)
    {
        if (s_stSTORAGEMNGCtx[s32Idx].bUsed
            && (0 == strncmp(s_stSTORAGEMNGCtx[s32Idx].stCfg.szDevPath, pszDevPath, HI_APPCOMM_MAX_PATH_LEN)))
        {
            *ppstCtx = &s_stSTORAGEMNGCtx[s32Idx];
        }
    }
    if(*ppstCtx == NULL)
    {
        MLOGE("get context fail by DevPath[%s]\n",pszDevPath);
        return HI_STORAGEMNG_ENOTINIT;
    }
    return HI_SUCCESS;
}

static HI_STORAGE_FSTOOL_ERR_E STORAGEMNG_FsCheck(const HI_CHAR *pszDevPath)
{
    HI_APPCOMM_CHECK_POINTER(pszDevPath, HI_ERR_FSTOOL_ILLEGAL_PARAM);
    HI_FSTOOL_CHECK_CFG_S stCfg;
    stCfg.enMode = HI_FSTOOL_CHECK_MODE_DIRTY_MAIN_FAT;
    stCfg.u32FragmentThr = STORAGEMNG_FRAGMENT_THR;
    return HI_FSTOOL_Check(pszDevPath, &stCfg);
}

#if !defined(CFG_EMMC_FLASH)
static HI_S32 STORAGEMNG_CheckDeviceValid(const HI_CHAR* aszDevice)
{
    if (NULL == aszDevice)
    {
        return HI_FAILURE;
    }
    if(0 == access(aszDevice, F_OK))
    {
        return HI_SUCCESS;
    }
    else
    {
        return HI_FAILURE;
    }
}

static HI_VOID STORAGEMNG_CheckDeviceProc(const HI_CHAR *pszDevPath)
{
    HI_S32  s32Ret,u32PartitionNumber;
    HI_CHAR szShellCmd[128] = {0};
    s32Ret = STORAGEMNG_CheckDeviceValid(pszDevPath);
    if(s32Ret!=HI_SUCCESS)
    {
        MLOGD("--> Need create paration for format !!!\n");
        s32Ret = HI_system("echo -e \"d\n1\nd\n2\nd\n3\nd\nn\np\n1\n\n\n\n\nt\nb\nw\n\" | fdisk /dev/mmcblk0");
        if (0 != s32Ret)
        {
            MLOGE("HI_system() failed\n");
        }
        sync();
    }
    else
    {
        MLOGD("--> start check fs type!\n");
        snprintf(szShellCmd, sizeof(szShellCmd), "fdisk -l | grep %s |grep FAT32", pszDevPath);
        s32Ret = HI_system(szShellCmd);
        if (0 != s32Ret)
        {
            MLOGD("--> Reset paration type!\n");
            (HI_VOID)sscanf(pszDevPath, "%*[^1-9]%i", &u32PartitionNumber);
            MLOGD("\npszDevPath is %s, current number is %d\n",pszDevPath,u32PartitionNumber);
            snprintf(szShellCmd, sizeof(szShellCmd), "echo -e \"t\nb\n%d\nb\nw\n\" | fdisk /dev/mmcblk0", u32PartitionNumber);
            s32Ret = HI_system(szShellCmd);
            if (0 != s32Ret)
            {
                MLOGE("HI_system() failed\n");
            }
            sync();
        }
        MLOGD("--> start check fs type end!\n");
    }
}
#endif

static HI_STORAGE_FSTOOL_ERR_E STORAGEMNG_FsFormat(const HI_CHAR *pszDevPath, HI_U64 u32ClusterSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    HI_APPCOMM_CHECK_POINTER(pszDevPath, HI_ERR_FSTOOL_ILLEGAL_PARAM);
    s32Ret = STORAGEMNG_GetCtxByDevPath(pszDevPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);
#if !defined(CFG_EMMC_FLASH)
    STORAGEMNG_CheckDeviceProc(pszDevPath);
#endif

    HI_FSTOOL_FORMAT_CFG_S stFormatCfg;
    stFormatCfg.enMode = pstCtx->enFormatMode;
    stFormatCfg.u32ClusterSize = u32ClusterSize;
#if defined(CFG_EMMC_FLASH)
    stFormatCfg.enable4KAlignCheck = 0;
#else
    stFormatCfg.enable4KAlignCheck = 1;
#endif
    return HI_FSTOOL_Format(pszDevPath, &stFormatCfg);
}

static HI_VOID STORAGEMNG_OnStateChange(HI_MW_PTR pStorageObj, const HI_CHAR* pszPartitionPath,
                    HI_STORAGE_STATE_E enState, HI_S32 s32ErrCode)
{
    /* Check Instance */
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < STORAGEMNG_INSTANCE_MAX_CNT; ++s32Idx)
    {
        if (s_stSTORAGEMNGCtx[s32Idx].bUsed && pStorageObj == s_stSTORAGEMNGCtx[s32Idx].pStorageObj)
        {
            pstCtx = &s_stSTORAGEMNGCtx[s32Idx];
        }
    }
    if (!pstCtx)
    {
        MLOGW("Invalid storage obj\n");
        return;
    }

    /* Publish Event */
    HI_EVENT_S stEvent;
    switch(enState)
    {
        case HI_STORAGE_STATE_DEV_UNPLUGGED:
            stEvent.EventID = HI_EVENT_STORAGEMNG_DEV_UNPLUGED;
            break;
        case HI_STORAGE_STATE_DEV_CONNECTING:
            stEvent.EventID = HI_EVENT_STORAGEMNG_DEV_CONNECTING;
            break;
        case HI_STORAGE_STATE_DEV_ERROR:
            stEvent.EventID = HI_EVENT_STORAGEMNG_DEV_ERROR;
            break;
        case HI_STORAGE_STATE_FS_CHECKING:
            stEvent.EventID = HI_EVENT_STORAGEMNG_FS_CHECKING;
            break;
        case HI_STORAGE_STATE_FS_CHECK_FAILED:
            stEvent.EventID = HI_EVENT_STORAGEMNG_FS_CHECK_FAILED;
            break;
        case HI_STORAGE_STATE_FS_EXCEPTION:
            stEvent.EventID = HI_EVENT_STORAGEMNG_FS_EXCEPTION;
            break;
        case HI_STORAGE_STATE_MOUNTED:
            stEvent.EventID = HI_EVENT_STORAGEMNG_MOUNTED;
            break;
        case HI_STORAGE_STATE_MOUNT_FAILED:
            stEvent.EventID = HI_EVENT_STORAGEMNG_MOUNT_FAILED;
            break;
        case HI_STORAGE_STATE_IDEL:
            MLOGD("Idle State, igore\n");
            return;
        default:
            MLOGW("Invalid State[%d]\n", enState);
            return;
    }

    stEvent.arg1 = s32ErrCode;

    snprintf(stEvent.aszPayload, HI_APPCOMM_MAX_PATH_LEN, "%s", pszPartitionPath);
    MLOGI("DevPath[%s] State[%d]\n", pszPartitionPath, enState);
    HI_EVTHUB_Publish(&stEvent);
    return;
}

HI_S32 HI_STORAGEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_DEV_UNPLUGED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_DEV_CONNECTING);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_DEV_ERROR);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_FS_CHECKING);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_FS_CHECK_FAILED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_FS_EXCEPTION);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_MOUNTED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STORAGEMNG_MOUNT_FAILED);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_STORAGEMNG_EREGISTER_EVENT);
    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_Create(const HI_STORAGEMNG_CFG_S *pstCfg,const HI_STORAGEMNG_CALLBACK_S *pstCallback)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_STORAGEMNG_EINVAL);

    /* Check Instance */
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    HI_S32 s32Idx;
    for (s32Idx = 0; s32Idx < STORAGEMNG_INSTANCE_MAX_CNT; ++s32Idx)
    {
        if (s_stSTORAGEMNGCtx[s32Idx].bUsed)
        {
            if (0 == strncmp(s_stSTORAGEMNGCtx[s32Idx].stCfg.szDevPath, pstCfg->szDevPath, HI_APPCOMM_MAX_PATH_LEN))
            {
                MLOGW("[%s] mng has already create\n", pstCfg->szDevPath);
                return HI_STORAGEMNG_EINITIALIZED;
            }
        }
        else
        {
            pstCtx = &s_stSTORAGEMNGCtx[s32Idx];
            MLOGI("Idx = %d,szDevPath[%s],szMntPath[%s]\n", s32Idx,pstCfg->szDevPath,pstCfg->szMntPath);
        }
    }
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(pstCtx, HI_STORAGEMNG_EMAXINSTANCE, "beyond maximum instance");
    HI_S32 s32Ret;
    /* register fstool */
    HI_STORAGE_FS_TOOL_S stFsTool;
    stFsTool.pfnCheck = STORAGEMNG_FsCheck;
    stFsTool.pfnFormat = STORAGEMNG_FsFormat;
    snprintf(stFsTool.aszFSType, HI_STORAGE_PATH_LEN_MAX, "vfat");
    s32Ret = HI_STORAGE_RegisterFSTool(&stFsTool);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);

    /* Create Storage Instance */
    HI_STORAGE_CFG_S stStorageCfg;
    stStorageCfg.u8DevPortNo = STORAGEMNG_DEVPORT_NUM;
    stStorageCfg.u8DevErrCountThr = STORAGEMNG_THRESHOLD_ERRCOUNT;
    snprintf(stStorageCfg.aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, "%s", pstCfg->szDevPath);
    snprintf(stStorageCfg.aszMountPath, HI_STORAGE_PATH_LEN_MAX, "%s", pstCfg->szMntPath);
    pstCtx->bUsed = HI_TRUE;
    s32Ret = HI_STORAGE_Create(&stStorageCfg, STORAGEMNG_OnStateChange, &pstCtx->pStorageObj);
    if (s32Ret != HI_SUCCESS)
    {
        pstCtx->bUsed = HI_FALSE;
        return HI_STORAGEMNG_ESTORAGE;
    }
    /* Storagemng Context Information */
    memcpy(&pstCtx->stCfg, pstCfg, sizeof(HI_STORAGEMNG_CFG_S));
    memcpy(&pstCtx->stCallback, pstCallback, sizeof(HI_STORAGEMNG_CALLBACK_S));
    pstCtx->enFsType = STORAGEMNG_FSTYPE_VFAT;
    HI_FSTOOL_SetLogLevel(HI_FSTOOL_LOG_LEVEL_WARN);
    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_Destroy(const HI_CHAR* pszMntPath)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);

    /* Get Ctx Instance */
    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    /* preproc before Destroy Storage Instance */
    if(NULL != pstCtx->stCallback.pfnFormatPreProc)
    {
        s32Ret = pstCtx->stCallback.pfnFormatPreProc(pszMntPath);
        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);
    }
    /* Destroy Storage Instance */
    s32Ret = HI_STORAGE_Destroy(pstCtx->pStorageObj);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);

    /* Unregister FSTool*/
    HI_CHAR aszFSType[HI_STORAGE_PATH_LEN_MAX]; /**<such as vfat/exfat/ntfs/nfs */
    snprintf(aszFSType, HI_STORAGE_PATH_LEN_MAX, "vfat");
    s32Ret = HI_STORAGE_UnregisterFSTool(aszFSType);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);

    /* Update Ctx Informatin */
    pstCtx->bUsed = HI_FALSE;
    pstCtx->pStorageObj = NULL;
    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_GetState(const HI_CHAR* pszMntPath, HI_STORAGE_STATE_E *penState)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(penState, HI_STORAGEMNG_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    s32Ret = HI_STORAGE_GetState(pstCtx->pStorageObj, penState);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);

    if((HI_STORAGE_STATE_MOUNTED != *penState) && (HI_STORAGE_STATE_DEV_UNPLUGGED != *penState))
    {
        MLOGW("State[%d]\n", *penState);
    }

    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_GetFSInfo(const HI_CHAR* pszMntPath, HI_STORAGE_FS_INFO_S *pstFSInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstFSInfo, HI_STORAGEMNG_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    s32Ret = HI_STORAGE_GetFSInfo(pstCtx->pStorageObj, pstFSInfo);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);
    //MLOGD("ClusterSize[%llu] TotalSize[%llu] AvailableSize[%llu] UsedSize[%llu]\n",
    //    pstFSInfo->u64ClusterSize, pstFSInfo->u64TotalSzie,
    //    pstFSInfo->u64AvailableSize, pstFSInfo->u64UsedSize);

    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_GetDevInfo(const HI_CHAR* pszMntPath, HI_STORAGE_DEV_INFO_S* pstDevInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDevInfo, HI_STORAGEMNG_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    s32Ret = HI_STORAGE_GetDevInfo(pstCtx->pStorageObj, pstDevInfo);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);
#ifdef __HuaweiLite__
    pstDevInfo->enTranSpeed = HI_STORAGE_TRANSMISSION_SPEED_10_30M;
#endif
    MLOGD("CID[%s] WorkClock[%s] WorkMode[%s] TranSpeed[%d]\n",
        pstDevInfo->aszCID, pstDevInfo->aszWorkClock, pstDevInfo->aszWorkMode, pstDevInfo->enTranSpeed);

    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_Format(const HI_CHAR *pszMntPath, HI_FSTOOL_FORMAT_MODE_E enMode)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszFsType = NULL;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    HI_APPCOMM_CHECK_EXPR((enMode < HI_FSTOOL_FORMAT_MODE_BUTT) &&
                          (enMode >= HI_FSTOOL_FORMAT_MODE_ADVANCED), HI_STORAGEMNG_EINVAL);
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);
    switch (pstCtx->enFsType) {
        case STORAGEMNG_FSTYPE_VFAT:
            pszFsType = "vfat";
            break;
        default:
            MLOGE("enFsType(%d),invalid\n", pstCtx->enFsType);
            return HI_STORAGEMNG_EINTER;
    }
    if (NULL != pstCtx->stCallback.pfnFormatPreProc) {
        s32Ret = pstCtx->stCallback.pfnFormatPreProc(pszMntPath);
        HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);
    }
    pstCtx->enFormatMode = enMode;
    s32Ret = HI_STORAGE_Format(pstCtx->pStorageObj, pszFsType, STORAGEMNG_CLUSTERSIZE);
    if (HI_SUCCESS != s32Ret) {
        HI_CHAR cmd[HI_APPCOMM_COMM_STR_LEN];
        snprintf(cmd, HI_APPCOMM_COMM_STR_LEN, "fuser -m %s", pszMntPath);
        HI_system(cmd);
        HI_system("ps");
        HI_system("lsof");
        return HI_STORAGEMNG_ESTORAGE;
    }
    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_Pause(const HI_CHAR* pszMntPath)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    s32Ret = HI_STORAGE_Pause(pstCtx->pStorageObj);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);
    return HI_SUCCESS;
}

HI_S32 HI_STORAGEMNG_Resume(const HI_CHAR* pszMntPath)
{
    HI_APPCOMM_CHECK_POINTER(pszMntPath, HI_STORAGEMNG_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    STORAGEMNG_CONTEXT_S *pstCtx = NULL;
    s32Ret = STORAGEMNG_GetCtxByMntPath(pszMntPath, &pstCtx);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, s32Ret);

    s32Ret = HI_STORAGE_Resume(pstCtx->pStorageObj);
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_STORAGEMNG_ESTORAGE);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

