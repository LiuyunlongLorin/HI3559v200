#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "hi_powercontrol.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define PWRCTRL_TASK_NAME_LEN 16
#define HI_MUTEX_TRYLOCK(mutex) (pthread_mutex_trylock(&mutex))
/** power control task context */
typedef struct tagPWRCTRL_TASK_CONTEXT_S
{
    HI_BOOL bDormantState;/**<true:dormant,false:normal*/
    HI_S32 s32PauseCount;
    HI_HANDLE TimedTaskHdl;
    HI_CHAR szPwrCtrlTaskName[PWRCTRL_TASK_NAME_LEN];
    HI_BOOL bCtrlEventHandleOnDormant;
    HI_PWRCTRL_TASK_CFG_S stCfg;
} PWRCTRL_TASK_CONTEXT_S;

/** power control mangae info */
typedef struct tagPWRCTRL_MGR_S
{
    pthread_mutex_t Mutex;
    HI_BOOL bInitState;/**<true:initallized,false:not init or deinitallize*/
    PWRCTRL_TASK_CONTEXT_S astTaskContext[HI_PWRCTRL_TASK_BUIT];
} PWRCTRL_MGR_S;

static PWRCTRL_MGR_S s_stPWRCTRLMgr = {.Mutex = PTHREAD_MUTEX_INITIALIZER,.bInitState = HI_FALSE,\
    .astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT].szPwrCtrlTaskName="screendormant",\
    .astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT].bCtrlEventHandleOnDormant=HI_TRUE,\
    .astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT].szPwrCtrlTaskName="sysdormant",\
    .astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT].bCtrlEventHandleOnDormant=HI_TRUE};


static HI_S32 POWERCTRL_DormantProc(HI_VOID* pvPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_ATTR_S stAttr;

    HI_APPCOMM_CHECK_POINTER(pvPrivData,HI_PWRCTRL_EINVAL);
    s32Ret = HI_MUTEX_TRYLOCK(s_stPWRCTRLMgr.Mutex);/**HI_MUTEX_LOCK(s_stPWRCTRLMgr.Mutex)*/
    if(s32Ret != 0)/** <mutex fail */
    {
        MLOGI("be about to dormant,but busy\n");
        return HI_PWRCTRL_EBUSY;
    }
    if(HI_TRUE != s_stPWRCTRLMgr.bInitState)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("power control not init\n");
        return HI_PWRCTRL_ENOINIT;
    }
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;

    if(NULL == pstTaskContext->stCfg.pfnDormantProc)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("pfnDormantProc is NULL.should run here\n");
        return HI_PWRCTRL_EFATA;
    }
    if(HI_TRUE != pstTaskContext->bDormantState)
    {
        if(0 == pstTaskContext->s32PauseCount)
        {
            s32Ret = pstTaskContext->stCfg.pfnDormantProc(pstTaskContext->stCfg.pvDormantPrivData);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGW("pfnScreenDormantProc fail\n");
                s32Ret = HI_PWRCTRL_EDORMANTCB;
            }
            else
            {
                 pstTaskContext->bDormantState = HI_TRUE;
                 stAttr.bEnable = HI_FALSE;
                 stAttr.u32Time_sec = pstTaskContext->stCfg.stAttr.u32Time_sec;
                 s32Ret = HI_TIMEDTASK_SetAttr(pstTaskContext->TimedTaskHdl,&stAttr);
                 if(HI_SUCCESS != s32Ret)
                 {
                     MLOGE("set task[%s] attr error\n",pstTaskContext->szPwrCtrlTaskName);
                     s32Ret = HI_PWRCTRL_ETIMEDTASK;
                 }
            }
        }
        else
        {
            MLOGE("task(%s) need dormant,but s32PauseCount(%d),need not run here\n", \
                pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
            s32Ret = HI_PWRCTRL_EINTER;
        }
    }
    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return s32Ret;
}

static HI_S32 POWERCTRL_WakeupEventTaskProc(HI_VOID* pvPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    if(NULL == pstTaskContext->stCfg.pfnWakeupProc)
    {
        MLOGE("pfnWakeupProc is NULL.should run here\n");
        return HI_PWRCTRL_EFATA;
    }
    s32Ret = pstTaskContext->stCfg.pfnWakeupProc(pstTaskContext->stCfg.pvWakeupPrivData);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("pfnWakeupProc (%s) fail\n",pstTaskContext->szPwrCtrlTaskName);
        s32Ret = HI_PWRCTRL_EWAKEUPCB;
    }
    return s32Ret;
}

static HI_S32 POWERCTRL_WakeupEventProc(HI_PWRCTRL_WAKEUP_TACTICS_E enWakeupTacticsType,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = NULL;
    HI_S32 s32Index = HI_PWRCTRL_TASK_BUIT;
    *pbEventContinueHandle = HI_TRUE;
    switch(enWakeupTacticsType)
    {
        case HI_PWRCTRL_WAKEUP_TACTICS_DISCARD: /** pause screen dormant check proc */
        case HI_PWRCTRL_WAKEUP_TACTICS_CONTINUE:
            break;
        default:
            return HI_PWRCTRL_EINVAL;
    }
    for(s32Index = HI_PWRCTRL_TASK_SYSTEMDORMANT ;s32Index >= HI_PWRCTRL_TASK_SCREENDORMANT;s32Index--)
    {
        pstTaskContext = &s_stPWRCTRLMgr.astTaskContext[s32Index];
        if(HI_TRUE == pstTaskContext->bDormantState)
        {
            s32Ret = POWERCTRL_WakeupEventTaskProc(pstTaskContext);
            if(HI_SUCCESS !=s32Ret)
            {
                MLOGE("(%s) wakeup fail\n",pstTaskContext->szPwrCtrlTaskName);
                return s32Ret;
            }
            pstTaskContext->bDormantState = HI_FALSE;

            if(0 == pstTaskContext->s32PauseCount)
            {
               s32Ret = HI_TIMEDTASK_SetAttr(pstTaskContext->TimedTaskHdl,&pstTaskContext->stCfg.stAttr);
               if(HI_SUCCESS != s32Ret)
               {
                    MLOGE("reset dormant (%s) attr fail\n",pstTaskContext->szPwrCtrlTaskName);
                    return HI_PWRCTRL_ETIMEDTASK;
               }
            }
            else
            {
                MLOGI("wakeup TaskName(%s),but s32PauseCount(%d) can not restore timetask\n",pstTaskContext->szPwrCtrlTaskName, \
                    pstTaskContext->s32PauseCount);
                if(pstTaskContext->bCtrlEventHandleOnDormant == HI_FALSE)
                {
                    MLOGE("task(%s) wakeup,but s32PauseCount(%d),never run here,fail\n", pstTaskContext->szPwrCtrlTaskName, \
                        pstTaskContext->s32PauseCount);
                    return HI_PWRCTRL_EINTER;
                }
            }
            /** when dormant,the event just use for wakeup ,no need proc the event */
            if(HI_PWRCTRL_WAKEUP_TACTICS_DISCARD == enWakeupTacticsType)
            {
                *pbEventContinueHandle = HI_FALSE;
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 POWERCTRL_PauseEventTaskProc(HI_VOID* pvPrivData,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_ATTR_S stAttr;

    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = HI_TRUE;
    if(HI_TRUE == pstTaskContext->bDormantState)
    {
        MLOGI("pause TaskName(%s) dormant,control event Low probability come here\n",pstTaskContext->szPwrCtrlTaskName);
        *pbEventContinueHandle = HI_FALSE;/**event discard on system dormant,to solve occur switch hdmi on dormant*/
        if(pstTaskContext->bCtrlEventHandleOnDormant == HI_FALSE)
        {
            return HI_PWRCTRL_ELOGICFLOW;
        }
    }
    if(0 == pstTaskContext->s32PauseCount)
    {
        pstTaskContext->s32PauseCount++;

        stAttr.bEnable = HI_FALSE;
        stAttr.u32Time_sec = pstTaskContext->stCfg.stAttr.u32Time_sec;

        s32Ret = HI_TIMEDTASK_SetAttr(pstTaskContext->TimedTaskHdl,&stAttr);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("set task[%s] attr error\n",pstTaskContext->szPwrCtrlTaskName);
            s32Ret = HI_PWRCTRL_ETIMEDTASK;
        }
    }
    else if(0 < pstTaskContext->s32PauseCount)
    {
        pstTaskContext->s32PauseCount++;
    }
    else
    {
        MLOGE("TaskName[%s] pause ,s32PauseCount(%d),error\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
        return HI_PWRCTRL_EINTER;
    }
    MLOGD("after pause TaskName[%s] dormant check,s32PauseCount(%d)\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
    return HI_SUCCESS;
}


static HI_S32 POWERCTRL_PauseEventProc(HI_PWRCTRL_EVENT_SCOPE_E enScopeType,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bContinueHandle = HI_TRUE;
    switch(enScopeType)
    {
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** pause screen dormant check proc */
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT],&bContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM:
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        default:
            s32Ret = HI_PWRCTRL_EINVAL;
            break;
    }
    if(HI_FALSE == bContinueHandle)
        *pbEventContinueHandle = HI_FALSE;

    return s32Ret;
}

static HI_S32 POWERCTRL_ResumeEventTaskProc(HI_VOID* pvPrivData,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = HI_TRUE;
    if(HI_TRUE == pstTaskContext->bDormantState)
    {
        MLOGE("resumme TaskName[%s] dormant,control event should not come here,fail\n",pstTaskContext->szPwrCtrlTaskName);
        *pbEventContinueHandle = HI_FALSE;
        if(pstTaskContext->bCtrlEventHandleOnDormant == HI_FALSE)
        {
            return HI_PWRCTRL_ELOGICFLOW;
        }
    }
    if(1 == pstTaskContext->s32PauseCount)
    {
        pstTaskContext->s32PauseCount = 0;

        s32Ret = HI_TIMEDTASK_SetAttr(pstTaskContext->TimedTaskHdl,&pstTaskContext->stCfg.stAttr);
        if(HI_SUCCESS != s32Ret)
        {
             MLOGE("reset dormant (%s) attr fail\n",pstTaskContext->szPwrCtrlTaskName);
             return HI_PWRCTRL_ETIMEDTASK;
        }
    }
    else if(1 < pstTaskContext->s32PauseCount)
    {
        pstTaskContext->s32PauseCount--;
    }
    else
    {
        MLOGE("TaskName[%s] resumme ,s32PauseCount(%d),error\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
        return HI_PWRCTRL_EFATA;
    }
    MLOGD("after resumme TaskName[%s] dormant check,s32PauseCount(%d)\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
    return HI_SUCCESS;
}

static HI_S32 POWERCTRL_ResumeEventProc(HI_PWRCTRL_EVENT_SCOPE_E enScopeType,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bContinueHandle = HI_TRUE;
    switch(enScopeType)
    {
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** pause screen dormant check proc */
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT],&bContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM:
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        default:
            s32Ret = HI_PWRCTRL_EINVAL;
            break;
    }
    if(HI_FALSE == bContinueHandle)
        *pbEventContinueHandle = HI_FALSE;
    return s32Ret;
}

static HI_S32 POWERCTRL_CommonEventTaskProc(HI_VOID* pvPrivData,HI_BOOL* pbEventContinueHandle)
{
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = HI_TRUE;
    if(HI_TRUE == pstTaskContext->bDormantState)
    {
        *pbEventContinueHandle = HI_FALSE;
    }
    return HI_SUCCESS;
}


static HI_S32 POWERCTRL_CommonEventProc(HI_PWRCTRL_EVENT_SCOPE_E enScopeType,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    *pbEventContinueHandle = HI_TRUE;
    switch(enScopeType)
    {
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** touch action discard on screen dormant*/
            s32Ret = POWERCTRL_CommonEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT],pbEventContinueHandle);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            /**event continue handler on system dormant,to solve common message and system dormant supervene*/
            break;
        case HI_PWRCTRL_EVENT_SCOPE_SYSTEM:
            /**event continue handler on system dormant,to solve common message and system dormant supervene*/
            break;
        default:
            s32Ret = HI_PWRCTRL_EINVAL;
            break;
    }
    return s32Ret;
}

static HI_S32 POWERCTRL_ResetTaskCheckTimer(HI_VOID* pvPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    if(HI_TRUE == pstTaskContext->stCfg.stAttr.bEnable)
    {
        s32Ret=HI_TIMEDTASK_ResetTime(pstTaskContext->TimedTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("reset timer (%s) error\n",pstTaskContext->szPwrCtrlTaskName);
            s32Ret = HI_PWRCTRL_ETIMEDTASK;
        }
    }
    return s32Ret;
}
static HI_S32 POWERCTRL_ResetCheckTimer(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = POWERCTRL_ResetTaskCheckTimer(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT]);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s32Ret = POWERCTRL_ResetTaskCheckTimer(&s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT]);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    return s32Ret;
}


HI_S32 HI_POWERCTRL_Init(const HI_PWRCTRL_CFG_S* pstCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = HI_PWRCTRL_TASK_BUIT;
    HI_HANDLE TimedTaskHdl = 0;
    HI_TIMEDTASK_CFG_S stTimedTaskCfg;
    HI_PWRCTRL_TASK_PROC_CALLBACK_FN_PTR pfnDormantProc = NULL;
    HI_PWRCTRL_TASK_PROC_CALLBACK_FN_PTR pfnWakeupProc = NULL;
    if(HI_TRUE == s_stPWRCTRLMgr.bInitState)
    {
        MLOGE("power control already init\n");
        return HI_PWRCTRL_EINITIALIZED;
    }
    /** parm invalid check */
    HI_APPCOMM_CHECK_POINTER(pstCfg,HI_PWRCTRL_EINVAL);
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT ;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        pfnDormantProc = pstCfg->astTaskCfg[s32Index].pfnDormantProc;
        pfnWakeupProc = pstCfg->astTaskCfg[s32Index].pfnWakeupProc;
        HI_APPCOMM_CHECK_EXPR(((pfnDormantProc != NULL && pfnWakeupProc != NULL) \
            || (pfnDormantProc == NULL && pfnWakeupProc == NULL)),HI_PWRCTRL_EINVAL);

        HI_APPCOMM_CHECK_EXPR((pstCfg->astTaskCfg[s32Index].stAttr.bEnable ==HI_FALSE) || \
            ((pstCfg->astTaskCfg[s32Index].stAttr.bEnable ==HI_TRUE)&&(pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec !=0)), \
            HI_PWRCTRL_EINVAL);
    }
    /**check relation screen dormant time and system dormant time invalid*/
    if(pstCfg->astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].stAttr.bEnable && pstCfg->astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].stAttr.bEnable)
    {
        HI_APPCOMM_CHECK_EXPR((pstCfg->astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].stAttr.u32Time_sec <= \
            pstCfg->astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].stAttr.u32Time_sec),HI_PWRCTRL_EINVAL);
    }

    /** time task register */
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT ;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        stTimedTaskCfg.pfnProc = POWERCTRL_DormantProc;
        stTimedTaskCfg.pvPrivData = (HI_VOID*)&s_stPWRCTRLMgr.astTaskContext[s32Index];
        stTimedTaskCfg.stAttr.bEnable = pstCfg->astTaskCfg[s32Index].stAttr.bEnable;
        stTimedTaskCfg.stAttr.u32Time_sec = pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec;

        s32Ret = HI_TIMEDTASK_Create(&stTimedTaskCfg,&TimedTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("dormant task(%s) creat error\n",s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
            return HI_PWRCTRL_ETIMEDTASK;
        }
        s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskHdl= TimedTaskHdl;
    }
    /** parm init */
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT ;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        s_stPWRCTRLMgr.astTaskContext[s32Index].bDormantState = HI_FALSE;
        s_stPWRCTRLMgr.astTaskContext[s32Index].s32PauseCount = 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnDormantProc = pstCfg->astTaskCfg[s32Index].pfnDormantProc;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvDormantPrivData = pstCfg->astTaskCfg[s32Index].pvDormantPrivData;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnWakeupProc = pstCfg->astTaskCfg[s32Index].pfnWakeupProc;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvWakeupPrivData = pstCfg->astTaskCfg[s32Index].pvWakeupPrivData;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable = pstCfg->astTaskCfg[s32Index].stAttr.bEnable;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec = pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec;
        MLOGD("bEnable[%d] u32Time_sec[%d],szPwrCtrlTaskName[%s]\n",s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable,
            s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec,s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
    }
    s_stPWRCTRLMgr.bInitState = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_POWERCTRL_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = HI_PWRCTRL_TASK_BUIT;
    HI_MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if(HI_TRUE != s_stPWRCTRLMgr.bInitState)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("power control not init\n");
        return HI_PWRCTRL_ENOINIT;
    }
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT ;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        s32Ret = HI_TIMEDTASK_Destroy(s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            MLOGE("dormant task(%s) destroy error\n",s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
            return HI_PWRCTRL_ETIMEDTASK;
        }
    }
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT ;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskHdl= 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].s32PauseCount = 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnDormantProc = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvDormantPrivData = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnWakeupProc = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvWakeupPrivData = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable = HI_FALSE;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec = 0;
    }
    s_stPWRCTRLMgr.bInitState = HI_FALSE;
    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return HI_SUCCESS;
}

HI_S32 HI_POWERCTRL_GetTaskAttr(HI_POWERCTRL_TASK_E enType,HI_TIMEDTASK_ATTR_S* pstTaskAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstTaskAttr,HI_PWRCTRL_EINVAL);
    HI_MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if(HI_TRUE != s_stPWRCTRLMgr.bInitState)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("power control not init\n");
        return HI_PWRCTRL_ENOINIT;
    }
    switch(enType)
    {
        case HI_PWRCTRL_TASK_SCREENDORMANT:
            break;
        case HI_PWRCTRL_TASK_SYSTEMDORMANT:
            break;
        default:
            HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            MLOGE("set time enType(%d),value exceed\n",enType);
            return HI_PWRCTRL_EINVAL;
    }
    pstTaskAttr->bEnable = s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.bEnable;
    pstTaskAttr->u32Time_sec = s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.u32Time_sec;
    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return HI_SUCCESS;
}

HI_S32 HI_POWERCTRL_SetTaskAttr(HI_POWERCTRL_TASK_E enType,const HI_TIMEDTASK_ATTR_S* pstTaskAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Index = 0;
    HI_POWERCTRL_TASK_E enAutoTaskType = HI_PWRCTRL_TASK_BUIT;
    HI_APPCOMM_CHECK_POINTER(pstTaskAttr,HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_EXPR((pstTaskAttr->bEnable == HI_FALSE) || ((pstTaskAttr->bEnable == HI_TRUE)&&(pstTaskAttr->u32Time_sec != 0)), \
        HI_PWRCTRL_EINVAL);

    HI_MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if(HI_TRUE != s_stPWRCTRLMgr.bInitState)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("power control not init\n");
        return HI_PWRCTRL_ENOINIT;
    }
    switch(enType)
    {
        case HI_PWRCTRL_TASK_SCREENDORMANT:
            if((HI_TRUE == pstTaskAttr->bEnable) && \
                (HI_TRUE == s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT].stCfg.stAttr.bEnable) && \
                (pstTaskAttr->u32Time_sec  > s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SYSTEMDORMANT].stCfg.stAttr.u32Time_sec ))
            {
                enAutoTaskType = HI_PWRCTRL_TASK_SYSTEMDORMANT;
            }
            break;
        case HI_PWRCTRL_TASK_SYSTEMDORMANT:
            if((HI_TRUE == pstTaskAttr->bEnable) && \
                (HI_TRUE == s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT].stCfg.stAttr.bEnable) && \
               (pstTaskAttr->u32Time_sec  < s_stPWRCTRLMgr.astTaskContext[HI_PWRCTRL_TASK_SCREENDORMANT].stCfg.stAttr.u32Time_sec ))
            {
                enAutoTaskType = HI_PWRCTRL_TASK_SCREENDORMANT;
            }
            break;
        default:
            HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            MLOGE("set time enType(%d),value exceed\n",enType);
            return HI_PWRCTRL_EINVAL;
    }
    for(s32Index = HI_PWRCTRL_TASK_SCREENDORMANT;s32Index < HI_PWRCTRL_TASK_BUIT;s32Index++)
    {
        if(s32Index == enType)
        {
            s32Ret = HI_TIMEDTASK_SetAttr(s_stPWRCTRLMgr.astTaskContext[enType].TimedTaskHdl,pstTaskAttr);
            if(HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
                MLOGE("timetask[%s] set Attr error\n",s_stPWRCTRLMgr.astTaskContext[enType].szPwrCtrlTaskName);
                return HI_PWRCTRL_ETIMEDTASK;
            }
            s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.bEnable = pstTaskAttr->bEnable;
            s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.u32Time_sec = pstTaskAttr->u32Time_sec;
            if(HI_PWRCTRL_TASK_BUIT != enAutoTaskType)
            {
                s32Ret = HI_TIMEDTASK_SetAttr(s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].TimedTaskHdl,pstTaskAttr);
                if(HI_SUCCESS != s32Ret)
                {
                    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
                    MLOGE("timetask[%s] set Attr error\n",s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].szPwrCtrlTaskName);
                    return HI_PWRCTRL_ETIMEDTASK;
                }
                s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].stCfg.stAttr.u32Time_sec = pstTaskAttr->u32Time_sec;
                s32Ret = HI_PWRCTRL_ETASKTIMEAUTO;
            }
            break;
        }
    }
    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return s32Ret;
}

HI_S32 HI_POWERCTRL_EventPreProc(const HI_PWRCTRL_EVENT_ATTR_S* pstEventAttr,HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bResetTimer = HI_FALSE;
    HI_APPCOMM_CHECK_POINTER(pstEventAttr,HI_PWRCTRL_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pbEventContinueHandle,HI_PWRCTRL_EINVAL);
    HI_MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if(HI_TRUE != s_stPWRCTRLMgr.bInitState)
    {
        HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        MLOGE("power control not init\n");
        return HI_PWRCTRL_ENOINIT;
    }
    switch(pstEventAttr->enType)
    {
        case HI_PWRCTRL_EVENT_TYPE_WAKEUP:
            s32Ret = POWERCTRL_WakeupEventProc(pstEventAttr->unCfg.stWakeupCfg.enType,pbEventContinueHandle);
            bResetTimer = pstEventAttr->unCfg.stWakeupCfg.stCommonCfg.bResetTimer;
            break;

        case HI_PWRCTRL_EVENT_TYPE_CONTROL:
            switch(pstEventAttr->unCfg.stCtrlCfg.enType)
            {

                case HI_PWRCTRL_EVENT_CONTROL_PAUSE:
                    s32Ret = POWERCTRL_PauseEventProc(pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.enType,pbEventContinueHandle);
                    bResetTimer = pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.bResetTimer;
                    break;
                case HI_PWRCTRL_EVENT_CONTROL_RESUME:
                    s32Ret = POWERCTRL_ResumeEventProc(pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.enType,pbEventContinueHandle);
                    bResetTimer = pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.bResetTimer;
                    break;
                default:
                    s32Ret = HI_PWRCTRL_EINVAL;
                    break;
            }
            break;
        case HI_PWRCTRL_EVENT_TYPE_COMMON:
            s32Ret = POWERCTRL_CommonEventProc(pstEventAttr->unCfg.stCommonCfg.enType,pbEventContinueHandle);
            bResetTimer = pstEventAttr->unCfg.stCommonCfg.bResetTimer;
            break;
        default:
            s32Ret = HI_PWRCTRL_EINVAL;
            break;
    }
    if(HI_SUCCESS == s32Ret)
    {
        if(HI_TRUE == bResetTimer)
            s32Ret = POWERCTRL_ResetCheckTimer();
    }
    HI_MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */
