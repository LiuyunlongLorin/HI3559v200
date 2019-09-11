/**
 * @file    hi_playback.c
 * @brief   playback module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include "hi_playback.h"
#include <pthread.h>
#include "hi_math.h"
#include "hi_mapi_vproc.h"
#include "player_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "PLAYBACK"

#ifdef SUPPORT_PLAY_BACK
typedef struct tagPLAYBACK_CTX_S
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    HI_LITEPLAYER_STATE_E enState;
    HI_LITEPLAYER_TPLAY_ATTR_S stTrickPlayAttr;
    HI_PLAYBACK_CFG_S stCfg;
    HI_FORMAT_FILE_INFO_S stMediaInfo;
} PLAYBACK_CTX_S;

static PLAYBACK_CTX_S s_stPLAYBACKCtx =
{
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .enState = HI_LITEPLAYER_STATE_IDLE,
    .stTrickPlayAttr =
    {
        .enDirection = HI_LITEPLAYER_TPLAY_FORWARD,
        .enSpeed = HI_LITEPLAYER_PLAY_SPEED_BASE,
    }
};

static HI_VOID PLAYBACK_PlayerEventProc(HI_MW_PTR pPlayer, HI_LITEPLAYER_EVENT_E enEvent, HI_VOID* pData)
{
    switch (enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
        {
            MLOGI("player state change=%d->%d\n", s_stPLAYBACKCtx.enState, *(HI_LITEPLAYER_STATE_E*)pData);
            s_stPLAYBACKCtx.enState = *(HI_LITEPLAYER_STATE_E*)pData;
            break;
        }

        case HI_LITEPLAYER_EVENT_EOF:
            MLOGI("the film ends\n");
            break;

        case HI_LITEPLAYER_EVENT_PROGRESS:
            break;

        case HI_LITEPLAYER_EVENT_SEEK_END:
            HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);
            MLOGI("seek end\n");
            HI_COND_SIGNAL(s_stPLAYBACKCtx.cond);
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
        case HI_LITEPLAYER_EVENT_BUTT:
        default:
            MLOGE("player error\n");
            break;
    }

    if (s_stPLAYBACKCtx.stCfg.pfnEventProc)
    {
        s_stPLAYBACKCtx.stCfg.pfnEventProc(enEvent, pData, s_stPLAYBACKCtx.stCfg.pvUserData);
    }
}

/** function interface */
/**
 * @brief    init playback.
 * @param[in] pstCfg:playback configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Init(const HI_PLAYBACK_CFG_S* pstCfg)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_EINVAL);
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL != PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_EINITIALIZED;
    }

    PLAYER_CFG_S stPlayerCfg;
    stPlayerCfg.bEnableVideo = HI_TRUE;
    stPlayerCfg.bEnableAudio = HI_TRUE;
    stPlayerCfg.stPlayerParam = pstCfg->stPlayerParam;
    stPlayerCfg.stVoutOpt = pstCfg->stVoutOpt;
    stPlayerCfg.stAoutOpt = pstCfg->stAoutOpt;
    stPlayerCfg.pfnPlayerCallback = PLAYBACK_PlayerEventProc;
    s32Ret = PLAYER_Create(PLAYER_TYPE_PLAYBACK, &stPlayerCfg);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(PLAYER_Create, s32Ret);
        return HI_EINTER;
    }

    memcpy(&s_stPLAYBACKCtx.stCfg, pstCfg, sizeof(HI_PLAYBACK_CFG_S));
    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    deinit playback.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    s32Ret = PLAYER_Destroy(PLAYER_TYPE_PLAYBACK);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(PLAYER_Destroy, s32Ret);
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    get media file information.
 * @param[in] pszFilePath:the file path.
 * @param[out] pstMediaInfo:media information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetMediaInfo(const HI_CHAR* pszFilePath, HI_FORMAT_FILE_INFO_S* pstMediaInfo)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstMediaInfo, HI_EINVAL);
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    MLOGD("pszFilePath:%s\n", pszFilePath);
    s32Ret = HI_LITEPLAYER_SetDataSource(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), pszFilePath);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_SetDataSource, s32Ret);
        return HI_EINTER;
    }

    s32Ret = HI_LITEPLAYER_Prepare(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

    if (HI_SUCCESS != s32Ret)
    {
        HI_LITEPLAYER_Stop(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Prepare, s32Ret);
        return HI_EINTER;
    }

    s32Ret = HI_LITEPLAYER_GetFileInfo(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &s_stPLAYBACKCtx.stMediaInfo);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_GetFileInfo, s32Ret);
        return HI_EINTER;
    }

    HI_S32 i = 0;

    for (i = HI_DEMUXER_RESOLUTION_CNT - 1; i >= 0; i--)
    {
        if (0 < s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].u32Width * s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].u32Height)
        {
            MLOGD("Video(%d) type:%d, Res:%dx%d\n", i,
                  s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].enVideoType,
                  s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].u32Width,
                  s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].u32Height);
            s_stPLAYBACKCtx.stMediaInfo.s32UsedVideoStreamIndex = s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[i].s32VideoStreamIndex;
        }
    }

    if (0 <= s_stPLAYBACKCtx.stMediaInfo.s32UsedAudioStreamIndex)
    {
        MLOGD("Audio type:%d %d Hz, %d channels\n",
              s_stPLAYBACKCtx.stMediaInfo.enAudioType,
              s_stPLAYBACKCtx.stMediaInfo.u32SampleRate,
              s_stPLAYBACKCtx.stMediaInfo.u32AudioChannelCnt);
    }

    memcpy(pstMediaInfo, &s_stPLAYBACKCtx.stMediaInfo, sizeof(HI_FORMAT_FILE_INFO_S));
    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    MLOGD("File:%s size:%lld byte duration:%lld ms\n", pszFilePath, s_stPLAYBACKCtx.stMediaInfo.s64FileSize, s_stPLAYBACKCtx.stMediaInfo.s64Duration);
    return HI_SUCCESS;
}

/**
 * @brief    start play media file.
 * @param[in] s32VideoIdx:video stream index.
 * @param[in] s32AudioIdx:audio stream index.
 * @param[in] pstPlaySize:vpss port size in VoutTypeVpss, use default when NULL or w/h is 0.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Play(HI_S32 s32VideoIdx, HI_S32 s32AudioIdx, const SIZE_S* pstPlaySize)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_EXPR(HI_DEMUXER_RESOLUTION_CNT > s32VideoIdx, HI_EINVAL);
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    HI_LITEPLAYER_ATTR_S stMediaAttr;
    stMediaAttr.s32VidStreamId = s32VideoIdx;
    stMediaAttr.s32AudStreamId = s32AudioIdx;
    s32Ret = HI_LITEPLAYER_SetMedia(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &stMediaAttr);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_SetMedia, s32Ret);
        return HI_EINTER;
    }

    if (HI_LITEPLAYER_VOUT_TYPE_VPSS == s_stPLAYBACKCtx.stCfg.stVoutOpt.enVoutType &&
        HI_FORMAT_VIDEO_TYPE_BUTT != s_stPLAYBACKCtx.stMediaInfo.enVideoType)
    {
        HI_MAPI_VPORT_ATTR_S stVPortAttr;
        memset(&stVPortAttr, 0, sizeof(HI_MAPI_VPORT_ATTR_S));

        if (pstPlaySize && (0 != pstPlaySize->u32Width) && (0 != pstPlaySize->u32Height))
        {
            stVPortAttr.u32Width = pstPlaySize->u32Width;
            stVPortAttr.u32Height = pstPlaySize->u32Height;
        }
        else
        {
            stVPortAttr.u32Width = ALIGN_DOWN(s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[s32VideoIdx].u32Width, 2);
            stVPortAttr.u32Height = ALIGN_DOWN(s_stPLAYBACKCtx.stMediaInfo.stSteamResolution[s32VideoIdx].u32Height, 2);
        }

        stVPortAttr.stFrameRate.s32SrcFrameRate = -1;
        stVPortAttr.stFrameRate.s32DstFrameRate = -1;
        stVPortAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
        stVPortAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stVPortAttr.enCompressMode = COMPRESS_MODE_NONE;

        s32Ret = HI_MAPI_VPROC_SetPortAttr(s_stPLAYBACKCtx.stCfg.stVoutOpt.hModHdl, s_stPLAYBACKCtx.stCfg.stVoutOpt.hChnHdl, &stVPortAttr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_MAPI_VPROC_SetPortAttr, s32Ret);
            return HI_EINTER;
        }

        s32Ret = HI_MAPI_VPROC_StartPort(s_stPLAYBACKCtx.stCfg.stVoutOpt.hModHdl, s_stPLAYBACKCtx.stCfg.stVoutOpt.hChnHdl);

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_MAPI_VPROC_StartPort, s32Ret);
            return HI_EINTER;
        }
    }

    s32Ret = HI_LITEPLAYER_Play(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Play, s32Ret);
        return HI_EINTER;
    }

    if (HI_LITEPLAYER_PLAY_SPEED_BASE != s_stPLAYBACKCtx.stTrickPlayAttr.enSpeed
        && HI_FORMAT_VIDEO_TYPE_JPEG != s_stPLAYBACKCtx.stMediaInfo.enVideoType)
    {
        s32Ret = HI_LITEPLAYER_TPlay(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &s_stPLAYBACKCtx.stTrickPlayAttr);

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_TPlay, s32Ret);
            return HI_EINTER;
        }
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    stop player.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Stop(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    s32Ret = HI_LITEPLAYER_Stop(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Stop, s32Ret);
        return HI_EINTER;
    }

    if (HI_LITEPLAYER_VOUT_TYPE_VPSS == s_stPLAYBACKCtx.stCfg.stVoutOpt.enVoutType && HI_FORMAT_VIDEO_TYPE_BUTT != s_stPLAYBACKCtx.stMediaInfo.enVideoType)
    {
        s32Ret = HI_MAPI_VPROC_StopPort(s_stPLAYBACKCtx.stCfg.stVoutOpt.hModHdl, s_stPLAYBACKCtx.stCfg.stVoutOpt.hChnHdl);

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_MAPI_VPROC_StopPort, s32Ret);
            return HI_EINTER;
        }
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    pause player.
 * @return 0 success,non-zero error code.
 * @exception    just support pause when the play speed is normal.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Pause(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    if (HI_LITEPLAYER_STATE_PLAY == s_stPLAYBACKCtx.enState)
    {
        s32Ret = HI_LITEPLAYER_Pause(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_Pause, s32Ret);
            return HI_EINTER;
        }
    }
    else if (HI_LITEPLAYER_STATE_TPLAY == s_stPLAYBACKCtx.enState)
    {
        s32Ret = HI_LITEPLAYER_Play(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_Play, s32Ret);
            return HI_EINTER;
        }

        s32Ret = HI_LITEPLAYER_Pause(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_Pause, s32Ret);
            return HI_EINTER;
        }
    }
    else
    {
        MLOGD("player is not Play.State=%d\n", s_stPLAYBACKCtx.enState);
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    resume player.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Resume(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    if (HI_LITEPLAYER_STATE_PAUSE == s_stPLAYBACKCtx.enState)
    {
        s32Ret = HI_LITEPLAYER_Play(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_Play, s32Ret);
            return HI_EINTER;
        }

        if (HI_LITEPLAYER_PLAY_SPEED_BASE != s_stPLAYBACKCtx.stTrickPlayAttr.enSpeed)
        {
            s32Ret = HI_LITEPLAYER_TPlay(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &s_stPLAYBACKCtx.stTrickPlayAttr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
                HI_LOG_PrintFuncErr(HI_LITEPLAYER_TPlay, s32Ret);
                return HI_EINTER;
            }
        }
    }
    else
    {
        MLOGD("player is not Pause.State=%d\n", s_stPLAYBACKCtx.enState);
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    set play speed.
 * @param[in] enSpeed:play speed
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_E enSpeed)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    s_stPLAYBACKCtx.stTrickPlayAttr.enSpeed = enSpeed;

    if (HI_LITEPLAYER_STATE_PLAY == s_stPLAYBACKCtx.enState
        || HI_LITEPLAYER_STATE_TPLAY == s_stPLAYBACKCtx.enState)
    {
        if (HI_LITEPLAYER_PLAY_SPEED_BASE == enSpeed)
        {
            s32Ret = HI_LITEPLAYER_Play(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK));

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
                HI_LOG_PrintFuncErr(HI_LITEPLAYER_Play, s32Ret);
                return HI_EINTER;
            }
        }
        else
        {
            s32Ret = HI_LITEPLAYER_TPlay(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &s_stPLAYBACKCtx.stTrickPlayAttr);

            if (HI_SUCCESS != s32Ret)
            {
                HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
                HI_LOG_PrintFuncErr(HI_LITEPLAYER_TPlay, s32Ret);
                return HI_EINTER;
            }
        }
    }

    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    get play speed.
 * @param[out] penSpeed:play speed
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetSpeed(HI_LITEPLAYER_PLAY_SPEED_E* penSpeed)
{
    *penSpeed = s_stPLAYBACKCtx.stTrickPlayAttr.enSpeed;
    return HI_SUCCESS;
}

/**
 * @brief    seek.
 * @param[in] s64Time_ms:seek time in ms.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_Seek(HI_S64 s64Time_ms)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    if (s64Time_ms > s_stPLAYBACKCtx.stMediaInfo.s64Duration)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        MLOGD("seek input time beyond total time seektime: %lld, total time:%lld\n", s64Time_ms, s_stPLAYBACKCtx.stMediaInfo.s64Duration);
        return HI_EINVAL;
    }

    s32Ret = HI_LITEPLAYER_Seek(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), s64Time_ms);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Seek, s32Ret);
        return HI_EINTER;
    }

    MLOGD("wait seek end\n");
    HI_COND_WAIT(s_stPLAYBACKCtx.cond, s_stPLAYBACKCtx.mutex);
    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}

/**
 * @brief    get play states.
 * @param[out] penState:play states
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
HI_S32 HI_PLAYBACK_GetPlayState(HI_LITEPLAYER_STATE_E* penState)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stPLAYBACKCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK))
    {
        HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
        return HI_ENOINIT;
    }

    if (HI_LITEPLAYER_STATE_ERR <= s_stPLAYBACKCtx.enState)
    {
        s32Ret = HI_LITEPLAYER_GetPlayStatus(PLAYER_GetHdl(PLAYER_TYPE_PLAYBACK), &s_stPLAYBACKCtx.enState);

        if (HI_SUCCESS != s32Ret)
        {
            HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_GetPlayStatus, s32Ret);
            return HI_EINTER;
        }
    }

    *penState = s_stPLAYBACKCtx.enState;
    HI_MUTEX_UNLOCK(s_stPLAYBACKCtx.mutex);
    return HI_SUCCESS;
}
#else
HI_S32 HI_PLAYBACK_Init(const HI_PLAYBACK_CFG_S* pstCfg)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Deinit(HI_VOID)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_GetMediaInfo(const HI_CHAR* pszFilePath, HI_FORMAT_FILE_INFO_S* pstMediaInfo)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Play(HI_S32 s32VideoIdx, HI_S32 s32AudioIdx, const SIZE_S* pstPlaySize)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Stop(HI_VOID)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Pause(HI_VOID)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Resume(HI_VOID)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_SetSpeed(HI_LITEPLAYER_PLAY_SPEED_E enSpeed)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_GetSpeed(HI_LITEPLAYER_PLAY_SPEED_E* penSpeed)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_Seek(HI_S64 s64Time_ms)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_PLAYBACK_GetPlayState(HI_LITEPLAYER_STATE_E* penState)
{
    MLOGW("playback is not support.\n");
    return HI_SUCCESS;
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

