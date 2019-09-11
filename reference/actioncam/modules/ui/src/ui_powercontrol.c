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
#include "hi_hal_touchpad.h"
#include "hi_product_media.h"
#include "hi_hal_screen.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static HI_BOOL s_bPowerCtrlInited = HI_FALSE;
static HI_BOOL s_bScreenDormant = HI_FALSE;
static HI_BOOL s_bSystemDormant = HI_FALSE;


HI_S32 PDT_UI_POWERCTRL_DormantScreen(HI_VOID* pvPrivData)
{
    MLOGD("Dormant Screen\n");
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_VO_SWITCH;
    stMsg.arg2 = stWorkModeState.enWorkMode;
    HI_BOOL *pbValue = (HI_BOOL*)stMsg.aszPayload;
    *pbValue = HI_FALSE;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s_bScreenDormant = HI_TRUE;

    PDT_UI_COMM_DisableUSB();

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = HI_GV_Gesture_Enable(HI_FALSE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DisableTouchpad");

    s32Ret = HI_HAL_TOUCHPAD_Deinit();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DeinitTouchpad");
#endif

    return HI_SUCCESS;
}
HI_S32 PDT_UI_POWERCTRL_WakeupScreen(HI_VOID* pvPrivData)
{
    MLOGD("Wakeup Screen\n");
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_VO_SWITCH;
    stMsg.arg2 = stWorkModeState.enWorkMode;
    HI_BOOL *pbValue = (HI_BOOL*)stMsg.aszPayload;
    *pbValue = HI_TRUE;

    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    while(HI_SUCCESS != s32Ret)
    {
        HI_usleep(50000);
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    }

    s_bScreenDormant = HI_FALSE;

    PDT_UI_COMM_EnableUSB();

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = HI_HAL_TOUCHPAD_Init();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "InitTouchPad");

    s32Ret = HI_GV_Gesture_Enable(HI_TRUE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "EnableTouchPad");
#endif

    return HI_SUCCESS;
}
HI_S32 PDT_UI_POWERCTRL_DormantSystem(HI_VOID* pvPrivData)
{
    MLOGD("Dormant System\n");

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};
    stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMsg.arg2 = HI_PDT_WORKMODE_SUSPEND;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s_bSystemDormant = HI_TRUE;

    PDT_UI_COMM_DisableUSB();

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = HI_GV_Gesture_Enable(HI_FALSE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DisableTouchpad");

    s32Ret = HI_HAL_TOUCHPAD_Deinit();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DeinitTouchpad");
#endif

#if defined(CONFIG_WIFI_ON)
    PDT_UI_WIFI_STATUS_S stWiFiStatus;
    s32Ret = PDT_UI_COMM_GetWiFiStatus(&stWiFiStatus);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(HI_TRUE == stWiFiStatus.bInit && HI_TRUE == stWiFiStatus.bStart)
    {
        s32Ret = PDT_UI_COMM_StopWiFi();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        stWiFiStatus.bStart = HI_FALSE;
        stWiFiStatus.bResume = HI_TRUE;
        s32Ret = PDT_UI_COMM_SetWiFiStatus(&stWiFiStatus);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
#endif

    return HI_SUCCESS;
}
HI_S32 PDT_UI_POWERCTRL_WakeupSystem(HI_VOID* pvPrivData)
{
    MLOGD("Wakeup System\n");

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_NORM_REC;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMsg.arg2 = enWorkMode;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s_bSystemDormant = HI_FALSE;

    PDT_UI_COMM_EnableUSB();

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = HI_HAL_TOUCHPAD_Init();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "InitTouchPad");

    s32Ret = HI_GV_Gesture_Enable(HI_TRUE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "EnableTouchPad");
#endif

#if defined(CONFIG_WIFI_ON)
        PDT_UI_WIFI_STATUS_S stWiFiStatus;
        s32Ret = PDT_UI_COMM_GetWiFiStatus(&stWiFiStatus);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if(HI_TRUE == stWiFiStatus.bResume)
        {
            s32Ret = PDT_UI_COMM_StartWiFi();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            stWiFiStatus.bStart = HI_TRUE;
            stWiFiStatus.bResume = HI_FALSE;
            s32Ret = PDT_UI_COMM_SetWiFiStatus(&stWiFiStatus);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
#endif

    return HI_SUCCESS;
}

HI_BOOL PDT_UI_POWERCTRL_IsScreenDormant(HI_VOID)
{
    return s_bScreenDormant;
}

HI_BOOL PDT_UI_POWERCTRL_IsSystemDormant(HI_VOID)
{
    return s_bSystemDormant;
}

HI_S32 PDT_UI_POWERCTRL_Init(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_PWRCTRL_CFG_S stPowerCtrlCfg = {};
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pfnDormantProc = PDT_UI_POWERCTRL_DormantScreen;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pvDormantPrivData = NULL;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pfnWakeupProc = PDT_UI_POWERCTRL_WakeupScreen;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].pvWakeupPrivData = NULL;

    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].pfnDormantProc = PDT_UI_POWERCTRL_DormantSystem;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].pvDormantPrivData = NULL;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].pfnWakeupProc = PDT_UI_POWERCTRL_WakeupSystem;
    stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].pvWakeupPrivData = NULL;

    if(s_bPowerCtrlInited)
    {
        MLOGD("Power Control Inited\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SCREEN_DORMANT_MODE,
        &(stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SCREENDORMANT].stAttr));
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SYS_DORMANT_MODE,
        &(stPowerCtrlCfg.astTaskCfg[HI_PWRCTRL_TASK_SYSTEMDORMANT].stAttr));
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_POWERCTRL_Init(&stPowerCtrlCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = HI_TRUE;
    s_bScreenDormant = HI_FALSE;
    s_bSystemDormant = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_POWERCTRL_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;

    s32Ret = HI_POWERCTRL_Deinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = HI_FALSE;
    s_bScreenDormant = HI_TRUE;
    s_bSystemDormant = HI_TRUE;

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
        /** CommomEvent: use to reset timer */
        {HI_EVENT_UI_TOUCH,                 HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_PDT_MEDIA_HDMI_IN,        HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_PDT_MEDIA_HDMI_OUT,       HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},

        {HI_EVENT_USB_INSERT,               HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_USB_OUT,                  HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_USB_STORAGE_READY,        HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_USB_UVC_READY,            HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},

#if 0
        {HI_EVENT_STORAGEMNG_DEV_UNPLUGED,  HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_STORAGEMNG_DEV_CONNECTING,HI_PWRCTRL_EVENT_TYPE_COMMON, HI_PWRCTRL_EVENT_CONTROL_BUIT, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
#endif
        {HI_EVENT_KEYMNG_SHORT_CLICK,HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_LONG_CLICK, HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_HOLD_DOWN,  HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_HOLD_UP,    HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},
        {HI_EVENT_KEYMNG_GROUP,      HI_PWRCTRL_EVENT_TYPE_WAKEUP, HI_PWRCTRL_WAKEUP_TACTICS_DISCARD, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_TRUE},

        {HI_EVENT_STATEMNG_START, HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_PAUSE,   HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_FALSE},
        {HI_EVENT_STATEMNG_STOP,  HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_RESUME, HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_TRUE},

        {HI_EVENT_LIVESVR_CLIENT_CONNECT,   HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_PAUSE,   HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_FALSE},
        {HI_EVENT_LIVESVR_CLIENT_DISCONNECT,HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_RESUME, HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_TRUE},

        {HI_EVENT_NETCTRL_CLIENT_CONNECTED,   HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_PAUSE,   HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_FALSE},
        {HI_EVENT_NETCTRL_CLIENT_DISCONNECTED,HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_RESUME, HI_PWRCTRL_EVENT_SCOPE_SYSTEM, HI_TRUE},

        /*for work mode : UVC,USB-Storage, HDMI,HDMI-Preview, Playback*/
        {HI_EVENT_STATEMNG_SWITCH_WORKMODE,HI_PWRCTRL_EVENT_TYPE_CONTROL, HI_PWRCTRL_EVENT_CONTROL_PAUSE, HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, HI_FALSE},
    };

    *pbEventContinueHandle = HI_TRUE;

    for (i = 0; i < ARRAY_SIZE(as32EventMatrix); i++)
    {
        if (pstEvent->EventID != as32EventMatrix[i][0])
        {
            continue;
        }
        stEventAttr.enType = (HI_POWERCTRL_EVENT_TYPE_E)as32EventMatrix[i][1];

        if (HI_PWRCTRL_EVENT_TYPE_WAKEUP == stEventAttr.enType)
        {
            stEventAttr.unCfg.stWakeupCfg.enType                  = (HI_PWRCTRL_WAKEUP_TACTICS_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.enType      = (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.bResetTimer = as32EventMatrix[i][4];
        }
        else if (HI_PWRCTRL_EVENT_TYPE_CONTROL == stEventAttr.enType)
        {
            stEventAttr.unCfg.stCtrlCfg.enType                   = (HI_PWRCTRL_EVENT_CONTROL_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = as32EventMatrix[i][4];
        }
        else if (HI_PWRCTRL_EVENT_TYPE_COMMON == stEventAttr.enType)
        {
            stEventAttr.unCfg.stCommonCfg.enType       = (HI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCommonCfg.bResetTimer  = as32EventMatrix[i][4];
        }

        if (HI_EVENT_STATEMNG_SWITCH_WORKMODE == pstEvent->EventID)
        {
            switch (pstEvent->arg2)
            {
                case HI_PDT_WORKMODE_SUSPEND:
                    return HI_SUCCESS;

                case HI_PDT_WORKMODE_PLAYBACK:
                case HI_PDT_WORKMODE_HDMI_PREVIEW:
                case HI_PDT_WORKMODE_HDMI_PLAYBACK:
                case HI_PDT_WORKMODE_USB_STORAGE:
                case HI_PDT_WORKMODE_UVC:
                    if (!s_bLastModeIsCommon) /** special mode(playback/hdmi/usb/uvc) switch to special mode */
                    {
                        return HI_SUCCESS;
                    }
                    else /** last mode is photo/delay/burst/lapse/normal/loop/slow/recordlapse/recordsnap */
                    {
                        s_bLastModeIsCommon = HI_FALSE;
                        stEventAttr.unCfg.stCtrlCfg.enType                   = HI_PWRCTRL_EVENT_CONTROL_PAUSE;
                        stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                        stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = HI_FALSE;
                    }
                    break;

                default:
                    if (s_bLastModeIsCommon) /** common mode switch to common mode */
                    {
                        return HI_SUCCESS;
                    }
                    else /** special mode(playback/hdmi/usb/uvc) switch to common mode */
                    {
                        s_bLastModeIsCommon = HI_TRUE;
                        stEventAttr.unCfg.stCtrlCfg.enType                   = HI_PWRCTRL_EVENT_CONTROL_RESUME;
                        stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = HI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                        stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = HI_TRUE;
                    }
                    break;
            }
        }
        break;
    }

    if (HI_EVENT_NETCTRL_CLIENT_CONNECTED == pstEvent->EventID)
    {
        s_bLastModeIsCommon = HI_TRUE;
    }

    if (HI_EVENT_NETCTRL_CLIENT_DISCONNECTED == pstEvent->EventID)
    {
        if (!s_bLastModeIsCommon)
        {
           return HI_SUCCESS;
        }
    }

    if (i >= ARRAY_SIZE(as32EventMatrix))
    {
        MLOGD("ignore event %x\n", pstEvent->EventID);
        return HI_SUCCESS;
    }

    s32Ret = HI_POWERCTRL_EventPreProc(&stEventAttr, pbEventContinueHandle);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

