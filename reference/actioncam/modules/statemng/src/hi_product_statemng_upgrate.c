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
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern sem_t g_PowerOffSem;
extern PDT_STATEMNG_CONTEXT g_stSTATEMNGContext;

/**Upgrade Mode Abnormal*/
HI_VOID PDT_STATEMNG_UpgradeAbnormal(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    g_stSTATEMNGContext.bRunning = HI_FALSE;
    g_stSTATEMNGContext.bInProgress = HI_FALSE;

    /** publish HI_EVENT_STATEMNG_UPGRADE_ABNORMAL event to EventHub */
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_UPGRADE_ABNORMAL,
                0, 0, HI_FALSE, 0L, NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish switch workmode failed(0x%08X)\n"NONE, s32Ret);
    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
}

/** Enter Upgrade state */
HI_S32 PDT_STATEMNG_UpgradeStateEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"try to enter state(%s)\n\n"NONE,
                pstStateAttr->stState.name);

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    if(HI_PDT_WORKMODE_UPGRADE != enEnterWorkMode)
    {
        MLOGE(RED"enter workmode[%d] error\n\n"NONE,enEnterWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }
    else
    {
        g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
        g_stSTATEMNGContext.bRunning = HI_FALSE;
    }

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
        g_stSTATEMNGContext.bRunning = HI_TRUE;
        g_stSTATEMNGContext.bInProgress = HI_FALSE;
    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** Exit Upgrade state */
HI_S32 PDT_STATEMNG_UpgradeStateExit(HI_VOID *pvArg)
{
    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    return HI_SUCCESS;
}

/** Upgrade state message process */
HI_S32 PDT_STATEMNG_UpgradeStateMsgProc(HI_MESSAGE_S *pstMsg, HI_MW_PTR pvArg, HI_STATE_ID *pStateID)
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
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_SD_FORMAT:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        case HI_EVENT_STATEMNG_DEBUG_SWITCH:
        {
            MLOGD(YELLOW"not process msg(0x%08X)\n"NONE, pstMsg->what);
            PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_UPGRADE_START:
        {
            HI_EVENT_S stUpgradeEvent = {};
            HI_S32 loop_count = 10;

            while (loop_count-- > 0)
            {
                s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_UPGRADE_NEWPKT, &stUpgradeEvent);
                if (HI_SUCCESS == s32Ret)
                {
                    /**upgrade*/
                    MLOGI("start board upgrade ... ...\n");

                    s32Ret = HI_UPGRADE_DoUpgrade(g_stSTATEMNGContext.stStorageMngCfg.szMntPath, stUpgradeEvent.aszPayload);
                    if (HI_SUCCESS == s32Ret)
                    {
                        MLOGI("Do Upgrade Success, reboot!\n");
                        MLOGI(YELLOW"try to process message HI_EVENT_STATEMNG_SYATEM_REBOOT"NONE);
                        HI_PDT_PARAM_Deinit();
                        HI_SYSTEM_Reboot();
                    }
                }

                HI_usleep(100*1000);
            }

            PDT_STATEMNG_UpgradeAbnormal();
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_UPGRADE_FAILED:
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        {
            PDT_STATEMNG_UpgradeAbnormal();
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        default:
        {
            MLOGD(YELLOW"pload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

/** Init Upgrade state */
HI_S32 PDT_STATEMNG_UpgradeStateInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret;

    static PDT_STATEMNG_STATE_ATTR_S stUpgrade =
        {{HI_PDT_WORKMODE_UPGRADE,
          PDT_STATEMNG_STATE_UPGRADE,
          PDT_STATEMNG_UpgradeStateEnter,
          PDT_STATEMNG_UpgradeStateExit,
          PDT_STATEMNG_UpgradeStateMsgProc,
          NULL}};
    stUpgrade.stState.argv = &stUpgrade;

    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stUpgrade.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"Add Upgrade");

    return s32Ret;
}

/** Deinit Upgrate state */
HI_S32 PDT_STATEMNG_UpgradeStateDeinit(HI_VOID)
{

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */