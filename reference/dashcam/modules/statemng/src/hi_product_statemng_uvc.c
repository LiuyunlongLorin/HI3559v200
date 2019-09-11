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

static HI_S32 PDT_STATEMNG_ResetUVCMediaCfg(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_UVC_CFG_S *pstUvcCfg = NULL;
    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_UVC;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enWorkMode, &stWorkModeCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "GetUVCAttr");
    pstUvcCfg = &stWorkModeCfg.unModeCfg.stUvcCfg.stUvcCfg;

    HI_S32 i, j, k;

    /* disable venc */
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        pstMediaCfg->stVideoCfg.astVencCfg[i].bEnable = HI_FALSE;
    }

    /* disable osd */
    pstMediaCfg->stVideoCfg.stOsdCfg.s32OsdCnt = 0;

    /* disable display */
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        pstMediaCfg->stVideoOutCfg.astDispCfg[i].bEnable = HI_FALSE;
    }

    /* disable unused vpss */
    HI_BOOL bVpssFound = HI_FALSE;
    HI_HANDLE VcapPipeHdl = -1;
    HI_HANDLE VcapPipeChnHdl = -1;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++i)
    {
        if ((!pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].bEnable) ||
                (pstUvcCfg->stDataSource.VprocHdl != pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].VpssHdl))
        {
            pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].bEnable = HI_FALSE;
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if ((!pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].bEnable) ||
                (pstUvcCfg->stDataSource.VportHdl != pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].VportHdl))
            {
                pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].bEnable = HI_FALSE;
                continue;
            }
            bVpssFound = HI_TRUE;
            VcapPipeHdl = pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].VcapPipeHdl;
            VcapPipeChnHdl = pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].VcapPipeChnHdl;
        }
    }
    if (!bVpssFound)
    {
        MLOGE("Corresponding UVC VpssPort not found\n");
        return HI_FAILURE;
    }

    /* disable unused vcap */
    HI_BOOL bVcapFound = HI_FALSE;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if ((!pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable) ||
                (VcapPipeHdl != pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl))
            {
                pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable = HI_FALSE;
                continue;
            }

            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if ((!pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable) ||
                    (VcapPipeChnHdl != pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl))
                {
                    pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable = HI_FALSE;
                    continue;
                }
                bVcapFound = HI_TRUE;
            }
        }

        if (bVcapFound)
        {
            break;
        }
        else
        {
            pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable = HI_FALSE;
        }
    }

    for (j = i + 1; j < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++j)
    {
        pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[j].bEnable = HI_FALSE;
    }
    if (!bVcapFound)
    {
        MLOGE("Corresponding UVC Vcap not found\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/** enter UVC state */
HI_S32 PDT_STATEMNG_UVCStateEnter(HI_VOID *pvArg)
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
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"change state to workmode");

    pstStateMngCtx->enCurrentWorkMode = enEnterWorkMode;
    pstStateMngCtx->bRunning = pstStateAttr->bRunning;

    /** get media config **/
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /* reset media config for uvc */
    s32Ret = PDT_STATEMNG_ResetUVCMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media config");

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"reset media");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    HI_EVENT_S stEvent;
    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stEvent.arg2 = enEnterWorkMode;
    stEvent.s32Result = HI_SUCCESS;
    MUTEX_LOCK(pstStateMngCtx->Mutex);
    pstStateMngCtx->bInProgress = HI_FALSE;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    HI_EVTHUB_Publish(&stEvent);
    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return HI_SUCCESS;
}

/** exit UVC state */
HI_S32 PDT_STATEMNG_UVCStateExit(HI_VOID *pvArg)
{
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR," pvArg ");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));
    HI_system("echo 3 > /proc/sys/vm/drop_caches");
    pstStateMngCtx->bForceReBuid = HI_TRUE;
    return HI_SUCCESS;
}

/** UVC state message process */
HI_S32 PDT_STATEMNG_UVCStateMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg,
                                          pStateID,
                                          pstMsg,
                                          pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n",
          pstStateAttr->stState.name, (pstStateAttr->bRunning));

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"change state to workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        case HI_EVENT_STATEMNG_STOP:
        case HI_EVENT_STATEMNG_TRIGGER:
        case HI_EVENT_STATEMNG_SETTING:
        case HI_EVENT_STATEMNG_LIVE_SWITCH:
        case HI_EVENT_STATEMNG_VO_SWITCH:
        case HI_EVENT_STATEMNG_PREVIEW_PIP:
        {
            MLOGD(YELLOW"no need to process message what(%x)\n\n"NONE,
                        pstMsg->what);
            PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FAILURE,HI_FALSE);
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
          NULL},HI_FALSE};
    stUVCState.stState.argv = &stUVCState;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stUVCState.stState,
                              (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM add Playback state");
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



