/**
 * @file      ui_prompt.c
 * @brief     ui prompt windows. Date and time, SD format, Reset, USB choices...
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/5/28
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ui_common.h"
#include "ui_common_playback.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

extern HIGV_HANDLE s_u32WinCallDialog;
static HIGV_HANDLE s_ActiveDiaHdl = HDMI_DIALOG_WINDOW_BUTTON_CANCEL;

HI_VOID PDT_UI_HDMI_DIALOG_OnClick(HIGV_HANDLE WidgetHdl)
{
    if (HDMI_DIALOG_WINDOW_BUTTON_CONFIRM == WidgetHdl)
    {
        switch (s_u32WinCallDialog)
        {
            case HDMI_FILELIST:
                PDT_HDMI_FILELIST_DIALOG_Delete();
                break;

            case HDMI_PLAYBACK_PAGE:
                PDT_UI_COMM_PLAYBACK_DeleteFile(HDMI_DIALOG_WINDOW, HDMI_PLAYBACK_PAGE);
                break;
        }
    }

    PDT_UI_COMM_FinishWindow(HDMI_DIALOG_WINDOW);
}
static HI_S32 PDT_HDMI_DIALOG_ActiveLastWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
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
            s_ActiveDiaHdl = aWidgetHdl[u32NextIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}
static HI_S32 PDT_HDMI_DIALOG_ActiveNextWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
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
            s_ActiveDiaHdl = aWidgetHdl[u32NextIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}

HI_S32 PDT_HDMI_DIALOG_WinOnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S * pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 startX, startY, endX, endY;
    HI_S32 s32Ret = HI_SUCCESS;

    HIGV_HANDLE aDiaHdl[] =
    {
        HDMI_DIALOG_WINDOW_BUTTON_CANCEL,
        HDMI_DIALOG_WINDOW_BUTTON_CONFIRM,
    };

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
            s32Ret = PDT_HDMI_DIALOG_ActiveLastWidget(aDiaHdl,ARRAY_SIZE(aDiaHdl),s_ActiveDiaHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
        else
        {
            MLOGD("fling right\n");
            s32Ret = PDT_HDMI_DIALOG_ActiveNextWidget(aDiaHdl,ARRAY_SIZE(aDiaHdl),s_ActiveDiaHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);

        }
    }
    return s32Ret;
}

HI_S32 PDT_UI_HDMI_DIALOG_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        PDT_UI_COMM_FinishWindow(HDMI_DIALOG_WINDOW);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (HDMI_DIALOG_WINDOW_BUTTON_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(HDMI_DIALOG_WINDOW_BUTTON_CONFIRM);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(HDMI_DIALOG_WINDOW_BUTTON_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(HDMI_DIALOG_WINDOW_BUTTON_CANCEL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        PDT_UI_HDMI_DIALOG_OnClick(WidgetHdl);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HDMI_ALARM_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    if ((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) || (PDT_UI_KEY_ENTER == wParam))
    {
        PDT_UI_COMM_FinishWindow(HDMI_ALARM_WINDOW);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_HDMI_ALARM_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    PDT_UI_COMM_FinishWindow(HDMI_ALARM_WINDOW);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

