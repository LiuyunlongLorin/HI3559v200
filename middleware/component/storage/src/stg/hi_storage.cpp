#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <errno.h>

#include "hi_mw_type.h"
#include "stg_log.h"
#include "hi_storage.h"
#include "stg_common.h"
#include "stg_proc.h"
#include "stg_fstool.h"
#include "stg_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct tagSTG_INST_S
{
    pthread_mutex_t mLock;
    HI_BOOL bUsed;
    HI_MW_PTR pFSTool;
    HI_MW_PTR pDev;
}STG_INST_S;

static pthread_mutex_t s_mModuleLock = PTHREAD_MUTEX_INITIALIZER;
static STG_INST_S s_astInst[HI_STORAGE_INST_CNT_MAX] =
{
    {
        PTHREAD_MUTEX_INITIALIZER, HI_FALSE, HI_NULL, HI_NULL
    }
};

#define STG_CHECK_NULL_ERR(ptr)  \
    do{ \
        if (NULL == ptr) \
        { \
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  null ptr err\n", __FUNCTION__, __LINE__); \
            return HI_ERR_STORAGE_NULL_PTR; \
        } \
    }while(0)

#define STG_CHECK_INST_HANDLE(ptr,mutex)  \
    do{\
        if(NULL == ptr)\
        {\
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  null ptr err\n", __FUNCTION__, __LINE__);\
            (HI_VOID)pthread_mutex_unlock(&mutex);\
            return HI_ERR_STORAGE_NULL_PTR;\
        }\
        HI_U8 i = 0;\
        for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)\
        {\
            if (s_astInst[i].bUsed && (&s_astInst[i] == ptr)) \
            { \
                break; \
            } \
        }\
        if (HI_STORAGE_INST_CNT_MAX == i)\
        {\
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  invalid handle :%p\n", __FUNCTION__, __LINE__, ptr);\
            (HI_VOID)pthread_mutex_unlock(&mutex); \
            return HI_ERR_STORAGE_INVALID_HANDLE;\
        }\
    }while(0)

static HI_S32 STG_LockInst(STG_INST_S* pstInst)
{
    STG_THREAD_MUTEX_LOCK(&pstInst->mLock);
    return HI_SUCCESS;
}

static HI_VOID STG_UnlockInst(STG_INST_S* pstInst)
{
    (HI_VOID)pthread_mutex_unlock(&pstInst->mLock);
    return;
}

static STG_INST_S* STG_GetFreeInst()
{
    HI_U8 i = 0;
    for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)
    {
        if (!s_astInst[i].bUsed)
        {
            s_astInst[i].bUsed = HI_TRUE;
            return &s_astInst[i];
        }
    }

    return HI_NULL;
}

static HI_VOID STG_FreeInst(STG_INST_S* pstInst)
{
    pstInst->bUsed = HI_FALSE;
    pstInst->pFSTool = HI_NULL;
    pstInst->pDev = HI_NULL;
    return;
}

HI_S32 HI_STORAGE_RegisterFSTool(const HI_STORAGE_FS_TOOL_S* pstFSTool)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_CHECK_NULL_ERR(pstFSTool);
    STG_CHECK_NULL_ERR(pstFSTool->pfnCheck);
    STG_CHECK_NULL_ERR(pstFSTool->pfnFormat);

    if (0 == strlen(pstFSTool->aszFSType))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  len of aszFSType is invalid, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    s32Ret = STG_FSTOOL_Register(pstFSTool);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  len of aszFSType is invalid, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
    }
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_UnregisterFSTool(const HI_CHAR* pszFSType)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_CHECK_NULL_ERR(pszFSType);

    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    s32Ret = STG_FSTOOL_Unregister(pszFSType);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  FSType(%s) is not support, and ret:%x!\n",__FUNCTION__,__LINE__, pszFSType, HI_ERR_STORAGE_ILLEGAL_PARAM);
    }
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_Create(const HI_STORAGE_CFG_S *pstCfg, const HI_STORAGE_ON_STATE_CHANGE_PFN pfnOnStateChange, HI_MW_PTR* ppStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_INST_S* pstInst = HI_NULL;
    STG_FSTOOL_CFG_S stFSToolCfg = {{0}};
    STG_DEV_CFG_S stDevCfg = {0};
    HI_CHAR aszMntPath[PATH_MAX + 1] = {0};
    STG_CHECK_NULL_ERR(pstCfg);
    STG_CHECK_NULL_ERR(ppStorage);
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);

    if (0 == pstCfg->u8DevErrCountThr)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  param pstCfg->u8DevErrCountThr should be over 0, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    if (0 == strlen(pstCfg->aszPartitionPath))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  param pstCfg->aszPartitionPath length is 0, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    if(HI_NULL == realpath(pstCfg->aszMountPath, aszMntPath))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  param pstCfg->aszMountPath is illegal(system errno %d), and ret:%x!\n",__FUNCTION__,__LINE__, errno, HI_ERR_STORAGE_ILLEGAL_PARAM);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    pstInst = STG_GetFreeInst();
    if (HI_NULL == pstInst)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  has no free inst, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_INST_CNT_UPPER_LIMIT);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_INST_CNT_UPPER_LIMIT;
    }

    s32Ret = snprintf_s(stFSToolCfg.aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX-1, "%s", pstCfg->aszPartitionPath);
    if( s32Ret<0 )
    {
        STG_FreeInst(pstInst);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "snprintf_s stFSToolCfg.aszPartitionPath failed!\n");
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    s32Ret = snprintf_s(stFSToolCfg.aszMountPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX-1, "%s", pstCfg->aszMountPath);
    if( s32Ret<0 )
    {
        STG_FreeInst(pstInst);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "snprintf_s stFSToolCfg.aszPartitionPath failed!\n");
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_FSTOOL_Create(&stFSToolCfg, &pstInst->pFSTool);
    if (HI_SUCCESS != s32Ret)
    {
        STG_FreeInst(pstInst);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  FSTool create failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    stDevCfg.u8DevPortNo = pstCfg->u8DevPortNo;
    stDevCfg.u8DevErrCountThr = pstCfg->u8DevErrCountThr;
    s32Ret = snprintf_s(stDevCfg.aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX-1, "%s", pstCfg->aszPartitionPath);
    if( s32Ret<0 )
    {
        STG_FreeInst(pstInst);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "snprintf_s stFSToolCfg.aszPartitionPath failed!\n");
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    stDevCfg.pStorage = (HI_MW_PTR)pstInst;
    stDevCfg.pFSTool = pstInst->pFSTool;
    stDevCfg.pfnOnStateChange = pfnOnStateChange;
    s32Ret = STG_DEV_Create(&stDevCfg, &pstInst->pDev);
    if (HI_SUCCESS != s32Ret)
    {
        STG_FSTOOL_Destroy(pstInst->pFSTool);
        STG_FreeInst(pstInst);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev create failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    *ppStorage = pstInst;

    STG_PROC_EnableInst((HI_MW_PTR)pstInst, pstCfg->aszPartitionPath);

    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return HI_SUCCESS;
}

HI_S32 HI_STORAGE_Destroy(HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_FS_STATE_E enFSState = STG_FS_STATE_IDEL;
    STG_INST_S* pstInst = HI_NULL;
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);
    pstInst = (STG_INST_S*)pStorage;
    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    STG_DEV_Destroy(pstInst->pDev);

    STG_FSTOOL_GetState(pstInst->pFSTool, &enFSState);
    if (STG_FS_STATE_MOUNTED == enFSState)
    {
        (HI_VOID)STG_FSTOOL_Umount(pstInst->pFSTool, HI_TRUE);
    }
    STG_FSTOOL_Destroy(pstInst->pFSTool);

    STG_FreeInst(pstInst);
    STG_PROC_DisableInst((HI_MW_PTR)pstInst);
    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return HI_SUCCESS;
}

HI_S32 HI_STORAGE_GetState(HI_MW_PTR pStorage, HI_STORAGE_STATE_E *penState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_INST_S* pstInst = HI_NULL;
    STG_DEV_STATE_E enDevState = STG_DEV_STATE_IDEL;
    STG_FS_STATE_E enFSState = STG_FS_STATE_IDEL;
    STG_CHECK_NULL_ERR(penState);
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);
    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if (STG_DEV_IsPaused(pstInst->pDev))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage is paused, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_PAUSED);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_PAUSED;
    }

    s32Ret = STG_DEV_GetState(pstInst->pDev, &enDevState);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev state failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if (STG_DEV_STATE_IDEL == enDevState)
    {
        *penState = HI_STORAGE_STATE_IDEL;
    }
    else if (STG_DEV_STATE_UNPLUGGED == enDevState)
    {
        *penState = HI_STORAGE_STATE_DEV_UNPLUGGED;
    }
    else if (STG_DEV_STATE_CONNECTING == enDevState)
    {
        *penState = HI_STORAGE_STATE_DEV_CONNECTING;
    }
    else
    {
        STG_FSTOOL_GetState(pstInst->pFSTool, &enFSState);
        if (STG_FS_STATE_CHECKING == enFSState)
        {
            *penState = HI_STORAGE_STATE_FS_CHECKING;
        }
        else if (STG_FS_STATE_CHECK_FAILED == enFSState)
        {
            *penState = HI_STORAGE_STATE_FS_CHECK_FAILED;
        }
        else if (STG_FS_STATE_EXCEPTION == enFSState)
        {
            *penState = HI_STORAGE_STATE_FS_EXCEPTION;
        }
        else if (STG_FS_STATE_MOUNTED == enFSState)
        {
            *penState = HI_STORAGE_STATE_MOUNTED;
        }
        else
        {
            *penState = HI_STORAGE_STATE_MOUNT_FAILED;
        }
    }

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_GetFSInfo(HI_MW_PTR pStorage, HI_STORAGE_FS_INFO_S *pstFSInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_FS_STATE_E enState = STG_FS_STATE_IDEL;
    STG_INST_S* pstInst = HI_NULL;
    STG_CHECK_NULL_ERR(pstFSInfo);
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);
    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if (STG_DEV_IsPaused(pstInst->pDev))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage is paused, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_PAUSED);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_PAUSED;
    }

    STG_FSTOOL_GetState(pstInst->pFSTool, &enState);
    if (STG_FS_STATE_MOUNTED != enState)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fs not mounted, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_FS_NOT_MOUNTED);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_FS_NOT_MOUNTED;
    }

    s32Ret = STG_FSTOOL_GetInfo(pstInst->pFSTool, pstFSInfo);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get fs info failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
    }

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_GetDevInfo(HI_MW_PTR pStorage, HI_STORAGE_DEV_INFO_S* pstDevInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_DEV_STATE_E enState = STG_DEV_STATE_IDEL;
    STG_INST_S* pstInst = HI_NULL;
    STG_CHECK_NULL_ERR(pstDevInfo);
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);

    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if (STG_DEV_IsPaused(pstInst->pDev))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage is paused, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_PAUSED);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_PAUSED;
    }

    s32Ret = STG_DEV_GetState(pstInst->pDev, &enState);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev state failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if ((STG_DEV_STATE_UNPLUGGED == enState)
        ||(STG_DEV_STATE_CONNECTING == enState))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev diconnect, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_DEV_DISCONNECT);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_DEV_DISCONNECT;
    }
    s32Ret = STG_DEV_GetInfo(pstInst->pDev, pstDevInfo);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev info failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
    }

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_Format(HI_MW_PTR pStorage, const HI_CHAR* pszFSType, HI_U64 u64ClusterSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_DEV_STATE_E enDevState = STG_DEV_STATE_IDEL;
    STG_FS_STATE_E enFSState = STG_FS_STATE_IDEL;
    STG_INST_S* pstInst = HI_NULL;
    STG_CHECK_NULL_ERR(pszFSType);
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);

    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if (STG_DEV_IsPaused(pstInst->pDev))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage is paused, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_PAUSED);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_PAUSED;
    }

    s32Ret = STG_DEV_GetState(pstInst->pDev, &enDevState);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev state failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    if ((STG_DEV_STATE_UNPLUGGED == enDevState)
        ||(STG_DEV_STATE_CONNECTING == enDevState))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev diconnect, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_DEV_DISCONNECT);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_DEV_DISCONNECT;
    }
    STG_FSTOOL_GetState(pstInst->pFSTool, &enFSState);
    if ((STG_FS_STATE_CHECKING == enFSState) || (STG_FS_STATE_IDEL == enFSState))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fs is checking now, and ret:%x!\n",__FUNCTION__,__LINE__, HI_ERR_STORAGE_FS_CHECKING);
        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return HI_ERR_STORAGE_FS_CHECKING;
    }

    if (STG_FS_STATE_MOUNTED == enFSState)
    {
        s32Ret = STG_FSTOOL_Umount(pstInst->pFSTool, HI_TRUE);
        if (HI_SUCCESS != s32Ret)
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fs umout failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
            STG_UnlockInst(pstInst);
            (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
            return s32Ret;
        }
    }
    else
    {
        (HI_VOID)STG_FSTOOL_Umount(pstInst->pFSTool, HI_FALSE);//fault tolerant for fs has been mounted by user but not via storage
    }

    STG_PROC_SetStartTime((HI_MW_PTR)pstInst, STG_PROC_DURATION_TYPE_FORMAT);
    s32Ret = STG_FSTOOL_Format(pstInst->pFSTool, pszFSType,u64ClusterSize);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fs format failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        if (STG_FS_STATE_MOUNTED == enFSState)
        {
            (HI_VOID)STG_FSTOOL_Mount(pstInst->pFSTool, pStorage);
        }

        STG_UnlockInst(pstInst);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }
    STG_PROC_RefDuration((HI_MW_PTR)pstInst, STG_PROC_DURATION_TYPE_FORMAT);

    s32Ret = STG_FSTOOL_Mount(pstInst->pFSTool, pStorage);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fs mount failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        STG_FSTOOL_SetState(pstInst->pFSTool, STG_FS_STATE_MOUNT_FAILED);
    }
    else
    {
        STG_FSTOOL_SetState(pstInst->pFSTool, STG_FS_STATE_MOUNTED);
    }

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_Pause(HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_INST_S* pstInst = HI_NULL;
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);
    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    s32Ret = STG_DEV_PauseMonitor(pstInst->pDev);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage pause failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
    }

    STG_PROC_RefRunningState((HI_MW_PTR)pstInst, HI_FALSE);

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

HI_S32 HI_STORAGE_Resume(HI_MW_PTR pStorage)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_INST_S* pstInst = HI_NULL;
    STG_THREAD_MUTEX_LOCK(&s_mModuleLock);
    STG_CHECK_INST_HANDLE(pStorage,s_mModuleLock);
    pstInst = (STG_INST_S*)pStorage;

    s32Ret = STG_LockInst(pstInst);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  lock inst failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
        return s32Ret;
    }

    s32Ret = STG_DEV_ResumeMonitor(pstInst->pDev);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  storage pause failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
    }

    STG_PROC_RefRunningState((HI_MW_PTR)pstInst, HI_TRUE);

    STG_UnlockInst(pstInst);
    (HI_VOID)pthread_mutex_unlock(&s_mModuleLock);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
