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
#include "hi_system.h"
#include "hi_liveserver.h"
#ifdef CONFIG_AHD_ON
#include "hi_ahdmng.h"
#endif
#ifdef CONFIG_MOTIONDETECT_ON
#include "hi_videodetect.h"
#endif
#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#include "hi_tempcycle.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** message process callback function for HFSM module */
static HI_S32 PDT_STATEMNG_HfsmEventProc(HI_MW_PTR pvHfsmHdl,
                                         const HI_HFSM_EVENT_INFO_S* pstEventInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check paramerter */
    if((pvHfsmHdl != pstStateMngCtx->pvHfsmHdl) || (HI_NULL == pstEventInfo))
    {
        MLOGE(RED"parameter error\n\n"NONE);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** check event info */
    if(HI_HFSM_EVENT_UNHANDLE_MSG == pstEventInfo->enEventCode)
    {
        HI_MESSAGE_S *pstMsg =
            pstEventInfo->unEventInfo.stunHandlerMsgInfo.pstunHandlerMsg;
        MLOGD(RED"HI_HFSM_EVENT_UNHANDLE_MSG info:\n"NONE);
        MLOGD(RED"message id: %d\n\n"NONE, pstMsg->what);

        MUTEX_LOCK(pstStateMngCtx->Mutex);
        if(HI_TRUE == pstStateMngCtx->bInProgress)
        {
            pstStateMngCtx->bInProgress = HI_FALSE;
            PDT_STATEMNG_PublishEvent(pstMsg, HI_FAILURE);
        }
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    }
    else if(HI_HFSM_EVENT_TRANSTION_ERROR == pstEventInfo->enEventCode)
    {
        MLOGD(RED"HI_HFSM_EVENT_TRANSTION_ERROR \n"NONE);
        MUTEX_LOCK(pstStateMngCtx->Mutex);
        if(HI_TRUE == pstStateMngCtx->bInProgress)
        {
            HI_EVENT_S stEvent;
            memset(&stEvent, 0 , sizeof(HI_EVENT_S));
            stEvent.EventID = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stEvent.arg2 = (pstEventInfo->unEventInfo.stTranstionInfo.pstDestState)->stateID;
            stEvent.s32Result = HI_FAILURE;
            pstStateMngCtx->bInProgress = HI_FALSE;
            s32Ret = HI_EVTHUB_Publish(&stEvent);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE(RED"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event failed\n\n"NONE);
            }
            else
            {
                MLOGD(YELLOW"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE(%x), arg2(%s), switch failed\n\n"NONE,
                            HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                            (pstEventInfo->unEventInfo.stTranstionInfo.pstDestState)->name);
            }

        }
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    }
    else
    {
        MLOGD(RED"pstEventInfo enEventCode[%d] not support\n"NONE,pstEventInfo->enEventCode);
        MUTEX_LOCK(pstStateMngCtx->Mutex);
        if(HI_TRUE == pstStateMngCtx->bInProgress)
        {
            pstStateMngCtx->bInProgress = HI_FALSE;
        }
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    }
    return HI_SUCCESS;
}

/** event process callback function for EventHub module */
static HI_S32 PDT_STATEMNG_EventProc(HI_EVENT_S* pstEvent, HI_VOID* pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check paramerter */
    PDT_STATEMNG_CHECK_POINTER(pstEvent,HI_PDT_STATEMNG_ENULLPTR, "pstEvent");

    /** check whether it has been intialized or not */
    MUTEX_LOCK(pstStateMngCtx->Mutex);
    if(HI_FALSE == pstStateMngCtx->bInited)
    {
        MLOGE(RED"StateMng module has not been inited\n\n"NONE);
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    /** push message to state machine queue */
    s32Ret = HI_HFSM_SendAsyncMessage(pstStateMngCtx->pvHfsmHdl, (HI_MESSAGE_S *)pstEvent);
    PDT_STATEMNG_CHECK_RET_WITH_UNLOCK(s32Ret,HI_PDT_STATEMNG_EINTER,"send message to HFSM(from eventhub)");

    MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    return s32Ret;
}

/** subscribe event from EventHub module */
static HI_S32 PDT_STATEMNG_SubscribeEvents(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** create subscriber */
    HI_MW_PTR pvSubscriberID = NULL;
    HI_SUBSCRIBER_S stSubscriber = {PDT_STATEMNG_MODULE_NAME,
                                    PDT_STATEMNG_EventProc,
                                    NULL, HI_FALSE};
    s32Ret = HI_EVTHUB_CreateSubscriber(&stSubscriber, &pvSubscriberID);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"create subscriber");

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
        HI_EVENT_STORAGEMNG_DEV_CONNECTING,
        /** RecMng module */
        HI_EVENT_RECMNG_FILE_END,
        HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END,
        HI_EVENT_RECMNG_ERROR,
        HI_EVENT_RECMNG_TASK_AUTO_STOP,

        /** PhotoMng module */
        HI_EVENT_PHOTOMNG_TASK_END,
        HI_EVENT_PHOTOMNG_FILE_END,
        HI_EVENT_PHOTOMNG_ERROR,
        /**upgrade moudle*/
        HI_EVENT_UPGRADE_NEWPKT,
        HI_EVENT_UPGRADE_SUCCESS,
        HI_EVENT_UPGRADE_FAILURE,
#ifdef CONFIG_AHD_ON
        HI_EVENT_AHDMNG_PLUG_STATUS,
#endif
#ifdef CONFIG_TEMPRATURE_CYCLE_ON
        HI_EVENT_TEMPCYCLE_TEMP_CHANGE,
#endif

    };

    /** subscribe event */
    HI_S32 i = 0;
    for(i = 0; i<PDT_STATEMNG_ARRAY_SIZE(aEvents); i++)
    {
        HI_CHAR aszText[HI_APPCOMM_MAX_PATH_LEN] = {0};
        snprintf(aszText,
                 HI_APPCOMM_MAX_PATH_LEN,
                 "subscribe event(%#x), index(%d)",
                 aEvents[i], i);

        s32Ret = HI_EVTHUB_Subscribe(pvSubscriberID, aEvents[i]);
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, aszText);
    }

    return s32Ret;
}


/** create HFSM instance */
static HI_S32 PDT_STATEMNG_CreateHFSMInstance(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HFSM_ATTR_S stHfsmAttr;
    memset(&stHfsmAttr, 0, sizeof(HI_HFSM_ATTR_S));
    stHfsmAttr.fnHfsmEventCallback = PDT_STATEMNG_HfsmEventProc;
    stHfsmAttr.u32StateMaxAmount = HI_PDT_WORKMODE_BUTT+1;/**< include base */
    stHfsmAttr.u32MessageQueueSize = 64;
    s32Ret = HI_HFSM_Create(&stHfsmAttr, &(PDT_STATEMNG_GetCtx()->pvHfsmHdl));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM instance create");

    return s32Ret;
}

/** destory HFSM instance */
static HI_S32 PDT_STATEMNG_DestoryHFSMInstance(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_HFSM_Destroy(PDT_STATEMNG_GetCtx()->pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM destory");

    return s32Ret;
}

/** init and add all states */
static HI_S32 PDT_STATEMNG_InitStates(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    s32Ret = PDT_STATEMNG_BaseStateInit();/** base state must be init at first*/
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Base state init");

    s32Ret = PDT_STATEMNG_RecStatesInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Rec states init");

    s32Ret = PDT_STATEMNG_PhotoStatesInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"photo states init");

    s32Ret = PDT_STATEMNG_PlaybackStateInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Playback state init");

    s32Ret = PDT_STATEMNG_UVCStateInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"UVC state init");

    s32Ret = PDT_STATEMNG_USBStorageStateInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"USB Storage state init");

    s32Ret = PDT_STATEMNG_ParkingRecStatesInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Parking recv state init");

    s32Ret = PDT_STATEMNG_UpgradeStateInit(&(pstStateMngCtx->stBase));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"init Upgrade");

    return s32Ret;
}

/** deinit all states */
static HI_S32 PDT_STATEMNG_DeinitStates(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_STATEMNG_RecStatesDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Rec states deinit");

    s32Ret = PDT_STATEMNG_PlaybackStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Playback state deinit");

    s32Ret = PDT_STATEMNG_UVCStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"UVC state deinit");

    s32Ret = PDT_STATEMNG_USBStorageStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"USB Storage state deinit");

    s32Ret = PDT_STATEMNG_BaseStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Base state deinit");/** base state must be deinit at last*/

    s32Ret = PDT_STATEMNG_UpgradeStateDeinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"deinit Upgrate");

    return s32Ret;
}

/** deinit media pipe */
static HI_S32 PDT_STATEMNG_DeinitMedia(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PDT_MEDIA_VideoOutStop();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"VideoOut not inited\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"video out stop");
    }

    s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"VideoOut not inited\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"video out deinit");
    }

    s32Ret = HI_PDT_MEDIA_AudioOutStop();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"AudioOut not inited\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"audio out stop");
    }

    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"AudioOut not inited\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"audio out deinit");
    }

    s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"Video not inited\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"video deinit");
    }

    return s32Ret;
}

/** activate HFSM */
static HI_S32 PDT_STATEMNG_ActivateHFSM(HI_PDT_WORKMODE_E enWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** set initial state for HFSM */
    HI_CHAR szStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(enWorkmode, szStateName, HI_STATE_NAME_LEN);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"change workmode to state");

    s32Ret = HI_HFSM_SetInitialState(PDT_STATEMNG_GetCtx()->pvHfsmHdl, szStateName);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM set initial state");

    /** active HFSM */
    s32Ret = HI_HFSM_Start(PDT_STATEMNG_GetCtx()->pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM start");

    return s32Ret;
}

/** deactive HFSM */
static HI_S32 PDT_STATEMNG_DeactiveHFSM(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** stop HFSM instance */
    s32Ret = HI_HFSM_Stop(PDT_STATEMNG_GetCtx()->pvHfsmHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM stop");

    return s32Ret;
}

/**poweron  process*/
static HI_S32 PDT_STATEMNG_PoweronProc(HI_PDT_WORKMODE_E enPoweronWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check sd state */
    s32Ret = HI_STORAGEMNG_GetState(pstStateMngCtx->stStorageMngCfg.szMntPath, &pstStateMngCtx->enPowerOnState);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"get storage state");
    MLOGI("PowerOn Storage State is %d.\n",pstStateMngCtx->enPowerOnState);
    HI_MESSAGE_S stMessage;
    memset(&stMessage, 0 , sizeof(HI_MESSAGE_S));

    if(HI_STORAGE_STATE_MOUNTED == pstStateMngCtx->enPowerOnState)
    {
        PDT_STATEMNG_MountedMsgProc(enPoweronWorkmode);
    }
    else
    {
        if((HI_STORAGE_STATE_DEV_UNPLUGGED == pstStateMngCtx->enPowerOnState) ||
            (HI_STORAGE_STATE_DEV_ERROR == pstStateMngCtx->enPowerOnState) ||
            (HI_STORAGE_STATE_MOUNT_FAILED == pstStateMngCtx->enPowerOnState))
        {
            MLOGI(YELLOW"power on check storage state[%d] is plug out or error\n"NONE,pstStateMngCtx->enPowerOnState);

            if(HI_PDT_WORKMODE_PARKING_REC == enPoweronWorkmode)
            {
                MLOGI(YELLOW"need power off\n"NONE);
                stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
                stMessage.arg2 = enPoweronWorkmode;
                s32Ret = HI_HFSM_SendAsyncMessage(pstStateMngCtx->pvHfsmHdl, &stMessage);
                PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"send power off message for poweron action");
            }
            else
            {
                s32Ret = PDT_STATEMNG_SetUsbMode();
                PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
            }
        }
    }

    return HI_SUCCESS;
}

/** statemng initialization */
HI_S32 HI_PDT_STATEMNG_Init(const HI_PDT_STATEMNG_CONFIG_S* pstStatemngCfg)
{
    /** check paramerter */
    PDT_STATEMNG_CHECK_POINTER(pstStatemngCfg,HI_PDT_STATEMNG_ENULLPTR, "stStatemngCfg");
    HI_S32 s32Ret = HI_SUCCESS;

    /** check whether it has been intialized or not */
    if(HI_TRUE == PDT_STATEMNG_GetCtx()->bInited)
    {
        MLOGE(RED"StateMng module has already been inited\n\n"NONE);
        return HI_PDT_STATEMNG_EINITIALIZED;
    }

    /** init global context structure */
    s32Ret = PDT_STATEMNG_ContextInit(pstStatemngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"StateMng context init");

    /** create HFSM instance */
    s32Ret = PDT_STATEMNG_CreateHFSMInstance();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM instance create");
    /** init all States */
    s32Ret = PDT_STATEMNG_InitStates();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"add all states");

    /** init liveserver */
    HI_LIVESVR_MEDIA_OPERATE_S stLivesrvMediaOps;
    stLivesrvMediaOps.pfnGetVideoInfo = HI_PDT_MEDIA_GetVideoInfo;
    stLivesrvMediaOps.pfnGetAudioInfo = HI_PDT_MEDIA_GetAudioInfo;
    stLivesrvMediaOps.pfnVencStart = HI_PDT_MEDIA_VencStart;
    stLivesrvMediaOps.pfnVencStop = HI_PDT_MEDIA_VencStop;
    stLivesrvMediaOps.pfnAencStart = HI_PDT_MEDIA_AencStart;
    stLivesrvMediaOps.pfnAencStop = HI_PDT_MEDIA_AencStop;

    s32Ret = HI_LIVESVR_Init(PDT_STATEMNG_LIVESVR_MAX_CONNECT_CNT, &stLivesrvMediaOps);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_LIVESVR_Init");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = HI_VIDEODETECT_Init();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_Init");
#endif

    /** subscribe event from EventHub module */
    s32Ret = PDT_STATEMNG_SubscribeEvents();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"subscribe events");

    /** activate HFSM */
    /*TODO,if monitor workmode ?*/
    HI_PDT_WORKMODE_E enWorkmode = HI_PDT_WORKMODE_NORM_REC;
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_STARTUP;
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if(HI_SYSTEM_STARTUP_SRC_WAKEYP == enStartupSrc)
    {
        MLOGI(RED"parking monitor\n"NONE);
        enWorkmode = HI_PDT_WORKMODE_PARKING_REC;
    }
    else if(HI_SYSTEM_STARTUP_SRC_STARTUP == enStartupSrc)
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkmode);
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"get power on work mode");
        /*start check acc level*/
        PDT_STATEMNG_CreatePowerSupplyCheckTask();

    }
    s32Ret = PDT_STATEMNG_ActivateHFSM(enWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM set initial state");

    PDT_STATEMNG_GetCtx()->bInited = HI_TRUE;
    /** process PoweronAction */
    PDT_STATEMNG_PoweronProc(enWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"power on proc");

    HI_TIME_STAMP;

    return s32Ret;
}

/** statemng deinitialization */
HI_S32 HI_PDT_STATEMNG_Deinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check whether it has been deintialized or not */
    if(HI_FALSE == PDT_STATEMNG_GetCtx()->bInited)
    {
        MLOGE(RED"StateMng module has already been deinited\n\n"NONE);
        return HI_SUCCESS;
    }

/** deinit videodetect*/
#ifdef CONFIG_MOTIONDETECT_ON
        s32Ret = HI_VIDEODETECT_Deinit();
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_Deinit");
#endif

    /** deinit liveserver */
    s32Ret = HI_LIVESVR_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_LIVESVR_Deinit");

    /** deinit all states */
    s32Ret = PDT_STATEMNG_DeinitStates();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"deinit all states");
#if 0
    /** deinit usb ctrl module*/
    s32Ret = HI_PDT_USBCTRL_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"deinit usb ctrl module");
#endif
    /** deinit media */
    s32Ret = PDT_STATEMNG_DeinitMedia();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"deinit media");

    /** deactive HFSM */
    s32Ret = PDT_STATEMNG_DeactiveHFSM();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"deactive HFSM");

    /** destory HFSM */
    s32Ret = PDT_STATEMNG_DestoryHFSMInstance();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"destory HFSM");

    PDT_STATEMNG_GetCtx()->bInited = HI_FALSE;
    return s32Ret;
}

/** get current wokemode state */
HI_S32 HI_PDT_STATEMNG_GetState(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkModeState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check paramerter */
    PDT_STATEMNG_CHECK_POINTER(pstWorkModeState,HI_PDT_STATEMNG_ENULLPTR, "pstWorkModeState");

    /** check whether it has been intialized or not */
    if(HI_FALSE == pstStateMngCtx->bInited)
    {
        MLOGE(RED"StateMng Module has not been inited\n\n"NONE);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    MUTEX_LOCK(pstStateMngCtx->Mutex);
    pstWorkModeState->enWorkMode = pstStateMngCtx->enCurrentWorkMode;
    pstWorkModeState->bRunning = pstStateMngCtx->bRunning;
    pstWorkModeState->bStateMngInProgress = pstStateMngCtx->bInProgress;
    pstWorkModeState->bEMRRecord = pstStateMngCtx->stRecCtx.bEMRRecord;
    pstWorkModeState->bSDAvailable = pstStateMngCtx->bSDAvailable;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    return s32Ret;
}

/** send message to statemng with parameter, synchronize UI and NetCtrl modules */
HI_S32 HI_PDT_STATEMNG_SendMessage(const HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check paramerter */
    PDT_STATEMNG_CHECK_POINTER(pstMsg,HI_PDT_STATEMNG_ENULLPTR, "pstMsg");

    /** check whether it has been intialized or not */
    if(HI_FALSE == pstStateMngCtx->bInited)
    {
        MLOGE(RED"StateMng module has not been inited\n\n"NONE);
        return HI_PDT_STATEMNG_ENOTINIT;
    }

    MUTEX_LOCK(pstStateMngCtx->Mutex);
    /** check whether it was in progress or not */
    if(HI_TRUE == pstStateMngCtx->bInProgress)
    {
        MLOGE(RED"StateMng module is InProgress\n\n"NONE);
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
        return HI_PDT_STATEMNG_EINPROGRESS;
    }

    s32Ret = HI_HFSM_SendAsyncMessage(pstStateMngCtx->pvHfsmHdl, (HI_MESSAGE_S *)pstMsg);
    PDT_STATEMNG_CHECK_RET_WITH_UNLOCK(s32Ret,HI_PDT_STATEMNG_EINTER,"send message to HFSM(from Terminal)");

    pstStateMngCtx->bInProgress = HI_TRUE;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    return s32Ret;
}

/** register all event published from this module */
HI_S32 HI_PDT_STATEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SWITCH_WORKMODE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_START);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_STOP);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_TRIGGER);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SETTING);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_POWEROFF);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_AVAILABLE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_UNAVAILABLE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SD_FORMAT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_LIVE_SWITCH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_VO_SWITCH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SYATEM_REBOOT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_FACTORY_RESET);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_EMR_BEGIN);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_EMR_END);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SCREEN_DORMANT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_DEBUG_SWITCH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_START);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_DONOTUPGRADE);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_ABNORMAL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_CHECK_ACC_OFF);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_UPGRADE_FAILED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_PREVIEW_PIP);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_STATEMNG_SHUTDOWN_PROC);

    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "StateMng register event for publish");
    return s32Ret;
}

HI_S32 HI_PDT_STATEMNG_GetPreViewCamID(HI_VOID)
{
     return PDT_STATEMNG_GetPreViewCamID();
}



//HI_S32 HI_PDT_STATEMNG_GetDebugFlag(HI_VOID)
//{
//     return PDT_STATEMNG_GetPreViewCamID();
//}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
