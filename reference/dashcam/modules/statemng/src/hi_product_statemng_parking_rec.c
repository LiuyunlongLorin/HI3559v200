/**
 * \file   hi_product_statemng_parking_rec.c
 * \brief  Realize the interface about record states.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include <unistd.h>
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"
#include "hi_timedtask.h"
#include "hi_hal_touchpad.h"
#include "hi_ahdmng.h"
#include "hi_mapi_aenc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_S32 PDT_STATEMNG_DestroyParkingRecCheckTask(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    s32Ret = HI_TIMEDTASK_Destroy(pstStateMngCtx->stParkingRecCtx.TimeTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"destroy timed task");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_ParkingRecStart(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_BOOL bRunning = HI_FALSE;
    HI_BOOL bEMRRecord = HI_FALSE;

    s32Ret = PDT_STATEMNG_StartRec();
    s32Ret |= PDT_STATEMNG_RecManualSplit(HI_TRUE);
    bEMRRecord = ((HI_SUCCESS == s32Ret)?HI_TRUE : HI_FALSE);
    bRunning = bEMRRecord;
    HI_PrintBootTime("linux start record");
    MUTEX_LOCK(pstStateMngCtx->Mutex);
    pstStateMngCtx->bInProgress = HI_FALSE;
    pstStateMngCtx->bRunning = bRunning;
    pstStateMngCtx->stRecCtx.bEMRRecord = bEMRRecord;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_STATEMNG_EMR_BEGIN;
    stEvent.arg2 = HI_PDT_WORKMODE_PARKING_REC;
    stEvent.s32Result = s32Ret;
    return HI_EVTHUB_Publish(&stEvent);
}

HI_S32 PDT_STATEMNG_InitEnc(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        HI_PDT_MEDIA_VENC_CFG_S*  pstVencCfg = &pstMediaCfg->stVideoCfg.astVencCfg[i];
        if (!pstVencCfg->bEnable)
        {
            continue;
        }

        s32Ret = HI_PDT_MEDIA_VencInit(pstVencCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    if(pstMediaCfg->stAudioCfg.astAencCfg[0].bEnable)
    {
        /* Aenc Init */
        MLOGD("Aenc  Hdl[%d]\n", pstMediaCfg->stAudioCfg.astAencCfg[0].AencHdl);
        HI_MAPI_AENC_ATTR_S stAencAttr;
        stAencAttr.enAencFormat   = HI_MAPI_AUDIO_FORMAT_AACLC;
        stAencAttr.u32PtNumPerFrm = pstMediaCfg->stAudioCfg.astAencCfg[0].u32PtNumPerFrm;
        stAencAttr.pValue         = &pstMediaCfg->stAudioCfg.astAencCfg[0].unAttr.stAACAttr;
        stAencAttr.u32Len         = sizeof(HI_MAPI_AENC_ATTR_AAC_S);
        s32Ret = HI_MAPI_AENC_Init(pstMediaCfg->stAudioCfg.astAencCfg[0].AencHdl, &stAencAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

/** enter Rec states */
HI_S32 PDT_STATEMNG_ParkingRecStatesEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"parameter pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"try to enter state(%s), bRunning(%d)\n\n"NONE,pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"change state to workmode");
    if(HI_PDT_WORKMODE_PARKING_REC != enEnterWorkMode)
    {
        MLOGE(RED"enter workmode[%d] error\n\n"NONE,enEnterWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }
    else
    {
        pstStateMngCtx->enCurrentWorkMode = enEnterWorkMode;
        pstStateMngCtx->bRunning = pstStateAttr->bRunning;
    }

    /*get media config*/
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get media config");

    s32Ret = HI_PDT_PARAM_GetCamParam(enEnterWorkMode, pstStateMngCtx->s32PreviewCamID, HI_PDT_PARAM_TYPE_CROP, &stEnterMediaCfg.stVideoOutCfg.astDispCfg[0].astWndCfg[0].stCropCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get crop");


    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get stEnterWorkModeCfg parameter");


    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(enEnterWorkMode, &stEnterWorkModeCfg,
                                    &pstStateMngCtx->stRecCtx,&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"generate record context");

    /** init enc */
    s32Ret = PDT_STATEMNG_InitEnc(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"init enc");

    HI_TIME_STAMP;
    s32Ret = PDT_STATEMNG_ParkingRecStart();
    return s32Ret;
}

/** exit Rec states */
HI_S32 PDT_STATEMNG_ParkingRecStatesExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** destroy Emr record timed task */
    PDT_STATEMNG_DestroyParkingRecCheckTask();

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"change state to workmode");

    if(HI_PDT_WORKMODE_PARKING_REC != enExitWorkMode)
    {
        MLOGE(RED"exit workmode[%d] error \n\n"NONE,enExitWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** exit workmode directly when it is running */
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    if(pstStateAttr->bRunning)
    {
        s32Ret = PDT_STATEMNG_StopRec(HI_TRUE);
        PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"stop rec");
        /** file data sync */
        HI_async();

        pstStateAttr->bRunning = HI_FALSE;
        MUTEX_LOCK(pstStateMngCtx->Mutex);
        pstStateMngCtx->bRunning = HI_FALSE;
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    }

    /* init usb */
    s32Ret = HI_PDT_USBCTRL_Init();
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"init usb ctrl module");

#ifdef CFG_LCD_TOUCHPAD_ON
    s32Ret = HI_HAL_TOUCHPAD_Resume();
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_HAL_TOUCHPAD_Suspend");
#endif
    /** init keymng*/
    HI_KEYMNG_CFG_S stKeyCfg;
    s32Ret = HI_PDT_PARAM_GetKeyMngCfg(&stKeyCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_PDT_PARAM_GetKeyMngCfg");
    s32Ret = HI_KEYMNG_Init(&stKeyCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "HI_KEYMNG_Init");

    return s32Ret;
}

/** Rec states message process */
HI_S32 PDT_STATEMNG_ParkingRecStatesMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_EVENT_S stEvent;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg,pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n",pstStateAttr->stState.name, pstStateAttr->bRunning);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"change state to workmode");

    MLOGD(YELLOW"process message what(%x) \n\n"NONE,pstMsg->what);
    switch(pstMsg->what)
    {
        /** RecMng error or sd plug out*/
        case HI_EVENT_RECMNG_ERROR:
        case HI_EVENT_AHDMNG_PLUG_STATUS:
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            {
                if (HI_TRUE == pstStateMngCtx->bRunning)
                {
                    PDT_STATEMNG_StopRec(HI_TRUE);

                    MUTEX_LOCK(pstStateMngCtx->Mutex);
                    pstStateAttr->bRunning = HI_FALSE;
                    pstStateMngCtx->bRunning = HI_FALSE;
                    pstStateMngCtx->stRecCtx.bEMRRecord = HI_FALSE;
                    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

                    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
                    stEvent.EventID = HI_EVENT_STATEMNG_EMR_END;
                    stEvent.arg2 = enCurrentWorkMode;
                    stEvent.s32Result = HI_SUCCESS;
                    HI_EVTHUB_Publish(&stEvent);
                }
                return HI_PROCESS_MSG_RESULTE_OK;
            }

        case HI_EVENT_RECMNG_TASK_AUTO_STOP:
            {
                PDT_STATEMNG_EmrTaskEndProc(pstMsg->what,pstMsg->arg1,HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
        case HI_EVENT_STATEMNG_POWEROFF:
            {
                if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
                {
                    PDT_STATEMNG_StopRec(HI_TRUE);
                    MUTEX_LOCK(pstStateMngCtx->Mutex);
                    pstStateMngCtx->bInProgress = HI_FALSE;
                    pstStateAttr->bRunning = HI_FALSE;
                    pstStateMngCtx->bRunning = HI_FALSE;
                    pstStateMngCtx->stRecCtx.bEMRRecord = HI_FALSE;
                    MUTEX_UNLOCK(pstStateMngCtx->Mutex);
                }
                /** file data sync */
                sync();
                /** sd destroy */
                HI_STORAGEMNG_CFG_S stStorageMngCfg;
                memset(&stStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
                HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
                HI_STORAGEMNG_Destroy(stStorageMngCfg.szMntPath);
                return HI_PROCESS_MSG_UNHANDLER;/** need base state precess */
            }
        case HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END:
            {
                return HI_PROCESS_MSG_UNHANDLER;
            }
        /** sd mounted message process */
        case HI_EVENT_STORAGEMNG_MOUNTED:
            {
                return HI_PROCESS_MSG_UNHANDLER;
            }

        /** below ui send message */
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            {
                HI_BOOL bIsOn = HI_FALSE;
                PDT_STATEMNG_GetPowerSupplyStatue(&bIsOn);
                if((HI_TRUE == bIsOn) && (HI_TRUE != pstStateAttr->bRunning))
                {
                    MLOGI(RED"can switch workmode on parking monitor state when acc is on\n\n"NONE);
                    return HI_PROCESS_MSG_UNHANDLER;
                }
                else
                {
                    MLOGE(RED"can not switch workmode on parking monitor state when acc is off\n\n"NONE);
                    PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
        case HI_EVENT_STATEMNG_STOP:
            {
                MLOGE(RED"can not stop record task on parking monitor state\n\n"NONE);
            }
        case HI_EVENT_STATEMNG_TRIGGER:
            {
                MLOGE(RED"can not start photo task on parking monitor state\n\n"NONE);
            }
        case HI_EVENT_STATEMNG_SETTING:
            {
                MLOGE(RED"can not do setting operation on parking monitor state\n\n"NONE);
            }
        case HI_EVENT_STATEMNG_SYATEM_REBOOT:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
            {
                MLOGE(RED"can not do factory or reboot on parking monitor state\n\n"NONE);
            }
            PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;

        /**below event message */
        case HI_EVENT_FILEMNG_SPACE_FULL:
            {
                MLOGE(RED"space full event need not be received on parking monitor state\n\n"NONE);
            }
        case HI_EVENT_PHOTOMNG_FILE_END:
        case HI_EVENT_PHOTOMNG_TASK_END:
            {
                MLOGE(RED"photo task end event or photo file end need not be received on parking monitor state\n\n"NONE);
            }
        default:
            {
                MLOGD(RED"can not process this message(%d),return\n\n"NONE,pstMsg->what);
            }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Rec states, add normal Rec states to HFSM, init RecMng module */
HI_S32 PDT_STATEMNG_ParkingRecStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stParkingRec =
        {{HI_PDT_WORKMODE_PARKING_REC,
          PDT_STATEMNG_STATE_PARKING_REC,
          PDT_STATEMNG_ParkingRecStatesEnter,
          PDT_STATEMNG_ParkingRecStatesExit,
          PDT_STATEMNG_ParkingRecStatesMsgProc,
          NULL},HI_FALSE};
    stParkingRec.stState.argv = &stParkingRec;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stParkingRec.stState,
                              (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM add ParkingRec state");
    return s32Ret;
}

/** deinit RecMng module */
HI_S32 PDT_STATEMNG_ParkingRecStatesDeinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit APP Common RecMng module */
    s32Ret = HI_RECMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Record Deinit");

    PDT_STATEMNG_DestroyParkingRecCheckTask();
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
