/**
 * @file      hi_product_statemng.c
 * @brief     Realize the interface of statemng module.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/20
 * @version   1.0
 */
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** statemng context */
PDT_STATEMNG_CONTEXT g_stSTATEMNGContext = {
    .Mutex = PTHREAD_MUTEX_INITIALIZER,
    .bInited = HI_FALSE,
    .enCurrentWorkMode = HI_PDT_WORKMODE_BUTT
};

/** message process callback function for HFSM module */
static HI_S32 PDT_STATEMNG_HfsmEventProc(HI_MW_PTR pvHfsmHdl,
    const HI_HFSM_EVENT_INFO_S* pstEventInfo)
{
    /** check paramerter */
    if((pvHfsmHdl != g_stSTATEMNGContext.pvHfsmHdl) || (NULL == pstEventInfo))
    {
        MLOGE(RED"param err\n"NONE);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** check event info */
    if(HI_HFSM_EVENT_UNHANDLE_MSG == pstEventInfo->enEventCode)
    {
        HI_MESSAGE_S *pstMsg =
            pstEventInfo->unEventInfo.stunHandlerMsgInfo.pstunHandlerMsg;
        MLOGD(YELLOW"unhandle msg(0x%08X)\n"NONE, pstMsg->what);

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        if(HI_TRUE == g_stSTATEMNGContext.bInProgress)
        {
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_FALSE);
            g_stSTATEMNGContext.bInProgress = HI_FALSE;
        }
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    }
    else if(HI_HFSM_EVENT_TRANSTION_ERROR == pstEventInfo->enEventCode)
    {
        HI_HFSM_TRANSTION_INFO_S*  pstTranstionInfo =
            (HI_HFSM_TRANSTION_INFO_S*)&(pstEventInfo->unEventInfo.stTranstionInfo);
        MLOGD(YELLOW"transition failed, orig(%s), dest(%s), curr(%s)\n"NONE,
            pstTranstionInfo->pstOrgState?(pstTranstionInfo->pstOrgState)->name:"null",
            pstTranstionInfo->pstDestState?(pstTranstionInfo->pstDestState)->name:"null",
            pstTranstionInfo->pstCurrentState?(pstTranstionInfo->pstCurrentState)->name:"null");

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        if(HI_TRUE == g_stSTATEMNGContext.bInProgress)
        {
            PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, pstTranstionInfo->pstOrgState?((pstTranstionInfo->pstDestState)->stateID):(HI_PDT_WORKMODE_BUTT),
                HI_FALSE, 0L, NULL, 0);
            g_stSTATEMNGContext.bInProgress = HI_FALSE;
        }
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    }
    else
    {
        MLOGE(RED"not support\n"NONE);
        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        if(HI_TRUE == g_stSTATEMNGContext.bInProgress)
        {
            g_stSTATEMNGContext.bInProgress = HI_FALSE;
        }
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    }
    return HI_SUCCESS;
}

/** event process callback function for EventHub module */
static HI_S32 PDT_STATEMNG_EventProc(HI_EVENT_S* pstEvent, HI_VOID* pvArg)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstEvent, "pstEvent");

    /** check whether it has been intialized or not */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    if(HI_FALSE == g_stSTATEMNGContext.bInited)
    {
        MLOGD("not inited, discard event(0x%08X)\n", pstEvent->EventID);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    /** change event to message, push it to state machine queue */
    HI_MESSAGE_S stMsg;
    memset(&stMsg, 0, sizeof(HI_MESSAGE_S));
    stMsg.what = pstEvent->EventID;
    stMsg.arg1 = pstEvent->arg1;
    stMsg.arg2 = pstEvent->arg2;
    stMsg.s32Result = pstEvent->s32Result;
    memcpy_s(&stMsg.aszPayload, MESSAGE_PAYLOAD_LEN, pstEvent->aszPayload, EVENT_PAYLOAD_LEN);
    s32Ret = HI_HFSM_SendAsyncMessage(g_stSTATEMNGContext.pvHfsmHdl, (HI_MESSAGE_S *)&stMsg);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"send msg(0x%08X)(from eventhub) failed(0x%08X)\n"NONE, stMsg.what, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_STORAGEMNG_EINTER;
    }

    MLOGI(YELLOW"send msg(0x%08X)(from eventhub) succeed\n"NONE, stMsg.what);
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return s32Ret;
}

/** subscribe event from EventHub module */
static HI_S32 PDT_STATEMNG_SubscribeEvents(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** create subscriber */
    HI_MW_PTR pvSubscriberID = NULL;
    HI_SUBSCRIBER_S stSubscriber = {PDT_STATEMNG_MODULE_NAME,
        PDT_STATEMNG_EventProc, NULL, HI_FALSE};
    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &pvSubscriberID);
    PDT_STATEMNG_CHECK_RET(s32Ret,"create subscriber");

    /** array of event to subscribe */
    HI_EVENT_ID aEvents[] =
    {
        /** StorageMng module */
        HI_EVENT_STORAGEMNG_MOUNTED,
        HI_EVENT_STORAGEMNG_DEV_UNPLUGED,
        HI_EVENT_STORAGEMNG_DEV_ERROR,
        HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
        HI_EVENT_STORAGEMNG_FS_EXCEPTION,
        HI_EVENT_STORAGEMNG_MOUNT_FAILED,
        /** RecMng module */
        HI_EVENT_RECMNG_FILE_END,
        HI_EVENT_RECMNG_ERROR,
        /** FileMng module */
        HI_EVENT_FILEMNG_SPACE_FULL,
        /** PhotoMng module */
        HI_EVENT_PHOTOMNG_TASK_END,
        HI_EVENT_PHOTOMNG_FILE_END,
        HI_EVENT_PHOTOMNG_ERROR,
        /** scene module */
        HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE,
        /** upgrade moudle*/
        HI_EVENT_UPGRADE_NEWPKT
    };

    /** subscribe event */
    HI_S32 i = 0;
    for(i = 0; i < PDT_STATEMNG_ARRAY_SIZE(aEvents); i++)
    {
        HI_CHAR aszText[HI_APPCOMM_MAX_PATH_LEN] = {0};
        snprintf(aszText, HI_APPCOMM_MAX_PATH_LEN,
                 "subscribe event(%#x), index(%d)", aEvents[i], i);

        s32Ret = HI_EVTHUB_Subscribe(pvSubscriberID, aEvents[i]);
        PDT_STATEMNG_CHECK_RET(s32Ret, aszText);
    }

    return s32Ret;
}

/** init global context structure */
static HI_S32 PDT_STATEMNG_ContextInit(const HI_PDT_STATEMNG_CONFIG_S* pstStatemngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** init context structure */
    g_stSTATEMNGContext.enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    g_stSTATEMNGContext.bInProgress = HI_FALSE;
    g_stSTATEMNGContext.bPoweronAction = HI_FALSE;
    g_stSTATEMNGContext.stRecMngCtx.bFileMngNewGrp = HI_TRUE;
    g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp = HI_TRUE;
    g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_BUTT;
    g_stSTATEMNGContext.stStatemngCfg.pfnExitCB = pstStatemngCfg->pfnExitCB;
    g_stSTATEMNGContext.stStatemngCfg.pfnFormatPreProc = pstStatemngCfg->pfnFormatPreProc;

    g_stSTATEMNGContext.stSpotMetryParam.stPoint.s32X = 50;/** spot metry defualt setting */
    g_stSTATEMNGContext.stSpotMetryParam.stPoint.s32Y = 50;
    g_stSTATEMNGContext.stSpotMetryParam.stSize.u32Width =
        PDT_STATEMNG_SPOT_METERING_WIDTH_PERCENT;
    g_stSTATEMNGContext.stSpotMetryParam.stSize.u32Height =
        PDT_STATEMNG_SPOT_METERING_HEIGHT_PERCENT;

#ifdef CONFIG_RAWCAP_ON
    g_stSTATEMNGContext.RawCapTaskHdl = -1;
    memset(g_stSTATEMNGContext.szPrefix, 0, HI_APPCOMM_MAX_FILENAME_LEN);
#endif
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&(g_stSTATEMNGContext.stStorageMngCfg));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get storage cfg failed\n"NONE);
    }
    return s32Ret;
}

/** create HFSM instance */
static HI_S32 PDT_STATEMNG_CreateHFSMInstance(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HFSM_ATTR_S stHfsmAttr;
    stHfsmAttr.fnHfsmEventCallback = PDT_STATEMNG_HfsmEventProc;
    stHfsmAttr.u32StateMaxAmount = HI_PDT_WORKMODE_BUTT + 1;/** all workmode and base state */
    stHfsmAttr.u32MessageQueueSize = PDT_STATEMNG_MAX_MSG_QUEUE_SIZE;

    s32Ret = HI_HFSM_Create(&stHfsmAttr, &(g_stSTATEMNGContext.pvHfsmHdl));
    PDT_STATEMNG_CHECK_RET(s32Ret,"create HFSM instance");

    return s32Ret;
}

/** destory HFSM instance */
static HI_S32 PDT_STATEMNG_DestoryHFSMInstance(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_HFSM_Destroy(g_stSTATEMNGContext.pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"destory HFSM instance");

    return s32Ret;
}

/** init and add all states */
static HI_S32 PDT_STATEMNG_InitStates(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_STATEMNG_BaseStateInit();/** base state must be init at first*/
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Base");

    s32Ret = PDT_STATEMNG_RecStatesInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Rec");

    s32Ret = PDT_STATEMNG_PhotoStatesInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Photo");

    s32Ret = PDT_STATEMNG_RecSnapStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init RecSnap");

    s32Ret = PDT_STATEMNG_HDMIPreviewStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init HDMI Preview");

    s32Ret = PDT_STATEMNG_PlaybackStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Playback");

    s32Ret = PDT_STATEMNG_SuspendStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Suspend");

    s32Ret = PDT_STATEMNG_UVCStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init UVC");

    s32Ret = PDT_STATEMNG_USBStorageStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init USB Storage");

    s32Ret = PDT_STATEMNG_HDMIPlaybackStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init HDMI Playback");

    s32Ret = PDT_STATEMNG_UpgradeStateInit(&(g_stSTATEMNGContext.stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,"init Upgrade");

    return s32Ret;
}

/** deinit all states */
static HI_S32 PDT_STATEMNG_DeinitStates(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_STATEMNG_RecSnapStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit RecSnap");

    s32Ret = PDT_STATEMNG_RecStatesDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Rec");

    s32Ret = PDT_STATEMNG_PhotoStatesDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Photo");

    s32Ret = PDT_STATEMNG_HDMIPreviewStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit HDMI Preview");

    s32Ret = PDT_STATEMNG_PlaybackStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Playback");

    s32Ret = PDT_STATEMNG_SuspendStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Suspend");

    s32Ret = PDT_STATEMNG_UVCStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit UVC");

    s32Ret = PDT_STATEMNG_USBStorageStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit USB Storage");

    s32Ret = PDT_STATEMNG_HDMIPlaybackStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit HDMI Playback");

    s32Ret = PDT_STATEMNG_UpgradeStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Upgrade");

    s32Ret = PDT_STATEMNG_BaseStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Base");/** base state must be deinit at last*/

    return s32Ret;
}

/** init media pipe */
static HI_S32 PDT_STATEMNG_InitMediaPipe(HI_PDT_WORKMODE_E enPoweronWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** init scene msg client */
    s32Ret = HI_PDT_SCENE_MSG_CLIENT_Init();
    PDT_STATEMNG_CHECK_RET(s32Ret,"init scene msg client");

#ifndef CONFIG_RELEASE
    /** init scene module */
    s32Ret = HI_PDT_SCENE_Init();
    PDT_STATEMNG_CHECK_RET(s32Ret,"init scene");
#endif

    return s32Ret;
}

/** deinit media pipe */
static HI_S32 PDT_STATEMNG_DeinitMediaPipe(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //Lorin Modity
    // s32Ret = HI_PDT_MEDIA_VideoOutStop();
    // if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    // {
    //     MLOGD(YELLOW"VO not inited\n"NONE);
    // }
    // else
    // {
    //     PDT_STATEMNG_CHECK_RET(s32Ret,"stop VO");
    // }

    // s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
    // if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    // {
    //     MLOGD(YELLOW"VO not inited\n"NONE);
    // }
    // else
    // {
    //     PDT_STATEMNG_CHECK_RET(s32Ret,"deinit VO");
    // }

    s32Ret = HI_PDT_MEDIA_AudioOutStop();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"AO not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop AO");
    }

    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"AO not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Audio");
    }

    s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"Video not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit Video");
    }

    /** deinit scene module */
    s32Ret = HI_PDT_SCENE_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit scene");

    return s32Ret;
}

/** activate HFSM */  //Lorin Modity
static HI_S32 PDT_STATEMNG_ActivateHFSM(HI_PDT_WORKMODE_E enPoweronWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** set initial state for HFSM */
    HI_CHAR szStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(enPoweronWorkmode, szStateName, HI_STATE_NAME_LEN);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode name");

    s32Ret = HI_HFSM_SetInitialState(g_stSTATEMNGContext.pvHfsmHdl, szStateName);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set HFSM initial state");

    /** active HFSM */
    s32Ret = HI_HFSM_Start(g_stSTATEMNGContext.pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"start HFSM");

    return s32Ret;
}

/** deactive HFSM */
static HI_S32 PDT_STATEMNG_DeactiveHFSM(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** stop HFSM instance */
    s32Ret = HI_HFSM_Stop(g_stSTATEMNGContext.pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"stop HFSM");

    return s32Ret;
}

/** process poweron action */
static HI_S32 PDT_STATEMNG_ProcessPoweronAction(HI_PDT_POWERON_ACTION_E enPoweronAction,
    HI_PDT_WORKMODE_E enPoweronWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check sd state */
    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    s32Ret = HI_STORAGEMNG_GetState(g_stSTATEMNGContext.stStorageMngCfg.szMntPath, &enState);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get storage state");
    MLOGD(YELLOW"storage state(%d)\n"NONE, enState);

    /** check upgrade package */
    if(HI_STORAGE_STATE_MOUNTED == enState)
    {
        /** init filemng */
        s32Ret = PDT_STATEMNG_InitFileMng();
        PDT_STATEMNG_CHECK_RET(s32Ret, "init filemng");

        /** upgrade */
        if(1 != PDT_STATEMNG_UpgradeSrchNewPkt())
        {
            MLOGD(YELLOW"PDT_STATEMNG_UpgradeSrchNewPkt no pkt(0x%08X)\n"NONE, s32Ret);
            MLOGD(YELLOW"set disk state(HI_TRUE)\n"NONE);
            s32Ret = PDT_STATEMNG_FileMngProc();
            PDT_STATEMNG_CHECK_RET(s32Ret,"PDT_STATEMNG_FileMngProc");

            s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SD_AVAILABLE,
                        0, 0, HI_FALSE, 0L, NULL, 0);
            PDT_STATEMNG_CHECK_RET(s32Ret,"publish sd available event");
        }
        else
        {
            if(HI_PDT_POWERON_ACTION_IDLE != enPoweronAction)
            {
                PDT_STATEMNG_StopEncoder(&g_stSTATEMNGContext.stMediaCfg, g_stSTATEMNGContext.enCurrentWorkMode);
            }
            return HI_SUCCESS;
        }
    }
    else
    {
        MLOGD(YELLOW"NO set disk state(HI_TRUE)\n"NONE);
    }
    if(HI_PDT_POWERON_ACTION_IDLE != enPoweronAction)
    {
        switch(enState)
        {
            case HI_STORAGE_STATE_MOUNTED:
            {
                HI_MESSAGE_S stMessage;
                memset(&stMessage, 0 , sizeof(HI_MESSAGE_S));
                stMessage.what = HI_EVENT_STATEMNG_START;
                stMessage.arg2 = enPoweronWorkmode;

                /** send trigger/start message for poweron action */
                s32Ret = HI_HFSM_SendAsyncMessage(g_stSTATEMNGContext.pvHfsmHdl, &stMessage);
                PDT_STATEMNG_CHECK_RET(s32Ret,"send msg(start) for poweron action");
                break;
            }
            case HI_STORAGE_STATE_DEV_CONNECTING:
            case HI_STORAGE_STATE_FS_CHECKING:
            {
                g_stSTATEMNGContext.bPoweronAction = HI_TRUE;
                MLOGD("set bPoweronAction(HI_TRUE)\n");/** do poweron action until sd mounted */
                break;
            }
            case HI_STORAGE_STATE_DEV_UNPLUGGED:
            {
                MLOGD(YELLOW"no SD, need stop encoding(LiteOS)\n"NONE);
                g_stSTATEMNGContext.bPreRecNoSD = HI_TRUE;
                break;
            }
            default:
            {
                MLOGD("no process poweron action\n");
                break;
            }
        }
    }

    return s32Ret;
}

/** statemng initialization */
HI_S32 HI_PDT_STATEMNG_Init(const HI_PDT_STATEMNG_CONFIG_S* stStatemngCfg)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_NULL_PTR(stStatemngCfg, "stStatemngCfg");

    /** check whether it has been intialized or not */
    if(HI_TRUE == g_stSTATEMNGContext.bInited)
    {
        MLOGE(RED"already inited\n"NONE);
        return HI_PDT_STATEMNG_EINITIALIZED;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    /** init global context structure */
    s32Ret = PDT_STATEMNG_ContextInit(stStatemngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"init context");

    /** create HFSM instance */
    s32Ret = PDT_STATEMNG_CreateHFSMInstance();
    PDT_STATEMNG_CHECK_RET(s32Ret,"create HFSM instance");

    /** init usb ctrl module*/
    // s32Ret = HI_PDT_USBCTRL_Init();
    // PDT_STATEMNG_CHECK_RET(s32Ret,"init usb ctrl");

    /** init all States */
    s32Ret = PDT_STATEMNG_InitStates();
    PDT_STATEMNG_CHECK_RET(s32Ret,"add all states");

    /** init liveserver */
    HI_LIVESVR_MEDIA_OPERATE_S stLivesrvMediaOps;
    stLivesrvMediaOps.pfnGetVideoInfo = HI_PDT_MEDIA_GetVideoInfo;
    stLivesrvMediaOps.pfnGetAudioInfo = HI_PDT_MEDIA_GetAudioInfo;
    stLivesrvMediaOps.pfnVencStart = HI_PDT_MEDIA_VencStart;
    stLivesrvMediaOps.pfnVencStop = HI_PDT_MEDIA_VencStop;
    stLivesrvMediaOps.pfnAencStart = HI_PDT_MEDIA_AencStart;
    stLivesrvMediaOps.pfnAencStop = HI_PDT_MEDIA_AencStop;

    s32Ret = HI_LIVESVR_Init(PDT_STATEMNG_LIVESVR_MAX_CONNECT_CNT, &stLivesrvMediaOps);
    PDT_STATEMNG_CHECK_RET(s32Ret, "init livesrv");

    /** subscribe event from EventHub module */
    s32Ret = PDT_STATEMNG_SubscribeEvents();
    PDT_STATEMNG_CHECK_RET(s32Ret,"subscribe events");

    /** generate poweron workmode */
    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    HI_PDT_POWERON_ACTION_E enPoweronAction = HI_PDT_POWERON_ACTION_BUTT;
    s32Ret = HI_PDT_STATEMNG_GeneratePoweronWorkmode(&enPoweronAction, &enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate poweron workmode");

    /** init media pipe */
    s32Ret = PDT_STATEMNG_InitMediaPipe(enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"init media pipe");

    /** activate HFSM */
    s32Ret = PDT_STATEMNG_ActivateHFSM(enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"active HFSM");

    /** process PoweronAction */
    s32Ret = PDT_STATEMNG_ProcessPoweronAction(enPoweronAction, enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"process PoweronAction");

    g_stSTATEMNGContext.bInited = HI_TRUE;
    HI_TIME_STAMP;

    return s32Ret;
}

/** statemng deinitialization */
HI_S32 HI_PDT_STATEMNG_Deinit()
{
    /** check whether it has been deintialized or not */
    if(HI_FALSE == g_stSTATEMNGContext.bInited)
    {
        MLOGE(RED"already deinited\n"NONE);
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit liveserver */
    s32Ret = HI_LIVESVR_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret, "deinit livesrv");

    /** deinit all states */
    s32Ret = PDT_STATEMNG_DeinitStates();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit all states");

    /** deinit usb ctrl module*/
    s32Ret = HI_PDT_USBCTRL_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit usb ctrl");

    /** deinit media pipe */
    s32Ret = PDT_STATEMNG_DeinitMediaPipe();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit media pipe");

    /** deactive HFSM */
    s32Ret = PDT_STATEMNG_DeactiveHFSM();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deactive HFSM");

    /** destory HFSM */
    s32Ret = PDT_STATEMNG_DestoryHFSMInstance();
    PDT_STATEMNG_CHECK_RET(s32Ret,"destory HFSM instance");

    g_stSTATEMNGContext.bInited = HI_FALSE;
    return s32Ret;
}

/** get current wokemode state */
HI_S32 HI_PDT_STATEMNG_GetState(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkModeState)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstWorkModeState, "pstWorkModeState");

    /** check whether it has been intialized or not */
    if(HI_FALSE == g_stSTATEMNGContext.bInited)
    {
        MLOGD(YELLOW"not inited\n"NONE);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    memset(pstWorkModeState, 0, sizeof(HI_PDT_STATEMNG_WORKMODE_STATE_S));

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    HI_CHAR szStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(g_stSTATEMNGContext.enCurrentWorkMode,
        szStateName, HI_STATE_NAME_LEN);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get workmode name failed(0x%08X)\n"NONE, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_STORAGEMNG_EINTER;
    }

    pstWorkModeState->enWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
    pstWorkModeState->bRunning = g_stSTATEMNGContext.bRunning;
    pstWorkModeState->bStateMngInProgress = g_stSTATEMNGContext.bInProgress;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    return s32Ret;
}

/** send message to statemng with parameter, synchronize UI and NetCtrl modules */
HI_S32 HI_PDT_STATEMNG_SendMessage(const HI_MESSAGE_S *pstMsg)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstMsg, "pstMsg");

    /** check whether it has been intialized or not */
    if(HI_FALSE == g_stSTATEMNGContext.bInited)
    {
        MLOGE(RED"not inited\n"NONE);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    /** check whether it was in progress or not */
    if(HI_TRUE == g_stSTATEMNGContext.bInProgress)
    {
        MLOGD(YELLOW"module is InProgress\n"NONE);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_PDT_STATEMNG_EINPROGRESS;
    }

    HI_TIME_CLEAN;
    s32Ret = HI_HFSM_SendAsyncMessage(g_stSTATEMNGContext.pvHfsmHdl, (HI_MESSAGE_S *)pstMsg);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"send msg(0x%08X)(from Terminal) failed(0x%08X)\n"NONE, pstMsg->what, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_STORAGEMNG_EINTER;
    }

    MLOGI(YELLOW"send msg(0x%08X)(from Terminal) succeed\n"NONE, pstMsg->what);
    g_stSTATEMNGContext.bInProgress = HI_TRUE;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return s32Ret;
}

/** generate poweron workmode */
HI_S32 HI_PDT_STATEMNG_GeneratePoweronWorkmode(HI_PDT_POWERON_ACTION_E *penPoweronAction,
    HI_PDT_WORKMODE_E *penPoweronWorkmode)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_NULL_PTR(penPoweronAction, "penPoweronAction");
    PDT_STATEMNG_CHECK_NULL_PTR(penPoweronWorkmode, "penPoweronWorkmode");

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_ACTION, penPoweronAction);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get poweron Action");
    MLOGD(YELLOW"poweron action(%d)\n"NONE, *penPoweronAction);

    switch(*penPoweronAction)
    {
        case HI_PDT_POWERON_ACTION_NORM_REC:
            *penPoweronWorkmode = HI_PDT_WORKMODE_NORM_REC;
            break;
        case HI_PDT_POWERON_ACTION_LOOP_REC:
            *penPoweronWorkmode = HI_PDT_WORKMODE_LOOP_REC;
            break;
        case HI_PDT_POWERON_ACTION_LPSE_REC:
            *penPoweronWorkmode = HI_PDT_WORKMODE_LPSE_REC;
            break;
        case HI_PDT_POWERON_ACTION_SLOW_REC:
            *penPoweronWorkmode = HI_PDT_WORKMODE_SLOW_REC;
            break;
        case HI_PDT_POWERON_ACTION_RECSNAP:
            *penPoweronWorkmode = HI_PDT_WORKMODE_RECSNAP;
            break;
        case HI_PDT_POWERON_ACTION_LPSE_PHOTO:
            *penPoweronWorkmode = HI_PDT_WORKMODE_LPSE_PHOTO;
            break;
        case HI_PDT_POWERON_ACTION_SING_PHOTO:
            *penPoweronWorkmode = HI_PDT_WORKMODE_SING_PHOTO;
            break;
        case HI_PDT_POWERON_ACTION_IDLE:
        default:
        {
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, penPoweronWorkmode);
            PDT_STATEMNG_CHECK_RET(s32Ret,"get poweron workmode");
            break;
        }
    }

    MLOGD(YELLOW"poweron workmode(%d)\n"NONE, *penPoweronWorkmode);
    return s32Ret;
}

/** register all event published from this module */
HI_S32 HI_PDT_STATEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SWITCH_WORKMODE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_START);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_STOP);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SETTING);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_POWEROFF);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_AVAILABLE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_UNAVAILABLE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_FORMAT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_VO_SWITCH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_FACTORY_RESET);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_DEBUG_SWITCH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_START);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_DONOTUPGRADE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_ABNORMAL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_FAILED);

    PDT_STATEMNG_CHECK_RET(s32Ret, "register event");
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
