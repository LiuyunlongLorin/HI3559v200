/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    adapt_audio_acodec.c
 * @brief   adapt audio acodec function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "hi_comm_aio.h"
#include "acodec.h"
#include "adpt_audio_acodec.h"
#include "mapi_acap_inner.h"
#include "mapi_ao_inner.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ACODEC_DEV "/dev/acodec"

static HI_S32 g_s32AcodecFd = -1;

HI_S32 ADPT_ACODEC_Open(HI_VOID)
{
    if (g_s32AcodecFd != -1) {
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "Acodec have already open, fd:%d\n", g_s32AcodecFd);
        return HI_SUCCESS;
    }

    g_s32AcodecFd = open(ACODEC_DEV, O_RDWR);
    if (g_s32AcodecFd < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "can't open Acodec, errno: %d\n", errno);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_Close(HI_VOID)
{
    HI_S32 s32Ret;

    if (g_s32AcodecFd == -1) {
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "Acodec have already close\n");
        return HI_SUCCESS;
    }

    s32Ret = close(g_s32AcodecFd);
    if (s32Ret < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "close Acodec fail, errno: %d\n", errno);
        return HI_FAILURE;
    }

    g_s32AcodecFd = -1;
    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_ConfigAttr(AUDIO_SAMPLE_RATE_E enSampleRate, ACODEC_MIXER_E enMixerMicMode)
{
    ACODEC_FS_E i2s_fs_sel = 0;
    ACODEC_MIXER_E input_mode = 0;
    HI_U32 u32MuteCtrl = 0x0;

    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acodec not open\n");
        return HI_FAILURE;
    }

    /* set samplerate , recommend set same ai ao samplerate */
    switch (enSampleRate) {
        case AUDIO_SAMPLE_RATE_8000:
            i2s_fs_sel = ACODEC_FS_8000;
            break;
        case AUDIO_SAMPLE_RATE_16000:
            i2s_fs_sel = ACODEC_FS_16000;
            break;
        case AUDIO_SAMPLE_RATE_48000:
            i2s_fs_sel = ACODEC_FS_48000;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support enSample:%d\n", enSampleRate);
            return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_I2S1_FS, &i2s_fs_sel)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "set acodec sample rate failed\n");
        return HI_FAILURE;
    }

    /* set audio input config */
    if (enMixerMicMode != ACODEC_MIXER_BUTT) {
        input_mode = enMixerMicMode;
        if (ioctl(g_s32AcodecFd, ACODEC_SET_MIXER_MIC, &input_mode)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "select acodec input_mode failed\n");
            return HI_FAILURE;
        }

        if (ioctl(g_s32AcodecFd, ACODEC_SET_MICL_MUTE, &u32MuteCtrl)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICL_MUTE failed\n");
            return HI_FAILURE;
        }
        if (ioctl(g_s32AcodecFd, ACODEC_SET_MICR_MUTE, &u32MuteCtrl)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICR_MUTE failed\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_SetAIVol(HI_S32 s32Vol)
{
    HI_S32 s32InputVol;

    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acodec not open\n");
        return HI_FAILURE;
    }

    s32InputVol = s32Vol;
    if (ioctl(g_s32AcodecFd, ACODEC_SET_INPUT_VOL, &s32InputVol)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "set acodec micin volume failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_GetAIVol(HI_S32 *s32Vol)
{
    HI_S32 s32InputVol = 0;

    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acodec not open\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_GET_INPUT_VOL, &s32InputVol)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "set acodec micin volume failed\n");
        return HI_FAILURE;
    }

    *s32Vol = s32InputVol;

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_SetAOVol(HI_S32 s32Vol)
{
    HI_S32 s32OutputVol;

    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Acodec not open\n");
        return HI_FAILURE;
    }

    s32OutputVol = s32Vol;

    if (ioctl(g_s32AcodecFd, ACODEC_SET_OUTPUT_VOL, &s32OutputVol)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "set acodec mic out volume failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_GetAOVol(HI_S32 *s32Vol)
{
    HI_S32 s32OutputVol = 0;

    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "Acodec not open\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_GET_OUTPUT_VOL, &s32OutputVol)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AO, "set acodec mic out volume failed\n");
        return HI_FAILURE;
    }

    *s32Vol = s32OutputVol;
    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_MuteAI()
{
    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acodec not open\n");
        return HI_FAILURE;
    }

    unsigned int u32MuteCtrl;
    u32MuteCtrl = 0x1;
    if (ioctl(g_s32AcodecFd, ACODEC_SET_MICL_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICL_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_DACL_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICL_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_MICR_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICR_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_DACR_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_DACR_MUTE failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ADPT_ACODEC_UnmuteAI()
{
    if (g_s32AcodecFd == -1) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "Acodec not open\n");
        return HI_FAILURE;
    }

    unsigned int u32MuteCtrl;
    u32MuteCtrl = 0x0;
    if (ioctl(g_s32AcodecFd, ACODEC_SET_MICL_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICL_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_DACL_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICL_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_MICR_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_MICR_MUTE failed\n");
        return HI_FAILURE;
    }

    if (ioctl(g_s32AcodecFd, ACODEC_SET_DACR_MUTE, &u32MuteCtrl)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "ACODEC_SET_DACR_MUTE failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
