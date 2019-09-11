/**
 * \file   hi_product_statemng_hdmi_preview.c
 * \brief  Realize the interface about hdmi preview states.
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

static HI_S32 PDT_STATEMNG_HDMIPreviewUpdateProTune(HI_PDT_WORKMODE_E enPoweronWorkMode,
    HI_PDT_WORKMODE_CFG_S* pstEnterWorkModeCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstEnterWorkModeCfg, HI_PDT_STATEMNG_ENULLPTR);
    HI_S32 s32Ret = HI_SUCCESS;

    switch(enPoweronWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stNormRecAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_LOOP_REC:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stLoopRecAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_LPSE_REC:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stLapseRecAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_SLOW_REC:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stSlowRecAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_BURST:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stBurstAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_RECSNAP:
        {
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                &(pstEnterWorkModeCfg->unModeAttr.stRecSnapAttr.stProTune),
                sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_SING_PHOTO:
        {
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   &(pstEnterWorkModeCfg->unModeAttr.stSinglePhotoAttr.stProTune),
                   sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        {
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   &(pstEnterWorkModeCfg->unModeAttr.stDelayPhotoAttr.stProTune),
                   sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_LPSE_PHOTO:
        {
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   &(pstEnterWorkModeCfg->unModeAttr.stLapsePhotoAttr.stProTune),
                   sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
            break;
        }
        case HI_PDT_WORKMODE_HDMI_PREVIEW:
        {
            g_stSTATEMNGContext.stVideoProTune.u32WB = 0;
            g_stSTATEMNGContext.stVideoProTune.u32ISO = 0;
            g_stSTATEMNGContext.stVideoProTune.enEV = HI_PDT_SCENE_EV_0;
            g_stSTATEMNGContext.stVideoProTune.enMetryType = HI_PDT_SCENE_METRY_TYPE_CENTER;
            g_stSTATEMNGContext.stVideoProTune.u16MetryCenterParamIdx = 0;
            g_stSTATEMNGContext.stVideoProTune.u16MetryAverageParamIdx = 0;
            break;
        }
        default:
        {
            MLOGE(RED"param err\n"NONE);
            s32Ret = HI_FAILURE;
            break;
        }
    }

    return s32Ret;
}

/** enter HDMIPreview state */
HI_S32 PDT_STATEMNG_HDMIPreviewStateEnter(HI_VOID* pvArg)
{
    HI_APPCOMM_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR);
    HI_TIME_STAMP;

    /** check workmode(stateID) and update context */
    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    if (HI_PDT_WORKMODE_HDMI_PREVIEW != pstStateAttr->stState.stateID)
    {
        return HI_PDT_STATEMNG_EINTER;
    }
    g_stSTATEMNGContext.enCurrentWorkMode = pstStateAttr->stState.stateID;
    g_stSTATEMNGContext.bRunning = HI_FALSE;
    MLOGD(YELLOW"enter state(%s)"NONE"\n", pstStateAttr->stState.name);
    g_stSTATEMNGContext.bForceReBuid = HI_TRUE;

    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_WORKMODE_E  enEnterWorkMode = HI_PDT_WORKMODE_HDMI_PREVIEW;
    HI_PDT_MEDIAMODE_E enEnterMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enEnterWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE,
                                           (HI_VOID*)&enEnterMediaMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);

    if (HI_PDT_MEDIAMODE_BUTT == enEnterMediaMode)
    {
        /** get poweron workmode */
        HI_PDT_WORKMODE_E enPoweronWorkMode = HI_PDT_WORKMODE_BUTT;
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);
        enEnterWorkMode = enPoweronWorkMode;

        s32Ret = HI_PDT_PARAM_GetWorkModeParam(enPoweronWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE,
                                               (HI_VOID*)&enEnterMediaMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);
    }

    /** get workmode cfg */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get WorkModeCfg");

    /** update ProTune */
    s32Ret = PDT_STATEMNG_HDMIPreviewUpdateProTune(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"update ProTune");

    /** get mediacfg */
    s32Ret = HI_PDT_PARAM_GetMediaCfg(g_stSTATEMNGContext.enCurrentWorkMode,
        enEnterMediaMode, &(g_stSTATEMNGContext.stMediaCfg),
        &(g_stSTATEMNGContext.stSceneModeCfg));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);

    /** update mediacfg  */
    s32Ret = HI_PDT_MEDIA_GenerateHDMIMediaCfg(&g_stSTATEMNGContext.stMediaCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);
    HI_TIME_STAMP;

#ifdef CONFIG_SCREEN
    s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(off)");

    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(off)");
#endif

    /** reset Media */
    PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    HI_TIME_STAMP;

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, pstStateAttr->stState.stateID, HI_TRUE, 0L, NULL, 0);
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

/** exit HDMIPreview state */
HI_S32 PDT_STATEMNG_HDMIPreviewStateExit(HI_VOID *pvArg)
{
    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg, "pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);
    g_stSTATEMNGContext.bForceReBuid = HI_TRUE;

    return HI_SUCCESS;
}

/** HDMIPreview state message process */
HI_S32 PDT_STATEMNG_HDMIPreviewStateMsgProc(HI_MESSAGE_S *pstMsg,
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
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
        {
#ifdef CONFIG_SCREEN
            if(pstMsg->arg2 != HI_PDT_WORKMODE_HDMI_PLAYBACK)
            {
                s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
                PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(on)");

                s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
                PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(on)");
            }
#endif
            return HI_PROCESS_MSG_UNHANDLER;
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

/** init HDMIPreview state, add HDMIPreview state to HFSM */
HI_S32 PDT_STATEMNG_HDMIPreviewStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stHDMIPreviewState =
        {{HI_PDT_WORKMODE_HDMI_PREVIEW,
          PDT_STATEMNG_STATE_HDMI_PREVIEW,
          PDT_STATEMNG_HDMIPreviewStateEnter,
          PDT_STATEMNG_HDMIPreviewStateExit,
          PDT_STATEMNG_HDMIPreviewStateMsgProc,
          NULL}};
    stHDMIPreviewState.stState.argv = &stHDMIPreviewState;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stHDMIPreviewState.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add HDMIPreview");

    return s32Ret;
}

/** deinit HDMIPreview State module */
HI_S32 PDT_STATEMNG_HDMIPreviewStateDeinit()
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

