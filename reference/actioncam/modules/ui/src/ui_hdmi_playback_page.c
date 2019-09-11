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
static HI_BOOL s_bShowUI = HI_TRUE;
static HI_HANDLE s_AoChnHdl = HI_INVALID_HANDLE;
static HI_HANDLE s_AoHdl = HI_INVALID_HANDLE;
extern HIGV_HANDLE s_u32WinCallDialog;
static HIGV_HANDLE s_ActiveBtnHdl = HDMI_PLAYBACK_BACK;


HI_S32 PDT_HDMI_PlAYBACK_Refresh(HI_VOID)
{
    if(HI_FALSE == s_bRefresh)
    {
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;
    HIGV_HANDLE aPlaybackBtnHdl[] =
    {
        HDMI_PLAYBACK_BACK,
        HDMI_PLAYBACK_VOLUME,
        HDMI_PLAYBACK_SPEED,
        HDMI_PLAYBACK_DELETE,
        HDMI_PLAYBACK_PREV,
        HDMI_PLAYBACK_PLAY,
        HDMI_PLAYBACK_NEXT,
        HDMI_PLAYBACK_SPEED,
    };

    PDT_UI_COMM_PLAYBACK_SetPlayTime(HDMI_PLAYBACK_INFO);

    if (!s_bShowUI)
    {
        for (i=0; i<ARRAY_SIZE(aPlaybackBtnHdl); i++)
        {
            s32Ret = HI_GV_Widget_Hide(aPlaybackBtnHdl[i]);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s32Ret = HI_GV_Widget_Paint(HDMI_PLAYBACK_PAGE, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_SUCCESS;
    }

    s32Ret = HI_GV_Widget_Show(HDMI_PLAYBACK_BACK);
    s32Ret |= HI_GV_Widget_Show(HDMI_PLAYBACK_DELETE);
    s32Ret |= HI_GV_Widget_Show(HDMI_PLAYBACK_PREV);
    s32Ret |= HI_GV_Widget_Show(HDMI_PLAYBACK_NEXT);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
    if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
    {
        s32Ret = HI_GV_Widget_Hide(HDMI_PLAYBACK_VOLUME);
        s32Ret |= HI_GV_Widget_Hide(HDMI_PLAYBACK_PLAY);
        s32Ret |= HI_GV_Widget_Hide(HDMI_PLAYBACK_SPEED);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Show(HDMI_PLAYBACK_VOLUME);
        s32Ret |= HI_GV_Widget_Show(HDMI_PLAYBACK_SPEED);
        s32Ret |= HI_GV_Widget_Show(HDMI_PLAYBACK_PLAY);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* if no widget is active, then active back button */
    for(i = 0; i < ARRAY_SIZE(aPlaybackBtnHdl); i++)
    {
        if(HI_GV_Widget_IsActive(aPlaybackBtnHdl[i]))
        {
            break;
        }
    }
    if(i >= ARRAY_SIZE(aPlaybackBtnHdl))
    {
        s32Ret = HI_GV_Widget_Active(HDMI_PLAYBACK_BACK);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
    {
        HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if (HI_LITEPLAYER_STATE_PLAY == enPlayState || HI_LITEPLAYER_STATE_TPLAY == enPlayState
            || HI_LITEPLAYER_STATE_PREPARED == enPlayState)
        {
            s32Ret = HI_GV_Widget_SetSkin(HDMI_PLAYBACK_PLAY, HIGV_SKIN_NORMAL, SKIN_HDMI_PLAYBACK_PAUSE_NORMAL);
            s32Ret |= HI_GV_Widget_SetSkin(HDMI_PLAYBACK_PLAY, HIGV_SKIN_ACITVE, SKIN_HDMI_PLAYBACK_PAUSE_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            s32Ret = HI_GV_Widget_SetSkin(HDMI_PLAYBACK_PLAY, HIGV_SKIN_NORMAL, SKIN_HDMI_PLAYBACK_PLAY_NORMAL);
            s32Ret |= HI_GV_Widget_SetSkin(HDMI_PLAYBACK_PLAY, HIGV_SKIN_ACITVE, SKIN_HDMI_PLAYBACK_PLAY_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    s32Ret = HI_GV_Widget_Paint(HDMI_PLAYBACK_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_HDMI_PLAYBACK_ShowUI(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s_bShowUI = HI_TRUE;
    s32Ret = PDT_HDMI_PlAYBACK_Refresh();
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Refresh");

    s32Ret = HI_GV_Widget_Show(s_ActiveBtnHdl);
    s32Ret |= HI_GV_Widget_Active(s_ActiveBtnHdl);
    s32Ret |= HI_GV_Widget_Paint(HDMI_PLAYBACK_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Show_Active_Paint");
}

HI_S32 PDT_HDMI_PLAYBACK_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    switch(wParam)
    {
        case PLAYBACK_EVENT_STATE_CHANGED:
        case PLAYBACK_EVENT_REFRESH_UI:
            s32Ret = PDT_HDMI_PlAYBACK_Refresh();
            break;

        case PLAYBACK_EVENT_PLAY_END:
            s32Ret = PDT_UI_COMM_PLAYBACK_HandlePlayEndEvent();
            s32Ret |= PDT_HDMI_PlAYBACK_Refresh();
            break;

        case PLAYBACK_EVENT_PLAY_ERROR:
            s32Ret = PDT_UI_COMM_PLAYBACK_HandlePlayErrorEvent();
            s32Ret |= PDT_HDMI_PlAYBACK_Refresh();
            break;
        default:
            MLOGE("unhandled event: %lx\n", (HI_UL)wParam);
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    return HIGV_PROC_GOON;
}

static HI_S32 PDT_HDMI_PLAYBACK_DelOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_u32WinCallDialog = HDMI_PLAYBACK_PAGE;

    s32Ret = HI_PLAYBACK_Pause();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_DIALOG_WINDOW, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 PDT_HDMI_PLAYBACK_VolumeOnClick(HI_VOID)
{
    s_s32Volume = (s_s32Volume / 25 + 1) % 5 * 25;
    return PDT_UI_COMM_PLAYBACK_SetVolume(s_AoHdl, HDMI_PLAYBACK_VOLUME, s_s32Volume);
}

static HI_S32 PDT_HDMI_PLAYBACK_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case HDMI_PLAYBACK_BACK:
            s_bRefresh = HI_FALSE;

            PDT_UI_COMM_PLAYBACK_PlayerDeinit();
            s32Ret = PDT_UI_WINMNG_FinishWindow(HDMI_PLAYBACK_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;

        case HDMI_PLAYBACK_VOLUME:
            s32Ret = PDT_HDMI_PLAYBACK_VolumeOnClick();
            break;

        case HDMI_PLAYBACK_SPEED:
            s32Ret = PDT_UI_COMM_ChangePlaySpeed(HDMI_PLAYBACK_SPEED);
            break;

        case HDMI_PLAYBACK_DELETE:
            s32Ret = PDT_HDMI_PLAYBACK_DelOnClick();
            break;

        case HDMI_PLAYBACK_PREV:
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayPrevFile();
            break;

        case HDMI_PLAYBACK_PLAY:
            s32Ret = PDT_UI_COMM_ChangePlayStatus();
            break;

        case HDMI_PLAYBACK_NEXT:
            s32Ret = PDT_UI_COMM_PLAYBACK_PlayNextFile();
            break;

        default:
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_HDMI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_PLAYBACK_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s_bRefresh = HI_TRUE;

    PDT_UI_COMM_PLAYBACK_UpdateGrpIndex();

    s32Ret = PDT_UI_COMM_PLAYBACK_UpdateGrpInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_GetAVHdl(HI_PDT_WORKMODE_HDMI_PLAYBACK, &s_AoHdl, &s_AoChnHdl, NULL, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_PLAYBACK_PlayerInit(HI_PDT_WORKMODE_HDMI_PLAYBACK, s_AoHdl, s_AoChnHdl);
    s32Ret = HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_BASE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_PLAYBACK_GetVolume(HDMI_PLAYBACK_VOLUME, &s_s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_SetText(HDMI_PLAYBACK_SPEED, "x1");
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_GV_Widget_Active(HDMI_PLAYBACK_PREV);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_HDMI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Timer_Create(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI, PLAYBACK_HIDE_UI_TIMER_INTERVAL_MS);
    s32Ret |= HI_GV_Timer_Start(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_PLAYBACK_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Timer_Destroy(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_PLAYBACK_ResetPlayTime();

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_PLAYBACK_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bShowUI = HI_FALSE;

    s32Ret = PDT_HDMI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_HDMI_PLAYBACK_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
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
static HI_S32 PDT_HDMI_PLAYBACK_ActiveLastWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
{
    HI_U32 u32NextIndex = 0;
    HI_U32 u32CurIndex = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for (u32CurIndex = 0;  u32CurIndex < u32ArraySize; u32CurIndex++)
    {
        if (CurWidgetHdl == aWidgetHdl[u32CurIndex])
        {
            u32NextIndex = (u32CurIndex + u32ArraySize - 1) % u32ArraySize;
            s32Ret = HI_GV_Widget_Active( aWidgetHdl[u32NextIndex] );
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s_ActiveBtnHdl = aWidgetHdl[u32NextIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}


HI_S32 PDT_HDMI_PLAYBACK_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("WidgetHdl: %d, wParam: %lx, lParam = %lx\n",WidgetHdl, (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;

    s32Ret = HI_GV_Timer_Reset(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);

    if (!s_bShowUI)
    {
        PDT_HDMI_PLAYBACK_ShowUI();
        return HIGV_PROC_STOP;
    }

    HIGV_HANDLE aPlaybackHdlRec[] =
    {
        HDMI_PLAYBACK_BACK,
        HDMI_PLAYBACK_VOLUME,
        HDMI_PLAYBACK_SPEED,
        HDMI_PLAYBACK_DELETE,
        HDMI_PLAYBACK_PREV,
        HDMI_PLAYBACK_PLAY,
        HDMI_PLAYBACK_NEXT
    };

    HIGV_HANDLE aPlaybackHdlPhoto[] =
    {
        HDMI_PLAYBACK_BACK,
        HDMI_PLAYBACK_DELETE,
        HDMI_PLAYBACK_PREV,
        HDMI_PLAYBACK_NEXT
    };

    if (PDT_UI_KEY_BACK == wParam)
    {
        s_bRefresh = HI_FALSE;
        PDT_UI_COMM_PLAYBACK_PlayerDeinit();
        s32Ret = PDT_UI_WINMNG_FinishWindow(HDMI_PLAYBACK_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
        if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
        {
            s32Ret = PDT_HDMI_PLAYBACK_ActiveNextWidget(aPlaybackHdlRec,ARRAY_SIZE(aPlaybackHdlRec),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
        else if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
        {
            s32Ret = PDT_HDMI_PLAYBACK_ActiveNextWidget(aPlaybackHdlPhoto,ARRAY_SIZE(aPlaybackHdlPhoto),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_HDMI_PLAYBACK_BUTTON_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
    }

    return HIGV_PROC_STOP;
}

HI_S32 PDT_HDMI_PLAYBACK_WinOnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MLOGD("WidgetHdl: %d, wParam: %lx, lParam = %lx\n",WidgetHdl, (HI_UL)wParam, (HI_UL)lParam);
    s32Ret = HI_GV_Timer_Reset(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    if (!s_bShowUI)
    {
        PDT_HDMI_PLAYBACK_ShowUI();
        return HIGV_PROC_GOON;
    }

    s32Ret = PDT_UI_COMM_ChangePlayStatus();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_PLAYBACK_WinOnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S * pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 startX, startY, endX, endY;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Reset(HDMI_PLAYBACK_PAGE, TIMER_HDMI_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    HI_FILEMNG_FILE_TYPE_E enFileType = HI_FILEMNG_FILE_TYPE_BUTT;
    MLOGD("WidgetHdl: %d, wParam: %lx, lParam = %lx,s_ActiveBtnHdl %d\n",WidgetHdl, (HI_UL)wParam, (HI_UL)lParam,s_ActiveBtnHdl);

    if (!s_bShowUI)
    {
        PDT_HDMI_PLAYBACK_ShowUI();
        return HIGV_PROC_GOON;
    }

    HIGV_HANDLE aPlaybackHdlRec[] =
      {
          HDMI_PLAYBACK_BACK,
          HDMI_PLAYBACK_VOLUME,
          HDMI_PLAYBACK_SPEED,
          HDMI_PLAYBACK_DELETE,
          HDMI_PLAYBACK_PREV,
          HDMI_PLAYBACK_PLAY,
          HDMI_PLAYBACK_NEXT
      };

      HIGV_HANDLE aPlaybackHdlPhoto[] =
      {
          HDMI_PLAYBACK_BACK,
          HDMI_PLAYBACK_DELETE,
          HDMI_PLAYBACK_PREV,
          HDMI_PLAYBACK_NEXT
      };

    startX = pstGestureEvent->gesture.fling.start.x;
    startY = pstGestureEvent->gesture.fling.start.y;
    endX   = pstGestureEvent->gesture.fling.end.x;
    endY   = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", startX, startY, endX, endY);

    if (abs(startX - endX) > abs(startY - endY)) /* left or right */
    {
        PDT_UI_COMM_PLAYBACK_GetFileType(&enFileType);
        if (startX > endX)
        {
            MLOGD("fling left\n");
            if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
            {
                s32Ret = PDT_HDMI_PLAYBACK_ActiveLastWidget(aPlaybackHdlRec,ARRAY_SIZE(aPlaybackHdlRec),s_ActiveBtnHdl);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
            }
            else if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
            {
                s32Ret = PDT_HDMI_PLAYBACK_ActiveLastWidget(aPlaybackHdlPhoto,ARRAY_SIZE(aPlaybackHdlPhoto),s_ActiveBtnHdl);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
            }
        }
        else
        {
            MLOGD("fling right\n");
            if (HI_FILEMNG_FILE_TYPE_RECORD == enFileType)
            {
                s32Ret = PDT_HDMI_PLAYBACK_ActiveNextWidget(aPlaybackHdlRec,ARRAY_SIZE(aPlaybackHdlRec),s_ActiveBtnHdl);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
            }
            else if (HI_FILEMNG_FILE_TYPE_PHOTO == enFileType)
            {
                s32Ret = PDT_HDMI_PLAYBACK_ActiveNextWidget(aPlaybackHdlPhoto,ARRAY_SIZE(aPlaybackHdlPhoto),s_ActiveBtnHdl);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
            }

        }
    }

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

