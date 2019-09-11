/**
* @file    ui_home_page.c
* @brief   ui home page,live preview. do snap, record work.
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
#include "hi_hal_wifi.h"
#include "hi_filemng_dtcf.h"
#include "hi_storagemng.h"
#include "hi_gaugemng.h"
#include "hi_gsensormng.h"
#include "hi_product_netctrl.h"
#include "hi_system.h"
#include "hi_product_statemng.h"
#include "hi_playback.h"
#include "hi_voiceplay.h"
#include "hi_recordmng.h"

#ifdef CONFIG_GAUGE_ON
#include "hi_gaugemng.h"
#endif




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

typedef enum tagPDT_UI_KEY_ACTION_E
{
    PDT_UI_KEY_ACTION_SETTING = 0,
    PDT_UI_KEY_ACTION_ALBUM,
    PDT_UI_KEY_ACTION_BUTT
} PDT_UI_KEY_ACTION_E;

static HI_U32 s_u32TimerCount = 0;
static PDT_UI_KEY_ACTION_E s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
static HI_U32 s_u32EMRcurtime = 0;

#ifdef CONFIG_GAUGE_ON
static HI_EVENT_GAUGEMNG_E s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_BUIT;
#endif

extern HI_S32 PDT_UI_PLAYBACK_PlayerEventProc(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pvData, HI_VOID* pvUserData);

 HI_VOID PDT_UI_PLAYBACK_VideoPlayerInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_WORKMODE_CFG_S stCfg;
    HI_PLAYBACK_CFG_S stPlayBackCfg;

    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_PLAYBACK, &stCfg);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_PDT_PARAM_GetWorkModeCfg failed: %#x\n", s32Ret);
    }

    stPlayBackCfg.pfnEventProc = PDT_UI_PLAYBACK_PlayerEventProc;
    stPlayBackCfg.pvUserData = NULL;
    memcpy(&stPlayBackCfg.stPlayerParam, &stCfg.unModeCfg.stPlayBackCfg.stPlayerParam, sizeof(HI_LITEPLAYER_PARAM_S));
    memcpy(&stPlayBackCfg.stVoutOpt, &stCfg.unModeCfg.stPlayBackCfg.stVoutOpt, sizeof(HI_LITEPLAYER_VOUT_OPT_S));
    memcpy(&stPlayBackCfg.stAoutOpt, &stCfg.unModeCfg.stPlayBackCfg.stAoutOpt, sizeof(HI_LITEPLAYER_AOUT_OPT_S));

    s32Ret = HI_PLAYBACK_Init(&stPlayBackCfg);
    if(s32Ret && HI_EINITIALIZED != s32Ret)
    {
        MLOGE("HI_PLAYBACK_Init: %#x\n", s32Ret);
    }
}

/*State Machine Msg Result Callback*/
static HI_S32 PDT_UI_HOME_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    if(HI_EVENT_STATEMNG_TRIGGER == pstEvent->EventID)
    {
        PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_PrepareToFilelist(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");

    if(stWorkModeState.bStateMngInProgress)
    {
        MLOGE("BUSY\n");
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        return HI_SUCCESS;
    }

    if (PDT_UI_WINMNG_WindowIsShow(BUSY_PAGE))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(ALBUM_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = HI_PDT_WORKMODE_PLAYBACK;

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



static HI_S32 PDT_UI_HOME_SetAudioButtonSkin(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bAudio = HI_FALSE;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_AUDIO, &bAudio);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PDT_PARAM_GetCommParam");

    if (bAudio)
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_AUDIO, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_AUDIO_ON);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_AUDIO, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_AUDIO_ON);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_SetSkin");
    }
    else
    {
        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_AUDIO, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_AUDIO_OFF);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_AUDIO, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_AUDIO_OFF);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_SetSkin");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetWifiButtonSkin(HI_VOID)
{
        HI_S32 s32Ret = HI_SUCCESS;
#if defined(CONFIG_WIFI_ON)
        HI_BOOL bWifiOn = HI_FALSE;

        s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWifiOn);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_HAL_WIFI_GetStartedStatus");

    s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL,
                                  bWifiOn ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);
    s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE,
                                  bWifiOn ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);
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

static HI_S32 PDT_UI_HOME_SetRecFlagButtonSkin(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(HI_PDT_WORKMODE_PHOTO == pstWorkState->enWorkMode)
    {
        s32Ret =  HI_GV_Widget_SetTextByID(HOME_LABEL_VIDEO,STRING_HOME_PHOTO_MODE);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_PHOTO);
        s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_PHOTO);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_VIDEO);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    }
    else
    {
        if( !pstWorkState->bRunning)
        {
            s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_VIDEO, STRING_HOME_VIDEO_OFF);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            return HI_SUCCESS;
        }

        if (pstWorkState->bEMRRecord)
        {
            s32Ret =  HI_GV_Widget_SetTextByID(HOME_LABEL_VIDEO,STRING_HOME_EMR);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_VIDEO_EMR);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_VIDEO_EMR);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            s32Ret = HI_GV_Widget_SetTextByID(HOME_LABEL_VIDEO, STRING_HOME_VIDEO_ON);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_VIDEO);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_VIDEO, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_VIDEO);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }

        if (s_u32TimerCount % 2)
        {
            s32Ret = HI_GV_Widget_Show(HOME_BUTTON_VIDEO);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }


    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetLabelSysTimeInfo(HI_VOID)
{
    HI_S32  s32Ret = 0;
    HI_CHAR szBuffer[32] = {};
    PDT_UI_DATETIME_S stDateTime = {};

    s32Ret = PDT_UI_COMM_GetDateTime(&stDateTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    snprintf(szBuffer,sizeof(szBuffer),"%02u:%02u:%02u",stDateTime.hour,stDateTime.minute,stDateTime.second);
    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_SYS_TIME, szBuffer);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"SetText");

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_DoKeyAction(PDT_UI_KEY_ACTION_E enKeyAction)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (enKeyAction)
    {
        case PDT_UI_KEY_ACTION_SETTING:

            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            if(PDT_NETCTRL_IsClientConnecting() && PDT_UI_COMM_isMdReady())
            {
                MLOGI("Client Connected, ignore start set_page\n");

                HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};
                s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
                if(s32Ret)
                {
                    MLOGD("GetState failed, skip refresh home\n");
                    return HI_SUCCESS;
                }
                if(HI_PDT_WORKMODE_NORM_REC == stWorkState.enWorkMode)
                {
                    s32Ret = PDT_UI_COMM_CheckSd();
                    if (s32Ret != HI_SUCCESS)
                    {
                        MLOGE(" SD state is not normal \n");
                        return HI_FAILURE;
                    }

                    HI_MESSAGE_S stMessage = {};
                    stMessage.what = HI_EVENT_STATEMNG_START;
                    stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
                    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }

            }
            else
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(SET_PAGE, HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        case PDT_UI_KEY_ACTION_ALBUM:
            s32Ret = PDT_UI_HOME_PrepareToFilelist();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        default:
            MLOGI("stop REC ,but not KeyAction \n");
            break;
    }

    return HI_SUCCESS;
}


static HI_S32 PDT_UI_HOME_ToFilelist(HI_VOID)
{
    HI_S32  s32Ret;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    if (HI_FAILURE == PDT_UI_COMM_CheckSd())
    {
        MLOGE("SD is not normal\n");
        return HI_FAILURE;
    }

    if ( PDT_NETCTRL_IsClientConnecting())
    {
        MLOGI("Client connecting!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_PDT_STATEMNG_GetState");

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

    if (!stWorkModeState.bSDAvailable)
    {
        PDT_UI_COMM_ShowAlarm(STRING_SD_ERROR_FORMAT);
        MLOGE("sd have illegal files to format \n");
        return HI_FAILURE;
    }
    if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
    {
        s32Ret = PDT_UI_HOME_PrepareToFilelist();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        if (stWorkModeState.bRunning)
        {
            s32Ret = PDT_UI_COMM_SendStopMsg(PDT_UI_HOME_OnReceiveMsgResult);
            if (s32Ret)
            {
                MLOGE("s32Ret %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }

            s_enKeyAction = PDT_UI_KEY_ACTION_ALBUM;

            return HI_SUCCESS;
        }
        else
        {
            s32Ret = PDT_UI_HOME_PrepareToFilelist();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }



    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_RefreshPhoneState(HI_VOID)
{
    HI_S32 s32Ret = 0;

    if( PDT_NETCTRL_IsClientConnecting())
    {
        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_PHONE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_PHONE);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

/**Refresh by timer*/
HI_S32 PDT_UI_HOME_RefreshOnTimer(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    if (s_u32TimerCount == 1)
    {
        s32Ret = PDT_UI_COMM_CheckSd();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetWorkState");

    s32Ret = PDT_UI_HOME_SetRecFlagButtonSkin(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetLabelSysTimeInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_RefreshPhoneState();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}


/**Refresh all widgets*/
HI_S32 PDT_UI_HOME_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

#ifdef CONFIG_GAUGE_ON
    s32Ret = PDT_UI_HOME_SetBatteryInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
#else
    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideWidget");
#endif

    s32Ret = PDT_UI_HOME_SetLabelSysTimeInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_HOME_SetAudioButtonSkin();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_RefreshPhoneState();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetWifiButtonSkin();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    if(s32Ret)
    {
        MLOGD("GetState failed, skip refresh home\n");
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_HOME_SetRecFlagButtonSkin(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_PDT_WORKMODE_PHOTO == stWorkState.enWorkMode)
    {
        s32Ret =  HI_GV_Widget_SetTextByID(HOME_LABEL_EMR,STRING_TAKE_PHOTO);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else
    {
        s32Ret =  HI_GV_Widget_SetTextByID(HOME_LABEL_EMR,STRING_HOME_EMR);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    return HI_SUCCESS;
}


/** Process events from EVENTHUB*/
HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S * pstEvent)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);
    HI_S32 s32Ret = 0;
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1,
          pstEvent->arg2,pstEvent->s32Result);
    HI_MESSAGE_S stMessage = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState;
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_START:
#ifdef  CFG_TIME_STAMP_ON
            PDT_UI_COMM_SwitchPageTime_end();
#endif
            break;

        case HI_EVENT_STATEMNG_STOP:
            s32Ret = PDT_UI_HOME_DoKeyAction(s_enKeyAction);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            if ((PDT_UI_KEY_ACTION_SETTING == s_enKeyAction) || (PDT_UI_KEY_ACTION_ALBUM == s_enKeyAction))
            {
                return HI_SUCCESS;
            }
            break;

        case HI_EVENT_GSENSORMNG_COLLISION:
            {
                if(HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode)
                {
                    if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
                    {
                        MLOGE("statemng busy !\n");
                        return HI_FAILURE;
                    }

                    s32Ret = PDT_UI_COMM_CheckSd();
                    if(s32Ret != HI_SUCCESS)
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
            if (HI_SUCCESS == pstEvent->s32Result)
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
                    s32Ret = HI_GV_Timer_Stop(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
                    s32Ret |= HI_GV_Timer_Destroy(HOME_PAGE, TIMER_REFRESH_HOME_EMR);
                    s32Ret |= HI_GV_Widget_Hide(HOME_EMR_PROGRESS);
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

                    HI_S32 s32CamID = 0;
                    HI_REC_TYPE_E enRecType = HI_REC_TYPE_BUTT;
                    s32CamID = HI_PDT_STATEMNG_GetPreViewCamID();
                    s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32CamID,
                        HI_PDT_PARAM_RECORD_TYPE, &enRecType);

                    if((!PDT_UI_COMM_isMdReady()) && (stWorkModeState.bRunning) && ((!stWorkModeState.bStateMngInProgress)) &&
                        (HI_REC_TYPE_LAPSE != enRecType))
                    {
                        /** stop rec */
                        stMessage.what = HI_EVENT_STATEMNG_STOP;
                        stMessage.arg1 = HI_FALSE; /**HI_TRUE:record stop sync,HI_FALSE:record stop async*/
                        stMessage.arg2 = stWorkModeState.enWorkMode;
                        s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                            return HI_FAILURE;
                        }
                   }
                }
            }
            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if (HI_SUCCESS == pstEvent->s32Result)
            {
                HI_EVENT_S stEvent = {};
                if ((HI_PDT_WORKMODE_NORM_REC == pstEvent->arg2) || (HI_PDT_WORKMODE_PHOTO == pstEvent->arg2))
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
                        if(((HI_SUCCESS != s32Ret) ||(-1 == stEvent.arg1)) && PDT_UI_COMM_isMdReady())
                        {
                            s32Ret = PDT_UI_COMM_CheckSd();
                            if(s32Ret != HI_SUCCESS)
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
                    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT,&s32CollisionCnt);
                    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                    if (s32CollisionCnt > 0)
                    {
                        MLOGI(RED"parking REC \n"NONE);
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
                            if(PDT_UI_COMM_isMdReady())
                            {
                                MLOGI(RED"Normal REC \n"NONE);

                                s32Ret = PDT_UI_COMM_CheckSd();
                                if (s32Ret != HI_SUCCESS)
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
                                    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                                }
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
                    if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress) ||
                        (stWorkModeState.bRunning) || (!PDT_UI_COMM_isMdReady()))
                    {
                        MLOGW("recording or statemng busy, not need to start rec!\n");
                        return HI_SUCCESS;
                    }
                    MLOGI("HI_EVENT_RECMNG_MUXER_CNT_CHANGE task is zero ,start rec!\n");

                    s32Ret = PDT_UI_COMM_CheckSd();
                    if (s32Ret != HI_SUCCESS)
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

#ifdef CONFIG_MOTIONDETECT_ON
        case HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE:
        {
            MLOGI(YELLOW"MD Status change to [%d]\n\n"NONE, pstEvent->arg1);
            if((PDT_UI_KEY_ACTION_BUTT == s_enKeyAction) && (HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode))
            {
                if((1 == pstEvent->arg1) && (!stWorkModeState.bRunning) && (!stWorkModeState.bStateMngInProgress) &&
                    PDT_UI_COMM_isMdReady())
                {
                    /** start rec */
                    s32Ret = PDT_UI_COMM_CheckSd();
                    if (s32Ret != HI_SUCCESS)
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
                else if((0 == pstEvent->arg1) && (stWorkModeState.bRunning) && (!stWorkModeState.bEMRRecord) &&
                    (!stWorkModeState.bStateMngInProgress) && (HI_PDT_WORKMODE_NORM_REC == stWorkModeState.enWorkMode))
                {
                    /** stop rec */
                    stMessage.what = HI_EVENT_STATEMNG_STOP;
                    stMessage.arg1 = HI_FALSE; /**HI_TRUE:record stop sync,HI_FALSE:record stop async*/
                    stMessage.arg2 = stWorkModeState.enWorkMode;
                    s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
                        return HI_FAILURE;
                    }
                }
            }
        }
            break;
#endif
        default:
            break;
    }

    s32Ret = PDT_UI_HOME_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_HOME_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s_enKeyAction = PDT_UI_KEY_ACTION_BUTT;
    s32Ret = HI_GV_ProgressBar_SetRange(HOME_EMR_PROGRESS,0,HI_PDT_STATEMNG_REC_EMR_TIME_SEC*1000); /* ms */
    s32Ret = HI_GV_Widget_Hide(HOME_EMR_PROGRESS);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret," HOME_EMR_PROGRESS ");

    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideWidget");

    s32Ret = HI_GV_Timer_Create(HOME_PAGE, TIMER_REFRESH_HOME, 1000);
    s32Ret |= HI_GV_Timer_Start(HOME_PAGE, TIMER_REFRESH_HOME);
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
    s32Ret |= HI_GV_Timer_Destroy(HOME_PAGE, TIMER_REFRESH_HOME);
    if(s32Ret)
    {
        MLOGE("s32Ret %x\n", s32Ret);
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (TIMER_REFRESH_HOME == wParam)
    {
        s_u32TimerCount++;
        s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME);

        if (s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
        }

        s32Ret = PDT_UI_HOME_RefreshOnTimer();

        if (s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
        }
    }
    else if (TIMER_REFRESH_HOME_EMR == wParam)
    {
        s_u32EMRcurtime++;
        s32Ret = HI_GV_Timer_Reset(HOME_PAGE,TIMER_REFRESH_HOME_EMR);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        s32Ret = HI_GV_ProgressBar_SetPos(HOME_EMR_PROGRESS,s_u32EMRcurtime*UI_HOME_EMR_TIME_INTERVAL);
        s32Ret |= HI_GV_Widget_Paint(HOME_EMR_PROGRESS,NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_HOME_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_EventProc((HI_EVENT_S *)lParam);
    if(s32Ret)
    {
        MLOGE("s32Ret %x\n", s32Ret);
    }


    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_HOME_OnKeyUp(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);


    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_HOME_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_STATEMNG_GetState");

    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("WorkMode: %u, StateMngInProgress!\n", stWorkModeState.enWorkMode);
        return HIGV_PROC_GOON;
    }
#ifndef ONE_SENSOR_CONNECT

    if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_SET_VoSwitch();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HIGV_PROC_STOP;
    }
#endif
    if (PDT_UI_KEY_BACK == wParam)
    {

        if(HI_FAILURE == PDT_UI_COMM_CheckSd())
        {
            MLOGE("SD is not normal\n");
            return HIGV_PROC_GOON;
        }

        HI_MESSAGE_S stMessage = {};
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
        s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
        if(s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
            return HIGV_PROC_GOON;
        }
    }
    else if ( PDT_UI_KEY_MENU == wParam)
    {
#ifdef CFG_TIME_STAMP_ON
        PDT_UI_COMM_SwitchPageTime_begin();
#endif
        s32Ret = PDT_UI_HOME_ToFilelist();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
#ifdef CFG_TIME_STAMP_ON
        PDT_UI_COMM_SwitchPageTime_begin();
#endif

        if( PDT_NETCTRL_IsClientConnecting())
        {
            MLOGI("Client connecting!\n");
            return HIGV_PROC_GOON;
        }

        if(HI_PDT_WORKMODE_PHOTO == stWorkModeState.enWorkMode)
        {
            s_enKeyAction = PDT_UI_KEY_ACTION_SETTING;
            s32Ret = PDT_UI_HOME_DoKeyAction(s_enKeyAction);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            if(stWorkModeState.bEMRRecord)
            {
                MLOGE("EMRRecording, not support keys\n");
                return HIGV_PROC_GOON;

            }

            s32Ret = PDT_UI_COMM_SendStopMsg(PDT_UI_HOME_OnReceiveMsgResult);
            if(s32Ret)
            {
                MLOGE("s32Ret %x\n", s32Ret);
                s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
            }
            s_enKeyAction = PDT_UI_KEY_ACTION_SETTING;
        }

    }
    else if (PDT_UI_HOME_KEY_WIFI == wParam)
    {
#if defined(CONFIG_WIFI_ON)
        HI_S32 s32Ret = HI_SUCCESS;
        HI_BOOL bWiFiOn = HI_FALSE;

        if (stWorkModeState.bEMRRecord)
        {
            MLOGE("EMRRecording, not support keys\n");
            return HIGV_PROC_GOON;
        }

        s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWiFiOn);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        if(HI_FALSE == bWiFiOn)
        {
            HI_HAL_WIFI_APMODE_CFG_S stApCfg;
            HI_HAL_WIFI_CFG_S stCfg;
            memset(&stApCfg, '\0', sizeof(HI_HAL_WIFI_APMODE_CFG_S));

            stCfg.enMode = HI_HAL_WIFI_MODE_AP;
            s32Ret = HI_HAL_WIFI_Init(stCfg.enMode);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            /* after init wifi,do get ap param and start wifi */
            s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stApCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            memcpy(&stCfg.unCfg.stApCfg, &stApCfg, sizeof(HI_HAL_WIFI_APMODE_CFG_S));
            s32Ret = HI_HAL_WIFI_Start(&stCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL, SKIN_HOME_BUTTON_WIFI_ON);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_WIFI_ON);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            HI_VOICEPLAY_VOICE_S stVoice=
            {
                .au32VoiceIdx={PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_OPEN_IDX},
                .u32VoiceCnt=2,
                .bDroppable=HI_TRUE,
            };
            HI_VOICEPLAY_Push(&stVoice, 0);

            MLOGD("HI_HAL_WIFI_Start\n");
        }
        else
        {
            s32Ret = HI_HAL_WIFI_Stop();
            HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

            s32Ret = HI_HAL_WIFI_Deinit();
            HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

            s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL,SKIN_HOME_BUTTON_WIFI_OFF);
            s32Ret |= HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE, SKIN_HOME_BUTTON_WIFI_OFF);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

            HI_VOICEPLAY_VOICE_S stVoice=
            {
                .au32VoiceIdx={PDT_UI_VOICE_WIFI_IDX,PDT_UI_VOICE_CLOSE_IDX},
                .u32VoiceCnt=2,
                .bDroppable=HI_TRUE,
            };
            HI_VOICEPLAY_Push(&stVoice, 0);

            MLOGD("HI_HAL_WIFI_Stop\n");
        }
        s32Ret = HI_GV_Widget_Paint(HOME_BUTTON_WIFI, HI_NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
#endif
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    /*HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 s32StartX, s32StartY, s32EndX, s32EndY;

    s32StartX = pstGestureEvent->gesture.fling.start.x;
    s32StartY = pstGestureEvent->gesture.fling.start.y;
    s32EndX = pstGestureEvent->gesture.fling.end.x;
    s32EndY = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", s32StartX, s32StartY, s32EndX, s32EndY);*/

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;

    MLOGD("widget:%d, [%d,%d]\n",WidgetHdl, pstGestureEvent->gesture.tap.pointer.x, pstGestureEvent->gesture.tap.pointer.y);

    return HIGV_PROC_GOON;
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
    MLOGD("[%d,%d] --> [%d,%d], Distance[%d,%d]\n",
          stGestureEvent.gesture.scroll.start.x, pstGestureEvent->gesture.scroll.start.y,
          stGestureEvent.gesture.scroll.end.x, pstGestureEvent->gesture.scroll.end.y,
          stGestureEvent.gesture.scroll.distanceX, pstGestureEvent->gesture.scroll.distanceY);
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

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

