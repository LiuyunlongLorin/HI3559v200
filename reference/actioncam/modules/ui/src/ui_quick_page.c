/**
 * @file      ui_quick_page.c
 * @brief     ui quick buttons.
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
#include <math.h>

#include "ui_common.h"
#include "ui_common_playback.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#if defined(CONFIG_WIFI_ON)
static HI_BOOL s_bWiFiOn = HI_FALSE;
#endif

static HI_S32 PDT_UI_QUICK_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if(pstEvent->s32Result && HI_PDT_WORKMODE_PLAYBACK == pstEvent->arg2)
            {
                s32Ret = PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_PLAYBACK);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            }
            break;

        default:
            MLOGD("Event ID %u need to be processed here\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_QUICK_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
#if defined(CONFIG_WIFI_ON)
    PDT_UI_WIFI_STATUS_S stWiFiStatus;
#endif
    switch (WidgetHdl)
    {
        case QUICK_BUTTON_SETTINGS_BG:
        case QUICK_BUTTON_SETTINGS:
        case QUICK_LABEL_SETTINGS:
            s32Ret = PDT_UI_WINMNG_StartWindow(SET_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Ret = PDT_UI_WINMNG_FinishWindow(QUICK_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case QUICK_BUTTON_FILELIST_BG:
        case QUICK_BUTTON_FILELIST:
        case QUICK_LABEL_FILELIST:
            stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMessage.arg2 = HI_PDT_WORKMODE_PLAYBACK;

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if(stWorkModeState.bStateMngInProgress)
            {
                MLOGE("BUSY\n");
                return HI_SUCCESS;
            }

            s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_QUICK_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                PDT_UI_WINMNG_StartWindow(QUICK_PAGE, HI_TRUE);
                return HI_FAILURE;
            }
            break;

        case QUICK_BUTTON_WIFI_BG:
        case QUICK_LABEL_WIFI:
        case QUICK_BUTTON_WIFI:
#if defined(CONFIG_WIFI_ON)
            s32Ret = PDT_UI_COMM_GetWiFiStatus(&stWiFiStatus);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            if(HI_FALSE == stWiFiStatus.bInit)
            {
                /** init wifi service */
                s32Ret = HI_HAL_WIFI_Init(HI_HAL_WIFI_MODE_AP);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

                stWiFiStatus.bInit = HI_TRUE;
            }

            if(HI_FALSE == stWiFiStatus.bStart)
            {
                s32Ret = PDT_UI_COMM_StartWiFi();
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

                stWiFiStatus.bStart = HI_TRUE;
            }
            else
            {
                s32Ret = PDT_UI_COMM_StopWiFi();
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

                stWiFiStatus.bStart = HI_FALSE;
            }

            s32Ret = PDT_UI_COMM_SetWiFiStatus(&stWiFiStatus);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            s32Ret = HI_GV_Widget_SetSkin(QUICK_BUTTON_WIFI, HIGV_SKIN_NORMAL,
                stWiFiStatus.bStart ? SKIN_QUICK_BUTTON_WIFI_ON : SKIN_QUICK_BUTTON_WIFI_OFF);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            s32Ret = HI_GV_Widget_Paint(QUICK_BUTTON_WIFI, HI_NULL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
#endif
            break;

        case QUICK_BUTTON_POWER_OFF_BG:
        case QUICK_BUTTON_POWER_OFF:
        case QUICK_LABEL_POWER_OFF:
            stMessage.what = HI_EVENT_STATEMNG_POWEROFF;

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if(stWorkModeState.bStateMngInProgress)
            {
                MLOGE("BUSY\n");
                return HI_SUCCESS;
            }

            //s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            //HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

            s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_QUICK_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                return HI_FAILURE;
            }
            break;

        case QUICK_BUTTON_LOCK_SCREEN_BG:
        case QUICK_BUTTON_LOCK_SCREEN:
        case QUICK_LABEL_LOCK_SCREEN:
            s32Ret = PDT_UI_WINMNG_StartWindow(LOCK_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        case QUICK_BUTTON_BACK:
            s32Ret = PDT_UI_WINMNG_FinishWindow(QUICK_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_QUICK_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

#if defined(CONFIG_WIFI_ON)
    /** refesh WiFi icon */
    s32Ret = HI_HAL_WIFI_GetStartedStatus(&s_bWiFiOn);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_SetSkin(QUICK_BUTTON_WIFI, HIGV_SKIN_NORMAL,
        s_bWiFiOn ? SKIN_QUICK_BUTTON_WIFI_ON : SKIN_QUICK_BUTTON_WIFI_OFF);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Paint(QUICK_BUTTON_WIFI, HI_NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
#else
    s32Ret = HI_GV_Widget_Hide(QUICK_BUTTON_WIFI);
    s32Ret |= HI_GV_Widget_Hide(QUICK_LABEL_WIFI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

    s32Ret = HI_GV_Widget_Show(QUICK_BUTTON_SETTINGS_BG);
    s32Ret |= HI_GV_Widget_Active(QUICK_BUTTON_SETTINGS_BG);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_QUICK_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE aBtnHdl[] = {
        QUICK_BUTTON_SETTINGS_BG,
        QUICK_BUTTON_FILELIST_BG,
        QUICK_BUTTON_LOCK_SCREEN_BG,
        QUICK_BUTTON_POWER_OFF_BG,
#if defined(CONFIG_WIFI_ON)
        QUICK_BUTTON_WIFI_BG,
#endif
        QUICK_BUTTON_BACK
    };

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(QUICK_PAGE);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
       s32Ret = PDT_UI_COMM_ActiveNextWidget(aBtnHdl, ARRAY_SIZE(aBtnHdl), WidgetHdl);
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
       s32Ret = PDT_UI_QUICK_BUTTON_OnClick(WidgetHdl);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_QUICK_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("widget:%d, pointer[%d,%d]\n", WidgetHdl, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.x,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.y);

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_QUICK_BUTTON_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

