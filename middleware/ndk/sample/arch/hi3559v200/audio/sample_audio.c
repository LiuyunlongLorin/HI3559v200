/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_audio.c
 * @brief   sample audio module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include "hi_type.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_acap.h"
#include "hi_mapi_aenc.h"
#include "hi_mapi_aenc_adpt.h"
#include "hi_mapi_ao.h"
#include "hi_mapi_disp.h"
#ifdef SUPPORT_HDMI
#include "hi_mapi_hdmi.h"
#endif
#include "mpi_sys.h"
#include "hi_common.h"
#include "hi_mapi_log.h"
#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef CHECK_RET
#define CHECK_RET(express)                                                                      \
    do {                                                                                        \
        s32Ret = express;                                                                       \
        if (s32Ret != HI_SUCCESS) {                                                             \
            printf("\nFailed at %s: LINE: %d  (ret:0x%#x!)\n", __FUNCTION__, __LINE__, s32Ret); \
            goto exit;                                                                          \
        }                                                                                       \
    } while (0)
#endif

#define SAMPLE_MAPI_AI_EXTERN_DEV 0 /** dev for extern acodec */
#define SAMPLE_MAPI_AI_INNER_DEV  0 /** dev for inner acodec */
#define SAMPLE_MAPI_AO_EXTERN_DEV 0 /** ao extern dev id */
#define SAMPLE_MAPI_AO_INNER_DEV  0 /** ao inner acodec dev id */
#define SAMPLE_MAPI_AO_HDMI_DEV   1 /** ao hdmi dev id */

#define SAMPLE_MAPI_AENC_CHN 0

#define FILE_FULL_PATH_MAX_LENGTH 64

static FILE *g_pAEncAacFile = HI_NULL;
static FILE *g_pACapRawFile[2] = { HI_NULL };

static HI_BOOL g_bACapDumpThdFlg = HI_FALSE;
static pthread_t g_ACapDumpThd = HI_NULL;

static HI_BOOL g_bSendFrmToAOChnThdFlg = HI_FALSE;
static pthread_t g_sendFrmAOChnThd = HI_NULL;

static HI_HANDLE g_acapHdl = SAMPLE_MAPI_AI_INNER_DEV;
static HI_HANDLE g_acapChnHdl = SAMPLE_MAPI_AI_CHN;
static HI_HANDLE g_aencHdl = SAMPLE_MAPI_AENC_CHN;
static HI_HANDLE g_aoHdl = SAMPLE_MAPI_AO_INNER_DEV;

HI_S32 SAMPLE_AUDIO_ConfigACapAttr(HI_MAPI_ACAP_ATTR_S *pstACapAttr)
{
    pstACapAttr->enSampleRate = AUDIO_SAMPLE_RATE_48000;
    pstACapAttr->enBitwidth = AUDIO_BIT_WIDTH_16;
    pstACapAttr->enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
    pstACapAttr->enWorkmode = AIO_MODE_I2S_MASTER;
    pstACapAttr->u32PtNumPerFrm = 1024;
    pstACapAttr->enMixerMicMode = ACODEC_MIXER_IN1;
    pstACapAttr->enResampleRate = AUDIO_SAMPLE_RATE_48000;
    pstACapAttr->enI2sType = AIO_I2STYPE_INNERCODEC;
    return HI_SUCCESS;
}

HI_S32 SAMPLE_AUDIO_ConfigAEncAttr(HI_MAPI_AENC_ATTR_S *pstAEncAttr,
    HI_MAPI_AENC_ATTR_AAC_S *pstAacAencAttr)
{
    pstAacAencAttr->enAACType = AAC_TYPE_AACLC;
    pstAacAencAttr->enBitRate = AAC_BPS_48K;
    pstAacAencAttr->enBitWidth = AUDIO_BIT_WIDTH_16;
    pstAacAencAttr->enSmpRate = AUDIO_SAMPLE_RATE_48000;
    pstAacAencAttr->enSoundMode = AUDIO_SOUND_MODE_STEREO;
    pstAacAencAttr->enTransType = AAC_TRANS_TYPE_ADTS;
    pstAacAencAttr->s16BandWidth = 0;
    pstAEncAttr->enAencFormat = HI_MAPI_AUDIO_FORMAT_AACLC;
    pstAEncAttr->u32PtNumPerFrm = 1024;
    pstAEncAttr->pValue = pstAacAencAttr;
    pstAEncAttr->u32Len = sizeof(HI_MAPI_AENC_ATTR_AAC_S);
    return HI_SUCCESS;
}

HI_S32 SAMPLE_AUDIO_ConfigAOAttr(HI_MAPI_AO_ATTR_S *pstAOAttr)
{
    pstAOAttr->enWorkmode = AIO_MODE_I2S_MASTER;
    pstAOAttr->enBitwidth = AUDIO_BIT_WIDTH_16;
    pstAOAttr->enSampleRate = AUDIO_SAMPLE_RATE_16000;
    pstAOAttr->enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_STEREO;
    pstAOAttr->u32PtNumPerFrm = 1024;
    pstAOAttr->enInSampleRate = AUDIO_SAMPLE_RATE_16000;
    pstAOAttr->enI2sType = AIO_I2STYPE_INNERCODEC;
    return HI_SUCCESS;
}

HI_S32 SAMPLE_AUDIO_DataProc(HI_HANDLE AencHdl, const AUDIO_STREAM_S *pAStreamData, HI_VOID *pPrivateData)
{
    HI_CHAR au8TempValueStr[FILE_FULL_PATH_MAX_LENGTH] = {0};
    memset(au8TempValueStr, 0, sizeof(HI_CHAR) * FILE_FULL_PATH_MAX_LENGTH);

    if (pAStreamData == HI_NULL) {
        return HI_FAILURE;
    }

    if (g_pAEncAacFile == HI_NULL) {
        snprintf(au8TempValueStr, FILE_FULL_PATH_MAX_LENGTH, "audio_stream_chn%d.%s", AencHdl, "aac");
        g_pAEncAacFile = fopen(au8TempValueStr, "a+");
        if (!g_pAEncAacFile) {
            g_pAEncAacFile = fopen(au8TempValueStr, "wb");
            printf("[%s][%d] fopen wb\n", __FUNCTION__, __LINE__);
        }
        if (!g_pAEncAacFile) {
            printf("open file failed!\n");
        }
    }

    if (g_pAEncAacFile != HI_NULL) {
        fwrite((const void *)pAStreamData->pStream, 1, pAStreamData->u32Len, g_pAEncAacFile);
        if (fflush(g_pAEncAacFile)) {
            perror("fflush file");
        }

        if (fclose(g_pAEncAacFile)) {
            perror("close file");
        }
        g_pAEncAacFile = HI_NULL;
    }
    return HI_SUCCESS;
}

HI_VOID ACapDumpThread(HI_VOID *args)
{
    HI_S32 s32Ret;
    HI_HANDLE acapHdl = g_acapHdl;
    HI_HANDLE acapChnHdl = g_acapChnHdl;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;

    prctl(PR_SET_NAME, (unsigned long)"Hi_ACapDumpThread", 0, 0, 0);
    while (g_bACapDumpThdFlg) {
        s32Ret = HI_MAPI_ACAP_GetFrame(acapHdl, acapChnHdl, &stFrm, &stAecFrm);
        if (s32Ret != HI_SUCCESS) {
            printf("HI_MAPI_ACAP_GetFrame error:0x%x\n", s32Ret);
            break;
        }

        /* only dump file 0 for mono(left or right channel),  dump file 0 and 1 for stereo */
        fwrite(stFrm.u64VirAddr[0], 1, stFrm.u32Len, g_pACapRawFile[0]);
        fwrite(stFrm.u64VirAddr[1], 1, stFrm.u32Len, g_pACapRawFile[1]);
        s32Ret = HI_MAPI_ACAP_ReleaseFrame(acapHdl, acapChnHdl, &stFrm, &stAecFrm);
        if (s32Ret != HI_SUCCESS) {
            printf("HI_MAPI_ACAP_ReleaseFrame error:0x%x\n", s32Ret);
        }
    }
    return;
}

HI_VOID SendFrameToAOChnThread(HI_VOID *args)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_HANDLE acapHdl = g_acapHdl;
    HI_HANDLE acapChnHdl = g_acapChnHdl;
    HI_HANDLE hAoHdl = g_aoHdl;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;

    prctl(PR_SET_NAME, (unsigned long)"Hi_SendFrameToAOChnThread", 0, 0, 0);
    while (g_bSendFrmToAOChnThdFlg) {
        s32Ret = HI_MAPI_ACAP_GetFrame(acapHdl, acapChnHdl, &stFrm, &stAecFrm);
        if (s32Ret != HI_SUCCESS) {
            printf("HI_MAPI_ACAP_GetFrame error:0x%x\n", s32Ret);
            break;
        }

        s32Ret = HI_MAPI_AO_SendFrame(hAoHdl, &stFrm, 500);
        if (s32Ret != HI_SUCCESS) {
            printf("HI_MAPI_AO_SendFrame error:0x%x\n", s32Ret);
        }

        s32Ret = HI_MAPI_ACAP_ReleaseFrame(acapHdl, acapChnHdl, &stFrm, &stAecFrm);
        if (s32Ret != HI_SUCCESS) {
            printf("HI_MAPI_ACAP_ReleaseFrame error:0x%x\n", s32Ret);
        }
    }
    return;
}

void SAMPLE_AUDIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo) {
        pthread_join(g_ACapDumpThd, 0);
        pthread_join(g_sendFrmAOChnThd, 0);
        HI_MAPI_Media_Deinit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_VOID SAMPLE_AUDIO_Usage(HI_VOID)
{
    printf("Usage : \n\t\033[33m[ao sample send frame from linux (big-little) only for demo.\n\tpls send frame to ao on liteos for general use]\033[0m\n");
    printf("\t 0) ACap bind AEnc and dump raw.\n");
    printf("\t 1) ACap send frame to Ao normal chn\n");
    printf("\t q/Q) quit\n");
    return;
}

HI_S32 SAMPLE_AUDIO_ACapBindAEncWithResampleDump(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;

    /* init sys */
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));

    CHECK_RET(HI_MAPI_Media_Init(&stMediaAttr));

    /* init and start acap */
    g_acapHdl = SAMPLE_MAPI_AI_INNER_DEV;
    g_acapChnHdl = SAMPLE_MAPI_AI_CHN;
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    CHECK_RET(SAMPLE_AUDIO_ConfigACapAttr(&stACapAttr));
    stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_16000; /* resample */
    CHECK_RET(HI_MAPI_ACAP_Init(g_acapHdl, &stACapAttr));
    CHECK_RET(HI_MAPI_ACAP_Start(g_acapHdl, g_acapChnHdl));

    HI_S32 s32Gain = 20;
    CHECK_RET(HI_MAPI_ACAP_SetVolume(g_acapHdl, s32Gain));
    printf("init and start acap complete\n");

    /* init and start aenc */
    CHECK_RET(HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_AACLC));

    g_aencHdl = SAMPLE_MAPI_AENC_CHN;
    HI_MAPI_AENC_ATTR_S stAencAttr;
    HI_MAPI_AENC_ATTR_AAC_S stAacAencAttr;
    CHECK_RET(SAMPLE_AUDIO_ConfigAEncAttr(&stAencAttr, &stAacAencAttr));
    CHECK_RET(HI_MAPI_AENC_Init(g_aencHdl, &stAencAttr));

    HI_MAPI_AENC_CALLBACK_S stAEncCB = { HI_NULL, HI_NULL };
    memset(&stAEncCB, 0, sizeof(HI_MAPI_AENC_CALLBACK_S));
    stAEncCB.pfnDataCB = SAMPLE_AUDIO_DataProc;
    stAEncCB.pPrivateData = HI_NULL;
    CHECK_RET(HI_MAPI_AENC_RegisterCallback(g_aencHdl, &stAEncCB));
    CHECK_RET(HI_MAPI_AENC_Start(g_aencHdl));
    printf("init and start aenc complete\n");

    /* aenc bind acap */
    CHECK_RET(HI_MAPI_AENC_BindACap(g_acapHdl, g_acapChnHdl, g_aencHdl));
    printf("acap bind aenc complete\n");

    /* acap dump raw */
    g_pACapRawFile[0] = fopen("ai_dump_ch0.raw", "wb");
    if (!g_pACapRawFile[0]) {
        printf("open file ai_dump_ch0.raw error!\n");
        goto exit;
    }

#if 1
    g_pACapRawFile[1] = fopen("ai_dump_ch1.raw", "wb");
    if (!g_pACapRawFile[1]) {
        printf("open file ai_dump_ch1.raw error!\n");
        goto exit;
    }
#endif

    g_bACapDumpThdFlg = HI_TRUE;
    s32Ret = pthread_create(&g_ACapDumpThd, HI_NULL, (HI_VOID *)ACapDumpThread, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        printf("pthread_create error!...\n");
        g_bACapDumpThdFlg = HI_FALSE;
    }
    printf("acap dump raw ...\n");

    printf("Press Enter key to stop audio record...\n");
    (void)getchar();
    if (g_bACapDumpThdFlg) {
        g_bACapDumpThdFlg = HI_FALSE;
        pthread_join(g_ACapDumpThd, HI_NULL);
    }
    printf("ACapDumpThread stop\n");

exit:

    if (g_pACapRawFile[0]) {
        fclose(g_pACapRawFile[0]);
        g_pACapRawFile[0] = HI_NULL;
    }

#if 1
    if (g_pACapRawFile[1]) {
        fclose(g_pACapRawFile[1]);
        g_pACapRawFile[1] = HI_NULL;
    }
#endif

    HI_MAPI_AENC_UnregisterCallback(g_aencHdl, &stAEncCB);
    HI_MAPI_AENC_Stop(g_aencHdl);
    HI_MAPI_AENC_Deinit(g_aencHdl);
    HI_MAPI_ACAP_Stop(g_acapHdl, g_acapChnHdl);
    HI_MAPI_ACAP_Deinit(g_acapHdl);
    CHECK_RET(HI_MAPI_Media_Deinit());

    return s32Ret;
}

HI_S32 SAMPLE_AUDIO_ACapSendFrameToAO(HI_VOID)
{
    HI_S32 s32Ret;

    /* init sys */
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));
    CHECK_RET(HI_MAPI_Media_Init(&stMediaAttr));

    /* init and start acap */
    g_acapHdl = SAMPLE_MAPI_AI_INNER_DEV;
    g_acapChnHdl = SAMPLE_MAPI_AI_CHN;
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    CHECK_RET(SAMPLE_AUDIO_ConfigACapAttr(&stACapAttr));
    stACapAttr.enSampleRate = AUDIO_SAMPLE_RATE_16000;
    stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_16000;
    CHECK_RET(HI_MAPI_ACAP_Init(g_acapHdl, &stACapAttr));
    CHECK_RET(HI_MAPI_ACAP_Start(g_acapHdl, g_acapChnHdl));

    HI_S32 s32InGain = 20;
    CHECK_RET(HI_MAPI_ACAP_SetVolume(g_acapHdl, s32InGain));
    printf("init and start acap complete\n");

    /* init and start ao */
    g_aoHdl = SAMPLE_MAPI_AO_INNER_DEV;
    HI_MAPI_AO_ATTR_S stAoAttr;
    CHECK_RET(SAMPLE_AUDIO_ConfigAOAttr(&stAoAttr));
    stAoAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

    CHECK_RET(HI_MAPI_AO_Init(g_aoHdl, &stAoAttr));
    CHECK_RET(HI_MAPI_AO_Start(g_aoHdl));

    HI_S32 s32OutGain = 0;
    CHECK_RET(HI_MAPI_AO_SetVolume(g_aoHdl, s32OutGain));
    printf("init and start ao complete\n");

    /* acap send frame to ao */
    g_bSendFrmToAOChnThdFlg = HI_TRUE;
    s32Ret = pthread_create(&g_sendFrmAOChnThd, HI_NULL, (HI_VOID *)SendFrameToAOChnThread, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        printf("pthread_create error!...\n");
        g_bSendFrmToAOChnThdFlg = HI_FALSE;
    }
    printf("Press Enter key to stop audio record...\n");
    (void)getchar();

    if (g_bSendFrmToAOChnThdFlg) {
        g_bSendFrmToAOChnThdFlg = HI_FALSE;
        pthread_join(g_sendFrmAOChnThd, HI_NULL);
    }

    printf("SendFrameThread stop\n");

exit:

    HI_MAPI_AO_Stop(g_aoHdl);
    HI_MAPI_AO_Deinit(g_aoHdl);
    HI_MAPI_ACAP_Stop(g_acapHdl, g_acapChnHdl);
    HI_MAPI_ACAP_Deinit(g_acapHdl);
    CHECK_RET(HI_MAPI_Media_Deinit());

    return s32Ret;
}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret;
    HI_CHAR ch;

    HI_MAPI_LOG_SetEnabledLevel(HI_MAPI_LOG_LEVEL_WARN);

    //    signal(SIGINT, SAMPLE_AUDIO_HandleSig);
    //    signal(SIGTERM, SAMPLE_AUDIO_HandleSig);

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Init();
    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_Sys_Init fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }
#endif

    SAMPLE_AUDIO_Usage();
    ch = (char)getchar();
    (void)getchar();
    switch (ch) {
        case '0':
            s32Ret = SAMPLE_AUDIO_ACapBindAEncWithResampleDump();
            break;
        case '1':
            s32Ret = SAMPLE_AUDIO_ACapSendFrameToAO();
            break;
        default:
            printf("the index is invaild!\n");
            SAMPLE_AUDIO_Usage();
            return HI_FAILURE;
    }

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Deinit();
#endif

    if (s32Ret == HI_SUCCESS) {
        printf("The program exited successfully!\n");
    } else {
        printf("The program exits abnormally!\n");
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
