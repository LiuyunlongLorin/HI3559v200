/**
 * @file      ui_playback_page.c
 * @brief     playback video and photo.
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

#include "ui_common_playback.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


static HI_S32 s_s32Volume = -1;
static HI_BOOL s_bRefresh = HI_FALSE;
static HI_BOOL s_bIsTracking = HI_FALSE;
static HI_BOOL s_bShowUI = HI_TRUE;
static HI_HANDLE s_AoChnHdl = HI_INVALID_HANDLE;
static HI_HANDLE s_AoHdl = HI_INVALID_HANDLE;
extern HIGV_HANDLE s_u32WinCallDialog;
static HIGV_HANDLE s_ActiveBtnHdl = PLAYBACK_BUTTON_BACK;


HI_S32 PDT_UI_PlAYBACK_Refresh(HI_VOID)
{
    if(HI_FALSE == s_bRefresh)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;
    HIGV_HANDLE aPlaybackBtnHdl[] = {
        PLAYBACK_BUTTON_BACK,
        PLAYBACK_BUTTON_VOLUME,
        PLAYBACK_BUTTON_SPEED,
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_PLAY,
        PLAYBACK_BUTTON_NEXT,
        PLAYBACK_TRACKBAR_PLAY
    };

    PDT_UI_COMM_PLAYBACK_SetPlayTime(PLAYBACK_BUTTON_INFO);

    if (!s_bShowUI)
    {
        for (i=0; i<ARRAY_SIZE(aPlaybackBtnHdl); i++)
        {
            s32Ret = HI_GV_Widget_Hide(aPlaybackBtnHdl[i]);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s32Ret = HI_GV_Widget_Paint(PLAYBACK_PAGE, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_SUCCESS;
    }

    s32Ret = HI_GV_Widget_Show(PLAYBACK_BUTTON_BACK);
    s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_DELETE);
    s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_PREV);
    s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_NEXT);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
    if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
    {
        s32Ret = HI_GV_Widget_Hide(PLAYBACK_BUTTON_PLAY);
        s32Ret |= HI_GV_Widget_Hide(PLAYBACK_TRACKBAR_PLAY);
        s32Ret |= HI_GV_Widget_Hide(PLAYBACK_BUTTON_VOLUME);
        s32Ret |= HI_GV_Widget_Hide(PLAYBACK_BUTTON_SPEED);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Show(PLAYBACK_BUTTON_VOLUME);
        s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_SPEED);
        s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_PLAY);
        s32Ret |= HI_GV_Widget_Show(PLAYBACK_TRACKBAR_PLAY);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* if no widget is active, then active back button */
    for (i = 0; i < ARRAY_SIZE(aPlaybackBtnHdl); i++)
    {
        if (HI_GV_Widget_IsActive(aPlaybackBtnHdl[i]))
        {
            break;
        }
    }
    if (i >= ARRAY_SIZE(aPlaybackBtnHdl))
    {
        s32Ret = HI_GV_Widget_Active(PLAYBACK_BUTTON_BACK);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (!s_bIsTracking)
    {
        PDT_UI_COMM_PLAYBACK_SetTrack(PLAYBACK_TRACKBAR_PLAY);
    }

    if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
    {
        HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if (HI_LITEPLAYER_STATE_PLAY == enPlayState || HI_LITEPLAYER_STATE_TPLAY == enPlayState
            || HI_LITEPLAYER_STATE_PREPARED == enPlayState)
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL, SKIN_PLAYBACK_BUTTON_PAUSE);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE, SKIN_PLAYBACK_BUTTON_PAUSE_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL, SKIN_PLAYBACK_BUTTON_PLAY);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE, SKIN_PLAYBACK_BUTTON_PLAY_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    s32Ret = HI_GV_Widget_Paint(PLAYBACK_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_PLAYBACK_ShowUI(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s_bShowUI = HI_TRUE;
    s32Ret = PDT_UI_PlAYBACK_Refresh();
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Refresh");

    s32Ret = HI_GV_Widget_Show(s_ActiveBtnHdl);
    s32Ret |= HI_GV_Widget_Active(s_ActiveBtnHdl);
    s32Ret |= HI_GV_Widget_Paint(PLAYBACK_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Show_Active_Paint");
}

HI_S32 PDT_UI_PLAYBACK_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    switch(wParam)
    {
        case PLAYBACK_EVENT_STATE_CHANGED:
            s_bIsTracking = HI_FALSE;
            /** no break */
        case PLAYBACK_EVENT_REFRESH_UI:
            s32Ret = PDT_UI_PlAYBACK_Refresh();
            break;

        case PLAYBACK_EVENT_PLAY_END:
            s32Ret = PDT_UI_COMM_PLAYBACK_HandlePlayEndEvent();
            s32Ret |= PDT_UI_PlAYBACK_Refresh();
            break;

        case PLAYBACK_EVENT_PLAY_ERROR:
            s32Ret = PDT_UI_COMM_PLAYBACK_HandlePlayErrorEvent();
            s32Ret |= PDT_UI_PlAYBACK_Refresh();
            break;
        default:
            MLOGE("unhandled event: %lx\n", (HI_UL)wParam);
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_PLAYBACK_DelOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_u32WinCallDialog = PLAYBACK_PAGE;

    s32Ret = HI_PLAYBACK_Pause();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_GV_Widget_SetTextByID(DIALOG_WINDOW_LABEL_ASK, STRING_DIALOG_Delete_ASK);
    if(s32Ret)
    {
        MLOGE("SetTextByID failed. %x\n", s32Ret);
    }
    HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);

    s32Ret = PDT_UI_WINMNG_StartWindow(DIALOG_WINDOW, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_VolumeOnClick(HI_VOID)
{
    s_s32Volume = (s_s32Volume / 25 + 1) % 5 * 25;
    return PDT_UI_COMM_PLAYBACK_SetVolume(s_AoHdl, PLAYBACK_BUTTON_VOLUME, s_s32Volume);
}


static HI_S32 PDT_UI_PLAYBACK_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case PLAYBACK_BUTTON_BACK:
            s_bRefresh = HI_FALSE;

            PDT_UI_COMM_PLAYBACK_PlayerDeinit();
            s32Ret = PDT_UI_WINMNG_FinishWindow(PLAYBACK_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;

        case PLAYBACK_BUTTON_VOLUME:
            s32Ret = PDT_UI_PLAYBACK_VolumeOnClick();
            break;

        case PLAYBACK_BUTTON_SPEED:
            s32Ret = PDT_UI_COMM_ChangePlaySpeed(PLAYBACK_BUTTON_SPEED);
            break;

        case PLAYBACK_BUTTON_DELETE:
            s32Ret = PDT_UI_PLAYBACK_DelOnClick();
            break;

        case PLAYBACK_BUTTON_PREV:
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayPrevFile();
            break;

        case PLAYBACK_BUTTON_PLAY:
            s32Ret = PDT_UI_COMM_ChangePlayStatus();
            break;

        case PLAYBACK_BUTTON_NEXT:
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayNextFile();
            break;

        default:
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s_bRefresh = HI_TRUE;

    PDT_UI_COMM_PLAYBACK_UpdateGrpIndex();

    s32Ret = PDT_UI_COMM_PLAYBACK_UpdateGrpInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_GetAVHdl(HI_PDT_WORKMODE_PLAYBACK, &s_AoHdl, &s_AoChnHdl, NULL, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_PLAYBACK_PlayerInit(HI_PDT_WORKMODE_PLAYBACK, s_AoHdl, s_AoChnHdl);
    s32Ret = HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_BASE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_PLAYBACK_GetVolume(PLAYBACK_BUTTON_VOLUME, &s_s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_SetText(PLAYBACK_BUTTON_SPEED, "x1");

    s32Ret |= HI_GV_Widget_Active(PLAYBACK_BUTTON_BACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Timer_Create(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI, PLAYBACK_HIDE_UI_TIMER_INTERVAL_MS);
    s32Ret |= HI_GV_Timer_Start(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);

    s32Ret |= HI_GV_Timer_Destroy(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_PLAYBACK_ResetPlayTime();

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bShowUI = HI_FALSE;

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_WinOnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(HI_FALSE == s_bIsTracking)
    {
        s32Ret = HI_GV_Timer_Reset(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    if (!s_bShowUI)
    {
        PDT_UI_PLAYBACK_ShowUI();
        return HIGV_PROC_GOON;
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_WinOnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_GV_Timer_Reset(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    if (!s_bShowUI)
    {
        PDT_UI_PLAYBACK_ShowUI();
        return HIGV_PROC_GOON;
    }

    if(HI_TRUE == s_bIsTracking)
    {
        return HI_SUCCESS;
    }

    HIGV_GESTURE_EVENT_S * pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 startX, startY, endX, endY;
    static HI_U32 u32LastTimeStamp = 0;

    if(u32LastTimeStamp == pstGestureEvent->gesture.fling.start.timeStamp)
    {
        return HIGV_PROC_GOON;
    }
    u32LastTimeStamp = pstGestureEvent->gesture.fling.start.timeStamp;

    startX = pstGestureEvent->gesture.fling.start.x;
    startY = pstGestureEvent->gesture.fling.start.y;
    endX   = pstGestureEvent->gesture.fling.end.x;
    endY   = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", startX, startY, endX, endY);

    if (abs(startX - endX) > abs(startY - endY)) /* left or right */
    {
        if (startX > endX)
        {
            MLOGD("fling left\n");
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayNextFile();
        }
        else
        {
            MLOGD("fling right\n");
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayPrevFile();
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_WinOnGestureScroll(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("[%d,%d] --> [%d,%d], Distance[%d,%d]\n",
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.start.x,   ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.start.y,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.end.x,     ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.end.y,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.distanceX, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.distanceY);

    HIGV_GESTURE_EVENT_S gestureEvent;
    HI_S32 startX, startY, endX, endY;
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;

    PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
    if (HI_FILEMNG_FILE_TYPE_RECORD != enFileType)
    {
        return HIGV_PROC_GOON;/*if file is pic, return.*/
    }

    memset(&gestureEvent, 0x0, sizeof(gestureEvent));
    memcpy(&gestureEvent, (HIGV_GESTURE_EVENT_S*)lParam, wParam);

    startX = gestureEvent.gesture.fling.start.x;
    startY = gestureEvent.gesture.fling.start.y;
    endX = gestureEvent.gesture.fling.end.x;
    endY = gestureEvent.gesture.fling.end.y;

    if (abs(startX - endX) < abs(startY - endY)) /* up or down */
    {
        if (startY > endY) /** up */
        {
            s_s32Volume = s_s32Volume + 100 * (startY - endY) / LCD_XML_LAYOUT_HEIGHT;
            s_s32Volume = s_s32Volume > 100 ? 100 : s_s32Volume;
        }
        else /** down */
        {
            s_s32Volume = s_s32Volume - 100 * (endY - startY) / LCD_XML_LAYOUT_HEIGHT;
            s_s32Volume= s_s32Volume < 0 ? 0 : s_s32Volume;
        }

        s32Ret = PDT_UI_COMM_PLAYBACK_SetVolume(s_AoHdl, PLAYBACK_BUTTON_VOLUME, s_s32Volume);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = PDT_UI_PlAYBACK_Refresh();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_PLAYBACK_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
{
    HI_U32 u32NextIndex = 0;
    HI_U32 u32CurIndex = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for (u32CurIndex = 0;  u32CurIndex < u32ArraySize; u32CurIndex++)
    {
        if (CurWidgetHdl == aWidgetHdl[u32CurIndex])
        {
            u32NextIndex = (u32CurIndex + 1) % u32ArraySize;
            s32Ret = HI_GV_Widget_Active( aWidgetHdl[u32NextIndex] );
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s_ActiveBtnHdl = aWidgetHdl[u32NextIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}

HI_S32 PDT_UI_PLAYBACK_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("WidgetHdl: %d, wParam: %lx, lParam = %lx\n",WidgetHdl, (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;

    s32Ret = HI_GV_Timer_Reset(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);

    if (!s_bShowUI)
    {
        PDT_UI_PLAYBACK_ShowUI();
        return HIGV_PROC_STOP;
    }

    HIGV_HANDLE aPlaybackHdlRec[] = {
        PLAYBACK_BUTTON_BACK,
        PLAYBACK_BUTTON_VOLUME,
        PLAYBACK_BUTTON_SPEED,
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_PLAY,
        PLAYBACK_BUTTON_NEXT
    };

    HIGV_HANDLE aPlaybackHdlPhoto[] = {
        PLAYBACK_BUTTON_BACK,
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_NEXT
    };

    if (PDT_UI_KEY_BACK == wParam)
    {
        s_bRefresh = HI_FALSE;
        PDT_UI_COMM_PLAYBACK_PlayerDeinit();
        s32Ret = PDT_UI_WINMNG_FinishWindow(PLAYBACK_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
        if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
        {
            s32Ret = PDT_UI_PLAYBACK_ActiveNextWidget(aPlaybackHdlRec,ARRAY_SIZE(aPlaybackHdlRec),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
        else if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
        {
            s32Ret = PDT_UI_PLAYBACK_ActiveNextWidget(aPlaybackHdlPhoto,ARRAY_SIZE(aPlaybackHdlPhoto),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }

    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_UI_PLAYBACK_BUTTON_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
    }

    return HIGV_PROC_STOP;
}

HI_S32 PDT_UI_PLAYBACK_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret;

    HI_GV_Widget_Active(WidgetHdl);
    s_ActiveBtnHdl = WidgetHdl;

    s32Ret = PDT_UI_PLAYBACK_BUTTON_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_TrackbarOnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HI_APPCOMM_CHECK_POINTER(pstTouchEvent, HIGV_PROC_GOON);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed = HI_LITEPLAYER_PLAY_SPEED_BUTT;

    switch (pstTouchEvent->last.type)
    {
        case HIGV_TOUCH_END:
            if(s_bIsTracking)
            {
                s32Ret = HI_GV_Timer_Start(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
                s_bIsTracking = HI_FALSE;

                s32Ret = HI_PLAYBACK_GetSpeed(&enSpeed);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                if(HI_LITEPLAYER_PLAY_SPEED_BASE != enSpeed)
                {
                    return HIGV_PROC_GOON;
                }

                s32Ret = PDT_UI_COMM_PLAYBACK_Seek();
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        case HIGV_TOUCH_START:
            s_bIsTracking = HI_TRUE;
            s32Ret = HI_GV_Timer_Stop(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
            break;

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_TrackbarOnMouseOut(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("mouse out\n");
    HIGV_TOUCH_EVENT_S touchEvent;
    memset(&touchEvent, 0x0, sizeof(touchEvent));
    touchEvent.last.id = 0;
    touchEvent.last.type = HIGV_TOUCH_END;
    HI_GV_Msg_SendAsyncWithData(PLAYBACK_TRACKBAR_PLAY, HIGV_MSG_TOUCH,
            &touchEvent, sizeof(HIGV_TOUCH_EVENT_S));

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

