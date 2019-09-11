/**
 * \file   hi_product_statemng_rec.c
 * \brief  Realize the interface about record states.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include <unistd.h>
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"
#include "hi_liveserver.h"
#include "hi_ahdmng.h"
#include "hi_product_scene.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_S32 PDT_STATEMNG_GetJPEGUserInfo(HI_PHOTOMNG_JPEG_USERINFO_S* pstJPEGUserInfo)
{
    static HI_CHAR* pszUniqueCameraModel = "HiMobileCam DashCam";
    pstJPEGUserInfo->u32UniqueCameraModelLen = strlen(pszUniqueCameraModel)+1;
    pstJPEGUserInfo->pUniqueCameraModel = pszUniqueCameraModel;

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_PhotoStatesSetMediaMode(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS, s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_PDT_MEDIAMODE_E enSettingMediaMode;
    memcpy(&enSettingMediaMode, pstMsg->aszPayload, sizeof(HI_PDT_MEDIAMODE_E));
    MLOGD(YELLOW"setting MediaMode(%d) for workmode(%d)\n\n"NONE, enSettingMediaMode, enSettingWorkMode);

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        if (pstStateMngCtx->s32PreviewCamID == pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID)
        {
            break;
        }
    }

    if (s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("PreviewCamID[%d] error \n", pstStateMngCtx->s32PreviewCamID);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** filter out the same parameter */
    if (enSettingMediaMode == pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.enMediaMode)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingMediaMode value */
    s32Ret = PDT_STATEMNG_CheckCamSettingItemValue(pstMsg, pstStateMngCtx->s32PreviewCamID, (HI_S32)enSettingMediaMode);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"the setting item value not support\n\n"NONE);
        return s32Ret;
    }

    pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.enMediaMode = enSettingMediaMode;
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode, s32Idx, HI_PDT_PARAM_TYPE_MEDIAMODE, &enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "set media mode failed");

    /** get setting MediaModeCfg from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaModeCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stSettingMediaModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** Reset Media for setting parameter */
    s32Ret = PDT_STATEMNG_ResetMedia(&stSettingMediaModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_MEDIAMODE,
                                      (HI_VOID*)&enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set stSettingMediaMode parameter");
    return s32Ret;
}


HI_S32 PDT_STATEMNG_PhotoStatesSettingMsgProc(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (pstMsg->arg1)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_STATEMNG_PhotoStatesSetMediaMode(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_STATEMNG_SetFlip(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_MIRROR:
            s32Ret = PDT_STATEMNG_SetMirror(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_STATEMNG_SetLDC(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_STATEMNG_SetOSD(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_CROP:
            s32Ret = PDT_STATEMNG_SetCrop(pstMsg);
            break;

        default:
            MLOGE(YELLOW"not support param type(%d)\n\n"NONE, pstMsg->arg1);
            return HI_PDT_STATEMNG_EINVAL;
    }

    return s32Ret;
}


/** enter Photo states */
HI_S32 PDT_STATEMNG_PhotoStatesEnter(HI_VOID* pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR, "parameter pvArg");

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD(YELLOW"try to enter state(%s), bRunning(%d)\n\n"NONE, pstStateAttr->stState.name, (pstStateAttr->bRunning));
    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "change state to workmode");

    if (HI_PDT_WORKMODE_PHOTO != enEnterWorkMode)
    {
        MLOGE(RED"enter workmode[%d] error\n\n"NONE, enEnterWorkMode);
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
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");

    /** generate photo Context */
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
    {
        s32Ret = PDT_STATEMNG_GeneratePhotoCtx(s32Idx, enEnterWorkMode, &stEnterWorkModeCfg,
                                               &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx]);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate photo context");
        if(HI_TRUE == stEnterMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[s32Idx].astVcapPipeAttr[0].bIspBypass)
        {
            PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].bISPBypss = HI_TRUE;
        }
        else
        {
            PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].bISPBypss = HI_FALSE;
        }

        if(HI_TRUE == pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable)
        {
            /*create the photo task*/
            HI_PHOTOMNG_TASK_CFG_S stPhotoTaskCfg;
            memset(&stPhotoTaskCfg,'\0',sizeof(HI_PHOTOMNG_TASK_CFG_S));
            pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].s32CamID = s32Idx;
            stPhotoTaskCfg.stGetNameCB.pfnGetNameCallBack = PDT_STATEMNG_GetPhotoFileName;
            stPhotoTaskCfg.stGetNameCB.pvPrivateData = &PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx];
            stPhotoTaskCfg.stGetParamCB.pfnGetParamCallBack = PDT_STATEMNG_GetPhotoParam;
            stPhotoTaskCfg.stGetParamCB.pvPrivateData = &PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx];
            MLOGD("photo task create[%d]!!\n",s32Idx);
            s32Ret = HI_PHOTOMNG_TaskCreate(&stPhotoTaskCfg, &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "Photo task create");

            /*set photo attr */
            s32Ret = HI_PHOTOMNG_SetPhotoAttr(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl,
                &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].stPhotoAttr);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set photo attr");
        }
    }

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset media");

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

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event failed\n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }
    else
    {
        MLOGD(YELLOW"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE(%x), arg2(%s) succeed\n\n"NONE,
              stEvent.EventID, pstStateAttr->stState.name);
    }

    /** save poweron workmode */
    s32Ret = PDT_STATEMNG_SetPoweronWorkMode(enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "save poweron workmode param");

    HI_TIME_STAMP;
    return s32Ret;
}

/** exit Photo states */
HI_S32 PDT_STATEMNG_PhotoStatesExit(HI_VOID* pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR, "pvArg");

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
          pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "change state to workmode");

    if (HI_PDT_WORKMODE_PHOTO != enExitWorkMode)
    {
        MLOGE(RED"exit workmode[%d] error \n\n"NONE, enExitWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }

    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /*destory the photo task*/
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        if(HI_TRUE == pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable)
        {
            s32Ret = HI_PHOTOMNG_TaskDestroy(PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "photo task destory");
        }

    }

    return s32Ret;
}

/** photo states message process */
HI_S32 PDT_STATEMNG_PhotoStatesMsgProc(HI_MESSAGE_S* pstMsg, HI_VOID* pvArg, HI_STATE_ID* pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n", pstStateAttr->stState.name, pstStateAttr->bRunning);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "change state to workmode");

    MLOGD(YELLOW" will process message what(%x) \n\n"NONE, pstMsg->what);
    switch (pstMsg->what)
    {
        case HI_EVENT_STATEMNG_TRIGGER:
        {
            HI_BOOL bPhotoEnable = HI_FALSE;
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            if (HI_TRUE == pstStateMngCtx->bSDAvailable)
            {
                for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
                {
                    if(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable && pstStateMngCtx->astCamCtx[s32Idx].bEnable)
                    {
                        s32Ret = HI_PHOTOMNG_TaskStart(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);

                        if (HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED" start photo task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl, s32Ret);
                            PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                            return HI_PROCESS_MSG_RESULTE_OK;
                        }
                        else
                        {
                            bPhotoEnable = HI_TRUE;
                        }
                    }
                }
                if(HI_TRUE != bPhotoEnable)
                {
                    MLOGW("No enabled photo task execution!!\n");
                    PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                }else
                {
                    PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
                }
            }
            else
            {
                MLOGE(RED"sd not available, start emr record task error\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
            }

            MLOGD("bInProgress:%d \n", pstStateMngCtx->bInProgress);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            MLOGD(YELLOW"no need to process message HI_EVENT_PHOTOMNG_TASK_END(%x)\n\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        /** need pre-process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        {
            if (HI_STORAGE_STATE_DEV_UNPLUGGED == pstStateMngCtx->enPowerOnState)
            {
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            else if(HI_STORAGE_STATE_IDEL == pstStateMngCtx->enPowerOnState)
            {
                /**let base state to chage POS be unpluggedcd*/
                MLOGI("Storage PowerOn State is %d.\n",pstStateMngCtx->enPowerOnState);
                return HI_PROCESS_MSG_UNHANDLER;
            }
        }

        case HI_EVENT_AHDMNG_PLUG_STATUS:
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_SYATEM_REBOOT:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        {
            for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
            {
                if(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable && pstStateMngCtx->astCamCtx[s32Idx].bEnable)
                {
                    s32Ret = HI_PHOTOMNG_TaskStop(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);
                }
            }

            sync();

            /** sd destroy */
            HI_STORAGEMNG_CFG_S stStorageMngCfg;
            memset(&stStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
            HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
            HI_STORAGEMNG_Destroy(stStorageMngCfg.szMntPath);
            return HI_PROCESS_MSG_UNHANDLER;/** need base state precess */
        }

        /** PhotoMng error */
        case HI_EVENT_PHOTOMNG_ERROR:
        /** FileMng space full */
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"process msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_UNHANDLER;
        }

        case HI_EVENT_STATEMNG_SETTING:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            /** process all the setting type*/
            s32Ret = PDT_STATEMNG_PhotoStatesSettingMsgProc(pstMsg);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE(YELLOW"set msg proc fail, s32Ret(%d)\n\n"NONE, s32Ret);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
            }
            else
            {
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
            }

            return HI_PROCESS_MSG_RESULTE_OK;
        }

        default:
        {
            /** need base state precess */
            MLOGD(YELLOW"can not process this message, upload it to Base state\n\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Photo states, add normal Photo states to HFSM, init PhotoMng module */
HI_S32 PDT_STATEMNG_PhotoStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stNormalPhoto =
    {
        {
            HI_PDT_WORKMODE_PHOTO,
            PDT_STATEMNG_STATE_PHOTO,
            PDT_STATEMNG_PhotoStatesEnter,
            PDT_STATEMNG_PhotoStatesExit,
            PDT_STATEMNG_PhotoStatesMsgProc,
            NULL
        }, HI_FALSE
    };
    stNormalPhoto.stState.argv = &stNormalPhoto;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stNormalPhoto.stState,
                              (HI_STATE_S*) & (pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HFSM add Photo state");

    HI_PHOTOMNG_VENC_OPERATE_S stVencOps;
    stVencOps.pfnStart = HI_PDT_MEDIA_VencStart;
    stVencOps.pfnStop = HI_PDT_MEDIA_VencStop;
    s32Ret = HI_PHOTOMNG_Init(&stVencOps);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "Photo init");
    s32Ret = HI_PHOTOMNG_RegGetJPEGUserInfoCB(PDT_STATEMNG_GetJPEGUserInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"RegGetJPEGUserInfoCB");

    return s32Ret;
}

/** deinit PhotoMng module */
HI_S32 PDT_STATEMNG_PhotoStatesDeinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit APP Common PhotoMng module */
    s32Ret = HI_PHOTOMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "photo Deinit");

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
