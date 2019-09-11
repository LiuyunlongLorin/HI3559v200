/**
 * @file      ui_mode_page.c
 * @brief     ui mode page, change work mode.
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


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_S32 PDT_UI_MODE_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_WINMNG_FinishWindow(MODE_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_MODE_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;

    switch (WidgetHdl)
    {
        case MODE_BUTTON_PHOTO_NORMAL_BG:
        case MODE_BUTTON_PHOTO_NORMAL:
        case MODE_LABEL_PHOTO_NORMAL:
            stMessage.arg2 = HI_PDT_WORKMODE_SING_PHOTO;
            break;

        case MODE_BUTTON_PHOTO_DELAY_BG:
        case MODE_BUTTON_PHOTO_DELAY:
        case MODE_LABEL_PHOTO_DELAY:
            stMessage.arg2 = HI_PDT_WORKMODE_DLAY_PHOTO;
            break;

        case MODE_BUTTON_MULTI_BURST_BG:
        case MODE_BUTTON_MULTI_BURST:
        case MODE_LABEL_MULTI_BURST:
            stMessage.arg2 = HI_PDT_WORKMODE_BURST;
            break;

        case MODE_BUTTON_MULTI_LAPSE_BG:
        case MODE_BUTTON_MULTI_LAPSE:
        case MODE_LABEL_MULTI_LAPSE:
            stMessage.arg2 = HI_PDT_WORKMODE_LPSE_PHOTO;
            break;

        case MODE_BUTTON_RECORD_NORMAL_BG:
        case MODE_BUTTON_RECORD_NORMAL:
        case MODE_LABEL_RECORD_NORMAL:
            stMessage.arg2 = HI_PDT_WORKMODE_NORM_REC;
            break;

        case MODE_BUTTON_RECORD_LOOP_BG:
        case MODE_BUTTON_RECORD_LOOP:
        case MODE_LABEL_RECORD_LOOP:
            stMessage.arg2 = HI_PDT_WORKMODE_LOOP_REC;
            break;

        case MODE_BUTTON_RECORD_SLOW_BG:
        case MODE_BUTTON_RECORD_SLOW:
        case MODE_LABEL_RECORD_SLOW:
            stMessage.arg2 = HI_PDT_WORKMODE_SLOW_REC;
            break;

        case MODE_BUTTON_RECORD_LAPSE_BG:
        case MODE_BUTTON_RECORD_LAPSE:
        case MODE_LABEL_RECORD_LAPSE:
            stMessage.arg2 = HI_PDT_WORKMODE_LPSE_REC;
            break;

        case MODE_BUTTON_RECORD_SNAP_BG:
        case MODE_BUTTON_RECORD_SNAP:
        case MODE_LABEL_RECORD_SNAP:
            stMessage.arg2 = HI_PDT_WORKMODE_RECSNAP;
            break;

        case MODE_BUTTON_BACK:
            s32Ret = PDT_UI_WINMNG_FinishWindow(MODE_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;

        default:
            MLOGE("widget %d click not processed\n", WidgetHdl);
            return HI_FAILURE;
    }

    s32Ret =  PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_MODE_OnReceiveMsgResult);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_MODE_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {};
    HI_U32 a_u32Mode2Button[][2] =
    {
        {HI_PDT_WORKMODE_NORM_REC,   MODE_BUTTON_RECORD_NORMAL_BG},
        {HI_PDT_WORKMODE_LOOP_REC,   MODE_BUTTON_RECORD_LOOP_BG},
        {HI_PDT_WORKMODE_LPSE_REC,   MODE_BUTTON_RECORD_LAPSE_BG},
        {HI_PDT_WORKMODE_SLOW_REC,   MODE_BUTTON_RECORD_SLOW_BG},
        {HI_PDT_WORKMODE_RECSNAP,    MODE_BUTTON_RECORD_SNAP_BG},
        {HI_PDT_WORKMODE_SING_PHOTO, MODE_BUTTON_PHOTO_NORMAL_BG},
        {HI_PDT_WORKMODE_DLAY_PHOTO, MODE_BUTTON_PHOTO_DELAY_BG},
        {HI_PDT_WORKMODE_LPSE_PHOTO, MODE_BUTTON_MULTI_LAPSE_BG},
        {HI_PDT_WORKMODE_BURST,      MODE_BUTTON_MULTI_BURST_BG},
    };

    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    for(i=0; i<ARRAY_SIZE(a_u32Mode2Button); i++)
    {
        if(stWorkModeState.enWorkMode == a_u32Mode2Button[i][0])
        {
            s32Ret = HI_GV_Widget_Active(a_u32Mode2Button[i][1]);
            HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
            return HIGV_PROC_GOON;
        }
    }

    MLOGE("not found work mode %u\n", stWorkModeState.enWorkMode);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_MODE_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("widget:%d, pointer[%d,%d]\n", WidgetHdl, ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.x,
        ((HIGV_GESTURE_EVENT_S*)lParam)->gesture.tap.pointer.y);

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_MODE_BUTTON_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_MODE_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE aModeBtnHdl[] = {
        MODE_BUTTON_PHOTO_NORMAL_BG,
        MODE_BUTTON_PHOTO_DELAY_BG,
        MODE_BUTTON_MULTI_BURST_BG,
        MODE_BUTTON_MULTI_LAPSE_BG,
        MODE_BUTTON_RECORD_NORMAL_BG,
        MODE_BUTTON_RECORD_LOOP_BG,
        MODE_BUTTON_RECORD_SLOW_BG,
        MODE_BUTTON_RECORD_LAPSE_BG,
        MODE_BUTTON_RECORD_SNAP_BG,
        MODE_BUTTON_BACK
    };

    if(PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(MODE_PAGE);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_COMM_ActiveNextWidget(aModeBtnHdl, ARRAY_SIZE(aModeBtnHdl),WidgetHdl);
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
       s32Ret = PDT_UI_MODE_BUTTON_OnClick(WidgetHdl);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

