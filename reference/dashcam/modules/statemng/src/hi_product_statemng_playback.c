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


/** enter Playback state */
HI_S32 PDT_STATEMNG_PlaybackStateEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"parameter pvArg error");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"try to enter state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_STORAGEMNG_EINTER,"change state to workmode");
    if(HI_PDT_WORKMODE_PLAYBACK != enEnterWorkMode)
    {
        MLOGE(RED"enter workmode[%d] error\n\n"NONE,enEnterWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }
    else
    {
        pstStateMngCtx->enCurrentWorkMode = enEnterWorkMode;
        pstStateMngCtx->bRunning = pstStateAttr->bRunning;
    }
    /*get media mode config*/
    s32Ret = PDT_STATEMNG_GetMediaModeCfg(pstStateMngCtx->enCurrentWorkMode,pstStateMngCtx->astCamCtx);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get media mode config");


    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enEnterWorkMode, pstStateMngCtx->astCamCtx,&stEnterMediaCfg);

    /**update disp configure*/
    s32Ret = PDT_STATEMNG_UpdateDispCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"update video out configure");


    /** generate MediaCfg */
    PDT_STATEMNG_MediaCfgResetForPlayback(&stEnterMediaCfg);
    /** reset Media */

    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_STORAGEMNG_EINTER,"reset media");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    HI_EVENT_S stEvent;
    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stEvent.arg2 = enEnterWorkMode;
    stEvent.s32Result = HI_SUCCESS;
    MUTEX_LOCK(pstStateMngCtx->Mutex);
    pstStateMngCtx->bInProgress = HI_FALSE;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    s32Ret = HI_EVTHUB_Publish(&stEvent);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event failed\n\n"NONE);
        return HI_FAILURE;
    }
    else
    {
        MLOGD(YELLOW"publish event:HI_EVENT_STATEMNG_SWITCH_WORKMODE(%x), "
                    "arg2(%s) to EventHub succeed\n\n"NONE,
                    stEvent.EventID, pstStateAttr->stState.name);
    }

    HI_TIME_STAMP;
    return s32Ret;
}

/** exit Playback state */
HI_S32 PDT_STATEMNG_PlaybackStateExit(HI_VOID *pvArg)
{
    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"parameter pvArg error");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));

    return HI_SUCCESS;
}

/** Playback state message process */
HI_S32 PDT_STATEMNG_PlaybackStateMsgProc(HI_MESSAGE_S *pstMsg,
                                         HI_VOID *pvArg,
                                         HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg,
                                          pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n",
          pstStateAttr->stState.name, (pstStateAttr->bRunning));

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_STORAGEMNG_EINTER,"change state to workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        case HI_EVENT_STATEMNG_STOP:
        case HI_EVENT_STATEMNG_TRIGGER:
        case HI_EVENT_STATEMNG_SETTING:
        case HI_EVENT_STATEMNG_LIVE_SWITCH:
        case HI_EVENT_STATEMNG_VO_SWITCH:
        {
            MLOGD(YELLOW"no need to process message what(%x)\n\n"NONE,
                        pstMsg->what);
            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateMngCtx->bInProgress = HI_FALSE;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);
            PDT_STATEMNG_PublishEvent(pstMsg, HI_SUCCESS);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_RECMNG_ERROR:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"no need to process message what(%x)\n\n"NONE,
                        pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        /** need base state process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        default:
        {
            MLOGD(YELLOW"can not process this message, upload it to Base state\n\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;/** need base state precess */
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
          NULL},HI_FALSE};
    stPlaybackState.stState.argv = &stPlaybackState;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stPlaybackState.stState,
                              (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_STORAGEMNG_EINTER,"HFSM add Playback state");

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


