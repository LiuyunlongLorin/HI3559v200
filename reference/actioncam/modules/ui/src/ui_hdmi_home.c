/**
 * @file      ui_himi_home.c
 * @brief     ui home in HDMI scene. live preview.
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

#include "hi_timer_ext.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


#define PDT_HDMI_HOME_INTERVAL_SEC (10)

static HI_S32 s_s32TimerGrp = -1;
static HI_MW_PTR s_pTimerHdl = 0;

static HI_VOID PDT_HDMI_HOME_HideNoticLabel(HI_VOID* pvClientData, struct timeval* pstNow)
{
    HI_S32 s32Ret;
    s32Ret = HI_GV_Widget_Hide(HDMI_HOME_NOTIC);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Hide");

    return;
}

static HI_S32 PDT_HDMI_HOME_CreateTimer(HI_VOID)
{
    HI_S32 s32Ret;

    if(s_s32TimerGrp < 0)
    {
        s_s32TimerGrp = HI_Timer_Init(HI_FALSE);
        HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO((s_s32TimerGrp >= 0), HI_FAILURE, "Timer_Init");
    }

    HI_TIMER_S  stDelayTimer;
    memset(&stDelayTimer, 0x0, sizeof(HI_TIMER_S));
    stDelayTimer.now = HI_NULL;
    stDelayTimer.interval_ms = PDT_HDMI_HOME_INTERVAL_SEC * 1000;
    stDelayTimer.periodic = HI_FALSE;
    stDelayTimer.timer_proc = PDT_HDMI_HOME_HideNoticLabel;
    stDelayTimer.clientData = HI_NULL;

    if(s_pTimerHdl != 0){
        s32Ret = HI_Timer_Destroy(s_s32TimerGrp, s_pTimerHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s_pTimerHdl = HI_Timer_Create(s_s32TimerGrp, &stDelayTimer);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO((s_pTimerHdl != 0), HI_FAILURE, "Timer_Create");

    if(!HI_GV_Widget_IsShow(HDMI_HOME_NOTIC))
    {
        s32Ret = HI_GV_Widget_Show(HDMI_HOME_NOTIC);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Paint(HDMI_HOME_NOTIC, HI_NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_HDMI_HOME_DestroyTimer(HI_VOID)
{
    HI_S32 s32Ret;

    if(s_pTimerHdl != 0)
    {
        s32Ret = HI_Timer_Destroy(s_s32TimerGrp, s_pTimerHdl);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Timer_Destroy");
        s_pTimerHdl = 0;
    }

    if(s_s32TimerGrp != -1)
    {
        s32Ret = HI_Timer_DeInit(s_s32TimerGrp);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Timer_DeInit");
        s_s32TimerGrp = -1;
    }

    if(HI_GV_Widget_IsShow(HDMI_HOME_NOTIC))
    {
        s32Ret = HI_GV_Widget_Hide(HDMI_HOME_NOTIC);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Hide");
    }

    return HI_SUCCESS;
}


/*State Machine Msg Result Callback*/
static HI_S32 PDT_HDMI_HOME_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{

    return HI_SUCCESS;
}

static HI_S32 PDT_HDMI_HOME_PrepareToFilelist(HI_VOID)
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

    s32Ret = PDT_HDMI_HOME_DestroyTimer();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DestroyTimer");

    s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_FILELIST, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = HI_PDT_WORKMODE_HDMI_PLAYBACK;

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_HDMI_HOME_OnReceiveMsgResult);
    if(s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        PDT_UI_WINMNG_StartWindow(HDMI_HOME, HI_TRUE);
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_HDMI_PLAYBACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HI_SUCCESS;
}

/** Process events from EVENTHUB*/
HI_S32 PDT_HDMI_HOME_EventProc(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = 0;
    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1, pstEvent->arg2,pstEvent->s32Result);

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

            break;

        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_FILEMNG_SPACE_FULL:
        case HI_EVENT_STATEMNG_SD_AVAILABLE:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            s32Ret = PDT_HDMI_HOME_CreateTimer();
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "CreateTimer");
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_HOME_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");

    HI_S32 s32Ret;

    PDT_UI_COMM_SdIsReady();
    s32Ret = PDT_HDMI_HOME_CreateTimer();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "CreateTimer");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_HOME_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");

    HI_S32 s32Ret;
    s32Ret = PDT_HDMI_HOME_DestroyTimer();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "DestroyTimer");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_HOME_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_COMM_HDMIEventProc((HI_EVENT_S *)lParam);
    if(s32Ret)
    {
        MLOGE("s32Ret %x\n", s32Ret);
    }

    return HIGV_PROC_GOON;

}

HI_S32 PDT_HDMI_HOME_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    HI_S32 s32Ret = HI_SUCCESS;
    if (PDT_UI_KEY_BACK == wParam || PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_HDMI_HOME_CreateTimer();
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "CreateTimer");
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        if(HI_TRUE == PDT_UI_COMM_SdIsReady())
        {
            s32Ret = PDT_HDMI_HOME_PrepareToFilelist();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        }
        else
        {
            s32Ret = PDT_HDMI_HOME_CreateTimer();
            PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "CreateTimer");
        }
    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_HDMI_HOME_OnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 s32StartX, s32StartY, s32EndX, s32EndY;
    HI_S32 s32Ret = HI_SUCCESS;

    s32StartX = pstGestureEvent->gesture.fling.start.x;
    s32StartY = pstGestureEvent->gesture.fling.start.y;
    s32EndX   = pstGestureEvent->gesture.fling.end.x;
    s32EndY   = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", s32StartX, s32StartY, s32EndX, s32EndY);

    if (abs(s32StartX - s32EndX) > abs(s32StartY - s32EndY))
    {
        if (s32StartX < s32EndX)
        {
            MLOGD("fling right\n");
            if(HI_TRUE == PDT_UI_COMM_SdIsReady())
            {
                s32Ret = PDT_HDMI_HOME_PrepareToFilelist();
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            }
            else
            {
                s32Ret = PDT_HDMI_HOME_CreateTimer();
                PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "CreateTimer");
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

