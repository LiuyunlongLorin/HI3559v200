/**
 * \file   hi_product_statemng_suspend.c
 * \brief  Realize the interface about suspend state.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern PDT_STATEMNG_CONTEXT g_stSTATEMNGContext;

/** enter Suspend state */
HI_S32 PDT_STATEMNG_SuspendStateEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"enter state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
    g_stSTATEMNGContext.bRunning = HI_FALSE;
#ifdef CONFIG_SCREEN
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(off)");

    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(off)");
#endif

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    /** pause storagemng */
    s32Ret = HI_STORAGEMNG_Pause((const HI_CHAR*)g_stSTATEMNGContext.stStorageMngCfg.szMntPath);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause storagemng");

    /** deinit media pipe */ //Lorin Modity
    // s32Ret = HI_PDT_MEDIA_VideoOutStop();
    // if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    // {
    //     MLOGD(YELLOW"not inited\n"NONE);
    // }
    // else
    // {
    //     PDT_STATEMNG_CHECK_RET(s32Ret,"stop VO");
    // }

#if 0
    s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit VO");
    }
#endif

    s32Ret = HI_PDT_MEDIA_AudioOutStop();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop AO");
    }

    s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit video");
    }

    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit audio");
    }

    g_stSTATEMNGContext.bSuspend = HI_TRUE;

    /** publish switch workmode event */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, enEnterWorkMode, HI_TRUE, 0L, NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish switch workmode failed(0x%08X)\n"NONE, s32Ret);
    }
    else
    {
        MLOGD(YELLOW"publish switch to workmode(%s) msg(0x%08X) succeed\n"NONE,
            pstStateAttr->stState.name, HI_EVENT_STATEMNG_SWITCH_WORKMODE);
        g_stSTATEMNGContext.bInProgress = HI_FALSE;

    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** exit Suspend state */
HI_S32 PDT_STATEMNG_SuspendStateExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    s32Ret = HI_STORAGEMNG_Resume((const HI_CHAR*)g_stSTATEMNGContext.stStorageMngCfg.szMntPath);
    PDT_STATEMNG_CHECK_RET(s32Ret,"resume storagemng");

    return HI_SUCCESS;
}

/** Suspend state message process */
HI_S32 PDT_STATEMNG_SuspendStateMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(g_stSTATEMNGContext.Mutex, pvArg, pStateID,
        pstMsg, g_stSTATEMNGContext.bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("curr state(%s)\n", pstStateAttr->stState.name);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        case HI_EVENT_STATEMNG_STOP:
        case HI_EVENT_STATEMNG_SETTING:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_RECMNG_ERROR:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        default:
        {
            MLOGD(YELLOW"upload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Suspend state, add Suspend state to HFSM */
HI_S32 PDT_STATEMNG_SuspendStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stSuspendState =
        {{HI_PDT_WORKMODE_SUSPEND,
          PDT_STATEMNG_STATE_SUSPEND,
          PDT_STATEMNG_SuspendStateEnter,
          PDT_STATEMNG_SuspendStateExit,
          PDT_STATEMNG_SuspendStateMsgProc,
          NULL}};
    stSuspendState.stState.argv = &stSuspendState;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stSuspendState.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add Suspend");

    return s32Ret;
}

/** deinit Suspend State module */
HI_S32 PDT_STATEMNG_SuspendStateDeinit()
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



