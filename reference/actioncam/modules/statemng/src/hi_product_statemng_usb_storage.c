/**
 * \file   hi_product_statemng_usb_storage.c
 * \brief  Realize the interface about usb storage state.
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

/** enter USBStorage state */
HI_S32 PDT_STATEMNG_USBStorageStateEnter(HI_VOID *pvArg)
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

    g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
    g_stSTATEMNGContext.bRunning = HI_FALSE;

#if 1
    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    s32Ret = HI_PDT_MEDIA_SetTimeOSD(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret, "Shutdown TimeOSD");

    s32Ret = HI_PDT_MEDIA_AudioOutStop();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop AO");
    }

    s32Ret = HI_PDT_MEDIA_AudioDeinit();
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit AO");
    }

    s32Ret = HI_PDT_MEDIA_VideoDeinit(HI_TRUE);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"deinit video");
    }

#endif

    /** deinit filemng */
    s32Ret = HI_FILEMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret, "deinit filemng");

    /** deinit storagemng */
    sync();
    HI_STORAGEMNG_CFG_S stStorageMngCfg;
    memset(&stStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, "get storage cfg");
    MLOGD("DevPath(%s) MountPath(%s)\n"NONE, stStorageMngCfg.szDevPath, stStorageMngCfg.szMntPath);
    s32Ret = HI_STORAGEMNG_Destroy(stStorageMngCfg.szMntPath);
    PDT_STATEMNG_CHECK_RET(s32Ret, "destroy storagemng");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, enEnterWorkMode, HI_TRUE, 0L, NULL, 0);
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

/** exit USBStorage state */
HI_S32 PDT_STATEMNG_USBStorageStateExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    /** init storagemng*/
    HI_STORAGEMNG_CFG_S stStorageMngCfg;
    memset(&stStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, "get storage cfg");
    MLOGD("DevPath(%s) MountPath(%s)\n"NONE, stStorageMngCfg.szDevPath, stStorageMngCfg.szMntPath);

    HI_STORAGEMNG_CALLBACK_S stCallBack;
    stCallBack.pfnFormatPreProc = g_stSTATEMNGContext.stStatemngCfg.pfnFormatPreProc;
    s32Ret = HI_STORAGEMNG_Create(&stStorageMngCfg, &stCallBack);
    PDT_STATEMNG_CHECK_RET(s32Ret, "create storagemng");

    return HI_SUCCESS;
}

/** USBStorage state message process */
HI_S32 PDT_STATEMNG_USBStorageStateMsgProc(HI_MESSAGE_S *pstMsg,
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
            PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_RECMNG_ERROR:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        /** need base state process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        default:
        {
            MLOGD(YELLOW"pload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
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
          NULL}};
    stUSBStorageState.stState.argv = &stUSBStorageState;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stUSBStorageState.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add USB Storage");

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

