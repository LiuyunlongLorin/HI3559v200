/**
 * \file   hi_product_statemng_usb_storage.c
 * \brief  Realize the interface about usb storage state.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include <unistd.h>
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"
#include "hi_product_scene.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** enter USBStorage state */
HI_S32 PDT_STATEMNG_USBStorageStateEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"try to enter state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"change state to workmode");

    pstStateMngCtx->enCurrentWorkMode = enEnterWorkMode;
    pstStateMngCtx->bRunning = pstStateAttr->bRunning;

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"scene pause");

    /* keep AO for keytone and voiceplay*/

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

    s32Ret = HI_FILEMNG_SetDiskState(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "deinit filemng");

    /** deinit storagemng */
    sync();
    pstStateMngCtx->bSDAvailable = HI_FALSE;
    s32Ret = HI_STORAGEMNG_Destroy(pstStateMngCtx->stStorageMngCfg.szMntPath);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "destroy storagemng");

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

/** exit USBStorage state */
HI_S32 PDT_STATEMNG_USBStorageStateExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg,HI_PDT_STATEMNG_ENULLPTR,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
                pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** init storagemng*/
    HI_STORAGEMNG_CALLBACK_S stCallBack;
    stCallBack.pfnFormatPreProc = pstStateMngCtx->stStatemngCfg.pfnFormatPreProc;
    s32Ret = HI_STORAGEMNG_Create(&pstStateMngCtx->stStorageMngCfg,&stCallBack);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_STORAGEMNG_Create");

    /** init filemng */
#if 0
    s32Ret = HI_FILEMNG_Init(&pstStateMngCtx->stFileMngCfg.stCommCfg, &pstStateMngCtx->stFileMngCfg.stDtcfCfg);
#endif
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER, "HI_FILEMNG_Init");
    HI_system("echo 3 > /proc/sys/vm/drop_caches");
    pstStateMngCtx->bForceReBuid = HI_TRUE;
    return HI_SUCCESS;
}

/** USBStorage state message process */
HI_S32 PDT_STATEMNG_USBStorageStateMsgProc(HI_MESSAGE_S *pstMsg,
                                           HI_VOID *pvArg,
                                           HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg,
                                          pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n",
          pstStateAttr->stState.name, (pstStateAttr->bRunning));

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"change state to workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        case HI_EVENT_STATEMNG_STOP:
        case HI_EVENT_STATEMNG_TRIGGER:
        case HI_EVENT_STATEMNG_SETTING:
        {
            MLOGD(YELLOW"no need to process message what(%x)\n\n"NONE,
                        pstMsg->what);
            PDT_STATEMNG_UPDATESTATUS(pstMsg,HI_FALSE,HI_FALSE);
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

/** init USBStorage state, add USBStorage state to HFSM */
HI_S32 PDT_STATEMNG_USBStorageStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stUSBStorageState =
        {{HI_PDT_WORKMODE_USB_STORAGE,
          PDT_STATEMNG_STATE_USB_STORAGE,
          PDT_STATEMNG_USBStorageStateEnter,
          PDT_STATEMNG_USBStorageStateExit,
          PDT_STATEMNG_USBStorageStateMsgProc,
          NULL},HI_FALSE};
    stUSBStorageState.stState.argv = &stUSBStorageState;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stUSBStorageState.stState,
                              (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HFSM add Playback state");

    return s32Ret;
}

/** deinit USBStorage State module */
HI_S32 PDT_STATEMNG_USBStorageStateDeinit()
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

