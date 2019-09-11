/**
 * @file      ui_himi_home.c
 * @brief     ui home in HDMI scene. live preview.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/7
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ui_common.h"
#include "hi_gsensormng.h"
#include "hi_storagemng.h"
#include "hi_product_netctrl.h"
#include "hi_system.h"
#include "hi_product_statemng.h"

#ifdef CONFIG_GAUGE_ON
#include "hi_gaugemng.h"
#include "hi_hal_gauge.h"

#endif

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#include "hi_tempcycle.h"
#endif



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define UI_HOME_CHECK_SD_TIME_SEC (1)       /** check SD state time ,the unit is second*/

typedef enum tagPDT_UI_KEY_ACTION_E
{
    PDT_UI_KEY_ACTION_SETTING = 0,
    PDT_UI_KEY_ACTION_FILEMNG,
    PDT_UI_KEY_ACTION_VO_SWITCH,
    PDT_UI_KEY_ACTION_BUTT
} PDT_UI_KEY_ACTION_E;

static PDT_UI_KEY_ACTION_E s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
static HI_U32 s_u32TimerCount = 0;
static HI_U32 s_u32EMRcurtime = 0;
static HI_BOOL s_bShowUI = HI_TRUE;

#ifdef CONFIG_GAUGE_ON
static HI_EVENT_GAUGEMNG_E s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_BUIT;
#endif

static HI_S32 PDT_UI_HOME_PrepareToFilelist(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");

    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("StateMng busy, Give up\n");
        return HI_SUCCESS;
    }

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = HI_PDT_WORKMODE_PLAYBACK;

    if (!PDT_UI_WINMNG_WindowIsShow(BUSY_PAGE))
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");
    }

    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);

    if (s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_RefreshRecordState(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_PDT_WORKMODE_PHOTO == pstWorkState->enWorkMode)
    {
        if (s_bShowUI)
        {
            s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_RECORD_STATE, STRING_HOME_PHOTO_MODE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_PHOTO);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_PHOTO);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_RECORD_FLAG);
        s32Ret |= HI_GV_Widget_Show(HOME_BUTTON_PHOTO_TAKE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_PHOTO_TAKE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Widget_Hide");

        if ( !pstWorkState->bRunning)
        {

            if (s_bShowUI)
            {
                s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_RECORD_STATE, STRING_PAUSED);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }

            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_RECORD_FLAG);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


            return HI_SUCCESS;
        }

        /* Recording */
        if ( pstWorkState->bEMRRecord)
        {
            s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_NORMAL, SKIN_HOME_RECORD_EMR);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_ACITVE, SKIN_HOME_RECORD_EMR);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


            if (s_bShowUI)
            {
                s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_RECORD_STATE, STRING_EMR);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
        }
        else
        {
            s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_NORMAL, SKIN_HOME_RECORD_NORMAL);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_RECORD_FLAG, HIGV_SKIN_ACITVE, SKIN_HOME_RECORD_NORMAL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            if (s_bShowUI)
            {
                s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_RECORD_STATE, STRING_RECORDING);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }

        }

        if (0 == s_u32TimerCount % 2)
        {
            s32Ret = HI_GV_Widget_Show(HOME_BUTTON_RECORD_FLAG);
        }
        else
        {
            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_RECORD_FLAG);
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    }
    return s32Ret;
}

static HI_S32 PDT_UI_HOME_RefreshPhoneState(HI_VOID)
{
    HI_S32 s32Ret = 0;

    if ( PDT_NETCTRL_IsClientConnecting())
    {
        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_PHONE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_PHONE);
    }

    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}
static HI_S32 PDT_UI_HOME_RefreshWifiState(HI_VOID)
{
    HI_S32 s32Ret = 0;
#if defined(CONFIG_WIFI_ON)
    HI_BOOL bState = HI_FALSE;
    s32Ret = HI_HAL_WIFI_GetStartedStatus((HI_BOOL*)&bState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if (bState)
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL, SKIN_HOME_WIFI_ON);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE, SKIN_HOME_WIFI_ON);
    }
    else
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL, SKIN_HOME_WIFI_OFF);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE, SKIN_HOME_WIFI_OFF);
    }

    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#else
    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_WIFI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

    return HI_SUCCESS;
}


#ifdef CONFIG_GAUGE_ON
static HI_S32 PDT_UI_HOME_SetBatteryInfo(HI_VOID)
{
    HI_S32 s32Ret;

    HI_BOOL bCharge = HI_FALSE;
    HI_S32 s32BatteryLevel;

    s32Ret = HI_GAUGEMNG_GetChargeState(&bCharge);
    if(HI_GAUGEMNG_ENOINIT == s32Ret)
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideWidget");
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    s32Ret = HI_GAUGEMNG_GetBatteryLevel(&s32BatteryLevel);
    if(HI_GAUGEMNG_ENOINIT == s32Ret)
    {
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    s32Ret = HI_GV_ProgressBar_SetPos(HOME_PROGRESSBAR_BATTERY, s32BatteryLevel);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    MLOGD("Set BatteryLevel[%d]\n", s32BatteryLevel);

    if(bCharge)
    {
        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_BATTERY_CHARGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_UI_HOME_RefreshAudioState(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bCurValue = HI_FALSE;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_AUDIO, &bCurValue);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetCommParam");

    if (bCurValue)
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_MIC, HIGV_SKIN_NORMAL, SKIN_HOME_MIC_ON);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_MIC, HIGV_SKIN_ACITVE, SKIN_HOME_MIC_ON);
    }
    else
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_MIC, HIGV_SKIN_NORMAL, SKIN_HOME_MIC_OFF);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_MIC, HIGV_SKIN_ACITVE, SKIN_HOME_MIC_OFF);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_RefreshTime(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szBuffer[32] = {};
    PDT_UI_DATETIME_S stDateTime = {};

    s32Ret = PDT_UI_COMM_GetDateTime(&stDateTime);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetDateTime");

    snprintf(szBuffer, UI_ARRAY_SIZE(szBuffer), "%u-%02u-%02u %02u:%02u:%02u",
             stDateTime.year, stDateTime.month,  stDateTime.day,
             stDateTime.hour, stDateTime.minute, stDateTime.second);

    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_TIME, szBuffer);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "SetText");

    return s32Ret;
}

HI_S32 PDT_UI_HOME_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    HIGV_HANDLE aHomeBtnHdl[] = {
        HOME_GROUP_TITLE,
    };

    if (!s_bShowUI)
    {
        HI_S32 i;
        for (i=0; i<UI_ARRAY_SIZE(aHomeBtnHdl); i++)
        {
            s32Ret = HI_GV_Widget_Hide(aHomeBtnHdl[i]);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

    }
    else
    {
        if(!HI_GV_Widget_IsShow(HOME_GROUP_TITLE))
        {
            s32Ret = HI_GV_Widget_Show(HOME_GROUP_TITLE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }


#ifdef CONFIG_GAUGE_ON
        s32Ret = PDT_UI_HOME_SetBatteryInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
#else
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideWidget");
#endif

        s32Ret = PDT_UI_HOME_RefreshTime();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_UI_HOME_RefreshPhoneState();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_UI_HOME_RefreshWifiState();
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "RefreshWifiState");

        s32Ret = PDT_UI_HOME_RefreshAudioState();
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "RefreshWifiState");

    }

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Paint");
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);

    if (s32Ret)
    {
        MLOGD("GetState failed, skip refresh home\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_HOME_RefreshRecordState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_HOME_ShowUI(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s_bShowUI = HI_TRUE;
    s32Ret = PDT_UI_HOME_Refresh();
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Refresh");

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_GV_Widget_Paint");
}


HI_S32 PDT_UI_HOME_RefreshOnTimer(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    if (s_u32TimerCount == UI_HOME_CHECK_SD_TIME_SEC)
    {
        PDT_UI_COMM_SdIsReady();
    }

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    if (s32Ret)
    {
        MLOGD("GetState failed, skip refresh home\n");
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_HOME_RefreshRecordState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    if (s_bShowUI)
    {
        s32Ret = PDT_UI_HOME_RefreshTime();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_UI_HOME_RefreshPhoneState();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Paint");

    return HI_SUCCESS;
}

/* EMR  or Take_PHOTO  */
static HI_S32 PDT_UI_HOME_Action(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMessage = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "HI_PDT_STATEMNG_GetState");

    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    if (!PDT_UI_COMM_SdIsReady())
    {
        MLOGE("SD not ready\n");
        return HI_FAILURE;
    }

    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_StartWindow");
        stMessage.what = HI_EVENT_STATEMNG_TRIGGER;
    }
    else
    {
        if(stWorkModeState.bEMRRecord)
        {
            MLOGE("EMRRecording, not support keys\n");
            return HIGV_PROC_GOON;
        }
        stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
    }

    stMessage.arg2 = stWorkModeState.enWorkMode;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_ToSeting(HI_VOID)
{
    HI_S32  s32Ret;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMessage = {};

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_begin();
#endif

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "HI_PDT_STATEMNG_GetState");

    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    if (stWorkModeState.bEMRRecord)
    {
        MLOGI("emr recording!\n");
        return HI_FAILURE;
    }

    s_enKeyAction = PDT_UI_KEY_ACTION_SETTING;

    if (!stWorkModeState.bRunning)
    {
        s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
        s32Ret = PDT_UI_WINMNG_StartWindow(SET_PAGE, HI_TRUE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FAILURE);
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "StartWindow");

    stMessage.what = HI_EVENT_STATEMNG_STOP;
    stMessage.arg1 = HI_FALSE;
    stMessage.arg2 = stWorkModeState.enWorkMode;
    s32Ret =  HI_PDT_STATEMNG_SendMessage(&stMessage);

    if (s32Ret)
    {
        s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
        MLOGE("s32Ret %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FAILURE);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_ToFilelist(HI_VOID)
{
    HI_S32  s32Ret;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMessage = {};

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_begin();
#endif

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "HI_PDT_STATEMNG_GetState");

    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HI_FAILURE;
    }

    if (!PDT_UI_COMM_SdIsReady())
    {
        MLOGE("SD not ready\n");
        return HI_FAILURE;
    }

    if (stWorkModeState.bEMRRecord)
    {
        MLOGI("emr recording!\n");
        return HI_FAILURE;
    }

    if (!stWorkModeState.bSDAvailable)
    {
        PDT_UI_COMM_ShowAlarm(STRING_SD_ERROR_FORMAT);
        MLOGW("sd have illegal files to format \n");
        return HI_FAILURE;
    }

    s_enKeyAction = PDT_UI_KEY_ACTION_FILEMNG;

    if (!stWorkModeState.bRunning)
    {
        s32Ret = PDT_UI_HOME_PrepareToFilelist();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "StartWindow");

    stMessage.what = HI_EVENT_STATEMNG_STOP;
    stMessage.arg1 = HI_TRUE;
    stMessage.arg2 = stWorkModeState.enWorkMode;
    s32Ret =  HI_PDT_STATEMNG_SendMessage(&stMessage);

    if (s32Ret)
    {
        s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
        MLOGE("s32Ret %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_HOME_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;

    s32Ret = HI_GV_ProgressBar_SetRange(HOME_EMR_PROGRESS, 0, HI_PDT_STATEMNG_REC_EMR_TIME_SEC * 1000); /* ms*/
    s32Ret |= HI_GV_Widget_Hide(HOME_EMR_PROGRESS);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HOME EMR Progressbar");

    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_RECORD_FLAG);
    s32Ret |= HI_GV_Widget_Hide(HOME_BUTTON_PHOTO_TAKE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Hide widget ");

    s32Ret = HI_GV_Timer_Create(HOME_PAGE, TIMER_REFRESH_HOME, 1000);
    s32Ret |= HI_GV_Timer_Start(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Create or Start Timer");

    s32Ret = HI_GV_Timer_Create(HOME_PAGE, TIMER_HOME_HIDE_UI, 5000);
    s32Ret |= HI_GV_Timer_Start(HOME_PAGE, TIMER_HOME_HIDE_UI);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Create or Start Timer");

    s32Ret = PDT_UI_HOME_Refresh();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HOME_Refresh");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Stop");

    s32Ret = HI_GV_Timer_Destroy(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Destroy");

    s32Ret = HI_GV_Timer_Stop(HOME_PAGE, TIMER_HOME_HIDE_UI);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Stop");

    s32Ret = HI_GV_Timer_Destroy(HOME_PAGE, TIMER_HOME_HIDE_UI);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Destroy");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (TIMER_REFRESH_HOME == wParam)
    {
        s_u32TimerCount++;
        s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Reset");

        s32Ret = PDT_UI_HOME_RefreshOnTimer();
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "RefreshOnTimer");

    }
    else if (TIMER_REFRESH_HOME_EMR == wParam)
    {
        s_u32EMRcurtime++;
        s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s32Ret = HI_GV_ProgressBar_SetPos(HOME_EMR_PROGRESS, s_u32EMRcurtime * UI_HOME_EMR_TIME_INTERVAL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s32Ret = HI_GV_Widget_Paint(HOME_EMR_PROGRESS, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (TIMER_HOME_HIDE_UI == wParam)
    {
        s_bShowUI = HI_FALSE;

        s32Ret = PDT_UI_HOME_Refresh();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_EventProc((HI_EVENT_S*)lParam);

    if (s32Ret)
    {
        MLOGE("s32Ret %x\n", s32Ret);
    }
    return HIGV_PROC_GOON;
}

/** Process events from EVENTHUB*/
HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = 0;
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2, pstEvent->s32Result);
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_START:
#ifdef  CFG_TIME_STAMP_ON
            PDT_UI_COMM_SwitchPageTime_end();
#endif
            break;

        case HI_EVENT_STATEMNG_STOP:
            if (HI_SUCCESS != pstEvent->s32Result)
            {
                s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                break;
            }

            if (PDT_UI_KEY_ACTION_FILEMNG == s_enKeyAction)
            {
                PDT_UI_HOME_PrepareToFilelist();
                s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
            }
            else if (PDT_UI_KEY_ACTION_SETTING == s_enKeyAction)
            {
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                if( PDT_NETCTRL_IsClientConnecting())
                {
                    MLOGI("Client Connected, ignore start set_page\n");
                    if (!PDT_UI_COMM_SdIsReady())
                    {
                        MLOGE(" SD state is not normal \n");
                        return HI_FAILURE;
                    }
                    stMessage.what = HI_EVENT_STATEMNG_START;
                    stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
                else
                {
                    s32Ret = PDT_UI_WINMNG_StartWindow(SET_PAGE, HI_TRUE);
                    s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }
            return HI_SUCCESS;

        case HI_EVENT_GSENSORMNG_COLLISION:
            {
                if((HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode) ||
                    (HI_PDT_WORKMODE_PARKING_REC == stWorkModeState.enWorkMode))
                {
                    if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
                    {
                        MLOGE("statemng busy !\n");
                        return HI_FAILURE;
                    }

                    if (!PDT_UI_COMM_SdIsReady())
                    {
                        MLOGE(" SD state is not normal \n");
                        return HI_FAILURE;
                    }

                    stMessage.what = HI_EVENT_STATEMNG_EMR_BEGIN;
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }
            return HI_SUCCESS;

        case HI_EVENT_STATEMNG_EMR_BEGIN:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                /* change USB mode to avoid EMR and insert USB conflict when EMR begin  */
                if (HI_PDT_WORKMODE_PARKING_REC != stWorkModeState.enWorkMode)
                {
                    s32Ret = HI_USB_SetMode(HI_USB_MODE_CHARGE);
                    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"set usb mode failed");
                }
                s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                s_u32EMRcurtime = 0;
                s32Ret = HI_GV_Timer_Create(HOME_PAGE, TIMER_REFRESH_HOME_EMR, UI_HOME_EMR_TIME_INTERVAL);
                s32Ret |= HI_GV_Timer_Start(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                s32Ret = HI_GV_ProgressBar_SetPos(HOME_EMR_PROGRESS, 0);
                s32Ret |= HI_GV_Widget_Show(HOME_EMR_PROGRESS);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        case HI_EVENT_STATEMNG_EMR_END:
            if (pstEvent->s32Result == HI_SUCCESS)
            {
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                HI_S32  s32CollisionCnt = 0;
                /* change USB mode from param when EMR end  */
                if (HI_PDT_WORKMODE_PARKING_REC != stWorkModeState.enWorkMode)
                {
                    if(HI_FALSE == PDT_NETCTRL_IsClientConnecting())
                    {
                        HI_USB_MODE_E enUsbMode;

                        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &enUsbMode);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetCommParam failed");

                        s32Ret = HI_USB_SetMode(enUsbMode);
                        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"set usb mode failed");
                    }
                }

                if (stWorkModeState.bEMRRecord == HI_FALSE)
                {
                    s32Ret |= HI_GV_Timer_Stop(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
                    s32Ret |= HI_GV_Timer_Destroy(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                    s32Ret = HI_GV_Widget_Hide(HOME_EMR_PROGRESS);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);

                    if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_WAKEYP)
                    {
                        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
                        s32CollisionCnt++;
                        s32Ret |= HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                        s32Ret = PDT_UI_COMM_POWEROFF();
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    }
                }

                return HI_SUCCESS;
            }

            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                HI_S32  s32CollisionCnt;
                HI_EVENT_S stEvent = {};
                if (HI_PDT_WORKMODE_PLAYBACK == pstEvent->arg2)
                {
                    s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE, HI_TRUE);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    if (s32CollisionCnt > 0)
                    {
                        PDT_UI_WINMNG_StartWindow(PLAYPARK_WINDOW, HI_FALSE);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    }
                }
                else if (HI_PDT_WORKMODE_NORM_REC == pstEvent->arg2)
                {
                    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_AVAILABLE, &stEvent);
                    if (HI_SUCCESS == s32Ret) /* not  PowerAction   */
                    {
                        if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress) || (stWorkModeState.bRunning))
                        {
                            MLOGE("statemng busy !\n");
                            return HI_FAILURE;
                        }
                        s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_RECMNG_MUXER_CNT_CHANGE, &stEvent);
                        if((HI_SUCCESS != s32Ret) ||(-1 == stEvent.arg1))
                        {
                            if(!PDT_UI_COMM_SdIsReady())
                            {
                                MLOGE(" SD state is not normal \n");
                                return HI_FAILURE;
                            }

                            if(!(PDT_NETCTRL_IsClientConnecting()))
                            {
                                stMessage.what = HI_EVENT_STATEMNG_START;
                                stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                                s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                                if (s32Ret)
                                {
                                    MLOGE("SendMessage failed. %x\n", s32Ret);
                                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                                }
                            }

                        }
                        else
                        {
                            MLOGI("task[%d] is not zero ,can not start rec!\n",stEvent.arg1);
                        }
                    }
                    else
                    {
                        MLOGI("HI_EVTHUB_GetEventHistory fail %x\n", s32Ret);
                    }
                }
                return HI_SUCCESS;
            }
            break;

#ifdef CONFIG_GAUGE_ON
        case HI_EVENT_GAUGEMNG_LEVEL_CHANGE:
        case HI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE:
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_LOW:
            if(HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW != s_enLastBatteryEvent
                && HI_EVENT_GAUGEMNG_LEVEL_LOW != s_enLastBatteryEvent)
            {
                PDT_UI_COMM_ShowAlarm(STRING_LOW_BATTERY);
            }
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_LOW;
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW:
            if(HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW != s_enLastBatteryEvent)
            {
                PDT_UI_COMM_ShowAlarm(STRING_ULTRALOW_BATTERY);
            }
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW;
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_NORMAL:
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_NORMAL;
            break;
#endif

        case HI_EVENT_STATEMNG_SD_AVAILABLE:
            {
                HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;
                HI_S32  s32CollisionCnt;

                if(pstEvent->arg1 == 1)
                {
                    MLOGI("the event from format sd\n");
                    return HI_SUCCESS;
                }

                HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);
                if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_STARTUP)
                {
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                    if (s32CollisionCnt > 0)
                    {
                        MLOGI(RED"parking REC playback \n"NONE);
                        s32Ret = PDT_UI_HOME_ToFilelist();
                        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                        return HI_SUCCESS;
                    }
                    else
                    {
                        if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
                        {
                            MLOGI(RED"PHOTO MODE\n"NONE);
                        }
                        else
                        {
                            MLOGI(RED"Normal REC \n"NONE);
                            if (!PDT_UI_COMM_SdIsReady())
                            {
                                MLOGE(" SD state is not normal \n");
                                return HI_FAILURE;
                            }
                            stMessage.what = HI_EVENT_STATEMNG_START;
                            stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                            if (s32Ret)
                            {
                                MLOGE("SendMessage failed. %x\n", s32Ret);
                                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                            }
                        }
                    }
                }
                else if (enStartupSrc == HI_SYSTEM_STARTUP_SRC_WAKEYP)
                {
                    MLOGI(RED"parking REC \n"NONE);
                }
            }
            break;
            case HI_EVENT_RECMNG_MUXER_CNT_CHANGE:
                MLOGI("HI_EVENT_RECMNG_MUXER_CNT_CHANGE taskhal[%d] cnt[%d]!\n",pstEvent->arg1,pstEvent->arg2);
                if((-1 == pstEvent->arg1) && (PDT_UI_KEY_ACTION_BUTT == s_enKeyAction))
                {
                    HI_USB_MODE_E enUsbMode;
                    s32Ret = HI_USB_GetMode(&enUsbMode);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

                    HI_USB_STATE_E enUsbState;
                    s32Ret = HI_USB_GetState(&enUsbState);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);


                    if( PDT_NETCTRL_IsClientConnecting())
                    {
                        MLOGI("Client connecting!\n");
                        return HI_SUCCESS;
                    }

                    if((HI_USB_MODE_CHARGE != enUsbMode) && (HI_USB_STATE_OUT != enUsbState))
                    {
                        MLOGI("current usb_mode[%d] usb_state[%d]; can not start record!\n", enUsbMode, enUsbState);
                        return HI_SUCCESS;
                    }
                    HI_EVENT_S stEvent;
                    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_AVAILABLE, &stEvent);
                    if (HI_SUCCESS == s32Ret) /* not  PowerAction   */
                    {
                        if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress) || (stWorkModeState.bRunning))
                        {
                            MLOGW("recording or statemng busy, not need to start rec!\n");
                            return HI_SUCCESS;
                        }
                        MLOGI("HI_EVENT_RECMNG_MUXER_CNT_CHANGE task is zero ,start rec!\n");

                        if (!PDT_UI_COMM_SdIsReady())
                        {
                            MLOGE(" SD state is not normal \n");
                            return HI_FAILURE;
                        }

                        stMessage.what = HI_EVENT_STATEMNG_START;
                        stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                        if (s32Ret)
                        {
                            MLOGE("SendMessage failed. %x\n", s32Ret);
                            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                        }
                    }
                }
                break;
        case HI_EVENT_STATEMNG_TRIGGER:
            PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            break;

        default:
            break;
    }

    s32Ret = PDT_UI_HOME_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}


HI_S32 PDT_UI_HOME_OnKeyUp(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_SET_VoSwitch();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HIGV_PROC_STOP;
    }

    if (!s_bShowUI)
    {
        PDT_UI_HOME_ShowUI();

        s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_HOME_HIDE_UI);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        return HIGV_PROC_STOP;
    }

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_HOME_Action();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if ( PDT_NETCTRL_IsClientConnecting())
        {
            MLOGI("Client connecting!\n");
            return HIGV_PROC_GOON;
        }

        s32Ret = PDT_UI_HOME_ToFilelist();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (PDT_UI_KEY_OK == wParam)
    {

        if ( PDT_NETCTRL_IsClientConnecting())
        {
            MLOGI("Client connecting!\n");
            return HIGV_PROC_GOON;
        }

        s32Ret = PDT_UI_HOME_ToSeting();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 s32StartX, s32StartY, s32EndX, s32EndY;
    HI_S32  s32Ret;
    HI_PDT_STATEMNG_WORKMODE_STATE_S    stWorkModeState;
    s32StartX = pstGestureEvent->gesture.fling.start.x;
    s32StartY = pstGestureEvent->gesture.fling.start.y;
    s32EndX   = pstGestureEvent->gesture.fling.end.x;
    s32EndY   = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", s32StartX, s32StartY, s32EndX, s32EndY);

    if ( PDT_NETCTRL_IsClientConnecting())
    {
        MLOGI("Client connecting!\n");
        return HIGV_PROC_GOON;
    }

    if (abs(s32StartX - s32EndX) > abs(s32StartY - s32EndY))
    {
        if (s32StartX > s32EndX)
        {
            MLOGD("fling left\n");

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_STATEMNG_GetState");

            if (stWorkModeState.bStateMngInProgress)
            {
                MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
                return HIGV_PROC_GOON;
            }

            if (stWorkModeState.bEMRRecord)
            {
                MLOGI("emr recording!\n");
                return HIGV_PROC_GOON;
            }

            s32Ret = PDT_UI_SET_VoSwitch();
            if (s32Ret)
            {
                MLOGE("PDT_UI_COMM_VoSwitch %x\n", s32Ret);
                return HIGV_PROC_GOON;
            }
        }
        else
        {
            MLOGD("fling right\n");
            s32Ret = PDT_UI_HOME_ToFilelist();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }
    else
    {
        if (s32StartY > s32EndY)
        {
            MLOGD("fling up\n");
            s32Ret = PDT_UI_WINMNG_StartWindow(PERIPHERAL_WINDOW, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            MLOGD("fling down\n");
            s32Ret = PDT_UI_HOME_ToSeting();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HIGV_HANDLE hWidget;

    s32Ret = HI_GV_Widget_GetWidgetByPos_TouchDevice(pstTouchEvent->last.x, pstTouchEvent->last.y, &hWidget);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_STOP, "HI_GV_Widget_GetWidgetByPos_TouchDevice");

    MLOGD("touch x[%d] y[%d] hWidget[%d]\n", pstTouchEvent->last.x, pstTouchEvent->last.y, hWidget);

    if (hWidget == HOME_BUTTON_PHOTO_TAKE)
    {
        s32Ret = PDT_UI_HOME_Action();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (!s_bShowUI)
    {
        PDT_UI_HOME_ShowUI();

        s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_HOME_HIDE_UI);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"HI_GV_Timer_Reset");
    }

    return HIGV_PROC_STOP;
}

HI_S32 PDT_UI_HOME_OnGestureLongTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
#if 0
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureScroll(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
#if 0
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
#endif

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
#if 0
    HIGV_TOUCH_EVENT_S* pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
#endif
    return HIGV_PROC_GOON;
}

HI_S32  PDT_UI_HOME_SET_VoCrop(PDT_UI_VO_VIEWCROP_E e_viewcrop)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32  s32Value = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("get statemng state failed \n");
        return HI_FAILURE;
    }

    stMessage.what = HI_EVENT_STATEMNG_SETTING;
    stMessage.arg1 = HI_PDT_PARAM_TYPE_CROP;
    stMessage.arg2 = stWorkModeState.enWorkMode;

    if (e_viewcrop == UI_VO_VIEW_UP)
    {
        s32Value = 1;
    }
    else if (e_viewcrop == UI_VO_VIEW_DOWN)
    {
        s32Value = -1;
    }
    memcpy(&stMessage.aszPayload[0], &s32Value,sizeof(HI_S32));
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("send msg to statemng failed \n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

