/**
 * \file   hi_product_statemng_recsnap.c
 * \brief  Realize the interface about RecSnap state, this state dependent Rec/Photo States.
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
pthread_mutex_t g_StartMutex;
pthread_cond_t  g_StartCond;
HI_BOOL g_bRecStarted = HI_FALSE;

/** Reset all(meida/RecTask) for setting parameter */
HI_S32 PDT_STATEMNG_RecSnapStateResetAll(HI_HANDLE RecMngTaskHdl, HI_PDT_MEDIA_CFG_S *pstMediaCfg,
    HI_RECMNG_ATTR_S *pstRecMngAttr, HI_BOOL bResetMedia)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** destoty RecMng task */
    s32Ret = HI_RECMNG_DestroyTask(RecMngTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"destroy RecMng task");

    /** reset Media */
    if(HI_TRUE == bResetMedia)
    {
        s32Ret = PDT_STATEMNG_ResetMedia(pstMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
        PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");
    }

    /** set system configure */
    PDT_STATEMNG_SetSysConfig();

    /** create RecMng task */
    s32Ret = HI_RECMNG_CreateTask(pstRecMngAttr, &RecMngTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"create RecMng task");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecSnapStateSetMediaMode(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_MEDIAMODE_E enSettingMediaMode = *(HI_PDT_MEDIAMODE_E *)pvPayload;
    MLOGD(YELLOW"set MediaMode(%d), workmode(%d)\n"NONE, enSettingMediaMode, enSettingWorkMode);

    /** filter out the same parameter */
    if(enSettingMediaMode == g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/enSettingMediaMode pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingMediaMode);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** get setting MediaCfg from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, enSettingMediaMode,
        &stSettingMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stSettingMediaCfg, pstMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** Reset all(meida/RecTask) for setting parameter */
    PDT_STATEMNG_RecSnapStateResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_TRUE);

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save MediaMode param");
    MLOGD(YELLOW"save MediaMode(%d) param succeed\n"NONE, enSettingMediaMode);

    g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode = enSettingMediaMode;
    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecSnapStateSetLapseInterval(HI_MESSAGE_S * pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 s32SettingLapseInterval = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set LapseInterval(%d)\n"NONE, s32SettingLapseInterval);

    /** filter out the same parameter */
    if(s32SettingLapseInterval == pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out rong enSettingWorkMode/enSettingLapseInterval pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)s32SettingLapseInterval);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms = s32SettingLapseInterval;

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, (HI_VOID *)&s32SettingLapseInterval);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save LapseInterval param");
    MLOGD(YELLOW"save LapseInterval(%d) param succeed\n"NONE, s32SettingLapseInterval);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecSnapStateSetPlayloadType(HI_MESSAGE_S * pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_MAPI_PAYLOAD_TYPE_E enSettingPlayloadType = *(HI_MAPI_PAYLOAD_TYPE_E *)pvPayload;
    MLOGD(YELLOW"set PlayloadType(%d), workmode(%d)\n"NONE, enSettingPlayloadType, enSettingWorkMode);

    /** get param to Param module */
    HI_MAPI_PAYLOAD_TYPE_E enCurrentPlayloadType = HI_MAPI_PAYLOAD_TYPE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, (HI_VOID *)&enCurrentPlayloadType);
    MLOGD(YELLOW"curr PlayloadType(%d), workmode(%d)\n"NONE, enCurrentPlayloadType, enSettingWorkMode);

    /** filter out the same parameter */
    if(enCurrentPlayloadType == enSettingPlayloadType)/** just set main stream */
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingPlayloadType);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, "get mediamode");

    HI_MEDIA_VENC_ATTR_S stVencAttr;
    s32Ret = HI_PDT_PARAM_GetVencAttrByPayloadType(enMediaMode,
        enSettingPlayloadType, &stVencAttr);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get vencAttr");

    HI_BOOL bAddMainStream = HI_FALSE;
    /** remove main video stream */
    s32Ret = HI_LIVESVR_RemoveStream(PDT_STATEMNG_MAIN_STREAM_NAME);
    if(HI_SUCCESS == s32Ret)
    {
        MLOGD(YELLOW"remove main stream succeed\n"NONE);
        bAddMainStream = HI_TRUE;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret, "remove main stream");
    }

    s32Ret = HI_PDT_MEDIA_SetVencAttr(0, &stVencAttr);
    PDT_STATEMNG_CHECK_RET(s32Ret, "set payload type");

    /** Reset all(meida/RecTask) for setting param */
    PDT_STATEMNG_RecSnapStateResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_FALSE);

    /** add main video stream */
    HI_HANDLE AencHdl = pstMediaCfg->stAudioCfg.astAcapCfg[0].bEnable? 0 : HI_INVALID_HANDLE;
    HI_HANDLE VencHdl = pstMediaCfg->stVideoCfg.astVencCfg[0].VencHdl;

    if(bAddMainStream)
    {
        s32Ret = HI_LIVESVR_AddStream(VencHdl, AencHdl, PDT_STATEMNG_MAIN_STREAM_NAME);
        PDT_STATEMNG_CHECK_RET(s32Ret, "add main stream");
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, (HI_VOID *)&enSettingPlayloadType);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save PlayloadType param");
    MLOGD(YELLOW"save PlayloadType(%d) param succeed\n"NONE, enSettingPlayloadType);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecSnapStateSettingMsgProc(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr,
    HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->arg1)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_STATEMNG_RecSnapStateSetMediaMode(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_STATEMNG_RecSnapStateSetLapseInterval(pstMsg, pstMediaCfg, pstPhotoAttr);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_STATEMNG_RecSnapStateSetPlayloadType(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_STATEMNG_SetEV(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            //s32Ret = PDT_STATEMNG_SetExpTime(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_STATEMNG_SetISO(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_STATEMNG_SetWB(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_STATEMNG_SetMetry(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
            s32Ret = PDT_STATEMNG_SetAudio(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_STATEMNG_SetFlip(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_STATEMNG_SetOSD(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_DIS:
            s32Ret = PDT_STATEMNG_SetDIS(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_STATEMNG_SetLDC(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        default:
            MLOGE(RED"not support type(%d)\n"NONE, pstMsg->arg1);
            return HI_FAILURE;
    }

    return s32Ret;
}

/** enter Rec states */
HI_S32 PDT_STATEMNG_RecSnapStateEnter(HI_VOID *pvArg)
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

    if(HI_PDT_WORKMODE_RECSNAP != enEnterWorkMode)
    {
        MLOGE(RED"param err\n"NONE);
        return HI_FAILURE;
    }
    else
    {
        g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
        g_stSTATEMNGContext.bRunning = HI_FALSE;
    }

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get WorkModeCfg");

    g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl = PDT_STATEMNG_INVALID_HANDLE;
    /** get enter mediamode */
    g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode =
        stEnterWorkModeCfg.unModeAttr.stRecSnapAttr.stRecAttr.enMediaMode;
    g_stSTATEMNGContext.stRecMngCtx.u8FileMngObjCfgIdx =
        stEnterWorkModeCfg.unModeAttr.stRecSnapAttr.stRecAttr.u8FileTypeIdx;
    g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx =
        stEnterWorkModeCfg.unModeAttr.stRecSnapAttr.u32SnapFileTypeIndex;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enEnterWorkMode, g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode,
        &stEnterMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stEnterMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** generate RecMng atrribute */
    s32Ret = PDT_STATEMNG_GenerateRecMngAttr(enEnterWorkMode, &stEnterWorkModeCfg,
                                   &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate recordmng attr");

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** create RecMng task */
    s32Ret = HI_RECMNG_CreateTask(&(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
                                  &(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl));
    PDT_STATEMNG_CHECK_RET(s32Ret,"create RecMng task");

    /** generate PhotoMng attr */
    s32Ret = PDT_STATEMNG_GeneratePhotoAttr(enEnterWorkMode, &stEnterWorkModeCfg,
                                   &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate photo attr");

    /** publish switch workmode event */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, enEnterWorkMode, HI_TRUE, 0L, NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish switch workmode failed(0x%08X)\n"NONE, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_FAILURE;
    }
    else
    {
        MLOGD(YELLOW"publish switch to workmode(%s) msg(0x%08X) succeed\n"NONE,
            pstStateAttr->stState.name, HI_EVENT_STATEMNG_SWITCH_WORKMODE);
        g_stSTATEMNGContext.bInProgress= HI_FALSE;
    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    /** save poweron workmode */
    s32Ret = PDT_STATEMNG_SavePoweronWorkMode(g_stSTATEMNGContext.enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save poweron workmode");

    if(g_stSTATEMNGContext.bSuspend)
    {
#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(on)");

        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(on)");
#endif
        g_stSTATEMNGContext.bSuspend = HI_FALSE;
    }

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** exit Rec states */
HI_S32 PDT_STATEMNG_RecSnapStateExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    if(HI_PDT_WORKMODE_RECSNAP != enExitWorkMode)
    {
        MLOGE(RED"param error\n"NONE);
        return HI_FAILURE;
    }

    /** exit workmode directly when it is running */
    if(g_stSTATEMNGContext.bRunning)
    {
        /** stop PhotoMng task */
        s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop PhotoMng task");

        /** stop RecMng task */
        s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
            HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop RecMng task");
        MLOGD(YELLOW"stop photo/record task succeed\n"NONE);

        MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
        g_bRecStarted = HI_FALSE;
        /** file data sync */
        sync();

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        g_stSTATEMNGContext.bRunning = HI_FALSE;
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    }

    if(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl != PDT_STATEMNG_INVALID_HANDLE)
    {
        s32Ret = HI_RECMNG_DestroyTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"destroy RecMng task");
        g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl = PDT_STATEMNG_INVALID_HANDLE;
    }

    return s32Ret;
}

/** Rec states message process */
HI_S32 PDT_STATEMNG_RecSnapStateMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
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

    HI_BOOL bRecordStarted = HI_FALSE;
    HI_BOOL bPhotoStarted  = HI_FALSE;

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        {
             MLOGD(YELLOW"process start msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** DCF no need to check file scan completed, because the strategy changed.
             ** when UI recieved SD_AVAILABLE, it allow start a record task */

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp = HI_TRUE;

                /** start normal record task */
                g_stSTATEMNGContext.stRecMngCtx.bFileMngNewGrp = HI_TRUE;
                s32Ret = HI_RECMNG_StartTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    PDT_STATEMNG_RECMNG_OPERATION_TIMEOUT_MS);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"start Recmng task failed(0x%08X)\n"NONE, s32Ret);
                    goto PROC_ERROR;

                }
                else
                {
                    bRecordStarted = HI_TRUE;
                    MLOGD(YELLOW"start RecMng task succeed\n"NONE);
                }

                HI_MUTEX_LOCK(g_StartMutex);
                if(HI_FALSE == g_bRecStarted)
                {
                    MLOGI(YELLOW"wait record post cond signal first\n"NONE);
                    HI_COND_WAIT(g_StartCond, g_StartMutex);
                    MLOGI(YELLOW"RecMng task got name\n"NONE);
                }

                MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
                g_bRecStarted = HI_FALSE;

                HI_MUTEX_UNLOCK(g_StartMutex);

                /** set PhotoMng attr */
                s32Ret = HI_PHOTOMNG_SetPhotoAttr(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl,
                    &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"set PhotoMng task attr failed(0x%08X)\n"NONE, s32Ret);
                    goto PROC_ERROR;
                }

                /** start PhotoMng task */
                s32Ret = HI_PHOTOMNG_TaskStart(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"start PhotoMng task failed(0x%08X)\n"NONE, s32Ret);
                    goto PROC_ERROR;
                }
                else
                {
                    bPhotoStarted = HI_TRUE;
                    MLOGD(YELLOW"start PhotoMng task succeed\n"NONE);
                }

                MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                g_stSTATEMNGContext.bRunning = HI_TRUE;
                g_stSTATEMNGContext.bInProgress = HI_FALSE;
                MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            else
            {
                MLOGD(YELLOW"task already started, busing\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                 g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_STATEMNG_STOP:
        {
            MLOGD(YELLOW"process stop msg(%x)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                /** stop PhotoMng task */
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop PhotoMng task");

                /** stop RecMng task */
                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop RecMng task");

                MLOGD(YELLOW"stop photo/record task succeed\n"NONE);

                /** file data async */
                s32Ret = HI_async();
                PDT_STATEMNG_CHECK_RET(s32Ret,"async");

                MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
                g_bRecStarted = HI_FALSE;
                MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                g_stSTATEMNGContext.bRunning = HI_FALSE;
                g_stSTATEMNGContext.bInProgress = HI_FALSE;
                MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            else
            {
                MLOGD(YELLOW"no need stop\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            MLOGD(YELLOW"not process photo task end msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_STATEMNG_SETTING:
        {
            MLOGD(YELLOW"process setting msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                /** process all the setting type*/
                s32Ret = PDT_STATEMNG_RecSnapStateSettingMsgProc(pstMsg,
                    &(g_stSTATEMNGContext.stMediaCfg),
                    &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
                    &(g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr));
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"process failed(%d)\n"NONE, s32Ret);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
                else
                {
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"can not process, busy\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        /** need pre-process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
        {
            if(pstMsg->what == HI_EVENT_STORAGEMNG_DEV_UNPLUGED)
            {
                MLOGD(YELLOW"process unplug msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_POWEROFF)
            {
                MLOGD(YELLOW"process poweroff msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_FACTORY_RESET)
            {
                MLOGD(YELLOW"process factory reset msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_SWITCH_WORKMODE && pstMsg->arg2 != HI_PDT_WORKMODE_UPGRADE)
            {
                MLOGD(YELLOW"upload msg to Base state\n"NONE);
                return HI_PROCESS_MSG_UNHANDLER;
                break;
            }

            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                /** stop PhotoMng task */
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop PhotoMng task");

                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop RecMng task");

                /** file data sync */
                sync();

                MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
                g_bRecStarted = HI_FALSE;

                MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                g_stSTATEMNGContext.bRunning = HI_FALSE;
                MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                return HI_PROCESS_MSG_UNHANDLER;
            }
            else
            {
                MLOGD(YELLOW"upload msg to Base state\n"NONE);
                return HI_PROCESS_MSG_UNHANDLER;
            }
            break;
        }
        /** RecMng error and */
        case HI_EVENT_RECMNG_ERROR:
        /** PhotoMng error */
        case HI_EVENT_PHOTOMNG_ERROR:
        /** FileMng space full */
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"process err msg(0x%08X)\n"NONE, pstMsg->what);
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                /** stop PhotoMng task */
                s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop PhotoMng task");

                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                PDT_STATEMNG_CHECK_RET(s32Ret,"stop RecMng task");

                MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
                g_bRecStarted = HI_FALSE;

                MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                g_stSTATEMNGContext.bRunning = HI_FALSE;

                s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                            0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"publish stop msg failed(0x%08X)\n"NONE, s32Ret);
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_FAILURE;
                }
                MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        default:
        {
            MLOGD(YELLOW"upload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    g_stSTATEMNGContext.bInProgress = HI_FALSE;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return HI_PROCESS_MSG_RESULTE_OK;

PROC_ERROR:
    if(bPhotoStarted)
    {
        /** stop PhotoMng task */
        s32Ret = HI_PHOTOMNG_TaskStop(g_stSTATEMNGContext.stPhotoMngCtx.PhotoMngTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGD(YELLOW"stop photo task failed(0x%08X)\n"NONE, s32Ret);
        }
    }

    if(bRecordStarted)
    {

        /** stop RecMng task */
        s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
            HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGD(YELLOW"stop record task failed(0x%08X)\n"NONE, s32Ret);
        }

        MLOGI(YELLOW"reset g_bRecStarted to HI_FALSE\n"NONE);
        g_bRecStarted = HI_FALSE;
    }

    return HI_PROCESS_MSG_UNHANDLER;

}

/** init Rec states, add recsnap states to HFSM, init RecMng module */
HI_S32 PDT_STATEMNG_RecSnapStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stRecSnap =
        {{HI_PDT_WORKMODE_RECSNAP,
          PDT_STATEMNG_STATE_REC_SNAP,
          PDT_STATEMNG_RecSnapStateEnter,
          PDT_STATEMNG_RecSnapStateExit,
          PDT_STATEMNG_RecSnapStateMsgProc,
          NULL}};
    stRecSnap.stState.argv = &stRecSnap;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stRecSnap.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add Recsnap");

    HI_MUTEX_INIT_LOCK(g_StartMutex);
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&g_StartCond, &condattr);
    return s32Ret;
}

/** deinit RecMng module */
HI_S32 PDT_STATEMNG_RecSnapStateDeinit()
{
    HI_MUTEX_DESTROY(g_StartMutex);
    HI_COND_DESTROY(g_StartCond);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

