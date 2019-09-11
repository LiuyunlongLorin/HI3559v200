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
#define STG_DEVMONITOR_SAMPLING_TIME    (200000)   //unit us
#define hi_usleep(usec) \
do { \
    struct timespec req; \
    req.tv_sec  = (usec) / 1000000; \
    req.tv_nsec = ((usec) % 1000000) * 1000; \
    nanosleep (&req, NULL); \
} while (0)

typedef enum tagSTG_DEVMONITOR_REPORT_STATE_E
{
    STG_DEVMONITOR_STATE_RUNNING = 0,
    STG_DEVMONITOR_STATE_SUSPEND ,
    STG_DEVMONITOR_STATE_BUTT
} STG_DEVMONITOR_STATE_E;


typedef struct tagSTG_DEVMONITOR_S
{
    pthread_t pID;
    HI_BOOL bDestroy;
    HI_BOOL bStopNotify;
    HI_BOOL bPauseNotify;
    HI_MW_PTR pDevInfo;
    HI_MW_PTR pDevFSM;
}STG_DEVMONITOR_S;

static HI_VOID* STG_DEVMONITOR_Routine(HI_VOID* pArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bStateChangeDurHandle = HI_FALSE;
    STG_DEV_STATE_E enLastState = STG_DEV_STATE_IDEL;
    STG_DEV_STATE_E enCurState = STG_DEV_STATE_IDEL;
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pArg;
    STG_DEV_EVENT_INFO_S stEventInfo;

    prctl(PR_SET_NAME, "mw_devmonitor", 0, 0, 0);

    while (!pMonitor->bDestroy)
    {
        s32Ret = STG_DEVINFO_GetEventInfo(pMonitor->pDevInfo, &stEventInfo);
        if (HI_SUCCESS != s32Ret)
        {
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get dev eventinfo failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
        }
        enCurState = stEventInfo.enState;
        if (!bStateChangeDurHandle)
        {
            bStateChangeDurHandle = ((enLastState != enCurState) || (stEventInfo.u32ErrorCnt != 0))? HI_TRUE : HI_FALSE;
        }

        if (pMonitor->bStopNotify
            || pMonitor->bPauseNotify
            || !bStateChangeDurHandle
            || ((STG_DEV_STATE_UNPLUGGED == enLastState) && (STG_DEV_STATE_UNPLUGGED == enCurState)))
        {
            //this case not send event.
        }
        else
        {
            //this case send event
            enLastState = enCurState;
            bStateChangeDurHandle = HI_FALSE;
            pMonitor->bPauseNotify = HI_TRUE;
            s32Ret = STG_DEVFSM_SendEvent(pMonitor->pDevFSM, &stEventInfo);
            if (HI_SUCCESS != s32Ret)
            {
                STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  SendAsync failure, and ret:%x!\n",__FUNCTION__,__LINE__, s32Ret);
            }
        }
        if(STG_DEV_STATE_CONNECTED != enLastState || STG_DEV_STATE_CONNECTED != enCurState)
        {
            //for quick detect sd card mounted,sleep less.
            hi_usleep(STG_DEVMONITOR_SAMPLING_TIME/4);
        }
        else
        {
            //when at connected status,sleep more.
            hi_usleep(STG_DEVMONITOR_SAMPLING_TIME);
        }
    }

    return HI_NULL;
}

HI_S32 STG_DEVMONITOR_Create(HI_MW_PTR pDevInfo, HI_MW_PTR pDevFSM, HI_MW_PTR* ppDevMonitor)
{
    HI_S32 s32Ret = EOK;
    STG_DEVMONITOR_S* pstMonitor = (STG_DEVMONITOR_S*)malloc(sizeof(STG_DEVMONITOR_S));
    if (HI_NULL == pstMonitor)
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstMonitor, sizeof(STG_DEVMONITOR_S), 0x00, sizeof(STG_DEVMONITOR_S));
    if( EOK!=s32Ret )
    {
        free(pstMonitor);
        pstMonitor = NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }
    pstMonitor->pDevInfo = pDevInfo;
    pstMonitor->pDevFSM = pDevFSM;
    pstMonitor->bDestroy = HI_FALSE;
    pstMonitor->bStopNotify = HI_FALSE;
    pstMonitor->bPauseNotify = HI_FALSE;
    *ppDevMonitor = pstMonitor;

    if (HI_SUCCESS != pthread_create(&pstMonitor->pID, HI_NULL, STG_DEVMONITOR_Routine, (HI_VOID*)(*ppDevMonitor)))
    {
        *ppDevMonitor = HI_NULL;
        free(pstMonitor);
        pstMonitor = HI_NULL;
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID STG_DEVMONITOR_Destroy(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    pMonitor->bStopNotify = HI_TRUE;
    pMonitor->bDestroy = HI_TRUE;
    (HI_VOID)pthread_join(pMonitor->pID, HI_NULL);
    free(pMonitor);
    pMonitor = HI_NULL;
    return;
}

HI_S32 STG_DEVMONITOR_StopNotify(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    if (pMonitor->bStopNotify)
    {
        return HI_ERR_STORAGE_PAUSED;
    }

    pMonitor->bStopNotify = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 STG_DEVMONITOR_RestartNotify(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    if (!pMonitor->bStopNotify)
    {
        return HI_ERR_STORAGE_RUNNING;
    }

    pMonitor->bStopNotify = HI_FALSE;
    return HI_SUCCESS;
}

HI_BOOL STG_DEVMONITOR_IsStopNotify(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    return pMonitor->bStopNotify;
}

HI_VOID STG_DEVMONITOR_PauseNotify(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    pMonitor->bPauseNotify = HI_TRUE;
    return;
}
HI_VOID STG_DEVMONITOR_ResumeNotify(HI_MW_PTR pDevMonitor)
{
    STG_DEVMONITOR_S* pMonitor = (STG_DEVMONITOR_S*)pDevMonitor;
    pMonitor->bPauseNotify = HI_FALSE;
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
