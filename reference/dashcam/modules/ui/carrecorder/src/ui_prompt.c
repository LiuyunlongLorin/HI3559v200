/**
* @file    ui_prompt.c
* @brief   ui prompt windows. Date and time, SD format, Reset, USB choices...
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/12
* @version   1.0

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ui_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define UI_START_YEAR (2018)
#define UI_MAX_YEAR (2037)
#define UI_DEVINFO_LENGTH (128)

static PDT_UI_DATETIME_S s_stUIDateTime = {};
static HI_S32 s_u32AlarmPoweroffTimerCount = 0;
static HI_BOOL s_bFastFormat = HI_TRUE;

HI_S32 PDT_UI_ALARM_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if ((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) ||
        (PDT_UI_KEY_OK == wParam) || (PDT_UI_HOME_KEY_WIFI == wParam) ||
        (PDT_UI_KEY_EXPAND == wParam))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALARM_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    if (TIMER_REFRESH_ALARM_POWEROFF == wParam)
    {
        s_u32AlarmPoweroffTimerCount++;

        if (s_u32AlarmPoweroffTimerCount == 10)
        {
            s_u32AlarmPoweroffTimerCount = 0;

            s32Ret = HI_GV_Timer_Stop(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
            s32Ret |= HI_GV_Timer_Destroy(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_STOP, "stop and destroy timer");

            stMessage.what = HI_EVENT_STATEMNG_POWEROFF;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_STOP, "Send poweroff msg");
            return HIGV_PROC_GOON;
        }

        s32Ret = HI_GV_Timer_Reset(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HIGV_PROC_GOON;
}

static inline HI_BOOL PDT_UI_TIME_SET_IsLeapYear(HI_U32 u32Year)
{
    if (0 == u32Year%400 || (0 == u32Year%4 && 0 != u32Year%100))
    {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_U32 PDT_UI_TIME_SET_GetDaysOfMonth(HI_U32 u32Year, HI_U32 u32Month)
{
    HI_U32 s_au32DaysOfMonth [ ] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (u32Month > 12)
    {
        MLOGE("mont is > 12, return 31\n" );
        return 31;
    }
    if (PDT_UI_TIME_SET_IsLeapYear(u32Year) && 2 == u32Month)
    {
        return s_au32DaysOfMonth[u32Month-1] + 1;
    }
    else
    {
        return s_au32DaysOfMonth[u32Month-1];
    }
}

static HI_VOID PDT_UI_TIME_SET_NumberToString(HI_U32 u32Number, HI_CHAR * pszBuf, HI_U32 u32Size)
{
    if (NULL == pszBuf)
    {
        MLOGE("pszBuf is NULL");
        return;
    }

    snprintf(pszBuf,u32Size,"%02u",u32Number);

}

HI_S32 PDT_UI_TIME_SET_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32DayOfMonth = 0;
    HI_CHAR aszText[5][10] = {};
    HI_HANDLE aTimeBtnsHdl [ ] =
    {
        TIME_SET_YEAR,
        TIME_SET_MONTH,
        TIME_SET_DAY,
        TIME_SET_HOUR,
        TIME_SET_MINUTE
    };

    u32DayOfMonth = PDT_UI_TIME_SET_GetDaysOfMonth(s_stUIDateTime.year, s_stUIDateTime.month);
    if (s_stUIDateTime.day > u32DayOfMonth)
    {
        s_stUIDateTime.day = u32DayOfMonth;
    }

    PDT_UI_TIME_SET_NumberToString(s_stUIDateTime.year, aszText[0], sizeof(aszText[0]));
    PDT_UI_TIME_SET_NumberToString(s_stUIDateTime.month, aszText[1], sizeof(aszText[0]));
    PDT_UI_TIME_SET_NumberToString(s_stUIDateTime.day, aszText[2], sizeof(aszText[0]));
    PDT_UI_TIME_SET_NumberToString(s_stUIDateTime.hour, aszText[3], sizeof(aszText[0]));
    PDT_UI_TIME_SET_NumberToString(s_stUIDateTime.minute, aszText[4], sizeof(aszText[0]));

    HI_S32 i;
    for (i=0; i< UI_ARRAY_SIZE(aTimeBtnsHdl); i++)
    {
        s32Ret = HI_GV_Widget_SetText(aTimeBtnsHdl[i], aszText[i]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        s32Ret = HI_GV_Widget_Paint(aTimeBtnsHdl[i], NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_TIME_SET_SubOnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case TIME_SET_YEAR:
            if (UI_START_YEAR == s_stUIDateTime.year)
            {
                s_stUIDateTime.year = UI_MAX_YEAR;
            }
            else
            {
                s_stUIDateTime.year--;
            }
            break;

        case TIME_SET_MONTH:
            if (1 == s_stUIDateTime.month)
            {
                s_stUIDateTime.month = 12;
            }
            else
            {
                s_stUIDateTime.month--;
            }
            break;

        case TIME_SET_DAY:
            if (1 == s_stUIDateTime.day)
            {
                s_stUIDateTime.day = PDT_UI_TIME_SET_GetDaysOfMonth(s_stUIDateTime.year, s_stUIDateTime.month);
            }
            else
            {
                s_stUIDateTime.day--;
            }
            break;

        case TIME_SET_HOUR:
            if (0 == s_stUIDateTime.hour)
            {
                s_stUIDateTime.hour = 23;
            }
            else
            {
                s_stUIDateTime.hour--;
            }
            break;

        case TIME_SET_MINUTE:
            if (0 == s_stUIDateTime.minute)
            {
                s_stUIDateTime.minute = 59;
            }
            else
            {
                s_stUIDateTime.minute--;
            }
            break;

        default:
            break;

    }

    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_UI_TIME_SET_Refresh");
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_TIME_SET_AddOnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case TIME_SET_YEAR:
            s_stUIDateTime.year++;
            if (s_stUIDateTime.year > UI_MAX_YEAR)
            {
                s_stUIDateTime.year = UI_START_YEAR;
            }
            break;

        case TIME_SET_MONTH:
            s_stUIDateTime.month++;
            if (s_stUIDateTime.month > 12)
            {
                s_stUIDateTime.month = 1;
            }
            break;

        case TIME_SET_DAY:
            s_stUIDateTime.day++;
            if (s_stUIDateTime.day > PDT_UI_TIME_SET_GetDaysOfMonth(s_stUIDateTime.year,
                s_stUIDateTime.month))
            {
                s_stUIDateTime.day = 1;
            }
            break;

        case TIME_SET_HOUR:
            s_stUIDateTime.hour++;
            if (s_stUIDateTime.hour > 23)
            {
                s_stUIDateTime.hour = 0;
            }
            break;

        case TIME_SET_MINUTE:
            s_stUIDateTime.minute++;
            if (s_stUIDateTime.minute > 59)
            {
                s_stUIDateTime.minute = 0;
            }
            break;

        case TIME_SET_CANCEL:
            s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_FinishWindow");
            return HI_SUCCESS;

        case TIME_SET_CONFIRM:
            s32Ret = PDT_UI_COMM_SetDateTime(&s_stUIDateTime);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_COMM_SetDateTime");
            s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_FinishWindow");
            return HI_SUCCESS;

        default:
            break;

    }

    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_UI_TIME_SET_Refresh");
    return HI_SUCCESS;
}

HI_S32 PDT_UI_TIME_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_GetDateTime(&s_stUIDateTime);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"GetDateTime");

    if (s_stUIDateTime.year < UI_START_YEAR)
    {
        s_stUIDateTime.year = UI_START_YEAR;
    }

    s32Ret = HI_GV_Widget_Active(TIME_SET_YEAR);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
    PDT_UI_TIME_SET_Refresh();
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE aTimeBtnsHdl [ ] =
    {
        TIME_SET_YEAR,
        TIME_SET_MONTH,
        TIME_SET_DAY,
        TIME_SET_HOUR,
        TIME_SET_MINUTE,
        TIME_SET_CANCEL,
        TIME_SET_CONFIRM
    };

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_TIME_SET_SubOnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_TIME_SET_SubOnClick");
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_COMM_ActiveNextWidget(aTimeBtnsHdl, UI_ARRAY_SIZE(aTimeBtnsHdl), WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "ActiveNextWidget");
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_COMM_ActivePreviousWidget(aTimeBtnsHdl, UI_ARRAY_SIZE(aTimeBtnsHdl), WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "ActiveNextWidget");
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_TIME_SET_AddOnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_TIME_SET_AddOnClick");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SD_FORMAT_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bFastFormat = HI_TRUE;

    s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_NORMAL,
         SKIN_DIALOG_FORMAT_CHOSED_NORMAL);
    s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_ACITVE,
         SKIN_DIALOG_FORMAT_CHOSED_ACTIVE);

    s32Ret = HI_GV_Widget_Active(FORMAT_SD_CANCEL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SD_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_UI_WINMNG_FinishWindow");
    s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PDT_UI_WINMNG_FinishWindow");

    if(HI_SUCCESS != pstEvent->s32Result)
    {
        PDT_UI_COMM_ShowAlarm(STRING_FORMAT_FAILED);
    }

    return HI_SUCCESS;
}

HI_VOID PDT_UI_SD_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};

    if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
    {
        s_bFastFormat = s_bFastFormat ? HI_FALSE : HI_TRUE;

        s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_NORMAL,
            s_bFastFormat ? SKIN_DIALOG_FORMAT_CHOSED_NORMAL
            : SKIN_DIALOG_FORMAT_NOT_CHOSED_NORMAL);
        s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_ACITVE,
            s_bFastFormat ? SKIN_DIALOG_FORMAT_CHOSED_ACTIVE
            : SKIN_DIALOG_FORMAT_NOT_CHOSED_ACTIVE);

        s32Ret = HI_GV_Widget_Paint(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, NULL);
        return;
    }

    if (FORMAT_SD_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (FORMAT_SD_CONFIRM == WidgetHdl)
    {

        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"StartWindow");

        stMsg.what = HI_EVENT_STATEMNG_SD_FORMAT;
        stMsg.arg1 = s_bFastFormat ? HI_FSTOOL_FORMAT_MODE_ADVANCED : HI_FSTOOL_FORMAT_MODE_DISCARD;
        s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, PDT_UI_SD_OnReceiveMsgResult);
        if(s32Ret)
        {
            MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"FinishWindow");
        }
    }
}

HI_S32 PDT_UI_SD_FORMAT_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (FORMAT_SD_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CONFIRM);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (FORMAT_SD_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CANCEL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        if (FORMAT_SD_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (FORMAT_SD_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CONFIRM);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        PDT_UI_SD_OnClick(WidgetHdl);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_RESET_FACTORY_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Active(RESET_FACTORY_CANCEL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_RESET_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (RESET_FACTORY_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(RESET_FACTORY_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"PDT_UI_WINMNG_FinishWindow");
    }
    else if (RESET_FACTORY_CONFIRM == WidgetHdl)
    {
        HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
        if (stWorkModeState.bStateMngInProgress)
        {
            MLOGE("In progress, give up reset\n");
            return HI_FAILURE;
        }

        HI_MESSAGE_S stMessage;
        memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
        stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "HI_PDT_STATEMNG_SendMessage");
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_RESET_FACTORY_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(RESET_FACTORY_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (PDT_UI_KEY_MENU == wParam || PDT_UI_KEY_EXPAND == wParam )
    {
        if (RESET_FACTORY_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(RESET_FACTORY_CONFIRM);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (RESET_FACTORY_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(RESET_FACTORY_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_RESET_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_RESET_OnClick");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_USB_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Active(USB_CANCEL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_USB_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(USB_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (USB_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_STORAGE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (USB_STORAGE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_UVC);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (USB_UVC == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        if (USB_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_UVC);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (USB_STORAGE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (USB_UVC == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(USB_STORAGE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_PDT_DEV_INFO_S stDevInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szTitle[32] = {};
    HI_CHAR szRow[64] = {};
    HI_CHAR szBuffer[512] = {};

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON, "HI_PDT_PARAM_GetCommParam");

    PDT_UI_COMM_GetStringByID(STRING_MODEL, szTitle, sizeof(szTitle));
    snprintf(szBuffer, sizeof(szBuffer), "%s%s\n", szTitle, stDevInfo.szModel);

    memset(szTitle, 0, sizeof(szTitle));
    memset(szRow, 0, sizeof(szRow));
    PDT_UI_COMM_GetStringByID(STRING_SOFTWARE, szTitle, sizeof(szTitle));
    snprintf(szRow, sizeof(szRow), "%s%s\n", szTitle, stDevInfo.szSoftVersion);
    strncat(szBuffer, szRow,  sizeof(szBuffer) - strlen(szBuffer) - 1);

    memset(szTitle, 0, sizeof(szTitle));
    memset(szRow, 0, sizeof(szRow));
    PDT_UI_COMM_GetStringByID(STRING_KERNEL, szTitle, sizeof(szTitle));
    snprintf(szRow, sizeof(szRow), "%s%s\n", szTitle, stDevInfo.szSysVersion);
    strncat(szBuffer, szRow,  sizeof(szBuffer) - strlen(szBuffer) - 1);

#if defined(CONFIG_WIFI_ON)
    memset(szTitle, 0, sizeof(szTitle));
    memset(szRow, 0, sizeof(szRow));
    HI_HAL_WIFI_APMODE_CFG_S stWifiCfg = {};
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stWifiCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON, "HI_PDT_PARAM_GetCommParam");

    memset(szTitle, 0, sizeof(szTitle));
    memset(szRow, 0, sizeof(szRow));
    PDT_UI_COMM_GetStringByID(STRING_WIFI_SSID, szTitle, sizeof(szTitle));
    snprintf(szRow, sizeof(szRow), "%s%s\n", szTitle, stWifiCfg.stCfg.szWiFiSSID);
    strncat(szBuffer, szRow,  sizeof(szBuffer) - strlen(szBuffer) - 1);
#endif
    s32Ret = HI_GV_Widget_SetText(ABOUT_SYS_INFO, szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ABOUT_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PlayPark_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("show select PlayPark_page \n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szBuffer[HI_APPCOMM_COMM_STR_LEN] = {};
    HI_CHAR szTmpBuffer[HI_APPCOMM_COMM_STR_LEN] = {};
    HI_S32  s32CollisionCnt = 0;

    memset(szBuffer, 0, HI_APPCOMM_COMM_STR_LEN);
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_COMM_GetStringByID(STRING_FILE_CNT_PARKING, szBuffer, HI_APPCOMM_COMM_STR_LEN);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    snprintf(szTmpBuffer, HI_APPCOMM_COMM_STR_LEN, "%s%d", szBuffer, s32CollisionCnt);
    s32Ret = HI_GV_Widget_SetText(PLAYPARK_LABEL_COUNT, szTmpBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if (s32CollisionCnt == 0)
    {
        memset(szTmpBuffer, 0, HI_APPCOMM_COMM_STR_LEN);
        s32Ret = PDT_UI_COMM_GetStringByID(STRING_PLAYBACK_FILE_ERR, szTmpBuffer, HI_APPCOMM_COMM_STR_LEN);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_GV_Widget_SetText(PLAYPARK_LABEL_ASK, szTmpBuffer);
        s32Ret |= HI_GV_Widget_Hide(PLAYPARK_BUTTON_CONFIRM);
        s32Ret |= HI_GV_Widget_Active(PLAYPARK_BUTTON_CANCEL);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Active(PLAYPARK_BUTTON_CONFIRM);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
    }

    return HIGV_PROC_GOON;

}

HI_S32 PDT_UI_PlayPark_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32CollisionCnt = 0;

    if ((PDT_UI_KEY_EXPAND != wParam) && (PDT_UI_KEY_MENU != wParam))
    {
        s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE,HI_TRUE);
        s32Ret |= PDT_UI_WINMNG_FinishWindow(PLAYPARK_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else if (PDT_UI_KEY_MENU == wParam || PDT_UI_KEY_EXPAND == wParam)
    {
        if (PLAYPARK_BUTTON_CANCEL == WidgetHdl)
        {
            if (HI_GV_Widget_IsShow(PLAYPARK_BUTTON_CONFIRM))
            {
                s32Ret = HI_GV_Widget_Active(PLAYPARK_BUTTON_CONFIRM);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
            }
        }
        else if (PLAYPARK_BUTTON_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(PLAYPARK_BUTTON_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        if (PLAYPARK_BUTTON_CONFIRM == WidgetHdl)
        {

            s32Ret = PDT_UI_WINMNG_StartWindow(PLAYBACK_PAGE,HI_TRUE);
            s32Ret |= PDT_UI_WINMNG_FinishWindow(PLAYPARK_WINDOW);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        }
        else if (PLAYPARK_BUTTON_CANCEL == WidgetHdl)
        {
            s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE,HI_TRUE);
            s32Ret |= PDT_UI_WINMNG_FinishWindow(PLAYPARK_WINDOW);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_USB_STATE_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {};

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    if(HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode)
    {
        s32Ret = HI_GV_Widget_SetTextByID(USB_STATE_LABEL, STRING_UVC);
    }
    else if(HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
    {
        s32Ret = HI_GV_Widget_SetTextByID(USB_STATE_LABEL, STRING_USB_STORAGE);
    }
    else
    {
        MLOGE("It's not usb mode\n");
        s32Ret = PDT_UI_WINMNG_FinishWindow(USB_STATE_WINDOW);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SD_UPGRADE_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_GV_Widget_Active(SD_UPGRADE_CANCEL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");

    return HIGV_PROC_GOON;
}

static HI_S32 UI_SD_UPGRADE_BACK(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};

    s32Ret = PDT_UI_WINMNG_FinishWindow(SD_UPGRADE_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_STATEMNG_GetState");

    if (!stWorkModeState.bRunning)
    {
        s32Ret = PDT_UI_COMM_CheckSd();
        if (s32Ret != HI_SUCCESS)
        {
            MLOGE(" SD state is not normal \n");
            return HI_SUCCESS;
        }

        if (PDT_UI_COMM_isMdReady())
        {
            stMsg.what = HI_EVENT_STATEMNG_START;
            stMsg.arg2 = stWorkModeState.enWorkMode;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_SD_UPGRADE_EventProc(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};

    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (HI_SUCCESS != pstEvent->s32Result)
    {
        MLOGE("Event result is failed \n");
        return HI_FAILURE;
    }

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_STOP:
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMsg.arg2 = HI_PDT_WORKMODE_UPGRADE;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
            if (s32Ret)
            {
                MLOGE("send message failed !\n");
                PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                UI_SD_UPGRADE_BACK();
                return HI_FAILURE;
            }
            break;

        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if(pstEvent->arg2 != HI_PDT_WORKMODE_UPGRADE)
            {
                MLOGW("Prevent upgrades in non-upgrade mode\n");
                return HI_SUCCESS;
            }
            stMsg.what = HI_EVENT_STATEMNG_UPGRADE_START;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;

        default:
            MLOGD("not support the event \n");
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_SD_UPGRADE_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMsg = {};

    memset(&stMsg,'\0',sizeof(HI_MESSAGE_S));
    if (SD_UPGRADE_CANCEL == WidgetHdl)
    {
        s32Ret = UI_SD_UPGRADE_BACK();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else if (SD_UPGRADE_CONFIRM == WidgetHdl)
    {
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if ((stWorkModeState.bEMRRecord) || (stWorkModeState.bStateMngInProgress))
        {
            MLOGE("stWorkModeState.bStateMngInProgress [%d]\n", stWorkModeState.bStateMngInProgress);
            MLOGE("stWorkModeState.bEMRRecord [%d]\n", stWorkModeState.bEMRRecord);
            return HI_FAILURE;
        }

        if (stWorkModeState.bRunning)
        {
            PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
            stMsg.what = HI_EVENT_STATEMNG_STOP;
            stMsg.arg1 = HI_TRUE;
            stMsg.arg2 = stWorkModeState.enWorkMode;
            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
            if (s32Ret)
            {
                MLOGE("send message failed !\n");
                PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
                UI_SD_UPGRADE_BACK();
                return HI_FAILURE;
            }
            return HI_SUCCESS;
        }
        PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
        stMsg.arg2 = HI_PDT_WORKMODE_UPGRADE;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
        if (s32Ret)
        {
            MLOGE("send message failed !\n");
            PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            UI_SD_UPGRADE_BACK();
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_SD_UPGRADE_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = UI_SD_UPGRADE_BACK();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (PDT_UI_KEY_MENU == wParam || PDT_UI_KEY_EXPAND == wParam)
    {
        if (SD_UPGRADE_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(SD_UPGRADE_CONFIRM);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (SD_UPGRADE_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(SD_UPGRADE_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_SD_UPGRADE_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SDUpgrade_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if ((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) || (PDT_UI_KEY_OK == wParam))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_Upgrade_result_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

