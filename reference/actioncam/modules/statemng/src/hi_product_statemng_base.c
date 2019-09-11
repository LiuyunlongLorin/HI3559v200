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
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern sem_t g_PowerOffSem;
extern PDT_STATEMNG_CONTEXT g_stSTATEMNGContext;

static HI_S32 PDT_STATEMNG_BaseStateSwitchWorkmodeMsgProc(HI_MESSAGE_S *pstMsg,
    HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD(YELLOW"process switch to workmode(%d) msg(0x%08X)\n"NONE, pstMsg->arg2, pstMsg->what);

    /** check parameter workmode(pstMsg->arg2) */
    if((pstMsg->arg2 >= HI_PDT_WORKMODE_BUTT) || (pstMsg->arg2 < HI_PDT_WORKMODE_NORM_REC))
    {
        MLOGE(RED"param workmode err\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
            g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    /** check IsRunning */
    if(HI_TRUE == g_stSTATEMNGContext.bRunning)
    {
        MLOGD(YELLOW"task running, can't tansition\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
            g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    HI_CHAR szTranToStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(pstMsg->arg2, szTranToStateName, HI_STATE_NAME_LEN);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode name");
    if(g_stSTATEMNGContext.enCurrentWorkMode == pstMsg->arg2)
    {
        MLOGD(YELLOW"same workmode(%s), needn't tansition\n"NONE, szTranToStateName);
        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
            g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    else
    {
        /** Inportant Note:
         ** if set the variable pStateID, the state machine
         ** will swich state before process next message
         **/
        MLOGD("transition to workmode(%s)\n", szTranToStateName);
        *pStateID = pstMsg->arg2;

        g_stSTATEMNGContext.bPoweronAction = HI_FALSE;/** reset bPoweronAction after switch workmod */
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateMountedMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD(YELLOW"process mounted msg(0x%08X)\n"NONE, pstMsg->what);

    /** init filemng */
    s32Ret = PDT_STATEMNG_InitFileMng();
    PDT_STATEMNG_CHECK_RET(s32Ret, "init filemng");

    /** check upgrade package */
    if(1 != PDT_STATEMNG_UpgradeSrchNewPkt())
    {
        MLOGD(YELLOW"PDT_STATEMNG_UpgradeSrchNewPkt no pkt(0x%08X)\n"NONE, s32Ret);

        s32Ret = PDT_STATEMNG_FileMngProc();
        PDT_STATEMNG_CHECK_RET(s32Ret,"PDT_STATEMNG_FileMngProc");

        /** process PoweronAction */
        if(HI_TRUE == g_stSTATEMNGContext.bPoweronAction)
        {
            HI_MESSAGE_S stMessage;
            memset(&stMessage, 0 , sizeof(HI_MESSAGE_S));
            stMessage.what = HI_EVENT_STATEMNG_START;
            stMessage.arg2 = g_stSTATEMNGContext.enCurrentWorkMode;

            s32Ret = HI_HFSM_SendAsyncMessage(g_stSTATEMNGContext.pvHfsmHdl, &stMessage);
            PDT_STATEMNG_CHECK_RET(s32Ret,"send start msg for poweron action");

            g_stSTATEMNGContext.bPoweronAction = HI_FALSE;
        }

        /** publish HI_EVENT_STATEMNG_SD_AVAILABLE event */
        s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SD_AVAILABLE,
                    0, 0, HI_FALSE, 0L, NULL, 0);
        PDT_STATEMNG_CHECK_RET(s32Ret,"publish sd available event");
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateUnplugedMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD(YELLOW"process unplug msg(0x%08X)\n"NONE, pstMsg->what);

    s32Ret = HI_FILEMNG_SetDiskState(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"filemng set disk state(HI_FALSE)");

    /** cancel bPoweronAction */
    if(HI_TRUE == g_stSTATEMNGContext.bPoweronAction)
    {
        MLOGD("set bPoweronAction(HI_FALSE)\n");
        g_stSTATEMNGContext.bPoweronAction = HI_FALSE;
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateErrorMsgProc(HI_MESSAGE_S *pstMsg)
{
    MLOGD(YELLOW"process sd error msg(0x%08X)\n"NONE, pstMsg->what);

    /** cancel g_bPoweronAction */
    if(HI_TRUE == g_stSTATEMNGContext.bPoweronAction)
    {
        MLOGD("set bPoweronAction(HI_FALSE)\n");
        g_stSTATEMNGContext.bPoweronAction = HI_FALSE;
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateFormatMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD(YELLOW"process format msg(0x%08X)\n"NONE, pstMsg->what);

    /** check IsRunning */
    if(HI_TRUE == g_stSTATEMNGContext.bRunning)
    {
        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
            g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    s32Ret = HI_FILEMNG_SetDiskState(HI_FALSE);
    if(HI_FILEMNG_ENOTINIT == s32Ret)
    {
        MLOGI(BLUE"FileMng not inited\n"NONE);
        s32Ret = HI_SUCCESS;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"filemng set disk state(HI_FALSE)");
    }

    s32Ret = HI_STORAGEMNG_Format(g_stSTATEMNGContext.stStorageMngCfg.szMntPath, pstMsg->arg1);
    PDT_STATEMNG_CHECK_RET(s32Ret,"storagemng format");

    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    HI_S32 loop_count = 10;
    while ((HI_STORAGE_STATE_MOUNTED != enState) && (loop_count-- > 0))
    {
        s32Ret = HI_STORAGEMNG_GetState(
            g_stSTATEMNGContext.stStorageMngCfg.szMntPath, &enState);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get storage state");
        HI_usleep(100*1000);
    }

    if(HI_STORAGE_STATE_MOUNTED == enState)
    {
        /** init filemng */
        s32Ret = PDT_STATEMNG_InitFileMng();
        PDT_STATEMNG_CHECK_RET(s32Ret, "init filemng");

        MLOGD(YELLOW"filemng set disk state(HI_TRUE)\n"NONE);
        s32Ret = HI_FILEMNG_SetDiskState(HI_TRUE);
        if (HI_SUCCESS == s32Ret) {
            s32Ret = HI_FILEMNG_CheckDiskSpace();
            PDT_STATEMNG_CHECK_RET(s32Ret, "check disk space err");

            /** publish HI_EVENT_STATEMNG_SD_AVAILABLE event */
            s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SD_AVAILABLE,
                        0, 0, HI_FALSE, 0L, NULL, 0);
            PDT_STATEMNG_CHECK_RET(s32Ret,"publish sd available event");
        }
        else
        {
            MLOGD(YELLOW"filemng set disk state(HI_TRUE) failed\n"NONE);
            return HI_FAILURE;
        }
    }
    else
    {
        MLOGE(RED"NO set disk state(HI_TRUE)\n"NONE);
        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
            g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }

    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateFileEndMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_CHAR* pszFilePath = pstMsg->aszPayload;
    if(HI_EVENT_RECMNG_FILE_END == pstMsg->what)
    {
        MLOGD(YELLOW"process rec file end msg(0x%08X), name(%s)\n"NONE, pstMsg->what, pszFilePath);
    }
    else if(HI_EVENT_PHOTOMNG_FILE_END == pstMsg->what)
    {
        MLOGD(YELLOW"process photo file end msg(0x%08X), name(%s)\n"NONE, pstMsg->what, pszFilePath);
    }

    s32Ret =  HI_FILEMNG_AddFile(pszFilePath);
    if (HI_FILEMNG_ENOTMAIN == s32Ret || HI_FILEMNG_EEXIST == s32Ret)
    {
        MLOGD(YELLOW"Not main file or file exist\n"NONE);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"filemng add file");
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_S32 PDT_STATEMNG_BaseStateVoSwitchMsgProc(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_S32 s32Switch = *(HI_S32*)pvPayload;
    MLOGD(YELLOW"process vo switch msg(0x%08X), switch(%d)\n"NONE, pstMsg->what, s32Switch);

    if(!s32Switch)
    {
#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set back light(off)");

        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_OFF);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(off)");
#endif
        /** stop VO */
        s32Ret = HI_PDT_MEDIA_VideoOutStop();
        if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            s32Ret = HI_SUCCESS;
            MLOGD(YELLOW"not inited\n"NONE);
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret,"stop VO");
        }

#if 0
        /** Video out deinit */
        s32Ret = HI_PDT_MEDIA_VideoOutDeinit();
        if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            s32Ret = HI_SUCCESS;
            MLOGD(YELLOW"Media or VO not inited\n\n"NONE);
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret,"deinit VO");
        }
#endif

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        g_stSTATEMNGContext.bVOShutdown = HI_TRUE;
        if(HI_SUCCESS == s32Ret)
        {
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
        }
        else
        {
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_FALSE);
        }
        g_stSTATEMNGContext.bInProgress = HI_FALSE;
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    else
    {
        /** init VO */
        s32Ret = HI_PDT_MEDIA_VideoOutInit(&(g_stSTATEMNGContext.stMediaCfg.stVideoOutCfg));
        if(HI_PDT_MEDIA_EINITIALIZED == s32Ret)
        {
            MLOGD(YELLOW"already inited\n"NONE);
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret,"init VO");
        }

        /** start VO */
        s32Ret = HI_PDT_MEDIA_VideoOutStart();
        if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            MLOGD(YELLOW"not inited\n"NONE);
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret,"start VO");
        }

#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(on)");

        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(on)");
#endif

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        g_stSTATEMNGContext.bVOShutdown = HI_FALSE;
        if(HI_SUCCESS == s32Ret)
        {
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
        }
        else
        {
            PDT_STATEMNG_PublishResultEvent(pstMsg, HI_FALSE);
        }
        g_stSTATEMNGContext.bInProgress = HI_FALSE;
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
}

static HI_S32 PDT_STATEMNG_ExitProc(HI_VOID)
{
    if(HI_EXIT_MODE_POWEROFF == g_stSTATEMNGContext.enExitMode)
    {
        MLOGI(YELLOW"process poweroff msg\n"NONE);
        sync();
        if(NULL != g_stSTATEMNGContext.stStatemngCfg.pfnExitCB)
        {
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_BUTT;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.stStatemngCfg.pfnExitCB(HI_EXIT_MODE_POWEROFF);
        }
        else
        {
            MLOGE(RED"pfnExitCB(null) err\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }

        return HI_PROCESS_MSG_RESULTE_OK;
    }

    if(HI_EXIT_MODE_RESET == g_stSTATEMNGContext.enExitMode)
    {
        MLOGI(YELLOW"process facrory reset msg\n"NONE);

        if(NULL != g_stSTATEMNGContext.stStatemngCfg.pfnExitCB)
        {
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_BUTT;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.stStatemngCfg.pfnExitCB(HI_EXIT_MODE_RESET);
        }
        else
        {
            MLOGE(RED"pfnExitCB(null) err\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }

        return HI_PROCESS_MSG_RESULTE_OK;
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

static HI_VOID PDT_STATEMNG_GetVencDstFrameRate(HI_MEDIA_VENC_ATTR_S* pstVencAttr, HI_S32* ps32DstFrmRate)
{
    if(HI_MAPI_PAYLOAD_TYPE_H264 == pstVencAttr->stTypeAttr.enType)
    {
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
        {
            *ps32DstFrmRate = pstVencAttr->stRcAttr.unAttr.stH264Cbr.stAttr.fr32DstFrameRate;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencAttr->stRcAttr.enRcMode)
        {
            *ps32DstFrmRate = pstVencAttr->stRcAttr.unAttr.stH264Vbr.stAttr.fr32DstFrameRate;
        }
    }

    if(HI_MAPI_PAYLOAD_TYPE_H265 == pstVencAttr->stTypeAttr.enType)
    {
        if(HI_MAPI_VENC_RC_MODE_CBR == pstVencAttr->stRcAttr.enRcMode)
        {
            *ps32DstFrmRate = pstVencAttr->stRcAttr.unAttr.stH265Cbr.stAttr.fr32DstFrameRate;
        }
        else if (HI_MAPI_VENC_RC_MODE_VBR == pstVencAttr->stRcAttr.enRcMode)
        {
            *ps32DstFrmRate = pstVencAttr->stRcAttr.unAttr.stH265Vbr.stAttr.fr32DstFrameRate;
        }
    }
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

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(g_stSTATEMNGContext.Mutex, pvArg, pStateID,
        pstMsg, g_stSTATEMNGContext.bInProgress);

    /** use the gloable context enCurrentWorkMode to process message */
    HI_CHAR szCurrentStateName[HI_STATE_NAME_LEN] = {0};
    s32Ret = PDT_STATEMNG_WorkMode2State(g_stSTATEMNGContext.enCurrentWorkMode,
        szCurrentStateName, HI_STATE_NAME_LEN);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode name");
    MLOGD("curr state(%s), bRunning(%d)\n", szCurrentStateName, g_stSTATEMNGContext.bRunning);

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
        /** switch sd unpluged message process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        {
            return PDT_STATEMNG_BaseStateUnplugedMsgProc(pstMsg);
        }
        /** switch sd error message process */
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
        {
            return PDT_STATEMNG_BaseStateErrorMsgProc(pstMsg);
        }
        /** switch sd format message process */
        case HI_EVENT_STATEMNG_SD_FORMAT:
        {
            return PDT_STATEMNG_BaseStateFormatMsgProc(pstMsg);
        }
        case HI_EVENT_STATEMNG_POWEROFF:
        {
            MLOGI(YELLOW"process poweroff\n"NONE);
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_POWEROFF;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
            return PDT_STATEMNG_ExitProc();
        }
        case HI_EVENT_RECMNG_FILE_END:
        case HI_EVENT_PHOTOMNG_FILE_END:
        {
            if(HI_EVENT_RECMNG_FILE_END == pstMsg->what)
            {
                g_stSTATEMNGContext.s32RecPair += 1;
                g_stSTATEMNGContext.s32RecPair %= 2;
            }

            if(g_stSTATEMNGContext.enExitMode == HI_EXIT_MODE_BUTT)
            {
                return PDT_STATEMNG_BaseStateFileEndMsgProc(pstMsg);
            }
            else
            {
                PDT_STATEMNG_BaseStateFileEndMsgProc(pstMsg);

                if(HI_EVENT_RECMNG_FILE_END == pstMsg->what)
                {
                    /** main stream and sub stream all end, process poweroff msg */
                    if((g_stSTATEMNGContext.s32RecPair %= 2) == 0)
                    {
                        return PDT_STATEMNG_ExitProc();
                    }
                }

                if(HI_EVENT_PHOTOMNG_FILE_END == pstMsg->what)
                {
                    return PDT_STATEMNG_ExitProc();
                }
            }
        }
        break;
        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            HI_BOOL bNeedProcess = HI_FALSE;
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            if(g_stSTATEMNGContext.enExitMode != HI_EXIT_MODE_BUTT)
            {
                bNeedProcess = HI_TRUE;
            }
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

            if(HI_TRUE == bNeedProcess)
            {
                return PDT_STATEMNG_ExitProc();
            }

            return HI_PROCESS_MSG_RESULTE_OK;
        }
        break;
        case HI_EVENT_STATEMNG_VO_SWITCH:
        {
            return PDT_STATEMNG_BaseStateVoSwitchMsgProc(pstMsg);
        }
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        case HI_EVENT_STATEMNG_RESET:
        {
            MLOGD(YELLOW"process msg(0x%08X)\n"NONE, pstMsg->what);
            if(HI_EVENT_STATEMNG_FACTORY_RESET == pstMsg->what)
            {
                s32Ret = HI_PDT_PARAM_SetDefault();
                PDT_STATEMNG_CHECK_RET(s32Ret,"set param to default");
            }
            MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
            g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_RESET;
            MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

            return PDT_STATEMNG_ExitProc();
        }
        case HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE:
        {
            MLOGD("framerate change(%d fps)\n", pstMsg->arg1);

            /** check main steam venc enable */
            if(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[0].bEnable)
            {
                HI_HANDLE VencHdl = 0;
                VENC_FRAME_RATE_S stFRParam;
                MLOGD(YELLOW"update main stream venc srcFrameRate(%d fps)\n"NONE, pstMsg->arg1);
                stFRParam.s32SrcFrmRate = pstMsg->arg1;

                HI_S32 s32DstFrmRate = pstMsg->arg1;
                PDT_STATEMNG_GetVencDstFrameRate(
                    &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[0].stVencAttr),
                    &s32DstFrmRate);
                stFRParam.s32DstFrmRate = s32DstFrmRate;
                MLOGD(YELLOW"update main stream venc dstFrameRate(%d fps)\n"NONE, s32DstFrmRate);

                s32Ret = HI_MAPI_VENC_SetAttrEx(VencHdl, HI_MAPI_VENC_CMD_FRAME_RATE,
                    &stFRParam, sizeof(VENC_FRAME_RATE_S));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            /** check sub steam venc enable */
            if(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[1].bEnable)
            {
                HI_HANDLE VencHdl = 1;
                VENC_FRAME_RATE_S stFRParam;
                MLOGD(YELLOW"update sub stream venc srcFrameRate(%d fps)\n"NONE, pstMsg->arg1);
                stFRParam.s32SrcFrmRate = pstMsg->arg1;

                HI_S32 s32DstFrmRate = pstMsg->arg1;
                PDT_STATEMNG_GetVencDstFrameRate(
                    &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[1].stVencAttr),
                    &s32DstFrmRate);
                stFRParam.s32DstFrmRate = s32DstFrmRate;
                MLOGD(YELLOW"update sub stream venc dstFrameRate(%d fps)\n"NONE, s32DstFrmRate);

                s32Ret = HI_MAPI_VENC_SetAttrEx(VencHdl, HI_MAPI_VENC_CMD_FRAME_RATE,
                    &stFRParam, sizeof(VENC_FRAME_RATE_S));
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            break;
        }
        case HI_EVENT_STATEMNG_DONOTUPGRADE:
        {
            s32Ret = PDT_STATEMNG_DoNotUpgrade(pstMsg);
            PDT_STATEMNG_CHECK_RET(s32Ret,"PDT_STATEMNG_DoNotUpgrade");
            break;
        }
        default:
        {
            MLOGD(YELLOW"can't process msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

HI_S32 PDT_STATEMNG_BaseStateInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    g_stSTATEMNGContext.stBase.stState.stateID = HI_PDT_WORKMODE_BUTT;
    snprintf(g_stSTATEMNGContext.stBase.stState.name,
        HI_STATE_NAME_LEN, "%s", PDT_STATEMNG_STATE_BASE);
    g_stSTATEMNGContext.stBase.stState.enter = PDT_STATEMNG_BaseStateEnter;
    g_stSTATEMNGContext.stBase.stState.exit = PDT_STATEMNG_BaseStateExit;
    g_stSTATEMNGContext.stBase.stState.processMessage = PDT_STATEMNG_BaseStateMsgProc;
    g_stSTATEMNGContext.stBase.stState.argv = &(g_stSTATEMNGContext.stBase);

    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &(g_stSTATEMNGContext.stBase.stState), NULL);
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
