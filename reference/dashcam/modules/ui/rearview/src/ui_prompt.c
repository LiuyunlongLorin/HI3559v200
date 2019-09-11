/**
* @file    ui_prompt.c
* @brief   ui prompt windows. Date and time, SD format, Reset, USB choices...
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/16
* @version   1.0

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ui_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define UI_START_YEAR (2018)
#define UI_PROMPT_PARK_SHOW_TIME_SEC (1)    /**show park prompt window time,the unit is second**/

/**VoCrop,bright use**/
#define UI_PROMPT_COMMON_TIME_SEC (4)   /**common time,the unit is second**/

extern HI_S32 HI_GV_WheelView_SetUpImage(HI_HANDLE hWheelview, HI_HANDLE hRes);
extern HI_S32 HI_GV_WheelView_SetDownImage(HI_HANDLE hWheelview, HI_HANDLE hRes);
extern HI_S32 HI_GV_WheelView_GetSelItem(HI_HANDLE hWheelview, HI_U32* pItem);
extern HI_S32 HI_GV_WheelView_SetSelItem(HI_HANDLE hWheelview, HI_U32 pItem);
extern HI_S32  HI_GV_WheelView_GetItemNum(HI_HANDLE hWheelview, HI_U32* pItemNum);

static const HI_U32 s_au32UIDaysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static PDT_UI_DATETIME_S s_UIstDateTime = {};
static HI_U32 s_u32ShowParkWintime = 0;
static HI_BOOL s_bFastFormat = HI_TRUE;

static inline HI_BOOL PDT_UI_IsLeapYear(HI_U32 u32Year)
{
    return ( 0 == u32Year%400) || (0==u32Year%4 && 0 != u32Year%100) ;
}

static HI_U32 PDT_UI_GetDaysOfMonth(HI_U32 u32Year, HI_U32 u32Month)
{
    if(u32Month > 12)
    {
        MLOGE("mont is > 12, return 31\n" );
        return 31;
    }
    if(PDT_UI_IsLeapYear(u32Year) && 2 == u32Month)
    {
        return s_au32UIDaysOfMonth[u32Month-1] + 1;
    }
    else
    {
        return s_au32UIDaysOfMonth[u32Month-1];
    }
}



HI_S32 PDT_UI_ALARM_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if ((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) || (PDT_UI_KEY_OK == wParam) ||
        (PDT_UI_KEY_EXPAND == wParam))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALARM_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALARM_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALARM_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    if (TIMER_REFRESH_ALARM_POWEROFF == wParam)
    {
        s_u32ShowParkWintime++;

        if (s_u32ShowParkWintime == UI_PROMPT_PARK_SHOW_TIME_SEC*10)
        {
            s_u32ShowParkWintime = 0;

            s32Ret = HI_GV_Timer_Stop(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
            s32Ret |= HI_GV_Timer_Destroy(ALARM_WINDOW, TIMER_REFRESH_ALARM_POWEROFF);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_STOP, "stop and destroy timer");

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



HI_S32 PDT_UI_SD_FORMAT_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bFastFormat = HI_TRUE;

    s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_NORMAL,
         SKIN_DIALOG_FORMAT_CHOSED_NORMAL);
    s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_ACITVE,
         SKIN_DIALOG_FORMAT_CHOSED_ACTIVE);

    s32Ret = HI_GV_Widget_Active(FORMAT_SD_CANCEL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SD_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


    if(HI_SUCCESS != pstEvent->s32Result)
    {
        PDT_UI_COMM_ShowAlarm(STRING_FORMAT_FAILED);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_SD_FORMAT_OnClick(HIGV_HANDLE WidgetHdl)
{

    HI_S32 s32Ret = HI_SUCCESS;

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
        return HI_SUCCESS;
    }

    if (FORMAT_SD_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (FORMAT_SD_CONFIRM == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        HI_MESSAGE_S stMessage = {0};
        stMessage.what = HI_EVENT_STATEMNG_SD_FORMAT;
        stMessage.arg1 = s_bFastFormat ? HI_FSTOOL_FORMAT_MODE_ADVANCED : HI_FSTOOL_FORMAT_MODE_DISCARD;
        s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_SD_OnReceiveMsgResult);
        if (s32Ret)
        {
            MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }

    return HI_SUCCESS;

}


HI_S32 PDT_UI_SD_FORMAT_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_SD_FORMAT_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HIGV_PROC_GOON;
}
HI_S32 PDT_UI_SD_FORMAT_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_FORMAT_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (FORMAT_SD_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CONFIRM);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if (FORMAT_SD_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
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
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if (FORMAT_SD_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CANCEL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FORMAT_SD_CONFIRM);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_SD_FORMAT_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_RESET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Active(RESET_BUTTON_CANCEL);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_RESET_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (RESET_BUTTON_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(RESET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (RESET_BUTTON_CONFIRM == WidgetHdl)
    {
        HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
        s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);

        if (stWorkModeState.bStateMngInProgress)
        {
            MLOGE("In progress, give up reset\n");
            return HIGV_PROC_GOON;
        }

        HI_MESSAGE_S stMessage;
        memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
        stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_RESET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_RESET_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_RESET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(RESET_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (PDT_UI_KEY_MENU == wParam  || PDT_UI_KEY_EXPAND == wParam)
    {
        if (RESET_BUTTON_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(RESET_BUTTON_CONFIRM);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else if (RESET_BUTTON_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(RESET_BUTTON_CANCEL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_RESET_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_YEAR);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_MONTH);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_DAY);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_HOUR);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_MIN);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_YEAR,  s_UIstDateTime.year - UI_START_YEAR);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_MONTH, s_UIstDateTime.month - 1);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_DAY,   s_UIstDateTime.day - 1);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_HOUR,  s_UIstDateTime.hour);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_MIN, s_UIstDateTime.minute);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_TIME_SET_WheelViewSetFocus(HI_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE aWidgetHdl[] = {
        INVALID_HANDLE,
        TIME_SET_YEAR,
        TIME_SET_MONTH,
        TIME_SET_DAY,
        TIME_SET_HOUR,
        TIME_SET_MIN
    };

    HI_S32 i;
    for (i=0; i<UI_ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= UI_ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    for (i=0; i<UI_ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (INVALID_HANDLE == aWidgetHdl[i])
        {
            continue;
        }

        if (WidgetHdl == aWidgetHdl[i])
        {
            s32Ret = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetUpImage");
            s32Ret = HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetDownImage");
        }
        else
        {
            s32Ret = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_NO_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetUpImage");
            s32Ret = HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_NO_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetDownImage");
        }
    }

    s32Ret = HI_GV_Widget_Paint(TIME_SET_WINDOW, 0);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Widget_Paint");
}

HI_VOID PDT_UI_TIME_SET_WheelViewSetSelItem(HI_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE aWidgetHdl[] = {
        INVALID_HANDLE,
        TIME_SET_YEAR,
        TIME_SET_MONTH,
        TIME_SET_DAY,
        TIME_SET_HOUR,
        TIME_SET_MIN
    };

    HI_S32 i;
    for (i=0; i<UI_ARRAY_SIZE(aWidgetHdl); i++)
    {
        if(WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= UI_ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    if(INVALID_HANDLE == WidgetHdl)
    {
        MLOGE("Widget Handle:%d is invalid\n",WidgetHdl);
        return;
    }

    HI_U32 u32Item = 0;
    HI_U32 u32NextItem = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_WheelView_GetItemNum(WidgetHdl, &u32ItemNum);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_GV_WheelView_GetItemNum");

    s32Ret = HI_GV_WheelView_GetSelItem(WidgetHdl, &u32Item);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_GetSelItem");

    u32NextItem = (u32Item+1)%u32ItemNum;

    s32Ret = HI_GV_WheelView_SetSelItem(WidgetHdl, u32NextItem);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetSelItem");

    switch(WidgetHdl)
    {
        case TIME_SET_YEAR:
            s_UIstDateTime.year = UI_START_YEAR + u32NextItem;
            if(s_UIstDateTime.day >= PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month))
            {
                s_UIstDateTime.day = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
                s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_DAY, s_UIstDateTime.day - 1);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetSelItem");
            }
            break;
        case TIME_SET_MONTH:
            s_UIstDateTime.month = 1+u32NextItem;
            if(s_UIstDateTime.day >= PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month))
            {
                s_UIstDateTime.day = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
                s32Ret = HI_GV_WheelView_SetSelItem(TIME_SET_DAY, s_UIstDateTime.day - 1);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetSelItem");
            }
            break;
        case TIME_SET_DAY:
            s_UIstDateTime.day = 1+u32NextItem;
            break;
        case TIME_SET_HOUR:
            s_UIstDateTime.hour = u32NextItem;
            break;
        case TIME_SET_MIN:
            s_UIstDateTime.minute= u32NextItem;
            break;
    }

    PDT_UI_TIME_SET_Refresh();

}



HI_S32 PDT_UI_TIME_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_GetDateTime(&s_UIstDateTime);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"GetDateTime");

    if (s_UIstDateTime.year < UI_START_YEAR)
    {
        s_UIstDateTime.year = UI_START_YEAR;
    }
    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"TIME_SET_Refresh");
    s32Ret = HI_GV_Widget_Active(TIME_SET_YEAR);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");

    PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnFocusSelect(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0;
    MLOGD("wParam: %#lx  lParam: %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    switch(WidgetHdl)
    {
        case TIME_SET_YEAR:
            s_UIstDateTime.year = UI_START_YEAR + wParam;
            if(s_UIstDateTime.day >= PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month))
            {
                s_UIstDateTime.day = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
            }
            break;
        case TIME_SET_MONTH:
            s_UIstDateTime.month = 1 + wParam;
            if(s_UIstDateTime.day >= PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month))
            {
                s_UIstDateTime.day = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
            }
            break;
        case TIME_SET_DAY:
            s_UIstDateTime.day =  1 + wParam;
            break;
        case TIME_SET_HOUR:
            s_UIstDateTime.hour = wParam;
            break;
        case TIME_SET_MIN:
            s_UIstDateTime.minute = wParam;
            break;
        default:
            break;
    }
    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"DATE_SET_Refresh");

    return HIGV_PROC_GOON;
}




HI_S32 PDT_UI_TIME_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        switch(WidgetHdl)
        {
            case TIME_SET_YEAR:
                s32Ret = HI_GV_Widget_Active(TIME_SET_MONTH);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MONTH);
                break;

            case TIME_SET_MONTH:
                s32Ret = HI_GV_Widget_Active(TIME_SET_DAY);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_DAY);
                break;

            case TIME_SET_DAY:
                s32Ret = HI_GV_Widget_Active(TIME_SET_HOUR);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_HOUR);
                break;

            case TIME_SET_HOUR:
                s32Ret = HI_GV_Widget_Active(TIME_SET_MIN);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MIN);
                break;

            case TIME_SET_MIN:
                s32Ret = HI_GV_Widget_Active(TIME_SET_CANCEL);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(INVALID_HANDLE);
                break;

            case TIME_SET_CANCEL:
                s32Ret = HI_GV_Widget_Active(TIME_SET_CONFIRM);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                break;

            case TIME_SET_CONFIRM:
                s32Ret = HI_GV_Widget_Active(TIME_SET_YEAR);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);
                break;

            default:
                break;
        }
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        switch(WidgetHdl)
        {
            case TIME_SET_YEAR:
                s32Ret = HI_GV_Widget_Active(TIME_SET_CONFIRM);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(INVALID_HANDLE);
                break;

            case TIME_SET_MONTH:
                s32Ret = HI_GV_Widget_Active(TIME_SET_YEAR);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);
                break;

            case TIME_SET_DAY:
                s32Ret = HI_GV_Widget_Active(TIME_SET_MONTH);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MONTH);
                break;

            case TIME_SET_HOUR:
                s32Ret = HI_GV_Widget_Active(TIME_SET_DAY);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_DAY);
                break;

            case TIME_SET_MIN:
                s32Ret = HI_GV_Widget_Active(TIME_SET_HOUR);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_HOUR);
                break;

            case TIME_SET_CANCEL:
                s32Ret = HI_GV_Widget_Active(TIME_SET_MIN);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MIN);
                break;

            case TIME_SET_CONFIRM:
                s32Ret = HI_GV_Widget_Active(TIME_SET_CANCEL);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"HI_GV_Widget_Active");
                break;

            default:
                break;
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
         switch(WidgetHdl)
        {
            case TIME_SET_YEAR:
                PDT_UI_TIME_SET_WheelViewSetSelItem(TIME_SET_YEAR);
                break;

            case TIME_SET_MONTH:
                PDT_UI_TIME_SET_WheelViewSetSelItem(TIME_SET_MONTH);
                break;

            case TIME_SET_DAY:
                PDT_UI_TIME_SET_WheelViewSetSelItem(TIME_SET_DAY);
                break;

            case TIME_SET_HOUR:
                PDT_UI_TIME_SET_WheelViewSetSelItem(TIME_SET_HOUR);
                break;

            case TIME_SET_MIN:
                PDT_UI_TIME_SET_WheelViewSetSelItem(TIME_SET_MIN);
                break;

            case TIME_SET_CANCEL:
                s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_WINMNG_FinishWindow");
                break;

            case TIME_SET_CONFIRM:
                s32Ret = PDT_UI_COMM_SetDateTime(&s_UIstDateTime);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_TIME_SET_Confirm");
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);
                s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_WINMNG_FinishWindow");
                break;

            default:
                break;
        }
    }



    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_PDT_DEV_INFO_S stDevInfo = {};
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szTitle[HI_APPCOMM_COMM_STR_LEN] = {};
    HI_CHAR szBuffer[HI_APPCOMM_COMM_STR_LEN] = {};

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HI_FAILURE, "HI_PDT_PARAM_GetCommParam");

    PDT_UI_COMM_GetStringByID(STRING_KERNEL, szTitle, HI_APPCOMM_COMM_STR_LEN);
    snprintf(szBuffer, HI_APPCOMM_COMM_STR_LEN, " %s %s", szTitle,  stDevInfo.szSysVersion);
    s32Ret = HI_GV_Widget_SetText(ABOUT_WINDOW_BUTTON_KERNEL, szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    PDT_UI_COMM_GetStringByID(STRING_SOFTWARE, szTitle, HI_APPCOMM_COMM_STR_LEN);
    snprintf(szBuffer, HI_APPCOMM_COMM_STR_LEN, " %s %s", szTitle,  stDevInfo.szSoftVersion);
    s32Ret = HI_GV_Widget_SetText(ABOUT_WINDOW_BUTTON_SOFTWARE, szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    PDT_UI_COMM_GetStringByID(STRING_MODEL, szTitle, HI_APPCOMM_COMM_STR_LEN);
    snprintf(szBuffer, HI_APPCOMM_COMM_STR_LEN, " %s %s", szTitle,  stDevInfo.szModel);
    s32Ret = HI_GV_Widget_SetText(ABOUT_WINDOW_BUTTON_MODEL, szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

#if defined(CONFIG_WIFI_ON)
    HI_HAL_WIFI_APMODE_CFG_S stWifiCfg = {};
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_WIFI_AP, &stWifiCfg);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON, "HI_PDT_PARAM_GetCommParam");

    PDT_UI_COMM_GetStringByID(STRING_WIFI_SSID, szTitle, HI_APPCOMM_COMM_STR_LEN);
    snprintf(szBuffer, HI_APPCOMM_COMM_STR_LEN, " %s %s", szTitle,  stWifiCfg.stCfg.szWiFiSSID);
    s32Ret = HI_GV_Widget_SetText(ABOUT_WINDOW_BUTTON_WIFI_SSID, szBuffer);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_WINMNG_FinishWindow(ABOUT_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    return HIGV_PROC_STOP;
}

HI_S32 PDT_UI_ABOUT_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ABOUT_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON,"PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_TIME_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0;
    if(TIME_SET_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_WINMNG_FinishWindow");
    }
    else if(TIME_SET_CONFIRM == WidgetHdl)
    {
        s32Ret = PDT_UI_COMM_SetDateTime(&s_UIstDateTime);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_TIME_SET_Confirm");
        PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);
        s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HIGV_TOUCH_E type;

    type = pstTouchEvent->last.type;

    if (HIGV_TOUCH_START == type)
    {
        switch(WidgetHdl)
        {
            case TIME_SET_YEAR:
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_YEAR);
                break;

            case TIME_SET_MONTH:
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MONTH);
                break;

            case TIME_SET_DAY:
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_DAY);
                break;

            case TIME_SET_HOUR:
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_HOUR);
                break;

            case TIME_SET_MIN:
                PDT_UI_TIME_SET_WheelViewSetFocus(TIME_SET_MIN);
                break;

            default:
                break;

        }

    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnGestureScroll(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    //HI_GV_Msg_SendAsync(WHEELVIEW_DAY, HIGV_MSG_GESTURE_SCROLL, wParam, lParam);

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_WHEELVIEW_Register(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

HI_S32 PDT_UI_WHEELVIEW_Unregister(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

HI_S32 PDT_UI_WHEELVIEW_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt)
{
    switch(u32DBSource)
    {
        case DATAMODEL_YEAR:
            (*pu32RowCnt) = 20;
            break;

        case DATAMODEL_MONTH:
            (*pu32RowCnt) = 12;
            break;

        case DATAMODEL_DAY:
            (*pu32RowCnt) = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
            break;

        case DATAMODEL_HOUR:
            (*pu32RowCnt) = 24;
            break;

        case DATAMODEL_MIN:
            (*pu32RowCnt) = 60;
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_WHEELVIEW_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartRow, HI_U32 u32RowNumber, HI_VOID* pData, HI_U32* pu32RowCnt)
{
    HI_S32 i,s32MaxNum,s32BaseValue;
    HI_CHAR * pszRowValue = HI_NULL;

    pszRowValue = (HI_CHAR *)pData;
    *pu32RowCnt = 0;

    switch(u32DBSource)
    {
        case DATAMODEL_YEAR:
            s32BaseValue = UI_START_YEAR;
            s32MaxNum = 20;
            break;

        case DATAMODEL_MONTH:
            s32BaseValue = 1;
            s32MaxNum = 12;
            break;

        case DATAMODEL_DAY:
            s32BaseValue = 1;
            s32MaxNum = PDT_UI_GetDaysOfMonth(s_UIstDateTime.year, s_UIstDateTime.month);
            break;

        case DATAMODEL_HOUR:
            s32BaseValue = 0;
            s32MaxNum = 24;
            break;

        case DATAMODEL_MIN:
            s32BaseValue = 0;
            s32MaxNum = 60;
            break;

        default:
            return HI_SUCCESS;
    }

    for(i=u32StartRow; i<u32StartRow+u32RowNumber && i<s32MaxNum; i++)
    {
        snprintf(pszRowValue, 8, "%02d", i + s32BaseValue);
        pszRowValue += 8;
        (*pu32RowCnt)++;
    }

    return HI_SUCCESS;
}

static HI_U32  s_u32Totalvalue = 100;
static HI_S32  s_u32brightnesscur = 0;
static HI_U32  s_u32HideTime = 0;
static HI_S32 PDT_UI_PERIPHERAL_RefreshUI(HI_VOID)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szvalue[16] = {};

    memset(szvalue,0,16);
    snprintf(szvalue,16, "%d%s",s_u32brightnesscur,"%");
    s32Ret = HI_GV_Widget_SetText(PERIPHERAL_WINDOW_BRIGHTNESS_VALUE, szvalue);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s32Ret = HI_GV_ProgressBar_SetRange(PERIPHERAL_WINDOW_BRIGHTNESS_PIC, 0,s_u32Totalvalue);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s32Ret = HI_GV_ProgressBar_SetPos(PERIPHERAL_WINDOW_BRIGHTNESS_PIC, s_u32brightnesscur);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_GV_Widget_Paint(PERIPHERAL_WINDOW, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Paint");

    return s32Ret;
}

HI_S32 PDT_UI_PERIPHERAL_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS, &s_u32brightnesscur);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "volume or brightness");

    s32Ret = HI_GV_Timer_Create(PERIPHERAL_WINDOW, TIMER_REFRESH_PERIPHERAL, 1000);
    s32Ret |= HI_GV_Timer_Start(PERIPHERAL_WINDOW, TIMER_REFRESH_PERIPHERAL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Create or Start Timer");

    PDT_UI_PERIPHERAL_RefreshUI();

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_PERIPHERAL_Back(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_u32HideTime = 0;
    s32Ret = HI_GV_Timer_Stop(PERIPHERAL_WINDOW, TIMER_REFRESH_PERIPHERAL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Stop");

    s32Ret = HI_GV_Timer_Destroy(PERIPHERAL_WINDOW, TIMER_REFRESH_PERIPHERAL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Destroy");

    s32Ret = PDT_UI_WINMNG_FinishWindow(PERIPHERAL_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PERIPHERAL_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Reset(PERIPHERAL_WINDOW,TIMER_REFRESH_PERIPHERAL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_GV_Timer_Reset");

    s_u32HideTime++;

    if (s_u32HideTime == UI_PROMPT_COMMON_TIME_SEC)
    {
        s32Ret = PDT_UI_PERIPHERAL_Back();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_GOON,"PDT_UI_PERIPHERAL_Back");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PERIPHERAL_OnGestureScroll(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    s_u32HideTime = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("[%d,%d] --> [%d,%d], Distance[%d,%d]\n",
          ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.start.x,   ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.start.y,
          ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.end.x,     ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.end.y,
          ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.distanceX, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.scroll.distanceY);

    HIGV_GESTURE_EVENT_S gestureEvent;
    HI_S32 startX, startY, endX, endY;
    HI_S32 s32curvalue = 0;

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
            s32curvalue =  PDT_UI_SCREEN_BRIGHTNESS_LEVEL * (startY - endY) / XML_LAYOUT_HEIGHT;
        }
        else /** down */
        {
            s32curvalue = s32curvalue - PDT_UI_SCREEN_BRIGHTNESS_LEVEL * (endY - startY) / XML_LAYOUT_HEIGHT;
        }

        if((startX>0)&&(startX<XML_LAYOUT_WIDTH/2)&&(endX>0)&&(endX<XML_LAYOUT_WIDTH/2))
        {
            /* NO DO */
            return HIGV_PROC_GOON;
        }

        else if((startX>XML_LAYOUT_WIDTH/2)&&(startX<XML_LAYOUT_WIDTH)  \
            &&(endX>XML_LAYOUT_WIDTH/2)&&(endX<XML_LAYOUT_WIDTH))
        {
            s_u32brightnesscur += s32curvalue;
            s_u32brightnesscur = s_u32brightnesscur  > 100 ? 100 : s_u32brightnesscur ;
            s_u32brightnesscur = s_u32brightnesscur  < 0 ? 0 : s_u32brightnesscur ;

            s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_SCREEN_BRIGHTNESS,&s_u32brightnesscur);
#ifdef CONFIG_SCREEN
            s32Ret |= HI_HAL_SCREEN_SetLuma(HI_HAL_SCREEN_IDX_0, (HI_U32)s_u32brightnesscur);
#endif
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

        }

        PDT_UI_PERIPHERAL_RefreshUI();

    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PERIPHERAL_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    s_u32HideTime = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HIGV_HANDLE hWidget;
    PDT_UI_VO_VIEWCROP_E e_VoViewCrop;

    s32Ret = HI_GV_Widget_GetWidgetByPos_TouchDevice(pstTouchEvent->last.x,pstTouchEvent->last.y,&hWidget);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_STOP,"HI_GV_Widget_GetWidgetByPos_TouchDevice");

    MLOGD("touch x[%d] y[%d] hWidget[%d]\n",pstTouchEvent->last.x,pstTouchEvent->last.y,hWidget);

    if (hWidget == PERIPHERAL_WINDOW_VOGROPUP)
    {
        e_VoViewCrop = UI_VO_VIEW_UP;
        s32Ret = PDT_UI_HOME_SET_VoCrop(e_VoViewCrop);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (hWidget == PERIPHERAL_WINDOW_VOGROPDW)
    {
        e_VoViewCrop = UI_VO_VIEW_DOWN;
        s32Ret = PDT_UI_HOME_SET_VoCrop(e_VoViewCrop);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s32Ret = PDT_UI_PERIPHERAL_Back();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,HIGV_PROC_STOP,"PDT_UI_PERIPHERAL_Back");
    }
    return HIGV_PROC_STOP;
}


HI_S32 PDT_UI_PlayPark_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("show select PlayPark_page \n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szBuffer[HI_APPCOMM_COMM_STR_LEN] = {};
    HI_CHAR szTmpBuffer[HI_APPCOMM_COMM_STR_LEN] = {};
    HI_S32  s32CollisionCnt = 0;

    memset(szBuffer, 0, HI_APPCOMM_COMM_STR_LEN);
    memset(szTmpBuffer, 0, HI_APPCOMM_COMM_STR_LEN);
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
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Active(PLAYPARK_BUTTON_CONFIRM);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
    }

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_PlayPark_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32CollisionCnt = 0;

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT,&s32CollisionCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_WINMNG_FinishWindow(PLAYPARK_WINDOW);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (PLAYPARK_BUTTON_CONFIRM == WidgetHdl)
    {
        s32Ret = PDT_UI_PLAYBACK_PlayFile();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (PLAYPARK_BUTTON_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_FILELIST_Back();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PlayPark_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32CollisionCnt = 0;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT,&s32CollisionCnt);
        s32Ret |= PDT_UI_WINMNG_FinishWindow(PLAYPARK_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }
    else if (PDT_UI_KEY_MENU == wParam || PDT_UI_KEY_EXPAND == wParam )
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
        s32Ret = PDT_UI_PlayPark_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PlayPark_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_PlayPark_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

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
        s32Ret = HI_GV_Widget_SetTextByID(USB_LABEL_STATE, STRING_UVC);
    }
    else if(HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
    {
        s32Ret = HI_GV_Widget_SetTextByID(USB_LABEL_STATE, STRING_USB_STORAGE);
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
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_MESSAGE_S stMsg = {};

    s32Ret = PDT_UI_WINMNG_FinishWindow(SD_UPGRADE_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_STATEMNG_GetState");
    if (!stWorkModeState.bRunning)
    {
        if (!PDT_UI_COMM_SdIsReady())
        {
            MLOGE(" SD state is not normal \n");
            return HI_FAILURE;
        }
        stMsg.what = HI_EVENT_STATEMNG_START;
        stMsg.arg2 = stWorkModeState.enWorkMode;
        s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_SD_UPGRADE_EventProc(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};

    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    if(HI_SUCCESS != pstEvent->s32Result)
    {
        MLOGE("Event result is failed \n");
        return HI_FAILURE;
    }

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_STOP:
            s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE,HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            stMsg.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
            stMsg.arg1 = HI_TRUE;
            stMsg.arg2 = HI_PDT_WORKMODE_UPGRADE;

            s32Ret = HI_PDT_STATEMNG_SendMessage(&stMsg);
            if(s32Ret)
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
            MLOGE("statemng busy \n");
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

        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
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
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    }
    else if (PDT_UI_KEY_MENU == wParam || PDT_UI_KEY_EXPAND == wParam )
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

HI_S32 PDT_UI_SD_UPGRADE_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_SD_UPGRADE_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HIGV_PROC_GOON;
}


/* show  SD upgrade  result window  */
HI_S32 PDT_UI_SDUpgrade_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if ((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) || (PDT_UI_KEY_OK == wParam) ||
        (PDT_UI_KEY_EXPAND == wParam))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(SD_Upgrade_result_WINDOW);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SDUpgrade_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_FinishWindow(SD_Upgrade_result_WINDOW);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_WINMNG_FinishWindow");

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

