/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap.c
 * @brief   NDK acap server functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "hi_mapi_acap.h"
#include "mpi_audio.h"
#include "mapi_acap_inner.h"
#include "adpt_audio_acodec.h"
#include "mpi_sys.h"
#include "mapi_comm_inner.h"
#include "mapi_acap_adapt.h"
#include "mapi_acap_arch.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_BOOL g_bACapInited = HI_FALSE; /* module inited flag */
static MAPI_ACAP_CONTEXT_S g_astAcapContext[HI_MAPI_ACAP_DEV_MAX_NUM];
static HI_S16 *g_ps16AcapResampleRatebuff[HI_MAPI_ACAP_DEV_MAX_NUM] = { NULL };
static pthread_mutex_t g_acapFuncLock[HI_MAPI_ACAP_DEV_MAX_NUM] = { PTHREAD_MUTEX_INITIALIZER };

static HI_S32 MAPI_ACAP_GetVqeConfig(HI_HANDLE AcapHdl, AI_RECORDVQE_CONFIG_S *pstVqeAttr)
{
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstVqeAttr);

    pstVqeAttr->s32WorkSampleRate = AUDIO_SAMPLE_RATE_48000;
    pstVqeAttr->s32FrameSample = g_astAcapContext[AcapHdl].stAcapAttr.u32PtNumPerFrm;
    pstVqeAttr->enWorkstate = VQE_WORKSTATE_COMMON;
    if (g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_LEFT ||
        g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_RIGHT) {
        pstVqeAttr->s32InChNum = 1;
        pstVqeAttr->s32OutChNum = 1;
    } else {
        pstVqeAttr->s32InChNum = 2;
        pstVqeAttr->s32OutChNum = 2;
    }
    pstVqeAttr->enRecordType = VQE_RECORD_NORMAL;

    pstVqeAttr->stDrcCfg.bUsrMode = HI_FALSE;
    pstVqeAttr->stRnrCfg.bUsrMode = HI_FALSE;
    pstVqeAttr->stHdrCfg.bUsrMode = HI_FALSE;  // Only in the case of built-in codec can be set to auto mode

    pstVqeAttr->stHpfCfg.bUsrMode = HI_TRUE;
    pstVqeAttr->stHpfCfg.enHpfFreq = AUDIO_HPF_FREQ_80;

    pstVqeAttr->stAgcCfg.bUsrMode = HI_FALSE;

    pstVqeAttr->stEqCfg.s8GaindB[0] = -10;
    pstVqeAttr->stEqCfg.s8GaindB[1] = -3;
    pstVqeAttr->stEqCfg.s8GaindB[2] = -2;
    pstVqeAttr->stEqCfg.s8GaindB[3] = -2;
    pstVqeAttr->stEqCfg.s8GaindB[4] = -1;
    pstVqeAttr->stEqCfg.s8GaindB[5] = -1;
    pstVqeAttr->stEqCfg.s8GaindB[6] = -2;
    pstVqeAttr->stEqCfg.s8GaindB[7] = -5;
    pstVqeAttr->stEqCfg.s8GaindB[8] = -6;
    pstVqeAttr->stEqCfg.s8GaindB[9] = -8;

    pstVqeAttr->u32OpenMask = AI_RECORDVQE_MASK_DRC | AI_RECORDVQE_MASK_HDR | AI_RECORDVQE_MASK_HPF |
                              AI_RECORDVQE_MASK_RNR | AI_RECORDVQE_MASK_AGC;

    return HI_SUCCESS;
}

HI_S32 MAPI_ACAP_GetChnNum(HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode, HI_S32 *pChnNum)
{
    CHECK_MAPI_ACAP_NULL_PTR(pChnNum);

    switch (enSoundMode) {
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
            *pChnNum = 2;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            *pChnNum = 1;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support sound mode\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }
    return HI_SUCCESS;
}

/** this interface is called by HI_MAPI_Media_Init() */
HI_S32 MAPI_ACAP_Init(HI_VOID)
{
    HI_S32 i = 0;

    /* check state */
    if (g_bACapInited == HI_TRUE) {
        return HI_SUCCESS;
    }

    /* init context and load inner param */
    for (i = 0; i < HI_MAPI_ACAP_DEV_MAX_NUM; i++) {
        g_astAcapContext[i].enAcapState = MAPI_ACAP_UNINITED;
        g_astAcapContext[i].stAcapAttr.u32ChnCnt = HI_MAPI_ACAP_CHN_MAX_NUM;
        g_astAcapContext[i].stAcapAttr.u32ClkSel = 0;
        g_astAcapContext[i].stAcapAttr.u32EXFlag = 0;
        /* use 5  from experience in small memory scenes */
        g_astAcapContext[i].stAcapAttr.u32FrmNum = 5;
        g_astAcapContext[i].stAcapAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
        memset(g_astAcapContext[i].bAcapChnstart, 0, sizeof(HI_BOOL) * HI_MAPI_ACAP_CHN_MAX_NUM);
    }

    /* open audio codec file */
    ADPT_ACODEC_Open();

    /* init module */
    g_bACapInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 MAPI_ACAP_Deinit(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    /* check staus */
    if (g_bACapInited != HI_TRUE) {
        return HI_SUCCESS;
    }
    for (i = 0; i < HI_MAPI_ACAP_DEV_MAX_NUM; i++) {
        if (g_astAcapContext[i].enAcapState != MAPI_ACAP_UNINITED) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACap is not Deinit,media can not deinit , try deinit acap\n");
            s32Ret = HI_MAPI_ACAP_Deinit(i);
            if (s32Ret != HI_SUCCESS) {
                return s32Ret;
            }
        }
    }

    /* deinit context */
    for (i = 0; i < HI_MAPI_ACAP_DEV_MAX_NUM; i++) {
        memset(&g_astAcapContext[i].stAcapAttr, 0, sizeof(AIO_ATTR_S));
        memset(&g_astAcapContext[i].stAcapAttrEx, 0, sizeof(MAPI_ACAP_ATTR_EX_S));
        g_astAcapContext[i].enAcapState = MAPI_ACAP_BUTT;
        memset(g_astAcapContext[i].bAcapChnstart, 0, sizeof(HI_BOOL) * HI_MAPI_ACAP_CHN_MAX_NUM);
    }

    /* close audio codec file */
    ADPT_ACODEC_Close();

    /* deinit module */
    g_bACapInited = HI_FALSE;
    return HI_SUCCESS;
}

HI_VOID MAPI_ACAP_DualMono2Stereo(const HI_S16 *ps16LeftChnData, const HI_S16 *ps16RightChnData, HI_U32 u32SampleNumPerChn,
                                  HI_S16 *ps16StereoData)
{
    HI_U32 i = 0;
    for (i = 0; i < u32SampleNumPerChn; i++) {
        ps16StereoData[2 * i] = ps16LeftChnData[i];
        ps16StereoData[(2 * i) + 1] = ps16RightChnData[i];
    }
}

HI_VOID MAPI_ACAP_Stereo2DualMono(HI_S16 *ps16LeftChnData, HI_S16 *ps16RightChnData, HI_U32 u32SampleNumPerChn,
                                  const HI_S16 *ps16StereoData)
{
    HI_U32 i = 0;
    for (i = 0; i < u32SampleNumPerChn * 2; i++) {
        ps16LeftChnData[i / 2] = ps16StereoData[i];
        i++;
        ps16RightChnData[i / 2] = ps16StereoData[i];
    }
}

static HI_VOID MAPI_ACAP_SetExternAttr(AIO_ATTR_S *pstAioAttr)
{
    if (pstAioAttr->enI2sType == AIO_I2STYPE_EXTERN) {
        pstAioAttr->u32ClkSel = 1;
        pstAioAttr->u32ChnCnt = 1;
    }
}

static HI_S32 MAPI_ACAP_ConfigAttr(HI_HANDLE AcapHdl, const HI_MAPI_ACAP_ATTR_S *pstACapAttr,
                                   AIO_ATTR_S *pstAioAttr)
{
    memcpy(pstAioAttr, &g_astAcapContext[AcapHdl].stAcapAttr, sizeof(AIO_ATTR_S));
    switch (pstACapAttr->enSampleRate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_48000:
            pstAioAttr->enSamplerate = pstACapAttr->enSampleRate;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support samplerate !\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }

    switch (pstACapAttr->enBitwidth) {
        case AUDIO_BIT_WIDTH_16:
            pstAioAttr->enBitwidth = pstACapAttr->enBitwidth;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support bitwidth !\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }

    switch (pstACapAttr->enSoundMode) {
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
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support sound mode\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }

    pstAioAttr->enWorkmode = pstACapAttr->enWorkmode;

    if (pstACapAttr->u32PtNumPerFrm > MAXFRAMESIZE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Point Number per Frame Overrange !\n");
        return HI_MAPI_ACAP_EILLPARAM;
    }
    pstAioAttr->u32PtNumPerFrm = pstACapAttr->u32PtNumPerFrm;

    switch (pstACapAttr->enResampleRate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_48000:
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support resamplerate!\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }

    pstAioAttr->enI2sType = pstACapAttr->enI2sType;

    MAPI_ACAP_SetExternAttr(pstAioAttr);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Init(HI_HANDLE AcapHdl, const HI_MAPI_ACAP_ATTR_S *pstACapAttr)
{
    HI_S32 s32Ret;
    AIO_ATTR_S stACapAttr;

    /* check param and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstACapAttr);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACap is started,no need init acap repeatedly!\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STOPED
        && g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate == pstACapAttr->enSampleRate
        && g_astAcapContext[AcapHdl].stAcapAttr.enBitwidth == pstACapAttr->enBitwidth
        && g_astAcapContext[AcapHdl].stAcapAttr.enWorkmode == pstACapAttr->enWorkmode
        && g_astAcapContext[AcapHdl].stAcapAttr.u32PtNumPerFrm == pstACapAttr->u32PtNumPerFrm
        && g_astAcapContext[AcapHdl].stAcapAttrEx.enMixerMicMode == pstACapAttr->enMixerMicMode
        && g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx == pstACapAttr->enSoundMode
        && g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate == pstACapAttr->enResampleRate) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACAP attribute not change return successful\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    /* config acodec and set pub attr */
    s32Ret = MAPI_ACAP_ConfigAttr(AcapHdl, pstACapAttr, &stACapAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP attribute check error\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    /* config inner audio codec */
    if (stACapAttr.enI2sType == AIO_I2STYPE_INNERCODEC) {
        s32Ret = ADPT_ACODEC_ConfigAttr(pstACapAttr->enSampleRate, pstACapAttr->enMixerMicMode);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ADPT_ACODEC_ConfigAttr fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AI_SetPubAttr(AcapHdl, &stACapAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_SetPubAttr fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    /* creat resampler */
    if (pstACapAttr->enResampleRate != pstACapAttr->enSampleRate) {
        HI_S32 s32ChnNum = 0;
        MAPI_ACAP_GetChnNum(pstACapAttr->enSoundMode, &s32ChnNum);
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "resampler create , InSamplerate = %u , OutSamplerate = %u , chnNum = %d \n",
                         pstACapAttr->enSampleRate, pstACapAttr->enResampleRate, s32ChnNum);
        g_astAcapContext[AcapHdl].stAcapResmpInfo.pAcapResampler = HAL_MAPI_Resampler_Create(pstACapAttr->enSampleRate,
            pstACapAttr->enResampleRate, s32ChnNum);

        s32Ret = HI_MPI_SYS_MmzAlloc(&g_astAcapContext[AcapHdl].stAcapResmpInfo.u64ResMMZPhyAddr,
            (HI_VOID **)&g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr, "AcapResBuf",
            "anonymous", (sizeof(HI_U16) * HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM * 6));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_SYS_MmzAlloc_Cached fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }
    }

    /* set attr and state */
    memcpy(&g_astAcapContext[AcapHdl].stAcapAttr, &stACapAttr, sizeof(AIO_ATTR_S));
    g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate = pstACapAttr->enResampleRate;
    g_astAcapContext[AcapHdl].stAcapAttrEx.enMixerMicMode = pstACapAttr->enMixerMicMode;
    g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx = pstACapAttr->enSoundMode;

    g_astAcapContext[AcapHdl].enAcapState = MAPI_ACAP_STOPED;

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Deinit(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(g_bACapInited);

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STARTED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACap is started,you must stop ACap first, try stop\n");
        for (i = 0; i < HI_MAPI_ACAP_CHN_MAX_NUM; i++) {
            if (g_astAcapContext[AcapHdl].bAcapChnstart[i] != HI_FALSE) {
                s32Ret = HI_MAPI_ACAP_Stop(AcapHdl, i);
                if (s32Ret != HI_SUCCESS) {
                    return s32Ret;
                }
            }
        }
    }

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_UNINITED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "acap has already deinited ,return success!\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    /* destory resampler */
    if (g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate != g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate) {
        s32Ret = HI_MPI_SYS_MmzFree(g_astAcapContext[AcapHdl].stAcapResmpInfo.u64ResMMZPhyAddr,
                                    g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MAPI_FreeBuffer fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }

        HAL_MAPI_Resampler_Destroy(g_astAcapContext[AcapHdl].stAcapResmpInfo.pAcapResampler);
    }

    /* deinit attr and state */
    g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate = AUDIO_SAMPLE_RATE_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttr.enBitwidth = AUDIO_BIT_WIDTH_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttr.enWorkmode = AIO_MODE_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttr.enSoundmode = AUDIO_SOUND_MODE_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttr.u32PtNumPerFrm = 0;
    g_astAcapContext[AcapHdl].stAcapAttrEx.enMixerMicMode = ACODEC_MIXER_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx = HI_MAPI_AUDIO_SOUND_MODE_BUTT;
    g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate = AUDIO_SAMPLE_RATE_BUTT;

    g_astAcapContext[AcapHdl].enAcapState = MAPI_ACAP_UNINITED;

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Start(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "acap not been inited,can not start!\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }
    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STARTED) {
        if (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_TRUE) {
            s32Ret = HI_MPI_AI_EnableChn(AcapHdl, AcapChnHdl);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_EnableChn fail,ret:%x\n", s32Ret);
                MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
                return s32Ret;
            }
            g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] = HI_TRUE;
        }
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACap is already started, return success\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    /* enable ai */
    s32Ret = HI_MPI_AI_Enable(AcapHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_Enable fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    /* set trackmode */
    if (g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT) {
        s32Ret = HI_MPI_AI_SetTrackMode(AcapHdl, AUDIO_TRACK_BOTH_LEFT);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_SetTrackMode fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }
    } else if (g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx == HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT) {
        s32Ret = HI_MPI_AI_SetTrackMode(AcapHdl, AUDIO_TRACK_BOTH_RIGHT);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_SetTrackMode fail,ret:%x\n", s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AI_EnableChn(AcapHdl, AcapChnHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_EnableChn fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    /* set state */
    g_astAcapContext[AcapHdl].enAcapState = MAPI_ACAP_STARTED;
    g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] = HI_TRUE;

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_EnableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    AI_RECORDVQE_CONFIG_S stVqeAttr;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if ((g_astAcapContext[AcapHdl].enAcapState != MAPI_ACAP_STARTED) ||
        (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_TRUE)) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACap or Acap Chn is not been started,can not EableVqe\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }

    /* set vqe */
    memset(&stVqeAttr, 0, sizeof(AI_RECORDVQE_CONFIG_S));
    s32Ret = MAPI_ACAP_GetVqeConfig(AcapHdl, &stVqeAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_EnableChn fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_AI_SetRecordVqeAttr(AcapHdl, AcapChnHdl, &stVqeAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_SetRecordVqeAttr fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    s32Ret = HI_MPI_AI_EnableVqe(AcapHdl, AcapChnHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_EnableVqe fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_DisableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "acap not been inited , can not DisableVqe\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STOPED ||
        (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_TRUE)) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACAP or ACAP CHN is stopped, return successful\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_AI_DisableVqe(AcapHdl, AcapChnHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_DisableVqe fail, ChnId:%d ,ret:%x\n", i, s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Stop(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_UNINITED) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "acap not been inited , can not stop\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }

    if (g_astAcapContext[AcapHdl].enAcapState == MAPI_ACAP_STOPED) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACAP is stopped, return successful\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_SUCCESS;
    }

    /* disable all chn vqe and chn , prevent get chn id error */
    if (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_FALSE) {
        s32Ret = HI_MPI_AI_DisableChn(AcapHdl, AcapChnHdl);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_DisableChn fail, ChnId:%d, ret:%x\n", i, s32Ret);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return s32Ret;
        }
        g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] = HI_FALSE;
    }

    /* ai chn need all stoped */
    for (i = 0; i < HI_MAPI_ACAP_CHN_MAX_NUM; i++) {
        if (g_astAcapContext[AcapHdl].bAcapChnstart[i] != HI_FALSE) {
            MAPI_WARN_TRACE(HI_MAPI_MOD_ACAP, "ACAP Chn[%d] is started, return successful\n", i);
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return HI_SUCCESS;
        }
    }

    /* disable ai */
    s32Ret = HI_MPI_AI_Disable(AcapHdl);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_Enable fail,ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    if (g_ps16AcapResampleRatebuff[AcapHdl] != NULL) {
        free(g_ps16AcapResampleRatebuff[AcapHdl]);
        g_ps16AcapResampleRatebuff[AcapHdl] = NULL;
    }

    g_astAcapContext[AcapHdl].enAcapState = MAPI_ACAP_STOPED;

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_SetVolume(HI_HANDLE AcapHdl, HI_S32 s32AudioGain)
{
    HI_S32 s32Ret;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    if (s32AudioGain < HI_MAPI_ACAP_MIN_GAIN || s32AudioGain > HI_MAPI_ACAP_MAX_GAIN) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Gain out of range\n");
        return HI_MAPI_ACAP_EILLPARAM;
    }

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    /* set vol */
    s32Ret = ADPT_ACODEC_SetAIVol(s32AudioGain);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ADPT_ACODEC_SetVol fail\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_GetVolume(HI_HANDLE AcapHdl, HI_S32 *ps32AudioGain)
{
    HI_S32 s32Ret;
    HI_S32 s32Vol;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_NULL_PTR(ps32AudioGain);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    /* get vol */
    s32Ret = ADPT_ACODEC_GetAIVol(&s32Vol);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ADPT_ACODEC_SetVol fail\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }
    *ps32AudioGain = s32Vol;

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Mute(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    /* mute */
    s32Ret = ADPT_ACODEC_MuteAI();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ADPT_ACODEC_Mute fail\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Unmute(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret;

    /* check attr and status */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    /* unmute */
    s32Ret = ADPT_ACODEC_UnmuteAI();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ADPT_ACODEC_UnMute fail\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_GetFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, AUDIO_FRAME_S *pstFrm,
                             AEC_FRAME_S *pstAecFrm)
{
    HI_S32 s32Ret;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;
    AI_CHN_PARAM_S stChnParam;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstFrm);
    CHECK_MAPI_ACAP_NULL_PTR(pstAecFrm);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if ((g_astAcapContext[AcapHdl].enAcapState != MAPI_ACAP_STARTED) ||
        (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_TRUE)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACap or Acap Chn is not started, can not get frame!\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }

    /* set chn frame depth */
    s32Ret = HI_MPI_AI_GetChnParam(AcapHdl, AcapChnHdl, &stChnParam);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, " HI_MPI_AI_GetChnParam fail, s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }
    stChnParam.u32UsrFrmDepth = 5;
    s32Ret = HI_MPI_AI_SetChnParam(AcapHdl, AcapChnHdl, &stChnParam);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, " HI_MPI_AI_SetChnParam fail, s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return s32Ret;
    }

    /* get frame */
    s32Ret = HI_MPI_AI_GetFrame(AcapHdl, AcapChnHdl, &stFrm, &stAecFrm, -1);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_GetFrame Failed!\n");
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_GetFrame : phyAddr0 = %llu , phyAddr1 = %llu , frameLen = %u \n",
                     stFrm.u64PhyAddr[0], stFrm.u64PhyAddr[1], stFrm.u32Len);

    /* resample process , extra support up sample , now get resample frame not have cache buff */
    if (g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate != g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate) {
        HI_U32 u32InSamples = 0;
        HI_S32 s32OutSamples = 0;
        HI_S16 as16Inbuff[HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM];
        if (g_ps16AcapResampleRatebuff[AcapHdl] == NULL) {
            g_ps16AcapResampleRatebuff[AcapHdl] = (HI_S16 *)malloc(HI_MAPI_AIO_MAX_POINT_PER_FRAME *
                                                                   HI_MAPI_ACAP_CHN_MAX_NUM * 6);
            if (g_ps16AcapResampleRatebuff[AcapHdl] == NULL) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "malloc Outbuff Failed!\n");
                MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
                return HI_FAILURE;
            }
            memset(g_ps16AcapResampleRatebuff[AcapHdl], 0,
                   HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM * 6);
        }

        g_astAcapContext[AcapHdl].stAcapResmpInfo.u32SavedFrmLen = stFrm.u32Len;
        u32InSamples = stFrm.u32Len / (g_astAcapContext[AcapHdl].stAcapAttr.enBitwidth + 1);

        switch (g_astAcapContext[AcapHdl].stAcapAttr.enSoundmode) {
            case AUDIO_SOUND_MODE_STEREO:
                /* compose mono pcm to stereo pcm */
                MAPI_ACAP_DualMono2Stereo((HI_S16 *)stFrm.u64VirAddr[0], (HI_S16 *)stFrm.u64VirAddr[1], u32InSamples,
                    as16Inbuff);
                break;
            case AUDIO_SOUND_MODE_MONO:
                memcpy(as16Inbuff, stFrm.u64VirAddr[0], stFrm.u32Len);
                break;
            default:
                break;
        }

        s32OutSamples = HAL_MAPI_Resampler_Process(g_astAcapContext[AcapHdl].stAcapResmpInfo.pAcapResampler, as16Inbuff,
                                                   u32InSamples, g_ps16AcapResampleRatebuff[AcapHdl]);
        if (s32OutSamples < 0) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, " HAL_MAPI_Resampler_Process exec fail, s32Ret:%x\n", s32OutSamples);
            s32Ret = HI_MPI_AI_ReleaseFrame(AcapHdl, AcapChnHdl, &stFrm, &stAecFrm);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, " HI_MPI_AI_GetChnParam fail, s32Ret:%x\n", s32Ret);
            }
            MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
            return HI_MAPI_ACAP_EOPERATE;
        }

        MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "resample outsamples:%d\n", s32OutSamples);
        stFrm.u32Len = s32OutSamples * (g_astAcapContext[AcapHdl].stAcapAttr.enBitwidth + 1);

        switch (g_astAcapContext[AcapHdl].stAcapAttr.enSoundmode) {
            case AUDIO_SOUND_MODE_STEREO:
                /* detach stereo pcm to mono pcm */
                stFrm.u64VirAddr[0] = (HI_U8 *)g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr;
                stFrm.u64VirAddr[1] = (HI_U8 *)g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr + stFrm.u32Len;
                MAPI_ACAP_Stereo2DualMono((HI_S16 *)stFrm.u64VirAddr[0], (HI_S16 *)stFrm.u64VirAddr[1], s32OutSamples,
                                          g_ps16AcapResampleRatebuff[AcapHdl]);
                break;
            case AUDIO_SOUND_MODE_MONO:
                stFrm.u64VirAddr[0] = (HI_U8 *)g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr;
                memcpy(stFrm.u64VirAddr[0], g_ps16AcapResampleRatebuff[AcapHdl], stFrm.u32Len);
                break;
            default:
                break;
        }
    }

    memcpy(pstFrm, &stFrm, sizeof(AUDIO_FRAME_S));
    memcpy(pstAecFrm, &stAecFrm, sizeof(AEC_FRAME_S));

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_ReleaseFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, const AUDIO_FRAME_S *pstFrm,
                                 const AEC_FRAME_S *pstAecFrm)
{
    HI_S32 s32Ret;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstFrm);
    CHECK_MAPI_ACAP_NULL_PTR(pstAecFrm);

    MUTEX_LOCK(g_acapFuncLock[AcapHdl]);

    if (!g_bACapInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACAP has not been pre inited , mapi media has not been inited\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ENOINITED;
    }

    if ((g_astAcapContext[AcapHdl].enAcapState != MAPI_ACAP_STARTED) ||
        (g_astAcapContext[AcapHdl].bAcapChnstart[AcapChnHdl] != HI_TRUE)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACap or Acap Chn is not started, can not release frame!\n");
        MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
        return HI_MAPI_ACAP_ESTATEERR;
    }

    memcpy(&stFrm, pstFrm, sizeof(AUDIO_FRAME_S));
    memcpy(&stAecFrm, pstAecFrm, sizeof(AEC_FRAME_S));

    /* resample frame release process */
    if (g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate != g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate) {
        stFrm.u32Len = g_astAcapContext[AcapHdl].stAcapResmpInfo.u32SavedFrmLen;
        memset(g_astAcapContext[AcapHdl].stAcapResmpInfo.pResMMZVirAddr, 0,
               (sizeof(HI_U16) * HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM * 6));
    }

    /* prevent client addr error */
    stFrm.u64VirAddr[0] = (HI_U8 *)(HI_UL)stFrm.u64PhyAddr[0];
    stFrm.u64VirAddr[1] = (HI_U8 *)(HI_UL)stFrm.u64PhyAddr[1];
    stAecFrm.stRefFrame.u64VirAddr[0] = (HI_U8 *)(HI_UL)stAecFrm.stRefFrame.u64PhyAddr[0];
    stAecFrm.stRefFrame.u64VirAddr[1] = (HI_U8 *)(HI_UL)stAecFrm.stRefFrame.u64PhyAddr[1];

    s32Ret = HI_MPI_AI_ReleaseFrame(AcapHdl, AcapChnHdl, &stFrm, &stAecFrm);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "HI_MPI_AI_ReleaseFrame Failed!\n");
    }

    MUTEX_UNLOCK(g_acapFuncLock[AcapHdl]);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
