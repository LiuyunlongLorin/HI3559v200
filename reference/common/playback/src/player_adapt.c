/**
 * @file    player_adapt.c
 * @brief   player adapt interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include "player_adapt.h"
#include "hi_player.h"
#include <string.h>
#include <pthread.h>
#include "hi_liteplayer_err.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "PLAYER"

#define FFMPEG_DEMUXER  (HI_APP_LIB_PATH"/libffmpeg_demuxer.so")
#ifdef AMP_LINUX_HUAWEILITE
#define AAC_DECODER     (HI_SHAREFS_ROOT_PATH"/libhiaacdec.so")
#else
#define AAC_DECODER     (HI_APP_LIB_PATH"/libhiaacdec.so")
#endif

typedef struct tagPLAYER_CTX_S
{
    HI_MW_PTR pPlayer;
    HI_BOOL bAvailable;
    PLAYER_CFG_S stPlayerCfg;
} PLAYER_CTX_S;

typedef struct tagPLAYERMNG_CTX_S
{
    pthread_mutex_t mutex;
    HI_BOOL bInit;
    PLAYER_CTX_S stPlayer[PLAYER_TYPE_BUTT];
} PLAYERMNG_CTX_S;

static PLAYERMNG_CTX_S s_PlayerCtx =
{
    .bInit = HI_FALSE,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

HI_S32 PLAYER_Destroy(PLAYER_TYPE_E enType)
{
    HI_S32 s32Ret = 0;
    if (enType>=PLAYER_TYPE_BUTT)
    {
        MLOGE("invalid player type.\n");
        return HI_EINTER;
    }
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    s32Ret = HI_LITEPLAYER_Destroy(s_PlayerCtx.stPlayer[enType].pPlayer);
    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Destroy, s32Ret);
    }
    s_PlayerCtx.stPlayer[enType].pPlayer = NULL;
    s_PlayerCtx.stPlayer[enType].bAvailable = HI_FALSE;
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
}

HI_S32 PLAYER_Create(PLAYER_TYPE_E enType,const PLAYER_CFG_S* pstPlayerCfg)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pstPlayerCfg, HI_EINVAL);
    if (enType>=PLAYER_TYPE_BUTT)
    {
        MLOGE("invalid player type.\n");
        return HI_EINTER;
    }
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    s32Ret = HI_LITEPLAYER_Create(&s_PlayerCtx.stPlayer[enType].pPlayer, &pstPlayerCfg->stPlayerParam);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Create, s32Ret);
        return HI_EINTER;
    }

    if(HI_TRUE==pstPlayerCfg->bEnableVideo)
    {
        s32Ret = HI_LITEPLAYER_SetVOHandle(s_PlayerCtx.stPlayer[enType].pPlayer, (HI_LITEPLAYER_VOUT_OPT_S*)&pstPlayerCfg->stVoutOpt);
        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_SetVOHandle, s32Ret);
            goto err;
        }
    }

    if(HI_TRUE==pstPlayerCfg->bEnableAudio)
    {
        s32Ret = HI_LITEPLAYER_SetAOHandle(s_PlayerCtx.stPlayer[enType].pPlayer, (HI_LITEPLAYER_AOUT_OPT_S*)&pstPlayerCfg->stAoutOpt);
        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_SetAOHandle, s32Ret);
            goto err;
        }
    }

    if(pstPlayerCfg->pfnPlayerCallback)
    {
        s32Ret = HI_LITEPLAYER_RegCallback(s_PlayerCtx.stPlayer[enType].pPlayer, pstPlayerCfg->pfnPlayerCallback);
        if (HI_SUCCESS != s32Ret)
        {
            HI_LOG_PrintFuncErr(HI_LITEPLAYER_RegCallback, s32Ret);
            goto err;
        }
    }

    memcpy(&s_PlayerCtx.stPlayer[enType].stPlayerCfg,pstPlayerCfg,sizeof(PLAYER_CFG_S));
    s_PlayerCtx.stPlayer[enType].bAvailable = HI_TRUE;
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
err:
    HI_LITEPLAYER_Destroy(s_PlayerCtx.stPlayer[enType].pPlayer);
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_EINTER;
}

HI_VOID* PLAYER_GetHdl(PLAYER_TYPE_E enType)
{
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    if(enType>=PLAYER_TYPE_BUTT)
    {
        return NULL;
    }
    HI_MW_PTR pPlayer = s_PlayerCtx.stPlayer[enType].bAvailable?s_PlayerCtx.stPlayer[enType].pPlayer:NULL;
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return (HI_VOID*)pPlayer;
}

HI_S32 PLAYER_Deinit(HI_VOID)
{
    HI_LITEPLAYER_UnRegisterDemuxer(FFMPEG_DEMUXER);
    HI_LITEPLAYER_UnRegisterAudioDecoder(AAC_DECODER);
    HI_LITEPLAYER_DeInit();
    return HI_SUCCESS;
}

HI_S32 HI_PLAYER_Deinit(HI_VOID)
{
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    if(s_PlayerCtx.bInit == HI_FALSE)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        MLOGW("Not inited\n");
        return HI_ENOINIT;
    }

    HI_S32 i=0;
    for (i=0;i<PLAYER_TYPE_BUTT;i++)
    {
        if (s_PlayerCtx.stPlayer[i].pPlayer!=NULL)
        {
            s_PlayerCtx.stPlayer[i].bAvailable = HI_FALSE;
            HI_LITEPLAYER_Destroy(s_PlayerCtx.stPlayer[i].pPlayer);
            s_PlayerCtx.stPlayer[i].pPlayer = NULL;
        }
    }
    PLAYER_Deinit();
    s_PlayerCtx.bInit = HI_FALSE;
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
}

HI_S32 HI_PLAYER_Init(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    if(s_PlayerCtx.bInit == HI_TRUE)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        MLOGW("Already Initialized\n");
        return HI_EINITIALIZED;
    }

    memset(s_PlayerCtx.stPlayer,0,PLAYER_TYPE_BUTT*sizeof(PLAYER_CTX_S));
    s32Ret = HI_LITEPLAYER_Init();
    if (HI_SUCCESS != s32Ret)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_Init, s32Ret);
        return HI_EINTER;
    }

    s32Ret = HI_LITEPLAYER_RegisterDemuxer(FFMPEG_DEMUXER, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_RegisterDemuxer, s32Ret);
        goto err;
    }

    s32Ret = HI_LITEPLAYER_RegisterAudioDecoder(AAC_DECODER);
    if (HI_SUCCESS != s32Ret)
    {
        HI_LOG_PrintFuncErr(HI_LITEPLAYER_RegisterAudioDecoder, s32Ret);
        return HI_EINTER;
    }
    s_PlayerCtx.bInit = HI_TRUE;
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
err:
    PLAYER_Deinit();
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_EINTER;
}

HI_S32 HI_PLAYER_RegAdec(HI_VOID)
{
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    if(s_PlayerCtx.bInit == HI_FALSE)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        return HI_ENOINIT;
    }
    HI_S32 i=0;
    for (i=0;i<PLAYER_TYPE_BUTT;i++)
    {
        if (s_PlayerCtx.stPlayer[i].pPlayer!=NULL)
        {
            HI_LITEPLAYER_Stop(s_PlayerCtx.stPlayer[i].pPlayer);
        }
    }
    HI_LITEPLAYER_RegisterAudioDecoder(AAC_DECODER);
    for (i=0;i<PLAYER_TYPE_BUTT;i++)
    {
        if (s_PlayerCtx.stPlayer[i].pPlayer!=NULL)
        {
            s_PlayerCtx.stPlayer[i].bAvailable = HI_TRUE;
        }
    }
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
}

HI_S32 HI_PLAYER_UnRegAdec(HI_VOID)
{
    HI_MUTEX_LOCK(s_PlayerCtx.mutex);
    if(s_PlayerCtx.bInit == HI_FALSE)
    {
        HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
        return HI_ENOINIT;
    }
    HI_S32 i=0;
    for (i=0;i<PLAYER_TYPE_BUTT;i++)
    {
        if (s_PlayerCtx.stPlayer[i].pPlayer!=NULL)
        {
            s_PlayerCtx.stPlayer[i].bAvailable = HI_FALSE;
            HI_LITEPLAYER_Stop(s_PlayerCtx.stPlayer[i].pPlayer);
        }
    }
    HI_LITEPLAYER_UnRegisterAudioDecoder(AAC_DECODER);
    HI_MUTEX_UNLOCK(s_PlayerCtx.mutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

