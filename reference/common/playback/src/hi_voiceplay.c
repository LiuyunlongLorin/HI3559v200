/**
 * @file    hi_voiceplay.c
 * @brief   voiceplay module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include "hi_voiceplay.h"
#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>
#include "hi_queue.h"
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
#define HI_MODULE "VOICEPLAY"

#ifdef SUPPORT_VOICE_PLAY
#define VOICE_QUEUE_MAXLEN (3)
#define VOICE_TIME_INTERVAL_MS  (500)

typedef struct tagVOICEPLAY_CTX_S
{
    pthread_t taskid;
    pthread_mutex_t mutex;
    pthread_mutex_t playCrtlMutex;
    pthread_cond_t playCrtlCond;
    HI_VOICEPLAY_CFG_S stCfg;
    HI_HANDLE queueHdl;
    HI_BOOL bRunFlag;
} VOICEPLAY_CTX_S;

static VOICEPLAY_CTX_S s_stVOICEPLAYCtx =
{
    .taskid = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .playCrtlMutex = PTHREAD_MUTEX_INITIALIZER,
    .playCrtlCond = PTHREAD_COND_INITIALIZER,
    .queueHdl = 0,
    .bRunFlag = 0,
};

static HI_CHAR* VOICEPLAY_GetPath(HI_U32 u32Index)
{
    HI_S32 i = 0;

    MLOGI("u32Index=%d\n", u32Index);

    for (i = 0; i < s_stVOICEPLAYCtx.stCfg.u32MaxVoiceCnt; i++)
    {
        if (u32Index == s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i].u32VoiceIdx)
        {
            return s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i].aszFilePath;
        }
    }

    return NULL;
}

static HI_S32 VOICEPLAY_PlayToEof(HI_CHAR* pszPath)
{
    MLOGD("pszPath=%s\n", pszPath);
    HI_S32 s32Ret = 0;

    s32Ret = HI_LITEPLAYER_SetDataSource(PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY), pszPath);

    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_SetDataSource, s32Ret);
        goto end;
    }

    s32Ret = HI_LITEPLAYER_Prepare(PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY));

    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Prepare, s32Ret);
        goto end;
    }

    s32Ret = HI_LITEPLAYER_Play(PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY));

    if (s32Ret != HI_SUCCESS)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Play, s32Ret);
        goto end;
    }

    HI_MUTEX_LOCK(s_stVOICEPLAYCtx.playCrtlMutex);
    HI_COND_WAIT(s_stVOICEPLAYCtx.playCrtlCond, s_stVOICEPLAYCtx.playCrtlMutex);
    HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.playCrtlMutex);
    MLOGD("%s play over\n", pszPath);
end:
    HI_LITEPLAYER_Stop(PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY));
    return HI_SUCCESS;
}

static HI_VOID* VOICEPLAY_TaskQueue_Proc(HI_VOID* pvParam)
{
    HI_S32 s32Ret = 0;
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
    HI_HANDLE queue = s_stVOICEPLAYCtx.queueHdl;

    while (s_stVOICEPLAYCtx.bRunFlag)
    {
        if (0 == HI_QUEUE_GetLen(queue))
        {
            HI_usleep(VOICE_TIME_INTERVAL_MS * 1000);
            continue;
        }
        else
        {
            HI_VOICEPLAY_VOICE_S stVoice;
            s32Ret = HI_QUEUE_Pop(queue, &stVoice);

            if (HI_SUCCESS != s32Ret)
            {
                HI_LOG_PrintFuncErr(HI_QUEUE_Pop, s32Ret);
                continue;
            }

            HI_S32 i = 0;

            for (i = 0; i < stVoice.u32VoiceCnt; i++)
            {
                VOICEPLAY_PlayToEof(VOICEPLAY_GetPath(stVoice.au32VoiceIdx[i]));
            }
        }
    }

    return NULL;
}

static HI_VOID VOICEPLAY_PlayerEventProc(HI_MW_PTR pPlayer, HI_LITEPLAYER_EVENT_E enEvent, HI_VOID* pData)
{
    switch (enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
        {
            MLOGD("player state change to %d\n", *(HI_LITEPLAYER_STATE_E*)pData);
            if (*(HI_LITEPLAYER_STATE_E*)pData==HI_LITEPLAYER_STATE_IDLE)
            {
                break;
            }
            else
            {
                return;
            }
        }

        case HI_LITEPLAYER_EVENT_PROGRESS:
            return;

        case HI_LITEPLAYER_EVENT_EOF:
            MLOGD("the voice ends\n");
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
            MLOGE("player error\n");

        default:

            break;
    }

    HI_MUTEX_LOCK(s_stVOICEPLAYCtx.playCrtlMutex);
    HI_COND_SIGNAL(s_stVOICEPLAYCtx.playCrtlCond);
    HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.playCrtlMutex);
}

HI_S32 HI_VOICEPLAY_Init(const HI_VOICEPLAY_CFG_S* pstCfg)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(pstCfg->u32MaxVoiceCnt > 0, HI_EINVAL);
    HI_MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

    if (NULL != PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY))
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        return HI_EINITIALIZED;
    }

    PLAYER_CFG_S stPlayerCfg;
    stPlayerCfg.bEnableVideo = HI_FALSE;
    stPlayerCfg.bEnableAudio = HI_TRUE;
    memset(&stPlayerCfg.stPlayerParam, 0, sizeof(HI_LITEPLAYER_PARAM_S));
    stPlayerCfg.stPlayerParam.u32PlayPosNotifyIntervalMs = 500;
    stPlayerCfg.stAoutOpt = pstCfg->stAoutOpt;
    stPlayerCfg.pfnPlayerCallback = VOICEPLAY_PlayerEventProc;
    s32Ret = PLAYER_Create(PLAYER_TYPE_VOICEPLAY, &stPlayerCfg);

    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        HI_LOG_PrintFuncErr(PLAYER_Create, s32Ret);
        return HI_EINTER;
    }

    s_stVOICEPLAYCtx.queueHdl = HI_QUEUE_Create(sizeof(HI_VOICEPLAY_VOICE_S), VOICE_QUEUE_MAXLEN);

    if (s_stVOICEPLAYCtx.queueHdl == 0)
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        HI_LOG_PrintFuncErr(HI_QUEUE_Create, s32Ret);
        return HI_EINTER;
    }

    s_stVOICEPLAYCtx.bRunFlag = HI_TRUE;
    s32Ret = pthread_create(&s_stVOICEPLAYCtx.taskid, NULL, VOICEPLAY_TaskQueue_Proc, NULL);

    if (s32Ret != 0)
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        HI_LOG_PrintFuncErr(pthread_create, s32Ret);
        return HI_EINTER;
    }

    memcpy(&s_stVOICEPLAYCtx.stCfg, pstCfg, sizeof(HI_VOICEPLAY_CFG_S));

    s_stVOICEPLAYCtx.stCfg.pstVoiceTab = (HI_VOICEPLAY_VOICETABLE_S*)malloc(pstCfg->u32MaxVoiceCnt * sizeof(HI_VOICEPLAY_VOICETABLE_S));
    if (NULL == s_stVOICEPLAYCtx.stCfg.pstVoiceTab)
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        return HI_EINTER;
    }

    HI_S32 i = 0;

    for (i = 0; i < pstCfg->u32MaxVoiceCnt; i++)
    {
        s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i] = pstCfg->pstVoiceTab[i];
    }

    HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
    return HI_SUCCESS;
}

HI_S32 HI_VOICEPLAY_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY))
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        return HI_ENOINIT;
    }

    s_stVOICEPLAYCtx.bRunFlag = HI_FALSE;
    pthread_join(s_stVOICEPLAYCtx.taskid, NULL);

    HI_APPCOMM_SAFE_FREE(s_stVOICEPLAYCtx.stCfg.pstVoiceTab);
    HI_QUEUE_Destroy(s_stVOICEPLAYCtx.queueHdl);

    s32Ret = PLAYER_Destroy(PLAYER_TYPE_VOICEPLAY);

    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(PLAYER_Destroy, s32Ret);
    }

    HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
    return HI_SUCCESS;
}

HI_S32 HI_VOICEPLAY_Push(const HI_VOICEPLAY_VOICE_S* pstVoice, HI_U32 u32Timeout_ms)
{
    HI_S32 s32Ret = -1;
    HI_U32 u32Timewait_ms = 0;
    HI_APPCOMM_CHECK_POINTER(pstVoice, HI_EINVAL);
    HI_MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

    if (NULL == PLAYER_GetHdl(PLAYER_TYPE_VOICEPLAY))
    {
        HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
        return HI_ENOINIT;
    }

    s32Ret = HI_QUEUE_GetLen(s_stVOICEPLAYCtx.queueHdl);

    if (s32Ret == 0)
    {
        /*if queue is empty ,push voice into queue*/
        s32Ret = HI_QUEUE_Push(s_stVOICEPLAYCtx.queueHdl, pstVoice);
    }
    else if (s32Ret > 0)
    {
        /*if queue is not empty ,drop droppable voice*/
        if (pstVoice->bDroppable == HI_TRUE)
        {
            MLOGW("voice queue is busy, drop this voice.s32Ret=%d\n", s32Ret);
        }
        else
        {
            /*wait queue is not full*/
            while ((u32Timewait_ms < u32Timeout_ms) && (VOICE_QUEUE_MAXLEN == HI_QUEUE_GetLen(s_stVOICEPLAYCtx.queueHdl)))
            {
                MLOGW("voice queue is full, Waiting ......\n");
                HI_usleep(VOICE_TIME_INTERVAL_MS * 1000);
                u32Timewait_ms += VOICE_TIME_INTERVAL_MS;
            }

            s32Ret = HI_QUEUE_Push(s_stVOICEPLAYCtx.queueHdl, pstVoice);
        }
    }

    HI_MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
    return s32Ret;
}
#else
HI_S32 HI_VOICEPLAY_Init(const HI_VOICEPLAY_CFG_S* pstCfg)
{
    MLOGW("voiceplay is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_VOICEPLAY_Deinit(HI_VOID)
{
    MLOGW("voiceplay is not support.\n");
    return HI_SUCCESS;
}

HI_S32 HI_VOICEPLAY_Push(const HI_VOICEPLAY_VOICE_S* pstVoice, HI_U32 u32Timeout_ms)
{
    MLOGW("voiceplay is not support.\n");
    return HI_SUCCESS;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

