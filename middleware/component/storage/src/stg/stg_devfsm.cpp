#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include "hi_mw_type.h"
#include "stg_fsm.h"
#include "stg_log.h"
#include "hi_storage.h"
#include "stg_common.h"
#include "stg_proc.h"
#include "stg_dev.h"
#include "stg_devfsm.h"
#include "stg_fstool.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STG_DEVFSM_MSG_QUEUE_SIZE_MAX     (1)
#define STG_DEVFSM_MSG_PAYLOAD_LEN_MAX    (64)
#define STG_DEVFSM_STATE_STR_IDEL         "idel"
#define STG_DEVFSM_NAME "stg_fsm"

typedef struct tagSTG_DEVFSM_S
{
    HI_MW_PTR hFSM;
    HI_MW_PTR pFSTool;
    HI_MW_PTR pStorage;
    STG_DEV_STATE_E enCurState;
    HI_STORAGE_ON_STATE_CHANGE_PFN pfnOnStateChange;
    STG_DEVFSM_EVENT_CALLBACK pfnEventCB;
}STG_DEVFSM_S;

static HI_VOID STG_DEVFSM_PreHandle(STG_DEVFSM_S* pstDevFSM)
{
    STG_FS_STATE_E enState = STG_FS_STATE_IDEL;
    STG_FSTOOL_GetState(pstDevFSM->pFSTool, &enState);
    if (STG_FS_STATE_MOUNTED == enState)
    {
        if (HI_SUCCESS != STG_FSTOOL_Umount(pstDevFSM->pFSTool, HI_TRUE))
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  umount failed!\n",__FUNCTION__,__LINE__);
        }
    }
    return;
}

static HI_VOID STG_DEVFSM_HandleUnplugged(STG_DEVFSM_S* pstDevFSM, const HI_CHAR* pszPartitionPath)
{
    STG_DEVFSM_PreHandle(pstDevFSM);
    STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_IDEL);

    STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_DEV_UNPLUGGED);

    pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_DEV_UNPLUGGED, 0);
    return;
}

static HI_VOID STG_DEVFSM_HandleConnecting(STG_DEVFSM_S* pstDevFSM, const HI_CHAR* pszPartitionPath)
{
    STG_DEVFSM_PreHandle(pstDevFSM);
    STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_IDEL);

    STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_DEV_CONNECTING);

    pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_DEV_CONNECTING, 0);
    return;
}

static HI_VOID STG_DEVFSM_HandleConnected(STG_DEVFSM_S* pstDevFSM, const HI_CHAR* pszPartitionPath)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_FSTOOL_ERR_E enFSToolErr = HI_STORAGE_FSTOOL_ERR_ERR_BUTT;

    STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_CHECKING);
    pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_FS_CHECKING, HI_SUCCESS);

    STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_FS_CHECKING);
    STG_PROC_SetStartTime(pstDevFSM->pStorage, STG_PROC_DURATION_TYPE_FSCHECK);

    enFSToolErr = STG_FSTOOL_Check(pstDevFSM->pFSTool, pstDevFSM->pStorage);

    STG_PROC_RefDuration(pstDevFSM->pStorage, STG_PROC_DURATION_TYPE_FSCHECK);

    if ((HI_STORAGE_FSTOOL_SUCCESS == enFSToolErr)
        ||(HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY == enFSToolErr))
    {
        //do nothing
    }
    else if (HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION == enFSToolErr)
    {
        STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_FS_EXCEPTION);

        STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_EXCEPTION);
        pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_FS_EXCEPTION, enFSToolErr);
        return ;
    }
    else if (HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER == enFSToolErr)
    {
        STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_FS_EXCEPTION);
        STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_EXCEPTION);
        pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_FS_EXCEPTION, enFSToolErr);
        return ;
    }
    else
    {
        STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_FS_CHECK_FAILED);

        STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_CHECK_FAILED);
        pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_FS_CHECK_FAILED, enFSToolErr);
        return ;
    }

    STG_PROC_RefFragment(pstDevFSM->pStorage, enFSToolErr);

    (HI_VOID)STG_FSTOOL_Umount(pstDevFSM->pFSTool, HI_FALSE);//fault tolerant for fs has been mounted by user but not via storage

    s32Ret = STG_FSTOOL_Mount(pstDevFSM->pFSTool, pstDevFSM->pStorage);
    if (HI_SUCCESS != s32Ret)
    {
        STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_MOUNT_FAILED);

        STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_MOUNT_FAILED);
        pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_MOUNT_FAILED, s32Ret);
    }
    else
    {
        STG_PROC_RefCurState(pstDevFSM->pStorage, HI_STORAGE_STATE_MOUNTED);

        STG_FSTOOL_SetState(pstDevFSM->pFSTool, STG_FS_STATE_MOUNTED);
        pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_MOUNTED, enFSToolErr);
    }
    return;
}

static HI_VOID STG_DEVFSM_HandleError(STG_DEVFSM_S* pstDevFSM, const HI_CHAR* pszPartitionPath,HI_U32 u32Arg)
{
    pstDevFSM->pfnOnStateChange(pstDevFSM->pStorage, pszPartitionPath, HI_STORAGE_STATE_DEV_ERROR, u32Arg);
    return;
}

static HI_S32 STG_DEVFSM_HandleEvent(HI_VOID* pPriv, const STG_MSG_INFO_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX] = {0};
    STG_DEVFSM_S* pstDevFSM = (STG_DEVFSM_S*)pPriv;
    s32Ret = STG_FSTOOL_GetPartitionPath(pstDevFSM->pFSTool, aszPartitionPath);
    if( HI_SUCCESS!=s32Ret )
    {
        return s32Ret;
    }

    switch (pstMsg->s32What)
    {
        case STG_DEV_STATE_UNPLUGGED :
            STG_DEVFSM_HandleUnplugged(pstDevFSM, aszPartitionPath);
            break;
        case STG_DEV_STATE_CONNECTING :
            STG_DEVFSM_HandleConnecting(pstDevFSM, aszPartitionPath);
            break;
        case STG_DEV_STATE_CONNECTED :
            //connect state need check u32Arg1,u32Arg1 represent erron cnt changed.
            if(0 == pstMsg->u32Arg1)
            {
                STG_DEVFSM_HandleConnected(pstDevFSM, aszPartitionPath);
            }
            else
            {
                STG_DEVFSM_HandleError(pstDevFSM, aszPartitionPath,pstMsg->u32Arg1);
            }
            break;
        case STG_DEV_STATE_IDEL :
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 STG_DEVFSM_StateEnter(__attribute__((unused))HI_VOID* pPriv, const HI_CHAR* pstStatename)
{
    STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_INFO, "%s  %d  enter state(%s)!\n",__FUNCTION__,__LINE__,pstStatename);
    return HI_SUCCESS;
}

static HI_S32 STG_DEVFSM_StateExit(__attribute__((unused))HI_VOID* pPriv, const HI_CHAR* pstStatename)
{
    STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_INFO, "%s  %d  exit state(%s)!\n",__FUNCTION__,__LINE__,pstStatename);
    return HI_SUCCESS;
}

static HI_S32 STG_DEVFSM_Configure(HI_MW_PTR hFSM, HI_VOID* pPriv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_FSM_STATE_S stIdel = {STG_DEVFSM_STATE_STR_IDEL, STG_DEVFSM_HandleEvent, STG_DEVFSM_StateEnter, STG_DEVFSM_StateExit, pPriv};

    if (HI_SUCCESS != STG_FSM_AddState(hFSM, &stIdel))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fsm add state failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_FSM_SetInitState(hFSM, &stIdel);
    if(s32Ret != HI_SUCCESS)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fsm set init state failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 STG_DEVFSM_Create(const STG_DEVFSM_CFG_S* pstCfg, HI_MW_PTR* ppDevFsm)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszFsmName[HI_STORAGE_PATH_LEN_MAX] = {0};

    STG_DEVFSM_S* pstDevFSM = (STG_DEVFSM_S*)malloc(sizeof(STG_DEVFSM_S));
    if (HI_NULL == pstDevFSM)
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstDevFSM, sizeof(STG_DEVFSM_S), 0x00, sizeof(STG_DEVFSM_S));
    if( EOK!=s32Ret )
    {
        free(pstDevFSM);
        pstDevFSM = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    pstDevFSM->pFSTool = pstCfg->pFSTool;
    pstDevFSM->pStorage = pstCfg->pStorage;
    pstDevFSM->enCurState = STG_DEV_STATE_IDEL;
    pstDevFSM->pfnOnStateChange = pstCfg->pfnOnStateChange;

    s32Ret = snprintf_s(aszFsmName, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX-1, "%s_%p", STG_DEVFSM_NAME, pstCfg->pStorage);
    if( s32Ret<0 )
    {
        free(pstDevFSM);
        pstDevFSM = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_FSM_Create(&pstDevFSM->hFSM, STG_DEVFSM_MSG_QUEUE_SIZE_MAX, STG_DEVFSM_MSG_PAYLOAD_LEN_MAX, aszFsmName);
    if (HI_SUCCESS != s32Ret)
    {
        free(pstDevFSM);
        pstDevFSM = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  create fsm failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_DEVFSM_Configure(pstDevFSM->hFSM, (HI_VOID*) pstDevFSM);
    if (HI_SUCCESS != s32Ret)
    {
        (HI_VOID)STG_FSM_Destroy(pstDevFSM->hFSM);
        free(pstDevFSM);
        pstDevFSM = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  configure fsm failure, and ret is:%x!\n",__FUNCTION__,__LINE__,s32Ret);
        return s32Ret;
    }

    s32Ret = STG_FSM_RegMsgCallback(pstDevFSM->hFSM, pstCfg->pfnFSMEvntCB, (HI_VOID*)pstCfg->pDev);
    if(s32Ret != HI_SUCCESS)
    {
        (HI_VOID)STG_FSM_Destroy(pstDevFSM->hFSM);
        free(pstDevFSM);
        pstDevFSM = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fsm reg event cb failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_FSM_Start(pstDevFSM->hFSM);
    if(s32Ret != HI_SUCCESS)
    {
        (HI_VOID)STG_FSM_Destroy(pstDevFSM->hFSM);
        free(pstDevFSM);
        pstDevFSM = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  start fsm failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    *ppDevFsm = pstDevFSM;
    return HI_SUCCESS;
}

HI_VOID STG_DEVFSM_Destroy(HI_MW_PTR pDevFsm)
{
    STG_DEVFSM_S* pstDevFSM = (STG_DEVFSM_S*)pDevFsm;
    (HI_VOID)STG_FSM_Destroy(pstDevFSM->hFSM);
    free(pstDevFSM);
    pstDevFSM = HI_NULL;
    return;
}

HI_S32 STG_DEVFSM_SendEvent(HI_MW_PTR pDevFsm, STG_DEV_EVENT_INFO_S *pInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_MSG_INFO_S stMsgInfo = {0};

    STG_DEVFSM_S* pstDevFSM = (STG_DEVFSM_S*)pDevFsm;
    pstDevFSM->enCurState = pInfo->enState;
    stMsgInfo.s32What = pInfo->enState;
    stMsgInfo.u32Arg1 = pInfo->u32ErrorCnt;
    s32Ret = STG_FSM_SendAsync(pstDevFSM->hFSM, &stMsgInfo, 0);
    if(s32Ret != HI_SUCCESS)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  fsm send event failure, and ret is:%x!\n",__FUNCTION__,__LINE__,HI_ERR_STORAGE_SYSTEM_CALL_FAILURE);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 STG_DEVFSM_GetCurState(HI_MW_PTR pDevFsm, STG_DEV_STATE_E* penState)
{
    STG_DEVFSM_S* pstDevFSM = (STG_DEVFSM_S*)pDevFsm;
    *penState = pstDevFSM->enCurState;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
