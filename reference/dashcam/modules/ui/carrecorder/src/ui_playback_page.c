/**
* @file    ui_playback_page.c
* @brief   playback video and photo.
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
#include <math.h>
#include "hi_mapi_disp.h"
#include "ui_common.h"
#include "hi_appcomm_util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define TIMER_PLAYBACK_HIDE_UI (0x20180123)
#define PLAYBACK_HIDE_UI_TIMER_INTERVAL_MS (5000)

typedef struct tagPDT_UI_PLAYBACK_SPEED_S
{
    HI_CHAR szPlaySpeed[8];
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed;
} PDT_UI_PLAYBACK_SPEED_S;

typedef enum tagEVENT_UI_PLAYBACK_E
{
    PLAYBACK_EVENT_PLAY_END,
    PLAYBACK_EVENT_PLAY_ERROR,
    PLAYBACK_EVENT_REFRESH_UI
} EVENT_UI_PLAYBACK_E;

static HI_DTCF_DIR_E  s_enDir = DTCF_DIR_BUTT;
static HI_U32 s_u32CurFileIdx = 0;
static HI_U32 s_u32FileCnt = 0;
static HI_CHAR s_szCurFileAbsPath[HI_APPCOMM_MAX_PATH_LEN];
static HI_U32  s_u32PlayTime_ms = 0;
static HI_U32  s_u32FileTotalTime_ms = 0;
static HI_BOOL s_bShowUI = HI_TRUE;
static HIGV_HANDLE s_ActiveBtnHdl = PLAYBACK_BUTTON_NEXT;


HI_U32 PDT_UI_PLAYBACK_MillisecondToSecond(HI_U32 u32Time_ms)
{
    return u32Time_ms/1000 + (u32Time_ms%1000>500 ? 1 : 0);
}

HI_S32 PDT_UI_PlAYBACK_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szPlayTime[16] = {};
    HIGV_HANDLE aPlaybackBtnHdl[] = {
        PLAYBACK_BUTTON_SPEED,
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_PLAY,
        PLAYBACK_BUTTON_NEXT
    };

    if( ! PDT_UI_WINMNG_WindowIsShow(PLAYBACK_PAGE) )
    {
        MLOGD("Playback page hided, no need to refresh\n");
        return HI_SUCCESS;
    }

    if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
    {
        snprintf(szPlayTime, 16, "%02d/%02d", s_u32CurFileIdx+1, s_u32FileCnt);
    }
    else
    {
        snprintf(szPlayTime, 16, "%02d:%02d/%02d:%02d",
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms)/60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms)%60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms)/60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms)%60);
    }
    s32Ret = HI_GV_Widget_SetText(PLAYBACK_BUTTON_INFO, szPlayTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (!s_bShowUI)
    {
        HI_S32 i;
        for (i=0; i<UI_ARRAY_SIZE(aPlaybackBtnHdl); i++)
        {
            s32Ret = HI_GV_Widget_Hide(aPlaybackBtnHdl[i]);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }

        s32Ret = HI_GV_Widget_Paint(PLAYBACK_PAGE, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_SUCCESS;
    }

    s32Ret = HI_GV_Widget_Show(PLAYBACK_BUTTON_DELETE);
    s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_PREV);
    s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_NEXT);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
    {
        s32Ret = HI_GV_Widget_Hide(PLAYBACK_BUTTON_PLAY);
        s32Ret |= HI_GV_Widget_Hide(PLAYBACK_BUTTON_SPEED);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s32Ret = HI_GV_Widget_Show(PLAYBACK_BUTTON_SPEED);
        s32Ret |= HI_GV_Widget_Show(PLAYBACK_BUTTON_PLAY);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if (HI_LITEPLAYER_STATE_PLAY == enPlayState || HI_LITEPLAYER_STATE_TPLAY == enPlayState
            || HI_LITEPLAYER_STATE_PREPARED == enPlayState)
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL, SKIN_BUTTON_PAUSE);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE, SKIN_BUTTON_PAUSE_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL, SKIN_BUTTON_PLAY);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE, SKIN_BUTTON_PLAY_ACTIVE);
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
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_GV_Widget_Active");

    s32Ret = HI_GV_Widget_Paint(PLAYBACK_PAGE, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_GV_Widget_Paint");
}

HI_S32 PDT_UI_PLAYBACK_HandlePlayEndEvent(HI_VOID)
{
    if(DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        HI_S32 s32Ret = HI_SUCCESS;
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s_u32PlayTime_ms = 0;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_HandlePlayErrorEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_u32PlayTime_ms = s_u32FileTotalTime_ms;

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_PlayerEventProc(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pvData, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
            MLOGD("HI_LITEPLAYER_EVENT_STATE_CHANGED:%d\n", *(HI_LITEPLAYER_STATE_E*)pvData);
            s32Ret = HI_GV_Msg_SendAsync(PLAYBACK_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_REFRESH_UI, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_EOF:
            s32Ret = HI_GV_Msg_SendAsync(PLAYBACK_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_END, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_SEEK_END:
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
            s32Ret = HI_GV_Msg_SendAsync(PLAYBACK_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_ERROR, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_PROGRESS:
            if(DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
            {
                s_u32PlayTime_ms = *((HI_U32 *)pvData);
                s32Ret = HI_GV_Msg_SendAsync(PLAYBACK_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_REFRESH_UI, HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        default:
            MLOGD("Event ID %u not need to be processed here\n", enEvent);
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_UpdateFileInfo(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_SetSearchScope(&s_enDir, 1, &s_u32FileCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (s_u32CurFileIdx >= s_u32FileCnt)
    {
        s_u32CurFileIdx = 0;
    }

    MLOGD("CurGrpIdx:%u, GroupCnt:%u \n", s_u32CurFileIdx, s_u32FileCnt);

    if (0 == s_u32FileCnt)
    {
        MLOGE("file count:%d \n", s_u32FileCnt);
        return HI_FAILURE;
    }

    s32Ret = HI_FILEMNG_GetFileByIndex(s_u32CurFileIdx, s_szCurFileAbsPath, sizeof(s_szCurFileAbsPath));
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    MLOGD("CurFileIdx:%d, GrpFileCnt:%d \n", s_u32CurFileIdx, s_u32FileCnt);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_ClearVoBuf(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_WORKMODE_CFG_S stCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_PLAYBACK, &stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_HANDLE DispHdl = stCfg.unModeCfg.stPlayBackCfg.stVoutOpt.hModHdl;
    HI_HANDLE WndHdl = stCfg.unModeCfg.stPlayBackCfg.stVoutOpt.hChnHdl;
    s32Ret = HI_MAPI_DISP_ClearWindow(DispHdl, WndHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_PlayFile(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;

    HI_FORMAT_FILE_INFO_S stMediaInfo = {};

    s32Ret = HI_PLAYBACK_GetPlayState( &enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if( HI_LITEPLAYER_STATE_PLAY == enPlayState
        || HI_LITEPLAYER_STATE_PAUSE == enPlayState
        || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = HI_PLAYBACK_GetMediaInfo(s_szCurFileAbsPath, &stMediaInfo);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PLAYBACK_GetMediaInfo");
    s_u32FileTotalTime_ms = (HI_U32)stMediaInfo.s64Duration;

    PDT_UI_PLAYBACK_ClearVoBuf();

    MLOGI("pszFilePath:%s\n", s_szCurFileAbsPath);
    s32Ret = HI_PLAYBACK_Play(stMediaInfo.s32UsedVideoStreamIndex, stMediaInfo.s32UsedAudioStreamIndex, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_DelOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;

    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (HI_LITEPLAYER_STATE_PLAY == enPlayState
        || HI_LITEPLAYER_STATE_TPLAY == enPlayState
        || HI_LITEPLAYER_STATE_PAUSE == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (1 < s_u32FileCnt)
    {
        s32Ret = HI_FILEMNG_RemoveFile(s_szCurFileAbsPath);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HI_async();

        s_u32FileCnt--;
        if (s_u32CurFileIdx >= s_u32FileCnt)
        {
            s_u32CurFileIdx = s_u32FileCnt-1;
        }

        s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        PDT_UI_FILELIST_SetCurFileIdx(UI_FILELIST_INVALID_FILE_IDX);

        s32Ret = HI_FILEMNG_RemoveFile(s_szCurFileAbsPath);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        HI_async();

        s32Ret = PDT_UI_WINMNG_FinishWindow(PLAYBACK_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_PrevOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (0 == s_u32CurFileIdx)
    {
        if (0 == s_u32CurFileIdx)
        {
            s_u32CurFileIdx = s_u32FileCnt-1;
        }
        else
        {
            s_u32CurFileIdx -= 1;
        }

        s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s_u32CurFileIdx -= 1;
        s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = PDT_UI_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_NextOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_u32FileCnt-1 == s_u32CurFileIdx)
    {
        s_u32CurFileIdx = 0;

        s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s_u32CurFileIdx += 1;
        s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    s32Ret = PDT_UI_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_PlayOnClick(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;

    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    if (HI_LITEPLAYER_STATE_PLAY == enPlayState || HI_LITEPLAYER_STATE_TPLAY == enPlayState
        || HI_LITEPLAYER_STATE_PREPARED == enPlayState)
    {
        MLOGD("HI_PLAYBACK_Pause\n");
        s32Ret = HI_PLAYBACK_Pause();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else if (HI_LITEPLAYER_STATE_PAUSE == enPlayState)
    {
        MLOGD("HI_PLAYBACK_Resume\n");
        s32Ret = HI_PLAYBACK_Resume();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        MLOGD("PDT_UI_PLAYBACK_PlayFile\n");
        s32Ret = PDT_UI_PLAYBACK_PlayFile();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_SpeedOnClick(HI_VOID)
{
   HI_S32 s32Ret = HI_SUCCESS;

   PDT_UI_PLAYBACK_SPEED_S stSpeedItems[ ]= {
        {"x2",   HI_LITEPLAYER_PLAY_SPEED_2X_FAST},      /* 2 x speed fast */
        {"x4",   HI_LITEPLAYER_PLAY_SPEED_4X_FAST},      /* 4 x speed fast */
        {"x8",   HI_LITEPLAYER_PLAY_SPEED_8X_FAST},      /* 8 x speed fast */
        {"x16",  HI_LITEPLAYER_PLAY_SPEED_16X_FAST},     /* 16 x speed fast */
        {"x32",  HI_LITEPLAYER_PLAY_SPEED_32X_FAST},     /* 32 x speed fast */
        {"x64",  HI_LITEPLAYER_PLAY_SPEED_64X_FAST},     /* 64 x speed fast */
        {"x128", HI_LITEPLAYER_PLAY_SPEED_128X_FAST},    /* 128 x speed fast */
        {"x1",   HI_LITEPLAYER_PLAY_SPEED_BASE}
    };

    HI_LITEPLAYER_PLAY_SPEED_E enSpeed = HI_LITEPLAYER_PLAY_SPEED_BUTT;
    s32Ret = HI_PLAYBACK_GetSpeed(&enSpeed);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_S32 i;
    HI_S32 s32NextIdx;
    HI_S32 s32ArraySize = UI_ARRAY_SIZE(stSpeedItems);
    for (i=0; i<s32ArraySize; i++)
    {
        if (stSpeedItems[i].enSpeed == enSpeed)
        {
            MLOGD(" Current Play Speed : %s \n",stSpeedItems[i].szPlaySpeed);
            s32NextIdx = (i+1)%s32ArraySize;
            enSpeed = stSpeedItems[s32NextIdx].enSpeed;

            s32Ret = HI_PLAYBACK_SetSpeed(enSpeed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_GV_Widget_SetText(PLAYBACK_BUTTON_SPEED, stSpeedItems[s32NextIdx].szPlaySpeed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_GV_Widget_Paint(PLAYBACK_BUTTON_SPEED, NULL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            return HI_SUCCESS;
        }

    }

    MLOGE("not support the play speed:%d \n",enSpeed);

    return HI_FAILURE;
}

static HI_S32 PDT_UI_PLAYBACK_BUTTON_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (WidgetHdl)
    {
        case PLAYBACK_BUTTON_SPEED:
            s32Ret = PDT_UI_PLAYBACK_SpeedOnClick();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case PLAYBACK_BUTTON_DELETE:
            s32Ret = PDT_UI_PLAYBACK_DelOnClick();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case PLAYBACK_BUTTON_PREV:
            s32Ret = HI_GV_Widget_Active(PLAYBACK_BUTTON_PREV);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Ret = PDT_UI_PLAYBACK_PrevOnClick();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case PLAYBACK_BUTTON_PLAY:
            s32Ret = PDT_UI_PLAYBACK_PlayOnClick();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case PLAYBACK_BUTTON_NEXT:
            s32Ret = HI_GV_Widget_Active(PLAYBACK_BUTTON_NEXT);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Ret = PDT_UI_PLAYBACK_NextOnClick();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        default:
            break;
    }

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    s_ActiveBtnHdl = PLAYBACK_BUTTON_NEXT;

    PDT_UI_FILELIST_GetSearchScope(&s_enDir);
    PDT_UI_FILELIST_GetCurFileIdx(&s_u32CurFileIdx);

    if (DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        s32Ret = HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_BASE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = HI_GV_Widget_SetText(PLAYBACK_BUTTON_SPEED, "x1");
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Timer_Create(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI, PLAYBACK_HIDE_UI_TIMER_INTERVAL_MS);
    s32Ret |= HI_GV_Timer_Start(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_PLAYBACK_ShowUI();

    s32Ret = PDT_UI_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_PLAYBACK_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_GV_Timer_Destroy(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);


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

HI_S32 PDT_UI_PLAYBACK_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    switch(wParam)
    {
        case PLAYBACK_EVENT_PLAY_END:
            s32Ret = PDT_UI_PLAYBACK_HandlePlayEndEvent();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        case PLAYBACK_EVENT_REFRESH_UI:
            s32Ret = PDT_UI_PlAYBACK_Refresh();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        case PLAYBACK_EVENT_PLAY_ERROR:
            s32Ret = PDT_UI_PLAYBACK_HandlePlayErrorEvent();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        default:
            MLOGE("unhandled event: %#lx !\n", (HI_UL)wParam);
            break;
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
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_Active");
            s_ActiveBtnHdl = aWidgetHdl[u32NextIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}

static HI_S32 PDT_UI_PLAYBACK_ActivePreviousWidget(HI_HANDLE aWidgetHdl[], HI_U32 u32ArraySize, HI_HANDLE CurWidgetHdl)
{
    HI_U32 u32PreviousIndex = 0;
    HI_U32 u32CurIndex = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for (u32CurIndex = 0;  u32CurIndex < u32ArraySize; u32CurIndex++)
    {
        if (CurWidgetHdl == aWidgetHdl[u32CurIndex])
        {
            u32PreviousIndex = u32CurIndex ? (u32CurIndex - 1) : (u32ArraySize - 1);
            s32Ret = HI_GV_Widget_Active( aWidgetHdl[u32PreviousIndex]);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_GV_Widget_Active");
            s_ActiveBtnHdl = aWidgetHdl[u32PreviousIndex];
            return HI_SUCCESS;
        }
    }

    MLOGE("not found the given widget %d, NO widget activated\n", CurWidgetHdl);
    return HI_FAILURE;
}

HI_S32 PDT_UI_PLAYBACK_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE aPlaybackHdlRec[] = {
        PLAYBACK_BUTTON_SPEED,
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_PLAY,
        PLAYBACK_BUTTON_NEXT
    };

    HIGV_HANDLE aPlaybackHdlPhoto[] = {
        PLAYBACK_BUTTON_DELETE,
        PLAYBACK_BUTTON_PREV,
        PLAYBACK_BUTTON_NEXT
    };


    if (!s_bShowUI)
    {
        PDT_UI_PLAYBACK_ShowUI();

        s32Ret = HI_GV_Timer_Reset(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        return HIGV_PROC_STOP;
    }


    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        s_u32PlayTime_ms = 0;

        PDT_UI_FILELIST_SetCurFileIdx(s_u32CurFileIdx);
        s32Ret = PDT_UI_WINMNG_FinishWindow(PLAYBACK_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        return HIGV_PROC_STOP;
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        if (DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
        {
            s32Ret = PDT_UI_PLAYBACK_ActiveNextWidget(aPlaybackHdlRec,UI_ARRAY_SIZE(aPlaybackHdlRec),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
        else if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
        {
            s32Ret = PDT_UI_PLAYBACK_ActiveNextWidget(aPlaybackHdlPhoto,UI_ARRAY_SIZE(aPlaybackHdlPhoto),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        if (DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
        {
            s32Ret = PDT_UI_PLAYBACK_ActivePreviousWidget(aPlaybackHdlRec,UI_ARRAY_SIZE(aPlaybackHdlRec),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
        else if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
        {
            s32Ret = PDT_UI_PLAYBACK_ActivePreviousWidget(aPlaybackHdlPhoto,UI_ARRAY_SIZE(aPlaybackHdlPhoto),WidgetHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        }
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_PLAYBACK_BUTTON_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);
        if(PLAYBACK_BUTTON_PREV == WidgetHdl || PLAYBACK_BUTTON_NEXT == WidgetHdl)
        {
            s_u32PlayTime_ms = 0;
        }
    }

    s32Ret = HI_GV_Timer_Reset(PLAYBACK_PAGE, TIMER_PLAYBACK_HIDE_UI);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_STOP);

    return HIGV_PROC_STOP;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

