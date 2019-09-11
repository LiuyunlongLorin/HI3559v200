/**
 * \file   hi_product_statemng_playback.c
 * \brief  Realize the interface about playback state.
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

/** enter Playback state */
HI_S32 PDT_STATEMNG_PlaybackStateEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_APPCOMM_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR);
    HI_TIME_STAMP;

    /** check workmode(stateID) and update context */
    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    if (HI_PDT_WORKMODE_PLAYBACK != pstStateAttr->stState.stateID)
    {
        return HI_PDT_STATEMNG_EINTER;
    }
    g_stSTATEMNGContext.enCurrentWorkMode = pstStateAttr->stState.stateID;
    g_stSTATEMNGContext.bRunning = HI_FALSE;
    MLOGD(YELLOW"enter state(%s)"NONE"\n", pstStateAttr->stState.name);

    /** get enter mediamode */
    HI_PDT_MEDIAMODE_E enEnterMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(g_stSTATEMNGContext.enCurrentWorkMode,
                                           HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID*)&enEnterMediaMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);

    /** generate MediaCfg */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(g_stSTATEMNGContext.enCurrentWorkMode,
        enEnterMediaMode, &stMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);

    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);
    g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stOsdCfg.stTimeOsd.bShow = HI_FALSE;

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, HI_PDT_WORKMODE_PLAYBACK, HI_TRUE, 0L, NULL, 0);
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

/** exit Playback state */
HI_S32 PDT_STATEMNG_PlaybackStateExit(HI_VOID *pvArg)
{
    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    return HI_SUCCESS;
}

/** Playback state message process */
HI_S32 PDT_STATEMNG_PlaybackStateMsgProc(HI_MESSAGE_S *pstMsg,
    HI_VOID *pvArg, HI_STATE_ID *pStateID)
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
        case HI_EVENT_STATEMNG_SD_FORMAT:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_FALSE);
            g_stSTATEMNGContext.bInProgress = HI_FALSE;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_RECMNG_ERROR:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        /** need base state process */
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

/** init Playback state, add Playback state to HFSM */
HI_S32 PDT_STATEMNG_PlaybackStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stPlaybackState =
        {{HI_PDT_WORKMODE_PLAYBACK,
          PDT_STATEMNG_STATE_PLAYBACK,
          PDT_STATEMNG_PlaybackStateEnter,
          PDT_STATEMNG_PlaybackStateExit,
          PDT_STATEMNG_PlaybackStateMsgProc,
          NULL}};
    stPlaybackState.stState.argv = &stPlaybackState;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stPlaybackState.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add Playback");

    return s32Ret;
}

/** deinit Playback State module */
HI_S32 PDT_STATEMNG_PlaybackStateDeinit()
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


