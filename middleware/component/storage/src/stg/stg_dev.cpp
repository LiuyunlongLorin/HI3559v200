#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include "hi_mw_type.h"
#include "stg_log.h"
#include "hi_storage.h"
#include "stg_common.h"
#include "stg_dev.h"
#include "stg_devinfo.h"
#include "stg_devfsm.h"
#include "stg_devmonitor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
typedef struct tagSTG_DEV_S
{
    HI_MW_PTR pDevInfo;
    HI_MW_PTR pDevFSM;
    HI_MW_PTR pDevMonitor;
}STG_DEV_S;

static HI_S32 STG_DEV_FSMEventCB(__attribute__((unused))HI_VOID* pStateMachine, HI_VOID* pPriv, HI_S32 s32Event, __attribute__((unused))HI_S32 s32RetVal)
{
    STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_INFO, "%s  %d  event(%d) handled!\n",__FUNCTION__,__LINE__, s32Event);
    STG_DEV_S* pstDev = (STG_DEV_S*)pPriv;
    if (pstDev->pDevMonitor)
    {
        STG_DEVMONITOR_ResumeNotify(pstDev->pDevMonitor);
    }
    return HI_SUCCESS;
}

HI_S32 STG_DEV_Create(const STG_DEV_CFG_S* pstCfg, HI_MW_PTR* ppDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_DEVFSM_CFG_S stDevFSMCfg = {0};
    STG_DEV_S* pstDev = (STG_DEV_S*)malloc(sizeof(STG_DEV_S));
    if (HI_NULL == pstDev)
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstDev, sizeof(STG_DEV_S), 0x00, sizeof(STG_DEV_S));
    if( EOK!=s32Ret )
    {
        free(pstDev);
        pstDev = HI_NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_DEVINFO_Create(pstCfg->u8DevPortNo, pstCfg->u8DevErrCountThr, pstCfg->aszPartitionPath, &pstDev->pDevInfo);
    if (HI_SUCCESS != s32Ret)
    {
        free(pstDev);
        pstDev = HI_NULL;
        return s32Ret;
    }

    stDevFSMCfg.pFSTool = pstCfg->pFSTool;
    stDevFSMCfg.pStorage = pstCfg->pStorage;
    stDevFSMCfg.pDev = pstDev;
    stDevFSMCfg.pfnOnStateChange = pstCfg->pfnOnStateChange;
    stDevFSMCfg.pfnFSMEvntCB = STG_DEV_FSMEventCB;
    s32Ret = STG_DEVFSM_Create(&stDevFSMCfg, &pstDev->pDevFSM);
    if (HI_SUCCESS != s32Ret)
    {
        STG_DEVINFO_Destroy(pstDev->pDevInfo);
        pstDev->pDevInfo = HI_NULL;
        free(pstDev);
        pstDev = HI_NULL;
        return s32Ret;
    }

    s32Ret = STG_DEVMONITOR_Create(pstDev->pDevInfo, pstDev->pDevFSM, &pstDev->pDevMonitor);
    if (HI_SUCCESS != s32Ret)
    {
        STG_DEVFSM_Destroy(pstDev->pDevFSM);
        pstDev->pDevFSM = HI_NULL;

        STG_DEVINFO_Destroy(pstDev->pDevInfo);
        pstDev->pDevInfo = HI_NULL;

        free(pstDev);
        pstDev = HI_NULL;
        return s32Ret;
    }

    *ppDev = pstDev;
    return HI_SUCCESS;
}

HI_VOID STG_DEV_Destroy(HI_MW_PTR pDev)
{
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;

    STG_DEVMONITOR_Destroy(pstDev->pDevMonitor);
    pstDev->pDevMonitor = HI_NULL;

    STG_DEVFSM_Destroy(pstDev->pDevFSM);
    pstDev->pDevFSM = HI_NULL;

    STG_DEVINFO_Destroy(pstDev->pDevInfo);
    pstDev->pDevInfo = HI_NULL;

    free(pstDev);
    pstDev = HI_NULL;
    return;
}

HI_S32 STG_DEV_GetState(HI_MW_PTR pDev, STG_DEV_STATE_E* penState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;

    s32Ret = STG_DEVFSM_GetCurState(pstDev->pDevFSM, penState);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev state failure, and ret is:%x!\n",__FUNCTION__,__LINE__,s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 STG_DEV_GetInfo(HI_MW_PTR pDev, HI_STORAGE_DEV_INFO_S* pInfo)
{
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;
    return STG_DEVINFO_GetInfo(pstDev->pDevInfo, pInfo);
}

HI_S32 STG_DEV_PauseMonitor(HI_MW_PTR pDev)
{
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;
    return STG_DEVMONITOR_StopNotify(pstDev->pDevMonitor);
}

HI_S32 STG_DEV_ResumeMonitor(HI_MW_PTR pDev)
{
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;
    return STG_DEVMONITOR_RestartNotify(pstDev->pDevMonitor);
}

HI_BOOL STG_DEV_IsPaused(HI_MW_PTR pDev)
{
    STG_DEV_S* pstDev = (STG_DEV_S*)pDev;
    return STG_DEVMONITOR_IsStopNotify(pstDev->pDevMonitor);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
