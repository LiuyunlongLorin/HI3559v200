/**
 * @file      ui_home_page.c
 * @brief     ui home page,live preview. do snap, record work.
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

#include "hi_filemng_dcf.h"
#include "hi_storagemng.h"

#include "ui_common.h"
#include "ui_powercontrol.h"
#include "ui_common_playback.h"

#ifdef CONFIG_GAUGE_ON
#include "hi_gaugemng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_BOOL s_bRefreshOnce = HI_TRUE;
static HI_BOOL s_bRefreshSuccessed = HI_FALSE;
static HI_BOOL s_bHide = HI_FALSE;
static HI_BOOL s_bTakingPhoto = HI_FALSE;
static HI_BOOL s_bCheckSD = HI_FALSE;
static HI_BOOL s_bFileMngReady = HI_FALSE;
static HI_U32 s_u32TimerCount = 0;

#ifdef CFG_TIME_MEASURE_ON
struct timespec stTimespecBegin, stTimespecEnd;
#endif

#ifdef CONFIG_GAUGE_ON
static HI_BOOL s_bRefreshBatteryInfo = HI_TRUE;
static HI_EVENT_GAUGEMNG_E s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_BUIT;
#endif

/*State Machine Msg Result Callback*/
static HI_S32 PDT_UI_HOME_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_PrepareToFilelist(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(stWorkModeState.bStateMngInProgress)
    {
        MLOGE("BUSY\n");
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = HI_PDT_WORKMODE_PLAYBACK;

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_PLAYBACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HI_SUCCESS;
}

/**
 *  [x,y] is view's central coordinate.
 */
static HI_VOID MoveSpotMetering(HI_S32 x, HI_S32 y)
{
    HI_RECT rectSpot = {0};
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_GetRect(HOME_SPOT_METERING, &rectSpot);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%#x\n", s32Ret);
        return;
    }

    s32Ret = HI_GV_Widget_Move(HOME_SPOT_METERING, x - rectSpot.w/2, y - rectSpot.h/2);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%#x\n", s32Ret);
        return;
    }

}

static HI_S32 PDT_UI_HOME_SendSpotMeteringMsg(HI_S32 s32X, HI_S32 s32Y, HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret;
    HI_S32 s32GestureTapX;
    HI_S32 s32GestureTapY;
    HI_U32 u32SpotMeteringIconWidth;
    HI_U32 u32SpotMeteringIconHeight;
    HI_PDT_SCENE_METRY_SPOT_PARAM_S stMetrySpotParam;

    /** calculate x */
    if(s32X < ICON_SPOT_METERING_WIDTH / 2)
    {
        s32GestureTapX = ICON_SPOT_METERING_WIDTH / 2;
    }
    else if(s32X > (UI_SCREEN_WIDTH - ICON_SPOT_METERING_WIDTH / 2))
    {
        s32GestureTapX = UI_SCREEN_WIDTH - ICON_SPOT_METERING_WIDTH / 2;
    }
    else
    {
        s32GestureTapX = s32X;
    }

    /** calculate y */
    if(s32Y < ICON_SPOT_METERING_HEIGHT / 2)
    {
        s32GestureTapY = ICON_SPOT_METERING_HEIGHT / 2;
    }
    else if(s32Y > (UI_SCREEN_HEIGHT - ICON_SPOT_METERING_HEIGHT / 2))
    {
        s32GestureTapY = UI_SCREEN_HEIGHT - ICON_SPOT_METERING_HEIGHT / 2;
    }
    else
    {
        s32GestureTapY = s32Y;
    }

    /** move icon */
    MoveSpotMetering(s32GestureTapX, s32GestureTapY);

    /** calculate width and height */
    u32SpotMeteringIconWidth = 100 * ICON_SPOT_METERING_WIDTH / UI_SCREEN_WIDTH;
    u32SpotMeteringIconHeight = 100 * ICON_SPOT_METERING_HEIGHT / UI_SCREEN_HEIGHT;

#if defined(CONFIG_VERTICAL_SCREEN)
    HIGV_ROTATE_E enRotate = HIGV_ROTATE_90;
#else
    HIGV_ROTATE_E enRotate = HIGV_ROTATE_NONE;
#endif

    /** rotate */
    if(HIGV_ROTATE_NONE == enRotate)
    {
        stMetrySpotParam.stPoint.s32X = 100 * s32GestureTapX / UI_SCREEN_WIDTH;
        stMetrySpotParam.stPoint.s32Y = 100 * s32GestureTapY / UI_SCREEN_HEIGHT;
        stMetrySpotParam.stSize.u32Width = u32SpotMeteringIconWidth;
        stMetrySpotParam.stSize.u32Height = u32SpotMeteringIconHeight;
    }
    else if(HIGV_ROTATE_90 == enRotate)
    {
        stMetrySpotParam.stPoint.s32X = 100 * s32GestureTapY / UI_SCREEN_HEIGHT;
        stMetrySpotParam.stPoint.s32Y = 100 - 100 * s32GestureTapX / UI_SCREEN_WIDTH;
        stMetrySpotParam.stSize.u32Width = u32SpotMeteringIconHeight;
        stMetrySpotParam.stSize.u32Height = u32SpotMeteringIconWidth;
    }

    HI_MESSAGE_S stMessage;
    memset(&stMessage, 0x0, sizeof(HI_MESSAGE_S));

    stMessage.what = HI_EVENT_STATEMNG_SETTING;
    stMessage.arg1 = HI_PDT_PARAM_TYPE_PROTUNE_METRY;
    stMessage.arg2 = enWorkMode;

    HI_CHAR* payload = (stMessage.aszPayload);
    *(HI_PDT_SCENE_METRY_TYPE_E *)payload = HI_PDT_SCENE_METRY_TYPE_SPOT;
    memcpy((payload + sizeof(HI_PDT_SCENE_METRY_TYPE_E)), &stMetrySpotParam,
        sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));

    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_ShowSpotMeteringIcon(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 s32Value = 0;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_PDT_WORKMODE_E enWorkMode;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, (HI_VOID *)&enWorkMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enWorkMode, HI_PDT_PARAM_TYPE_PROTUNE_METRY, &s32Value);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_PDT_SCENE_METRY_TYPE_SPOT == s32Value)
    {
        if(!HI_GV_Widget_IsShow(HOME_SPOT_METERING))
        {
            s32Ret = HI_GV_Widget_Show(HOME_SPOT_METERING);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            if(HI_PDT_STATEMNG_ENOTINIT != s32Ret)
            {
                s32Ret = PDT_UI_HOME_SendSpotMeteringMsg(UI_SCREEN_WIDTH / 2, UI_SCREEN_HEIGHT / 2, enWorkMode);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            }
        }
    }
    else
    {
        if(HI_GV_Widget_IsShow(HOME_SPOT_METERING))
        {
            s32Ret = HI_GV_Widget_Hide(HOME_SPOT_METERING);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetSDInfo(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszBuffer[8] = {0};
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;
    HI_STORAGE_FS_INFO_S stFSInfo = {};

    s32Ret = PDT_UI_COMM_GetStorageState(&enStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_STORAGE_STATE_MOUNTED != enStorageState)
    {
        snprintf(aszBuffer,sizeof(aszBuffer), "?");
    }
    else
    {
        s32Ret = PDT_UI_COMM_GetFSInfo(&stFSInfo);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        snprintf(aszBuffer, sizeof(aszBuffer), "%llu%%",
                 (stFSInfo.u64AvailableSize + (stFSInfo.u64TotalSize * 5 / 1000)) * 100 / stFSInfo.u64TotalSize); /** rounding */
    }

    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_SD_INFO, aszBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetWifiButtonSkin(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

#if defined(CONFIG_WIFI_ON)
    HI_BOOL bWifiOn = HI_FALSE;

    s32Ret = HI_HAL_WIFI_GetStartedStatus(&bWifiOn);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE,
        bWifiOn ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);

    s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL,
        bWifiOn ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
#else
    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_WIFI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetClientButtonState(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_TRUE == HI_PDT_NETCTRL_CheckClientConnect(HI_NULL))
    {
        s32Ret = HI_GV_Widget_Show(HOME_BUTTON_CLIENT);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_CLIENT);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    return HI_SUCCESS;
}

#ifdef CONFIG_GAUGE_ON
static HI_S32 PDT_UI_HOME_SetBatteryInfo(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 s32BatteryLevel;
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

    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_UI_HOME_SetModeButtonSkin(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    PDT_UI_WORKMODE_AND_SKIN_S astWorkModeSkin[] =
    {
        {HI_PDT_WORKMODE_NORM_REC,   SKIN_HOME_BUTTON_MODE_RECORD_NORMAL},
        {HI_PDT_WORKMODE_LOOP_REC,   SKIN_HOME_BUTTON_MODE_RECORD_LOOP},
        {HI_PDT_WORKMODE_LPSE_REC,   SKIN_HOME_BUTTON_MODE_RECORD_LAPSE},
        {HI_PDT_WORKMODE_SLOW_REC,   SKIN_HOME_BUTTON_MODE_RECORD_SLOW},
        {HI_PDT_WORKMODE_RECSNAP,    SKIN_HOME_BUTTON_MODE_RECORD_SNAP},
        {HI_PDT_WORKMODE_LPSE_PHOTO, SKIN_HOME_BUTTON_MODE_MULTI_LAPSE},
        {HI_PDT_WORKMODE_BURST,      SKIN_HOME_BUTTON_MODE_MULTI_BURST},
        {HI_PDT_WORKMODE_SING_PHOTO, SKIN_HOME_BUTTON_MODE_PHOTO_SINGLE},
        {HI_PDT_WORKMODE_DLAY_PHOTO, SKIN_HOME_BUTTON_MODE_PHOTO_DELAY},
    };

    for(i=0; i<ARRAY_SIZE(astWorkModeSkin); i++)
    {
        if(pstWorkState->enWorkMode == astWorkModeSkin[i].enWorkMode)
        {
            break;
        }
    }

    if(i>= ARRAY_SIZE(astWorkModeSkin))
    {
        MLOGE("not found workmode(%d)'s skin\n", pstWorkState->enWorkMode);
        return HI_FAILURE;
    }

    s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_MODE, HIGV_SKIN_NORMAL, astWorkModeSkin[i].hSkin);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_MODE, HIGV_SKIN_ACITVE, astWorkModeSkin[i].hSkin);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_Paint(HOME_BUTTON_MODE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetRecFlagButtonSkin(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstWorkState->enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_RECSNAP:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            if (pstWorkState->bRunning && s_u32TimerCount % 2)
            {
                s32Ret = HI_GV_Widget_Show(HOME_BUTTON_VIDEO);
            }
            else
            {
                s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
            }
            break;

        default:
            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
            break;
    }
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Error:%#x\n", s32Ret);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetDelayTime(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32  s32Ret = 0;
    HI_CHAR aszBuffer[32] = {};
    HI_U64 u64RemainTime = 0;

    if(pstWorkState->bRunning && HI_PDT_WORKMODE_DLAY_PHOTO == pstWorkState->enWorkMode)
    {
        u64RemainTime = PDT_UI_COMM_GetRemainDelayTime();

        if(u64RemainTime > 0)
        {
            snprintf(aszBuffer, sizeof(aszBuffer), "%llu", u64RemainTime);
        }
        else if(pstWorkState->bRunning && !PDT_UI_WINMNG_WindowIsShow(BUSY_PAGE) )
        {
            /* delay time finished, show BUSY_PAGE */
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("StartWindow failed. %x\n", s32Ret);
            }
        }
    }
    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_DELAY_TIME, aszBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetBottomLeftInfo(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32RemainTime = 0;
    HI_U32 u32PhotoFileCnt = 0;
    HI_U32 u32RemainCount = 0;
    HI_CHAR aszBuffer[32] = {};

    if(HI_FALSE == pstWorkState->bRunning && HI_FALSE == s_bRefreshOnce)
    {
        return HI_SUCCESS;
    }
    else
    {
        s_bRefreshOnce = HI_TRUE;
    }

    switch(pstWorkState->enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        case HI_PDT_WORKMODE_LOOP_REC:
        case HI_PDT_WORKMODE_LPSE_REC:
        case HI_PDT_WORKMODE_SLOW_REC:
        case HI_PDT_WORKMODE_RECSNAP:
            if(pstWorkState->bRunning)
            {
                PDT_UI_COMM_Second2String(PDT_UI_COMM_GetExecTime(), aszBuffer, sizeof(aszBuffer));
            }
            else
            {
                PDT_UI_COMM_Second2String(0, aszBuffer, sizeof(aszBuffer));
            }

            s32Ret = PDT_UI_COMM_GetRemainRecordTime(&u32RemainTime);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            snprintf(aszBuffer + strnlen(aszBuffer, sizeof(aszBuffer)),
                sizeof(aszBuffer) - strnlen(aszBuffer, sizeof(aszBuffer)), "%s", "/");

            PDT_UI_COMM_Second2String((HI_U64)u32RemainTime, aszBuffer + strnlen(aszBuffer, sizeof(aszBuffer)),
                sizeof(aszBuffer) - strnlen(aszBuffer, sizeof(aszBuffer)));
            break;

        case HI_PDT_WORKMODE_SING_PHOTO:
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        case HI_PDT_WORKMODE_BURST:
        case HI_PDT_WORKMODE_LPSE_PHOTO:
                if(s_bFileMngReady)
                {
                    s32Ret = HI_FILEMNG_GetFileObjCnt(HI_FILEMNG_FILE_TYPE_PHOTO, &u32PhotoFileCnt);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
                else
                {
                    u32PhotoFileCnt = 0;
                }

                s32Ret = PDT_UI_COMM_GetRemainPhotoCount(&u32RemainCount);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

                snprintf(aszBuffer, sizeof(aszBuffer), "%04u/%04u", u32PhotoFileCnt, u32RemainCount);
            break;

        default:
            break;
    }

    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_LEFT, aszBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_FALSE == pstWorkState->bRunning)
    {
        if((HI_PDT_WORKMODE_SING_PHOTO == pstWorkState->enWorkMode || HI_PDT_WORKMODE_DLAY_PHOTO == pstWorkState->enWorkMode
            || HI_PDT_WORKMODE_BURST == pstWorkState->enWorkMode || HI_PDT_WORKMODE_LPSE_PHOTO == pstWorkState->enWorkMode)
            && HI_FALSE == s_bFileMngReady)
        {
            s_bRefreshOnce = HI_TRUE;
        }
        else
        {
            s_bRefreshOnce = HI_FALSE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_HOME_SetBottomRightInfo(HI_PDT_STATEMNG_WORKMODE_STATE_S* pstWorkState)
{
    HI_S32  s32Ret = 0;
    HI_CHAR aszBuffer[32] = {};

    s32Ret = PDT_UI_COMM_GetParamValueDesc(pstWorkState->enWorkMode, HI_PDT_PARAM_TYPE_MEDIAMODE, aszBuffer, sizeof(aszBuffer));
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_SetText(HOME_LABEL_RIGHT, aszBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

/**Refresh by timer*/
HI_S32 PDT_UI_HOME_RefreshOnTimer(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    if(HI_PDT_STATEMNG_ENOTINIT == s32Ret)
    {
        return HI_SUCCESS;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = PDT_UI_HOME_SetRecFlagButtonSkin(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetDelayTime(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetBottomLeftInfo(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    /*refresh once every 10 times*/
    if(0 == s_u32TimerCount % 10)
    {
        s32Ret = PDT_UI_HOME_SetSDInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}


/** Refresh all widgets when 'EvencProc' or 'OnShow' be called back */
HI_S32 PDT_UI_HOME_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_HOME_ShowSpotMeteringIcon();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {.enWorkMode = HI_PDT_WORKMODE_NORM_REC,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    if(HI_SUCCESS != s32Ret)
    {
        s_bRefreshSuccessed = HI_FALSE;
        MLOGD("GetState failed, skip refresh home\n");
        return HI_SUCCESS;
    }

    if(HI_FALSE == s_bCheckSD)
    {
        PDT_UI_COMM_SdIsReady();
        s_bCheckSD = HI_TRUE;
    }

    s32Ret = PDT_UI_HOME_SetWifiButtonSkin();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetSDInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetClientButtonState();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

#ifdef CONFIG_GAUGE_ON
    if(HI_TRUE == s_bRefreshBatteryInfo)
    {
        s32Ret = PDT_UI_HOME_SetBatteryInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    s_bRefreshBatteryInfo = HI_FALSE;
#endif

    s32Ret = PDT_UI_HOME_SetRecFlagButtonSkin(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetModeButtonSkin(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetDelayTime(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetBottomLeftInfo(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_HOME_SetBottomRightInfo(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_Widget_Paint(HOME_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bRefreshSuccessed = HI_TRUE;
    return HI_SUCCESS;
}


/** Process events from EVENTHUB*/
HI_S32 PDT_UI_HOME_EventProc(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = 0;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2,pstEvent->s32Result);

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_START:
            s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

        case HI_EVENT_PHOTOMNG_TASK_STARTED:
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            if(HI_PDT_WORKMODE_SING_PHOTO == stWorkModeState.enWorkMode
                || HI_PDT_WORKMODE_BURST == stWorkModeState.enWorkMode)
            {
                s_bTakingPhoto = HI_TRUE;
            }
            break;

        case HI_EVENT_PHOTOMNG_TASK_END:
#ifdef CFG_TIME_MEASURE_ON
            s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
            if((HI_PDT_WORKMODE_SING_PHOTO == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_BURST == stWorkModeState.enWorkMode)
            && stTimespecBegin.tv_sec > 0)
            {
                (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecEnd);
                HI_U32 u32WaitTime_ms = (stTimespecEnd.tv_sec * 1000 + stTimespecEnd.tv_nsec / 1000000)
                    - (stTimespecBegin.tv_sec * 1000 + stTimespecBegin.tv_nsec / 1000000);
                MLOGI(GREEN"Take photo %dms\n"NONE, u32WaitTime_ms);
            }
#endif
            /** no break */
        case HI_EVENT_STATEMNG_STOP:
            s_bTakingPhoto = HI_FALSE;
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            s_bRefreshOnce = HI_TRUE;
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

#ifdef CONFIG_GAUGE_ON
        case HI_EVENT_GAUGEMNG_LEVEL_CHANGE:
            s_bRefreshBatteryInfo = HI_TRUE;
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_LOW:
            if(HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW != s_enLastBatteryEvent
                && HI_EVENT_GAUGEMNG_LEVEL_LOW != s_enLastBatteryEvent)
            {
                PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_LOW_BATTERY);
            }
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_LOW;
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW:
            if(HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW != s_enLastBatteryEvent)
            {
                PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_ULTRALOW_BATTERY);
            }
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_ULTRALOW;
            break;

        case HI_EVENT_GAUGEMNG_LEVEL_NORMAL:
            s_enLastBatteryEvent = HI_EVENT_GAUGEMNG_LEVEL_NORMAL;
            break;
#endif
        case HI_EVENT_USB_OUT:
            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

        case HI_EVENT_USB_INSERT:
            s32Ret = HI_GV_Widget_Show(HOME_BUTTON_BATTERY_CHARGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_FORMAT:
            s_bRefreshOnce = HI_TRUE;
        break;

        case HI_EVENT_NETCTRL_CLIENT_CONNECTED:
            s32Ret = HI_GV_Widget_Show(HOME_BUTTON_CLIENT);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;

        case HI_EVENT_NETCTRL_CLIENT_DISCONNECTED:
            if (HI_FALSE == HI_PDT_NETCTRL_CheckClientConnect(HI_NULL))
            {
                s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_CLIENT);
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            }
            break;

        default:
            break;
    }

    s32Ret = PDT_UI_HOME_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_HOME_OnRefresh(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret;

    s32Ret = HI_GV_SetRefreshCombine(HI_TRUE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetRefreshCombine");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_Hide(HI_BOOL bHide)
{
    HI_S32 s32Ret;

    if(HI_TRUE == bHide)
    {
        s32Ret  = HI_GV_Widget_Hide(HOME_GROUPBOX_TITLE_ABOVE);
        s32Ret |= HI_GV_Widget_Hide(HOME_LABEL_LEFT);
        s32Ret |= HI_GV_Widget_Hide(HOME_LABEL_RIGHT);
        s32Ret |= HI_GV_Widget_Hide(HOME_BUTTON_MODE);
    }
    else
    {
        s32Ret  = HI_GV_Widget_Show(HOME_GROUPBOX_TITLE_ABOVE);
        s32Ret |= HI_GV_Widget_Show(HOME_LABEL_LEFT);
        s32Ret |= HI_GV_Widget_Show(HOME_LABEL_RIGHT);
        s32Ret |= HI_GV_Widget_Show(HOME_BUTTON_MODE);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}


HI_S32 PDT_UI_HOME_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s_bTakingPhoto = HI_FALSE;

    s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_VIDEO);
    s32Ret |= HI_GV_Widget_Hide(HOME_BUTTON_CLIENT);
#ifndef CONFIG_WIFI_ON
    s32Ret |= HI_GV_Widget_Hide(HOME_BUTTON_WIFI);
#endif
    if(HI_TRUE == PDT_UI_COMM_IsUSBIn())
    {
        s32Ret |= HI_GV_Widget_Show(HOME_BUTTON_BATTERY_CHARGE);
    }
    else
    {
        s32Ret |= HI_GV_Widget_Hide(HOME_BUTTON_BATTERY_CHARGE);
    }
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideWidget");

    s32Ret = PDT_UI_COMM_UpdateTotalBitRate();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "UpdateTotalBitRate");

    s_bRefreshOnce = HI_TRUE;
    s32Ret = PDT_UI_HOME_Refresh();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HOME_Refresh");

    s32Ret = PDT_UI_HOME_Hide(HI_FALSE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideHome");
    s_bHide = HI_FALSE;

    s32Ret = HI_GV_Timer_Create(HOME_PAGE, TIMER_REFRESH_HOME, 1000);
    s32Ret |= HI_GV_Timer_Start(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Create or Start Timer");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Timer_Stop");

    s32Ret = HI_GV_Timer_Destroy(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Timer_Destory");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s_u32TimerCount++;
    s32Ret = HI_GV_Timer_Reset(HOME_PAGE, TIMER_REFRESH_HOME);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Timer_Reset");

    if(HI_FALSE == PDT_UI_POWERCTRL_IsSystemDormant())
    {
        if(HI_TRUE == s_bRefreshSuccessed)
        {
            s32Ret = PDT_UI_HOME_RefreshOnTimer();
        }
        else
        {
            s32Ret = PDT_UI_HOME_Refresh();
        }
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Refresh");
    }

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_HOME_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_EVENT_S *pstEvent = (HI_EVENT_S *)lParam;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            s_bFileMngReady = HI_FALSE;
            break;

        case HI_EVENT_FILEMNG_SCAN_COMPLETED:
            s_bFileMngReady = HI_TRUE;
            break;

        default:
            break;
    }

    s32Ret = PDT_UI_COMM_LCDEventProc((HI_EVENT_S *)lParam);
    if(s32Ret)
    {
        MLOGE("s32Ret %x\n", s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMessage = {};
#if defined(CONFIG_WIFI_ON)
    PDT_UI_WIFI_STATUS_S stWiFiStatus;
#endif

    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(s32Ret)
    {
        MLOGE("GetState\n");
        return HIGV_PROC_GOON;
    }

    if (stWorkModeState.bStateMngInProgress || (HI_TRUE == s_bTakingPhoto))
    {
        MLOGI("WorkMode=%u, InProgress=%d, bTakingPhoto=%d\n", stWorkModeState.enWorkMode,
            stWorkModeState.bStateMngInProgress, s_bTakingPhoto);
        return HIGV_PROC_GOON;
    }

    if (PDT_UI_KEY_BACK == wParam && !stWorkModeState.bRunning)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(QUICK_PAGE, HI_TRUE);
        if(s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
        }
    }
    else if (PDT_UI_KEY_MENU == wParam && !stWorkModeState.bRunning)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(MODE_PAGE, HI_TRUE);
        if(s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_UI_HOME_Hide(HI_FALSE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideHome");
        s_bHide = HI_FALSE;

        if (!PDT_UI_COMM_SdIsReady())
        {
            return HIGV_PROC_GOON;
        }

#ifdef CFG_TIME_MEASURE_ON
        if(HI_PDT_WORKMODE_SING_PHOTO == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_BURST == stWorkModeState.enWorkMode)
        {
            (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecBegin);
        }
#endif

        switch(stWorkModeState.enWorkMode)
        {
            case HI_PDT_WORKMODE_BURST:
            case HI_PDT_WORKMODE_SING_PHOTO:
                if(stWorkModeState.bRunning)
                {
                    /*we will send HI_EVENT_STATEMNG_STOP until receive event TASKEND*/
                    MLOGD("ignore keys while taking photo\n");
                    return HIGV_PROC_GOON;
                }
                stMessage.what = HI_EVENT_STATEMNG_START;
                break;
            case HI_PDT_WORKMODE_NORM_REC:
            case HI_PDT_WORKMODE_LOOP_REC:
            case HI_PDT_WORKMODE_LPSE_REC:
            case HI_PDT_WORKMODE_SLOW_REC:
            case HI_PDT_WORKMODE_DLAY_PHOTO:
            case HI_PDT_WORKMODE_LPSE_PHOTO:
            case HI_PDT_WORKMODE_RECSNAP:
                stMessage.what = stWorkModeState.bRunning ? HI_EVENT_STATEMNG_STOP : HI_EVENT_STATEMNG_START;
                break;

            default:
                MLOGE("WorkMode %d ignored\n", stWorkModeState.enWorkMode);
                return HIGV_PROC_GOON;
        }

        if(HI_EVENT_STATEMNG_START == stMessage.what)
        {
            if(HI_PDT_WORKMODE_LOOP_REC == stWorkModeState.enWorkMode)
            {
                HI_S32 s32LoopTime = 0;
                HI_U32 u32RemainTime = 0;
                s32Ret = HI_PDT_PARAM_GetWorkModeParam(HI_PDT_WORKMODE_LOOP_REC, HI_PDT_PARAM_TYPE_LOOP_TIME, &s32LoopTime);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

                s32Ret = PDT_UI_COMM_GetRemainRecordTime(&u32RemainTime);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

                if(u32RemainTime < (HI_U32)(s32LoopTime * 60))
                {
                    PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_SD_FULL);
                    return HIGV_PROC_GOON;
                }
            }
            else
            {
                if(PDT_UI_COMM_SdIsFull())
                {
                    PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_SD_FULL);
                    return HIGV_PROC_GOON;
                }
            }
        }

        if(HI_PDT_WORKMODE_SING_PHOTO == stWorkModeState.enWorkMode
            || HI_PDT_WORKMODE_BURST == stWorkModeState.enWorkMode)
        {
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            if(s32Ret)
            {
                MLOGE("s32Ret %x\n", s32Ret);
            }
        }

        stMessage.arg2 = stWorkModeState.enWorkMode;
        s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_HOME_OnReceiveMsgResult);
        if(s32Ret)
        {
            MLOGE("s32Ret %x\n", s32Ret);
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
        }

    }
    else if (PDT_UI_HOME_KEY_WIFI == wParam)
    {
#if defined(CONFIG_WIFI_ON)
#ifdef CFG_TIME_MEASURE_ON
        (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecBegin);
#endif
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

            s32Ret = HI_GV_Widget_Hide(HOME_BUTTON_CLIENT);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        }

        s32Ret = PDT_UI_COMM_SetWiFiStatus(&stWiFiStatus);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_ACITVE,
                                      stWiFiStatus.bStart ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);

        s32Ret = HI_GV_Widget_SetSkin(HOME_BUTTON_WIFI, HIGV_SKIN_NORMAL,
            stWiFiStatus.bStart ? SKIN_HOME_BUTTON_WIFI_ON : SKIN_HOME_BUTTON_WIFI_OFF);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = HI_GV_Widget_Paint(HOME_BUTTON_WIFI, HI_NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

#ifdef CFG_TIME_MEASURE_ON
        (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &stTimespecEnd);
        HI_U32 u32WaitTime_ms = (stTimespecEnd.tv_sec * 1000 + stTimespecEnd.tv_nsec / 1000000)
            - (stTimespecBegin.tv_sec * 1000 + stTimespecBegin.tv_nsec / 1000000);
        MLOGI(GREEN"%s %dms\n"NONE, stWiFiStatus.bStart ? "Start WiFi" : "Stop WiFi", u32WaitTime_ms);
#endif
#endif
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32StartX, s32StartY, s32EndX, s32EndY;

    s32StartX = pstGestureEvent->gesture.fling.start.x;
    s32StartY = pstGestureEvent->gesture.fling.start.y;
    s32EndX = pstGestureEvent->gesture.fling.end.x;
    s32EndY = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", s32StartX, s32StartY, s32EndX, s32EndY);

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if(s32Ret)
    {
        MLOGE("GetState\n");
        return HIGV_PROC_GOON;
    }
    if (stWorkModeState.bStateMngInProgress || stWorkModeState.bRunning || (HI_TRUE == s_bTakingPhoto))
    {
        MLOGI("WorkMode=%u, InProgress=%d, IsRunning=%d, bTakingPhoto=%d\n", stWorkModeState.enWorkMode,
            stWorkModeState.bStateMngInProgress, stWorkModeState.bRunning, s_bTakingPhoto);
        return HIGV_PROC_GOON;
    }

    if (abs(s32StartX - s32EndX) > abs(s32StartY - s32EndY))
    {
        if (s32StartX > s32EndX)
        {
            MLOGD("fling left\n");
            s32Ret = PDT_UI_WINMNG_StartWindow(SET_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else
        {
            MLOGD("fling right\n");
            s32Ret = PDT_UI_HOME_PrepareToFilelist();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else
    {
        if (s32StartY > s32EndY)
        {
            MLOGD("fling up\n");
            s32Ret = PDT_UI_WINMNG_StartWindow(MODE_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else
        {
            MLOGD("fling down\n");
            s32Ret = PDT_UI_WINMNG_StartWindow(QUICK_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HOME_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    MLOGD("widget:%d, [%d,%d]\n",WidgetHdl, pstGestureEvent->gesture.tap.pointer.x,
        pstGestureEvent->gesture.tap.pointer.y);

    HI_S32 s32Ret;
    HI_PDT_SCENE_METRY_TYPE_E enMetryType = HI_PDT_SCENE_METRY_TYPE_BUTT;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_PARAM_GetWorkModeParam(stWorkModeState.enWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_METRY, (HI_VOID *)&enMetryType);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_PDT_SCENE_METRY_TYPE_SPOT == enMetryType)
    {
        s32Ret = PDT_UI_HOME_SendSpotMeteringMsg(pstGestureEvent->gesture.tap.pointer.x,
            pstGestureEvent->gesture.tap.pointer.y, stWorkModeState.enWorkMode);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else
    {
        s_bHide = (HI_TRUE == s_bHide ? HI_FALSE : HI_TRUE);
        s32Ret = PDT_UI_HOME_Hide(s_bHide);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HideHome");
    }

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

