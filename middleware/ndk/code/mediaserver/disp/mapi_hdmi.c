/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_hdmi.c
 * @brief   hdmi server function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <pthread.h>

#include "hi_mapi_comm_define.h"
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_log.h"
#include "mpi_hdmi.h"
#include "mapi_hdmi_inner.h"
#include "msg_hdmi.h"
#include "mapi_comm_inner.h"
#include "mapi_hdmi_os.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef SUPPORT_HDMI

static MAPI_HDMI_CONTEXT_S g_astHdmiCtx[HI_HDMI_ID_BUTT] = {
    {
        HI_FALSE,
        HI_FALSE,
        {
            HI_NULL,
            HI_NULL,
        },
    },
};
static pthread_mutex_t g_Hdmi_Lock[HI_HDMI_ID_BUTT] = { PTHREAD_MUTEX_INITIALIZER };

static HI_VOID MAPI_HDMI_HdmiSampleRateConvert(HI_HDMI_SAMPLE_RATE_E enHdmiSampleRate,
                                               AUDIO_SAMPLE_RATE_E *penAoSampleRate)
{
    switch (enHdmiSampleRate) {
        case HI_HDMI_SAMPLE_RATE_8K:
            *penAoSampleRate = AUDIO_SAMPLE_RATE_8000;
            break;

        case HI_HDMI_SAMPLE_RATE_16K:
            *penAoSampleRate = AUDIO_SAMPLE_RATE_16000;
            break;

        case HI_HDMI_SAMPLE_RATE_48K:
            *penAoSampleRate = AUDIO_SAMPLE_RATE_48000;
            break;

        default:
            *penAoSampleRate = AUDIO_SAMPLE_RATE_BUTT;
            break;
    }
    return;
}

static HI_VOID MAPI_HDMI_AoSampleRateConvert(AUDIO_SAMPLE_RATE_E enAoSampleRate,
                                             HI_HDMI_SAMPLE_RATE_E *penHdmiSampleRate)
{
    switch (enAoSampleRate) {
        case AUDIO_SAMPLE_RATE_8000:
            *penHdmiSampleRate = HI_HDMI_SAMPLE_RATE_8K;
            break;

        case AUDIO_SAMPLE_RATE_16000:
            *penHdmiSampleRate = HI_HDMI_SAMPLE_RATE_16K;
            break;

        case AUDIO_SAMPLE_RATE_48000:
            *penHdmiSampleRate = HI_HDMI_SAMPLE_RATE_48K;
            break;

        default:
            *penHdmiSampleRate = HI_HDMI_SAMPLE_RATE_BUTT;
            break;
    }
    return;
}

static HI_VOID MAPI_HDMI_IntfSyncConvert(VO_INTF_SYNC_E enIntfSync, HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync) {
        case VO_OUTPUT_1080P24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;

        case VO_OUTPUT_1080P25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;

        case VO_OUTPUT_1080P30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;

        case VO_OUTPUT_720P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;

        case VO_OUTPUT_720P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;

        case VO_OUTPUT_1080P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;

        case VO_OUTPUT_1080P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;

        case VO_OUTPUT_576P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
            break;

        case VO_OUTPUT_480P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
            break;

        case VO_OUTPUT_800x600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;

        case VO_OUTPUT_1024x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;

        case VO_OUTPUT_1280x1024_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;

        case VO_OUTPUT_1366x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;

        case VO_OUTPUT_1440x900_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60_RB;
            break;

        case VO_OUTPUT_1280x800_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;

        case VO_OUTPUT_1600x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;
            break;

        case VO_OUTPUT_1680x1050_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1680X1050_60;
            break;

        case VO_OUTPUT_1920x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1200_60;
            break;

        case VO_OUTPUT_640x480_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_861D_640X480_60;
            break;

        case VO_OUTPUT_1920x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1920x2160_30;
            break;

        case VO_OUTPUT_2560x1440_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1440_30;
            break;

        case VO_OUTPUT_2560x1600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1600_60;
            break;

        case VO_OUTPUT_3840x2160_24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_24;
            break;

        case VO_OUTPUT_3840x2160_25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_25;
            break;

        case VO_OUTPUT_3840x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;

        case VO_OUTPUT_3840x2160_50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_50;
            break;

        case VO_OUTPUT_3840x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;
            break;

        case VO_OUTPUT_4096x2160_24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_4096X2160P_24;
            break;

        case VO_OUTPUT_4096x2160_25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_4096X2160P_25;
            break;

        case VO_OUTPUT_4096x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_4096X2160P_30;
            break;

        case VO_OUTPUT_4096x2160_50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_4096X2160P_50;
            break;

        case VO_OUTPUT_4096x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_4096X2160P_60;
            break;

        case VO_OUTPUT_USER:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE;
            break;

        default:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_BUTT;
            break;
    }
    return;
}

static HI_VOID MAPI_HDMI_VideoFmtConvert(HI_HDMI_VIDEO_FMT_E enVideoFmt, VO_INTF_SYNC_E *penIntfSync)
{
    switch (enVideoFmt) {
        case HI_HDMI_VIDEO_FMT_1080P_60:
            *penIntfSync = VO_OUTPUT_1080P60;
            break;

        case HI_HDMI_VIDEO_FMT_1080P_50:
            *penIntfSync = VO_OUTPUT_1080P50;
            break;

        case HI_HDMI_VIDEO_FMT_1080P_30:
            *penIntfSync = VO_OUTPUT_1080P30;
            break;

        case HI_HDMI_VIDEO_FMT_1080P_25:
            *penIntfSync = VO_OUTPUT_1080P25;
            break;

        case HI_HDMI_VIDEO_FMT_1080P_24:
            *penIntfSync = VO_OUTPUT_1080P24;
            break;

        case HI_HDMI_VIDEO_FMT_720P_60:
            *penIntfSync = VO_OUTPUT_720P60;
            break;

        case HI_HDMI_VIDEO_FMT_720P_50:
            *penIntfSync = VO_OUTPUT_720P50;
            break;

        case HI_HDMI_VIDEO_FMT_576P_50:
            *penIntfSync = VO_OUTPUT_576P50;
            break;

        case HI_HDMI_VIDEO_FMT_480P_60:
            *penIntfSync = VO_OUTPUT_480P60;
            break;

        case HI_HDMI_VIDEO_FMT_861D_640X480_60:
            *penIntfSync = VO_OUTPUT_640x480_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_800X600_60:
            *penIntfSync = VO_OUTPUT_800x600_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1024X768_60:
            *penIntfSync = VO_OUTPUT_1024x768_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1280X800_60:
            *penIntfSync = VO_OUTPUT_1280x800_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1280X1024_60:
            *penIntfSync = VO_OUTPUT_1280x1024_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1366X768_60:
            *penIntfSync = VO_OUTPUT_1366x768_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1440X900_60_RB:
            *penIntfSync = VO_OUTPUT_1440x900_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1600X1200_60:
            *penIntfSync = VO_OUTPUT_1600x1200_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1680X1050_60:
            *penIntfSync = VO_OUTPUT_1680x1050_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_1920X1200_60:
            *penIntfSync = VO_OUTPUT_1920x1200_60;
            break;

        case HI_HDMI_VIDEO_FMT_2560x1440_30:
            *penIntfSync = VO_OUTPUT_2560x1440_30;
            break;

        case HI_HDMI_VIDEO_FMT_2560x1600_60:
            *penIntfSync = VO_OUTPUT_2560x1600_60;
            break;

        case HI_HDMI_VIDEO_FMT_1920x2160_30:
            *penIntfSync = VO_OUTPUT_1920x2160_30;
            break;

        case HI_HDMI_VIDEO_FMT_3840X2160P_24:
            *penIntfSync = VO_OUTPUT_3840x2160_24;
            break;

        case HI_HDMI_VIDEO_FMT_3840X2160P_25:
            *penIntfSync = VO_OUTPUT_3840x2160_25;
            break;

        case HI_HDMI_VIDEO_FMT_3840X2160P_30:
            *penIntfSync = VO_OUTPUT_3840x2160_30;
            break;

        case HI_HDMI_VIDEO_FMT_3840X2160P_50:
            *penIntfSync = VO_OUTPUT_3840x2160_50;
            break;

        case HI_HDMI_VIDEO_FMT_3840X2160P_60:
            *penIntfSync = VO_OUTPUT_3840x2160_60;
            break;

        case HI_HDMI_VIDEO_FMT_4096X2160P_24:
            *penIntfSync = VO_OUTPUT_4096x2160_24;
            break;

        case HI_HDMI_VIDEO_FMT_4096X2160P_25:
            *penIntfSync = VO_OUTPUT_4096x2160_25;
            break;

        case HI_HDMI_VIDEO_FMT_4096X2160P_30:
            *penIntfSync = VO_OUTPUT_4096x2160_30;
            break;

        case HI_HDMI_VIDEO_FMT_4096X2160P_50:
            *penIntfSync = VO_OUTPUT_4096x2160_50;
            break;

        case HI_HDMI_VIDEO_FMT_4096X2160P_60:
            *penIntfSync = VO_OUTPUT_4096x2160_60;
            break;

        case HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE:
            *penIntfSync = VO_OUTPUT_USER;
            break;

        default:
            *penIntfSync = VO_OUTPUT_BUTT;
            break;
    }

    return;
}

static HI_VOID MAPI_HDMI_VideoFmtCapConvert(const HI_BOOL abVideoFormat[], HI_BOOL abIntfSync[])
{
    HI_HDMI_VIDEO_FMT_E enHdmiVideoFmt;
    VO_INTF_SYNC_E enVoIntfSync;

    for (enVoIntfSync = 0; enVoIntfSync < VO_OUTPUT_BUTT; enVoIntfSync++) {
        abIntfSync[enVoIntfSync] = HI_FALSE;
    }

    for (enHdmiVideoFmt = 0; enHdmiVideoFmt < HI_HDMI_VIDEO_FMT_BUTT; enHdmiVideoFmt++) {
        if (abVideoFormat[enHdmiVideoFmt]) {
            MAPI_HDMI_VideoFmtConvert(enHdmiVideoFmt, &enVoIntfSync);
            if (enVoIntfSync != VO_OUTPUT_BUTT) {
                abIntfSync[enVoIntfSync] = HI_TRUE;
            }
        }
    }

    return;
}

static HI_VOID MAPI_HDMI_HdmiSampleRateCapConvert(const HI_HDMI_SAMPLE_RATE_E aenHdmiSampleRatecap[],
                                                  AUDIO_SAMPLE_RATE_E aenAoSampleratecap[])
{
    HI_S32 i, j;
    j = 0;
    for (i = 0; i < HI_HDMI_MAX_SAMPE_RATE_NUM; i++) {
        switch (aenHdmiSampleRatecap[i]) {
            case HI_HDMI_SAMPLE_RATE_8K:
                aenAoSampleratecap[j] = AUDIO_SAMPLE_RATE_8000;
                j++;
                break;

            case HI_HDMI_SAMPLE_RATE_16K:
                aenAoSampleratecap[j] = AUDIO_SAMPLE_RATE_16000;
                j++;
                break;

            case HI_HDMI_SAMPLE_RATE_48K:
                aenAoSampleratecap[j] = AUDIO_SAMPLE_RATE_48000;
                j++;
                break;
            default:
                break;
        }

        if (j >= HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM) {
            break;
        }
    }

    for (i = j; i < HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM; i++) {
        aenAoSampleratecap[i] = AUDIO_SAMPLE_RATE_BUTT;
    }

    return;
}

static HI_VOID MAPI_HDMI_GetAOSampleRateCap(HI_HDMI_AUDIO_INFO_S stAudioInfo[],
                                            AUDIO_SAMPLE_RATE_E aenSupportSampleRate[])
{
    HI_S32 i, j;
    HI_BOOL bSupportAudio = HI_FALSE;

    for (i = 0; i < HI_HDMI_MAX_AUDIO_CAP_COUNT; i++) {
        if (stAudioInfo[i].enAudFmtCode == HI_HDMI_AUDIO_FORMAT_CODE_PCM) {
            for (j = 0; j < HI_HDMI_MAX_BIT_DEPTH_NUM; j++) {
                if (stAudioInfo[i].enSupportBitDepth[j] == HI_HDMI_BIT_DEPTH_16) {
                    MAPI_HDMI_HdmiSampleRateCapConvert(stAudioInfo[i].enSupportSampleRate, aenSupportSampleRate);
                    bSupportAudio = HI_TRUE;
                    break;
                }
            }
            break;
        }
    }

    if (!bSupportAudio) {
        for (i = 0; i < HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM; i++) {
            aenSupportSampleRate[i] = AUDIO_SAMPLE_RATE_BUTT;
        }
    }

    return;
}

extern HI_BOOL MAPI_HDMI_Check_Attr(const HI_HDMI_ATTR_S *pstGetHdmiAttr,
                                    const HI_MAPI_HDMI_ATTR_S *pstSetHdmiAttr,
                                    const HI_HDMI_VIDEO_FMT_E enVideoFmt, const HI_HDMI_SAMPLE_RATE_E enHdmiSampleRate)
{
    if ((pstSetHdmiAttr->bEnableHdmi == pstGetHdmiAttr->bEnableHdmi) &&
        (pstSetHdmiAttr->bEnableVideo == pstGetHdmiAttr->bEnableVideo) &&
        (enVideoFmt == pstGetHdmiAttr->enVideoFmt) && (pstSetHdmiAttr->bEnableAudio == pstGetHdmiAttr->bEnableAudio) &&
        (enHdmiSampleRate == pstGetHdmiAttr->enSampleRate)) {
        if ((enVideoFmt != HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE) ||
            ((enVideoFmt == HI_HDMI_VIDEO_FMT_VESA_CUSTOMER_DEFINE) &&
             (pstSetHdmiAttr->u32PixClk == pstGetHdmiAttr->u32PixClk))) {
            return HI_TRUE;
        }
    }

    return HI_FALSE;
}

HI_S32 HI_MAPI_HDMI_Init(HI_HDMI_ID_E enHdmi, const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc)
{
    HI_S32 s32Ret;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallBackFN;

    HDMI_CHECK_HANDLE(enHdmi);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);

    if (g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has been init, do not need to init again!\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_HDMI_Init();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Init 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    s32Ret = HI_MPI_HDMI_Open(enHdmi);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Open 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        goto EXIT_2;
    }

    s32Ret = HAL_MAPI_HDMI_Config_CallbackFunc(pstHdmiCallBackFunc, &stHdmiCallBackFN);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HAL_MAPI_HDMI_Config_CallbackFunc 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        goto EXIT_1;
    }

    s32Ret = HI_MPI_HDMI_RegCallbackFunc(enHdmi, &stHdmiCallBackFN);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_RegCallbackFunc 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        goto EXIT_1;
    }

    memcpy(&g_astHdmiCtx[enHdmi].stHdmiCBFunc, &stHdmiCallBackFN, sizeof(HI_HDMI_CALLBACK_FUNC_S));

    g_astHdmiCtx[enHdmi].bHdmiInited = HI_TRUE;
    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return s32Ret;

EXIT_1:

    s32Ret = HI_MPI_HDMI_Close(enHdmi);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Close 0x%x ERR.\n", s32Ret);
    }

EXIT_2:

    s32Ret = HI_MPI_HDMI_DeInit();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_DeInit 0x%x ERR.\n", s32Ret);
    }

    return HI_FAILURE;
}

HI_S32 HI_MAPI_HDMI_Deinit(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;

    HDMI_CHECK_HANDLE(enHdmi);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    s32Ret = HI_MPI_HDMI_UnRegCallbackFunc(enHdmi, &g_astHdmiCtx[enHdmi].stHdmiCBFunc);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_UnRegCallbackFunc 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    s32Ret = HI_MPI_HDMI_Close(enHdmi);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Close 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }
    s32Ret = HI_MPI_HDMI_DeInit();
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_DeInit 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    memset(&g_astHdmiCtx[enHdmi].stHdmiCBFunc, 0, sizeof(HI_HDMI_CALLBACK_FUNC_S));
    g_astHdmiCtx[enHdmi].bHdmiInited = HI_FALSE;
    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return s32Ret;
}

HI_S32 HI_MAPI_HDMI_GetSinkCapability(HI_HDMI_ID_E enHdmi,
    HI_MAPI_HDMI_SINKCAPABILITY_S *pstsinkcapabiility)
{
    HI_S32 s32Ret;
    VO_INTF_SYNC_E IntfSync;
    HI_HDMI_SINK_CAPABILITY_S stSinkCap;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstsinkcapabiility);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    memset(&stSinkCap, 0, sizeof(HI_HDMI_SINK_CAPABILITY_S));
    s32Ret = HI_MPI_HDMI_GetSinkCapability(enHdmi, &stSinkCap);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_GetSinkCapability 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    pstsinkcapabiility->bConnected = stSinkCap.bConnected;
    pstsinkcapabiility->bIsSinkPowerOn = stSinkCap.bIsSinkPowerOn;
    pstsinkcapabiility->bSupportHdmi = stSinkCap.bSupportHdmi;

    /* native sync */
    MAPI_HDMI_VideoFmtConvert(stSinkCap.enNativeVideoFormat, &IntfSync);
    pstsinkcapabiility->enNativeIntfSync = IntfSync;

    /* sync capbility */
    MAPI_HDMI_VideoFmtCapConvert(stSinkCap.bVideoFmtSupported, pstsinkcapabiility->abIntfSync);

    /* aosample rate capbility */
    MAPI_HDMI_GetAOSampleRateCap(stSinkCap.stAudioInfo, pstsinkcapabiility->aenSupportSampleRate);

    pstsinkcapabiility->bSupportHdmi_2_0 = stSinkCap.bSupportHdmi_2_0;

    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return s32Ret;
}

HI_S32 HI_MAPI_HDMI_GetAttr(HI_HDMI_ID_E enHdmi, HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S sthdmiattr;
    VO_INTF_SYNC_E enHdmisync;
    AUDIO_SAMPLE_RATE_E enHdmiSampleRate;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstHdmiAttr);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    memset(&sthdmiattr, 0, sizeof(HI_HDMI_ATTR_S));
    s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &sthdmiattr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_GetAttr 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    MAPI_HDMI_VideoFmtConvert(sthdmiattr.enVideoFmt, &enHdmisync);
    MAPI_HDMI_HdmiSampleRateConvert(sthdmiattr.enSampleRate, &enHdmiSampleRate);

    pstHdmiAttr->bEnableHdmi = sthdmiattr.bEnableHdmi;
    pstHdmiAttr->bEnableVideo = sthdmiattr.bEnableVideo;
    pstHdmiAttr->enHdmiSync = enHdmisync;
    pstHdmiAttr->bEnableAudio = sthdmiattr.bEnableAudio;
    pstHdmiAttr->enSampleRate = enHdmiSampleRate;
    pstHdmiAttr->u32PixClk = sthdmiattr.u32PixClk;

    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_E enHdmi, const HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;
    HI_HDMI_SAMPLE_RATE_E enHdmiSampleRate;

    HDMI_CHECK_HANDLE(enHdmi);
    HDMI_CHECK_NULL_PTR(pstHdmiAttr);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    MAPI_HDMI_IntfSyncConvert(pstHdmiAttr->enHdmiSync, &enVideoFmt);
    MAPI_HDMI_AoSampleRateConvert(pstHdmiAttr->enSampleRate, &enHdmiSampleRate);

    memset(&stHdmiAttr, 0, sizeof(HI_HDMI_ATTR_S));
    s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_GetAttr 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    if (g_astHdmiCtx[enHdmi].bHdmiStarted) {
        if (MAPI_HDMI_Check_Attr(&stHdmiAttr, pstHdmiAttr, enVideoFmt, enHdmiSampleRate)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI Attr is not change, return successful\n\n");
            MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
            return HI_SUCCESS;
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has started and does not support setting HDMI Attr\n");
            MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
            return HI_MAPI_HDMI_ENOT_SUPPORT;
        }
    }

    if (pstHdmiAttr->bEnableHdmi) {
        stHdmiAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;
        stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
        stHdmiAttr.bEnableAviInfoFrame = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;
    } else {
        stHdmiAttr.enDefaultMode = HI_HDMI_FORCE_DVI;
        stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_RGB444;
        stHdmiAttr.bEnableAviInfoFrame = HI_FALSE;
        stHdmiAttr.bEnableAudInfoFrame = HI_FALSE;
    }
    if (enHdmiSampleRate == HI_HDMI_SAMPLE_RATE_BUTT) {
        stHdmiAttr.enSampleRate = HI_HDMI_SAMPLE_RATE_48K;
    } else {
        stHdmiAttr.enSampleRate = enHdmiSampleRate;
    }

    stHdmiAttr.bEnableHdmi = pstHdmiAttr->bEnableHdmi;
    stHdmiAttr.bEnableVideo = pstHdmiAttr->bEnableVideo;
    stHdmiAttr.enVideoFmt = enVideoFmt;
    stHdmiAttr.bEnableAudio = pstHdmiAttr->bEnableAudio;
    stHdmiAttr.bxvYCCMode = HI_FALSE;
    stHdmiAttr.enOutCscQuantization = HDMI_QUANTIZATION_LIMITED_RANGE;
    stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stHdmiAttr.bIsMultiChannel = HI_FALSE;
    stHdmiAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;
    stHdmiAttr.bEnableSpdInfoFrame = HI_FALSE;
    stHdmiAttr.bEnableMpegInfoFrame = HI_FALSE;
    stHdmiAttr.bDebugFlag = HI_FALSE;
    stHdmiAttr.bHDCPEnable = HI_FALSE;
    stHdmiAttr.b3DEnable = HI_FALSE;
    stHdmiAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;

    s32Ret = HI_MPI_HDMI_SetAttr(enHdmi, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_SetAttr 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_Start(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;

    HDMI_CHECK_HANDLE(enHdmi);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    if (g_astHdmiCtx[enHdmi].bHdmiStarted) {
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_HDMI_Start(enHdmi);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Start 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    g_astHdmiCtx[enHdmi].bHdmiStarted = HI_TRUE;
    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_HDMI_Stop(HI_HDMI_ID_E enHdmi)
{
    HI_S32 s32Ret;
    HDMI_CHECK_HANDLE(enHdmi);

    MUTEX_LOCK(g_Hdmi_Lock[enHdmi]);
    if (!g_astHdmiCtx[enHdmi].bHdmiInited) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HDMI has not been inited\n");
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_MAPI_HDMI_ENOTREADY;
    }

    if (!g_astHdmiCtx[enHdmi].bHdmiStarted) {
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_HDMI_Stop(enHdmi);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "HI_MPI_HDMI_Stop 0x%x ERR.\n", s32Ret);
        MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);
        return s32Ret;
    }

    g_astHdmiCtx[enHdmi].bHdmiStarted = HI_FALSE;
    MUTEX_UNLOCK(g_Hdmi_Lock[enHdmi]);

    return HI_SUCCESS;
}

#else

HI_S32 HI_MAPI_HDMI_Init(HI_HDMI_ID_E enHdmi, const HI_HDMI_CALLBACK_FUNC_S *pstHdmiCallBackFunc)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_Deinit(HI_HDMI_ID_E enHdmi)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_GetSinkCapability(HI_HDMI_ID_E enHdmi,
    HI_MAPI_HDMI_SINKCAPABILITY_S *pstsinkcapabiility)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_GetAttr(HI_HDMI_ID_E enHdmi, HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_E enHdmi, const HI_MAPI_HDMI_ATTR_S *pstHdmiAttr)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_Start(HI_HDMI_ID_E enHdmi)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

HI_S32 HI_MAPI_HDMI_Stop(HI_HDMI_ID_E enHdmi)
{
    MAPI_ERR_TRACE(HI_MAPI_MOD_HDMI, "Not support HDMI.\n");
    return HI_MAPI_HDMI_ENOT_SUPPORT;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
