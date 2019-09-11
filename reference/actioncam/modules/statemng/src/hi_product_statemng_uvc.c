/**
 * \file   hi_product_statemng_uvc.c
 * \brief  Realize the interface about uvc state.
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

/** enter UVC state */
HI_S32 PDT_STATEMNG_UVCStateEnter(HI_VOID *pvArg)
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
    MLOGD(YELLOW"enEnterWorkMode(%d)\n"NONE, enEnterWorkMode);

    g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
    g_stSTATEMNGContext.bRunning = HI_FALSE;

    /** get enter mediamode */
    HI_PDT_MEDIAMODE_E enEnterMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enEnterWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE,
        (HI_VOID *)&enEnterMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaMode");
    MLOGD("enEnterMediaMode(%d)\n", enEnterMediaMode);

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enEnterWorkMode, enEnterMediaMode,
        &stEnterMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stEnterMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
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

/** exit UVC state */
HI_S32 PDT_STATEMNG_UVCStateExit(HI_VOID *pvArg)
{
    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE,pstStateAttr->stState.name);
    g_stSTATEMNGContext.bForceReBuid = HI_TRUE;

    return HI_SUCCESS;
}

/** UVC state message process */
HI_S32 PDT_STATEMNG_UVCStateMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
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

/** init UVC state, add UVC state to HFSM */
HI_S32 PDT_STATEMNG_UVCStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stUVCState =
        {{HI_PDT_WORKMODE_UVC,
          PDT_STATEMNG_STATE_UVC,
          PDT_STATEMNG_UVCStateEnter,
          PDT_STATEMNG_UVCStateExit,
          PDT_STATEMNG_UVCStateMsgProc,
          NULL}};
    stUVCState.stState.argv = &stUVCState;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stUVCState.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add UVC");

    return s32Ret;
}

/** deinit UVC State module */
HI_S32 PDT_STATEMNG_UVCStateDeinit()
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



