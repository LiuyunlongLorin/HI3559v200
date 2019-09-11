/**
 * @file      ui_prompt.c
 * @brief     ui prompt windows. Date and time, SD format, Reset, USB choices...
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

#include "ui_common.h"
#include "ui_common_playback.h"
#include "hi_hal_touchpad.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define UI_START_YEAR (2018)
#define UI_DEVINFO_LENGTH (128)
#define UI_DIALOG_TIMER_ID (10)
#define UI_DIALOG_DELAY_TIME (5) //ms
#define UI_DIALOG_EVENT_STARTTIMER_ID (10)

static HI_U32 s_TimerID = UI_DIALOG_TIMER_ID;
static HI_U32 s_DelayTime = UI_DIALOG_DELAY_TIME;
static HI_PARAM s_EventID = UI_DIALOG_EVENT_STARTTIMER_ID;


extern HI_S32 HI_GV_WheelView_SetUpImage(HIGV_HANDLE hWheelview, HIGV_HANDLE hRes);
extern HI_S32 HI_GV_WheelView_SetDownImage(HIGV_HANDLE hWheelview, HIGV_HANDLE hRes);
extern HI_S32 HI_GV_WheelView_GetSelItem(HIGV_HANDLE hWheelview, HI_U32* pItem);
extern HI_S32 HI_GV_WheelView_SetSelItem(HIGV_HANDLE hWheelview, HI_U32 pItem);
extern HI_S32 HI_GV_WheelView_GetItemNum(HIGV_HANDLE hWheelview, HI_U32* pItemNum);

static HI_BOOL s_bIsTracking = HI_TRUE;
static HI_BOOL s_bQuickFormat = HI_TRUE;
static const HI_U32 s_au32UIDaysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static PDT_UI_DATETIME_S stUIDateTime = {};
extern HIGV_HANDLE s_u32WinCallDialog;

static inline HI_BOOL PDT_UI_IsLeapYear(HI_U32 u32Year)
{
    HI_BOOL bRet = (((0 == u32Year % 400) || (0 == u32Year % 4 && 0 != u32Year % 100)) ? HI_TRUE : HI_FALSE);
    return bRet;
}

static HI_U32 PDT_UI_GetDaysOfMonth(HI_U32 u32Year, HI_U32 u32Month)
{
    if(u32Month > 12)
    {
        MLOGE("mont is > 12, return 31\n" );
        return 31;
    }
    if(PDT_UI_IsLeapYear(u32Year) && 2==u32Month)
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
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (((PDT_UI_KEY_BACK == wParam) || (PDT_UI_KEY_MENU == wParam) || (PDT_UI_KEY_ENTER == wParam)) && (s_bQuickFormat == HI_TRUE))
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALARM_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_bQuickFormat == HI_TRUE)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_SD_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bQuickFormat = HI_TRUE;

    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_UPGRADE_OnReceiveMsgResult(HI_EVENT_S* pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    stMessage.what = HI_EVENT_STATEMNG_UPGRADE_START;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_SD_DIALOG_Format(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMsg = {};

    PDT_UI_COMM_ShowAlarm(ALARM_WINDOW, ALARM_WINDOW_LABEL_INFO, STRING_QUICK_FORMATING);

    stMsg.what = HI_EVENT_STATEMNG_SD_FORMAT;
    stMsg.arg1 = s_bQuickFormat ? HI_FSTOOL_FORMAT_MODE_ADVANCED : HI_FSTOOL_FORMAT_MODE_DISCARD;
    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMsg, PDT_UI_SD_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(ALARM_WINDOW);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"FinishWindow");
    }
}

HI_VOID PDT_UI_DIALOG_Reset(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    if (stWorkModeState.bStateMngInProgress)
    {
        MLOGE("In progress, give up reset\n");
        return;
    }

    HI_MESSAGE_S stMessage;
    memset(&stMessage, 0, sizeof(HI_MESSAGE_S));
    stMessage.what = HI_EVENT_STATEMNG_FACTORY_RESET;
    s32Ret = HI_PDT_STATEMNG_SendMessage(&stMessage);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "HI_PDT_STATEMNG_SendMessage");
}

HI_VOID PDT_UI_DIALOG_Upgrade(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_FS_INFO_S stFSInfo = {};
    HI_MESSAGE_S stMessage = {};
    HI_EVENT_S stEvent = {};
    memset(&stEvent, 0, sizeof(stEvent));

    s32Ret = PDT_UI_COMM_GetFSInfo(&stFSInfo);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetFSInfo");

    HI_EVTHUB_GetEventHistory(HI_EVENT_UPGRADE_NEWPKT, &stEvent);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"GetEventHistory");

    HI_UPGRADE_EVENT_INFO_S* pstEventInf = (HI_UPGRADE_EVENT_INFO_S*)stEvent.aszPayload;
    MLOGI("PktLen = [%d]\n",pstEventInf->u32PktLen);

    //TODO Battery detection
    if(stFSInfo.u64AvailableSize <= pstEventInf->u32PktLen)
    {
        memset(&stEvent, 0, sizeof(stEvent));
        stEvent.EventID = HI_EVENT_UPGRADE_FAILURE;
        MLOGE("Upgrade Failed! Not enough storage space!\n");
        HI_EVTHUB_Publish(&stEvent);
        pstEventInf = HI_NULL;
        return;
    }

    MLOGD("switch to upgrade mode\n");
    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = HI_PDT_WORKMODE_UPGRADE;
    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_UPGRADE_OnReceiveMsgResult);

    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"SendAsyncMsg");
}

HI_VOID PDT_UI_DIALOG_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
    {
        s_bQuickFormat = s_bQuickFormat ? HI_FALSE : HI_TRUE;

        s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_NORMAL,
            s_bQuickFormat ? SKIN_DIALOG_WINDOW_BUTTON_FORMAT_CHOSED_NORMAL
            : SKIN_DIALOG_WINDOW_BUTTON_FORMAT_NOT_CHOSED_NORMAL);
        s32Ret = HI_GV_Widget_SetSkin(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, HIGV_SKIN_ACITVE,
            s_bQuickFormat ? SKIN_DIALOG_WINDOW_BUTTON_FORMAT_CHOSED_ACTIVE
            : SKIN_DIALOG_WINDOW_BUTTON_FORMAT_NOT_CHOSED_ACTIVE);

        s32Ret = HI_GV_Widget_Paint(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE, NULL);
        return;
    }
    else if(DIALOG_WINDOW_BUTTON_CONFIRM == WidgetHdl)
    {
        switch (s_u32WinCallDialog)
        {
            case FILELIST_PAGE:
                PDT_UI_FILELIST_DIALOG_Delete();
                break;

            case PLAYBACK_PAGE:
                s32Ret = PDT_UI_COMM_PLAYBACK_DeleteFile(DIALOG_WINDOW, PLAYBACK_PAGE);
                HI_GV_Widget_Active(PLAYBACK_BUTTON_DELETE);
                break;

            case STRING_FORMAT_SD:
                PDT_UI_SD_DIALOG_Format();
                break;

            case STRING_RESET_SYSTEM:
                PDT_UI_DIALOG_Reset();
                break;

            case STRING_UPGRADE_AT_ONCE:
                PDT_UI_DIALOG_Upgrade();
                break;

            default:
                break;
        }
    }
    else if(DIALOG_WINDOW_BUTTON_CANCEL == WidgetHdl)
    {
        switch (s_u32WinCallDialog)
        {
            case PLAYBACK_PAGE:
                HI_GV_Widget_Active(PLAYBACK_BUTTON_DELETE);
                break;

            case STRING_UPGRADE_AT_ONCE:
                stMessage.what = HI_EVENT_STATEMNG_DONOTUPGRADE;
                HI_PDT_STATEMNG_SendMessage(&stMessage);
                break;

            default:
                break;
        }
    }

    s32Ret = HI_GV_Widget_Show(DIALOG_WINDOW_GROUPBOX_CONTENT);
    s32Ret = HI_GV_Widget_Hide(DIALOG_WINDOW_GROUPBOX_FORMAT);

    s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"ActiveWidget");

    s32Ret = PDT_UI_WINMNG_FinishWindow(DIALOG_WINDOW);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "FinishWindow");
}

HI_S32 PDT_UI_DIALOG_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = HI_GV_Widget_Show(DIALOG_WINDOW_GROUPBOX_CONTENT);
        s32Ret = HI_GV_Widget_Hide(DIALOG_WINDOW_GROUPBOX_FORMAT);
        s32Ret = PDT_UI_WINMNG_FinishWindow(DIALOG_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if(HI_GV_Widget_IsShow(DIALOG_WINDOW_GROUPBOX_FORMAT)
            &&(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl))
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(DIALOG_WINDOW_BUTTON_CANCEL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CONFIRM);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(DIALOG_WINDOW_BUTTON_CONFIRM == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(HI_GV_Widget_IsShow(DIALOG_WINDOW_GROUPBOX_FORMAT)
                ? DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE : DIALOG_WINDOW_BUTTON_CANCEL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        PDT_UI_DIALOG_OnClick(WidgetHdl);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DIALOG_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(DIALOG_WINDOW_BUTTON_CONFIRM == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CONFIRM);
        s32Ret |= HI_GV_Widget_Paint(DIALOG_WINDOW_BUTTON_CONFIRM, NULL);
        s32Ret |= HI_GV_Msg_SendAsync(DIALOG_WINDOW, HIGV_MSG_EVENT, 0, s_EventID);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else
    {
        if(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_FORMAT_CHOOSE);
        }
        PDT_UI_DIALOG_OnClick(WidgetHdl);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DIALOG_OnShow(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s_bQuickFormat = HI_TRUE;

    s32Ret = HI_GV_Timer_Create(WidgetHdl, s_TimerID, s_DelayTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DIALOG_OnHide(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Destroy(WidgetHdl, s_TimerID);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DIALOG_OnEvent(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (s_EventID == lParam)
    {
        s32Ret = HI_GV_Timer_Start(WidgetHdl, s_TimerID);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DIALOG_OnTimer(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
    if (s_TimerID == wParam)
    {
        HI_GV_Timer_Stop(WidgetHdl, s_TimerID);
        PDT_UI_DIALOG_OnClick(DIALOG_WINDOW_BUTTON_CONFIRM);
    }

    return HIGV_PROC_GOON;
}

#if 0
static HI_S32 PDT_UI_USB_CHOICE_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    switch(WidgetHdl)
    {
        case USB_CHOICES_WINDOW_BUTTON_CANCEL:
            s32Ret = PDT_UI_WINMNG_FinishWindow(USB_CHOICES_WINDOW);
            break;
        case USB_CHOICES_WINDOW_BUTTON_UVC:
            s32Ret = HI_USB_SetMode(HI_USB_MODE_UVC);
            break;
        case USB_CHOICES_WINDOW_BUTTON_STORAGE:
            s32Ret = HI_USB_SetMode(HI_USB_MODE_STORAGE);
            break;
        default:
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_USB_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    HIGV_HANDLE aBtnHdl[] = {
        USB_CHOICES_WINDOW_BUTTON_CANCEL,
        USB_CHOICES_WINDOW_BUTTON_UVC,
        USB_CHOICES_WINDOW_BUTTON_STORAGE
    };

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(USB_CHOICES_WINDOW);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_COMM_ActiveNextWidget(aBtnHdl,ARRAY_SIZE(aBtnHdl),WidgetHdl);
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_UI_USB_CHOICE_OnClick(WidgetHdl);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_USB_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
#if 0
    HIGV_GESTURE_EVENT_S * pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    MLOGD("widget:%d, [%d,%d]\n",WidgetHdl, pstGestureEvent->gesture.tap.pointer.x, pstGestureEvent->gesture.tap.pointer.y);
#endif

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_USB_CHOICE_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}
#endif

HI_S32 PDT_UI_USB_STATE_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkState = {};

#if defined(CFG_LCD_TOUCHPAD_ON)
    s32Ret = HI_GV_Gesture_Enable(HI_FALSE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DisableTouchpad");

    s32Ret = HI_HAL_TOUCHPAD_Deinit();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DeinitTouchpad");
#endif

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    if(HI_PDT_WORKMODE_UVC == stWorkState.enWorkMode)
    {
        s32Ret = HI_GV_Widget_SetSkin(USB_BUTTON_STATE, HIGV_SKIN_NORMAL, SKIN_USB_CHOICES_WINDOW_BUTTON_UVC);
        s32Ret |= HI_GV_Widget_SetSkin(USB_BUTTON_STATE, HIGV_SKIN_ACITVE, SKIN_USB_CHOICES_WINDOW_BUTTON_UVC);
        s32Ret |= HI_GV_Widget_SetTextByID(USB_LABEL_STATE, STRING_USB_UVC);
        s32Ret |= HI_GV_Widget_Paint(USB_STATE_WINDOW, NULL);
    }
    else if(HI_PDT_WORKMODE_USB_STORAGE == stWorkState.enWorkMode)
    {
        s32Ret = HI_GV_Widget_SetSkin(USB_BUTTON_STATE, HIGV_SKIN_NORMAL, SKIN_USB_CHOICES_WINDOW_BUTTON_STORAGE);
        s32Ret |= HI_GV_Widget_SetSkin(USB_BUTTON_STATE, HIGV_SKIN_ACITVE, SKIN_USB_CHOICES_WINDOW_BUTTON_STORAGE);
        s32Ret |= HI_GV_Widget_SetTextByID(USB_LABEL_STATE, STRING_USB_SRORAGE);
        s32Ret |= HI_GV_Widget_Paint(USB_STATE_WINDOW, NULL);
    }
    else
    {
        MLOGE("It's not usb mode\n");
        s32Ret = PDT_UI_WINMNG_FinishWindow(USB_STATE_WINDOW);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_USB_STATE_OnHide(HIGV_HANDLE WidgetHdl,HI_PARAM wParam, HI_PARAM lParam)
{
#if defined(CFG_LCD_TOUCHPAD_ON)
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_HAL_TOUCHPAD_Init();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "InitTouchPad");

    s32Ret = HI_GV_Gesture_Enable(HI_TRUE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "EnableTouchPad");
#endif

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_UPGRADE_DEV_INFO_S stDevInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szDevInfo[UI_DEVINFO_LENGTH] = {};

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HI_FAILURE);

    HI_U32 u32DevInfoLen = strnlen("Model:\n", UI_DEVINFO_LENGTH) + strnlen(stDevInfo.szModel, UI_DEVINFO_LENGTH) +
        strnlen("\n\n", UI_DEVINFO_LENGTH) + strnlen("SoftVersion:\n", UI_DEVINFO_LENGTH) +
        strnlen(stDevInfo.szSoftVersion, UI_DEVINFO_LENGTH) + 1;
    if(u32DevInfoLen > UI_DEVINFO_LENGTH)
    {
        HI_GV_Widget_SetText(ABOUT_SYS_INFO, "");
        return HIGV_PROC_GOON; // buffer overflow
    }

    strncat(szDevInfo, "Model:\n", UI_DEVINFO_LENGTH-strnlen(szDevInfo, UI_DEVINFO_LENGTH)-1);
    strncat(szDevInfo, stDevInfo.szModel, UI_DEVINFO_LENGTH-strnlen(szDevInfo, UI_DEVINFO_LENGTH)-1);
    strncat(szDevInfo, "\n\n", UI_DEVINFO_LENGTH-strnlen(szDevInfo, UI_DEVINFO_LENGTH)-1);

    strncat(szDevInfo, "SoftVersion:\n", UI_DEVINFO_LENGTH-strnlen(szDevInfo, UI_DEVINFO_LENGTH)-1);
    strncat(szDevInfo, stDevInfo.szSoftVersion, UI_DEVINFO_LENGTH-strnlen(szDevInfo, UI_DEVINFO_LENGTH)-1);

    MLOGD("DevInfo:%s\n",szDevInfo);
    s32Ret = HI_GV_Widget_SetText(ABOUT_SYS_INFO, szDevInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_ABOUT_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case ABOUT_WINDOW_BUTTON_BACK:
            s32Ret = PDT_UI_WINMNG_FinishWindow(ABOUT_WINDOW);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_ABOUT_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("widget:%d, pointer[%d,%d]\n", WidgetHdl, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.x,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.y);

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_ABOUT_BUTTON_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ABOUT_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(ABOUT_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if(PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_UI_ABOUT_BUTTON_OnClick(WidgetHdl);
       HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DATE_SET_Refresh()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_GV_ADM_Sync(DATAMODEL_YEAR);
    s32Ret |= HI_GV_ADM_Sync(DATAMODEL_MONTH);
    s32Ret |= HI_GV_ADM_Sync(DATAMODEL_DAY);

    s32Ret |= HI_GV_WheelView_SetSelItem(DATE_SET_WINDOW_YEAR,  stUIDateTime.year - UI_START_YEAR);
    s32Ret |= HI_GV_WheelView_SetSelItem(DATE_SET_WINDOW_MONTH, stUIDateTime.month - 1);
    s32Ret |= HI_GV_WheelView_SetSelItem(DATE_SET_WINDOW_DAY,   stUIDateTime.day - 1);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}
HI_S32 PDT_UI_TIME_SET_Refresh()
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_GV_ADM_Sync(DATAMODEL_HOUR);
    s32Ret |= HI_GV_ADM_Sync(DATAMODEL_MIN);
    s32Ret |= HI_GV_WheelView_SetSelItem(TIME_SET_WINDOW_HOUR,  stUIDateTime.hour);
    s32Ret |= HI_GV_WheelView_SetSelItem(TIME_SET_WINDOW_MIN, stUIDateTime.minute);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_DATE_SET_Confirm()
{
    HI_S32 s32Ret = 0;
    PDT_UI_DATETIME_S stTempTime = {};

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"StartWindow");

    s32Ret = PDT_UI_COMM_GetDateTime(&stTempTime);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "GetDateTime");

    stUIDateTime.hour = stTempTime.hour;
    stUIDateTime.minute = stTempTime.minute;
    s32Ret = PDT_UI_COMM_SetDateTime(&stUIDateTime);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetDateTime");

    s32Ret  = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    s32Ret |= PDT_UI_WINMNG_FinishWindow(DATE_SET_WINDOW);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"FinishWindow");
}

HI_VOID PDT_UI_DateWheelViewSetFocus(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE aWidgetHdl[] = {
        INVALID_HANDLE,
        DATE_SET_WINDOW_YEAR,
        DATE_SET_WINDOW_MONTH,
        DATE_SET_WINDOW_DAY
    };

    HI_S32 i;
    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (INVALID_HANDLE == aWidgetHdl[i])
        {
            continue;
        }

        if (WidgetHdl == aWidgetHdl[i])
        {
            s32Ret  = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_FOCUS_COVER);
            s32Ret |= HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewImage");
        }
        else
        {
            s32Ret  = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_NO_FOCUS_COVER);
            s32Ret |= HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_NO_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewImage");
        }
    }

    s32Ret  = HI_GV_Widget_Paint(DATE_SET_WINDOW_YEAR, 0);
    s32Ret |= HI_GV_Widget_Paint(DATE_SET_WINDOW_MONTH, 0);
    s32Ret |= HI_GV_Widget_Paint(DATE_SET_WINDOW_DAY, 0);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Widget_Paint");
}

HI_S32 PDT_UI_DATE_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_GetDateTime(&stUIDateTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    if (stUIDateTime.year < UI_START_YEAR)
    {
        stUIDateTime.year = UI_START_YEAR;
    }
    s32Ret = PDT_UI_DATE_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_YEAR);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_YEAR);

    return HIGV_PROC_GOON;
}

HI_VOID PDT_UI_DateWheelViewSetSelItem(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 pItem = 0;
    HIGV_HANDLE aWidgetHdl[] = {
        INVALID_HANDLE,
        DATE_SET_WINDOW_YEAR,
        DATE_SET_WINDOW_MONTH,
        DATE_SET_WINDOW_DAY
    };

    HI_S32 i;
    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if(WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    if(INVALID_HANDLE == WidgetHdl)
    {
        MLOGE("Widget Handle:%d is invalid\n",WidgetHdl);
        return;
    }

    HI_U32 u32ItemNum;
    s32Ret  = HI_GV_WheelView_GetItemNum(WidgetHdl, &u32ItemNum);
    s32Ret |= HI_GV_WheelView_GetSelItem(WidgetHdl, &pItem);
    s32Ret |= HI_GV_WheelView_SetSelItem(WidgetHdl, (pItem+1)%u32ItemNum);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewSelItem");

    switch(WidgetHdl)
    {
        case DATE_SET_WINDOW_YEAR:
            stUIDateTime.year = UI_START_YEAR + (pItem + 1) % u32ItemNum;
            if(stUIDateTime.day >= PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month))
            {
                stUIDateTime.day = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);

                s32Ret = HI_GV_WheelView_SetSelItem(DATE_SET_WINDOW_DAY, stUIDateTime.day - 1);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetSelItem");
            }
            break;

        case DATE_SET_WINDOW_MONTH:
            stUIDateTime.month = 1 + (pItem + 1) % u32ItemNum;
            if(stUIDateTime.day >= PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month))
            {
                stUIDateTime.day = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);

                s32Ret = HI_GV_WheelView_SetSelItem(DATE_SET_WINDOW_DAY, stUIDateTime.day - 1);
                PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "WheelView_SetSelItem");
            }
            break;

        case DATE_SET_WINDOW_DAY:
            stUIDateTime.day =  1 + (pItem + 1) % u32ItemNum;
            break;

        default:
            break;
    }
}


HI_S32 PDT_UI_DATE_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(DATE_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        switch (WidgetHdl)
        {
            case DATE_SET_WINDOW_YEAR:
                s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_MONTH);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_MONTH);
                break;

            case DATE_SET_WINDOW_MONTH:
                s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_DAY);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_DAY);
                break;

            case DATE_SET_WINDOW_DAY:
                s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_BUTTON_CANCEL);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_DateWheelViewSetFocus(INVALID_HANDLE);
                break;

            case DATE_SET_WINDOW_BUTTON_CANCEL:
                s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_BUTTON_CONFIRM);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                break;

            case DATE_SET_WINDOW_BUTTON_CONFIRM:
                s32Ret = HI_GV_Widget_Active(DATE_SET_WINDOW_YEAR);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_YEAR);
                break;

            default:
                break;
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
         switch(WidgetHdl)
        {
            case DATE_SET_WINDOW_YEAR:
                PDT_UI_DateWheelViewSetSelItem(DATE_SET_WINDOW_YEAR);
                break;

            case DATE_SET_WINDOW_MONTH:
                PDT_UI_DateWheelViewSetSelItem(DATE_SET_WINDOW_MONTH);
                break;

            case DATE_SET_WINDOW_DAY:
                PDT_UI_DateWheelViewSetSelItem(DATE_SET_WINDOW_DAY);
                break;

            case DATE_SET_WINDOW_BUTTON_CANCEL:
                s32Ret = PDT_UI_WINMNG_FinishWindow(DATE_SET_WINDOW);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                break;

            case DATE_SET_WINDOW_BUTTON_CONFIRM:
                PDT_UI_DATE_SET_Confirm();
                break;

            default:
                break;
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DATE_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if(DATE_SET_WINDOW_BUTTON_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(DATE_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }
    else if(DATE_SET_WINDOW_BUTTON_CONFIRM == WidgetHdl)
    {
        PDT_UI_DATE_SET_Confirm();
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DATE_SET_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HIGV_TOUCH_E type;

    type = pstTouchEvent->last.type;

    if (HIGV_TOUCH_START == type)
    {
        switch(WidgetHdl)
        {
            case DATE_SET_WINDOW_YEAR:
                HI_GV_Widget_Active(DATE_SET_WINDOW_YEAR);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_YEAR);
                break;

            case DATE_SET_WINDOW_MONTH:
                HI_GV_Widget_Active(DATE_SET_WINDOW_MONTH);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_MONTH);
                break;

            case DATE_SET_WINDOW_DAY:
                HI_GV_Widget_Active(DATE_SET_WINDOW_DAY);
                PDT_UI_DateWheelViewSetFocus(DATE_SET_WINDOW_DAY);
                break;

            default:
                break;
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_DATETIME_OnFocusSelect(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0;
    MLOGD("wParam: %lx, lParam: %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    switch(WidgetHdl)
    {
        case DATE_SET_WINDOW_YEAR:
            stUIDateTime.year = UI_START_YEAR + wParam;
            if(stUIDateTime.day >= PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month))
            {
                stUIDateTime.day = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);
            }
            break;
        case DATE_SET_WINDOW_MONTH:
            stUIDateTime.month = 1 + wParam;
            if(stUIDateTime.day >= PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month))
            {
                stUIDateTime.day = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);
            }
            break;
        case DATE_SET_WINDOW_DAY:
            stUIDateTime.day =  1 + wParam;
            break;
        case TIME_SET_WINDOW_HOUR:
            stUIDateTime.hour = wParam;
            break;
        case TIME_SET_WINDOW_MIN:
            stUIDateTime.minute = wParam;
            break;
        default:
            break;
    }
    s32Ret = PDT_UI_DATE_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_Confirm()
{
    HI_S32 s32Ret;
    PDT_UI_DATETIME_S stTempTime = {};

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = PDT_UI_COMM_GetDateTime(&stTempTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    stUIDateTime.year = stTempTime.year;
    stUIDateTime.month = stTempTime.month;
    stUIDateTime.day = stTempTime.day;
    s32Ret = PDT_UI_COMM_SetDateTime(&stUIDateTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret  = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    s32Ret |= PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return s32Ret;
}

HI_VOID PDT_UI_TimeWheelViewSetFocus(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE aWidgetHdl[] = {
        INVALID_HANDLE,
        TIME_SET_WINDOW_HOUR,
        TIME_SET_WINDOW_MIN
    };

    HI_S32 i;
    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if (INVALID_HANDLE == aWidgetHdl[i])
        {
            continue;
        }

        if (WidgetHdl == aWidgetHdl[i])
        {
            s32Ret  = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_FOCUS_COVER);
            s32Ret |= HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewImage");
        }
        else
        {
            s32Ret  = HI_GV_WheelView_SetUpImage(aWidgetHdl[i], SKIN_WHEELVIEW_UP_NO_FOCUS_COVER);
            s32Ret |= HI_GV_WheelView_SetDownImage(aWidgetHdl[i], SKIN_WHEELVIEW_DOWN_NO_FOCUS_COVER);
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewImage");
        }
    }

    s32Ret  = HI_GV_Widget_Paint(TIME_SET_WINDOW_HOUR, 0);
    s32Ret |= HI_GV_Widget_Paint(TIME_SET_WINDOW_MIN, 0);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Widget_Paint");
}

HI_VOID PDT_UI_TimeWheelViewSetSelItem(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 pItem = 0;
    HIGV_HANDLE aWidgetHdl[] = {
          INVALID_HANDLE,
          TIME_SET_WINDOW_HOUR,
          TIME_SET_WINDOW_MIN
      };


    HI_S32 i;
    for (i=0; i<ARRAY_SIZE(aWidgetHdl); i++)
    {
        if(WidgetHdl == aWidgetHdl[i])
        {
            break;
        }
    }

    if (i >= ARRAY_SIZE(aWidgetHdl))
    {
        MLOGE("Widget Handle:%d is not found\n",WidgetHdl);
        return;
    }

    if(INVALID_HANDLE == WidgetHdl)
    {
        MLOGE("Widget Handle:%d is invalid\n",WidgetHdl);
        return;
    }

    HI_U32 u32ItemNum;
    s32Ret  = HI_GV_WheelView_GetItemNum(WidgetHdl, &u32ItemNum);
    s32Ret |= HI_GV_WheelView_GetSelItem(WidgetHdl, &pItem);
    s32Ret |= HI_GV_WheelView_SetSelItem(WidgetHdl, (pItem+1)%u32ItemNum);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetWheelViewSelItem");

    switch(WidgetHdl)
    {
        case TIME_SET_WINDOW_HOUR:
            stUIDateTime.hour = (pItem + 1) % u32ItemNum;
            break;

        case TIME_SET_WINDOW_MIN:
            stUIDateTime.minute = (pItem + 1) % u32ItemNum;
            break;

        default:
            break;
    }
}

HI_S32 PDT_UI_TIME_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_GetDateTime(&stUIDateTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    if (stUIDateTime.year < UI_START_YEAR)
    {
        stUIDateTime.year = UI_START_YEAR;
    }
    s32Ret = PDT_UI_TIME_SET_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    s32Ret = HI_GV_Widget_Active(TIME_SET_WINDOW_HOUR);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    PDT_UI_TimeWheelViewSetFocus(TIME_SET_WINDOW_HOUR);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        switch(WidgetHdl)
        {
            case TIME_SET_WINDOW_HOUR:
                s32Ret = HI_GV_Widget_Active(TIME_SET_WINDOW_MIN);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_TimeWheelViewSetFocus(TIME_SET_WINDOW_MIN);
                break;

            case TIME_SET_WINDOW_MIN:
                s32Ret = HI_GV_Widget_Active(TIME_SET_WINDOW_BUTTON_CANCEL);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_TimeWheelViewSetFocus(INVALID_HANDLE);
                break;

            case TIME_SET_WINDOW_BUTTON_CANCEL:
                s32Ret = HI_GV_Widget_Active(TIME_SET_WINDOW_BUTTON_CONFIRM);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                break;

            case TIME_SET_WINDOW_BUTTON_CONFIRM:
                s32Ret = HI_GV_Widget_Active(TIME_SET_WINDOW_HOUR);
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                PDT_UI_TimeWheelViewSetFocus(TIME_SET_WINDOW_HOUR);
                break;

            default:
                break;
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
         switch(WidgetHdl)
        {
            case TIME_SET_WINDOW_HOUR:
                PDT_UI_TimeWheelViewSetSelItem(TIME_SET_WINDOW_HOUR);
                break;

            case TIME_SET_WINDOW_MIN:
                PDT_UI_TimeWheelViewSetSelItem(TIME_SET_WINDOW_MIN);
                break;

            case TIME_SET_WINDOW_BUTTON_CANCEL:
                PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
                break;

            case TIME_SET_WINDOW_BUTTON_CONFIRM:
                s32Ret = PDT_UI_TIME_SET_Confirm();
                HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
                break;

            default:
                break;
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_TIME_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0;
    if(TIME_SET_WINDOW_BUTTON_CANCEL == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(TIME_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }
    else if(TIME_SET_WINDOW_BUTTON_CONFIRM == WidgetHdl)
    {
        s32Ret = PDT_UI_TIME_SET_Confirm();
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
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
            case TIME_SET_WINDOW_HOUR:
                HI_GV_Widget_Active(TIME_SET_WINDOW_HOUR);
                PDT_UI_TimeWheelViewSetFocus(TIME_SET_WINDOW_HOUR);
                break;

            case TIME_SET_WINDOW_MIN:
                HI_GV_Widget_Active(TIME_SET_WINDOW_MIN);
                PDT_UI_TimeWheelViewSetFocus(TIME_SET_WINDOW_MIN);
                break;

            default:
                break;
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_WHEELVIEW_Register(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

HI_S32 PDT_UI_WHEELVIEW_Unregister(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
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
            (*pu32RowCnt) = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);
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
    HI_S32 i = 0, s32MaxNum = 0, s32BaseValue = 0;
    HI_CHAR * pszRowValue = HI_NULL;

    pszRowValue = (HI_CHAR *)pData;
    *pu32RowCnt = 0;

    switch(u32DBSource)
    {
        case DATAMODEL_YEAR:
            s32BaseValue = UI_START_YEAR;
            s32MaxNum = 20;
            goto GetRowValue;
            break;

        case DATAMODEL_MONTH:
            s32BaseValue = 1;
            s32MaxNum = 12;
            goto GetRowValue;
            break;

        case DATAMODEL_DAY:
            s32BaseValue = 1;
            s32MaxNum = PDT_UI_GetDaysOfMonth(stUIDateTime.year, stUIDateTime.month);
            goto GetRowValue;
            break;

        case DATAMODEL_HOUR:
            s32BaseValue = 0;
            s32MaxNum = 24;
            goto GetRowValue;
            break;

        case DATAMODEL_MIN:
            s32BaseValue = 0;
            s32MaxNum = 60;
            goto GetRowValue;
            break;

        default:
            break;
    }

GetRowValue :
    for(i=u32StartRow; i<u32StartRow+u32RowNumber && i<s32MaxNum; i++)
    {
        snprintf(pszRowValue, 8, "%d", i + s32BaseValue);
        pszRowValue += 8;
        (*pu32RowCnt)++;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_VOLUME_SET_SetVolume(HI_U32 u32VolumeValue)
{
    HI_S32 s32Ret;
    HI_CHAR szVolume[16] = {};
    HI_HANDLE AoHdl = HI_INVALID_HANDLE;

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &u32VolumeValue);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_COMM_GetAVHdl(HI_PDT_WORKMODE_PLAYBACK, &AoHdl, NULL, NULL, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_PDT_MEDIA_SetAoVolume(AoHdl, u32VolumeValue);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    snprintf(szVolume, 16, "%u%s", u32VolumeValue, "%");
    s32Ret = HI_GV_Widget_SetText(VOLUME_SET_LABEL_VOLUME, szVolume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Paint(VOLUME_SET_LABEL_VOLUME, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_VOLUME_SET_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret;
    HI_S32 s32Volume = 0;
    HI_CHAR szVolume[16] = {};

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    snprintf(szVolume, 16, "%d%s", s32Volume, "%");
    s32Ret = HI_GV_Widget_SetText(VOLUME_SET_LABEL_VOLUME, szVolume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Track_SetRange(VOLUME_SET_TRACKBAR_VOLUME, 0, 100);
    s32Ret |= HI_GV_Track_SetCurVal(VOLUME_SET_TRACKBAR_VOLUME, s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Active(VOLUME_SET_BUTTON_BACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_VOID PDT_UI_VOLUME_SET_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret;
    HI_S32 s32Volume = 0;

    if(VOLUME_SET_BUTTON_BACK == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(VOLUME_SET_WINDOW);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "FinishWindow");
    }
    else if(VOLUME_SET_BUTTON_MINUS == WidgetHdl || VOLUME_SET_BUTTON_PLUS == WidgetHdl)
    {
        s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "StartWindow");

        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &s32Volume);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "GetCommParam");

        if(VOLUME_SET_BUTTON_MINUS == WidgetHdl)
        {
            if(s32Volume != 0)
            {
                s32Volume -= 1;
            }

            s32Volume =(s32Volume / 25) * 25;
        }
        else
        {
            if(s32Volume != 100)
            {
                s32Volume += 1;
            }

            s32Volume =((s32Volume + 24)/ 25) * 25;
        }

        s32Ret = PDT_UI_VOLUME_SET_SetVolume(s32Volume);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetVolume");

        s32Ret = HI_GV_Track_SetCurVal(VOLUME_SET_TRACKBAR_VOLUME, s32Volume);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetCurVal");

        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "FinishWindow");
    }
}

HI_S32 PDT_UI_VOLUME_SET_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret;

    if(PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(VOLUME_SET_WINDOW);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if(PDT_UI_KEY_MENU == wParam)
    {
        if(VOLUME_SET_BUTTON_BACK == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(VOLUME_SET_BUTTON_MINUS);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(VOLUME_SET_BUTTON_MINUS == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(VOLUME_SET_BUTTON_PLUS);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else if(VOLUME_SET_BUTTON_PLUS == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(VOLUME_SET_BUTTON_BACK);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
    }
    else if(PDT_UI_KEY_ENTER == wParam)
    {
        PDT_UI_VOLUME_SET_OnClick(WidgetHdl);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_VOLUME_SET_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret;
    s32Ret = HI_GV_Widget_Active(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_VOLUME_SET_OnClick(WidgetHdl);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_VOLUME_SET_TrackbarOnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S * pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    HI_U32 u32CurValue;
    HI_S32 s32Ret;

    HI_APPCOMM_CHECK_POINTER(pstTouchEvent, HIGV_PROC_GOON);

    switch (pstTouchEvent->last.type)
    {
        case HIGV_TOUCH_END:
            if(s_bIsTracking)
            {
                s_bIsTracking = HI_FALSE;

                s32Ret = HI_GV_Track_GetCurVal(VOLUME_SET_TRACKBAR_VOLUME, &u32CurValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

                s32Ret = PDT_UI_VOLUME_SET_SetVolume(u32CurValue);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            }
            break;

        case HIGV_TOUCH_START:
            s_bIsTracking = HI_TRUE;
            break;

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_VOLUME_SET_TrackbarOnValueChange(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("%lld\n", (HI_S64)wParam);

    HI_U32 u32CurValue;
    HI_CHAR szVolume[16] = {};
    HI_S32 s32Ret;

    s32Ret = HI_GV_Track_GetCurVal(VOLUME_SET_TRACKBAR_VOLUME, &u32CurValue);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    snprintf(szVolume, 16, "%u%s", u32CurValue, "%");
    s32Ret = HI_GV_Widget_SetText(VOLUME_SET_LABEL_VOLUME, szVolume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Paint(VOLUME_SET_LABEL_VOLUME, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_VOLUME_SET_TrackbarOnMouseOut(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    memset(&touchEvent, 0x0, sizeof(touchEvent));
    touchEvent.last.id = 0;
    touchEvent.last.type = HIGV_TOUCH_END;
    HI_GV_Msg_SendAsyncWithData(VOLUME_SET_TRACKBAR_VOLUME, HIGV_MSG_TOUCH, &touchEvent, sizeof(HIGV_TOUCH_EVENT_S));

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

