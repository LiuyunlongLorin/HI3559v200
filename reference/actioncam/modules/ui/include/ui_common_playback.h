/**
 * @file      ui_common_playback.h
 * @brief     ui common playback interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/12
 * @version   1.0

 */

#ifndef __UI_COMMON_PLAYBACK_H__
#define __UI_COMMON_PLAYBACK_H__

#include "ui_common_filelist.h"

#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


#define PLAYBACK_HIDE_UI_TIMER_INTERVAL_MS (5000)

typedef struct tagPDT_UI_COMM_PLAYBACK_SPEED_S
{
    HI_CHAR szPlaySpeed[8];
    HI_LITEPLAYER_PLAY_SPEED_E enSpeed;
} PDT_UI_COMM_PLAYBACK_SPEED_S;

typedef enum tagEVENT_UI_PLAYBACK_E
{
    PLAYBACK_EVENT_STATE_CHANGED,
    PLAYBACK_EVENT_PLAY_END,
    PLAYBACK_EVENT_PLAY_ERROR,
    PLAYBACK_EVENT_REFRESH_UI
} EVENT_UI_PLAYBACK_E;


HI_S32 PDT_UI_COMM_PLAYBACK_SetPlayTime(HIGV_HANDLE InfoHdl);
HI_VOID PDT_UI_COMM_PLAYBACK_ResetPlayTime();
HI_VOID PDT_UI_COMM_PLAYBACK_GetFileType(HI_FILEMNG_FILE_TYPE_E* penFileType);
HI_VOID PDT_UI_COMM_PLAYBACK_SetTrack(HIGV_HANDLE TrackHdl);
HI_S32 PDT_UI_COMM_PLAYBACK_Seek(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_E enWorkMode);
HI_S32 PDT_UI_COMM_PLAYBACK_GetVolume(HIGV_HANDLE BtnVolumeHdl, HI_S32* ps32Volume);
HI_S32 PDT_UI_COMM_PLAYBACK_SetVolume(HIGV_HANDLE AoHdl, HIGV_HANDLE BtnVolumeHdl, HI_S32 s32Volume);
HI_VOID PDT_UI_COMM_PLAYBACK_PlayerInit(HI_PDT_WORKMODE_E enWorkMode, HI_HANDLE AoHdl, HI_HANDLE AoChnHdl);
HI_VOID PDT_UI_COMM_PLAYBACK_PlayerDeinit(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_PlayFile(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_PlayPrevFile(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_PlayNextFile(HI_VOID);
HI_S32 PDT_UI_COMM_ChangePlayStatus(HI_VOID);
HI_S32 PDT_UI_COMM_ChangePlaySpeed(HI_HANDLE SpeedBtnHdl);
HI_S32 PDT_UI_COMM_PLAYBACK_DeleteFile(HIGV_HANDLE DialogHdl, HIGV_HANDLE PlaybackHdl);
HI_S32 PDT_UI_COMM_PLAYBACK_OnEventProc(HI_EVENT_S* pstEvent, HIGV_HANDLE WinHdl);
HI_S32 PDT_UI_COMM_PlayerEventProc(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pvData, HI_VOID* pvUserData);
HI_S32 PDT_UI_COMM_PLAYBACK_HandlePlayEndEvent(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_HandlePlayErrorEvent(HI_VOID);
HI_VOID PDT_UI_COMM_PLAYBACK_UpdateFileObjCnt(HI_VOID);
HI_S32 PDT_UI_COMM_PLAYBACK_UpdateFileInfo(HI_VOID);
HI_VOID PDT_UI_COMM_PLAYBACK_UpdateGrpIndex();
HI_S32 PDT_UI_COMM_PLAYBACK_UpdateGrpInfo(HI_VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

