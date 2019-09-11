/**
 * @file      ui_powercontrol.c
 * @brief     ui powercontrol code. change power control strategy here.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "hi_gv_input.h"
#include "ui_common.h"
#include "hi_keymng.h"
#include "hi_powercontrol.h"
#include "hi_product_media.h"
#include "hi_hal_screen.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static HI_BOOL s_bPowerCtrlInited = HI_FALSE;


HI_S32  PDT_UI_POWERCTRL_DormantScreen(HI_VOID* pvPrivData)
{
    MLOGD("Sleep Screen\n");
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");

    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SCREEN_DORMANT;
    stMsg.arg2 = stWorkModeState.enWorkMode;
    HI_BOOL *pbValue = (HI_BOOL*)stMsg.aszPayload;
    *pbValue = HI_TRUE;  /*true means make screen sleep*/
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}
HI_S32  PDT_UI_POWERCTRL_WakeupScreen(HI_VOID* pvPrivData)
{
    MLOGD("Wakeup Screen\n");
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");

    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SCREEN_DORMANT;
    stMsg.arg2 = stWorkModeState.enWorkMode;
    HI_BOOL *pbValue = (HI_BOOL*)stMsg.aszPayload;
    *pbValue = HI_FALSE; /*false means wakeup screen*/
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_POWERCTRL_Init(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_PWRCTRL_CFG_S stPowerCtrlCfg = {};
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pfnDormantProc = PDT_UI_POWERCTRL_DormantScreen;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pvDormantPrivData = NULL;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pfnWakeupProc = PDT_UI_POWERCTRL_WakeupScreen;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pvWakeupPrivData = NULL;

    if(s_bPowerCtrlInited)
    {
        MLOGD("Power Control Inited\n");
        return HI_SUCCESS;
    }


    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,
        &(stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].stAttr));
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_POWERCTRL_Init(&stPowerCtrlCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 PDT_UI_POWERCTRL_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;

    s32Ret = HI_POWERCTRL_Deinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 PDT_UI_POWERCTRL_PreProcessEvent(HI_EVENT_S* pstEvent, HI_BOOL* pbEventContinueHandle)
{
    HI_S32 s32Ret = 0;
    HI_U32 i = 0;
    HI_PWRCTRL_EVENT_ATTR_S stEventAttr= {};
    static HI_BOOL s_bLastModeIsCommon = HI_TRUE;

    /*process touch event in another file*/

    HI_S32 as32EventMatrix[][5] =
    {
        /* Column :  0 Event,     1 Event Type,     2 Wake Type or Control Type,     3 Event Scope,     4 Reset Timer*/
        {HI_EVENT_UI_TOUCH,   HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},

        {HI_EVENT_KEYMNG_SHORT_CLICK,HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_LONG_CLICK, HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_HOLD_DOWN,  HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_HOLD_UP,    HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_GROUP,      HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_USB_INSERT, HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_CONTINUE,  HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_USB_OUT,    HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_CONTINUE, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        /*for work mode : UVC,USB-Storage, HDMI,HDMI-Preview, Playback*/
        {HI_EVENT_STATEMNG_SWITCH_WORKMODE,HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_PAUSE, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_FALSE},
    };

    *pbEventContinueHandle = HI_TRUE;

    for(i=0; i<UI_ARRAY_SIZE(as32EventMatrix); i++)
    {
        if(pstEvent->EventID != as32EventMatrix[i][0])
        {
            continue;
        }
        stEventAttr.enType = (HI_POWERCTRL_EVENT_TYPE_E)as32EventMatrix[i][1];

        if(HI_PWRCTRL_EVENT_TYPE_WAKEUP == stEventAttr.enType)
        {
            stEventAttr.unCfg.stWakeupCfg.enType                  = (HI_PWRCTRL_WAKEUP_TACTICS_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.enType      =   (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.bResetTimer = as32EventMatrix[i][4];
        }
        else if(HI_PWRCTRL_EVENT_TYPE_CONTROL == stEventAttr.enType)
        {
            stEventAttr.unCfg.stCtrlCfg.enType                   = (HI_PWRCTRL_EVENT_CONTROL_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       =  (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = as32EventMatrix[i][4];
        }
        else if(HI_PWRCTRL_EVENT_TYPE_COMMON == stEventAttr.enType)
        {
            stEventAttr.unCfg.stCommonCfg.enType       = (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCommonCfg.bResetTimer  = as32EventMatrix[i][4];
        }

        if(HI_EVENT_STATEMNG_SWITCH_WORKMODE == pstEvent->EventID)
        {
            switch(pstEvent->arg2)
            {
                case HI_PDT_WORKMODE_PLAYBACK:
                case HI_PDT_WORKMODE_USB_STORAGE:
                case HI_PDT_WORKMODE_UVC:
                    if( !s_bLastModeIsCommon )
                    {
                        return HI_SUCCESS;
                    }
                    s_bLastModeIsCommon = HI_FALSE;
                    stEventAttr.unCfg.stCtrlCfg.enType                   = HI_PWRCTRL_EVENT_CONTROL_PAUSE;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = HI_FALSE;
                    break;

                default:
                    if(s_bLastModeIsCommon)
                    {
                        return HI_SUCCESS;
                    }
                    s_bLastModeIsCommon = HI_TRUE;
                    stEventAttr.unCfg.stCtrlCfg.enType                   = HI_PWRCTRL_EVENT_CONTROL_RESUME;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = HI_TRUE;
                    break;
            }
        }
        break;
    }

    if(i >= UI_ARRAY_SIZE(as32EventMatrix))
    {
        MLOGD("ignore event %x\n", pstEvent->EventID);
        return HI_SUCCESS;
    }

    s32Ret = HI_POWERCTRL_EventPreProc(&stEventAttr, pbEventContinueHandle);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"EventPreProc");

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

