/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_ao.c
 * @brief   NDK ao server functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "acodec.h"
#include "hi_comm_aio.h"
#include "mpi_audio.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_ao_define.h"
#include "mapi_ao_inner.h"
#include "adpt_audio_acodec.h"
#include "mapi_comm_inner.h"
#include "mapi_ao_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static MAPI_AO_CONTEXT_S g_astAoContext[HI_MAPI_AO_DEV_MAX_NUM];
static HI_BOOL g_bAoInited = HI_FALSE; /* module inited flag */
static pthread_mutex_t g_aoFuncLock[HI_MAPI_AO_DEV_MAX_NUM] = { PTHREAD_MUTEX_INITIALIZER };

HI_S32 MAPI_AO_Init(HI_VOID)
{
    HI_S32 i = 0;

    /* check state */
    if (g_bAoInited == HI_TRUE) {
        return HI_SUCCESS;
    }

    /* init context and load inner param */
    for (i = 0; i < HI_MAPI_AO_DEV_MAX_NUM; i++) {
        HAL_MAPI_AO_InitContext(&g_astAoContext[i]);
    }

    /* open audio codec file */
    ADPT_ACODEC_Open();

    /* init module */
    g_bAoInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 MAPI_AO_Deinit(HI_VOID)
{
    HI_S32 i = 0;

    /* check state */
    if (g_bAoInited != HI_TRUE) {
        return HI_SUCCESS;
    }
    for (i = 0; i < HI_MAPI_AO_DEV_MAX_NUM; i++) {
        if (g_astAoContext[i].enAoState != MAPI_AO_UNINITED) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO %d is not uninited , media can not deinit! try deinit %d \n", i, i);
            HI_S32 s32Ret = HI_MAPI_AO_Deinit(i);
            if (s32Ret != HI_SUCCESS) {
                return s32Ret;
            }
        }
    }

    /* deinit context */
    for (i = 0; i < HI_MAPI_AO_DEV_MAX_NUM; i++) {
        memset(&g_astAoContext[i], 0, sizeof(MAPI_AO_CONTEXT_S));
        g_astAoContext[i].enAoState = MAPI_AO_BUTT;
    }

    /* close audio codec file */
    ADPT_ACODEC_Close();

    /* deinit module */
    g_bAoInited = HI_FALSE;
    return HI_SUCCESS;
}

static HI_VOID MAPI_AO_SetExternAttr(AIO_ATTR_S *pstAioAttr)
{
    if (pstAioAttr->enI2sType == AIO_I2STYPE_EXTERN) {
        pstAioAttr->u32ClkSel = 1;
        pstAioAttr->u32ChnCnt = 1;
    }
}

static HI_S32 MAPI_AO_ConfigAttr(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S *pstAoAttr, AIO_ATTR_S *pstAioAttr)
{
    memcpy(pstAioAttr, &g_astAoContext[AoHdl].stAoAttr, sizeof(AIO_ATTR_S));
    switch (pstAoAttr->enSampleRate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_48000:
            pstAioAttr->enSamplerate = pstAoAttr->enSampleRate;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "do not support this sampleRate:%d error.\n", pstAoAttr->enSampleRate);
            return HI_MAPI_AO_EILLPARAM;
    }
    switch (pstAoAttr->enBitwidth) {
        case AUDIO_BIT_WIDTH_16:
            pstAioAttr->enBitwidth = pstAoAttr->enBitwidth;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "not support bitwidth\n");
            return HI_MAPI_AO_EILLPARAM;
    }
    pstAioAttr->enWorkmode = pstAoAttr->enWorkmode;
    switch (pstAoAttr->enSoundMode) {
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            pstAioAttr->enSoundmode = AUDIO_SOUND_MODE_MONO;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
            pstAioAttr->enSoundmode = AUDIO_SOUND_MODE_STEREO;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "enSoundMode %d is error.\n", pstAoAttr->enSoundMode);
            return HI_MAPI_AO_EILLPARAM;
    }

    pstAioAttr->u32PtNumPerFrm = pstAoAttr->u32PtNumPerFrm;
    pstAioAttr->enI2sType = pstAoAttr->enI2sType;

    MAPI_AO_SetExternAttr(pstAioAttr);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AO_Init(HI_HANDLE AoHdl, const HI_MAPI_AO_ATTR_S *pstAoAttr)
{
    HI_S32 s32Ret;
    AIO_ATTR_S stAoAttr;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAoAttr);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is started , can not be init!\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_STOPED
        && pstAoAttr->enSampleRate == g_astAoContext[AoHdl].stAoAttr.enSamplerate
        && pstAoAttr->enBitwidth == g_astAoContext[AoHdl].stAoAttr.enBitwidth
        && pstAoAttr->enSoundMode == g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx
        && pstAoAttr->enWorkmode == g_astAoContext[AoHdl].stAoAttr.enWorkmode
        && pstAoAttr->u32PtNumPerFrm == g_astAoContext[AoHdl].stAoAttr.u32PtNumPerFrm
        && pstAoAttr->enInSampleRate == g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AO, "AO init attr is not change , return success !\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_SUCCESS;
    }

    /* set acodec and pub attr */
    s32Ret = MAPI_AO_ConfigAttr(AoHdl, pstAoAttr, &stAoAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "MAPI_AO_CheckAttr error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    /* config inner audio codec */
    if (stAoAttr.enI2sType == AIO_I2STYPE_INNERCODEC) {
        s32Ret = ADPT_ACODEC_ConfigAttr(stAoAttr.enSamplerate, ACODEC_MIXER_BUTT);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "audio inner codec config error.\n");
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AO_SetPubAttr(AoHdl, &stAoAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SetPubAttr error. ret = %x \n", s32Ret);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    /* save attr and state */
    memcpy(&g_astAoContext[AoHdl].stAoAttr, &stAoAttr, sizeof(AIO_ATTR_S));
    g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx = pstAoAttr->enSoundMode;
    g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate = pstAoAttr->enInSampleRate;

    g_astAoContext[AoHdl].enAoState = MAPI_AO_STOPED;
    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_Deinit(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_INIT(g_bAoInited);

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Ao is started, can't be Deinit ,try stop now \n");
        s32Ret = HI_MAPI_AO_Stop(AoHdl);
        if (s32Ret != HI_SUCCESS) {
            return s32Ret;
        }
    }

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_UNINITED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_AO, "ao has already uninited, return success\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_SUCCESS;
    }

    /* deinit attr */
    g_astAoContext[AoHdl].stAoAttr.enBitwidth = AUDIO_BIT_WIDTH_BUTT;
    g_astAoContext[AoHdl].stAoAttr.enSamplerate = AUDIO_SAMPLE_RATE_BUTT;
    g_astAoContext[AoHdl].stAoAttr.enSoundmode = AUDIO_SOUND_MODE_BUTT;
    g_astAoContext[AoHdl].stAoAttr.enWorkmode = AIO_MODE_BUTT;
    g_astAoContext[AoHdl].stAoAttr.u32PtNumPerFrm = 0;
    g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate = AUDIO_SAMPLE_RATE_BUTT;
    g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx = AUDIO_SOUND_MODE_BUTT;

    /* set state */
    g_astAoContext[AoHdl].enAoState = MAPI_AO_UNINITED;

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_Start(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret;
    HI_S32 s32ChnId = 0;

    /* set attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is not inited, can't be start!\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }
    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao has already started, return success!\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_SUCCESS;
    }

    /* enable ao */
    s32Ret = HI_MPI_AO_Enable(AoHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_Enable error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    /* set trackmode */
    if (g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT) {
        s32Ret = HI_MPI_AO_SetTrackMode(AoHdl, AUDIO_TRACK_BOTH_LEFT);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SetTrackMode fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return s32Ret;
        }
    } else if (g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT) {
        s32Ret = HI_MPI_AO_SetTrackMode(AoHdl, AUDIO_TRACK_BOTH_RIGHT);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SetTrackMode fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return s32Ret;
        }
    }

    /* enable normal chn and system chn */
    switch (g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx) {
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
            s32ChnId = 0;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            s32ChnId = 1;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "enSoundMode %d is error.\n",
                           g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx);
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return HI_MAPI_AO_EILLPARAM;
    }

    s32Ret = HI_MPI_AO_EnableChn(AoHdl, s32ChnId);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_EnableChn error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_AO_EnableChn(AoHdl, MAPI_AO_SYS_CHN);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_EnableSysChn error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    if (g_astAoContext[AoHdl].stAoAttr.enSamplerate != g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate) {
        s32Ret = HI_MPI_AO_DisableReSmp(AoHdl, s32ChnId);
        s32Ret |= HI_MPI_AO_EnableReSmp(AoHdl, s32ChnId, g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "enable ao resample error. chnId:%d\n", s32ChnId);
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return s32Ret;
        }
    }

    /* set state */
    g_astAoContext[AoHdl].enAoState = MAPI_AO_STARTED;

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_Stop(HI_HANDLE AoHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is uninited, can't be stop\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    if (g_astAoContext[AoHdl].enAoState == MAPI_AO_STOPED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao has already stop, return success !\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_SUCCESS;
    }

    /* disable all chn ,prevent get chn id error */
    for (i = 0; i < HI_MAPI_AO_CHN_MAX_NUM; i++) {
        if (g_astAoContext[AoHdl].stAoAttr.enSamplerate != g_astAoContext[AoHdl].stAoAttrEx.enInSampleRate) {
            s32Ret = HI_MPI_AO_DisableReSmp(AoHdl, i);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "disable ao resample error.\n");
                MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_AO_DisableChn(AoHdl, i);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_DisableChn [%d] error. s32Ret: %x\n", i, s32Ret);
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return s32Ret;
        }
    }

    /* disable ao */
    s32Ret = HI_MPI_AO_Disable(AoHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_Disable error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    /* set state */

    g_astAoContext[AoHdl].enAoState = MAPI_AO_STOPED;

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_SetVolume(HI_HANDLE AoHdl, HI_S32 s32AudioGain)
{
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    if (s32AudioGain < HI_MAPI_AO_MIN_GAIN || s32AudioGain > HI_MAPI_AO_MAX_GAIN) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Gain out of range\n");
        return HI_MAPI_AO_EILLPARAM;
    }

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    s32Ret = ADPT_ACODEC_SetAOVol(s32AudioGain);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ADPT_ACODEC_SetAOVol error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_GetVolume(HI_HANDLE AoHdl, HI_S32 *ps32AudioGain)
{
    HI_S32 s32VolumeDb;
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(ps32AudioGain);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    s32Ret = ADPT_ACODEC_GetAOVol(&s32VolumeDb);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ADPT_ACODEC_GetAOVol error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    *ps32AudioGain = s32VolumeDb;

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_Mute(HI_HANDLE AoHdl)
{
    AUDIO_FADE_S stFade;
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState != MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is not started, can't set mute\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    /* mute ao */
    stFade.bFade = HI_TRUE;
    stFade.enFadeInRate = AUDIO_FADE_RATE_1;
    stFade.enFadeOutRate = AUDIO_FADE_RATE_64;
    s32Ret = HI_MPI_AO_SetMute(AoHdl, HI_TRUE, &stFade);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SetMute error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_Unmute(HI_HANDLE AoHdl)
{
    AUDIO_FADE_S stFade;
    HI_S32 s32Ret;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState != MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is not started, can't set unmute\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    /* unmute ao */
    stFade.bFade = HI_TRUE;
    stFade.enFadeInRate = AUDIO_FADE_RATE_8;
    stFade.enFadeOutRate = AUDIO_FADE_RATE_1;
    s32Ret = HI_MPI_AO_SetMute(AoHdl, HI_FALSE, &stFade);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SetMute error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_SendFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout)
{
    HI_S32 s32Ret;
    HI_S32 s32AoChn;
    AUDIO_FRAME_S stAFrm;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAudioFrame);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState != MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is not started, can't send frame\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    /* send frame */
    switch (g_astAoContext[AoHdl].stAoAttrEx.enSoundModeEx) {
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
            s32AoChn = 0;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            s32AoChn = 1;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "enSoundMode is error.\n");
            MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
            return HI_MAPI_AO_EILLPARAM;
    }

    memcpy(&stAFrm, pstAudioFrame, sizeof(AUDIO_FRAME_S));

#ifdef __HuaweiLite__
    /* prevent client addr error */
    stAFrm.u64VirAddr[0] = (HI_U8 *)(HI_UL)stAFrm.u64PhyAddr[0];
    stAFrm.u64VirAddr[1] = (HI_U8 *)(HI_UL)stAFrm.u64PhyAddr[1];
#endif

    s32Ret = HI_MPI_AO_SendFrame(AoHdl, s32AoChn, &stAFrm, u32Timeout);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SendFrame error. ret:%x \n", s32Ret);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

HI_S32 HI_MAPI_AO_SendSysFrame(HI_HANDLE AoHdl, const AUDIO_FRAME_S *pstAudioFrame, HI_U32 u32Timeout)
{
    HI_S32 s32Ret;
    AUDIO_FRAME_S stAFrm;

    /* check attr and state */
    CHECK_MAPI_AO_HANDLE_RANGE(AoHdl);
    CHECK_MAPI_AO_NULL_PTR(pstAudioFrame);

    MUTEX_LOCK(g_aoFuncLock[AoHdl]);

    if (g_bAoInited == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "AO is not inited!FUNCTION %s,LINE %d!\n", __FUNCTION__, __LINE__);
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ENOTINITED;
    }

    if (g_astAoContext[AoHdl].enAoState != MAPI_AO_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "ao is not started, can't send frame\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return HI_MAPI_AO_ESTATEERR;
    }

    memcpy(&stAFrm, pstAudioFrame, sizeof(AUDIO_FRAME_S));

#ifdef __HuaweiLite__
    /* prevent client addr error */
    stAFrm.u64VirAddr[0] = (HI_U8 *)(HI_UL)stAFrm.u64PhyAddr[0];
    stAFrm.u64VirAddr[1] = (HI_U8 *)(HI_UL)stAFrm.u64PhyAddr[1];
#endif

    /* send frame */
    s32Ret = HI_MPI_AO_SendFrame(AoHdl, MAPI_AO_SYS_CHN, &stAFrm, u32Timeout);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "HI_MPI_AO_SendSysFrame error.\n");
        MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_aoFuncLock[AoHdl]);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
