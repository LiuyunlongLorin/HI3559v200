/**
 * @file      ui_common_playback.c
 * @brief     ui common playback function code
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/12
 * @version   1.0

 */
#include "hi_mapi_disp.h"
#include "ui_common_playback.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PDT_UI_PLAYBACK_LCD_MAX_WIDTH  1280

static HI_U32 s_u32CurGrpIdx = INVALID_GROUP_INDEX;
static HI_U32 s_u32GrpCnt = 0;
static HI_U32 s_u32FileIdx = 0;
static HI_U32 s_u32GrpFileCnt = 0;

static HI_U32  s_u32PlayTime_ms = 0;
static HI_U32  s_u32FileTotalTime_ms = 0;

static HI_S32 s_s32Volume = -1;

static HI_BOOL s_bEndOfFile = HI_FALSE;
static HI_CHAR s_szCurFileAbsPath[HI_APPCOMM_MAX_PATH_LEN];

static HI_FILEMNG_FILE_TYPE_E s_enFileType = HI_FILEMNG_FILE_TYPE_BUTT;
static HI_PDT_WORKMODE_E s_enWorkMode = HI_PDT_WORKMODE_PLAYBACK;

HI_S32 PDT_UI_COMM_PLAYBACK_ClearVoBuf(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = 0;
    HI_HANDLE WndHdl = 0;
    s32Ret = PDT_UI_COMM_GetAVHdl(s_enWorkMode, NULL, NULL, &DispHdl, &WndHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s32Ret = HI_MAPI_DISP_ClearWindow(DispHdl, WndHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

static HI_U32 PDT_UI_COMM_PLAYBACK_MillisecondToSecond(HI_U32 u32Time_ms)
{
    return u32Time_ms / 1000 + (u32Time_ms % 1000 > 500 ? 1 : 0);
}

HI_S32 PDT_UI_COMM_PLAYBACK_SetPlayTime(HIGV_HANDLE InfoHdl)
{
    HI_CHAR szPlayTime[16] = {};
    HI_S32 s32Ret;

    if (HI_FILEMNG_FILE_TYPE_RECORD == s_enFileType)
    {
        snprintf(szPlayTime, 16, "%02d:%02d/%02d:%02d",
            PDT_UI_COMM_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms) / 60,
            PDT_UI_COMM_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms) % 60,
            PDT_UI_COMM_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms) / 60,
            PDT_UI_COMM_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms) % 60);
    }
    else if (HI_FILEMNG_FILE_TYPE_PHOTO == s_enFileType)
    {
        snprintf(szPlayTime, 16, "%02d/%02d", s_u32FileIdx + 1, s_u32GrpFileCnt);
    }
    s32Ret = HI_GV_Widget_SetText(InfoHdl, szPlayTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return s32Ret;
}

HI_VOID PDT_UI_COMM_PLAYBACK_ResetPlayTime(HI_VOID)
{
    s_u32PlayTime_ms = 0;
    s_u32FileTotalTime_ms = 0;
    s_u32FileIdx = 0;
    s_u32FileTotalTime_ms = 0;

    return;
}


HI_VOID PDT_UI_COMM_PLAYBACK_GetFileType(HI_FILEMNG_FILE_TYPE_E* penFileType)
{
    *penFileType = s_enFileType;
}

HI_VOID PDT_UI_COMM_PLAYBACK_SetTrack(HIGV_HANDLE TrackHdl)
{
    if(HI_FILEMNG_FILE_TYPE_RECORD == s_enFileType)
    {
        HI_S32 s32Ret = HI_SUCCESS;
        s32Ret = HI_GV_Track_SetRange(PLAYBACK_TRACKBAR_PLAY, 0, s_u32FileTotalTime_ms > 0 ? s_u32FileTotalTime_ms : 100);
        s32Ret |= HI_GV_Track_SetCurVal(PLAYBACK_TRACKBAR_PLAY, s_u32PlayTime_ms);
        PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "SetTrack");
    }
}

HI_S32 PDT_UI_COMM_PLAYBACK_Seek(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurValue;
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed = HI_LITEPLAYER_PLAY_SPEED_BUTT;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;

    s32Ret = HI_PLAYBACK_GetSpeed(&enSpeed);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(HI_LITEPLAYER_PLAY_SPEED_BASE == enSpeed)
    {
        HI_GV_Track_GetCurVal(PLAYBACK_TRACKBAR_PLAY, &u32CurValue);
        s_u32PlayTime_ms = u32CurValue;

        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if(HI_LITEPLAYER_STATE_IDLE != enPlayState)
        {
            s32Ret = HI_PLAYBACK_Seek((HI_S64)u32CurValue);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret;
    HI_HANDLE AoHdl = HI_INVALID_HANDLE;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &s_s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_COMM_GetAVHdl(enWorkMode, &AoHdl, NULL, NULL, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = HI_PDT_MEDIA_SetAoVolume(AoHdl, s_s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}


HI_S32 PDT_UI_COMM_PLAYBACK_GetVolume(HIGV_HANDLE BtnVolumeHdl, HI_S32* ps32Volume)
{
    HI_S32 s32Ret;
    HI_CHAR szVolume[16] = {};

    snprintf(szVolume, 16, "%d%s", s_s32Volume, "%");
    s32Ret = HI_GV_Widget_SetText(BtnVolumeHdl, szVolume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    *ps32Volume = s_s32Volume;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_SetVolume(HIGV_HANDLE AoHdl, HIGV_HANDLE BtnVolumeHdl, HI_S32 s32Volume)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szVolume[16] = {};

    s32Ret = HI_PDT_MEDIA_SetAoVolume(AoHdl, s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    snprintf(szVolume, 16, "%d%s", s32Volume, "%");
    s32Ret = HI_GV_Widget_SetText(BtnVolumeHdl, szVolume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s_s32Volume = s32Volume;

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_PLAYBACK_PlayerInit(HI_PDT_WORKMODE_E enWorkMode, HI_HANDLE AoHdl, HI_HANDLE AoChnHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    HI_PLAYBACK_CFG_S stPlayBackCfg;

    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enWorkMode, &stWorkModeCfg);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_PDT_PARAM_GetWorkModeCfg failed: %#x\n", s32Ret);
    }

    stPlayBackCfg.pfnEventProc = PDT_UI_COMM_PlayerEventProc;
    stPlayBackCfg.pvUserData = NULL;
    if (HI_PDT_WORKMODE_HDMI_PLAYBACK == enWorkMode)
    {
        memcpy(&stPlayBackCfg.stPlayerParam,
            &stWorkModeCfg.unModeAttr.stHDMIPlaybackAttr.stPlayerParam,
            sizeof(HI_LITEPLAYER_PARAM_S));
        memcpy(&stPlayBackCfg.stVoutOpt,
            &stWorkModeCfg.unModeAttr.stHDMIPlaybackAttr.stVoutOpt,
            sizeof(HI_LITEPLAYER_VOUT_OPT_S));
        memcpy(&stPlayBackCfg.stAoutOpt,
            &stWorkModeCfg.unModeAttr.stHDMIPlaybackAttr.stAoutOpt,
            sizeof(HI_LITEPLAYER_AOUT_OPT_S));
    }
    else if (HI_PDT_WORKMODE_PLAYBACK == enWorkMode)
    {
        memcpy(&stPlayBackCfg.stPlayerParam,
            &stWorkModeCfg.unModeAttr.stPlaybackAttr.stPlayerParam,
            sizeof(HI_LITEPLAYER_PARAM_S));
        memcpy(&stPlayBackCfg.stVoutOpt,
            &stWorkModeCfg.unModeAttr.stPlaybackAttr.stVoutOpt,
            sizeof(HI_LITEPLAYER_VOUT_OPT_S));
        memcpy(&stPlayBackCfg.stAoutOpt,
            &stWorkModeCfg.unModeAttr.stPlaybackAttr.stAoutOpt,
            sizeof(HI_LITEPLAYER_AOUT_OPT_S));
    }
    MLOGI(YELLOW"player config interval_ms[%d] video_buff[%d] audio_buff[%d] pause_mode[%d]"NONE"\n",
        stPlayBackCfg.stPlayerParam.u32PlayPosNotifyIntervalMs,
        stPlayBackCfg.stPlayerParam.u32VideoEsBufSize,
        stPlayBackCfg.stPlayerParam.u32AudioEsBufSize,
        stPlayBackCfg.stPlayerParam.bPauseMode);
    MLOGI(YELLOW"VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]"NONE"\n",
        stPlayBackCfg.stVoutOpt.enVoutType, stPlayBackCfg.stVoutOpt.hModHdl, stPlayBackCfg.stVoutOpt.hChnHdl);
    MLOGI(YELLOW"DevHdl[%d] TrackHdl[%d]"NONE"\n",
        stPlayBackCfg.stAoutOpt.hAudDevHdl, stPlayBackCfg.stAoutOpt.hAudTrackHdl);

    s32Ret = HI_PLAYBACK_Init(&stPlayBackCfg);

    if (HI_SUCCESS != s32Ret && HI_EINITIALIZED != s32Ret)
    {
        MLOGE("Playback init failed[%#x]\n", s32Ret);
    }

    s_enWorkMode = enWorkMode;
}

HI_VOID PDT_UI_COMM_PLAYBACK_PlayerDeinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Stop");
    s_u32PlayTime_ms = 0;

    s32Ret = HI_PLAYBACK_Deinit();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "Deinit");

    PDT_UI_COMM_FILELIST_SetCurGrpIdx(s_u32CurGrpIdx);
}

HI_S32 PDT_UI_COMM_PLAYBACK_PlayFile(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
    HI_FORMAT_FILE_INFO_S stMediaInfo = {};

    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if( HI_LITEPLAYER_STATE_PLAY == enPlayState
        || HI_LITEPLAYER_STATE_PAUSE == enPlayState
        || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s_u32PlayTime_ms = 0;
    }

    s32Ret = HI_PLAYBACK_GetMediaInfo(s_szCurFileAbsPath, &stMediaInfo);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    MLOGI(YELLOW"UsedVideoStreamIndex[%d] UsedAudioStreamIndex[%d]"NONE"\n",
        stMediaInfo.s32UsedVideoStreamIndex, stMediaInfo.s32UsedAudioStreamIndex);
    HI_S32 s32SteamResolutionIndex = 0;
    HI_S32 i = 0;
    for (i = 0; i < HI_DEMUXER_RESOLUTION_CNT; i++)
    {
        MLOGI("stSteamResolution[%d][%ux%u], "
            "s32VideoStreamIndex[%d], enVideoType[%d]\n", i,
            stMediaInfo.stSteamResolution[i].u32Width,
            stMediaInfo.stSteamResolution[i].u32Height,
            stMediaInfo.stSteamResolution[i].s32VideoStreamIndex,
            stMediaInfo.stSteamResolution[i].enVideoType);

        if(stMediaInfo.s32UsedVideoStreamIndex ==
            stMediaInfo.stSteamResolution[i].s32VideoStreamIndex)
        {
            s32SteamResolutionIndex = i;
        }
    }
    MLOGI(YELLOW"s32SteamResolutionIndex[%d]"NONE"\n", s32SteamResolutionIndex);

    MLOGI(GREEN"\nFileAbsPath = %s \nFileSize    = %lldMB \nDuration    = %lldms \n"
        "Resolution  = %u * %u \nFrameRate   = %.2f \nBitrate     = %uKb/s\n"NONE,
        s_szCurFileAbsPath, stMediaInfo.s64FileSize>>20, stMediaInfo.s64Duration,
        stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Width,
        stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Height,
        stMediaInfo.fFrameRate, stMediaInfo.u32Bitrate>>10);
    s_u32FileTotalTime_ms = (HI_U32)stMediaInfo.s64Duration;

    s32Ret = PDT_UI_COMM_PLAYBACK_ClearVoBuf();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    SIZE_S stPlaySize = {0, 0};
    if (HI_PDT_WORKMODE_HDMI_PLAYBACK == s_enWorkMode)
    {
        s32Ret = HI_PDT_MEDIA_GetHDMIDispSize(&stPlaySize);
        if (HI_SUCCESS != s32Ret)
        {
            stPlaySize.u32Width = 0;
            stPlaySize.u32Height = 0;
        }
    }
    else
    {
        stPlaySize.u32Width = PDT_UI_PLAYBACK_LCD_MAX_WIDTH;
        stPlaySize.u32Height = stPlaySize.u32Width *
            stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Height /
            stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Width;
        stPlaySize.u32Height = HI_APPCOMM_ALIGN(stPlaySize.u32Height, 2);
    }
    MLOGI(YELLOW"playsize[%ux%u]"NONE"\n", stPlaySize.u32Width, stPlaySize.u32Height);

    if ((stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Height < stPlaySize.u32Height) ||
        (stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Width < stPlaySize.u32Width) )
    {
        stPlaySize.u32Width = stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Width;
        stPlaySize.u32Height = stMediaInfo.stSteamResolution[s32SteamResolutionIndex].u32Height;
    }
    MLOGI(YELLOW"playsize[%ux%u]"NONE"\n", stPlaySize.u32Width, stPlaySize.u32Height);

    s32Ret = HI_PLAYBACK_Play(stMediaInfo.s32UsedVideoStreamIndex,
        stMediaInfo.s32UsedAudioStreamIndex, &stPlaySize);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_PlayPrevFile(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_u32PlayTime_ms = 0;

    if (0 == s_u32FileIdx)
    {
        if (0 == s_u32CurGrpIdx)
        {
            s_u32CurGrpIdx = s_u32GrpCnt-1;
        }
        else
        {
            s_u32CurGrpIdx -= 1;
        }

        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateGrpInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s_u32FileIdx -= 1;
        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_PlayNextFile(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_u32PlayTime_ms = 0;

    if (s_u32GrpFileCnt-1 == s_u32FileIdx)
    {
        if (s_u32CurGrpIdx < s_u32GrpCnt-1)
        {
            s_u32CurGrpIdx += 1;
        }
        else
        {
            s_u32CurGrpIdx = 0;
        }

        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateGrpInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        s_u32FileIdx += 1;
        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_ChangePlayStatus(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
#if 0
    if(HI_TRUE == s_bEndOfFile)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s_u32PlayTime_ms = 0;
    }
#endif

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
        s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s_bEndOfFile = HI_FALSE;

        if(s_u32PlayTime_ms != s_u32FileTotalTime_ms)
        {
            s32Ret = HI_PLAYBACK_Seek((HI_S64)s_u32PlayTime_ms);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_ChangePlaySpeed(HI_HANDLE SpeedBtnHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_S32 s32NextIdx;
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed = HI_LITEPLAYER_PLAY_SPEED_BUTT;
    PDT_UI_COMM_PLAYBACK_SPEED_S stSpeedItems[ ] =
    {
        {"x2",   HI_LITEPLAYER_PLAY_SPEED_2X_FAST},      /* 2 x speed fast */
        {"x4",   HI_LITEPLAYER_PLAY_SPEED_4X_FAST},      /* 4 x speed fast */
        {"x8",   HI_LITEPLAYER_PLAY_SPEED_8X_FAST},      /* 8 x speed fast */
        {"x16",  HI_LITEPLAYER_PLAY_SPEED_16X_FAST},     /* 16 x speed fast */
        {"x32",  HI_LITEPLAYER_PLAY_SPEED_32X_FAST},     /* 32 x speed fast */
        {"x64",  HI_LITEPLAYER_PLAY_SPEED_64X_FAST},     /* 64 x speed fast */
        {"x128", HI_LITEPLAYER_PLAY_SPEED_128X_FAST},    /* 128 x speed fast */
        {"x1",   HI_LITEPLAYER_PLAY_SPEED_BASE}
    };
    HI_S32 s32ArraySize = ARRAY_SIZE(stSpeedItems);

    s32Ret = HI_PLAYBACK_GetSpeed(&enSpeed);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    for (i=0; i<s32ArraySize; i++)
    {
        if (stSpeedItems[i].enSpeed == enSpeed)
        {
            MLOGD(" Current Play Speed : %s \n",stSpeedItems[i].szPlaySpeed);
            s32NextIdx = (i+1)%s32ArraySize;
            enSpeed = stSpeedItems[s32NextIdx].enSpeed;

            s32Ret = HI_PLAYBACK_SetSpeed(enSpeed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_GV_Widget_SetText(SpeedBtnHdl, stSpeedItems[s32NextIdx].szPlaySpeed);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = HI_GV_Widget_Paint(SpeedBtnHdl, NULL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            return HI_SUCCESS;
        }
    }
    MLOGE("not support the play speed:%d \n",enSpeed);
    return HI_FAILURE;
}

HI_S32 PDT_UI_COMM_PLAYBACK_DeleteFile(HIGV_HANDLE DialogHdl, HIGV_HANDLE PlaybackHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;

    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (HI_LITEPLAYER_STATE_PLAY == enPlayState
        || HI_LITEPLAYER_STATE_PAUSE == enPlayState
        || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if(1 < s_u32GrpFileCnt)
    {
        s32Ret = HI_FILEMNG_RemoveFile(s_szCurFileAbsPath);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s_u32GrpFileCnt--;
     if(s_u32FileIdx >= s_u32GrpFileCnt)
     {
        s_u32FileIdx = s_u32GrpFileCnt-1;
     }
        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateFileInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    else
    {
        if (1 == s_u32GrpCnt)
        {
            s32Ret = HI_FILEMNG_DelGrpsFwd(s_u32CurGrpIdx, 1);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s_u32CurGrpIdx = INVALID_GROUP_INDEX;

            PDT_UI_COMM_FILELIST_SetCurGrpIdx(s_u32CurGrpIdx);

            s32Ret = PDT_UI_WINMNG_FinishWindow(DialogHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

            s32Ret = PDT_UI_WINMNG_FinishWindow(PlaybackHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            return HI_SUCCESS;
        }
        s32Ret = HI_FILEMNG_DelGrpsFwd(s_u32CurGrpIdx, 1);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = PDT_UI_COMM_PLAYBACK_UpdateGrpInfo();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    s32Ret = PDT_UI_COMM_PLAYBACK_PlayFile();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_OnEventProc(HI_EVENT_S* pstEvent, HIGV_HANDLE WinHdl)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);
    HI_S32 s32Ret = HI_SUCCESS;

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            PDT_UI_COMM_PLAYBACK_PlayerDeinit();
            s32Ret = PDT_UI_WINMNG_FinishWindow(WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;

        default:
            MLOGD("Event ID %u not need to be processed here\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_COMM_PlayerEventProc(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pvData, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE WinHdl = INVALID_HANDLE;
    MLOGD("EventID = %u\n", enEvent);

    if(PDT_UI_WINMNG_WindowIsShow(PLAYBACK_PAGE))
    {
        WinHdl = PLAYBACK_PAGE;
    }
    else if(PDT_UI_WINMNG_WindowIsShow(HDMI_PLAYBACK_PAGE))
    {
        WinHdl = HDMI_PLAYBACK_PAGE;
    }
    else
    {
        return HI_SUCCESS;
    }

    switch(enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
            s32Ret = HI_GV_Msg_SendAsync(WinHdl, HIGV_MSG_EVENT, PLAYBACK_EVENT_STATE_CHANGED, HI_TRUE);
            break;

        case HI_LITEPLAYER_EVENT_EOF:
            s32Ret = HI_GV_Msg_SendAsync(WinHdl, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_END, HI_TRUE);
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
            s32Ret = HI_GV_Msg_SendAsync(WinHdl, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_ERROR, HI_TRUE);
            break;

        case HI_LITEPLAYER_EVENT_PROGRESS:
            s_u32PlayTime_ms = *((HI_U32 *)pvData);
            s32Ret = HI_GV_Msg_SendAsync(WinHdl, HIGV_MSG_EVENT, PLAYBACK_EVENT_REFRESH_UI, HI_TRUE);
            break;

        default:
            return HI_SUCCESS;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_HandlePlayEndEvent(HI_VOID)
{
    HI_S32 s32Ret;
    s_bEndOfFile = HI_TRUE;
    if(HI_FILEMNG_FILE_TYPE_PHOTO==s_enFileType)
    {
        s32Ret = HI_PLAYBACK_Pause();
    }
    else
    {
        s32Ret = HI_PLAYBACK_Stop();
        s_u32PlayTime_ms = s_u32FileTotalTime_ms;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_HandlePlayErrorEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_u32PlayTime_ms = s_u32FileTotalTime_ms;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_UpdateGrpFileObjCntCB(const HI_DCF_WRAPPER_GRP_S* pstGrp, HI_VOID* pvUserData)
{
    *(HI_U32 *)pvUserData = pstGrp->u16ObjCnt;

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_PLAYBACK_UpdateFileObjCnt(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_SrchGrpsFwd(s_u32CurGrpIdx, 1, PDT_UI_COMM_PLAYBACK_UpdateGrpFileObjCntCB, &s_u32GrpFileCnt);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SrchGrpsFwd");
    MLOGD("GrpFileCnt = %d\n",s_u32GrpFileCnt);
}

HI_S32 PDT_UI_COMM_PLAYBACK_UpdateFileInfoCB(const HI_DCF_WRAPPER_GRP_S* pstGrp, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8ObjTypIdx;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg = {};
    HI_FILEMNG_OBJ_FILENAME_S stFileNames = {};

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_FILEMNG_GetObjFilenames(s_u32FileIdx, &stFileNames);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s_u32GrpFileCnt = pstGrp->u16ObjCnt;
    u8ObjTypIdx = pstGrp->u8ObjTypIdx;

    memset(s_szCurFileAbsPath, 0, sizeof(s_szCurFileAbsPath));

    HI_U32 u32FileIdx = 0;
    for(u32FileIdx = 0; u32FileIdx < stFileNames.u8FileCnt; ++ u32FileIdx)
    {
        if(strstr(stFileNames.szFileName[u32FileIdx], "JPG") != NULL)
        {
            break;
        }
    }

    if(u32FileIdx >= stFileNames.u8FileCnt)
    {
        u32FileIdx = 0;
        /** TODO:This group does not contain JPG file */
    }

    snprintf(s_szCurFileAbsPath, HI_APPCOMM_MAX_PATH_LEN, "%s", stFileNames.szFileName[u32FileIdx]);

    if(HI_FILEMNG_FILE_TYPE_RECORD == stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].enType)
    {
        s_enFileType = HI_FILEMNG_FILE_TYPE_RECORD;
    }
    else if(HI_FILEMNG_FILE_TYPE_PHOTO == stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].enType)
    {
        s_enFileType = HI_FILEMNG_FILE_TYPE_PHOTO;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_PLAYBACK_UpdateFileInfo()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_SrchGrpsFwd(s_u32CurGrpIdx, 1, PDT_UI_COMM_PLAYBACK_UpdateFileInfoCB, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_PLAYBACK_UpdateGrpIndex()
{
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&s_u32CurGrpIdx);
}

HI_S32 PDT_UI_COMM_PLAYBACK_UpdateGrpInfo()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_GetGrpCnt(&s_u32GrpCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (s_u32CurGrpIdx >= s_u32GrpCnt)
    {
        s_u32CurGrpIdx = 0;
    }

    MLOGD("CurGrpIdx:%u, GroupCnt:%u \n", s_u32CurGrpIdx, s_u32GrpCnt);
    PDT_UI_COMM_PLAYBACK_UpdateFileObjCnt();

    if (0 == s_u32GrpFileCnt)
    {
        MLOGE("file count = %d \n", s_u32GrpFileCnt);
        return HI_FAILURE;
    }

    s_u32FileIdx = 0;
    s32Ret = PDT_UI_COMM_PLAYBACK_UpdateFileInfo();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    MLOGD("CurFileIdx:%d, GrpFileCnt:%d \n", s_u32FileIdx, s_u32GrpFileCnt);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

