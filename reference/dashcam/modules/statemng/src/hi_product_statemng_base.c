/**
 * @file      hi_product_statemng_base.c
 * @brief     Realize the interface about Base state.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 * @version
 */
#include "hi_system.h"
#include "hi_ahdmng.h"
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern sem_t g_PowerOffSem;

static HI_S32 PDT_STATEMNG_BaseStateSwitchWorkmodeMsgProc(HI_MESSAGE_S *pstMsg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    MLOGD(YELLOW"try to process message HI_EVENT_STATEMNG_SWITCH_WORKMODE(%x), "
                        "arg2:workmode switch to(%d)\n\n"NONE, pstMsg->what, pstMsg->arg2);
    /** check parameter pstMsg->arg2 */
    if((pstMsg->arg2 >= HI_PDT_WORKMODE_BUTT) ||
       (pstMsg->arg2 < HI_PDT_WORKMODE_NORM_REC))
    {
        MLOGE(RED"the workmode(arg2) to set is error\n\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    /** check IsRunning */
    if(HI_TRUE == pstStateMngCtx->bRunning)
    {
        MLOGE(RED"current workmode isRunning, can't tansition\n\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    HI_CHAR szTranToStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(pstMsg->arg2, szTranToStateName, HI_STATE_NAME_LEN);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINVAL,"change workmode to state");
    if(pstStateMngCtx->enCurrentWorkMode == pstMsg->arg2)
    {
        MLOGE(RED"the same workmode(%s), needn't tansition\n\n"NONE, szTranToStateName);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    else
    {
        /** Important Note:
         ** if set the variable pStateID, the state machine
         ** will swich state before process next message
         **/
        MLOGD("transition to workmode(%s)\r\n", szTranToStateName);
        *pStateID = pstMsg->arg2;

    }
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateMountedMsgProc(HI_MESSAGE_S *pstMsg)
{
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    MLOGI("Current enPowerOnState is %d.\n",pstStateMngCtx->enPowerOnState);
    if(HI_STORAGE_STATE_DEV_UNPLUGGED == pstStateMngCtx->enPowerOnState)
    {
        HI_SYSTEM_Reboot();
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    HI_TIME_STAMP
    pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_MOUNT_FAILED;

    PDT_STATEMNG_MountedMsgProc(pstStateMngCtx->enCurrentWorkMode);

    /** no need reset g_bStateMngInProgress to HI_FALSE */
    return HI_PROCESS_MSG_RESULTE_OK;
}


static HI_S32 PDT_STATEMNG_BaseStateErrorMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;

    MLOGD(YELLOW"try to process sd error messages, what(%x)\n\n"NONE, pstMsg->what);
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
    if (enStartupSrc != HI_SYSTEM_STARTUP_SRC_WAKEYP)
    {
        s32Ret = PDT_STATEMNG_SetUsbMode();
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
    }
    /** no need publish result event and reset g_bStateMngInProgress to HI_FALSE */
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateFormatMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS,s32Result = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    MLOGD(YELLOW"try to process message HI_EVENT_STATEMNG_SD_FORMAT(%x)\n\n"NONE,
                        pstMsg->what);

    /** check IsRunning */
    if(HI_TRUE == pstStateMngCtx->bRunning)
    {
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    s32Ret = HI_FILEMNG_SetDiskState(HI_FALSE);
    if(HI_FILEMNG_ENOTINIT == s32Ret)
    {
        MLOGI("filemng not init\n");
        s32Ret = HI_SUCCESS;
    }

    /** use default parameter u64ClusterSize, or pass it by message sended by UI */
    HI_FSTOOL_FORMAT_MODE_E enMode = HI_FSTOOL_FORMAT_MODE_BUTT;
    enMode = (pstMsg->arg1 == HI_FSTOOL_FORMAT_MODE_DISCARD)?HI_FSTOOL_FORMAT_MODE_DISCARD:HI_FSTOOL_FORMAT_MODE_ADVANCED;
#ifdef CONFIG_COREDUMP_ON
    HI_system("/app/debug_coredump umount");
#endif
    s32Result = HI_STORAGEMNG_Format(pstStateMngCtx->stStorageMngCfg.szMntPath,enMode);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Result,"storagemng format");
#ifdef CONFIG_COREDUMP_ON
    HI_system("/app/debug_coredump mount");
#endif

    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    HI_S32 loop_count = 10;
    while ((HI_STORAGE_STATE_MOUNTED != enState) && (loop_count-- > 0))
    {
        s32Ret = HI_STORAGEMNG_GetState(pstStateMngCtx->stStorageMngCfg.szMntPath,&enState);
        PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,s32Ret,HI_PROCESS_MSG_RESULTE_OK,"storagemng getstate");
        HI_usleep(100*1000);
    }

    if(HI_STORAGE_STATE_MOUNTED == enState)
    {
        pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_MOUNTED;
        /** init filemng */
        s32Ret = PDT_STATEMNG_InitFileMng();
        PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"InitFileMng");

        MLOGD(YELLOW"call HI_FILEMNG_SetDiskState after format\n\n"NONE);
        HI_FILEMNG_SetDiskState(HI_TRUE);
        pstStateMngCtx->bSDAvailable = HI_TRUE;
        HI_EVENT_S stEvent;
        memset(&stEvent, 0 , sizeof(HI_EVENT_S));
        stEvent.EventID = HI_EVENT_STATEMNG_SD_AVAILABLE;
        stEvent.arg1 = 1; /**event triggerd by format*/
        s32Ret = HI_EVTHUB_Publish(&stEvent);
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"publish HI_EVENT_STATEMNG_SD_AVAILABLE event");
    }
    else
    {
        pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_MOUNT_FAILED;
        MLOGE(YELLOW"no call HI_FILEMNG_SetDiskState after format\n\n"NONE);
        pstStateMngCtx->bSDAvailable = HI_FALSE;
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    PDT_STATEMNG_UPDATESTATUS(pstMsg,s32Result,HI_FALSE);
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateFileEndMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_CHAR szFilePath[HI_APPCOMM_MAX_PATH_LEN] = {0};
    MLOGD(YELLOW"process message [%d] file name(%s)\n\n"NONE, pstMsg->what, pstMsg->aszPayload);
    if(HI_EVENT_RECMNG_FILE_END == pstMsg->what)
    {
        snprintf(szFilePath,HI_APPCOMM_MAX_PATH_LEN,"%s",pstMsg->aszPayload);
    }
    else if(HI_EVENT_PHOTOMNG_FILE_END == pstMsg->what)
    {
        snprintf(szFilePath,HI_APPCOMM_MAX_PATH_LEN,"%s",pstMsg->aszPayload);
    }
    else if(HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END == pstMsg->what)
    {

        s32Ret = HI_DTCF_GetEmrFilePath(pstMsg->aszPayload,szFilePath,HI_APPCOMM_MAX_PATH_LEN);
        rename(pstMsg->aszPayload,szFilePath);
    }

    s32Ret =  HI_FILEMNG_AddFile(szFilePath);
    if (HI_FILEMNG_ENOTMAIN == s32Ret||HI_FILEMNG_EEXIST == s32Ret)
    {
        MLOGD("NotMainFileName[%s] or File exists\n", szFilePath);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"filemng add file");
    }
    /** no need publish result event and reset g_bStateMngInProgress to HI_FALSE */
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateLiveSwitchMsgProc(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0,s32Switch = 0;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    memcpy(&s32Switch,pstMsg->aszPayload,sizeof(HI_S32));

    MLOGD(YELLOW"try to process message HI_EVENT_STATEMNG_LIVE_SWITCH(%x)"
                        "switch(%d)\n\n"NONE,pstMsg->what, s32Switch);
    PDT_STATEMNG_UpdateLiveSwitchOsdState(s32Switch);
    if(pstStateMngCtx->s32LiveCamID == s32Switch)
    {
        MLOGD(RED"current live camid[%d] is the same as switch cam\n"NONE,pstStateMngCtx->s32LiveCamID);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
    {
        if((pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID == s32Switch) && (pstStateMngCtx->astCamCtx[s32Idx].bEnable))
        {
            break;
        }
    }
    if(s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE(RED"switch cam[%d] is out of range or the cam is not enable \n"NONE,s32Switch);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    pstStateMngCtx->s32LiveCamID  = s32Switch;
    /*rebind live venc*/
    HI_PDT_MEDIA_BIND_SRC_CFG_S stLiveSrc;
    s32Ret = HI_PDT_PARAM_GetLiveSrcByCamID(s32Switch, &stLiveSrc);
    PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,s32Ret,HI_PROCESS_MSG_RESULTE_OK,"Get Venc src");
    s32Ret = HI_PDT_MEDIA_VencReBind(&stLiveSrc);
    PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,s32Ret,HI_PROCESS_MSG_RESULTE_OK,"Venc ReBind");
    PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
    MLOGI("switch live finished\n"NONE);
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateVoSwitchMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0,s32Switch = 0;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    memcpy(&s32Switch,pstMsg->aszPayload,sizeof(HI_S32));

    MLOGD(YELLOW"try to process message HI_EVENT_STATEMNG_VO_SWITCH(%x)"
                        "switch(%d)\n\n"NONE,pstMsg->what, s32Switch);
    if(pstStateMngCtx->s32PreviewCamID == s32Switch)
    {
        MLOGE(RED"current preview camid[%d] is the same as switch cam\n"NONE,pstStateMngCtx->s32PreviewCamID);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
    {
        if((pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID == s32Switch) && (pstStateMngCtx->astCamCtx[s32Idx].bEnable))
        {
            break;
        }
    }
    if(s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE(RED"switch cam[%d] is out of range or the cam is not enable \n"NONE,s32Switch);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    if(HI_TRUE == pstStateMngCtx->bScreenDormantStatus)
    {
        MLOGE(RED"Screen is dormant,can not config vo\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    pstStateMngCtx->s32PreviewCamID  = s32Switch;
    /*get media config*/
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get media config");

    /*media proc*/
    s32Ret = HI_PDT_MEDIA_VideoOutStop();
    //s32Ret |= HI_PDT_MEDIA_VideoOutDeinit();
    //PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,s32Ret,HI_PROCESS_MSG_RESULTE_OK,"Video Stop");

    s32Ret = HI_PDT_MEDIA_VideoOutInit(&stMediaCfg.stVideoOutCfg);
    s32Ret |= HI_PDT_MEDIA_VideoOutStart(NULL,&stMediaCfg.stVideoCfg.stVprocCfg);
    PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg,s32Ret,HI_PROCESS_MSG_RESULTE_OK,"Video Start");
    PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
    MLOGI("switch vo finished\n"NONE);
    return HI_PROCESS_MSG_RESULTE_OK;

}

static HI_S32 PDT_STATEMNG_BaseStateScreenDormantProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bScreenDormant;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    memcpy(&bScreenDormant,pvPayload,sizeof(HI_BOOL));

    MLOGD(YELLOW"try to process message HI_EVENT_STATEMNG_SCREEN_DORMANT(%x)"
                "bScreenDormant(%d)\n\n"NONE,pstMsg->what, bScreenDormant);

    if(pstStateMngCtx->bScreenDormantStatus == bScreenDormant)
    {
        MLOGE(RED"context bScreenDormantStatus[%d] is the same as cmd bScreenDormant[%d]\n"NONE,
            pstStateMngCtx->bScreenDormantStatus,bScreenDormant);
        PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_SUCCESS,HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    pstStateMngCtx->bScreenDormantStatus = bScreenDormant;

    if(bScreenDormant)
    {
#ifdef CONFIG_SCREEN
        /**screen dormant*/
        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"set backlight state off failed,s32Ret:0x%x \n"NONE,s32Ret);
        }

        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"set display state off failed,s32Ret:0x%x \n"NONE,s32Ret);
        }
#endif
        /** Video out stop */
        s32Ret = HI_PDT_MEDIA_VideoOutStop();
        if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            s32Ret = HI_SUCCESS;
            MLOGD(YELLOW"Media or VideoOut not init\n\n"NONE);
        }
        else if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"video out stop failed,s32Ret:0x%x \n"NONE,s32Ret);
        }
    }
    else
    {
        /**screen wakeup*/
        /** video out start */
        s32Ret = HI_PDT_MEDIA_VideoOutStart(NULL,NULL);
        if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            MLOGD(YELLOW"Media or VideoOut not init\n\n"NONE);
        }
        else if (HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"video out start failed,s32Ret:0x%x \n"NONE,s32Ret);
        }
#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"set backlight state on failed,s32Ret:0x%x \n"NONE,s32Ret);
        }

        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"set display state on failed,s32Ret:0x%x \n"NONE,s32Ret);
        }
#endif
    }

    PDT_STATEMNG_UPDATESTATUS(pstMsg,s32Ret,HI_FALSE);
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStatePreviewPIPProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 i = 0,s32Ret = HI_SUCCESS;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bPreviewPIP = HI_FALSE;
    HI_BOOL bpreviewPIPOld = HI_FALSE;
    HI_PDT_PARAM_MEDIA_COMM_CFG_S stMediaCommCfg = {};

    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    memcpy(&bPreviewPIP, pvPayload, sizeof(HI_BOOL));
    MLOGI("process PreviewPIP newvalue[%d]\n",bPreviewPIP);

    s32Ret = HI_PDT_PARAM_GetMediaCommCfg(&stMediaCommCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "HI_PDT_PARAM_GetMediaCommCfg");
    if(stMediaCommCfg.stVideoOutCfg.astDispCfg[0].u32WindowCnt <= 1)
    {
        MLOGW("can not support Preview PIP \n");
        PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&bpreviewPIPOld);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");
    if(bPreviewPIP == bpreviewPIPOld)
    {
        MLOGW("the new value same oldvalue\n");
        PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if (!pstStateMngCtx->astCamCtx[i].bEnable)
        {
            MLOGW("one cam enable , not support Preview PIP \n");
            PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
    }

    /* save new PIP_param */
    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&bPreviewPIP);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "set comm param");

    /*get media config*/
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    if (HI_PDT_WORKMODE_NORM_REC == pstStateMngCtx->enCurrentWorkMode) {
        /*LAPSE: update venc attr */
        HI_PDT_PARAM_RecordUpdateMediaCfg(&stMediaCfg);
    }

    /*media proc*/
    s32Ret = HI_PDT_MEDIA_VideoOutStop();
    PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg, s32Ret, HI_PROCESS_MSG_RESULTE_OK, "Video Stop");
    HI_BOOL bCompress = HI_TRUE;
#ifdef CONFIG_RAWCAP_ON
    if ((HI_PDT_WORKMODE_PLAYBACK != pstStateMngCtx->enCurrentWorkMode)
        && (HI_TRUE == pstStateMngCtx->stRawCapCtx.bDebug))
    {
        bCompress = HI_FALSE;
    }
#endif
    s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg, bCompress);
    s32Ret |= HI_PDT_MEDIA_VideoOutStart(NULL,NULL);
    if (s32Ret)
    {
        HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PREVIEWPIP,&bpreviewPIPOld);
    }
    PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg, s32Ret, HI_PROCESS_MSG_RESULTE_OK, "Video Stop");
    PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
    return HI_PROCESS_MSG_RESULTE_OK;
}

/** enter Base state */
HI_S32 PDT_STATEMNG_BaseStateEnter(HI_VOID *pvArg)
{
    return HI_SUCCESS;
}

/** exit Base state */
HI_S32 PDT_STATEMNG_BaseStateExit(HI_VOID *pvArg)
{
    return HI_SUCCESS;
}

/** Base state message process */
HI_S32 PDT_STATEMNG_BaseStateMsgProc(HI_MESSAGE_S *pstMsg, HI_MW_PTR pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg,pstStateMngCtx->bInProgress);

    MLOGD(YELLOW" will process message what(%x) \n\n"NONE,pstMsg->what);
    switch(pstMsg->what)
    {
        /** switch workmode message process */
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            {
                return PDT_STATEMNG_BaseStateSwitchWorkmodeMsgProc(pstMsg, pStateID);
            }
        /** sd mounted message process */
        case HI_EVENT_STORAGEMNG_MOUNTED:
            {
                return PDT_STATEMNG_BaseStateMountedMsgProc(pstMsg);
            }
        case HI_EVENT_STORAGEMNG_DEV_CONNECTING:
            if(HI_STORAGE_STATE_DEV_UNPLUGGED != pstStateMngCtx->enPowerOnState)
            {
                pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_DEV_CONNECTING;
            }
            return HI_PROCESS_MSG_RESULTE_OK;
        /** switch sd error message process */
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            {
                HI_STORAGEMNG_GetState(pstStateMngCtx->stStorageMngCfg.szMntPath, &pstStateMngCtx->enPowerOnState);
                return PDT_STATEMNG_BaseStateErrorMsgProc(pstMsg);
            }
        /** switch sd format message process */
        case HI_EVENT_STATEMNG_SD_FORMAT:
            {
                return PDT_STATEMNG_BaseStateFormatMsgProc(pstMsg);
            }
        case HI_EVENT_AHDMNG_PLUG_STATUS:
            if(HI_AHDMNG_PLUG_IN == pstMsg->arg1)
            {
                HI_SYSTEM_Reboot();
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            PDT_STATEMNG_SendShutdownMsg();
            break;
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:/** switch sd unpluged message process */
            if(HI_STORAGE_STATE_IDEL == pstStateMngCtx->enPowerOnState)
            {
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                MLOGI("Change StorageIdle to Real State. %d\n",pstStateMngCtx->enPowerOnState);
                HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
                if (enStartupSrc != HI_SYSTEM_STARTUP_SRC_WAKEYP)
                {
                    s32Ret = PDT_STATEMNG_SetUsbMode();
                    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
                }
                pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_DEV_UNPLUGGED;
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            else if(HI_STORAGE_STATE_DEV_UNPLUGGED == pstStateMngCtx->enPowerOnState)
            {
                MLOGI("The Current PowerOnstate is %d.\n",pstStateMngCtx->enPowerOnState);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            PDT_STATEMNG_SendShutdownMsg();
            break;
        case HI_EVENT_STATEMNG_POWEROFF:
            {
                /*TODO*/
                MLOGI(GREEN"try to process message (%x) to poweroff device\n\n"NONE,
                            pstMsg->what);
                HI_PDT_PARAM_Deinit();
                HI_SYSTEM_Poweroff();

                /** no need publish result event and reset g_bStateMngInProgress to HI_FALSE */
                return HI_PROCESS_MSG_RESULTE_OK;
            }
        case HI_EVENT_RECMNG_FILE_END:
        case HI_EVENT_PHOTOMNG_FILE_END:
        case HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END:
            {
                return PDT_STATEMNG_BaseStateFileEndMsgProc(pstMsg);
            }
        case HI_EVENT_STATEMNG_LIVE_SWITCH:
            {
                return PDT_STATEMNG_BaseStateLiveSwitchMsgProc(pstMsg);
            }
        case HI_EVENT_STATEMNG_VO_SWITCH:
            {
                return PDT_STATEMNG_BaseStateVoSwitchMsgProc(pstMsg);
            }
        case HI_EVENT_STATEMNG_SYATEM_REBOOT:
            {
                MLOGI(YELLOW"try to process message HI_EVENT_STATEMNG_SYATEM_REBOOT"NONE);

                HI_PDT_PARAM_Deinit();
                HI_SYSTEM_Reboot();
                return HI_PROCESS_MSG_RESULTE_OK;
            }
        case HI_EVENT_STATEMNG_FACTORY_RESET:
            {
                MLOGI(YELLOW"try to process message HI_EVENT_STATEMNG_FACTORY_RESET\n\n"NONE);

                HI_PDT_PARAM_SetDefault();
                HI_PDT_PARAM_Deinit();
                HI_SYSTEM_Reboot();
                return HI_PROCESS_MSG_RESULTE_OK;
            }
        case HI_EVENT_STATEMNG_SCREEN_DORMANT:
            {
                return PDT_STATEMNG_BaseStateScreenDormantProc(pstMsg);
            }
        case HI_EVENT_STATEMNG_PREVIEW_PIP:
            {
                return PDT_STATEMNG_BaseStatePreviewPIPProc(pstMsg);
            }
        case HI_EVENT_UPGRADE_SUCCESS:
            MLOGI(LIGHT_GREEN"Upgrade Successfully.\n\n"NONE);
        case HI_EVENT_UPGRADE_FAILURE:
            {
                HI_PDT_DEV_INFO_S stDevInfo;
                HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
                HI_UPGRADE_DEV_INFO_S stUpgradeDevInfo;
                snprintf(stUpgradeDevInfo.szSoftVersion,HI_APPCOMM_COMM_STR_LEN,"%s",stDevInfo.szSoftVersion);
                snprintf(stUpgradeDevInfo.szModel,HI_APPCOMM_COMM_STR_LEN,"%s",stDevInfo.szModel);
                HI_UPGRADE_DelUpgradeFiles(pstStateMngCtx->stStorageMngCfg.szMntPath,&stUpgradeDevInfo);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
        default:
        {
            MLOGD(YELLOW"can not process this message(%x)\n\n"NONE, pstMsg->what);
            /** func PDT_STATEMNG_HFSM_Event_Proc process HI_HFSM_EVENT_UNHANDLE_MSG event */
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

HI_S32 PDT_STATEMNG_BaseStateInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    pstStateMngCtx->stBase.stState.stateID = HI_PDT_WORKMODE_BUTT;
    snprintf(pstStateMngCtx->stBase.stState.name, HI_STATE_NAME_LEN, "%s", PDT_STATEMNG_STATE_BASE);
    pstStateMngCtx->stBase.stState.enter = PDT_STATEMNG_BaseStateEnter;
    pstStateMngCtx->stBase.stState.exit = PDT_STATEMNG_BaseStateExit;
    pstStateMngCtx->stBase.stState.processMessage = PDT_STATEMNG_BaseStateMsgProc;
    pstStateMngCtx->stBase.stState.argv = &(pstStateMngCtx->stBase);
    pstStateMngCtx->stBase.bRunning = HI_FALSE;

    s32Ret = HI_HFSM_AddState(pstStateMngCtx->pvHfsmHdl,
                              &(pstStateMngCtx->stBase.stState),
                              NULL);
    return s32Ret;
}

HI_S32 PDT_STATEMNG_BaseStateDeinit(HI_VOID)
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
